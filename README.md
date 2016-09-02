Mooltipass Project Repository
=============================
<p align="center">
  <img src="https://raw.githubusercontent.com/limpkin/mooltipass/master/pictures/mini/prototypes/mini_alum_final_small_cropped.JPG" alt="Mooltipass first prototype"/>
</p>
This is the GitHub repository dedicated to the Mooltipass device family. It contains all the resources that were used and made for this community driven product line since the project was first started back in December 2013.  
You may order a Mooltipass device <b><a href="https://www.tindie.com/products/limpkin/mooltipass-offline-password-keeper/">on our Tindie store</a></b> and find the hidden link in <b><a href="https://www.themooltipass.com">our official web page</a></b> to be notified of the upcoming Mooltipass Mini crowdfunding campaign launch.  

What is the Mooltipass Project?
-------------------------------
With time, logins and passwords have become critical elements we need to remember to access the different websites and services we use daily. If we want to achieve good security, each of these credential sets should be unique.  
We therefore created the Mooltipass, a physical password keeper that remembers and encrypts your credentials so you don't have to. With this device, you can generate and safely store long and complex passwords. A personal PIN locked smartcard allows the decryption of your credentials and ensures that only you have access to them. Simply visit a website and the device will ask for your confirmation to enter your credentials when login is required. The Mooltipass is a standalone device connected through USB, is completely driver-less and is compatible with all major operating systems on PCs, Macs and Smartphones.  
It is extremely simple to use our device:  
- Plug the Mooltipass to your computer/tablet/phone, no driver installation required.  
- Insert your smartcard, unlock it with your PIN. Without the PIN, the card is useless.  
- Visit the website that needs a login. If using our browser plugin the Mooltipass asks your permission to send the stored name and password, or asks you to save/generate new credentials if you are logging in for the first time.  
- If you are not using the browser plugin or logging in to something other than a web browser, you can manually tell MP to send the correct name and password. It will type it in for you, just like a keyboard. In this way it can be used anywhere.    

Advantages over software-based solutions
----------------------------------------
A software-based password keeper uses a passphrase to decrypt a credentials database located inside a device (computer, smartphone...). As at a given moment your passphrase and your database are stored inside your device's memory, a malicious program with access to both elements could <b><a href="http://thehackernews.com/2014/11/new-citadel-trojan-targets-your.html">compromise all your passwords at once</a></b>. In some cases, security flaws on software-based solutions can lead to entire unencrypted databases beeing extracted by <b><a href="http://www.theregister.co.uk/2016/07/27/zero_day_hole_can_pwn_millions_of_lastpass_users_who_visit_a_site">visiting a simple website</a></b>.  
We therefore offer the following advantages:  
- **stronger security**: we reduce the number of attack vectors by basically having our device type your passwords for you. 
- a **non-proprietary device**: as our product is open anyone can develop new tools for it. There will never be fees for the services we offer.  
- an **open-source platform**: all our source code can be viewed, allowing you to check that your credentials are only kept inside the device and not leaked to the outside  
- a **trusted platform**: as only our tested source code is running on the Mooltipass, there can't be any viruses or malicious programs compromising your stored credentials  

The Platform
------------
The Mooltipass is composed of the main device shown above and a smartcard.  
On the device are stored your AES-256 encrypted credentials. The smartcard is a read protected memory that needs a PIN code to unlock its contents (AES-256 key + a few websites credentials). As with your chip and pin card, too many tries will permanently lock the smart card.

The Firmware
------------
**We want the device to be as simple as possible.** Ideally, the end user shouldn't have to spend more than a few seconds to use its basic functionalities.  
A browser extension runs on the user's computer and sends the current website to the Mooltipass. When the user has to login, the Mooltipass will light up and ask for confirmation to enter the credentials.

Data Safety
-----------
Safety is a primary concern for the Mooltipass development team, which is why we offer several **secure** ways to backup your credentials.  
The smart card containing the AES key used for encrypting the passwords can be **cloned** using the Mooltipass, copying its PIN code as well. The encrypted credentials stored in the Mooltipass internal flash can be **exported** to the official Mooltipass website or simply somewhere on your local computer.

A Brief History of Security Flaws and Breaches
----------------------------------------------
We often think that the devices and websites we use are exempt of security flaws. We compiled a brief list of major security breaches and vulnerabilities found during the last few months:  
- 06/2016 <a href="http://www.independent.co.uk/life-style/gadgets-and-tech/news/ios-933-iphone-users-urged-to-update-after-apple-fixes-huge-password-vulnerability-a7149851.html">iOS bug allows attackers to gain full remote access to an iPhone by sending a simple iMesssage</a>  
- 06/2016 <a href="http://www.theregister.co.uk/2016/06/01/teamviewer_mass_breach_report/">Teamviewer hacked, users' Paypal account drained</a>  
- 06/2016 <a href="https://yro.slashdot.org/story/16/06/16/2035205/github-presses-big-red-password-reset-button-after-third-party-breach">GitHub Presses Big Red Password Reset Button After Third-Party Breach </a>  
- 05/2016 <a href="https://it.slashdot.org/story/16/05/27/1845202/hackers-claim-to-have-427-million-myspace-passwords">Hackers Claim to Have 427 Million Myspace Passwords</a>  
- 05/2016 <a href="https://it.slashdot.org/story/16/05/30/1227252/hackers-stole-65-million-passwords-from-tumblr">Hackers Stole 65 Million Passwords From Tumblr</a>  
- 06/2016 <a href="https://techcrunch.com/2016/06/08/twitter-hack/">Passwords for 32M Twitter accounts may have been hacked and leaked</a>  
- 06/2016 <a href="https://bits-please.blogspot.fr/2016/06/extracting-qualcomms-keymaster-keys.html">Android Full Disk Encryption Can Be Broken</a>  
- 07/2016 <a href="https://labs.bitdefender.com/2016/07/new-mac-backdoor-nukes-os-x-systems/">New Backdoor Allows Full Access to Mac Systems</a>  
- 08/2016 <a href="http://arstechnica.com/security/2016/08/qualcomm-chip-flaws-expose-900-million-android-devices/">Qualcomm security flaw allows a malicious app to gain root access</a>  
- 06/2016 <a href="https://it.slashdot.org/story/16/08/13/0325204/new-cache-attack-can-monitor-keystrokes-on-android-phones">New Cache Attack Can Monitor Keystrokes On Android Phones </a>  
- 03/2016 <a href="https://bogner.sh/2016/03/mitm-attack-against-keepass-2s-update-check/">MitM Attack against KeePass 2’s Update Check</a>  
- 07/2016 <a href="https://it.slashdot.org/story/16/07/08/2011205/apple-devices-held-for-ransom-rumors-claim-40m-icloud-accounts-hacked-apple-related-forums-compromised">Apple Devices Held For Ransom, Rumors Claim 40M iCloud Accounts Hacked; Apple-Related Forums Compromised</a>  
- 06/2016 <a href="http://www.theregister.co.uk/2016/07/27/zero_day_hole_can_pwn_millions_of_lastpass_users_who_visit_a_site/">Lastpass: remote 'complete account compromise' possible</a>  
- 08/2016 <a href="http://mashable.com/2016/08/26/iphone-malware-platform-secure">iPhone malware that steals your data is a reminder no platform is ever safe</a>  

Frequently Asked Questions
--------------------------

**Is your solution better than a piece of paper?**  
A piece of paper contains passwords that can easily be read when you are not paying attention to it. The Mooltipass stores encrypted passwords that can only be read when providing your PIN code.  

**If I only need to remember a PIN code, does it mean the Mooltipass is not safe?**  
Not at all, as the Mooltipass system is exactly like your chip and pin card: 3 false tries will permanently block the smart card and make credential decryption impossible.  

**Why do I need different passwords for different websites?**  
Websites are compromised on a daily basis. If you are using the same password for different websites, one attacker could use a password he discovered on all of them.

**Why do you need an OLED screen?**  
An offline password keeper needs to provide a way to prevent *impersonation*. The user has to check that the website/service he's approving the credential sending for is the same that the website/service he's currently visiting/using, as a malicious program could emit forged requests. Moreover, having a display allows the user to operate the Mooltipass without the browser plugin using our dedicated touch interface.  
  
**Why are you using both a smart card and a main Mooltipass device?**  
There are many reasons, the main one being that it is much easier to carry a smart card around than any other object. This smart card is a secure element that contains your credentials' encryption key, it is cheap and may be cloned without compromising the system security.   
  
**What if I lose my smartcard?**  
Our device is shipped with two smartcards, so you can keep your copy somewhere safe. The Mooltipass therefore allows the user to clone his smartcard as many times as he wants, provided that the card PIN is correctly entered.  

**Can the smartcard be used with multiple Mooltipass?**  
You can synchronize your credentials between multiple devices. This allows you to have one Mooltipass at work and one at home.  
  
**What if I lose my Mooltipass device?**  
Your encrypted credentials can be exported to either your computer or the Mooltipass official website. If you lose your device, you may purchase another one and restore your credentials.  

**Are you sure about your encryption implementation?**  
The AES-256 used in the Mooltipass has been compared again standard Nessie test vectors for correctness. Moreover, our security chain has been checked by qualified individuals.  
  
**Can I use it on Windows/Linux/Mac?**  
Yes, as no drivers are required to use the Mooltipass. It is recognized as a standard USB keyboard that will enter passwords for you.  
**Can I use on my computer/laptop/phone/tablet...?**  
Most (if not all) devices (including smart phones and tablet PCs) include a USB host capable port. The Mooltipass will work with all of them.  

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
- Pierre C. (GUI, encryption implementation supervision, pen testing) - France
- Bjorn W. (GUI, graphics, wise man, project advisor) - Canada
- Raoul H. (cross platform daemon, browser extensions - France
- Henryk P. (encryption implementation supervision) - Unknown
- Olivier G. (mecanics, project advisor) - Switzerland
- Josh W. (mechanics, plugin, fw supervision) - USA
- Eric E. (schematics & layout verification) - USA
- Mike N. (flash storage, node management) - USA
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
- Jesse V. - USA
- Kyle W. - USA
- Erik M. - USA
- Josh W. - USA
- Don F. - USA

Device History
--------------
1) Mooltipass Standard  
You may find all the articles detailing the Mooltipass Standard different life steps using <a href="http://hackaday.com/tag/developed-on-hackaday/">this link</a>.  
Our <a href="https://www.indiegogo.com/projects/mooltipass-open-source-offline-password-keeper">crowdfunding campaign</a> achieved its goal by raising around $125k in December 2014.

