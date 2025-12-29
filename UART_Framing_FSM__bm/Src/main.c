/*This code below is a basic representation of UART Packet Reception using FSM(Finite State Machine).
this was done as an improvisation to byte transfer using UART. real world communication often require packing data into frames,
and the frames are expected to be immune to random noise/garbage values. This is my first attempt at framing bytes into packets and sending them over UART as a tool.
for this to be done, a core concept called FSM has to be understood.
it is responsible for knowing the status of frame arrival, whether the frame has started to arrive, when the frame should end etc.
To send bytes via serial monitor, I installed a tool called HTerm serial monitor. it has got a lot of features, we can send bytes in various formats(ASCII,HEX,BIN etc);
this is not possible in Arduino IDE serial monitor, where data is sent only in the ASCII format*/

#include<stdint.h>
#include<string.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define USART1_BASE 0x40013800UL

//used register address
#define APB2_ENR            REG32((RCC_BASE + 0x18UL))  //for port a, USART
#define NVIC_ISER1 			REG32(0xE000E104UL) 		//this is from cortex M3 architecture Nested Vector Interrupt Controller_Interrrupt Set Enable Register. address taken from m3 ref manual


#define GPIOA_CRL 			REG32((GPIOA_BASE + 0x00UL))
#define GPIOA_CRH 			REG32((GPIOA_BASE + 0x04UL))
#define GPIOA_ODR 			REG32((GPIOA_BASE + 0x0CUL))

#define USART1_SR			REG32((USART1_BASE + 0x00UL))
#define USART1_DR			REG32((USART1_BASE + 0x04UL))
#define USART1_BRR			REG32((USART1_BASE + 0x08UL))
#define USART1_CR1			REG32((USART1_BASE + 0x0CUL))

#define SIZE_OF_BUFFER 32

//this enumeration is to know the current state of FSM, whether it is waiting for a start byte or data or the end of frame(EOF)
typedef enum{
	RX_WAIT_SOF,
	RX_WAIT_DATA,
	RX_WAIT_EOF
}rx_state_t;

//this strcut is created to actively track the reception and keep store of the state. it will contain all data about the frame handling
typedef struct{
	rx_state_t state;
	uint8_t buffer[SIZE_OF_BUFFER];
	uint8_t index;
}rx_ctx_t;

rx_ctx_t rx;

//every time data is
void USART1_IRQHandler(){
	if(USART1_SR & (1 << 5)){

	   // GPIOC_ODR ^= (1U << 13);   // DEBUG: ISR entry

		uint8_t byte = USART1_DR;
		switch(rx.state){
		case RX_WAIT_SOF:
			if (byte == 0xAA){
				rx.state = RX_WAIT_DATA;
				rx.index = 0;
				GPIOA_ODR |= (1U << 0);
			}
			break;
		case RX_WAIT_DATA:
			if (byte == 0x55){
				rx.state = RX_WAIT_SOF;
				GPIOA_ODR |= (1U << 2);   // packet complete LED
			}
			else {
				if(rx.index < SIZE_OF_BUFFER){
                    rx.buffer[rx.index++] = byte;
                    GPIOA_ODR |= (1U << 1);
				}
				rx.state = RX_WAIT_EOF;
			}
			break;
		case RX_WAIT_EOF:
			if(byte == 0x55){
				rx.state = RX_WAIT_SOF;
                GPIOA_ODR |= (1U << 2);
			}
			break;
		}
	}
}

	int main(){
		//config clock for portA, USART
		APB2_ENR |= (1 << 2);
		APB2_ENR |= (1 << 14);

		//configure PA0 as output push pull
		GPIOA_CRL &= ~(0xF << 0);
		GPIOA_CRL |= (0x1 << 0);

		//configure PA1 as output push pull
		GPIOA_CRL &= ~(0xF << 4);
		GPIOA_CRL |= (0x1 << 4);

		//configure PA2 as output push pull
		GPIOA_CRL &= ~(0xF << 8);
		GPIOA_CRL |= (0x1 << 8);

		//set PA9 (TX of USART1) as AF o/p
		GPIOA_CRH &= ~(0xF << 4);
		GPIOA_CRH |= (0xB << 4);

		//config PA10 (RX of USART1) as floating input
		GPIOA_CRH &= ~(0xF << 8);
		GPIOA_CRH |= (0x4 << 8);

		//set baud rate at 9600 for 8MHz clk freq.
		USART1_BRR = 0x341;

		USART1_CR1 |= (1U << 2);  //receiver enable
		USART1_CR1 |= (1U << 5);  //RX interrupt enable
		USART1_CR1 |= (1U << 13);  //UART enable

		NVIC_ISER1 |= (1 << 5); //enable USART interrupt
		while(1){

		}
}
