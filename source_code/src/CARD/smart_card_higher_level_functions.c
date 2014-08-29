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
/*! \file   smart_card_higher_level_functions.c
 *  \brief  Smart Card high level functions
 *  Copyright [2014] [Mathieu Stephan]
 */
#include "smart_card_higher_level_functions.h"
#include "smartcard.h"
#include "defines.h"
#include "oledmp.h"
#include "utils.h"
#include "usb.h"
#include <string.h>


/*! \fn     checkSecurityMode2(void)
*   \brief  Check that the smartcard is in mode two by trying to write his manufacturer zone
*   \return Success status
*/
RET_TYPE checkSecurityMode2(void)
{
    uint16_t manZoneRead, temp_uint;
    
    // Read manufacturer zone, set temp_uint to its opposite
    readManufacturerZone((uint8_t*)&manZoneRead);
    temp_uint = ~manZoneRead;
    
    // Perform test write
    writeManufacturerZone((uint8_t*)&temp_uint);
    readManufacturerZone((uint8_t*)&manZoneRead);
    
    if (temp_uint != manZoneRead)
    {
        return RETURN_OK;
    } 
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     mooltipassDetectedRoutine(uint16_t pin_code)
*   \brief  Function called when a Mooltipass is inserted into the smart card slot
*   \param  pin_code    Mooltipass pin code
*   \return If we managed to unlock it / if there is other problem (see mooltipass_detect_return_t)
*/
RET_TYPE mooltipassDetectedRoutine(uint16_t pin_code)
{
    RET_TYPE temp_rettype;

    // Try unlocking card with provided code
    temp_rettype = securityValidationSMC(pin_code);

    if (temp_rettype == RETURN_PIN_OK)                                   // Unlock successful
    {
        // Check that the card is in security mode 2
        if (checkSecurityMode2() == RETURN_NOK)
        {
            // Card is in mode 1... how could this happen?
            #ifdef DEBUG_SMC_USB_PRINT
                usbPutstr_P(PSTR("Card in mode 1!\r\n"));
            #endif
            return RETURN_MOOLTIPASS_PB;
        }
        else                                                            // Everything is in order - proceed
        {
            // Check that read / write accesses are correctly configured
            if ((checkAuthenticatedReadWriteAccessToZone1() == RETURN_NOK) || (checkAuthenticatedReadWriteAccessToZone2() == RETURN_NOK))
            {
                #ifdef DEBUG_SMC_USB_PRINT
                    usbPutstr_P(PSTR("Bad access settings!\r\n"));
                #endif
                return RETURN_MOOLTIPASS_PB;
            }
            else
            {
                #ifdef DEBUG_SMC_USB_PRINT
                    usbPutstr_P(PSTR("PIN code checked!\r\n"));
                #endif
                return RETURN_MOOLTIPASS_4_TRIES_LEFT;
            }
        }
    }
    else                                                                // Unlock failed
    {
        #ifdef DEBUG_SMC_USB_PRINT
            usbPrintf_P(PSTR("%d tries left, wrong pin\r\n"), getNumberOfSecurityCodeTriesLeft());
        #endif
        
        // The enum allows us to do so
        return RETURN_MOOLTIPASS_0_TRIES_LEFT + getNumberOfSecurityCodeTriesLeft();
    }
}

/*! \fn     cardDetectedRoutine(void)
*   \brief  Function called when something is inserted into the smart card slot
*   \return What the card is / what needs to be done (see mooltipass_detect_return_t)
*/
RET_TYPE cardDetectedRoutine(void)
{
    RET_TYPE card_detection_result;
    uint8_t temp_buffer[10];
    RET_TYPE temp_rettype;

    card_detection_result = firstDetectFunctionSMC();            // Get a first card detection result

    if (card_detection_result == RETURN_CARD_NDET)               // This is not a card
    {
        #ifdef DEBUG_SMC_USB_PRINT
            usbPutstr_P(PSTR("Not a card\r\n"));
        #endif
        return RETURN_MOOLTIPASS_INVALID;
    }
    else if (card_detection_result == RETURN_CARD_TEST_PB)       // Card test problem
    {
        #ifdef DEBUG_SMC_USB_PRINT
            usbPutstr_P(PSTR("Card test problem\r\n"));
        #endif
        return RETURN_MOOLTIPASS_PB;
    }
    else if (card_detection_result == RETURN_CARD_0_TRIES_LEFT)  // Card blocked
    {
        #ifdef DEBUG_SMC_USB_PRINT
            usbPutstr_P(PSTR("Card blocked\r\n"));
        #endif
        return RETURN_MOOLTIPASS_BLOCKED;
    }
    else                                                        // Card is of the correct type and not blocked
    {
        // Detect if the card is blank by checking that the manufacturer zone is different from FFFF
        if (swap16(*(uint16_t*)readManufacturerZone(temp_buffer)) == 0xFFFF)
        {
            #ifdef ENABLE_MOOLTIPASS_CARD_FORMATTING
                // Card is new - transform into mooltipass
                #ifdef DEBUG_SMC_USB_PRINT
                    usbPutstr_P(PSTR("Blank card, transforming...\r\n"));
                #endif

                // Try to authenticate with factory pin
                temp_rettype = securityValidationSMC(SMARTCARD_FACTORY_PIN);

                if (temp_rettype == RETURN_PIN_OK)                   // Card is unlocked - transform
                {
                    if (transformBlankCardIntoMooltipass() == RETURN_OK)
                    {
                        #ifdef DEBUG_SMC_USB_PRINT
                            usbPutstr_P(PSTR("Card transformed!\r\n"));
                        #endif
                        return RETURN_MOOLTIPASS_BLANK;
                    }
                    else
                    {
                        #ifdef DEBUG_SMC_USB_PRINT
                            usbPutstr_P(PSTR("Couldn't transform card!\r\n"));
                        #endif
                        return RETURN_MOOLTIPASS_PB;
                    }
                }
                else                                                            // Card unlock failed. Show number of tries left
                {
                    #ifdef DEBUG_SMC_USB_PRINT
                        usbPrintf_P(PSTR("%d tries left, wrong pin\r\n"),getNumberOfSecurityCodeTriesLeft());
                    #endif
                    return RETURN_MOOLTIPASS_PB;
                }
            #else
                return RETURN_MOOLTIPASS_PB;
            #endif
        }
        else                                                                // Card is already converted into a mooltipass
        {
            // Check that the user setup his mooltipass card by checking that the CPZ is different from FFFF....
            readCodeProtectedZone(temp_buffer);            
            if (memcmp(temp_buffer, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", SMARTCARD_CPZ_LENGTH) != 0)
            {
                #ifdef DEBUG_SMC_USB_PRINT
                    usbPutstr_P(PSTR("Mooltipass card detected\r\n"));
                #endif
                return RETURN_MOOLTIPASS_USER;                
            }

            #ifdef DEBUG_SMC_USB_PRINT
                usbPutstr_P(PSTR("Unconfigured Mooltipass\r\n"));
            #endif

            // If we're here it means the user hasn't configured his blank mooltipass card, so try to unlock it using the default pin
            temp_rettype = mooltipassDetectedRoutine(SMARTCARD_DEFAULT_PIN);

            // If we unlocked it, it means we can personalize it
            if (temp_rettype != RETURN_MOOLTIPASS_4_TRIES_LEFT)
            {
                return RETURN_MOOLTIPASS_PB;
            }
            else
            {
                return RETURN_MOOLTIPASS_BLANK;
            }
        }
    }
}

/*! \fn     transformBlankCardIntoMooltipass(void)
*   \brief  Transform the card into a Mooltipass card (Security mode 1 - Authenticated!)
*   \return If we succeeded
*/
RET_TYPE transformBlankCardIntoMooltipass(void)
{
    uint8_t temp_buffer[20];
    uint16_t *temp_buf16 = (uint16_t*)temp_buffer;

    /* Check that we are in security mode 1 */
    if (checkSecurityMode2() == RETURN_OK)
    {
        return RETURN_NOK;
    }

    /* Perform block erase, resetting the entire memory excluding FZ/MTZ/MFZ to FFFF... */
    resetBlankCard();

    /* Set new security password, keep zone 1 and zone 2 security key to FFFF... */
    writeSecurityCode(SMARTCARD_DEFAULT_PIN);

    /* Write "hackaday" to issuer zone */
    hm_str_cpy("hackaday", (char*)temp_buffer, 8);
    writeIssuerZone(temp_buffer);

    /* Write 2014 to the manufacturer zone */
    *temp_buf16 = swap16(2014);
    writeManufacturerZone(temp_buffer);

    /* Set application zone 1 and zone 2 permissions: read/write when authenticated only */
    setAuthenticatedReadWriteAccessToZone1();
    setAuthenticatedReadWriteAccessToZone2();

    /* Burn manufacturer fuse */
    writeManufacturerFuse();
    
    /* Burn EC2EN fuse */
    write_ec2en_fuse();

    /* Burn issuer fuse */
    write_issuers_fuse();

    return RETURN_OK;
}

/*! \fn     eraseSmartCard(void)
*   \brief  Completely erase mooltipass card (Security mode 2 - Authenticated!)
*/
void eraseSmartCard(void)
{
    uint8_t temp_buffer[SMARTCARD_CPZ_LENGTH];
    
    // Write 0xFF in CPZ
    memset(temp_buffer, 0xFF, SMARTCARD_CPZ_LENGTH);
    writeCodeProtectedZone(temp_buffer);
    
    // Erase AZ1 & AZ2
    eraseApplicationZone1NZone2SMC(FALSE);
    eraseApplicationZone1NZone2SMC(TRUE);

    /* Set application zone 1 and zone 2 permissions: read/write when authenticated only */
    setAuthenticatedReadWriteAccessToZone1();
    setAuthenticatedReadWriteAccessToZone2();
    
    // Reset default pin code
    writeSecurityCode(SMARTCARD_DEFAULT_PIN);
}

/*! \fn     writeToApplicationZoneAndCheck(uint16_t addr, uint16_t nb_bits, uint8_t* buffer, uint8_t* temp_buffer)
*   \brief  Write to one application zone and check what we wrote
*   \param  addr    Address in bits of the place to write
*   \param  nb_bits Number of bits to write
*   \param  buffer  Buffer containing the data to write
*   \param  temp_buffer A temporary buffer having the same size as buffer
*   \return If we succeeded
*/
RET_TYPE writeToApplicationZoneAndCheck(uint16_t addr, uint16_t nb_bits, uint8_t* buffer, uint8_t* temp_buffer)
{    
    writeSMC(addr, nb_bits, buffer);
    readSMC((addr + nb_bits) >> 3, (addr >> 3), temp_buffer);
    
    if (memcmp(buffer, temp_buffer, (nb_bits >> 3)) == 0)
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }    
}

/*! \fn     writeAES256BitsKey(uint8_t* buffer)
*   \brief  Write the AES 256 bits key to the card
*   \param  buffer  Buffer containing the AES key
*   \return Operation success
*/
RET_TYPE writeAES256BitsKey(uint8_t* buffer)
{
    uint8_t temp_buffer[AES_KEY_LENGTH/8];
    return writeToApplicationZoneAndCheck(SMARTCARD_AZ1_BIT_START + SMARTCARD_AZ1_BIT_RESERVED, AES_KEY_LENGTH, buffer, temp_buffer);
}

/*! \fn     writeMooltipassWebsitePassword(uint8_t* buffer)
*   \brief  Write the Mooltipass website password to the card
*   \param  buffer  Buffer containing the password
*   \return Operation success
*/
RET_TYPE writeMooltipassWebsitePassword(uint8_t* buffer)
{
    uint8_t temp_buffer[SMARTCARD_MTP_PASS_LENGTH/8];
    return writeToApplicationZoneAndCheck(SMARTCARD_AZ1_BIT_START + SMARTCARD_AZ1_BIT_RESERVED + AES_KEY_LENGTH, SMARTCARD_MTP_PASS_LENGTH, buffer, temp_buffer);
}

/*! \fn     writeMooltipassWebsiteLogin(uint8_t* buffer)
*   \brief  Write the Mooltipass website login to the card
*   \param  buffer  Buffer containing the login
*   \return Operation success
*/
RET_TYPE writeMooltipassWebsiteLogin(uint8_t* buffer)
{
    uint8_t temp_buffer[SMARTCARD_MTP_LOGIN_LENGTH/8];
    return writeToApplicationZoneAndCheck(SMARTCARD_AZ2_BIT_START + SMARTCARD_AZ2_BIT_RESERVED, SMARTCARD_MTP_LOGIN_LENGTH, buffer, temp_buffer);
}

/*! \fn     resetBlankCard(void)
*   \brief  Reinitialize the card to its default settings & default pin (Security mode 1 - Authenticated!)
*/
void resetBlankCard(void)
{
    uint8_t data_buffer[2] = {0xFF, 0xFF};
    writeSMC(1441, 1, data_buffer);
    writeSecurityCode(SMARTCARD_FACTORY_PIN);
}

/*! \fn     readSecurityCode(void)
*   \brief  Read the security code (security mode 1 - Authenticated!)
*   \return The security code
*/
uint16_t readSecurityCode(void)
{
    uint16_t temp_uint;
    readSMC(12, 10, (uint8_t*)&temp_uint);
    return swap16(temp_uint);
}

/*! \fn     writeSecurityCode(uint16_t code)
*   \brief  Write a new security code (security mode 1&2 - Authenticated!)
*   \param  code  The pin code
*/
void writeSecurityCode(uint16_t code)
{
    code = swap16(code);
    writeSMC(80, 16, (uint8_t*)&code);
}

/*! \fn     setAuthenticatedReadWriteAccessToZone1(void)
*   \brief  Function called to only allow reads and writes to the application zone 1 when authenticated
*   \return Operation success
*/
RET_TYPE setAuthenticatedReadWriteAccessToZone1(void)
{
    // Set P1 to 1 to allow write, remove R1 to prevent non authenticated reads
    uint8_t temp_buffer[2] = {0x80, 0x00};
    writeSMC(176, 16, temp_buffer);
    
    return checkAuthenticatedReadWriteAccessToZone1();
}

/*! \fn     checkAuthenticatedReadWriteAccessToZone1(void)
*   \brief  Function called to check that only reads and writes are allowed to the application zone 1 when authenticated
*   \return OK or NOK
*/
RET_TYPE checkAuthenticatedReadWriteAccessToZone1(void)
{
    uint8_t temp_buffer[2];

    readSMC(24, 22, temp_buffer);

    if ((temp_buffer[0] == 0x80) && (temp_buffer[1] == 0x00))
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     setAuthenticatedReadWriteAccessToZone2(void)
*   \brief  Function called to only allow reads and writes to the application zone 2 when authenticated
*   \return Operation success
*/
RET_TYPE setAuthenticatedReadWriteAccessToZone2(void)
{
    // Set P2 to 1 to allow write, remove R2 to prevent non authenticated reads
    uint8_t temp_buffer[2] = {0x80, 0x00};
    writeSMC(736, 16, temp_buffer);
    
    return checkAuthenticatedReadWriteAccessToZone2();
}

/*! \fn     checkAuthenticatedReadWriteAccessToZone2(void)
*   \brief  Function called to check that only reads and writes are allowed to the application zone 2 when authenticated
*   \return OK or NOK
*/
RET_TYPE checkAuthenticatedReadWriteAccessToZone2(void)
{
    uint8_t temp_buffer[2];

    readSMC(94, 92, temp_buffer);

    if ((temp_buffer[0] == 0x80) && (temp_buffer[1] == 0x00))
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
}

/*! \fn     printSMCDebugInfoToUSB(void)
*   \brief  Print the card info
*/
void printSMCDebugInfoToUSB(void)
{
    uint8_t data_buffer[20];
    uint8_t i;

    /* Extrapolate security mode */
    usbPrintf_P(PSTR("Security mode %c\n"), (checkSecurityMode2() == RETURN_OK) ? '2' : '1');

    /* Read FZ, SC, and SCAC */
    oledSetXY(0,0);
    usbPrintf_P(PSTR("FZ: %04X SC: %04X SCAC: %04X\n"), swap16(*(uint16_t *)readFabricationZone(data_buffer)), readSecurityCode(), swap16(*(uint16_t*)readSecurityCodeAttemptsCounters(data_buffer)));

    /* Read IZ */
    readIssuerZone(data_buffer);
    usbPrintf_P(PSTR("IZ:  "));
    for (i = 0; i < 4; i++)
    {
        usbPrintf_P(PSTR("%04X "), swap16(((uint16_t*)data_buffer)[i]));
    }
    usbPrintf_P(PSTR("\n"));

    /* Recompose CPZ */
    readCodeProtectedZone(data_buffer);
    usbPrintf_P(PSTR("CPZ: "));
    for (i = 0; i < 4; i++)
    {
        usbPrintf_P(PSTR("%04X "), swap16(((uint16_t*)data_buffer)[i]));
    }
    usbPrintf_P(PSTR("\n"));

    /* Read EZ1 */
    readApplicationZone1EraseKey(data_buffer);
    usbPrintf_P(PSTR("EZ1: "));
    for (i = 0; i < 3; i++)
    {
        usbPrintf_P(PSTR("%04X "), swap16(((uint16_t*)data_buffer)[i]));
    }
    usbPrintf_P(PSTR("\n"));

    /* Read EZ2 */
    readApplicationZone2EraseKey(data_buffer);
    usbPrintf_P(PSTR("EZ2: "));
    for (i = 0; i < 2; i++)
    {
        usbPrintf_P(PSTR("%04X "), swap16(((uint16_t*)data_buffer)[i]));
    }
    usbPrintf_P(PSTR("\n"));

    /* Read MTZ and MFZ */
    usbPrintf_P(PSTR("MTZ: %04X MFZ: %04X\n"),
            swap16(*(uint16_t*)readMemoryTestZone(data_buffer)),
            swap16(*(uint16_t*)readManufacturerZone(data_buffer)));

    /* Show first 8 bytes of AZ1 and AZ2 */
    readSMC(30,22,data_buffer);
    usbPrintf_P(PSTR("AZ1: "));
    for (i = 0; i < 4; i++)
    {
        usbPrintf_P(PSTR("%04X "), swap16(((uint16_t*)data_buffer)[i]));
    }
    usbPrintf_P(PSTR("\n"));
        
    readSMC(100,92,data_buffer);
    usbPrintf_P(PSTR("AZ2: "));
    for (i = 0; i < 4; i++)
    {
        usbPrintf_P(PSTR("%04X "), swap16(((uint16_t*)data_buffer)[i]));
    }
    usbPrintf_P(PSTR("\n"));
        
    /* Show EC2 counter */
    usbPrintf_P(PSTR("EC2: %02X\n"), getNumberOfAZ2WritesLeft());
}

/*! \fn     getNumberOfSecurityCodeTriesLeft(void)
*   \brief  Get the number of security code tries left
*   \return Number of tries left
*/
uint8_t getNumberOfSecurityCodeTriesLeft(void)
{
    uint8_t temp_buffer[2];
    uint8_t return_val = 0;
    uint8_t i;

    readSecurityCodeAttemptsCounters(temp_buffer);
    for(i = 0; i < 4; i++)
    {
        if ((temp_buffer[0] >> (4+i)) & 0x01)
        {
            return_val++;
        }
    }

    return return_val;
}

/*! \fn     getNumberOfAZ2WritesLeft(void)
*   \brief  Get the number of AZ2 writes left in case EC2 is not blown
*   \return Number of tries left
*/
uint8_t getNumberOfAZ2WritesLeft(void)
{
    uint8_t temp_buffer[16];
    uint8_t return_val = 0;
    uint8_t i;

    readSMC(176, 160, temp_buffer);
    for(i = 0; i < 128; i++)
    {
        if ((temp_buffer[i>>3] >> (i&0x07)) & 0x01)
        {
            return_val++;
        }
    }

    return return_val;  
}
