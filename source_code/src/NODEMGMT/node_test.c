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
    oledSetXY(0, 0);
    printf_P(PSTR("Node Test"));
    oledSetXY(0, 8);
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
    oledSetXY(0, 16);
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
    oledSetXY(0, 16);
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
*    \brief    Test user profile offsets (static results vector).
*/
RET_TYPE userProfileOffsetTest()
{
    uint8_t uid = 0;
    uint16_t pageNumber = 0;
    uint16_t pageOffset = 0;
    
    #if BYTES_PER_PAGE==264
        uint16_t pages[] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
        uint16_t offsets[] = {0, 66, 132, 198, 0, 66, 132, 198, 0, 66, 132, 198, 0, 66, 132, 198};
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

/*!  \fn       printParentNode(pNode *p)
*    \brief    A function to print the contents of a parent node
*    \param    p  The parent node to print
*/
void printParentNode(pNode *p)
{
    uint8_t i = 0;
    
    usbPrintf_P(PSTR("Service: "));
    for(i = 0; i < NODE_PARENT_SIZE_OF_SERVICE; i++)
    {
        if(p->service[i] != '\0')
        {
            usbPrintf_P(PSTR("%c"), (char)p->service[i]);
        }
    }
    usbPrintf_P(PSTR("\nPrev Parent (%u, %u)\n"), pageNumberFromAddress(p->prevParentAddress), nodeNumberFromAddress(p->prevParentAddress));
    usbPrintf_P(PSTR("Next Parent (%u, %u)\n\n"), pageNumberFromAddress(p->nextParentAddress), nodeNumberFromAddress(p->nextParentAddress));
}

/*!  \fn       parentNodeTest(mgmtHandle *h, uint8_t *code)
*    \brief    A function to test all functionality of parent nodes
*    \param    h  The user allocated node management handle
*    \param    code  The status return code
*    \return   Status
*/
RET_TYPE parentNodeTest(mgmtHandle *h, uint8_t *code)
{
    RET_TYPE ret = RETURN_NOK;
    uint8_t i = 0;
    pNode parent;
    pNode *parentPtr = &parent; 
    
    uint16_t oldHandleNextFreeParenNode;
    uint16_t oldHandleFirstParentNode;
    
    // format flash
    usbPrintf_P(PSTR("Erasing Sectors\n"));
    for(i = SECTOR_START; i < SECTOR_END; i++)
    {
        ret = sectorErase(i);
        if(ret != RETURN_OK)
        {
            *code = PARENT_NODE_TEST_ERASE_ALL_SECTORS_ERROR;
            return ret;
        }
    }
    
    // init handle as user 0
    usbPrintf_P(PSTR("Init Handle\n"));
    ret = initNodeManagementHandle(h, 0);
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_INIT_HANDLE_FUNCTION_FAIL;
        return ret;
    }
    
    // setup (clear) user profile
    usbPrintf_P(PSTR("Setup User Profile\n"));
    usbPrintf_P(PSTR("Init Profile\n"));
    ret = setStartingParent(h, NODE_ADDR_NULL);
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_USER_PROFILE_SET_START_NULL_ERROR;
        return ret;
    }
    
    usbPrintf_P(PSTR("-Clearing User Favs\n"));
    for(i = 0; i < USER_MAX_FAV; i++)
    {
        ret = setFav(h, i, NODE_ADDR_NULL, NODE_ADDR_NULL);
        if(ret != RETURN_OK)
        {
            *code = PARENT_NODE_TEST_USER_PROFILE_CLEAR_FAV_ERROR;
            return ret;
        }
    }
    
    usbPrintf_P(PSTR("Verify Handle Profile\n"));
    // Verify the following:
    // h->currentUserId = 0;
    // h->firstParentNode = NODE_ADDR_NULL;
    // h->nextFreeParentNode = (PAGE_PER_SECTOR, 0);
    
    if(h->currentUserId != 0)
    {
        *code = PARENT_NODE_TEST_HANDLE_UID_ERROR;
        return RETURN_NOK;
    }
    
    if(h->firstParentNode != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_HANDLE_FIRST_PARENT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_HANDLE_FREE_PARENT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    /***************************************************** Handle and Profile Setup Complete *****************************************************/
    // Construct Base Parent Node
    usbPrintf_P(PSTR("Constructing the base Parent Node\n"));
    // setup flags (type-> parent, vb-> valid, uid-> 0, cid-> 0)
    nodeTypeToFlags(&(parentPtr->flags), NODE_TYPE_PARENT);
    validBitToFlags(&(parentPtr->flags), NODE_VBIT_VALID);
    userIdToFlags(&(parentPtr->flags), 0);
    credentialTypeToFlags(&(parentPtr->flags), 0);
    //set node fields to null addr
    parentPtr->nextChildAddress = NODE_ADDR_NULL;
    parentPtr->nextParentAddress = NODE_ADDR_NULL;
    parentPtr->prevParentAddress = NODE_ADDR_NULL;
    // clear service
    for(i = 0; i < NODE_PARENT_SIZE_OF_SERVICE; i++)
    {
        parentPtr->service[i] = '\0';
    }
    
    /*
    Test Plan (Format of the 1M Flash Chip):
    ---STEP 1---
      Assuming nextFreeParentNode -> (128, 0) TESTED ABOVE
      Create Node('c': Addr->(128,0))
      Check Node('c'):
        service[0] = 'c';
        nextParentAddress = NODE_ADDR_NULL;
        prevParentAddress = NODE_ADDR_NULL;
      Check Handle:
        firstParenNode -> (128,0) (expecting change)
        nextFreeParentNode -> (128,1) (expecting change)
    ---STEP 2---
      Assuming nextFreeParentNode -> (128, 1) TESTED IN STEP 1
      Create Node('m': Addr->(128,1))
      Check Node('m'):
        service[0] = 'm';
        nextParentAddress = NODE_ADDR_NULL; // Should be last Node
        prevParentAddress = (128,0); // Should point to Node 'c'
      Read Node('c')
      Check Node('c'):
        service[0] = 'c';
        nextParentAddress = (128,1); // Node 'm'
        prevParentAddress = NODE_ADDR_NULL; // Still first node
      Check Handle:
        firstParenNode -> (128,0) (expecting no change)
        nextFreeParentNode -> (128,2) (expecting change)
    ---STEP 3---
      Assuming nextFreeParentNode -> (128, 2) TESTED IN STEP 2
      Create Node('k': Addr->(128,2))
      Check Node('k'): (128,2)
        service[0] = 'k';
        nextParentAddress = (128,1); // Should point to Node 'm'
        prevParentAddress = (128,0); // Should point to Node 'c'
      Read Node('c')
      Check Node('c'): (128,0)
        service[0] = 'c';
        nextParentAddress = (128,2); // Should point to Node 'k'
        prevParentAddress = NODE_ADDR_NULL; // Still first node
      Read Node('m')
      Check Node('m'): (128,1)
        service[0] = 'm';
        nextParentAddress = NODE_ADDR_NULL; // Should point to Node NULL
        prevParentAddress = (128,2); // Should point to Node 'k'
      Check Handle:
        firstParenNode -> (128,0) (expecting no change)
        nextFreeParentNode -> (128,3) (expecting change)
    ---STEP 4---
      Assuming nextFreeParentNode -> (128, 3) TESTED IN STEP 3
      Create Node('a': Addr->(128,3))
      Check Node('a'): (128,3)
        service[0] = 'a';
        nextParentAddress = (128,0); // Should point to Node 'c'
        prevParentAddress = NODE_ADDR_NULL; // Should point to Node NULL (First Node)
      Read Node('c')
      Check Node('c'): (128,0)
        service[0] = 'c';
        nextParentAddress = (128,2); // Should point to Node 'k'
        prevParentAddress = (128,3); // Should point to Node 'a'
      Read Node('k')
      Check Node('k'): (128,2)
        service[0] = 'k';
        nextParentAddress = (128,1); // Should point to Node 'm'
        prevParentAddress = (128,0); // Should point to Node 'c'
      Read Node('m')
      Check Node('m'): (128,1)
        service[0] = 'm';
        nextParentAddress = NODE_ADDR_NULL; // Should point to Node NULL
        prevParentAddress = (128,2); // Should point to Node 'k'
      Check Handle:
        firstParenNode -> (128,0) (expecting no change)
        nextFreeParentNode -> (129,0)/(128,4) (expecting change)
    ---STEP 5---
      Assuming nextFreeParentNode -> (129,0)/(128,4) TESTED IN STEP 4
      Delete Node('k': Addr->(128,2)
      Read Node('a')
      Check Node('a'): (128,3)
        service[0] = 'a';
        nextParentAddress = (128,0); // Should point to Node 'c'
        prevParentAddress = NODE_ADDR_NULL; // Should point to Node NULL (First Node)
      Read Node('c')
      Check Node('c'): (128,0)
        service[0] = 'c';
        nextParentAddress = (128,1); // Should point to Node 'm'
        prevParentAddress = (128,3); // Should point to Node 'a'
      Read Node('m')
      Check Node('m'): (128,1)
        service[0] = 'm';
        nextParentAddress = NODE_ADDR_NULL; // Should point to Node NULL
        prevParentAddress = (128,0); // Should point to Node 'c'
      Check Handle:
        firstParenNode -> (128,0) (expecting no change)
        nextFreeParentNode -> (128,2)/(128,2) (expecting change)
      ---STEP 6---
        Assuming nextFreeParentNode -> (128,2)TESTED IN STEP 5
        Delete Node('m': Addr->(128,1)
        Check Node('a'): (128,3)
          service[0] = 'a';
          nextParentAddress = (128,0); // Should point to Node 'c'
          prevParentAddress = NODE_ADDR_NULL; // Should point to Node NULL (First Node)
        Read Node('c')
        Check Node('c'): (128,0)
          service[0] = 'c';
          nextParentAddress = NODE_ADDR_NULL; // Should point to Node NULL
          prevParentAddress = (128,3); // Should point to Node 'a'
        Check Handle:
          firstParenNode -> (128,3)
          nextFreeParentNode -> (128,1)
      ---STEP 7---
        Assuming nextFreeParentNode -> (128,1) TESTED IN STEP 6
        Delete Node('a': Addr->(128,3)
        Read Node('c')
        Check Node('c'): (128,0)
          service[0] = 'c';
          nextParentAddress = NODE_ADDR_NULL; // Should point to Node NULL
          prevParentAddress = NODE_ADDR_NULL; // Should point to Node NULL
        Check Handle:
          firstParenNode -> (128,0)
          nextFreeParentNode -> (128,1)
      ---STEP 8---
        Assuming nextFreeParentNode -> (128,1) TESTED IN STEP 7
        Delete Node('c': Addr->(128,0)
        Check Handle:
          firstParenNode -> NODE_ADDR_NULL
          nextFreeParentNode -> (128,0)
      ---STEP 9---
        Assuming nextFreeParentNode -> (128,0) TESTED IN STEP 8
        Create Node('b': Addr->(128, 0))
        Create Node('a': Addr->(128, 1))
          Set child address -> (129, 0)
        Check Handle:
          firstParentNode -> (128,1)
          nextFreeParentNode -> (128, 2)
        Update Node('a': Addr->(128,1))
          By modifying CID in flags
        Check Handle:
          firstParentNode -> (128,1)
          nextFreeParentNode -> (128, 2)
        Update Node('a': Addr->(128,1))
          By modifying service to 'c'
        Check Node('c': (128, 1))
          nextParentAddress -> NODE_ADDR_NULL
          prevParentAddress -> (128, 0)
          nextChildAddress -> (129, 0)
        Check Handle:
          firstParentNode -> (128,0)
          nextFreeParentNode -> (128, 2)
    */
    
    /**********************************************************FIRST PARENT NODE**********************************************************/
    usbPrintf_P(PSTR("---STEP 1---\n"));
    // store old handle vals
    oldHandleNextFreeParenNode = h->nextFreeParentNode;
    oldHandleFirstParentNode = h->firstParentNode;
    
    // setup parentPtr for Node 'c'
    parentPtr->service[0] = 'c';
    
    // create node (store in flash)
    ret = createParentNode(h, parentPtr);
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_1_CREATE_NODE_ERROR;
        return ret;
    }
    
    // check the node
    if(parentPtr->service[0] != 'c')
    {
        *code = PARENT_NODE_TEST_STEP_1_CREATE_NODE_VERIFY_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_1_CREATE_NODE_VERIFY_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_1_CREATE_NODE_VERIFY_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // check the handle
    if(h->firstParentNode == oldHandleFirstParentNode || h->firstParentNode != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_1_VERIFY_HANDLE_FIRST_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(h->nextFreeParentNode == oldHandleNextFreeParenNode || h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 1))
    {
        *code = PARENT_NODE_TEST_STEP_1_VERIFY_HANDLE_NEXT_FREE_NODE_ERROR;
        return RETURN_NOK;
    }
    
    /**********************************************************SECOND PARENT NODE**********************************************************/
    usbPrintf_P(PSTR("---STEP 2---\n"));
    // store old handle vals
    oldHandleNextFreeParenNode = h->nextFreeParentNode;
    oldHandleFirstParentNode = h->firstParentNode;
    
    // setup parentPtr for Node 'm'
    parentPtr->service[0] = 'm';
    
    // create node (store in flash)
    ret = createParentNode(h, parentPtr);
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_2_CREATE_NODE_ERROR;
        return ret;
    }

    // check the node 'm'
    if(parentPtr->service[0] != 'm')
    {
        *code = PARENT_NODE_TEST_STEP_2_CREATE_NODE_VERIFY_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_2_CREATE_NODE_VERIFY_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_2_CREATE_NODE_VERIFY_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // read node 'c'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 0));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_2_READ_NODE_C_ERROR;
        return ret;
    }
    
    // check the node 'c'
    if(parentPtr->service[0] != 'c')
    {
        *code = PARENT_NODE_TEST_STEP_2_VERIFY_NODE_C_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != constructAddress(PAGE_PER_SECTOR, 1))
    {
        *code = PARENT_NODE_TEST_STEP_2_VERIFY_NODE_C_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_2_VERIFY_NODE_C_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // check the handle
    if(h->firstParentNode != oldHandleFirstParentNode || h->firstParentNode != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_2_VERIFY_HANDLE_FIRST_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(h->nextFreeParentNode == oldHandleNextFreeParenNode || h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 2))
    {
        *code = PARENT_NODE_TEST_STEP_2_VERIFY_HANDLE_NEXT_FREE_NODE_ERROR;
        return RETURN_NOK;
    }
    
    /**********************************************************THIRD PARENT NODE**********************************************************/
    usbPrintf_P(PSTR("---STEP 3---\n"));
    // store old handle vals
    oldHandleNextFreeParenNode = h->nextFreeParentNode;
    oldHandleFirstParentNode = h->firstParentNode;
    
    // setup parentPtr for Node 'k'
    parentPtr->service[0] = 'k';
    
    // create node (store in flash)
    ret = createParentNode(h, parentPtr);
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_3_CREATE_NODE_ERROR;
        return ret;
    }

    // check the node 'k'
    if(parentPtr->service[0] != 'k')
    {
        *code = PARENT_NODE_TEST_STEP_3_CREATE_NODE_VERIFY_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != constructAddress(PAGE_PER_SECTOR, 1))
    {
        *code = PARENT_NODE_TEST_STEP_3_CREATE_NODE_VERIFY_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_3_CREATE_NODE_VERIFY_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // read node 'c'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 0));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_3_READ_NODE_C_ERROR;
        return ret;
    }
    
    // check the node 'c'
    if(parentPtr->service[0] != 'c')
    {
        *code = PARENT_NODE_TEST_STEP_3_VERIFY_NODE_C_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != constructAddress(PAGE_PER_SECTOR, 2))
    {
        *code = PARENT_NODE_TEST_STEP_3_VERIFY_NODE_C_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_3_VERIFY_NODE_C_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // read node 'm'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 1));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_3_READ_NODE_M_ERROR;
        return ret;
    }
    
    // check the node 'm'
    if(parentPtr->service[0] != 'm')
    {
        *code = PARENT_NODE_TEST_STEP_3_VERIFY_NODE_M_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_3_VERIFY_NODE_M_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != constructAddress(PAGE_PER_SECTOR, 2))
    {
        *code = PARENT_NODE_TEST_STEP_3_VERIFY_NODE_M_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // check the handle
    if(h->firstParentNode != oldHandleFirstParentNode || h->firstParentNode != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_3_VERIFY_HANDLE_FIRST_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(h->nextFreeParentNode == oldHandleNextFreeParenNode || h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 3))
    {
        *code = PARENT_NODE_TEST_STEP_3_VERIFY_HANDLE_NEXT_FREE_NODE_ERROR;
        return RETURN_NOK;
    }
    
    /**********************************************************Fourth PARENT NODE**********************************************************/
    usbPrintf_P(PSTR("---STEP 4---\n"));
    // store old handle vals
    oldHandleNextFreeParenNode = h->nextFreeParentNode;
    oldHandleFirstParentNode = h->firstParentNode;
    
    // setup parentPtr for Node 'a'
    parentPtr->service[0] = 'a';
    
    // create node (store in flash)
    ret = createParentNode(h, parentPtr);
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_4_CREATE_NODE_ERROR;
        return ret;
    }

    // check the node 'a'
    if(parentPtr->service[0] != 'a')
    {
        *code = PARENT_NODE_TEST_STEP_4_CREATE_NODE_VERIFY_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_4_CREATE_NODE_VERIFY_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_4_CREATE_NODE_VERIFY_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // read node 'c'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 0));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_4_READ_NODE_C_ERROR;
        return ret;
    }
    
    // check the node 'c'
    if(parentPtr->service[0] != 'c')
    {
        *code = PARENT_NODE_TEST_STEP_4_VERIFY_NODE_C_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != constructAddress(PAGE_PER_SECTOR, 2))
    {
        *code = PARENT_NODE_TEST_STEP_4_VERIFY_NODE_C_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != constructAddress(PAGE_PER_SECTOR, 3))
    {
        *code = PARENT_NODE_TEST_STEP_4_VERIFY_NODE_C_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // read node 'k'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 2));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_4_READ_NODE_K_ERROR;
        return ret;
    }
    
    // check the node 'k'
    if(parentPtr->service[0] != 'k')
    {
        *code = PARENT_NODE_TEST_STEP_4_VERIFY_NODE_K_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != constructAddress(PAGE_PER_SECTOR, 1))
    {
        *code = PARENT_NODE_TEST_STEP_4_VERIFY_NODE_K_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_4_VERIFY_NODE_K_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // read node 'm'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 1));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_4_READ_NODE_M_ERROR;
        return ret;
    }
    
    // check the node 'm'
    if(parentPtr->service[0] != 'm')
    {
        *code = PARENT_NODE_TEST_STEP_4_VERIFY_NODE_M_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_4_VERIFY_NODE_M_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != constructAddress(PAGE_PER_SECTOR, 2))
    {
        *code = PARENT_NODE_TEST_STEP_4_VERIFY_NODE_M_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // check the handle (expecting change)
    if(h->firstParentNode == oldHandleFirstParentNode || h->firstParentNode != constructAddress(PAGE_PER_SECTOR, 3))
    {
        *code = PARENT_NODE_TEST_STEP_4_VERIFY_HANDLE_FIRST_NODE_ERROR;
        return RETURN_NOK;
    }
    
    #if NODE_PARENT_PER_PAGE==4
    if(h->nextFreeParentNode == oldHandleNextFreeParenNode || h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR+1, 0))
    #else
    if(h->nextFreeParentNode == oldHandleNextFreeParenNode || h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 4))
    {
        *code = PARENT_NODE_TEST_STEP_4_VERIFY_HANDLE_NEXT_FREE_NODE_ERROR;
        return RETURN_NOK;
    }
    #endif
    

    /**********************************************************DELETE PARENT NODE 'k'**********************************************************/
    usbPrintf_P(PSTR("---STEP 5---\n"));
    // store old handle vals
    oldHandleNextFreeParenNode = h->nextFreeParentNode;
    oldHandleFirstParentNode = h->firstParentNode;
    
    // delete node 'k'
    ret = deleteParentNode(h, constructAddress(PAGE_PER_SECTOR, 2), DELETE_POLICY_WRITE_ONES);
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_5_DELETE_NODE_ERROR;
        return ret;
    }
    
    // read node 'a'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 3));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_5_READ_NODE_A_ERROR;
        return ret;
    }
    
    // check the node 'a'
    if(parentPtr->service[0] != 'a')
    {
        *code = PARENT_NODE_TEST_STEP_5_VERIFY_NODE_A_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_5_VERIFY_NODE_A_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_5_VERIFY_NODE_A_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // read node 'c'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 0));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_5_READ_NODE_C_ERROR;
        return ret;
    }
    
    // check the node 'c'
    if(parentPtr->service[0] != 'c')
    {
        *code = PARENT_NODE_TEST_STEP_5_VERIFY_NODE_C_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != constructAddress(PAGE_PER_SECTOR, 1))
    {
        *code = PARENT_NODE_TEST_STEP_5_VERIFY_NODE_C_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != constructAddress(PAGE_PER_SECTOR, 3))
    {
        *code = PARENT_NODE_TEST_STEP_5_VERIFY_NODE_C_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // read node 'm'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 1));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_5_READ_NODE_M_ERROR;
        return ret;
    }
    
    // check the node 'm'
    if(parentPtr->service[0] != 'm')
    {
        *code = PARENT_NODE_TEST_STEP_5_VERIFY_NODE_M_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_5_VERIFY_NODE_M_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_5_VERIFY_NODE_M_PREV_NODE_ERROR;
        return RETURN_NOK;
    }


    // check the handle
    if(h->firstParentNode != oldHandleFirstParentNode || h->firstParentNode != constructAddress(PAGE_PER_SECTOR, 3))
    {
        *code = PARENT_NODE_TEST_STEP_5_VERIFY_HANDLE_FIRST_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(h->nextFreeParentNode == oldHandleNextFreeParenNode || h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 2))
    {
        *code = PARENT_NODE_TEST_STEP_5_VERIFY_HANDLE_NEXT_FREE_NODE_ERROR;
        return RETURN_NOK;
    }
    
    /**********************************************************DELETE PARENT NODE 'm'**********************************************************/
    usbPrintf_P(PSTR("---STEP 6---\n"));
    // store old handle vals
    oldHandleNextFreeParenNode = h->nextFreeParentNode;
    oldHandleFirstParentNode = h->firstParentNode;
    
    // delete node 'm'
    ret = deleteParentNode(h, constructAddress(PAGE_PER_SECTOR, 1), DELETE_POLICY_WRITE_ONES);
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_6_DELETE_NODE_ERROR;
        return ret;
    }
    
    // read node 'a'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 3));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_6_READ_NODE_A_ERROR;
        return ret;
    }
    
    // check the node 'a'
    if(parentPtr->service[0] != 'a')
    {
        *code = PARENT_NODE_TEST_STEP_6_VERIFY_NODE_A_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_6_VERIFY_NODE_A_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_6_VERIFY_NODE_A_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // read node 'c'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 0));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_6_READ_NODE_C_ERROR;
        return ret;
    }
    
    // check the node 'c'
    if(parentPtr->service[0] != 'c')
    {
        *code = PARENT_NODE_TEST_STEP_6_VERIFY_NODE_C_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_6_VERIFY_NODE_C_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != constructAddress(PAGE_PER_SECTOR, 3))
    {
        *code = PARENT_NODE_TEST_STEP_6_VERIFY_NODE_C_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // check the handle
    if(h->firstParentNode != oldHandleFirstParentNode || h->firstParentNode != constructAddress(PAGE_PER_SECTOR, 3))
    {
        *code = PARENT_NODE_TEST_STEP_6_VERIFY_HANDLE_FIRST_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(h->nextFreeParentNode == oldHandleNextFreeParenNode || h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 1))
    {
        *code = PARENT_NODE_TEST_STEP_6_VERIFY_HANDLE_NEXT_FREE_NODE_ERROR;
        return RETURN_NOK;
    }
    
    /**********************************************************DELETE PARENT NODE 'a'**********************************************************/
    usbPrintf_P(PSTR("---STEP 7---\n"));
    // store old handle vals
    oldHandleNextFreeParenNode = h->nextFreeParentNode;
    oldHandleFirstParentNode = h->firstParentNode;
    
    // delete node 'a'
    ret = deleteParentNode(h, constructAddress(PAGE_PER_SECTOR, 3), DELETE_POLICY_WRITE_ONES);
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_7_DELETE_NODE_ERROR;
        return ret;
    }
    
    // read node 'c'
    ret = readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 0));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_7_READ_NODE_C_ERROR;
        return ret;
    }
    
    // check the node 'c'
    if(parentPtr->service[0] != 'c')
    {
        *code = PARENT_NODE_TEST_STEP_7_VERIFY_NODE_C_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_7_VERIFY_NODE_C_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_7_VERIFY_NODE_C_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // check the handle
    if(h->firstParentNode == oldHandleFirstParentNode || h->firstParentNode != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_7_VERIFY_HANDLE_FIRST_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(h->nextFreeParentNode != oldHandleNextFreeParenNode || h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 1))
    {
        *code = PARENT_NODE_TEST_STEP_7_VERIFY_HANDLE_NEXT_FREE_NODE_ERROR;
        return RETURN_NOK;
    }
    
    
    /**********************************************************DELETE PARENT NODE 'c'**********************************************************/
    usbPrintf_P(PSTR("---STEP 8---\n"));
    // store old handle vals
    oldHandleNextFreeParenNode = h->nextFreeParentNode;
    oldHandleFirstParentNode = h->firstParentNode;
    
    // delete node 'c'
    ret = deleteParentNode(h, constructAddress(PAGE_PER_SECTOR, 0), DELETE_POLICY_WRITE_ONES);
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_8_DELETE_NODE_ERROR;
        return ret;
    }
    
    // check the handle
    if(h->firstParentNode == oldHandleFirstParentNode || h->firstParentNode != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_8_VERIFY_HANDLE_FIRST_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(h->nextFreeParentNode == oldHandleNextFreeParenNode || h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_8_VERIFY_HANDLE_NEXT_FREE_NODE_ERROR;
        return RETURN_NOK;
    }
    
    /**********************************************************UPDATE NODE TEST**********************************************************/
    usbPrintf_P(PSTR("---STEP 9---\n"));
    // store old handle vals
    oldHandleNextFreeParenNode = h->nextFreeParentNode;
    oldHandleFirstParentNode = h->firstParentNode;
    
    // setup flags (type-> parent, vb-> valid, uid-> 0, cid-> 0)
    nodeTypeToFlags(&(parentPtr->flags), NODE_TYPE_PARENT);
    validBitToFlags(&(parentPtr->flags), NODE_VBIT_VALID);
    userIdToFlags(&(parentPtr->flags), 0);
    credentialTypeToFlags(&(parentPtr->flags), 0);
    //set node fields to null addr
    parentPtr->nextChildAddress = NODE_ADDR_NULL;
    parentPtr->nextParentAddress = NODE_ADDR_NULL;
    parentPtr->prevParentAddress = NODE_ADDR_NULL;
    // clear service
    for(i = 0; i < NODE_PARENT_SIZE_OF_SERVICE; i++)
    {
        parentPtr->service[i] = '\0';
    }
    
    parentPtr->service[0] = 'b';
    
    ret = createParentNode(h, parentPtr);
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_9_CREATE_NODE_B_ERROR;
        return ret;
    }
    
    parentPtr->service[0] = 'a';
    parentPtr->nextChildAddress = constructAddress(PAGE_PER_SECTOR+1, 0);
    
    ret = createParentNode(h, parentPtr);
    if(ret != RETURN_OK)
    {
        *code= PARENT_NODE_TEST_STEP_9_CREATE_NODE_A_ERROR;
        return ret;
    }
    
    // check the handle
    if(h->firstParentNode != constructAddress(PAGE_PER_SECTOR, 1))
    {
        *code = PARENT_NODE_TEST_STEP_9_VERIFY_HANDLE_FIRST_NODE_1_ERROR;
        return RETURN_NOK;
    }
    
    if(h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 2))
    {
        *code = PARENT_NODE_TEST_STEP_9_VERIFY_HANDLE_NEXT_FREE_NODE_1_ERROR;
        return RETURN_NOK;
    }
    
    credentialTypeToFlags(&(parentPtr->flags), 2);
    
    ret = updateParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 1));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_9_UPDATE_NODE_A_1_ERROR;
        return ret;
    }
    
    // check the handle
    if(h->firstParentNode != constructAddress(PAGE_PER_SECTOR, 1))
    {
        *code = PARENT_NODE_TEST_STEP_9_VERIFY_HANDLE_FIRST_NODE_2_ERROR;
        return RETURN_NOK;
    }
    
    if(h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 2))
    {
        *code = PARENT_NODE_TEST_STEP_9_VERIFY_HANDLE_NEXT_FREE_NODE_2_ERROR;
        return RETURN_NOK;
    }
    
    readParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 1));
    
    parentPtr->service[0] = 'c';
    
    ret = updateParentNode(h, parentPtr, constructAddress(PAGE_PER_SECTOR, 1));
    if(ret != RETURN_OK)
    {
        *code = PARENT_NODE_TEST_STEP_9_UPDATE_NODE_A_2_ERROR;
        return ret;
    }
    
    // check the node 'c'
    if(parentPtr->service[0] != 'c')
    {
        *code = PARENT_NODE_TEST_STEP_9_VERIFY_NODE_C_SERVICE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->nextParentAddress != NODE_ADDR_NULL)
    {
        *code = PARENT_NODE_TEST_STEP_9_VERIFY_NODE_C_NEXT_NODE_ERROR;
        return RETURN_NOK;
    }
    
    if(parentPtr->prevParentAddress != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_9_VERIFY_NODE_C_PREV_NODE_ERROR;
        return RETURN_NOK;
    }
    
    // check the handle
    if(h->firstParentNode != constructAddress(PAGE_PER_SECTOR, 0))
    {
        *code = PARENT_NODE_TEST_STEP_9_VERIFY_HANDLE_FIRST_NODE_3_ERROR;
        return RETURN_NOK;
    }
    
    if(h->nextFreeParentNode != constructAddress(PAGE_PER_SECTOR, 2))
    {
        *code = PARENT_NODE_TEST_STEP_9_VERIFY_HANDLE_NEXT_FREE_NODE_3_ERROR;
        return RETURN_NOK;
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
    parentNodeTestError ret_code = PARENT_NODE_TEST_PARENT_NODE_AOK;
    
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
    credentialTypeToFlags(&flags, 0);
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
    credentialTypeToFlags(&flags, 0);
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
    usbPrintf_P(PSTR("Parent Node Test returned: %u\n"),ret_code);
    
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