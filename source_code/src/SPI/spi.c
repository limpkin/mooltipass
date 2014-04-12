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
#include "spi.h"

/**
 * Initialise the SPI USART interface to the specified data rate
 * @param rate frequency to run the SPI interface at
 */
void spiUsartBegin(uint16_t rate)
{
    // enable pins
    UBRR1 = 0;

    DDRD |= (1 << PORTD5) | (1 << PORTD3); // MOSI & SCK as ouputs (enables master mode)
    DDRD &= ~(1 << PORTD2);              // MISO as input
    PORTD &= ~(1 << PORTD2);             // Disable pull-up

    // Set MSPI mode of operation and SPI data mode 0.
    UCSR1C = (1 << UMSEL11) | (1 << UMSEL10) | (0 << UCPOL1) | (0 << UCSZ10);
    UCSR1B = (1<<RXEN1) | (1<<TXEN1);   // Enable receiver and transmitter
    UBRR1 = rate;                       // Set data rate (must be done after TX is enabled)
}

/**
 * change the SPI USART interface data rate
 * @param rate - frequency to run the SPI interface at
 */
void spiUsartSetRate(uint16_t rate)
{
    UBRR1 = rate;
}

