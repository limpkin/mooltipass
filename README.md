Mooltipass Project Repository
=============================
![Mooltipass first prototype](https://raw.githubusercontent.com/limpkin/mooltipass/master/design_pictures/mooltipass_first_prototype_small.jpg)  
This is the GitHub repository dedicated to the Mooltipass project. It contains all the resources that were used and generated for this community driven product. This project was first started in December 2013 and you may find all the Hackaday articles detailing the different steps using <a href="http://hackaday.com/tag/developed-on-hackaday/">this link</a>.


What is the Mooltipass Project?
-------------------------------
The Mooltipass is an **offline** password keeper.

The concept behind this product is to minimize the number of ways your passwords can be compromised, while generating and storing long and complex random passwords for the different websites you use daily. It is designed to be as small as possible to fit in your pocket. Simply visit a website and the device will ask for your confirmation to enter your credentials when login is required.  
The Mooltipass is a standalone device connected through USB, is completely driver-less and is compatible with all major operating systems on PCs, Macs and Smartphones. Contrary to software-based password keeping solutions your passwords can't all be compromised at once, as our device essentially is a smart keyboard typing passwords for you.

The Platform
------------
The Mooltipass is composed of the main device shown above and a smartcard.  
On the device are stored your AES-256 encrypted credentials. The smartcard is a read protected EEPROM that needs a PIN code to unlock its contents (AES-256 key + a few websites credentials). As with your credit card, too many tries will permanently lock the smart card.

The Firmware
------------
We want the device to be as simple as possible to use. Ideally, the end user shouldn't have to spend more than a few seconds to use its basic functionalities.  
A browser extension runs on the user's computer and sends the current website to the Mooltipass. When the user has to login, the Mooltipass will light up and will ask for confirmation to enter the credentials.

Data Safety
-----------
Safety is a primary concern for the Mooltipass development team, which is why we offer several **safe** ways to backup your credentials.  
The smart card containing the AES key used for encrypting the password can be cloned using the Mooltipass, copying its PIN code as well. The encrypted credentials stored in the Mooltipass internal flash can be exported to the official Mooltipass website or simply somewhere on your local computer. A very convenient functionally suggested by one of Hackaday's readers is also implemented: the ability to generate hashed answers for websites' security questions in case credentials are lost.

Frequently Asked Questions
--------------------------
**How expensive will the Mooltipass be?**  
Price was one of our main constraints when designing the Mooltipass. It's still too early to tell the final price of this device but our current goal is a $70 price. In the meantime, you can still have a look at our current Bill of Materials located in the kicad folder to get an overall idea of the current costs.  
  
**Why not making the device tiny?**  
Size is a compromise between transportability and user friendliness.  
As the Mooltipass is intended for many different users, we decided to opt for a normal-sized OLED screen providing a good readability and therefore a better user experience. The device also includes Arduino headers that will allow any Arduino shield to be connected to it. Hence, we made the Mooltipass as small as possible while keeping its great features.  
  
**Why are you using both a smart card and a main Mooltipass device?**  
There are many reasons, the main one being that it is much easier to carry a smart card around than any other object. This smart card is a secure element that contains your credentials' encryption key, it is cheap and may be cloned without compromising the system security.   
  
**What if I lose my smartcard?**  
Our device is shipped with two smartcards, so you can keep your copy somewhere safe. The Mooltipass therefore allows the user to clone his smartcard as many times as he wants, provided that the card PIN is correctly entered.  
  
**What if I lose my Mooltipass device?**  
Your encrypted credentials can be exported to either your computer or the Mooltipass official website. If you lose your device, you may purchase another one and restore your credentials.  
  
**Can I use it on Windows/Linux/Mac?**  
Yes, as no drivers are required to use the Mooltipass. It is recognized as a standard USB keyboard that will enter passwords for you.  
  
**Can I use on my computer/laptop/phone/tablet...?**  
Most devices (including smart phones and tablet PCs) include a USB host capable port. The Mooltipass will work with all of them.  

**How secure is the Mooltipass?**  
We are using the most secure encryption algorithms and designed our case to make physical tampering practically impossible. Our solution is therefore perfectly suited for individuals wanting to improve their credentials safety.  

Features Suggested by Hackaday Readers
--------------------------------------
- Have multiple smart cards for multiple users on one Mooltipass  
- Generate a QR code instead of sending the password via HID  
- Have a hole on the device's side to pour resin  
- buy a PID from MCS electronics  
- implement OTP  

Contact us
----------
You may contact the development team via <a href="https://groups.google.com/forum/?hl=en#!forum/Mooltipass">the official Google group</a>.
