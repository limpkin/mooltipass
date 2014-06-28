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

/**
 * Send bytes to the flash memory.
 * @param   opcodeSize      The number of bytes for the opcode
 * @param   opcode          Pointer to the opcode. Opcode is used to inform the flash of the operation.
 * @param   bufferSize      The size of the buffer.
 * @param   buffer          Pointer to the buffer.
 * @return  success status
 * @note    The data in the buffer will be destroyed on a write base opcode.
 * @note    The buffer is used to direct data from flash to a user pointer on read. 
 * @note    The buffer is used to provide data to the flash on write.
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

/**
 * Waits for the flash to be ready (polls the flash chip status register)
 * @return  success status
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

/**
 * Attempts to read the Manufacturers Information Register.
 * @note    Performs a comparison to verify the size of the flash chip
 * @return  success status
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

/**
 * Initializes SPI for the Flash Chip
 * @return  success status
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

/**
 * Erases sector 0a if sectorNumber is FLASH_SECTOR_ZERO_A_CODE. Deletes sector 0b if sectorNumber is FLASH_SECTOR_ZERO_B_CODE.
 * @param   sectorNumber    The sector to erase
 * @return  success status
 * @note    Sets all bits in sector to Logic 1 (High)
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

/**
 * Erases sector sectorNumber (SECTOR_START -> SECTOR_END inclusive valid).
 * @param   sectorNumber    The sector to erase
 * @return  success status
 * @note    Sets all bits in sector to Logic 1 (High)
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

/**
 * Erases block blockNumber (0 up to BLOCK_COUNT valid).
 * @param   blockNumber     The block to erase
 * @return  success status
 * @note    Sets all bits in block to Logic 1 (High)
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

/**
 * Erases page pageNumber (0 up to PAGE_COUNT valid).
 * @param   pageNumber      The page to erase
 * @return  success status
 * @note    Sets all bits in page to Logic 1 (High)
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

/**
 * Erases the entirety of spi flash memory by calling the appropriate erase functions.
 * @return  success status
 * @note    Sets all bits in spi flash memory to Logic 1 (High)
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

/**
 * Writes a data buffer to flash memory. The data is written starting at offset of a page.  
 * @param   pageNumber      The target page number of flash memory
 * @param   offset          The starting byte offset to begin writing in pageNumber
 * @param   dataSize        The number of bytes to write from the data buffer (assuming the data buffer is sufficiently large)
 * @param   data            The buffer containing the data to write to flash memory
 * @return  success status
 * @note    The buffer will be destroyed.
 * @note    Function does not allow crossing page boundaries.
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

/**
 * Reads a data buffer of flash memory. The data is read starting at offset of a page.  
 * @param   pageNumber      The target page number of flash memory
 * @param   offset          The starting byte offset to begin reading in pageNumber
 * @param   dataSize        The number of bytes to read from the flash memory into the data buffer (assuming the data buffer is sufficiently large)
 * @param   data            The buffer used to store the data read from flash
 * @return  success status
 * @note    Function does not allow crossing page boundaries.
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

/**
 * Contiguous data read across flash page boundaries
 * @param   datap           pointer to the buffer to store the read data
 * @param   addr            byte offset in the flash
 * @param   size            the number of bytes to read
 * @return  success status
 * @note bypasses the memory buffer
 */
RET_TYPE flashRawRead(uint8_t* datap, uint32_t addr, uint16_t size)
{
    addr = ((addr/BYTES_PER_PAGE) << READ_OFFSET_SHT_AMT) | (addr % BYTES_PER_PAGE);
    uint8_t op[] = {FLASH_OPCODE_LOWF_READ, (uint8_t)(addr >> 16), (uint8_t)(addr >> 8), (uint8_t)addr};
    
    if ((addr+size) >= (uint32_t)BYTES_PER_PAGE*(uint32_t)PAGE_COUNT)
    {
        return RETURN_NOK;
    }                

    /* Read from flash */
    sendDataToFlash(4, op, size, datap);
    /* Wait until memory is ready */
    waitForFlash();
    
    return RETURN_OK;
}

/**
 * Write data into the internal memory buffer
 * @param datap pointer to data to write
 * @param offset offset to start writing to in the internal memory buffer
 * @param size the number of bytes to write
 * @note if the end of the internal buffer is reached then writing will
 *       wrap to the start of the internal buffer.
 */
void flashWriteBuffer(uint8_t* datap, uint16_t offset, uint16_t size)
{
    if (size) 
    {
        uint8_t op[] = {FLASH_OPCODE_BUF_WRITE, 0, (uint8_t)(offset >> 8), (uint8_t)offset};
        sendDataToFlash(4, op, size, datap);
    }
}

/**
 * write the contents of the internal memory buffer to a page in flash
 * @param   page the page to store the buffer in
 * @return  success status
 */
RET_TYPE flashWriteBufferToPage(uint16_t page)
{
    uint32_t addr = (uint32_t)page << WRITE_SHT_AMT;
    
    if (page >= PAGE_COUNT) 
    {
        return RETURN_NOK;
    }

    uint8_t op[] = {FLASH_OPCODE_BUF_TO_PAGE, (uint8_t)(addr >> 16), (uint8_t)(addr >> 8), (uint8_t)addr};
    sendDataToFlash(4, op, 0, 0);
    waitForFlash();
    return RETURN_OK;
}