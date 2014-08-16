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
/*! \file   touch_higher_level_functions.h
 *  \brief  Touch higher level sensing functions
 *  Copyright [2014] [Mathieu Stephan]
 */

#ifndef TOUCH_HIGHER_LEVEL_FUNCTIONS_H_
#define TOUCH_HIGHER_LEVEL_FUNCTIONS_H_

#include "defines.h"
#include "touch.h"

// Prototypes
void activateGuardKey(void);
RET_TYPE initTouchSensing(void);
void activateProxDetection(void);
uint8_t getLastRawWheelPosition(void);
void touchClearCurrentDetections(void);
uint8_t getWheelTouchDetectionQuarter(void);
RET_TYPE touchDetectionRoutine(uint8_t led_mask);

// AT42QT2120 ID
#define AT42QT2120_ID       0x3E

// AT42QT2120 masks defines
#define AT42QT2120_ADDR             (0x1C << 1)
#define AT42QT2120_TDET_MASK        0x01
#define AT42QT2120_SDET_MASK        0x02
#define AT42QT2120_TOUCH_KEY_VAL    0x00
#define AT42QT2120_OUTPUT_L_VAL     0x01
#define AT42QT2120_OUTPUT_H_VAL     0x03
#define AT42QT2120_GUARD_VAL        0x10
#define AT42QT2120_AKS_GP1_MASK     0x04

// AT42QT2120 registers defines
#define REG_AT42QT_CHIP_ID          0x00
#define REG_AT42QT_FW_VER           0x01
#define REG_AT42QT_DET_STAT         0x02
#define REG_AT42QT_KEY_STAT1        0x03
#define REG_AT42QT_KEY_STAT2        0x04
#define REG_AT42QT_SLIDER_POS       0x05
#define REG_AT42QT_CALIB            0x06
#define REG_AT42QT_nRESET           0x07
#define REG_AT42QT_LP               0x08
#define REG_AT42QT_TTD              0x09
#define REG_AT42QT_ATD              0x0A
#define REG_AT42QT_DI               0x0B
#define REG_AT42QT_TRD              0x0C
#define REG_AT42QT_DHT              0x0D
#define REG_AT42QT_SLID_OPT         0x0E
#define REG_AT42QT_CHARGE_TIME      0x0F
#define REG_AT42QT_KEY0_DET_THR     0x10
#define REG_AT42QT_KEY1_DET_THR     0x11
#define REG_AT42QT_KEY2_DET_THR     0x12
#define REG_AT42QT_KEY3_DET_THR     0x13
#define REG_AT42QT_KEY4_DET_THR     0x14
#define REG_AT42QT_KEY5_DET_THR     0x15
#define REG_AT42QT_KEY6_DET_THR     0x16
#define REG_AT42QT_KEY7_DET_THR     0x17
#define REG_AT42QT_KEY8_DET_THR     0x18
#define REG_AT42QT_KEY9_DET_THR     0x19
#define REG_AT42QT_KEY10_DET_THR    0x1A
#define REG_AT42QT_KEY11_DET_THR    0x1B
#define REG_AT42QT_KEY0_CTRL        0x1C
#define REG_AT42QT_KEY1_CTRL        0x1D
#define REG_AT42QT_KEY2_CTRL        0x1E
#define REG_AT42QT_KEY3_CTRL        0x1F
#define REG_AT42QT_KEY4_CTRL        0x20
#define REG_AT42QT_KEY5_CTRL        0x21
#define REG_AT42QT_KEY6_CTRL        0x22
#define REG_AT42QT_KEY7_CTRL        0x23
#define REG_AT42QT_KEY8_CTRL        0x24
#define REG_AT42QT_KEY9_CTRL        0x25
#define REG_AT42QT_KEY10_CTRL       0x26
#define REG_AT42QT_KEY11_CTRL       0x27
#define REG_AT42QT_KEY0_PULSE_SCL   0x28
#define REG_AT42QT_KEY1_PULSE_SCL   0x29
#define REG_AT42QT_KEY2_PULSE_SCL   0x2A
#define REG_AT42QT_KEY3_PULSE_SCL   0x2B
#define REG_AT42QT_KEY4_PULSE_SCL   0x2C
#define REG_AT42QT_KEY5_PULSE_SCL   0x2D
#define REG_AT42QT_KEY6_PULSE_SCL   0x2E
#define REG_AT42QT_KEY7_PULSE_SCL   0x2F
#define REG_AT42QT_KEY8_PULSE_SCL   0x30
#define REG_AT42QT_KEY9_PULSE_SCL   0x31
#define REG_AT42QT_KEY10_PULSE_SCL  0x32
#define REG_AT42QT_KEY11_PULSE_SCL  0x33
#define REG_AT42QT_KEY0_SIG_MSB     0x34
#define REG_AT42QT_KEY0_SIG_LSB     0x35
#define REG_AT42QT_KEY1_SIG_MSB     0x36
#define REG_AT42QT_KEY1_SIG_LSB     0x37
#define REG_AT42QT_KEY2_SIG_MSB     0x38
#define REG_AT42QT_KEY2_SIG_LSB     0x39
#define REG_AT42QT_KEY3_SIG_MSB     0x3A
#define REG_AT42QT_KEY3_SIG_LSB     0x3B
#define REG_AT42QT_KEY4_SIG_MSB     0x3C
#define REG_AT42QT_KEY4_SIG_LSB     0x3D
#define REG_AT42QT_KEY5_SIG_MSB     0x3E
#define REG_AT42QT_KEY5_SIG_LSB     0x3F
#define REG_AT42QT_KEY6_SIG_MSB     0x40
#define REG_AT42QT_KEY6_SIG_LSB     0x41
#define REG_AT42QT_KEY7_SIG_MSB     0x42
#define REG_AT42QT_KEY7_SIG_LSB     0x43
#define REG_AT42QT_KEY8_SIG_MSB     0x44
#define REG_AT42QT_KEY8_SIG_LSB     0x45
#define REG_AT42QT_KEY9_SIG_MSB     0x46
#define REG_AT42QT_KEY9_SIG_LSB     0x47
#define REG_AT42QT_KEY10_SIG_MSB    0x48
#define REG_AT42QT_KEY10_SIG_LSB    0x49
#define REG_AT42QT_KEY11_SIG_MSB    0x4A
#define REG_AT42QT_KEY11_SIG_LSB    0x4B
#define REG_AT42QT_REF_DATA0_MSB    0x4C
#define REG_AT42QT_REF_DATA0_LSB    0x4D
#define REG_AT42QT_REF_DATA1_MSB    0x4E
#define REG_AT42QT_REF_DATA1_LSB    0x4F
#define REG_AT42QT_REF_DATA2_MSB    0x50
#define REG_AT42QT_REF_DATA2_LSB    0x51
#define REG_AT42QT_REF_DATA3_MSB    0x52
#define REG_AT42QT_REF_DATA3_LSB    0x53
#define REG_AT42QT_REF_DATA4_MSB    0x54
#define REG_AT42QT_REF_DATA4_LSB    0x55
#define REG_AT42QT_REF_DATA5_MSB    0x56
#define REG_AT42QT_REF_DATA5_LSB    0x57
#define REG_AT42QT_REF_DATA6_MSB    0x58
#define REG_AT42QT_REF_DATA6_LSB    0x59
#define REG_AT42QT_REF_DATA7_MSB    0x5A
#define REG_AT42QT_REF_DATA7_LSB    0x5B
#define REG_AT42QT_REF_DATA8_MSB    0x5C
#define REG_AT42QT_REF_DATA8_LSB    0x5D
#define REG_AT42QT_REF_DATA9_MSB    0x5E
#define REG_AT42QT_REF_DATA9_LSB    0x5F
#define REG_AT42QT_REF_DATA10_MSB   0x60
#define REG_AT42QT_REF_DATA10_LSB   0x61
#define REG_AT42QT_REF_DATA11_MSB   0x62
#define REG_AT42QT_REF_DATA11_LSB   0x63

// LEDs register defines
#define LEFT_LED_REGISTER           REG_AT42QT_KEY8_CTRL
#define RIGHT_LED_REGISTER          REG_AT42QT_KEY5_CTRL
#define WHEEL_TLEFT_LED_REGISTER    REG_AT42QT_KEY10_CTRL
#define WHEEL_TRIGHT_LED_REGISTER   REG_AT42QT_KEY4_CTRL
#define WHEEL_BLEFT_LED_REGISTER    REG_AT42QT_KEY7_CTRL
#define WHEEL_BRIGHT_LED_REGISTER   REG_AT42QT_KEY6_CTRL

// Macros
#define isTouchChangeDetected()     !(PINF & (1 << PORTID_TOUCH_C))
#define launchCalibrationCycle()    writeDataToTS(REG_AT42QT_CALIB, 0x12)
#define switchOnLeftButonLed()      writeDataToTS(LEFT_LED_REGISTER, AT42QT2120_OUTPUT_H_VAL)
#define switchOffLeftButonLed()     writeDataToTS(LEFT_LED_REGISTER, AT42QT2120_OUTPUT_L_VAL)
#define switchOnRightButonLed()     writeDataToTS(RIGHT_LED_REGISTER, AT42QT2120_OUTPUT_H_VAL)
#define switchOffRightButonLed()    writeDataToTS(RIGHT_LED_REGISTER, AT42QT2120_OUTPUT_L_VAL)
#define switchOnTopLeftWheelLed()   writeDataToTS(WHEEL_TLEFT_LED_REGISTER, AT42QT2120_OUTPUT_H_VAL)
#define switchOffTopLeftWheelLed()  writeDataToTS(WHEEL_TLEFT_LED_REGISTER, AT42QT2120_OUTPUT_L_VAL)
#define switchOnTopRightWheelLed()  writeDataToTS(WHEEL_TRIGHT_LED_REGISTER, AT42QT2120_OUTPUT_H_VAL)
#define switchOffTopRightWheelLed() writeDataToTS(WHEEL_TRIGHT_LED_REGISTER, AT42QT2120_OUTPUT_L_VAL)
#define switchOnBotLeftWheelLed()   writeDataToTS(WHEEL_BLEFT_LED_REGISTER, AT42QT2120_OUTPUT_H_VAL)
#define switchOffBotLeftWheelLed()  writeDataToTS(WHEEL_BLEFT_LED_REGISTER, AT42QT2120_OUTPUT_L_VAL)
#define switchOnBotRightWheelLed()  writeDataToTS(WHEEL_BRIGHT_LED_REGISTER, AT42QT2120_OUTPUT_H_VAL)
#define switchOffBotRightWheelLed() writeDataToTS(WHEEL_BRIGHT_LED_REGISTER, AT42QT2120_OUTPUT_L_VAL)
#define switchOffButtonWheelLeds()  switchOffLeftButonLed(); switchOffRightButonLed(); switchOffTopLeftWheelLed(); switchOffTopRightWheelLed(); switchOffBotLeftWheelLed(); switchOffBotRightWheelLed();
#define switchOnButtonWheelLeds()   switchOnLeftButonLed(); switchOnRightButonLed();  switchOnTopLeftWheelLed(); switchOnTopRightWheelLed(); switchOnBotLeftWheelLed(); switchOnBotRightWheelLed();

// Touch detect defines
#define TOUCH_PRESS_MASK    (RETURN_LEFT_PRESSED | RETURN_RIGHT_PRESSED | RETURN_WHEEL_PRESSED | RETURN_PROX_DETECTION)

// Touch detection defines
#define NB_KEYS                         6
#define TOUCHPOS_WHEEL_TLEFT            0
#define TOUCHPOS_WHEEL_TRIGHT           1
#define TOUCHPOS_WHEEL_BLEFT            2
#define TOUCHPOS_WHEEL_BRIGHT           3
#define TOUCHPOS_LEFT                   4
#define TOUCHPOS_RIGHT                  5

// LED mask define
#define LED_MASK_WHEEL_TLEFT            (1 << TOUCHPOS_WHEEL_TLEFT)
#define LED_MASK_WHEEL_TRIGHT           (1 << TOUCHPOS_WHEEL_TRIGHT)
#define LED_MASK_WHEEL_BLEFT            (1 << TOUCHPOS_WHEEL_BLEFT)
#define LED_MASK_WHEEL_BRIGHT           (1 << TOUCHPOS_WHEEL_BRIGHT)
#define LED_MASK_LEFT                   (1 << TOUCHPOS_LEFT)
#define LED_MASK_RIGHT                  (1 << TOUCHPOS_RIGHT)
#define LED_MASK_WHEEL                  (LED_MASK_WHEEL_TLEFT|LED_MASK_WHEEL_TRIGHT|LED_MASK_WHEEL_BLEFT|LED_MASK_WHEEL_BRIGHT)

#endif /* TOUCH_HIGHER_LEVEL_FUNCTIONS_H_ */