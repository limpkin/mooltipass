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
/*! \file   smartcard.c
*   \brief  Smart Card low level functions
*   Copyright [2014] [Mathieu Stephan]
*/
#include "smart_card_higher_level_functions.h"
#include <util/delay_basic.h>
#include "logic_smartcard.h"
#include <avr/interrupt.h>
#include "timer_manager.h"
#include <util/atomic.h>
#include "smartcard.h"
#include "entropy.h"
#include "defines.h"
#include <avr/io.h>
#include "utils.h"

/** Counter for successive card detects **/
volatile uint8_t card_detect_counter = 0;
/** Current detection state, see detect_return_t */
volatile uint8_t button_return;


/*! \fn     smartcardHPulseDelay(void)
*   \brief  2us half pulse delay, specified by datasheet (min 3.3us/2)
*/
void smartcardHPulseDelay(void)
{
    // CPU clock of MAX 16MHz, 3 clock cycles per loop => 1/16M * 3 * 11 = 2.0625us
    _delay_loop_1(11);
}

/*! \fn     smartcardPowerDelay(void)
*   \brief  Delay to let the card come online/offline
*/
void smartcardPowerDelay(void)
{
    timerBasedDelayMs(130);
}

/*! \fn     smartcardTchpDelay(void)
*   \brief  Tchp delay (3.0ms min)
*/
static inline void smartcardTchpDelay(void)
{
    timerBasedDelayMs(4);
}

/*! \fn     clockPulseSMC(void)
*   \brief  Send a 4us H->L clock pulse (datasheet: min 3.3us)
*/
void clockPulseSMC(void)
{
    #if SPI_SMARTCARD == SPI_NATIVE
        PORT_SPI_NATIVE |= (1 << SCK_SPI_NATIVE);
        smartcardHPulseDelay();
        PORT_SPI_NATIVE &= ~(1 << SCK_SPI_NATIVE);
        smartcardHPulseDelay();
    #else
        #error "SPI not supported"
    #endif
}

/*! \fn     invertedClockPulseSMC(void)
*   \brief  Send a 4us L->H clock pulse (datasheet: min 3.3us)
*/
void invertedClockPulseSMC(void)
{
    #if SPI_SMARTCARD == SPI_NATIVE
        PORT_SPI_NATIVE &= ~(1 << SCK_SPI_NATIVE);
        smartcardHPulseDelay();
        PORT_SPI_NATIVE |= (1 << SCK_SPI_NATIVE);
        smartcardHPulseDelay();
    #else
        #error "SPI not supported"
    #endif
}

/*! \fn     clearPgmRstSignals(void)
*   \brief  Clear PGM / RST signal for normal operation mode
*/
void clearPgmRstSignals(void)
{
    PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
    PORT_SC_RST &= ~(1 << PORTID_SC_RST);
    smartcardHPulseDelay();smartcardHPulseDelay();
}

/*! \fn     setPgmRstSignals(void)
*   \brief  Set PGM / RST signal for standby mode
*/
void setPgmRstSignals(void)
{
    PORT_SC_RST |= (1 << PORTID_SC_RST);
    PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
    smartcardHPulseDelay();
}

/*! \fn     performLowLevelWriteNErase(uint8_t is_write)
*   \brief  Perform a write or erase operation on the smart card
*   \param  is_write    Boolean to indicate if it is a write
*/
void performLowLevelWriteNErase(uint8_t is_write)
{
    #if SPI_SMARTCARD == SPI_NATIVE
        /* Set programming control signal */
        PORT_SC_PGM |= (1 << PORTID_SC_PGM);
        smartcardHPulseDelay();

        /* Set data according to write / erase */
        if (is_write != FALSE)
        {
            PORT_SPI_NATIVE |= (1 << MOSI_SPI_NATIVE);
        }
        else
        {
            PORT_SPI_NATIVE &= ~(1 << MOSI_SPI_NATIVE);
        }
        smartcardHPulseDelay();

        /* Set clock */
        PORT_SPI_NATIVE |= (1 << SCK_SPI_NATIVE);
        smartcardHPulseDelay();

        /* Release program signal and data, wait for tchp */
        PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
        smartcardTchpDelay();

        /* Release clock */
        PORT_SPI_NATIVE &= ~(1 << SCK_SPI_NATIVE);
        smartcardHPulseDelay();

        /* Release data */
        PORT_SPI_NATIVE &= ~(1 << MOSI_SPI_NATIVE);
        smartcardHPulseDelay();
    #else
        #error "SPI not supported"
    #endif
}

/*! \fn     setSPIModeSMC(void)
*   \brief  Activate SPI controller for the SMC
*/
void setSPIModeSMC(void)
{
    #if SPI_SMARTCARD == SPI_NATIVE
        /* Enable SPI in master mode at 125kbits/s */
        SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
    #else
        #error "SPI not supported"
    #endif
}

/*! \fn     setBBModeAndPgmRstSMC(void)
*   \brief  Switch to big banging, and clear pgm/rst signal for normal operation
*/
void setBBModeAndPgmRstSMC(void)
{
    #if SPI_SMARTCARD == SPI_NATIVE
        /* Deactivate SPI port */
        SPCR = 0;

        /* Clock & data low */
        PORT_SPI_NATIVE &= ~(1 << SCK_SPI_NATIVE);
        PORT_SPI_NATIVE &= ~(1 << MOSI_SPI_NATIVE);
        smartcardHPulseDelay();

        /* Clear PGM and RST signals */
        clearPgmRstSignals();
    #else
        #error "SPI not supported"
    #endif
}

/*! \fn     blowFuse(uint8_t fuse_name)
*   \brief  Blow the manufacturer or issuer fuse
*   \param  fuse_name    Which fuse to blow
*/
void blowFuse(uint8_t fuse_name)
{
    uint16_t i;

    /* Set the index to write */
    if (fuse_name == MAN_FUSE)
    {
        i = 1460;
    }
    else if (fuse_name == ISSUER_FUSE)
    {
        i = 1560;
    }
    else if (fuse_name == EC2EN_FUSE)
    {
        i = 1529;
    }
    else
    {
        i = 0;
    }

    /* Switch to bit banging */
    setBBModeAndPgmRstSMC();

    /* Get to the good index */
    while(i--)clockPulseSMC();

    /* Set RST signal */
    PORT_SC_RST |= (1 << PORTID_SC_RST);

    /* Perform a write */
    performLowLevelWriteNErase(TRUE);

    /* Set PGM / RST signals to standby mode */
    setPgmRstSignals();

    /* Switch to SPI mode */
    setSPIModeSMC();
}

/*! \fn     isCardPlugged(void)
*   \brief  Know if a card is plugged
*   \return just released/pressed, (non)detected
*/
RET_TYPE isCardPlugged(void)
{
    // This copy is an atomic operation
    volatile RET_TYPE return_val = button_return;

    if ((return_val != RETURN_DET) && (return_val != RETURN_REL))
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            if (button_return == RETURN_JDETECT)
            {
                button_return = RETURN_DET;
            }
            else if (button_return == RETURN_JRELEASED)
            {
                button_return = RETURN_REL;
            }
        }
    }

    return return_val;
}

/*! \fn     scanSMCDectect(void)
*   \brief  card detect debounce called by 1ms interrupt
*/
void scanSMCDectect(void)
{
#if defined(HARDWARE_V1)
    if (PIN_SC_DET & (1 << PORTID_SC_DET))
#elif defined(HARDWARE_OLIVIER_V1)
    if (!(PIN_SC_DET & (1 << PORTID_SC_DET)))
#endif
    {
        if (card_detect_counter == 250)
        {
            // We must make sure the user detected that the smartcard was removed before setting it as detected!
            if (button_return != RETURN_JRELEASED)
            {
                button_return = RETURN_JDETECT;
                card_detect_counter++;
            }
        }
        else if (card_detect_counter != 0xFF)
        {
            card_detect_counter++;            
        }
    }
    else
    {
        // Smartcard remove functions
        if (card_detect_counter != 0)
        {
            handleSmartcardRemoved();
        }
        if (button_return == RETURN_DET)
        {
            button_return = RETURN_JRELEASED;
        }
        else if (button_return != RETURN_JRELEASED)
        {
            button_return = RETURN_REL;
        }
        card_detect_counter = 0;
    }
}

/*! \fn     eraseApplicationZone1NZone2SMC(uint8_t zone1_nzone2)
*   \brief  Set E1 or E2 flag by presenting the correct erase key (always FFFF...) and erase the AZ1 or AZ2
*   \param  zone1_nzone2    Zone 1 or Not Zone 2
*/
void eraseApplicationZone1NZone2SMC(uint8_t zone1_nzone2)
{
    #ifdef SMARTCARD_FUSE_V1        
        uint8_t temp_bool;
    #endif
    uint16_t i;

    /* Which index to go to */
    if (zone1_nzone2 == FALSE)
    {
        i = 1248;
    }
    else
    {
        i = 688;
    }

    #if SPI_SMARTCARD == SPI_NATIVE
        /* Switch to bit banging */
        setBBModeAndPgmRstSMC();

        /* Get to the good EZx */
        while(i--) invertedClockPulseSMC();

        /* How many bits to compare */
        if (zone1_nzone2 == FALSE)
        {
            i = 32;
        }
        else
        {
            i = 48;
        }

        /* Clock is at high level now, as input must be switched during this time */
        /* Enter the erase key */
        smartcardHPulseDelay();
        while(i--)
        {
            // The code is always FFFF...
            smartcardHPulseDelay();

            /* Inverted clock pulse */
            invertedClockPulseSMC();
        }

        /* Bring clock and data low */
        PORT_SPI_NATIVE &= ~(1 << SCK_SPI_NATIVE);
        smartcardHPulseDelay();smartcardHPulseDelay();
        PORT_SPI_NATIVE &= ~(1 << MOSI_SPI_NATIVE);
        smartcardHPulseDelay();smartcardHPulseDelay();
        
        /* In smart card fuse V1 (early versions sent to beta testers), EC2EN is not blown so we're limited to 128 erase operations... */
        #ifdef SMARTCARD_FUSE_V1
            if (zone1_nzone2 == FALSE)
            {               
                i = 0;
                temp_bool = TRUE;
                /* Write one of the four SCAC bits to 0 and check if successful */
                while((temp_bool == TRUE) && (i < 128))
                {
                    /* If one of the four bits is at 1, write a 0 */
                    if (PINB & (1 << MISO_SPI_NATIVE))
                    {
                        /* Set write command */
                        performLowLevelWriteNErase(TRUE);

                        /* Wait for the smart card to output a 0 */
                        while(PINB & (1 << MISO_SPI_NATIVE));

                        /* Exit loop */
                        temp_bool = FALSE;
                    }
                    else
                    {
                        /* Clock pulse */
                        clockPulseSMC();
                        i++;
                    }
                }
            }
        #endif

        /* Erase AZ1/AZ2 */
        performLowLevelWriteNErase(FALSE);

        /* Set PGM / RST signals to standby mode */
        setPgmRstSignals();

        /* Switch to SPI mode */
        setSPIModeSMC();
    #else
        #error "SPI not supported"
    #endif
}

/*! \fn     securityValidationSMC(uint16_t code)
*   \brief  Check security code
*   \param  code    The code
*   \return success_status (see card_detect_return_t)
*/
RET_TYPE securityValidationSMC(uint16_t code)
{
    RET_TYPE return_val = RETURN_PIN_NOK_0;
    uint8_t temp_bool;
    uint16_t i;

    #if SPI_SMARTCARD == SPI_NATIVE
        /* Switch to bit banging */
        setBBModeAndPgmRstSMC();

        /* Get to the SC */
        for(i = 0; i < 80; i++)
            invertedClockPulseSMC();

        /* Clock is at high level now, as input must be switched during this time */
        /* Enter the SC */
        smartcardHPulseDelay();
        for(i = 0; i < 16; i++)
        {
            if (((code >> (15-i)) & 0x0001) != 0x0000)
            {
                PORT_SPI_NATIVE &= ~(1 << MOSI_SPI_NATIVE);
            }
            else
            {
                PORT_SPI_NATIVE |= (1 << MOSI_SPI_NATIVE);
            }
           smartcardHPulseDelay();

            /* Inverted clock pulse */
            invertedClockPulseSMC();
        }

        /* Bring clock and data low */
        PORT_SPI_NATIVE &= ~(1 << SCK_SPI_NATIVE);
        smartcardHPulseDelay();smartcardHPulseDelay();
        PORT_SPI_NATIVE &= ~(1 << MOSI_SPI_NATIVE);
        smartcardHPulseDelay();smartcardHPulseDelay();

        i = 0;
        temp_bool = TRUE;
        /* Write one of the four SCAC bits to 0 and check if successful */
        while((temp_bool == TRUE) && (i < 4))
        {
            /* If one of the four bits is at 1, write a 0 */
            if (PINB & (1 << MISO_SPI_NATIVE))
            {
                /* Set write command */
                performLowLevelWriteNErase(TRUE);

                /* Wait for the smart card to output a 0 */
                while(PINB & (1 << MISO_SPI_NATIVE));

                /* Now, erase SCAC */
                performLowLevelWriteNErase(FALSE);

                /* Were we successful? */
                if (PINB & (1 << MISO_SPI_NATIVE))
                {
                    // Success !
                    return_val = RETURN_PIN_OK;
                }
                else
                {
                    // Wrong pin, return number of tries left
                    if (i == 0)
                    {
                        return_val = RETURN_PIN_NOK_3;
                    }
                    else if (i == 1)
                    {
                        return_val = RETURN_PIN_NOK_2;
                    }
                    else if (i == 2)
                    {
                        return_val = RETURN_PIN_NOK_1;
                    }
                    else if (i == 3)
                    {
                        return_val = RETURN_PIN_NOK_0;
                    }
                }

                /* Clock pulse */
                clockPulseSMC();

                /* Exit loop */
                temp_bool = FALSE;
            }
            else
            {
                /* Clock pulse */
                clockPulseSMC();
                i++;
            }
        }

        /* If we couldn't find a spot to write, no tries left */
        if (i == 4)
        {
            return_val = RETURN_PIN_NOK_0;
        }

        /* Set PGM / RST signals to standby mode */
        setPgmRstSignals();

        /* Switch to SPI mode */
        setSPIModeSMC();
    #else
        #error "SPI not supported"
    #endif

    return return_val;
}

/*! \fn     writeSMC(uint16_t start_index_bit, uint16_t nb_bits, uint8_t* data_to_write)
*   \brief  Write bits to the smart card
*   \param  start_index_bit         Where to start writing bits
*   \param  nb_bits                 Number of bits to write
*   \param  data_to_write           Pointer to the buffer
*/
void writeSMC(uint16_t start_index_bit, uint16_t nb_bits, uint8_t* data_to_write)
{
    uint16_t current_written_bit = 0;
    uint8_t masked_bit_to_write = 0;
    uint16_t i;

    #if SPI_SMARTCARD == SPI_NATIVE
        /* Switch to bit banging */
        setBBModeAndPgmRstSMC();

        /* Try to not erase AZ1 if EZ1 is 0xFFFFFFF... and we're writing the first bit of the AZ2 */
        if (start_index_bit >= SMARTCARD_AZ2_BIT_START)
        {
            /* Clock pulses until AZ2 start - 1 */
            for(i = 0; i < SMARTCARD_AZ2_BIT_START - 1; i++)
            {
                clockPulseSMC();            
            }                
            PORT_SPI_NATIVE |= (1 << MOSI_SPI_NATIVE);
            clockPulseSMC();
            PORT_SPI_NATIVE &= ~(1 << MOSI_SPI_NATIVE);
            /* Clock for the rest */
            for(i = 0; i < (start_index_bit - SMARTCARD_AZ2_BIT_START); i++)
            {
                clockPulseSMC();            
            }                
        }
        else
        {
            /* Get to the good index, clock pulses */
            for(i = 0; i < start_index_bit; i++)
            {
                clockPulseSMC();
            }                
        }

        /* Start writing */
        for(current_written_bit = 0; current_written_bit < nb_bits; current_written_bit++)
        {
            /* If we are at the start of a 16bits word or writing our first bit, erase the word */
            if ((((start_index_bit+current_written_bit) & 0x000F) == 0) || (current_written_bit == 0))
            {
                performLowLevelWriteNErase(FALSE);
            }

            /* Get good bit to write */
            masked_bit_to_write = (data_to_write[(current_written_bit>>3)] >> (7 - (current_written_bit & 0x0007))) & 0x01;

            /* Write only if the data is a 0 */
            if (masked_bit_to_write == 0x00)
            {
                performLowLevelWriteNErase(TRUE);
            }

            /* Go to next address */
            clockPulseSMC();
        }

        /* Set PGM / RST signals to standby mode */
        setPgmRstSignals();

        /* Switch to SPI mode */
        setSPIModeSMC();
    #else
        #error "SPI not supported"
    #endif
}

/*! \fn     readSMC(uint8_t nb_bytes_total_read, uint8_t start_record_index, uint8_t* data_to_receive)
*   \brief  Read bytes from the smart card
*   \param  nb_bytes_total_read     The number of bytes to be read
*   \param  start_record_index      The index at which we start recording the answer
*   \param  data_to_receive        Pointer to the buffer
*   \return The buffer
*/
uint8_t* readSMC(uint8_t nb_bytes_total_read, uint8_t start_record_index, uint8_t* data_to_receive)
{
    uint8_t* return_val = data_to_receive;
    uint8_t i;

    /* Set PGM / RST signals for operation */
    clearPgmRstSignals();

    for(i = 0; i < nb_bytes_total_read; i++)
    {
        /* Start transmission */
        SPDR = 0x00;
        /* Wait for transmission complete */
        while(!(SPSR & (1<<SPIF)));
        /* Store data in buffer or discard it*/
        if (i >= start_record_index)
        {
            *(data_to_receive++) = SPDR;
        }
        else
        {
            SPDR;
        }
    }

    /* Set PGM / RST signals to standby mode */
    setPgmRstSignals();

    return return_val;
}

/*! \fn     firstDetectFunctionSMC(void)
*   \brief  functions performed once the smart card is detected
*   \return The detection result (see card_detect_return_t)
*/
RET_TYPE firstDetectFunctionSMC(void)
{
    uint8_t data_buffer[2];
    uint16_t *data_buf16 = (uint16_t*)data_buffer;
    uint16_t temp_uint;

    /* Enable power to the card */
    PORT_SC_POW &= ~(1 << PORTID_SC_POW);

    /* Default state: PGM to 0 and RST to 1 */
    PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
    DDR_SC_PGM |= (1 << PORTID_SC_PGM);
    PORT_SC_RST |= (1 << PORTID_SC_RST);
    DDR_SC_RST |= (1 << PORTID_SC_RST);

    /* Activate SPI port */
    #if SPI_SMARTCARD == SPI_NATIVE
        PORT_SPI_NATIVE &= ~((1 << SCK_SPI_NATIVE) | (1 << MOSI_SPI_NATIVE));
        DDRB |= (1 << SCK_SPI_NATIVE) | (1 << MOSI_SPI_NATIVE);
        setSPIModeSMC();
    #else
        #error "SPI not supported"
    #endif

    /* Let the card come online */
    smartcardPowerDelay();

    /* Check smart card FZ */
    readFabricationZone(data_buffer);
    if ((swap16(*data_buf16)) != SMARTCARD_FABRICATION_ZONE)
    {
        return RETURN_CARD_NDET;
    }

    /* Perform test write on MTZ */
    readMemoryTestZone((uint8_t*)&temp_uint);
    temp_uint = temp_uint + 5;
    writeMemoryTestZone((uint8_t*)&temp_uint);
    if (*(uint16_t*)readMemoryTestZone(data_buffer) != temp_uint)
    {
        return RETURN_CARD_TEST_PB;
    }

    /* Read security code attempts counter */
    switch(getNumberOfSecurityCodeTriesLeft())
    {
        case 4: return RETURN_CARD_4_TRIES_LEFT;
        case 3: return RETURN_CARD_3_TRIES_LEFT;
        case 2: return RETURN_CARD_2_TRIES_LEFT;
        case 1: return RETURN_CARD_1_TRIES_LEFT;
        case 0: return RETURN_CARD_0_TRIES_LEFT;
        default: return RETURN_CARD_0_TRIES_LEFT;
    }
}

/*! \fn     removeFunctionSMC(void)
*   \brief  functions performed once the smart card is removed
*/
void removeFunctionSMC(void)
{
    /* Deactivate power to the smart card */
    PORT_SC_POW |= (1 << PORTID_SC_POW);

    /* Setup all output pins as tri-state */
    PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
    DDR_SC_PGM &= ~(1 << PORTID_SC_PGM);
    PORT_SC_RST &= ~(1 << PORTID_SC_RST);
    DDR_SC_RST &= ~(1 << PORTID_SC_RST);

    /* Deactivate SPI port */
    #if SPI_SMARTCARD == SPI_NATIVE
        SPCR = 0;
        DDRB &= ~(1 << SCK_SPI_NATIVE) | (1 << MOSI_SPI_NATIVE);
        PORT_SPI_NATIVE &= ~((1 << SCK_SPI_NATIVE) | (1 << MOSI_SPI_NATIVE));
    #else
        #error "SPI not supported"
    #endif
}

/*! \fn     initPortSMC(void)
*   \brief  Initialize smart card port
*/
void initPortSMC(void)
{
    /* Setup card detection input with pull-up */
    DDR_SC_DET &= ~(1 << PORTID_SC_DET);
    PORT_SC_DET |= (1 << PORTID_SC_DET);

    /* Setup power enable, disabled by default */
    PORT_SC_POW |= (1 << PORTID_SC_POW);
    DDR_SC_POW |= (1 << PORTID_SC_POW);

    /* Setup MISO as input, SS as input with pull-up */
    #if SPI_SMARTCARD == SPI_NATIVE
        DDRB &= ~((1 << MISO_SPI_NATIVE) | (1 << SS_SPI_NATIVE));
        PORT_SPI_NATIVE &= ~(1 << MISO_SPI_NATIVE);
        PORT_SPI_NATIVE |= (1 << SS_SPI_NATIVE);
    #else
        #error "SPI not supported"
    #endif

    /* Set all output pins as tri-state */
    removeFunctionSMC();
}
