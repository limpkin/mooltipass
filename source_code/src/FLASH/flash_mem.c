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

/* Copyright (c) 2014, Michael Neiderhauser. All rights reserved. */

/*!  \file     flash_mem.c
*    \brief    Mooltipass Flash IC Library
*    Created:  31/3/2014
*    Author:   Michael Neiderhauser
*/

#include "flash_mem.h"
#include "defines.h"
#include <avr/io.h>
#include <stdint.h>
#include <spi.h>

#if SPI_FLASH != SPI_USART
#error "SPI not implemented"
#endif


/*!  \fn       sendDataToFlash(uint8_t opcodeSize, uint8_t* opcode, uint16_t bufferSize, uint8_t* buffer)
*    \brief    Send bytes to the flash memory. WARNING THE DATA IN THE BUFFER MAY BE DESTROYED
*    \param    opcodeSize        The number of bytes for the opcode
*    \param    opcode            Pointer to the opcode
*    \param    bufferSize        The number of bytes
*    \param    buffer            Pointer to the buffer
*    \return   Success status
*/
RET_TYPE sendDataToFlash(uint8_t opcodeSize, void *opcode, uint16_t bufferSize, void *buffer)
{
    /* Assert chip select */
    PORT_FLASH_nS &= ~(1 << PORTID_FLASH_nS);

    spiUsartWrite((uint8_t *)opcode, opcodeSize);
    while (bufferSize--) 
    {
        *(uint8_t *)buffer = spiUsartTransfer(*(uint8_t *)buffer);
        buffer++;
    }

    /* Deassert chip select */
    PORT_FLASH_nS |= (1 << PORTID_FLASH_nS);
    return RETURN_OK;
} // End sendDataToFlash

/*!  \fn      waitForFlash(void)
*    \brief   Wait for the flash to be ready (poll status register)
*    \return  Success status
*/
RET_TYPE waitForFlash(void)
{
    uint8_t opcode[2];
    uint8_t tempBool;
    
    opcode[0] = FLASH_OPCODE_READ_STAT_REG;
    tempBool = TRUE;
    while(tempBool == TRUE)
    {
        sendDataToFlash(1, opcode, 1, opcode+1);
        if(opcode[1]&FLASH_READY_BITMASK)
        {
            tempBool = FALSE;
        }            
    }
    return RETURN_OK;
} // End waitForFlash

/*!  \fn        checkFlashID(void)
*    \brief     Check the presence of the flash (manf info)
*    \return    Success status
*/
RET_TYPE checkFlashID(void)
{
    uint8_t dataBuffer[5] = {FLASH_OPCODE_READ_DEV_INFO , 0x00, 0x00, 0x00, 0x00};
    
    /* Read flash identification */
    sendDataToFlash(1, dataBuffer, 4, dataBuffer+1);
    
    /* Check ID */
    if((dataBuffer[1] != FLASH_MANUF_ID) || (dataBuffer[2] != MAN_FAM_DEN_VAL))
    {
        return RETURN_NOK;
    }
    else
    {
        return RETURN_OK;
    }
} // End checkFlashID

/*!  \fn        initFlash(void)
*    \brief     Initialize the flash memory (Sets up SPI)
*    \return    Success statusDD
*/
RET_TYPE initFlash(void)
{
    /* Setup chip select signal */
    DDR_FLASH_nS |= (1 << PORTID_FLASH_nS);
    PORT_FLASH_nS |= (1 << PORTID_FLASH_nS);
    
    /*  Check flash identification */
    if(checkFlashID() != RETURN_OK)
    {
        return RETURN_NOK;
    }
    else
    {
        return RETURN_OK;
    }
} // End initFlash

/*!  \fn       sectorZeroErase(uint8_t sectorNumber)
*    \brief    Deletes sub-section of sector 0X (see datasheet)
*    \param    sectorNumber     The sub-section of sector 0 to erase (0 for 0a, 1 for 0b)
*    \return   Success status.  Will return RETURN_NOK if sectorNumber is not 0 or 1
*/
RET_TYPE sectorZeroErase(uint8_t sectorNumber)
{
    uint32_t procBuff = (uint32_t)sectorNumber;
    uint8_t opcode[4];
    
    // Error check parameter sectorNumber
    if(!(sectorNumber == FLASH_SECTOR_ZERO_A_CODE || sectorNumber == FLASH_SECTOR_ZERO_B_CODE))
    {
        return RETURN_NOK;
    }
    
    // Format procBuff
    procBuff = (procBuff << (SECTOR_ERASE_0_SHT_AMT));
    
    // Extract procBuff into required 3 address bytes (see datasheet)
    opcode[0] = FLASH_OPCODE_SECTOR_ERASE;
    opcode[1] = (uint8_t)((procBuff & 0x00FF0000) >> 16);  // High byte
    opcode[2] = (uint8_t)((procBuff & 0x0000FF00) >> 8);   // Mid byte
    opcode[3] = (uint8_t)(procBuff & 0x000000FF);        // Low byte
    
    sendDataToFlash(4, opcode, 0, opcode);
    /* Wait until memory is ready */
    waitForFlash();
    return RETURN_OK;
} // End sectorZeroErase

/*!  \fn       sectorErase(uint16_t sectorNumber)
*    \brief    Erases a sector of flash memory (see datasheet)
*    \param    sectorNumber        The sector number in flash to erase
*    \return   Success status.     Will return RETURN_NOK if parameters are out of range. Otherwise will return RETURN_OK.
*/
RET_TYPE sectorErase(uint8_t sectorNumber)
{
    uint32_t procBuff = (uint32_t)sectorNumber;
    uint8_t opcode[4];
    
    // Error check parameter sectorNumber
    if((sectorNumber < SECTOR_START) || (sectorNumber > SECTOR_END)) // Ex: 1M -> SECTOR_START = 1, SECTOR_END = 3  sectorNumber must be 1, 2, or 3
    {
        return RETURN_NOK;
    }
    
    // Format procBuff
    procBuff = (procBuff << (SECTOR_ERASE_N_SHT_AMT));
    
    // Extract procBuff into required 3 address bytes (see datasheet)
    opcode[0] = FLASH_OPCODE_SECTOR_ERASE;
    opcode[1] = (uint8_t)((procBuff & 0x00FF0000) >> 16);  // High byte
    opcode[2] = (uint8_t)((procBuff & 0x0000FF00) >> 8);   // Mid byte
    opcode[3] = (uint8_t)(procBuff & 0x000000FF);        // Low byte
    
    sendDataToFlash(4, opcode, 0, opcode);
    /* Wait until memory is ready */
    waitForFlash();
    return RETURN_OK;    
} // End sectorErase

/*!  \fn       blockErase(uint16_t blockNumber)
*    \brief    Erases a block of flash memory (see datasheet)
*    \param    blockNumber       The block number in flash to erase
*    \return   Success status.     Will return RETURN_NOK if parameters are out of range. Otherwise will return RETURN_OK.
*/
RET_TYPE blockErase(uint16_t blockNumber)
{
    uint32_t procBuff = (uint32_t)blockNumber;
    uint8_t opcode[4];
    
    // Error check parameter blockNumber
    if(blockNumber >= BLOCK_COUNT)// Ex: 1M -> BLOCK_COUNT = 64.. valid pageNumber 0-63
    {
        return RETURN_NOK;
    }
    
    // Format procBuff
    procBuff = (procBuff << (BLOCK_ERASE_SHT_AMT));
    
    // Extract procBuff into required 3 address bytes (see datasheet)
    opcode[0] = FLASH_OPCODE_BLOCK_ERASE;
    opcode[1] = (uint8_t)((procBuff & 0x00FF0000) >> 16);  // High byte
    opcode[2] = (uint8_t)((procBuff & 0x0000FF00) >> 8);   // Mid byte
    opcode[3] = (uint8_t)(procBuff & 0x000000FF);        // Low byte
    
    sendDataToFlash(4, opcode, 0, opcode);
    /* Wait until memory is ready */
    waitForFlash();
    return RETURN_OK;
} // End blockErase

/*!  \fn       pageErase(uint16_t pageNumber)
*    \brief    Erases a page of flash memory (see datasheet)
*    \param    pageNumber        The page number if flash to erase
*    \return   Success status.     Will return RETURN_NOK if parameters are out of range. Otherwise will return RETURN_OK.
*/
RET_TYPE pageErase(uint16_t pageNumber)
{
    uint32_t procBuff = (uint32_t)pageNumber;
    uint8_t opcode[4];
    
    // Error check parameter pageNumber
    if(pageNumber >= PAGE_COUNT) // Ex: 1M -> PAGE_COUNT = 512.. valid pageNumber 0-511
    {
        return RETURN_NOK;
    }
    
    // Format procBuff
    procBuff = (procBuff << (PAGE_ERASE_SHT_AMT));
    
    // Extract procBuff into required 3 address bytes (see datasheet)
    opcode[0] = FLASH_OPCODE_PAGE_ERASE;
    opcode[1] = (uint8_t)((procBuff & 0x00FF0000) >> 16);  // High byte
    opcode[2] = (uint8_t)((procBuff & 0x0000FF00) >> 8);   // Mid byte
    opcode[3] = (uint8_t)(procBuff & 0x000000FF);        // Low byte
    
    sendDataToFlash(4, opcode, 0, opcode);
    /* Wait until memory is ready */
    waitForFlash();
    return RETURN_OK;
} // End pageErase

/*!  \fn       formatFlash()
*    \brief    Formats / Erases all of the flash memory using sectorZeroErase and sectorErase. Sets all bits in flash.
*    \return   Success status.
*/
RET_TYPE formatFlash() 
{
	RET_TYPE ret = RETURN_OK;
	
	ret = sectorZeroErase(FLASH_SECTOR_ZERO_A_CODE); // erase sector 0a
	if(ret != RETURN_OK)
	{
		return ret;
	}
	sectorZeroErase(FLASH_SECTOR_ZERO_B_CODE); // erase sector 0b
	if(ret != RETURN_OK)
	{
		return ret;
	}
	
	for(uint8_t i = SECTOR_START; i <= SECTOR_END; i++)
	{
		ret = sectorErase(i);
		if(ret != RETURN_OK)
		{
			return ret;
		}
	}	
	return ret;
}

/*!  \fn       writeDataToFlash(uint16_t pageNumber, uint16_t offset, uint16_t dataSize, uint8_t* data)
*    \brief    Write (dataSize starting at offset of pageNumber of) data to flash (see datasheet)
*    \param    pageNumber        The page number if flash to write
*    \param    offset            The byte offset in the page
*    \param    dataSize          The number of bytes in the buffer to write (size_of buffer to write entire buffer)
*    \param    data              The buffer (data structure) to write to flash
*    \return   Success status.     Will return RETURN_NOK if parameters are out of range. Otherwise will return RETURN_OK.
*/
RET_TYPE writeDataToFlash(uint16_t pageNumber, uint16_t offset, uint16_t dataSize, void *data)
{
    uint8_t opcode[4];
    uint32_t procBuff = (uint32_t)pageNumber;
    
    // Error check the parameter pageNumber
    if(pageNumber >= PAGE_COUNT) // Ex: 1M -> PAGE_COUNT = 512.. valid pageNumber 0-511
    {
        return RETURN_INVALID_PARAM;
    }
    
    // Error check the parameters offset and dataSize
    if((offset + dataSize - 1) >= BYTES_PER_PAGE) // Ex: 1M -> BYTES_PER_PAGE = 264 offset + dataSize MUST be less than 264 (0-263 valid)
    {
        return RETURN_INVALID_PARAM;
    }

    // Shift page address over WRITE_SHT_AMT
    procBuff = (procBuff << (WRITE_SHT_AMT));
    //procBuff = (procBuff<<(WRITE_SHT_AMT)) & 0xFFFFFE00;
    
    // Extract procBuff into required 3 address bytes (see datasheet)
    opcode[0] = FLASH_OPCODE_MAINP_TO_BUF;
    opcode[1] = (uint8_t)((procBuff & 0x00FF0000) >> 16);  // High byte
    opcode[2] = (uint8_t)((procBuff & 0x0000FF00) >> 8);   // Mid byte
    opcode[3] = (uint8_t)(procBuff & 0x000000FF);        // Low byte
    sendDataToFlash(4, opcode, 0, opcode);
    
    /* Wait until memory is ready */
    waitForFlash();
    
    // Add offset value
    procBuff |= (uint32_t)offset;
    //procBuff |= ((uint32_t)offset) & 0x000001FF;

    
    // Extract procBuff into required 3 address bytes (see datasheet)
    opcode[0] = FLASH_OPCODE_MMP_PROG_TBUF;
    opcode[1] = (uint8_t)((procBuff & 0x00FF0000) >> 16);  // High byte
    opcode[2] = (uint8_t)((procBuff & 0x0000FF00) >> 8);   // Mid byte
    opcode[3] = (uint8_t)(procBuff & 0x000000FF);        // Low byte
    sendDataToFlash(4, opcode, dataSize, data);
    
    /* Wait until memory is ready */
    waitForFlash();
    return RETURN_OK;
} // End writeDataToFlash

/*!  \fn       readDataFromFlash(uint16_t pageNumber, uint16_t offset, uint16_t dataSize, uint8_t* data)
*    \brief    Read (dataSize starting at offset of pageNumber of) flash to data (see datasheet)
*    \param    pageNumber        The page number if flash to read
*    \param    offset            The byte offset in the page
*    \param    dataSize          The number of bytes in the buffer to read (size_of buffer to read entire buffer)
*    \param    data              The buffer to store data from flash
*    \return   Success status.     Will return RETURN_NOK if parameters are out of range. Otherwise will return RETURN_OK.
*/
RET_TYPE readDataFromFlash(uint16_t pageNumber, uint16_t offset, uint16_t dataSize, void *data)
{    
    uint8_t opcode[4];
    uint32_t procBuff = (uint32_t)pageNumber;
    
    // Error check the parameter pageNumber
    if(pageNumber >= PAGE_COUNT) // Ex: 1M -> PAGE_COUNT = 512.. valid pageNumber 0-511
    {
        return RETURN_INVALID_PARAM;
    }
    
    // Error check the parameters offset and dataSize
    if((offset + dataSize - 1) >= BYTES_PER_PAGE) // Ex: 1M -> BYTES_PER_PAGE = 264 offset + dataSize MUST be less than 264 (0-263 valid)
    {
        return RETURN_INVALID_PARAM;
    }

    // Format procBuff
    procBuff = procBuff << READ_OFFSET_SHT_AMT;  // make room for offset
    //procBuff = (procBuff<<(READ_OFFSET_SHT_AMT)) & 0xFFFFFE00;
    procBuff |= (uint32_t)offset; // Add the offset
    //procBuff |= ((uint32_t)offset) & 0x000001FF;
    
    // Extract procBuff into required 3 address bytes (see datasheet)
    opcode[0] = FLASH_OPCODE_LOWF_READ;
    opcode[1] = (uint8_t)((procBuff & 0x00FF0000) >> 16);  // High byte
    opcode[2] = (uint8_t)((procBuff & 0x0000FF00) >> 8);   // Mid byte
    opcode[3] = (uint8_t)(procBuff & 0x000000FF);        // Low byte
    
    sendDataToFlash(4, opcode, dataSize, data);
    /* Wait until memory is ready */
    waitForFlash();
    return RETURN_OK;
} // End readDataFromFlash
