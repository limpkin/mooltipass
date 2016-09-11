#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#if defined(__AVR__) || defined(__i386__) //compatibility with Intel Galileo
 #define WIRE Wire
#else // Arduino Due
 #define WIRE Wire1
#endif

#include "mooltipass_touch_sensing.h"
#include <string.h>

/**************************************************************************/
/*!
    @brief  Instantiates a new touch sensing class
    @param  change	Location of the change pin
*/
/**************************************************************************/
mooltipass_touch_sensing::mooltipass_touch_sensing(void)
{
  DDRF &= ~(1 << 4);
  PORTF |= (1 << 4);
  DDRC |= (1 << 7);
  PORTC |= (1 << 7);
}

uint8_t mooltipass_touch_sensing::isTouchChangeDetected(void)
{
	return !(PINF & (1 << 4));
}

/*! \fn     readDataFromTS(uint8_t reg, uint8_t* data)
*   \brief  Read a byte inside the AT42QT2120
*   \param  reg         The register address
*   \param  data        uint8_t pointer in which we write the data
*   \return RETURN_OK if everything is alright, the pb code otherwise
*/
uint8_t mooltipass_touch_sensing::readDataFromTS(uint8_t reg, uint8_t* data)
{
    WIRE.beginTransmission(AT42QT2120_ADDR);
	WIRE.write(reg);
	Wire.endTransmission();
	WIRE.requestFrom((uint8_t)AT42QT2120_ADDR, (uint8_t)1);
	if (WIRE.available()==1)
	{
		*data = WIRE.read();
	}
    return RETURN_OK;
}

/*! \fn     writeDataToTS(uint8_t reg, uint8_t data)
*   \brief  Write a byte inside the AT42QT2120
*   \param  addr        The chip address
*   \param  reg         The register address
*   \param  data        The data to write
*   \return RETURN_OK if everything is alright, the pb code otherwise
*/
uint8_t mooltipass_touch_sensing::writeDataToTS(uint8_t reg, uint8_t data)
{
	WIRE.beginTransmission(AT42QT2120_ADDR);
    WIRE.write(reg);
    WIRE.write(data);
    WIRE.endTransmission();

    return RETURN_OK;
}

/*! \fn     checkTSPres()
*   \brief  Check that the AT42QT2120 is here
*   \return RETURN_OK or RETURN_NOK
*/
uint8_t mooltipass_touch_sensing::checkTSPres(void)
{
    uint8_t temp_return;
    uint8_t temp_byte;

    temp_return = readDataFromTS(REG_AT42QT_CHIP_ID, &temp_byte);
    if (temp_return != RETURN_OK)
    {
        return temp_return;
    }
    else if(temp_byte != AT42QT2120_ID)
    {
        return RETURN_NOK;
    }
    else
    {
        return RETURN_OK;
    }
}

/*! \fn     activateGuardKey(void)
*   \brief  Activate the guard key
*/
void mooltipass_touch_sensing::activateGuardKey(void)
{
    writeDataToTS(REG_AT42QT_KEY3_PULSE_SCL, 0x00);                                             // Disable proximity sensing
    writeDataToTS(REG_AT42QT_KEY3_CTRL, AT42QT2120_GUARD_VAL|AT42QT2120_AKS_GP1_MASK);          // Set key as guard
    launchCalibrationCycle();
}

/*! \fn     activateProxDetection(void)
*   \brief  Activate the proximity detection feature
*/
void mooltipass_touch_sensing::activateProxDetection(void)
{
    writeDataToTS(REG_AT42QT_KEY3_PULSE_SCL, 0x73);                                             // Activate proximity sensing
    writeDataToTS(REG_AT42QT_KEY3_CTRL, AT42QT2120_AKS_GP1_MASK);                               // Set as touch key
    launchCalibrationCycle();
}

/*! \fn     begin()
*   \brief  Initialize AT42QT2120
*/
uint8_t mooltipass_touch_sensing::begin(void)
{
	WIRE.begin();
	uint8_t temp_return = checkTSPres();

	if (temp_return == RETURN_OK)
	{
		// Perform measurements every 16ms
		writeDataToTS(REG_AT42QT_LP, 1);
		// LED settings
		writeDataToTS(REG_AT42QT_KEY4_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (top right)
		writeDataToTS(REG_AT42QT_KEY5_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (right button)
		writeDataToTS(REG_AT42QT_KEY6_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (bottom right)
		writeDataToTS(REG_AT42QT_KEY7_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (bottom left)
		writeDataToTS(REG_AT42QT_KEY8_CTRL, AT42QT2120_OUTPUT_H_VAL);                              // LED (left button)
		writeDataToTS(REG_AT42QT_KEY10_CTRL, AT42QT2120_OUTPUT_H_VAL);                             // LED (top left)
		// Sensitivity settings
		#ifndef LOW_SENSITIVITY
		writeDataToTS(REG_AT42QT_DI, 6);                                                           // Increase detection integrator value
		writeDataToTS(REG_AT42QT_KEY0_PULSE_SCL, 0x21);                                            // Oversample to gain one bit
		writeDataToTS(REG_AT42QT_KEY1_PULSE_SCL, 0x21);                                            // Oversample to gain one bit
		writeDataToTS(REG_AT42QT_KEY2_PULSE_SCL, 0x21);                                            // Oversample to gain one bit
		#endif
		writeDataToTS(REG_AT42QT_TRD, 50);                                                         // Recalibration if touch detected for more than 8 seconds
		// Key settings
		writeDataToTS(REG_AT42QT_KEY0_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Enable Wheel key
		writeDataToTS(REG_AT42QT_KEY1_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Enable Wheel key
		writeDataToTS(REG_AT42QT_KEY2_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Enable Wheel key
		writeDataToTS(REG_AT42QT_KEY9_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);     // Enable Left button
		writeDataToTS(REG_AT42QT_KEY11_CTRL, AT42QT2120_TOUCH_KEY_VAL|AT42QT2120_AKS_GP1_MASK);    // Enable Right button
		writeDataToTS(REG_AT42QT_SLID_OPT, 0x40);                                                  // Enable wheel
		writeDataToTS(REG_AT42QT_SLID_OPT, 0xC0);                                                  // Enable wheel
		activateGuardKey();                                                                   	   // Guard key
	}
	return temp_return;
}

/*! \fn     getLastRawWheelPosition(void)
*   \brief  Get the touched wheel position
*   \return The position
*/
uint8_t mooltipass_touch_sensing::getLastRawWheelPosition(void)
{
    return last_raw_wheel_position;
}

/*! \fn     getWheelTouchDetectionQuarter(void)
*   \brief  Get the touch quarter
*   \return The touched quarter
*/
uint8_t mooltipass_touch_sensing::getWheelTouchDetectionQuarter(void)
{
    if (last_raw_wheel_position < 0x3F)
    {
        return TOUCHPOS_WHEEL_TRIGHT;
    }
    else if (last_raw_wheel_position < 0x7F)
    {
        return TOUCHPOS_WHEEL_BRIGHT;
    }
    else if (last_raw_wheel_position < 0xBF)
    {
        return TOUCHPOS_WHEEL_BLEFT;
    }
    else
    {
        return TOUCHPOS_WHEEL_TLEFT;
    }
}

/*! \fn     touchClearCurrentDetections(void)
*   \brief  Clear interrupt line for detections
*/
void mooltipass_touch_sensing::touchClearCurrentDetections(void)
{
    uint8_t temp_uint;
    readDataFromTS(REG_AT42QT_SLIDER_POS, &temp_uint);
    readDataFromTS(REG_AT42QT_DET_STAT, &temp_uint);
    readDataFromTS(REG_AT42QT_KEY_STAT1, &temp_uint);
    readDataFromTS(REG_AT42QT_KEY_STAT2, &temp_uint);
}

/*! \fn     touchWaitForWheelReleased(void)
*   \brief  Wait for the user to remove his finger from the wheel
*/
void mooltipass_touch_sensing::touchWaitForWheelReleased(void)
{
    uint8_t keys_detection_status;

    do
    {
        readDataFromTS(REG_AT42QT_DET_STAT, &keys_detection_status);
    }
    while (keys_detection_status & AT42QT2120_SDET_MASK);
}

/*! \fn     touchDetectionRoutine(uint8_t led_mask)
*   \brief  Touch detection routine
*   \param  led_mask    Mask containing which LEDs to switchoff
*   \return Touch detection result (see touch_detect_return_t)
*/
uint8_t mooltipass_touch_sensing::touchDetectionRoutine(uint8_t led_mask)
{
    uint8_t return_val = RETURN_NO_CHANGE;
    uint8_t keys_detection_status;
    uint8_t led_states[NB_KEYS];
    uint8_t temp_bool = false;
    uint8_t temp_uint;

    // Set the LEDs on by default
    memset((void*)led_states, AT42QT2120_OUTPUT_H_VAL, NB_KEYS);

    // Switch them off depending on mask
    for (temp_uint = 0; temp_uint < NB_KEYS; temp_uint++)
    {
        if (led_mask & (1 << temp_uint))
        {
            led_states[temp_uint] = AT42QT2120_OUTPUT_L_VAL;
        }
    }

    if (isTouchChangeDetected())
    {
        // Set temp bool to true
        temp_bool = true;

        // Read detection status register
        readDataFromTS(REG_AT42QT_DET_STAT, &keys_detection_status);

        // Unused byte that needs to be read
        readDataFromTS(REG_AT42QT_KEY_STAT1, &temp_uint);

        // If wheel is touched
        if (keys_detection_status & AT42QT2120_SDET_MASK)
        {
            // Get position and update global var
            readDataFromTS(REG_AT42QT_SLIDER_POS, &last_raw_wheel_position);

            // Update LED states
            led_states[getWheelTouchDetectionQuarter()] = AT42QT2120_OUTPUT_L_VAL;
            return_val |= RETURN_WHEEL_PRESSED;
        }
        else
        {
            return_val |= RETURN_WHEEL_RELEASED;
        }

        // Read button touched register
        readDataFromTS(REG_AT42QT_KEY_STAT2, &temp_uint);

        // If one button is touched
        if ((keys_detection_status & AT42QT2120_TDET_MASK) && !(keys_detection_status & AT42QT2120_SDET_MASK))
        {
            if (temp_uint & 0x02)
            {
                // Left button
                led_states[TOUCHPOS_LEFT] = AT42QT2120_OUTPUT_L_VAL;
                return_val |= RETURN_LEFT_PRESSED;
                return_val |= RETURN_RIGHT_RELEASED;
            }
            else if(temp_uint & 0x08)
            {
                // Right button
                led_states[TOUCHPOS_RIGHT] = AT42QT2120_OUTPUT_L_VAL;
                return_val |= RETURN_RIGHT_PRESSED;
                return_val |= RETURN_LEFT_RELEASED;
            }
            else
            {
                return_val |= RETURN_PROX_DETECTION;
            }
        }
        else
        {
            return_val |= RETURN_PROX_RELEASED;
            return_val |= RETURN_LEFT_RELEASED;
            return_val |= RETURN_RIGHT_RELEASED;
        }

        // Switch on cathode if activity
        if (return_val & TOUCH_PRESS_MASK)
        {
            //activityDetectedRoutine();
        }
    }

    // If there's a touch change or led mask has changed
    if ((temp_bool == true) || (led_mask != last_led_mask))
    {
        last_led_mask = led_mask;
        writeDataToTS(LEFT_LED_REGISTER, led_states[TOUCHPOS_LEFT]);
        writeDataToTS(RIGHT_LED_REGISTER, led_states[TOUCHPOS_RIGHT]);
        writeDataToTS(WHEEL_TLEFT_LED_REGISTER, led_states[TOUCHPOS_WHEEL_TLEFT]);
        writeDataToTS(WHEEL_TRIGHT_LED_REGISTER, led_states[TOUCHPOS_WHEEL_TRIGHT]);
        writeDataToTS(WHEEL_BLEFT_LED_REGISTER, led_states[TOUCHPOS_WHEEL_BLEFT]);
        writeDataToTS(WHEEL_BRIGHT_LED_REGISTER,  led_states[TOUCHPOS_WHEEL_BRIGHT]);
    }

    return return_val;
}
