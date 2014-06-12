USB Information
===============

The mooltipass will enumerate as a composite hid device with a keyboard and generic hid. The keyboard is used for sending keypresses and the generic hid is used for transmitting data to/from pc side apps or plugins.

Data sent over the generic hid is made out of a 64 byte packet. The structure of the packet is as follows:

buffer[0] = length of data

buffer[1] = cmd identifier for this packet

buffer[2 - packetsize] = packet data

Commands
--------
The following commands are currently implemented:

0x01: send debug message
------------------------
(From plugin/app & Mooltipass): packet data contains the debug message.

0x02: ping request
------------------
(From plugin/app & Mooltipass): responds with a command packet with the same cmd id, no data in data packet

0x03: version request
---------------------
(From plugin/app & Mooltipass): responds with a command packet with the same cmd id, data contains major and minor version of mooltipass in first and second byte of data packet

0x04: set context
-----------------
From plugin/app: this allows the plugin/application to let the mooltipass know the website/service he's currently on

0x05: get login
---------------
From plugin/app: request the login for the current context
From Mooltipass: the login if the user has approved the sending of credential / has been authentified, error code otherwise 


Functions
=========

The following low level functions are implemented for sending data from the mooltipass to the pc:

RET_TYPE usbKeybPutStr(char* string)
------------------------------------
Type the desired string
