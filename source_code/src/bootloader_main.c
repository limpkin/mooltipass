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
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/boot.h>
#include <stdlib.h>
#include <string.h>
#include "logic_fwflash_storage.h"
#include "eeprom_addresses.h"
#include "watchdog_driver.h"
#include "aes256_ctr.h"
#include "node_mgmt.h"
#include "flash_mem.h"
#include "defines.h"
#include "aes.h"
#include "spi.h"
#define start_firmware()        asm volatile ("jmp 0x0000")
#define MAX_FIRMWARE_SIZE       28672
#define SPM_PAGE_SIZE_BYTES_BM  (SPM_PAGESIZE - 1)


/*! \fn     start(void)
*   \brief  Function replacing the reset boot vector
*   \note   This solution is compiled with the -nostartfiles flag, so no vectors or init routines are included in the final hex
*           We therefore need to initialize the stack, launch the main()
*/
void start(void) __attribute__((naked,used,section(".vectors")));
void start(void)
{
    SPH = (RAMEND) >> 8;                        // Initialize stack pointer
    SPL = RAMEND & 0xFF;                        // Initialize stack pointer
    asm volatile ( "clr __zero_reg__" );        // Set R1 to 0
    asm("rjmp main");                           // Jump to Main
}

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
    uint16_t i, w;

    // Check we are not overwriting this particular routine
    if ((page >= (FLASHEND - SPM_PAGESIZE + 1)) || ((page & SPM_PAGE_SIZE_BYTES_BM) != 0))
    {
        return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        // Erase page, wait for memories to be ready
        boot_page_erase(page);
        boot_spm_busy_wait();

        // Fill the bootloader temporary page buffer
        for (i = 0; i < SPM_PAGESIZE; i+=2)
        {
            // Set up little-endian word.
            w = (*buf++) & 0x00FF;
            w |= (((uint16_t)(*buf++)) << 8) & 0xFF00;
            boot_page_fill(page + i, w);
        }

        // Store buffer in flash page, wait until the memory is written, re-enable RWW section
        boot_page_write(page);
        boot_spm_busy_wait();
        boot_rww_enable();
    }
}

/*! \fn     sideChannelSafeMemCmp(uint8_t* dataA, uint8_t* dataB, uint8_t size)
*   \brief  A side channel attack safe implementation of memcmp
*   \param  dataA   First array
*   \param  dataB   Second array
*   \param  size    Arrays length
*   \return 0 for match
*/
uint8_t sideChannelSafeMemCmp(uint8_t* dataA, uint8_t* dataB, uint8_t size)
{
    volatile uint8_t return_value = 0x00;

    for (uint8_t i = 0; i < size; i++)
    {
        return_value |= dataA[i] ^ dataB[i];
    }

    return return_value;
}

/*! \fn     main(void)
*   \brief  Main function
*   \note   For our security chain to be valid, EEP_BOOT_PWD_SET in eeprom needs to be set to BOOTLOADER_PWDOK_KEY
*/
int main(void)
{
    /* Fetch bootkey in eeprom */
    uint16_t current_bootkey_val = eeprom_read_word((uint16_t*)EEP_BOOTKEY_ADDR);                                       // Bootkey in EEPROM
    uint8_t new_aes_key[AES_KEY_LENGTH/8];                                                                              // New AES encryption key
    uint8_t cur_aes_key[AES_KEY_LENGTH/8];                                                                              // AES encryption key
    uint8_t firmware_data[SPM_PAGESIZE];                                                                                // One page of firmware data
    aes256_context temp_aes_context;                                                                                    // AES context
    RET_TYPE flash_init_result;                                                                                         // Flash initialization result
    uint8_t cur_cbc_mac[16];                                                                                            // Current CBCMAC val
    uint8_t temp_data[16];                                                                                              // Temporary 16 bytes array
    uint8_t aes_key_update_bool;                                                                                        // Boolean specifying that we want to update the aes key
    uint8_t old_version_number[4];                                                                                      // Old firmware version identifier
    uint8_t new_version_number[4];                                                                                      // New firmware version identifier
    uint16_t firmware_start_address = UINT16_MAX - MAX_FIRMWARE_SIZE - sizeof(cur_cbc_mac) - sizeof(cur_aes_key) + 1;   // Start address of firmware in external memory
    uint16_t firmware_end_address = UINT16_MAX - sizeof(cur_cbc_mac) - sizeof(cur_aes_key) + 1;                         // End address of firmware in external memory


    /* The firmware uses the watchdog timer to get here */
    cli();
    wdt_reset();
    wdt_clear_flag();
    wdt_change_enable();
    wdt_stop();

    /* Check fuses: 2k words, SPIEN, BOD 4.3V, BOOTRST programming & ver disabled >> http://www.engbedded.com/fusecalc/ */
    if ((boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS) != 0xFF) || (boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS) != 0xD8) || (boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS) != 0xF8) || (boot_lock_fuse_bits_get(GET_LOCK_BITS) != 0xFC))
    {
        while(1);
    }

    /* If security isn't set in place yet, no point in launching the bootloader */
    if (eeprom_read_byte((uint8_t*)EEP_BOOT_PWD_SET) != BOOTLOADER_PWDOK_KEY)
    {
        start_firmware();
    }

    /* Check if the device is booting normally, if the bootloader was called, or unknown state */
    if (current_bootkey_val == CORRECT_BOOTKEY)
    {
        /* Security system set, correct bootkey for firmware */
        start_firmware();
    }
    else if (current_bootkey_val != BOOTLOADER_BOOTKEY)
    {
        /* Security system set, bootkey isn't the bootloader one nor the main fw one... */
        while(1);
    }

    /* Init IOs */
    UHWCON = 0x01;                                              // Enable USB 3.3V LDO
    initFlashIOs();                                             // Init EXT Flash IOs
    spiUsartBegin();                                            // Init SPI Controller    
    DDR_ACC_SS |= (1 << PORTID_ACC_SS);                         // Setup PORT for the Accelerometer SS
    PORT_ACC_SS |= (1 << PORTID_ACC_SS);                        // Setup PORT for the Accelerometer SS    
    DDR_OLED_SS |= (1 << PORTID_OLED_SS);                       // Setup PORT for the OLED SS
    PORT_OLED_SS |= (1 << PORTID_OLED_SS);                      // Setup PORT for the OLED SS
    for (uint16_t i = 0; i < 20000; i++) asm volatile ("NOP");  // Wait for 3.3V to come up

    /* Disable I2C block of the Accelerometer */
    PORT_ACC_SS &= ~(1 << PORTID_ACC_SS);
    spiUsartTransfer(0x23);
    spiUsartTransfer(0x02);
    PORT_ACC_SS |= (1 << PORTID_ACC_SS);

    /* Check Flash */
    flash_init_result = checkFlashID();
    if (flash_init_result != RETURN_OK)
    {
        while(1);
    }

    /* By default, brick the device so it's an all or nothing update procedure */
    eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, BRICKED_BOOTKEY);

    /* Update bundle composition: bundle | padding | firmware version | new aes key bool | firmware | padding | new aes key encoded | cbcmac */
    for (uint8_t pass_number = 0; pass_number < 2; pass_number++)
    {
        /* Init CBCMAC encryption context and read current firmware version ID */
        eeprom_read_block((void*)old_version_number, (void*)EEP_USER_DATA_START_ADDR, sizeof(old_version_number));      // Read old version number from eeprom (put there by firmware before jumping here)
        eeprom_read_block((void*)cur_aes_key, (void*)EEP_BOOT_PWD, sizeof(cur_aes_key));                                // Read current aes key from eeprom
        memset((void*)cur_cbc_mac, 0x00, sizeof(cur_cbc_mac));                                                          // Set IV for CBCMAC to 0
        aes256_init_ecb(&temp_aes_context, cur_aes_key);                                                                // Init AES context
        aes_key_update_bool = FALSE;                                                                                    // Set to False

        // Compute CBCMAC for between the start of the graphics zone until the max addressing space (65536) - the size of the CBCMAC
        for (uint16_t i = GRAPHIC_ZONE_START; i < (UINT16_MAX - sizeof(cur_cbc_mac) + 1); i += sizeof(cur_cbc_mac))
        {
            // Read data from external flash
            flashRawRead(temp_data, i, sizeof(temp_data));

            // 16 bytes before the firmware
            if (i == (firmware_start_address - 16))
            {
                // 16 bytes before the firmware: padding | version number (4 bytes) | aes key update bool (1 byte)
                memcpy(new_version_number, temp_data + (16 - sizeof(aes_key_update_bool) - sizeof(new_version_number)), sizeof(new_version_number));
                aes_key_update_bool = temp_data[16-sizeof(aes_key_update_bool)];
            }

            // If we got to the part containing to firmware
            if ((i >= firmware_start_address) && (i < firmware_end_address))
            {
                // Append firmware data to current buffer
                uint16_t firmware_data_address = i - firmware_start_address;
                memcpy(firmware_data + (firmware_data_address & SPM_PAGE_SIZE_BYTES_BM), temp_data, sizeof(temp_data));

                // If we have a full page in buffer, flash it
                firmware_data_address += sizeof(cur_cbc_mac);
                if (((firmware_data_address & SPM_PAGE_SIZE_BYTES_BM) == 0x0000) && (pass_number == 1))
                {
                    boot_program_page(firmware_data_address - SPM_PAGESIZE, firmware_data);
                }
            }

            // If we got to the part containing the encrypted new aes key (end of the for())
            if (i >= firmware_end_address)
            {
                memcpy(new_aes_key + i - firmware_end_address, temp_data, sizeof(temp_data));
            }

            // Continue computation of CBCMAC
            aesXorVectors(cur_cbc_mac, temp_data, sizeof(temp_data));
            aes256_encrypt_ecb(&temp_aes_context, cur_cbc_mac);
        }

        // Read & compare CBCMAC, check that the version number is above or egal to our current one to set the update condition boolean
        uint8_t update_condition = TRUE;
        flashRawRead(temp_data, (UINT16_MAX - sizeof(cur_cbc_mac) + 1), sizeof(cur_cbc_mac));
        if ((sideChannelSafeMemCmp(temp_data, cur_cbc_mac, sizeof(cur_cbc_mac)) != 0) || (memcmp((void*)old_version_number, (void*)new_version_number, sizeof(new_version_number)) > 0))
        {
            update_condition = FALSE;
        }

        if (pass_number == 0)
        {
            if (update_condition == FALSE)
            {
                /* Update condition error */
                sectorZeroErase(FLASH_SECTOR_ZERO_B_CODE);                                                                      // Erase graphics bundle
                eeprom_write_byte((uint8_t*)EEP_USER_DATA_START_ADDR + USER_PARAM_INIT_KEY_PARAM, USER_PARAM_CORRECT_INIT_KEY); // Reset parameters we overwrote by passing the version ID
                eeprom_write_byte((uint8_t*)EEP_USER_DATA_START_ADDR + KEYBOARD_LAYOUT_PARAM, ID_KEYB_EN_US_LUT);               // Reset parameters we overwrote by passing the version ID
                eeprom_write_byte((uint8_t*)EEP_USER_DATA_START_ADDR + USER_INTER_TIMEOUT_PARAM, 15);                           // Reset parameters we overwrote by passing the version ID
                eeprom_write_byte((uint8_t*)EEP_USER_DATA_START_ADDR + LOCK_TIMEOUT_ENABLE_PARAM, 60);                          // Reset parameters we overwrote by passing the version ID
                eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, CORRECT_BOOTKEY);                                                // Allow starting of the main firmware
                start_firmware();                                                                                               // Start firmware
            }
            else
            {
                // Otherwise, next pass!
            }
        }
        else
        {
            // Second pass, compare CBCMAC and then update AES keys
            if (update_condition == TRUE)
            {
                // Fetch the encrypted new aes key from flash, decrypt it, store it
                if (aes_key_update_bool != FALSE)
                {
                    aes256_decrypt_ecb(&temp_aes_context, new_aes_key);
                    aes256_decrypt_ecb(&temp_aes_context, new_aes_key+16);
                    eeprom_write_block((void*)new_aes_key, (void*)EEP_BOOT_PWD, sizeof(new_aes_key));
                }
                eeprom_write_word((uint16_t*)EEP_BOOTKEY_ADDR, CORRECT_BOOTKEY);
                start_firmware();
            }
            else
            {
                // Fail, stay bricked!
                while(1);
            }
        }
    }
}
