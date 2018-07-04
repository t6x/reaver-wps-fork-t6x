/*
 * Functions to generate default WPS PIN with BSSID and Serial Number of AP
 * 
 * Reference: https://3wifi.stascorp.com/wpspin
 */

#ifndef MACPIN_H
#define MACPIN_H

#include <defs.h>
#include <misc.h>

#define STATIC_PIN_SIZE		20
#define MAC_PIN_SIZE		30

int mac_byte_pin(char *mac, int last);

int mac_reverse_byte_pin(char *mac, int last);

int mac_reverse_nibble_pin(char *mac, int last);

int mac_reverse_bit_pin(char *mac, int last);

int mac_asus_pin(char *mac);

int mac_dlink_pin(char *mac, int add);

int mac_airocon_pin(char *mac);

int mac_inv_nic_pin(char *mac);

int mac_nicx_pin(char *mac, int x);

int mac_oui_add_nic_pin(char *mac);

int mac_oui_sub_nic_pin(char *mac);

int mac_oui_xor_nic_pin(char *mac);

int mac_sn_pin(char *mac, char *serial, int *init_bx, int add);

int mac_belkin_pin(char *mac, char *serial);

int mac_vodafone_easybox_pin(char *mac, char *serial);

int mac_livebox_arcadyan_pin(char *mac, char *serial);

int *build_mac_pins(char *mac, int *len);

int *build_mac_sn_pins(char *mac, char *serial, int *len);

#endif
