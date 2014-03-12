#include "spi.h"

#define UCPHA1	1
#define XCK1	5

#define MISO2 2		// PD2
#define MOSI2 3		// PD3

/**
 * Initialise the SPI USART interface to the specified data rate
 * @param rate frequency to run the SPI interface at
 */
void spiUsartBegin(uint16_t rate)
{
    // enable pins
    UBRR1 = 0;

    DDRD |= (1 << XCK1) | (1 << MOSI2); // MOSI & SCK as ouputs (enables master mode)
    DDRD &= ~(1 << MISO2);              // MISO as input
    PORTD &= ~(1 << MISO2);             // Disable pull-up

    // Set MSPI mode of operation and SPI data mode 0.
    UCSR1C = (1 << UMSEL11) | (1 << UMSEL10) | (0 << UCPOL1) | (0 << UCPHA1);
    UCSR1B = (1<<RXEN1)|(1<<TXEN1);  // Enable receiver and transmitter
    UBRR1 = rate;  // Set data rate (must be done after TX is enabled)
}

/**
 * change the SPI USART interface data rate
 * @param rate - frequency to run the SPI interface at
 */
void spiUsartSetRate(uint16_t rate)
{
    UBRR1 = rate;
}

