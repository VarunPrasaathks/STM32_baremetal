#include<stdint.h>

#define RCC_BASE 0x40021000UL
#define GPIOC_BASE 0x40011000UL

//derived from main
#define GPIOC_CRH (GPIOC_BASE + 0x04UL)
#define GPIOC_BSRR (GPIOC_BASE + 0x10UL)
#define GPIOC_ODR (GPIOC_BASE + 0x0CUL)
#define RCC_APB2ENR (RCC_BASE + 0x18UL)

#define REG32(addr) (*(volatile uint32_t *)(addr))


static void delay(volatile uint32_t count){
	while(count--){
		__asm__ volatile ("nop");
	}
}


int main(){
	REG32(RCC_APB2ENR) |= 1U << 4;
	delay(1000);

	REG32(GPIOC_CRH) &= ~(0xF << 20);
	REG32(GPIOC_CRH) |= (0x1U << 20);

	while(1){
		REG32(GPIOC_BSRR) = (1U << (13 + 16));
		delay(360000);
		REG32(GPIOC_BSRR) = (1U << 13);
		delay(360000);
	}
	return 0;
}
