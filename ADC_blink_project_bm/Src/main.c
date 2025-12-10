/*This project is to control the blinking speed of LED via a potentiometer using ADC.
 * When the potentiometer outputs minimal voltage, the blink speed is very slow. as the potentiometer output increases, the blink speed increases.
 */

#include<stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define TIM2_BASE 0x40000000UL
#define GPIOA_BASE 0x40010800UL
#define ADC1_BASE 0x40012400UL

//used register addresses
#define APB2_ENR            REG32((RCC_BASE + 0x18UL))  //for port a, ADC1
#define APB1_ENR            REG32((RCC_BASE + 0x1CUL))  //for TIM2
#define NVIC_ISER0 			REG32(0xE000E100UL) 		//this is from cortex M3 architecture Nested Vector Interrupt Controller_Interrrupt Set Enable Register. address taken from m3 ref manual


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
#define TIM2_CCR2   		REG32((TIM2_BASE + 0x38))

#define ADC1_CR1     		REG32((ADC1_BASE + 0x04))
#define ADC1_CR2     		REG32((ADC1_BASE + 0x08))
#define ADC1_SMPR2    		REG32((ADC1_BASE + 0X10))
#define ADC1_SQR1     		REG32((ADC1_BASE + 0x2C))
#define ADC1_SQR3    		REG32((ADC1_BASE + 0x34))
#define ADC1_DR       		REG32((ADC1_BASE + 0x4C))
#define ADC1_SR      		REG32((ADC1_BASE + 0x00))


#define GPIOA_CRL 			REG32((GPIOA_BASE + 0x00UL))
volatile uint16_t adc_value = 0;

void ADC1_2_IRQHandler(){
	if (ADC1_SR & (1 << 1))      // check whether EOC flag set?
		{
		    adc_value = ADC1_DR;
		    ADC1_SR &= ~(1 << 1);

		    TIM2_PSC = 20000 - ((adc_value * 19800) / 4095); // Here the range of PSC varies from 200 to 20000 for the range of ADC between 0 to 4095 (12bit ADC)
		}
}

int main(){
	//configure clocks for TIM2 and Port A
	APB1_ENR = (1U << 0);  //tim2
	APB2_ENR = (1U << 2)| (1 << 9);  //port a and ADC1

	//enable interrupt
	NVIC_ISER0 |= (1 << 18);

	//PA0 should be configured as analog input(potentiometer)
	GPIOA_CRL &= ~(0xF << 0);

	//PA1 is the output pin of TIM2_ch2. So configure it as AF push pull o/p
	GPIOA_CRL &= ~(0xF << 4);   // Clear PA1 config
	GPIOA_CRL |= (0xB << 4);

	//Configure ADC1//
	ADC1_SMPR2 &= ~(0x7 << 0);     // Set the sampling time for channel 0(PA0) : 55.5 cycles.
	ADC1_SMPR2 |=  (0x5 << 0);	   // bits[2:0] control the sampling time for channel 0. Set it as 101 for 55.5 cycles

	ADC1_SQR1 &= ~(0xF << 20);   //This is the sequence length. Seq = No of channels to be sampled - 1; Here set this to 0
	ADC1_SQR3 &= ~(0x1F << 0); //bits[4:0] control the first sequence of ADC. Here it is set to 0 since we are sampling channel 0;
	ADC1_SQR3 |=  (0 << 0);

	ADC1_CR1 |= (1 << 5);  //Interrupt enable for EOC(End of Conversion)
    ADC1_CR2 |= (1 << 1);   // Continuous conversion mode for continuous sampling
    ADC1_CR2 |= (1 << 0);  //turn ON ADC

    ADC1_CR2 |= (1 << 2);  // start calibration
    while (ADC1_CR2 & (1 << 2));   //wait for calibration to complete. after completion hardware sets the bit to 0 so the condition becomes false and the code continues after calibration


    ADC1_CR2 |= (1 << 20);   // EXTTRIG = 1 to allow SWSTART
    ADC1_CR2 &= ~(7 << 17); // EXTSEL clear
    ADC1_CR2 |=  (7 << 17); // EXTSEL = SWSTART
    ADC1_CR2 |= (1 << 22);  //start conversion

    TIM2_PSC = 20000;// PSC will be changed corresponding to the ADC value inside the interrupt so that speed of blinking changes accordingly.
    TIM2_ARR = 999; //999 + 1  = 1000;

    TIM2_CCR2 = 999; // this value is the one that will get compared with CNT value.
    //when CNT = CCRx an event will be performed. here let us toggle the pin at this condition

    TIM2_CCMR1 &= ~(3U << 8); //the two bits are set as 00 to config channel 2 as output
    TIM2_CCMR1 |= (1U << 11); //to enable preload register
    TIM2_CCMR1 |= (3U << 12); //this is the bit config for toggle mode. (bits 14:12 control channel 1 mode)
    TIM2_CCER |= (1U << 4); //enable channel 2

    TIM2_EGR |= (1U << 0); 	//generate update event using EGR
    //Enable auto-reload preload and start counter
     TIM2_CR1 |= (1 << 7);      // ARPE = 1
     TIM2_CR1 |= (1 << 0);      // CEN = 1


	while(1){

	}
	return 0;
}

