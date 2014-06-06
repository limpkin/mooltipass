/*
 * usb_descriptors.h
 *
 * Created: 01/06/2014 17:50:31
 *  Author: limpkin
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