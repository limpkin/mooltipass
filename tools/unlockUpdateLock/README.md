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

Instructions
------------
- download Mooltipass.hex here https://github.com/limpkin/mooltipass/releases/tag/v1.1
- backup all your Mooltipass users databases as the Mooltipass memory will be erased
- download all the files here of this folder
- install all the prerequisites listed above
- depending on how fast your computer is, you might want to increase this particular delay: https://github.com/limpkin/mooltipass/blob/master/tools/unlockUpdateLock/unlockUpdateLock.py#L359
- if you want to set your own new mooltipass password instead of using a random one, you can modify this line: https://github.com/limpkin/mooltipass/blob/master/tools/unlockUpdateLock/unlockUpdateLock.py#L220 (be careful of the data type)
- make sure you don't have any other USB device connected to your computer, seen as serial port devices (because of this line: https://github.com/limpkin/mooltipass/blob/master/tools/unlockUpdateLock/unlockUpdateLock.py#L360)

Uploading the firmware only on windows
--------------------------------------
- Follow this tutorial: http://blog.zakkemble.co.uk/avrdudess-a-gui-for-avrdude/
- Use the settings below:
<p align="center">
  <img src="https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/unlockUpdateLock/avrdudess.png" alt="avrdudess"/>
</p>