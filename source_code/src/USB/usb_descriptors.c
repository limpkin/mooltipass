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
/*! \file   usb_descriptors.c
 *  \brief  USB descriptors
 *  Copyright [2014] [Mathieu Stephan]
 */
#include "usb_descriptors.h"
#include "usb.h"


// Device descriptor
static const uint8_t PROGMEM device_descriptor[] =
{
    18,                                 // bLength
    1,                                  // bDescriptorType
    0x00, 0x02,                         // bcdUSB, USB 2.0
    0,                                  // bDeviceClass, see interface descriptors
    0,                                  // bDeviceSubClass
    0,                                  // bDeviceProtocol
    ENDPOINT0_SIZE,                     // bMaxPacketSize0
    LSB(VENDOR_ID), MSB(VENDOR_ID),     // idVendor
    LSB(PRODUCT_ID), MSB(PRODUCT_ID),   // idProduct
    0x00, 0x01,                         // bcdDevice
    1,                                  // iManufacturer
    2,                                  // iProduct
    0,                                  // iSerialNumber
    1                                   // bNumConfigurations
};

// Raw HID descriptor
static const uint8_t PROGMEM rawhid_hid_report_desc[] =
{
    0x06, LSB(RAWHID_USAGE_PAGE), MSB(RAWHID_USAGE_PAGE),
    0x0A, LSB(RAWHID_USAGE), MSB(RAWHID_USAGE),
    0xA1, 0x01,                         // Collection 0x01
    0x75, 0x08,                         // report size = 8 bits
    0x15, 0x00,                         // logical minimum = 0
    0x26, 0xFF, 0x00,                   // logical maximum = 255
    0x95, RAWHID_TX_SIZE,               // report count
    0x09, 0x01,                         // usage
    0x81, 0x02,                         // Input (array)
    0x95, RAWHID_RX_SIZE,               // report count
    0x09, 0x02,                         // usage
    0x91, 0x02,                         // Output (array)
    0xC0                                // end collection
};

// Keyboard HID descriptor, Keyboard Protocol 1, HID 1.11 spec, Appendix B, page 59-60
static const uint8_t PROGMEM keyboard_hid_report_desc[] =
{
    0x05, 0x01,                         // Usage Page (Generic Desktop),
    0x09, 0x06,                         // Usage (Keyboard),
    0xA1, 0x01,                         // Collection (Application),
    0x75, 0x01,                         //   Report Size (1),
    0x95, 0x08,                         //   Report Count (8),
    0x05, 0x07,                         //   Usage Page (Key Codes),
    0x19, 0xE0,                         //   Usage Minimum (224),
    0x29, 0xE7,                         //   Usage Maximum (231),
    0x15, 0x00,                         //   Logical Minimum (0),
    0x25, 0x01,                         //   Logical Maximum (1),
    0x81, 0x02,                         //   Input (Data, Variable, Absolute), ;Modifier byte
    0x95, 0x01,                         //   Report Count (1),
    0x75, 0x08,                         //   Report Size (8),
    0x81, 0x03,                         //   Input (Constant),                 ;Reserved byte
    0x95, 0x05,                         //   Report Count (5),
    0x75, 0x01,                         //   Report Size (1),
    0x05, 0x08,                         //   Usage Page (LEDs),
    0x19, 0x01,                         //   Usage Minimum (1),
    0x29, 0x05,                         //   Usage Maximum (5),
    0x91, 0x02,                         //   Output (Data, Variable, Absolute), ;LED report
    0x95, 0x01,                         //   Report Count (1),
    0x75, 0x03,                         //   Report Size (3),
    0x91, 0x03,                         //   Output (Constant),                 ;LED report padding
    0x95, 0x06,                         //   Report Count (6),
    0x75, 0x08,                         //   Report Size (8),
    0x15, 0x00,                         //   Logical Minimum (0),
    0x25, 0xff,                         //   Logical Maximum(255), was 0x68 (104) before
    0x05, 0x07,                         //   Usage Page (Key Codes),
    0x19, 0x00,                         //   Usage Minimum (0),
    0x29, 0xe7,                         //   Usage Maximum (231), was 0x68 (104) before
    0x81, 0x00,                         //   Input (Data, Array),
    0xc0                                // End Collection
};

#define CONFIG1_DESC_SIZE        (9+9+9+7+7+9+9+7)
#define RAWHID_HID_DESC_OFFSET   (9+9)
#define KEYBOARD_HID_DESC_OFFSET (9+9+9+7+7+9)

// Configuration descriptor
static const uint8_t PROGMEM config1_descriptor[CONFIG1_DESC_SIZE] =
{
    // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
    9,                                  // bLength;
    2,                                  // bDescriptorType;
    LSB(CONFIG1_DESC_SIZE),             // wTotalLength
    MSB(CONFIG1_DESC_SIZE),
    2,                                  // bNumInterfaces
    1,                                  // bConfigurationValue
    0,                                  // iConfiguration
    0x80,                               // bmAttributes
    50,                                 // bMaxPower

    // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
    9,                                  // bLength
    4,                                  // bDescriptorType
    RAWHID_INTERFACE,                   // bInterfaceNumber
    0,                                  // bAlternateSetting
    2,                                  // bNumEndpoints
    0x03,                               // bInterfaceClass (0x03 = HID)
    0x00,                               // bInterfaceSubClass (0x01 = Boot)
    0x00,                               // bInterfaceProtocol (0x01 = Keyboard)
    0,                                  // iInterface

    // HID interface descriptor, HID 1.11 spec, section 6.2.1
    9,                                  // bLength
    0x21,                               // bDescriptorType
    0x11, 0x01,                         // bcdHID
    0,                                  // bCountryCode
    1,                                  // bNumDescriptors
    0x22,                               // bDescriptorType
    sizeof(rawhid_hid_report_desc),     // wDescriptorLength
    0,

    // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,                                  // bLength
    5,                                  // bDescriptorType
    RAWHID_RX_ENDPOINT,                 // bEndpointAddress
    0x03,                               // bmAttributes (0x03=intr)
    RAWHID_RX_SIZE, 0,                  // wMaxPacketSize
    1,                                  // bInterval

    // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,                                  // bLength
    5,                                  // bDescriptorType
    RAWHID_TX_ENDPOINT | 0x80,          // bEndpointAddress
    0x03,                               // bmAttributes (0x03=intr)
    RAWHID_TX_SIZE, 0,                  // wMaxPacketSize
    1,                                  // bInterval

    // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
    9,                                  // bLength
    4,                                  // bDescriptorType
    KEYBOARD_INTERFACE,                 // bInterfaceNumber
    0,                                  // bAlternateSetting
    1,                                  // bNumEndpoints
    0x03,                               // bInterfaceClass (0x03 = HID)
    0x01,                               // bInterfaceSubClass (0x01 = Boot)
    0x01,                               // bInterfaceProtocol (0x01 = Keyboard)
    0,                                  // iInterface

    // HID interface descriptor, HID 1.11 spec, section 6.2.1
    9,                                  // bLength
    0x21,                               // bDescriptorType
    0x11, 0x01,                         // bcdHID
    0,                                  // bCountryCode
    1,                                  // bNumDescriptors
    0x22,                               // bDescriptorType
    sizeof(keyboard_hid_report_desc),   // wDescriptorLength
    0,

    // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,                                  // bLength
    5,                                  // bDescriptorType
    KEYBOARD_ENDPOINT | 0x80,           // bEndpointAddress
    0x03,                               // bmAttributes (0x03=intr)
    KEYBOARD_SIZE, 0,                   // wMaxPacketSize
    1                                   // bInterval
};

// USB strings
static const usb_string_descriptor_struct_t PROGMEM string0 =
{
    4,
    3,
    {0x0409}
};
static const usb_string_descriptor_struct_t PROGMEM string1 =
{
    sizeof(STR_MANUFACTURER),
    3,
    STR_MANUFACTURER
};
static const usb_string_descriptor_struct_t PROGMEM string2 =
{
    sizeof(STR_PRODUCT),
    3,
    STR_PRODUCT
};

// This table defines which descriptor data is sent for each specific request from the host (in wValue and wIndex).
const descriptor_list_struct_t PROGMEM descriptor_list[9] =
{
    {0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
    {0x0200, 0x0000, config1_descriptor, sizeof(config1_descriptor)},
    {0x2200, RAWHID_INTERFACE, rawhid_hid_report_desc, sizeof(rawhid_hid_report_desc)},
    {0x2100, RAWHID_INTERFACE, config1_descriptor+RAWHID_HID_DESC_OFFSET, 9},
    {0x2200, KEYBOARD_INTERFACE, keyboard_hid_report_desc, sizeof(keyboard_hid_report_desc)},
    {0x2100, KEYBOARD_INTERFACE, config1_descriptor+KEYBOARD_HID_DESC_OFFSET, 9},
    {0x0300, 0x0000, (const uint8_t *)&string0, 4},
    {0x0301, 0x0409, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
    {0x0302, 0x0409, (const uint8_t *)&string2, sizeof(STR_PRODUCT)}
};
