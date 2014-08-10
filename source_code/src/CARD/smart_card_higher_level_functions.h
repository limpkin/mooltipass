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


#ifndef SMART_CARD_HIGHER_LEVEL_FUNCTIONS_H_
#define SMART_CARD_HIGHER_LEVEL_FUNCTIONS_H_

#include "smartcard.h"
#include "defines.h"
#include <stdlib.h>

#ifdef DEBUG_SMC_DUMP_USB_PRINT
    #define printSmartCardInfo() printSMCDebugInfoToUSB()
#else
    #define printSmartCardInfo()
#endif


/************ INLINE FUNCTIONS ************/
/*! \fn     readAES256BitsKey(uint8_t* buffer)
*   \brief  Read the AES 256 bits key from the card. Note that it is up to the code calling this function to check that we're authenticated, otherwise 0s will be read
*   \param  buffer  Buffer to store the AES key
*/
inline void readAES256BitsKey(uint8_t* buffer)
{
    readSMC((SMARTCARD_AZ1_BIT_START + SMARTCARD_AZ1_BIT_RESERVED + AES_KEY_LENGTH)/8, (SMARTCARD_AZ1_BIT_START + SMARTCARD_AZ1_BIT_RESERVED)/8, buffer);
}

/*! \fn     readMooltipassWebsiteLogin(uint8_t* buffer)
*   \brief  Read the Mooltipass website login from the card. Note that it is up to the code calling this function to check that we're authenticated, otherwise 0s will be read
*   \param  buffer  Buffer to store the login
*/
inline void readMooltipassWebsiteLogin(uint8_t* buffer)
{
    // We take the space left in AZ2 -> 62 bytes (512 - 16 = 62 bytes)
    readSMC((SMARTCARD_AZ2_BIT_START + SMARTCARD_AZ2_BIT_RESERVED + SMARTCARD_MTP_LOGIN_LENGTH)/8, (SMARTCARD_AZ2_BIT_START + SMARTCARD_AZ2_BIT_RESERVED)/8, buffer);
}

/*! \fn     readMooltipassWebsitePassword(uint8_t* buffer)
*   \brief  Read the Mooltipass website password from the card. Note that it is up to the code calling this function to check that we're authenticated, otherwise 0s will be read
*   \param  buffer  Buffer to store the password
*/
inline void readMooltipassWebsitePassword(uint8_t* buffer)
{
    // We take the space left in AZ1 -> 30 bytes (512 - 256 - 16 = 30 bytes)
    readSMC((SMARTCARD_AZ1_BIT_START + SMARTCARD_AZ1_BIT_RESERVED + AES_KEY_LENGTH + SMARTCARD_MTP_PASS_LENGTH)/8, (SMARTCARD_AZ1_BIT_START + SMARTCARD_AZ1_BIT_RESERVED + AES_KEY_LENGTH)/8, buffer);
}

/*! \fn     readFabricationZone(uint8_t* buffer)
*   \brief  Read the fabrication zone (security mode 1&2)
*   \param  buffer  Pointer to a buffer (2 bytes required)
*   \return The provided pointer
*/
inline uint8_t* readFabricationZone(uint8_t* buffer)
{
    readSMC(2, 0, buffer);
    return buffer;
}

/*! \fn     readIssuerZone(uint8_t* buffer)
*   \brief  Read the issuer zone (security mode 1&2)
*   \param  buffer  Pointer to a buffer (8 bytes required)
*   \return The provided pointer
*/
inline uint8_t* readIssuerZone(uint8_t* buffer)
{
    readSMC(10, 2, buffer);
    return buffer;
}

/*! \fn     writeIssuerZone(uint8_t* buffer)
*   \brief  Write in the issuer zone (security mode 1 - Authenticated!)
*   \param  buffer  Pointer to a buffer (8 bytes required)
*/
inline void writeIssuerZone(uint8_t* buffer)
{
    writeSMC(16, 64, buffer);
}

/*! \fn     readSecurityCodeAttemptsCounters(uint8_t* buffer)
*   \brief  Read the number of code attempts left (security mode 1&2)
*   \param  buffer  Pointer to a buffer (2 bytes required)
*   \return The provided pointer
*/
inline uint8_t* readSecurityCodeAttemptsCounters(uint8_t* buffer)
{
    readSMC(14, 12, buffer);
    return buffer;
}

/*! \fn     readCodeProtectedZone(uint8_t* buffer)
*   \brief  Read the code protected zone (security mode 1&2 - Authenticated!)
*   \param  buffer  Pointer to a buffer (8 bytes required)
*   \return The provided pointer
*/
inline uint8_t* readCodeProtectedZone(uint8_t* buffer)
{
    readSMC(22, 14, buffer);
    return buffer;
}

/*! \fn     writeCodeProtectedZone(uint8_t* buffer)
*   \brief  Write in the code protected zone (security mode 1&2 - Authenticated!)
*   \param  buffer  Pointer to a buffer (8 bytes required)
*/
inline void writeCodeProtectedZone(uint8_t* buffer)
{
    writeSMC(112, 64, buffer);
}

/*! \fn     readApplicationZone1EraseKey(uint8_t* buffer)
*   \brief  Read the application zone1 erase key (security mode 1 - Authenticated!)
*   \param  buffer  Pointer to a buffer (6 bytes required)
*   \return The provided pointer
*/
inline uint8_t* readApplicationZone1EraseKey(uint8_t* buffer)
{
    readSMC(92, 86, buffer);
    return buffer;
}

/*! \fn     writeApplicationZone1EraseKey(uint8_t* buffer)
*   \brief  Write the application zone1 erase key (security mode 1 - Authenticated!)
*   \param  buffer  Pointer to a buffer (6 bytes required)
*/
inline void writeApplicationZone1EraseKey(uint8_t* buffer)
{
    writeSMC(688, 48, buffer);
}

/*! \fn     readApplicationZone2EraseKey(uint8_t* buffer)
*   \brief  Read the application zone2 erase key (security mode 1 - Authenticated!)
*   \param  buffer  Pointer to a buffer (4 bytes required)
*   \return The provided pointer
*/
inline uint8_t* readApplicationZone2EraseKey(uint8_t* buffer)
{
    readSMC(160, 156, buffer);
    return buffer;
}

/*! \fn     writeApplicationZone2EraseKey(uint8_t* buffer)
*   \brief  Write the application zone2 erase key (security mode 1 - Authenticated!)
*   \param  buffer  Pointer to a buffer (4 bytes required)
*/
inline void writeApplicationZone2EraseKey(uint8_t* buffer)
{
    writeSMC(1248, 32, buffer);
}

/*! \fn     readMemoryTestZone(uint8_t* buffer)
*   \brief  Read the Test zone (security mode 1&2)
*   \param  buffer  Pointer to a buffer (2 bytes required)
*   \return The provided pointer
*/
inline uint8_t* readMemoryTestZone(uint8_t* buffer)
{
    readSMC(178, 176, buffer);
    return buffer;
}

/*! \fn     writeMemoryTestZone(uint8_t* buffer)
*   \brief  Write in the Test zone (security mode 1&2)
*   \param  buffer  Pointer to a buffer (2 bytes required)
*/
inline void writeMemoryTestZone(uint8_t* buffer)
{
    writeSMC(1408, 16, buffer);
}

/*! \fn     readManufacturerZone(uint8_t* buffer)
*   \brief  Read the manufacturer zone (security mode 1&2)
*   \param  buffer  Pointer to a buffer (2 bytes required)
*   \return The provided pointer
*/
inline uint8_t* readManufacturerZone(uint8_t* buffer)
{
    readSMC(180, 178, buffer);
    return buffer;
}

/*! \fn     writeManufacturerZone(uint8_t* buffer)
*   \brief  Write in the manufacturer zone (security mode 1 - Authenticated!)
*   \param  buffer  Pointer to a buffer (2 bytes required)
*/
inline void writeManufacturerZone(uint8_t* buffer)
{
    writeSMC(1424, 16, buffer);
}

/*! \fn     writeManufacturerFuse(void)
*   \brief  Write manufacturer fuse, controlling access to the MFZ
*/
inline void writeManufacturerFuse(void)
{
    blowFuse(MAN_FUSE);
}

/*! \fn     write_issuers_fuse(void)
*   \brief  Write issuers fuse, setting the AT88SC102 into Security Mode 2, we need to be authenticated here
*/
inline void write_issuers_fuse(void)
{
    blowFuse(ISSUER_FUSE);
}

/*! \fn     write_ec2en_fuse(void)
*   \brief  Write ec2en fuse, to be done before blowing issuer fuse
*/
inline void write_ec2en_fuse(void)
{
    blowFuse(EC2EN_FUSE);
}


/************ PROTOTYPES ************/
RET_TYPE writeToApplicationZoneAndCheck(uint16_t addr, uint16_t nb_bits, uint8_t* buffer, uint8_t* temp_buffer);
RET_TYPE writeMooltipassWebsitePassword(uint8_t* buffer);
RET_TYPE checkAuthenticatedReadWriteAccessToZone1(void);
RET_TYPE checkAuthenticatedReadWriteAccessToZone2(void);
RET_TYPE writeMooltipassWebsiteLogin(uint8_t* buffer);
RET_TYPE setAuthenticatedReadWriteAccessToZone1(void);
RET_TYPE setAuthenticatedReadWriteAccessToZone2(void);
RET_TYPE mooltipassDetectedRoutine(uint16_t pin_code);
RET_TYPE transformBlankCardIntoMooltipass(void);
uint8_t getNumberOfSecurityCodeTriesLeft(void);
RET_TYPE writeAES256BitsKey(uint8_t* buffer);
uint8_t getNumberOfAZ2WritesLeft(void);
void writeSecurityCode(uint16_t code);
RET_TYPE cardDetectedRoutine(void);
void printSMCDebugInfoToUSB(void);
uint16_t readSecurityCode(void);
void eraseSmartCard(void);
void resetBlankCard(void);

/*
                SMART CARD MEMORY MAP
                
Bit Address                 Description                 Bits    Words
0–15            Fabrication Zone (FZ)                   16      1
16–79           Issuer Zone (IZ)                        64      4
80–95           Security Code (SC)                      16      1
96–111          Security Code Attempts counter (SCAC)   16      1
112–175         Code Protected Zone (CPZ)               64      4
176–687         Application Zone 1 (AZ1)                512     32
688–735         Application Zone 1 Erase Key (EZ1)      48      3
736–1247        Application Zone 2 (AZ2)                512     32
1248–1279       Application Zone 2 Erase Key (EZ2)      32      2
1280–1407       Application Zone 2 Erase Counter (EC2)  128     8
1408–1423       Memory Test Zone (MTZ)                  16      1
1424–1439       Manufacturer’s Zone (MFZ)               16      1
1440–1455       Block Write/Erase                       16      1
1456–1471       MANUFACTURER’S FUSE                     16      1
1529            EC2EN FUSE (Controls use of EC2)        1
1552 - 1567     ISSUER FUSE                             161

AZ1 composition (bits): 16 reserved + 256 AES key + 240 MTP password
AZ2 composition (bits): 16 reserved + 496 MTP login

*/

#endif /* SMART_CARD_HIGHER_LEVEL_FUNCTIONS_H_ */
