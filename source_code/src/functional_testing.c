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
/*! \file   functional_testing.c
 *  \brief  Functional testing functions
 *  Copyright [2016] [Mathieu Stephan]
 */ 
#include <avr/eeprom.h>
#include <avr/io.h>
#include "smart_card_higher_level_functions.h"
#include "touch_higher_level_functions.h"
#include "logic_fwflash_storage.h"
#include "gui_screen_functions.h"
#include "gui_basic_functions.h"
#include "eeprom_addresses.h"
#include "oled_wrapper.h"
#include "mini_inputs.h"
#include "smartcard.h"
#include "defines.h"
#include "delays.h"
#include "utils.h"
#include "pwm.h"


/*! \fn     electricalJumpToBootloaderCondition(void)
 *  \brief  Electrical condition to jump to the bootloader
 *  \return Boolean to know if this condition is fullfilled
 */
RET_TYPE electricalJumpToBootloaderCondition(void)
{
    #if defined(HARDWARE_OLIVIER_V1)
        /* Disable JTAG to get access to the pins */
        disableJTAG();
        
        /* Init SMC port */
        initPortSMC();
        
        /* Delay for detection */
        smallForLoopBasedDelay();
        
        if (!(PIN_SC_DET & (1 << PORTID_SC_DET)))
        {
            uint16_t tempuint16;
            
            /* What follows is a copy from firstDetectFunctionSMC() */
            /* Enable power to the card */
            PORT_SC_POW &= ~(1 << PORTID_SC_POW);
            
            /* Default state: PGM to 0 and RST to 1 */
            PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
            DDR_SC_PGM |= (1 << PORTID_SC_PGM);
            PORT_SC_RST |= (1 << PORTID_SC_RST);
            DDR_SC_RST |= (1 << PORTID_SC_RST);
            
            /* Activate SPI port */
            PORT_SPI_NATIVE &= ~((1 << SCK_SPI_NATIVE) | (1 << MOSI_SPI_NATIVE));
            DDRB |= (1 << SCK_SPI_NATIVE) | (1 << MOSI_SPI_NATIVE);
            setSPIModeSMC();
            
            /* Let the card come online */
            smallForLoopBasedDelay();
            
            /* Check smart card FZ */
            readFabricationZone((uint8_t*)&tempuint16);
            if ((swap16(tempuint16)) != SMARTCARD_FABRICATION_ZONE)
            {
                removeFunctionSMC();
                return TRUE;
            }
            else
            {
                removeFunctionSMC();
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    #elif defined(MINI_VERSION)
        /* Disable JTAG to get access to the pins */
        disableJTAG();
        
        /* Pressing center joystick starts the bootloader */
        DDR_JOYSTICK &= ~(1 << PORTID_JOY_CENTER);
        PORT_JOYSTICK |= (1 << PORTID_JOY_CENTER);
        
        /* Small delay for detection */
        smallForLoopBasedDelay();
        
        /* Check if low */
        if (!(PIN_JOYSTICK & (1 << PORTID_JOY_CENTER)))
        {
            return TRUE;
        }  
        else
        {
            return FALSE;
        }
    #endif
}

/*! \fn     mooltipassStandardElectricalTest(uint8_t fuse_ok)
 *  \brief  Mooltipass standard electrical test
 *  \param  fuse_ok Bool to know if fuses set are ok
 */
void mooltipassStandardElectricalTest(uint8_t fuse_ok)
{
    // Check if PB5 is low to start electrical test
    DDRB &= ~(1 << 5); PORTB |= (1 << 5);
    smallForLoopBasedDelay();
    if (!(PINB & (1 << 5)))
    {
        // Test result, true by default
        uint8_t test_result = TRUE;
        
        // Leave flash nS off
        DDR_FLASH_nS |= (1 << PORTID_FLASH_nS);
        PORT_FLASH_nS |= (1 << PORTID_FLASH_nS);
        
        // Set PORTD as output, leave PORTID_OLED_SS high
        DDRD |= 0xFF; PORTD |= 0xFF;
        
        // All other pins are input by default, run our test
        for (uint8_t i = 0; i < 4; i++)
        {
            PORTD |= 0xFF;
            smallForLoopBasedDelay();
            if (!(PINF & (0xC3)) || !(PINC & (1 << 6)) || !(PINE & (1 << 6)) || !(PINB & (1 << 4)))
            {
                test_result = FALSE;
            }
            PORTD &= (1 << PORTID_OLED_SS);
            smallForLoopBasedDelay();
            if ((PINF & (0xC3)) || (PINC & (1 << 6)) || (PINE & (1 << 6)) || (PINB & (1 << 4)))
            {
                test_result = FALSE;
            }
        }
        
        // PB6 as test result output
        DDRB |= (1 << 6);
        
        // If test successful, light green LED
        if ((test_result == TRUE) && (fuse_ok == TRUE))
        {
            PORTB |= (1 << 6);
        }
        else
        {
            PORTB &= ~(1 << 6);
        }
        while(1);
    }
}

/*! \fn     mooltipassMiniFunctionalTest(uint8_t current_bootkey_val, uint8_t flash_init_result, uint8_t touch_init_result, uint8_t fuse_ok)
 *  \brief  Mooltipass standard functional test
 *  \param  current_bootkey_val     Current boot key value
 *  \param  flash_init_result       Result of the flash initialization procedure
 *  \param  fuse_ok                 Bool to know if fuses set are ok
 */
void mooltipassMiniFunctionalTest(uint16_t current_bootkey_val, uint8_t flash_init_result, uint8_t fuse_ok)
{    
    // Only launch the functional test if the boot key isn't valid
    if (current_bootkey_val != CORRECT_BOOTKEY)
    {
        uint8_t test_result_ok = TRUE;        
        char temp_string[] = {'0', 0};
        RET_TYPE temp_rettype;
        
        // Wait for USB host to upload bundle, which then sets USER_PARAM_INIT_KEY_PARAM
        while(getMooltipassParameterInEeprom(USER_PARAM_INIT_KEY_PARAM) != 0x92)
        {
            usbProcessIncoming(USB_CALLER_MAIN);
        }
        
        // Bundle uploaded, start the screen
        miniOledFlushWrittenTextToDisplay();
        miniOledBegin(FONT_DEFAULT);
        oledSetXY(0,0);
        
        // Check flash initialization
        if (flash_init_result != RETURN_OK)
        {
            guiDisplayRawString(ID_STRING_TEST_FLASH_PB);
            test_result_ok = FALSE;
        }
        
        // Check fuse setting
        if (fuse_ok != TRUE)
        {
            guiDisplayRawString(ID_STRING_FUSE_PB);
            test_result_ok = FALSE;
        }
    
        // Test description
       uint8_t func_test_string_id = ID_STRING_FUNC_TEST;
       guiDisplayRawString(func_test_string_id++);
       guiDisplayRawString(func_test_string_id++);
    
        // Wait for inputs
        oledClear();oledSetXY(0,0);
        miniDirectionClearDetections();
        while(getMiniDirectionJustPressed() != WHEEL_POS_CLICK);
        guiDisplayRawString(func_test_string_id++);
        while(getMiniDirectionJustPressed() != JOYSTICK_POS_UP);
        guiDisplayRawString(func_test_string_id++);
        while(getMiniDirectionJustPressed() != JOYSTICK_POS_RIGHT);
        guiDisplayRawString(func_test_string_id++);
        while(getMiniDirectionJustPressed() != JOYSTICK_POS_DOWN);
        guiDisplayRawString(func_test_string_id++);
        while(getMiniDirectionJustPressed() != JOYSTICK_POS_LEFT);
        guiDisplayRawString(func_test_string_id++);
        while(getMiniDirectionJustPressed() != JOYSTICK_POS_CENTER);
        guiDisplayRawString(func_test_string_id++);
    
        // Test description
        oledClear();
        oledSetXY(0,0);
        guiDisplayRawString(func_test_string_id++);
    
        // Wait for scroll
        while(temp_string[0] != ':')
        {
            temp_string[0] += getWheelCurrentIncrement();
            oledPutstrXY(0,15,0,temp_string);
            oledFillXY(0,15,15,15,FALSE);
        }
        
        // Insert card
        oledClear();oledSetXY(0,0);
        guiDisplayRawString(ID_STRING_TEST_CARD_INS);
        while(isCardPlugged() != RETURN_JDETECT);
        temp_rettype = cardDetectedRoutine();
        
        // Check card
        if (!((temp_rettype == RETURN_MOOLTIPASS_BLANK) || (temp_rettype == RETURN_MOOLTIPASS_USER)))
        {
            guiDisplayRawString(ID_STRING_TEST_CARD_PB);
            test_result_ok = FALSE;
        }
        
        // Display result
        uint8_t script_return = RETURN_OK;
        if (test_result_ok == TRUE)
        {
            // Inform script of success
            usbSendMessage(CMD_FUNCTIONAL_TEST_RES, 1, &script_return);            
            
            // Wait for password to be set
            while(eeprom_read_byte((uint8_t*)EEP_BOOT_PWD_SET) != BOOTLOADER_PWDOK_KEY)
            {
                usbProcessIncoming(USB_CALLER_MAIN);
            }
            
            #if defined(AVR_BOOTLOADER_PROGRAMMING)
                // We actually remove the boot pwd set bool for units whose bootloader can be started by pressing a button at boot...
                eeprom_write_byte((uint8_t*)EEP_BOOT_PWD_SET, FALSE);
                eeprom_write_byte((uint8_t*)EEP_UID_REQUEST_KEY_SET_ADDR, FALSE);                
            #endif
        }
        else
        {
            // Set correct bool
            script_return = RETURN_NOK;
            
            // Display test result
            guiDisplayRawString(ID_STRING_TEST_NOK);
            
            // Inform script of failure
            usbSendMessage(CMD_FUNCTIONAL_TEST_RES, 1, &script_return);
            while(1);
        }
    }    
}

#ifdef HARDWARE_OLIVIER_V1
/*! \fn     mooltipassStandardFunctionalTest(uint8_t current_bootkey_val, uint8_t flash_init_result, uint8_t touch_init_result, uint8_t fuse_ok)
 *  \brief  Mooltipass standard functional test
 *  \param  current_bootkey_val     Current boot key value
 *  \param  flash_init_result       Result of the flash initialization procedure
 *  \param  touch_init_result       Result of the touch panel initialization procedure
 *  \param  fuse_ok                 Bool to know if fuses set are ok
 */
void mooltipassStandardFunctionalTest(uint16_t current_bootkey_val, uint8_t flash_init_result, uint8_t touch_init_result, uint8_t fuse_ok)
{
    // Only launch the functional test if the boot key isn't valid
    if (current_bootkey_val != CORRECT_BOOTKEY)
    {
        uint8_t test_result_ok = TRUE;
        RET_TYPE temp_rettype;
        
        // Wait for USB host to upload bundle, which then sets USER_PARAM_INIT_KEY_PARAM
        while(getMooltipassParameterInEeprom(USER_PARAM_INIT_KEY_PARAM) != 0x94)
        {
            usbProcessIncoming(USB_CALLER_MAIN);
        }
        
        // Bundle uploaded, start the screen
        stockOledBegin(FONT_DEFAULT);
        oledWriteActiveBuffer();
        oledSetXY(0,0);
        
        // LEDs ON, to check
        setPwmDc(MAX_PWM_VAL);
        touchDetectionRoutine(0);
        guiDisplayRawString(ID_STRING_TEST_LEDS_CH);
        
        // Check flash init
        if (flash_init_result != RETURN_OK)
        {
            guiDisplayRawString(ID_STRING_TEST_FLASH_PB);
            test_result_ok = FALSE;
        }
        
        // Check touch init
        if (touch_init_result != RETURN_OK)
        {
            guiDisplayRawString(ID_STRING_TEST_TOUCH_PB);
            test_result_ok = FALSE;
        }
        
        // Check fuse setting
        if (fuse_ok != TRUE)
        {
            test_result_ok = FALSE;
            guiDisplayRawString(ID_STRING_FUSE_PB);
        }
        
        // Touch instructions
        guiDisplayRawString(ID_STRING_TEST_INST_TCH);
        
        // Check proximity sensor
        while(!(touchDetectionRoutine(0) & RETURN_PROX_DETECTION));
        guiDisplayRawString(ID_STRING_TEST_DET);
        activateGuardKey();
        
        // Check left
        while(!(touchDetectionRoutine(0) & RETURN_LEFT_PRESSED));
        guiDisplayRawString(ID_STRING_TEST_LEFT);
        
        // Check wheel
        while(!(touchDetectionRoutine(0) & RETURN_WHEEL_PRESSED));
        guiDisplayRawString(ID_STRING_TEST_WHEEL);
        
        // Check right
        while(!(touchDetectionRoutine(0) & RETURN_RIGHT_PRESSED));
        guiDisplayRawString(ID_STRING_TEST_RIGHT);
        
        // Insert card
        guiDisplayRawString(ID_STRING_TEST_CARD_INS);
        while(isCardPlugged() != RETURN_JDETECT);
        temp_rettype = cardDetectedRoutine();
        
        // Check card
        if (!((temp_rettype == RETURN_MOOLTIPASS_BLANK) || (temp_rettype == RETURN_MOOLTIPASS_USER)))
        {
            guiDisplayRawString(ID_STRING_TEST_CARD_PB);
            test_result_ok = FALSE;
        }
        
        // Display result
        uint8_t script_return = RETURN_OK;
        if (test_result_ok == TRUE)
        {
            // Inform script of success
            usbSendMessage(CMD_FUNCTIONAL_TEST_RES, 1, &script_return);
            
            #if !defined(PREPRODUCTION_KICKSTARTER_SETUP)
            // Wait for password to be set
            while(eeprom_read_byte((uint8_t*)EEP_BOOT_PWD_SET) != BOOTLOADER_PWDOK_KEY)
            {
                usbProcessIncoming(USB_CALLER_MAIN);
            }
            #endif
        }
        else
        {
            // Set correct bool
            script_return = RETURN_NOK;
            
            // Display test result
            guiDisplayRawString(ID_STRING_TEST_NOK);
            
            // Inform script of failure
            usbSendMessage(CMD_FUNCTIONAL_TEST_RES, 1, &script_return);
            while(1);
        }
    }
}
#endif