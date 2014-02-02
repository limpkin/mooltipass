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


#ifndef SMART_CARD_HIGHER_LEVEL_FUNCTIONS_H_
#define SMART_CARD_HIGHER_LEVEL_FUNCTIONS_H_

uint8_t* readSecurityCodeAttemptsCounters(uint8_t* buffer);
uint8_t* readApplicationZone1EraseKey(uint8_t* buffer);
uint8_t* readApplicationZone2EraseKey(uint8_t* buffer);
void writeApplicationZone2EraseKey(uint8_t* buffer);
void writeApplicationZone1EraseKey(uint8_t* buffer);
void setAuthenticatedReadWriteAccessToZone1(void);
void setAuthenticatedReadWriteAccessToZone2(void);
uint8_t* readCodeProtectedZone(uint8_t* buffer);
RET_TYPE transformBlankCardIntoMooltipass(void);
uint8_t* readManufacturerZone(uint8_t* buffer);
uint8_t getNumberOfSecurityCodeTriesLeft(void);
uint8_t* readFabricationZone(uint8_t* buffer);
void writeCodeProtectedZone(uint8_t* buffer);
uint8_t* readMemoryTestZone(uint8_t* buffer);
void writeManufacturerZone(uint8_t* buffer);
uint8_t* readSecurityCode(uint8_t* buffer);
void writeMemoryTestZone(uint8_t* buffer);
uint8_t* readIssuerZone(uint8_t* buffer);
void writeSecurityCode(uint8_t* buffer);
void writeIssuerZone(uint8_t* buffer);
void printSMCDebugInfoToScreen(void);
void writeManufacturerFuse(void);
void write_issuers_fuse(void);
void resetBlankCard(void);

#endif /* SMART_CARD_HIGHER_LEVEL_FUNCTIONS_H_ */