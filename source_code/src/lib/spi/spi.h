#ifndef _SPI_H_
#define _SPI_H_

#include <stdio.h>
#include <Arduino.h>

/* 
 * BAUD = Fosc / 2*(URR1 + 1)
 *      = 16000000 / (2*URR1 + 2)
 *
 * URR1 = (Fosc / 2*BAUD) - 1
 */
#define SPI_BAUD_8_MHZ		0
#define SPI_BAUD_4_MHZ		1
#define SPI_BAUD_2_MHZ		3
#define SPI_BAUD_1_MHZ		7
#define SPI_BAUD_800_KHZ	9
#define SPI_BAUD_500_KHZ	15
#define SPI_BAUD_400_KHZ	19
#define SPI_BAUD_100_KHZ	79

class SPI {
public:
    SPI(uint16_t baud=SPI_BAUD_8_MHZ) : _baud(baud) {};
    inline static uint8_t transfer(uint8_t data);
    void begin();
    void setBaud(uint16_t baud);
private:
    uint16_t _baud;
};

byte SPI::transfer(byte data) 
{
    /* Wait for empty transmit buffer */
    while ( !( UCSR1A & (1<<UDRE1)) );
    UDR1 = data;
    /* Wait for data to be received */
    while ( !(UCSR1A & (1<<RXC1)) );
    return UDR1;
}

#endif
