USB Information
===============

* just some note while implementing the new usb stuff

usb hid

buffer is 32 bytes

buffer[0] packetsize
buffer[1] packetcmd
buffer[2 - packetsize] data

packetcmd:

0x01: debug message
