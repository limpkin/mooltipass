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

#define NODE_TYPE_PARENT 0
#define NODE_TYPE_CHILD 1
#define NODE_TYPE_CHILD_DATA 2
#define NODE_TYPE_DATA 3

#define NODE_ADDR_NULL 0x0000
#define NODE_VBIT_VALID 0
#define NODE_VBIT_INVALID 1

#define NODE_F_TYPE_MASK 0xc000
#define NODE_F_TYPE_SHMT 14
#define NODE_F_TYPE_MASK_FINAL 0x0003

#define NODE_F_VALID_BIT_MASK 0x2000
#define NODE_F_VALID_BIT_SHMT 13
#define NODE_F_VALID_BIT_MASK_FINAL 0x0001

#define NODE_F_UID_MASK 0x00f0
#define NODE_F_UID_SHMT 4
#define NODE_F_UID_MASK_FINAL 0x000f

#define NODE_F_CRED_TYPE_MASK 0x000f
//#define NODE_F_CRED_TYPE_MASK 0
//#define NODE_F_CRED_TYPE_MASK_FINAL 0x000f

#define NODE_F_DATA_SEQ_NUM_MASK 0x00ff

#define NODE_ADDR_SHMT 3
#define NODE_ADDR_PAGE_MASK 0x1fff
#define NODE_ADDR_NODE_MASK 0x0007

#define NODE_PARENT_SIZE_OF_SERVICE 58

#define NODE_MAX_UID 16
#define NODE_MAX_CRED_TYPE 16
#define NODE_MAX_DATA 256

#define USER_MAX_FAV 16
#define USER_PROFILE_SIZE (2+(USER_MAX_FAV*4))

/*!
*  Node Deletion Policy ENUM 
*/
typedef enum
{
    DELETE_POLICY_WRITE_NOTHING = 1, /*!< Flip valid bit */
    DELETE_POLICY_WRITE_ZEROS = 0x00,   /*!< Write node with all 0's */
    DELETE_POLICY_WRITE_ONES = 0xff,    /*!< Write node with all 1's */
} deletePolicy;

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
    uint16_t firstParentNode;       /*!< The address of the users first parent node (read from flash. eg cache) */
    uint16_t nextFreeParentNode;    /*!< The address of the next parent node */
    uint16_t nextFreeChildNode;     /*!< The address of the next child or data node */
    
    // TODO - Cache favorites? 64 additional bytes
} mgmtHandle;

/*!
* Struct containing a parent node
*/
typedef struct __attribute__((packed)) parentNode {
    uint16_t flags;                 /*!< Parent node flags 
                                    * 15 dn 14-> Node type (Always 00 for Parent Node)
                                    * 13 dn 13 -> Valid Bit
                                    * 12 dn 8 -> RESERVED
                                    * 7 dn 4 -> User ID
                                    * 3 dn0 -> credential type UID
                                    */
    uint16_t prevParentAddress;     /*!< Previous parent node address (Alphabetically) */
    uint16_t nextParentAddress;     /*!< Next parent node address (Alphabetically) */
	uint16_t nextChildAddress;      /*!< Parent node first child address */
    uint8_t service[NODE_PARENT_SIZE_OF_SERVICE];            /*!< (ASCII) Text describing service (domain name eg hackaday.com). Used for sorting and searching. */
} pNode;

/*!
* Struct containing a child node
*/
typedef struct __attribute__((packed)) childNode {
    uint16_t flags;                 /*!< Child node flags (0b01 -> credential child node. 0b10 -> start of data sequence)
                                    * 15 dn 14-> Node type
                                    * 13 dn 13 -> Valid Bit
                                    * 12 dn 0 -> RESERVED
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

/*!
* Struct containing a data node
*
* Note: Requires plugin for support.  Up to 256 data nodes can be used in sequence
*/
typedef struct __attribute__((packed)) dataNode {
    uint16_t flags;                 /*!< Data node flags (Always 0b11 for Data Node)
                                    * 15 dn 14-> Node type
                                    * 13 dn 13 -> Valid Bit
                                    * 12 dn 8 -> RESERVED
                                    * 7 dn 0 -> Data node sequence number
                                    */
    uint16_t nextDataAddress;       /*!< Next data node in sequence */
    uint8_t data[128];              /*!< 128 bytes of Large Data Store */
} dNode;


/* Helper Functions (flags and address) */
uint8_t nodeTypeFromFlags(uint16_t flags);                              // Verified 1M, 2M, 4M, 8M, 16M, and 32M
void  nodeTypeToFlags(uint16_t *flags, uint8_t nodeType);               // Verified 1M, 2M, 4M, 8M, 16M, and 32M

uint8_t validBitFromFlags(uint16_t flags);                              // Verified 1M, 2M, 4M, 8M, 16M, and 32M
void validBitToFlags(uint16_t *flags, uint8_t vb);                      // Verified 1M, 2M, 4M, 8M, 16M, and 32M

uint8_t userIdFromFlags(uint16_t flags);                                // Verified 1M, 2M, 4M, 8M, 16M, and 32M
void userIdToFlags(uint16_t *flags, uint8_t uid);                       // Verified 1M, 2M, 4M, 8M, 16M, and 32M

uint8_t credentialTypeFromFlags(uint16_t flags);                        // Verified 1M, 2M, 4M, 8M, 16M, and 32M
void credentialTypeToFlags(uint16_t *flags, uint8_t credType);          // Verified 1M, 2M, 4M, 8M, 16M, and 32M

uint8_t dataNodeSequenceNumberFromFlags(uint16_t flags);                // Verified 1M, 2M, 4M, 8M, 16M, and 32M
void dataNodeSequenceNumberToFlags(uint16_t *flags, uint8_t sid);       // Verified 1M, 2M, 4M, 8M, 16M, and 32M

uint16_t pageNumberFromAddress(uint16_t addr);                          // Verified 1M, 2M, 4M, 8M, 16M, and 32M
uint8_t nodeNumberFromAddress(uint16_t addr);                           // Verified 1M, 2M, 4M, 8M, 16M, and 32M
uint16_t constructAddress(uint16_t pageNumber, uint8_t nodeNumber);     // Verified 1M, 2M, 4M, 8M, 16M, and 32M

/* Init Handle */
RET_TYPE initNodeManagementHandle(mgmtHandle *h, uint8_t userIdNum);    // Verified 1M, 2M, 4M, 8M, 16M, and 32M

/* User Memory Functions */
// TODO user init - needed?
RET_TYPE userProfileStartingOffset(uint8_t uid, uint16_t *page, uint16_t *pageOffset);  // Verified 1M, 2M, 4M, 8M, 16M, and 32M
RET_TYPE setStartingParent(mgmtHandle *h, uint16_t parentAddress);                      // Verified 1M, 2M, 4M, 8M, 16M, and 32M
RET_TYPE readStartingParent(mgmtHandle *h, uint16_t *parentAddress);                    // Verified 1M, 2M, 4M, 8M, 16M, and 32M

RET_TYPE setFav(mgmtHandle *h, uint8_t favId, uint16_t parentAddress, uint16_t childAddress); // TODO Implement later
RET_TYPE readFav(mgmtHandle *h, uint8_t favId, uint16_t *parentAddress, uint16_t *childAddress); // TODO Implement later

RET_TYPE scanNextFreeParentNode(mgmtHandle *h, uint16_t startingAddress);                   // Done.. Somewhat tested. Not all cases tested.

RET_TYPE createParentNode(mgmtHandle *h, pNode *p);                                        
RET_TYPE readParentNode(mgmtHandle *h, pNode *p, uint16_t parentNodeAddress);              // Done.  Semi-Tested
RET_TYPE updateParentNode(mgmtHandle *h, pNode *p, uint16_t parentNodeAddress);             // Done.  Semi-Tested
RET_TYPE deleteParentNode(mgmtHandle *h, uint16_t parentNodeAddress, deletePolicy policy);  //Done.. Will Need Tested
RET_TYPE invalidateParentNode(pNode *p);

#endif /* NODE_MGMT_H_ */