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
#include "usb.h"
#include <avr/io.h>
#include <stdint.h>
#include <spi.h>
#if SPI_FLASH != SPI_USART
    #error "SPI not implemented"
#endif


/*! \fn     memoryBoundaryErrorCallback(void)
*   \brief  Function called when a memory boundary issue occurs
*/
void memoryBoundaryErrorCallback(void)
{
    // We'll add more debug later if needed
    usbPutstr(PSTR("#MBE"));
    while(1);
}

/*! \fn     fillPageReadWriteEraseOpcodeFromAddress(uint16_t pageNumber, uint16_t offset, uint8_t* buffer)
*   \brief  Fill the opcode address field from the page number and offset
*   \param  pageNumber  Page number
*   \param  offset      Offset in the page
*   \param  buffer      Pointer to the buffer to fill
*/
static inline void fillPageReadWriteEraseOpcodeFromAddress(uint16_t pageNumber, uint16_t offset, uint8_t* buffer)
{
    #if (READ_OFFSET_SHT_AMT != WRITE_SHT_AMT) || (READ_OFFSET_SHT_AMT != PAGE_ERASE_SHT_AMT)
        #error "read / write / erase bitshifts differ"
    #endif
    uint16_t temp_uint = (pageNumber << (READ_OFFSET_SHT_AMT-8)) | (offset >> 8);
    buffer[0] = (uint8_t)(temp_uint >> 8);
    buffer[1] = (uint8_t)temp_uint;
    buffer[2] = (uint8_t)offset;
}


/*! \fn     sendDataToFlashWithFourBytesOpcode(uint8_t* opcode, uint8_t* buffer, uint16_t buffer_size)
*   \brief  Send data with a four bytes opcode to flash
*   \param  opcode  Pointer to 4 bytes long opcode
*/
void sendDataToFlashWithFourBytesOpcode(uint8_t* opcode, uint8_t* buffer, uint16_t buffer_size)
{
    /* Assert chip select */
    PORT_FLASH_nS &= ~(1 << PORTID_FLASH_nS);

    // Send opcode
    for (uint8_t i = 0; i < 4; i++)
    {
        *opcode = spiUsartTransfer(*opcode);
        opcode++;
    }
    
    // Retrieve data
    while (buffer_size--)
    {
        *buffer = spiUsartTransfer(*buffer);
        buffer++;
    }    
    
    /* Deassert chip select */
    PORT_FLASH_nS |= (1 << PORTID_FLASH_nS);
}

/**
 * Waits for the flash to be ready (polls the flash chip status register)
 * @return  success status
 */
void waitForFlash(void)
{
    /* Assert chip select */
    PORT_FLASH_nS &= ~(1 << PORTID_FLASH_nS);
    
    uint8_t tempBool = TRUE;
    while(tempBool == TRUE)
    {
        spiUsartTransfer(FLASH_OPCODE_READ_STAT_REG);
        if(spiUsartTransfer(0)&FLASH_READY_BITMASK)
        {
            tempBool = FALSE;
        }            
    }
    
    /* Deassert chip select */
    PORT_FLASH_nS |= (1 << PORTID_FLASH_nS);
} // End waitForFlash

/**
 * Attempts to read the Manufacturers Information Register.
 * @note    Performs a comparison to verify the size of the flash chip
 * @return  success status
 */
static inline RET_TYPE checkFlashID(void)
{
    uint8_t dataBuffer[4];
    
    // Set the first byte to the correct op code
    dataBuffer[0] = FLASH_OPCODE_READ_DEV_INFO;
    
    /* Read flash identification */
    sendDataToFlashWithFourBytesOpcode(dataBuffer, dataBuffer, 0);
    
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
    return checkFlashID();
} // End initFlash

/**
 * Erases sector 0a if sectorNumber is FLASH_SECTOR_ZERO_A_CODE. Deletes sector 0b if sectorNumber is FLASH_SECTOR_ZERO_B_CODE.
 * @param   sectorNumber    The sector to erase
 * @note    Sets all bits in sector to Logic 1 (High)
 */
void sectorZeroErase(uint8_t sectorNumber)
{
    uint8_t opcode[4];
    
    #ifdef MEMORY_BOUNDARY_CHECKS
        // Error check parameter sectorNumber
        if(!(sectorNumber == FLASH_SECTOR_ZERO_A_CODE || sectorNumber == FLASH_SECTOR_ZERO_B_CODE))
        {
            memoryBoundaryErrorCallback();
        }    
    #endif
    
    uint16_t temp_uint = (uint16_t)sectorNumber << (SECTOR_ERASE_0_SHT_AMT-8);
    opcode[0] = FLASH_OPCODE_SECTOR_ERASE;
    opcode[1] = (uint8_t)(temp_uint >> 8);
    opcode[2] = (uint8_t)temp_uint;
    opcode[3] = 0;    
    sendDataToFlashWithFourBytesOpcode(opcode, opcode, 0);
    
    /* Wait until memory is ready */
    waitForFlash();
} // End sectorZeroErase

/**
 * Erases sector sectorNumber (SECTOR_START -> SECTOR_END inclusive valid).
 * @param   sectorNumber    The sector to erase
 * @note    Sets all bits in sector to Logic 1 (High)
 */
void sectorErase(uint8_t sectorNumber)
{
    uint8_t opcode[4];
    
    #ifdef MEMORY_BOUNDARY_CHECKS
        // Error check parameter sectorNumber
        if((sectorNumber < SECTOR_START) || (sectorNumber > SECTOR_END)) // Ex: 1M -> SECTOR_START = 1, SECTOR_END = 3  sectorNumber must be 1, 2, or 3
        {
            memoryBoundaryErrorCallback();
        }
    #endif
    
    uint16_t temp_uint = (uint16_t)sectorNumber << (SECTOR_ERASE_N_SHT_AMT-8);
    opcode[0] = FLASH_OPCODE_SECTOR_ERASE;
    opcode[1] = (uint8_t)(temp_uint >> 8);
    opcode[2] = (uint8_t)temp_uint;
    opcode[3] = 0;    
    sendDataToFlashWithFourBytesOpcode(opcode, opcode, 0);
    
    /* Wait until memory is ready */
    waitForFlash();   
} // End sectorErase

/**
 * Erases block blockNumber (0 up to BLOCK_COUNT valid).
 * @param   blockNumber     The block to erase
 * @return  success status
 * @note    Sets all bits in block to Logic 1 (High)
 */
void blockErase(uint16_t blockNumber)
{
    uint8_t opcode[4];
    
    #ifdef MEMORY_BOUNDARY_CHECKS
        // Error check parameter blockNumber
        if(blockNumber >= BLOCK_COUNT)// Ex: 1M -> BLOCK_COUNT = 64.. valid pageNumber 0-63
        {
            memoryBoundaryErrorCallback();
        }
    #endif
    
    uint16_t temp_uint = blockNumber << (BLOCK_ERASE_SHT_AMT-8);
    opcode[0] = FLASH_OPCODE_BLOCK_ERASE;
    opcode[1] = (uint8_t)(temp_uint >> 8);
    opcode[2] = (uint8_t)temp_uint;
    opcode[3] = 0;
    sendDataToFlashWithFourBytesOpcode(opcode, opcode, 0);
    
    /* Wait until memory is ready */
    waitForFlash();
} // End blockErase

/**
 * Erases page pageNumber (0 up to PAGE_COUNT valid).
 * @param   pageNumber      The page to erase
 * @return  success status
 * @note    Sets all bits in page to Logic 1 (High)
 */
void pageErase(uint16_t pageNumber)
{
    uint8_t opcode[4];
    
    #ifdef MEMORY_BOUNDARY_CHECKS
        // Error check parameter pageNumber
        if(pageNumber >= PAGE_COUNT) // Ex: 1M -> PAGE_COUNT = 512.. valid pageNumber 0-511
        {
            memoryBoundaryErrorCallback();
        }
    #endif
    
    opcode[0] = FLASH_OPCODE_PAGE_ERASE;
    fillPageReadWriteEraseOpcodeFromAddress(pageNumber, 0, &opcode[1]);    // We can add the offset as they're "don't care" in the datasheet
    sendDataToFlashWithFourBytesOpcode(opcode, opcode, 0);
    
    /* Wait until memory is ready */
    waitForFlash();
} // End pageErase

/**
 * Erases the entirety of spi flash memory by calling the appropriate erase functions.
 * @note    Sets all bits in spi flash memory to Logic 1 (High)
 */
void formatFlash(void) 
{    
    sectorZeroErase(FLASH_SECTOR_ZERO_A_CODE); // erase sector 0a
    sectorZeroErase(FLASH_SECTOR_ZERO_B_CODE); // erase sector 0b
    
    for(uint8_t i = SECTOR_START; i <= SECTOR_END; i++)
    {
        sectorErase(i);
    }
}

/**
 * Writes a data buffer to flash memory. The data is written starting at offset of a page.  
 * @param   pageNumber      The target page number of flash memory
 * @param   offset          The starting byte offset to begin writing in pageNumber
 * @param   dataSize        The number of bytes to write from the data buffer (assuming the data buffer is sufficiently large)
 * @param   data            The buffer containing the data to write to flash memory
 * @note    The buffer will be destroyed.
 * @note    Function does not allow crossing page boundaries.
 */
void writeDataToFlash(uint16_t pageNumber, uint16_t offset, uint16_t dataSize, void *data)
{
    uint8_t opcode[4];
    
    #ifdef MEMORY_BOUNDARY_CHECKS
        // Error check the parameter pageNumber
        if(pageNumber >= PAGE_COUNT) // Ex: 1M -> PAGE_COUNT = 512.. valid pageNumber 0-511
        {
            memoryBoundaryErrorCallback();
        }
    
        // Error check the parameters offset and dataSize
        if((offset + dataSize - 1) >= BYTES_PER_PAGE) // Ex: 1M -> BYTES_PER_PAGE = 264 offset + dataSize MUST be less than 264 (0-263 valid)
        {
            memoryBoundaryErrorCallback();
        }
    #endif
    
    // Load the page in the internal buffer
    opcode[0] = FLASH_OPCODE_MAINP_TO_BUF;
    fillPageReadWriteEraseOpcodeFromAddress(pageNumber, offset, &opcode[1]);    // We can add the offset as they're "don't care" in the datasheet
    sendDataToFlashWithFourBytesOpcode(opcode, opcode, 0);                      // Send command
    
    /* Wait until memory is ready */
    waitForFlash();
    
    // Write the byte in the buffer, write the buffer to page
    opcode[0] = FLASH_OPCODE_MMP_PROG_TBUF;
    fillPageReadWriteEraseOpcodeFromAddress(pageNumber, offset, &opcode[1]); 
    sendDataToFlashWithFourBytesOpcode(opcode, data, dataSize);
    
    /* Wait until memory is ready */
    waitForFlash();
} // End writeDataToFlash

/**
 * Reads a data buffer of flash memory. The data is read starting at offset of a page.  
 * @param   pageNumber      The target page number of flash memory
 * @param   offset          The starting byte offset to begin reading in pageNumber
 * @param   dataSize        The number of bytes to read from the flash memory into the data buffer (assuming the data buffer is sufficiently large)
 * @param   data            The buffer used to store the data read from flash
 * @note    Function does not allow crossing page boundaries.
 */
void readDataFromFlash(uint16_t pageNumber, uint16_t offset, uint16_t dataSize, void *data)
{    
    uint8_t opcode[4];
    
    #ifdef MEMORY_BOUNDARY_CHECKS
        // Error check the parameter pageNumber
        if(pageNumber >= PAGE_COUNT) // Ex: 1M -> PAGE_COUNT = 512.. valid pageNumber 0-511
        {
            memoryBoundaryErrorCallback();
        }    
        // Error check the parameters offset and dataSize
        if((offset + dataSize - 1) >= BYTES_PER_PAGE) // Ex: 1M -> BYTES_PER_PAGE = 264 offset + dataSize MUST be less than 264 (0-263 valid)
        {
            memoryBoundaryErrorCallback();
        }
    #endif
    
    opcode[0] = FLASH_OPCODE_LOWF_READ;
    fillPageReadWriteEraseOpcodeFromAddress(pageNumber, offset, &opcode[1]);
    sendDataToFlashWithFourBytesOpcode(opcode, data, dataSize);
} // End readDataFromFlash

/**
 * Contiguous data read across flash page boundaries with a max 65k bytes addressing space
 * @param   datap           pointer to the buffer to store the read data
 * @param   addr            byte offset in the flash
 * @param   size            the number of bytes to read
 * @note bypasses the memory buffer
 */
void flashRawRead(uint8_t* datap, uint16_t addr, uint16_t size)
{    
    addr = ((addr/BYTES_PER_PAGE) << READ_OFFSET_SHT_AMT) | (addr % BYTES_PER_PAGE);    
    uint8_t op[] = {FLASH_OPCODE_LOWF_READ, 0x00, (uint8_t)(addr >> 8), (uint8_t)addr};            

    /* Read from flash */
    sendDataToFlashWithFourBytesOpcode(op, datap, size);
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
    uint8_t op[4];
    
    op[0] = FLASH_OPCODE_BUF_WRITE;
    fillPageReadWriteEraseOpcodeFromAddress(0, offset, &op[1]);
    sendDataToFlashWithFourBytesOpcode(op, datap, size);
    waitForFlash();
}

/**
 * write the contents of the internal memory buffer to a page in flash
 * @param   page the page to store the buffer in
 */
void flashWriteBufferToPage(uint16_t page)
{
    uint8_t op[4];
    
    op[0] = FLASH_OPCODE_BUF_TO_PAGE;
    fillPageReadWriteEraseOpcodeFromAddress(page, 0, &op[1]);
    sendDataToFlashWithFourBytesOpcode(op, op, 0);
    waitForFlash();
}