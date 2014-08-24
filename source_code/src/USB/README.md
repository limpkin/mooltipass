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
(From Plugin/App & Mooltipass): packet data contains the debug message.

0x02: ping request
------------------
(From Plugin/App & Mooltipass): responds with a command packet with the same cmd id, 4 bytes of data

0x03: version request
---------------------
(From Plugin/App & Mooltipass): responds with a command packet with the same cmd id, data contains major, minor version and build number of the mooltipass. The first byte contains the FLASH_CHIP define which specifies how much memory the Mooltipass has.

0x04: set context
-----------------
From Plugin/app: this allows the plugin/application to let the mooltipass know the website/service he's currently on

From Mooltipass: 1 byte data packet, 0x00 indicates that the Mooltipass doesn't know the context, 0x01 if so and 0x03 that there's no card in the mooltipass

0x05: get login
---------------
From plugin/app: request the login for the current context

From Mooltipass: the login if the user has approved the sending of credential / has been authentified, 1 byte 0x00 packet otherwise.

0x06: get password
------------------
From plugin/app: request the password for the current context

From Mooltipass: 1 byte data packet, 0x00 indicates that the Mooltipass didn't send the password, 0x01 if so.

0x07: set login
---------------
From plugin/app: set the login for the current context (either create a credential or select a given credential set)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x08: set password
------------------
From plugin/app: set the password for the current context

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x09: check password
--------------------
From plugin/app: check the password for the current context & selected login

From Mooltipass: 1 byte data packet, 0x00 indicates that password is not correct, 0x01 indicates the password is correct, 0x02 indicates the timer is still running to the request is blocked

0x0A: add context
-----------------
From plugin/app: add a new context inside the mooltipass

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x45: export flash start (for full import/export fw version)
------------------------------------------------------------
From plugin/app: Request for flash contents export to Mooltipass.

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x30: export flash (for full import/export fw version)
------------------------------------------------------
From plugin/app: request for a bunch of data

From Mooltipass: the bunch of requested data

0x31: export flash end (for full import/export fw version)
----------------------------------------------------------
From plugin/app: stop flash export

From Mooltipass: end of flash export

0x32: import flash start (for full import/export fw version)
------------------------------------------------------------
From plugin/app: Request for flash contents sending to Mooltipass. A 0x00 in parameter implies user space while a 0x01 specifies the graphics part of flash

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x33: import flash (for full import/export fw version)
------------------------------------------------------
From plugin/app: A bunch of data to store inside the flash, particularly formatted (see source files)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x34: import flash end (for full import/export fw version)
----------------------
From plugin/app: Inform that we finished the flash import

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x46: export eeprom start
-------------------------
From plugin/app: Request for eeprom contents export to Mooltipass.

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x35: export eeprom
-------------------
From plugin/app: export eeprom contents request

From Mooltipass: the bunch of requested data

0x36: export eeprom end
-----------------------
From plugin/app: stop eeprom export

From Mooltipass: end of eeprom export

0x37: import eeprom start
-------------------------
From plugin/app: Request for eeprom contents sending to Mooltipass. 

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x38: import eeprom
-------------------
From plugin/app: A bunch of data to store inside the eeprom

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x39: import eeprom end
-----------------------
From plugin/app: Inform that we finished the flash import

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so


Functions
=========

The following low level functions are implemented for sending data from the mooltipass to the pc:

RET_TYPE usbKeybPutStr(char* string)
------------------------------------
Type the desired string
