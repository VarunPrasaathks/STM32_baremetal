//this code is to understand pwm generation in stm32f103c8t6. we have used TIM2 and the output is shown at PA0
// PA0 is the alternate function pin for TIM2 (internally multiplexed). So no need to map TIM2 and PA0 in code
// ensure to configure PA0 as 'alternate function' push pull

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
#define GPIOA_ODR 			REG32((GPIOA_BASE + 0x0CUL))

int main(){
	//initialise clocks for tim2 and port a
	APB1_ENR = (1U << 0);
	APB2_ENR = (1U << 2);

	//led at PA0 as AF o/p push pull
	GPIOA_CRL &= ~(0xF << 0);
	GPIOA_CRL |= (0xA << 0);

	TIM2_PSC = 7;  //7+1  =8MHz. So 8/8 = 1MHz i.e, 1 us ticks
	TIM2_ARR = 999; //999 + 1  = 1000;; so 1us * 1000 = 1ms = 1kHz frequency of pulses

	TIM2_CCR1 = 10; //(1% duty cycle)

	TIM2_CCMR1 |=  (6 << 4);
	TIM2_CCMR1 |= (1 << 3);

	//enable channel 1
	TIM2_CCER |= (1U << 0);

	//genreate update event using EGR
	TIM2_EGR |= (1U << 0);

	//Enable auto-reload preload and start counter
	 TIM2_CR1 |= (1 << 7);      // ARPE = 1
	 TIM2_CR1 |= (1 << 0);      // CEN = 1

	 while(1){

	 }
}
