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

/*!  \file     node_mgmt.c
*    \brief    Mooltipass Node Management Library
*    Created:  03/4/2014
*    Author:   Michael Neiderhauser
*/

#include "flash_mem.h"
#include "node_mgmt.h"
#include "defines.h"
#include "usb.h"
#include <string.h>  // For memset
#include <stdint.h>
#include <stddef.h>

// Private Function Prototypes

// Doc Strings Below
static RET_TYPE createChildTypeNode(mgmtHandle *h, uint16_t pAddr, cNode *c, nodeType t, uint8_t dnr);
static RET_TYPE writeReadDataFlash(uint16_t a, uint16_t s, void *d);

/* Flag Get/Set Helper Functions */
/**
 * Gets nodeType from flags  
 * @param   flags           The flags field of a node
 * @return  nodeType        (as uint8) (NODE_TYPE_PARENT, NODE_TYPE_CHILD, NODE_TYPE_CHILD_DATA, NODE_TYPE_DATA)
 * @note    No error checking is performed
 */
uint8_t nodeTypeFromFlags(uint16_t flags)
{
    return (uint8_t)(((flags & NODE_F_TYPE_MASK) >>  NODE_F_TYPE_SHMT) & NODE_F_TYPE_MASK_FINAL);
}

/**
 * Sets nodeType to flags  
 * @param   flags           The flags field of a node
 * @param   nodeType        (as uint8) (NODE_TYPE_PARENT, NODE_TYPE_CHILD, NODE_TYPE_CHILD_DATA, NODE_TYPE_DATA)
 * @return  Does not return
 * @note    No error checking is performed
 */
void  nodeTypeToFlags(uint16_t *flags, uint8_t nodeType)
{
    *flags = (*flags & ~NODE_F_TYPE_MASK) | ((uint16_t)nodeType << NODE_F_TYPE_SHMT);
}

/**
 * Gets the node valid bit from flags  
 * @param   flags           The flags field of a node
 * @return  valid bit       as uint8_t
 * @note    No error checking is performed
 */
uint8_t validBitFromFlags(uint16_t flags)
{
    return (uint8_t)(((flags & NODE_F_VALID_BIT_MASK) >> NODE_F_VALID_BIT_SHMT) & NODE_F_VALID_BIT_MASK_FINAL);
}

/**
 * Sets the node valid bit to flags  
 * @param   flags           The flags field of a node
 * @param   vb              The valid bit state to set in flags (NODE_VBIT_VALID, NODE_VBIT_INVALID)
 * @return  Does not return
 * @note    No error checking is performed
 */
void validBitToFlags(uint16_t *flags, uint8_t vb)
{
    *flags = (*flags & (~NODE_F_VALID_BIT_MASK)) | ((uint16_t)vb << NODE_F_VALID_BIT_SHMT);
}

/**
 * Gets the user id from flags  
 * @param   flags           The flags field of a node
 * @return  user id         as uint8_t
 * @note    No error checking is performed
 */
uint8_t userIdFromFlags(uint16_t flags)
{
    return (uint8_t)(((flags & NODE_F_UID_MASK) >> NODE_F_UID_SHMT) & NODE_F_UID_MASK_FINAL);
}

/**
 * Sets the user id to flags  
 * @param   flags           The flags field of a node
 * @param   uid             The user id to set in flags (0 up to NODE_MAX_UID)
 * @return  Does not return
 * @note    No error checking is performed
 */
void userIdToFlags(uint16_t *flags, uint8_t uid)
{
    *flags = (*flags & (~NODE_F_UID_MASK)) | ((uint16_t)uid << NODE_F_UID_SHMT);
}

/**
 * Gets the credential type from flags  
 * @param   flags           The flags field of a node
 * @return  cred type       as uint8_t
 * @note    No error checking is performed
 */
uint8_t credentialTypeFromFlags(uint16_t flags)
{
    return (uint8_t)(flags & NODE_F_CRED_TYPE_MASK);
}

/**
 * Sets the credential type to flags  
 * @param   flags           The flags field of a node
 * @param   credType        The credential type to set in flags (0 up to NODE_MAX_CRED_TYPE)
 * @return  Does not return
 * @note    No error checking is performed
 */
void credentialTypeToFlags(uint16_t *flags, uint8_t credType)
{
    *flags = (*flags & (~NODE_F_CRED_TYPE_MASK)) | ((uint16_t)credType);
}

/**
 * Gets the data sequence number from flags  
 * @param   flags           The flags field of a node
 * @return  seq num         as uint8_t (0->255)
 * @note    No error checking is performed
 * @note    should only be used with data nodes
 */
uint8_t dataNodeSequenceNumberFromFlags(uint16_t flags)
{
    return (uint8_t)(flags & NODE_F_DATA_SEQ_NUM_MASK);
}

/**
 * Sets the data sequence number to flags  
 * @param   flags           The flags field of a node
 * @param   sid             The sequence number to set in flags (0 -> 255)
 * @return  Does not return
 * @note    No error checking is performed
 * @note    should only be used with data nodes
 */
void dataNodeSequenceNumberToFlags(uint16_t *flags, uint8_t sid)
{
    *flags = (*flags & (~NODE_F_DATA_SEQ_NUM_MASK)) | ((uint16_t)sid);
}

/**
 * Extracts a page number from a constructed address
 * @param   flags           The flags field of a node
 * @param   addr            The constructed address used for extraction
 * @return  page num        A page number in flash memory (uin16_t)
 * @note    No error checking is performed
 * @note    See design notes for address format
 * @note    Max Page Number varies per flash size
 */
uint16_t pageNumberFromAddress(uint16_t addr)
{
    return (addr >> NODE_ADDR_SHMT) & NODE_ADDR_PAGE_MASK;
}

/**
 * Extracts a node number from a constructed address
 * @param   flags           The flags field of a node
 * @param   addr            The constructed address used for extraction
 * @return  node num        A node number of a node in a page in flash memory (uint8_t)
 * @note    No error checking is performed
 * @note    See design notes for address format
 * @note    Max Node Number varies per flash size
 */
uint8_t nodeNumberFromAddress(uint16_t addr)
{
    return (uint8_t)(addr & NODE_ADDR_NODE_MASK);
}

/**
 * Constructs an address for node storage in memory consisting of a page number and node number
 * @param   pageNumber      The page number to be encoded
 * @param   nodeNumber      The node number to be encoded
 * @return  address         The constructed / encoded address
 * @note    No error checking is performed
 * @note    See design notes for address format
 * @note    Max Page Number and Node Number vary per flash size
 */
uint16_t constructAddress(uint16_t pageNumber, uint8_t nodeNumber)
{
    return ((pageNumber << NODE_ADDR_SHMT) | ((uint16_t)nodeNumber));
}

/**
 * Packs a uint16_t type with a date code in format YYYYYYYMMMMDDDDD. Year Offset from 2010
 * @param   year            The year to pack into the uint16_t
 * @param   month           The month to pack into the uint16_t
 * @param   day             The day to pack into the uint16_t
 * @return  date            The constructed / encoded date in uint16_t
 * @note    No error checking is performed
 */
uint16_t constructDate(uint8_t year, uint8_t month, uint8_t day)
{
    return (day | ((month << NODE_MGMT_MONTH_SHT) & NODE_MGMT_MONTH_MASK) | ((year << NODE_MGMT_YEAR_SHT) & NODE_MGMT_YEAR_MASK));
}

/**
 * Unpacks a unint16_t to extract the year, month, and day information in format of YYYYYYYMMMMDDDDD. Year Offset from 2010
 * @param   date            The constructed / encoded date in uint16_t to unpack
 * @param   year            The unpacked year 
 * @param   month           The unpacked month
 * @param   day             The unpacked day
 * @return  success status
 * @note    No error checking is performed
 */
RET_TYPE extractDate(uint16_t date, uint8_t *year, uint8_t *month, uint8_t *day)
{
    *year = ((date >> NODE_MGMT_YEAR_SHT) & NODE_MGMT_YEAR_MASK_FINAL);
    *month = ((date >> NODE_MGMT_MONTH_SHT) & NODE_MGMT_MONTH_MASK_FINAL);
    *day = (date & NODE_MGMT_DAY_MASK_FINAL);
    return RETURN_OK;
}

/**
 * Formats the user profile flash memory of user uid.
 * @param   uid             The id of the user to format profile memory
 * @return  success status
 * @note    Only formats the users starting parent node address and favorites 
 */
RET_TYPE formatUserProfileMemory(uint8_t uid)
{
    uint16_t pageNumber;
    uint16_t pageOffset;
    uint8_t buf[USER_PROFILE_SIZE];
    RET_TYPE ret = RETURN_NOK;
    
    if(uid >= NODE_MAX_UID)
    {
        return RETURN_NOK;
    }
    
    // Set buffer to all 0's. Assuming NODE_ADDR_NULL = 0x0000.
    memset(buf, 0, USER_PROFILE_SIZE);
    
    // obtain user profile offset.
    // write buf (0's) to clear out user memory. Buffer is destroyed.. we no longer need it
    userProfileStartingOffset(uid, &pageNumber, &pageOffset);
    ret = writeDataToFlash(pageNumber, pageOffset, USER_PROFILE_SIZE, buf);
    
    return ret;
}

/**
 * Obtains page and page offset for a given user id
 * @param   uid             The id of the user to perform that profile page and offset calculation (0 up to NODE_MAX_UID)
 * @param   page            The page containing the user profile
 * @param   pageOffset      The offset of the page that indicates the start of the user profile
 * @return  success status
 * @note    Calculation will take place even if the uid is not valid (no starting parent)
 * @note    uid must be in range
 */
RET_TYPE userProfileStartingOffset(uint8_t uid, uint16_t *page, uint16_t *pageOffset)
{
    if(uid >= NODE_MAX_UID)
    {
        return RETURN_NOK;
    }
    
    uint16_t offset = uid * USER_PROFILE_SIZE;
    
    *page = (uint16_t)((offset / BYTES_PER_PAGE) - ((offset / BYTES_PER_PAGE) % 1));
    *pageOffset = (uint16_t)(offset % BYTES_PER_PAGE);
    
    return RETURN_OK;
}

/**
 * Initializes the Node Management Handle.
 *   Check userIdNum in range,  reads users profile to get the starting parent node, scans memory for the next free parent and child nodes.
 * @param   h               The user allocated node management handle
 * @param   userIdNum       The user id to initialize the handle for (0->NODE_MAX_UID)
 * @return  success status
 */
RET_TYPE initNodeManagementHandle(mgmtHandle *h, uint8_t userIdNum)
{    
    RET_TYPE ret = RETURN_NOK;
    uint8_t cNodeScanStartNode = 0;
    
    if(h == NULL || userIdNum >= NODE_MAX_UID)
    {
        // param error
        return ret;
    }
             
    h->flags = 0;
    h->currentUserId = userIdNum;
    
    // read starting parent address
    ret = readStartingParent(h, &(h->firstParentNode));
    
    if(ret != RETURN_OK)
    {
        // if flash error
        return ret;
    }
    
    // scan for next free parent node (starting sector 1, page 0, node 0)
    ret = scanNextFreeParentNode(h, constructAddress(PAGE_PER_SECTOR, 0));
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    if(NODE_PARENT_PER_PAGE == 4)
    {
        cNodeScanStartNode = 2;
    }
    else
    {
        cNodeScanStartNode = 6;    
    }
    ret = scanNextFreeChildNode(h, constructAddress(PAGE_COUNT-1, cNodeScanStartNode));
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    return RETURN_OK;
}

/**
 * Sets the users starting parent node both in the handle and user profile memory portion of flash
 * @param   h               The user allocated node management handle
 * @param   parentAddress   The constructed address of the users starting parent node (alphabetically) 
 * @return  success status
 */
RET_TYPE setStartingParent(mgmtHandle *h, uint16_t parentAddress)
{
    RET_TYPE ret = RETURN_NOK;
    uint16_t userProfilePage = 0;
    uint16_t userProfilePageOffset = 0;
    uint16_t nodePage = 0;
    uint16_t nodePageOffset;
    
    nodePage = pageNumberFromAddress(parentAddress);
    nodePageOffset = nodeNumberFromAddress(parentAddress);
    
    // error checking passed in address parts
    if(nodePage >= PAGE_COUNT || nodePageOffset >= NODE_PARENT_PER_PAGE)
    {
        // invalid address
        return RETURN_NOK;
    }
    
    // no error checking.. user id was validated at init
    userProfileStartingOffset(h->currentUserId, &userProfilePage, &userProfilePageOffset);
    
    // update handle
    h->firstParentNode = parentAddress;
    
    // write memory -> WARNING parent address will be destroyed
    ret = writeDataToFlash(userProfilePage, userProfilePageOffset, 2, &parentAddress);
    if(ret != RETURN_NOK)
    {
        return ret;
    }
    
    // restore parentAddress (todo from handle?)
    ret = readDataFromFlash(userProfilePage, userProfilePageOffset, 2, &parentAddress);
    if(ret != RETURN_NOK)
    {
        return ret;
    }
    
    return RETURN_OK;
}

/**
 * Gets the users starting parent node from the user profile memory portion of flash
 * @param   h               The user allocated node management handle
 * @param   parentAddress   The constructed address of the users starting parent node (alphabetically) 
 * @return  success status
 */
RET_TYPE readStartingParent(mgmtHandle *h, uint16_t *parentAddress)
{
    RET_TYPE ret = RETURN_NOK;
    uint16_t userProfilePage = 0;
    uint16_t userProfilePageOffset = 0;
    
    // no error checking.. user id was validated at init. Get profile Address
    userProfileStartingOffset(h->currentUserId, &userProfilePage, &userProfilePageOffset);
    
    // restore parentAddress
    ret = readDataFromFlash(userProfilePage, userProfilePageOffset, 2, parentAddress);
    if(ret != RETURN_NOK)
    {
        return ret;
    }
    
    return RETURN_OK;
}

/**
 * Sets a user favorite in the user profile
 * @param   h               The user allocated node management handle
 * @param   favId           The id number of the fav record
 * @param   parentAddress   The parent node address of the fav
 * @param   childAddress    The child node address of the fav
 * @return  success status
 */
RET_TYPE setFav(mgmtHandle *h, uint8_t favId, uint16_t parentAddress, uint16_t childAddress)
{
    RET_TYPE ret = RETURN_OK;
    uint16_t page;
    uint16_t offset;
    uint16_t addrs[2];
    
    if(favId >= USER_MAX_FAV)
    {
        return RETURN_NOK;
    }
    
    addrs[0] = parentAddress;
    addrs[1] = childAddress;
    
    // calculate user profile start
    ret  = userProfileStartingOffset(h->currentUserId, &page, &offset);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    // add to offset
    offset += (favId * 4) + 2;  // each fav is 4 bytes. +2 for starting parent node offset
    
    // write to flash
    ret = writeDataToFlash(page, offset, 4, (void *)addrs);
    if(ret != RETURN_OK)
    {
        return ret;
    }

    return ret;
}

/**
 * Reads a user favorite from the user profile
 * @param   h               The user allocated node management handle
 * @param   favId           The id number of the fav record
 * @param   parentAddress   The parent node address of the fav
 * @param   childAddress    The child node address of the fav
 * @return  success status
 */
RET_TYPE readFav(mgmtHandle *h, uint8_t favId, uint16_t *parentAddress, uint16_t *childAddress)
{
    RET_TYPE ret = RETURN_OK;
    uint16_t page;
    uint16_t offset;
    uint16_t addrs[2];
    
    if(favId >= USER_MAX_FAV)
    {
        return RETURN_NOK;
    }
    
    // calculate user profile start
    ret  = userProfileStartingOffset(h->currentUserId, &page, &offset);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    // add to offset
    offset += (favId * 4) + 2;  // each fav is 4 bytes. +2 for starting parent node offset
    
    // write to flash
    ret = readDataFromFlash(page, offset, 4, (void *)addrs);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    // return values to user
    *parentAddress = addrs[0];
    *childAddress = addrs[1];

    return ret;
}

/**
 * Sets the users base CTR in the user profile flash memory
 * @param   h               The user allocated node management handle
 * @param   buf             The buffer containing CTR
 * @param   bufSize         The size of the buffer containing CTR (Most likely 3)
 * @return  Success status
 */
RET_TYPE setProfileCtr(mgmtHandle *h, void *buf, uint8_t bufSize)
{
    RET_TYPE ret = RETURN_OK;
    uint16_t page;
    uint16_t offset;

    if(!buf)
    {
        return RETURN_NOK;
    }

    if(bufSize > USER_CTR_SIZE)
    {
        return RETURN_NOK;
    }

    // calculate user profile start
    ret  = userProfileStartingOffset(h->currentUserId, &page, &offset);
    if(ret != RETURN_OK)
    {
        return ret;
    }

    offset += USER_PROFILE_SIZE-USER_RES_CTR; // User CTR is at the end

    ret = writeDataToFlash(page, offset, bufSize, buf);

    return ret;
}

/**
 * Reads the users base CTR from the user profile flash memory
 * @param   h               The user allocated node management handle
 * @param   buf             The buffer to store the read CTR
 * @param   bufSize         The size of the buffer to store the read CTR (Most likely 3)
 * @return  Success status
 */
RET_TYPE readProfileCtr(mgmtHandle *h, void *buf, uint8_t bufSize)
{
    RET_TYPE ret = RETURN_OK;
    uint16_t page;
    uint16_t offset;

    if(!buf)
    {
        return RETURN_NOK;
    }

    if(bufSize > USER_CTR_SIZE)
    {
        return RETURN_NOK;
    }

    // calculate user profile start
    ret  = userProfileStartingOffset(h->currentUserId, &page, &offset);
    if(ret != RETURN_OK)
    {
        return ret;
    }

    offset += USER_PROFILE_SIZE; // does not include ctr val.. this will set the correct offset

    ret = readDataFromFlash(page, offset, bufSize, buf);

    return ret;
}

/**
 * Scans flash memory to find the address of the next free parent node (next free write location).
 *   Sets nextFreeParentNode in the node management handle.  If no free memory, nextFreeParentNode is set to NODE_ADDR_NULL
 * @param   h               The user allocated node management handle
 * @param   startingAddress The address of the first node to examine
 * @return  success status
 * @note    This operation may take some time
 */
RET_TYPE scanNextFreeParentNode(mgmtHandle *h, uint16_t startingAddress)
{
    uint16_t nodeFlags = 0xFFFF;
    
    uint16_t pageItr = 0;
    uint8_t nodeItr = 0;
    RET_TYPE ret = RETURN_OK;
    
    // for each page
    for(pageItr = pageNumberFromAddress(startingAddress); pageItr < PAGE_COUNT; pageItr++)
    {
        // for each possible parent node in the page (changes per flash chip)
        for(nodeItr = nodeNumberFromAddress(startingAddress); nodeItr < NODE_PARENT_PER_PAGE; nodeItr++)
        {
            // read node flags
            // 2 bytes - fixed size
            ret = readDataFromFlash(pageItr, NODE_SIZE_PARENT*nodeItr, 2, &nodeFlags);
            if(ret != RETURN_OK)
            {
                return ret;
            }
            
            // process node flags
            // match criteria - valid bit is invalid
            if(validBitFromFlags(nodeFlags) == NODE_VBIT_INVALID)
            {
                // next free parent node found.
                // verify page does not contain child nodes
                // NODE_PARENT_PER_PAGE/2 -> 2 parent nodes per child node
                for(uint8_t childOffset = 0; childOffset < (NODE_PARENT_PER_PAGE/2); childOffset++)
                {
                    ret = readDataFromFlash(pageItr, NODE_SIZE_CHILD*childOffset, 2, &nodeFlags);
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                    
                    // if node is valid and not parent..
                    if(validBitFromFlags(nodeFlags) == NODE_VBIT_VALID && nodeTypeFromFlags(nodeFlags) != NODE_TYPE_PARENT)
                    {
                        // parent node stack / child node heap collide
                        h->nextFreeParentNode = NODE_ADDR_NULL;
                        return RETURN_OK;
                    }    
                } // end if child not in page.. else continue
                            
                // construct address with pageItr (page number) and nodeItr (node number)
                h->nextFreeParentNode = constructAddress(pageItr, nodeItr);
                // return early
                return RETURN_OK;
            } // end valid bit invalid
            else 
            {
                // if node is valid check node type
                // if we read something other than a parent node.. memory is colliding. return
                // stack -> parent nodes, heap-> child / data nodes.  stack will go into heap.. prevent this
                // Returns OK but sets address to null
                if(nodeTypeFromFlags(nodeFlags) != NODE_TYPE_PARENT)
                {
                    h->nextFreeParentNode = NODE_ADDR_NULL;
                    return RETURN_OK;
                } // check for node type
            }// end if valid
        } // end for each possible node
    } // end for each page
    
    // we have visited the entire chip (should not happen?)
    // no free nodes found.. set to null
    h->nextFreeParentNode = NODE_ADDR_NULL;
    // Return OK.  Users responsibility to check nextFreeParentNode
    return RETURN_OK;
}

/**
 * Writes a parent node to memory (next free via handle) (in alphabetical order).
 *   Scans for nextFreeParentNode after completion.  Modifies the node management handle
 * @param   h               The user allocated node management handle
 * @param   p               The parent node to write to memory (nextFreeParentNode)
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE createParentNode(mgmtHandle *h, pNode *p)
{
    RET_TYPE ret = RETURN_OK;
    uint16_t addr = NODE_ADDR_NULL;
    //pNode memNode;
    pNode *memNodePtr = &(h->parent);
    int8_t res = 0;
    
    //uint16_t writeAddr;
    
    if((h->nextFreeParentNode) == NODE_ADDR_NULL)
    {
        // no space remaining in flash
        return RETURN_NOK;
    }
    
    userIdToFlags(&(p->flags), h->currentUserId);
    /*
    if((h->currentUserId) != userIdFromFlags(p->flags))
    {
        // cannot create a node with a different user ID
        return RETURN_NOK;
    }
    */
    
    // set node type
    nodeTypeToFlags(&(p->flags), NODE_TYPE_PARENT);
    
    // set valid bit
    validBitToFlags(&(p->flags), NODE_VBIT_VALID);
    
    // TODO verify nextChildAddress Fix (modified updateParentNode (backup / restore of next child addr) to allow setting p->nextChildAddress to NODE_ADDR_NULL)
    p->nextChildAddress = NODE_ADDR_NULL;
    p->nextParentAddress = NODE_ADDR_NULL;
    p->prevParentAddress = NODE_ADDR_NULL;
    
    // if user has no nodes. this node is the first node
    if(h->firstParentNode == NODE_ADDR_NULL)
    {
        // write parent node to flash (destructive)
        ret = writeDataToFlash(pageNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT, &(*p));
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        // read back from flash
        ret = readDataFromFlash(pageNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT, &(*p));
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        // set the starting node address
        setStartingParent(h, h->nextFreeParentNode);
        // set next free to null.. scan will happen at the end of the function
        h->nextFreeParentNode = NODE_ADDR_NULL;
    }
    else
    {
        // not the first node
        
        // get first node address
        addr = h->firstParentNode;
        while(addr != NODE_ADDR_NULL)
        {
            // read node
            ret = readParentNode(h, memNodePtr, addr);
            if(ret != RETURN_OK)
            {
                return ret;
            }
            
            // compare nodes (alphabetically) 
            res = memcmp(p->service, memNodePtr->service, NODE_PARENT_SIZE_OF_SERVICE);
            if(res > 0)
            {
                // to add parent node comes after current node in memory.. go to next node
                if(memNodePtr->nextParentAddress == NODE_ADDR_NULL)
                {
                    // end of linked list. Set to write node prev and next addr's
                    p->nextParentAddress = NODE_ADDR_NULL;
                    p->prevParentAddress = addr; // current memNode Addr
                    
                    // write new node to flash
                    ret = writeDataToFlash(pageNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT, &(*p));
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                    
                    // read back from flash
                    ret = readDataFromFlash(pageNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT, &(*p));
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                    
                    // set previous last node to point to new node. write to flash
                    memNodePtr->nextParentAddress = h->nextFreeParentNode;
                    ret = writeDataToFlash(pageNumberFromAddress(addr), NODE_SIZE_PARENT * nodeNumberFromAddress(addr), NODE_SIZE_PARENT, memNodePtr);
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                    
                    // read node from flash.. writes are destructive.
                    ret = readParentNode(h, &(*p), h->nextFreeParentNode);
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                    
                    // set loop exit case
                    h->nextFreeParentNode = NODE_ADDR_NULL; 
                    addr = NODE_ADDR_NULL; 
                }
                else
                {
                    // loop and read next node
                    addr = memNodePtr->nextParentAddress;
                }
            }
            else if(res < 0)
            {
                // to add parent node comes before current node in memory. Previous node is already not a memcmp match .. write node
                
                // set node to write next parent to current node in mem, set prev parent to current node in mems prev parent
                p->nextParentAddress = addr;
                p->prevParentAddress = memNodePtr->prevParentAddress;
                //writeAddr = h->nextFreeParentNode;
                // write new node to flash
                ret = writeDataToFlash(pageNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT, &(*p));
                if(ret != RETURN_OK)
                {
                    return ret;
                }
                
                // read back from flash (needed?)
                ret = readDataFromFlash(pageNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeParentNode), NODE_SIZE_PARENT, &(*p));
                if(ret != RETURN_OK)
                {
                    return ret;
                }
                
                // read p->next from flash
                ret = readDataFromFlash(pageNumberFromAddress(p->nextParentAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(p->nextParentAddress), NODE_SIZE_PARENT, memNodePtr);
                if(ret != RETURN_OK)
                {
                    return ret;
                }
                
                // update current node in mem. set prev parent to address node to write was written to.
                memNodePtr->prevParentAddress = h->nextFreeParentNode;
                ret = writeDataToFlash(pageNumberFromAddress(p->nextParentAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(p->nextParentAddress), NODE_SIZE_PARENT, memNodePtr);
                if(ret != RETURN_OK)
                {
                    return ret;
                }
                
                if(p->prevParentAddress != NODE_ADDR_NULL)
                {
                    // read p->prev node
                    ret = readParentNode(h, memNodePtr, p->prevParentAddress);
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                
                    // update prev node to point next parent to addr of node to write node
                    memNodePtr->nextParentAddress = h->nextFreeParentNode;
                    ret = writeDataToFlash(pageNumberFromAddress(p->prevParentAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(p->prevParentAddress), NODE_SIZE_PARENT, memNodePtr);
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                }                
                
                if(addr == h->firstParentNode)
                {
                    // new node comes before current address and current address in first node.
                    // new node should be first node
                    setStartingParent(h, h->nextFreeParentNode);
                }
                
                // read node from flash.. writes are destructive.
                ret = readParentNode(h, &(*p), h->nextFreeParentNode);
                if(ret != RETURN_OK)
                {
                    return ret;
                }
                // set handle nextFreeParent to null
                h->nextFreeParentNode = NODE_ADDR_NULL; 
                addr = NODE_ADDR_NULL; // exit case
            }
            else
            {
                // services match
                // return nok. Same parent node
                return RETURN_NOK;
            } // end cmp results
        } // end while
    } // end if first parent
    
    ret = scanNextFreeParentNode(h, constructAddress(PAGE_PER_SECTOR, 0));
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
   return RETURN_OK;
}

/**
 * Reads a parent node from memory. If the node does not have a proper user id, p should be considered undefined
 * @param   h               The user allocated node management handle
 * @param   p               Storage for the node from memory
 * @param   parentNodeAddress The address to read in memory
 * @return  success status
 */
RET_TYPE readParentNode(mgmtHandle *h, pNode * p, uint16_t parentNodeAddress)
{
    RET_TYPE ret = RETURN_OK;
    ret = readDataFromFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, &(*p));
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    if((h->currentUserId != userIdFromFlags(p->flags)) || (validBitFromFlags(p->flags) == NODE_VBIT_INVALID))
    {
        // if handle user id != id from node or node is invalid
        // clear local node.. return not ok
        invalidateParentNode(p);
        return RETURN_NOK;
    }
    return ret;
}

/**
 * Updates a parent node in memory. Handles alphabetical reorder of nodes.
 *   Scans for nextFreeParentNode after completion.  Modifies the node management handle
 * @param   h               The user allocated node management handle
 * @param   p               The parent node that has been modified and is requested to be updated in memory
 * @param   parentNodeAddress The address to read in memory
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE updateParentNode(mgmtHandle *h, pNode *p, uint16_t parentNodeAddress)
{
    RET_TYPE ret = RETURN_OK;
    pNode *ip = &(h->parent);
    uint16_t addr;
    uint16_t newParentAddr;
    // read the node at parentNodeAddress
    ret = readParentNode(h, ip, parentNodeAddress);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    // Do not allow the user to update another users node (passed in)
    if(userIdFromFlags(p->flags) != h->currentUserId)
    {
        invalidateParentNode(p);
        return RETURN_NOK;
    }
    
    // Do not allow the user to update another users node (read from memory)
    if(userIdFromFlags(ip->flags) != h->currentUserId)
    {
        // should hopefully be handled by readParentNode
        invalidateParentNode(ip);
        invalidateParentNode(p);
        return RETURN_NOK;
    }
    
    // Do not allow the user to change linked list links, or change child link (will be done internally)
    if((p->nextChildAddress != ip->nextChildAddress) || (p->nextParentAddress != ip->nextParentAddress) || (p->prevParentAddress != ip->prevParentAddress))
    {
        return RETURN_NOK;
    }
    
    // nodes are identical.. do not write
    if(memcmp(&(*p), ip, NODE_SIZE_PARENT) == 0)
    {        
        return RETURN_OK;
    }
    
    // only things allowed to change are the service and flags.
    if(memcmp(&(p->service), &(ip->service), NODE_PARENT_SIZE_OF_SERVICE) == 0)
    {
        // service is identical just rewrite the node
        ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, &(*p));
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        // write is destructive.. read
        ret = readParentNode(h, &(*p), parentNodeAddress);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    else
    {
        // service is not identical.. delete node, and create node (alphabetical sorting)
        
        // no additional error checking. Assuming the node the user wants to write is p.
        // next and prev links will be modified. 
        
        //Backup nextChildAddress from ip.  Must be removed for deleteParentNode function to work
        addr = ip->nextChildAddress;
        // remove ip->nextChildAddress in flash mem to allow for deleteParentNode to detect that the parent has no children and can be deleted
        if(ip->nextChildAddress != NODE_ADDR_NULL)
        {
            ip->nextChildAddress = NODE_ADDR_NULL; // bypass delete security measure
            // write node to memory
            ret = writeReadDataFlash(parentNodeAddress, NODE_SIZE_PARENT, ip);
            //ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, ip);
            if(ret != RETURN_OK)
            {
                return ret;
            }
        }
        
        // delete node in memory handles doubly linked list management
        ret = deleteParentNode(h, parentNodeAddress, DELETE_POLICY_WRITE_ONES);
        if(ret != RETURN_OK)
        {
            return ret;
        }

        // backup nextParentAddr (my not be at previous location due to how memory mgmt handles memory)
        newParentAddr = h->nextFreeParentNode;
        
        // create node in memory (new node p) handles doubly linked list management
        ret = createParentNode(h, *(&p));
        if(ret != RETURN_OK)
        {
            return ret;
        }

        // node should be at newParentAddr

        // read written node (into internal buffer)
        ret = readParentNode(h, ip, newParentAddr);
        if(ret != RETURN_OK)
        {
            return ret;
        }

        // restore addr backup (modify internal buffer)
        ip->nextChildAddress = addr;
        // write node to memory (from internal buffer)
        ret = writeReadDataFlash(newParentAddr, NODE_SIZE_PARENT, ip);
        //ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    return ret; 
}

/**
 * Deletes a parent node from memory.  This node CANNOT have any children
 * @param   h               The user allocated node management handle
 * @param   parentNodeAddress The address to read in memory
 * @param   policy          How to handle the delete @ref deletePolicy
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE deleteParentNode(mgmtHandle *h, uint16_t parentNodeAddress, deletePolicy policy)
{
    RET_TYPE ret = RETURN_OK;
    pNode *ip = &(h->parent);
    uint16_t prevAddress;
    uint16_t nextAddress;
    
    // read node to delete
    ret = readParentNode(h, ip, parentNodeAddress);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    if(userIdFromFlags(ip->flags) != h->currentUserId)
    {
        // cannot allow current user to modify node
        // node does not belong to user
        return RETURN_NOK;
    }
    
    if(ip->nextChildAddress != NODE_ADDR_NULL)
    {
        // parent has children. cannot delete
        return RETURN_NOK;
    }
    
    if(validBitFromFlags(ip->flags) != NODE_VBIT_VALID)
    {
        // cannot allow operation on invalid node
        return RETURN_NOK;
    }
    
    // store previous and next node of node to be deleted
    prevAddress = ip->prevParentAddress;
    nextAddress = ip->nextParentAddress;
    
    // delete node (not using update due to 'delete' operation)
    if(policy == DELETE_POLICY_WRITE_NOTHING)
    {
        // set node as invalid.. update
        validBitToFlags(&(ip->flags), NODE_VBIT_INVALID);
        ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    else if(policy == DELETE_POLICY_WRITE_ONES)
    {
        // memset parent node to all 0's.. set valid bit to invalid.. write
        memset(ip, DELETE_POLICY_WRITE_ONES, NODE_SIZE_PARENT);
        validBitToFlags(&(ip->flags), NODE_VBIT_INVALID); 
        ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    else if(policy == DELETE_POLICY_WRITE_ZEROS)
    {
        // memset parent node to all 1's.. set valid bit to invalid.. write
        memset(ip, DELETE_POLICY_WRITE_ZEROS, NODE_SIZE_PARENT);
        validBitToFlags(&(ip->flags), NODE_VBIT_INVALID);
        ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    
    // set previousParentNode.nextParentAddress to this.nextParentAddress
    if(prevAddress != NODE_ADDR_NULL)
    {
        // read node
        ret = readParentNode(h, ip, prevAddress);
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        // set address
        ip->nextParentAddress = nextAddress;
        
        // update node
        ret = writeDataToFlash(pageNumberFromAddress(prevAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(prevAddress), NODE_SIZE_PARENT, ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }

    // set nextParentNode.prevParentNode to this.prevParentNode
    if(nextAddress != NODE_ADDR_NULL)
    {
        // read node
        ret = readParentNode(h, ip, nextAddress);
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        // set address
        ip->prevParentAddress = prevAddress;
        // update node
        ret = writeDataToFlash(pageNumberFromAddress(nextAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(nextAddress), NODE_SIZE_PARENT, ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    
    if(h->firstParentNode == parentNodeAddress)
    {
        // removed starting node. prev should be null
        // if nextAddress == NODE_ADDR _NULL.. we have no nodes left
        //     set starting parent to null (eg next)
        // if nextAddress != NODE_ADDR_NULL.. we have nodes left
        //     set starting parent to next
        // Long story short.. set starting parent to next always
        setStartingParent(h, nextAddress);
    }
    
    if(pageNumberFromAddress(parentNodeAddress) < pageNumberFromAddress(h->nextFreeParentNode))
    {
        // removed node page is less than next free page (closer page)
        // set next free node to recently removed node address
        h->nextFreeParentNode = parentNodeAddress;
    }
    else if(pageNumberFromAddress(parentNodeAddress) == pageNumberFromAddress(h->nextFreeParentNode))
    {
        // removed node is in the same page as next free
        // check node number
        if(nodeNumberFromAddress(parentNodeAddress) < nodeNumberFromAddress(h->nextFreeParentNode))
        {
            // node number is lesser.. set next free
            h->nextFreeParentNode = parentNodeAddress;
        }            
    }
    // else parentNodeAddress > h->nextFreeParentNode.. do nothing
    
    return RETURN_OK;
}

/**
 * Sets to contents of a parent node to null
 * @param   p               The parent node to invalidate
 * @return  success status
 */
RET_TYPE invalidateParentNode(pNode *p)
{
    p->flags=0xFF;
    p->nextChildAddress = NODE_ADDR_NULL;
    p->nextParentAddress = NODE_ADDR_NULL;
    p->prevParentAddress = NODE_ADDR_NULL;
    for(uint8_t i = 0; i < NODE_PARENT_SIZE_OF_SERVICE; i++)
    {
        p->service[i] = 0;
    }
    return RETURN_OK;
}

/**
 * Sets to contents of a child node to null
 * @param   c               The child node to invalidate
 * @return  success status
 */
RET_TYPE invalidateChildNode(cNode *c)
{
    uint8_t i = 0;
    c->flags=0xFF;
    c->nextChildAddress = NODE_ADDR_NULL;
    c->nextChildAddress = NODE_ADDR_NULL;
    c->dateCreated = 0;
    c->dateLastUsed = 0;
    
    for(i = 0; i < NODE_CHILD_SIZE_OF_CTR; i++)
    {
        c->ctr[i] = 0;    
    }
    
    for(i = 0; i < NODE_CHILD_SIZE_OF_DESCRIPTION; i++)
    {
        c->description[i] = 0;
    }
    
    for(i = 0; i < NODE_CHILD_SIZE_OF_LOGIN; i++)
    {
        c->login[i] = 0;
    }
    
    for(i = 0; i < NODE_CHILD_SIZE_OF_PASSWORD; i++)
    {
        c->password[i] = 0;
    }
    return RETURN_OK;
}

/**
 * Scans flash memory to find the address of the next free child node (next free write location).
 *   Sets nextFreeChildNode in the node management handle.  If no free memory, nextFreeChildNode is set to NODE_ADDR_NULL
 * @param   h               The user allocated node management handle
 * @param   startingAddress The address of the first node to examine
 * @return  success status
 * @note    This operation may take some time
 */
RET_TYPE scanNextFreeChildNode(mgmtHandle *h, uint16_t startingAddress)
{
    uint16_t nodeFlags = 0xFFFF;
    uint16_t pageItr = pageNumberFromAddress(startingAddress);
    uint8_t nodeItr =  nodeNumberFromAddress(startingAddress);
    RET_TYPE ret = RETURN_OK;
    
    // shift node itr if needed.
    if(nodeItr % 2 != 0)
    {
        // nodeItr is odd. Child node must be aligned on 0, 2 (, 4, 6) index
        // set nodeItr to max idx (2 or 6)
        if(NODE_PARENT_PER_PAGE == 4)
        {
            // small pages
            nodeItr = 2;
        }
        else
        {
            // large pages
            nodeItr = 6;
        }
        
        // update starting address
        startingAddress = constructAddress(pageItr, nodeItr);
    }
    
    // for each page
    for(pageItr = pageNumberFromAddress(startingAddress); pageItr >= 0; pageItr--)
    {
        // for each possible child node in the page (changes per flash chip)
        // subtract 2 nodes (sizeof(child) = 2*sizeof(parent))
        for(nodeItr = nodeNumberFromAddress(startingAddress); nodeItr >= 0; nodeItr-=2)
        {
            // read node flags
            // 2 bytes - fixed size
            // Offset is of node size parent.  child node consists of 2 parent nodes
            ret = readDataFromFlash(pageItr, NODE_SIZE_PARENT*nodeItr, 2, &nodeFlags);
            if(ret != RETURN_OK)
            {
                return ret;
            }
            
            // process node flags
            // match criteria - valid bit is invalid
            if(validBitFromFlags(nodeFlags) == NODE_VBIT_INVALID)
            {
                // next free child node found.
                // verify page does not contain parent nodes
                for(uint8_t parentOffset = 0; parentOffset < NODE_PARENT_PER_PAGE; parentOffset++)
                {
                    ret = readDataFromFlash(pageItr, NODE_SIZE_PARENT*parentOffset, 2, &nodeFlags);
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                    
                    if(parentOffset % 2 == 0 && validBitFromFlags(nodeFlags) == NODE_VBIT_VALID && nodeTypeFromFlags(nodeFlags) != NODE_TYPE_PARENT)
                    {
                        // some sort of child node.. increment parentOffset to bypass false positive on next itr
                        parentOffset++;
                    }
                    else if(validBitFromFlags(nodeFlags) == NODE_VBIT_VALID && nodeTypeFromFlags(nodeFlags) == NODE_TYPE_PARENT)
                    {
                        // if node is valid and is parent..
                        // parent node stack / child node heap collide
                        h->nextFreeParentNode = NODE_ADDR_NULL;
                        return RETURN_OK;
                    }
                } // end if parent not in page.. else continue
                
                // construct address with pageItr (page number) and nodeItr (node number)
                h->nextFreeChildNode = constructAddress(pageItr, nodeItr);
                // return early
                return RETURN_OK;
            } // end valid bit invalid
            else
            {
                // if node is valid check node type
                // if we read something other than a Child node.. memory is colliding. return
                // stack -> parent nodes, heap-> child / data nodes.  stack will go into heap.. prevent this
                // Returns OK but sets address to null
                if(nodeTypeFromFlags(nodeFlags) == NODE_TYPE_PARENT)
                {
                    h->nextFreeChildNode = NODE_ADDR_NULL;
                    return RETURN_OK;
                } // check for node type
            }// end if valid
            
            if(nodeItr == 0)
            {
                break;  //due to using unsigned.. we must break out of the inner loop
            }
        } // end for each possible node
        
        if(pageItr == 0)
        {
            // should never happen because page 0 is reserved and parent
            // node collision should happen prior to making it to this case
            break;  // break out of outer loop
        }
    } // end for each page
    
    // we have visited the entire chip (should not happen?)
    // no free nodes found.. set to null
    h->nextFreeChildNode = NODE_ADDR_NULL;
    // Return OK.  Users responsibility to check nextFreeParentNode
    return RETURN_OK;
}

/**
 * Writes a child node to memory (next free via handle) (in alphabetical order).
 *   Scans for nextFreeChildNode after completion.  Modifies the node management handle
 * @param   h               The user allocated node management handle
 * @param   pAddr           The parent node address of the child
 * @param   p               The child node to write to memory (nextFreeChildNode)
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE createChildNode(mgmtHandle *h, uint16_t pAddr, cNode *c)
{
    return createChildTypeNode(h, pAddr, (void *)c, NODE_TYPE_CHILD, 0);
}

/**
 * Writes a child start of data node to memory (next free via handle) (in alphabetical order).
 *   Scans for nextFreeChildNode after completion.  Modifies the node management handle
 * @param   h               The user allocated node management handle
 * @param   pAddr           The parent node address of the child
 * @param   p               The child node to write to memory (nextFreeChildNode)
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE createChildStartOfDataNode(mgmtHandle *h, uint16_t pAddr, cNode *c, uint8_t dataNodeCount)
{
    return createChildTypeNode(h, pAddr, c, NODE_TYPE_CHILD_DATA, dataNodeCount);
}

/**
 * Reads a child or child start of data node from memory.
 * @param   h               The user allocated node management handle
 * @param   c               Storage for the node from memory
 * @param   childNodeAddress The address to read in memory
 * @return  success status
 */
RET_TYPE readChildNode(mgmtHandle *h, cNode *c, uint16_t childNodeAddress)
{
    RET_TYPE ret = RETURN_OK;
    ret = readDataFromFlash(pageNumberFromAddress(childNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(childNodeAddress), NODE_SIZE_CHILD, &(*c));
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    if((validBitFromFlags(c->flags) == NODE_VBIT_INVALID))
    {
        // node is invalid
        // clear local node.. return not ok
        invalidateChildNode(c);
        return RETURN_NOK;
    }
    return ret;
}

/**
 * Updates a child or child start of data node in memory. Handles alphabetical reorder of nodes.
 *   Scans for nextFreeChildNode after completion.  Modifies the node management handle
 * @param   h               The user allocated node management handle
 * @param   p               Parent Node of the Child Node
 * @param   c               Contents of node to update
 * @param   pAddr           The address to the parent node of the child
 * @param   cAddr           The address to the child node to update
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE updateChildNode(mgmtHandle *h, pNode *p, cNode *c, uint16_t pAddr, uint16_t cAddr)
{
        RET_TYPE ret = RETURN_OK;
        pNode *ip = &(h->parent);
        cNode *ic = &(h->child.child);
        
        // read the node at parentNodeAddress
        // userID check and valid Check performed in readParent
        ret = readParentNode(h, ip, pAddr);
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        ret = readChildNode(h, ic, cAddr);
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        // Do not allow the user to change linked list links, or change child link (will be done internally)
        // TODO -> disallow changing of CTR?
        if((p->nextChildAddress != ip->nextChildAddress) || (p->nextParentAddress != ip->nextParentAddress) || (p->prevParentAddress != ip->prevParentAddress) ||
           (c->nextChildAddress != ic->nextChildAddress) || (c->prevChildAddress != ic->prevChildAddress) || (c->dateCreated != ic->dateCreated))
        {
            return RETURN_NOK;
        }
        
        // nodes are identical.. do not write
        if(memcmp(&(*c), ic, NODE_SIZE_CHILD) == 0)
        {
            return RETURN_OK;
        }
        
        // reorder done on login.. 
        if(memcmp(&(c->login), &(ic->login), NODE_CHILD_SIZE_OF_LOGIN) == 0)
        {
            // service is identical just rewrite the node
            ret = writeDataToFlash(pageNumberFromAddress(cAddr), NODE_SIZE_PARENT * nodeNumberFromAddress(cAddr), NODE_SIZE_CHILD, &(*c));
            if(ret != RETURN_OK)
            {
                return ret;
            }
            
            // write is destructive.. read
            ret = readChildNode(h, &(*c), cAddr);
            if(ret != RETURN_OK)
            {
                return ret;
            }
        }
        else
        {            
            // delete node in memory
            ret = deleteChildNode(h, pAddr, cAddr, DELETE_POLICY_WRITE_ONES);
            if(ret != RETURN_OK)
            {
                return ret;
            }
            
            // create node in memory
            ret = createChildNode(h, pAddr, *(&c));
            if(ret != RETURN_OK)
            {
                return ret;
            }
        }
        return ret;
}

/**
 * Deletes a child node from memory. Handles reorder of nodes and update to parent if needed.
 * @param   h               The user allocated node management handle
 * @param   pAddr           The address of the parent of the child
 * @param   cAddr           The address of the child
 * @param   policy          How to handle the delete @ref deletePolicy
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE deleteChildNode(mgmtHandle *h, uint16_t pAddr, uint16_t cAddr, deletePolicy policy)
{
    // TODO REIMPLEMENT
    RET_TYPE ret = RETURN_OK;
    //pNode memNode;
    //pNode *ip = &memNode;
    pNode *ip = &(h->parent);
    //cNode cMemNode;
    //cNode *ic = &cMemNode;
    cNode *ic = &(h->child.child);
    uint16_t prevAddress;
    uint16_t nextAddress;
    
    // read parent node of child to delete
    ret = readParentNode(h, ip, pAddr);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    // parent valid bit checked in read
        
    if(userIdFromFlags(ip->flags) != h->currentUserId)
    {
        // cannot allow current user to modify node
        // node does not belong to user
        return RETURN_NOK;
    }
    
    // TODO - space permitting. verify cAddr belongs to parent
    
    // read child node to delete
    ret = readChildNode(h, ic, cAddr);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    // child valid bit checked in read

    // store previous and next node of node to be deleted
    prevAddress = ic->prevChildAddress;
    nextAddress = ic->nextChildAddress;
    
    // delete node (not using update due to 'delete' operation)
    if(policy == DELETE_POLICY_WRITE_NOTHING)
    {
        // set node as invalid.. update
        validBitToFlags(&(ic->flags), NODE_VBIT_INVALID);
        ret = writeDataToFlash(pageNumberFromAddress(cAddr), NODE_SIZE_PARENT * nodeNumberFromAddress(cAddr), NODE_SIZE_CHILD, ic);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    else if(policy == DELETE_POLICY_WRITE_ONES)
    {
        // memset parent node to all 0's.. set valid bit to invalid.. write
        memset(ic, DELETE_POLICY_WRITE_ONES, NODE_SIZE_CHILD);
        validBitToFlags(&(ic->flags), NODE_VBIT_INVALID);
        ret = writeDataToFlash(pageNumberFromAddress(cAddr), NODE_SIZE_PARENT * nodeNumberFromAddress(cAddr), NODE_SIZE_CHILD, ic);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    else if(policy == DELETE_POLICY_WRITE_ZEROS)
    {
        // memset parent node to all 1's.. set valid bit to invalid.. write
        memset(ic, DELETE_POLICY_WRITE_ZEROS, NODE_SIZE_CHILD);
        validBitToFlags(&(ic->flags), NODE_VBIT_INVALID);
        ret = writeDataToFlash(pageNumberFromAddress(cAddr), NODE_SIZE_PARENT * nodeNumberFromAddress(cAddr), NODE_SIZE_CHILD, ic);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    
    // set previousParentNode.nextParentAddress to this.nextParentAddress
    if(prevAddress != NODE_ADDR_NULL)
    {
        // read node
        ret = readChildNode(h, ic, prevAddress);
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        // set address
        ic->nextChildAddress = nextAddress;
        
        // update node
        ret = writeDataToFlash(pageNumberFromAddress(prevAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(prevAddress), NODE_SIZE_CHILD, ic);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }

    // set nextParentNode.prevParentNode to this.prevParentNode
    if(nextAddress != NODE_ADDR_NULL)
    {
        // read node
        ret = readChildNode(h, ic, nextAddress);
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        // set address
        ic->prevChildAddress = prevAddress;
        // update node
        ret = writeDataToFlash(pageNumberFromAddress(nextAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(nextAddress), NODE_SIZE_CHILD, ic);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    
    if(ip->nextChildAddress == cAddr)
    {
        // removed starting node. prev should be null
        // if nextAddress == NODE_ADDR _NULL.. we have no nodes left
        //     set starting parent to null (eg next)
        // if nextAddress != NODE_ADDR_NULL.. we have nodes left
        //     set starting parent to next
        // Long story short.. set parent to nextChildAddress to next always
        ip->nextChildAddress = nextAddress;
        //ret = updateParentNode(h, ip, pAddr);
        ret = writeDataToFlash(pageNumberFromAddress(pAddr), NODE_SIZE_PARENT * nodeNumberFromAddress(pAddr), NODE_SIZE_PARENT, ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    
    // greater.. closer to start of heap
    if(pageNumberFromAddress(cAddr) > pageNumberFromAddress(h->nextFreeChildNode))
    {
        // removed node page is less than next free page (closer page)
        // set next free node to recently removed node address
        h->nextFreeChildNode = cAddr;
    }
    else if(pageNumberFromAddress(cAddr) == pageNumberFromAddress(h->nextFreeChildNode))
    {
        // removed node is in the same page as next free
        // check node number
        if(nodeNumberFromAddress(cAddr) > nodeNumberFromAddress(h->nextFreeChildNode))
        {
            // node number is greater (closer to start of heap).. set next free
            h->nextFreeChildNode = cAddr;
        }
    }
    // else cAddr < h->nextFreeParentNode.. do nothing
    return RETURN_OK;
}

/**
 * Helper function.
 * Writes a child or child start of data node to memory (next free via handle) (in alphabetical order).
 *   Scans for nextFreeChildNode after completion.  Modifies the node management handle
 * @param   h               The user allocated node management handle
 * @param   pAddr           The parent node address of the child
 * @param   p               The child node to write to memory (nextFreeChildNode)
 * @return  success status
 * @note    Handles necessary doubly linked list management
 * @note    TODO. Implement pre-scan / allocation for child start of data nodes
 */
RET_TYPE createChildTypeNode(mgmtHandle *h, uint16_t pAddr, cNode *c, nodeType t, uint8_t dnr)
{
    RET_TYPE ret = RETURN_OK;
    uint16_t addr = NODE_ADDR_NULL;

    // storage for parent node
    //pNode memNode;
    //pNode *memNodePtr = &memNode;
    pNode *memNodePtr = &(h->parent);

    // storage for child Node
    //cNode cMemNode;
    cNode *cMemNodePtr = &(h->child.child);
    int8_t res = 0;

    if(((h->nextFreeChildNode) == NODE_ADDR_NULL) || pAddr == NODE_ADDR_NULL)
    {
        // no space remaining in flash or parent node address is null
        return RETURN_NOK;
    }

    // read childs assumed parent into memNodePtr (shared buffer)?
    ret = readParentNode(h, memNodePtr, pAddr);
    if(ret != RETURN_OK)
    {
        // error reading parent node
        return ret;
    }

    // verify uid
    if((h->currentUserId) != userIdFromFlags(memNodePtr->flags))
    {
        // cannot create child node stub on a parent node with a different user ID
        return RETURN_NOK;
    }

    // if node type child_data.. scan memory for allocated space
    // TODO. calculate if data size possible

    // set node type
    nodeTypeToFlags(&(c->flags), t);

    // set valid bit
    validBitToFlags(&(c->flags), NODE_VBIT_VALID);

    c->nextChildAddress = NODE_ADDR_NULL;
    c->prevChildAddress = NODE_ADDR_NULL;

    // if parent has no nodes. this node is the first node
    if(memNodePtr->nextChildAddress == NODE_ADDR_NULL)
    {
        // write child node to flash
        ret = writeReadDataFlash(h->nextFreeChildNode, NODE_SIZE_CHILD, c);
        /*
        ret = writeDataToFlash(pageNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
        if(ret != RETURN_OK)
        {
            return ret;
        }
    
        // read back from flash
        ret = readDataFromFlash(pageNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
        if(ret != RETURN_OK)
        {
            return ret;
        }
        */
    
        // set the next child address in the parent
        memNodePtr->nextChildAddress = h->nextFreeChildNode;
        ret = writeDataToFlash(pageNumberFromAddress(pAddr), NODE_SIZE_PARENT * nodeNumberFromAddress(pAddr), NODE_SIZE_PARENT, memNodePtr);
        if(ret != RETURN_OK)
        {
            #ifdef NODE_MGMT_CREATE_CHILD_NODE
            usbPrintf_P(PSTR("Error Updating Parent Node\n"));
            #endif
            // TODO handle this error more gracefully.. although should not occur
            return ret;
        }
    
        // set next free to null.. scan will happen at the end of the function
        h->nextFreeChildNode = NODE_ADDR_NULL;
    }
    else
    {
        // not the first node
    
        // get first node address (parent)
        addr = memNodePtr->nextChildAddress;
        while(addr != NODE_ADDR_NULL)
        {
            // read node
            #ifdef NODE_MGMT_CREATE_CHILD_NODE
            usbPrintf_P(PSTR("Read First Node\n"));
            #endif
            ret = readChildNode(h, cMemNodePtr, addr);
            if(ret != RETURN_OK)
            {
                return ret;
            }
        
            // compare nodes (alphabetically)
            res = memcmp(c->login, cMemNodePtr->login, NODE_CHILD_SIZE_OF_LOGIN);
            if(res > 0)
            {
                // to add child node comes after current node in memory.. go to next node
                if(cMemNodePtr->nextChildAddress == NODE_ADDR_NULL)
                {
                    // end of linked list. Set to write node prev and next addr's
                    c->nextChildAddress = NODE_ADDR_NULL;
                    c->prevChildAddress = addr; // current cMemNode Addr
                
                    // write new node to flash
                    ret = writeReadDataFlash(h->nextFreeChildNode, NODE_SIZE_CHILD, c);
                    /*
                    ret = writeDataToFlash(pageNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                
                    // read back from flash
                    ret = readDataFromFlash(pageNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                    */
                
                    // set previous last node to point to new node. write to flash
                    cMemNodePtr->nextChildAddress = h->nextFreeChildNode;
                    ret = writeDataToFlash(pageNumberFromAddress(addr), NODE_SIZE_PARENT * nodeNumberFromAddress(addr), NODE_SIZE_CHILD, cMemNodePtr);
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                
                    // read node from flash.. writes are destructive.
                    ret = readChildNode(h, &(*c), h->nextFreeChildNode);
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                
                    // set loop exit case
                    h->nextFreeChildNode = NODE_ADDR_NULL;
                    addr = NODE_ADDR_NULL;
                }
                else
                {
                    // loop and read next node
                    addr = cMemNodePtr->nextChildAddress;
                }
            }
            else if(res < 0)
            {
                // to add child node comes before current node in memory. Previous node is already not a memcmp match .. write node
                // set node to write next child to current node in mem, set prev child to current node in mems prev parent
                c->nextChildAddress = addr;
                c->prevChildAddress = cMemNodePtr->prevChildAddress;
                //writeAddr = h->nextFreeParentNode;
                // write new node to flash
                ret = writeReadDataFlash(h->nextFreeChildNode, NODE_SIZE_CHILD, c);
                /*
                ret = writeDataToFlash(pageNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
                if(ret != RETURN_OK)
                {
                    return ret;
                }
            
                // read back from flash (needed?)
                ret = readDataFromFlash(pageNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(h->nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
                if(ret != RETURN_OK)
                {
                    return ret;
                }
                */
            
                // read c->next from flash
                ret = readDataFromFlash(pageNumberFromAddress(c->nextChildAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(c->nextChildAddress), NODE_SIZE_CHILD, cMemNodePtr);
                if(ret != RETURN_OK)
                {
                    return ret;
                }
            
                // update current node in mem. set prev parent to address node to write was written to.
                cMemNodePtr->prevChildAddress = h->nextFreeChildNode;
                ret = writeDataToFlash(pageNumberFromAddress(c->nextChildAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(c->nextChildAddress), NODE_SIZE_CHILD, cMemNodePtr);
                if(ret != RETURN_OK)
                {
                    return ret;
                }
            
                if(c->prevChildAddress != NODE_ADDR_NULL)
                {
                    // read c->prev node
                    ret = readChildNode(h, cMemNodePtr, c->prevChildAddress);
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                
                    // update prev node to point next child to addr of node to write node
                    cMemNodePtr->nextChildAddress = h->nextFreeChildNode;
                    ret = writeDataToFlash(pageNumberFromAddress(c->prevChildAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(c->prevChildAddress), NODE_SIZE_CHILD, cMemNodePtr);
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                }
            
                if(addr == memNodePtr->nextChildAddress)
                {
                    // new node comes before current address and current address in first node.
                    // new node should be first node
                    memNodePtr->nextChildAddress = h->nextFreeChildNode;
                    ret = writeDataToFlash(pageNumberFromAddress(pAddr), NODE_SIZE_PARENT * nodeNumberFromAddress(pAddr), NODE_SIZE_PARENT, memNodePtr);
                    if(ret != RETURN_OK)
                    {
                        // TODO handle this error more gracefully.. although should not occur
                        return ret;
                    }
                }
            
                // read node from flash.. writes are destructive.
                ret = readChildNode(h, &(*c), h->nextFreeChildNode);
                if(ret != RETURN_OK)
                {
                    return ret;
                }
                // set handle nextFreeParent to null
                h->nextFreeChildNode = NODE_ADDR_NULL;
                addr = NODE_ADDR_NULL; // exit case
            }
            else
            {
                // services match
                // return nok. Same parent node
                return RETURN_NOK;
            } // end cmp results
        } // end while
    } // end if first parent

    // deterine last node slot
    if(NODE_PARENT_PER_PAGE == 4)
    {
        res = 2;
    }
    else
    {
        res = 6;
    }

    ret = scanNextFreeChildNode(h, constructAddress(PAGE_COUNT-1, res));
    if(ret != RETURN_OK)
    {
        return ret;
    }

    return RETURN_OK;

}

/**
 * Helper function.
 * Writes then reads from memory (avoids destroying memory buffers when writing to flash + saves code store)
 * @param   a               Theaddress of the node to write to memory
 * @param   s               The size of the node to write to memory
 * @param   d               The node buffer (pNode, cNode, dNode etc..)
 * @return  success status
 */
RET_TYPE writeReadDataFlash(uint16_t a, uint16_t s, void *d)
{
    RET_TYPE ret;
    // write data to flash
    ret = writeDataToFlash(pageNumberFromAddress(a), NODE_SIZE_PARENT * nodeNumberFromAddress(a), s, d);
    if(ret != RETURN_OK)
    {
        return ret;
    }

    // read back from flash
    ret = readDataFromFlash(pageNumberFromAddress(a), NODE_SIZE_PARENT * nodeNumberFromAddress(a), s, d);
    if(ret != RETURN_OK)
    {
        return ret;
    }

    return ret;
}