#include "spi.h"

#define UCPHA1	1
#define XCK1	5
#define MOSI2 1		// PD3
#define MISO2 0		// PD2

void SPI::begin() 
{
    pinMode(MISO2, INPUT);
    pinMode(MOSI2, OUTPUT);

    // enable pins
    UBRR1 = 0;
    DDRD |= 1<<XCK1; //  XCK1 port pin as output to enable master mode

    // Set MSPI mode of operation and SPI data mode 3.
    UCSR1C = (1 << UMSEL11) | (1 << UMSEL10) | (0 << UCPOL1) | (0 << UCPHA1);
    UCSR1B = (1<<RXEN1)|(1<<TXEN1);  // Enable receiver and transmitter
    UBRR1 = _baud;  // Set baud rate (must be done after TX is enabled)
}

void SPI::setBaud(uint16_t baud)
{
    _baud = baud;
    UBRR1 = _baud;
}

