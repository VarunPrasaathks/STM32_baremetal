/*This project is to experiment toggle mode of general purpose timer in STM32F103C8T6;
 * We are going to use TIM2 channel 1 for this purpose.
 */

#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define TIM2_BASE 0x40000000UL
#define GPIOA_BASE 0x40010800UL

//used register addresses
#define APB2_ENR            REG32((RCC_BASE + 0x18UL))  //for port a
#define APB1_ENR            REG32((RCC_BASE + 0x1CUL))  //for TIM2

#define TIM2_CR1            REG32((TIM2_BASE + 0x00))
#define TIM2_DIER           REG32((TIM2_BASE + 0x0C))
#define TIM2_SR 			REG32((TIM2_BASE + 0x10))
#define TIM2_EGR			REG32((TIM2_BASE + 0x14))
#define TIM2_CCMR1			REG32((TIM2_BASE + 0x18))
#define TIM2_CCER			REG32((TIM2_BASE + 0x20))
#define TIM2_CNT			REG32((TIM2_BASE + 0x24))
#define TIM2_PSC			REG32((TIM2_BASE + 0x28))
#define TIM2_ARR			REG32((TIM2_BASE + 0x2C))
#define TIM2_CCR1			REG32((TIM2_BASE + 0x34))

#define GPIOA_CRL 			REG32((GPIOA_BASE + 0x00UL))

int main(){
	//configure clocks for TIM2 and Port A
	APB1_ENR = (1U << 0);
	APB2_ENR = (1U << 2);

	//led at PA0 as AF o/p push pull. PA0 is the AF pin for channel 1 of timer 2
	GPIOA_CRL &= ~(0xF << 0);
	GPIOA_CRL |= (0xA << 0);

	TIM2_PSC = 7999;  //7999+1  =8000. So 8/8000 = 1KHz i.e, 1 ms ticks
	TIM2_ARR = 999; //999 + 1  = 1000;; so 1ms * 1000 = 1s = 1Hz frequency of pulses (Period = 1s)

	TIM2_CCR1 = 250; // this value is the one that will get compared with CNT value.
	//when CNT = CCRx an event will be performed. here let us toggle the pin at this condition

	TIM2_CCMR1 &= ~(3U << 0); //the first two bits are set as 00 to config channel 1 as output
	TIM2_CCMR1 |= (1U << 3); //to enable preload register
	TIM2_CCMR1 |= (3U << 4); //this is the bit config for toggle mode. (bits 6:4 control channel 1 mode)

	TIM2_CCER |= (1U << 0); //enable channel 1

	TIM2_EGR |= (1U << 0); 	//generate update event using EGR

	//Enable auto-reload preload and start counter
	TIM2_CR1 |= (1 << 7);      // ARPE = 1
	TIM2_CR1 |= (1 << 0);      // CEN = 1

	while(1){

	}
	return 0;
}
