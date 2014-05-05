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
*    \brief    Mooltipass Node Test Functions
*    Created:  2/5/2014
*    Author:   Michael Neiderhauser
*/

#include "node_test.h"

#include "../mooltipass.h"
#include "../defines.h"

#include "oledmp.h"
#include "usb_serial_hid.h"
#include "node_mgmt.h"
#include "flash_mem.h"


#include <stdint.h>
#include <avr/io.h>
#include <string.h> // for memcpy
#include <util/delay.h> // for delays

/*!  \fn       displayInitForTest()
*    \brief    Init OLED SCREEN per test
*/
void displayInitForNodeTest()
{
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
    oledClear();
    oledSetXY(0,0);
    printf_P(PSTR("Node Test"));
    oledSetXY(0,8);
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
    usbPrintf_P(PSTR("\n----Node Test----\n"));
    #endif
}

/*!  \fn       displayPassedNodeTest()
*    \brief    Display PASSED Message (with delay)
*/
void displayPassedNodeTest()
{
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
    oledSetXY(0,16);
    printf_P(PSTR("PASSED"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
    usbPrintf_P(PSTR("PASSED\n"));
    #endif
    
    _delay_ms(1000);
}

/*!  \fn       displayFailedNodeTest()
*    \brief    Display FAILED Message
*/
void displayFailedNodeTest()
{
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
    oledSetXY(0,16);
    printf_P(PSTR("FAILED"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
    usbPrintf_P(PSTR("FAILED\n"));
    #endif
}

/*!  \fn       nodeFlagFunctionTest()
*    \brief    Test flag modifier tests.  Manually set flags and verify manual value with function call returns
*/
RET_TYPE nodeFlagFunctionTest()
{
    //usbPrintf_P(PSTR("%u  -  %u  -  %u\n"),flagsExp, flagsOut, flagsIn);
    uint16_t flagsExp = 0;
    uint16_t flagsIn = 0;
    uint16_t flagsOut = 0;
    
    /* Node Type Test */
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("-Node Type (NT) Test\n"));
    #endif
    
    flagsExp = NODE_TYPE_PARENT << 14;
    nodeTypeToFlags(&flagsOut, NODE_TYPE_PARENT);
    flagsIn = nodeTypeFromFlags(flagsOut);
    if(flagsExp != flagsOut || flagsIn != NODE_TYPE_PARENT)
    {
        #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
            usbPrintf_P(PSTR("--Fail NT Parent\n"));
        #endif
        return RETURN_NOK;
    }
    
    flagsExp = NODE_TYPE_CHILD << 14;
    nodeTypeToFlags(&flagsOut, NODE_TYPE_CHILD);
    flagsIn = nodeTypeFromFlags(flagsOut);
    if(flagsExp != flagsOut || flagsIn != NODE_TYPE_CHILD)
    {
        #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
            usbPrintf_P(PSTR("--Fail NT Child\n"));
        #endif
        return RETURN_NOK;
    }
    
    flagsExp = NODE_TYPE_CHILD_DATA << 14;
    nodeTypeToFlags(&flagsOut, NODE_TYPE_CHILD_DATA);
    flagsIn = nodeTypeFromFlags(flagsOut);
    if(flagsExp != flagsOut || flagsIn != NODE_TYPE_CHILD_DATA)
    {
        #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
            usbPrintf_P(PSTR("--Fail NT Child Data\n"));
        #endif
        return RETURN_NOK;
    }
    
    flagsExp = NODE_TYPE_DATA << 14;
    nodeTypeToFlags(&flagsOut, NODE_TYPE_DATA);
    flagsIn = nodeTypeFromFlags(flagsOut);
    if(flagsExp != flagsOut || flagsIn != NODE_TYPE_DATA)
    {
        #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
            usbPrintf_P(PSTR("--Fail NT Data\n"));
        #endif
        return RETURN_NOK;
    }
    
    /* Valid Bit Test */
    
    flagsIn = 0;
    flagsOut = 0;
    flagsExp = 0;
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("-Valid Bit (VB) Test\n"));
    #endif
    
    flagsExp = NODE_VBIT_VALID << 13;
    validBitToFlags(&flagsOut, NODE_VBIT_VALID);
    flagsIn = validBitFromFlags(flagsOut);
    if(flagsExp != flagsOut || flagsIn != NODE_VBIT_VALID)
    {
        #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
            usbPrintf_P(PSTR("--Fail VB Valid\n"));
        #endif
        return RETURN_NOK;
    }
    
    flagsExp = NODE_VBIT_INVALID << 13;
    validBitToFlags(&flagsOut, NODE_VBIT_INVALID);
    flagsIn = validBitFromFlags(flagsOut);
    if(flagsExp != flagsOut || flagsIn != NODE_VBIT_INVALID)
    {
        #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
            usbPrintf_P(PSTR("--Fail VB Invalid\n"));
        #endif
        return RETURN_NOK;
    }

    /* User ID Test */
    
    flagsIn = 0;
    flagsOut = 0;
    flagsExp = 0;
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("-User ID (UID) Test\n"));
    #endif
    
    for(uint8_t uid = 0; uid < NODE_MAX_UID; uid++)
    {
        //usbPrintf_P(PSTR("UID: %u\n"), uid);
        flagsExp = uid << 4;
        userIdToFlags(&flagsOut, uid);
        flagsIn = userIdFromFlags(flagsOut);
        
        if(flagsExp != flagsOut || flagsIn != uid)
        {
            #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
                usbPrintf_P(PSTR("--Fail UID: %u\n"), uid);
            #endif
            return RETURN_NOK;
        }        
    }
    
    /* Credential Type Test */
    
    flagsIn = 0;
    flagsOut = 0;
    flagsExp = 0;
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("-Cred Type (CID) Test\n"));
    #endif
    
    for(uint8_t cid = 0; cid < NODE_MAX_CRED_TYPE; cid++)
    {
        //usbPrintf_P(PSTR("CRED TYPE: %u\n"), cid);
        flagsExp = cid;
        credentialTypeToFlags(&flagsOut, cid);
        flagsIn = credentialTypeFromFlags(flagsOut);
        
        if(flagsExp != flagsOut || flagsIn != cid)
        {
            #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
                usbPrintf_P(PSTR("--Fail CID: %u\n"), cid);
            #endif
            return RETURN_NOK;
        }
    }
    
    /* Data Node Sequence Number Test */
    
    flagsIn = 0;
    flagsOut = 0;
    flagsExp = 0;
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("-SEQ ID (SID) Test\n"));
    #endif
    
    for(uint16_t sid = 0; sid < NODE_MAX_DATA; sid++)
    {
        //usbPrintf_P(PSTR("SEQ ID: %u\n"), sid);
        flagsExp = sid;
        dataNodeSequenceNumberToFlags(&flagsOut, (uint8_t)sid);
        flagsIn = dataNodeSequenceNumberFromFlags(flagsOut);
        
        if(flagsExp != flagsOut || flagsIn != sid)
        {
            #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
                usbPrintf_P(PSTR("--Fail SID: %u\n"), sid);
            #endif
            return RETURN_NOK;
        }
    }
    
    return RETURN_OK;
}

/*!  \fn       nodeAddressTest()
*    \brief    Test address modifier tests.
*/
RET_TYPE nodeAddressTest()
{
    uint16_t addr = 0;
    uint16_t pageNum = 0;
    uint8_t nodeNum = 0;
    for(uint8_t nodeId = 0; nodeId < 8; nodeId++) // 8 2^3
    {
        for(uint16_t pageId = 0; pageId < 8192; pageId++) // 8192 2^13
        {
            addr = constructAddress(pageId, nodeId);
            pageNum = pageNumberFromAddress(addr);
            nodeNum = nodeNumberFromAddress(addr);
            
            if(nodeNum != nodeId || pageNum != pageId)
            {
                #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
                    usbPrintf_P(PSTR("--Fail Address Mods: %u %u\n"), nodeId, pageId);
                #endif
                return RETURN_NOK;
            }
        }
    }
    
    return RETURN_OK;
}

/*!  \fn       userProfileOffsetTest()
*    \brief    Test user profile offsets (hardcoded results vector).
*/
RET_TYPE userProfileOffsetTest()
{
    uint8_t uid = 0;
    uint16_t pageNumber = 0;
    uint16_t pageOffset = 0;
    
    #if BYTES_PER_PAGE==264
        uint16_t pages[] = {0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3};
        uint16_t offsets[] = {0,66,132,198,0,66,132,198,0,66,132,198,0,66,132,198};
    #else
        uint16_t pages[] = {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1};
        uint16_t offsets[] = {0,66,132,198,264,330,396,462,0,66,132,198,264,330,396,462};
    #endif
    
    for(uid = 0; uid < NODE_MAX_UID; uid++)
    {
        //usbPrintf_P(PSTR("Expected UID: %u  Page: %u  Offset: %u\n"), uid, pages[uid], offsets[uid]);
        userProfileStartingOffset(uid, &pageNumber, &pageOffset);
        //usbPrintf_P(PSTR("Actual   UID: %u  Page: %u  Offset: %u\n"), uid, pageNumber, pageOffset);
        if((pageNumber != pages[uid]) || (pageOffset != offsets[uid]))
        {
            #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
                usbPrintf_P(PSTR("--Fail UID: %u\n"), uid);
            #endif
            return RETURN_NOK;
        }
    }

    return RETURN_OK;
}

/*!  \fn       userProfileAddressTest()
*    \brief    Test user profile writing and reading starting parent address
*/
RET_TYPE userProfileAddressTest(mgmtHandle *h)
{
    uint8_t uid =  0;
    uint16_t pageNumber = 0;
    uint16_t pageOffset = 0;
    uint16_t address = constructAddress(PAGE_PER_SECTOR, 0);
    uint16_t addressBackup = address;
    
    RET_TYPE ret = RETURN_NOK;
    
    for(uid=0; uid < NODE_MAX_UID; uid++)
    {
        address = addressBackup;
        // init already tested.  put uid in handle
        initNodeManagementHandle(h, uid);
        // calculate profile offsets
        userProfileStartingOffset(uid, &pageNumber, &pageOffset);
        // set starting parent address in handle and memory
        ret = setStartingParent(h, address);
        if(ret != RETURN_OK || address != addressBackup)
        {
            return RETURN_NOK;
        }
        
        ret = readStartingParent(h, &address);
        if(ret != RETURN_OK || address != addressBackup)
        {
            return RETURN_NOK;
        }
    }
    return RETURN_OK;
}    








    
RET_TYPE nodeTest()
{
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        while (!(usb_serial_get_control() & USB_SERIAL_DTR)); /* wait for terminal to connect */
        usbPrintf_P(PSTR("START Node Test Suite %dM Chip\n"), (uint8_t)FLASH_CHIP);
    #endif
    
    mgmtHandle h;
    mgmtHandle *hp = &h;
    uint16_t flags = 0;
    
    RET_TYPE ret = RETURN_NOK;
       
    /****************************************** Node Flag Test **********************************************/
    displayInitForNodeTest();
   
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        printf_P(PSTR("Node Flag Test"));
    #endif
   
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("Node Flag Test\n"));
    #endif
   
    // run test
    ret = nodeFlagFunctionTest();
   
    // check result
    if(ret != RETURN_OK)
    {
        displayFailedNodeTest();
        return ret;
    }
    else
    {
        displayPassedNodeTest();
    }
    
    /****************************************** Address Test **********************************************/
    displayInitForNodeTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        printf_P(PSTR("Node Address Test"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("Node Address Test\n"));
    #endif
    
    // run test
    ret = nodeAddressTest();
    
    // check result
    if(ret != RETURN_OK)
    {
        displayFailedNodeTest();
        return ret;
    }
    else
    {
        displayPassedNodeTest();
    }
    
    /****************************************** User Profile Offset **********************************************/
    displayInitForNodeTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
    printf_P(PSTR("User Profile Offset Test"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
    usbPrintf_P(PSTR("User Profile Offset Test\n"));
    #endif
    
    // run test
    ret = userProfileOffsetTest();
    
    // check result
    if(ret != RETURN_OK)
    {
        displayFailedNodeTest();
        return ret;
    }
    else
    {
        displayPassedNodeTest();
    }
    
    /****************************************** Node MgMt Init Handle **********************************************/
    displayInitForNodeTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
    printf_P(PSTR("Node MgMt Handle Init Test"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
    usbPrintf_P(PSTR("Node MgMt Handle Init Test\n"));
    #endif
    
    
    // set next free node to next immediate address
    nodeTypeToFlags(&flags, NODE_TYPE_PARENT);
    validBitToFlags(&flags, NODE_VBIT_VALID);
    credentialTypeToFlags(&flags,0);
    userIdToFlags(&flags, 0);
    
    // write / read sector 1, page 0 (of sector 1), offset node 0, two bytes and flags
    ret = writeDataToFlash(PAGE_PER_SECTOR, 0, 2, &flags);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    // restore flags
    ret = readDataFromFlash(PAGE_PER_SECTOR, 0, 2, &flags);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    
    // set next free node to next immediate address
    nodeTypeToFlags(&flags, NODE_TYPE_PARENT);
    validBitToFlags(&flags, NODE_VBIT_INVALID);
    credentialTypeToFlags(&flags,0);
    userIdToFlags(&flags, 0);
    
    // write / read sector 1, page 0 (of sector 1), offset node 1, two bytes and flags
    ret = writeDataToFlash(PAGE_PER_SECTOR, NODE_SIZE_PARENT, 2, &flags);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    // restore flags
    ret = readDataFromFlash(PAGE_PER_SECTOR, NODE_SIZE_PARENT, 2, &flags);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    
    
    ret = initNodeManagementHandle(hp, 0);
    if(ret != RETURN_OK)
    {
        displayFailedNodeTest();
        return ret;
    }
    else
    {
        usbPrintf_P(PSTR("%u, %u\n"), h.nextFreeParentNode, constructAddress(PAGE_PER_SECTOR, 1));
        if(h.nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 1))
        {
            displayFailedNodeTest();
            return RETURN_NOK;
        }
        displayPassedNodeTest();
    }
    
    /****************************************** User Profile Address **********************************************/
    displayInitForNodeTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
    printf_P(PSTR("User Profile Address Test"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
    usbPrintf_P(PSTR("User Profile Address Test\n"));
    #endif
    
    // run test
    ret = userProfileAddressTest(hp);
    
    // check result
    if(ret != RETURN_OK)
    {
        displayFailedNodeTest();
        return ret;
    }
    else
    {
        displayPassedNodeTest();
    }
    
    /****************************************** Node Parent Read **********************************************/
    displayInitForNodeTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
    printf_P(PSTR("Read Parent Node Test"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
    usbPrintf_P(PSTR("Read Parent Node Test\n"));
    #endif
    
    pNode parentNode;
    pNode *parentNodePtr = &parentNode;
    
    // write parent node
    // construct flags
    nodeTypeToFlags(&flags, NODE_TYPE_PARENT);
    validBitToFlags(&flags, NODE_VBIT_VALID);
    credentialTypeToFlags(&flags,0);
    userIdToFlags(&flags, h.currentUserId);
    
    // set data to parent node
    parentNode.flags = flags;
    parentNode.nextChildAddress = constructAddress(PAGE_PER_SECTOR+1, 0);
    parentNode.nextParentAddress = constructAddress(PAGE_PER_SECTOR, 1);
    parentNode.prevParentAddress = NODE_ADDR_NULL;
    
    for(uint8_t i = 0; i < NODE_PARENT_SIZE_OF_SERVICE; i++)
    {
        parentNode.service[i] = i;
    }
    
    // write
    ret = writeDataToFlash(PAGE_PER_SECTOR, 0, NODE_SIZE_PARENT, parentNodePtr);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    // read
    ret = readParentNode(hp, parentNodePtr, constructAddress(PAGE_PER_SECTOR, 0));
    // check result
    if(ret != RETURN_OK)
    {
        displayFailedNodeTest();
        return ret;
    }
    else
    {
        if(parentNode.flags == flags &&
        parentNode.nextChildAddress == constructAddress(PAGE_PER_SECTOR+1, 0) &&
        parentNode.nextParentAddress == constructAddress(PAGE_PER_SECTOR, 1) &&
        parentNode.prevParentAddress == NODE_ADDR_NULL)
        {
            for(uint8_t i = 0; i < NODE_PARENT_SIZE_OF_SERVICE; i++)
            {
                if(parentNode.service[i] != i)
                {
                    displayFailedNodeTest();
                    return RETURN_NOK;
                }
            }
        }
        else
        {
            displayFailedNodeTest();
            return RETURN_NOK;
        }
        
        displayPassedNodeTest();
    }
    
    /****************************************** Node Parent Update **********************************************/
    displayInitForNodeTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
    printf_P(PSTR("Update Parent Node Test"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
    usbPrintf_P(PSTR("Update Parent Node Test\n"));
    #endif
    
    // write parent node
    // construct flags
    nodeTypeToFlags(&flags, NODE_TYPE_PARENT);
    validBitToFlags(&flags, NODE_VBIT_VALID);
    credentialTypeToFlags(&flags,0);
    userIdToFlags(&flags, h.currentUserId);
    
    // set data to parent node
    parentNode.flags = flags;
    parentNode.nextChildAddress = constructAddress(PAGE_PER_SECTOR+1, 0);
    parentNode.nextParentAddress = constructAddress(PAGE_PER_SECTOR, 1);
    parentNode.prevParentAddress = NODE_ADDR_NULL;
    
    for(uint8_t i = 0; i < NODE_PARENT_SIZE_OF_SERVICE; i++)
    {
        parentNode.service[i] = i;
    }
    
    // write
    ret = writeDataToFlash(PAGE_PER_SECTOR, 0, NODE_SIZE_PARENT, parentNodePtr);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    // modify node
    parentNode.service[0] = 255;
    
    // read
    ret = updateParentNode(hp, parentNodePtr, constructAddress(PAGE_PER_SECTOR, 0));
    // check result
    if(ret != RETURN_OK)
    {
        displayFailedNodeTest();
        return ret;
    }
    else
    {        
        displayPassedNodeTest();
    }
    
    

   
    return ret;
}