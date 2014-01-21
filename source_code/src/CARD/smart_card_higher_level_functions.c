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
/*
 * smart_card_higher_level_functions.c
 *
 * Created: 13/01/2014 23:06:54
 *  Author: Mathieu Stephan
 */ 
/*!	\file 	smart_card_higher_level_functions.c
*	\brief	Smart Card high level functions
*/
#include "../mooltipass.h"


/*!	\fn		perform_card_mooltipass_transformation(void)
*	\brief	Transform the card into a mooltipass card (Security mode 1 - Authenticated!)
*	\return	If we succeeded
*/
RET_TYPE perform_card_mooltipass_transformation(void)
{
	uint8_t temp_buffer[20];
	
	/* Check that the security code is readable, ensuring that we are in security mode 1 with SV flag */
	if(swap16(*(uint16_t*)read_security_code(temp_buffer)) == 0xFFFF)
		return RETURN_NOK;
		
	/* Perform block erase, resetting the entire memory excluding FZ/MTZ/MFZ to FFFF... */
	perform_card_reinit();
	
	/* Set new security password, keep zone 1 and zone 2 security key to FFFF... */
	*(uint16_t*)temp_buffer = swap16(SMARTCARD_FACTORY_PIN);
	write_security_code(temp_buffer);
	
	/* Write "hackaday" to issuer zone */
	hm_str_cpy("hackaday", (char*)temp_buffer, 8);
	write_issuer_zone(temp_buffer);
	
	/* Write 2014 to the manufacturer zone */
	*(uint16_t*)temp_buffer = swap16(2014);
	write_manufacturers_zone(temp_buffer);
	
	/* Set application zone 1 and zone 2 permissions: read/write when authenticated only */
	set_application_zone1_authenticated_read_and_write_access();
	set_application_zone2_authenticated_read_and_write_access();
	
	/* Burn manufacturer fuse */
	write_manufacturers_fuse();
	
	/* Burn issuer fuse*/
	write_issuers_fuse();
	
	return RETURN_OK;
}

/*!	\fn		perform_card_reinit(void)
*	\brief	Reinitialize the card to its default settings & default pin (Security mode 1 - Authenticated!)
*/
void perform_card_reinit(void)
{
	uint8_t data_buffer[2] = {0xFF, 0xFF};
	write_to_smartcard(1441, 1, data_buffer);
	*(uint16_t*)data_buffer = swap16(SMARTCARD_FACTORY_PIN);
	write_security_code(data_buffer);
}

/*!	\fn		read_fabrication_zone(uint8_t* buffer)
*	\brief	Read the fabrication zone (security mode 1&2)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*	\return	The provided pointer
*/
uint8_t* read_fabrication_zone(uint8_t* buffer)
{
	read_data_from_smartcard(2, 0, buffer);
	return buffer;
}

/*!	\fn		read_issuer_zone(uint8_t* buffer)
*	\brief	Read the issuer zone (security mode 1&2)
*	\param	buffer	Pointer to a buffer (8 bytes required)
*	\return	The provided pointer
*/
uint8_t* read_issuer_zone(uint8_t* buffer)
{
	read_data_from_smartcard(10, 2, buffer);
	return buffer;
}

/*!	\fn		write_issuer_zone(uint8_t* buffer)
*	\brief	Write in the issuer zone (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (8 bytes required)
*/
void write_issuer_zone(uint8_t* buffer)
{
	write_to_smartcard(16, 64, buffer);
}

/*!	\fn		read_security_code(uint8_t* buffer)
*	\brief	Read the security code (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*	\return	The provided pointer
*/
uint8_t* read_security_code(uint8_t* buffer)
{
	read_data_from_smartcard(12, 10, buffer);
	return buffer;
}

/*!	\fn		write_security_code(uint8_t* buffer)
*	\brief	Write a new security code (security mode 1&2 - Authenticated!)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*/
void write_security_code(uint8_t* buffer)
{
	write_to_smartcard(80, 16, buffer);
}

/*!	\fn		read_security_code_attemps_counter(uint8_t* buffer)
*	\brief	Read the number of code attempts left (security mode 1&2)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*	\return	The provided pointer
*/
uint8_t* read_security_code_attemps_counter(uint8_t* buffer)
{
	read_data_from_smartcard(14, 12, buffer);
	return buffer;
}

/*!	\fn		read_code_protected_zone(uint8_t* buffer)
*	\brief	Read the code protected zone (security mode 1&2 - Authenticated!)
*	\param	buffer	Pointer to a buffer (8 bytes required)
*	\return	The provided pointer
*/
uint8_t* read_code_protected_zone(uint8_t* buffer)
{
	read_data_from_smartcard(22, 14, buffer);
	return buffer;
}

/*!	\fn		write_code_protected_zone(uint8_t* buffer)
*	\brief	Write in the code protected zone (security mode 1&2 - Authenticated!)
*	\param	buffer	Pointer to a buffer (8 bytes required)
*/
void write_code_protected_zone(uint8_t* buffer)
{
	write_to_smartcard(112, 64, buffer);
}

/*!	\fn		read_application_zone1_erase_key(uint8_t* buffer)
*	\brief	Read the application zone1 erase key (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (6 bytes required)
*	\return	The provided pointer
*/
uint8_t* read_application_zone1_erase_key(uint8_t* buffer)
{
	read_data_from_smartcard(92, 86, buffer);
	return buffer;
}

/*!	\fn		write_application_zone1_erase_key(uint8_t* buffer)
*	\brief	Write the application zone1 erase key (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (6 bytes required)
*/
void write_application_zone1_erase_key(uint8_t* buffer)
{
	write_to_smartcard(688, 48, buffer);
}

/*!	\fn		read_application_zone2_erase_key(uint8_t* buffer)
*	\brief	Read the application zone2 erase key (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (4 bytes required)
*	\return	The provided pointer
*/
uint8_t* read_application_zone2_erase_key(uint8_t* buffer)
{
	read_data_from_smartcard(160, 156, buffer);
	return buffer;
}

/*!	\fn		write_application_zone2_erase_key(uint8_t* buffer)
*	\brief	Write the application zone2 erase key (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (4 bytes required)
*/
void write_application_zone2_erase_key(uint8_t* buffer)
{
	write_to_smartcard(1248, 32, buffer);
}

/*!	\fn		read_memory_test_zone(uint8_t* buffer)
*	\brief	Read the Test zone (security mode 1&2)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*	\return	The provided pointer
*/
uint8_t* read_memory_test_zone(uint8_t* buffer)
{
	read_data_from_smartcard(178, 176, buffer);
	return buffer;
}

/*!	\fn		write_memory_test_zone(uint8_t* buffer)
*	\brief	Write in the Test zone (security mode 1&2)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*/
void write_memory_test_zone(uint8_t* buffer)
{
	write_to_smartcard(1408, 16, buffer);
}

/*!	\fn		read_manufacturers_zone(uint8_t* buffer)
*	\brief	Read the manufacturer zone (security mode 1&2)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*	\return	The provided pointer
*/
uint8_t* read_manufacturers_zone(uint8_t* buffer)
{
	read_data_from_smartcard(180, 178, buffer);
	return buffer;
}

/*!	\fn		write_manufacturers_zone(uint8_t* buffer)
*	\brief	Write in the manufacturer zone (security mode 1 - Authenticated!)
*	\param	buffer	Pointer to a buffer (2 bytes required)
*/
void write_manufacturers_zone(uint8_t* buffer)
{
	write_to_smartcard(1424, 16, buffer);
}

/*!	\fn		write_manufacturers_fuse(void)
*	\brief	Write manufacturer fuse, controlling access to the MFZ
*/
void write_manufacturers_fuse(void)
{
	blow_man_nissuer_fuse(TRUE);
}

/*!	\fn		write_issuers_fuse(void)
*	\brief	Write issuers fuse, setting the AT88SC102 into Security Mode 2, we need to be authenticated here
*/
void write_issuers_fuse(void)
{
	blow_man_nissuer_fuse(FALSE);
}

/*!	\fn		set_application_zone1_authenticated_read_and_write_access(void)
*	\brief	Function called to only allow reads and writes to the application zone 1 when authenticated
*/
void set_application_zone1_authenticated_read_and_write_access(void)
{
	// Set P1 to 1 to allow write, remove R1 to prevent non authenticated reads
	uint8_t temp_buffer[2] = {0x80, 0x00};
	write_to_smartcard(176, 16, temp_buffer);
}

/*!	\fn		set_application_zone2_authenticated_read_and_write_access(void)
*	\brief	Function called to only allow reads and writes to the application zone 2 when authenticated
*/
void set_application_zone2_authenticated_read_and_write_access(void)
{
	// Set P2 to 1 to allow write, remove R2 to prevent non authenticated reads
	uint8_t temp_buffer[2] = {0x80, 0x00};
	write_to_smartcard(736, 16, temp_buffer);
}

/*!	\fn		print_smartcard_debug_info(void)
*	\brief	Print the card info
*/
void print_smartcard_debug_info(void)
{
	uint8_t data_buffer[20];
	char temp_string[10];
	uint8_t i;
	
	/* Clear screen */
	clear_screen();
	
	/* Read FZ */
	hexaint_to_string(swap16(*(uint16_t*)read_fabrication_zone(data_buffer)), temp_string);
	Show_String("FZ:", FALSE, 2, 0);
	Show_String(temp_string, FALSE, 11, 0);
	
	/* Read SC */
	hexaint_to_string(swap16(*(uint16_t*)read_security_code(data_buffer)), temp_string);
	Show_String("SC:", FALSE, 20, 0);
	Show_String(temp_string, FALSE, 29, 0);
	
	/* Extrapolate security mode */
	if(swap16(*(uint16_t*)read_security_code(data_buffer)) == 0xFFFF)
		Show_String("Security mode 2", FALSE, 2, 48);
	else
		Show_String("Security mode 1", FALSE, 2, 48);
	
	/* Read SCAC */
	hexaint_to_string(swap16(*(uint16_t*)read_security_code_attemps_counter(data_buffer)), temp_string);
	Show_String("SCAC:", FALSE, 38, 0);
	Show_String(temp_string, FALSE, 49, 0);
	
	/* Read IZ */
	read_issuer_zone(data_buffer);
	Show_String("IZ:", FALSE, 2, 8);
	for(i = 0; i < 4; i++)
	{
		hexaint_to_string(swap16(*(uint16_t*)(data_buffer+i*2)), temp_string);
		Show_String(temp_string, FALSE, 11+i*9, 8);
	}
	
	/* Recompose CPZ */
	read_code_protected_zone(data_buffer);
	Show_String("CPZ:", FALSE, 2, 16);
	for(i = 0; i < 4; i++)
	{
		hexaint_to_string(swap16(*(uint16_t*)(data_buffer+i*2)), temp_string);
		Show_String(temp_string, FALSE, 11+i*9, 16);
	}
	
	/* Read EZ1 */
	read_application_zone1_erase_key(data_buffer);
	Show_String("EZ1:", FALSE, 2, 24);
	for(i = 0; i < 3; i++)
	{
		hexaint_to_string(swap16(*(uint16_t*)(data_buffer+i*2)), temp_string);
		Show_String(temp_string, FALSE, 11+i*9, 24);
	}
	
	/* Read EZ2 */
	read_application_zone2_erase_key(data_buffer);
	Show_String("EZ2:", FALSE, 2, 32);
	for(i = 0; i < 2; i++)
	{
		hexaint_to_string(swap16(*(uint16_t*)(data_buffer+i*2)), temp_string);
		Show_String(temp_string, FALSE, 11+i*9, 32);
	}
	
	/* Read MTZ */
	hexaint_to_string(swap16(*(uint16_t*)read_memory_test_zone(data_buffer)), temp_string);
	Show_String("MTZ:", FALSE, 2, 40);
	Show_String(temp_string, FALSE, 11, 40);
	
	/* Read MFZ */
	hexaint_to_string(swap16(*(uint16_t*)read_manufacturers_zone(data_buffer)), temp_string);
	Show_String("MFZ:", FALSE, 20, 40);
	Show_String(temp_string, FALSE, 29, 40);
	
	/* Show first 2 bytes of AZ1 and AZ2 */
	hexaint_to_string(swap16(*(uint16_t*)read_data_from_smartcard(24,22,data_buffer)), temp_string);
	Show_String("AZ1:", FALSE, 2, 56);
	Show_String(temp_string, FALSE, 11, 56);
	hexaint_to_string(swap16(*(uint16_t*)read_data_from_smartcard(94,92,data_buffer)), temp_string);
	Show_String("AZ2:", FALSE, 20, 56);
	Show_String(temp_string, FALSE, 29, 56);
}

/*!	\fn		get_number_of_security_code_tries_left(void)
*	\brief	Get the number of security code tries left
*	\return	Number of tries left
*/
uint8_t get_number_of_security_code_tries_left(void)
{
	uint8_t temp_buffer[2];
	uint8_t return_val = 0;
	uint8_t i;
	
	read_security_code_attemps_counter(temp_buffer);
	for(i = 0; i < 4; i++)
	{
		if((temp_buffer[0] >> (4+i)) & 0x01)
			return_val++;
	}
	
	return return_val;
}