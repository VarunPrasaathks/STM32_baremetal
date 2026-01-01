/*This code is an improvisation of the -- UART_FSM_payload_basic_bm --
 * Over there, we assigned a state to know the length of the data/payload in the FSM, but the FSM logic was inside the ISR.
 * It is actually a bad practice, as ISR must be small, fast with minimal logic but FSM may have branches and can be long
 * So we introduce a ring buffer which gets data via the ISR and the FSM accesses data through the ring buffer
 */

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

#define RB_SIZE 64

//a struct to organise all the ring buffer related variables
typedef struct {
	uint8_t buf[RB_SIZE];
	volatile uint8_t head;
	volatile uint8_t tail;
}ringbuf_t;

ringbuf_t rx_rb;

//this enumeration is to know the current state of FSM, whether it is waiting for a start byte or data or the end of frame(EOF)
typedef enum{
	RX_WAIT_SOF,
	RX_WAIT_LEN,
	RX_WAIT_DATA,
	RX_WAIT_EOF
}rx_state_t;


#define SIZE_OF_BUFFER 32

//this struct is created to actively track the reception and keep store of the state. it will contain all data about the frame handling
typedef struct{
	rx_state_t state;
	uint8_t buffer[SIZE_OF_BUFFER];
	uint8_t index;
	uint8_t length;
}rx_ctx_t;

rx_ctx_t rx;


//this is the function which gets the byte from the ISR and puts it in the ring buffer
void rb_put(ringbuf_t *rb, uint8_t data){
	uint8_t next = (rb->head + 1) % RB_SIZE;

	if (next != rb->tail) {
	    rb->buf[rb->head] = data;
	    rb->head = next;
	}

}

//this function will be called in main to read data from the ring buffer
int rb_get(ringbuf_t *rb, uint8_t *data){
	if(rb->head == rb->tail)
		return 0;
	*data = rb->buf[rb->tail];
	rb->tail = (rb->tail + 1) % RB_SIZE;
	return 1;
}

// this is the ISR. we have improved it from the previous versions where the FSM logic was inside the ISR. here the ISR gets the data and just pushes it to the ring buffer
void USART1_IRQHandler(){
	if(USART1_SR & (1 << 5)){
		uint8_t byte = USART1_DR;
		rb_put(&rx_rb, byte);
	}
}

//the FSM logic
void rx_fsm_process(uint8_t byte){
	switch(rx.state){
	case RX_WAIT_SOF:
		if (byte == 0xAA){
			rx.state = RX_WAIT_LEN;
			rx.index = 0;
			GPIOA_ODR |= (1U << 0);  //led at PA0 glows when i send 0xAA via serial monitor
			}
			break;

	case RX_WAIT_LEN:
		if(byte >= SIZE_OF_BUFFER){
			rx.state = RX_WAIT_LEN;
		}
			else{
			rx.state = RX_WAIT_DATA;
			rx.length = byte;
			GPIOA_ODR |= (1U << 1);  //PA1 glows when i enter a valid length
			}
			break;

	case RX_WAIT_DATA:
	        rx.buffer[rx.index++] = byte;
	        if (rx.index >= rx.length) {
	            rx.state = RX_WAIT_EOF;
	            GPIOA_ODR |= (1U << 2);  //PA2 glows after the last byte of payload is entered.(if rx.length = 3, then I should have sent three bytes to make this led glow)
	        }
	        break;

	case RX_WAIT_EOF:
		if(byte == 0x55){
			rx.state = RX_WAIT_SOF;
            GPIOA_ODR |= (1U << 3);  //PA3 will glow only when the correct EOF byte is entered. all the other bytes after payload is neglected if it is not 0x55.
			}
			break;
			}
}

int main()
{
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

	//configure PA2 as output push pull
	GPIOA_CRL &= ~(0xF << 12);
	GPIOA_CRL |= (0x1 << 12);

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
    while (1) {
        uint8_t byte;
        if (rb_get(&rx_rb, &byte)) { //now, FSM logic will be executed only when there is data in the ring buffer.
            rx_fsm_process(byte);
        }
    }
}
