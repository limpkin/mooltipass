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

/*!  \file     flash_test.c
*    \brief    Mooltipass Flash IC Library Test Functions
*    Created:  31/3/2014
*    Author:   Michael Neiderhauser
*/

#include "timer_manager.h"
#include "mooltipass.h"
#include "flash_test.h"
#include "flash_mem.h"
#include "node_mgmt.h"
#include "defines.h"
#include "oledmp.h"
#include "usb.h"

#include <stdint.h>
#include <avr/io.h>
#include <string.h> // for memcpy

/*!  \fn       initBuffer(uint8_t* buffer, uint16_t bufferSize, uint8_t policy)
*    \brief    A helper function that populates a buffer
*    \param    buffer      The buffer to populate
*    \param    bufferSize  The size of the buffer
*    \param    policy      The population policy. (All Ones, All Zeros, Random, Increment)
*/
void initBuffer(uint8_t* buffer, uint16_t bufferSize, uint8_t policy)
{
    uint8_t ctr = 0;
    uint16_t i = 0;
    
    if(policy == FLASH_TEST_INIT_BUFFER_POLICY_ONE)
    {
        // all 1's
        for(i = 0; i < bufferSize; i++)
        {
            buffer[i] = 255;
        }
    }
    else if(policy == FLASH_TEST_INIT_BUFFER_POLICY_INC)
    {
        // increment
        for(i = 0; i < bufferSize; i++)
        {
            buffer[i] = ctr;
            // ctr will overflow if > 255
            ctr++;
        }
    }
    else if(policy == FLASH_TEST_INIT_BUFFER_POLICY_RND)
    {
        for(i = 0; i < bufferSize; i++)
        {
            //mooltipass_rand();
            buffer[i] = (uint8_t)rand();
        }
    }
    else
    {
        // all 0's
        for(i = 0; i < bufferSize; i++)
        {
            buffer[i] = 0;
        }
    }
} // End initBuffer

/*!  \fn       flashInitTest()
*    \brief    A Testing wrapper that tests flash init (reading manf id's)
*     THIS TEST MUST BE RUN -> Sets up SPI
*    \return   Test Status
*/
RET_TYPE flashInitTest()
{
    // simply call initFlash
    return initFlash();    
} // End flashInitTest

/*!  \fn       flashWriteReadTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
*    \brief    A Testing wrapper that tests page write, and page read.
*    \param    bufferIn   One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferOut  One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferSize Should always be BYTES_PER_PAGE
*    \return   Test Status
*/
RET_TYPE flashWriteReadTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
{
    uint16_t i = 0; // page number
    uint16_t j = 0; // byte in buffer

    // for each page in the flash
    for(i = 0; i < PAGE_COUNT; i++)
    {
        // get random buffer for bufferIn, copy to bufferOut
        initBuffer(bufferIn, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_RND);
        memcpy(bufferOut, bufferIn, bufferSize);
        
        // write bufferIn to flash (Entire page) -> bufferIn becomes corrupt
        writeDataToFlash(i, 0, bufferSize, bufferIn);

        // clear bufferIn
        initBuffer(bufferIn, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_ZERO);
        
        // read from flash to bufferIn
        readDataFromFlash(i, 0, bufferSize, bufferIn);
        
        // check buffer contents (compare bufferIn to the original copy)
        for(j = 0; j < bufferSize; j++)
        {
            // compare -> bufferIn == bufferOut
            if(bufferIn[j] != bufferOut[j])
            {
                return RETURN_NO_MATCH;
            }
        } // End for each byte in buffer
    } // End for each page
    return RETURN_OK;
} // End flashWriteReadTest

/*!  \fn       flashWriteReadOffsetTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
*    \brief    A Testing wrapper that tests page write, and page read.
*     Uses bufferSize / NODE_SIZE_PARENT for offset calculation
*    \param    bufferIn   One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferOut  One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferSize Should always be BYTES_PER_PAGE
*    \return   Test Status
*/
RET_TYPE flashWriteReadOffsetTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
{
    uint8_t offsetsPerPage = bufferSize / NODE_SIZE_PARENT;
    uint16_t offset = 0;
    uint16_t i = 0; // page number
    uint16_t j = 0; // offset counter
    uint16_t k = 0; // byte count in compare
    
    // for each page in the flash
    for(i = 0; i < PAGE_COUNT; i++)
    {
        // for each offsetBin in page (1M -> 4 bins -> 264/66)
        for(j = 0; j < offsetsPerPage; j++)
        {
            // get random buffer for bufferIn, copy to bufferOut
            initBuffer(bufferIn, NODE_SIZE_PARENT, FLASH_TEST_INIT_BUFFER_POLICY_RND);
            memcpy(bufferOut, bufferIn, NODE_SIZE_PARENT);
            
            // calculate offset
            offset = j * NODE_SIZE_PARENT;
            
            // write to page + offset.  buffer becomes corrupt 
            writeDataToFlash(i, offset, NODE_SIZE_PARENT, bufferIn);
            
            // clear bufferIn
            initBuffer(bufferIn, NODE_SIZE_PARENT, FLASH_TEST_INIT_BUFFER_POLICY_ZERO);
            
            // read from flash to bufferIn
            readDataFromFlash(i, offset, NODE_SIZE_PARENT, bufferIn);
            
            // compare buffers
            for(k = 0; k < NODE_SIZE_PARENT; k++)
            {
                // compare ->  bufferIn == bufferOut
                if(bufferIn[k] != bufferOut[k])
                {
                    return RETURN_NO_MATCH;
                }
            } // end compare loop
        } // end offset loop
    } // end page loop
    
    return RETURN_OK;
} // flashWriteReadOffsetTest

/*!  \fn       flashErasePageTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
*    \brief    A Testing wrapper that tests erasing all pages in flash
*    \param    bufferIn   One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferOut  One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferSize Should always be BYTES_PER_PAGE
*    \return   Test Status
*/
RET_TYPE flashErasePageTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
{
    uint16_t i = 0; // page number
    uint16_t j = 0; // byte in buffer
    
    // for each page in flash (populate flash)
    for(i = 0; i < PAGE_COUNT; i++)
    {
        // generate random buffer and copy
        initBuffer(bufferIn, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_RND);
        memcpy(bufferOut, bufferIn, bufferSize);
        
        // write input Buffer to flash -> buffer becomes corrupt
        writeDataToFlash(i, 0, bufferSize, bufferIn);

        // set input buffer to zero
        initBuffer(bufferIn, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_ZERO);
        
        // read from flash to bufferIn
        readDataFromFlash(i, 0, bufferSize, bufferIn);
        
        // check buffer contents (compare bufferIn to the original copy)
        for(j = 0; j < bufferSize; j++)
        {
            // compare -> bufferIn == bufferOut
            if(bufferIn[j] != bufferOut[j])
            {
                return RETURN_NO_MATCH;
            }            
        } // End for each byte in buffer

        // send page erase
        pageErase(i);
        
        // clear bufferIn (0's), set bufferOut (1's)
        initBuffer(bufferIn, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_ZERO);
        initBuffer(bufferOut, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_ONE);
        
        // read from flash to bufferIn
        readDataFromFlash(i, 0, bufferSize, bufferIn);
        
        // check buffer contents (compare bufferIn to the original copy)
        for(j = 0; j < bufferSize; j++)
        {
            // compare -> bufferIn == bufferOut
            if(bufferIn[j] != bufferOut[j])
            {
                return RETURN_NO_MATCH;
            }
        } // End for each byte in buffer
    } // End for each page in flash
    return RETURN_OK;        
} // End flashErasePageTest

/*!  \fn       flashEraseBlockTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
*    \brief    A Testing wrapper that tests erasing all blocks in flash
*    \param    bufferIn   One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferOut  One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferSize Should always be BYTES_PER_PAGE
*    \return   Test Status
*/
RET_TYPE flashEraseBlockTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
{
    RET_TYPE ret = RETURN_NOK;
    uint16_t i = 0; // page number
    uint16_t j = 0; // byte in buffer

    // assuming the flashWriteReadTest passes -> Populate entire flash chip
    ret = flashWriteReadTest(bufferIn, bufferOut, bufferSize);
    if(ret != RETURN_OK)
    {
        //return on error
        return ret;
    }
    
    // for each block issue block erase
    for(i = 0; i < BLOCK_COUNT; i++)
    {
        blockErase(i);
    }
    
    // init bufferOut with all ones (comparison buffer)
    initBuffer(bufferOut, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_ONE);
    
    // for each page in flash (verify erased)
    for(i = 0; i < PAGE_COUNT; i++)
    {
        // clear input buffer
        initBuffer(bufferIn, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_ZERO);
        
        // read from flash to bufferIn
        readDataFromFlash(i, 0, bufferSize, bufferIn);
        
        // check buffer contents (compare bufferIn to the original copy)
        for(j = 0; j < bufferSize; j++)
        {
            // compare -> bufferIn == bufferOut
            if(bufferIn[j] != bufferOut[j])
            {
                return RETURN_NO_MATCH;
            }
        } // End fore each byte in buffer
    } // End for each page
    return RETURN_OK;
} // End flashEraseBlockTest

/*!  \fn       flashEraseSectorXTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
*    \brief    A Testing wrapper that tests erasing SECTOR_START through SECTOR_END
*    \param    bufferIn   One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferOut  One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferSize Should always be BYTES_PER_PAGE
*    \return   Test Status
*/
RET_TYPE flashEraseSectorXTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
{
    uint16_t i = 0;
    uint16_t j = 0;
    uint16_t k = 0;
    
    uint16_t sectorPageOffset = 0;
    
    // for each sector
    for(i = SECTOR_START; i <= SECTOR_END; i++)
    {
        // calculate sector page offset
        sectorPageOffset = PAGE_PER_SECTOR * i; 
        
        // for each page in sector (populate sector)
        for(j = 0; j < PAGE_PER_SECTOR; j++)
        {
            // write random data
            initBuffer(bufferIn, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_RND);
            memcpy(bufferOut, bufferIn, bufferSize);
            
            writeDataToFlash(sectorPageOffset + j, 0, bufferSize, bufferIn);
            
            // re-init bufferIn with 0's
            initBuffer(bufferIn, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_ZERO);
            
            // read data
            readDataFromFlash(sectorPageOffset + j, 0, bufferSize, bufferIn);
            
            // check buffer contents (verify population)
            for(k = 0; k < bufferSize; k++)
            {
                // compare buffers (should match)
                if(bufferIn[k] != bufferOut[k])
                {
                    return RETURN_NO_MATCH;
                }
            } // end for each byte in page
        } // end for each page in sector
        
        // issue sector erase
        sectorErase(i);
        
        // init compare buffer (1's)
        initBuffer(bufferOut, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_ONE);
        
        // for each page in sector (check for erased data)
        for(j = 0; j < PAGE_PER_SECTOR; j++)
        {
            // read flash to bufferIn
             readDataFromFlash(sectorPageOffset + j, 0, bufferSize, bufferIn);
            
            // check buffer contents
            for(k = 0; k < bufferSize; k++)
            {
                // compare -> bufferIn == bufferOut
                if(bufferIn[k] != bufferOut[k])
                {
                    return RETURN_NO_MATCH;
                }
            } // end for each byte in page
        } // end for each page in sector    
    } // end for each sector
      
    return RETURN_OK;
}  // End flashEraseSectorXTest

/*!  \fn       flashEraseSectorXTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
*    \brief    A Testing wrapper that tests erasing FLASH_SECTOR_ZERO_A_CODE and FLASH_SECTOR_ZERO_B_CODE
*    \param    bufferIn   One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferOut  One of the two testing buffers -> Should be size of BYTES_PER_PAGE
*    \param    bufferSize Should always be BYTES_PER_PAGE
*    \return   Test Status
*/
RET_TYPE flashEraseSectorZeroTest(uint8_t* bufferIn, uint8_t* bufferOut, uint16_t bufferSize)
{
    uint16_t i = 0; // page number
    uint16_t j = 0; // byte in buffer
    
    // for each page in sector 0a and 0b
    for(i = 0; i < PAGE_PER_SECTOR; i++)
    {
        // generate buffers
        initBuffer(bufferIn, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_RND);
        memcpy(bufferOut, bufferIn, bufferSize);
        
        // write bufferIn to flash -> corrupts buffer
        writeDataToFlash(i, 0, bufferSize, bufferIn);
        
        // clear buffer in
        initBuffer(bufferIn, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_ZERO);
        
        // read flash to buffer
        readDataFromFlash(i, 0, bufferSize, bufferIn);
        
        //verify
        for(j = 0; j < bufferSize; j++)
        {
            // compare -> bufferIn == bufferOut
            if(bufferIn[j] != bufferOut[j])
            {
                return RETURN_NO_MATCH;
            }
        } // End for each byte in buffer
    } // end for each page in sector
    
    // issue erase sector 0a
    sectorZeroErase(FLASH_SECTOR_ZERO_A_CODE);
    
    //issue erase sector 0b
    sectorZeroErase(FLASH_SECTOR_ZERO_B_CODE);
    
    // set bufferOut (compare buffer)
    initBuffer(bufferOut, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_ONE);
    
    // for each page in sector (verify erase)
    for(i = 0; i < PAGE_PER_SECTOR; i++)
    {
        // clear bufferIn
        initBuffer(bufferIn, bufferSize, FLASH_TEST_INIT_BUFFER_POLICY_ZERO);   
        
        // read page
        readDataFromFlash(i, 0, bufferSize, bufferIn);
        
        // for each byte in buffer (compare)
        for(j = 0; j < bufferSize; j++)
        {
            // compare -> bufferIn == bufferOut
            if(bufferIn[j] != bufferOut[j])
            {
                return RETURN_NO_MATCH;
            }
        } // End for each byte in buffer
    } // End for each page in sector
    return RETURN_OK;
} // End flashEraseSectorZeroTest

/*!  \fn       displayInitForTest()
*    \brief    Init OLED SCREEN per test
*/
void displayInitForTest()
{
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        oledClear();
        oledSetXY(0,0);
        printf_P(PSTR("Flash Test"));
        oledSetXY(0,8);
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("\n----Flash Test----\n"));
    #endif
}

/*!  \fn       displayRWCode(RET_TYPE ret)
*    \brief    display Read/Write Return Code
*    \param    ret  return code
*/
void displayRWCode(RET_TYPE ret)
{
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        oledSetXY(0,24);
    #endif
    
    if(ret == RETURN_NO_MATCH)
    {
        #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
            printf_P(PSTR("NO MATCH"));
        #endif
        
        #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
            usbPrintf_P(PSTR("NO MATCH\n"));
        #endif
    }
    else if(ret == RETURN_READ_ERR)
    {
        #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
            printf_P(PSTR("READ ERROR"));
        #endif
        
        #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
            usbPrintf_P(PSTR("READ ERROR\n"));
        #endif
    }
    else if(ret == RETURN_WRITE_ERR)
    {
        #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
            printf_P(PSTR("WRITE ERROR"));
        #endif
        
        #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
            usbPrintf_P(PSTR("WRITE ERROR\n"));
        #endif
    }
    else
    {
        #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
            printf_P(PSTR("BAD PARAM / UNKNOWN"));
        #endif
        
        #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
            usbPrintf_P(PSTR("BAD PARAM / UNKNOWN\n"));
        #endif
    }
}

/*!  \fn       displayPassed()
*    \brief    Display PASSED Message (with delay)
*/
void displayPassed()
{
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        oledSetXY(0,16);
        printf_P(PSTR("PASSED"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("PASSED\n"));
    #endif
    
    timerBasedDelayMs(1000);
}

/*!  \fn       displayFailed()
*    \brief    Display FAILED Message
*/
void displayFailed()
{
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        oledSetXY(0,16);
        printf_P(PSTR("FAILED"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("FAILED\n"));
    #endif
}

/*!  \fn       flashTest()
*    \brief    Primary entry point for flash testing
*/
RET_TYPE flashTest()
{
    uint8_t inputBuffer[BYTES_PER_PAGE];
    uint8_t outputBuffer[BYTES_PER_PAGE];
    
    RET_TYPE ret = RETURN_NOK;
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("START Flash Test Suite %dM Chip\n"), (uint8_t)FLASH_CHIP);
    #endif
    
    /****************************************** Flash Init Test **********************************************/
    displayInitForTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        printf_P(PSTR("Flash Init Test"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("Flash Init Test\n"));
    #endif
    
    // run test
    ret = flashInitTest();
    
    // check result
    if(ret != RETURN_OK)
    {
        displayFailed();
        return ret;
    }
    else
    {
        displayPassed();
    }
    /************************************** Flash Write / Read Test ******************************************/
    #ifdef RUN_FLASH_TEST_WR
    displayInitForTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        printf_P(PSTR("Flash Write/Read Test"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("Flash Write/Read Test\n"));
    #endif
    
    // run test
    ret = flashWriteReadTest(inputBuffer, outputBuffer, BYTES_PER_PAGE);
    
    // check result
    if(ret != RETURN_OK)
    {
        displayFailed();
        displayRWCode(ret);
        return ret;
    }
    else
    {
        displayPassed();
    }
    #endif
    /*********************************** Flash Write / Read Offset Test **************************************/
    #ifdef RUN_FLASH_TEST_WRO
    displayInitForTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        printf_P(PSTR("Flash Write/Read + Offset Test"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("Flash Write/Read Test + Offset\n"));
    #endif
    
    // run test
    ret = flashWriteReadOffsetTest(inputBuffer, outputBuffer, BYTES_PER_PAGE);
    
    // check result
    if(ret != RETURN_OK)
    {
        displayFailed();
        displayRWCode(ret);
        return ret;
    }
    else
    {
        displayPassed();
    }
   
    #endif
    /****************************************** Flash Page Erase *********************************************/
    #ifdef RUN_FLASH_TEST_ERASE_PAGE
    displayInitForTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        printf_P(PSTR("Flash Erase Page"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("Flash Erase Page\n"));
    #endif
    
    // run test
    ret = flashErasePageTest(inputBuffer, outputBuffer, BYTES_PER_PAGE);
    
    // check result
    if(ret != RETURN_OK)
    {
        displayFailed();
        return ret;
    }
    else
    {
        displayPassed();
    }
    #endif
    /****************************************** Flash Block Erase ********************************************/
    #ifdef RUN_FLASH_TEST_ERASE_BLOCK
    displayInitForTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        printf_P(PSTR("Flash Erase Block"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("Flash Erase Block\n"));
    #endif
    
    // run test
    ret = flashEraseBlockTest(inputBuffer, outputBuffer, BYTES_PER_PAGE);
    
    // check result
    if(ret != RETURN_OK)
    {
        displayFailed();
        return ret;
    }
    else
    {
        displayPassed();
    }
    #endif
    /****************************************** Flash Sector Erase *******************************************/
    #ifdef RUN_FLASH_TEST_ERASE_SECTOR_X
    displayInitForTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        printf_P(PSTR("Flash Erase Sector X"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("Flash Erase Sector X\n"));
    #endif
    
    // run test
    ret = flashEraseBlockTest(inputBuffer, outputBuffer, BYTES_PER_PAGE);
    
    // check result
    if(ret != RETURN_OK)
    {
        displayFailed();
        return ret;
    }
    else
    {
        displayPassed();
    }
    #endif    
    /*************************************** Flash Sector Zero Erase *****************************************/
    #ifdef RUN_FLASH_TEST_ERASE_SECTOR_0
    displayInitForTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        printf_P(PSTR("Flash Erase Sector 0"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("Flash Erase Sector 0\n"));
    #endif
    
    // run test
    ret = flashEraseSectorZeroTest(inputBuffer, outputBuffer, BYTES_PER_PAGE);
    
    // check result
    if(ret != RETURN_OK)
    {
        displayFailed();
        return ret;
    }
    else
    {
        displayPassed();
    }
    #endif
    
    
    /*****************************************************************************************************/    
    /*************************************** Flash Suite Passeed *****************************************/
    /*****************************************************************************************************/ 
    displayInitForTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        printf_P(PSTR("Flash Test Suite"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("END Flash Test Suite %dM Chip\n"), (uint8_t)FLASH_CHIP);
    #endif
    
    displayPassed();
    return ret;
}
