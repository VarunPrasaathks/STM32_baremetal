/* This project is to understand and experiment an important concept in data transmission - acknowledging a frame.
 * Till now we have done projects where if we send a correct frame, an LED glows, if not nothing happens
 * All practical systems need to send an acknowledgment back to the sender so that the sender knows a frame is correctly received
 * Lets start simply - we enter a frame, if it is right, an ACK byte will be sent; if not a NACK byte will be sent
 */

#include<stdint.h>
#include<string.h>

#define ACK_BYTE 0x06  //this byte will be sent to indicate frame received correctly
#define NACK_BYTE 0x15 //not received correctly

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define USART1_BASE 0x40013800UL

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

//this struct puts together all the ring buffer related variables and arrays.
typedef struct {
	uint8_t head;
	uint8_t tail;
	uint8_t buf[RB_SIZE];
}ring_buf_t;

ring_buf_t rx_rb;

//this enum contains the flow of FSM states. note that we have newly included a state RX_WAIT_CHK for checksum
typedef enum{
	RX_WAIT_SOF,
	RX_WAIT_LEN,
	RX_WAIT_CMD,
	RX_WAIT_DATA,
	RX_WAIT_CHK,
	RX_WAIT_EOF
}rx_state_t;

#define SIZE_OF_BUFFER 32

//this struct is created to actively track the reception and keep store of the state. it will contain all data about the frame handling
typedef struct{
	rx_state_t state;
	uint8_t buffer[32];
	uint8_t index;
	uint8_t length;
	uint8_t checksum;
	uint8_t eof_flag;
}rx_ctx_t;

rx_ctx_t rx;

//this function will be called inside the ISR to put the received data in the ring buffer. by doing so, we make the ISR faster and it performs only its required job
void rb_put(ring_buf_t *rb, uint8_t data){
	uint8_t next = (rb->head + 1) % RB_SIZE;

	if(next != rb->tail){
		rb->buf[rb->head] = data;
		rb->head = next;
	}
}

//this function will be called in main to read data from the ring buffer
int rb_get(ring_buf_t *rb, uint8_t *data){
	if(rb->head == rb->tail)
		return 0;

	*data = rb->buf[rb->tail];
	rb->tail = (rb->tail + 1) % RB_SIZE;
	return 1;
}

void USART1_IRQHandler(){
	if(USART1_SR & (1<<5)){
		uint8_t byte = USART1_DR;
		rb_put(&rx_rb, byte);
	}
}

void uart_tx_byte(uint8_t data){
	while(!(USART1_SR & (1<<7)));
		USART1_DR = data;
}

//while(!(USART1_SR & (1 << 7)));  // wait until TXE = 1
	//USART1_DR = data;


void rx_fsm(uint8_t byte){
	switch(rx.state){
	case RX_WAIT_SOF:
		if(byte == 0xAA){
			rx.state = RX_WAIT_LEN;
			rx.index = 0;
			rx.checksum = 0;
			rx.eof_flag =0;
			//GPIOA_ODR |= (1U << 0);
		}
		break;


	case RX_WAIT_LEN:
		if(byte >= SIZE_OF_BUFFER){
			rx.state = RX_WAIT_SOF;
		}

		else{
		rx.state = RX_WAIT_CMD;
		rx.length = byte;
		}
		break;


	case RX_WAIT_CMD:
		rx.state = RX_WAIT_DATA;
		rx.checksum ^= byte;  //xor is being done for CMD as well. this was excluded in the previous code
		rx.buffer[0] = byte;
		rx.index = 1;
		break;

	case RX_WAIT_DATA:
		rx.buffer[rx.index++] = byte;
		rx.checksum ^= byte;
		if (rx.index >= rx.length) {
			rx.state = RX_WAIT_CHK;
		}
		break;


	case RX_WAIT_CHK:
		if(byte == rx.checksum){
			rx.state = RX_WAIT_EOF;
		}else{
			uart_tx_byte(NACK_BYTE);
			rx.state = RX_WAIT_SOF;
		}
		break;


	case RX_WAIT_EOF:
		if(byte == 0x55){
		rx.state = RX_WAIT_SOF;
		rx.eof_flag =1;
		}
		break;
	}
}

#define CMD_LED_CTRL   0x01  //command for LED control

void handle_command(uint8_t *buf, uint8_t len){
	uint8_t cmd = buf[0];
	uint8_t *data = &buf[1];

	switch(cmd){
	case CMD_LED_CTRL:
		if(len != 3)
			return;

		switch(data[0]){
		case 0x01: //led 1
			if(data[1]){
				GPIOA_ODR |= (1U << 0);
			}
			else
				GPIOA_ODR &= ~(1U << 0);
			break;

		case 0x02: //led 2
			if(data[1]){
				GPIOA_ODR |= (1U << 1);
			}
			else
				GPIOA_ODR &= ~(1U << 1);
			break;

		default:
			break;
		}
		break;

		default:
			break;  //to ignore unknown commands
	}
}


int main(){
	//enable clocks for UART, port A
	APB2_ENR |= (1 << 2);
	APB2_ENR |= (1 << 14);

	//configure PA0, PA1, PA2, PA3, PA4 as output push pull
	GPIOA_CRL &= ~(0xFF << 0);
	GPIOA_CRL |= (0x11 << 0);

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
	USART1_CR1 |= (1U << 3);   //transmitter enable

	USART1_CR1 |= (1U << 13);  //UART enable

	NVIC_ISER1 |= (1 << 5); //enable USART interrupt

	while(1){
		uint8_t byte;
			if (rb_get(&rx_rb, &byte)){
				rx_fsm(byte);
			}
			if(rx.eof_flag == 1){
					uart_tx_byte(ACK_BYTE);
					handle_command(rx.buffer,rx.length);

					rx.eof_flag =0;
				}
			}
	}


/*example frames to check the working of this code
 * LED 1 ON -- < AA 03 01 01 01 01 55 > ---- < SOF LEN CMD LED1 ON XOR(01,01,01 = 01) EOF>
 * LED 1 OFF-- < AA 03 01 01 00 00 55 > ---- < SOF LEN CMD LED1 OFF XOR(01,01,00 = 00) EOF>
 * LED 2 ON -- < AA 03 01 02 01 02 55 > ---- < SOF LEN CMD LED2 ON XOR(01,02,01 = 02) EOF>
 * LED 2 OFF-- < AA 03 01 02 00 03 55 > ---- < SOF LEN CMD LED2 OFF XOR(01,02,00 = 03) EOF>
 *
 * !!! XOR in hexadecimal !!!
 */

