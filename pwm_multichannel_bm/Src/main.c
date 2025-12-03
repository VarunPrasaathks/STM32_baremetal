/*
 *
 */

#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define TIM2_BASE 0x40000000UL
#define GPIOA_BASE 0x40010800UL
#define AFIO_BASE  0x40010000UL

//used register addresses
#define APB2_ENR            REG32((RCC_BASE + 0x18UL))  //for port a
#define APB1_ENR            REG32((RCC_BASE + 0x1CUL))  //for TIM2

#define TIM2_CR1            REG32((TIM2_BASE + 0x00))
#define TIM2_DIER           REG32((TIM2_BASE + 0x0C))
#define TIM2_SR 			REG32((TIM2_BASE + 0x10))
#define TIM2_EGR			REG32((TIM2_BASE + 0x14))
#define TIM2_CCMR1			REG32((TIM2_BASE + 0x18))
#define TIM2_CCMR2			REG32((TIM2_BASE + 0x1C))
#define TIM2_CCER			REG32((TIM2_BASE + 0x20))
#define TIM2_CNT			REG32((TIM2_BASE + 0x24))
#define TIM2_PSC			REG32((TIM2_BASE + 0x28))
#define TIM2_ARR			REG32((TIM2_BASE + 0x2C))
#define TIM2_CCR1			REG32((TIM2_BASE + 0x34))
#define TIM2_CCR2			REG32((TIM2_BASE + 0x38))
#define TIM2_CCR3			REG32((TIM2_BASE + 0x3C))
#define TIM2_CCR4			REG32((TIM2_BASE + 0x40))

#define GPIOA_CRL 			REG32((GPIOA_BASE + 0x00UL))
#define GPIOA_ODR 			REG32((GPIOA_BASE + 0x0CUL))
#define AFIO_MAPR			REG32((AFIO_BASE + 0x04UL))


int main(){
	//initialise clocks for tim2 and port a
	APB1_ENR |= (1U << 0);  // tim 2 clock
	APB2_ENR |= (1U << 0);  //AFIO clock
	APB2_ENR |= (1U << 2);  //Port a clock

	//led at PA0,1,2,3 as AF
	GPIOA_CRL &= ~(0xFFFF << 0);
	GPIOA_CRL |= (0xAAAA << 0);


	//clear afio remap register
	AFIO_MAPR &= ~(3 << 8);


	TIM2_PSC = 7;  //7+1  =8MHz. So 8/8 = 1MHz i.e, 1 us ticks
	TIM2_ARR = 999; //999 + 1  = 1000;; so 1us * 1000 = 1ms = 1kHz frequency of pulses

	//a timers' prescaler and arr value remains the same for all channels, the PWM can be varied
	TIM2_CCR1 = 100; //duty cycle of channel 1
	TIM2_CCR2 = 200; //channel 2
	TIM2_CCR3 = 999; //channel 3
	TIM2_CCR4 = 1000; //channel 4

	TIM2_CCMR1 |=  (6 << 4) | (6 << 12);  //channel 1 and 2 set to PWM 1 mode
	TIM2_CCMR1 |= (1 << 3) | (1 << 11);  //preload enable in channels 1 and 2

	TIM2_CCMR2 |=  (6 << 4) | (6 << 12);  //channel 3 and 4 set to PWM 1 mode
	TIM2_CCMR2 |= (1 << 3) | (1 << 11);  //preload enable in channels 3 and 4

	//enable channel 1 ,2, 3 and 4
	TIM2_CCER |= (1U << 0) | (1U << 4) | (1U << 8) | (1U << 12);

	//generate update event using EGR
	TIM2_EGR |= (1U << 0);

	//Enable auto-reload preload and start counter
	 TIM2_CR1 |= (1 << 7);      // ARPE = 1
	 TIM2_CR1 |= (1 << 0);      // CEN = 1

	 while(1){

	 }
}
