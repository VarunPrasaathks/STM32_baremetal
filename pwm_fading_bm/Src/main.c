/* this code is to implement fading of LEDs using PWM. We generate 1khz pulse in TIM2 and modify its duty cycle to the
 * max value and bring it down to 0 and then back to max
 * The output pin is PA0 which is the AF pin for TIM2
 */

#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define TIM2_BASE 0x40000000UL
#define GPIOA_BASE 0x40010800UL
#define NVIC_ISER0 (*(volatile uint32_t*)0xE000E100UL) //this is from cortex M3 architecture Nested Vector Interrupt Controller_Interrrupt Set Enable Register. address taken from m3 ref manual

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

int duty = 0;
int step = 1;
void TIM2_IRQHandler();
int main(){
	//initialise clocks for tim2 and port a
	APB1_ENR = (1U << 0);
	APB2_ENR = (1U << 2);

	//led at PA0 as o/p push pull
	GPIOA_CRL &= ~(0xF << 0);
	GPIOA_CRL |= (0xA << 0);

	TIM2_PSC = 7;  //Prescaler.    7+1  =8MHz. So 8/8 = 1MHz i.e, 1 us ticks
	TIM2_ARR = 999; //Auto reload register.    999 + 1  = 1000;; so 1us * 1000 = 1ms = 1kHz frequency of pulses
	TIM2_DIER |= (1U << 0);

	TIM2_CCMR1 |=  (6 << 4);   //sets PWM in mode 1.which means channel is high till CNT < CCR1
	TIM2_CCMR1 |= (1 << 3);  //preload on TIM2_CCR1 is enabled. the value gets updated after each update event

	//enable channel 1
	TIM2_CCER |= (1U << 0);

	//genreate update event using EGR
	TIM2_EGR |= (1U << 0);
	NVIC_ISER0 |= (1 << 28);  //for generating interrupt from TIM2 at overflow. IRQ for TIM2 is 28 (from ref manual)

	//Enable auto-reload preload and start counter
	 TIM2_CR1 |= (1 << 7);      // ARPE = 1
	 TIM2_CR1 |= (1 << 0);      // CEN = 1

	while(1){

	}
	 return 0;
}

void TIM2_IRQHandler(){
	if(TIM2_SR & 1){ //check if timer has overflowed
		TIM2_SR &= ~1; //make the update flag to 0

		duty += step;

        if (duty >= 999) step = -1; //max constrain
        if (duty <= 0)   step = 1; //min constrain

        TIM2_CCR1 = duty;   //value will vary between 0 to 1000 which means 0 to 100 in duty cycle
	}
}
