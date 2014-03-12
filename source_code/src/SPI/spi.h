#ifndef _SPI_H_
#define _SPI_H_

#include "mooltipass.h"
#include <stdint.h>
#include <avr/io.h>

/* 
 * RATE = Fosc / 2*(URR1 + 1)
 *      = 16000000 / (2*URR1 + 2)
 *
 * URR1 = (Fosc / 2*RATE) - 1
 *
 * SPI Data rates
 */
#define SPI_RATE_8_MHZ		0
#define SPI_RATE_4_MHZ		1
#define SPI_RATE_2_MHZ		3
#define SPI_RATE_1_MHZ		7
#define SPI_RATE_800_KHZ	9
#define SPI_RATE_500_KHZ	15
#define SPI_RATE_400_KHZ	19
#define SPI_RATE_100_KHZ	79

void spiUsartBegin(uint16_t rate);
void spiUsartSetRate(uint16_t rate);

/**
 * send and receive a byte of data via the SPI USART interface.
 * @param data - the byte to send
 * @returns the received byte
 */
static inline uint8_t spiUsartTransfer(uint8_t data) 
{
    /* Wait for empty transmit buffer */
    while (!(UCSR1A & (1<<UDRE1)));
    UDR1 = data;
    /* Wait for data to be received */
    while (!(UCSR1A & (1<<RXC1)));
    return UDR1;
}

#endif
