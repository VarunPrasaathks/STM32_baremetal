//this code implements timers(TIM6) with interrupts, moving on from polling

#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define TIM2_BASE 0x40000000UL
#define GPIOC_BASE 0x40011000UL
#define NVIC_ISER0 (*(volatile uint32_t*) 0xE000E100UL) //this is from cortex M3 architecture Nested Vector Interrupt Controller_Interrrupt Set Enable Register. address taken from m3 ref manual
//#define NVIC_ISER1 (*(volatile uint32_t*)0xE000E104)


//used register addresses
#define APB1_ENR            REG32((RCC_BASE + 0x1CUL))  //for TIM6
#define APB2_ENR            REG32((RCC_BASE + 0x18UL))  //for port c

#define TIM2_CR1            REG32((TIM2_BASE + 0x00UL))
#define TIM2_CR2            REG32((TIM2_BASE + 0x04UL))
#define TIM2_DIER           REG32((TIM2_BASE + 0x0CUL))
#define TIM2_SR             REG32((TIM2_BASE + 0x10UL))
#define TIM2_EGR            REG32((TIM2_BASE + 0x14UL))
#define TIM2_CNT            REG32((TIM2_BASE + 0x24UL))
#define TIM2_PSC            REG32((TIM2_BASE + 0x28UL))
#define TIM2_ARR            REG32((TIM2_BASE + 0x2CUL))


#define GPIOC_CRH          REG32((GPIOC_BASE + 0x04UL))
#define GPIOC_ODR          REG32((GPIOC_BASE + 0x0CUL))

void TIM6_IRQHandler(void);

int main()
{
	//initialise clocks for tim6 and port c
	APB1_ENR = (1U << 0);
	APB2_ENR = (1U << 4);

	//set PC13 as LED o/p push pull
	REG32(GPIOC_CRH) &= ~(0xF<<20);
	REG32(GPIOC_CRH) |= (0x1U<<20);

	//configure timer
	TIM2_PSC = 7;
	TIM2_ARR = 999;

	TIM2_EGR |= 1;

	TIM2_DIER |= (1U << 0);

	NVIC_ISER0 |= (1 << 28);
	//NVIC_ISER1 |= (1 << 22);   // enable index at 70 - 16 =54. 54 will be at ISER1. 54-32 = 22

	TIM2_CR1 |= (1U << 0);


	while (1){

	}
	return 0;
}

void TIM2_IRQHandler(void){
	if (TIM2_SR & (1U << 0)) {
	        // Toggle the LED on PC13
	        GPIOC_ODR ^= (1U << 13);
	        // Clear the update interrupt flag (write 0 to the bit)
	        TIM2_SR &= ~(1U << 0);
	    }
}
