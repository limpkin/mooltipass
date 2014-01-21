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
 * smartcard.h
 *
 * Created: 08/12/2013 16:50:23
 *  Author: Mathieu Stephan
 */ 


#ifndef SMARTCARD_H_
#define SMARTCARD_H_

uint8_t* read_data_from_smartcard(uint8_t nb_bytes_total_read, uint8_t start_record_index, uint8_t* data_to_receive);
void write_to_smartcard(uint16_t start_index_bit, uint16_t nb_bits, uint8_t* data_to_write);
void erase_application_zone1_nzone2(uint8_t zone1_nzone2);
void blow_man_nissuer_fuse(uint8_t bool_man_nissuer);
RET_TYPE security_code_validation(uint16_t code);
RET_TYPE smartcard_detection_functions(void);
void smartcard_removal_functions(void);
void scan_smartcard_detect(void);
void init_smartcard_port(void);
RET_TYPE is_card_plugged(void);

#endif /* SMARTCARD_H_ */