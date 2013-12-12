Mooltipass project github
=========================

This is the github repository dedicated to the mooltipass project. It contains all resources that were used and generated for this community driven product.


What is the mooltipass project?
-------------------------------
The mooltipass is an offline password keeper.

The concept behind this product is to minimize the number of ways your passwords can be compromised, while generating and storing long and complex random passwords for the different websites you use daily. It is designed to be as small as possible so it can fit in your pocket. Simply visit a website and the device will ask for confirmation to enter your credentials when you need to login.

The Platform
------------
Mooltipass is composed of one main device and a smartcard.  
On the device are stored your AES-256 encrypted passwords. The smartcard is a read protected EEPROM that needs a PIN code to unlock its contents (AES-256 key + a few websites credentials). As with your credit card, too many tries will permanently lock the smart card.  
The mooltipass main components are: a smart card connector, an Arduino compatible microcontroller, a FLASH memory, an OLED screen and its touchscreen panel. The OLED screen provides good contrast and good visibility.

The firmware
------------
We want the device to be as simple as possible to use. Ideally, the end user shouldn't have to spend more than a few seconds to use its basic functionalities.  
A browser extension will run on the user's computer, that will send (via HID) the current website to the mooltipass. When the user has to login, the mooltipass will light up and will ask for confirmation to enter the credentials.

Data safety
-----------
As we're dealing with peoples' credentials, we want to be as careful as possible.  
The smart card containing the AES key used for encrypting the password can be cloned using the mooltipass, copying its PIN code as well. The card copy should be stored somewhere safe. As some space is left in the smart card, the user should also be able to store his main email credentials.  
The encrypted credentials stored in the mooltipass internal flash can be exported.  
A very convenient functionally suggested by one of Hackaday's readers is also implemented: the ability to generate hashed answers for websites' security questions in case credentials are lost. Only the smartcard is required to use this feature. 
