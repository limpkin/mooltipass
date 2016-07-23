
set PATH=%PATH%;C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin
avr-nm.exe --size-sort -t decimal ../Release/bootloader_mini.elf > data_usage.txt
START elf_analysis.r