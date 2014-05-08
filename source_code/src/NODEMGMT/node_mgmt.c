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
               NO ERROR CHECKING PERFORMED
*    \param    flags  The flag field from a node
*    \return   Node Type Value (0 -> 3)
*/
uint8_t nodeTypeFromFlags(uint16_t flags)
{
    return (uint8_t)(((flags & NODE_F_TYPE_MASK) >>  NODE_F_TYPE_SHMT) & NODE_F_TYPE_MASK_FINAL);
}

/*!  \fn       nodeTypeToFlags(uint16_t *flags, uint8_t nodeType)
*    \brief    Sets nodeType in flags  
               NO ERROR CHECKING PERFORMED
*    \param    flags  The flag field from a node
*    \param    nodeType  The type of node
*/
void  nodeTypeToFlags(uint16_t *flags, uint8_t nodeType)
{
    *flags = (*flags & ~NODE_F_TYPE_MASK) | ((uint16_t)nodeType << NODE_F_TYPE_SHMT);
}

/*!  \fn       validBitFromFlags(uint16_t flags)
*    \brief    Gets validBit from flags
               NO ERROR CHECKING PERFORMED
*    \param    flags  The flag field from a node
*    \return   valid bit. (NODE_VBIT_VALID, NODE_VBIT_INVALID)
*/
uint8_t validBitFromFlags(uint16_t flags)
{
    return (uint8_t)(((flags & NODE_F_VALID_BIT_MASK) >> NODE_F_VALID_BIT_SHMT) & NODE_F_VALID_BIT_MASK_FINAL);
}

/*!  \fn       validBitToFlags(uint16_t *flags, uint8_t vb)
*    \brief    Sets validBit in flags    
               NO ERROR CHECKING PERFORMED
*    \param    flags  The flag field from a node
*    \param    vb  Valid bit state (NODE_VBIT_VALID, NODE_VBIT_INVALID)
*/
void validBitToFlags(uint16_t *flags, uint8_t vb)
{
    *flags = (*flags & (~NODE_F_VALID_BIT_MASK)) | ((uint16_t)vb << NODE_F_VALID_BIT_SHMT);
}

/*!  \fn       userIdFromFlags(uint16_t flags)
*    \brief    Gets userId from flags 
               NO ERROR CHECKING PERFORMED
*    \param    flags  The flag field from a node
*    \return   User ID Number (0 -> 15)
*/
uint8_t userIdFromFlags(uint16_t flags)
{
    return (uint8_t)(((flags & NODE_F_UID_MASK) >> NODE_F_UID_SHMT) & NODE_F_UID_MASK_FINAL);
}

/*!  \fn       userIdToFlags(uint16_t *flags, uint8_t vb)
*    \brief    Sets userId in flags   
               NO ERROR CHECKING PERFORMED
*    \param    flags  The flag field from a node
*    \param    uid User ID Number (0 -> 15)
*/
void userIdToFlags(uint16_t *flags, uint8_t uid)
{
    *flags = (*flags & (~NODE_F_UID_MASK)) | ((uint16_t)uid << NODE_F_UID_SHMT);
}

/*!  \fn       credentialTypeFromFlags(uint16_t flags)
*    \brief    Gets credentialType from flags  
               NO ERROR CHECKING PERFORMED. Only use on parent nodes.
*    \param    flags  The flag field from a node
*    \return   Credential Type
*/
uint8_t credentialTypeFromFlags(uint16_t flags)
{
    return (uint8_t)(flags & NODE_F_CRED_TYPE_MASK);
}

/*!  \fn       credentialTypeToFlags(uint16_t *flags, uint8_t credType)
*    \brief    Sets credType in flags   
               NO ERROR CHECKING PERFORMED
*    \param    flags  The flag field from a node
*    \param    credType credential Type
*/
void credentialTypeToFlags(uint16_t *flags, uint8_t credType)
{
    *flags = (*flags & (~NODE_F_CRED_TYPE_MASK)) | ((uint16_t)credType);
}

/*!  \fn       dataNodeSequenceNumberfromFlags(uint16_t flags)
*    \brief    Gets the sequence number from flags.  
               NO ERROR CHECKING PERFORMED. Only use on data nodes.
*    \param    flags  The flag field from a node
*    \return   Sequence Number (0 -> 255)
*/
uint8_t dataNodeSequenceNumberFromFlags(uint16_t flags)
{
    return (uint8_t)(flags & NODE_F_DATA_SEQ_NUM_MASK);
}

/*!  \fn       dataNodeSequenceNumberToFlags(uint16_t *flags, uint8_t sid)
*    \brief    Sets the sequence number in flags 
               NO ERROR CHECKING PERFORMED. Only use on data nodes.
*    \param    flags  The flag field from a node
*    \param    sid    sequence id
*/
void dataNodeSequenceNumberToFlags(uint16_t *flags, uint8_t sid)
{
    *flags = (*flags & (~NODE_F_DATA_SEQ_NUM_MASK)) | ((uint16_t)sid);
}

/*!  \fn       pageNumberFromAddress(uint16_t addr)
*    \brief    Gets the page number from an address.  
               NO ERROR CHECKING PERFORMED.
*    \param    addr  The address used for extraction
*    \return   Page Number (value varies per flash IC)
*/
uint16_t pageNumberFromAddress(uint16_t addr)
{
    return (addr >> NODE_ADDR_SHMT) & NODE_ADDR_PAGE_MASK;
}

/*!  \fn       nodeNumberFromAddress(uint16_t addr)
*    \brief    Gets the node number from an address.  
               NO ERROR CHECKING PERFORMED.
*    \param    addr  The address used for extraction
*    \return   Node Number (value varies per flash IC)
*/
uint8_t nodeNumberFromAddress(uint16_t addr)
{
    return (uint8_t)(addr & NODE_ADDR_NODE_MASK);
}

/*!  \fn       constructAddress(uint16_t pageNumber, uint8_t nodeNumber)
*    \brief    Constructs a Node Address from a flash page number and node number
               NO ERROR CHECKING PERFORMED.
*    \param    addr  The address used for extraction
*    \return   Address
*/
uint16_t constructAddress(uint16_t pageNumber, uint8_t nodeNumber)
{
    return ((pageNumber << NODE_ADDR_SHMT) | ((uint16_t)nodeNumber));
}



RET_TYPE userProfileStartingOffset(uint8_t uid, uint16_t *page, uint16_t *pageOffset)
{
    if(uid >= NODE_MAX_UID)
    {
        return RETURN_NOK;
    }
    
    uint16_t offset = uid * USER_PROFILE_SIZE;
    
    *page = (uint16_t)((offset/BYTES_PER_PAGE) - ((offset/BYTES_PER_PAGE) % 1));
    *pageOffset = (uint16_t)(offset % BYTES_PER_PAGE);
    
    return RETURN_OK;
}

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
    
    /*
    uint16_t userProfilePage = 0;
    uint16_t userProfilePageOffset = 0;
    // error checking already performed
    userProfileStartingOffset(userIdNum, &userProfilePage, &userProfilePageOffset);
    // read starting parent address
    ret = readDataFromFlash(userProfilePage, userProfilePageOffset, 2, &(h->firstParentNode));
    */
    
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

RET_TYPE readStartingParent(mgmtHandle *h, uint16_t *parentAddress)
{
    *parentAddress = (h->firstParentNode);
    return RETURN_OK;
}

RET_TYPE setFav(mgmtHandle *h, uint8_t favId, uint16_t parentAddress, uint16_t childAddress)
{
    RET_TYPE ret = RETURN_OK;
    uint16_t page;
    uint16_t offset;
    uint16_t addrs[2];
    
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


RET_TYPE readFav(mgmtHandle *h, uint8_t favId, uint16_t *parentAddress, uint16_t *childAddress)
{
    RET_TYPE ret = RETURN_OK;
    uint16_t page;
    uint16_t offset;
    uint16_t addrs[2];
    
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
               Sets the nextFreeParentNode value in the handle
               If no free memory node NODE_ADDR_NULL is set
               NO ERROR CHECKING PERFORMED.
*    \param    h  The node management handle
*    \param    startingAddress  The address of the first node to examine
*    \return   
*/
RET_TYPE scanNextFreeParentNode(mgmtHandle *h, uint16_t startingAddress)
{
    uint16_t nodeFlags = 0xFFFF;
    
    uint16_t pageItr = 0;
    uint8_t nodeItr = 0;
    RET_TYPE ret = RETURN_OK;
    
    //usbPrintf_P(PSTR("1\n"));
    // for each page
    for(pageItr = pageNumberFromAddress(startingAddress); pageItr < PAGE_COUNT; pageItr++)
    {
        // for each possible parent node in the page (changes per flash chip)
        for(nodeItr = nodeNumberFromAddress(startingAddress); nodeItr < NODE_PARENT_PER_PAGE; nodeItr++)
        {
            usbPrintf_P(PSTR("2: %u, %u\n"), pageItr, nodeItr);
            // read node flags
            // 2 bytes - fixed size
            ret = readDataFromFlash(pageItr, NODE_SIZE_PARENT*nodeItr, 2, &nodeFlags);
            if(ret != RETURN_OK)
            {
                usbPrintf_P(PSTR("3\n"));
                return ret;
            }
            
            // process node flags
            // match criteria - valid bit is invalid
            if(validBitFromFlags(nodeFlags) == NODE_VBIT_INVALID)
            {
                // next free parent node found.
                // construct address with pageItr (page number) and nodeItr (node number)
                h->nextFreeParentNode = constructAddress(pageItr, nodeItr);
                usbPrintf_P(PSTR("4\n"));
                // return early
                return RETURN_OK;
            } // end valid bit invalid
            else 
            {
                // if node is valid check node type
                // if we read something other than a parent node.. memory is colliding. return
                // stack -> parent nodes, heap-> child / data nodes.  stack will go into heap.. prevent this
                // Returns OK but sets address to null
                usbPrintf_P(PSTR("5\n"));
                if(nodeTypeFromFlags(nodeFlags) != NODE_TYPE_PARENT)
                {
                    usbPrintf_P(PSTR("6\n"));
                    h->nextFreeParentNode = NODE_ADDR_NULL;
                    return RETURN_OK;
                } // check for node type
            }// end if valid
        } // end for each possible node
    } // end for each page
    
    usbPrintf_P(PSTR("7\n"));
    
    // we have visited the entire chip (should not happen?)
    // no free nodes found.. set to null
    h->nextFreeParentNode = NODE_ADDR_NULL;
    // Return OK.  Users responsibility to check nextFreeParentNode
    return RETURN_OK;
}

RET_TYPE createParentNode(mgmtHandle *h, pNode *p)
{
    RET_TYPE ret = RETURN_OK;
    uint16_t addr = NODE_ADDR_NULL;
    pNode memNode;
    pNode *memNodePtr = &memNode;
    int8_t res = 0;
    
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
    
    p->nextChildAddress = NODE_ADDR_NULL;
    p->nextParentAddress = NODE_ADDR_NULL;
    p->prevParentAddress = NODE_ADDR_NULL;
    
    // if user has no nodes. this node is the first node
    if(h->firstParentNode == NODE_ADDR_NULL)
    {
        usbPrintf_P(PSTR("First\n"));
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
                usbPrintf_P(PSTR("After\n"));
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
                    
                    // set loop exit case
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
                usbPrintf_P(PSTR("Before\n"));
                // to add parent node comes before current node in memory. Previous node is already not a memcmp match .. write node
                
                // set node to write next parent to current node in mem, set prev parent to current node in mems prev parent
                p->nextParentAddress = addr;
                p->prevParentAddress = memNodePtr->prevParentAddress;
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
                
                // update current node in mem. set prev parent to address node to write was written to.
                memNodePtr->prevParentAddress = h->nextFreeParentNode;
                ret = writeDataToFlash(pageNumberFromAddress(addr), NODE_SIZE_PARENT * nodeNumberFromAddress(addr), NODE_SIZE_PARENT, memNodePtr);
                if(ret != RETURN_OK)
                {
                    return ret;
                }
                
                // read prev node (to node to write)
                ret = readParentNode(h, memNodePtr, p->prevParentAddress);
                if(ret != RETURN_OK)
                {
                    return ret;
                }
                
                // update prev node to point next parent to addr of node to write node
                memNodePtr->nextParentAddress = h->nextFreeParentNode;
                ret = writeDataToFlash(pageNumberFromAddress(addr), NODE_SIZE_PARENT * nodeNumberFromAddress(addr), NODE_SIZE_PARENT, memNodePtr);
                if(ret != RETURN_OK)
                {
                    return ret;
                }
                
                // set handle nextFreeParent to null
                h->nextFreeParentNode = NODE_ADDR_NULL; // exit case
            }
            else
            {
                usbPrintf_P(PSTR("Equal\n"));
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

RET_TYPE readParentNode(mgmtHandle *h, pNode * p, uint16_t parentNodeAddress)
{
    RET_TYPE ret = RETURN_OK;
    //usbPrintf_P(PSTR("Reading node\n"));
    ret = readDataFromFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT*nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, &(*p));
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    //usbPrintf_P(PSTR("Checking\n"));
    if((h->currentUserId != userIdFromFlags(p->flags)) || (validBitFromFlags(p->flags) == NODE_VBIT_INVALID))
    {
        // if handle user id != id from node or node is invalid
        // clear local node.. return not ok
        //usbPrintf_P(PSTR("Invalidating\n"));
        invalidateParentNode(p);
        return RETURN_NOK;
    }
    //usbPrintf_P(PSTR("AOK\n"));
    return ret;
}

RET_TYPE updateParentNode(mgmtHandle *h, pNode *p, uint16_t parentNodeAddress)
{
    RET_TYPE ret = RETURN_OK;
    pNode ip;
    
    ret = readParentNode(h, &ip, parentNodeAddress);
    //ret = readDataFromFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT*nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, &ip);
    if(ret != RETURN_OK)
    {
        return ret;
    }
    
    if(memcmp(p, &ip, NODE_SIZE_PARENT)==0)
    {
        // nodes match nothing to update
        return RETURN_OK;
    }
    
    if(userIdFromFlags(p->flags) != h->currentUserId || userIdFromFlags(ip.flags) != h->currentUserId)
    {
        // cannot allow current user to modify node
        return RETURN_NOK;
    }
    
    if(validBitFromFlags(ip.flags) != NODE_VBIT_VALID)
    {
        // cannot allow operation on invalid node
        return RETURN_NOK;
    }
    
    if(memcmp(&(ip.service), &(p->service), NODE_PARENT_SIZE_OF_SERVICE) != 0)
    {
        // disallow updating service name
        // TODO remove this node.. create new node (removing restriction)
    }
    
    
    return ret; 
}

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
    
    if(validBitFromFlags(ip.flags) != NODE_VBIT_VALID)
    {
        // cannot allow operation on invalid node
        return RETURN_NOK;
    }
    
    // store previous and next node of node to be deleted
    prevAddress = ip.prevParentAddress;
    nextAddress = ip.nextParentAddress;
    
    // TODO Check for children, check for favorites
    
    // delete node
    if(policy == deletePolicyWriteNothing)
    {
        // set node as invalid.. update
        validBitToFlags(&(ip.flags), NODE_VBIT_INVALID);
        ret = updateParentNode(h, &ip, parentNodeAddress);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    else if(policy == deletePolicyWriteOnes)
    {
        // memset parent node to all 0's.. set valid bit to invalid.. write
        memset(&ip, deletePolicyWriteOnes, NODE_SIZE_PARENT);
        validBitToFlags(&(ip.flags), NODE_VBIT_INVALID); 
        // cannot use updateParentNode. currently will not allow this operation
        ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT*nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, &ip);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    else if(policy == deletePolicyWriteZeros)
    {
        // memset parent node to all 1's.. set valid bit to invalid.. write
        memset(&ip, deletePolicyWriteZeros, NODE_SIZE_PARENT);
        validBitToFlags(&(ip.flags), NODE_VBIT_INVALID);
        // cannot use updateParentNode. currently will not allow this operation
        ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT*nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, &ip);
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
        ret = updateParentNode(h, &ip, prevAddress);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    else
    {
        // removed node was starting parent node
        setStartingParent(h, nextAddress);
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
        ret = updateParentNode(h, &ip, nextAddress);
        if(ret != RETURN_OK)
        {
            return ret;
        }
    }
    
    if(pageNumberFromAddress(parentNodeAddress) < pageNumberFromAddress(h->nextFreeParentNode))
    {
        if(nodeNumberFromAddress(parentNodeAddress) < nodeNumberFromAddress(h->nextFreeParentNode))
        {
            h->nextFreeParentNode = parentNodeAddress;
        }
        // else current address in handle is lesser. Next create will happen there.
    }
    
    return RETURN_OK;
}

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