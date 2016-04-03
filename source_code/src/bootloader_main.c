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
#include <avr/boot.h>
#include <stdlib.h>
#include <string.h>
#include "eeprom_addresses.h"
#include "aes256_ctr.h"
#include "node_mgmt.h"
#include "flash_mem.h"
#include "defines.h"
#include "aes.h"
#include "spi.h"
#define start_firmware()    asm volatile ( "jmp 0x0000" )
#define MAX_FIMRWARE_SIZE   28672
#if SPM_PAGESIZE == 128
    #define SPM_PAGE_SIZE_BYTES_BM  0x007F
#endif


/*! \fn     boot_program_page(uint32_t page, uint8_t* buf)
 *  \brief  Flash a page of data to the MCU flash
 *  \param  page    Page address in bytes
 *  \param  buf     Pointer to a buffer SPM_PAGESIZE long
 */
void boot_program_page(uint16_t page, uint8_t* buf)
{
    uint16_t i;

    // Erase page, wait for memories to be ready
    eeprom_busy_wait();
    boot_page_erase(page);
    boot_spm_busy_wait();

    // Fill the bootloader temporary page buffer
    for (i=0; i < SPM_PAGESIZE; i+=2)
    {
        // Set up little-endian word.
        uint16_t w = *buf++;
        w += (*buf++) << 8;    
        boot_page_fill(page + i, w);
    }

    // Store buffer in flash page, wait until the memory is written, re-enable RWW section
    boot_page_write(page);
    boot_spm_busy_wait();
    boot_rww_enable();
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
    
    /* See if we actually wanted to start the bootloader */
    //if (current_bootkey_val != BOOTLOADER_BOOTKEY)
    //{
    //    while(1);
    //}
    (void)current_bootkey_val;

    /* By default, brick the device so it's an all or nothing update procedure */
    eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, BRICKED_BOOTKEY);

    /* TO REMOVE */
    //memset((void*)cur_aes_key, 0x00, sizeof(cur_aes_key));
    //eeprom_write_block((void*)cur_aes_key, (void*)EEP_BOOT_PWD, sizeof(cur_aes_key));

    /* Initialize SPI controller, check flash presence */
    UHWCON = 0x01;
    spiUsartBegin();
    for (uint16_t i = 0; i < 20000; i++) asm volatile ("NOP");
    flash_init_result = initFlash();
    if (flash_init_result != RETURN_OK)
    {
        while(1);
    }    

    /* Init CBCMAC encryption context*/
    eeprom_read_block((void*)cur_aes_key, (void*)EEP_BOOT_PWD, sizeof(cur_aes_key));
    memset((void*)cur_cbc_mac, 0x00, sizeof(cur_cbc_mac));
    memset((void*)temp_data, 0x00, sizeof(temp_data));
    aes256_init_ecb(&temp_aes_context, cur_aes_key);

    // Compute CBCMAC for between the start of the graphics zone until the max addressing space (65536) - the size of the CBCMAC
    uint8_t firmware_data[SPM_PAGESIZE];
    for (uint16_t i = GRAPHIC_ZONE_START; i < (UINT16_MAX - sizeof(cur_cbc_mac) + 1); i += sizeof(cur_cbc_mac))
    {
        // Read data from external flash
        flashRawRead(temp_data, i, sizeof(temp_data));

        // If we got to the part containing to firmware
        if ((i >= (UINT16_MAX - MAX_FIMRWARE_SIZE - sizeof(cur_cbc_mac) - sizeof(cur_aes_key) + 1)) && (i < (UINT16_MAX - sizeof(cur_cbc_mac) - sizeof(cur_aes_key) + 1)))
        {
            // Append firmware data to current buffer
            uint16_t firmware_data_address = i - ((UINT16_MAX - MAX_FIMRWARE_SIZE - sizeof(cur_cbc_mac) - sizeof(cur_aes_key) + 1));
            memcpy(firmware_data + (firmware_data_address & SPM_PAGE_SIZE_BYTES_BM), temp_data, sizeof(temp_data));

            // If we have a full page in buffer, flash it
            firmware_data_address += sizeof(cur_cbc_mac);
            if ((firmware_data_address & SPM_PAGE_SIZE_BYTES_BM) == 0x0000)
            {
                boot_program_page(firmware_data_address - SPM_PAGESIZE, firmware_data);
            }
        }

        // Continue computation of CBCMAC
        aesXorVectors(cur_cbc_mac, temp_data, sizeof(temp_data));
        aes256_encrypt_ecb(&temp_aes_context, cur_cbc_mac);
    }

    // Read CBCMAC in memory, compare the two values, also read new encrypted AES key
    flashRawRead(temp_data, (UINT16_MAX - sizeof(cur_cbc_mac) + 1), sizeof(temp_data));
    flashRawRead(cur_aes_key, (UINT16_MAX - sizeof(cur_cbc_mac) - sizeof(cur_aes_key) + 1), sizeof(cur_aes_key));
    if (memcmp(temp_data, cur_cbc_mac, sizeof(temp_data)) == 0)
    {
        // Fetch the encrypted new aes key from flash, decrypt it, store it
        aes256_decrypt_ecb(&temp_aes_context, cur_aes_key);
        aes256_decrypt_ecb(&temp_aes_context, cur_aes_key+16);
        eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, CORRECT_BOOTKEY);
        eeprom_write_block((void*)cur_aes_key, (void*)EEP_BOOT_PWD, sizeof(cur_aes_key));
        start_firmware();
    }
    else
    {
        // Fail, erase everything! >> maybe just write a while(1) in the future?
        for (uint16_t i = 0; i < MAX_FIMRWARE_SIZE; i+=SPM_PAGESIZE)
        {
            boot_page_erase(i);
            boot_spm_busy_wait();
        }
        while(1);
    }
}