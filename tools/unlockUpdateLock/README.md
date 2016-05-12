Mooltipass Firmware Update Tool
=================================
Python script used to update the Mooltipass Standard Version, which is equipped with the Caterina bootloader

Requirements
------------
- Debian based Linux OS
- Python 2.7
- All python modules listed at the beginning of the .py file
- Pyusb: pip install --pre pyusb
- avrdude
- script launched as root
- New firmware file placed in the same dir, named "Mooltipass.hex"