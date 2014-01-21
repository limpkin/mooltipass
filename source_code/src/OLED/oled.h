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
 * oled.h
 *
 * Created: 08/12/2013 20:47:02
 *  Author: Mathieu Stephan
 */ 


#ifndef OLED_H_
#define OLED_H_

void Set_Partial_Display_On(unsigned char start_row, unsigned char end_row);
void Set_Column_Address(unsigned char start_addr, unsigned char end_addr);
void Set_Row_Address(unsigned char start_addr, unsigned char end_addr);
void Set_Master_Current(unsigned char master_current);
void Set_Contrast_Current(unsigned char contrast);
void Set_Remap_Format(unsigned char format);
void Set_Display_Mode(unsigned char mode);
void lcd_fill_ram(unsigned char data);
void Set_Partial_Display_Off();
void init_spi_and_lcd_ports();
void Set_Write_RAM();
void init_oled_screen(void);
void oled_relieve_slave_select();
void oled_write_data(uint8_t byte);
void oled_write_command(uint8_t byte);
void oled_write_cont_data(uint8_t byte);

#endif /* OLED_H_ */