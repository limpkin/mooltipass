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
/*!	\file 	graphics.c
*	\brief	Graphical functions
*/
#include "../mooltipass.h"
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "bitmaps.h"


/*!	\fn		clear_screen()
*	\brief	Clear the screen
*/
void clear_screen()
{
	unsigned char i,j;
	
	Set_Column_Address(OLED_Shift, OLED_Shift + OLED_Max_Column);
	Set_Row_Address(0, OLED_Max_Row);
	Set_Write_RAM();

	for(i = 0; i <= OLED_Max_Row; i++)
	{
		for(j = 0; j <= OLED_Max_Column; j++)
		{
			oled_write_cont_data(0x00);
			oled_write_cont_data(0x00);
		}
	}

	oled_relieve_slave_select();
}

/*!	\fn		Fill_Block(unsigned char Data, unsigned char Data2, unsigned char column_start, unsigned char end_column, unsigned char row_start, unsigned char row_end)
*	\brief	Show regular pattern (partial or full screen)
*	\param	Data			The pattern
*	\param	Data2			The pattern
*	\param	column_start	Column Address of Start
*	\param	end_column		Column Address of End (Total Columns Divided by 4)
*	\param	row_start		Row Address of Start
*	\param	row_end			Row Address of End
*/
void Fill_Block(unsigned char Data, unsigned char Data2, unsigned char column_start, unsigned char end_column, unsigned char row_start, unsigned char row_end)
{
	unsigned char i,j;
	
	Set_Column_Address(OLED_Shift + column_start, OLED_Shift + end_column);
	Set_Row_Address(row_start, row_end);
	Set_Write_RAM();

	for(i=0;i<(row_end-row_start+1);i++)
	{
		for(j=0;j<(end_column-column_start+1);j++)
		{
			oled_write_cont_data(Data);
			oled_write_cont_data(Data2);
		}
	}

	oled_relieve_slave_select();
}

/*!	\fn		draw_screen_frame()
*	\brief	Display a frame around the screen
*/
void draw_screen_frame()
{
	Fill_Block(0xFF, 0xFF, 0, OLED_Max_Column, 0, 0);
	Fill_Block(0xFF, 0xFF, 0, OLED_Max_Column, OLED_Max_Row, OLED_Max_Row);
	Fill_Block(0xF0, 0x00, 0, 0, 1, OLED_Max_Row - 1);
	Fill_Block(0x00, 0x0F, OLED_Max_Column, OLED_Max_Column, 1, OLED_Max_Row - 1);
}

/*!	\fn		lcd_display_grayscale()
*	\brief	Display a grayscale on the display
*/
void lcd_display_grayscale()
{
	unsigned char i;
	unsigned char j;
	unsigned char pixel;

	for(i = 0; i < 2; i++)
	{
		for(j = 0; j < 16; j++)
		{
			if(j != 14)
				pixel = (j << 4) + j;
			else
				pixel = 0xFF;

			Fill_Block(~pixel, ~pixel, j + (i << 5), j + (i << 5), 0, OLED_Max_Row);
			Fill_Block(~pixel, ~pixel, (31 - j) + (i << 5), (31 - j) + (i << 5), 0, OLED_Max_Row);
		}
		for(j = 0; j < 16; j++)
		{
			if(j != 14)
				pixel = (j << 4) + j;
			else
				pixel = 0xFF;

			Fill_Block(~pixel, ~pixel, j + (i << 5), (31 - j) + (i << 5), j << 1, (j << 1) + 1);
			Fill_Block(~pixel, ~pixel, j + (i << 5), (31 - j) + (i << 5), 62 - (j << 1), 62 - (j << 1) + 1);
		}
	}
}

/*!	\fn		Show_Font57(unsigned char data, unsigned char reverse, unsigned char x, unsigned char y)
*	\brief	Display a char on the display
*	\param	data		The ascii char
*	\param	reverse		To reverse the color
*	\param	x			Start x column address
*	\param	y			Start y row address
*/
void Show_Font57(unsigned char data, unsigned char reverse, unsigned char x, unsigned char y)
{
	unsigned char *Src_Pointer;
	unsigned char i,j,Font,MSB_1,LSB_1,MSB_2,LSB_2;

	if(data < 0x20 || data > 0x8C)
		return;

	Src_Pointer=&Ascii_1[(data-0x20)][0];
	Set_Remap_Format(0x15);

	for(i=0;i<=1;i++)
	{
		MSB_1=*Src_Pointer;
		Src_Pointer++;
		if(i == 1)
		{
			LSB_1=0x00;
			MSB_2=0x00;
			LSB_2=0x00;
		}
		else
		{
			LSB_1=*Src_Pointer;
			Src_Pointer++;
			MSB_2=*Src_Pointer;
			Src_Pointer++;
			LSB_2=*Src_Pointer;
			Src_Pointer++;
		}
 		Set_Column_Address(OLED_Shift+x,OLED_Shift+x);
		Set_Row_Address(y,y+7);
		Set_Write_RAM();

		for(j = 0; j < 8; j++)
		{
			if(!reverse)
			{
				if(MSB_1 & (1 << j))
					Font = 0xF0;
				else
					Font = 0x00;
				if(LSB_1 &  (1 << j))
					Font |= 0x0F;

				oled_write_data(Font);

				if(MSB_2 &  (1 << j))
					Font = 0xF0;
				else
					Font = 0x00;
				if(LSB_2 &  (1 << j))
					Font |= 0x0F;

				oled_write_data(Font);
			}
			else
			{
				if(MSB_1 & (1 << j))
					Font = 0x0F;
				else
					Font = 0xFF;
				if(LSB_1 &  (1 << j))
					Font &= 0xF0;

				oled_write_data(Font);

				if(MSB_2 &  (1 << j))
					Font = 0x0F;
				else
					Font = 0xFF;
				if(LSB_2 &  (1 << j))
					Font &= 0xF0;

				oled_write_data(Font);
			}
		}
		x++;
	}

	Set_Remap_Format(0x14);
}

/*!	\fn		Show_String(char* string, unsigned char reverse, unsigned char x, unsigned char y)
*	\brief	Display a string on the display
*	\param	string	The string
*	\param	reverse	If color reversed
*	\param	x		Start x column address
*	\param	y		Start y row address
*/
void Show_String(char* string, unsigned char reverse, unsigned char x, unsigned char y)
{
	while(*string)
	{
		Show_Font57((unsigned char)*string++, reverse, x, y);
		x+=2;
	}
}

/*!	\fn		Show_Nb_Chars(char* string, unsigned char reverse, unsigned char x, unsigned char y, unsigned char nb_chars)
*	\brief	Display a string on the display
*	\param	string		The string
*	\param	reverse		If color reversed
*	\param	x			Start x column address
*	\param	y			Start y row address
*	\param	nb_chars	Number of chars we want to send
*/
void Show_Nb_Chars(char* string, unsigned char reverse, unsigned char x, unsigned char y, unsigned char nb_chars)
{
	unsigned char i = 0;

	for(i = 0; i < nb_chars; i++)
	{
		Show_Font57((unsigned char)*string++, reverse, x, y);
		x+=2;
	}
}

/*!	\fn		display_picture(unsigned char picture, unsigned char x, unsigned char y)
*	\brief	Display a picture on the display
*	\param	picture	ID of the picture
*	\param	x		Start x pixel
*	\param	y		Start y pixel
*/
void display_picture(unsigned char picture, unsigned char x, unsigned char y)
{
	unsigned char width, height;
	unsigned char* src_pointer;
	unsigned char i,j;

	if(picture == HACKADAY_BMP)
		src_pointer = hackaday;
	else		
		return;
		
	width = pgm_read_byte(src_pointer++);
	height = pgm_read_byte(src_pointer++);
	
	Set_Column_Address(OLED_Shift + (x >> 2), OLED_Shift + (width  >> 2) + (x >> 2) - 1);	
	Set_Row_Address(y, y + height - 1);
	Set_Write_RAM();

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < (unsigned char)(width >> 1); j++)
		{
			oled_write_cont_data(pgm_read_byte(src_pointer++));
		}
	}

	oled_relieve_slave_select();
}
