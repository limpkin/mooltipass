Mooltipass Firmware Update Tool
=================================
Python script used to update the Mooltipass Standard Version, which is equipped with the Caterina bootloader

Requirements
------------
- Linux or Windows OS
- Python 2.7
- All python modules listed at the beginning of the unlockUpdateLock.py file
- Windows: libusb at http://sourceforge.net/projects/libusb-win32/files/
- Pyusb: pip install --pre pyusb
- script launched as root
- New firmware file placed in the same dir, named "Mooltipass.hex"

Instructions
------------
- download Mooltipass.hex here https://github.com/limpkin/mooltipass/releases/tag/v1.1
- backup all your Mooltipass users databases as the Mooltipass memory will be erased
- download all the files here of this folder
- install all the prerequisites listed above

Warning
-------
It seems that due to a possible Kernel bug our upadting script won't work on some Ubuntu computers: http://superuser.com/questions/979722/serialport-doenst-work-correctly-after-ubuntu-update
We therefore recommend using Windows for this particular script.