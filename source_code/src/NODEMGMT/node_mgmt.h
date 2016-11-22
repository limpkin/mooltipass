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

/*!  \file     node_mgmt.h
*    \brief    Mooltipass Node Management Library Header
*    Created:  03/4/2014
*    Author:   Michael Neiderhauser
*/


#ifndef NODE_MGMT_H_
#define NODE_MGMT_H_

#include "defines.h"

typedef enum _nodeType
{
    NODE_TYPE_PARENT = 0,
    NODE_TYPE_CHILD = 1,
    NODE_TYPE_PARENT_DATA = 2,
    NODE_TYPE_DATA = 3
} nodeType;

/** DEFINES NODES **/
#define NODE_SIZE_PARENT 66
#define NODE_SIZE_CHILD 132
#define NODE_SIZE_DATA 132

#define NODE_SIZE   132

#define NODE_ADDR_NULL 0x0000
#define NODE_VBIT_VALID 0
#define NODE_VBIT_INVALID 1

#define NODE_F_TYPE_MASK 0xc000
#define NODE_F_TYPE_SHMT 14
#define NODE_F_TYPE_MASK_FINAL 0x0003

#define NODE_F_VALID_BIT_MASK 0x2000
#define NODE_F_VALID_BIT_SHMT 13
#define NODE_F_VALID_BIT_MASK_FINAL 0x0001

#define NODE_F_UID_MASK 0x1f00
#define NODE_F_UID_SHMT 8
#define NODE_F_UID_MASK_FINAL 0x001f

#define NODE_F_CRED_TYPE_MASK 0x000f
//#define NODE_F_CRED_TYPE_MASK 0
//#define NODE_F_CRED_TYPE_MASK_FINAL 0x000f

#define NODE_F_CHILD_USERFLAGS_MASK 0x00ff // reserved bits in child node left available to user data (credential charset)

#define NODE_F_DATA_SEQ_NUM_MASK 0x00ff

#define NODE_ADDR_SHMT 3
#define NODE_ADDR_PAGE_MASK 0x1fff
#define NODE_ADDR_NODE_MASK 0x0007

#define NODE_MGMT_YEAR_SHT 9
#define NODE_MGMT_YEAR_MASK 0xFE00
#define NODE_MGMT_YEAR_MASK_FINAL 0x007F

#define NODE_MGMT_MONTH_SHT 5
#define NODE_MGMT_MONTH_MASK 0x03E0
#define NODE_MGMT_MONTH_MASK_FINAL 0x000F

#define NODE_MGMT_DAY_MASK_FINAL 0x001F

#define NODE_PARENT_SIZE_OF_SERVICE 58
#define NODE_CHILD_SIZE_OF_DESCRIPTION 24
#define NODE_CHILD_SIZE_OF_LOGIN 63
#define NODE_CHILD_SIZE_OF_PASSWORD 32
#define NODE_CHILD_SIZE_OF_CTR 3

#define NODE_MAX_UID 16
#define NODE_MAX_CRED_TYPE 16
#define NODE_MAX_DATA 256

/* User profile layout:
- 2B: user start node
- 4*14B: favorites
- 2B: user data start node
- 2B: user db change number (1B for cred DB, 1B for data DB)
- 3B: user CTR
- 1B: reserved
*/
#define USER_START_NODE_SIZE 2
#define USER_FAV_SIZE 4
#define USER_MAX_FAV 14
#define USER_DATA_START_NODE_SIZE 2
#define USER_DB_CHANGE_NB_SIZE 2
#define USER_RES_CTR 4
#define USER_PROFILE_SIZE (USER_START_NODE_SIZE + (USER_MAX_FAV*USER_FAV_SIZE) + USER_DATA_START_NODE_SIZE + USER_DB_CHANGE_NB_SIZE + USER_RES_CTR)
#define USER_CTR_SIZE 3             // USER_RES_CTR is set to 4 but the actual CTR is 3 bytes long and the last byte is reserved for later

#define GRAPHIC_ZONE_START          (8*BYTES_PER_PAGE)
#define GRAPHIC_ZONE_PAGE_START     (8)
#define GRAPHIC_ZONE_END            ((uint32_t)((uint32_t)SECTOR_START*(uint32_t)PAGE_PER_SECTOR*(uint32_t)BYTES_PER_PAGE))
#define GRAPHIC_ZONE_PAGE_END       (SECTOR_START*PAGE_PER_SECTOR)

#define DELETE_POLICY_WRITE_ONES 0xFF  /*! Node Deletion Policy Ones Memset Value */

// flags, prev & nextaddress bytes length
#define FLAGS_PREV_NEXT_ADDR_LENGTH 6

/*!
* Struct containing a generic node
*/
typedef struct __attribute__((packed)) genericNode {
    uint16_t flags;                 /*!< Parent node flags
                                    * 15 dn 14-> Node type (Always 00 for Parent Node)
                                    * 13 dn 13 -> Valid Bit
                                    * 12 dn 8 -> User ID
                                    * 7 dn 0 -> depends on the node type
                                    */
    uint16_t prevAddress;           /*!< Previous node address (Alphabetically) */
    uint16_t nextAddress;           /*!< Next node address (Alphabetically) */
    uint8_t data[NODE_SIZE - 3*sizeof(uint16_t)];
} gNode;

/*!
* Struct containing a parent node
*/
typedef struct __attribute__((packed)) parentNode {
    uint16_t flags;                 /*!< Parent node flags
                                    * 15 dn 14-> Node type (Always 00 for Parent Node)
                                    * 13 dn 13 -> Valid Bit
                                    * 12 dn 8 -> User ID
                                    * 7 dn 4 -> RESERVED
                                    * 3 dn0 -> credential type UID
                                    */
    uint16_t prevParentAddress;     /*!< Previous parent node address (Alphabetically) */
    uint16_t nextParentAddress;     /*!< Next parent node address (Alphabetically) */
    uint16_t nextChildAddress;      /*!< Parent node first child address */
    uint8_t service[NODE_SIZE - 4*sizeof(uint16_t) - 3*sizeof(uint8_t)];            /*!< (ASCII) Text describing service (domain name eg hackaday.com). Used for sorting and searching. */
    uint8_t startDataCtr[3];       /*!< Encryption counter in case the child is a data node */
} pNode;

// flags + prevParentAddress + nextParentAddress + nextChildAddress
#define PNODE_COMPARISON_FIELD_OFFSET   8
#define PNODE_LIB_FIELDS_LENGTH         8

// Size of the password field inside the child node
#define C_NODE_PWD_SIZE                 32

/*!
* Struct containing a child node
*/
typedef struct __attribute__((packed)) childNode {
    uint16_t flags;                     /*!< Child node flags (0b01 -> credential child node. 0b10 -> start of data sequence)
                                        * 15 dn 14-> Node type
                                        * 13 dn 13 -> Valid Bit
                                        * 12 dn 8 -> User ID
                                        * 7 dn 0 -> User-available flags: if credential management is enabled, stores allowed charset for credentials
                                        */
    uint16_t prevChildAddress;          /*!< Previous child node address (Alaphabetically) */
    uint16_t nextChildAddress;          /*!< Next child node address (Alphabetically) */
    uint8_t description[24];            /*!< (ASCII) Text describing login credentials (wordpress login) */
    uint16_t dateCreated;               /*!< The date the child node was added to mooltipass (requires plugin)
                                        * Date Encoding:
                                        * 15 dn 9 -> Year (2010 + value)
                                        * 8 dn 5 -> Month
                                        * 4 dn 0 -> Day
                                        */
    uint16_t dateLastUsed;              /*!< The date the child node was last used on the mooltipass (requires plugin)
                                        * Date Encoding:
                                        * 15 dn 9 -> Year (2010 + value)
                                        * 8 dn 5 -> Month
                                        * 4 dn 0 -> Day
                                        */
    uint8_t ctr[3];                     /*!< Encryption counter */
    uint8_t login[63];                  /*!< (ASCII) Text login name (username) */
    uint8_t password[C_NODE_PWD_SIZE];  /*!< Encrypted Password */
} cNode;

// flags + prevChildAddress + nextChildAddress + description + dateCreated + dateLastUsed + ctr
#define CNODE_COMPARISON_FIELD_OFFSET   37
#define CNODE_LIB_FIELDS_LENGTH         6

#define DATA_NODE_DATA_LENGTH           128

/*!
* Struct containing a data node
*
* Note: Requires plugin for support.  Up to 256 data nodes can be used in sequence
*/
typedef struct __attribute__((packed)) dataNode {
    uint16_t flags;                         /*!< Data node flags (Always 0b11 for Data Node)
                                            * 15 dn 14-> Node type
                                            * 13 dn 13 -> Valid Bit
                                            * 12 dn 8 -> User ID
                                            * 7 dn 0 -> Number of bytes stored in data[]
                                            */
    uint16_t nextDataAddress;               /*!< Next data node in sequence */
    uint8_t data[DATA_NODE_DATA_LENGTH];    /*!< 128 bytes of Large Data Store */
} dNode;

/*!
* Struct containing Node Management Handle
*
* Note: Do not directly modify these fields.  Node Mgmt will manage this structure
*/
typedef struct __attribute__((packed)) nodeMgmtH
{
    uint8_t datadbChanged;          /*!< Boolean to indicate if the user data DB has changed since user login */
    uint8_t dbChanged;              /*!< Boolean to indicate if the user DB has changed since user login */
    uint8_t currentUserId;          /*!< The users ID */
    uint16_t pageUserProfile;       /*!< The page of the user profile */
    uint16_t offsetUserProfile;     /*!< The offset of the user profile */
    uint16_t firstParentNode;       /*!< The address of the users first parent node (read from flash. eg cache) */
    uint16_t firstDataParentNode;   /*!< The address of the users first data parent node (read from flash. eg cache) */
    uint16_t lastParentNode;        /*!< The address of the users last parent node (read from flash. eg cache) */
    uint16_t nextFreeNode;          /*!< The address of the next free node */
    gNode tempgNode;                /*!< A generic node to be used as a buffer */
    uint16_t servicesLut[26];       /*!<Look up table for our services */
} mgmtHandle;

/**
 * Extracts a page number from a constructed address
 * @param   flags           The flags field of a node
 * @param   addr            The constructed address used for extraction
 * @return  page num        A page number in flash memory (uin16_t)
 * @note    No error checking is performed
 * @note    See design notes for address format
 * @note    Max Page Number varies per flash size
 */
static inline uint16_t pageNumberFromAddress(uint16_t addr)
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
static inline uint8_t nodeNumberFromAddress(uint16_t addr)
{
    return (uint8_t)(addr & NODE_ADDR_NODE_MASK);
}

/**
 * Gets the node valid bit from flags  
 * @param   flags           The flags field of a node
 * @return  valid bit       as uint8_t
 * @note    No error checking is performed
 */
static inline uint8_t validBitFromFlags(uint16_t flags)
{
    return (uint8_t)(((flags & NODE_F_VALID_BIT_MASK) >> NODE_F_VALID_BIT_SHMT) & NODE_F_VALID_BIT_MASK_FINAL);
}

/**
 * Gets the user id from flags  
 * @param   flags           The flags field of a node
 * @return  user id         as uint8_t
 * @note    No error checking is performed
 */
static inline uint8_t userIdFromFlags(uint16_t flags)
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
static inline void userIdToFlags(uint16_t *flags, uint8_t uid)
{
    *flags = (*flags & (~NODE_F_UID_MASK)) | ((uint16_t)uid << NODE_F_UID_SHMT);
}

/* Init Handle */
void initNodeManagementHandle(uint8_t userIdNum);

/* User Memory Functions */
uint8_t getCurrentUserID(void);
uint16_t getFreeNodeAddress(void);
void deleteCurrentUserFromFlash(void);
void formatUserProfileMemory(uint8_t uid);
RET_TYPE checkUserPermission(uint16_t node_addr);
void userProfileStartingOffset(uint8_t uid, uint16_t *page, uint16_t *pageOffset);

void setDataStartingParent(uint16_t dataParentAddress);
void setStartingParent(uint16_t parentAddress);
uint16_t getStartingDataParentAddress(void);
uint16_t getStartingParentAddress(void);
uint16_t getLastParentAddress(void);

void getPreviousNextFirstLetterForGivenLetter(char c, char* array, uint16_t* parent_addresses);
uint16_t getParentNodeForLetter(uint8_t letter);
void populateServicesLut(void);

void setFav(uint8_t favId, uint16_t parentAddress, uint16_t childAddress);
void readFav(uint8_t favId, uint16_t *parentAddress, uint16_t *childAddress);

void setProfileCtr(void *buf);
void readProfileCtr(void *buf);

RET_TYPE createGenericNode(gNode* g, uint16_t firstNodeAddress, uint16_t* newFirstNodeAddress, uint8_t comparisonFieldOffset, uint8_t comparisonFieldLength);

RET_TYPE writeNewDataNode(uint16_t context_parent_node_addr, pNode* parent_node_ptr, dNode* data_node_ptr, uint8_t first_data_block_flag, uint8_t last_packet_flag);
RET_TYPE createParentNode(pNode* p, uint8_t type);
void readParentNode(pNode *p, uint16_t parentNodeAddress);
RET_TYPE updateParentNode(pNode *p, uint16_t parentNodeAddress);
RET_TYPE deleteParentNode(uint16_t parentNodeAddress);
void deleteDataNodeChain(uint16_t dataNodeAddress, dNode* data_node_ptr);

RET_TYPE createChildNode(uint16_t pAddr, cNode *c);
RET_TYPE createChildStartOfDataNode(uint16_t pAddr, cNode *c, uint8_t dataNodeCount);
void readChildNode(cNode *c, uint16_t childNodeAddress);
RET_TYPE updateChildNode(pNode *p, cNode *c, uint16_t pAddr, uint16_t cAddr);
RET_TYPE deleteChildNode(uint16_t pAddr, uint16_t cAddr, cNode *ic);

void readNode(gNode* g, uint16_t nodeAddress);

uint8_t findFreeNodes(uint8_t nbNodes, uint16_t* nodeArray, uint16_t startPage, uint8_t startNode);
RET_TYPE updateChildNodePassword(cNode* c, uint16_t cAddr, uint8_t* password, uint8_t* ctr_value);
RET_TYPE updateChildNodeDescription(cNode* c, uint16_t cAddr, uint8_t* description);
void setProfileUserDbChangeNumber(void *buf);
void readProfileUserDbChangeNumber(void *buf);
void scanNodeUsage(void);

void setCurrentDate(uint16_t date);
void userDBChangedActions(uint8_t dataChanged);

#endif /* NODE_MGMT_H_ */
