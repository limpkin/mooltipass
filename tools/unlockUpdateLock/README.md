Mooltipass Firmware Update Tool
=================================
Python script used to update the Mooltipass Standard Version, which is equipped with the Caterina bootloader

Requirements
------------
- Python 2.7
- Linux or Windows
- Windows: libusb at http://sourceforge.net/projects/libusb-win32/files/
- All python modules listed at the beginning of the unlockUpdateLock.py file (pip install -modulename-)
- New firmware file placed in the same directory, named "Mooltipass.hex"
- Pyusb: pip install --pre pyusb
- script launched as root in Linux

Instructions
------------
- download Mooltipass.hex here https://github.com/limpkin/mooltipass/releases/tag/v1.1_main
- **backup all your Mooltipass users databases** as the Mooltipass memory will be erased
- Windows: use the libusb filter wizard to set filters for the 3 entries (VID 16D0 PID 09A0)
- install all the prerequisites listed above
- download all the files of this folder
- disable the Mooltipass Chrome App
- run the script

Warning
-------
It seems that due to a possible Kernel bug our updating script won't work on some Ubuntu computers. We therefore recommend using Windows for this particular script.  
Bug: http://superuser.com/questions/979722/serialport-doenst-work-correctly-after-ubuntu-update

Using the Beta Testers App & Extensions
---------------------------------------
Only stable versions of our App & Extension are pushed to our customers. However, several new features may not be pushed during a few weeks as they're being tested by our beta testers. If you want to use the same App & Extension as our beta testers, you may use these links:
- https://chrome.google.com/webstore/detail/mooltipass-extension/mkjlelalgdinanmcljpgkojjolkdcebh  
- https://chrome.google.com/webstore/detail/mooltipass-app/nbjmdaimooaemcgoodjmpjkabpdbaink  
**Don't forget to uninstall your previous App / Extension before installing these ones**

Troubleshooting
---------------
##### 1) Error after "reconnecting to device"
Script output:  
Programming done... reconnecting to device...  
Exception usb.core.USBError: USBError(None, 'libusb0-dll:err [release_interface] could not release interface 0, win error: The device does not recognize the command.\r\n')  
**Solution:** Make sure the Mooltipass App is disabled, then re-run the script: python unlockUpdateLock.py skip
