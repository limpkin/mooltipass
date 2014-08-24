Mooltipass Project Repository
=============================
<p align="center">
  <img src="https://raw.githubusercontent.com/limpkin/mooltipass/master/pictures/prototypes/holder/holder_with_mooltipass_small.JPG" alt="Mooltipass first prototype"/>
</p>
This is the GitHub repository dedicated to the Mooltipass device. It contains all the resources that were used and generated for this community driven product since this project was first started back in December 2013. You may find all the Hackaday articles detailing its different steps using <a href="http://hackaday.com/tag/developed-on-hackaday/">this link</a>.


What is the Mooltipass Project?
-------------------------------
The Mooltipass is an **offline** password keeper, it remembers and encrypts your passwords so you don't have to.  
With this device, you can generate and safely store different long and complex passwords for all the websites you use daily. A personal PIN locked smartcard allows the decryption of your credentials and ensures that only **you** has access to them. The Mooltipass is designed to be as small as possible to fit in your pocket. Simply visit a website and the device will ask for your confirmation to enter your credentials when login is required. The Mooltipass is a standalone device connected through USB, is completely driver-less and is compatible with all major operating systems on PCs, Macs and Smartphones. 

Advantages over software-based solutions
----------------------------------------
A software-based password keeper uses a passphrase to decrypt a credentials database located inside a device (computer, smartphone...). As at a given moment your passphrase and your database are stored inside your device's memory, a malicious program with access to both elements could **compromise all your passwords at once**.  
We therefore offer the following advantages:  
- **stronger security**: we reduce the number of attack vectors by basically having our device type your passwords for you. 
- a **non-proprietary device**: as our product is open anyone can develop new tools for it. There will never be fees for the services we offer.  
- an **open-source platform**: all our source code can be viewed, allowing you to check that your credentials are only kept inside the device and not leaked to the outside  
- a **trusted platform**: as only our tested source code is running on the Mooltipass, there can't be any viruses or malicious programs compromising your stored credentials  

The Platform
------------
The Mooltipass is composed of the main device shown above and a smartcard.  
On the device are stored your AES-256 encrypted credentials. The smartcard is a read protected EEPROM that needs a PIN code to unlock its contents (AES-256 key + a few websites credentials). As with your credit card, too many tries will permanently lock the smart card.

Arduino Compatibility
---------------------
Our complete platform is **Arduino compatible**.  
By simply cutting thin parts out of the ABS case you can get access to Arduino headers and therefore transform our device to a standard Arduino platform with an OLED screen and a touch interface.

The Firmware
------------
**We want the device to be as simple as possible.** Ideally, the end user shouldn't have to spend more than a few seconds to use its basic functionalities.  
A browser extension runs on the user's computer and sends the current website to the Mooltipass. When the user has to login, the Mooltipass will light up and ask for confirmation to enter the credentials.

Data Safety
-----------
Safety is a primary concern for the Mooltipass development team, which is why we offer several **secure** ways to backup your credentials.  
The smart card containing the AES key used for encrypting the passwords can be **cloned** using the Mooltipass, copying its PIN code as well. The encrypted credentials stored in the Mooltipass internal flash can be **exported** to the official Mooltipass website or simply somewhere on your local computer. A very convenient functionally suggested by one of Hackaday's readers is also implemented: the ability to generate hashed answers for websites' security questions in case credentials are lost.

Frequently Asked Questions
--------------------------
**How expensive will the Mooltipass be?**  
Price was one of our main constraints when designing the Mooltipass. It's still too early to tell but our current goal is a $70 price. In the meantime, you can have a look at our Bill of Materials located in the kicad folder to get an overall idea of the current costs.  

**Is your solution better than a piece of paper?**  
A piece of paper contains passwords that can easily be read when you are not paying attention to it. The Mooltipass stores encrypted passwords that can only be "read" when providing your PIN code.  

**If I only need to remember a PIN code, does it mean the Mooltipass is not safe?**  
Not at all, as the Mooltipass system is exactly like your bank card: 3 false tries will permanently block the smart card and make credential decryption impossible.  

**Why do I need different passwords for different websites?**  
Websites are compromised on a daily basis. If you are using the same password for different websites, one attacker could use a password he discovered on all of them.
  
**Why not make the device very tiny?**  
Size is a compromise between transportability and user friendliness.  
As the Mooltipass is intended for many different users we decided to opt for a normal-sized OLED screen providing a good readability and therefore a better user experience. The device also includes Arduino headers that will allow any Arduino shield to be connected to it. Hence, we made the Mooltipass as small as possible while keeping its great features.  

**Why do you need an OLED screen?**  
An offline password keeper needs to provide a way to prevent *impersonation*. The user has to check that the website/service he's approving the credential sending for is the same that the website/service he's currently visiting/using, as a malicious program could emit forged requests. Moreover, having a display allows the user to operate the Mooltipass without the browser plugin using our dedicated touch interface.  
  
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

**Are you planning to make a wireless version?**  
The Mooltipass isn't wireless to skip the added costs of a lithium-ion battery and a wireless interface. Customer survey also let us know that having a USB cable wasn't a problem for most use cases.  

**How are the credentials sent to the computer?**  
The Mooltipass is enumerated as a composite HID keyboard / HID proprietary device. The credentials are sent over the HID proprietary channel when using the browser plugin and over the keyboard channel when using the Mooltipass through its touch interface.

**Is it still possible to sniff the passwords sent over HID?**  
In theory yes. As mentioned in our project description the Mooltipass aims at reducing the number of attack vectors to a minimum: the device basically types your password as if you were doing it yourself. Perfect security could only be achieved by sharing dedicated secrets with every possible service and website... which is practically impossible to do.

**If I can export my encrypted credentials, does this mean someone could crack them?**  
In short, no. We are using AES-256 encryption in CTR mode, bruteforcing the encrypted credentials takes more than fifty years.  

**If it is open source, does it mean it is less secure?**  
Not at all. Having our code open source allows everyone to check our security implementation, which actually leads to a better code quality and more trust from our final users.

Contact us
----------
You may contact the development team via <a href="https://groups.google.com/forum/?hl=en#!forum/Mooltipass">the official Google group</a>.

Thanks and Acknowledgement
--------------------------
None of this would have been possible without the help of many people located all over the globe. Here is a non exhaustive list:
- Darran H. (graphics, plugin, comms, general development and more) - New Zealand
- Henryk P. (encryption implementation supervision) - Unknown
- Hackaday (for their resources and believing in us) - USA
- Olivier G. (mecanics, project advisor) - Switzerland
- Josh W. (mechanics, plugin, fw supervision) - USA
- Eric E. (schematics & layout verification) - USA
- Mike N. (flash storage, node management) - USA
- Bjorn W. (wise man, project advisor) - Canada
- Charles E. (legal, project advisor) - USA
- Miguel A. (AES encryption, RNG) - Spain
- Tom V. (part of USB) - South Africa
- Erik M. (IRC, general help) - USA
- Mikael A. (GUI, plugin) - Sweden
- Hans N. (general help) - Denmark

Awesome people who submitted their mecanical designs for a HaD readership vote: Louis, Joe, Andy, Olivier, Josh...

Finally our awesome beta testers who paid for their (expensive) beta units and provided us with invaluable feedback:
- Marcelo C. - Portugal
- Christopher H. - USA
- Dimitrios - Sweden
- Justin H. - Canada
- Pontus L. - Sweden
- Julien M. - France
- Donald P. - Canada
- Carlos S. - Spain
- Charles E. - USA
- Andrew B. - USA
- Joshua H. - USA
- Warren B. - USA
- David E. - USA
- Keith S. - USA
- Jason S. - USA
- Kyle W. - USA
- Erik M. - USA
- Josh W. - USA
- Don F. - USA
