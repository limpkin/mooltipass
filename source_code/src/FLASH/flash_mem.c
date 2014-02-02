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
 * flash_mem.c
 *
 * Created: 08/12/2013 14:16:53
 *  Author: Mathieu Stephan
 */
/*!	\file 	flash_mem.c
*	\brief	Flash memory low level functions
*/
#include "../mooltipass.h"
#include <avr/io.h>


/*!	\fn		send_data_to_flash(uint8_t nb_bytes_opcode, uint8_t* opcode, uint16_t nb_bytes, uint8_t* data_to_send_receive)
*	\brief	Send bytes to the flash memory... beware, buffer contents will be changed!
*	\param	nb_bytes_opcode	The number of bytes for the opcode
*	\param	opcode			Pointer to the opcode
*	\param	nb_bytes		The number of bytes
*	\param	data_to_send_receive	Pointer to the buffer
*/
void sendDataToFlash(uint8_t nb_bytes_opcode, uint8_t* opcode, uint16_t nb_bytes, uint8_t* data_to_send_receive)
{	
	/* Read UDR1 contents to clear previous flags */
	while(UCSR1A & (1<<RXC1))UDR1;
	
	/* Assert chip select */
	PORT_FLASH_nS &= ~(1 << PORTID_FLASH_nS);	
	
	while(nb_bytes_opcode--)
	{
		/* Wait for empty transmit buffer */
		while(!(UCSR1A & (1 << UDRE1)));
		/* Put data into buffer, sends the data */
		UDR1 = *(opcode++);
		/* Wait for data to be received */
		while(!(UCSR1A & (1<<RXC1)));
		/* Read received data to clear flag */
		UDR1;
	}
	
	while(nb_bytes--)
	{
		/* Wait for empty transmit buffer */
		while(!(UCSR1A & (1 << UDRE1)));
		/* Put data into buffer, sends the data */
		UDR1 = *data_to_send_receive;
		/* Wait for data to be received */
		while(!(UCSR1A & (1<<RXC1)));
		/* Store received data */
		*(data_to_send_receive++) = UDR1;
	}
	
	/* Deassert chip select */
	PORT_FLASH_nS |= (1 << PORTID_FLASH_nS);
}

/*!	\fn		read_credential_block_within_flash_page(uint16_t page_number, uint8_t block_id, uint8_t* buffer)
*	\brief	Read a block of credentials
*	\param	page_number		Page number in the flash
*	\param	block_id		The block ID in the page
*	\param	buffer			Pointer to the buffer
*/
void readCredentialBlock(uint16_t page_number, uint8_t block_id, uint8_t* buffer)
{
	uint16_t byte_addr;
	uint8_t opcode[4];
	
	/* Compute the byte address based on the block ID */
	byte_addr = block_id;
	byte_addr *= CREDENTIAL_BLOCK_SIZE;
	
	opcode[0] = OPCODE_LOWF_READ;
	opcode[1] = (uint8_t)((page_number>>7)&0x03);
	opcode[2] = ((uint8_t)(page_number<<1)&0x0E) | ((uint8_t)(byte_addr>>8)&0x01);
	opcode[3] = (uint8_t)byte_addr;
	
	sendDataToFlash(4, opcode, CREDENTIAL_BLOCK_SIZE, buffer);
}

/*!	\fn		wait_for_flash_memory_read_routine(void)
*	\brief	Wait for the flash to be ready
*/
void waitForFlash(void)
{
	uint8_t opcode[2];
	uint8_t temp_bool;
	
	opcode[0] = OPCODE_READ_STAT_REG;
	temp_bool = TRUE;
	while(temp_bool == TRUE)
	{
		sendDataToFlash(1, opcode, 1, opcode+1);
		if(opcode[1]&READY_FLASH_BITMASK)
			temp_bool = FALSE;
	}
}

/*!	\fn		write_credential_block_within_flash_page(uint16_t page_number, uint8_t block_id, uint8_t* buffer)
*	\brief	Write a block of credentials
*	\param	page_number		Page number in the flash
*	\param	block_id		The block ID in the page
*	\param	buffer			Pointer to the buffer
*/
void writeCredentialBlock(uint16_t page_number, uint8_t block_id, uint8_t* buffer)
{
	uint16_t byte_addr;
	uint8_t opcode[4];
	
	/* Compute the byte address based on the block ID */
	byte_addr = block_id;
	byte_addr *= CREDENTIAL_BLOCK_SIZE;
	
	/* Perform a main memory page to buffer transfer */
	opcode[0] = OPCODE_MAINP_TO_BUFFER;
	opcode[1] = (uint8_t)((page_number>>7)&0x03);
	opcode[2] = ((uint8_t)(page_number<<1)&0x0E);
	opcode[3] = 0;	
	sendDataToFlash(4, opcode, 0, opcode);
	
	/* Wait until memory is ready */
	waitForFlash();
	
	/* Start writing on the buffer and write the page (special opcode, just one command!) */
	opcode[0] = OPCODE_MMP_PROG_TBUFFER;
	opcode[1] = (uint8_t)((page_number>>7)&0x03);
	opcode[2] = ((uint8_t)(page_number<<1)&0x0E) | ((uint8_t)(byte_addr>>8)&0x01);
	opcode[3] = (uint8_t)byte_addr;
	sendDataToFlash(4, opcode, CREDENTIAL_BLOCK_SIZE, buffer);
	
	/* Wait until memory is ready */
	waitForFlash();
}

/*!	\fn		check_flash_memory_id(void)
*	\brief	Check the presence of the flash
*	\return	Success status
*/
RET_TYPE getFlashID(void)
{
	uint8_t data_buffer[5] = {OPCODE_MAN_DEV_ID_READ, 0x00, 0x00, 0x00, 0x00};
	
	/* Read flash identification */
	sendDataToFlash(1, data_buffer, 4, data_buffer+1);
	
	/* Check ID */
	if((data_buffer[1] != FLASH_MANUF_ID))
		return RETURN_NOK;
	else
		return RETURN_OK;
}

/*!	\fn		init_flash_memory(void)
*	\brief	Initialize the flash memory
*	\return	Success statusDD
*/
RET_TYPE initFlash(void)
{	
	/* Setup chip select signal */
	DDR_FLASH_nS |= (1 << PORTID_FLASH_nS);
	PORT_FLASH_nS |= (1 << PORTID_FLASH_nS);
	
	/* Setup SPI interface */
	#if SPI_OLED == SPI_USART
		if(!(UCSR1B & (1 << TXEN1)))
		{
			DDRD |= (1 << 3) | (1 << 5);												// MOSI & SCK as ouputs
			DDRD &= ~(1 << 2);															// MISO as input
			PORTD &= ~(1 << 2);															// Disable pull-up
			UBRR1 = 0x00;																// Set USART baud divider to 0
			UCSR1C = (1 << UMSEL11) | (1 << UMSEL10) | (1 << UCPOL1) | (1 << UCSZ10);	// Enable USART1 as Master SPI mode 3
			UCSR1B = (1 << TXEN1) | (1 << RXEN1);										// Enable receiver and transmitter
			UBRR1 = 0x00;																// Set USART baud divider to 0, final baud rate 8Mbit/s			
		}
	#else
		#error "SPI not implemented"
	#endif
	
	/*  Check flash identification */
	if(getFlashID() != RETURN_OK)
		return RETURN_NOK;
	else
		return RETURN_OK;
}