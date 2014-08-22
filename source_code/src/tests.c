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
/*! \file   tests.c
 *  \brief  Test functions
 *  Copyright [2014] [Mathieu Stephan]
 */
#include "touch_higher_level_functions.h"
#include "aes256_nessie_test.h"
#include "aes256_ctr_test.h"
#include "hid_defines.h"
#include "mooltipass.h"
#include "flash_test.h"
//#include "node_test.h"
#include "defines.h"
#include "entropy.h"
#include "oledmp.h"
#include "touch.h"
#include "pwm.h"
#include "usb.h"
#include "gui.h"


/*! \fn     beforeFlashInitTests(void)
*   \brief  Test functions launched before flash init
*/
void beforeFlashInitTests(void)
{
    //#define TEST_FLASH      // Comment out to not test flash
    #ifdef TEST_FLASH
        // run flash test
        flashTest();
        // spin
        while(1);
    #endif    
}

/*! \fn     afterFlashInitTests(void)
*   \brief  Test functions launched after flash init
*/
void afterFlashInitTests(void)
{    
    //#define TEST_NODE
    #ifdef TEST_NODE
        nodeTest();
        // spin
        while(1);
    #endif
    //#define TEST_HID_AND_CDC
    #ifdef TEST_HID_AND_CDC
        //Show_String("Z",FALSE,2,0);
        //usbKeyboardPress(KEY_S, 0);
        while(1)
        {
            int n = usb_serial_getchar();
            if (n >= 0)
            {
                usb_serial_putchar(n);
                oledSetXY(2,0);
                oledPutch((char)n);

                //usbKeyboardPress(n,0);
            }

        }
    #endif /* TEST_HID_AND_CDC */

    //#define NESSIE_TEST_VECTORS
    #ifdef NESSIE_TEST_VECTORS
        while(1)
        {
            // msg into oled display
            oledSetXY(2,0);
            printf_P(PSTR("send s to start nessie test"));

            int input0 = usb_serial_getchar();

            nessieOutput = &usb_serial_putchar;

            // do nessie test after sending s or S chars
            if (input0 == 's' || input0 == 'S')
            {
                nessieTest(1);
                nessieTest(2);
                nessieTest(3);
                nessieTest(4);
                nessieTest(5);
                nessieTest(6);
                nessieTest(7);
                nessieTest(8);
            }
        }
    #endif
    
    //#define CTR_TEST_VECTORS
    #ifdef CTR_TEST_VECTORS
        while(1)
        {
            // msg into oled display
            oledSetXY(2,0);
            printf_P(PSTR("send s to start CTR test"));

            int input1 = usb_serial_getchar();

            ctrTestOutput = &usb_serial_putchar;

            // do ctr test after sending s or S chars
            if (input1 == 's' || input1 == 'S')
            {
                aes256CtrTest();
            }
        }
    #endif

	//#define TEST_CTR_SPEED
	#ifdef TEST_CTR_SPEED
		// msg into oled display
		oledSetXY(2,0);
		usbPrintf_P(PSTR("CTR speed TEST with 1000 encryptions\n"));
		usbPrintf_P(PSTR("Time:"));
		usbPrintf_P(PSTR("%lu ms"), aes256CtrSpeedTest());
		while(1);
	#endif

    //#define TEST_RNG
    #ifdef TEST_RNG 
        while(1)
        {
            // init avrentropy library
            EntropyInit();

            // msg into oled display
            oledSetXY(2,0);
            printf_P(PSTR("send s to start entropy"));

            int input2 = usb_serial_getchar();

            uint32_t randomNumCtr;

            // do nessie test after sending s or S chars
            if (input2 == 's' || input2 == 'S')
            {
                while(EntropyAvailable() < 2);
                
                EntropyRandom8();

                usb_serial_putchar(EntropyBytesAvailable());

                for(randomNumCtr=0; randomNumCtr<25; randomNumCtr++)
                {
                        usb_serial_putchar(EntropyRandom8());
                }
            }

        }
    #endif    
}

void afterTouchInitTests(void)
{
    //#define TEST_TS
    #ifdef TEST_TS
    uint8_t temp_byte;
    uint16_t temp_uint = 0;
    RET_TYPE temp_ret_type = RETURN_RIGHT_PRESSED;
    
    activityDetectedRoutine();
    oledWriteActiveBuffer();
    activateProxDetection();
    while(!(temp_ret_type & RETURN_LEFT_PRESSED))
    {
        if (temp_ret_type != RETURN_NO_CHANGE)
        {
            oledSetXY(0,0);
            readDataFromTS(REG_AT42QT_SLIDER_POS, &temp_byte);
            printf("POS: %02X\r\n", temp_byte);
            readDataFromTS(REG_AT42QT_DET_STAT, &temp_byte);
            printf("DET STAT: %02X\r\n", temp_byte);
            readDataFromTS(REG_AT42QT_KEY_STAT1, &temp_byte);
            printf("DET1: %02X\r\n", temp_byte);
            readDataFromTS(REG_AT42QT_KEY_STAT2, &temp_byte);
            printf("DET2: %02X\r\n", temp_byte);
            printf("counter: %04X\r\n", temp_uint++);
        }
        temp_ret_type = touchDetectionRoutine();     
    }
    activateGuardKey();
    launchCalibrationCycle();
    while(1)
    {
        if (temp_ret_type != RETURN_NO_CHANGE)
        {            
            oledSetXY(0,0);
            readDataFromTS(REG_AT42QT_SLIDER_POS, &temp_byte);
            printf("POS: %02X\r\n", temp_byte);
            readDataFromTS(REG_AT42QT_DET_STAT, &temp_byte);
            printf("DET STAT: %02X\r\n", temp_byte);
            readDataFromTS(REG_AT42QT_KEY_STAT1, &temp_byte);
            printf("DET1: %02X\r\n", temp_byte);
            readDataFromTS(REG_AT42QT_KEY_STAT2, &temp_byte);
            printf("DET2: %02X\r\n", temp_byte);
            printf("counter: %04X\r\n", temp_uint++);
        }
        temp_ret_type = touchDetectionRoutine();
    }
    #endif
}

void afterHadLogoDisplayTests(void)
{
    //#define TEST_PWM
    #ifdef TEST_PWM
    uint8_t toto = 0;
    switchOnLeftButonLed();
    switchOnRightButonLed();
    switchOnTopLeftWheelLed();
    switchOnTopRightWheelLed();
    switchOnBotLeftWheelLed();
    switchOnBotRightWheelLed();
    oledWriteActiveBuffer();
    while(1)
    {
        oledSetXY(2,0);
        printf("%02X", MAX_PWM_VAL >> toto);
        setPwmDc(MAX_PWM_VAL >> toto);
        _delay_ms(1000);
        if(toto++ == 11)
            toto = 0;
    }
    #endif
    
    //#define TEST_HID
    #ifdef TEST_HID
    uint8_t i;
    while(1)
    {
        if (getKeyboardLeds() & HID_CAPS_MASK)
        {
            usbPutstr("NOPE!\r\n");
            while(getKeyboardLeds() & HID_CAPS_MASK)
            {
                usbKeyboardPress(KEY_CAPS_LOCK, 0);
                _delay_ms(30);
            }
            for(i = ' '; i < 0x7F; i++)
            {
                usbKeybPutChar(i);
            }
            usbKeybPutStr("\rBonjour oh grand dieu!\n");
        }
    }
    #endif
    
    //#define TEST_PLUGIN
    #ifdef TEST_PLUGIN    
    printf_P(PSTR("Plugin test\n"));
    oledFlipBuffers(OLED_SCROLL_UP,5);
    oledClear();    // clear inactive buffer
    oledWriteActiveBuffer();
    #endif
}
