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


#ifndef UTILS_H_
#define UTILS_H_ 


/* Prototypes */ 
uint8_t hm_uint8_strncmp(uint8_t* buffer1, uint8_t* buffer2, uint8_t nb_chars);
void hexachar_to_string(unsigned char c, char* string);
void char_to_string(unsigned char value, char* string);
void hm_str_cpy(char* source, char* dest, int nb_char);
void hexaint_to_string(unsigned int c, char* string);
void int_to_string(unsigned int value, char* string);
void clear_string(char* string, int nb_char);
unsigned char chr_strlen(char* string);
unsigned int int_strlen(char* string);
char numchar_to_char(unsigned char c);
uint16_t swap16(uint16_t val);

#endif /* UTILS_H_ */
