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
/*! \file   logic_fwflash_storage.h
 *  \brief  Logic for storing/getting fw data in the dedicated flash storage
 *  Copyright [2014] [Mathieu Stephan]
 */


#ifndef LOGIC_FWFLASH_STORAGE_H_
#define LOGIC_FWFLASH_STORAGE_H_

#include "defines.h"

// Defines
#define BITMAP_ID_OFFSET        128
#define MEDIA_TYPE_LENGTH       2

// Media file types
#define MEDIA_BITMAP            1
#define MEDIA_FONT              2

// Buffer size
#define TEXTBUFFERSIZE  32

#if defined(HARDWARE_OLIVIER_V1)
    // Font IDs
    #define FONT_NONE           255
    #define FONT_PROFONT_10     7
    #define FONT_PROFONT_18     16
    #define FONT_PROFONT_24     17
    #define FONT_DEFAULT FONT_PROFONT_10

    // Bimap IDs
    #define BITMAP_MOOLTIPASS     0
    #define BITMAP_LOGIN          1
    #define BITMAP_LOGIN_RARROW   2
    #define BITMAP_YES_NO         3
    #define BITMAP_TICK           4
    #define BITMAP_CROSS          5
    #define BITMAP_INFO           6
    #define BITMAP_INSERT         8
    #define BITMAP_MAIN_SCREEN    9
    #define BITMAP_SETTINGS_SC    10
    #define BITMAP_LEFT_ARROW     11
    #define BITMAP_RIGHT_ARROW    12
    #define BITMAP_PIN_LINES      13
    #define BITMAP_ZZZ            14
    #define BITMAP_LOGIN_FIND     15
    #define BITMAP_YES_NO_INT_L   40
    #define BITMAP_YES_NO_INT_R   41
    #define BITMAP_ZZZ_LOCKED     42
    #define BITMAP_TUTORIAL_1     43
    #define BITMAP_TUTORIAL_2     44
    #define BITMAP_TUTORIAL_3     45
    #define BITMAP_TUTORIAL_4     46
    #define BITMAP_TUTORIAL_5     47
    #define BITMAP_TUTORIAL_6     48
    #define BITMAP_EGG            49
    #define BITMAP_EGG_END        50

    // String IDs
    #define ID_STRING_PROCESSING        0
    #define ID_STRING_CARD_BLOCKED      1
    #define ID_STRING_PB_CARD           2
    #define ID_STRING_WRONG_PIN         3
    #define ID_STRING_REMOVE_CARD       4
    #define ID_STRING_INSERT_OTHER      5
    #define ID_STRING_FAILED            6
    #define ID_STRING_PIN_CHANGED       7
    #define ID_STRING_PIN_NCGHANGED     8
    #define ID_STRING_USER_ADDED        9
    #define ID_STRING_USER_NADDED       10
    #define ID_STRING_CARD_UNLOCKED     11
    #define ID_STRING_CARDID_NFOUND     12
    #define ID_STRING_INSERT_NCARD      13
    #define ID_STRING_DONE              14
    #define ID_STRING_CARD_REMOVED      15
    #define ID_STRING_AREYOUSURE        16
    #define ID_STRING_AREYOURLSURE      17
    #define ID_STRING_OTHECARDFUSER     18
    #define ID_STRING_ENTERLOGINQ       19
    #define ID_STRING_ENTERPASSQ        20
    #define ID_STRING_APPROVEMEMOP      21
    #define ID_STRING_INSERT_PIN        22
    #define ID_STRING_PIN_NEW_CARD      23
    #define ID_STRING_CONF_PIN          24
    #define ID_STRING_CONFACCESSTO      25
    #define ID_STRING_WITHTHISLOGIN     26
    #define ID_STRING_CONF_NEWCREDS     27
    #define ID_STRING_ADDUSERNAME       28
    #define ID_STRING_ON                29
    #define ID_STRING_CHANGEPASSFOR     30
    #define ID_STRING_WRONGPIN1LEFT     31
    #define ID_STRING_WRONGPIN2LEFT     32
    #define ID_STRING_WRONGPIN3LEFT     33
    #define ID_STRING_NEWMP_USER        34
    #define ID_STRING_GOINGTOSLEEP      35
    #define ID_STRING_MEMORYMGMTQ       36
    #define ID_STRING_MEMORYMGMT        37
    #define ID_STRING_CLOSEMEMMGMT      38
    #define ID_STRING_CREATEDBYPLUG     39
    #define ID_STRING_NOSTOREDFAV       40
    #define ID_STRING_SEND_SMC_PASS     41
    #define ID_STRING_SET_SMC_LOGIN     42
    #define ID_STRING_SET_SMC_PASS      43
    #define ID_STRING_YOUR_USERNAME     44
    #define ID_STRING_SHOW_LOGINQ       45
    #define ID_STRING_SHOW_PASSQ        46
    #define ID_STRING_TEST_FLASH_OK     47
    #define ID_STRING_TEST_FLASH_PB     48
    #define ID_STRING_TEST_TOUCH_OK     49
    #define ID_STRING_TEST_TOUCH_PB     50
    #define ID_STRING_TEST_INST_TCH     51
    #define ID_STRING_TEST_DET          52
    #define ID_STRING_TEST_LEFT         53
    #define ID_STRING_TEST_WHEEL        54
    #define ID_STRING_TEST_RIGHT        55
    #define ID_STRING_TEST_CARD_INS     56
    #define ID_STRING_TEST_CARD_OK      57
    #define ID_STRING_TEST_CARD_PB      58
    #define ID_STRING_TEST_LEDS_CH      59
    #define ID_STRING_TEST_OK           60
    #define ID_STRING_TEST_NOK          61
    #define ID_STRING_WARNING           62
    #define ID_STRING_ALLOW_UPDATE      63
    #define ID_STRING_PC_SLEEP          64
    #define ID_STRING_CONF_NEWDATA      65
    #define ID_STRING_ADD_DATA_FOR      66
    #define ID_STRING_GET_DATA_FOR      67
    #define ID_STRING_PIN_DIFF          68
    #define ID_STRING_TGT_CARD_NBL      69
    #define ID_STRING_NEW_PINQ          70
    #define ID_STRING_ERASE_TCARD       71
    #define ID_STRING_NO_CREDS          72
    #define ID_STRING_FUSE_PB           73
    #define ID_STRING_PIN_COMPUTER      74
    #define ID_STRING_SEND_PASS_FOR     75
    #define ID_STRING_CHANGE_DESC_FOR   76
    #define ID_STRING_UPDATE_DATA_FOR   77
#elif defined(MINI_VERSION)
    // Font IDs
    #define FONT_NONE               255
    #define FONT_CC_REGULAR         1
    #define FONT_PROFONT_14         2
    #define FONT_8BIT16             11
    #define FONT_DEFAULT            FONT_CC_REGULAR

    // Bitmap IDs
    #define BITMAP_MOOLTIPASS       0
    #define BITMAP_APPROVE          3
    #define BITMAP_DENY             4
    #define BITMAP_INSERT_CARD      5
    #define BITMAP_PIN_SLOT1        6
    #define BITMAP_PIN_SLOT2        7
    #define BITMAP_PIN_SLOT3        8
    #define BITMAP_PIN_SLOT4        9
    #define BITMAP_SCROLL_WHEEL     10   
    #define BITMAP_LOGIN_LPANE      12
    #define BITMAP_ZZZ              13 
    #define BITMAP_UPDATING         14
    #define BITMAP_LOCK_FULL        40
    #define BITMAP_LOCK_EMPTY       41
    #define BITMAP_PAC_FULL         42
    #define BITMAP_PAC_RIGHT        43
    #define BITMAP_PAC_RIGHT2       44
    #define BITMAP_PAC_BOT          45
    #define BITMAP_PAC_BOT2         46
    #define BITMAP_PAC_LEFT         47
    #define BITMAP_PAC_LEFT2        48
    #define BITMAP_PAC_UP           49
    #define BITMAP_PAC_UP2          50
    #define BITMAP_TUTORIAL_1       59
    #define BITMAP_TUTORIAL_2       60
    #define BITMAP_TUTORIAL_3       61
    #define BITMAP_TUTORIAL_4       62
    #define BITMAP_TUTORIAL_5       63
    #define BITMAP_MAIN_LOCK        64
    #define BITMAP_MAIN_LOGIN       68
    #define BITMAP_MAIN_FAVORITES   72
    #define BITMAP_MAIN_SETTINGS    76
    #define BITMAP_SETTINGS_PIN     80
    #define BITMAP_SETTINGS_BACKUP  84
    #define BITMAP_SETTINGS_HOME    88
    #define BITMAP_SETTINGS_ERASE   92

    // To reintegrate?
    #define BITMAP_EGG            49
    #define BITMAP_EGG_END        50

    // String IDs
    #define ID_STRING_PROCESSING        0
    #define ID_STRING_CARD_BLOCKED      1
    #define ID_STRING_PB_CARD           2
    #define ID_STRING_REMOVE_CARD       3
    #define ID_STRING_INSERT_OTHER      4
    #define ID_STRING_FAILED            5
    #define ID_STRING_PIN_CHANGED       6
    #define ID_STRING_PIN_NCGHANGED     7
    #define ID_STRING_USER_ADDED        8
    #define ID_STRING_USER_NADDED       9
    #define ID_STRING_CARD_UNLOCKED     10
    #define ID_STRING_CARDID_NFOUND     11
    #define ID_STRING_INSERT_NCARD      12
    #define ID_STRING_DONE              13
    #define ID_STRING_CARD_REMOVED      14
    #define ID_STRING_AREYOUSURE        15
    #define ID_STRING_AREYOURLSURE      16
    #define ID_STRING_OTHECARDFUSER     17
    #define ID_STRING_ENTERLOGINQ       18
    #define ID_STRING_ENTERPASSQ        19
    #define ID_STRING_INSERT_PIN        20
    #define ID_STRING_PIN_NEW_CARD      21
    #define ID_STRING_CONF_PIN          22
    #define ID_STRING_CONFACCESSTO      23
    #define ID_STRING_WITHTHISLOGIN     24
    #define ID_STRING_CONF_NEWCREDS     25
    #define ID_STRING_ADDUSERNAME       26
    #define ID_STRING_CHANGEPASSFOR     27
    #define ID_STRING_WRONGPIN1LEFT     28
    #define ID_STRING_WRONGPIN2LEFT     29
    #define ID_STRING_WRONGPIN3LEFT     30
    #define ID_STRING_NEWMP_USER        31
    #define ID_STRING_GOINGTOSLEEP      32
    #define ID_STRING_MEMORYMGMTQ       33
    #define ID_STRING_MEMORYMGMT        34
    #define ID_STRING_CLOSEMEMMGMT      35
    #define ID_STRING_CREATEDBYPLUG     36
    #define ID_STRING_NOSTOREDFAV       37
    #define ID_STRING_SEND_SMC_PASS     38
    #define ID_STRING_SET_SMC_LOGIN     39
    #define ID_STRING_SET_SMC_PASS      40
    #define ID_STRING_YOUR_USERNAME     41
    #define ID_STRING_SHOW_LOGINQ       42
    #define ID_STRING_SHOW_PASSQ        43
    #define ID_STRING_TEST_FLASH_PB     44    
    #define ID_STRING_TEST_CARD_INS     45
    #define ID_STRING_TEST_CARD_PB      46
    #define ID_STRING_TEST_NOK          47
    #define ID_STRING_WARNING           48
    #define ID_STRING_ALLOW_UPDATE      49
    #define ID_STRING_PC_SLEEP          50
    #define ID_STRING_CONF_NEWDATA      51
    #define ID_STRING_ADD_DATA_FOR      52
    #define ID_STRING_GET_DATA_FOR      53
    #define ID_STRING_PIN_DIFF          54
    #define ID_STRING_TGT_CARD_NBL      55
    #define ID_STRING_NEW_PINQ          56
    #define ID_STRING_ERASE_TCARD       57
    #define ID_STRING_NO_CREDS          58
    #define ID_STRING_FUSE_PB           59
    #define ID_STRING_FUNC_TEST         60
    #define ID_STRING_RESERVED2         61
    #define ID_STRING_FUNC_TEST_SCROLL  62
    #define ID_STRING_PIN_COMPUTER      63
    #define ID_STRING_SEND_PASS_FOR     64
    #define ID_STRING_SELECT_CREDENTIAL 65
    #define ID_STRING_INPUT_PB          66
    #define ID_STRING_LED1              67
    #define ID_STRING_LED2              68
    #define ID_STRING_LED3              69
    #define ID_STRING_LED4              70
    #define ID_STRING_RESERVED          71
    #define ID_STRING_HASH1             72
    #define ID_STRING_HASH2             73
    #define ID_STRING_CHANGE_DESC_FOR   74
    #define ID_STRING_DO_NOT_UNPLUG     75
    #define ID_STRING_LAST_PIN_TRY      76
    #define ID_STRING_UPDATE_DATA_FOR   77

#ifdef ENABLE_CREDENTIAL_MANAGEMENT
    /* reserved for main firmware branch usage
     * can be removed as they are added above */
    #define ID_STRING_MGMT_RESERVED8            78
    #define ID_STRING_MGMT_RESERVED9            79

    /* on-device credential management strings */
    #define ID_STRING_MGMT_CREATE               80
    #define ID_STRING_MGMT_EDIT                 81
    #define ID_STRING_MGMT_RENEW                82
    #define ID_STRING_MGMT_DELETE               83
    #define ID_STRING_MGMT_TYPE_SVCNAME         84
    #define ID_STRING_MGMT_TYPE_LOGIN           85
    #define ID_STRING_MGMT_ENABLE_ALPHA_UPPERQ  86
    #define ID_STRING_MGMT_ENABLE_ALPHA_LOWERQ  87
    #define ID_STRING_MGMT_ENABLE_NUMQ          88
    #define ID_STRING_MGMT_ENABLE_SPECIALSQ     89
    #define ID_STRING_MGMT_ENABLE_SPACEQ        90
    #define ID_STRING_MGMT_CHARSET_ALPHA_UPPER  91
    #define ID_STRING_MGMT_CHARSET_ALPHA_LOWER  92
    #define ID_STRING_MGMT_CHARSET_NUM          93
    #define ID_STRING_MGMT_CHARSET_SPECIALS1    94
    #define ID_STRING_MGMT_CHARSET_SPECIALS2    95
    #define ID_STRING_MGMT_CHARSET_SPECIALS3    96
    #define ID_STRING_MGMT_CHARSET_SPECIALS4    97
    #define ID_STRING_MGMT_ENTER_OLDPASSQ       98
    #define ID_STRING_MGMT_ENTER_NEWPASSQ       99
    #define ID_STRING_MGMT_ENTER_NEWPASS_AGAINQ 100
    #define ID_STRING_MGMT_GENERATE_NEW_PASSQ   101
    #define ID_STRING_MGMT_EDIT_SVCNAMEQ        102
    #define ID_STRING_MGMT_EDIT_LOGINQ          103
    #define ID_STRING_MGMT_EDIT_CHARSETQ        104
    #define ID_STRING_MGMT_DELETE_CREDSQ        105
    #define ID_STRING_MGMT_AREYOUSUREQ          106
    #define ID_STRING_MGMT_OPSUCCESS            107
    #define ID_STRING_MGMT_OPFAILURE            108
    #define ID_STRING_MGMT_SAVETOFLASHQ         109
    #define ID_STRING_MGMT_PASSWORDLENGTHQ      110
    #define ID_STRING_MGMT_CREATENEWSERVICEQ    111
    #define ID_STRING_MGMT_NOSERVICEAVAILABLE   112

    /* string ID boundaries */
    #define ID_FIRST_STRING             0
    #define ID_LAST_STRING              ID_STRING_MGMT_NOSERVICEAVAILABLE
#else
    #define ID_FIRST_STRING             0
    #define ID_LAST_STRING              ID_STRING_LED4
#endif
    #define NB_BMPS_PER_TRANSITION      4
#endif

// Keyboard LUTs
// Changes: 
// - ID_KEYB_MX_MX_LUT removed (same as ID_KEYB_ES_AR_LUT)
// - ID_KEYB_SW_SW_LUT removed (same as ID_KEYB_FI_FI_LUT)
// - ID_KEYB_DE_AU_LUT removed (same as ID_KEYB_DE_DE_LUT)
// - ID_KEYB_ZH_HK_LUT ID_KEYB_JP_JP_LUT ID_KEYB_KO_KO_LUT ID_KEYB_EN_NZ_LUT ID_KEYB_EN_SG_LUT ID_KEYB_EN_SA_LUT removed (same as ID_KEYB_EN_AU_LUT)
#define ID_KEYB_EN_US_LUT       BITMAP_ID_OFFSET+18
#define ID_KEYB_FR_FR_LUT       BITMAP_ID_OFFSET+19
#define ID_KEYB_ES_ES_LUT       BITMAP_ID_OFFSET+20
#define ID_KEYB_DE_DE_LUT       BITMAP_ID_OFFSET+21
#define ID_KEYB_ES_AR_LUT       BITMAP_ID_OFFSET+22
#define ID_KEYB_EN_AU_LUT       BITMAP_ID_OFFSET+23
#define ID_KEYB_FR_BE_LUT       BITMAP_ID_OFFSET+24
#define ID_KEYB_PO_BR_LUT       BITMAP_ID_OFFSET+25
#define ID_KEYB_EN_CA_LUT       BITMAP_ID_OFFSET+26
#define ID_KEYB_CZ_CZ_LUT       BITMAP_ID_OFFSET+27
#define ID_KEYB_DA_DK_LUT       BITMAP_ID_OFFSET+28
#define ID_KEYB_FI_FI_LUT       BITMAP_ID_OFFSET+29
#define ID_KEYB_HU_HU_LUT       BITMAP_ID_OFFSET+30
#define ID_KEYB_IS_IS_LUT       BITMAP_ID_OFFSET+31
#define ID_KEYB_IT_IT_LUT       BITMAP_ID_OFFSET+32  // So it seems Italian keyboards don't have ~`
#define ID_KEYB_NL_NL_LUT       BITMAP_ID_OFFSET+33
#define ID_KEYB_NO_NO_LUT       BITMAP_ID_OFFSET+34
#define ID_KEYB_PO_PO_LUT       BITMAP_ID_OFFSET+35  // Polish programmer keyboard
#define ID_KEYB_RO_RO_LUT       BITMAP_ID_OFFSET+36
#define ID_KEYB_SL_SL_LUT       BITMAP_ID_OFFSET+37
#define ID_KEYB_FRDE_CH_LUT     BITMAP_ID_OFFSET+38
#define ID_KEYB_EN_UK_LUT       BITMAP_ID_OFFSET+39
#define ID_KEYB_CZ_QWERTY_LUT   BITMAP_ID_OFFSET+51
#define ID_KEYB_EN_DV_LUT       BITMAP_ID_OFFSET+52
#define ID_KEYB_FR_MAC_LUT      BITMAP_ID_OFFSET+53
#define ID_KEYB_FR_CH_MAC_LUT   BITMAP_ID_OFFSET+54
#define ID_KEYB_DE_CH_MAC_LUT   BITMAP_ID_OFFSET+55
#define ID_KEYB_DE_MAC_LUT      BITMAP_ID_OFFSET+56
#define ID_KEYB_US_MAC_LUT      BITMAP_ID_OFFSET+57
#define FIRST_KEYB_LUT          ID_KEYB_EN_US_LUT
#define LAST_KEYB_LUT           ID_KEYB_US_MAC_LUT

// Prototypes
uint8_t getKeybLutEntryForLayout(uint8_t layout, uint8_t ascii_char);
RET_TYPE getStoredFileAddr(uint16_t fileId, uint16_t* addr);
char* readStoredStringToBuffer(uint8_t stringID);

// Global variables
extern uint8_t textBuffer1[TEXTBUFFERSIZE];
extern uint8_t textBuffer2[TEXTBUFFERSIZE];

#endif /* LOGIC_FWFLASH_STORAGE_H_ */
