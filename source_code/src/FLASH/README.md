#Flash Memory Library for Mooltipass
Michael Neiderhauser
Jun-15-2014

Files:
- flash_mem.c
- flash_mem.h

The Flash Library was written to add an access layer to write to the SPI flash.
Currently the library only permits reading and writing pages and offsets of pages meaning that the library does not support writing over page boundaries.

The Flash Library allows usage of a subset of the Adesto family of SPI flash chips. The Flash Library supports the following chips:
- 1M   (AT45DB011D) - (512  Pages at 264 Bytes Per Page)
- 2M   (AT45DB021E) - (1024 Pages at 264 Bytes Per Page)
- 4M   (AT45DB041E) - (2048 Pages at 264 Bytes Per Page)
- 8M   (AT45DB081E) - (4096 Pages at 264 Bytes Per Page)
- 16M  (AT45DB161E) - (4096 Pages at 528 Bytes Per Page)
- 32M  (AT45DB321E) - (8192 Pages at 528 Bytes Per Page)

To select a chip. Open the flash_mem.h file and change the FLASH_CHIP define.

The library currently implements the following:
- Initialize the Flash (sets up SPI communications)
- Send SPI data to the FLASH (R/W)
- Check the Flash Chip 'Ready' Register
- Check the Manf. Flash ID
- Sector Erase (Zero Sector)
- Sector Erase (All other Sectors)
- Block Erase
- Page Erase
- Format Flash
      Similar to a chip erase, this implementation uses the Sector Erase Library Functions to format the flash.
- Write Data To Flash  (Page + Offset. Write Buffer)
- Read Data From Flash (Page + Offset. Read Buffer)


## Flash Memory Testing
Files:
- flash_test.c
- flash_test.h

Flash Testing attempts to exercise the library and obtain status on the flash chip.
To run Flash Testing:
Include the flash_test.h file and call the flashTest() Function.

To run Flash Testing on the Mooltipass:
In the file tests.c uncomment the (pound)define TEST_FLASH in the beforeFlashInitTests() function.

Note:  The flash_test.c file does use oled and usb libs of the Mooltipass however this should be easy to remove if needed.


##Library TODO's
- Implement support for the 64M Flash Chip
- Remove 'Node' specific defines (remove integration with node_mgmt library)
- Update API to better use other flash chip operations
- Add other functionality (based off of the data sheet)
