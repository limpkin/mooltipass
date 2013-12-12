Mooltipass project github
=========================

This is the github repository dedicated to the mooltipass project. It contains all resources that were used and generated for this community driven product.


What is the mooltipass project?
-------------------------------
The mooltipass is an offline password keeper.

The concept behind this product is to minimize the number of ways your passwords can be compromised, while generating long and complex random passwords for the different websites you use daily. It is designed to be as small as possible so it can fit in your pocket. Simply visit a website and the device will ask for confirmation to enter your credentials when you need to login.

The Platform
-----------
Mooltipass is composed of one main device and a smartcard.  
On the device are stored your AES-256 encrypted passwords. The smartcard is a read protected EEPROM that needs a PIN code to unlock its contents (AES-256 key + a few websites credentials). As with your credit card, too many tries will permanently lock the smart card.  
The mooltipass main components are: a smart card connector, an Arduino compatible microcontroller, a FLASH memory, an OLED screen and its touchscreen panel. The OLED screen provides good contrast and good visibility.
