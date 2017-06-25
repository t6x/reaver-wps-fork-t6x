# Overview

**Reaver** has been designed to be a robust and practical attack against **Wi-Fi Protected Setup (WPS)** registrar PINs in order to **recover WPA/WPA2 passphrases**. It has been tested against a wide variety of access points and WPS implementations.

The **original** Reaver implements a **online brute force attack** against, as described in [http://sviehb.files.wordpress.com/2011/12/viehboeck_wps.pdf](http://sviehb.files.wordpress.com/2011/12/viehboeck_wps.pdf).
**reaver-wps-fork-t6x** version **1.6b** is a **community forked version**, which has included **various bug fixes** and additional attack method (the **offline Pixie Dust** attack).

**Depending on the target's Access Point (AP)**, to recover the plain text WPA/WPA2 passphrase the **average** amount of time for the transitional **online brute force** method is **between 4-10 hours**. In practice, it will generally take half this time to guess the correct WPS pin and recover the passphrase.
When using the **offline attack**, **if** the AP is vulnerable, it may take only a matter of **seconds to minutes**.

* The original Reaver (v1.4) can be found here: [https://code.google.com/p/reaver-wps/](https://code.google.com/p/reaver-wps/).
* The discontinued reaver-wps-fork-t6x community edition (which includes the Pixie Dust attack. v1.5.3) is now the old-master branch from this repository
* reaver-wps-fork-t6x community edition of Reaver version 1.6b (which includes the Pixie Dust attack): [https://github.com/t6x/reaver-wps-fork-t6x](https://github.com/t6x/reaver-wps-fork-t6x).
* For more information about the Pixie Dust attack (including **which APs are vulnerable**) can be found here:                  [https://github.com/wiire/pixiewps](https://github.com/wiire/pixiewps), 
[https://forums.kali.org/showthread.php?24286-WPS-Pixie-Dust-Attack-(Offline-WPS-Attack)](https://forums.kali.org/showthread.php?24286-WPS-Pixie-Dust-Attack-(Offline-WPS-Attack)) &                                                                    [https://docs.google.com/spreadsheets/d/1tSlbqVQ59kGn8hgmwcPTHUECQ3o9YhXR91A_p7Nnj5Y/edit?usp=sharing](https://docs.google.com/spreadsheets/d/1tSlbqVQ59kGn8hgmwcPTHUECQ3o9YhXR91A_p7Nnj5Y/edit?usp=sharing)

- - -

# Requirements

```
apt-get -y install build-essential libpcap-dev aircrack-ng pixiewps
```
_The example uses [Kali Linux](https://www.kali.org/) as the Operating System (OS) as `pixiewps` is included._

You **must** already have Wiire's Pixiewps installed.
The latest version can be found here: [https://github.com/wiire/pixiewps](https://github.com/wiire/pixiewps).

- - -

# Setup

**Download**

`git clone https://github.com/t6x/reaver-wps-fork-t6x`

or

`wget https://github.com/t6x/reaver-wps-fork-t6x/archive/master.zip && unzip master.zip`

**Build**

```bash
cd reaver-wps-fork-t6x*/
cd src/
./configure
make
```

**Install**

`sudo make install`

- - -

# About Reaver 1.6b Options 

 Please notice that work is in progress and the situation will progress soon, stay tuned! ;)

## -K and-or -Z  // --pixie-dust (in reaver)

The `-K` and `-Z` option perform the offline attack, Pixie Dust _(`pixiewps`)_, by automatically passing the **PKE**, **PKR**, **E-Hash1**, **E-Hash2**, **E-Nonce** and **Authkey** variables. `pixiewps` will then try to attack **Ralink**, **Broadcom** and **Realtek** detected chipset.
**Special note**: If you are attacking a **Realtek AP**, **do NOT** use small DH Keys (`-S`) option.
User will have to execute reaver with the cracked PIN (option -p) to get the WPA pass-phrase. 
This is a temporary solution and an option to do a full attack will be implemented soon


## -a // --all  (in wash) 

The option `-a` of Wash will list all access points, including those without WPS enabled.

## Deprecated and temporary left behind options

* **- n** (reaver): Automatically enabled, no need to invocate it. 
* **- W** (reaver): Temporary left behind. Integration of the default PIN generators was unstable, leading to many warnings at compilation time. It was also an issue to use a PIN attempt (risk of AP rating limit) in order to get a BSSID and an ESSID. For the moment PIN generation has to be done externally using the scripts provided in "doc".  
* **- a** (reaver): This option was the only option which required sqlite3 adding an extra dependency. It was only designed for automation scripts and this task (execute the last reaver command again) can be easily done internally by the script that calls reaver  
* **- p1** and **-p2** (reaver): Too much warnings and bugs.  
* **-H** (reaver): There is a need to find a way to perform it more cleanly, work is in progress.  
* **- vvv** (reaver): The highest level of verbose is temporary removed for the same reason.  
* **- g** (wash): Option was broken in latest release and need to be seriously rethought.  

## Options repaired/solved issues
  
  Issues with -g and -p (and their crossed usage) are left behind.  
Code is much more clean, robust and has less dependencies.  
We know that it looks like at first time as a regression but it is not!  
We were stuck with issues for years due the dificulty of the task and the lack of global direction.
  Now we have a much healthier base and it will be worth it.  
  
# Acknowledgements

## Contribution
Creator of reaver-wps-fork-t6x "community edition": 
`t6x`  
  
Main developer since version 1.6b: 
`rofl0r`  

Modifications made by:
`t6_x`, `DataHead`, `Soxrok2212`, `Wiire`, `AAnarchYY`, `kib0rg`, `KokoSoft`, `rofl0r`, `horrorho`, `binarymaster`, `Ǹotaz`  

Some ideas made by:
`nuroo`, `kcdtv`

Bug fixes made by:
`alxchk`, `USUARIONUEVO`, `ldm314`, `vk496`, `falsovsky`, `rofl0r`, `xhebox`

## Special Thanks

* `Soxrok2212` for all work done to help in the development of tools
* `Wiire` for developing Pixiewps
* `Craig Heffner` for creating Reaver and for the creation of default pin generators (D-Link, Belkin) - http://www.devttys0.com/
* `Dominique Bongard` for discovering the Pixie Dust attack.
