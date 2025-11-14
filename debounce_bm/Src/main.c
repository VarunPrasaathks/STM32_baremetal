#include <stdio.h>
#include <stdbool.h> // Required for using 'bool', 'true', and 'false'
#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t*)(addr))

//base adress of registers
#define GPIOA_BASE 0x40010800UL
#define GPIOC_BASE 0x40011000UL
#define RCC_BASE 0x40021000UL

//required register address'
#define RCC_APB2ENR (RCC_BASE + 0x18UL)
#define GPIOC_CRH (GPIOC_BASE + 0x04UL)
#define GPIOC_BSRR (GPIOC_BASE + 0x10UL)
#define GPIOA_CRL (GPIOA_BASE + 0x00UL)
#define GPIOA_IDR (GPIOA_BASE + 0x08UL)

//variables declaration
bool button_pressed = false;
bool last_state = false;
bool current_state = false;

int main(){
	// clocks enabling for port A and C
	REG32(RCC_APB2ENR) |= (1U<<4) | (1U<<2);

	//configure PC13 as output led
	REG32(GPIOC_CRH) &= ~(0xF<<20);
	REG32(GPIOC_CRH) |= (0x1U<<20);

	//button at A0 configured for input pullup/pull down
	REG32(GPIOA_CRL) &= ~(0xF<<0);
	REG32(GPIOA_CRL) |= (0x8U<<0);

	while(1){
		bool raw = (REG32(GPIOA_IDR)>>0 & 1U);
		current_state = !raw;
		if(current_state && !(last_state)){
			button_pressed = !button_pressed;

			if(button_pressed){
				REG32(GPIOC_BSRR) = (1U << (13));
			}
			else{
				REG32(GPIOC_BSRR) = (1U << (13 + 16));
			}
		}
		last_state = current_state;
	}
}



