/*This project is to implement the concept of ring buffer in UART.
 * https://www.youtube.com/watch?v=GbBrp6K7IvM  - watch this to understand the concept of ring buffers
 * This is to handle real time data without any losses in an efficient manner, even if the data is not read immediately while the CPU is busy doing other tasks
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

#define size_of_buffer  64
volatile char rx_buffer[size_of_buffer];
volatile uint8_t head =0;
volatile uint8_t tail =0;


//this ISR is used to write data or collect data(what we type via serial monitor) . The head is the index variable getting incremented to store successive characters.
// this is done till head is not equal to tail. when head becomes equal to tail, in this case, new data will not be read.
void USART1_IRQHandler(){
	if (USART1_SR & (1 << 5))   // RXNE
	    {
	        char c = USART1_DR;

	        uint8_t next = (head + 1) % size_of_buffer;

	        if(next != tail){
	        	rx_buffer[head] = c;
	        	head = next;
	        }
	       //if (next = tail), new data will not be taken, they will be taken only if the old data is read. So overflow is controlled
	       //in the ISR only data is collected, no other operation is done. ISR is quick
}
}

//the below function will be called in main(). It's helps to read data from the rx_buffer using tail as the index. Earlier we mentioned about writing data using head as index at the ISR
//read the while(1) condition inside main(), it will be easy to understand. There, we pass this function with the address of a variable. Here we use a pointer and dereference it to give the data back to the while loop.
//at the loop, each character is stringed together for retrieving the command (this is performed at the if condition inside the final else condition)
int uart_getchar(char *c)
{
    if (head == tail)
        return 0;   // buffer empty

    *c = rx_buffer[tail];
    tail = (tail + 1) % size_of_buffer;
    return 1;
}

int main(){
	//config clock for portA, USART
	APB2_ENR |= (1 << 2);
	APB2_ENR |= (1 << 14);

	//configure PA0 as output push pull
	GPIOA_CRL &= ~(0xF << 0);
	GPIOA_CRL |= (0x1 << 0);

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

	char cmd[32];
	uint8_t cmd_index = 0;

	while (1)
	{
	    char c;

	    if (uart_getchar(&c))
	    {
	        if (c == '\r')
	            continue;

	        if (c == '\n')
	        {
	            cmd[cmd_index] = '\0';
	            cmd_index = 0;

	            if (strcmp(cmd, "LED ON") == 0)
	            {
	                GPIOA_ODR |= (1 << 0);
	            }
	            else if (strcmp(cmd, "LED OFF") == 0)
	            {
	                GPIOA_ODR &= ~(1 << 0);
	            }
	        }
	        else
	        {
	            if (cmd_index < sizeof(cmd) - 1)
	            {
	                cmd[cmd_index++] = c;
	            }
	        }
	    }
	}

}
