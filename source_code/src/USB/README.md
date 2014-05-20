USB Information
===============

* just some notes while implementing the new usb stuff

usb hid

buffer is 64 bytes

buffer[0] packetsize

buffer[1] packetcmd

buffer[2 - packetsize] data

packetcmd:

0x01: debug message

0x02: ping

0x03: version
