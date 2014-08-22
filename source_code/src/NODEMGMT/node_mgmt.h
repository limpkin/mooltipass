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
    NODE_TYPE_CHILD_DATA = 2,
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

#define USER_START_NODE_SIZE 2
#define USER_FAV_SIZE 4
#define USER_MAX_FAV 15
#define USER_RES_CTR 4
#define USER_PROFILE_SIZE (USER_START_NODE_SIZE+(USER_MAX_FAV*USER_FAV_SIZE)+USER_RES_CTR)
#define USER_CTR_SIZE 3 // Have 1 byte remaining.. Not included in USER_PROFILE_SIZE

#define GRAPHIC_ZONE_START          (8*BYTES_PER_PAGE)
#define GRAPHIC_ZONE_PAGE_START     (8)
#define GRAPHIC_ZONE_END            ((uint32_t)((uint32_t)SECTOR_START*(uint32_t)PAGE_PER_SECTOR*(uint32_t)BYTES_PER_PAGE))
#define GRAPHIC_ZONE_PAGE_END       (SECTOR_START*PAGE_PER_SECTOR)

#define DELETE_POLICY_WRITE_ONES 0xFF  /*! Node Deletion Policy Ones Memset Value */

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
    uint8_t service[NODE_SIZE - 4*sizeof(uint16_t)];            /*!< (ASCII) Text describing service (domain name eg hackaday.com). Used for sorting and searching. */
} pNode;

// flags + prevParentAddress + nextParentAddress + nextChildAddress
#define PNODE_COMPARISON_FIELD_OFFSET   8
#define PNODE_LIB_FIELDS_LENGTH         8

/*!
* Struct containing a child node
*/
typedef struct __attribute__((packed)) childNode {
    uint16_t flags;                 /*!< Child node flags (0b01 -> credential child node. 0b10 -> start of data sequence)
                                    * 15 dn 14-> Node type
                                    * 13 dn 13 -> Valid Bit
                                    * 12 dn 8 -> User ID
                                    * 7 dn 0 -> RESERVED
                                    */
    uint16_t prevChildAddress;      /*!< Previous child node address (Alaphabetically) */
    uint16_t nextChildAddress;      /*!< Next child node address (Alphabetically) */
    uint8_t description[24];        /*!< (ASCII) Text describing login credentials (wordpress login) */
    uint16_t dateCreated;           /*!< The date the child node was added to mooltipass (requires plugin) 
                                    * Date Encoding:
                                    * 15 dn 9 -> Year (2010 + value)
                                    * 8 dn 5 -> Month
                                    * 4 dn 0 -> Day
                                    */
    uint16_t dateLastUsed;          /*!< The date the child node was last used on the mooltipass (requires plugin) 
                                    * Date Encoding:
                                    * 15 dn 9 -> Year (2010 + value)
                                    * 8 dn 5 -> Month
                                    * 4 dn 0 -> Day
                                    */
    uint8_t ctr[3];                 /*!< Encryption counter */
    uint8_t login[63];              /*!< (ASCII) Text login name (username) */
    uint8_t password[32];           /*!< Encrypted Password */
} cNode;

// flags + prevChildAddress + nextChildAddress + description + dateCreated + dateLastUsed + ctr
#define CNODE_COMPARISON_FIELD_OFFSET   37
#define CNODE_LIB_FIELDS_LENGTH         8

/*!
* Struct containing a data node
*
* Note: Requires plugin for support.  Up to 256 data nodes can be used in sequence
*/
typedef struct __attribute__((packed)) dataNode {
    uint16_t flags;                 /*!< Data node flags (Always 0b11 for Data Node)
                                    * 15 dn 14-> Node type
                                    * 13 dn 13 -> Valid Bit
                                    * 12 dn 8 -> User ID
                                    * 7 dn 0 -> Data node sequence number
                                    */
    uint16_t nextDataAddress;       /*!< Next data node in sequence */
    uint8_t data[128];              /*!< 128 bytes of Large Data Store */
} dNode;

/*!
* Struct containing Node Management Handle
*
* Note: Do not directly modify these fields.  Node Mgmt will manage this structure
*/
typedef struct __attribute__((packed)) nodeMgmtH
{
    uint16_t flags;
    /*
    15 dn 0 Free
    */
    
    uint8_t currentUserId;          /*!< The users ID */
    uint16_t pageUserProfile;       /*!< The page of the user profile */
    uint16_t offsetUserProfile;     /*!< The offset of the user profile */
    uint16_t firstParentNode;       /*!< The address of the users first parent node (read from flash. eg cache) */
    uint16_t nextFreeNode;          /*!< The address of the next free node */
    gNode tempgNode;                /*!< A generic node to be used as a buffer */
    union {
        cNode child;
        dNode data;
    } child;                        /*!< A child, child start of data, or child data node to be used as a buffer in the API */
} mgmtHandle;

/* Helper Functions (flags and address) */
uint8_t nodeTypeFromFlags(uint16_t flags);
void  nodeTypeToFlags(uint16_t *flags, uint8_t nodeType);


uint8_t validBitFromFlags(uint16_t flags);
void validBitToFlags(uint16_t *flags, uint8_t vb);

uint8_t userIdFromFlags(uint16_t flags);
void userIdToFlags(uint16_t *flags, uint8_t uid);

uint8_t credentialTypeFromFlags(uint16_t flags);
void credentialTypeToFlags(uint16_t *flags, uint8_t credType);

uint8_t dataNodeSequenceNumberFromFlags(uint16_t flags);
void dataNodeSequenceNumberToFlags(uint16_t *flags, uint8_t sid);

uint16_t pageNumberFromAddress(uint16_t addr);
uint8_t nodeNumberFromAddress(uint16_t addr);
uint16_t constructAddress(uint16_t pageNumber, uint8_t nodeNumber);

uint16_t constructDate(uint8_t year, uint8_t month, uint8_t day);
RET_TYPE extractDate(uint16_t date, uint8_t *year, uint8_t *month, uint8_t *day);

/* Init Handle */
void initNodeManagementHandle(uint8_t userIdNum);

/* User Memory Functions */
uint8_t getCurrentUserID(void);
void deleteCurrentUserFromFlash(void);
void formatUserProfileMemory(uint8_t uid);
void userProfileStartingOffset(uint8_t uid, uint16_t *page, uint16_t *pageOffset);

void setStartingParent(uint16_t parentAddress);
uint16_t getStartingParentAddress(void);

void setFav(uint8_t favId, uint16_t parentAddress, uint16_t childAddress);
void readFav(uint8_t favId, uint16_t *parentAddress, uint16_t *childAddress);

void setProfileCtr(void *buf);
void readProfileCtr(void *buf);

RET_TYPE createGenericNode(gNode* g, uint16_t firstNodeAddress, uint16_t* newFirstNodeAddress, uint8_t comparisonFieldOffset, uint8_t comparisonFieldLength);

RET_TYPE createParentNode(pNode *p);                                        
void readParentNode(pNode *p, uint16_t parentNodeAddress);
RET_TYPE updateParentNode(pNode *p, uint16_t parentNodeAddress);
RET_TYPE deleteParentNode(uint16_t parentNodeAddress);
RET_TYPE invalidateParentNode(pNode *p);

RET_TYPE invalidateChildNode(cNode *c);

RET_TYPE createChildNode(uint16_t pAddr, cNode *c);
RET_TYPE createChildStartOfDataNode(uint16_t pAddr, cNode *c, uint8_t dataNodeCount);
void readChildNode(cNode *c, uint16_t childNodeAddress);
RET_TYPE updateChildNode(pNode *p, cNode *c, uint16_t pAddr, uint16_t cAddr);
RET_TYPE deleteChildNode(uint16_t pAddr, uint16_t cAddr);

void scanNodeUsage(void);


#endif /* NODE_MGMT_H_ */
