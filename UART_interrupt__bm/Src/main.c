/*This project is to make UART RX run via interrupts.
 * Previously, did UART RX using polling just to understand the basics of its working. But it is a bad practice.
 * Data reception can be managed via interrupts and it can be buffered so that load on CPU is reduced.
 */

#include<stdint.h>
#include<string.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define USART1_BASE 0x40013800UL

//used register address
#define APB2_ENR            REG32((RCC_BASE + 0x18UL))  //for port a, USART
#define NVIC_ISER1 			REG32(0xE000E104UL) 		//this is from cortex M3 architecture Nested Vector Interrupt Controller_Interrrupt Set Enable Register. address taken from m3 ref manual


#define GPIOA_CRL 			REG32((GPIOA_BASE + 0x00UL))
#define GPIOA_CRH 			REG32((GPIOA_BASE + 0x04UL))
#define GPIOA_ODR 			REG32((GPIOA_BASE + 0x0CUL))

#define USART1_SR			REG32((USART1_BASE + 0x00UL))
#define USART1_DR			REG32((USART1_BASE + 0x04UL))
#define USART1_BRR			REG32((USART1_BASE + 0x08UL))
#define USART1_CR1			REG32((USART1_BASE + 0x0CUL))

char rx_buffer[32];
volatile int indx = 0;
volatile int cmd_ready = 0;
void USART1_IRQHandler(){
	if (USART1_SR & (1 << 5))   // RXNE
	    {
	        char c = USART1_DR;

	        if (c == '\r')          // ignore CR
	           return;
	        if (c == '\n')          // end of command
	        {
	            rx_buffer[indx] = '\0';
	            cmd_ready = 1;      // flag to signal main loop
	            indx = 0;
	        }
	        else if (indx < sizeof(rx_buffer) - 1)
	        {
	            rx_buffer[indx++] = c;
	        }
	    }
}

int main(){
	//config clock for portA, USART
	APB2_ENR |= (1 << 2);
	APB2_ENR |= (1 << 14);

	//configure PA0 as output push pull
	GPIOA_CRL &= ~(0xF << 0);
	GPIOA_CRL |= (0x1 << 0);

	//set PA9 (TX of USART1) as AF o/p
	GPIOA_CRH &= ~(0xF << 4);
	GPIOA_CRH |= (0xB << 4);

	//config PA10 (RX of USART1) as floating input
	GPIOA_CRH &= ~(0xF << 8);
	GPIOA_CRH |= (0x4 << 8);

	//set baud rate at 9600 for 8MHz clk freq.
	USART1_BRR = 0x341;

	USART1_CR1 |= (1U << 2);  //receiver enable
	USART1_CR1 |= (1U << 5);  //RX interrupt enable
	USART1_CR1 |= (1U << 13);  //UART enable

	NVIC_ISER1 |= (1 << 5); //enable USART interrupt

	while(1){
		if(cmd_ready){
			cmd_ready = 0;

			if(strcmp(rx_buffer,"LED ON") == 0){
				GPIOA_ODR |= (1 << 0);
			}
			else if(strcmp(rx_buffer,"LED OFF") == 0){
				GPIOA_ODR &= ~(1 << 0);

			}
		}
	}
}
