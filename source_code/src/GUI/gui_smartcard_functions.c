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
/*!  \file     gui_smartcard_functions.c
*    \brief    General user interface - smartcard functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/ 
#include <string.h>
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "logic_aes_and_comms.h"
#include "logic_smartcard.h"
#include "timer_manager.h"
#include "oled_wrapper.h"
#include "logic_eeprom.h"
#include "mini_inputs.h"
#include "hid_defines.h"
#include "aes256_ctr.h"
#include "defines.h"
#include "delays.h"
#include "anim.h"
#include "aes.h"
#include "gui.h"



/*! \fn     guiDisplayInsertSmartCardScreenAndWait(void)
*   \brief  Ask for the user to insert his smart card
*   \return RETURN_OK if the user inserted and unlocked his smartcard
*/
RET_TYPE guiDisplayInsertSmartCardScreenAndWait(void)
{
    RET_TYPE card_detect_ret = RETURN_JRELEASED;
        
    //#define KEYBOARD_LAYOUT_TEST
    #ifdef KEYBOARD_LAYOUT_TEST
    for (uint8_t i = ' '; i <= '~'; i++)
    {
        usbKeybPutChar(i);
        usbKeybPutChar(i);
        timerBasedDelayMs(20);
        usbKeybPutChar(' ');
    }
    //setMooltipassParameterInEeprom(KEYBOARD_LAYOUT_PARAM, getMooltipassParameterInEeprom(KEYBOARD_LAYOUT_PARAM)+1);
    //if (getMooltipassParameterInEeprom(KEYBOARD_LAYOUT_PARAM) > LAST_KEYB_LUT)
    //{
    //    setMooltipassParameterInEeprom(KEYBOARD_LAYOUT_PARAM, FIRST_KEYB_LUT);
    //}        
    #endif
    
    //#define EASTER_EGG
    #if !defined(FLASH_CHIP_1M) && defined(EASTER_EGG)
        uint8_t easter_egg_cnt = 0;
    #endif
    
    // Switch on lights
    activityDetectedRoutine();

    // Draw insert bitmap
    oledClear();
    #if defined(HARDWARE_OLIVIER_V1)
        oledBitmapDrawFlash(0, 16, BITMAP_INSERT, 0);
        oledDisplayOtherBuffer();
    #elif defined(MINI_VERSION)        
        #define BETA_TESTER_V
        #ifdef BETA_TESTER_V
            oledBitmapDrawFlash(0, 0, BITMAP_INSERT_CARD, OLED_SCROLL_NONE);
            miniOledPutCenteredString(21, "beta v0.58");
            miniOledFlushEntireBufferToDisplay();
        #else
            oledBitmapDrawFlash(0, 0, BITMAP_INSERT_CARD, OLED_SCROLL_FLIP);
        #endif
    #endif
    
    // Wait for either timeout or for the user to insert his smartcard
    while ((hasTimerExpired(TIMER_USERINT, TRUE) == TIMER_RUNNING) && (card_detect_ret != RETURN_JDETECT))
    {
        card_detect_ret = isCardPlugged();
        // Easter Egg
        #if !defined(FLASH_CHIP_1M) && defined(EASTER_EGG)
            if (touchDetectionRoutine(0) & RETURN_RIGHT_PRESSED)
            {
                if (easter_egg_cnt++ == 20)
                {
                    oledSetScrollSpeed(100);
                    oledBitmapDrawFlash(0, 0, BITMAP_MOOLTIPASS, OLED_SCROLL_UP);
                    oledBitmapDrawFlash(0, 0, BITMAP_EGG, OLED_SCROLL_UP);
                    oledBitmapDrawFlash(0, 0, BITMAP_EGG_END, OLED_SCROLL_UP);
                    oledSetScrollSpeed(3);
                    userViewDelay();
                }
            }
        #else
            #if defined(HARDWARE_OLIVIER_V1)
                touchDetectionRoutine(0);
            #elif defined(MINI_VERSION)
                miniGetWheelAction(FALSE, FALSE);
            #endif
        #endif
    }
    
    // If the user didn't insert his smart card
    if (card_detect_ret != RETURN_JDETECT)
    {
        // Get back to other screen
        guiGetBackToCurrentScreen();
        return RETURN_NOK;
    }
    else
    {
        return handleSmartcardInserted();
    }
}