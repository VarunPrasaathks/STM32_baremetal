#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))


#define RCC_BASE 0x40021000UL
#define GPIOC_BASE 0x40011000UL
#define GPIOA_BASE 0x40010800UL

#define RCC_APB2ENR (RCC_BASE + 0x18UL)
#define GPIOC_CRH (GPIOC_BASE + 0x04UL)
#define GPIOC_BSRR (GPIOC_BASE + 0x10UL)
#define GPIOA_CRL (GPIOA_BASE + 0x00UL)
#define GPIOA_IDR (GPIOA_BASE + 0x08UL)
#define GPIOA_ODR (GPIOA_BASE + 0x0CUL)

static void delay(volatile uint32_t count){
	while(count--){
		__asm__ volatile ("nop");
	}
}

int main(){
	//ports A and C are enabled
	REG32(RCC_APB2ENR) |= (1U<<4) | (1U<<2);
	delay(1000);

	//led configured for o/p push pull
	REG32(GPIOC_CRH) &= ~(0xF<<20);
	REG32(GPIOC_CRH) |= (0x1U<<20);

	//button at A0 configured for input pullup/pull down
	REG32(GPIOA_CRL) &= ~(0xF<<0);
	REG32(GPIOA_CRL) |= (0x8U<<0);
	//REG32(GPIOA_ODR) &= ~(1U << 0); //for pullup/down, ODR should be set to 0

	while(1){
		//reading the state of button
		if(REG32(GPIOA_IDR)>>0 & 1U){
			REG32(GPIOC_BSRR) = (1U << (13 + 16));
		}
		else{
			REG32(GPIOC_BSRR) = (1U << (13));

		}

	}

return 0;
}
