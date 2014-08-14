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
/*!  \file     stack.c
*    \brief    Stack Usage analyzer
*    Created:  09/6/2014
*    Author:   Miguel A. Borrego
*/
#include "stack.h"

/* External var, end of known static RAM (to be filled by linker) */
extern uint8_t _end;
/* External var, addr of bottom of stack (usually located at end of RAM)*/
extern uint8_t __stack;
/* Magic word to be placed between end of ram and stack */
#define STACK_INIT 0xB00B5

/*! \fn     static void stackInit(void)
*   \brief  Initializes all stack area with STACK_INIT word
*           The function location is compiler and linker dependant, 
*           it's placed in .init1 section, so it will be placed after
*           .init() section. For more information:
*           http://www.nongnu.org/avr-libc/user-manual/mem_sections.html
*
*                   &_end                    &__stack
*            -----------------------------------
*           | STATIC  |       STACK             |
*           |  RAM    |                         |
*            -----------------------------------    
*          0x100                               0xAFF
* 
*   \param  void
*   \return void
*/
#ifdef STACK_DEBUG
void stackInit(void) __attribute__ ((naked)) __attribute__ ((section (".init1")));

void stackInit(void)
{
    uint32_t *p = (uint32_t*)&_end; /* &_end end of static RAM (to be filled by linker) */

    while(p <= (uint32_t*)&__stack) /* &__stack addr of bottom of stack */
    {
        *p++ = STACK_INIT;
    }
}
#endif

/*! \fn     uint16_t stackFree(void)
*   \brief  Counts how many bytes of stack have not been overwritten
*   \param  void
*   \return number of bytes not overwritten by stack
*/
uint16_t stackFree(void)
{
    uint32_t *p = (uint32_t*)&_end;
    uint16_t words  = 0;
    while( (p <= (uint32_t*)&__stack) && (*p == STACK_INIT) )
    {
        p++;
	words++;
    }

    return words*sizeof(uint32_t);
}
