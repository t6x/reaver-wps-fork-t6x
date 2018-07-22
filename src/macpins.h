/*
 * Functions to generate default WPS PIN with BSSID and WPS Device Data of AP
 * 
 * Reference: https://3wifi.stascorp.com/wpspin
 *            http://standards-oui.ieee.org/oui.txt
 *            https://code.wireshark.org/review/gitweb?p=wireshark.git;a=blob_plain;f=manuf;hb=HEAD
 */

#ifndef MACPIN_H
#define MACPIN_H

#include <ctype.h>
#include "defs.h"
#include "misc.h"
#include "utils/vendor.h"

#define STATIC_PIN_SIZE		21
#define MAC_PIN_SIZE		30

/**
 * struct mac_pin_function - WPS Device Data
 * @(*pin)(): Pointer to function
 * @add: Aditional parameter to function (negative main not use it)
 * @serial: Use ou not Serial Number parameter (0|1)
 * @priority: Priority to order function running
 * @oui: List of oui associated with the function
 * @vendor: List of vendors associated with the function
 * @model: List of models associated with the function
 * @key: Key word of function
 */
struct mac_pin_function {
	int (*pin)();
	int add;
	unsigned char serial;
	short int priority;
	char *oui;
	char *vendor;
	char *model;
	char *key;
};

int mac_byte_pin(char *mac, int last);

int mac_reverse_byte_pin(char *mac, int last);

int mac_reverse_nibble_pin(char *mac, int last);

int mac_reverse_bit_pin(char *mac, int last);

int mac_asus_pin(char *mac);

int mac_dlink_pin(char *mac, int add);

int mac_airocon_pin(char *mac);

int mac_inv_nic_pin(char *mac);

int mac_nicx_pin(char *mac, int x);

int mac_oui_nic_pin(char *mac, int opt);

int mac_sn_pin(char *mac, char *serial, int *init_bx, int add);

int mac_belkin_pin(char *mac, char *serial);

int mac_vodafone_easybox_pin(char *mac, char *serial);

int mac_livebox_arcadyan_pin(char *mac, char *serial);

int *build_mac_pins(int *len);

#endif
