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
/* \file 	defines.h
 * \brief	Project definitions
 *  Created: 11/01/2014 11:54:26
 *  Author: Mathieu Stephan
 */


#ifndef DEFINES_H_
#define DEFINES_H_

#include <stdint.h>

/** DEBUG PRINTS **/
// Used for the smart card debug prints
#define DEBUG_SMC_SCREEN_PRINT

/** HARDWARE VERSION **/
#define	HARDWARE_V1
//#define HARDWARE_OLIVIER_V1

/** SMARTCARD FUSE VERSION **/
#define SMARTCARD_FUSE_V1

/** MACROS **/
#define CPU_PRESCALE(n)		    (CLKPR = 0x80, CLKPR = (n))

/** DEFINES FIRMWARE **/
#define FALSE				    0
#define TRUE				    (!FALSE)
#define AES_KEY_LENGTH          256

/** ASM "ENUMS" **/
#define SPI_NATIVE			    1
#define SPI_USART2              2

/** C ENUMS **/
enum mooltipass_detect_return_t	{RETURN_MOOLTIPASS_INVALID, RETURN_MOOLTIPASS_PB, RETURN_MOOLTIPASS_BLOCKED, RETURN_MOOLTIPASS_BLANK, RETURN_MOOLTIPASS_USER, RETURN_MOOLTIPASS_4_TRIES_LEFT,  RETURN_MOOLTIPASS_3_TRIES_LEFT,  RETURN_MOOLTIPASS_2_TRIES_LEFT,  RETURN_MOOLTIPASS_1_TRIES_LEFT, RETURN_MOOLTIPASS_0_TRIES_LEFT};
enum card_detect_return_t		{RETURN_CARD_NDET, RETURN_CARD_TEST_PB, RETURN_CARD_4_TRIES_LEFT,  RETURN_CARD_3_TRIES_LEFT,  RETURN_CARD_2_TRIES_LEFT,  RETURN_CARD_1_TRIES_LEFT, RETURN_CARD_0_TRIES_LEFT};
enum pin_check_return_t			{RETURN_PIN_OK, RETURN_PIN_NOK_3, RETURN_PIN_NOK_2, RETURN_PIN_NOK_1, RETURN_PIN_NOK_0};
enum detect_return_t			{RETURN_REL, RETURN_DET, RETURN_JDETECT, RETURN_JRELEASED};
enum return_type                {RETURN_NOK = 0, RETURN_OK, RETURN_NOT_INIT};
enum flash_ret_t                {RETURN_INVALID_PARAM, RETURN_WRITE_ERR, RETURN_READ_ERR, RETURN_NO_MATCH};
    
/** TYPEDEFS **/
typedef uint8_t RET_TYPE;

/** DEFINES FLASH **/
/** DEFINES NODES **/
#define NODE_SIZE_PARENT 66
#define NODE_SIZE_CHILD 132
#define NODE_SIZE_DATA 132

#define NODE_TYPE_PARENT 0
#define NODE_TYPE_CHILD 1
#define NODE_TYPE_CHILD_DATA 2
#define NODE_TYPE_DATA 3

/** DEFINES FLASH **/

// Chip selection
#define FLASH_CHIP_1M 1    // Used to identify a 1M Flash Chip (AT45DB011D)
#define FLASH_CHIP_2M 2    // Used to identify a 2M Flash Chip (AT45DB021E)
#define FLASH_CHIP_4M 4    // Used to identify a 4M Flash Chip (AT45DB041E)
#define FLASH_CHIP_8M 8    // Used to identify a 8M Flash Chip (AT45DB081E)
#define FLASH_CHIP_16M 16  // Used to identify a 16M Flash Chip (AT45DB161E)
#define FLASH_CHIP_32M 32  // Used to identify a 32M Flash Chip (AT45DB321E)
// mooltipass cannot use 64M flash (16-bit address space prevents it)

// Set FLASH_CHIP to value of FLASH_CHIP_<XX>M -> Possible compile time option / flag?
#define FLASH_CHIP FLASH_CHIP_1M       // Used to identify the flash chip in use

#if FLASH_CHIP==FLASH_CHIP_1M
    #define MAN_FAM_DEN_VAL 0x22       // Used for Chip Identity (see datasheet)
    #define PAGE_COUNT 512             // Number of pages in the chip
    #define BYTES_PER_PAGE 264         // Bytes per page of the chip
    #define MAP_BYTES 192              // Bytes required to make 'node' usage (map) -> (((PAGE_COUNT - PAGE_PER_SECTOR) * BYTES_PER_PAGE) / NODE_SIZE_PARENT) / 8bits
    #define MAP_PAGES 1                // Pages required to hold 'node' usage (map) -> CEILING(MAP_BYTES / BYTES_PER_PAGE)
    #define NODE_PARENT_PER_PAGE 4     // Number of parent nodes per page -> BYTES_PER_PAGE / NODE_SIZE_PARENT
    #define BLOCK_COUNT 64             // Number of blocks in the chip
    #define SECTOR_START 1             // The first whole sector number in the chip
    #define SECTOR_END 3               // The last whole sector number in the chip
    #define PAGE_PER_SECTOR 128        // Number of pages per sector in the chip
    
    #define SECTOR_ERASE_0_SHT_AMT 12  // The shift amount used for a sector zero part erase (see comments below)
    #define SECTOR_ERASE_N_SHT_AMT 16  // The shift amount used for a sector erase (see comments below)
    #define BLOCK_ERASE_SHT_AMT 12     // The shift amount used for a block erase (see comments below)
    #define PAGE_ERASE_SHT_AMT 9       // The shift amount used for a page erase (see comments below)
    #define WRITE_SHT_AMT 9            // The shift amount used for a write operation (see comments below)
    #define READ_OFFSET_SHT_AMT 9      // The shift amount used for a read operation (see comments below)
    
    // sector erase -> 264size -> OP: 0x7C -> 0a/0b -> 3 address bytes -> 5 D/C, 7 Page address bits (PA9-PA3), 12 D/C -> PA3=0 -> 0a, PA3=1 -> 0b
    // sector erase -> 264size -> OP: 0x7C -> 1/3 0> 3 address bytes -> 6 D/C, 2 Page address bits (PA8-PA7), 16 D/C
    // sector 0a -> 8 pages, sector 0b -> 120 pages, sector 1/3 -> 128 page
    // block erase -> 264size -> OP: 0x50 -> 3 address bytes -> 6 D/C, 6 Block Num, 12 D/C -> 64 Blocks
    // page erase -> 264size -> OP: 0x81 -> 3 address bytes -> 6 D/C, 9 Page Num, 9 D/C -> 512 Pages
    
    // Write_p1 -> 264size -> MMP to Buffer T -> OP: 0x53 -> 3 address bytes -> 6 D/C, 9 Page Address, 9 D/C
    // write_p2 -> 264size -> MMP PROG T Buffer -> OP: 0x82 -> 3 address bytes -> 6 D/C, 9 Page Address, 9 Buffer Address
    
    // Read -> 528size -> Low Freq Read -> 0P: 0x03 -> 3 address bytes -> 9 Page Address, 9 Offset, 6 D/C ?
#elif FLASH_CHIP==FLASH_CHIP_2M 
    #define MAN_FAM_DEN_VAL 0x23       // Used for Chip Identity (see datasheet)
    #define PAGE_COUNT 1024            // Number of pages in the chip
    #define BYTES_PER_PAGE 264         // Bytes per page of the chip
    #define MAP_BYTES 448              // Bytes required to make 'node' usage (map) -> (((PAGE_COUNT - PAGE_PER_SECTOR) * BYTES_PER_PAGE) / NODE_SIZE_PARENT) / 8bits
    #define MAP_PAGES 2                // Pages required to hold 'node' usage (map) -> CEILING(MAP_BYTES / BYTES_PER_PAGE)
    #define NODE_PARENT_PER_PAGE 4     // Number of parent nodes per page -> BYTES_PER_PAGE / NODE_SIZE_PARENT
    #define BLOCK_COUNT 128            // Number of blocks in the chip
    #define SECTOR_START 1             // The first whole sector number in the chip
    #define SECTOR_END 7               // The last whole sector number in the chip
    #define PAGE_PER_SECTOR 128        // Number of pages per sector in the chip
    
    #define SECTOR_ERASE_0_SHT_AMT 12  // The shift amount used for a sector zero part erase (see comments below)
    #define SECTOR_ERASE_N_SHT_AMT 16  // The shift amount used for a sector erase (see comments below)
    #define BLOCK_ERASE_SHT_AMT 12     // The shift amount used for a block erase (see comments below)
    #define PAGE_ERASE_SHT_AMT 9       // The shift amount used for a page erase (see comments below)
    #define WRITE_SHT_AMT 9            // The shift amount used for a write operation (see comments below)
    #define READ_OFFSET_SHT_AMT 9      // The shift amount used for a read operation (see comments below)
    
    // sector erase -> 264size -> OP: 0x7C -> 0a/0b -> 3 address bytes -> 5 D/C, 7 Sector Address (PA9-PA3), 12 D/C -> PA3=0 -> 0a, PA3=1 -> 0b
    // sector erase -> 264size -> OP: 0x7C -> 1/7 -> 3 address bytes -> 5 D/C, 3 Sector Address Bits, 16 D/C
    // sector 0a -> 8 pages, sector 0b -> 120 pages, sector 1/7 -> 128 pages
    // block erase -> 264size -> OP: 0x50 -> 3 address bytes -> 5 D/C, 7 Block Num, 12 D/C -> 128 Blocks
    // page erase -> 264size -> OP: 0x81 -> 3 address bytes -> 5 D/C, 10 Page Num, 9 D/C -> 1024 pages
    
    // Write_p1 -> 264size -> MMP to Buffer T -> OP: 0x53 -> 3 address bytes -> 5 D/C, 10 Page Address, 9 D/C
    // write_p2 -> 264size -> MMP PROG T Buffer -> OP: 0x82 -> 3 address bytes -> 5 D/C, 10 Page Address, 9 Buffer Address
    
    // Read -> 528size -> Low Freq Read -> 0P: 0x03 -> 3 address bytes -> 10 Page Address, 9 Offset, 5 D/C ?
#elif FLASH_CHIP==FLASH_CHIP_4M
    #define MAN_FAM_DEN_VAL 0x24       // Used for Chip Identity (see datasheet)
    #define PAGE_COUNT 2048            // Number of pages in the chip
    #define BYTES_PER_PAGE 264         // Bytes per page of the chip
    #define MAP_BYTES 896              // Bytes required to make 'node' usage (map) -> (((PAGE_COUNT - PAGE_PER_SECTOR) * BYTES_PER_PAGE) / NODE_SIZE_PARENT) / 8bits
    #define MAP_PAGES 4                // Pages required to hold 'node' usage (map) -> CEILING(MAP_BYTES / BYTES_PER_PAGE)
    #define NODE_PARENT_PER_PAGE 4     // Number of parent nodes per page -> BYTES_PER_PAGE / NODE_SIZE_PARENT
    #define BLOCK_COUNT 256            // Number of blocks in the chip
    #define SECTOR_START 1             // The first whole sector number in the chip
    #define SECTOR_END 7               // The last whole sector number in the chip
    #define PAGE_PER_SECTOR 256        // Number of pages per sector in the chip
    
    #define SECTOR_ERASE_0_SHT_AMT 12  // The shift amount used for a sector zero part erase (see comments below)
    #define SECTOR_ERASE_N_SHT_AMT 17  // The shift amount used for a sector erase (see comments below)
    #define BLOCK_ERASE_SHT_AMT 12     // The shift amount used for a block erase (see comments below)
    #define PAGE_ERASE_SHT_AMT 9       // The shift amount used for a page erase (see comments below)
    #define WRITE_SHT_AMT 9            // The shift amount used for a write operation (see comments below)
    #define READ_OFFSET_SHT_AMT 9      // The shift amount used for a read operation (see comments below)
    
    // sector erase -> 264size -> OP: 0x7C -> 0a/0b -> 3 address bytes -> 4 D/C, 8 Sector Address (PA10-PA3), 12 D/C -> PA3=0 -> 0a, PA3=1 -> 0b
    // sector erase -> 264size -> OP: 0x7C -> 1/7 -> 3 address bytes -> 4 D/C, 3 Sector Address Bits, 17 D/C
    // sector 0a -> 8 pages, sector 0b -> 248 pages, sector 1/7 -> 256 pages
    // block erase -> 264size -> OP: 0x50 -> 3 address bytes -> 4 D/C, 8 Block Num, 12 D/C -> 256 Blocks
    // page erase -> 264size -> OP: 0x81 -> 3 address bytes -> 4 D/C, 11 Page Num, 9 D/C -> 2048 Pages
    
    // Write_p1 -> 264size -> MMP to Buffer T -> OP: 0x53 -> 3 address bytes -> 4 D/C, 11 Page Address, 9 D/C
    // write_p2 -> 264size -> MMP PROG T Buffer -> OP: 0x82 -> 3 address bytes -> 4 D/C, 11 Page Address, 9 Buffer Address
    
    // Read -> 528size -> Low Freq Read -> 0P: 0x03 -> 3 address bytes -> 11 Page Address, 9 Offset, 4 D/C ?
#elif FLASH_CHIP==FLASH_CHIP_8M
    #define MAN_FAM_DEN_VAL 0x25       // Used for Chip Identity (see datasheet)
    #define PAGE_COUNT 4096            // Number of pages in the chip
    #define BYTES_PER_PAGE 264         // Bytes per page of the chip
    #define MAP_BYTES 1920             // Bytes required to make 'node' usage (map) -> (((PAGE_COUNT - PAGE_PER_SECTOR) * BYTES_PER_PAGE) / NODE_SIZE_PARENT) / 8bits
    #define MAP_PAGES 8                // Pages required to hold 'node' usage (map) -> CEILING(MAP_BYTES / BYTES_PER_PAGE)
    #define NODE_PARENT_PER_PAGE 4     // Number of parent nodes per page -> BYTES_PER_PAGE / NODE_SIZE_PARENT
    #define BLOCK_COUNT 512            // Number of blocks in the chip
    #define SECTOR_START 1             // The first whole sector number in the chip
    #define SECTOR_END 15               // The last whole sector number in the chip
    #define PAGE_PER_SECTOR 256        // Number of pages per sector in the chip
    
    #define SECTOR_ERASE_0_SHT_AMT 12  // The shift amount used for a sector zero part erase (see comments below)
    #define SECTOR_ERASE_N_SHT_AMT 17  // The shift amount used for a sector erase (see comments below)
    #define BLOCK_ERASE_SHT_AMT 12     // The shift amount used for a block erase (see comments below)
    #define PAGE_ERASE_SHT_AMT 9       // The shift amount used for a page erase (see comments below)
    #define WRITE_SHT_AMT 9            // The shift amount used for a write operation (see comments below)
    #define READ_OFFSET_SHT_AMT 9      // The shift amount used for a read operation (see comments below)
    
    // sector erase -> 264size -> OP: 0x7C -> 0a/0b -> 3 address bytes -> 3 D/C, 9 Sector Address (PA11-PA3), 12 D/C -> PA3=0 -> 0a, PA3=1 -> 0b
    // sector erase -> 264size -> OP: 0x7C -> 1/15 -> 3 address bytes -> 3 D/C, 4 Sector Address Bits, 17 D/C
    // sector 0a -> 8 pages, sector 0b -> 248 pages, sector 1/15 -> 256 pages
    // block erase -> 264size -> OP: 0x50 -> 3 address bytes -> 3 D/C, 9 Block Num, 12 D/C -> 512 Blocks
    // page erase -> 264size -> OP: 0x81 -> 3 address bytes -> 3 D/C, 12 Page Num, 9 D/C -> 4096 Pages
    
    // Write_p1 -> 264size -> MMP to Buffer T -> OP: 0x53 -> 3 address bytes -> 3 D/C, 12 Page Address, 9 D/C
    // write_p2 -> 264size -> MMP PROG T Buffer -> OP: 0x82 -> 3 address bytes -> 3 D/C, 12 Page Address, 9 Buffer Address
    
    // Read -> 528size -> Low Freq Read -> 0P: 0x03 -> 3 address bytes -> 12 Page Address, 9 Offset, 3 D/C ?
#elif FLASH_CHIP==FLASH_CHIP_16M
    #define MAN_FAM_DEN_VAL 0x26       // Used for Chip Identity (see datasheet)
    #define PAGE_COUNT 4096            // Number of pages in the chip
    #define BYTES_PER_PAGE 528         // Bytes per page of the chip
    #define MAP_BYTES 3840             // Bytes required to make 'node' usage (map) -> (((PAGE_COUNT - PAGE_PER_SECTOR) * BYTES_PER_PAGE) / NODE_SIZE_PARENT) / 8bits
    #define MAP_PAGES 8                // Pages required to hold 'node' usage (map) -> CEILING(MAP_BYTES / BYTES_PER_PAGE)
    #define NODE_PARENT_PER_PAGE 8     // Number of parent nodes per page -> BYTES_PER_PAGE / NODE_SIZE_PARENT
    #define BLOCK_COUNT 512            // Number of blocks in the chip
    #define SECTOR_START 1             // The first whole sector number in the chip
    #define SECTOR_END 15              // The last whole sector number in the chip
    #define PAGE_PER_SECTOR 256        // Number of pages per sector in the chip
    
    #define SECTOR_ERASE_0_SHT_AMT 13  // The shift amount used for a sector zero part erase (see comments below)
    #define SECTOR_ERASE_N_SHT_AMT 18  // The shift amount used for a sector erase (see comments below)
    #define BLOCK_ERASE_SHT_AMT 13     // The shift amount used for a block erase (see comments below)
    #define PAGE_ERASE_SHT_AMT 10      // The shift amount used for a page erase (see comments below)
    #define WRITE_SHT_AMT 10           // The shift amount used for a write operation (see comments below)
    #define READ_OFFSET_SHT_AMT 10     // The shift amount used for a read operation (see comments below)
    
    // sector erase -> 528size -> OP: 0x7C -> 0a/0b -> 3 address bytes -> 2 D/C, 9 Sector Address (PA11-PA3), 13 D/C -> PA3=0 -> 0a, PA3=1 -> 0b
    // sector erase -> 528size -> OP: 0x7C -> 1/15 -> 3 address bytes -> 2 D/C, 4 Sector Address Bits, 18 D/C
    // sector 0a -> 8 pages, sector 0b -> 248 pages, sector 1/15 -> 256 pages
    // block erase -> 528size -> OP: 0x50 -> 3 address bytes -> 2 D/C, 9 Block Num, 13 D/C -> 512 Blocks
    // page erase -> 528size -> OP: 0x81 -> 3 address bytes -> 2 D/C, 12 Page Num, 10 D/C -> 4096 Pages
    
    // Write_p1 -> 528size -> MMP to Buffer T -> OP: 0x53 -> 3 address bytes -> 2 D/C, 12 Page Address, 10 D/C
    // write_p2 -> 528size -> MMP PROG T Buffer -> OP: 0x82 -> 3 address bytes -> 2 D/C, 12 Page Address, 10 Buffer Address
    
    // Read -> 528size -> Low Freq Read -> 0P: 0x03 -> 3 address bytes -> 12 Page Address, 10 Offset, 2 D/C ?
#elif FLASH_CHIP==FLASH_CHIP_32M
    #define MAN_FAM_DEN_VAL 0x27       // Used for Chip Identity (see datasheet)
    #define PAGE_COUNT 8192            // Number of pages in the chip
    #define BYTES_PER_PAGE 528         // Bytes per page of the chip
    #define MAP_BYTES 8064             // Bytes required to make 'node' usage (map) -> (((PAGE_COUNT - PAGE_PER_SECTOR) * BYTES_PER_PAGE) / NODE_SIZE_PARENT) / 8bits
    #define MAP_PAGES 16               // Pages required to hold 'node' usage (map) -> CEILING(MAP_BYTES / BYTES_PER_PAGE)
    #define NODE_PARENT_PER_PAGE 8     // Number of parent nodes per page -> BYTES_PER_PAGE / NODE_SIZE_PARENT
    #define BLOCK_COUNT 1024           // Number of blocks in the chip
    #define SECTOR_START 1             // The first whole sector number in the chip
    #define SECTOR_END 63              // The last whole sector number in the chip
    #define PAGE_PER_SECTOR 128        // Number of pages per sector in the chip
    
    #define SECTOR_ERASE_0_SHT_AMT 13  // The shift amount used for a sector zero part erase (see comments below)
    #define SECTOR_ERASE_N_SHT_AMT 17  // The shift amount used for a sector erase (see comments below)
    #define BLOCK_ERASE_SHT_AMT 13     // The shift amount used for a block erase (see comments below)
    #define PAGE_ERASE_SHT_AMT 10      // The shift amount used for a page erase (see comments below)
    #define WRITE_SHT_AMT 10           // The shift amount used for a write operation (see comments below)
    #define READ_OFFSET_SHT_AMT 10     // The shift amount used for a read operation (see comments below)
    
    // sector erase -> 528size -> OP: 0x7C -> 0a/0b -> 3 address bytes -> 1 D/C, 10 Sector Address (PA12-PA3), 13 D/C -> PA3=0 -> 0a, PA3=1 -> 0b
    // sector erase -> 528size -> OP: 0x7C -> 1/63 -> 3 address bytes -> 1 D/C, 6 Sector Address Bits, 17 D/C
    // sector 0a -> 8 pages, sector 0b -> 120 pages, sector 1/63 -> 128 pages
    // block erase -> 528size -> OP: 0x50 -> 3 address bytes -> 1 D/C, 10 Block Num, 13 D/C -> 1024 Blocks
    // page erase -> 528size -> OP: 0x81 -> 3 address bytes -> 1 D/C, 13 Page Num, 10 D/C -> 8192 Pages
    
    // Write_p1 -> 528size -> MMP to Buffer T -> OP: 0x53 -> 3 address bytes -> 1 D/C, 13 Page Address, 10 D/C
    // write_p2 -> 528size -> MMP PROG T Buffer -> OP: 0x82 -> 3 address bytes -> 1 D/C, 13 Page Address, 10 Buffer Address
    
    // Read -> 528size -> Low Freq Read -> 0P: 0x03 -> 3 address bytes -> 13 Page Address, 10 Offset, 1 D/C ?
#endif

// Common for all flash chips
#define FLASH_MANUF_ID                0x1F
#define FLASH_OPCODE_SECTOR_ERASE     0x7C  // Opcode to perform a sector erase
#define FLASH_OPCODE_BLOCK_ERASE      0x50  // Opcode to perform a block erase
#define FLASH_OPCODE_PAGE_ERASE       0x81  // Opcode to perform a page erase
#define FLASH_OPCODE_READ_STAT_REG    0xD7  // Opcode to perform a read of the status register
#define FLASH_OPCODE_MAINP_TO_BUF     0x53  // Opcode to perform a Main Memory Page to Buffer Transfer
#define FLASH_OPCODE_MMP_PROG_TBUF    0x82  // Opcode to perform a Main Memory Page Program Through Buffer
#define FLASH_OPCODE_LOWF_READ        0x03  // Opcode to perform a Continuous Array Read (Low Frequency)
#define	FLASH_OPCODE_READ_DEV_INFO    0x9F  // Opcode to perform a Manufacturer and Device ID Read
#define FLASH_READY_BITMASK           0x80  // Bitmask used to determine if the chip is ready (poll status register). Used with FLASH_OPCODE_READ_STAT_REG.
#define FLASH_SECTOR_ZER0_A_PAGES     8
#define FLASH_SECTOR_ZERO_A_CODE      0
#define FLASH_SECTOR_ZERO_B_CODE      1

// Flash Testing Defines
#define TEST_FLASH                                   // Comment out to not test flash
//#define FLASH_TEST_DEBUG_OUTPUT_OLED
#define FLASH_TEST_DEBUG_OUTPUT_USB
#define FLASH_TEST_INIT_BUFFER_POLICY_ZERO           0
#define FLASH_TEST_INIT_BUFFER_POLICY_ONE            1
#define FLASH_TEST_INIT_BUFFER_POLICY_INC            2
#define FLASH_TEST_INIT_BUFFER_POLICY_RND            3

// Flash Test Selection
#define RUN_FLASH_TEST_WR
#define RUN_FLASH_TEST_WRO
#define RUN_FLASH_TEST_ERASE_PAGE
#define RUN_FLASH_TEST_ERASE_BLOCK
#define RUN_FLASH_TEST_ERASE_SECTOR_X
#define RUN_FLASH_TEST_ERASE_SECTOR_0

// Flash Page Mappings
#define FLASH_PAGE_MAPPING_NODE_META_DATA  0  // Reserving two (2) pages for node management meta data
#define FLASH_PAGE_MAPPING_NODE_MAP_START  2  // Reserving two (2) pages for node management meta data
#define FLASH_PAGE_MAPPING_NODE_MAP_END    (FLASH_PAGE_MAPPING_NODE_MAP_START + MAP_PAGES)  // Last page used for node mapping
#define FLASH_PAGE_MAPPING_GFX_START       (FLASH_PAGE_MAPPING_NODE_MAP_END + 1)  // Start GFX Mapping
#define FLASH_PAGE_MAPPING_GFX_END         (PAGE_PER_SECTOR) // End GFX Mapping
#define FLASH_PAGE_MAPPING_GFX_SIZE        (FLASH_PAGE_MAPPING_GFX_END - FLASH_PAGE_MAPPING_GFX_START)


/** DEFINES SMART CARD **/
#define SMARTCARD_FABRICATION_ZONE	0x0F0F
#define SMARTCARD_FACTORY_PIN		0xF0F0
#define SMARTCARD_DEFAULT_PIN		0xF0F0
#define SMARTCARD_AZ_BIT_LENGTH     512
#define SMARTCARD_AZ1_BIT_START     176
#define SMARTCARD_AZ1_BIT_RESERVED  16
#define SMARTCARD_MTP_PASS_LENGTH   (SMARTCARD_AZ_BIT_LENGTH - SMARTCARD_AZ1_BIT_RESERVED - AES_KEY_LENGTH)
#define SMARTCARD_AZ2_BIT_START     736
#define SMARTCARD_AZ2_BIT_RESERVED  16
#define SMARTCARD_MTP_LOGIN_LENGTH  (SMARTCARD_AZ_BIT_LENGTH - SMARTCARD_AZ2_BIT_RESERVED)

/** DEFINES PORTS **/
#ifdef HARDWARE_V1
	// SPIs
	#define SPI_SMARTCARD	SPI_NATIVE
	#define	SPI_FLASH		SPI_USART
	#define SPI_OLED		SPI_USART
	// Slave Select Flash
	#define PORTID_FLASH_nS	PORTB4
	#define PORT_FLASH_nS	PORTB
	#define DDR_FLASH_nS	DDRB
	// Detect smart card
	#define PORTID_SC_DET	PORTB6
	#define PORT_SC_DET		PORTB
	#define DDR_SC_DET		DDRB
	#define PIN_SC_DET		PINB
	// Smart card program
	#define PORTID_SC_PGM	PORTC6
	#define PORT_SC_PGM		PORTC
	#define DDR_SC_PGM		DDRC
	// Smart card power enable
	#define PORTID_SC_POW	PORTE6
	#define PORT_SC_POW		PORTE
	#define DDR_SC_POW		DDRE
	// Smart card reset
	#define PORTID_SC_RST	PORTB5
	#define PORT_SC_RST		PORTB
	#define DDR_SC_RST		DDRB
	// OLED Data / Command
	#define PORTID_OLED_DnC	PORTD7
	#define PORT_OLED_DnC	PORTD
	#define DDR_OLED_DnC	DDRD
	// OLED Slave Select
	#define PORTID_OLED_SS	PORTD6
	#define PORT_OLED_SS	PORTD
	#define DDR_OLED_SS		DDRD
	// OLED reset
	#define PORTID_OLED_nR	PORTD1
	#define PORT_OLED_nR	PORTD
	#define DDR_OLED_nR		DDRD
	// Power enable to the OLED
	#define PORTID_OLED_POW	PORTB7
	#define PORT_OLED_POW	PORTB
	#define DDR_OLED_POW	DDRB
#endif
#ifdef	HARDWARE_OLIVIER_V1
	// SPIs
	#define SPI_SMARTCARD	SPI_NATIVE
	#define	SPI_FLASH		SPI_USART
	#define SPI_OLED		SPI_USART
	// Slave Select Flash
	#define PORTID_FLASH_nS	PORTB7
	#define PORT_FLASH_nS	PORTB
	#define DDR_FLASH_nS	DDRB
	// Detect smart card
	#define PORTID_SC_DET	PORTF5
	#define PORT_SC_DET		PORTF
	#define DDR_SC_DET		DDRF
	#define PIN_SC_DET		PINF
	// Smart card program
	#define PORTID_SC_PGM	PORTC6
	#define PORT_SC_PGM		PORTC
	#define DDR_SC_PGM		DDRC
	// Smart card power enable
	#define PORTID_SC_POW	PORTB4
	#define PORT_SC_POW		PORTB
	#define DDR_SC_POW		DDRB
	// Smart card reset
	#define PORTID_SC_RST	PORTE6
	#define PORT_SC_RST		PORTE
	#define DDR_SC_RST		DDRE
	// OLED Data / Command
	#define PORTID_OLED_DnC	PORTD7
	#define PORT_OLED_DnC	PORTD
	#define DDR_OLED_DnC	DDRD
	// OLED Slave Select
	#define PORTID_OLED_SS	PORTD6
	#define PORT_OLED_SS	PORTD
	#define DDR_OLED_SS		DDRD
	// OLED reset
	#define PORTID_OLED_nR	PORTD4
	#define PORT_OLED_nR	PORTD
	#define DDR_OLED_nR		DDRD
	// Power enable to the OLED
	#define PORTID_OLED_POW	PORTE2
	#define PORT_OLED_POW	PORTE
	#define DDR_OLED_POW	DDRE
#endif

/** DEFINES OLED SCREEN **/
#define	OLED_Shift			0x1C
#define OLED_Max_Column		0x3F			// 256/4-1
#define OLED_Max_Row		0x3F			// 64-1
#define	OLED_Brightness		0x0A
#define OLED_Contrast		0x9F
#define OLED_WIDTH			256
#define OLED_HEIGHT			64

// Mooltipass bitmaps defines
#define HACKADAY_BMP		0x00

#endif /* DEFINES_H_ */
