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
/*! \file   functional_testing.h
 *  \brief  Functional testing functions
 *  Copyright [2016] [Mathieu Stephan]
 */ 


#ifndef FUNCTIONAL_TESTING_H_
#define FUNCTIONAL_TESTING_H_

RET_TYPE electricalJumpToBootloaderCondition(void);
void mooltipassStandardElectricalTest(uint8_t fuse_ok);
void mooltipassMiniFunctionalTest(uint8_t flash_init_result, uint8_t mini_inputs_result);
void mooltipassStandardFunctionalTest(uint16_t current_bootkey_val, uint8_t flash_init_result, uint8_t touch_init_result, uint8_t fuse_ok);

#endif