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
/* \file    version.h
 * \brief   Methods for retrieving version information
 *  Created: 13/08/2014 19:35:00
 *  Author: Josh Watts
 */
#ifndef VERSION_H_
#define VERSION_H_

#include <avr/io.h>

/* \brief Copies up to len bytes of the version string to dst, terminated with NULL character
 * \param dst Pointer to where memory where version string will be copied. Must not be NULL.
 * \param len Maximum number of bytes to copy, including NULL character
 * \return Returns number of bytes copied
 */
uint8_t getVersion(char *dst, uint8_t len);

#endif /* VERSION_H_ */
