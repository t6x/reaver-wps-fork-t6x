/*
 * Reaver - 802.11 functions
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

#include "iface.h"
#include "80211.h"
#include "send.h"
#include "utils/radiotap.h"
#include "crc.h"
#include "pcapfile.h"
#include <libwps.h>
#include <assert.h>

/* define NO_REPLAY_HTCAPS to 1 if you want to disable sending
   ht caps in association request for testing */
#ifndef NO_REPLAY_HTCAPS
#define NO_REPLAY_HTCAPS 0
#endif

static void deauthenticate(void);
static void authenticate(void);
static void associate(void);
static int check_fcs(const unsigned char *packet, size_t len);


/*Reads the next packet from pcap_next() and validates the FCS. */
unsigned char *next_packet(struct pcap_pkthdr *header)
{
	const unsigned char *packet = NULL;
	struct pcap_pkthdr *pkt_header;
	static int warning_shown = 0;
	int status;

	/* Loop until we get a valid packet, or until we run out of packets */
	while((status = pcap_next_ex(get_handle(), &pkt_header, &packet)) == 1 || !status)
	{
		if(!status) continue; /* timeout */

		memcpy(header, pkt_header, sizeof(*header));

		int fd;
		if((fd = get_output_fd()) != -1)
			pcapfile_write_packet(fd, header, packet);

		if(get_validate_fcs() && !check_fcs(packet, header->len)) {
			if(!warning_shown)
				cprintf(INFO, "[!] Found packet with bad FCS, skipping...\n");
			warning_shown = 1;
			continue;
		}

		break;
	}

	return (void*)packet;
}

#define BEACON_SIZE(rth_len) (rth_len + sizeof(struct dot11_frame_header) + sizeof(struct beacon_management_frame))
/* probe responses, just like beacons, start their management frame packet with the same
   fixed parameters of size 12 */
#define PROBE_RESP_SIZE(rth_len) BEACON_SIZE(rth_len)

/* return 1 if beacon, -1 if probe response */
/* additionally populates frameheader and management frame pointers */
int is_management_frame(
	/* input params */
	const struct pcap_pkthdr *header,
	unsigned const char *packet,
	/* output */
	const struct dot11_frame_header **fh,
	const struct beacon_management_frame **mf
) {
	struct radio_tap_header *rt_header = (void *) radio_header(packet, header->len);
	size_t rt_header_len = end_le16toh(rt_header->len);
	if(header->len < BEACON_SIZE(rt_header_len))
			return 0;

	*fh = (void *) (packet + rt_header_len);
	unsigned f_type = (*fh)->fc & end_htole16(IEEE80211_FCTL_FTYPE);
	unsigned fsub_type = (*fh)->fc & end_htole16(IEEE80211_FCTL_STYPE);

	int is_management_frame = f_type == end_htole16(IEEE80211_FTYPE_MGMT);
	int is_beacon = is_management_frame && fsub_type == end_htole16(IEEE80211_STYPE_BEACON);
	int is_probe_resp = is_management_frame && fsub_type == end_htole16(IEEE80211_STYPE_PROBE_RESP);

	if(is_management_frame) *mf = (void *) (packet + rt_header_len + sizeof(struct dot11_frame_header));

	if(is_beacon) return 1;
	if(is_probe_resp) return -1;
	return 0;
}

unsigned char* next_management_frame(
	struct pcap_pkthdr *header,
	const struct dot11_frame_header **fh,
	const struct beacon_management_frame **mf,
	int *type
) {
	unsigned char *packet;
	while((packet = next_packet(header))) {
		if((*type = is_management_frame(header, packet, fh, mf))) break;
	}
	return packet;
}
unsigned char* next_beacon(
	struct pcap_pkthdr *header,
	const struct dot11_frame_header **fh,
	const struct beacon_management_frame **mf
) {
	unsigned char *packet; int type;
	while((packet = next_management_frame(header, fh, mf, &type))) {
		if(type == 1) break;
	}
	return packet;
}


/* 
 * Waits for a beacon packet from the target AP and populates the globule->ap_capabilities field.
 * This is used for obtaining the capabilities field and AP SSID.
 */
void read_ap_beacon()
{
	struct pcap_pkthdr header;
	const unsigned char *packet;
	const struct dot11_frame_header *frame_header;
	const struct beacon_management_frame *beacon;
	time_t start_time = time(NULL);

	set_ap_capability(0);

	while((packet = next_packet(&header))) {
		int type = is_management_frame(&header, packet, &frame_header, &beacon);
		if(type != 1 || !is_target(frame_header)) {
			/* If we haven't seen any beacon packets from the target within BEACON_WAIT_TIME seconds, try another channel */
			if((time(NULL) - start_time) >= BEACON_WAIT_TIME)
			{
				next_channel();
				start_time = time(NULL);
			}
			continue;
		}

		set_ap_capability(end_le16toh(beacon->capability));

		/* Obtain the SSID and channel number from the beacon packet */
		int channel = parse_beacon_tags(packet, header.len);
		/* If no channel was manually specified, switch to the AP's current channel */
		if(!get_fixed_channel() && get_auto_channel_select() && channel > 0 && channel != get_channel())
		{
			change_channel(channel);
			set_channel(channel);
		}
		break;
        }
}

int freq_to_chan (uint16_t freq) {
	if (freq >= 2412 && freq <= 2472) {
		return (freq - 2407) / 5;
	} else if (freq == 2484) {
		return 14;
	} else if (freq >= 4900 && freq < 5000) {
		return (freq - 4000) / 5;
	} else if (freq >= 5000 && freq < 5900) {
		return (freq - 5000) / 5;
	} else if (freq >= 56160 + 2160 * 1 && freq <= 56160 + 2160 * 4) {
		return (freq - 56160) / 2160;
	}
	return 0;
}

#include "radiotap_flags.h"

/* readbuf must be of sufficient len to read the full flag, maximal 4 chars. */
static int get_radiotap_flag(const unsigned char *packet, size_t len, unsigned flagnumber, unsigned char* readbuf)
{
	if(has_rt_header() && (len > (sizeof(struct radio_tap_header))))
	{
		uint32_t offset, presentflags;
		if(!rt_get_presentflags(packet, len, &presentflags, &offset))
			return 0;
		if(!(presentflags & (1U << flagnumber)))
			return 0;
		offset = rt_get_flag_offset(presentflags, flagnumber, offset);
		if (offset + ieee80211_radiotap_type_size[flagnumber] < len) {
			memcpy(readbuf, packet+offset, ieee80211_radiotap_type_size[flagnumber]);
			return 1;
		}
	}

	return 0;
}


/* Extracts the channel frequency from the packet's radio tap header */
uint16_t rt_channel_freq(const unsigned char *packet, size_t len)
{
	unsigned char readbuf[4];
	/* the lower 2 byte of the channel flag seems to contain channel type, i.e. ABG.
	   but we need to read all 4. */
	uint16_t result;
	if(get_radiotap_flag(packet, len, IEEE80211_RADIOTAP_CHANNEL, readbuf)) {
		memcpy(&result, readbuf, 2);
		return end_le32toh(result);
	}
	return 0;
}

/* Extracts the signal strength field (if any) from the packet's radio tap header */
int8_t signal_strength(const unsigned char *packet, size_t len)
{
	unsigned char readbuf[1];
	if(get_radiotap_flag(packet, len, IEEE80211_RADIOTAP_DBM_ANTSIGNAL, readbuf))
		return (int8_t) readbuf[0];
	return 0;
}

/* 
 * Determines if the target AP has locked its WPS state or not.
 * Returns 0 if not locked, 1 if locked, -1 if wps has been turned off
 * pass data of a valid beacon packet
 */
int is_wps_locked(const struct pcap_pkthdr *header, const unsigned char *packet)
{
	struct libwps_data wps = { 0 };
	if(parse_wps_parameters(packet, header->len, &wps)) {
		if(wps.locked == WPSLOCKED) return 1;
		return 0;
	}
	return -1;
}


/* Waits for authentication and association responses from the target AP */
static int process_authenticate_associate_resp(int want_assoc)
{
	struct pcap_pkthdr header;
	unsigned char *packet;
	struct radio_tap_header *rt_header;
	struct dot11_frame_header *dot11_frame;
	struct authentication_management_frame *auth_frame;
	struct association_response_management_frame *assoc_frame;
	int ret_val = 0;

	start_timer();

	while(!get_out_of_time())
	{
		if((packet = next_packet(&header)) == NULL) break;

		if(header.len < MIN_AUTH_SIZE) continue;

		rt_header = (void*) radio_header(packet, header.len);
		size_t rt_header_len = end_le16toh(rt_header->len);
		dot11_frame = (void*)(packet + rt_header_len);

		if((memcmp(dot11_frame->addr3, get_bssid(), MAC_ADDR_LEN) != 0) ||
		   (memcmp(dot11_frame->addr1, get_mac(), MAC_ADDR_LEN) != 0))
			continue;

		int isMgmtFrame = (dot11_frame->fc & end_htole16(IEEE80211_FCTL_FTYPE)) == end_htole16(IEEE80211_FTYPE_MGMT);
		if(!isMgmtFrame) continue;

		void *ptr = (packet + sizeof(struct dot11_frame_header) + rt_header_len);
		auth_frame = ptr;
		assoc_frame = ptr;

		int isAuthResp = (dot11_frame->fc & end_htole16(IEEE80211_FCTL_STYPE)) == end_htole16(IEEE80211_STYPE_AUTH);
		int isAssocResp = (dot11_frame->fc & end_htole16(IEEE80211_FCTL_STYPE)) == end_htole16(IEEE80211_STYPE_ASSOC_RESP);

		if(!isAuthResp && !isAssocResp) continue;

		if(isAuthResp && want_assoc) continue;

		/* Did we get an authentication packet with a successful status? */
		if(isAuthResp && (auth_frame->status == end_htole16(AUTHENTICATION_SUCCESS))) {
			ret_val = AUTH_OK;
			break;
		}
		/* Did we get an association packet with a successful status? */
		else if(isAssocResp && (assoc_frame->status == end_htole16(ASSOCIATION_SUCCESS))) {
			ret_val = ASSOCIATE_OK;
			break;
		}
        }

        return ret_val;
}



/* Deauths and re-associates a MAC address with the AP. Returns 0 on failure, 1 for success. */
int reassociate(void)
{
	if (get_external_association()) return 1;

	int state = 0, ret;

	while(1) {
		switch(state) {
			case 0:
				deauthenticate();
				state++;
				break;
			case 1:
				authenticate();
				state++;
				break;
			case 2:
				ret = process_authenticate_associate_resp(0);
				if(ret) state++;
				else return 0;
				break;
			case 3:
				associate();
				state++;
				break;
			case 4:
				ret = process_authenticate_associate_resp(1);
				if(ret) state++;
				else return 0;
				break;
			case 5:
				return 1;

		}
	}
}

/* Deauthenticate ourselves from the AP */
static void deauthenticate(void)
{
	size_t radio_tap_len, dot11_frame_len, packet_len, offset = 0;
	struct radio_tap_header radio_tap;
	struct dot11_frame_header dot11_frame;

	radio_tap_len = build_radio_tap_header(&radio_tap);
        dot11_frame_len = build_dot11_frame_header(&dot11_frame, FC_DEAUTHENTICATE);
	packet_len = radio_tap_len + dot11_frame_len + DEAUTH_REASON_CODE_SIZE;

	unsigned char packet[sizeof radio_tap + sizeof dot11_frame + DEAUTH_REASON_CODE_SIZE];
	assert(sizeof packet == packet_len);

	memcpy(packet, &radio_tap, radio_tap_len);
	offset += radio_tap_len;
	memcpy(packet + offset, &dot11_frame, dot11_frame_len);
	offset += dot11_frame_len;
	memcpy(packet + offset, DEAUTH_REASON_CODE, DEAUTH_REASON_CODE_SIZE);

	send_packet(packet, packet_len, 1);
}

/* Authenticate ourselves with the AP */
static void authenticate(void)
{
	size_t radio_tap_len, dot11_frame_len, management_frame_len, packet_len, offset;
	struct radio_tap_header radio_tap;
	struct dot11_frame_header dot11_frame;
	struct authentication_management_frame management_frame;

	radio_tap_len = build_radio_tap_header(&radio_tap);
	dot11_frame_len = build_dot11_frame_header(&dot11_frame, FC_AUTHENTICATE);
	management_frame_len = build_authentication_management_frame(&management_frame);

	packet_len = radio_tap_len + dot11_frame_len + management_frame_len;

	unsigned char packet[ sizeof (struct radio_tap_header)
			    + sizeof (struct dot11_frame_header)
			    + sizeof (struct authentication_management_frame)];

	assert(packet_len == sizeof packet);

	offset = 0;

	memcpy(packet + offset, &radio_tap, radio_tap_len);
	offset += radio_tap_len;
	memcpy(packet + offset, &dot11_frame, dot11_frame_len);
	offset += dot11_frame_len;
	memcpy(packet + offset, &management_frame, management_frame_len);

	send_packet(packet, packet_len, 1);
	cprintf(VERBOSE, "[+] Sending authentication request\n");
}

/* Associate with the AP */
static void associate(void)
{
        size_t radio_tap_len, dot11_frame_len, management_frame_len, ssid_tag_len,
		wps_tag_len, rates_tag_len, ht_tag_len, packet_len, offset = 0;
	struct radio_tap_header radio_tap;
	struct dot11_frame_header dot11_frame;
	struct association_request_management_frame management_frame;
	char *essid = get_ssid();
	if(!essid) essid = "";
	unsigned char ssid_tag[sizeof (struct tagged_parameter) + IW_ESSID_MAX_SIZE];
	unsigned char rates_tag[128];
	unsigned char wps_tag[sizeof (struct tagged_parameter) + WPS_TAG_SIZE];
	unsigned char ht_tag[128];


        radio_tap_len = build_radio_tap_header(&radio_tap);
        dot11_frame_len = build_dot11_frame_header(&dot11_frame, FC_ASSOCIATE);
        management_frame_len = build_association_management_frame(&management_frame);
	ssid_tag_len = build_ssid_tagged_parameter(ssid_tag, essid);
	rates_tag_len = build_supported_rates_tagged_parameter(rates_tag, sizeof rates_tag);
	wps_tag_len = build_wps_tagged_parameter(wps_tag);

	if(!NO_REPLAY_HTCAPS) {
		ht_tag_len = build_htcaps_parameter(ht_tag, sizeof ht_tag);
	} else {
		ht_tag_len = 0;
	}
        packet_len = radio_tap_len + dot11_frame_len + management_frame_len + ssid_tag_len + wps_tag_len + rates_tag_len + ht_tag_len;

	unsigned char packet[512];
	assert(packet_len < sizeof packet);

	memcpy(packet, &radio_tap, radio_tap_len);
	offset += radio_tap_len;
	memcpy(packet+offset, &dot11_frame, dot11_frame_len);
	offset += dot11_frame_len;
	memcpy(packet+offset, &management_frame, management_frame_len);
	offset += management_frame_len;
	memcpy(packet+offset, ssid_tag, ssid_tag_len);
	offset += ssid_tag_len;
	memcpy(packet+offset, rates_tag, rates_tag_len);
	offset += rates_tag_len;

	memcpy(packet+offset, ht_tag, ht_tag_len);
	offset += ht_tag_len;

	memcpy(packet+offset, wps_tag, wps_tag_len);

	send_packet(packet, packet_len, 1);
	cprintf(VERBOSE, "[+] Sending association request\n");

}


/* Given a beacon / probe response packet, returns the reported encryption type (WPA, WEP, NONE)
   THIS IS BROKE!!! DO NOT USE!!!
*/
enum encryption_type supported_encryption(const unsigned char *packet, size_t len)
{
	enum encryption_type enc = NONE;
	const unsigned char *tag_data = NULL;
	struct radio_tap_header *rt_header = NULL;
	size_t vlen = 0, voff = 0, tag_offset = 0, tag_len = 0, offset = 0;
	struct beacon_management_frame *beacon = NULL;

	if(len > MIN_BEACON_SIZE)
	{
		rt_header = (struct radio_tap_header *) radio_header(packet, len);
		size_t rt_header_len = end_le16toh(rt_header->len);
		beacon = (struct beacon_management_frame *) (packet + rt_header_len + sizeof(struct dot11_frame_header));
		offset = tag_offset = rt_header_len + sizeof(struct dot11_frame_header) + sizeof(struct beacon_management_frame);
		
		tag_len = len - tag_offset;
		tag_data = (const unsigned char *) (packet + tag_offset);

		if((end_le16toh(beacon->capability) & CAPABILITY_WEP) == CAPABILITY_WEP)
		{
			enc = WEP;

			tag_data = parse_ie_data(tag_data, tag_len, (uint8_t) RSN_TAG_NUMBER, &vlen, &voff);
			if(tag_data && vlen > 0)
			{
				enc = WPA;
				free((void *) tag_data);
			}
			else
			{
				while(offset < len)
				{
					tag_len = len - offset;
					tag_data = (const unsigned char *) (packet + offset);

					tag_data = parse_ie_data(tag_data, tag_len, (uint8_t) VENDOR_SPECIFIC_TAG, &vlen, &voff);
					if(vlen > WPA_IE_ID_LEN)
					{
						if(memcmp(tag_data, WPA_IE_ID, WPA_IE_ID_LEN) == 0)
						{
							enc = WPA;
							break;
						}
						free((void *) tag_data);
					}

					offset = tag_offset + voff + vlen;
				}
			}
		}
	}

	return enc;
}

static int get_next_ie(const unsigned char *data, size_t len, size_t *currpos) {
	if(*currpos + 2 >= len) return 0;
	*currpos = *currpos + 2 + data[*currpos + 1];
	if(*currpos >= len) return 0;
	return 1;
}

/* Given the tagged parameter sets from a beacon packet, locate the AP's SSID and return its current channel number */
int parse_beacon_tags(const unsigned char *packet, size_t len)
{
	set_vendor(0, "\0\0\0");
	char *ssid = NULL;
	const unsigned char *tag_data = NULL;
	unsigned char *ie = NULL, *channel_data = NULL;
	size_t ie_len = 0, ie_offset = 0, tag_len = 0, tag_offset = 0;
	int channel = 0;
	struct radio_tap_header *rt_header = NULL;

	rt_header = (struct radio_tap_header *) radio_header(packet, len);
	tag_offset = end_le16toh(rt_header->len) + sizeof(struct dot11_frame_header) + sizeof(struct beacon_management_frame);

	if(tag_offset < len)
	{
		tag_len = (len - tag_offset); /* this actually denotes length of the entire tag data area */
		tag_data = (const unsigned char *) (packet + tag_offset);

		/* If no SSID was manually specified, parse and save the AP SSID */
		if(get_ssid() == NULL)
		{
			ie = parse_ie_data(tag_data, tag_len, (uint8_t) SSID_TAG_NUMBER, &ie_len, &ie_offset);
			if(ie)
			{
				/* Return data is not null terminated; allocate ie_len+1 and memcpy string */
				ssid = malloc(ie_len+1);
				if(ssid)
				{
					memset(ssid, 0, (ie_len+1));
					memcpy(ssid, ie, ie_len);
					set_ssid(ssid);
					free(ssid);
				}

				free(ie);
			}
		}

		ie = parse_ie_data(tag_data, tag_len, HT_CAPS_TAG_NUMBER, &ie_len, &ie_offset);
		if(ie)
		{
			set_ap_htcaps(ie, ie_len);
			free(ie);
		}

		ie = parse_ie_data(tag_data, tag_len, (uint8_t) RATES_TAG_NUMBER, &ie_len, &ie_offset);
		if(ie)
		{
			set_ap_rates(ie, ie_len);
			free(ie);
		}

		ie = parse_ie_data(tag_data, tag_len, (uint8_t) ERATES_TAG_NUMBER, &ie_len, &ie_offset);
		if(ie)
		{
			set_ap_ext_rates(ie, ie_len);
			free(ie);
		}

		channel_data = parse_ie_data(tag_data, tag_len, (uint8_t) CHANNEL_TAG_NUMBER, &ie_len, &ie_offset);
		if(channel_data)
		{
			if(ie_len  == 1)
			{
				channel = *(uint8_t*)channel_data;
			}
			free(channel_data);
		}

		size_t ie_iterator = 0;
		do {
			const unsigned char *tag = tag_data + ie_iterator;
			// check for the length of the tag, and that its not microsoft
			if(tag[0] == VENDOR_SPECIFIC_TAG &&
			   ie_iterator+2+3 < tag_len &&
			   ((tag[1] < 11 && memcmp(tag+2, "\x00\x14\x6c", 3) && memcmp(tag+2, "\x00\x50\xf2", 3)) ||
			    (tag[1] == 30 && !(memcmp(tag+2, "\x00\x26\x86", 3))))) {
				set_vendor(1, tag + 2);
				break;
			}

		} while(get_next_ie(tag_data, tag_len, &ie_iterator));
	}

	return channel;
}

/* Gets the data for a given IE inside a tagged parameter list */
unsigned char *parse_ie_data(const unsigned char *data, size_t len, uint8_t tag_number, size_t *ie_len, size_t *ie_offset)
{
	unsigned char *tag_data = NULL;
        int offset = 0, tag_size = 0;
        struct tagged_parameter *tag = NULL;

        tag_size = sizeof(struct tagged_parameter);
	*ie_len = 0;
	*ie_offset = 0;

        while((offset + tag_size) < len)
        {
                tag = (struct tagged_parameter *) (data + offset);
                /* Check for the tag number and a sane tag length value */
                if((tag->number == tag_number) &&
                   (tag->len <= (len - offset - tag_size))
                )
                {
                        tag_data = malloc(tag->len);
                        if(tag_data)
                        {
                                memset(tag_data, 0, (tag->len));
                                memcpy(tag_data, (data + offset + tag_size), tag->len);
				*ie_len = tag->len;
				*ie_offset = offset;
                        }
                        break;
                }

                offset += (tag_size + tag->len);
        }

        return tag_data;
}

/* Validates a packet's reported FCS value */
static int check_fcs(const unsigned char *packet, size_t len)
{
	if(!has_rt_header()) return 1;

	uint32_t offset = 0, match = 0;
	uint32_t fcs = 0, fcs_calc = 0;
	struct radio_tap_header *rt_header = NULL;

	if(len > 4)
	{

		/* FCS is not calculated over the radio tap header */
		if(len >= sizeof(*rt_header))
		{
			uint32_t presentflags, flags;
			/* only check FCS if the flag IEEE80211_RADIOTAP_F_FCS is set in
			   in IEEE80211_RADIOTAP_FLAGS. the packets we generate ourselves
			   do not have any of these flags set and would cause false positives
			*/
			if(!rt_get_presentflags(packet, len, &presentflags, &offset))
				return 1;
			if(!(presentflags & (1U << IEEE80211_RADIOTAP_FLAGS)))
				return 1;
			offset = rt_get_flag_offset(presentflags, IEEE80211_RADIOTAP_FLAGS, offset);
			if(offset < len) {
				memcpy(&flags, packet + offset, 4);
				flags = end_le32toh(flags);
				if(flags & IEEE80211_RADIOTAP_F_BADFCS)
					return 0;
				if(!(flags & IEEE80211_RADIOTAP_F_FCS))
					return 1;
			}

			rt_header = (struct radio_tap_header *) packet;
			offset = end_le16toh(rt_header->len);

		}

		/* Get the packet's reported FCS (last 4 bytes of the packet) */
		fcs = end_le32toh(*(uint32_t*)(packet + (len-4)));


		if(len > offset)
		{
			/* FCS is the inverse of the CRC32 checksum of the data packet minus the frame's FCS and radio tap header (if any) */
			fcs_calc = ~crc32((char *) packet+offset, (len-offset-4));

			if(fcs_calc == fcs)
			{
				match = 1;
			}
		}
	}

	return match;

}

/* Checks a given BSSID to see if it's on our target list */
int is_target(const struct dot11_frame_header *frame_header)
{
	return !memcmp(frame_header->addr3, get_bssid(), MAC_ADDR_LEN);
}

/* Make best guess to determine if a radio tap header is present */
int has_rt_header(void)
{
        int yn = 0;

	if(pcap_datalink(get_handle()) == DLT_IEEE802_11_RADIO)
	{
		yn = 1;
	}

        return yn;
}

/* 
 * Returns a pointer to the radio tap header. If there is no radio tap header,
 * it returns a pointer to a dummy radio tap header.
 */
#define FAKE_RADIO_TAP_HEADER "\0\0\0\0" "\0\0\0\0" "\0\0\0\0" "\0\0\0\0" "\0\0\0\0"
unsigned char *radio_header(const unsigned char *packet, size_t len)
{
        if(len >= 8 && has_rt_header())
        {
                return (void*)packet;
        }
        else
        {
                return FAKE_RADIO_TAP_HEADER;
        }

}
