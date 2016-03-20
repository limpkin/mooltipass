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
/* \file    defines.h
 * \brief   Project definitions
 *  Created: 11/01/2014 11:54:26
 *  Author: Mathieu Stephan
 */
#ifndef DEFINES_H_
#define DEFINES_H_

#include <avr/io.h>
#include <stdint.h>

/**************** DEFINES FIRMWARE ****************/
#define AES_KEY_LENGTH          256
#define FALSE                   0
#define TRUE                    (!FALSE)

/**************** C ENUMS ****************/
enum usb_com_return_t           {RETURN_COM_NOK = -1, RETURN_COM_TRANSF_OK = 0, RETURN_COM_TIMEOUT = 1};
enum timer_flag_t               {TIMER_EXPIRED = 0, TIMER_RUNNING = 1};

/**************** TYPEDEFS ****************/
typedef void (*bootloader_f_ptr_type)(void);
typedef int8_t RET_TYPE;

/**************** VERSION DEFINES ***************/
#ifndef MOOLTIPASS_VERSION
    #define MOOLTIPASS_VERSION "v1.1"
#endif

#endif /* DEFINES_H_ */
