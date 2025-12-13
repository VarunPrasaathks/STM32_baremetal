/*This code implements RX concepts of UART to make a simple learning project.
 * commands will be sent from serial monitor to mcu and the output is observed.
 * If we get the desired results, our communication is correct
 */

#include<stdint.h>
#include <string.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define USART1_BASE 0x40013800UL

//used register address
#define APB2_ENR            REG32((RCC_BASE + 0x18UL))  //for port a, USART

#define GPIOA_CRL 			REG32((GPIOA_BASE + 0x00UL))
#define GPIOA_CRH 			REG32((GPIOA_BASE + 0x04UL))
#define GPIOA_ODR 			REG32((GPIOA_BASE + 0x0CUL))

#define USART1_SR			REG32((USART1_BASE + 0x00UL))
#define USART1_DR			REG32((USART1_BASE + 0x04UL))
#define USART1_BRR			REG32((USART1_BASE + 0x08UL))
#define USART1_CR1			REG32((USART1_BASE + 0x0CUL))

uint8_t rx_index = 0;
char rx_buffer[32];

uint8_t read_fn(void){
		while(!(USART1_SR & 1 << 5));
		return (uint8_t)USART1_DR;
	}

void control_fn(char *cmd){
	if (strcmp(cmd, "LED ON") == 0)
	        GPIOA_ODR |= (1 << 0);

	    else if (strcmp(cmd, "LED OFF") == 0)
	        GPIOA_ODR &= ~(1 << 0);

	    else if (strcmp(cmd, "LED TOGGLE") == 0)
	        GPIOA_ODR ^= (1 << 0);
}
int main(){
	//configure clocks for port a and usart1
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

	USART1_CR1 |= (1U << 13);  //UART enable
	USART1_CR1 |= (1U << 2);  //receiver enable

	while(1){
		char c = read_fn();

		if(c == '\r' || c == '\n'){
			rx_buffer[rx_index] = '\0';
			control_fn(rx_buffer);
			rx_index = 0;
		}
		else{
			rx_buffer[rx_index++] = c;
			if (rx_index >= 31) rx_index = 0;
		}

	}
}
