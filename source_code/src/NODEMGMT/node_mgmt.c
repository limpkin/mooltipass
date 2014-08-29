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
/*!  \file     node_mgmt.c
*    \brief    Mooltipass Node Management Library
*    Created:  03/4/2014
*    Author:   Michael Neiderhauser
*    Modified: 18/08/2014
*    By:       Mathieu Stephan
*/
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "flash_mem.h"
#include "node_mgmt.h"
#include "defines.h"
#include "usb.h"

// Current node management handle
mgmtHandle currentNodeMgmtHandle;


/*! \fn     nodeMgmtCriticalErrorCallback(void)
*   \brief  Critical error catching
*/
void nodeMgmtCriticalErrorCallback(void)
{
    usbPutstr(PSTR("#NM"));
    while(1);
}

/*! \fn     nodeMgmtPermissionValidityErrorCallback(void)
*   \brief  Node read permission and validity error catching
*/
void nodeMgmtPermissionValidityErrorCallback(void)
{
    usbPutstr(PSTR("#NMP"));
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

/*! \fn     writeNodeDataBlockToFlash(uint16_t address, void* data)
*   \brief  Write a node data block to flash
*   \param  address Where to write
*   \param  data    Pointer to the data
*/
void writeNodeDataBlockToFlash(uint16_t address, void* data)
{
    writeDataToFlash(pageNumberFromAddress(address), NODE_SIZE * nodeNumberFromAddress(address), NODE_SIZE, data);
}

/*! \fn     readNodeDataBlockFromFlash(uint16_t address, void* data)
*   \brief  Read a node data block to flash
*   \param  address Where to read
*   \param  data    Pointer to the data
*/
void readNodeDataBlockFromFlash(uint16_t address, void* data)
{
    readDataFromFlash(pageNumberFromAddress(address), NODE_SIZE * nodeNumberFromAddress(address), NODE_SIZE, data);
}

/*! \fn     eraseNodeDataBlockToFlash(uint16_t address)
*   \brief  Erase a node data block to flash
*   \param  address Where to erase
*/
void eraseNodeDataBlockToFlash(uint16_t address)
{
    uint8_t data[NODE_SIZE];
    
    // Set data to 0xFF
    memset(data, 0xFF, NODE_SIZE);
    writeDataToFlash(pageNumberFromAddress(address), NODE_SIZE * nodeNumberFromAddress(address), NODE_SIZE, data);
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
    uint16_t temp_page, temp_offset;
    uint8_t buf[USER_PROFILE_SIZE];
    
    if(uid >= NODE_MAX_UID)
    {
        nodeMgmtCriticalErrorCallback();
    }
    
    // Set buffer to all 0's. Assuming NODE_ADDR_NULL = 0x0000.
    memset(buf, 0, USER_PROFILE_SIZE);
    userProfileStartingOffset(uid, &temp_page, &temp_offset);
    writeDataToFlash(temp_page, temp_offset, USER_PROFILE_SIZE, buf);
}

/*! \fn     getCurrentUserID(void)
*   \brief  Get the current user ID
*   \return The user ID
*/
uint8_t getCurrentUserID(void)
{
    return currentNodeMgmtHandle.currentUserId;
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
            
    // fill current user id, first parent node address, user profile page & offset 
    userProfileStartingOffset(userIdNum, &currentNodeMgmtHandle.pageUserProfile, &currentNodeMgmtHandle.offsetUserProfile);
    currentNodeMgmtHandle.firstParentNode = getStartingParentAddress();
    currentNodeMgmtHandle.currentUserId = userIdNum;
    currentNodeMgmtHandle.flags = 0;
    
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
    // update handle
    currentNodeMgmtHandle.firstParentNode = parentAddress;
    
    // Write parentaddress in the user profile page
    writeDataToFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile, 2, &parentAddress);
}

/**
 * Gets the users starting parent node from the user profile memory portion of flash
 * @return  The address
 */
uint16_t getStartingParentAddress(void)
{
    uint16_t temp_address;
    
    // restore parentAddress
    readDataFromFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile, 2, &temp_address);    
    
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
    uint16_t addrs[2] = {parentAddress, childAddress};
    
    if(favId >= USER_MAX_FAV)
    {
        nodeMgmtCriticalErrorCallback();
    }
    
    // write to flash, each fav is 4 bytes. +2 for starting parent node offset
    writeDataToFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile + (favId * USER_FAV_SIZE) + USER_START_NODE_SIZE, USER_FAV_SIZE, (void *)addrs);
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
    uint16_t temp_uint;
    uint16_t addrs[2];
    
    if(favId >= USER_MAX_FAV)
    {
        nodeMgmtCriticalErrorCallback();
    }
    
    // Read from flash
    readDataFromFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile + (favId * USER_FAV_SIZE) + USER_START_NODE_SIZE, USER_FAV_SIZE, (void *)addrs);
    
    // return values to user
    *parentAddress = addrs[0];
    *childAddress = addrs[1];
    
    // Check valid bit flag
    readDataFromFlash(pageNumberFromAddress(*childAddress), NODE_SIZE * nodeNumberFromAddress(*childAddress), 2, &temp_uint);    
    if(validBitFromFlags(temp_uint) == NODE_VBIT_INVALID)
    {
        // Delete fav and return node_addr_null
        setFav(favId, NODE_ADDR_NULL, NODE_ADDR_NULL);
        *parentAddress = NODE_ADDR_NULL;
        *childAddress = NODE_ADDR_NULL;
    }
}

/**
 * Sets the users base CTR in the user profile flash memory
 * @param   h               The user allocated node management handle
 * @param   buf             The buffer containing CTR
 */
void setProfileCtr(void *buf)
{    
    // User CTR is at the end
    writeDataToFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile + USER_PROFILE_SIZE - USER_RES_CTR, USER_CTR_SIZE, buf);
}

/**
 * Reads the users base CTR from the user profile flash memory
 * @param   h               The user allocated node management handle
 * @param   buf             The buffer to store the read CTR
 */
void readProfileCtr(void *buf)
{
    // User CTR is at the end
    readDataFromFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile + USER_PROFILE_SIZE - USER_RES_CTR, USER_CTR_SIZE, buf);
}

/**
 * Reads a node from memory. If the node does not have a proper user id, g should be considered undefined
 * @param   g               Storage for the node from memory
 * @param   nodeAddress     The address to read in memory
 */
void readNode(gNode* g, uint16_t nodeAddress)
{
    readNodeDataBlockFromFlash(nodeAddress, g);
    
    if((currentNodeMgmtHandle.currentUserId != userIdFromFlags(g->flags)) || (validBitFromFlags(g->flags) == NODE_VBIT_INVALID))
    {
        // if handle user id != id from node or node is invalid
        // clear local node.. return not ok
        nodeMgmtPermissionValidityErrorCallback();
    }    
}

/**
 * Reads a parent node from memory. If the node does not have a proper user id, p should be considered undefined
 * @param   p               Storage for the node from memory
 * @param   parentNodeAddress The address to read in memory
 */
void readParentNode(pNode* p, uint16_t parentNodeAddress)
{
    readNode((gNode*)p, parentNodeAddress);
}

/**
 * Reads a child or child start of data node from memory.
 * @param   c               Storage for the node from memory
 * @param   childNodeAddress The address to read in memory
 */
void readChildNode(cNode *c, uint16_t childNodeAddress)
{
    readNode((gNode*)c, childNodeAddress);
}

/**
 * Writes a parent node to memory (next free via handle) (in alphabetical order).
 * @param   p               The parent node to write to memory (nextFreeParentNode)
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE createParentNode(pNode* p)
{
    uint16_t temp_address;
    RET_TYPE temprettype;
    
    // This is particular to parent nodes...
    p->nextChildAddress = NODE_ADDR_NULL;
    nodeTypeToFlags(&(p->flags), NODE_TYPE_PARENT);
    
    // Call createGenericNode to add a node
    temprettype = createGenericNode((gNode*)p, currentNodeMgmtHandle.firstParentNode, &temp_address, PNODE_COMPARISON_FIELD_OFFSET, NODE_PARENT_SIZE_OF_SERVICE);
    
    // If the return is ok & we changed the first node address
    if ((temprettype == RETURN_OK) && (currentNodeMgmtHandle.firstParentNode != temp_address))
    {
        setStartingParent(temp_address);
    }
    
    return temprettype;
}

/**
 * Writes a child node to memory (next free via handle) (in alphabetical order).
 * @param   pAddr           The parent node address of the child
 * @param   p               The child node to write to memory (nextFreeChildNode)
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE createChildNode(uint16_t pAddr, cNode *c)
{
    pNode* tempPNodePointer = (pNode*)&(currentNodeMgmtHandle.tempgNode);
    uint16_t childFirstAddress, temp_address;
    RET_TYPE temprettype;
    
    // Set node type to child
    nodeTypeToFlags(&(c->flags), NODE_TYPE_CHILD);
    
    // Read parent to get the first child address
    readNode((gNode*)tempPNodePointer, pAddr);
    childFirstAddress = tempPNodePointer->nextChildAddress;
    
    // Call createGenericNode to add a node
    temprettype = createGenericNode((gNode*)c, childFirstAddress, &temp_address, CNODE_COMPARISON_FIELD_OFFSET, NODE_CHILD_SIZE_OF_LOGIN);
    
    // If the return is ok & we changed the first child address
    if ((temprettype == RETURN_OK) && (childFirstAddress != temp_address))
    {
       readNode((gNode*)tempPNodePointer, pAddr);
       tempPNodePointer->nextChildAddress = temp_address;
       writeNodeDataBlockToFlash(pAddr, tempPNodePointer);
    }
    
    return temprettype;
}   

/**
 * Writes a generic node to memory (next free via handle) (in alphabetical order).
 * @param   p                   The parent node to write to memory (nextFreeParentNode)
 * @param   firstNodeAddress    Address of the first node of its kind
 * @param   newFirstNodeAddress If the firstNodeAddress changed, this var will store the new value
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE createGenericNode(gNode* g, uint16_t firstNodeAddress, uint16_t* newFirstNodeAddress, uint8_t comparisonFieldOffset, uint8_t comparisonFieldLength)
{
    gNode* memNodePtr = &(currentNodeMgmtHandle.tempgNode);
    uint16_t addr = NODE_ADDR_NULL;
    int8_t res = 0;
    
    // Set newFirstNodeAddress to firstNodeAddress by default
    *newFirstNodeAddress = firstNodeAddress;
    
    // Check space in flash
    if (currentNodeMgmtHandle.nextFreeNode == NODE_ADDR_NULL)
    {
        return RETURN_NOK;
    }
    
    // Set correct user id to the node
    userIdToFlags(&(g->flags), currentNodeMgmtHandle.currentUserId);
    
    // set valid bit
    validBitToFlags(&(g->flags), NODE_VBIT_VALID);

    // clear next/prev address
    g->prevAddress = NODE_ADDR_NULL;
    g->nextAddress = NODE_ADDR_NULL;
    
    // if user has no nodes. this node is the first node
    if(firstNodeAddress == NODE_ADDR_NULL)
    {
        // write parent node to flash (destructive)
        writeNodeDataBlockToFlash(currentNodeMgmtHandle.nextFreeNode, g);
        
        // read back from flash
        readNodeDataBlockFromFlash(currentNodeMgmtHandle.nextFreeNode, g);
        
        // set new first node address
        *newFirstNodeAddress = currentNodeMgmtHandle.nextFreeNode;
    }
    else
    {        
        // set first node address
        addr = firstNodeAddress;
        while(addr != NODE_ADDR_NULL)
        {
            // read node
            readNode(memNodePtr, addr);
            
            // compare nodes (alphabetically)
            res = strncmp((char*)g+comparisonFieldOffset, (char*)memNodePtr+comparisonFieldOffset, comparisonFieldLength);
            if(res > 0)
            {
                // to add parent node comes after current node in memory.. go to next node
                if(memNodePtr->nextAddress == NODE_ADDR_NULL)
                {
                    // end of linked list. Set to write node prev and next addr's
                    g->prevAddress = addr; // current memNode Addr
                    
                    // write new node to flash
                    writeNodeDataBlockToFlash(currentNodeMgmtHandle.nextFreeNode, g);
                    
                    // set previous last node to point to new node. write to flash
                    memNodePtr->nextAddress = currentNodeMgmtHandle.nextFreeNode;
                    writeNodeDataBlockToFlash(addr, memNodePtr);
                    
                    // read node from flash.. writes are destructive.
                    readNode(g, currentNodeMgmtHandle.nextFreeNode);
                                        
                    // set loop exit case
                    addr = NODE_ADDR_NULL; 
                }
                else
                {
                    // loop and read next node
                    addr = memNodePtr->nextAddress;
                }
            }
            else if(res < 0)
            {
                // to add parent node comes before current node in memory. Previous node is already not a memcmp match .. write node
                
                // set node to write next parent to current node in mem, set prev parent to current node in mems prev parent
                g->nextAddress = addr;
                g->prevAddress = memNodePtr->prevAddress;
                
                // write new node to flash
                writeNodeDataBlockToFlash(currentNodeMgmtHandle.nextFreeNode, g);
                
                // read back from flash
                readNodeDataBlockFromFlash(currentNodeMgmtHandle.nextFreeNode, g);
                
                // update current node in mem. set prev parent to address node to write was written to.
                memNodePtr->prevAddress = currentNodeMgmtHandle.nextFreeNode;
                writeNodeDataBlockToFlash(addr, memNodePtr);
                
                if(g->prevAddress != NODE_ADDR_NULL)
                {
                    // read p->prev node
                    readNode(memNodePtr, g->prevAddress);
                
                    // update prev node to point next parent to addr of node to write node
                    memNodePtr->nextAddress = currentNodeMgmtHandle.nextFreeNode;
                    writeNodeDataBlockToFlash(g->prevAddress, memNodePtr);
                }                
                
                if(addr == firstNodeAddress)
                {
                    // new node comes before current address and current address in first node.
                    // new node should be first node
                    *newFirstNodeAddress = currentNodeMgmtHandle.nextFreeNode;
                }
                
                // set exit case
                addr = NODE_ADDR_NULL;
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

/*! \fn     scanNodeUsage(void)
*   \brief  Scan memory to find empty slots
*/
void scanNodeUsage(void)
{
    uint16_t nodeFlags = 0xFFFF;
    uint16_t pageItr;
    uint8_t nodeItr;
    
    // Set free node addr to NODE_ADDR_NULL
    currentNodeMgmtHandle.nextFreeNode = NODE_ADDR_NULL;

    // for each page
    for(pageItr = PAGE_PER_SECTOR; pageItr < PAGE_COUNT; pageItr++)
    {
        // for each possible parent node in the page (changes per flash chip)
        for(nodeItr = 0; nodeItr < NODE_PER_PAGE; nodeItr++)
        {
            // read node flags (2 bytes - fixed size)
            readDataFromFlash(pageItr, NODE_SIZE*nodeItr, 2, &nodeFlags);
            
            // If this slot is OK
            if(validBitFromFlags(nodeFlags) == NODE_VBIT_INVALID)
            {
                currentNodeMgmtHandle.nextFreeNode = constructAddress(pageItr, nodeItr);
                return;
            }           
        }
    }
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
            memset(&temp_cnode, 0xFF, NODE_SIZE);
            writeNodeDataBlockToFlash(next_child_addr, &temp_cnode);
            
            // Set correct next address
            next_child_addr = temp_address;
        }
        
        // Store the next parent address in temp
        temp_address = temp_pnode.nextParentAddress;
        
        // Delete parent data block
        memset(&temp_pnode, 0xFF, NODE_SIZE);
        writeNodeDataBlockToFlash(next_parent_addr, &temp_pnode);
        
        // Set correct next address
        next_parent_addr = temp_address;
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
        pNode* ip = (pNode*)&(currentNodeMgmtHandle.tempgNode);
        cNode* ic = &(currentNodeMgmtHandle.child.child);
        
        // read the node at parentNodeAddress
        // userID check and valid Check performed in readParent
        readParentNode(ip, pAddr);        
        readChildNode(ic, cAddr);
        
        // Do not allow the user to change linked list links, or change child link (will be done internally)
        if ((memcmp((void*)p, (void*)ip, PNODE_LIB_FIELDS_LENGTH) != 0) || (memcmp((void*)c, (void*)ic, CNODE_LIB_FIELDS_LENGTH) != 0))
        {
            return RETURN_NOK;
        }
        
        // reorder done on login.. 
        if(strncmp((char*)&(c->login[0]), (char*)&(ic->login[0]), NODE_CHILD_SIZE_OF_LOGIN) == 0)
        {
            // service is identical just rewrite the node
            writeNodeDataBlockToFlash(cAddr, c);
            
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
    pNode *ip = (pNode*)&(currentNodeMgmtHandle.tempgNode);
    cNode *ic = &(currentNodeMgmtHandle.child.child);
    uint16_t prevAddress, nextAddress;
    
    // read parent node of child to delete
    readParentNode(ip, pAddr);
    
    // read child node to delete
    readChildNode(ic, cAddr);

    // store previous and next node of node to be deleted
    prevAddress = ic->prevChildAddress;
    nextAddress = ic->nextChildAddress;
    
    // Set child contents to FF
    memset(ic, 0xFF, NODE_SIZE);
    writeNodeDataBlockToFlash(cAddr, ic);
    
    // set previousParentNode.nextParentAddress to this.nextParentAddress
    if(prevAddress != NODE_ADDR_NULL)
    {
        // read node
        readChildNode(ic, prevAddress);
        
        // set address
        ic->nextChildAddress = nextAddress;
        
        // update node
        writeNodeDataBlockToFlash(prevAddress, ic);
    }

    // set nextParentNode.prevParentNode to this.prevParentNode
    if(nextAddress != NODE_ADDR_NULL)
    {
        // read node
        readChildNode(ic, nextAddress);
        
        // set address
        ic->prevChildAddress = prevAddress;
        
        // update node
        writeNodeDataBlockToFlash(nextAddress, ic);
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
        writeNodeDataBlockToFlash(pAddr, ip);
    }
    
    scanNodeUsage();
    return RETURN_OK;
}