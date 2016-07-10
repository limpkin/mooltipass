# Building Mooltipass

## Windows
Use the Atmel Studio files

## Linux

### Dependencies
```bash
# Install avr tools
# avr-gcc 4.9.2 of Ubuntu 16.04 is recommended to use the LTO plugin
sudo apt-get install gcc-avr avr-libc
avr-gcc -v

# Remove the modem manager
# It causes problems with the old caternia bootloader
sudo apt-get purge modemmanager

# Download dependencies (for dmbs)
git submodule init
git submodule update

# Compile the code
make clean
make
```

### DMBS
[DMBS](https://github.com/abcminiuser/dmbs) is a makefile build system for AVR.
It is very simple to maintain and the makefile itself is very compact.

DMBS is used inside the `source_code/src` path. Run `make` to compile the
firmware. You can also compile the bootloader. Edit the `makefile` and set
`BOOTLOADER = true`.

### Legacy makefile
The legacy makefile is located in `source_code`. Run `make` to
compile the firmware.
