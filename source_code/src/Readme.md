# Building Mooltipass

## Windows
Use the Atmel Studio files

## Linux

### Dependencies
```bash
# Install avr tools
sudo apt-get install gcc-avr avr-libc

# Remove the modem manager
# It causes problems with the old caternia bootloader
sudo apt-get purge modemmanager

# Download dependencies (for dmbs)
git submodule init
git submodule update

# Compile avr-gcc 6.1 (optional for smaller code size)
cd /source_codesource_code/AVR-Development-Environment-Script/
./build
```

### DMBS
[DMBS](https://github.com/abcminiuser/dmbs) is a makefile build system for AVR.
It is very simple to maintain and the makefile itself is very compact.

DMBS is used inside the `source_code/src` path. Run `make` to compile the
firmware. You can also compile the bootloader. Edit the `makefile` and set
`BOOTLOADER = true`.

### Legacy makefile
The legacy makefile is located in `source_code`. Run `make` to
compile the firmware. It might be outdated, as it is harder to maintain.
Use DMBS if you run into any issues.
