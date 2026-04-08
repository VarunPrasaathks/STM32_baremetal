/*Now we are going to start learning a new communication protocol - SPI.
 * We had earlier done a lot of projects in UART, starting with basic byte transmission, reception;
 * learnt polling, interrupt, ring buffers, FSM, UART based custom protocol framing, checksum etc.
 *
 * This is the first synchronous comm. protocol I am going to do in bare metal. Planning to analyze the signals using logic analyzer and I will also document all the visuals in a drive so that others can be benefited as well
 */

#include<stdint.h>
#include<string.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

//base addresses
#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define SPI1_BASE 0x40013000UL

#define AFIO_BASE 0x40010000UL
#define AFIO_MAPR 			REG32((AFIO_BASE + 0x04UL))

#define APB2_ENR            REG32((RCC_BASE + 0x18UL))  //for port a(bit 2), SPI(bit 12), afio(bit 0)


#define GPIOA_CRL 			REG32((GPIOA_BASE + 0x00UL))
#define GPIOA_CRH 			REG32((GPIOA_BASE + 0x04UL))
#define GPIOA_ODR			REG32((GPIOA_BASE + 0x0CUL))

#define SPI_CR1				REG32((SPI1_BASE + 0x00UL))
#define SPI_CR2				REG32((SPI1_BASE + 0x04UL))
#define SPI_SR				REG32((SPI1_BASE + 0x08UL))
#define SPI_DR				REG32((SPI1_BASE + 0x0CUL))

//we control chip select(CS) manually. This line between master and slave is pulled low to initiate communication
void cs_high(){
	GPIOA_ODR |= (1U << 4);
}

void cs_low(){
	GPIOA_ODR &= ~(1U << 4);
}

uint8_t spi_transfer(uint8_t data){
    while(!(SPI_SR & (1U << 1)));  // TXE

    SPI_DR = data;                 // STARTS CLOCK

    while(!(SPI_SR & (1U << 0)));  // RXNE

    return (uint8_t)SPI_DR;
}
int main(void){

    APB2_ENR |= (1U << 0) | (1U << 2) | (1U << 12); //Enable clocks of AFIO, GPIOA, SPI1

    // PA5 SCK (AF Push-Pull, 50 MHz)
    GPIOA_CRL &= ~(0xF << 20);
    GPIOA_CRL |=  (0xB << 20);

    // PA7 MOSI (AF Push-Pull, 50 MHz)
    GPIOA_CRL &= ~(0xF << 28);
    GPIOA_CRL |=  (0xB << 28);

    // PA6 MISO (Floating input)
    GPIOA_CRL &= ~(0xF << 24);
    GPIOA_CRL |=  (0x4 << 24);

    // PA4 CS (General purpose push-pull output)
    GPIOA_CRL &= ~(0xF << 16);
    GPIOA_CRL |=  (0x3 << 16);

    cs_high(); //to ensure no communication happens before us starting

    SPI_CR1 = 0; //Reset SPI

    //Configure SPI (MASTER, MODE 0)

    SPI_CR1 |= (1 << 2);   // Master mode
    SPI_CR1 |= (1 << 3);   // Baud rate = fPCLK / 4

    SPI_CR1 |= (1 << 9);   // SSM = 1 (software slave management)
    SPI_CR1 |= (1 << 8);   // SSI = 1 (internal NSS high)

    SPI_CR1 |= (1 << 6);   // SPI Enable

    //using polling only because we are trying SPI for the first time. Will improvise in future projects
    while(1){

        cs_low();  // Select slave

        uint8_t response = spi_transfer(0x45);  //send bit 0x45

        cs_high(); // Deselect slave

        for(volatile int i = 0; i < 100000; i++); // small delay
    }
}


/* ---------------------- receiver code on Arduino Mega 2560 - Slave -----------------------------------------------------

 #include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

volatile uint8_t received_data = 0;
volatile uint8_t data_ready = 0;

void SPI_SlaveInit(void)
{
    // MISO output
    DDRB |= (1 << PB3);   // Pin 50

    // MOSI, SCK, SS input
    DDRB &= ~((1 << PB2) | (1 << PB1) | (1 << PB0));

    // Enable SPI + Interrupt
    SPCR = (1 << SPE) | (1 << SPIE);

    // Preload first byte
    SPDR = 0xAA;
}

ISR(SPI_STC_vect)
{
    received_data = SPDR;
    data_ready = 1;

    // Load next response
    SPDR = received_data + 1;
}

int main(void)
{
    init();                // Arduino core init (IMPORTANT)
    Serial.begin(9600);   // Start UART

    SPI_SlaveInit();
    sei();

    while (1)
    {
        if (data_ready)
        {
            data_ready = 0;

            Serial.print("Received: ");
            Serial.println(received_data, HEX);
        }
    }
}



 */
