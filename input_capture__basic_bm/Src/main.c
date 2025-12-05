/*This project is to implement input capture feature in STM32F103C8T6.
 * I don't have a DSO right now to see the captured waveform so I thought of this small Project.
 * One channel of a timer will generate PWM which will be captured by another timer.
 * This captured value will be used to light an LED according to the received duty cycle.
 * By varying the duty cycle, I can know if input capture is working or not
 */

#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define GPIOB_BASE 0x40010C00UL
#define TIM2_BASE 0x40000000UL
#define TIM3_BASE 0x40000400UL
#define TIM4_BASE 0x40000800UL


#define NVIC_ISER0  REG32(0xE000E100UL)

//used registers
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

#define TIM3_CR1            REG32((TIM3_BASE + 0x00))
#define TIM3_DIER           REG32((TIM3_BASE + 0x0C))
#define TIM3_SR 			REG32((TIM3_BASE + 0x10))
#define TIM3_EGR			REG32((TIM3_BASE + 0x14))
#define TIM3_CCMR1			REG32((TIM3_BASE + 0x18))
#define TIM3_CCER			REG32((TIM3_BASE + 0x20))
#define TIM3_CNT			REG32((TIM3_BASE + 0x24))
#define TIM3_PSC			REG32((TIM3_BASE + 0x28))
#define TIM3_ARR			REG32((TIM3_BASE + 0x2C))
#define TIM3_CCR1			REG32((TIM3_BASE + 0x34))
#define TIM3_CCR2			REG32((TIM3_BASE + 0x38))

#define TIM4_CR1   			REG32(TIM4_BASE + 0x00)
#define TIM4_CCMR1 			REG32(TIM4_BASE + 0x18)
#define TIM4_CCER  			REG32(TIM4_BASE + 0x20)
#define TIM4_PSC   			REG32(TIM4_BASE + 0x28)
#define TIM4_ARR   			REG32(TIM4_BASE + 0x2C)
#define TIM4_CCR1  			REG32(TIM4_BASE + 0x34)
#define TIM4_EGR   			REG32(TIM4_BASE + 0x14)

#define GPIOA_CRL 			REG32((GPIOA_BASE + 0x00UL))
#define GPIOA_ODR 			REG32((GPIOA_BASE + 0x0CUL))
#define GPIOB_CRL  			REG32((GPIOB_BASE + 0x00UL))


volatile uint32_t rise_time = 0;
volatile uint32_t fall_time = 0;
volatile uint32_t period = 0;
volatile uint32_t high_time = 0;
volatile uint32_t old_rise = 0;

void TIM3_IRQHandler(void)
{
    // Rising edge
    if (TIM3_SR & (1 << 1))
    {
        rise_time = TIM3_CCR1;

        if (rise_time >= old_rise)
            period = rise_time - old_rise;
        else
            period = (0xFFFF - old_rise) + rise_time;

        old_rise = rise_time;
        TIM3_SR &= ~(1 << 1);
    }

    // Falling edge
    if (TIM3_SR & (1 << 2))
    {
        fall_time = TIM3_CCR2;

        if (fall_time >= rise_time)
            high_time = fall_time - rise_time;
        else
            high_time = (0xFFFF - rise_time) + fall_time;

        TIM3_SR &= ~(1 << 2);

        if (period > 0)
        {
            uint32_t duty_raw = (high_time * 1000) / period;

            if (duty_raw > 1000) duty_raw = 1000;  // clamp

            TIM4_CCR1 = duty_raw; // LED brightness
        }
    }
}


int main(){
	//configure clocks for TIM2, TIM3 and Port A
	APB1_ENR |= (1U << 0) | (1U << 1);   //Timer 2 and Timer 3
	APB2_ENR |= (1U << 2);

	//configure PA0 , PA6 as AF o/p push pull pin. PA0 is CH1 of TIM2 , PA6 is CH1 of TIM3 .
	//we are going to use TIM3 channel 1(PA6) for input capture
	GPIOA_CRL &= ~((0xF << (0 * 4)) | (0xF << (6 * 4)));
	GPIOA_CRL |= (0xB << (0 * 4)); 	//PA0 = AF push pull (0b1011 = 0xB)
	GPIOA_CRL |= (0x4 << (6 * 4)); //PA6 = floating input (0b0100 = 0x4)

	//interrupt enable
	NVIC_ISER0 |= (1 << 29);   // TIM3 IRQ number = 29

	TIM2_PSC = 7;  //7+1  =8MHz. So 8/8 = 1MHz i.e, 1 us ticks
	TIM2_ARR = 999; //999 + 1  = 1000;; so 1us * 1000 = 1ms = 1kHz frequency of pulses

	TIM2_CCR1 = 250; //(duty cycle)

	TIM2_CCMR1 |=  (6 << 4);
	TIM2_CCMR1 |= (1 << 3);

	//enable channel 1
	TIM2_CCER |= (1U << 0);

	//generate update event using EGR
	TIM2_EGR |= (1U << 0);

	//Enable auto-reload preload and start counter
	 TIM2_CR1 |= (1 << 7) | (1 << 0);      // ARPE = 1 & CEN = 1


	 //setting up TIM3 for input capture
	 TIM3_PSC = 7;
	 TIM3_ARR = 0xFFFF;

	 // CC1S = 01 → CC1 channel mapped to TI1 (input)
	 TIM3_CCMR1 &= ~(3 << 0) | ~(3 << 8);
	 TIM3_CCMR1 |= (1U << 0) | (2U << 8);  //channel 1 for rising edge and 2 for falling edge

	 TIM3_CCMR1 &= ~(0xF << 4) | ~(0xF << 12); // filters of input capture 1 and 2 are kept 0 (no filter)

	 TIM3_CCER  &= ~(1 << 1);  //rising edge
	 TIM3_CCER  |=  (1 << 0);  // CC1E = enable

	 TIM3_CCER  |=  (1 << 5);  // CC2P = falling (CC2P bit)
	 TIM3_CCER  |=  (1 << 4);  // CC2E = enable

	 TIM3_DIER |= (1 << 1); // CC1IE rising
	 TIM3_DIER |= (1 << 2); // CC2IE falling

	 TIM3_CR1 |= (1 << 0);   // Enable Timer 3

	 //now we have generated PWM on TIM2(PA0) and capture it in PA6(using input capture in TIM3)
	 // the next step will be to use the TIM3 captured value to drive an LED
	 // we can use TIM4 to drive it. TIM4_CH1 = PB6

	 //enable clocks of port B and TIM4 clock
	 APB2_ENR |= (1 << 3);
	 APB1_ENR |= (1 << 2);

	 //configure PB6 as AF push pull
	 GPIOB_CRL &= ~(0xF << (6 * 4));
	 GPIOB_CRL |=  (0xB << (6 * 4));

	 TIM4_PSC = 7;       // 1 MHz timer
	 TIM4_ARR = 999;     // 0–999 = 1000 steps (same as TIM2)
	 TIM4_CCR1 = 0;      // initially LED OFF

	 // PWM Mode 1 on CH1
	 TIM4_CCMR1 |= (6 << 4);  // OC1M = 110 (PWM mode 1)
	 TIM4_CCMR1 |= (1 << 3);  // OC1PE preload enable

	 TIM4_CCER |= (1 << 0);  // Enable CH1 output

	 TIM4_EGR |= (1 << 0);  	 // Update event

	 TIM4_CR1 |= (1 << 7) | (1 << 0);   // ARPE + CEN
	 while(1){

	 }


}
