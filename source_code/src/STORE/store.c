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
#include <avr/eeprom.h>
#include "eeprom_addresses.h"
#include "usb.h"
#include "flash_mem.h"
#include "flash.h"
#include "store.h"

typedef struct
{
    uint32_t available;     // 33792 for 1Mbit
    uint32_t nextFreeAddr;      // next free space in flash (byte address)
    struct {
        uint32_t start;         // Start of slot in flash (byte address)
        uint16_t writePos;      // current write position (byte address)
        uint16_t size;          // size of slot in bytes (0 = available)
    } slot[8];  // bitmap slots (page of bitmap start)
} bitmapStore_t;

#define NUM_ENTRIES(array)  (sizeof(array)/sizeof(array[0]))

#define STORE_START_PAGE FLASH_PAGE_MAPPING_GFX_START
#define STORE_END_PAGE   (FLASH_PAGE_MAPPING_GFX_END-1)
#define STORE_NUM_PAGES  (STORE_END_PAGE - STORE_START_PAGE)


/**
 * Initialize the slot storage, format the flash.
 */
void storeInit()
{
    // set empty EEPROM management
    bitmapStore_t *bmStore = (bitmapStore_t *)EEP_STORE_MANAGEMENT;
    for (uint8_t ind=0; ind<NUM_ENTRIES(bmStore->slot); ind++)
    {
        eeprom_write_dword(&bmStore->slot[ind].start, 0);
        eeprom_write_word(&bmStore->slot[ind].writePos, 0);
        eeprom_write_word(&bmStore->slot[ind].size, 0);
    }
    eeprom_write_dword(&bmStore->available, ((uint32_t)STORE_NUM_PAGES)*BYTES_PER_PAGE);
    eeprom_write_dword(&bmStore->nextFreeAddr, STORE_START_PAGE*BYTES_PER_PAGE);

    // erase flash storage
    for (uint16_t page=STORE_START_PAGE; page < STORE_END_PAGE; page++)
    {
        pageErase(page);
    }
}

uint8_t storeFreeSlot(uint8_t slotId)
{
    // not implemented. Call storeInit() instead to erase all.
    return 0;
}

/**
 * Allocate a new slot to store data in.
 * @param size the size of the slot.  This is how much data the slot can store.
 * @retval 0 failed to allocate slot (not enough space left in flash, or no slots left)
 * @retval 1-8 the identifier for the new slot.  Use this to write to it and read from it.
 * @Note Only 8 slots are available, and a shared pool 126 pages are available for storage.
 */
uint8_t storeAllocateSlot(uint16_t size)
{
    bitmapStore_t *bmStore = (bitmapStore_t *)EEP_STORE_MANAGEMENT;
    uint32_t available = eeprom_read_dword(&bmStore->available);

    if (size > available) {
        return 0;
    }
    for (uint8_t ind=0; ind<NUM_ENTRIES(bmStore->slot); ind++) 
    {
        if (eeprom_read_word(&bmStore->slot[ind].size) == 0)
        {
            uint32_t start = eeprom_read_dword(&bmStore->nextFreeAddr);
            eeprom_write_dword(&bmStore->slot[ind].start, start);      // allocated
            eeprom_write_word(&bmStore->slot[ind].writePos, 0);
            eeprom_write_word(&bmStore->slot[ind].size, size);
            eeprom_write_dword(&bmStore->nextFreeAddr, start+size);
            eeprom_write_dword(&bmStore->available, available-size);
            usbPrintf_P(PSTR("store: Allocated slot %u size %u start %lu\n"), ind+1, size, start);
            return ind+1; // the allocated slot
        }
    }

    // no slots left
    return 0;
}

/**
 * Write a chunk of data to a storage slot.  The data is appended
 * to any data already in the slot.
 * @param slotId the slot to write to (1 to 8)
 * @param size the amount of data to write. 
 * @param datap buffer read the data from.
 * @retval 0 failed to write to slot (invalid slot or not enough room).
 * @retval 1 success.
 * @Note the size of the slot is defined when it is allocated.
 */
uint8_t storeWriteSlot(uint8_t slotId, uint16_t size, void *datap)
{
    bitmapStore_t *bmStore = (bitmapStore_t *)EEP_STORE_MANAGEMENT;

    slotId -= 1;

    if (slotId >= NUM_ENTRIES(bmStore->slot))
    {
        usbPrintf_P(PSTR("store: slot %u invalid\n"), slotId+1);
        return 0; // invalid slot
    }

    uint16_t slotSize = eeprom_read_word(&bmStore->slot[slotId].size);
    uint16_t writePos = eeprom_read_word(&bmStore->slot[slotId].writePos);
    uint32_t addr = eeprom_read_dword(&bmStore->slot[slotId].start) + writePos;
    usbPrintf_P(PSTR("store: slot %u size %u pos %u addr %lu\n"), slotId+1, slotSize, writePos, addr);
    
    if (size > (slotSize - writePos))
    {
        usbPrintf_P(PSTR("store: slot %u not enough space (%u) for %u bytes\n"), slotId+1, slotSize, size);
        return 0;   // not enough space
    }
    eeprom_write_word(&bmStore->slot[slotId].writePos, writePos+size);

    usbPrintf_P(PSTR("store: slot %u writing %u bytes at %u\n"), slotId+1, size, writePos);

    if (flashWrite(datap, addr, size) < 0)
    {
        return 0;
    }

    return 1;
}


/**
 * Read a chunk of data from a storage slot.
 * @param slotId the slot to read from (1 to 8)
 * @param offset the offset to read the data from in the slot
 * @param size the amount of data to read. 
 * @param datap buffer to store the data in.
 * @returns the number of bytes read into the buffer.  May be less
 * than size if there is less than size bytes available. May be 0
 * if there is no data to read from the offset or the slot is empy.
 */
uint16_t storeReadSlot(uint8_t slotId, uint16_t offset, uint16_t size, void *datap)
{
    bitmapStore_t *bmStore = (bitmapStore_t *)EEP_STORE_MANAGEMENT;

    slotId -= 1;

    if (slotId >= sizeof(bmStore->slot)/sizeof(bmStore->slot[0])) 
    {
        return 0; // invalid slot
    }

    //uint16_t slotSize = eeprom_read_word(&bmStore->slot[slotId].size);
    uint16_t available = eeprom_read_word(&bmStore->slot[slotId].writePos);

    if (offset > available)
    {
        return 0;
    }

    if ((offset + size) > available)
    {
        // less than size data available, just return all of it
        size = available - offset;
    }

    uint32_t addr = eeprom_read_dword(&bmStore->slot[slotId].start) + offset;

    
    if (flashRead(datap, addr, size) < 0)
    {
        return 0;
    }

    return size;
}
