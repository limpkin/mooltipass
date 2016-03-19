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
#include "timer_manager.h"
#include "hid_defines.h"
#include "defines.h"
#include "usb.h"
#include <string.h>
#include <stdio.h>


// Zero when we are not configured, non-zero when enumerated
static volatile uint8_t usb_configuration = 0;

// Endpoint configuration table
static const uint8_t PROGMEM endpoint_config_table[] =
{
    1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(RAWHID_TX_SIZE) | RAWHID_TX_BUFFER,
    1, EP_TYPE_INTERRUPT_OUT, EP_SIZE(RAWHID_RX_SIZE) | RAWHID_RX_BUFFER,
    0,
    0
};


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
    UDCON = 0x00;                   // enable attach resistor
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

/*! \fn     usbRawHidRecv(uint8_t *buffer, uint8_t timeout)
*   \brief  Receive a packet, with timeout
*   \param  buffer    Pointer to the buffer to store received data
*   \return RETURN_COM_TRANSF_OK or RETURN_COM_NOK or RETURN_COM_TIMEOUT
*/
RET_TYPE usbRawHidRecv(uint8_t *buffer)
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
    // Activate timeout timer
    activateTimer(TIMER_WAIT_FUNCTS, USB_READ_TIMEOUT);
    UENUM = RAWHID_RX_ENDPOINT;
    // wait for data to be available in the FIFO
    while (1)
    {
        if (UEINTX & (1<<RWAL))
        {
            break;
        }
        SREG = intr_state;
        if (hasTimerExpired(TIMER_WAIT_FUNCTS, TRUE) == TIMER_EXPIRED)
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
    uint8_t intbits;

    intbits = UDINT;
    UDINT = 0;
    
    // Device reset
    if (intbits & (1<<EORSTI))
    {
        UENUM = 0;
        UECONX = 1;
        UECFG0X = EP_TYPE_CONTROL;
        UECFG1X = EP_SIZE(ENDPOINT0_SIZE) | EP_SINGLE_BUFFER;
        UEIENX = (1<<RXSTPE);
        usb_configuration = 0;
    }
}

// Misc functions to wait for ready and send/receive packets
static inline void usb_wait_in_ready(void)
{
    //USB_OLED_PRINT_DEBUG_CODE("010");
    while (!(UEINTX & (1<<TXINI))) ;
}
static inline void usb_send_in(void)
{
    UEINTX = ~(1<<TXINI);
}
static inline void usb_wait_receive_out(void)
{
    //USB_OLED_PRINT_DEBUG_CODE("020");
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
        UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI) | (1 << NAKINI));

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
                    //USB_OLED_PRINT_DEBUG_CODE("030");
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
                        //USB_OLED_PRINT_DEBUG_CODE("040");
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
                // It seems that this code could be stuck as some computer may not send all the bytes
                //do
                //{
                //    n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
                //    usb_wait_receive_out();
                //    // ignore incoming bytes
                //    usb_ack_out();
                //    len -= n;
                //}
                //while (len);
                // Instead we look for the NAIKI flag
                do 
                {
                    if(UEINTX & (1<<RXOUTI))
                    {
                        usb_ack_out();
                    }
                } 
                while (!(UEINTX & (1<<NAKINI)));
                usb_wait_in_ready();
                usb_send_in();
                return;
            }
        }
    }
    UECONX = (1<<STALLRQ) | (1<<EPEN);  // stall
}

/*!
*   \brief  Wait for the TX fifo to be ready to accept data.
*   \param  intr_state   pointer to storage to save the interrupt state
*   \retval RETURN_TRANSF_COM_OK fifo is ready, and interrupts are disabled
*   \retval RETURN_COM_TIMEOUT timeout waiting for fifo to be ready
*   \retval RETURN_COM_NOK USB not configured
*/
static RET_TYPE usbWaitFifoReady(uint8_t *intr_state)
{
    // if we're not online (enumerated and configured), error
    if (!usb_configuration)
    {
        return RETURN_COM_NOK;
    }
    *intr_state = SREG;
    cli();
    // Activate timeout timer
    activateTimer(TIMER_WAIT_FUNCTS, USB_WRITE_TIMEOUT);
    UENUM = RAWHID_TX_ENDPOINT;
    // wait for the FIFO to be ready to accept data
    while (1)
    {
        if (UEINTX & (1<<RWAL))
        {
            break;
        }
        SREG = *intr_state;
        if (hasTimerExpired(TIMER_WAIT_FUNCTS, TRUE) == TIMER_EXPIRED)
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
*   \return RETURN_TRANSF_COM_OK or RETURN_COM_NOK or RETURN_COM_TIMEOUT
*/
RET_TYPE usbHidSend(uint8_t cmd, const void *buffer, uint8_t buflen)
{
    uint8_t intr_state;
    int8_t res, rem;
    
    // How many bytes to add to send a full packet
    if (cmd)
    {
        // If there is a command, the first 2 bytes are taken
        rem = PACKET_EXPORT_SIZE - buflen;
    }
    else
    {
        rem = RAWHID_TX_SIZE - buflen;
    }

    // Check that we don't want to send too many bytes
    if (rem < 0)
    {
        return RETURN_COM_NOK;
    }

    res = usbWaitFifoReady(&intr_state);

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
*   \return RETURN_TRANSF_COM_OK or RETURN_COM_NOK or RETURN_COM_TIMEOUT
*/
RET_TYPE usbHidSend_P(uint8_t cmd, const void *buffer, uint8_t buflen)
{
    uint8_t intr_state;
    int8_t res, rem;
    
    // How many bytes to add to send a full packet
    if (cmd)
    {
        // If there is a command, the first 2 bytes are taken
        rem = PACKET_EXPORT_SIZE - buflen;
    }
    else
    {
        rem = RAWHID_TX_SIZE - buflen;
    }

    // Check that we don't want to send too many bytes
    if (rem < 0)
    {
        return RETURN_COM_NOK;
    }

    res = usbWaitFifoReady(&intr_state);

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
int8_t usbRawHidSend(uint8_t* buffer)
{
    return usbHidSend(0, buffer, RAWHID_TX_SIZE);
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
    /* Send message in chunks */
    while (size >= PACKET_EXPORT_SIZE)
    {
        if (usbHidSend(cmd, msg, PACKET_EXPORT_SIZE) != RETURN_COM_TRANSF_OK)
        {
            return RETURN_COM_NOK;
        }
        msg += PACKET_EXPORT_SIZE;
        size -= PACKET_EXPORT_SIZE;
        if (size == 0)
        {
            // We sent all the packets
            return RETURN_COM_TRANSF_OK;
        }
    }
    
    /* Send our only or last packet */
    if (usbHidSend(cmd, msg, size) != RETURN_COM_TRANSF_OK)
    {
        return RETURN_COM_NOK;
    }
    else
    {
        return RETURN_COM_TRANSF_OK;        
    }
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
    /* Send message in chunks */
    while (size >= PACKET_EXPORT_SIZE)
    {
        if (usbHidSend_P(cmd, msg, PACKET_EXPORT_SIZE) != RETURN_COM_TRANSF_OK)
        {
            return RETURN_COM_NOK;
        }
        msg += PACKET_EXPORT_SIZE;
        size -= PACKET_EXPORT_SIZE;
        if (size == 0)
        {
            // We sent all the packets
            return RETURN_COM_TRANSF_OK;
        }
    }
    
    /* Send our only or last packet */
    if (usbHidSend_P(cmd, msg, size) != RETURN_COM_TRANSF_OK)
    {
        return RETURN_COM_NOK;
    }
    else
    {
        return RETURN_COM_TRANSF_OK;
    }
}
