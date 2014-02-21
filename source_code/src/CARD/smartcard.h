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


#ifndef SMARTCARD_H_
#define SMARTCARD_H_

#include "../defines.h"
#include <stdint.h>

uint8_t* readSMC(uint8_t nb_bytes_total_read, uint8_t start_record_index, uint8_t* data_to_receive);
void writeSMC(uint16_t start_index_bit, uint16_t nb_bits, uint8_t* data_to_write);
void blowManufacturerNIssuerFuse(uint8_t bool_man_nissuer);
void eraseApplicationZone1NZone2SMC(uint8_t zone1_nzone2);
RET_TYPE securityValidationSMC(uint16_t code);
RET_TYPE firstDetectFunctionSMC(void);
RET_TYPE isCardPlugged(void);
void removeFunctionSMC(void);
void scanSMCDectect(void);
void initPortSMC(void);

#endif /* SMARTCARD_H_ */
