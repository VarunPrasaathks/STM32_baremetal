/*This project is to experiment gated mode feature of a slave timer in STM32F103C8T6;
 * In this configuration, we can dictate when the slave timer's counter can be enabled.
 * It can be disabled when the master timers pulse of on/off.
 * Here the master is TIM2 and the slave is TIM3.
 * LEDs are used to monitor the high and lowtimes at PA0(for TIM2) and PA6(for TIM3)
 */

#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define TIM2_BASE 0x40000000UL
#define GPIOA_BASE 0x40010800UL
#define TIM3_BASE 0x40000400UL
#define TIM4_BASE 0x40000800UL


//used register addresses
#define APB2_ENR            REG32((RCC_BASE + 0x18UL))  //for port a
#define APB1_ENR            REG32((RCC_BASE + 0x1CUL))  //for TIM2

#define TIM2_CR1            REG32((TIM2_BASE + 0x00))
#define TIM2_CR2			REG32((TIM2_BASE + 0x04))
#define TIM2_DIER           REG32((TIM2_BASE + 0x0C))
#define TIM2_SR 			REG32((TIM2_BASE + 0x10))
#define TIM2_EGR			REG32((TIM2_BASE + 0x14))
#define TIM2_CCMR1			REG32((TIM2_BASE + 0x18))
#define TIM2_CCER			REG32((TIM2_BASE + 0x20))
#define TIM2_CNT			REG32((TIM2_BASE + 0x24))
#define TIM2_PSC			REG32((TIM2_BASE + 0x28))
#define TIM2_ARR			REG32((TIM2_BASE + 0x2C))
#define TIM2_CCR1			REG32((TIM2_BASE + 0x34))

#define TIM3_CR1            REG32((TIM3_BASE + 0x00))
#define TIM3_CR2			REG32((TIM3_BASE + 0x04))
#define TIM3_DIER           REG32((TIM3_BASE + 0x0C))
#define TIM3_SR 			REG32((TIM3_BASE + 0x10))
#define TIM3_EGR			REG32((TIM3_BASE + 0x14))
#define TIM3_CCMR1			REG32((TIM3_BASE + 0x18))
#define TIM3_CCER			REG32((TIM3_BASE + 0x20))
#define TIM3_CNT			REG32((TIM3_BASE + 0x24))
#define TIM3_PSC			REG32((TIM3_BASE + 0x28))
#define TIM3_ARR			REG32((TIM3_BASE + 0x2C))
#define TIM3_CCR1			REG32((TIM3_BASE + 0x34))
#define TIM3_SMCR			REG32((TIM3_BASE + 0x08))

#define GPIOA_CRL 			REG32((GPIOA_BASE + 0x00UL))

int main(){
	//configure clocks for TIM2 and Port A
	APB1_ENR |= (1U << 0) | (1U << 1);  //TIM2 and TIM3
	APB2_ENR = (1U << 2); //port A

	//led at PA0 as AF o/p push pull. PA0 is the AF pin for channel 1 of timer 2
	GPIOA_CRL &= ~(0xF << 0);
	GPIOA_CRL |= (0xB << 0);

	TIM2_PSC = 7999;  //7999+1  =8000. So 8/8000 = 1KHz i.e, 1 ms ticks
	TIM2_ARR = 999; //999 + 1  = 1000;; so 1ms * 1000 = 1s = 1Hz frequency of pulses (Period = 1s)

	TIM2_CCMR1 |=  (6 << 4); //normal PWM
	TIM2_CCMR1 |= (1 << 3); //Output compare 1 preload enable

	TIM2_CCER |= (1U << 0); //enable channel 1 of TIM2

	TIM2_CCR1 = 500; //duty cycle control

	TIM2_CR2 |= (4U << 4);

	TIM2_EGR |= 1;
	TIM2_CR1 |= (1U << 0);

	// PA6 = AF Push-Pull (TIM3_CH1)
	GPIOA_CRL &= ~(0xF << (6 * 4));     // clear PA6 config
	GPIOA_CRL |=  (0xB << (6 * 4));     // MODE=11 (50MHz), CNF=10 (AF PP)

	TIM3_PSC = 7999;       // 1 ms tick
	TIM3_ARR = 499;        // 1000 ms
	TIM3_CCMR1 &= ~(7 << 4);
	TIM3_CCMR1 |=  (3 << 4);   // OC1 toggle mode
	TIM3_CCMR1 &= ~(3 << 0);

	TIM3_CCER |= (1 << 0);     // Enable CH1 output on PA6

	// TS = 001 (trigger = TIM2). SMCR is the slave mode configuration register
	TIM3_SMCR &= ~(7 << 4);
	TIM3_SMCR |=  (1 << 4);

	TIM3_SMCR &= ~(7 << 0);
	TIM3_SMCR |=  (5 << 0);   // SMS = 101 gated mode

	TIM3_CR1 |= 1;

	while(1){

	}

	return 0;

}
