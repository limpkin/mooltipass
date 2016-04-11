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
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "eeprom_addresses.h"
#include "aes256_ctr.h"
#include "node_mgmt.h"
#include "flash_mem.h"
#include "defines.h"
#include "aes.h"
#include "spi.h"
#define start_firmware()    asm volatile ("jmp 0x0000")
#define MAX_FIMRWARE_SIZE   28672
#define SPM_PAGE_SIZE_BYTES_BM (SPM_PAGESIZE - 1)


/*! \fn     boot_program_page(uint16_t page, uint8_t* buf)
 *  \brief  Flash a page of data to the MCU flash
 *  \param  page    Page address in bytes
 *  \param  buf     Pointer to a buffer SPM_PAGESIZE long
 *  \note   If the function needs to be called from the firmware:
 *  \note   typedef void (*boot_program_page_t)(uint16_t page, uint8_t* buf);
 *  \note   const boot_program_page_t boot_program_page = (boot_program_page_t)0x3FC0;
 */
static void boot_program_page(uint16_t page, uint8_t* buf)  __attribute__ ((section (".spmfunc"))) __attribute__((noinline));
static void boot_program_page(uint16_t page, uint8_t* buf)
{
    uint16_t i;

    // Erase page, wait for memories to be ready
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
    uint8_t new_aes_key[AES_KEY_LENGTH/8];                                          // New AES encryption key
    uint8_t cur_aes_key[AES_KEY_LENGTH/8];                                          // AES encryption key
    uint8_t firmware_data[SPM_PAGESIZE];                                            // One page of firmware data
    aes256_context temp_aes_context;                                                // AES context
    uint8_t cur_cbc_mac[16];                                                        // Current CBCMAC val
    uint8_t temp_data[16];                                                          // Temporary 16 bytes vector
    RET_TYPE flash_init_result;                                                     // Flash initialization result

    // Check fuses: 2k words, SPIEN, BOD 4.3V, programming & ver disabled >> http://www.engbedded.com/fusecalc/
    //if ((boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS) != 0xFF) || (boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS) != 0xD8) || (boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS) != 0xF8) || (boot_lock_fuse_bits_get(GET_LOCK_BITS) != 0xFC))
    //{
    //    while(1);
    //}
    //
    //// Check that the bootloader was actually called from the firmware!
    //if (current_bootkey_val != BOOTLOADER_BOOTKEY)
    //{
    //    while(1);
    //}
    (void)current_bootkey_val;

    // By default, brick the device so it's an all or nothing update procedure
    eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, BRICKED_BOOTKEY);

    // Enable USB 3.3V LDO, Initialize SPI controller, Check flash presence
    UHWCON = 0x01;
    spiUsartBegin();
    _delay_ms(10);
    flash_init_result = initFlash();
    if (flash_init_result != RETURN_OK)
    {
        while(1);
    }

    // Init CBCMAC encryption context
    eeprom_read_block((void*)cur_aes_key, (void*)EEP_BOOT_PWD, sizeof(cur_aes_key));
    memset((void*)cur_cbc_mac, 0x00, sizeof(cur_cbc_mac));
    memset((void*)temp_data, 0x00, sizeof(temp_data));
    aes256_init_ecb(&temp_aes_context, cur_aes_key);

    // Compute CBCMAC for between the start of the graphics zone until the max addressing space (65536) - the size of the CBCMAC
    for (uint16_t i = GRAPHIC_ZONE_START; i < (UINT16_MAX - sizeof(cur_cbc_mac) + 1); i += sizeof(cur_cbc_mac))
    {
        // Read data from external flash
        flashRawRead(temp_data, i, sizeof(temp_data));

        // If we got to the part containing to firmware
        uint16_t firmware_start_address = UINT16_MAX - MAX_FIMRWARE_SIZE - sizeof(cur_cbc_mac) - sizeof(cur_aes_key) + 1;
        uint16_t firmware_end_address = UINT16_MAX - sizeof(cur_cbc_mac) - sizeof(cur_aes_key) + 1;
        if ((i >= firmware_start_address) && (i < firmware_end_address))
        {
            // Append firmware data to current buffer
            uint16_t firmware_data_address = i - firmware_start_address;
            memcpy(firmware_data + (firmware_data_address & SPM_PAGE_SIZE_BYTES_BM), temp_data, sizeof(temp_data));

            // If we have a full page in buffer, flash it
            firmware_data_address += sizeof(cur_cbc_mac);
            if ((firmware_data_address & SPM_PAGE_SIZE_BYTES_BM) == 0x0000)
            {
                boot_program_page(firmware_data_address - SPM_PAGESIZE, firmware_data);
            }
        }

        // If we got to the part containing the encrypted new aes key (end of the for())
        if (i >= (firmware_end_address - sizeof(cur_aes_key)))
        {
            memcpy(new_aes_key, temp_data, sizeof(new_aes_key));
        }

        // Continue computation of CBCMAC
        aesXorVectors(cur_cbc_mac, temp_data, sizeof(cur_cbc_mac));
        aes256_encrypt_ecb(&temp_aes_context, cur_cbc_mac);
    }

    // Read CBCMAC in memory and compare it with the computed value
    flashRawRead(temp_data, (UINT16_MAX - sizeof(cur_cbc_mac) + 1), sizeof(temp_data));
    if (memcmp(temp_data, cur_cbc_mac, sizeof(temp_data)) == 0)
    {
        // Fetch the encrypted new aes key from flash, decrypt it, store it
        aes256_decrypt_ecb(&temp_aes_context, new_aes_key);
        aes256_decrypt_ecb(&temp_aes_context, new_aes_key+16);
        eeprom_write_block((void*)new_aes_key, (void*)EEP_BOOT_PWD, sizeof(new_aes_key));
        eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, CORRECT_BOOTKEY);
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
