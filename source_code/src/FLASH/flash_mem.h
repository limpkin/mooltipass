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
/*
 * flash_mem.h
 *
 * Created: 08/12/2013 14:16:34
 *  Author: Mathieu Stephan
 */ 


#ifndef FLASH_MEM_H_
#define FLASH_MEM_H_

void sendDataToFlash(uint8_t nb_bytes_opcode, uint8_t* opcode, uint16_t nb_bytes, uint8_t* data_to_send_receive);
void writeCredentialBlock(uint16_t page_number, uint8_t block_id, uint8_t* buffer);
void readCredentialBlock(uint16_t page_number, uint8_t block_id, uint8_t* buffer);
RET_TYPE getFlashID(void);
RET_TYPE initFlash(void);

#endif /* FLASH_MEM_H_ */