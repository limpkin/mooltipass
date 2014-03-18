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
A browser extension will run on the user's computer, that will send (via HID) the current website to the Mooltipass. When the user has to login, the Mooltipass will light up and will ask for confirmation to enter the credentials.

Data safety
-----------
As we're dealing with peoples' credentials, we want to be as careful as possible.  
The smart card containing the AES key used for encrypting the password can be cloned using the Mooltipass, copying its PIN code as well. The card copy should be stored somewhere safe. As some space is left in the smart card, the user should also be able to store his main email credentials.  
The encrypted credentials stored in the Mooltipass internal flash can be exported.  
A very convenient functionally suggested by one of Hackaday's readers is also implemented: the ability to generate hashed answers for websites' security questions in case credentials are lost. Only the smartcard is required to use this feature. 

Features suggested by Hackaday readers
--------------------------------------
- Have multiple smart cards for multiple users on one Mooltipass  
- Generate a QR code instead of sending the password via HID  
- Have a hole on the device's side to pour resin  
- buy a PID from MCS electronics  
- implement OTP  

Frequently Asked Questions
--------------------------
**How expensive will the Mooltipass be?**  
Price is one of our main constraints when designing the Mooltipass. It's still too early to tell the final price of this project, especially given that it will depend on how many people are willing to buy it.  
In the meantime, you can still have a look at our current Bill of Materials located in the kicad folder.  
  
**Why not making the device tiny?**  
The Mooltipass is intended for many different persons. Having a normally sized OLED screen provides good readability and therefore better user experience.   
The device also includes Arduino headers that will allow any Arduino shield to be connected to it. Hence, we made the Mooltipass as small as possible while keeping great features.  
  
**Why are you using both a smart card and a main Mooltipass device?**  
There are many reasons, the main one being that it is much easier to carry a smart card around than any other object.   
This smart card is a secure element that contains your credentials' encryption key, it is cheap and may be cloned without compromising the system security.  
  
**What if I loose my smartcard?**  
Ideally, you should have at least two copies of your smartcard, stored in different places. The Mooltipass allows the user to clone his smartcard, provided that the card PIN is correctly entered.  
  
**What if I loose my Mooltipass device?**  
Your encrypted credentials can be exported to either your computer or the Mooltipass official website. If you loose your device, you may purchase another one and restore your credentials.  
  
**Can I use it on Windows/Linux/Mac?**  
No drivers are required to use the Mooltipass. It is recognized as a standard USB keyboard that will enter passwords for you.  
  
**Can I use on my computer/laptop/phone/tablet...?**  
All devices (including smart phones and tablet PCs) include a USB host capable port. The Mooltipass will work with all of them.  



