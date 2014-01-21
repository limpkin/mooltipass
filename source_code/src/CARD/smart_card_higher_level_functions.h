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
 * smart_card_higher_level_functions.h
 *
 * Created: 13/01/2014 23:07:01
 *  Author: Mathieu Stephan
 */ 


#ifndef SMART_CARD_HIGHER_LEVEL_FUNCTIONS_H_
#define SMART_CARD_HIGHER_LEVEL_FUNCTIONS_H_

uint8_t* read_fabrication_zone(uint8_t* buffer);
uint8_t* read_issuer_zone(uint8_t* buffer);
void write_issuer_zone(uint8_t* buffer);
uint8_t* read_security_code(uint8_t* buffer);
void write_security_code(uint8_t* buffer);
uint8_t* read_security_code_attemps_counter(uint8_t* buffer);
uint8_t* read_code_protected_zone(uint8_t* buffer);
void write_code_protected_zone(uint8_t* buffer);
uint8_t* read_application_zone1_erase_key(uint8_t* buffer);
void write_application_zone1_erase_key(uint8_t* buffer);
uint8_t* read_application_zone2_erase_key(uint8_t* buffer);
void write_application_zone2_erase_key(uint8_t* buffer);
uint8_t* read_memory_test_zone(uint8_t* buffer);
void write_memory_test_zone(uint8_t* buffer);
uint8_t* read_manufacturers_zone(uint8_t* buffer);
void write_manufacturers_zone(uint8_t* buffer);
void perform_card_reinit(void);
void write_manufacturers_fuse(void);
void write_issuers_fuse(void);
void print_smartcard_debug_info(void);
uint8_t get_number_of_security_code_tries_left(void);
RET_TYPE perform_card_mooltipass_transformation(void);
void set_application_zone1_authenticated_read_and_write_access(void);
void set_application_zone2_authenticated_read_and_write_access(void);

#endif /* SMART_CARD_HIGHER_LEVEL_FUNCTIONS_H_ */