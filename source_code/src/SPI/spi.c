/* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*! \file   spi.c
 *  \brief  USART SPI functions
 *  Copyright [2014] [Darran Hunt]
 */
#include "defines.h"
#include "spi.h"

/**
 * Initialise the SPI USART interface to the specified data rate
 */
void spiUsartBegin(void)
{
    // enable pins
    UBRR1 = 0;

    DDR_SPI_USART |= (1 << SCK_SPI_USART) | (1 << MOSI_SPI_USART);  // MOSI & SCK as ouputs (enables master mode)
    DDR_SPI_USART &= ~(1 << MISO_SPI_USART);                        // MISO as input
    PORT_SPI_USART &= ~(1 << MISO_SPI_USART);                       // Disable pull-up

    // Set MSPI mode of operation and SPI data mode 0.
    UCSR1C = (1 << UMSEL11) | (1 << UMSEL10) | (0 << UCPOL1) | (0 << UCSZ10);
    UCSR1B = (1<<RXEN1) | (1<<TXEN1);                               // Enable receiver and transmitter
    #if defined(HARDWARE_OLIVIER_V1) || defined(MINI_BOOTLOADER)
        UBRR1 = SPI_RATE_8_MHZ;                                     // Set data rate to 8Mhz
    #elif defined(MINI_VERSION)
        UBRR1 = SPI_RATE_4_MHZ;                                     // Set data rate to 4Mhz
    #endif
}

/**
 * change the SPI USART interface data rate
 * @param rate - frequency to run the SPI interface at
 */
void spiUsartSetRate(uint16_t rate)
{
    UBRR1 = rate;
}

#ifdef MINI_BOOTLOADER
/**
 * send and receive a byte of data via the SPI USART interface.
 * @param data - the byte to send
 * @returns the received byte
 */
uint8_t spiUsartTransfer(uint8_t data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR1A & (1<<UDRE1)));
    UDR1 = data;
    /* Wait for data to be received */
    while (!(UCSR1A & (1<<RXC1)));
    return UDR1;
}
#endif