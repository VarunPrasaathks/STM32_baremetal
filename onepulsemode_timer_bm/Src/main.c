/* This project is to implement one pulse mode feature of General Purpose timer(TIM2) in STM32F103C8T6.
 * There is a push button at PA0. After 2 seconds of pressing the push button, led lights up for 3 seconds then turns off
 */

#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define TIM2_BASE 0x40000000UL
#define GPIOA_BASE 0x40010800UL

#define AFIO_BASE 0x40010000UL   //to enable alternate fn of a GPIO
#define EXTI_BASE 0x40010400UL //to map GPIO interrupts to CPU
#define NVIC_ISER0 0xE000E100UL //this is from cortex M3 architecture Nested Vector Interrupt Controller_Interrrupt Set Enable Register. address taken from m3 ref manual


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
#define GPIOA_IDR 			REG32((GPIOA_BASE + 0x08UL))
#define GPIOA_ODR 			REG32((GPIOA_BASE + 0x0CUL))

//EXTI registers
#define EXTI_IMR     REG32((EXTI_BASE + 0x00UL))
#define EXTI_EMR     REG32((EXTI_BASE + 0x04UL))
#define EXTI_RTSR    REG32((EXTI_BASE + 0x08UL))
#define EXTI_FTSR    REG32((EXTI_BASE + 0x0CUL))
#define EXTI_SWIER   REG32((EXTI_BASE + 0x10UL))
#define EXTI_PR      REG32((EXTI_BASE + 0x14UL))

//AFIO_EXTICR1 ---> EXTI Config Register 1 ---> for configuring interrupt lines 0 to 3
#define AFIO_EXTICR1 REG32(((AFIO_BASE)+ 0x08))


// NVIC bit for EXTI1 on Cortex-M3 (IRQ7)  -  from reference manual, will be mentioned as position
#define EXTI1_NVIC_BIT (1U << 7)

// EXTI1 ISR
void EXTI1_IRQHandler(void) {
	// 1. Clear EXTI pending flag
	EXTI_PR = (1U << 1);

	// 2. Reset CNT to 0
	TIM2_CNT = 0;

	// 3. Generate update event to load PSC, ARR, CCR registers
	TIM2_EGR |= (1U << 0);   // UG = 1

	// 4. Start timer (CEN = 1)
	TIM2_CR1 |= (1U << 0);   // CEN = 1
}

int main(){
	//initialise clocks for tim2 and port a
	APB1_ENR |= (1U << 0);  // tim 2 clock
	APB2_ENR |= (1U << 0);  //AFIO clock
	APB2_ENR |= (1U << 2);  //Port a clock

	//led at PA0 as AF
	GPIOA_CRL &= ~(0xF << 0);
	GPIOA_CRL |= (0xA << 0);

	//config push button at PA1 as input
	GPIOA_CRL &= ~(0xF << 4);
	GPIOA_CRL |= (0x8U << 4);
    GPIOA_ODR |= (1U << 1);   //for input pullup

	//map PA1 to EXTI1
	AFIO_EXTICR1 &= ~(0xFUL <<4);
	AFIO_EXTICR1 |= (0x0UL <<4);

	//configure EXTI1
	EXTI_IMR  |= (1U << 1);   /* unmask EXTI0 */
	EXTI_RTSR &= ~(1U << 1);  /* disable rising */
	EXTI_FTSR |=  (1U << 1);  /* enable falling */
	EXTI_PR   =  (1U << 1);  /* clear pending */

	//enable EXTI1 in NVIC_ISER0
	REG32(NVIC_ISER0) |= (1U<<7);  // can also be mentioned as EXTI1_NVIC_BIT

	TIM2_PSC = 7999;  //to generate 1ms ticks
	TIM2_ARR = 5000;  //5000 * 1ms ticks = 5 sec. I want to generate counters running from 0 to 5 seconds

	//one pulse mode setting
	TIM2_CR1 |= (1U << 3);

	TIM2_CCMR1 |= (6U << 4);   // OC1M = 110 (PWM1 mode)
	TIM2_CCMR1 |= (1U << 3);   // OC1PE = 1 (output preload enable)

	//enable tim2 channel 1 (PA0)
	TIM2_CCER |= (1U << 0);

	TIM2_CCER |= (1U << 1);  // CC1P = 1 (invert output)

	TIM2_CCR1 = 2000;

	//generate update event using EGR
	TIM2_EGR |= (1U << 0);

	while(1){

	}


	return 0;

}
