Arduino support for the Mooltipass
==================================

Modifying boards.txt
--------------------
- Open C:\Program Files (x86)\Arduino\hardware\arduino\avr\boards.txt
- At its end, add:
```
##############################################################

mooltipass.name=Mooltipass
mooltipass.vid.0=0x1209
mooltipass.pid.0=0x4321

#mooltipass.vid.0x2A03.warning=Uncertified

mooltipass.upload.tool=avrdude
mooltipass.upload.protocol=avr109
mooltipass.upload.maximum_size=28672
mooltipass.upload.maximum_data_size=2560
mooltipass.upload.speed=57600
mooltipass.upload.disable_flushing=true
mooltipass.upload.use_1200bps_touch=true
mooltipass.upload.wait_for_upload_port=true

mooltipass.bootloader.tool=avrdude
mooltipass.bootloader.low_fuses=0xff
mooltipass.bootloader.high_fuses=0xd8
mooltipass.bootloader.extended_fuses=0xf8
mooltipass.bootloader.file=mooltipass_caterina_bl
mooltipass.bootloader.unlock_bits=0x3F
mooltipass.bootloader.lock_bits=0xFC

mooltipass.build.mcu=atmega32u4
mooltipass.build.f_cpu=16000000L
mooltipass.build.vid=0x1209
mooltipass.build.pid=0x4322
mooltipass.build.usb_product="Mooltipass Arduino Sketch"
mooltipass.build.board=AVR_LEONARDO
mooltipass.build.core=arduino
mooltipass.build.variant=leonardo
mooltipass.build.extra_flags={build.usb_flags}
```

Windows Drivers
---------------
On windows, install our driver located in arduino\driver

Mooltipass Arduino Libraries
----------------------------
Copy the contents of the arduino\libraries folder in C:\Program Files (x86)\Arduino\libraries

Arduino IDE preferences
-----------------------
File > Preferences > tick show verbose output during compilation & upload

Pinout
------
<p align="center">
  <img src="https://raw.githubusercontent.com/limpkin/mooltipass/master/arduino/pinout_small.jpg" alt="Arduino pinout"/>
</p>

Warning!
--------
At least one of the two-byte words present at the beginning and at the end of the eeprom must remain intact.
