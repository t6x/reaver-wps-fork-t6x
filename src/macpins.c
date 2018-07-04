/*
 * Functions to generate default WPS PIN with BSSID and Serial Number of AP
 * 
 * Reference: https://3wifi.stascorp.com/wpspin
 */

#include "macpins.h"

/**
 * All MAC of AP (BSSID) without ':', e.g 0123456789AB,
 * pin is mod (%) by 10000000
 */

/**
 * Generate default pin with last 24-bit, 28-bit, 32-bit, 36-bit, 40-bit, 44-bit, 48-bit of mac
 * 
 * @params char* The BSSID without ':'
 * @params int   Last x hex character of BSSID to calculate
 * 
 * @return int   7-digit pin
 */
int mac_byte_pin(char *mac, int last)
{
	char key[13];
	long long int pin;
	int i;

	if (last > 12 || last < 6) {
		last = 6;
	}
	i = 12 - last;
	strncpy(key, mac+i, last);
	key[last] = '\0';
	pin = strtoll(key, NULL, 16);

	return pin % 10000000;
}

/**
 * Generate default pin with Reverse byte from last 24-bit, 32-bit, 48-bit of mac
 * 
 * @params char* The BSSID without ':'
 * @params int   Last x hex character of BSSID to calculate
 * 
 * @return int   7-digit pin
 */
int mac_reverse_byte_pin(char *mac, int last)
{
	char key[13];
	long long int pin;
	int i, j;

	if (last > 12 || last < 6) {
		last = 6;
	}
	for(i=0, j=12-2; i < last; i+=2, j-=2) {
		key[i] = mac[j];
		key[i+1] = mac[j+1];
	}
	key[last] = '\0';
	pin = strtoll(key, NULL, 16);

	return pin % 10000000;
}

/**
 * Generate default pin with Reverse nibble from last 24-bit, 32-bit, 48-bit of mac
 * 
 * @params char* The BSSID without ':'
 * @params int   Last x hex character of BSSID to calculate
 * 
 * @return int   7-digit pin
 */
int mac_reverse_nibble_pin(char *mac, int last)
{
	char key[13];
	long long int pin;
	int i, j;

	if (last > 12 || last < 6) {
		last = 6;
	}
	for(i=0, j=12-1; i < last; ++i, --j) {
		key[i] = mac[j];
	}
	key[last] = '\0';
	pin = strtoll(key, NULL, 16);

	return pin % 10000000;
}

/**
 * Generate default pin with Reverse bits from last 24-bit, 32-bit, 48-bit of mac
 * 
 * @params char* The BSSID without ':'
 * @params int   Last x hex character of BSSID to calculate
 * 
 * @return int   7-digit pin
 */
int mac_reverse_bit_pin(char *mac, int last)
{
	char key[64];
	int i, r;
	long long int pin, q;

	if (last > 12 || last < 6) {
		last = 6;
	}
	i = 12 - last;
	strncpy(key, mac+i, last);
	key[last] = '\0';
	pin = strtoll(key, NULL, 16);

	i=0;
	do {
		q = pin/2;
		r = pin%2;
		key[i++] = r+'0';
		pin = q;
	} while (q);
	while(i<last*4) {
		key[i++] = '0';
	}
	key[i] = '\0';
	pin = strtoll(key, NULL, 2);

	return pin % 10000000;
}

/**
 * Generate default pin for ASUS router
 * 
 * @params char* The BSSID without ':'
 * 
 * @return int   7-digit pin
 */
int mac_asus_pin(char *mac)
{
	char key[13];
	int b[6], pin[7];
	int i, j;

	key[2] = '\0';
	for (i=j=0; i<6; i++, j+=2) {
		key[0] = mac[j];
		key[1] = mac[j+1];
		b[i] = strtol(key, NULL, 16);
	}
	for (i = 0; i < 7; i++)	{
		pin[i] = (b[i % 6] + b[5]) % (10 - ((i + b[1] + b[2] + b[3] + b[4] + b[5]) % 7));
	}
	sprintf(key, "%d%d%d%d%d%d%d", pin[0], pin[1], pin[2], pin[3], pin[4], pin[5], pin[6]);

	return strtoll(key, NULL, 10) % 10000000;
}

/**
 * Generate default pin for D-Link router
 * 
 * @params char* The BSSID without ':'
 * @params int   The addicional number to calculate, default is 0 or 1
 * 
 * @return int   7-digit pin
 */
int mac_dlink_pin(char *mac, int add)
{
	int nic=0, pin=0;
	char buff[10];

	nic = strtol(strncpy(buff, mac+6, sizeof(buff)), NULL, 16);
	nic = nic + add;

	pin = nic ^ 0x55AA55;
	pin = pin ^ (((pin & 0x0F) << 4) +
		 ((pin & 0x0F) << 8) +
		 ((pin & 0x0F) << 12) +
		 ((pin & 0x0F) << 16) +
		 ((pin & 0x0F) << 20));
	pin = pin % 10000000;

	if (pin < 1000000)
	{
		pin += ((pin % 9) * 1000000) + 1000000;
	}

	return pin % 10000000;
}

/**
 * Generate default pin for Airocon Realtek router
 * 
 * @params char* The BSSID without ':'
 * 
 * @return int   7-digit pin
 */
int mac_airocon_pin(char *mac)
{
	char key[13];
	int b[6];
	int i, j, pin;

	key[2] = '\0';
	for (i=j=0; i<6; i++, j+=2) {
		key[0] = mac[j];
		key[1] = mac[j+1];
		b[i] = strtol(key, NULL, 16);
	}

	pin = ((b[0] + b[1]) % 10)
		+ (((b[5] + b[0]) % 10) * 10)
		+ (((b[4] + b[5]) % 10) * 100)
		+ (((b[3] + b[4]) % 10) * 1000)
		+ (((b[2] + b[3]) % 10) * 10000)
		+ (((b[1] + b[2]) % 10) * 100000)
		+ (((b[0] + b[1]) % 10) * 1000000);

	return pin  % 10000000;
}

/**
 * Generate default pin with Inv NIC
 * 
 * @params char* The BSSID without ':'
 * 
 * @return int   7-digit pin
 */
int mac_inv_nic_pin(char *mac)
{
	char key[13];
	int pin;

	strncpy(key, mac+6, 6);
	key[6] = '\0';
	pin = strtol(key, NULL, 16);

	return (~pin & 0xFFFFFF) % 10000000;
}

/**
 * Generate default pin with NIC * 2 or NIC * 3
 * 
 * @params char* The BSSID without ':'
 * @params int   The addicional number to calculate, default is 2 or 3
 * 
 * @return int   7-digit pin
 */
int mac_nicx_pin(char *mac, int x)
{
	char key[13];
	int pin;

	strncpy(key, mac+6, 6);
	key[6] = '\0';
	pin = strtol(key, NULL, 16);

	return (pin * x) % 10000000;
}

/**
 * Generate default pin with OUI + NIC
 * 
 * @params char* The BSSID without ':'
 * 
 * @return int   7-digit pin
 */
int mac_oui_add_nic_pin(char *mac)
{
	char key[13];
	int oui, nic;

	strncpy(key, mac, 6);
	key[6] = '\0';
	oui = strtol(key, NULL, 16);
	strncpy(key, mac+6, 6);
	key[6] = '\0';
	nic = strtol(key, NULL, 16);

	return ((oui + nic) % 0x1000000) % 10000000;
}

/**
 * Generate default pin with OUI - NIC
 * 
 * @params char* The BSSID without ':'
 * 
 * @return int   7-digit pin
 */
int mac_oui_sub_nic_pin(char *mac)
{
	char key[13];
	int pin, oui, nic;

	strncpy(key, mac, 6);
	key[6] = '\0';
	oui = strtol(key, NULL, 16);
	strncpy(key, mac+6, 6);
	key[6] = '\0';
	nic = strtol(key, NULL, 16);

	if (nic < oui)
	{
		pin = oui - nic;
	} else {
		pin = (oui + 0x1000000 - nic) & 0xFFFFFF;
	}

	return pin % 10000000;
}

/**
 * Generate default pin with OUI ^ NIC
 * 
 * @params char* The BSSID without ':'
 * 
 * @return int   7-digit pin
 */
int mac_oui_xor_nic_pin(char *mac)
{
	char key[13];
	int pin, oui, nic;

	strncpy(key, mac, 6);
	key[6] = '\0';
	oui = strtol(key, NULL, 16);
	strncpy(key, mac+6, 6);
	key[6] = '\0';
	nic = strtol(key, NULL, 16);
	pin = oui ^ nic;

	return pin % 10000000;
}

/*
	universal algorithm, returns pin derived from MAC and S/N
	used by: Belkin, DSL-EasyBox, Arcadyan, and possibly other manufacturers

	reverse-engineered by Stas'M
*/
int mac_sn_pin(char *mac, char *serial, int *init_bx, int add)
{
	int sn[4], nic[4];
	int mac_len, serial_len;
	int k1, k2, pin;
	int xor, i, j;
	char buff_mac[24], buff_serial[6];
	char key[2];
	int init_bk1, init_bk2, init_k1, init_k2, init_pin, init_xor, init_sub, init_sk, init_skv;
	long long unsigned int mac_i;

	mac_i = strtoll(mac, NULL, 16);
	mac_i = mac_i + add;
	sprintf(buff_mac, "%llX", mac_i);

	serial_len = strlen(serial);
	if (serial_len>=4) {
		strncpy(buff_serial, serial+(serial_len-4), 4);
	} else {
		for (i = 0; i< 4-serial_len; ++i) {
			buff_serial[i] = '0';
		}
		buff_serial[i] = '\0';
		strcat(buff_serial, serial);
	}
	buff_serial[4] = '\0';
	serial_len = strlen(buff_serial);

	key[1] = '\0';
	/* Get the four least significant digits of the serial number */
	for (i=0; i<4; ++i) {
		key[0] = buff_serial[i];
		sn[i] = strtol(key, NULL, 16);
	}

	/* Get the four least significant nibbles of the MAC address */
	nic[0] = (mac_i & 0xFFFF) >> 12;
	nic[1] = (mac_i & 0xFFF) >> 8;
	nic[2] = (mac_i & 0xFF) >> 4;
	nic[3] = mac_i & 0xF;

	/*
		init vector:
		bk1  - k1 sum composite bits (0~255)
		bk2  - k2 sum composite bits (0~255)
		bx[] - xor composite bits (array of 0~255)
		k1   - k1 init nibble (0~15)
		k2   - k2 init nibble (0~15)
		pin  - pin init value (0~9999999)
		xor  - xor init nibble (0~15)
		sub  - substraction mode (0 - no sub, 1 - substraction, 2 - addition)
		sk   - substraction multiple coefficient (0 - skv value, 1 - k1, 2 - k2)
		skv  - substraction multiple value
	*/
	init_bk1 = 60;
	init_bk2 = 195;
	init_k1 = 0;
	init_k2 = 0;
	init_pin = 0;
	init_xor = 0;
	init_sub = 0;
	init_sk = 0;
	init_skv = 0;

	k1 = init_k1 & 0xF;
	i = 0;
	while (init_bk1)
	{
		// 0      1      2      3      4     5     6     7
		// nic[0] nic[1] nic[2] nic[3] sn[0] sn[1] sn[2] sn[3]
		if (init_bk1 & 1)
		{
			k1 += (i < 4 ? nic[i] : sn[i - 4]);
			k1 &= 0xF;
		}
		init_bk1 >>= 1;
		i++;
	}
	k2 = init_k2 & 0xF;
	i = 0;
	while (init_bk2)
	{
		// 0      1      2      3      4     5     6     7
		// nic[0] nic[1] nic[2] nic[3] sn[0] sn[1] sn[2] sn[3]
		if (init_bk2 & 1)
		{
			k2 += (i < 4 ? nic[i] : sn[i - 4]);
			k2 &= 0xF;
		}
		init_bk2 >>= 1;
		i++;
	}

	pin = init_pin;
	for (j = 0; j < 7; j++)
	{
		xor = init_xor & 0xF;
		i = 0;
		while (init_bx[j])
		{
			// 0   1   2       3       4       5      6      7
			// k1, k2, nic[1], nic[2], nic[3], sn[1], sn[2], sn[3]
			if (init_bx[j] & 1)
			{
				xor ^= (i > 4 ? sn[i - 4] : (i > 1 ? nic[i - 1] : (i > 0 ? k2 : k1)));
			}
			init_bx[j] >>= 1;
			i++;
		}
		pin <<= 4;
		pin |= xor;
	}
	switch (init_sub)
	{
		case 1: // spoiled from Belkin's reversed algo
			pin = (pin % 10000000) - ((pin / 10000000) * (init_sk > 1 ? k2 : (init_sk > 0 ? k1 : init_skv)));
		case 2:
			pin = (pin % 10000000) + ((pin / 10000000) * (init_sk > 1 ? k2 : (init_sk > 0 ? k1 : init_skv)));
	}
	return pin % 10000000;
}

/**
 * Generate default pin for Belkin router
 * 
 * @params char* The BSSID without ':'
 * @params char* The serial number of AP
 * 
 * @return int   7-digit pin
 */
int mac_belkin_pin(char *mac, char *serial) {
	int bx[7] = {66, 129, 209, 10, 24, 3, 39};
	return mac_sn_pin(mac, serial, bx, 0);
}

/**
 * Generate default pin for Vodafone EasyBox router
 * 
 * @params char* The BSSID without ':'
 * @params char* The serial number of AP
 * 
 * @return int   7-digit pin
 */
int mac_vodafone_easybox_pin(char *mac, char *serial) {
	char key[13];
	char *ser;
	long int pin;

	int bx[7] = {129, 65, 6, 10, 136, 80, 33};
	if (strlen(serial) == 0) {
		strncpy(key, mac+8, 4);
		key[4] = '\0';
		pin = strtol(key, NULL, 16);
		sprintf(key, "%li", pin);
		ser = key;
	} else {
		ser = serial;
	}

	return mac_sn_pin(mac, ser, bx, 0);
}

/**
 * Generate default pin for Livebox Arcadyan router
 * 
 * @params char* The BSSID without ':'
 * @params char* The serial number of AP
 * 
 * @return int   7-digit pin
 */
int mac_livebox_arcadyan_pin(char *mac, char *serial) {
	int bx[7] = {129, 65, 6, 10, 136, 80, 33};
	//mac-2
	return mac_sn_pin(mac, serial, bx, -2);
}

/**
 * Generate various defaults pins with MAC
 * 
 * @params char* The BSSID without ':'
 * @params int*  Length of array by reference
 * 
 * @return int*  Pointer to all 7-digit pin in an array
 */
int *build_mac_pins(char *mac, int *len)
{
	int mac_pins[MAC_PIN_SIZE];
	int static_pins[STATIC_PIN_SIZE] = {
		1234567, /* Cisco */
		2017252, /* Broadcom 1 */
		4626484, /* Broadcom 2 */
		7622990, /* Broadcom 3 */
		6232714, /* Broadcom 4 */
		1086411, /* Broadcom 5 */
		3195719, /* Broadcom 6 */
		3043203, /* Airocon 1 */
		7141225, /* Airocon 2 */
		6817554, /* DSL-2740R */
		9566146, /* Realtek 1 */
		9571911, /* Realtek 2 */
		2085483, /* Upvel */
		4397768, /* UR-814AC */
		529417, /* UR-825AC 0529417 (not octal) */
		9995604, /* Onlime */
		3561153, /* Edimax */
		6795814, /* Thomson */
		3425928, /* HG532x */
		9575521 /* CBN ONO */
	};
	int *pins = NULL;
	int index, i, j;

	index = i = j = 0;
	mac_pins[index++] = mac_byte_pin(mac, 6); /* 24-bit PIN */
	mac_pins[index++] = mac_byte_pin(mac, 7); /* 28-bit PIN */
	mac_pins[index++] = mac_byte_pin(mac, 8); /* 32-bit PIN */
	mac_pins[index++] = mac_byte_pin(mac, 9); /* 36-bit PIN */
	mac_pins[index++] = mac_byte_pin(mac, 10); /* 40-bit PIN */
	mac_pins[index++] = mac_byte_pin(mac, 11); /* 44-bit PIN */
	mac_pins[index++] = mac_byte_pin(mac, 12); /* 48-bit PIN */
	mac_pins[index++] = mac_reverse_byte_pin(mac, 6); /* Reverse byte 24-bit */
	mac_pins[index++] = mac_reverse_byte_pin(mac, 8); /* Reverse byte 32-bit */
	mac_pins[index++] = mac_reverse_byte_pin(mac, 12); /* Reverse byte 48-bit */
	mac_pins[index++] = mac_reverse_nibble_pin(mac, 6); /* Reverse nibble 24-bit */
	mac_pins[index++] = mac_reverse_nibble_pin(mac, 8); /* Reverse nibble 32-bit */
	mac_pins[index++] = mac_reverse_nibble_pin(mac, 12); /* Reverse nibble 48-bit */
	mac_pins[index++] = mac_reverse_bit_pin(mac, 6); /* Reverse bits 24-bit */
	mac_pins[index++] = mac_reverse_bit_pin(mac, 8); /* Reverse bits 32-bit */
	mac_pins[index++] = mac_reverse_bit_pin(mac, 12); /* Reverse bits 48-bit */
	mac_pins[index++] = mac_dlink_pin(mac, 0); /* D-Link PIN */
	mac_pins[index++] = mac_dlink_pin(mac, 1); /* D-Link PIN + 1 */
	mac_pins[index++] = mac_asus_pin(mac); /* ASUS PIN */
	mac_pins[index++] = mac_airocon_pin(mac); /* Airocon Realtek */
	mac_pins[index++] = mac_inv_nic_pin(mac); /* Inv NIC to PIN */
	mac_pins[index++] = mac_nicx_pin(mac, 2); /* NIC * 2 */
	mac_pins[index++] = mac_nicx_pin(mac, 3); /* NIC * 3 */
	mac_pins[index++] = mac_oui_add_nic_pin(mac); /* OUI + NIC */
	mac_pins[index++] = mac_oui_sub_nic_pin(mac); /* OUI - NIC */
	mac_pins[index++] = mac_oui_xor_nic_pin(mac); /* OUI ^ NIC */
	mac_pins[index++] = mac_belkin_pin(mac, ""); /* Belkin PIN without serial */
	mac_pins[index++] = mac_vodafone_easybox_pin(mac, ""); /* Vodafone EasyBox without serial */
	mac_pins[index++] = mac_livebox_arcadyan_pin(mac, ""); /* Livebox Arcadyan without serial */

	*len = index + STATIC_PIN_SIZE;
	pins = malloc(sizeof(mac_pins) + sizeof(static_pins));
	if (pins)
	{
		for (i=0; i<index; ++i)
		{
			pins[i] = mac_pins[i];
		}
		for (j=0; j<STATIC_PIN_SIZE; ++i, ++j)
		{
			pins[i] = static_pins[j];
		}
	}
	else
	{
		*len = 0;
	}

	return pins;
}

/**
 * Generate various defaults pins with MAC and serial
 * 
 * @params char* The BSSID without ':'
 * @params char* The serial number of AP
 * @params int*  Length of array by reference
 * 
 * @return int*  Pointer to all 7-digit pin in an array
 */
int *build_mac_sn_pins(char *mac, char *serial, int *len)
{
	int mac_pins[4];
	int *pins = NULL;
	int index, i;

	index = i = 0;
	mac_pins[index++] = mac_belkin_pin(mac, serial);/* Belkin PIN */
	mac_pins[index++] = mac_vodafone_easybox_pin(mac, serial); /* Vodafone EasyBox */
	mac_pins[index++] = mac_livebox_arcadyan_pin(mac, serial); /* Livebox Arcadyan */

	*len = index;
	pins = malloc(sizeof(mac_pins));
	if (pins)
	{
		for (i=0; i<index; ++i)
		{
			pins[i] = mac_pins[i];
		}
	}
	else
	{
		*len = 0;
	}

	return pins;
}

