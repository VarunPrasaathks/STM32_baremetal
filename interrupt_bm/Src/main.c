// execute the concept of interrupt. initially, inbuilt led of stm will be blinking, when push button is pressed, falling edge of the press will trigger an interrupt
// ISR: an external led will blink 5 times during this time inbuilt led will be off. after this the normal inbuilt LED blinking will continue

#include<stdint.h>
#include<stddef.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//define the base registers
#define RCC_BASE 0x40021000UL
#define GPIOC_BASE 0x40011000UL
#define GPIOA_BASE 0x40010800UL
#define AFIO_BASE 0x40010000UL   //to enable alternate fn of a GPIO
#define EXTI_BASE 0x40010400UL //to map GPIO interrupts to CPU
#define NVIC_ISER0 0xE000E100UL //this is from cortex M3 architecture Nested Vector Interrupt Controller_Interrrupt Set Enable Register. address taken from m3 ref manual


//define the registers which will be required in the code
//peripherals
#define RCC_APB2ENR (RCC_BASE + 0x18UL)
#define GPIOC_CRH (GPIOC_BASE + 0x04UL)
#define GPIOC_BSRR (GPIOC_BASE + 0x10UL)
#define GPIOA_CRL (GPIOA_BASE + 0x00UL)
#define GPIOA_IDR (GPIOA_BASE + 0x08UL)
#define GPIOA_ODR (GPIOA_BASE + 0x0CUL)
#define GPIOA_BSRR (GPIOA_BASE + 0x10UL)

//AFIO_EXTICR1 ---> EXTI Config Register 1 ---> for configuring interrupt lines 0 to 3
#define AFIO_EXTICR1 ((AFIO_BASE)+ 0x08)

//EXTI registers
#define EXTI_IMR     (EXTI_BASE + 0x00UL)
#define EXTI_EMR     (EXTI_BASE + 0x04UL)
#define EXTI_RTSR    (EXTI_BASE + 0x08UL)
#define EXTI_FTSR    (EXTI_BASE + 0x0CUL)
#define EXTI_SWIER   (EXTI_BASE + 0x10UL)
#define EXTI_PR      (EXTI_BASE + 0x14UL)

// NVIC bit for EXTI0 on Cortex-M3 (IRQ6)  -  from reference manual, will be mentioned as position
#define EXTI0_NVIC_BIT (1U << 6)

static void delay(volatile uint32_t count){
	while(count--){
		__asm__ volatile ("nop");
	}
}

static void led_toggle(void){
	for(int i=0; i<5; i++){
		REG32(GPIOA_BSRR) = (1U<< (1));
		delay(30000);
		REG32(GPIOA_BSRR) = (1U<< (1 + 16));
		delay(30000);
	}
}
// EXTI0 ISR
void EXTI0_IRQHandler(void) {
	REG32(EXTI_IMR) &= ~(1U << 0);

    delay(5000);
    REG32(GPIOC_BSRR) = (1U<< 13);
    led_toggle();

    // clear EXTI pending flag for line 0 by writing 1
    REG32(EXTI_PR) = (1U << 0);

    REG32(EXTI_IMR) |= (1U << 0);
}

int main(){
	REG32(RCC_APB2ENR) |= (1U<<0) | (1U<<2) | (1U<<4);

	//button at A0 configured for input pullup
	REG32(GPIOA_CRL) &= ~(0xF<<0);
	REG32(GPIOA_CRL) |= (0x8U<<0);
    REG32(GPIOA_ODR) |= (1U << 0);   //for input pullup

	//LED is connected at A1 which has to be configured as output push pull. after that PC13 also config
	REG32(GPIOA_CRL) &= ~(0xF<<4);
	REG32(GPIOA_CRL) |= (0x1U<<4);
	REG32(GPIOC_CRH) &= ~(0xF<<20);
	REG32(GPIOC_CRH) |= (0x1U<<20);

	//turn off PA1 initially. Active Low
	REG32(GPIOA_BSRR) = (0U<< 1);

	//map PA0 to EXTI0
	REG32(AFIO_EXTICR1) &= ~(0xFUL <<0);
	REG32(AFIO_EXTICR1) |= (0x0UL <<0);

	//configure EXTI0
	REG32(EXTI_IMR)  |= (1U << 0);   /* unmask EXTI0 */
	REG32(EXTI_RTSR) &= ~(1U << 0);  /* disable rising */
	REG32(EXTI_FTSR) |=  (1U << 0);  /* enable falling */
	REG32(EXTI_PR)    =  (1U << 0);  /* clear pending */

	//enable EXTI0 in NVIC_ISER0
	REG32(NVIC_ISER0) |= (1U<<6);  // can also be mentioned as EXTI0_NVIC_BIT

	while (1) {
		REG32(GPIOC_BSRR) = (1U << (13 + 16));
		delay(360000);
		REG32(GPIOC_BSRR) = (1U << 13);
		delay(360000);
		//__asm__ volatile ("wfi");
	}

	return 0;

}
