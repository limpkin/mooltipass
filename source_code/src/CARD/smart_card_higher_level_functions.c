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
/*!	\file 	smart_card_higher_level_functions.c
 *	\brief	Smart Card high level functions
 *	Copyright [2014] [Mathieu Stephan]
 */
#include "smart_card_higher_level_functions.h"
#include "smartcard.h"
#include "graphics.h"
#include "utils.h"


/*!	\fn		mooltipassDetectedRoutine(uint16_t pin_code)
*	\brief	Function called when something a mooltipass is inserted into the smartcard slot
*	\param	pin_code	Mooltipass pin code
*	\return	If we managed to unlock it / if there is other problem
*/
RET_TYPE mooltipassDetectedRoutine(uint16_t pin_code)
{
	uint8_t temp_buffer[2];
	RET_TYPE temp_rettype;
	
	#ifdef DEBUG_SMC_SCREEN_PRINT		
		char temp_string[20];											// Temp string for displaying text
	#endif
	
	temp_rettype = securityValidationSMC(pin_code);	
	
	if(temp_rettype == RETURN_PIN_OK)									// Unlock successful
	{
		// Check that the card is in security mode 2 by reading the SC
		if(swap16(*(uint16_t*)readSecurityCode(temp_buffer)) != 0xFFFF)
		{
			// Card is in mode 1... how could this happen?
			#ifdef DEBUG_SMC_SCREEN_PRINT
				Show_String("Card in mode 1!", FALSE, 2, 16);
			#endif
			return RETURN_MOOLTIPASS_PB;
		}
		else															// Everything is in order - proceed
		{
			// Check that read / write accesses are correctly configured
			if((checkAuthenticatedReadWriteAccessToZone1() == RETURN_NOK) || (checkAuthenticatedReadWriteAccessToZone2() == RETURN_NOK))
			{
				#ifdef DEBUG_SMC_SCREEN_PRINT
					Show_String("Bad access settings!", FALSE, 2, 16);
				#endif
				return RETURN_MOOLTIPASS_PB;				
			}
			else
			{
				#ifdef DEBUG_SMC_SCREEN_PRINT
					Show_String("PIN code checked!", FALSE, 2, 16);
				#endif
				return RETURN_MOOLTIPASS_4_TRIES_LEFT;				
			}
		}
	}
	else																// Unlock failed
	{		
		#ifdef DEBUG_SMC_SCREEN_PRINT
			int_to_string(getNumberOfSecurityCodeTriesLeft(), temp_string);
			Show_String(temp_string, FALSE, 2, 16);
			Show_String("tries left, wrong pin", FALSE, 6, 16);
		#endif
		
		switch(getNumberOfSecurityCodeTriesLeft())
		{
			case 0 :	return RETURN_MOOLTIPASS_0_TRIES_LEFT;
			case 1 :	return RETURN_MOOLTIPASS_1_TRIES_LEFT;
			case 2 :	return RETURN_MOOLTIPASS_2_TRIES_LEFT;
			case 3 :	return RETURN_MOOLTIPASS_3_TRIES_LEFT;
			case 4 :	return RETURN_MOOLTIPASS_4_TRIES_LEFT;
			default :	return RETURN_MOOLTIPASS_0_TRIES_LEFT;
		}
	}
}

/*!	\fn		cardDetectedRoutine(void)
*	\brief	Function called when something is inserted into the smartcard slot
*	\return	What the card is / what needs to be done
*/
RET_TYPE cardDetectedRoutine(void)
{
	RET_TYPE card_detection_result;
	uint8_t temp_buffer[10];
	RET_TYPE temp_rettype;
	uint8_t i;
	
	#ifdef DEBUG_SMC_SCREEN_PRINT		
		char temp_string[20];									// Temp string for displaying text
		clear_screen();											// Clear screen before writing anything new
	#endif
	
	card_detection_result = firstDetectFunctionSMC();			// Get a first card detection result

	if(card_detection_result == RETURN_CARD_NDET)				// This is not a card
	{
		#ifdef DEBUG_SMC_SCREEN_PRINT
			Show_String("Not a card", FALSE, 2, 8);
		#endif
		return RETURN_MOOLTIPASS_INVALID;
	}
	else if(card_detection_result == RETURN_CARD_TEST_PB)		// Card test problem
	{
		#ifdef DEBUG_SMC_SCREEN_PRINT
			Show_String("Card test problem", FALSE, 2, 8);
		#endif
		return RETURN_MOOLTIPASS_PB;
	}
	else if(card_detection_result == RETURN_CARD_0_TRIES_LEFT)	// Card blocked
	{
		#ifdef DEBUG_SMC_SCREEN_PRINT
			Show_String("Card blocked", FALSE, 2, 8);
		#endif
		return RETURN_MOOLTIPASS_BLOCKED;
	}
	else														// Card is of the correct type and not blocked
	{
		// Detect if the card is blank by checking that the manufacturer zone is different from FFFF
		if(swap16(*(uint16_t*)readManufacturerZone(temp_buffer)) == 0xFFFF)
		{
			// Card is new - transform into mooltipass
			#ifdef DEBUG_SMC_SCREEN_PRINT
				Show_String("Blank card, transforming...", FALSE, 2, 8);
			#endif

			// Try to authenticate with factory pin
			temp_rettype = securityValidationSMC(SMARTCARD_FACTORY_PIN);

			if(temp_rettype == RETURN_PIN_OK)					// Card is unlocked - transform
			{
				if(transformBlankCardIntoMooltipass() == RETURN_OK)
				{
					#ifdef DEBUG_SMC_SCREEN_PRINT
						Show_String("Card transformed!", FALSE, 2, 16);
					#endif
					return RETURN_MOOLTIPASS_BLANK;
				}
				else
				{
					#ifdef DEBUG_SMC_SCREEN_PRINT
						Show_String("Couldn't transform card!", FALSE, 2, 16);
					#endif
					return RETURN_MOOLTIPASS_PB;
				}
			}
			else															// Card unlock failed. Show number of tries left
			{
				#ifdef DEBUG_SMC_SCREEN_PRINT
					int_to_string(getNumberOfSecurityCodeTriesLeft(), temp_string);
					Show_String(temp_string, FALSE, 2, 16);
					Show_String("tries left, wrong pin", FALSE, 6, 16);
				#endif
				return RETURN_MOOLTIPASS_PB;
			}
		}
		else																// Card is already converted into a mooltipass
		{			
			// Check that the user setup his mooltipass card by checking that the CPZ is different from FFFF....
			readCodeProtectedZone(temp_buffer);
			
			for(i = 0; i < 8; i++)
			{
				if(temp_buffer[i] != 0xFF)
				{
					#ifdef DEBUG_SMC_SCREEN_PRINT
						Show_String("Mooltipass card detected", FALSE, 2, 8);
					#endif
					return RETURN_MOOLTIPASS_USER;			
				}
			}
			
			#ifdef DEBUG_SMC_SCREEN_PRINT
				Show_String("Unconfigured Mooltipass", FALSE, 2, 8);
			#endif

			// If we're here it means the user hasn't configured his blank mooltipass card, so try to unlock it using the default pin
			temp_rettype = mooltipassDetectedRoutine(SMARTCARD_DEFAULT_PIN);
			
			if(temp_rettype != RETURN_MOOLTIPASS_4_TRIES_LEFT)
				return RETURN_MOOLTIPASS_PB;
			else
				return RETURN_MOOLTIPASS_BLANK;		
		}
	}
}

/*!	\fn		transformBlankCardIntoMooltipass(void)
*	\brief	Transform the card into a Mooltipass card (Security mode 1 - Authenticated!)
*	\return	If we succeeded
*/
RET_TYPE transformBlankCardIntoMooltipass(void)
{
	uint8_t temp_buffer[20];
	uint16_t *temp_buf16 = (uint16_t*)temp_buffer;
	
	/* Check that the security code is readable, ensuring that we are in security mode 1 with SV flag */
	if(swap16(*(uint16_t*)readSecurityCode(temp_buffer)) == 0xFFFF)
		return RETURN_NOK;
		
	/* Perform block erase, resetting the entire memory excluding FZ/MTZ/MFZ to FFFF... */
	resetBlankCard();
	
	/* Set new security password, keep zone 1 and zone 2 security key to FFFF... */
	*temp_buf16 = swap16(SMARTCARD_DEFAULT_PIN);
	writeSecurityCode(temp_buffer);
	
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
	
	/* Burn issuer fuse*/
	write_issuers_fuse();
	
	return RETURN_OK;
}

/*!	\fn		resetBlankCard(void)
*	\brief	Reinitialize the card to its default settings & default pin (Security mode 1 - Authenticated!)
*/
void resetBlankCard(void)
{
	uint8_t data_buffer[2] = {0xFF, 0xFF};
	uint16_t *data_buf16 = (uint16_t*)data_buffer;
	writeSMC(1441, 1, data_buffer);
	*data_buf16 = swap16(SMARTCARD_FACTORY_PIN);
	writeSecurityCode(data_buffer);
}

/*!	\fn		readFabricationZone(uint8_t* buffer)
*	\brief	Read the fabrication zone (security mode 1&2)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*	\return	The provided pointer
*/
uint8_t* readFabricationZone(uint8_t* buffer)
{
	readSMC(2, 0, buffer);
	return buffer;
}

/*!	\fn		readIssuerZone(uint8_t* buffer)
*	\brief	Read the issuer zone (security mode 1&2)
*	\param	buffer	Pointer to a buffer (8 bytes required)
*	\return	The provided pointer
*/
uint8_t* readIssuerZone(uint8_t* buffer)
{
	readSMC(10, 2, buffer);
	return buffer;
}

/*!	\fn		writeIssuerZone(uint8_t* buffer)
*	\brief	Write in the issuer zone (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (8 bytes required)
*/
void writeIssuerZone(uint8_t* buffer)
{
	writeSMC(16, 64, buffer);
}

/*!	\fn		readSecurityCode(uint8_t* buffer)
*	\brief	Read the security code (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*	\return	The provided pointer
*/
uint8_t* readSecurityCode(uint8_t* buffer)
{
	readSMC(12, 10, buffer);
	return buffer;
}

/*!	\fn		writeSecurityCode(uint8_t* buffer)
*	\brief	Write a new security code (security mode 1&2 - Authenticated!)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*/
void writeSecurityCode(uint8_t* buffer)
{
	writeSMC(80, 16, buffer);
}

/*!	\fn		readSecurityCodeAttemptsCounters(uint8_t* buffer)
*	\brief	Read the number of code attempts left (security mode 1&2)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*	\return	The provided pointer
*/
uint8_t* readSecurityCodeAttemptsCounters(uint8_t* buffer)
{
	readSMC(14, 12, buffer);
	return buffer;
}

/*!	\fn		readCodeProtectedZone(uint8_t* buffer)
*	\brief	Read the code protected zone (security mode 1&2 - Authenticated!)
*	\param	buffer	Pointer to a buffer (8 bytes required)
*	\return	The provided pointer
*/
uint8_t* readCodeProtectedZone(uint8_t* buffer)
{
	readSMC(22, 14, buffer);
	return buffer;
}

/*!	\fn		writeCodeProtectedZone(uint8_t* buffer)
*	\brief	Write in the code protected zone (security mode 1&2 - Authenticated!)
*	\param	buffer	Pointer to a buffer (8 bytes required)
*/
void writeCodeProtectedZone(uint8_t* buffer)
{
	writeSMC(112, 64, buffer);
}

/*!	\fn		readApplicationZone1EraseKey(uint8_t* buffer)
*	\brief	Read the application zone1 erase key (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (6 bytes required)
*	\return	The provided pointer
*/
uint8_t* readApplicationZone1EraseKey(uint8_t* buffer)
{
	readSMC(92, 86, buffer);
	return buffer;
}

/*!	\fn		writeApplicationZone1EraseKey(uint8_t* buffer)
*	\brief	Write the application zone1 erase key (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (6 bytes required)
*/
void writeApplicationZone1EraseKey(uint8_t* buffer)
{
	writeSMC(688, 48, buffer);
}

/*!	\fn		readApplicationZone2EraseKey(uint8_t* buffer)
*	\brief	Read the application zone2 erase key (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (4 bytes required)
*	\return	The provided pointer
*/
uint8_t* readApplicationZone2EraseKey(uint8_t* buffer)
{
	readSMC(160, 156, buffer);
	return buffer;
}

/*!	\fn		writeApplicationZone2EraseKey(uint8_t* buffer)
*	\brief	Write the application zone2 erase key (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (4 bytes required)
*/
void writeApplicationZone2EraseKey(uint8_t* buffer)
{
	writeSMC(1248, 32, buffer);
}

/*!	\fn		readMemoryTestZone(uint8_t* buffer)
*	\brief	Read the Test zone (security mode 1&2)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*	\return	The provided pointer
*/
uint8_t* readMemoryTestZone(uint8_t* buffer)
{
	readSMC(178, 176, buffer);
	return buffer;
}

/*!	\fn		writeMemoryTestZone(uint8_t* buffer)
*	\brief	Write in the Test zone (security mode 1&2)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*/
void writeMemoryTestZone(uint8_t* buffer)
{
	writeSMC(1408, 16, buffer);
}

/*!	\fn		readManufacturerZone(uint8_t* buffer)
*	\brief	Read the manufacturer zone (security mode 1&2)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*	\return	The provided pointer
*/
uint8_t* readManufacturerZone(uint8_t* buffer)
{
	readSMC(180, 178, buffer);
	return buffer;
}

/*!	\fn		writeManufacturerZone(uint8_t* buffer)
*	\brief	Write in the manufacturer zone (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*/
void writeManufacturerZone(uint8_t* buffer)
{
	writeSMC(1424, 16, buffer);
}

/*!	\fn		writeManufacturerFuse(void)
*	\brief	Write manufacturer fuse, controlling access to the MFZ
*/
void writeManufacturerFuse(void)
{
	blowManufacturerNIssuerFuse(TRUE);
}

/*!	\fn		write_issuers_fuse(void)
*	\brief	Write issuers fuse, setting the AT88SC102 into Security Mode 2, we need to be authenticated here
*/
void write_issuers_fuse(void)
{
	blowManufacturerNIssuerFuse(FALSE);
}

/*!	\fn		setAuthenticatedReadWriteAccessToZone1(void)
*	\brief	Function called to only allow reads and writes to the application zone 1 when authenticated
*/
void setAuthenticatedReadWriteAccessToZone1(void)
{
	// Set P1 to 1 to allow write, remove R1 to prevent non authenticated reads
	uint8_t temp_buffer[2] = {0x80, 0x00};
	writeSMC(176, 16, temp_buffer);
}

/*!	\fn		checkAuthenticatedReadWriteAccessToZone1(void)
*	\brief	Function called to check that only reads and writes are allowed to the application zone 1 when authenticated
*	\return	OK or NOK
*/
RET_TYPE checkAuthenticatedReadWriteAccessToZone1(void)
{
	uint8_t temp_buffer[2];
	
	readSMC(24, 22, temp_buffer);
	
	if((temp_buffer[0] == 0x80) && (temp_buffer[1] == 0x00))
		return RETURN_OK;
	else
		return RETURN_NOK;
}

/*!	\fn		setAuthenticatedReadWriteAccessToZone2(void)
*	\brief	Function called to only allow reads and writes to the application zone 2 when authenticated
*/
void setAuthenticatedReadWriteAccessToZone2(void)
{
	// Set P2 to 1 to allow write, remove R2 to prevent non authenticated reads
	uint8_t temp_buffer[2] = {0x80, 0x00};
	writeSMC(736, 16, temp_buffer);
}

/*!	\fn		checkAuthenticatedReadWriteAccessToZone2(void)
*	\brief	Function called to check that only reads and writes are allowed to the application zone 2 when authenticated
*	\return	OK or NOK
*/
RET_TYPE checkAuthenticatedReadWriteAccessToZone2(void)
{
	uint8_t temp_buffer[2];
	
	readSMC(94, 92, temp_buffer);
	
	if((temp_buffer[0] == 0x80) && (temp_buffer[1] == 0x00))
		return RETURN_OK;
	else
		return RETURN_NOK;
}

/*!	\fn		printSMCDebugInfoToScreen(void)
*	\brief	Print the card info
*/
void printSMCDebugInfoToScreen(void)
{
	#ifdef DEBUG_SMC_SCREEN_PRINT
		uint8_t data_buffer[20];
		char temp_string[10];
		uint8_t i;
	
		/* Clear screen */
		clear_screen();
	
		/* Read FZ */
		hexaint_to_string(swap16(*(uint16_t*)readFabricationZone(data_buffer)), temp_string);
		Show_String("FZ:", FALSE, 2, 0);
		Show_String(temp_string, FALSE, 11, 0);
	
		/* Read SC */
		hexaint_to_string(swap16(*(uint16_t*)readSecurityCode(data_buffer)), temp_string);
		Show_String("SC:", FALSE, 20, 0);
		Show_String(temp_string, FALSE, 29, 0);
	
		/* Extrapolate security mode */
		if(swap16(*(uint16_t*)readSecurityCode(data_buffer)) == 0xFFFF)
			Show_String("Security mode 2", FALSE, 2, 48);
		else
			Show_String("Security mode 1", FALSE, 2, 48);
	
		/* Read SCAC */
		hexaint_to_string(swap16(*(uint16_t*)readSecurityCodeAttemptsCounters(data_buffer)), temp_string);
		Show_String("SCAC:", FALSE, 38, 0);
		Show_String(temp_string, FALSE, 49, 0);
	
		/* Read IZ */
		readIssuerZone(data_buffer);
		Show_String("IZ:", FALSE, 2, 8);
		for(i = 0; i < 4; i++)
		{
			hexaint_to_string(swap16(*(uint16_t*)(data_buffer+i*2)), temp_string);
			Show_String(temp_string, FALSE, 11+i*9, 8);
		}
	
		/* Recompose CPZ */
		readCodeProtectedZone(data_buffer);
		Show_String("CPZ:", FALSE, 2, 16);
		for(i = 0; i < 4; i++)
		{
			hexaint_to_string(swap16(*(uint16_t*)(data_buffer+i*2)), temp_string);
			Show_String(temp_string, FALSE, 11+i*9, 16);
		}
	
		/* Read EZ1 */
		readApplicationZone1EraseKey(data_buffer);
		Show_String("EZ1:", FALSE, 2, 24);
		for(i = 0; i < 3; i++)
		{
			hexaint_to_string(swap16(*(uint16_t*)(data_buffer+i*2)), temp_string);
			Show_String(temp_string, FALSE, 11+i*9, 24);
		}
	
		/* Read EZ2 */
		readApplicationZone2EraseKey(data_buffer);
		Show_String("EZ2:", FALSE, 2, 32);
		for(i = 0; i < 2; i++)
		{
			hexaint_to_string(swap16(*(uint16_t*)(data_buffer+i*2)), temp_string);
			Show_String(temp_string, FALSE, 11+i*9, 32);
		}
	
		/* Read MTZ */
		hexaint_to_string(swap16(*(uint16_t*)readMemoryTestZone(data_buffer)), temp_string);
		Show_String("MTZ:", FALSE, 2, 40);
		Show_String(temp_string, FALSE, 11, 40);
	
		/* Read MFZ */
		hexaint_to_string(swap16(*(uint16_t*)readManufacturerZone(data_buffer)), temp_string);
		Show_String("MFZ:", FALSE, 20, 40);
		Show_String(temp_string, FALSE, 29, 40);
	
		/* Show first 2 bytes of AZ1 and AZ2 */
		hexaint_to_string(swap16(*(uint16_t*)readSMC(24,22,data_buffer)), temp_string);
		Show_String("AZ1:", FALSE, 2, 56);
		Show_String(temp_string, FALSE, 11, 56);
		hexaint_to_string(swap16(*(uint16_t*)readSMC(94,92,data_buffer)), temp_string);
		Show_String("AZ2:", FALSE, 20, 56);
		Show_String(temp_string, FALSE, 29, 56);
	#endif
}

/*!	\fn		getNumberOfSecurityCodeTriesLeft(void)
*	\brief	Get the number of security code tries left
*	\return	Number of tries left
*/
uint8_t getNumberOfSecurityCodeTriesLeft(void)
{
	uint8_t temp_buffer[2];
	uint8_t return_val = 0;
	uint8_t i;
	
	readSecurityCodeAttemptsCounters(temp_buffer);
	for(i = 0; i < 4; i++)
	{
		if((temp_buffer[0] >> (4+i)) & 0x01)
			return_val++;
	}
	
	return return_val;
}
