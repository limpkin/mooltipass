USB Information
===============

The mooltipass will enumerate as a composite hid device with a keyboard and generic hid. The keyboard is used for sending keypresses and the generic hid is used for transmitting data to/from pc side apps or plugins.

Data sent over the generic hid is made out of a 64 byte packet. The structure of the packet is as follows:

buffer[0] = length of data

buffer[1] = cmd identifier for this packet

buffer[2 - packetsize] = packet data

Commands
--------
Every sent packet will get one or more packets as an answer.
In case the user currently is entering his PIN, the MP will send an empty 0x70 packet
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

From Mooltipass: 1 byte data packet, 0x00 indicates that the Mooltipass didn't send the password, data otherwise.

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

0x46: export eeprom start (for full import/export fw version)
-------------------------------------------------------------
From plugin/app: Request for eeprom contents export to Mooltipass.

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x35: export eeprom (for full import/export fw version)
-------------------------------------------------------
From plugin/app: export eeprom contents request

From Mooltipass: the bunch of requested data

0x36: export eeprom end (for full import/export fw version)
-----------------------------------------------------------
From plugin/app: stop eeprom export

From Mooltipass: end of eeprom export

0x37: import eeprom start (for full import/export fw version)
-------------------------------------------------------------
From plugin/app: Request for eeprom contents sending to Mooltipass.

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x38: import eeprom (for full import/export fw version)
-------------------------------------------------------
From plugin/app: A bunch of data to store inside the eeprom

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x39: import eeprom end (for full import/export fw version)
-----------------------------------------------------------
From plugin/app: Inform that we finished the flash import

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x4B: Get random number
-----------------------
From plugin/app: Ask 32 random bytes

From Mooltipass: The 32 random bytes

0x50: Start memory management mode
----------------------------------
From plugin/app: Ask the user to approve memory management mode

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x51: End memory management mode
--------------------------------
From plugin/app: Leave memory management mode

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x52: Media import start
------------------------
From plugin/app: Request for media contents sending to Mooltipass.

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x53: Media import
------------------
From plugin/app: A bunch of data to store inside the media part of flash, particularly formatted (see source files)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x54: Media import end
----------------------
From plugin/app: Inform that we finished the media flash import

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x55: Read node in flash
------------------------
From plugin/app: With two bytes indicating the node number, read a user node in flash

From Mooltipass: The node

0x56: Write node in flash
-------------------------
From plugin/app: With two bytes indicating the node number and another indicating the packet #, write a node in flash

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x57: Set favorite
------------------
From plugin/app: First byte indicates favId, next 2 the parent addr, next 2 the child addr

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x58: Set starting parent
-------------------------
From plugin/app: First two bytes is the new starting parent (LSB first)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x59: Set CTR value
-------------------
From plugin/app: First three bytes is the new CTR value

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x5A: Add CPZ CTR value
-----------------------
From plugin/app: Add a known card to the MP, 8 first bytes is the CPZ, next 16 is the CTR nonce

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x5B: Get CPZ CTR value
-----------------------
From plugin/app: Get all the cpz ctr values for current user

From Mooltipass: 0x00 if denied, data otherwise

0x5C: CPZ CTR packet export
---------------------------
From Mooltipass: One CPZ CTR LUT entry packet

0x5D: Set Mooltipass parameter
------------------------------
From plugin/app: Set Mooltipass parameter, first byte is the param ID, second is the value

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x5E: Get Mooltipass parameter
------------------------------
From plugin/app: Get parameter from Mooltipass, first byte is the param ID

From Mooltipass: The param value

0x5F: Get Favorite
------------------
From plugin/app: Get favorite for current user, first byte is the slot ID

From Mooltipass: 0x00 if not performed, data otherwise

0x60: Reset card
----------------
From plugin/app: Reset inserted card, first 2 bytes is the pin code

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x61: Read Card Login
---------------------
From plugin/app: Read login stored inside the smartcard (no confirmation asked from the user)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, data otherwise

0x62: Read Card Password
------------------------
From plugin/app: Read password stored inside the smartcard (confirmation asked to the user)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, data otherwise

0x63: Set Card Login
--------------------
From plugin/app: Set login stored inside the smartcard, 62bytes max length (confirmation asked to the user)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x64: Set Card Password
-----------------------
From plugin/app: Set password stored inside the smartcard, 30bytes max length (confirmation asked to the user)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x65: Get Free Slot Address
---------------------------
From plugin/app: In management mode, get the address for a free slot to store data

From Mooltipass: 0x00 if failure, slot address otherwise

0x66: Get Starting parent Address
---------------------------------
From plugin/app: In management mode, get the address of the starting parent

From Mooltipass: 0x00 if failure, slot address otherwise

0x67: Get CTR value
-------------------
From plugin/app: In management mode, get the current user CTR value

From Mooltipass: 0x00 if failure, CTR value otherwise

0x68: Add unknown smartcard
---------------------------
From plugin/app: When an unknown smartcard is inserted, tell the Mooltipass to store it. First 2 bytes are the pin code, next 16 are our CTR value

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, data otherwise

0x70: Mooltipass status
-----------------------
From plugin/app: Query Mooltipass status

From Mooltipass: 1 bytes bit field. BIT0: smartcard presence. BIT1: pin unlocking screen. BIT2: smartcard present and unlocked

```
0b000 -> No Card
0b001 -> Locked
0b010 -> Error (shouldn't happen)
0b011 -> Locked (unlocking screen)
0b100 -> Error (shouldn't happen)
0b101 -> Unlocked
0b110 -> Error (shouldn't happen)
0b111 -> Error (shouldn't happen)
```

Functions
=========

The following low level functions are implemented for sending data from the mooltipass to the pc:

RET_TYPE usbKeybPutStr(char* string)
------------------------------------
Type the desired string
