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

/* Copyright (c) 2014, Darran Hunt. All rights reserved. */

/*!  \file     flash.c
*    \brief    Mooltipass Flash extended functions
*    Created:  24/6/2014
*    Author:   Darran Hunt
*/

#include <stddef.h>
#include <stdint.h>
#include "defines.h"
#include "flash_mem.h"
#include "flash.h"
#include "usb.h"

/**
 * Contiguous data read across flash page boundaries
 * @param   datap           pointer to the buffer to store the read data
 * @param   addr            byte offset in the flash
 * @param   size            the number of bytes to read
 * @return  success status
 * @note bypasses the memory buffer
 */
int flashRead(uint8_t *datap, uint32_t addr, uint16_t size)
{
    if ((addr+size) >= (uint32_t)BYTES_PER_PAGE*(uint32_t)PAGE_COUNT)
    {
        return -1;
    }        
        
    addr = ((addr/BYTES_PER_PAGE) << READ_OFFSET_SHT_AMT) | (addr % BYTES_PER_PAGE);

    uint8_t op[] = {FLASH_OPCODE_LOWF_READ, (uint8_t)(addr >> 16), (uint8_t)(addr >> 8), (uint8_t)addr};

    /* Read from flash */
    sendDataToFlash(4, op, size, datap);
    /* Wait until memory is ready */
    waitForFlash();
    
    return 0;
}

/**
 * Write data into the internal memory buffer
 * @param datap pointer to data to write
 * @param offset offset to start writing to in the internal memory buffer
 * @param size the number of bytes to write
 * @note if the end of the internal buffer is reached then writing will
 *       wrap to the start of the internal buffer.
 */
void flashBufWrite(void *datap, uint16_t offset, uint16_t size)
{
    if (size) {
        uint8_t op[] = {FLASH_OP_BUF_WRITE, 0, (uint8_t)(offset >> 8), (uint8_t)offset};
        sendDataToFlash(4, op, size, datap);
    }
}

/**
 * Load a page from flash into the internal buffer.
 * @param page the page to load
 * @retval 0 success
 * @retval -1 failed, page out of range
 */
int flashBufLoad(uint16_t page)
{
    if (page >= PAGE_COUNT) 
    {
        return -1;
    }
    page <<= (READ_OFFSET_SHT_AMT - 8);

    uint8_t op[] = {FLASH_OP_BUF_LOAD, (uint8_t)(page >> 8), (uint8_t)page, 0};
    sendDataToFlash(4, op, 0, NULL);

    return 0;
}

/**
 * write the contents of the internal memory buffer to a page in flash
 * @param page the page to store the buffer in
 * @param erase set this to true to erase the page before writing
 * @retval 0 success
 * @retval -1 failed, page out of range
 */
int flashBufStore(uint16_t page, bool erase)
{
    if (page >= PAGE_COUNT) 
    {
        return -1;
    }

    //uint32_t addr = (((uint32_t)page) << flashGeom[flashId].pageOffset) | offset;
    //uint8_t op[] = { opcode, ((uint8_t *)&addr)[2], ((uint8_t *)&addr)[1], ((uint8_t *)&addr)[0] };

    page <<= (READ_OFFSET_SHT_AMT - 8);
    uint8_t op[] = {erase ? FLASH_OP_BUF_ERASE_STORE : FLASH_OP_BUF_STORE, (uint8_t)(page >> 8), (uint8_t)page, 0};
    sendDataToFlash(4, op, 0, NULL);

    return 0;
}

/**
 * Write data into a flash page via the internal memory buffer, performing and
 * erase for the target page.
 * @param datap pointer to data to write
 * @param page the flash page to write to
 * @param offset offset to start writing to in the flash page
 * @param size the number of bytes to write
 * @note if the end of the flash page is reached then writing will
 *       wrap to the start of the flash page.
 * @retval 0 success
 * @retval -1 page out of range, or size is too big
 */
int flashWritePage(void *datap, uint16_t page, uint16_t offset, uint16_t size)
{
    if ((page >= PAGE_COUNT) || (size > BYTES_PER_PAGE)) {
        return -1;
    }

    page <<= (WRITE_SHT_AMT - 8);

    if (size) 
    {
        uint8_t op[] = {FLASH_OP_PAGE_WRITE, (uint8_t)(page >> 8), (uint8_t)(page | (offset >> 8)), (uint8_t)offset};
        sendDataToFlash(4, op, size, datap);
    }

    return 0;
}

int flashWrite(void *datap, uint32_t addr, uint16_t size)
{
    uint16_t offset = addr % BYTES_PER_PAGE;
    addr /= BYTES_PER_PAGE;
    if (offset)
    {
        // partial page, load the existing data first
        flashBufLoad(addr);
        uint16_t space = BYTES_PER_PAGE - offset;

        if (size > space) 
        {
            usbPrintf_P(PSTR("fw: 1 addr 0x%04lx %u bytes\n"), addr, size);
            flashBufWrite(datap, offset, space);
            size -= space;
            datap += space;
            flashBufStore(addr, true);
        }
        else 
        {
            usbPrintf_P(PSTR("fw: 2 addr 0x%04lx %u bytes\n"), addr, size);
            flashBufWrite(datap, offset, size);
            flashBufStore(addr, true);
            return 0;
        }

        addr++;
    }

    while (size > BYTES_PER_PAGE)
    {
        usbPrintf_P(PSTR("fw: 3 addr 0x%04lx %u bytes\n"), addr, size);
        flashWritePage(datap, addr++, 0, BYTES_PER_PAGE);
        datap += BYTES_PER_PAGE;
        size -= BYTES_PER_PAGE;
    }

    usbPrintf_P(PSTR("fw: 4 addr 0x%04lx %u bytes\n"), addr, size);
    flashWritePage(datap, addr, 0, size);

    return 0;
}
