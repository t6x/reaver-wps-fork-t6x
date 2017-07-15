# Overview

**Reaver** has been designed to be a robust and practical attack against **Wi-Fi Protected Setup (WPS)** registrar PINs in order to **recover WPA/WPA2 passphrases**. It has been tested against a wide variety of access points and WPS implementations.

The **original** Reaver implements a **online brute force attack** against, as described in [http://sviehb.files.wordpress.com/2011/12/viehboeck_wps.pdf](http://sviehb.files.wordpress.com/2011/12/viehboeck_wps.pdf). **Reaver-wps-fork-t6x** version **1.6.1** (and superior) is a **community forked version**, which has included **various bug fixes** and additional attack method (the **offline Pixie Dust** attack).

**Depending on the target's Access Point (AP)**, to recover the plain text WPA/WPA2 passphrase the **average** amount of time for the transitional **online brute force** method is **between 4-10 hours**. In practice, it will generally take half this time to guess the correct WPS pin and recover the passphrase.
When using the **offline attack**, **if** the AP is vulnerable, it may take only a matter of **seconds to minutes**.

* The original Reaver (v1.4) can be found here: [https://gitlab.com/billhibadb/reaver-wps.git](https://gitlab.com/billhibadb/reaver-wps.git).
* The discontinued reaver-wps-fork-t6x "_community edition_" (version **1.5.3**, includes the "Pixie Dust" attack) is now our "old-master" branch: [https://github.com/t6x/reaver-wps-fork-t6x/tree/master-old](https://github.com/t6x/reaver-wps-fork-t6x/tree/master-old).
* The latest revision of reaver version **1.6.x** "_community edition_" (includes the Pixie Dust attack) is in the main branch from our repository: [https://github.com/t6x/reaver-wps-fork-t6x](https://github.com/t6x/reaver-wps-fork-t6x).
* More information about the Pixie Dust attack (including **which APs are vulnerable**) can be found here:                  [https://github.com/wiire/pixiewps](https://github.com/wiire/pixiewps), 
[https://forums.kali.org/showthread.php?24286-WPS-Pixie-Dust-Attack-(Offline-WPS-Attack)](https://forums.kali.org/showthread.php?24286-WPS-Pixie-Dust-Attack-(Offline-WPS-Attack)) &                                                                    [https://docs.google.com/spreadsheets/d/1tSlbqVQ59kGn8hgmwcPTHUECQ3o9YhXR91A_p7Nnj5Y/edit?usp=sharing](https://docs.google.com/spreadsheets/d/1tSlbqVQ59kGn8hgmwcPTHUECQ3o9YhXR91A_p7Nnj5Y/edit?usp=sharing)

- - -

# Requirements

```
apt-get -y install build-essential libpcap-dev aircrack-ng pixiewps
```
_The example uses [Kali Linux](https://www.kali.org/) as the Operating System (OS) as `pixiewps` is included._

You must already have Wiire's pixiewps installed to be able to perform a pixie dust attack.
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

# Reaver Usage

```
Reaver v1.6.1 WiFi Protected Setup Attack Tool
Copyright (c) 2011, Tactical Network Solutions, Craig Heffner <cheffner@tacnetsol.com>

Required Arguments:
	-i, --interface=<wlan>          Name of the monitor-mode interface to use
	-b, --bssid=<mac>               BSSID of the target AP

Optional Arguments:
	-m, --mac=<mac>                 MAC of the host system
	-e, --essid=<ssid>              ESSID of the target AP
	-c, --channel=<channel>         Set the 802.11 channel for the interface (implies -f)
	-o, --out-file=<file>           Send output to a log file [stdout]
	-s, --session=<file>            Restore a previous session file
	-C, --exec=<command>            Execute the supplied command upon successful pin recovery
	-D, --daemonize                 Daemonize reaver
	-f, --fixed                     Disable channel hopping
	-5, --5ghz                      Use 5GHz 802.11 channels
	-v, --verbose                   Display non-critical warnings (-vv for more)
	-q, --quiet                     Only display critical messages
	-h, --help                      Show help

Advanced Options:
	-p, --pin=<wps pin>             Use the specified pin (may be arbitrary string or 4/8 digit WPS pin)
	-d, --delay=<seconds>           Set the delay between pin attempts [1]
	-l, --lock-delay=<seconds>      Set the time to wait if the AP locks WPS pin attempts [60]
	-g, --max-attempts=<num>        Quit after num pin attempts
	-x, --fail-wait=<seconds>       Set the time to sleep after 10 unexpected failures [0]
	-r, --recurring-delay=<x:y>     Sleep for y seconds every x pin attempts
	-t, --timeout=<seconds>         Set the receive timeout period [5]
	-T, --m57-timeout=<seconds>     Set the M5/M7 timeout period [0.20]
	-A, --no-associate              Do not associate with the AP (association must be done by another application)
	-N, --no-nacks                  Do not send NACK messages when out of order packets are received
	-S, --dh-small                  Use small DH keys to improve crack speed
	-L, --ignore-locks              Ignore locked state reported by the target AP
	-E, --eap-terminate             Terminate each WPS session with an EAP FAIL packet
	-n, --nack                      Target AP always sends a NACK [Auto]
	-w, --win7                      Mimic a Windows 7 registrar [False]
	-K, --pixie-dust                Run pixiedust attack
	-Z                              Run pixiedust attack

Example:
	./reaver -i wlan0mon -b 00:90:4C:C1:AC:21 -vv

```

 
## -K and-or -Z  // --pixie-dust (in reaver)

The `-K` and `-Z` option perform the offline attack, Pixie Dust _(`pixiewps`)_, by automatically passing the **PKE**, **PKR**, **E-Hash1**, **E-Hash2**, **E-Nonce** and **Authkey** variables. `pixiewps` will then try to attack **Ralink**, **Broadcom** and **Realtek** detected chipset.
**Special note**: If you are attacking a **Realtek AP**, **do NOT** use small DH Keys (`-S`) option.
User will have to execute reaver with the cracked PIN (option -p) to get the WPA pass-phrase. 
This is a temporary solution and an option to do a full attack will be implemented soon


## -a // --all  (in wash) 

The option `-a` of Wash will list all access points, including those without WPS enabled.

  
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
