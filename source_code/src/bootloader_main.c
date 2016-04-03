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
/*! \file   bootloader_main.c
 *  \brief  main file for bootloader
 *  Copyright [2016] [Mathieu Stephan]
 */
#include <avr/eeprom.h>
#include <stdlib.h>
#include <string.h>
#include "eeprom_addresses.h"
#include "aes256_ctr.h"
#include "node_mgmt.h"
#include "flash_mem.h"
#include "defines.h"
#include "aes.h"
#include "spi.h"

// Define the bootloader function
bootloader_f_ptr_type start_bootloader = (bootloader_f_ptr_type)0x3800;
// Define the main program function
bootloader_f_ptr_type start_program = (bootloader_f_ptr_type)0x0000;


/*! \fn     disableJTAG(void)
*   \brief  Disable the JTAG module
*/
static inline void disableJTAG(void)
{
    unsigned char temp;

    temp = MCUCR;
    temp |= (1<<JTD);
    MCUCR = temp;
    MCUCR = temp;
}

/*! \fn     electricalJumpToBootloaderCondition(void)
 *  \brief  Electrical condition to jump to the bootloader
 *  \return Boolean to know if this condition is fullfilled
 */
 static inline RET_TYPE electricalJumpToBootloaderCondition(void)
 {
    /* Disable JTAG to get access to the pins */
    disableJTAG();
    
    /* Pressing wheel starts the bootloader */
    DDR_CLICK &= ~(1 << PORTID_CLICK);
    PORT_CLICK |= (1 << PORTID_CLICK);
    
    /* Small delay for detection */
    for (uint16_t i = 0; i < 20000; i++) asm volatile ("NOP");
    
    /* Check if low */
    if (!(PIN_CLICK & (1 << PORTID_CLICK)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*! \fn     main(void)
*   \brief  Main function
*/
int main(void)
{
    /* Fetch bootkey in eeprom */
    uint16_t current_bootkey_val = eeprom_read_word((uint16_t*)EEP_BOOTKEY_ADDR);   // Bootkey in EEPROM
    uint8_t cur_aes_key[AES_KEY_LENGTH/8];                                          // AES encryption key
    aes256_context temp_aes_context;                                                // AES context
    uint8_t cur_cbc_mac[16];                                                        // Current CBCMAC val    
    uint8_t temp_data[16];                                                          // Temporary 16 bytes vector
    RET_TYPE flash_init_result;                                                     // Flash initialization result

    /* TODO: check fuses? */
    
    /* Disable JTAG & set pre-scaler to 0 */
    disableJTAG();
    CPU_PRESCALE(0);
    
    /* See if we actually wanted to start the bootloader */
    //#define BOOTKEY_CHECK
    #ifdef BOOTKEY_CHECK
        if (current_bootkey_val != BOOTLOADER_BOOTKEY)
        {
            start_program();
        }
    #else
        (void)current_bootkey_val;
    #endif

    /* Early development stages, when we develop in main firmware memory */
    #define EARLY_DEV
    #ifdef EARLY_DEV
    if(electricalJumpToBootloaderCondition() == TRUE)
    {
        start_bootloader();
    }
    #endif

    /* Initialize SPI controller, check flash presence */
    spiUsartBegin();
    flash_init_result = initFlash();
    if (flash_init_result != RETURN_OK)
    {
        while(1);
    }

    /* Init CBCMAC encryption context*/
    memset((void*)&temp_data, 0x00, sizeof(temp_data));
    memset((void*)&cur_cbc_mac, 0x00, sizeof(cur_cbc_mac));
    memset((void*)&cur_aes_key, 0x00, sizeof(cur_aes_key));
    aes256_init_ecb(&temp_aes_context, cur_aes_key);

    // Compute CBCMAC for between the start of the graphics zone until the max addressing space (65536) - the size of the CBCMAC
    for (uint16_t i = GRAPHIC_ZONE_START; i < (UINT16_MAX - sizeof(cur_cbc_mac) + 1); i += sizeof(cur_cbc_mac))
    {
        // Read data from external flash
        flashRawRead(temp_data, i, sizeof(temp_data));

        // Continue computation of CBCMAC
        aesXorVectors(cur_cbc_mac, temp_data, sizeof(temp_data));
        aes256_encrypt_ecb(&temp_aes_context, cur_cbc_mac);
    }

    // Read CBCMAC in memory, compare the two values
    flashRawRead(temp_data, (UINT16_MAX - sizeof(cur_cbc_mac) + 1), sizeof(temp_data));
    if (memcmp(temp_data, cur_cbc_mac, sizeof(temp_data)) == 0)
    {
        // Match, start the main program
        start_bootloader();
    }
    else
    {
        // Fail, erase everything!
    }

    while(1);
}