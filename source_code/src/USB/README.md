The Mooltipass will enumerate as a composite device with hid keyboard and generic hid classes. The keyboard interface is used for sending keypresses and the generic hid is used for transmitting data to/from pc side apps and plugins.

Data sent over the generic hid is made out of a 64 byte packet. The structure of the packet is as follows:

buffer[0] = length of data

buffer[1] = cmd identifier for this packet

buffer[2 - 2 + buffer[0]] = packet data

Current commands
================
Every sent packet will get one or more packets as an answer.
Texts sent to and from the Mooltipass have payload length that include the terminating 0.
In case the user currently is entering his PIN, the MP will only reply with a 0xB9 packet
The following commands are currently implemented:

0xA0: send debug message
------------------------
From Mooltipass: packet data containing the debug message.

0xA1: ping
----------
From Plugin/app: ping packet with 4 bytes payload

From Mooltipass: same packet that the plugin/app sent

0xA2: version request
---------------------
From Plugin/app: Mooltipass version request

From Mooltipass: The first byte contains the FLASH_CHIP define which specifies how much memory the Mooltipass has. The rest is a string identifying the version

0xA3: set context
-----------------
From Plugin/app: this allows the plugin/application to let the mooltipass know the website/service he's currently on

From Mooltipass: 1 byte data packet, 0x00 indicates that the Mooltipass doesn't know the context, 0x01 if so and 0x03 that there's no card in the mooltipass

0xA4: get login
---------------
From plugin/app: request the login for the current context

From Mooltipass: the login if the user has approved the sending of credential / has been authorized, 1 byte 0x00 packet otherwise.

0xA5: get password
------------------
From plugin/app: request the password for the current context & login

From Mooltipass: 1 byte data packet, 0x00 indicates that the Mooltipass didn't send the password, data otherwise.

0xA6: set login
---------------
From plugin/app: set the login for the current context (either create a credential or select a given credential set)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xA7: set password
------------------
From plugin/app: set the password for the current context

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xA8: check password
--------------------
From plugin/app: check the password for the current context & selected login

From Mooltipass: 1 byte data packet, 0x00 indicates that password is not correct, 0x01 indicates the password is correct, 0x02 indicates the timer is still running so the request is blocked

0xA9: add context
-----------------
From plugin/app: add a new context inside the mooltipass

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xAC: Get random number
-----------------------
From plugin/app: Ask 32 random bytes

From Mooltipass: The 32 random bytes

0xAD: Start memory management mode
----------------------------------
From plugin/app: Ask the user to approve memory management mode

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xAE: Media import start
------------------------
From plugin/app: Request for media contents sending to Mooltipass.

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xAF: Media import
------------------
From plugin/app: A bunch of data to store inside the media part of flash, particularly formatted (see source files)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xB0: Media import end
----------------------
From plugin/app: Inform that we finished the media flash import

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xB1: Set Mooltipass parameter
------------------------------
From plugin/app: Set Mooltipass parameter, first byte is the param ID, second is the value

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xB2: Get Mooltipass parameter
------------------------------
From plugin/app: Get parameter from Mooltipass, first byte is the param ID

From Mooltipass: The param value

0xB3: Reset card
----------------
From plugin/app: Reset inserted card

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xB4: Read Card Login
---------------------
From plugin/app: Read login stored inside the smartcard (no confirmation asked from the user)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, data otherwise

0xB5: Read Card Password
------------------------
From plugin/app: Read password stored inside the smartcard (confirmation asked to the user)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, data otherwise

0xB6: Set Card Login
--------------------
From plugin/app: Set login stored inside the smartcard, 62bytes max length (confirmation asked to the user)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xB7: Set Card Password
-----------------------
From plugin/app: Set password stored inside the smartcard, 30bytes max length (confirmation asked to the user)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xB8: Add unknown smartcard
---------------------------
From plugin/app: When an unknown smartcard is inserted, tell the Mooltipass to store it. First 8 bytes are the current card CPZ, 16 next bytes are our user CTR value

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, data otherwise

0xB9: Mooltipass status
-----------------------
From plugin/app: Query Mooltipass status

From Mooltipass: 1 bytes bit field. BIT0: smartcard presence. BIT1: pin unlocking screen. BIT2: smartcard present and unlocked. BIT3: Unknown smartcard inserted

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

0xBB: Set current date
----------------------
From plugin/app: Set current date (16 bits encoding: 15 dn 9 -> Year (2010 + val), 8 dn 5 -> Month, 4 dn 0 -> Day of Month)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xBC: Set Mooltipass UID
------------------------
From plugin/app: Set the Mooltipass UID, 16 + 6 bytes packet with the request key and the UID

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, data otherwise

0xBD: Get Mooltipass UID
------------------------
From plugin/app: Get Mooltipass UID. The 16 bytes request key must be sent

From Mooltipass: Either a one byte packet when an error occurs or the mooltipass UID

0xBE: set data context
----------------------
From Plugin/app: this allows the plugin/application to let the mooltipass know the data service he's currently on

From Mooltipass: 1 byte data packet, 0x00 indicates that the Mooltipass doesn't know the context, 0x01 if so and 0x03 that there's no card in the mooltipass

0xBF: add data context
----------------------
From plugin/app: add a new data context inside the mooltipass

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xC0: Write 32 bytes in current context
---------------------------------------
From plugin/app: add 32 bytes of data to the current data context. If first byte different to 0, means it is the last 32B block. 32 bytes data block starts at payload[1]

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xC1: Read 32 bytes in current context
---------------------------------------
From plugin/app: after a set data context has been sent, get successive 32bytes data blocks

From Mooltipass: 0x00 when error or end of data, 32 bytes of data otherwise

0xC2: Get current card CPZ
--------------------------
From plugin/app: ask the CPZ of the currently inserted card, when unknown card is inserted

From Mooltipass: 0x00 when error or end of data, the CPZ otherwise

0xC3: Cancel User Request
-------------------------
From plugin/app: cancel input request from user

0xC4: Please Retry
------------------
From Mooltipass: Message informing the app to re-send the previous packet

Commands in data management mode
================================

0xC5: Read node in flash
------------------------
From plugin/app: With two bytes indicating the node number, read a user node in flash

From Mooltipass: The node or 0x00 when error

0xC6: Write node in flash
-------------------------
From plugin/app: With two bytes indicating the node number and another indicating the packet #, write a node in flash. See source for data formatting

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xC7: Get Favorite
------------------
From plugin/app: Get favorite for current user, first byte is the slot ID

From Mooltipass: 0x00 if not performed, 4 bytes of data otherwise (parent + child addr)

0xC8: Set favorite
------------------
From plugin/app: First byte indicates favId, next 2 the parent addr, next 2 the child addr

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xC9: Get Starting parent Address
---------------------------------
From plugin/app: In management mode, get the address of the starting parent

From Mooltipass: 0x00 if failure, slot address otherwise

0xCA: Set starting parent
-------------------------
From plugin/app: First two bytes is the new starting parent (LSB first)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xCB: Get CTR value
-------------------
From plugin/app: In management mode, get the current user CTR value

From Mooltipass: 0x00 if failure, CTR value otherwise

0xCC: Set CTR value
-------------------
From plugin/app: First three bytes is the new CTR value

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xCD: Add CPZ CTR value
-----------------------
From plugin/app: Add a known card to the MP, 8 first bytes is the CPZ, next 16 is the CTR nonce

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xCE: Get CPZ CTR value
-----------------------
From plugin/app: Get all the cpz ctr values for current user

From Mooltipass: 0x00 if denied, 0xCF packets for LUT entries, then a final 0x01

0xCF: CPZ CTR packet export
---------------------------
From Mooltipass: One CPZ CTR LUT entry packet

0xD0: Get Free Slot Addresses
-----------------------------
From plugin/app: 2 bytes payload indicating the address to start scanning from (in doubt, set 0x00 0x00).

From Mooltipass: 0x00 if failure, 31 slot addresses max otherwise (see payload length field)

0xD1: Get Starting data parent Address
--------------------------------------
From plugin/app: In management mode, get the address of the data starting parent

From Mooltipass: 0x00 if failure, slot address otherwise

0xD2: Set starting data parent
------------------------------
From plugin/app: First two bytes is the new data starting parent (LSB first)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0xD3: End memory management mode
--------------------------------
From plugin/app: Leave memory management mode

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

Obsolete commands
=================

0x8A: export flash start (for full import/export fw version)
------------------------------------------------------------
From plugin/app: Request for flash contents export to Mooltipass.

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x8B: export flash (for full import/export fw version)
------------------------------------------------------
From plugin/app: request for a bunch of data

From Mooltipass: the bunch of requested data

0x8C: export flash end (for full import/export fw version)
----------------------------------------------------------
From plugin/app: stop flash export

From Mooltipass: end of flash export

0x8D: import flash start (for full import/export fw version)
------------------------------------------------------------
From plugin/app: Request for flash contents sending to Mooltipass. A 0x00 in parameter implies user space while a 0x01 specifies the graphics part of flash

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x8E: import flash (for full import/export fw version)
------------------------------------------------------
From plugin/app: A bunch of data to store inside the flash, particularly formatted (see source files)

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x8F: import flash end (for full import/export fw version)
----------------------
From plugin/app: Inform that we finished the flash import

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x90: export eeprom start (for full import/export fw version)
-------------------------------------------------------------
From plugin/app: Request for eeprom contents export to Mooltipass.

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x91: export eeprom (for full import/export fw version)
-------------------------------------------------------
From plugin/app: export eeprom contents request

From Mooltipass: the bunch of requested data

0x92: export eeprom end (for full import/export fw version)
-----------------------------------------------------------
From plugin/app: stop eeprom export

From Mooltipass: end of eeprom export

0x93: import eeprom start (for full import/export fw version)
-------------------------------------------------------------
From plugin/app: Request for eeprom contents sending to Mooltipass.

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x94: import eeprom (for full import/export fw version)
-------------------------------------------------------
From plugin/app: A bunch of data to store inside the eeprom

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so

0x95: import eeprom end (for full import/export fw version)
-----------------------------------------------------------
From plugin/app: Inform that we finished the flash import

From Mooltipass: 1 byte data packet, 0x00 indicates that the request wasn't performed, 0x01 if so
