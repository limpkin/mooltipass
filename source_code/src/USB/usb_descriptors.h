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
/*! \file   usb_descriptors.h
 *  \brief  USB descriptors
 *  Copyright [2014] [Mathieu Stephan]
 */


#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include <stdint.h>

// Typedef for USB string
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    int16_t wString[];
} usb_string_descriptor_struct_t;

// Type def for descriptor list
typedef struct
{
    uint16_t        wValue;
    uint16_t        wIndex;
    const uint8_t*  addr;
    uint8_t         length;
} descriptor_list_struct_t;

// Array containing all of our descriptors
extern const descriptor_list_struct_t descriptor_list[9];

// Number of descriptors we have
#define NUM_DESC_LIST (sizeof(descriptor_list)/sizeof(descriptor_list_struct_t))

#endif /* USB_DESCRIPTORS_H_ */