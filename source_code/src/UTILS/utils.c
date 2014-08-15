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
/*! \file   utils.c
*   \brief  Useful functions
*/
#include "mooltipass.h"
#include "utils.h"


/*! \fn     swap16(uint16_t val)
*   \brief  Swap low and high bytes
*   \param  val The val
*   \return The val
*/
uint16_t swap16(uint16_t val)
{
    return ((val << 8) | (uint8_t)(val >> 8));
}

/*! \fn     numchar_to_char(unsigned char c)
*   \brief  Convert a char value (0 to 9) to be displayed
*   \param  c   The char
*   \return Displayable char
*/
char numchar_to_char(unsigned char c)
{
    return c + 0x30;
}

/*! \fn     hexachar_to_string(unsigned char c, char* string)
*   \brief  Convert a char to a string that we can display
*   \param  c   The char
*   \param  string  Pointer to the string in which we will write
*/
void hexachar_to_string(unsigned char c, char* string)
{
    unsigned char temp = c & 0x0F;

    if(temp > 9)
    {
        string[1] = temp + 0x37;
    }
    else
    {
        string[1] = temp + 0x30;
    }

    temp = (c >> 4) & 0x0F;

    if(temp > 9)
    {
        string[0] = temp + 0x37;
    }
    else
    {
        string[0] = temp + 0x30;
    }

    string[2] = 0x00;
}

/*! \fn     hexaint_to_string(unsigned int c, char* string)
*   \brief  Convert a int to a string that we can display
*   \param  c   The int
*   \param  string  Pointer to the string in which we will write
*/
void hexaint_to_string(unsigned int c, char* string)
{
    unsigned char temp = (unsigned char)(c >> 8);
    hexachar_to_string(temp, string);
    temp = (unsigned char)(c & 0x00FF);
    hexachar_to_string(temp, string + 2);
}

/*! \fn     chr_strlen(char* string)
*   \brief  Ported strlen function
*   \param  string  The string
*   \return The length
*/
unsigned char chr_strlen(char* string)                      // A light version of strlen()
{
    unsigned char i = 0;

    while(*string++)
    {
        i++;
    }

    return i;
}

/*! \fn     int_strlen(char* string)
*   \brief  Ported strlen function
*   \param  string  The string
*   \return The length
*/
unsigned int int_strlen(char* string)                       // A light version of strlen()
{
    unsigned int i = 0;

    while(*string++)
    {
        i++;
    }

    return i;
}

/*! \fn     clear_string(char* string, int nb_char)
*   \brief  Clear a string
*   \param  string  The string
*   \param  nb_char Length
*/
void clear_string(char* string, int nb_char)
{
    unsigned int i;

    for(i = 0; i < nb_char; i++)
    {
        string[i] = 0;
    }
}

/*! \fn     hm_str_cpy(char* source, char* dest, int nb_char)
*   \brief  Ported strcpy() with the args inverted
*   \param  source  Pointer to the source
*   \param  dest    Pointer to the destination
*   \param  nb_char Number of chars to copy
*/
void hm_str_cpy(char* source, char* dest, int nb_char)
{
    unsigned int i;

    for(i = 0; i < nb_char; i++)
    {
        dest[i] = source[i];
    }
}

/*! \fn     char_to_string(unsigned char value, char* string)
*   \brief  Convert a char to a string that we can display (numerical form)
*   \param  value   The char
*   \param  string  Pointer to the string in which we will write
*/
void char_to_string(unsigned char value, char* string)
{
    unsigned char index = 0;
    unsigned char div_rest;
    unsigned char temp;
    unsigned char i;

    if(value == 0)
    {
        string[index] = numchar_to_char(0);
        string[index + 1] = 0x00;
        return;
    }

    while(value != 0)
    {
        div_rest = (unsigned char)(value % 10);
        value = value / 10;
        string[index++] = numchar_to_char(div_rest);
    }

    for(i = 0; i < (index >> 1); i++)
    {
        temp = string[i];
        string[i] = string[index - i - 1];
        string[index - i - 1] = temp;
    }

    string[index] = 0x00;
}

/*! \fn     int_to_string(unsigned int value, char* string)
*   \brief  Convert a int to a string that we can display (numerical form)
*   \param  value   The int
*   \param  string  Pointer to the string in which we will write
*/
void int_to_string(unsigned int value, char* string)
{
    unsigned char index = 0;
    unsigned char div_rest;
    unsigned char temp;
    unsigned char i;

    if(value == 0)
    {
        string[index] = numchar_to_char(0);
        string[index + 1] = 0x00;
        return;
    }

    while(value != 0)
    {
        div_rest = (unsigned char)(value % 10);
        value = value / 10;
        string[index++] = numchar_to_char(div_rest);
    }

    for(i = 0; i < (index >> 1); i++)
    {
        temp = string[i];
        string[i] = string[index - i - 1];
        string[index - i - 1] = temp;
    }

    string[index] = 0x00;
}

/*! \fn		hm_uint8_strncmp(uint8_t* buffer1, uint8_t* buffer2, uint8_t nb_chars)
*   \brief  A simplified strncmp
*   \param  buffer1   First buffer to compare
*   \param  buffer2   Second buffer to compare
*   \param  nb_chars  Number of uint8 to compare
*/
uint8_t hm_uint8_strncmp(uint8_t* buffer1, uint8_t* buffer2, uint8_t nb_chars)
{
	uint8_t i = 0;
	
	for (i = 0; i < nb_chars; i++)
	{
		if (*buffer1++ != *buffer2++)
		{
			return i+1;
		}
	}
	
	return 0;
}