## Mew - hardware password manager
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=3LJA6GYKRW6PU&lc=US&item_name=MeW%20HPM%20Donate&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)

<img src="https://raw.githubusercontent.com/konachan700/konachan700.github.io/master/q2.png"> 

*   [Forum](https://forum.mewhpm.com)
*   [Wiki](https://wiki.mewhpm.com)
*   [Code repo (github)](https://github.com/konachan700/Mew)
*   [Code repo (gitlab mirror)](https://gitlab.com/mewhpm/firmware)
*   [Hardware](https://easyeda.com/konachan.700/MeW_Pro_v2.1)

##### • What is it?
It's a small USB device to hold your passwords, private keys and cryptowallets.

*   Easy to use.
*   Fully open sourced.
*   No drivers or propietary software required.
*   Highly protected for critical data.

<img src="https://raw.githubusercontent.com/konachan700/konachan700.github.io/master/q1.png"> 

##### • How does it work?

Connect MeW to a PC or mobile device. Pair MeW with your smartphone via bluetooth. Install the MeW HPM app from Google Play or Appstore. Use it.  

_Demo video will be here a bit later :3_

12 passwords, those you use more often, can be entered directly on the device. Other passwords could be entered by MeW HPM app.  

_Demo video will be here a bit later :3_

You can exchange public keys with your friends directly via bluetooth.

Technically, it is a HID-keyboard.

Passwords are not stored in the device, all passwords are generated on-the-fly depending on the entered PIN-code and internal master key. Nobody and nothing can steal your passwords database, because there is none. If anybody steals the device, he can't bruteforce it - for each PIN-code the device generate different passwords, and, if you set this option, fake logins will be shown.  

Your private keys and wallets, that are stored on the device, are encrypted with AES-256. Encryption key is generated from master key and pin-code on-the-fly. Master key has been stored internally and can't be extracted, of course, if you set the maximum security level.

##### • I lost my MeW. How can I get my passwords back?

When you are starting your MeW for the first time with normal security level, the device key will shown. Save the device key at a secure place. MeW has a cloud backup for encrypted master keys. For recovery of your passwords on a new device you have to enter a device key in the application, and enter you PIN-code in MeW.

In the high security mode cloud backup will disabled, because in this mode the encrypted master key i can't be extracted from the device. If you lose your MeW or forgot a PIN-code, you loose all you passwords.

##### • Gallery
<img src="https://raw.githubusercontent.com/konachan700/konachan700.github.io/master/IMG_0865.JPG"> 
<img src="https://raw.githubusercontent.com/konachan700/konachan700.github.io/master/IMG_0869.JPG">
<img src="https://raw.githubusercontent.com/konachan700/konachan700.github.io/master/case1.jpg"> 
