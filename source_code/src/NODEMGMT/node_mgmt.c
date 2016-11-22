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
#include "logic_eeprom.h"
#include "flash_mem.h"
#include "node_mgmt.h"
#include "defines.h"
#include "usb.h"

// Current node management handle
mgmtHandle currentNodeMgmtHandle;
// Current date
uint16_t currentDate;


/*! \fn     nodeMgmtCriticalErrorCallback(void)
*   \brief  Critical error catching
*/
void nodeMgmtCriticalErrorCallback(void)
{
    usbPutstr("#NM");
    while(1);
}

/*! \fn     nodeMgmtPermissionValidityErrorCallback(void)
*   \brief  Node read permission and validity error catching
*/
void nodeMgmtPermissionValidityErrorCallback(void)
{
    usbPutstr("#NMP");
    while(1);
}

/*! \fn     setCurrentDate(uint16_t date)
*   \brief  Set current date
*   \param  date    The correctly formatted date (16 bits encoding: 15 dn 9 -> Year (2010 + val), 8 dn 5 -> Month, 4 dn 0 -> Day of Month)
*/
void setCurrentDate(uint16_t date)
{
    currentDate = date;
}

/* Flag Get/Set Helper Functions */
/**
 * Gets nodeType from flags  
 * @param   flags           The flags field of a node
 * @return  nodeType        (as uint8) (NODE_TYPE_PARENT, NODE_TYPE_CHILD, NODE_TYPE_CHILD_DATA, NODE_TYPE_DATA)
 * @note    No error checking is performed
 */
static inline uint8_t nodeTypeFromFlags(uint16_t flags)
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
static inline void nodeTypeToFlags(uint16_t *flags, uint8_t nodeType)
{
    *flags = (*flags & ~NODE_F_TYPE_MASK) | ((uint16_t)nodeType << NODE_F_TYPE_SHMT);
}

/**
 * Sets the node valid bit to flags  
 * @param   flags           The flags field of a node
 * @param   vb              The valid bit state to set in flags (NODE_VBIT_VALID, NODE_VBIT_INVALID)
 * @return  Does not return
 * @note    No error checking is performed
 */
static inline void validBitToFlags(uint16_t *flags, uint8_t vb)
{
    *flags = (*flags & (~NODE_F_VALID_BIT_MASK)) | ((uint16_t)vb << NODE_F_VALID_BIT_SHMT);
}

/**
 * Gets the credential type from flags  
 * @param   flags           The flags field of a node
 * @return  cred type       as uint8_t
 * @note    No error checking is performed
 */
static inline uint8_t credentialTypeFromFlags(uint16_t flags)
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
static inline void credentialTypeToFlags(uint16_t *flags, uint8_t credType)
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
static inline uint8_t dataNodeSequenceNumberFromFlags(uint16_t flags)
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
static inline void dataNodeSequenceNumberToFlags(uint16_t *flags, uint8_t sid)
{
    *flags = (*flags & (~NODE_F_DATA_SEQ_NUM_MASK)) | ((uint16_t)sid);
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
static inline uint16_t constructAddress(uint16_t pageNumber, uint8_t nodeNumber)
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
static inline uint16_t constructDate(uint8_t year, uint8_t month, uint8_t day)
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

/*! \fn     checkUserPermission(uint16_t node_addr)
*   \brief  Check that the user has the right to read/write a node
*   \param  node_addr   Node address
*   \return OK / NOK
*/
RET_TYPE checkUserPermission(uint16_t node_addr)
{
    // Future node flags
    uint16_t temp_flags;
    // Node Page
    uint16_t page_addr = pageNumberFromAddress(node_addr);
    // Node byte address
    uint16_t byte_addr = NODE_SIZE * (uint16_t)nodeNumberFromAddress(node_addr);
    
    // Fetch the flags
    readDataFromFlash(page_addr, byte_addr, 2, (void*)&temp_flags);
                    
    // Either the node belongs to us or it is invalid, check that the address is after sector 1 (upper check done at the flashread/write level)
    if(((getCurrentUserID() == userIdFromFlags(temp_flags)) || (validBitFromFlags(temp_flags) == NODE_VBIT_INVALID)) && (page_addr >= PAGE_PER_SECTOR))
    {
        return RETURN_OK;
    }
    else
    {
        return RETURN_NOK;
    }
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

/*! \fn     getFreeNodeAddress(void)
*   \brief  Get next free node address
*   \return The address
*/
uint16_t getFreeNodeAddress(void)
{
    return currentNodeMgmtHandle.nextFreeNode;
}

/**
 * Initializes the Node Management Handle.
 *   Check userIdNum in range,  reads users profile to get the starting parent node, scans memory for the next free parent and child nodes.
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
    currentNodeMgmtHandle.firstDataParentNode = getStartingDataParentAddress();
    currentNodeMgmtHandle.firstParentNode = getStartingParentAddress();
    currentNodeMgmtHandle.currentUserId = userIdNum;
    currentNodeMgmtHandle.datadbChanged = FALSE;
    currentNodeMgmtHandle.dbChanged = FALSE;
    
    // scan for next free parent and child nodes from the start of the memory
    if (findFreeNodes(1, &currentNodeMgmtHandle.nextFreeNode, 0, 0) == 0)
    {
        currentNodeMgmtHandle.nextFreeNode = NODE_ADDR_NULL;
    }
    
    // populate services LUT
    populateServicesLut();
}


/**
 * Function called to inform that the DB has been changed
 * Currently called on password change & add, 32B data write
 * @param   dataChanged  FALSE when a standard credential is changed, something else when it is a data node that is changed
 */
void userDBChangedActions(uint8_t dataChanged)
{
    if (((currentNodeMgmtHandle.dbChanged == FALSE) && (dataChanged == FALSE)) || ((currentNodeMgmtHandle.datadbChanged == FALSE) && (dataChanged != FALSE)))
    {
        // If the DB wasn't marked as changed, update the user db changed number
        uint8_t current_db_change_nb[2];

        // Read current user db change number
        readProfileUserDbChangeNumber((void*)&current_db_change_nb);

        // Increment the correct byte
        if (dataChanged == FALSE)
        {
            current_db_change_nb[0]++;
            currentNodeMgmtHandle.dbChanged = TRUE;
        } 
        else
        {
            current_db_change_nb[1]++;
            currentNodeMgmtHandle.datadbChanged = TRUE;
        }

        // Store updated db change number
        setProfileUserDbChangeNumber(&current_db_change_nb);
    }
}


/**
 * Sets the users starting parent node both in the handle and user profile memory portion of flash
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
 * Sets the users starting data parent node both in the handle and user profile memory portion of flash
 * @param   dataParentAddress   The constructed address of the users data starting parent node (alphabetically) 
 */
void setDataStartingParent(uint16_t dataParentAddress)
{    
    // update handle
    currentNodeMgmtHandle.firstDataParentNode = dataParentAddress;
    
    // Write data parent address in the user profile page
    writeDataToFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile + (USER_MAX_FAV * USER_FAV_SIZE) + USER_START_NODE_SIZE, 2, &dataParentAddress);
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
 * Gets the users last parent node from the node management handle
 * @return  The address
 */
uint16_t getLastParentAddress(void)
{
    return currentNodeMgmtHandle.lastParentNode;
}

/**
 * Gets the users starting data parent node from the user profile memory portion of flash
 * @return  The address
 */
uint16_t getStartingDataParentAddress(void)
{
    uint16_t temp_address;
    
    // Each user profile is within a page, data starting parent node is at the end of the favorites
    readDataFromFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile + (USER_MAX_FAV * USER_FAV_SIZE) + USER_START_NODE_SIZE, 2, &temp_address);    
    
    return temp_address;
}

/**
 * Sets a user favorite in the user profile
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
 * @param   favId           The id number of the fav record
 * @param   parentAddress   The parent node address of the fav
 * @param   childAddress    The child node address of the fav
 */
void readFav(uint8_t favId, uint16_t* parentAddress, uint16_t* childAddress)
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
    if (((validBitFromFlags(temp_uint) == NODE_VBIT_INVALID) || (checkUserPermission(*childAddress) != RETURN_OK)) && (*childAddress != NODE_ADDR_NULL))
    {
        // Delete fav and return node_addr_null
        setFav(favId, NODE_ADDR_NULL, NODE_ADDR_NULL);
        *parentAddress = NODE_ADDR_NULL;
        *childAddress = NODE_ADDR_NULL;
    }
}

/**
 * Sets the users base CTR in the user profile flash memory
 * @param   buf             The buffer containing CTR
 */
void setProfileCtr(void *buf)
{    
    // User CTR is at the end
    writeDataToFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile + USER_PROFILE_SIZE - USER_RES_CTR, USER_CTR_SIZE, buf);
}

/**
 * Reads the users base CTR from the user profile flash memory
 * @param   buf             The buffer to store the read CTR
 */
void readProfileCtr(void *buf)
{
    // User CTR is at the end
    readDataFromFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile + USER_PROFILE_SIZE - USER_RES_CTR, USER_CTR_SIZE, buf);
}

/**
 * Sets the user DB change number in the user profile flash memory
 * @param   buf             The buffer containing the user db change number
 */
void setProfileUserDbChangeNumber(void *buf)
{    
    // User CTR is at the end
    writeDataToFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile + USER_START_NODE_SIZE + (USER_MAX_FAV*USER_FAV_SIZE) + USER_DATA_START_NODE_SIZE, USER_DB_CHANGE_NB_SIZE, buf);
}

/**
 * Reads the user DB change number
 * @param   buf             The buffer to store the user db change number
 */
void readProfileUserDbChangeNumber(void *buf)
{
    // User CTR is at the end
    readDataFromFlash(currentNodeMgmtHandle.pageUserProfile, currentNodeMgmtHandle.offsetUserProfile + USER_START_NODE_SIZE + (USER_MAX_FAV*USER_FAV_SIZE) + USER_DATA_START_NODE_SIZE, USER_DB_CHANGE_NB_SIZE, buf);
}

/**
 * Reads a node from memory. If the node does not have a proper user id, g should be considered undefined
 * @param   g               Storage for the node from memory
 * @param   nodeAddress     The address to read in memory
 */
void readNode(gNode* g, uint16_t nodeAddress)
{
    readNodeDataBlockFromFlash(nodeAddress, g);
    
    if (checkUserPermission(nodeAddress) != RETURN_OK)
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
    p->service[sizeof(p->service)-1] = 0;
}

/**
 * Reads a child or child start of data node from memory.
 * @param   c               Storage for the node from memory
 * @param   childNodeAddress The address to read in memory
 */
void readChildNode(cNode *c, uint16_t childNodeAddress)
{
    readNode((gNode*)c, childNodeAddress);
    
    // If we have a date, update last used field
    if (currentDate != 0x0000)
    {
        // Just update the good field and write at the same place, write is destructive!
        c->dateLastUsed = currentDate;
        writeNodeDataBlockToFlash(childNodeAddress, c);
        readNode((gNode*)c, childNodeAddress);
    }

    c->description[sizeof(c->description)-1] = 0;
    c->login[sizeof(c->login)-1] = 0;
}

/**
 * Writes a parent node to memory (next free via handle) (in alphabetical order).
 * @param   p               The parent node to write to memory (nextFreeParentNode)
 * @param   type            Type of context (data or credential)
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE createParentNode(pNode* p, uint8_t type)
{
    uint16_t temp_address, first_parent_addr;
    RET_TYPE temprettype;
    
    // Set the first parent address depending on the type
    if (type == SERVICE_CRED_TYPE)
    {
        first_parent_addr = currentNodeMgmtHandle.firstParentNode;
    } 
    else
    {
        first_parent_addr = currentNodeMgmtHandle.firstDataParentNode;
    }
    
    // This is particular to parent nodes...
    p->nextChildAddress = NODE_ADDR_NULL;
    
    if (type == SERVICE_CRED_TYPE)
    {
        nodeTypeToFlags(&(p->flags), NODE_TYPE_PARENT);
    }
    else
    {
        nodeTypeToFlags(&(p->flags), NODE_TYPE_PARENT_DATA);
    }
    
    // Call createGenericNode to add a node
    temprettype = createGenericNode((gNode*)p, first_parent_addr, &temp_address, PNODE_COMPARISON_FIELD_OFFSET, NODE_PARENT_SIZE_OF_SERVICE);
    
    // If the return is ok & we changed the first node address
    if ((temprettype == RETURN_OK) && (first_parent_addr != temp_address))
    {
        if (type == SERVICE_CRED_TYPE)
        {
            setStartingParent(temp_address);
        }
        else
        {
            setDataStartingParent(temp_address);
        }
    }
    
    // Populate services LUT
    populateServicesLut();
    
    return temprettype;
}

/**
 * Writes a child node to memory (next free via handle) (in alphabetical order).
 * @param   pAddr           The parent node address of the child
 * @param   c               The child node to write to memory (nextFreeChildNode)
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
    
    // Write date created & used fields
    c->dateCreated = currentDate;
    c->dateLastUsed = currentDate;
    
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
 * Writes a new data node to memory (next free via handle) (in alphabetical order).
 * @param   context_parent_node_addr    Address to the parent node
 * @param   parent_node_ptr             Parent node pointer
 * @param   data_node_ptr               Data node pointer
 * @param   first_data_block_flag       Flag to see if it is the first data block
 * @param   last_packet_flag            Flag to know if it is the last block
 * @return  success status
 */
RET_TYPE writeNewDataNode(uint16_t context_parent_node_addr, pNode* parent_node_ptr, dNode* data_node_ptr, uint8_t first_data_block_flag, uint8_t last_packet_flag)
{
    gNode* memNodePtr = &(currentNodeMgmtHandle.tempgNode);
    // Our next 2 free addresses
    uint16_t next_free_addresses[2];
    
    // This is what scan node usage uses internally, check space in flash
    if (findFreeNodes(2, next_free_addresses, 0, 0) != 2)
    {
        return RETURN_NOK;
    }    
    
    // If it is the first data node we need to update the parent
    if (first_data_block_flag == TRUE)
    {
        // Read supposed parent node
        readNode(memNodePtr, context_parent_node_addr);
        
        // Check the parent nodes fields are the same, update parent node at the right address
        if (memcmp((void*)memNodePtr, (void*)parent_node_ptr, FLAGS_PREV_NEXT_ADDR_LENGTH) == 0)
        {
            parent_node_ptr->nextChildAddress = next_free_addresses[0];
            writeNodeDataBlockToFlash(context_parent_node_addr, parent_node_ptr);
        }
        else
        {
            return RETURN_NOK;
        }
    }
    
    // Set correct user id to the node
    userIdToFlags(&(data_node_ptr->flags), currentNodeMgmtHandle.currentUserId);
    
    // set valid bit
    validBitToFlags(&(data_node_ptr->flags), NODE_VBIT_VALID);
    
    // Set correct node type
    nodeTypeToFlags(&(data_node_ptr->flags), NODE_TYPE_DATA);
    
    // If it is not the last packet, set next address
    if (last_packet_flag == FALSE)
    {
        // Because we can't interrupt data transfer, the next address will automatically be the next one we have in memory
        data_node_ptr->nextDataAddress = next_free_addresses[1];
    }
    
    // write parent node to flash (destructive)
    writeNodeDataBlockToFlash(next_free_addresses[0], data_node_ptr);
    
    // Update free node address
    currentNodeMgmtHandle.nextFreeNode = next_free_addresses[1];
    
    return RETURN_OK;
}

/**
 * Writes a generic node to memory (next free via handle) (in alphabetical order).
 * @param   g                       The node to write to memory (nextFreeParentNode)
 * @param   firstNodeAddress        Address of the first node of its kind
 * @param   newFirstNodeAddress     If the firstNodeAddress changed, this var will store the new value
 * @param   comparisonFieldOffset   The offset used to do the comparison used for the sorting
 * @param   comparisonFieldLength   The length of the field used for comparison
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

/*! \fn     populateServicesLut(void)
*   \brief  Populate our LUT for our services
*/
void populateServicesLut(void)
{
    uint16_t next_node_addr = currentNodeMgmtHandle.firstParentNode;
    uint8_t temp_node_buffer[9];
    uint16_t temp_page_number;
    pNode* pnode_ptr = (pNode*)temp_node_buffer;
    uint8_t first_service_letter;
    
    // Empty our current services list
    memset(currentNodeMgmtHandle.servicesLut, 0x00, sizeof(currentNodeMgmtHandle.servicesLut));
    
    // If the dedicated boolean in eeprom is sent, do not actually populate the LUT
    if (getMooltipassParameterInEeprom(LUT_BOOT_POPULATING_PARAM) == FALSE)
    {
        currentNodeMgmtHandle.lastParentNode = getStartingParentAddress();
        return;
    }
    
    // If we have at least one node, loop through our credentials
    while(next_node_addr != NODE_ADDR_NULL)
    {
        // Get the node page number
        temp_page_number = pageNumberFromAddress(next_node_addr);
        
        // Check that we're not out of memory bounds
        if(temp_page_number >= PAGE_COUNT)
        {
            // TODO: Set a bool somewhere to mention corrupted memory
            return;
        }

        // Read first 9 bytes of the parent node as we just want to know the first letter
        readDataFromFlash(temp_page_number, NODE_SIZE * nodeNumberFromAddress(next_node_addr), sizeof(temp_node_buffer), temp_node_buffer);
        first_service_letter = pnode_ptr->service[0];
            
        // LUT is only for chars between 'a' and 'z'
        if ((first_service_letter >= 'a') && (first_service_letter <= 'z'))
        {
            // If LUT entry not populated, populate it
            if (currentNodeMgmtHandle.servicesLut[first_service_letter - 'a'] == NODE_ADDR_NULL)
            {
                currentNodeMgmtHandle.servicesLut[first_service_letter - 'a'] = next_node_addr;
            }
        }            

        // Store last node address
        currentNodeMgmtHandle.lastParentNode = next_node_addr;
            
        // Fetch next node
        next_node_addr = pnode_ptr->nextParentAddress;
    }
}

/*! \fn     getPreviousNextFirstLetterForGivenLetter(char c, char* array, uint16_t* parent_addresses)
*   \brief  Get the previous and next letter around a given letter
*   \param  c                   The first letter
*   \param  array               Three char array to store the previous and next one
*   \param  parent_addresses    Three uint16_t array to store the corresponding parent addresses
*   \note   In the array, all letters will be higher case
*/
void getPreviousNextFirstLetterForGivenLetter(char c, char* array, uint16_t* parent_addresses)
{
    // Set -s by default
    memset(array, '-', 3);
    parent_addresses[0] = NODE_ADDR_NULL;
    parent_addresses[2] = NODE_ADDR_NULL;

    // Store the provided char as first letter for the current credential
    if ((c >= 'a') && (c <= 'z'))
    {
        array[1] = c - 'a' + 'A';
    } 
    else
    {
        array[1] = c;
    }

    // Loop through our LUT
    for (uint8_t i = 0; i < sizeof(currentNodeMgmtHandle.servicesLut)/sizeof(currentNodeMgmtHandle.servicesLut[0]); i++)
    {
        if (currentNodeMgmtHandle.servicesLut[i] != NODE_ADDR_NULL)
        {
            if (((i + 'a') < c) && (array[0] != (i + 'A')))
            {
                // First letter before the current one, only run once for each letter
                array[0] = i + 'A';
                parent_addresses[0] = currentNodeMgmtHandle.servicesLut[i];
            }
            if ((i + 'a') > c)
            {
                // First letter after the current one
                array[2] = i + 'A';
                parent_addresses[2] = currentNodeMgmtHandle.servicesLut[i];
                return;
            }
        }
    }
}

/*! \fn     getParentNodeForLetter(uint8_t letter, uint8_t empty_mode)
*   \brief  Use the LUT to find the first parent node for a given letter
*   \note   If we don't know the letter, the first previous one will be returned
*   \param  letter      The first letter
*/
uint16_t getParentNodeForLetter(uint8_t letter)
{    
    // LUT is only for chars between 'a' and 'z'
    if ((letter >= 'a') && (letter <= 'z'))
    {
        // If the entry is populated, return it
        if (currentNodeMgmtHandle.servicesLut[letter - 'a'] != NODE_ADDR_NULL)
        {
            return currentNodeMgmtHandle.servicesLut[letter - 'a'];
        }
        else
        {            
            // No entry, return the one before
            for (int8_t i = letter - 'a'; i >= 0; i--)
            {
                if (currentNodeMgmtHandle.servicesLut[(uint8_t)i] != NODE_ADDR_NULL)
                {
                    return currentNodeMgmtHandle.servicesLut[(uint8_t)i];
                }
            }
                
            // If we're here it means nothing was found so we return the starting parent
            return currentNodeMgmtHandle.firstParentNode;
        }
    }
    else
    {
        return currentNodeMgmtHandle.firstParentNode;
    }
}

/*! \fn     findFreeNodes(uint8_t nbNodes, uint16_t* array)
*   \brief  Find Free Nodes inside our external memory
*   \param  nbNodes     Number of nodes we want to find
*   \param  nodeArray   An array where to store the addresses
*   \param  startPage   Page where to start the scanning
*   \param  startNode   Scan start node address inside the start page
*   \return the number of nodes found
*/
uint8_t findFreeNodes(uint8_t nbNodes, uint16_t* nodeArray, uint16_t startPage, uint8_t startNode)
{
    uint8_t nbNodesFound = 0;
    uint16_t nodeFlags;
    uint16_t pageItr;
    uint8_t nodeItr;
    
    // Check the start page
    if (startPage < PAGE_PER_SECTOR)
    {
        startPage = PAGE_PER_SECTOR;
    }

    // for each page
    for(pageItr = startPage; pageItr < PAGE_COUNT; pageItr++)
    {
        // for each possible parent node in the page (changes per flash chip)
        for(nodeItr = startNode; nodeItr < NODE_PER_PAGE; nodeItr++)
        {
            // read node flags (2 bytes - fixed size)
            readDataFromFlash(pageItr, NODE_SIZE*nodeItr, 2, &nodeFlags);
            
            // If this slot is OK
            if(validBitFromFlags(nodeFlags) == NODE_VBIT_INVALID)
            {
                if (nbNodesFound < nbNodes)
                {
                    nodeArray[nbNodesFound++] = constructAddress(pageItr, nodeItr);
                }
                else
                {
                    return nbNodesFound;
                }
            }
        }
        startNode = 0;
    }    
    
    return nbNodesFound;
}

/*! \fn     scanNodeUsage(void)
*   \brief  Scan memory to find empty slots
*/
void scanNodeUsage(void)
{
    // Find one free node. If we don't find it, set the next to the null addr, we start looking from the just taken node
    if (findFreeNodes(1, &currentNodeMgmtHandle.nextFreeNode, pageNumberFromAddress(currentNodeMgmtHandle.nextFreeNode), nodeNumberFromAddress(currentNodeMgmtHandle.nextFreeNode)) == 0)
    {
        currentNodeMgmtHandle.nextFreeNode = NODE_ADDR_NULL;
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
    for (uint8_t i = 0; i < 2; i++)
    {
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
                if (i == 0)
                {
                    // First loop is cnode
                    temp_address = temp_cnode.nextChildAddress;
                } 
                else
                {
                    // Second loop is dnode
                    dNode* temp_dnode_ptr = (dNode*)&temp_cnode;
                    temp_address = temp_dnode_ptr->nextDataAddress;
                }
                
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
        // First loop done, remove data nodes
        next_parent_addr = currentNodeMgmtHandle.firstDataParentNode;
    }
    
    // Empty service lut (not needed as the user is deleted)
    //memset(currentNodeMgmtHandle.servicesLut, 0x00, sizeof(currentNodeMgmtHandle.servicesLut));
}

/**
 * Updates a child or child start of data node in memory. Handles alphabetical reorder of nodes.
 *   Scans for nextFreeChildNode after completion.  Modifies the node management handle
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
        cNode buf_cnode;
        cNode* ic = &buf_cnode;
        
        // read the node at parentNodeAddress
        // userID check and valid Check performed in readParent
        readParentNode(ip, pAddr);        
        readChildNode(ic, cAddr);
        
        // Do not allow the user to change linked list links, or change child link (will be done internally)
        if ((memcmp((void*)p, (void*)ip, PNODE_LIB_FIELDS_LENGTH) != 0)
            || (memcmp( ((void*)c)  + sizeof(c->flags),                     /* skip flags comparison */
                        ((void*)ic) + sizeof(ic->flags),                    /* skip flags comparison */
                        CNODE_LIB_FIELDS_LENGTH - sizeof(ic->flags)) != 0 ) /* reduce compared size by that of the flags */
            || ((c->flags & ~NODE_F_CHILD_USERFLAGS_MASK) != (ic->flags & ~NODE_F_CHILD_USERFLAGS_MASK)) ) /* compare non-reserved flag bits */
        {
            return RETURN_NOK;
        }
		
        // Write date created & used fields
        c->dateCreated = currentDate;
        c->dateLastUsed = currentDate;
        
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
            ret = deleteChildNode(pAddr, cAddr, ic);
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
 * @param   pAddr           The address of the parent of the child
 * @param   cAddr           The address of the child
 * @param   ic              Pointer to a temporary childNode for buffer purposes
 * @return  success status
 * @note    Handles necessary doubly linked list management
 */
RET_TYPE deleteChildNode(uint16_t pAddr, uint16_t cAddr, cNode *ic)
{
    pNode *ip = (pNode*)&(currentNodeMgmtHandle.tempgNode);
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

/**
 * Updates the password field of a given child node
 * @param   c               Pointer to a temporary child node for buffer purposes
 * @param   cAddr           The address to the child node to update
 * @param   password        Contents of the new password field, NODE_CHILD_SIZE_OF_PASSWORD long
 * @param   ctr_value       New CTR value
 * @note    cNode will be filled with the child node in case it may be useful....
 * @return  success status
 */
RET_TYPE updateChildNodePassword(cNode* c, uint16_t cAddr, uint8_t* password, uint8_t* ctr_value)
{    
    // userID check and valid check performed in readChild
    readChildNode(c, cAddr);
    
    // Write date created & used fields
    c->dateCreated = currentDate;
    c->dateLastUsed = currentDate;

    // Update password & ctr fields
    memcpy(c->password, password, NODE_CHILD_SIZE_OF_PASSWORD);
    memcpy(c->ctr, ctr_value, USER_CTR_SIZE);

    // service is identical just rewrite the node
    writeNodeDataBlockToFlash(cAddr, c);
    
    // write is destructive.. read
    readChildNode(c, cAddr);
    
    return RETURN_OK;
}

/**
 * Updates the description field of a given child node
 * @param   c               Pointer to a temporary child node for buffer purposes
 * @param   cAddr           The address to the child node to update
 * @param   description     Contents of the new description field, NODE_CHILD_SIZE_OF_DESCRIPTION long
 * @note    cNode will be filled with the child node in case it may be useful....
 * @return  success status
 */
RET_TYPE updateChildNodeDescription(cNode* c, uint16_t cAddr, uint8_t* description)
{    
    // userID check and valid check performed in readChild
    readChildNode(c, cAddr);

    // Update description field
    memcpy(c->description, description, NODE_CHILD_SIZE_OF_DESCRIPTION);

    // service is identical just rewrite the node
    writeNodeDataBlockToFlash(cAddr, c);
    
    // write is destructive.. read
    readChildNode(c, cAddr);
    
    return RETURN_OK;
}

/**
 * Delete all data nodes child starting from firstChildNodeAddress
 * @param dataNodeAddress     The address of the first child in the chain
 * @note it is safe to call this function even with a NULL address (nothing is done)
 */
void deleteDataNodeChain(uint16_t dataNodeAddress, dNode* data_node_ptr)
{
    uint16_t nextAddress;

    while (dataNodeAddress != NODE_ADDR_NULL)
    {
        //read the actual block pointed by the given address
        readNode((gNode *)data_node_ptr, dataNodeAddress);

        //save the next address for deleting the next block
        nextAddress = data_node_ptr->nextDataAddress;

        // Set child contents to FF and write it back to flash
        memset((void*)data_node_ptr, 0xFF, NODE_SIZE);
        writeNodeDataBlockToFlash(dataNodeAddress, data_node_ptr);

        dataNodeAddress = nextAddress;
    }
}

