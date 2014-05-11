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
#include "usb_serial_hid.h"
#include <string.h>  // For memset
#include <stdint.h>
#include <stddef.h>


/* Flag Get/Set Helper Functions */
/*!  \fn       nodeTypeFromFlags(uint16_t flags)
*    \brief    Gets nodeType from flags  
*              NO ERROR CHECKING PERFORMED.
*    \param    flags  The flag field from a node
*    \return   Node Type Value (0 -> 3)
*/
uint8_t nodeTypeFromFlags(uint16_t flags)
{
    return (uint8_t)(((flags & NODE_F_TYPE_MASK) >>  NODE_F_TYPE_SHMT) & NODE_F_TYPE_MASK_FINAL);
}

/*!  \fn       nodeTypeToFlags(uint16_t *flags, uint8_t nodeType)
*    \brief    Sets nodeType in flags  
*              NO ERROR CHECKING PERFORMED.
*    \param    flags  The flag field from a node
*    \param    nodeType  The type of node
*/
void  nodeTypeToFlags(uint16_t *flags, uint8_t nodeType)
{
    *flags = (*flags & ~NODE_F_TYPE_MASK) | ((uint16_t)nodeType << NODE_F_TYPE_SHMT);
}

/*!  \fn       validBitFromFlags(uint16_t flags)
*    \brief    Gets validBit from flags
*              NO ERROR CHECKING PERFORMED.
*    \param    flags  The flag field from a node
*    \return   valid bit. (NODE_VBIT_VALID, NODE_VBIT_INVALID)
*/
uint8_t validBitFromFlags(uint16_t flags)
{
    return (uint8_t)(((flags & NODE_F_VALID_BIT_MASK) >> NODE_F_VALID_BIT_SHMT) & NODE_F_VALID_BIT_MASK_FINAL);
}

/*!  \fn       validBitToFlags(uint16_t *flags, uint8_t vb)
*    \brief    Sets validBit in flags    
*              NO ERROR CHECKING PERFORMED.
*    \param    flags  The flag field from a node
*    \param    vb  Valid bit state (NODE_VBIT_VALID, NODE_VBIT_INVALID)
*/
void validBitToFlags(uint16_t *flags, uint8_t vb)
{
    *flags = (*flags & (~NODE_F_VALID_BIT_MASK)) | ((uint16_t)vb << NODE_F_VALID_BIT_SHMT);
}

/*!  \fn       userIdFromFlags(uint16_t flags)
*    \brief    Gets userId from flags 
*              NO ERROR CHECKING PERFORMED.
*    \param    flags  The flag field from a node
*    \return   User ID Number (0 -> 15)
*/
uint8_t userIdFromFlags(uint16_t flags)
{
    return (uint8_t)(((flags & NODE_F_UID_MASK) >> NODE_F_UID_SHMT) & NODE_F_UID_MASK_FINAL);
}

/*!  \fn       userIdToFlags(uint16_t *flags, uint8_t vb)
*    \brief    Sets userId in flags   
*              NO ERROR CHECKING PERFORMED.
*    \param    flags  The flag field from a node
*    \param    uid User ID Number (0 -> 15)
*/
void userIdToFlags(uint16_t *flags, uint8_t uid)
{
    *flags = (*flags & (~NODE_F_UID_MASK)) | ((uint16_t)uid << NODE_F_UID_SHMT);
}

/*!  \fn       credentialTypeFromFlags(uint16_t flags)
*    \brief    Gets credentialType from flags  
*              NO ERROR CHECKING PERFORMED. Only use on parent nodes.
*    \param    flags  The flag field from a node
*    \return   Credential Type
*/
uint8_t credentialTypeFromFlags(uint16_t flags)
{
    return (uint8_t)(flags & NODE_F_CRED_TYPE_MASK);
}

/*!  \fn       credentialTypeToFlags(uint16_t *flags, uint8_t credType)
*    \brief    Sets credType in flags   
*              NO ERROR CHECKING PERFORMED.
*    \param    flags  The flag field from a node
*    \param    credType credential Type
*/
void credentialTypeToFlags(uint16_t *flags, uint8_t credType)
{
    *flags = (*flags & (~NODE_F_CRED_TYPE_MASK)) | ((uint16_t)credType);
}

/*!  \fn       dataNodeSequenceNumberfromFlags(uint16_t flags)
*    \brief    Gets the sequence number from flags.  
*              NO ERROR CHECKING PERFORMED. Only use on data nodes.
*    \param    flags  The flag field from a node
*    \return   Sequence Number (0 -> 255)
*/
uint8_t dataNodeSequenceNumberFromFlags(uint16_t flags)
{
    return (uint8_t)(flags & NODE_F_DATA_SEQ_NUM_MASK);
}

/*!  \fn       dataNodeSequenceNumberToFlags(uint16_t *flags, uint8_t sid)
*    \brief    Sets the sequence number in flags 
*              NO ERROR CHECKING PERFORMED. Only use on data nodes.
*    \param    flags  The flag field from a node
*    \param    sid    sequence id
*/
void dataNodeSequenceNumberToFlags(uint16_t *flags, uint8_t sid)
{
    *flags = (*flags & (~NODE_F_DATA_SEQ_NUM_MASK)) | ((uint16_t)sid);
}

/*!  \fn       pageNumberFromAddress(uint16_t addr)
*    \brief    Gets the page number from an address.  
*              NO ERROR CHECKING PERFORMED.
*    \param    addr  The address used for extraction
*    \return   Page Number (value varies per flash IC)
*/
uint16_t pageNumberFromAddress(uint16_t addr)
{
    return (addr >> NODE_ADDR_SHMT) & NODE_ADDR_PAGE_MASK;
}

/*!  \fn       nodeNumberFromAddress(uint16_t addr)
*    \brief    Gets the node number from an address.  
*              NO ERROR CHECKING PERFORMED.
*    \param    addr  The address used for extraction
*    \return   Node Number (value varies per flash IC)
*/
uint8_t nodeNumberFromAddress(uint16_t addr)
{
    return (uint8_t)(addr & NODE_ADDR_NODE_MASK);
}

/*!  \fn       constructAddress(uint16_t pageNumber, uint8_t nodeNumber)
*    \brief    Constructs a Node Address from a flash page number and node number
*              NO ERROR CHECKING PERFORMED.
*    \param    addr  The address used for extraction
*    \return   Address
*/
uint16_t constructAddress(uint16_t pageNumber, uint8_t nodeNumber)
{
    return ((pageNumber << NODE_ADDR_SHMT) | ((uint16_t)nodeNumber));
}


/*!  \fn       userProfileStartingOffset(uint8_t uid, uint16_t *page, uint16_t *pageOffset)
*    \brief    Obtains page and page offset for user profile 
*    \param    uid  The ID number to calculate page and page offset
*    \param    uid  The page number (pointer) set by this function.
*    \param    uid  The page offset (pointer) set by this function.
*    \return   Status
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

/*!  \fn       initNodeManagementHandle(mgmtHandle *h, uint8_t userIdNum)
*    \brief    Inits the Node Management Handle 
*    \param    h  A pointer to the user allocated management handle
*    \param    userIdNum  The id of the user to init the handle for
*    \return   Status
*/
RET_TYPE initNodeManagementHandle(mgmtHandle *h, uint8_t userIdNum)
{    
    RET_TYPE ret = RETURN_NOK;
    
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
    
    // TODOscan for next free child node 
    h->nextFreeChildNode = NODE_ADDR_NULL;
    
    return RETURN_OK;
}

/*!  \fn       setStartingParent(mgmtHandle *h, uint16_t parentAddress)
*    \brief    Sets the users starting parent node both in the handle and flash
*    \param    h  A pointer to the user allocated management handle
*    \param    userIdNum  The id of the user to init the handle for
*    \return   Status
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

/*!  \fn       readStartingParent(mgmtHandle *h, uint16_t *parentAddress)
*    \brief    Gets the users starting parent node both from the handle
*    \param    h  A pointer to the user allocated management handle
*    \param    parentAddress  A pointer to the location to store the starting parent address
*    \return   Status
*/
RET_TYPE readStartingParent(mgmtHandle *h, uint16_t *parentAddress)
{
    *parentAddress = (h->firstParentNode);
    return RETURN_OK;
}

/*!  \fn       setFav(mgmtHandle *h, uint8_t favId, uint16_t parentAddress, uint16_t childAddress)
*    \brief    Sets a user favorite in the user profile
*    \param    h  A pointer to the user allocated management handle
*    \param    favId  The id number of the fav record
*    \param    parentAddress  The parent node address of the fav
*    \param    childAddress   The child node address of the fav
*    \return   Status
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

/*!  \fn       readFav(mgmtHandle *h, uint8_t favId, uint16_t *parentAddress, uint16_t *childAddress)
*    \brief    Gets a user favorite in the user profile
*    \param    h  A pointer to the user allocated management handle
*    \param    favId  The id number of the fav record
*    \param    parentAddress  The parent node address of the fav
*    \param    childAddress   The child node address of the fav
*    \return   Status
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

/*!  \fn       scanNextFreeParentNode(mgmtHandle *h, uint16_t startingAddress)
*    \brief    Determines the next parent node location (next write location).
*              Sets the nextFreeParentNode value in the handle
*              If no free memory node NODE_ADDR_NULL is set
*    \param    h  The node management handle
*    \param    startingAddress  The address of the first node to examine
*    \return   Status
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
                // construct address with pageItr (page number) and nodeItr (node number)
                h->nextFreeParentNode = constructAddress(pageItr, nodeItr);
                //usbPrintf_P(PSTR("--FOUND FREE PARENT NODE (%u, %u)--\n"), pageItr, nodeItr);
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

/*!  \fn       createParentNode(mgmtHandle *h, pNode *p)
*    \brief    Writes a parent node to memory (next free via handle) (in alphabetical order)
*              Modifies the handle after write
*    \param    h  The node management handle
*    \param    p  The parent node to write to memory
*    \return   Status
*/
RET_TYPE createParentNode(mgmtHandle *h, pNode *p)
{
    RET_TYPE ret = RETURN_OK;
    uint16_t addr = NODE_ADDR_NULL;
    pNode memNode;
    pNode *memNodePtr = &memNode;
    int8_t res = 0;
    
    //uint16_t writeAddr;
    
    if((h->nextFreeParentNode) == NODE_ADDR_NULL)
    {
        // no space remaining in flash
        return RETURN_NOK;
    }
    
    if((h->currentUserId) != userIdFromFlags(p->flags))
    {
        // cannot create a node with a different user ID
        return RETURN_NOK;
    }
    
    // set node type
    nodeTypeToFlags(&(p->flags), NODE_TYPE_PARENT);
    
    // set valid bit
    validBitToFlags(&(p->flags), NODE_VBIT_VALID);
    
    //p->nextChildAddress = NODE_ADDR_NULL; // Removed for compatability for UpdateParentNode
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

/*!  \fn       readParentNode(mgmtHandle *h, pNode * p, uint16_t parentNodeAddress)
*    \brief    Reads a parent node from memory
*              If the node does not have a proper user id. p becomes invalid
*    \param    h  The node management handle
*    \param    p  Storage for the node from memory
*    \param    parentNodeAddress  The address to read in memory
*    \return   Status
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

/*!  \fn       updateParentNode(mgmtHandle *h, pNode *p, uint16_t parentNodeAddress)
*    \brief    Updates a parent node in memory
*              Handles Re-order
*    \param    h  The node management handle
*    \param    p  Contents of node to update
*    \param    parentNodeAddress  The address to update in memory
*    \return   Status
*/
RET_TYPE updateParentNode(mgmtHandle *h, pNode *p, uint16_t parentNodeAddress)
{
    RET_TYPE ret = RETURN_OK;
    pNode ip;
    
    // read the node at parentNodeAddress
    ret = readParentNode(h, &ip, parentNodeAddress);
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
    if(userIdFromFlags(ip.flags) != h->currentUserId)
    {
        // should hopefully be handled by readParentNode
        invalidateParentNode(&ip);
        invalidateParentNode(p);
        return RETURN_NOK;
    }
    
    // Do not allow the user to change linked list links, or change child link (will be done internally)
    if((p->nextChildAddress != ip.nextChildAddress) || (p->nextParentAddress != ip.nextParentAddress) || (p->prevParentAddress != ip.prevParentAddress))
    {
        return RETURN_NOK;
    }
    
    // nodes are identical.. do not write
    if(memcmp(&(*p), &ip, NODE_SIZE_PARENT) == 0)
    {        
        return RETURN_OK;
    }
    
    // only things allowed to change are the service and flags.
    if(memcmp(&(p->service), &(ip.service), NODE_PARENT_SIZE_OF_SERVICE) == 0)
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
        
        // nextChildAddress may be updated in p.. if ip contains nextChildAddress.. p will overwrite. but to delete node at address. nextChildAddress must be NULL.
        // bypass delete security
        if(ip.nextChildAddress != NODE_ADDR_NULL)
        {
            ip.nextChildAddress = NODE_ADDR_NULL; // bypass delete security measure
            // write node to memory
            ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, &ip);
            if(ret != RETURN_OK)
            {
                return ret;
            }
        }
        
        // delete node in memory
        ret = deleteParentNode(h, parentNodeAddress, DELETE_POLICY_WRITE_ONES);
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        // create node in memory
        ret = createParentNode(h, *(&p));
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    return ret; 
}

/*!  \fn       deleteParentNode(mgmtHandle *h, uint16_t parentNodeAddress, deletePolicy policy)
*    \brief    Deletes a parent node from memory.  This node CANNOT have any children
*    \param    h  The node management handle
*    \param    parentNodeAddress  The address to delete in memory
*    \param    policy  How to handle the delete @ref deletePolicy
*    \return   Status
*/
RET_TYPE deleteParentNode(mgmtHandle *h, uint16_t parentNodeAddress, deletePolicy policy)
{
    RET_TYPE ret = RETURN_OK;
    pNode ip;
    uint16_t prevAddress;
    uint16_t nextAddress;
    
    // read node to delete
    ret = readParentNode(h, &ip, parentNodeAddress);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    if(userIdFromFlags(ip.flags) != h->currentUserId)
    {
        // cannot allow current user to modify node
        // node does not belong to user
        return RETURN_NOK;
    }
    
    if(ip.nextChildAddress != NODE_ADDR_NULL)
    {
        // parent has children. cannot delete
        return RETURN_NOK;
    }
    
    if(validBitFromFlags(ip.flags) != NODE_VBIT_VALID)
    {
        // cannot allow operation on invalid node
        return RETURN_NOK;
    }
    
    // store previous and next node of node to be deleted
    prevAddress = ip.prevParentAddress;
    nextAddress = ip.nextParentAddress;
    
    // delete node (not using update due to 'delete' operation)
    if(policy == DELETE_POLICY_WRITE_NOTHING)
    {
        // set node as invalid.. update
        validBitToFlags(&(ip.flags), NODE_VBIT_INVALID);
        ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, &ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    else if(policy == DELETE_POLICY_WRITE_ONES)
    {
        // memset parent node to all 0's.. set valid bit to invalid.. write
        memset(&ip, DELETE_POLICY_WRITE_ONES, NODE_SIZE_PARENT);
        validBitToFlags(&(ip.flags), NODE_VBIT_INVALID); 
        ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, &ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    else if(policy == DELETE_POLICY_WRITE_ZEROS)
    {
        // memset parent node to all 1's.. set valid bit to invalid.. write
        memset(&ip, DELETE_POLICY_WRITE_ZEROS, NODE_SIZE_PARENT);
        validBitToFlags(&(ip.flags), NODE_VBIT_INVALID);
        ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, &ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    
    // set previousParentNode.nextParentAddress to this.nextParentAddress
    if(prevAddress != NODE_ADDR_NULL)
    {
        // read node
        ret = readParentNode(h, &ip, prevAddress);
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        // set address
        ip.nextParentAddress = nextAddress;
        
        // update node
        ret = writeDataToFlash(pageNumberFromAddress(prevAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(prevAddress), NODE_SIZE_PARENT, &ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }

    // set nextParentNode.prevParentNode to this.prevParentNode
    if(nextAddress != NODE_ADDR_NULL)
    {
        // read node
        ret = readParentNode(h, &ip, nextAddress);
        if(ret != RETURN_OK)
        {
            return ret;
        }
        
        // set address
        ip.prevParentAddress = prevAddress;
        // update node
        ret = writeDataToFlash(pageNumberFromAddress(nextAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(nextAddress), NODE_SIZE_PARENT, &ip);
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

/*!  \fn       invalidateParentNode(pNode *p)
*    \brief    Sets to contents of a parent node to null
*    \param    p  The parent node to invalidate
*    \return   Status
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