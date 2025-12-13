/*This code is to understand the basic working of UART.
 * The registers are configured and a simple message is sent. Output is viewed via serial monitor
 */

#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define USART1_BASE 0x40013800UL

//used register address
#define APB2_ENR            REG32((RCC_BASE + 0x18UL))  //for port a, USART

#define GPIOA_CRH 			REG32((GPIOA_BASE + 0x04UL))


#define USART1_SR			REG32((USART1_BASE + 0x00UL))
#define USART1_DR			REG32((USART1_BASE + 0x04UL))
#define USART1_BRR			REG32((USART1_BASE + 0x08UL))
#define USART1_CR1			REG32((USART1_BASE + 0x0CUL))

int main(){
	//configure clocks for port a and usart1
	APB2_ENR |= (1 << 2);
	APB2_ENR |= (1 << 14);

	//set PA9 (TX of USART1) as AF o/p
	GPIOA_CRH &= ~(0xF << 4);
	GPIOA_CRH |= (0xB << 4);

	//set baud rate at 9600 for 8MHz clk freq. calculation given after main() fn.
	USART1_BRR = 0x341;

	USART1_CR1 |= (1U << 13);  //UART enable and transmitter enable
	USART1_CR1 |= (1U << 3);

	while(1){
		while(!(USART1_SR & (1 <<7)));
		USART1_DR = 'v';

		while(!(USART1_SR & (1 <<7)));
		USART1_DR = 'a';

		while(!(USART1_SR & (1 <<7)));
		USART1_DR = 'r';

		while(!(USART1_SR & (1 <<7)));
		USART1_DR = 'u';

		while(!(USART1_SR & (1 <<7)));
		USART1_DR = 'n';

		while(!(USART1_SR & (1 << 7)));
		USART1_DR = '\n';

		for(volatile int i=0; i<200000; i++);
	}
}

/*
 * Value, x = (Freq of main clock) / (16 * desired baud rate)
 * 			= (8)/(16 * 9600)  ~ 52.0833
 *
 * 			Here, mantissa = 52 .
 * 			multiply decimal part by 16, 0.0833 * 16 = 1.33  ~ 1 (rounded off to nearest integer)
 *
 * 			BRR = (Mantissa << 4) | decimal part approximation
 * 				(0x34 << 4) | (0x1)
 *
 * 			BRR = (0x340) + (0x1) = (0x341)
 */

