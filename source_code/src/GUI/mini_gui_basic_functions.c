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
/*!  \file     gui_mini_basic_functions.c
*    \brief    General user interface - basic functions
*    Created:  22/6/2014
*    Author:   Mathieu Stephan
*/
#include <string.h>

#include "touch_higher_level_functions.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "timer_manager.h"
#include "logic_eeprom.h"
#include "mini_inputs.h"
#include "oledmini.h"
#include "defines.h"
#include "delays.h"
#include "anim.h"
#include "pwm.h"
#include "gui.h"
#ifdef MINI_VERSION

// Screen saver on bool
uint8_t screenSaverOn = FALSE;


/*! \fn     isScreenSaverOn(void)
*   \brief  Returns screen saver bool
*   \return screen saver bool
*/
uint8_t isScreenSaverOn(void)
{
    return screenSaverOn;
}

/*! \fn     activityDetectedRoutine(void)
*   \brief  What to do when user activity has been detected
*/
void activityDetectedRoutine(void)
{    
    // Activate timers for automatic switch off & user interaction timeout
    activateTimer(TIMER_SCREEN, SCREEN_TIMER_DEL);
    activateTimer(SLOW_TIMER_LOCKOUT, getMooltipassParameterInEeprom(LOCK_TIMEOUT_PARAM));
    activateTimer(TIMER_USERINT, ((uint16_t)controlEepromParameter(getMooltipassParameterInEeprom(USER_INTER_TIMEOUT_PARAM), MIN_USER_INTER_DEL/1000, MAX_USER_INTER_DEL/1000)) << 10);
    
    // If the screen was off, turn it on!
    if (miniOledIsScreenOn() == FALSE)
    {
        miniOledOn();
        screenComingOnDelay();
    }
    
    // If we are in screen saver mode, exit it!
    if (screenSaverOn == TRUE)
    {
        screenSaverOn = FALSE;
    }
}

/*! \fn     guiMainLoop(void)
*   \brief  Main user interface loop
*/
void guiMainLoop(void)
{
    RET_TYPE input_interface_result;
    uint8_t screenSaverOnCopy;
    uint8_t isScreenOnCopy;
    
    // Make a copy of the screen on & screensaver on bools
    screenSaverOnCopy = screenSaverOn;
    isScreenOnCopy = miniOledIsScreenOn();
    
    /* Get possible wheel action */
    input_interface_result = miniGetWheelAction(FALSE, FALSE);

    #if defined(HARDWARE_MINI_CLICK_V2)
    if ((scanAndGetDoubleZTap(FALSE) == RETURN_OK) && (miniOledIsScreenOn() == FALSE))
    {
        // knock detecting algo to wakup the device
        activityDetectedRoutine();
    }
    #endif
    
    // No activity, switch off screen
    if (hasTimerExpired(TIMER_SCREEN, TRUE) == TIMER_EXPIRED)
    {
        #ifndef MINI_DEMO_VIDEO
            guiDisplayGoingToSleep();
            userViewDelay();
            if (getMooltipassParameterInEeprom(SCREENSAVER_PARAM) != FALSE)
            {
                screenSaverOn = TRUE;
            }
            else
            {
                miniOledOff();
                guiGetBackToCurrentScreen();
            }
        #else
            miniOledBitmapDrawFlash(0, 0, BITMAP_MOOLTIPASS, OLED_SCROLL_UP);
        #endif
    }

    // If there was some activity and we are showing the screen saver
    if ((input_interface_result != WHEEL_ACTION_NONE) && (screenSaverOnCopy == TRUE))
    {
        guiGetBackToCurrentScreen();
    }

    // Run the main gui screen loop only if we didn't just wake up the display
    if ((input_interface_result != WHEEL_ACTION_NONE) && (((isScreenOnCopy != FALSE) && (screenSaverOnCopy == FALSE)) || (getCurrentScreen() == SCREEN_DEFAULT_INSERTED_LCK)))
    {
        guiScreenLoop(input_interface_result);
    }
}

/*! \fn     miniTextEntry(char * dst, uint8_t buflen, uint8_t filled_len, uint8_t min, uint8_t max, char * question)
*   \brief  Text-input GUI
*
*   Text input function that displays a character or unsigned 8-bit integer value selection widget,
*   and stores either the submitted string or value to a destination buffer.
*   User-typed string is displayed on-screen before final validation.
*   This can accept arbitrary-length strings, but does not implement text-scrolling.
*   For memory consumption reasons, the destination buffer will be modified regardless of user validation.
*   In integer entry mode:
*   - a short click will confirm the selected value;
*   - a long click will cancel the input.
*   In text entry mode:
*   - any wheel click flashes the screen;
*   - a short click selects a character;
*   - a long click erases the previous character;
*   - "ENTRY_LONGCLICK_ERASE" number of long clicks in a row clear the input field;
*   - a long click with an empty field cancels the input;
*   - 3 "special" characters provide alternative confirmation methods:
*     - ENTRY_CHAR_OK_STR ("OK")       : confirms text input,
*     - ENTRY_CHAR_CANCEL_STR ("<<")   : cancels text input,
*     - ENTRY_CHAR_BACKSPACE_STR ("<-"): erases the previous character;
*   - the screen colors will be inverted if the maximum allowed input length was reached.
*   Selectable charset is:
*   ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~° !"#$%&'()*+,-./0123456789:;<=>?@
*   Special characters are appended at the end of the selectable charset.
*
*   \param  dst                   Pointer to string to be filled with user-submitted text.
*                                 Can be pre-filled with a null-terminated string.
*                                 If max > 0, only the first byte will be affected and treated as an uint8_t.
*                                 If max == 0, text entry mode is used, dst is guaranteed to be null-terminated.
*   \param  buflen                dst buffer size, including NULL terminator when used in text entry mode.
*   \param  filled_len            length of pre-filled string data in dst buffer for text entry mode.
*                                 dst will be truncated to that length.
*   \param  min                   minimum pickable unsigned 8-bit integer value.
*   \param  max                   maximum pickable unsigned 8-bit integer value.
*   \param  question              Pointer to string representing the question asked to the user,
*                                 printed at the top display row.
*   \return RETURN_OK if a dst has been filled with valid data, RETURN_NOK if entry was cancelled.
*   In that case, dst value should be discarded as it now contains inconsistent data.
*/
RET_TYPE miniTextEntry(char * dst, uint8_t buflen, uint8_t filled_len, uint8_t min, uint8_t max, char * question)
{
    char ctr[4];            /* numeric counter converted as a string */
    char sel[2];            /* currently selected character, as a string to benefit from text-centering functions */
    char * special[3];      /* special validation/cancellation/erasure characters, inserted as last characters in the set */
    uint8_t i;              /* misc iterator */
    uint8_t offset;         /* charset selection offset */
    uint8_t pos;            /* cursor position in destination string */
    uint8_t long_click_ctr; /* long-click tracker to detect successive long clicks */
    RET_TYPE wheel_action;  /* detected wheel action */

    if(filled_len) /* truncate destination string if pre-filled */
    {
        dst[filled_len] = '\0';
    }
    else /* erase string to prevent leakage */
    {
        for(i = 0; i < buflen; ++i)
        {
            dst[i] = 0;
        }
    }

    /* force minimum offset (initially displayed character) in text-mode */
    if(max == 0)
    {
        min = 0;
    }

    offset         = ((max > 0) ? max : min);   /* offset relative to 'A', including special characters */
    pos            = filled_len;                /* set current cursor position */
    filled_len     = 0;                         /* reset filled_len to 0 to force resetting the cursor offset when the user clears the input field */
    long_click_ctr = 0;                         /* initialize long click tracker */

    /* Compute ASCII char-code accounting for special characters */
    sel[0]         = '\x20' + ((ENTRY_FIRST_CHAR + offset - ENTRY_NB_SPECIAL_CHAR) % ENTRY_CHARSET_LENGTH); /*  */
    sel[1]         = '\0';

    /* special input validation/cancellation/erasure "characters" */
    special[ENTRY_CHAR_OK - ENTRY_CHARSET_LENGTH]        = ENTRY_CHAR_OK_STR;
    special[ENTRY_CHAR_CANCEL - ENTRY_CHARSET_LENGTH]    = ENTRY_CHAR_CANCEL_STR;
    special[ENTRY_CHAR_BACKSPACE - ENTRY_CHARSET_LENGTH] = ENTRY_CHAR_BACKSPACE_STR;

    /* Clear buffer */
    miniOledClearFrameBuffer();

    /* Draw selection widget by reusing pin-entry graphics */
    miniOledBitmapDrawFlash(0, 0, BITMAP_PIN_SLOT4, 0);

    /* Force default font */
    miniOledSetFont(FONT_DEFAULT);

    while(TRUE) /* rendering loop */
    {
        /* compute ASCII representation of "offset" */
        ctr[0] = ((offset/100)%10 == 0 ? ' ' : '0' + (offset/100)%10);
        ctr[1] = ((((offset/10 )%10 == 0) && (ctr[0] == ' ')) ? ' ' : '0' + (offset/10 )%10);
        ctr[2] = '0' + offset%10;
        ctr[3] = '\0';

        /* erase left part of the screen, occluding 3 of the 4 digit entry widgets */
        miniOledDrawRectangle(0, 0, 110, 32, FALSE);

        /* prevent text from overlapping with widget */
        miniOledSetMaxTextY(108);

        /* allow multi-line wrap */
        miniOledAllowTextWritingYIncrement();

        /* print question text */
        if(max > 0)
        {
                miniOledPutstrXY(0, THREE_LINE_TEXT_SECOND_POS, OLED_LEFT, question);
        }
        else
        {
            miniOledPutstrXY(0, 0, OLED_LEFT, question);
        }

        /* display offset value in text-entry mode */
        if(max == 0)
        {
            miniOledPutstrXY(108, 0, OLED_RIGHT, ctr);
        }

        /* display current (possibly pre-filled) user input */
        if(strlen(dst) > 18)
        {
            miniOledPutCenteredString(THREE_LINE_TEXT_SECOND_POS, dst + strlen(dst) - 18);
        }
        else
        {
            miniOledPutCenteredString(THREE_LINE_TEXT_SECOND_POS, dst);
        }

        /* disable multi-line wrap */
        miniOledPreventTextWritingYIncrement();

        /* reset text boundaries */
        miniOledResetMaxTextY();

        /* clear currently selected value/character in the widget */
        miniOledDrawRectangle(108, 7, 19, 15, FALSE);

        /* set widget text boundaries */
        miniOledSetMinTextY(110);
        miniOledSetMaxTextY(127);

        if(max > 0) /* in integer entry mode, display integer */
        {
            miniOledSetMinTextY(108);
            miniOledSetMaxTextY(127);
            miniOledPutCenteredString(THREE_LINE_TEXT_SECOND_POS, ctr);
        }
        else if(offset > ENTRY_CHARSET_MAX) /* in text entry mode, display special character */
        {
            miniOledSetMinTextY(112);
            miniOledSetMaxTextY(127);
            miniOledPutCenteredString(THREE_LINE_TEXT_SECOND_POS, special[(offset - ENTRY_CHARSET_LENGTH)]);
        }
        else /* in text entry mode, display standard character */
        {
            miniOledPutCenteredString(THREE_LINE_TEXT_SECOND_POS, sel);
        }

        /* reset text boundaries */
        miniOledSetMinTextY(0);
        miniOledResetMaxTextY();

        /* render display */
        miniOledFlushEntireBufferToDisplay();

        /* invert screen if maximum allowed length was reached in text entry mode */
        if(!(buflen-pos-1) && (max == 0))
        {
            miniOledInvertedDisplay();
        }

        /* handle wheel actions */
        wheel_action = miniGetWheelAction(TRUE, FALSE);
        switch(wheel_action)
        {
            case WHEEL_ACTION_SHORT_CLICK:
                /* in integer entry mode, fill destination with selected value, and return */
                if(max > 0)
                {
                    dst[0] = offset;
                    miniOledNormalDisplay(); /* force normal colors in case the screen was inverted */
                    return RETURN_OK;
                }

                /* in text-entry mode, handle special chars */
                switch(offset)
                {
                    case ENTRY_CHAR_OK:
                        miniOledNormalDisplay();
                        return RETURN_OK;
                    case ENTRY_CHAR_CANCEL:
                        miniOledNormalDisplay();
                        return RETURN_NOK;
                    case ENTRY_CHAR_BACKSPACE:
                        /* erase previous char */
                        pos--;
                        dst[pos] = '\0';

                        /* flash screen for 50 ms */
                        miniOledInvertedDisplay();
                        timerBasedDelayMs(50);
                        miniOledNormalDisplay();
                        continue;
                }

                /* ignore input if maximum length was reached */
                if(!(buflen-pos-1))
                {
                    continue;
                }

                /* in test entry mode, compute character code, increment cursor, null-terminate and reset long click tracker */
                dst[pos] = '\x20' + ((ENTRY_FIRST_CHAR + offset - ENTRY_NB_SPECIAL_CHAR) % ENTRY_CHARSET_LENGTH);
                pos++;
                dst[pos] = '\0';
                long_click_ctr=0;

                /* flash screen for 50 ms */
                miniOledInvertedDisplay();
                timerBasedDelayMs(50);
                miniOledNormalDisplay();
                continue;
            case WHEEL_ACTION_LONG_CLICK:
                /* keep track of long clicks */
                long_click_ctr++;

                /* handle cancellation if input field is empty */
                if(pos == 0)
                {
                    miniOledNormalDisplay();
                    return RETURN_NOK;
                }

                /* handle default behaviour: backspace */
                pos--;
                dst[pos] = '\0';

                /* clear text input and reset tracker if we have reached the required count of long clicks */
                if(long_click_ctr >= ENTRY_LONGCLICK_ERASE)
                {
                    long_click_ctr = 0;
                    for(i = 0; i < buflen; ++i)
                    {
                        dst[i] = 0;
                    }
                    pos = 0;
                }

                /* flash screen for 50 ms */
                miniOledInvertedDisplay();
                timerBasedDelayMs(50);
                miniOledNormalDisplay();
                continue;
            case WHEEL_ACTION_UP:
                /* handle offset increment */
                if((max == 0 && offset >= ENTRY_LAST_CHAR) || (max > 0 && offset >= max)) /* wrap around */
                {
                    offset = min;
                }
                else
                {
                    offset++;
                }

                /* render new selected character */
                sel[0] = '\x20' + ((ENTRY_FIRST_CHAR + offset - ENTRY_NB_SPECIAL_CHAR) % ENTRY_CHARSET_LENGTH);
                sel[1] = '\0';
                continue;
            case WHEEL_ACTION_DOWN:
                /* handle offset decrement */
                if(offset == min) /* wrap around */
                {
                    offset = (max > 0) ? max : ENTRY_LAST_CHAR;
                }
                else
                {
                    offset--;
                }

                /* render new selected character */
                sel[0] = '\x20' + ((ENTRY_FIRST_CHAR + offset - ENTRY_NB_SPECIAL_CHAR) % ENTRY_CHARSET_LENGTH);
                sel[1] = '\0';
                continue;
            default:
                /* might occur with scroll+click events */
                miniOledNormalDisplay();
                return RETURN_NOK;
        } /* end switch: wheel action handling */
    } /* end while: rendering loop */
}
#endif