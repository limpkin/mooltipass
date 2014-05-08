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

#define NODE_TEST_PARENT_NODE_AOK 0
#define NODE_TEST_PARENT_NODE_SECTOR_ERASE_ERROR 1
#define NODE_TEST_PARENT_NODE_MGMT_INIT_ERROR 2
#define NODE_TEST_PARENT_NODE_PROFILE_SET_STARTING_PARENT 3
#define NODE_TEST_PARENT_NODE_PROFILE_SET_FAV 4
#define NODE_TEST_PARENT_NODE_CREATE 5
#define NODE_TEST_PARENT_NODE_READ 6
#define NODE_TEST_PARENT_NODE_
#define NODE_TEST_PARENT_NODE_
#define NODE_TEST_PARENT_NODE_

RET_TYPE parentNodeTest(mgmtHandle *h, uint8_t *code)
{
    RET_TYPE ret = RETURN_NOK;
    uint8_t i = 0;
    pNode parent;
    pNode *parentPtr = &parent;    
    
    // format flash
    usbPrintf_P(PSTR("Erasing Sectors\n"));
    for(i = SECTOR_START; i < SECTOR_END; i++)
    {
        ret = sectorErase(i);
        if(ret != RETURN_OK)
        {
            *code = NODE_TEST_PARENT_NODE_SECTOR_ERASE_ERROR;
            return ret;
        }
    }
    
    // init handle as user 0
    usbPrintf_P(PSTR("Init Handle\n"));
    ret = initNodeManagementHandle(h, 0);
    if(ret != RETURN_OK)
    {
        *code = NODE_TEST_PARENT_NODE_MGMT_INIT_ERROR;
        return ret;
    }
   
    // setup (clear) user profile
    usbPrintf_P(PSTR("Init Profile\n"));
    ret = setStartingParent(h, NODE_ADDR_NULL);
    if(ret != RETURN_OK)
    {
        *code = NODE_TEST_PARENT_NODE_PROFILE_SET_STARTING_PARENT;
        return ret;
    }
    
    for(i = 0; i < USER_MAX_FAV; i++)
    {
        ret = setFav(h, i, NODE_ADDR_NULL, NODE_ADDR_NULL);
        if(ret != RETURN_OK)
        {
            *code = NODE_TEST_PARENT_NODE_PROFILE_SET_FAV;
            return ret;
        }
    }
    
    if(h->nextFreeParentNode == constructAddress(PAGE_PER_SECTOR, 0))
    {
        usbPrintf_P(PSTR("Scan Passed %u %u\n"), pageNumberFromAddress(h->nextFreeParentNode), nodeNumberFromAddress(h->nextFreeParentNode));
    }
    else
    {
        usbPrintf_P(PSTR("Scan Failed %u %u\n"), pageNumberFromAddress(h->nextFreeParentNode), nodeNumberFromAddress(h->nextFreeParentNode));
    }
    
    // create single parent node
    usbPrintf_P(PSTR("Creating Parent Node\n"));
    nodeTypeToFlags(&(parentPtr->flags), NODE_TYPE_PARENT);
    validBitToFlags(&(parentPtr->flags), NODE_VBIT_VALID);
    userIdToFlags(&(parentPtr->flags), 0);
    credentialTypeToFlags(&(parentPtr->flags), 0);
    
    parentPtr->nextChildAddress = NODE_ADDR_NULL;
    parentPtr->nextParentAddress = NODE_ADDR_NULL;
    parentPtr->prevParentAddress = NODE_ADDR_NULL;
    
    for(i = 0; i < NODE_PARENT_SIZE_OF_SERVICE; i++)
    {
        parentPtr->service[i] = '\0';
    }
    
    parentPtr->service[0] = 'c';
    
    ret = createParentNode(h, parentPtr);
    if(ret != RETURN_OK)
    {
        *code = NODE_TEST_PARENT_NODE_CREATE;
        return ret;
    }
    
    if(h->firstParentNode == constructAddress(PAGE_PER_SECTOR, 0))
    {
        usbPrintf_P(PSTR("Create Passed %u %u\n"), pageNumberFromAddress(h->firstParentNode), nodeNumberFromAddress(h->firstParentNode));
    }
    else
    {
        usbPrintf_P(PSTR("Create Failed %u %u\n"), pageNumberFromAddress(h->firstParentNode), nodeNumberFromAddress(h->firstParentNode));
    }
    
    ret = readParentNode(h, parentPtr, h->firstParentNode);
    if(ret != RETURN_OK)
    {
        *code = NODE_TEST_PARENT_NODE_READ;
        return ret;
    }
    
    if(h->nextFreeParentNode == constructAddress(PAGE_PER_SECTOR, 1))
    {
        usbPrintf_P(PSTR("Scan Passed %u %u\n"), pageNumberFromAddress(h->nextFreeParentNode), nodeNumberFromAddress(h->nextFreeParentNode));
    }
    else
    {
        usbPrintf_P(PSTR("Scan Failed %u %u\n"), pageNumberFromAddress(h->nextFreeParentNode), nodeNumberFromAddress(h->nextFreeParentNode));
    }
    
    parentPtr->service[0] = 'a';
    
    ret = createParentNode(h, parentPtr);
    if(ret != RETURN_OK)
    {
        *code = NODE_TEST_PARENT_NODE_CREATE;
        return ret;
    }
    
    if(h->firstParentNode == constructAddress(PAGE_PER_SECTOR, 1))
    {
        usbPrintf_P(PSTR("Create Passed %u %u\n"), pageNumberFromAddress(h->firstParentNode), nodeNumberFromAddress(h->firstParentNode));
    }
    else
    {
        usbPrintf_P(PSTR("Create Failed %u %u\n"), pageNumberFromAddress(h->firstParentNode), nodeNumberFromAddress(h->firstParentNode));
    }
    
    
    
    return ret;
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
    hp->flags = flags++;
    uint8_t ret_code = NODE_TEST_PARENT_NODE_AOK;
    
    RET_TYPE ret = RETURN_NOK;
       
    /****************************************** Node Flag Test **********************************************/
    //#define NODE_TEST_FLAGS
    #ifdef NODE_TEST_FLAGS
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
    #endif
    
    /****************************************** Address Test **********************************************/
    //#define NODE_TEST_ADDR
    #ifdef NODE_TEST_ADDR
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
    #endif
    
    /****************************************** User Profile Offset **********************************************/
    //#define NODE_TEST_USER_PROFILE_OFFSET
    #ifdef NODE_TEST_USER_PROFILE_OFFSET
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
    #endif
    
    /****************************************** Node MgMt Init Handle **********************************************/
    //#define NODE_TEST_MGMT_HANDLE
    #ifdef NODE_TEST_MGMT_HANDLE
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
    #endif
    
    /****************************************** User Profile Address **********************************************/
    //#define NODE_TEST_USER_PROFILE
    #ifdef NODE_TEST_USER_PROFILE
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
    #endif
    
    /****************************************** Node Parent Read **********************************************/
    //#define NODE_TEST_PARENT_READ
    #ifdef NODE_TEST_PARENT_READ
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
    #endif
    
    /****************************************** Node Parent Update **********************************************/
    //#define NODE_TEST_PARENT_UPDATE
    #ifdef NODE_TEST_PARENT_UPDATE
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
    #endif
    
    
    
    
    
    
    
    /****************************************** Parent Node Test **********************************************/
    displayInitForNodeTest();
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
    printf_P(PSTR("Parent Node Test"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
    usbPrintf_P(PSTR("Parent Node Test\n"));
    #endif
    
    
    // run test
    ret = parentNodeTest(hp, &ret_code);
    
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

   
   
    #ifdef FLASH_TEST_DEBUG_OUTPUT_OLED
        printf_P(PSTR("DONE"));
    #endif
    
    #ifdef FLASH_TEST_DEBUG_OUTPUT_USB
        usbPrintf_P(PSTR("DONE\n"));
    #endif
    
    return ret;
}