//want to implement hardware timer in baremetal. configure the registers to blink an LED every 1 second


#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define TIM2_BASE 0x40001000UL
#define GPIOC_BASE 0x40011000UL


//used register addresses
#define APB1_ENR (RCC_BASE + 0x1CUL)
#define APB2_ENR (RCC_BASE + 0x18UL)

#define TIM6_CR1 (TIM2_BASE + 0x00UL)
#define TIM6_CR2 (TIM2_BASE + 0x04UL)
#define TIM6_DIER (TIM2_BASE + 0x0CUL)
#define TIM6_SR (TIM2_BASE + 0x10UL)
#define TIM6_EGR (TIM2_BASE + 0x14UL)
#define TIM6_CNT (TIM2_BASE + 0x24UL)
#define TIM6_PSC (TIM2_BASE + 0x28UL)
#define TIM6_ARR (TIM2_BASE + 0x2CUL)


#define GPIOC_CRH (GPIOC_BASE + 0x04UL)
#define GPIOC_ODR (GPIOC_BASE + 0x0CUL)

int main(){
	//enable clocks for timer, port A
	REG32(APB1_ENR) |= (1U<<4); //tim6
	REG32(APB2_ENR) |= (1U<<4); //port c


	//led configured for o/p push pull
	REG32(GPIOC_CRH) &= ~(0xF<<20);
	REG32(GPIOC_CRH) |= (0x1U<<20);

	//configure the timer for
	REG32(TIM6_PSC) = 7;  //main clock at 8MHz
	REG32(TIM6_ARR) = 999;

	REG32(TIM6_EGR) |= 1;   // UG

	REG32(TIM6_CR1) |= (1U << 0);
	uint32_t counter = 0;

	while(1){
		if(REG32(TIM6_SR) & 1){
			REG32(TIM6_SR) = 0;
			counter++;
		}

		if(counter>=1000){
			REG32(GPIOC_ODR) ^= (1U << 13);
			counter = 0;
		}
	}
}


