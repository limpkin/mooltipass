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
/*! \file   logic_fwflash_storage.c
 *  \brief  Logic for storing/getting fw data in the dedicated flash storage
 *  Copyright [2014] [Mathieu Stephan]
 */
#include <stdint.h>
#include "flash_mem.h"
#include "node_mgmt.h"
#include "defines.h"
#define TEXTBUFFERSIZE  30
// Buffers to store text
uint8_t textBuffer1[TEXTBUFFERSIZE];
uint8_t textBuffer2[TEXTBUFFERSIZE];
// Pointer to our current free buffer
uint8_t* curTextBufferPtr = textBuffer1;


/*!	\fn     getStoredFileAddr(uint16_t fileId, uint16_t* addr)
*	\brief	Get the flash address of a stored file
*   \param  fileId  File ID
*   \param  addr    Where to store the address
*   \return RETURN_OK or RETURN_NOK
*/
RET_TYPE getStoredFileAddr(uint16_t fileId, uint16_t* addr)
{
    uint16_t fileCount;

    flashRawRead((uint8_t*)&fileCount, GRAPHIC_ZONE_START, sizeof(fileCount));

    // Invalid file index or flash not formatted
    if ((fileId >= fileCount) || (fileCount == 0xFFFF))
    {
        return RETURN_NOK;
    }

    flashRawRead((uint8_t*)addr, GRAPHIC_ZONE_START + fileId * sizeof(uint16_t) + sizeof(uint16_t), sizeof(*addr));
    *addr += GRAPHIC_ZONE_START;
    
    return RETURN_OK;
}

/*!	\fn     readStoredStringToBuffer(uint8_t stringID, uint8_t* buffer)
*	\brief	Read a Flash stored string in a buffer and return the pointer to this buffer (2 buffers implemented)
*   \param  stringID    String ID
*   \return Pointer to the buffer
*/
char* readStoredStringToBuffer(uint8_t stringID)
{
    uint8_t* ret_val = curTextBufferPtr;
    uint16_t temp_addr;
    
    // Get address in flash
    if ((getStoredFileAddr((uint16_t)stringID, &temp_addr) == RETURN_OK) && (temp_addr != 0x0000))
    {
        // We can read more chars...
        flashRawRead(ret_val, temp_addr, TEXTBUFFERSIZE);
        
        // Switch buffers
        if (curTextBufferPtr == textBuffer2)
        {
            curTextBufferPtr = textBuffer1;
        }
        else
        {
            curTextBufferPtr = textBuffer2;
        }
    }
    
    return (char*)ret_val;
}