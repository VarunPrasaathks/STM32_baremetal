/*This code is made to send simple UART messages and view them via a logic analyzer. Just now, I bought a 8 Channel Logic Analyzer and installed the Logic 2 software by saleae.
 * Want to test it and see the frame in the graphical format
 */

#include<stdint.h>
#include<string.h>
#include <stdbool.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define USART1_BASE 0x40013800UL

#define APB2_ENR            REG32((RCC_BASE + 0x18UL))  //for port a, USART
#define NVIC_ISER1 			REG32(0xE000E104UL) 		//this is from cortex M3 architecture Nested Vector Interrupt Controller_Interrrupt Set Enable Register. address taken from m3 ref manual

#define GPIOA_CRL 			REG32((GPIOA_BASE + 0x00UL))
#define GPIOA_CRH 			REG32((GPIOA_BASE + 0x04UL))
#define GPIOA_ODR 			REG32((GPIOA_BASE + 0x0CUL))
#define GPIOA_IDR 			REG32((GPIOA_BASE + 0x08UL))


#define USART1_SR			REG32((USART1_BASE + 0x00UL))
#define USART1_DR			REG32((USART1_BASE + 0x04UL))
#define USART1_BRR			REG32((USART1_BASE + 0x08UL))
#define USART1_CR1			REG32((USART1_BASE + 0x0CUL))

//variables declaration
bool button_pressed = false;
bool last_state = false;
bool current_state = false;

int main(){
	//config clocks for port A,USART
	APB2_ENR |= (1 << 2);
	APB2_ENR |= (1 << 14);

	//button at A0 configured for input pullup/pull down
	GPIOA_CRL &= ~(0xF<<0);
	GPIOA_CRL |= (0x8U<<0);

	GPIOA_ODR |= (1<<0);   // enable pull-up

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
		USART1_CR1 |= (1U << 3);   //transmitter enable

		USART1_CR1 |= (1U << 13);  //UART enable

		while(1){
			bool raw = ((GPIOA_IDR)>>0 & 1U);
			current_state = !raw;
			if(current_state && !(last_state)){
				button_pressed = !button_pressed;

				if(button_pressed){
					while(!(USART1_SR & (1 <<7)));
						USART1_DR = 'v';
				}

			}
			last_state = current_state;
		}
}


/*when I check this in logic analyser, I see the 8N1 signal of 'v' visually. It served as a good way to reinforce the concept.
letter 'v' in ASCII: 118(decimal), 0x76(hexadecimal)....
01110110 in binary.
UART sends the LSB first. So the sequence is 0(start bit) | 0 1 1 0 1 1 1 0 | 1(stop bit).
before the start of transmission, the line will be 1(HIGH). start bit pulls it 0, marking the start of data reception.
*/

