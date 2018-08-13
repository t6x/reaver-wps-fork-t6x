/*
 * Functions to generate default WPS PIN with BSSID and WPS Device Data of AP
 * 
 * Reference: https://3wifi.stascorp.com/wpspin
 *            http://standards-oui.ieee.org/oui.txt
 *            https://code.wireshark.org/review/gitweb?p=wireshark.git;a=blob_plain;f=manuf;hb=HEAD
 */

#ifndef PINGEN_H
#define PINGEN_H

#include <ctype.h>
#include <sys/time.h>

#include "defs.h"
#include "globule.h"
#include "pins.h"
#include "misc.h"
#include "80211.h"
#include "builder.h"
#include "send.h"

#include "libwps/libwps.h"
#include "utils/vendor.h"

#define STATIC_PIN_SIZE		21
#define PROBE_WAIT_TIME		6

/**
 * struct pingen_function - WPS Device Data
 * @(*pin)(): Pointer to function
 * @add: Aditional parameter to function (negative main not use it)
 * @serial: Use ou not Serial Number parameter (0|1)
 * @priority: Priority to order function running
 * @oui: List of oui associated with the function
 * @vendor: List of vendors associated with the function
 * @model: List of models associated with the function
 * @name: Name of algorithm
 */
struct pingen_function {
	int (*pin)();
	int add;
	unsigned char serial;
	short int priority;
	char *oui;
	char *vendor;
	char *model;
	char *name;
};

int pingen_byte(char *mac, int last);

int pingen_reverse_byte(char *mac, int last);

int pingen_reverse_nibble(char *mac, int last);

int pingen_reverse_bit(char *mac, int last);

int pingen_asus(char *mac);

int pingen_dlink(char *mac, int add);

int pingen_airocon(char *mac);

int pingen_inv_nic(char *mac);

int pingen_nicx(char *mac, int x);

int pingen_oui_nic(char *mac, int opt);

int pingen_sn(char *mac, char *serial, int *init_bx, int add);

int pingen_belkin(char *mac, char *serial);

int pingen_vodafone_easybox(char *mac, char *serial);

int pingen_livebox_arcadyan(char *mac, char *serial);

int pingen_static(char *mac, int i);

int get_k1_key_index(int value);

int get_k2_key_index(int value);

void insert_pingen_p1(int *pins, int len);

void insert_pingen_p2(int *pins, int len);

int *build_pingen(int *len);

struct libwps_data *pingen_get_ap_info();

struct libwps_data *pingen_parse_wps_settings(const u_char *packet, struct pcap_pkthdr *header);

void pingen_send_probe_request(unsigned char *bssid, char *essid);

#endif
