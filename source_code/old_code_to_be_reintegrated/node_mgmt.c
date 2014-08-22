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
*    Modified: 18/08/2014
*    By:       Mathieu Stephan
*/

#include "flash_mem.h"
#include "node_mgmt.h"
#include "defines.h"
#include "usb.h"
#include <string.h>  // For memset
#include <stdint.h>
#include <stddef.h>

// Current node management handle
mgmtHandle currentNodeMgmtHandle;

// Private Function Prototypes
static RET_TYPE createChildTypeNode(uint16_t pAddr, cNode *c, nodeType t, uint8_t dnr);
static void writeReadDataFlash(uint16_t a, uint16_t s, void *d);

// Critical error catching
void nodeMgmtCriticalErrorCallback(void)
{
    usbPutstr_P(PSTR("#NM\r\n"));
    while(1);
}

// Critical user permission / node validity error catching
void nodeMgmtPermissionValidityErrorCallback(void)
{
    usbPutstr_P(PSTR("#NMP\r\n"));
    while(1);    
}

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

void writeParentNodeDataBlockToFlash(uint16_t address, void* data)
{
    writeDataToFlash(pageNumberFromAddress(address), NODE_SIZE_PARENT * nodeNumberFromAddress(address), NODE_SIZE_PARENT, data);
}

void readParentNodeDataBlockFromFlash(uint16_t address, void* data)
{
    readDataFromFlash(pageNumberFromAddress(address), NODE_SIZE_PARENT * nodeNumberFromAddress(address), NODE_SIZE_PARENT, data);
}

void writeChildNodeDataBlockToFlash(uint16_t address, void* data)
{
    writeDataToFlash(pageNumberFromAddress(address), NODE_SIZE_PARENT * nodeNumberFromAddress(address), NODE_SIZE_CHILD, data);
}

void readChildNodeDataBlockFromFlash(uint16_t address, void* data)
{
    readDataFromFlash(pageNumberFromAddress(address), NODE_SIZE_PARENT * nodeNumberFromAddress(address), NODE_SIZE_CHILD, data);
}

/**
 * Obtains page and page offset for a given user id
 * @param   uid             The id of the user to perform that profile page and offset calculation (0 up to NODE_MAX_UID)
 * @param   page            The page containing the user profile
 * @param   pageOffset      The offset of the page that indicates the start of the user profile
 * @note    Calculation will take place even if the uid is not valid (no starting parent)
 * @note    uid must be in range
 */
void userProfileStartingOffset(uint8_t uid, uint16_t *page, uint16_t *pageOffset)
{
    if(uid >= NODE_MAX_UID)
    {
        nodeMgmtCriticalErrorCallback();
    }
    
    #if ((BYTES_PER_PAGE % USER_PROFILE_SIZE) != 0)
        #error "User profile size is not aligned with pages"
    #endif
    
    *page = uid/(BYTES_PER_PAGE/USER_PROFILE_SIZE);
    *pageOffset = ((uint16_t)uid % (BYTES_PER_PAGE/USER_PROFILE_SIZE))*USER_PROFILE_SIZE;
}

/**
 * Formats the user profile flash memory of user uid.
 * @param   uid             The id of the user to format profile memory
 * @note    Only formats the users starting parent node address and favorites 
 */
void formatUserProfileMemory(uint8_t uid)
{
    uint8_t buf[USER_PROFILE_SIZE];
    uint16_t pageNumber;
    uint16_t pageOffset;
    
    if(uid >= NODE_MAX_UID)
    {
        nodeMgmtCriticalErrorCallback();
    }
    
    // Set buffer to all 0's. Assuming NODE_ADDR_NULL = 0x0000.
    memset(buf, 0, USER_PROFILE_SIZE);
    
    // obtain user profile offset.
    // write buf (0's) to clear out user memory. Buffer is destroyed.. we no longer need it
    userProfileStartingOffset(uid, &pageNumber, &pageOffset);
    writeDataToFlash(pageNumber, pageOffset, USER_PROFILE_SIZE, buf);
}

/*! \fn     getCurrentUserID(void)
*   \brief  Get the current user ID
*   \return The user ID
*/
uint8_t getCurrentUserID(void)
{
    return currentNodeMgmtHandle.currentUserId;
}

/*! \fn     deleteCurrentUserFromFlash(void)
*   \brief  Delete user data from flash
*/
void deleteCurrentUserFromFlash(void)
{
    uint16_t next_parent_addr = currentNodeMgmtHandle.firstParentNode;
    uint16_t next_child_addr;
    uint16_t temp_address;
    pNode temp_pnode;
    cNode temp_cnode;
    
    // Delete user profile memory
    formatUserProfileMemory(currentNodeMgmtHandle.currentUserId);
    
    // Then browse through all the credentials to delete them
    while (next_parent_addr != NODE_ADDR_NULL)
    {
        // Read current parent node
        readParentNode(&temp_pnode, next_parent_addr);
        
        // Read his first child
        next_child_addr = temp_pnode.nextChildAddress;
        
        // Browse through all children
        while (next_child_addr != NODE_ADDR_NULL)
        {
            // Read child node
            readChildNode(&temp_cnode, next_child_addr);
            
            // Store the next child address in temp
            temp_address = temp_cnode.nextChildAddress;
            
            // Delete child data block
            memset(&temp_cnode, 0xFF, NODE_SIZE_CHILD);
            writeChildNodeDataBlockToFlash(next_child_addr, &temp_cnode);
            
            // Set correct next address
            next_child_addr = temp_address;
        }
        
        // Store the next parent address in temp
        temp_address = temp_pnode.nextParentAddress;
        
        // Delete parent data block
        memset(&temp_pnode, 0xFF, NODE_SIZE_PARENT);
        writeParentNodeDataBlockToFlash(next_parent_addr, &temp_pnode);
        
        // Set correct next address
        next_parent_addr = temp_address;
    }
}

/**
 * Initializes the Node Management Handle.
 *   Check userIdNum in range,  reads users profile to get the starting parent node, scans memory for the next free parent and child nodes.
 * @param   h               The user allocated node management handle
 * @param   userIdNum       The user id to initialize the handle for (0->NODE_MAX_UID)
 */
void initNodeManagementHandle(uint8_t userIdNum)
{        
    if(userIdNum >= NODE_MAX_UID)
    {
        nodeMgmtPermissionValidityErrorCallback();
    }
             
    currentNodeMgmtHandle.flags = 0;
    currentNodeMgmtHandle.currentUserId = userIdNum;
    currentNodeMgmtHandle.firstParentNode = getStartingParentAddress();
    
    // scan for next free parent and child nodes
    scanNodeUsage();
}

/**
 * Sets the users starting parent node both in the handle and user profile memory portion of flash
 * @param   h               The user allocated node management handle
 * @param   parentAddress   The constructed address of the users starting parent node (alphabetically) 
 */
void setStartingParent(uint16_t parentAddress)
{
    uint16_t userProfilePage = 0;
    uint16_t userProfilePageOffset = 0;
    
    if (((parentAddress & NODE_ADDR_NODE_MASK) > NODE_PARENT_PER_PAGE) || ((parentAddress >> NODE_ADDR_SHMT) > PAGE_COUNT))
    {
        nodeMgmtCriticalErrorCallback();
    }
    
    // no error checking.. user id was validated at init
    userProfileStartingOffset(currentNodeMgmtHandle.currentUserId, &userProfilePage, &userProfilePageOffset);
    
    // update handle
    currentNodeMgmtHandle.firstParentNode = parentAddress;
    
    // Write parentaddress in the user profile page
    writeDataToFlash(userProfilePage, userProfilePageOffset, 2, &parentAddress);
}

/**
 * Gets the users starting parent node from the user profile memory portion of flash
 * @return  The address
 */
uint16_t getStartingParentAddress(void)
{
    uint16_t userProfilePageOffset;
    uint16_t userProfilePage;
    uint16_t temp_address;
    
    // no error checking.. user id was validated at init. Get profile Address
    userProfileStartingOffset(currentNodeMgmtHandle.currentUserId, &userProfilePage, &userProfilePageOffset);
    
    // restore parentAddress
    readDataFromFlash(userProfilePage, userProfilePageOffset, 2, &temp_address);
    
    return temp_address;
}

/**
 * Sets a user favorite in the user profile
 * @param   h               The user allocated node management handle
 * @param   favId           The id number of the fav record
 * @param   parentAddress   The parent node address of the fav
 * @param   childAddress    The child node address of the fav
 */
void setFav(uint8_t favId, uint16_t parentAddress, uint16_t childAddress)
{
    uint16_t page;
    uint16_t offset;
    uint16_t addrs[2];
    
    if(favId >= USER_MAX_FAV)
    {
        nodeMgmtCriticalErrorCallback();
    }
    
    addrs[0] = parentAddress;
    addrs[1] = childAddress;
    
    // calculate user profile start    
    userProfileStartingOffset(currentNodeMgmtHandle.currentUserId, &page, &offset);
    
    // add to offset
    offset += (favId * 4) + 2;  // each fav is 4 bytes. +2 for starting parent node offset
    
    // write to flash
    writeDataToFlash(page, offset, 4, (void *)addrs);
}

/**
 * Reads a user favorite from the user profile
 * @param   h               The user allocated node management handle
 * @param   favId           The id number of the fav record
 * @param   parentAddress   The parent node address of the fav
 * @param   childAddress    The child node address of the fav
 */
void readFav(uint8_t favId, uint16_t *parentAddress, uint16_t *childAddress)
{
    uint16_t page;
    uint16_t offset;
    uint16_t addrs[2];
    
    if(favId >= USER_MAX_FAV)
    {
        nodeMgmtCriticalErrorCallback();
    }
    
    // calculate user profile start
    userProfileStartingOffset(currentNodeMgmtHandle.currentUserId, &page, &offset);
    
    // add to offset
    offset += (favId * 4) + 2;  // each fav is 4 bytes. +2 for starting parent node offset
    
    // write to flash
    readDataFromFlash(page, offset, 4, (void *)addrs);
    
    // return values to user
    *parentAddress = addrs[0];
    *childAddress = addrs[1];
}

/**
 * Sets the users base CTR in the user profile flash memory
 * @param   h               The user allocated node management handle
 * @param   buf             The buffer containing CTR
 */
void setProfileCtr(void *buf)
{
    uint16_t page;
    uint16_t offset;

    // calculate user profile start
    userProfileStartingOffset(currentNodeMgmtHandle.currentUserId, &page, &offset);

    offset += USER_PROFILE_SIZE-USER_RES_CTR; // User CTR is at the end

    writeDataToFlash(page, offset, USER_CTR_SIZE, buf);
}

/**
 * Reads the users base CTR from the user profile flash memory
 * @param   h               The user allocated node management handle
 * @param   buf             The buffer to store the read CTR
 */
void readProfileCtr(void *buf)
{
    uint16_t page;
    uint16_t offset;

    // calculate user profile start
    userProfileStartingOffset(currentNodeMgmtHandle.currentUserId, &page, &offset);

    offset += USER_PROFILE_SIZE-USER_RES_CTR; // does not include ctr val.. this will set the correct offset

    readDataFromFlash(page, offset, USER_CTR_SIZE, buf);
}

/**
 * Writes a parent node to memory (next free via handle) (in alphabetical order).
 *   Scans for nextFreeParentNode after completion.  Modifies the node management handle
 * @param   h               The user allocated node management handle
 * @param   p               The parent node to write to memory (nextFreeParentNode)
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE createParentNode(pNode *p)
{
    uint16_t addr = NODE_ADDR_NULL;
    //pNode memNode;
    pNode *memNodePtr = &(currentNodeMgmtHandle.parent);
    int8_t res = 0;
    
    //uint16_t writeAddr;
    
    if((currentNodeMgmtHandle.nextFreeParentNode) == NODE_ADDR_NULL)
    {
        // no space remaining in flash
        return RETURN_NOK;
    }
    
    userIdToFlags(&(p->flags), currentNodeMgmtHandle.currentUserId);
    /*
    if((currentNodeMgmtHandle.currentUserId) != userIdFromFlags(p->flags))
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
    if(currentNodeMgmtHandle.firstParentNode == NODE_ADDR_NULL)
    {
        // write parent node to flash (destructive)
        writeParentNodeDataBlockToFlash(currentNodeMgmtHandle.nextFreeParentNode, p);
        
        // read back from flash
        readParentNodeDataBlockFromFlash(currentNodeMgmtHandle.nextFreeParentNode, p);
        
        // set the starting node address
        setStartingParent(currentNodeMgmtHandle.nextFreeParentNode);
        // set next free to null.. scan will happen at the end of the function
        currentNodeMgmtHandle.nextFreeParentNode = NODE_ADDR_NULL;
    }
    else
    {
        // not the first node
        
        // get first node address
        addr = currentNodeMgmtHandle.firstParentNode;
        while(addr != NODE_ADDR_NULL)
        {
            // read node
            readParentNode(memNodePtr, addr);
            
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
                    writeParentNodeDataBlockToFlash(currentNodeMgmtHandle.nextFreeParentNode, p);
                    
                    // read back from flash
                    readParentNodeDataBlockFromFlash(currentNodeMgmtHandle.nextFreeParentNode, p);
                    
                    // set previous last node to point to new node. write to flash
                    memNodePtr->nextParentAddress = currentNodeMgmtHandle.nextFreeParentNode;
                    writeParentNodeDataBlockToFlash(addr, memNodePtr);
                    
                    // read node from flash.. writes are destructive.
                    readParentNode(&(*p), currentNodeMgmtHandle.nextFreeParentNode);
                                        
                    // set loop exit case
                    currentNodeMgmtHandle.nextFreeParentNode = NODE_ADDR_NULL; 
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
                //writeAddr = currentNodeMgmtHandle.nextFreeParentNode;
                // write new node to flash
                writeParentNodeDataBlockToFlash(currentNodeMgmtHandle.nextFreeParentNode, p);
                
                // read back from flash (needed?)
                readParentNodeDataBlockFromFlash(currentNodeMgmtHandle.nextFreeParentNode, p);
                
                // read p->next from flash
                readParentNodeDataBlockFromFlash(p->nextParentAddress, memNodePtr);
                
                // update current node in mem. set prev parent to address node to write was written to.
                memNodePtr->prevParentAddress = currentNodeMgmtHandle.nextFreeParentNode;
                writeParentNodeDataBlockToFlash(p->nextParentAddress, memNodePtr);
                
                if(p->prevParentAddress != NODE_ADDR_NULL)
                {
                    // read p->prev node
                    readParentNode(memNodePtr, p->prevParentAddress);
                
                    // update prev node to point next parent to addr of node to write node
                    memNodePtr->nextParentAddress = currentNodeMgmtHandle.nextFreeParentNode;
                    writeParentNodeDataBlockToFlash(p->prevParentAddress, memNodePtr);
                }                
                
                if(addr == currentNodeMgmtHandle.firstParentNode)
                {
                    // new node comes before current address and current address in first node.
                    // new node should be first node
                    setStartingParent(currentNodeMgmtHandle.nextFreeParentNode);
                }
                
                // read node from flash.. writes are destructive.
                readParentNode(&(*p), currentNodeMgmtHandle.nextFreeParentNode);
                
                // set handle nextFreeParent to null
                currentNodeMgmtHandle.nextFreeParentNode = NODE_ADDR_NULL; 
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
    
    scanNodeUsage();
    
   return RETURN_OK;
}

/**
 * Reads a parent node from memory. If the node does not have a proper user id, p should be considered undefined
 * @param   h               The user allocated node management handle
 * @param   p               Storage for the node from memory
 * @param   parentNodeAddress The address to read in memory
 */
void readParentNode(pNode* p, uint16_t parentNodeAddress)
{
    readParentNodeDataBlockFromFlash(parentNodeAddress, p);
    
    if((currentNodeMgmtHandle.currentUserId != userIdFromFlags(p->flags)) || (validBitFromFlags(p->flags) == NODE_VBIT_INVALID))
    {
        // if handle user id != id from node or node is invalid
        // clear local node.. return not ok
        nodeMgmtPermissionValidityErrorCallback();
    }
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
RET_TYPE updateParentNode(pNode *p, uint16_t parentNodeAddress)
{
    RET_TYPE ret = RETURN_OK;
    pNode *ip = &(currentNodeMgmtHandle.parent);
    uint16_t addr;
    uint16_t newParentAddr;
    
    // read the node at parentNodeAddress, checks the user id as well!
    readParentNode(ip, parentNodeAddress);
    
    // Force user id flags on the passed node
    userIdToFlags(&(p->flags), currentNodeMgmtHandle.currentUserId);
    
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
        writeParentNodeDataBlockToFlash(parentNodeAddress, p);
        
        // write is destructive.. read
        readParentNode(&(*p), parentNodeAddress);
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
            writeReadDataFlash(parentNodeAddress, NODE_SIZE_PARENT, ip);
        }
        
        // delete node in memory handles doubly linked list management
        ret = deleteParentNode(parentNodeAddress);
        if(ret != RETURN_OK)
        {
            return ret;
        }

        // backup nextParentAddr (my not be at previous location due to how memory mgmt handles memory)
        newParentAddr = currentNodeMgmtHandle.nextFreeParentNode;
        
        // create node in memory (new node p) handles doubly linked list management
        ret = createParentNode(*(&p));
        if(ret != RETURN_OK)
        {
            return ret;
        }

        // node should be at newParentAddr

        // read written node (into internal buffer)
        readParentNode(ip, newParentAddr);

        // restore addr backup (modify internal buffer)
        ip->nextChildAddress = addr;
        // write node to memory (from internal buffer)
        writeReadDataFlash(newParentAddr, NODE_SIZE_PARENT, ip);
        //ret = writeDataToFlash(pageNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT * nodeNumberFromAddress(parentNodeAddress), NODE_SIZE_PARENT, ip);
    }
    return ret; 
}

/**
 * Deletes a parent node from memory.  This node CANNOT have any children
 * @param   h               The user allocated node management handle
 * @param   parentNodeAddress The address to read in memory
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE deleteParentNode(uint16_t parentNodeAddress)
{
    pNode *ip = &(currentNodeMgmtHandle.parent);
    uint16_t prevAddress;
    uint16_t nextAddress;
    
    // read node to delete
    readParentNode(ip, parentNodeAddress);
    
    if(userIdFromFlags(ip->flags) != currentNodeMgmtHandle.currentUserId)
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
    
    // memset parent node to all 0's.. set valid bit to invalid.. write
    memset(ip, DELETE_POLICY_WRITE_ONES, NODE_SIZE_PARENT);
    validBitToFlags(&(ip->flags), NODE_VBIT_INVALID); 
    writeParentNodeDataBlockToFlash(parentNodeAddress, ip);
    
    // set previousParentNode.nextParentAddress to this.nextParentAddress
    if(prevAddress != NODE_ADDR_NULL)
    {
        // read node
         readParentNode(ip, prevAddress);
        
        // set address
        ip->nextParentAddress = nextAddress;
        
        // update node
        writeParentNodeDataBlockToFlash(prevAddress, ip);
    }

    // set nextParentNode.prevParentNode to this.prevParentNode
    if(nextAddress != NODE_ADDR_NULL)
    {
        // read node
        readParentNode(ip, nextAddress);
        
        // set address
        ip->prevParentAddress = prevAddress;
        // update node
        writeParentNodeDataBlockToFlash(nextAddress, ip);
    }
    
    if(currentNodeMgmtHandle.firstParentNode == parentNodeAddress)
    {
        // removed starting node. prev should be null
        // if nextAddress == NODE_ADDR _NULL.. we have no nodes left
        //     set starting parent to null (eg next)
        // if nextAddress != NODE_ADDR_NULL.. we have nodes left
        //     set starting parent to next
        // Long story short.. set starting parent to next always
        setStartingParent(nextAddress);
    }
    
    if(pageNumberFromAddress(parentNodeAddress) < pageNumberFromAddress(currentNodeMgmtHandle.nextFreeParentNode))
    {
        // removed node page is less than next free page (closer page)
        // set next free node to recently removed node address
        currentNodeMgmtHandle.nextFreeParentNode = parentNodeAddress;
    }
    else if(pageNumberFromAddress(parentNodeAddress) == pageNumberFromAddress(currentNodeMgmtHandle.nextFreeParentNode))
    {
        // removed node is in the same page as next free
        // check node number
        if(nodeNumberFromAddress(parentNodeAddress) < nodeNumberFromAddress(currentNodeMgmtHandle.nextFreeParentNode))
        {
            // node number is lesser.. set next free
            currentNodeMgmtHandle.nextFreeParentNode = parentNodeAddress;
        }            
    }
    // else parentNodeAddress > currentNodeMgmtHandle.nextFreeParentNode.. do nothing
    
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
 * Writes a child node to memory (next free via handle) (in alphabetical order).
 *   Scans for nextFreeChildNode after completion.  Modifies the node management handle
 * @param   h               The user allocated node management handle
 * @param   pAddr           The parent node address of the child
 * @param   p               The child node to write to memory (nextFreeChildNode)
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE createChildNode(uint16_t pAddr, cNode *c)
{
    return createChildTypeNode(pAddr, (void *)c, NODE_TYPE_CHILD, 0);
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
RET_TYPE createChildStartOfDataNode(uint16_t pAddr, cNode *c, uint8_t dataNodeCount)
{
    return createChildTypeNode(pAddr, c, NODE_TYPE_CHILD_DATA, dataNodeCount);
}

/**
 * Reads a child or child start of data node from memory.
 * @param   h               The user allocated node management handle
 * @param   c               Storage for the node from memory
 * @param   childNodeAddress The address to read in memory
 * @return  success status
 */
void readChildNode(cNode *c, uint16_t childNodeAddress)
{
    readChildNodeDataBlockFromFlash(childNodeAddress, c);
    
    if((validBitFromFlags(c->flags) == NODE_VBIT_INVALID))
    {
        // node is invalid
        // clear local node.. return not ok
        nodeMgmtPermissionValidityErrorCallback();
    }
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
RET_TYPE updateChildNode(pNode *p, cNode *c, uint16_t pAddr, uint16_t cAddr)
{
        RET_TYPE ret = RETURN_OK;
        pNode *ip = &(currentNodeMgmtHandle.parent);
        cNode *ic = &(currentNodeMgmtHandle.child.child);
        
        // read the node at parentNodeAddress
        // userID check and valid Check performed in readParent
        readParentNode(ip, pAddr);        
        readChildNode(ic, cAddr);
        
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
            writeChildNodeDataBlockToFlash(cAddr, c);
            
            // write is destructive.. read
            readChildNode(&(*c), cAddr);
        }
        else
        {            
            // delete node in memory
            ret = deleteChildNode(pAddr, cAddr);
            if(ret != RETURN_OK)
            {
                return ret;
            }
            
            // create node in memory
            ret = createChildNode(pAddr, *(&c));
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
RET_TYPE deleteChildNode(uint16_t pAddr, uint16_t cAddr)
{
    // TODO REIMPLEMENT
    //pNode memNode;
    //pNode *ip = &memNode;
    pNode *ip = &(currentNodeMgmtHandle.parent);
    //cNode cMemNode;
    //cNode *ic = &cMemNode;
    cNode *ic = &(currentNodeMgmtHandle.child.child);
    uint16_t prevAddress;
    uint16_t nextAddress;
    
    // read parent node of child to delete
    readParentNode(ip, pAddr);
    
    // parent valid bit checked in read
        
    if(userIdFromFlags(ip->flags) != currentNodeMgmtHandle.currentUserId)
    {
        // cannot allow current user to modify node
        // node does not belong to user
        return RETURN_NOK;
    }
    
    // TODO - space permitting. verify cAddr belongs to parent
    
    // read child node to delete
    readChildNode(ic, cAddr);
    
    // child valid bit checked in read

    // store previous and next node of node to be deleted
    prevAddress = ic->prevChildAddress;
    nextAddress = ic->nextChildAddress;
    
    // memset parent node to all 0's.. set valid bit to invalid.. write
    memset(ic, DELETE_POLICY_WRITE_ONES, NODE_SIZE_CHILD);
    validBitToFlags(&(ic->flags), NODE_VBIT_INVALID);
    writeChildNodeDataBlockToFlash(cAddr, ic);
    
    // set previousParentNode.nextParentAddress to this.nextParentAddress
    if(prevAddress != NODE_ADDR_NULL)
    {
        // read node
        readChildNode(ic, prevAddress);
        
        // set address
        ic->nextChildAddress = nextAddress;
        
        // update node
        writeChildNodeDataBlockToFlash(prevAddress, ic);
    }

    // set nextParentNode.prevParentNode to this.prevParentNode
    if(nextAddress != NODE_ADDR_NULL)
    {
        // read node
        readChildNode(ic, nextAddress);
        
        // set address
        ic->prevChildAddress = prevAddress;
        // update node
        writeChildNodeDataBlockToFlash(nextAddress, ic);
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
        writeParentNodeDataBlockToFlash(pAddr, ip);
    }
    
    // greater.. closer to start of heap
    if(pageNumberFromAddress(cAddr) > pageNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode))
    {
        // removed node page is less than next free page (closer page)
        // set next free node to recently removed node address
        currentNodeMgmtHandle.nextFreeChildNode = cAddr;
    }
    else if(pageNumberFromAddress(cAddr) == pageNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode))
    {
        // removed node is in the same page as next free
        // check node number
        if(nodeNumberFromAddress(cAddr) > nodeNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode))
        {
            // node number is greater (closer to start of heap).. set next free
            currentNodeMgmtHandle.nextFreeChildNode = cAddr;
        }
    }
    // else cAddr < currentNodeMgmtHandle.nextFreeParentNode.. do nothing
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
RET_TYPE createChildTypeNode(uint16_t pAddr, cNode *c, nodeType t, uint8_t dnr)
{
    uint16_t addr = NODE_ADDR_NULL;

    // storage for parent node
    //pNode memNode;
    //pNode *memNodePtr = &memNode;
    pNode *memNodePtr = &(currentNodeMgmtHandle.parent);

    // storage for child Node
    //cNode cMemNode;
    cNode *cMemNodePtr = &(currentNodeMgmtHandle.child.child);
    int8_t res = 0;

    if(((currentNodeMgmtHandle.nextFreeChildNode) == NODE_ADDR_NULL) || pAddr == NODE_ADDR_NULL)
    {
        // no space remaining in flash or parent node address is null
        return RETURN_NOK;
    }

    // read childs assumed parent into memNodePtr (shared buffer)?
    readParentNode(memNodePtr, pAddr);

    // verify uid
    if((currentNodeMgmtHandle.currentUserId) != userIdFromFlags(memNodePtr->flags))
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
        writeReadDataFlash(currentNodeMgmtHandle.nextFreeChildNode, NODE_SIZE_CHILD, c);
        /*
        ret = writeDataToFlash(pageNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
        if(ret != RETURN_OK)
        {
            return ret;
        }
    
        // read back from flash
        ret = readDataFromFlash(pageNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
        if(ret != RETURN_OK)
        {
            return ret;
        }
        */
    
        // set the next child address in the parent
        memNodePtr->nextChildAddress = currentNodeMgmtHandle.nextFreeChildNode;
        writeParentNodeDataBlockToFlash(pAddr, memNodePtr);
    
        // set next free to null.. scan will happen at the end of the function
        currentNodeMgmtHandle.nextFreeChildNode = NODE_ADDR_NULL;
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
            readChildNode(cMemNodePtr, addr);
        
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
                    writeReadDataFlash(currentNodeMgmtHandle.nextFreeChildNode, NODE_SIZE_CHILD, c);
                    /*
                    ret = writeDataToFlash(pageNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                
                    // read back from flash
                    ret = readDataFromFlash(pageNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
                    if(ret != RETURN_OK)
                    {
                        return ret;
                    }
                    */
                
                    // set previous last node to point to new node. write to flash
                    cMemNodePtr->nextChildAddress = currentNodeMgmtHandle.nextFreeChildNode;
                    writeChildNodeDataBlockToFlash(addr, cMemNodePtr);
                
                    // read node from flash.. writes are destructive.
                    readChildNode(&(*c), currentNodeMgmtHandle.nextFreeChildNode);
                
                    // set loop exit case
                    currentNodeMgmtHandle.nextFreeChildNode = NODE_ADDR_NULL;
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
                //writeAddr = currentNodeMgmtHandle.nextFreeParentNode;
                // write new node to flash
                writeReadDataFlash(currentNodeMgmtHandle.nextFreeChildNode, NODE_SIZE_CHILD, c);
                /*
                ret = writeDataToFlash(pageNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
                if(ret != RETURN_OK)
                {
                    return ret;
                }
            
                // read back from flash (needed?)
                ret = readDataFromFlash(pageNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_PARENT * nodeNumberFromAddress(currentNodeMgmtHandle.nextFreeChildNode), NODE_SIZE_CHILD, &(*c));
                if(ret != RETURN_OK)
                {
                    return ret;
                }
                */
            
                // read c->next from flash
                readChildNodeDataBlockFromFlash(c->nextChildAddress, cMemNodePtr);
            
                // update current node in mem. set prev parent to address node to write was written to.
                cMemNodePtr->prevChildAddress = currentNodeMgmtHandle.nextFreeChildNode;
                writeChildNodeDataBlockToFlash(c->nextChildAddress, cMemNodePtr);
            
                if(c->prevChildAddress != NODE_ADDR_NULL)
                {
                    // read c->prev node
                    readChildNode(cMemNodePtr, c->prevChildAddress);
                
                    // update prev node to point next child to addr of node to write node
                    cMemNodePtr->nextChildAddress = currentNodeMgmtHandle.nextFreeChildNode;
                    writeChildNodeDataBlockToFlash(c->prevChildAddress, cMemNodePtr);
                }
            
                if(addr == memNodePtr->nextChildAddress)
                {
                    // new node comes before current address and current address in first node.
                    // new node should be first node
                    memNodePtr->nextChildAddress = currentNodeMgmtHandle.nextFreeChildNode;
                    writeParentNodeDataBlockToFlash(pAddr, memNodePtr);
                }
            
                // read node from flash.. writes are destructive.
                readChildNode(&(*c), currentNodeMgmtHandle.nextFreeChildNode);
                // set handle nextFreeParent to null
                currentNodeMgmtHandle.nextFreeChildNode = NODE_ADDR_NULL;
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

    scanNodeUsage();

    return RETURN_OK;

}

void scanNodeUsage(void)
{
	uint16_t nodeFlags = 0xFFFF;
	uint16_t pageItr;
	uint8_t nodeItr;
	
	uint16_t lastSeenParent = NODE_ADDR_NULL;  // stores parent boundary
	uint16_t nextFreeParent = NODE_ADDR_NULL;  // stores nextFreeParent (first free node seen Free)
	uint16_t firstSeenChild = NODE_ADDR_NULL;  // stores child boundary
	uint16_t nextFreeChild  = NODE_ADDR_NULL;  // stores nextFreeChild (last free child seen)

	// for each page
	for(pageItr = PAGE_PER_SECTOR; pageItr < PAGE_COUNT; pageItr++)
	{
		// for each possible parent node in the page (changes per flash chip)
		for(nodeItr = 0; nodeItr < NODE_PARENT_PER_PAGE; nodeItr++)
		{
			// read node flags (2 bytes - fixed size)
			readDataFromFlash(pageItr, NODE_SIZE_PARENT*nodeItr, 2, &nodeFlags);
			
			if(validBitFromFlags(nodeFlags) == NODE_VBIT_VALID)
			{
				// node is valid
				if(firstSeenChild == NODE_ADDR_NULL)
				{
					if(nodeTypeFromFlags(nodeFlags) == NODE_TYPE_PARENT)
					{
						// found a valid parent node. Mark it (this branch will hit a lot)
						lastSeenParent = constructAddress(pageItr, nodeItr);
					}
					else
					{
						// found the first valid child node (branch will hit once)
						firstSeenChild = constructAddress(pageItr, nodeItr);
					}
				}
			}
			else
			{
				// node is invalid
				if(nextFreeParent == NODE_ADDR_NULL)
				{
					// found an invlaid (free) node either in pNode mem or after pNode mem
					nextFreeParent = constructAddress(pageItr, nodeItr);
				}
				else if(firstSeenChild != NODE_ADDR_NULL)
				{
					// in child node memory.  nodes are already aligned
					nextFreeChild = constructAddress(pageItr, nodeItr);
				}
			}
			
			if(firstSeenChild != NODE_ADDR_NULL)
			{
				nodeItr++; // account for 2 node units per child node
			}
		} // end for node
	} // end for page
	
	if(nextFreeChild == NODE_ADDR_NULL && nextFreeParent != NODE_ADDR_NULL)
	{
		// did not find a hole in child node memory and memory is not full
		// check if we have enough memory
		// set next free child to DMZ.
		// if nextFreeChild page is same page as last seen parent.. set to null
		
		if(firstSeenChild == NODE_ADDR_NULL)
		{
			nodeItr = NODE_CHILD_MAX_NODE;
			
			nextFreeChild = constructAddress(PAGE_COUNT-1, nodeItr);
		}
		else
		{
			pageItr = pageNumberFromAddress(firstSeenChild);
			nodeItr =  nodeNumberFromAddress(firstSeenChild);
			if(nodeItr == 0)
			{
				pageItr--;
				nodeItr = NODE_CHILD_MAX_NODE;
			}
			else
			{
				nodeItr = nodeItr - 2; // 2 units per child page
			}
			
			nextFreeChild = constructAddress(pageItr, nodeItr);
			
			if(pageItr == pageNumberFromAddress(lastSeenParent))
			{
				// nextfreeChild and lastSeenParent are on the same page
				nextFreeChild = NODE_ADDR_NULL;
			}
		}
		
	}
	
	// set handle vars (TODO move into above)
	currentNodeMgmtHandle.nextFreeParentNode = nextFreeParent;
	currentNodeMgmtHandle.nextFreeChildNode = nextFreeChild;
	currentNodeMgmtHandle.lastSeenParent = lastSeenParent;
	currentNodeMgmtHandle.fistSeenChild = firstSeenChild;
}

/**
 * Helper function.
 * Writes then reads from memory (avoids destroying memory buffers when writing to flash + saves code store)
 * @param   a               Theaddress of the node to write to memory
 * @param   s               The size of the node to write to memory
 * @param   d               The node buffer (pNode, cNode, dNode etc..)
 */
void writeReadDataFlash(uint16_t a, uint16_t s, void *d)
{    // write data to flash
    writeDataToFlash(pageNumberFromAddress(a), NODE_SIZE_PARENT * nodeNumberFromAddress(a), s, d);

    // read back from flash
    readDataFromFlash(pageNumberFromAddress(a), NODE_SIZE_PARENT * nodeNumberFromAddress(a), s, d);
}