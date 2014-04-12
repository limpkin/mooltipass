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

/*!
*  Node Deletion Policy ENUM 
*/
typedef enum
{
    deletePolicyWriteNothing, /*!< Flip valid bit */
    deletePolicyWriteZeros,   /*!< Write node with all 0's */
    deletePolicyWriteOnes,    /*!< Write node with all 1's */
} deletePolicy;

/*!
* Struct containing Node Management Handle
*
* Note: Do not directly modify these fields.  Node Mgmt will manage this structure
*/
typedef struct nodeMgmtH
{
    uint16_t flags;
    /*
    15 dn 2 Free
    1 dn 1 -> is child contig
    0 dn 0 -> is parent contig
    */
    uint16_t parentNodeCount;       /*!< The count of parent nodes (determined @ init) */
    uint16_t firstParentNode;       /*!< The address of the first parent node */
    uint16_t nextFreeParentNode;    /*!< The address of the next parent node */
    uint16_t maxParentNodeAddress;  /*!< The max address of the stack of the last known parent node */
    
    uint16_t childNodeCount;        /*!< The count of child nodes (determined @ init) */
    uint16_t firstChildNode;        /*!< The address of the first child node */
    uint16_t nextFreeChildNode;     /*!< The address of the next child or data node */
    uint16_t maxChildNodeAddress;   /*!< The max address of the heap of the last known child or data node */
} mgmtHandle __attribute__((__packed__));

/*!
* Struct containing a parent node
*/
typedef struct parentNode {
    uint16_t flags;                 /*!< Parent node flags 
                                    * 15 dn 14-> Node type (Always 00 for Parent Node)
                                    * 13 dn 13 -> Valid Bit
                                    * 12 dn 8 -> RESERVED
                                    * 7 dn 4 -> User ID
                                    * 3 dn0 -> credential type UID
                                    */
    uint16_t nextChildAddress;      /*!< Parent node first child address */
    uint16_t prevParentAddress;     /*!< Previous parent node address (Alphabetically) */
    uint16_t nextParentAddress;     /*!< Next parent node address (Alphabetically) */
    uint8_t service[58];            /*!< (ASCII) Text describing service (domain name eg hackaday.com). Used for sorting and searching. */
} pNode __attribute__ ((__packed__));

/*!
* Struct containing a child node
*/
typedef struct childNode {
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
} cNode __attribute__ ((__packed__));

/*!
* Struct containing a data node
*
* Note: Requires plugin for support.  Up to 256 data nodes can be used in sequence
*/
typedef struct datadNode {
    uint16_t flags;                 /*!< Data node flags (Always 0b11 for Data Node)
                                    * 15 dn 14-> Node type
                                    * 13 dn 13 -> Valid Bit
                                    * 12 dn 8 -> RESERVED
                                    * 7 dn 0 -> Data node sequence number
                                    */
    uint16_t nextDataAddress;       /*!< Next data node in sequence */
    uint8_t data[128];              /*!< 128 bytes of Large Data Store */
} dNode __attribute__ ((__packed__));


RET_TYPE obtainNodeManagementHandle(mgmtHandle **h);

RET_TYPE scanNextFreeParentNode(mgmtHandle *h, uint16_t starting_address);
RET_TYPE createParentNode(mgmtHandle *h, pNode * p);
RET_TYPE readParentNode(mgmtHandle *h, pNode * p, uint16_t parentNodeAddress);
RET_TYPE updateParentNode(mgmtHandle *h, pNode *p, uint16_t parentNodeAddress);
RET_TYPE deleteParentNode(mgmtHandle *h, uint16_t parentNodeaddress, deletePolicy policy);

#endif /* NODE_MGMT_H_ */