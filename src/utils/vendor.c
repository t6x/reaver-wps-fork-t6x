#include <string.h>

char *get_vendor_string(const unsigned char* oui) {
	#define VENDOR_STR_SIZE (8 + 1)
	struct vendor {
		unsigned char id[3];
		char name[VENDOR_STR_SIZE];
	};

	static const struct vendor vendors[] =
	{ /* Using the same names as Wireshark */
		{"\x00\x10\x18", "Broadcom"}, /* Broadcom */
		{"\x00\x03\x7f", "AtherosC"}, /* Atheros Communications */
		{"\x00\x0c\x43", "RalinkTe"}, /* Ralink Technology, Corp. */
		{"\x00\x17\xa5", "RalinkTe"}, /* Ralink Technology Corp */
		{"\x00\xe0\x4c", "RealtekS"}, /* Realtek Semiconductor Corp. */
		{"\x00\x0a\x00", "Mediatek"}, /* Mediatek Corp. */
		{"\x00\x0c\xe7", "Mediatek"}, /* Mediatek MediaTek Inc. */
		{"\x00\x1c\x51", "CelenoCo"}, /* Celeno Communications */
		{"\x00\x50\x43", "MarvellS"}, /* Marvell Semiconductor, Inc. */
		{"\x00\x26\x86", "Quantenn"}, /* Quantenna */
		{"\x00\x09\x86", "LantiqML"}, /* Lantiq/MetaLink */
		{"\x00\x50\xf2", "Microsof"}  /* Microsoft */
	};

	#define VENDOR_LIST_SIZE (sizeof(vendors)/sizeof(vendors[0]))

	if(!oui) return 0;

	int i;
	for (i = 0; i < VENDOR_LIST_SIZE; i++)
		if (!memcmp(oui, vendors[i].id, 3))
			return (void*) vendors[i].name;

	return "Unknown ";

}
