/*
 * Reaver - Common definitions
 * Copyright (c) 2011, Tactical Network Solutions, Craig Heffner <cheffner@tacnetsol.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of portions of this program with the
 *  OpenSSL library under certain conditions as described in each
 *  individual source file, and distribute linked combinations
 *  including the two.
 *  You must obey the GNU General Public License in all respects
 *  for all of the code used other than OpenSSL. *  If you modify
 *  file(s) with this exception, you may extend this exception to your
 *  version of the file(s), but you are not obligated to do so. *  If you
 *  do not wish to do so, delete this exception statement from your
 *  version. *  If you delete this exception statement from all source
 *  files in the program, then also delete it here.
 */

#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pcap.h>
#include "utils/endianness.h"

#include "wps.h"
#include "cprintf.h"

#define NULL_MAC		"\x00\x00\x00\x00\x00\x00"
#define DEFAULT_MAX_NUM_PROBES	15
#define MAX_ASSOC_FAILURES	10

#define TIMESTAMP_LEN		8
#define MAC_ADDR_LEN    	6
#define SSID_TAG_NUMBER		0
#define RATES_TAG_NUMBER	1
#define CHANNEL_TAG_NUMBER	3
#define WPS_TAG_NUMBER		0xDD
#define VENDOR_SPECIFIC_TAG	0xDD
#define RSN_TAG_NUMBER		0x30
#define HT_CAPS_TAG_NUMBER	0x2d

#define CAPABILITY_WEP		0x10

#define WPA_IE_ID               "\x00\x50\xF2\x01\x01\x00"
#define WPA_IE_ID_LEN           6

#define MANAGEMENT_FRAME	0x00
#define SUBTYPE_BEACON		0x08

#define DOT1X_AUTHENTICATION	0x888E
#define DOT1X_EAP_PACKET	0x00

#define SIMPLE_CONFIG		0x00000001

#define P1_SIZE			10000
#define P2_SIZE			1000

#define EAPOL_START_MAX_TRIES	10
#define WARN_FAILURE_COUNT	10

#define EAPOL_START		1
#define EAP_IDENTITY 		0x01
#define EAP_EXPANDED            0xFE

#define M57_DEFAULT_TIMEOUT     400000          /* uSeconds */
#define M57_MAX_TIMEOUT         1000000         /* uSeconds */
#define DEFAULT_DELAY           1               /* Seconds */
#define DEFAULT_TIMEOUT         10               /* Seconds */
#define DEFAULT_LOCK_DELAY      60              /* Seconds */
#define SEC_TO_US               1000000         /* uSeconds in a Second */

#define WPS_DEVICE_NAME		"Glau"
#define WPS_MANUFACTURER	"Microsoft"
#define WPS_MODEL_NAME		"Windows"
#define WPS_MODEL_NUMBER	"6.1.7601"
#define WPS_DEVICE_TYPE		"\x00\x01\x00\x50\xF2\x04\x00\x01"
#define WPS_OS_VERSION		"\x01\x00\x06\x00"
#define WPS_RF_BANDS		0x01

enum encryption_type
{
        NONE,
        WEP,
        WPA
};

enum key_state
{
	KEY1_WIP = 0,
	KEY2_WIP = 1,
	KEY_DONE = 2
};

enum eap_codes
{
	EAP_REQUEST = 1,
	EAP_RESPONSE = 2,
	EAP_SUCCESS = 3,
	EAP_FAILURE = 4
};

enum wps_result
{
	KEY_ACCEPTED = 0,
	KEY_REJECTED = 1,
	RX_TIMEOUT = 2,
	EAP_FAIL = 3,
	UNKNOWN_ERROR = 4
};

enum nack_code
{
	NO_NACK = -1,
	NO_ERROR = 0,
	OOB_RRAD_ERROR = 1,
	CRC_FAILURE = 2,
	CHANNEL_24_NS = 3,
	CHANNEL_50_NS = 4,
	WEAK_SIGNAL = 5,
	NET_AUTH_FAILURE = 6,
	NET_ASSOCIATION_FAILURE = 7,
	NO_DHCP_RESPONSE = 8,
	FAILED_DHCP_CONFIG = 9,
	IP_ADDR_CONFLICT = 10,
	REGISTRAR_CONNECT_FAILURE = 11,
	MULTIPLE_PBC = 12,
	ROGUE_ACTIVITY = 13,
	DEVICE_BUSY = 14,
	SETUP_LOCKED = 15,
	MESSAGE_TIMEOUT = 16,
	REGISTRATION_TIMEOUT = 17,
	AUTH_FAILURE = 18
};

enum wps_type
{
	TERMINATE = -1,
	UNKNOWN = 0,
	IDENTITY_REQUEST = 1,
	IDENTITY_RESPONSE = 2,
	M1 = 0x04,
        M2 = 0x05,
        M3 = 0x07,
        M4 = 0x08,
        M5 = 0x09,
        M6 = 0x0A,
        M7 = 0x0B,
        M8 = 0x0C,
        DONE = 0x0F,
        NACK = 0x0E,
        WPS_PT_DEAUTH = 0xFF
};

enum wfa_elements
{
	AP_CHANNEL = 0x1001,
	ASSOCIATION_STATE = 0x1002,
	AUTHENTICATION_TYPE = 0x1003,
	AUTHENTICATION_TYPE_FLAGS = 0x1004,
	AUTHENTICATOR = 0x1005,
	CONFIG_METHODS = 0x1008,
	CONFIGURATION_ERROR = 0x1009,
	CONFIRMATION_URL_4 = 0x100A,
	CONFIRMATION_URL_6 = 0x100B,
	CONNECTION_TYPE = 0x100C,
	CONNECTION_TYPE_FLAGS = 0x100D,
	CREDENTIAL = 0x100E,
	DEVICE_NAME = 0x1011,
	DEVICE_PASSWORD_ID = 0x1012,
	ENROLLEE_HASH_1 = 0x1014,
	ENROLLEE_HASH_2 = 0x1015,
	ENROLLEE_SNONCE_1 = 0x1016,
	ENROLLEE_SNONCE_2 = 0x1017,
	ENCRYPTED_SETTINGS = 0x1018,
	ENCRYPTION_TYPE = 0x100F,
	ENCRYPTION_TYPE_FLAGS = 0x1010,
	ENROLLEE_NONCE = 0x101A,
	FEATURE_ID = 0x101B,
	IDENTITY = 0x101C,
	IDENTITY_PROOF = 0x101D,
	KEY_WRAP_AUTHENTICATOR = 0x101E,
	KEY_IDENTIFIER = 0x101F,
	MAC_ADDRESS = 0x1020,
	MANUFACTURER = 0x1021,
	MESSAGE_TYPE = 0x1022,
	MODEL_NAME = 0x1023,
	MODEL_NUMBER = 0x1024,
	NETWORK_INDEX = 0x1026,
	NETWORK_KEY = 0x1027,
	NETWORK_KEY_INDEX = 0x1028,
	NEW_DEVICE_NAME = 0x1029,
	NEW_PASSWORD = 0x102A,
	OOB_DEVICE_PASSWORD = 0x102C,
	OS_VERSION = 0x102D,
	POWER_LEVEL = 0x102F,
	PSK_CURRENT = 0x1030,
	PSK_MAX = 0x1031,
	PUBLIC_KEY = 0x1032,
	RADIO_ENABLED = 0x1033,
	REBOOT = 0x1034,
	REGISTRAR_CURRENT = 0x1035,
	REGISTRAR_ESTABLISHED = 0x1036,
	REGISTRAR_LIST = 0x1037,
	REGISTRAR_MAX = 0x1038,
	REGISTRAR_NONCE = 0x1039,
	REQUEST_TYPE = 0x103A,
	RESPONSE_TYPE = 0x103B,
	RF_BANDS = 0x103C,
	REGISTRAR_HASH_1 = 0x103D,
	REGISTRAR_HASH_2 = 0x103E,
	REGISTRAR_SNONCE_1 = 0x103F,
	REGISTRAR_SNONCE_2 = 0x1040,
	SELECTED_REGISTRAR = 0x1041,
	SERIAL_NUMBER = 0x1042,
	WPS_STATE = 0x1044,
	SSID = 0x1045,
	TOTAL_NETWORKS = 0x1046,
	ENROLLEE_UUID = 0x1047,
	REGISTRAR_UUID = 0x1048,
	VENDOR_EXTENSION = 0x1049,
	VERSION = 0x104A,
	X509_CERT_REQUEST = 0x104B,
	X509_CERT = 0x104C,
	WPS_EAP_IDENTITY = 0x104D,
	MESSAGE_COUNTER = 0x104E,
	PUBLIC_KEY_HASH = 0x104F,
	REKEY_KEY = 0x1050,
	KEY_LIFETIME = 0x1051,
	PERMITTED_CONFIG_METHODS = 0x1052,
	SELECTED_REGISTRAR_CONFIG_METHODS = 0x1053,
	PRIMARY_DEVICE_TYPE = 0x1054,
	SECONDARY_DEVICE_TYPE_LIST = 0x1055,
	PORTABLE_DEVICE = 0x1056,
	AP_SETUP_LOCKED = 0x1057,
	APPLICATION_EXTENSION = 0x1058,
	EAP_TYPE = 0x1059,
	INITIALIZATION_VECTOR = 0x1060,
	KEY_PROVIDED_AUTOMATICALLY = 0x1061,
	ENABLED_8021X = 0x1062,
	APP_SESSION_KEY = 0x1063,
	WEP_TRANSMIT_KEY = 0x10064
};

#define IEEE80211_FCTL_VERS		0x0003
#define IEEE80211_FCTL_FTYPE		0x000c
#define IEEE80211_FCTL_STYPE		0x00f0
#define IEEE80211_FCTL_TODS		0x0100
#define IEEE80211_FCTL_FROMDS		0x0200
#define IEEE80211_FCTL_MOREFRAGS	0x0400
#define IEEE80211_FCTL_RETRY		0x0800
#define IEEE80211_FCTL_PM		0x1000
#define IEEE80211_FCTL_MOREDATA		0x2000
#define IEEE80211_FCTL_PROTECTED	0x4000
#define IEEE80211_FCTL_ORDER		0x8000

#define IEEE80211_SCTL_FRAG		0x000F
#define IEEE80211_SCTL_SEQ		0xFFF0

#define IEEE80211_FTYPE_MGMT		0x0000
#define IEEE80211_FTYPE_CTL		0x0004
#define IEEE80211_FTYPE_DATA		0x0008

/* management */
#define IEEE80211_STYPE_ASSOC_REQ	0x0000
#define IEEE80211_STYPE_ASSOC_RESP	0x0010
#define IEEE80211_STYPE_REASSOC_REQ	0x0020
#define IEEE80211_STYPE_REASSOC_RESP	0x0030
#define IEEE80211_STYPE_PROBE_REQ	0x0040
#define IEEE80211_STYPE_PROBE_RESP	0x0050
#define IEEE80211_STYPE_BEACON		0x0080
#define IEEE80211_STYPE_ATIM		0x0090
#define IEEE80211_STYPE_DISASSOC	0x00A0
#define IEEE80211_STYPE_AUTH		0x00B0
#define IEEE80211_STYPE_DEAUTH		0x00C0
#define IEEE80211_STYPE_ACTION		0x00D0

/* control */
#define IEEE80211_STYPE_BACK_REQ	0x0080
#define IEEE80211_STYPE_BACK		0x0090
#define IEEE80211_STYPE_PSPOLL		0x00A0
#define IEEE80211_STYPE_RTS		0x00B0
#define IEEE80211_STYPE_CTS		0x00C0
#define IEEE80211_STYPE_ACK		0x00D0
#define IEEE80211_STYPE_CFEND		0x00E0
#define IEEE80211_STYPE_CFENDACK	0x00F0

/* data */
#define IEEE80211_STYPE_DATA			0x0000
#define IEEE80211_STYPE_DATA_CFACK		0x0010
#define IEEE80211_STYPE_DATA_CFPOLL		0x0020
#define IEEE80211_STYPE_DATA_CFACKPOLL		0x0030
#define IEEE80211_STYPE_NULLFUNC		0x0040
#define IEEE80211_STYPE_CFACK			0x0050
#define IEEE80211_STYPE_CFPOLL			0x0060
#define IEEE80211_STYPE_CFACKPOLL		0x0070
#define IEEE80211_STYPE_QOS_DATA		0x0080
#define IEEE80211_STYPE_QOS_DATA_CFACK		0x0090
#define IEEE80211_STYPE_QOS_DATA_CFPOLL		0x00A0
#define IEEE80211_STYPE_QOS_DATA_CFACKPOLL	0x00B0
#define IEEE80211_STYPE_QOS_NULLFUNC		0x00C0
#define IEEE80211_STYPE_QOS_CFACK		0x00D0
#define IEEE80211_STYPE_QOS_CFPOLL		0x00E0
#define IEEE80211_STYPE_QOS_CFACKPOLL		0x00F0

/* these types denote that the values are stored in a specific byte order */
#ifndef LE16_DEFINED
#define LE16_DEFINED
typedef uint16_t le16;
typedef uint32_t le32;

typedef uint16_t be16;
typedef uint32_t be32;
#endif

#pragma pack(1)
struct radio_tap_header
{
	uint8_t revision;
	uint8_t pad;
	le16 len;
	le32 flags;
#ifdef RADIOTAP_HEADER_WITH_RATE
	uint8_t rate;
	uint8_t pad2;
#endif
	le16 txflags;
};

struct dot11_frame_header
{
	le16 fc;
	le16 duration;
	unsigned char addr1[MAC_ADDR_LEN];
	unsigned char addr2[MAC_ADDR_LEN];
	unsigned char addr3[MAC_ADDR_LEN];
	le16 frag_seq;
};

struct authentication_management_frame
{
	le16 algorithm;
	le16 sequence;
	le16 status;
};

struct association_request_management_frame
{
	le16 capability;
	le16 listen_interval;
};

struct association_response_management_frame
{
	le16 capability;
	le16 status;
	le16 id;
};

struct beacon_management_frame
{
	unsigned char timestamp[TIMESTAMP_LEN];
	le16 beacon_interval;
	le16 capability;
};

struct llc_header
{
	uint8_t dsap;
	uint8_t ssap;
	uint8_t control_field;
	unsigned char org_code[3];
	be16 type;
};

struct dot1X_header
{
	uint8_t version;
	uint8_t type;
	uint16_t len;
};

struct eap_header
{
	uint8_t code;
	uint8_t id;
	uint16_t len;
	uint8_t type;
};

struct wfa_expanded_header
{
	unsigned char id[3];
	be32 type;
	uint8_t opcode;
	uint8_t flags;
};

struct wfa_element_header
{
        uint16_t type;
        uint16_t length;
};

struct tagged_parameter
{
	uint8_t number;
	uint8_t len;
};
#pragma pack()

#define MIN_BEACON_SIZE		(sizeof(struct radio_tap_header) + sizeof(struct dot11_frame_header) + sizeof(struct beacon_management_frame))

#endif
