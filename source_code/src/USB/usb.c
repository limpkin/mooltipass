/* Copyright (c) 2009 PJRC.COM, LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above description, website URL and copyright notice and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "usb_descriptors.h"
#include "usb_cmd_parser.h"
#include "timer_manager.h"
#include "hid_defines.h"
#include "defines.h"
#include "usb.h"
#include <string.h>
#include <stdio.h>

// Zero when we are not configured, non-zero when enumerated
static volatile uint8_t usb_configuration=0;

// These are a more reliable timeout than polling the frame counter (UDFNUML)
static volatile uint8_t rx_timeout_count=0;
static volatile uint8_t tx_timeout_count=0;

// Which modifier keys are currently pressed
// 1=left ctrl,    2=left shift,   4=left alt,    8=left gui
// 16=right ctrl, 32=right shift, 64=right alt, 128=right gui
uint8_t keyboard_modifier_keys=0;

// Which keys are currently pressed, up to 6 keys may be down at once
uint8_t keyboard_keys[6]={0,0,0,0,0,0};

// protocol setting from the host.  We use exactly the same report
// either way, so this variable only stores the setting since we
// are required to be able to report which setting is in use.
static uint8_t keyboard_protocol = 1;

// The idle configuration, how often we send the report to the host (ms * 4) even when it hasn't changed
static uint8_t keyboard_idle_config = 125;

// Count until idle timeout
static uint8_t keyboard_idle_count = 0;

// 1=num lock, 2=caps lock, 4=scroll lock, 8=compose, 16=kana
volatile uint8_t keyboard_leds = 0;

// Endpoint configuration table
static const uint8_t PROGMEM endpoint_config_table[] =
{
    1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(RAWHID_TX_SIZE) | RAWHID_TX_BUFFER,
    1, EP_TYPE_INTERRUPT_OUT, EP_SIZE(RAWHID_RX_SIZE) | RAWHID_RX_BUFFER,
    1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(KEYBOARD_SIZE) | KEYBOARD_BUFFER,
    0
};

/*! \fn     pluginMessageRetryDelay(void)
*   \brief  Delay between message retries
*/
void pluginMessageRetryDelay(void)
{
    // Delay will be between 0 & 1 ms due to the function
    timerBasedDelayMs(0);
}

/*! \fn     initUsb(void)
*   \brief  USB controller initialization
*/
void initUsb(void)
{
    HW_CONFIG();                    // enable regulator
    USB_FREEZE();                   // enable USB
    PLL_CONFIG();                   // config PLL
    while (!(PLLCSR & (1<<PLOCK))); // wait for PLL lock
    USB_CONFIG();                   // start USB clock
    UDCON = 0;                      // enable attach resistor
    usb_configuration = 0;          // usb not configured by default
    UDIEN = (1<<EORSTE)|(1<<SOFE);  // start USB
}

/*! \fn     isUsbConfigured(void)
*   \brief  Know if the PC has enumerated the mooltipass
*   \return 0 if not configured, any other value if so
*/
uint8_t isUsbConfigured(void)
{
    return usb_configuration;
}

// 
/*! \fn     usbKeyboardSend(void)
*   \brief  Send the contents of keyboard_keys and keyboard_modifier_keys
*   \return If we managed to send the keyboard keys
*/
RET_TYPE usbKeyboardSend(void)
{
    uint8_t i, intr_state, timeout;

    if (!usb_configuration)
    {
        return RETURN_COM_NOK;
    }
    intr_state = SREG;
    cli();
    UENUM = KEYBOARD_ENDPOINT;
    timeout = UDFNUML + 50;
    while (1)
    {
        // are we ready to transmit?
        if (UEINTX & (1<<RWAL))
        {
            break;
        }
        SREG = intr_state;
        // has the USB gone offline?
        if (!usb_configuration)
        {
            return RETURN_COM_NOK;
        }
        // have we waited too long?
        if (UDFNUML == timeout)
        {
            return RETURN_COM_TIMEOUT;
        }
        // get ready to try checking again
        intr_state = SREG;
        cli();
        UENUM = KEYBOARD_ENDPOINT;
    }
    UEDATX = keyboard_modifier_keys;
    UEDATX = 0;
    for (i=0; i<6; i++)
    {
        UEDATX = keyboard_keys[i];
    }
    UEINTX = 0x3A;
    keyboard_idle_count = 0;
    SREG = intr_state;
    return RETURN_COM_TRANSF_OK;
}

/*! \fn     usbKeyboardPress(uint8_t key, uint8_t modifier)
*   \brief  Perform a single keystroke
*   \param  key         Key to send
*   \param  modifier    Modifier (alt, shift...)
*   \return If we managed to send the keyboard key
*/
RET_TYPE usbKeyboardPress(uint8_t key, uint8_t modifier)
{
    int8_t r;

    keyboard_modifier_keys = modifier;
    keyboard_keys[0] = key;
    r = usbKeyboardSend();
    if (r != RETURN_COM_TRANSF_OK)
    {
        return r;
    }        
    keyboard_modifier_keys = 0;
    keyboard_keys[0] = 0;
    return usbKeyboardSend();
}

/*! \fn     getKeyboardLeds(void)
*   \brief  Get keyboard leds
*   \return The keyboard leds
*/
uint8_t getKeyboardLeds(void)
{
    return keyboard_leds;
}

/*! \fn     usbRawHidRecv(uint8_t *buffer, uint8_t timeout)
*   \brief  Receive a packet, with timeout
*   \param  buffer    Pointer to the buffer to store received data
*   \param  timeout   Timeout in ms
*   \return RETURN_COM_TRANSF_OK or RETURN_COM_NOK or RETURN_COM_TIMEOUT
*/
RET_TYPE usbRawHidRecv(uint8_t *buffer, uint8_t timeout)
{
    uint8_t intr_state;
    uint8_t i = 0;

    // if we're not online (enumerated and configured), error
    if (!usb_configuration)
    {
        return RETURN_COM_NOK;
    }
    intr_state = SREG;
    cli();
    rx_timeout_count = timeout;
    UENUM = RAWHID_RX_ENDPOINT;
    // wait for data to be available in the FIFO
    while (1)
    {
        if (UEINTX & (1<<RWAL))
        {
            break;
        }
        SREG = intr_state;
        if (rx_timeout_count == 0)
        {
            return RETURN_COM_TIMEOUT;
        }
        if (!usb_configuration)
        {
            return RETURN_COM_NOK;
        }
        intr_state = SREG;
        cli();
        UENUM = RAWHID_RX_ENDPOINT;
    }
    for(i = 0; i < RAWHID_RX_SIZE; i++)
    {
        *buffer++ = UEDATX;
    }
    // release the buffer
    UEINTX = 0x6B;
    SREG = intr_state;
    return RETURN_COM_TRANSF_OK;
}


/*! \fn     ISR(USB_GEN_vect)
*   \brief  USB Device Interrupt - handle all device-level events
*           the transmit buffer flushing is triggered by the start of frame
*/
ISR(USB_GEN_vect)
{
    uint8_t intbits, t;

    intbits = UDINT;
    UDINT = 0;
    if (intbits & (1<<EORSTI))
    {
        UENUM = 0;
        UECONX = 1;
        UECFG0X = EP_TYPE_CONTROL;
        UECFG1X = EP_SIZE(ENDPOINT0_SIZE) | EP_SINGLE_BUFFER;
        UEIENX = (1<<RXSTPE);
        usb_configuration = 0;
    }
    if ((intbits & (1<<SOFI)) && usb_configuration)
    {
        t = rx_timeout_count;
        if (t)
        {
            rx_timeout_count = --t;
        }            
        t = tx_timeout_count;
        if (t)
        {
            tx_timeout_count = --t;
        }            
    }
}

// Misc functions to wait for ready and send/receive packets
static inline void usb_wait_in_ready(void)
{
    while (!(UEINTX & (1<<TXINI))) ;
}
static inline void usb_send_in(void)
{
    UEINTX = ~(1<<TXINI);
}
static inline void usb_wait_receive_out(void)
{
    while (!(UEINTX & (1<<RXOUTI))) ;
}
static inline void usb_ack_out(void)
{
    UEINTX = ~(1<<RXOUTI);
}

/*! \fn     ISR(USB_COM_vect)
*   \brief  USB Endpoint Interrupt - endpoint 0 is handled here.  The
*           other endpoints are manipulated by the user-callable
*           functions, and the start-of-frame interrupt.
*/
ISR(USB_COM_vect)
{
    uint8_t intbits;
    const uint8_t *list;
    const uint8_t *cfg;
    uint8_t i, n, len, en;
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
    uint16_t desc_val;
    const uint8_t *desc_addr;
    uint8_t desc_length;

    UENUM = 0;
    intbits = UEINTX;
    if (intbits & (1<<RXSTPI))
    {
        bmRequestType = UEDATX;
        bRequest = UEDATX;
        wValue = UEDATX;
        wValue |= (UEDATX << 8);
        wIndex = UEDATX;
        wIndex |= (UEDATX << 8);
        wLength = UEDATX;
        wLength |= (UEDATX << 8);
        UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI));

        if (bRequest == GET_DESCRIPTOR)
        {
            list = (const uint8_t *)descriptor_list;
            for (i=0; ; i++)
            {
                if (i >= NUM_DESC_LIST)
                {
                    UECONX = (1<<STALLRQ)|(1<<EPEN);  //stall
                    return;
                }
                desc_val = pgm_read_word(list);
                if (desc_val != wValue)
                {
                    list += sizeof(descriptor_list_struct_t);
                    continue;
                }
                list += 2;
                desc_val = pgm_read_word(list);
                if (desc_val != wIndex)
                {
                    list += sizeof(descriptor_list_struct_t)-2;
                    continue;
                }
                list += 2;
                desc_addr = (const uint8_t *)pgm_read_word(list);
                list += 2;
                desc_length = pgm_read_byte(list);
                break;
            }
            len = (wLength < 256) ? wLength : 255;
            if (len > desc_length)
            {
                len = desc_length;
            }
            do
            {
                // wait for host ready for IN packet
                do
                {
                    i = UEINTX;
                }
                while (!(i & ((1<<TXINI)|(1<<RXOUTI))));

                if (i & (1<<RXOUTI))
                {
                    return; // abort
                }
                // send IN packet
                n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
                for (i = n; i; i--)
                {
                    UEDATX = pgm_read_byte(desc_addr++);
                }
                len -= n;
                usb_send_in();
            }
            while (len || n == ENDPOINT0_SIZE);
            return;
        }
        if (bRequest == SET_ADDRESS)
        {
            usb_send_in();
            usb_wait_in_ready();
            UDADDR = wValue | (1<<ADDEN);
            return;
        }
        if (bRequest == SET_CONFIGURATION && bmRequestType == 0)
        {
            usb_configuration = wValue;
            usb_send_in();
            cfg = endpoint_config_table;
            for (i=1; i<5; i++)
            {
                UENUM = i;
                en = pgm_read_byte(cfg++);
                UECONX = en;
                if (en)
                {
                    UECFG0X = pgm_read_byte(cfg++);
                    UECFG1X = pgm_read_byte(cfg++);
                }
            }
            UERST = 0x1E;
            UERST = 0;
            return;
        }
        if (bRequest == GET_CONFIGURATION && bmRequestType == 0x80)
        {
            usb_wait_in_ready();
            UEDATX = usb_configuration;
            usb_send_in();
            return;
        }
        if (bRequest == GET_STATUS)
        {
            usb_wait_in_ready();
            i = 0;
            if (bmRequestType == 0x82)
            {
                UENUM = wIndex;
                if (UECONX & (1<<STALLRQ))
                {
                    i = 1;
                }
                UENUM = 0;
            }
            UEDATX = i;
            UEDATX = 0;
            usb_send_in();
            return;
        }
        if ((bRequest == CLEAR_FEATURE || bRequest == SET_FEATURE) && bmRequestType == 0x02 && wValue == 0)
        {
            i = wIndex & 0x7F;
            if (i >= 1 && i <= MAX_ENDPOINT)
            {
                usb_send_in();
                UENUM = i;
                if (bRequest == SET_FEATURE)
                {
                    UECONX = (1<<STALLRQ)|(1<<EPEN);
                }
                else
                {
                    UECONX = (1<<STALLRQC)|(1<<RSTDT)|(1<<EPEN);
                    UERST = (1 << i);
                    UERST = 0;
                }
                return;
            }
        }
        if (wIndex == RAWHID_INTERFACE)
        {
            if (bmRequestType == 0xA1 && bRequest == HID_GET_REPORT)
            {
                len = RAWHID_TX_SIZE;
                do
                {
                    // wait for host ready for IN packet
                    do
                    {
                        i = UEINTX;
                    }
                    while (!(i & ((1<<TXINI)|(1<<RXOUTI))));

                    if (i & (1<<RXOUTI))
                    {
                        return; // abort
                    }
                    // send IN packet
                    n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
                    for (i = n; i; i--)
                    {
                        // just send zeros
                        UEDATX = 0;
                    }
                    len -= n;
                    usb_send_in();
                }
                while (len || n == ENDPOINT0_SIZE);
                return;
            }
            if (bmRequestType == 0x21 && bRequest == HID_SET_REPORT)
            {
                len = RAWHID_RX_SIZE;
                do
                {
                    n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
                    usb_wait_receive_out();
                    // ignore incoming bytes
                    usb_ack_out();
                    len -= n;
                }
                while (len);
                usb_wait_in_ready();
                usb_send_in();
                return;
            }
        }
        if (wIndex == KEYBOARD_INTERFACE)
        {
            if (bmRequestType == 0xA1)
            {
                if (bRequest == HID_GET_REPORT)
                {
                    usb_wait_in_ready();
                    UEDATX = keyboard_modifier_keys;
                    UEDATX = 0;
                    for (i=0; i<6; i++)
                    {
                        UEDATX = keyboard_keys[i];
                    }
                    usb_send_in();
                    return;
                }
                if (bRequest == HID_GET_IDLE)
                {
                    usb_wait_in_ready();
                    UEDATX = keyboard_idle_config;
                    usb_send_in();
                    return;
                }
                if (bRequest == HID_GET_PROTOCOL)
                {
                    usb_wait_in_ready();
                    UEDATX = keyboard_protocol;
                    usb_send_in();
                    return;
                }
            }
            if (bmRequestType == 0x21)
            {
                if (bRequest == HID_SET_REPORT)
                {
                    usb_wait_receive_out();
                    keyboard_leds = UEDATX;
                    usb_ack_out();
                    usb_send_in();
                    return;
                }
                if (bRequest == HID_SET_IDLE)
                {
                    keyboard_idle_config = (wValue >> 8);
                    keyboard_idle_count = 0;
                    usb_send_in();
                    return;
                }
                if (bRequest == HID_SET_PROTOCOL)
                {
                    keyboard_protocol = wValue;
                    usb_send_in();
                    return;
                }
            }
        }
    }
    UECONX = (1<<STALLRQ) | (1<<EPEN);  // stall
}

/*! \fn     pluginSendMessage_P(uint8_t cmd, uint8_t len, const char* str)
*   \brief  Send a message to the mooltipass plugin
*   \param  cmd command UID
*   \param  len message length
*   \param  str pointer to the string in FLASH
*   \return If we managed send the data
*/
RET_TYPE pluginSendMessage_P(uint8_t cmd, uint8_t len, const char* str)
{
    uint8_t buffer[RAWHID_TX_SIZE];
    uint8_t i = HID_DATA_START;
    uint8_t j;
    char ch;

    #ifdef USB_DEBUG_OUTPUT
    if ((cmd != CMD_DEBUG && cmd < CMD_EXPORT_FLASH))
    {
        if (len)
        {
            usbPrintf_P(PSTR("tx: cmd 0x%02x len %d data 0x%02x\n"), cmd, len, *str);
        }
        else
        {
            usbPrintf_P(PSTR("tx: cmd 0x%02x len %d\n"), cmd, len);
        }
    }
    #endif
    
    buffer[HID_LEN_FIELD] = len;
    buffer[HID_TYPE_FIELD] = cmd;

    for(j = 0; j < len; j++)
    {
        ch = pgm_read_byte(str++);
        buffer[i++] = ch;
        if (i == RAWHID_TX_SIZE)
        {
            i = 0;
            if(usbRawHidSend(buffer, USB_WRITE_TIMEOUT) != RETURN_COM_TRANSF_OK)
            {
                return RETURN_COM_NOK;
            }
        }
    }

    if (i != 0)
    {
        memset((void*)buffer + i, 0, RAWHID_TX_SIZE - i);
        if(usbRawHidSend(buffer, USB_WRITE_TIMEOUT) != RETURN_COM_TRANSF_OK)
        {
            return RETURN_COM_NOK;
        }
    }
    return RETURN_COM_TRANSF_OK;    
}

/*!
*   \brief  Wait for the TX fifo to be ready to accept data.
*   \param  intr_state   pointer to storage to save the interrupt state
*   \param  timeout   Timeout in ms
*   \retval RETURN_TRANSF_COM_OK fifo is ready, and interrupts are disabled
*   \retval RETURN_COM_TIMEOUT timeout waiting for fifo to be ready
*   \retval RETURN_COM_NOK USB not configured
*/
static RET_TYPE usbWaitFifoReady(uint8_t *intr_state, uint8_t timeout)
{
    // if we're not online (enumerated and configured), error
    if (!usb_configuration)
    {
        return RETURN_COM_NOK;
    }
    *intr_state = SREG;
    cli();
    tx_timeout_count = timeout;
    UENUM = RAWHID_TX_ENDPOINT;
    // wait for the FIFO to be ready to accept data
    while (1)
    {
        if (UEINTX & (1<<RWAL))
        {
            break;
        }
        SREG = *intr_state;
        if (tx_timeout_count == 0)
        {
            return RETURN_COM_TIMEOUT;
        }
        if (!usb_configuration)
        {
            return RETURN_COM_NOK;
        }
        *intr_state = SREG;
        cli();
        UENUM = RAWHID_TX_ENDPOINT;
    }
    return RETURN_COM_TRANSF_OK;
}


/*!
*   \brief  Send a packet, with timeout
*           If cmd is non-zero then the lenght and cmd byte are
*           sent first, followed by the buffer.
*           If cmd is zero, the buffer is sent as-is.
*   \param  cmd       optional command byte to send. Ignored if 0
*   \param  buffer    Pointer to the buffer to send data from
*   \param  buflen    amount of data to send from buffer
*   \param  timeout   Timeout in ms
*   \return RETURN_TRANSF_COM_OK or RETURN_COM_NOK or RETURN_COM_TIMEOUT
*/
RET_TYPE usbHidSend(uint8_t cmd, const void *buffer, uint8_t buflen, uint8_t timeout)
{
    uint8_t intr_state;
    int8_t res;
    int8_t rem = RAWHID_TX_SIZE - buflen;

    if (buflen > RAWHID_TX_SIZE)
    {
        return RETURN_COM_NOK;
    }


    res = usbWaitFifoReady(&intr_state, timeout);

    if (res != RETURN_COM_TRANSF_OK) 
    {
        return res;
    }

    if (cmd)
    {
        UEDATX = buflen;
        UEDATX = cmd;
    }

    // write bytes to the FIFO
    while (buflen--)
    {
        UEDATX = *(uint8_t *)buffer++;
    }

    // make up the remainder
    while (rem--)
    {
        UEDATX = 0;
    }

    // transmit it now
    UEINTX = 0x3A;
    SREG = intr_state;
    return RETURN_COM_TRANSF_OK;
}

/*!
*   \brief  Send a packet, with timeout
*           If cmd is non-zero then the lenght and cmd byte are
*           sent first, followed by the buffer.
*           If cmd is zero, the buffer is sent as-is.
*   \param  cmd       optional command byte to send. Ignored if 0
*   \param  buffer    Pointer to the data to send from flash
*   \param  buflen    number of bytes to send
*   \param  timeout   Timeout in ms
*   \return RETURN_TRANSF_COM_OK or RETURN_COM_NOK or RETURN_COM_TIMEOUT
*/
RET_TYPE usbHidSend_P(uint8_t cmd, const void *buffer, uint8_t buflen, uint8_t timeout)
{
    uint8_t intr_state;
    int8_t res;
    int8_t rem = RAWHID_TX_SIZE - buflen;

    if (buflen > RAWHID_TX_SIZE)
    {
        return RETURN_COM_NOK;
    }

    res = usbWaitFifoReady(&intr_state, timeout);

    if (res != RETURN_COM_TRANSF_OK) 
    {
        return res;
    }

    if (cmd)
    {
        UEDATX = buflen;
        UEDATX = cmd;
    }

    // write bytes to the FIFO
    while (buflen--)
    {
        UEDATX = pgm_read_byte(buffer++);
    }

    // make up the remainder
    while (rem--)
    {
        UEDATX = 0;
    }

    // transmit it now
    UEINTX = 0x3A;
    SREG = intr_state;
    return RETURN_COM_TRANSF_OK;
}

/*! \fn     usbRawHidSend(uint8_t *buffer, uint8_t timeout)
*   \brief  Send a packet, with timeout
*   \param  buffer    Pointer to the buffer to store received data
*   \param  timeout   Timeout in ms
*   \return RETURN_TRANSF_COM_OK or RETURN_COM_NOK or RETURN_COM_TIMEOUT
*/
int8_t usbRawHidSend(uint8_t* buffer, uint8_t timeout)
{
    return usbHidSend(0, buffer, RAWHID_TX_SIZE, timeout);
}

/*!
*   \brief  Send a message to the mooltipass plugin
*   \param  cmd command ID
*   \param  size number of bytes in message
*   \param  msg pointer to the message data in RAM
*   \retval RETURN_COM_TRANSF_OK success
*   \retval RETURN_COM_NOK failed to send
*/
RET_TYPE usbSendMessage(uint8_t cmd, uint8_t size, const void *msg)
{
    uint8_t chunk = cmd ? RAWHID_TX_SIZE-2 : RAWHID_TX_SIZE;

    /* Send message in chunks */
    while (size >= chunk)
    {
        if (usbHidSend(cmd, msg, chunk, USB_WRITE_TIMEOUT) != RETURN_COM_TRANSF_OK)
        {
            return RETURN_COM_NOK;
        }
        msg += chunk;
        size -= chunk;
        if (cmd)
        {
            chunk += 2;
            cmd = 0;
        }
    }
    if (size || cmd)
    {
        if (usbHidSend(cmd, msg, size, USB_WRITE_TIMEOUT) != RETURN_COM_TRANSF_OK)
        {
            return RETURN_COM_NOK;
        }
    }
    return RETURN_COM_TRANSF_OK;
}

/*!
*   \brief  Send a message to the mooltipass plugin
*   \param  cmd command ID
*   \param  size number of bytes in message
*   \param  msg pointer to the message data in program memory
*   \retval RETURN_COM_TRANSF_OK success
*   \retval RETURN_COM_NOK failed to send
*/
RET_TYPE usbSendMessage_P(uint8_t cmd, uint8_t size, const void *msg)
{
    uint8_t chunk = cmd ? RAWHID_TX_SIZE-2 : RAWHID_TX_SIZE;

    /* Send message in chunks */
    while (size >= chunk)
    {
        if (usbHidSend_P(cmd, msg, chunk, USB_WRITE_TIMEOUT) != RETURN_COM_TRANSF_OK)
        {
            return RETURN_COM_NOK;
        }
        msg += chunk;
        size -= chunk;
        if (cmd)
        {
            chunk += 2;
            cmd = 0;
        }
    }
    if (size || cmd)
    {
        if (usbHidSend_P(cmd, msg, size, USB_WRITE_TIMEOUT) != RETURN_COM_TRANSF_OK)
        {
            return RETURN_COM_NOK;
        }
    }
    return RETURN_COM_TRANSF_OK;
}

/*! \fn     pluginSendMessage(uint8_t cmd, uint8_t len, const char* str)
*   \brief  Send a message to the mooltipass plugin
*   \param  cmd command UID
*   \param  len message length
*   \param  str pointer to the string in RAM
*   \return If we managed send the data
*/
RET_TYPE pluginSendMessage(uint8_t cmd, uint8_t len, const char* str)
{
    uint8_t buffer[RAWHID_TX_SIZE];
    uint8_t nb_for_loops = 0;
    uint8_t remaining = 0;
    uint8_t i = 0;

    #ifdef USB_DEBUG_OUTPUT
        if ((cmd != CMD_DEBUG && cmd < CMD_EXPORT_FLASH)) 
        {
            if (len)
            {
                usbPrintf_P(PSTR("tx: cmd 0x%02x len %d data 0x%02x\n"), cmd, len, *str);
            } 
            else 
            {
                usbPrintf_P(PSTR("tx: cmd 0x%02x len %d\n"), cmd, len);
            }
        }
    #endif
    
    memset((void*)buffer, 0, RAWHID_TX_SIZE);
    buffer[HID_LEN_FIELD] = len;
    buffer[HID_TYPE_FIELD] = cmd;

    nb_for_loops = (len+HID_DATA_START) / RAWHID_TX_SIZE;
    remaining = (len+HID_DATA_START) % RAWHID_TX_SIZE;

    for (i = 0; i < nb_for_loops; i++)
    {
        if (i == 0)
        {
            memcpy((void*)(buffer+2), (void*)str, RAWHID_TX_SIZE-HID_DATA_START);
            if(usbRawHidSend(buffer, USB_WRITE_TIMEOUT) != RETURN_COM_TRANSF_OK)
            {
                return RETURN_COM_NOK;
            }
        } 
        else
        {
            if(usbRawHidSend((uint8_t*)str+(i*RAWHID_TX_SIZE)-HID_DATA_START, USB_WRITE_TIMEOUT) != RETURN_COM_TRANSF_OK)
            {
                return RETURN_COM_NOK;
            }
        }
    }

    if (remaining != 0)
    {
        if (i == 0)
        {
            memcpy((void*)buffer+HID_DATA_START, (void*)str, remaining-HID_DATA_START);
        } 
        else
        {
            memset((void*)buffer, 0, RAWHID_TX_SIZE);
            memcpy((void*)buffer, (void*)str+(i*RAWHID_TX_SIZE)-HID_DATA_START, remaining);
        }        
        if(usbRawHidSend(buffer, USB_WRITE_TIMEOUT) != RETURN_COM_TRANSF_OK)
        {
            return RETURN_COM_NOK;
        }
    }
    return RETURN_COM_TRANSF_OK;
}

/*! \fn     pluginSendMessageWithRetries(uint8_t cmd, uint8_t len, const char* str)
*   \brief  Send a message to the mooltipass plugin
*   \param  cmd         command UID
*   \param  len         message length
*   \param  str         pointer to the string in RAM
*   \param  nb_retries  number of retries
*   \return If we managed send the data
*/
RET_TYPE pluginSendMessageWithRetries(uint8_t cmd, uint8_t len, const char* str, uint8_t nb_retries)
{    
    while ((nb_retries) && (pluginSendMessage(cmd, len, str) != RETURN_COM_TRANSF_OK))
    {
        nb_retries--;
        pluginMessageRetryDelay();
    }
    if (nb_retries)
    {
        return RETURN_COM_TRANSF_OK;
    } 
    else
    {
        return RETURN_COM_NOK;
    }
}

RET_TYPE usbSendMessageWithRetries(uint8_t cmd, uint8_t size, const void *msg, uint8_t nb_retries)
{
    while ((nb_retries) && (usbSendMessage(cmd, size, msg) != RETURN_COM_TRANSF_OK))
    {
        nb_retries--;
        pluginMessageRetryDelay();
    }

    return nb_retries ? RETURN_COM_TRANSF_OK : RETURN_COM_NOK;
}

/*! \fn     usbPutstr_P(const char *str)
*   \brief  print an progmem ASCIIZ string to the usb serial port.
*   \param  str     pointer to the string in FLASH.
*   \return If we managed send the data
*/
RET_TYPE usbPutstr_P(const char *str)
{
    return usbSendMessage_P(CMD_DEBUG, strlen_P(str), str);
}

/*! \fn     usbPutstr(const char *str)
*   \brief  print an ASCIIZ string to the usb serial port.
*   \param  str     pointer to the string in RAM.
*   \return If we managed send the data
*/
RET_TYPE usbPutstr(const char *str)
{
    return usbSendMessage(CMD_DEBUG, strlen(str), str);
}


#ifdef ENABLE_PRINTF
    /*! \fn     usbPrintf(const char *fmt, ...)
    *   \brief  print a printf formated string and arguments to the serial port.
    *   \param  fmt    pointer to the printf format string in RAM
    *   \return the number of characters printed
    *   \note   maximum output is limited to 64 characters
    */
    uint8_t usbPrintf(const char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        char printBuf[64];  // scratch buffer for printf

        memset(printBuf, 0, sizeof(printBuf));
        uint8_t ret = vsnprintf(printBuf, sizeof(printBuf), fmt, ap);

        if(usbPutstr(printBuf) != RETURN_COM_TRANSF_OK)
        {
            return 0;
        }
        else
        {
            return ret;
        }
    }

    /*! \fn     usbPrintf_P(const char *fmt, ...)
    *   \brief  print a printf formated string and arguments to the serial port.
    *   \param  fmt    pointer to the printf format string in progmem
    *   \return the number of characters printed
    *   \note   maximum output is limited to 64 characters
    */
    uint8_t usbPrintf_P(const char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        char printBuf[64];  // scratch buffer for printf

        memset(printBuf, 0, sizeof(printBuf));
        uint8_t ret = vsnprintf_P(printBuf, sizeof(printBuf), fmt, ap);

        if(usbPutstr(printBuf) != RETURN_COM_TRANSF_OK)
        {
            return 0;
        }
        else
        {
            return ret;
        }
    }
#endif

/*! \fn     usbKeybPutChar(char ch)
*   \brief  press a given char on the keyboard
*   \param  ch    char to press
*   \return if the key was sent
*/
RET_TYPE usbKeybPutChar(char ch)
{
    uint8_t key;
    
    if((ch == '\r') || (ch == '\n'))
    {
        return usbKeyboardPress(KEY_RETURN, 0);
    }
    else if ((ch < ' ') || (ch > '~'))
    {
        return RETURN_COM_NOK;
    }
    else
    {
        key = pgm_read_byte(keyboardLUT_EN+(ch - ' '));
        if (key & SHIFT_MASK)
        {
            return usbKeyboardPress(key & ~SHIFT_MASK, KEY_SHIFT);
        }
        else
        {
            return usbKeyboardPress(key, 0);
        }
    }    
}

/*! \fn     usbKeybPutStr(char* string)
*   \brief  press a given text on the keyboard
*   \param  string    string to press
*   \return if the string was sent
*/
RET_TYPE usbKeybPutStr(char* string)
{
    RET_TYPE temp_ret = RETURN_COM_TRANSF_OK;
    
    while((*string) && (temp_ret == RETURN_COM_TRANSF_OK))
    {
        temp_ret = usbKeybPutChar(*string++);
    }
    
    return temp_ret;
}
