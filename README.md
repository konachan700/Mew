## Mew - hardware password manager
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=3LJA6GYKRW6PU&lc=US&item_name=MeW%20HPM%20Donate&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)

<img src="https://raw.githubusercontent.com/konachan700/konachan700.github.io/master/q2.png"> 

*   [Forum](https://forum.mewhpm.com)
*   [Wiki](https://wiki.mewhpm.com)
*   [Code repo (github)](https://github.com/konachan700/Mew)
*   [Code repo (gitlab mirror)](https://gitlab.com/mewhpm/firmware)
*   [Hardware](https://easyeda.com/konachan.700/MeW_Pro_v2.1)

##### • What is it?
It's a small USB device for hold a your passwords, private keys and cryptowallets.

*   Easy to use.
*   Fully open sourced.
*   No drivers or propietary software required.
*   Highly protected for stolen critical data.

<img src="https://raw.githubusercontent.com/konachan700/konachan700.github.io/master/q1.png"> 

##### • How it's work?

Connect MeW to PC or mobile device. Pair MeW with your smartphone via bluetooth. Install a MeW HPM app from Google Play or Appstore. Use it.  

_Demo video will be here a bit later :3_

12 passwords, what you use more often, you can input directly from device. Other passwords could be entered by MeW HPM app.  

_Demo video will be here a bit later :3_

You can exchanging public keys with your friends directly via bluetooth.

Technically, it is a HID-keyboard.

Passwords are not stored in device, all passwords was generate on-tne-fly in depends of entered PIN-code and internal master key. Nobody and nothing can't theft your passwords database, because it isn't exist. If anybody will theft the device, he can't bruteforce it - for each PIN-code device generate different passwords, and, if you set this option, fake logins will be show.  

Your private keys and wallets, what was stored on the device, was encrypted by use AES-256. Encryption key is generated from master key and pin-code on-the-fly. Master key has been stored internally and can't be extracted, of course, if you are set the maximum security level.

##### • I was lost my MeW? How I can repair my passwords?

When you are starting your MeW at first time with normal security level, device key will shown. Save device key at secure place. MeW has a cloud backup for encrypted master key. For recovery your passwords on new device you should be enter a device key in the application, and enter you PIN-code in MeW.

In the high security level cloud backup will disabled, because encrypted master key in this level can't be extracted from device. If you was lost you MeW or forget a PIN-code, you lost all you passwords.

##### • Gallery
<img src="https://raw.githubusercontent.com/konachan700/konachan700.github.io/master/IMG_0865.JPG"> 
<img src="https://raw.githubusercontent.com/konachan700/konachan700.github.io/master/IMG_0869.JPG">
<img src="https://raw.githubusercontent.com/konachan700/konachan700.github.io/master/case1.jpg"> 
