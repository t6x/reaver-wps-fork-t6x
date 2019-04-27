/*
 * Reaver - Packet building functions
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

#include "builder.h"
#include <assert.h>

size_t build_radio_tap_header(void *rt_header)
{
#ifdef RADIOTAP_HEADER_WITH_RATE
	#define RADIOTAP_HEADER_LENGTH \
	"\x0c\0" /* header length */
	#define RADIOTAP_HEADER_PRESENT_FLAGS \
	"\x04\x80\0\0" /* present flags: rate & tx flags */
	#define RADIOTAP_HEADER_RATE_OPTION \
	"\0\0" /* rate, padding */
#else
	#define RADIOTAP_HEADER_LENGTH \
	"\x0a\0" /* header length */
	#define RADIOTAP_HEADER_PRESENT_FLAGS \
	"\x00\x80\0\0" /* present flags: tx flags */
	#define RADIOTAP_HEADER_RATE_OPTION ""
#endif

	#define RADIOTAP_HEADER \
	"\0\0" /* version */ \
	RADIOTAP_HEADER_LENGTH \
	RADIOTAP_HEADER_PRESENT_FLAGS \
	RADIOTAP_HEADER_RATE_OPTION \
	"\x18\0" /* TX flags: F_TX_NOACK | F_TX_NOSEQ */

	memcpy(rt_header, RADIOTAP_HEADER, sizeof(RADIOTAP_HEADER)-1);
	return sizeof(RADIOTAP_HEADER)-1;
}

static size_t build_dot11_frame_header_m(struct dot11_frame_header *fh, uint16_t fc, unsigned char dstmac[6])
{
	static uint16_t frag_seq;

	fh->duration = end_htole16(52); /* DEFAULT_DURATION */
	fh->fc = end_htole16(fc);
	fh->frag_seq = end_htole16(frag_seq);

	memcpy(fh->addr1, dstmac, MAC_ADDR_LEN);
	memcpy(fh->addr2, get_mac(), MAC_ADDR_LEN);
	memcpy(fh->addr3, dstmac, MAC_ADDR_LEN);

	frag_seq += 0x10; /* SEQ_MASK */

	return sizeof *fh;
}

size_t build_dot11_frame_header(struct dot11_frame_header *fh, uint16_t fc) {
	return build_dot11_frame_header_m(fh, fc, get_bssid());
}

size_t build_dot11_frame_header_broadcast(struct dot11_frame_header *fh, uint16_t fc) {
	return build_dot11_frame_header_m(fh, fc, "\xff\xff\xff\xff\xff\xff");
}

size_t build_authentication_management_frame(struct authentication_management_frame *f)
{
	f->algorithm = end_htole16(OPEN_SYSTEM);
	f->sequence = end_htole16(1);
	f->status = 0;

	return sizeof *f;
}

size_t build_association_management_frame(struct association_request_management_frame *f)
{
	f->capability = end_htole16(get_ap_capability());
	f->listen_interval = end_htole16(LISTEN_INTERVAL);

	return sizeof *f;
}

static size_t build_llc_header(struct llc_header *h)
{
	h->dsap = LLC_SNAP;
	h->ssap = LLC_SNAP;
	h->control_field = UNNUMBERED_FRAME;
	h->org_code[0] = 0;
	h->org_code[1] = 0;
	h->org_code[2] = 0;
	h->type = end_htobe16(DOT1X_AUTHENTICATION);

	return sizeof *h;
}

void *build_wps_probe_request(unsigned char *bssid, char *essid, size_t *len)
{
	// TODO: one might actually first (or only) try to send broadcast probes.
	// the only 2 differences are that in build_dot11_frame_header instead of the
	// target bssid the special mac address ff:ff:ff:ff:ff:ff is used,
	// and the SSID tag is always "\x00\x00"
	// sending only broadcast probes would at least make the operation much more
	// stealthy! the directed probes basically only make sense when sent to *hidden*
	// ESSIDs, after finding out their real ESSID by watching other client's probes.

	struct tagged_parameter ssid_tag = { 0 };
	void *packet = NULL;
	size_t offset = 0, rt_len = 0, dot11_len = 0, ssid_tag_len = 0, packet_len = 0;
	int broadcast = !memcmp(bssid, "\xff\xff\xff\xff\xff\xff", 6);

	if(!broadcast && essid != NULL)
	{
		ssid_tag.len = (uint8_t) strlen(essid);
	}
	else
	{
		ssid_tag.len = 0;
	}

	ssid_tag.number = SSID_TAG_NUMBER;
	ssid_tag_len = ssid_tag.len + sizeof(struct tagged_parameter);

	struct radio_tap_header rt_header;
	rt_len = build_radio_tap_header(&rt_header);
	struct dot11_frame_header dot11_header;
	dot11_len = build_dot11_frame_header_m(&dot11_header, FC_PROBE_REQUEST, bssid);

	packet_len = rt_len + dot11_len + ssid_tag_len;

	#define TAG_SUPPORTED_RATES "\x01\x08\x02\x04\x0b\x16\x0c\x12\x18\x24"
	#define TAG_EXT_RATES "\x32\x04\x30\x48\x60\x6c"
	// it seems some OS don't send this tag, so leave it away
	//#define TAG_DS_PARAM "\x03\x01\x07"
	#define TAG_HT_CAPS "\x2d\x1a\x72\x01\x13\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

	// maybe we should leave this away too, it is usually not sent, but the
	// AP responds with WPS info anyway
	#define WPS_PROBE_IE      "\xdd\x09\x00\x50\xf2\x04\x10\x4a\x00\x01\x10"

	#define ALL_TAGS TAG_SUPPORTED_RATES TAG_EXT_RATES TAG_HT_CAPS WPS_PROBE_IE

	packet_len += sizeof(ALL_TAGS) -1;

	packet = malloc(packet_len);

	if(packet)
	{
		memset((void *) packet, 0, packet_len);
		memcpy((void *) packet, &rt_header, rt_len);
		offset += rt_len;
		memcpy((void *) ((char *) packet+offset), &dot11_header, dot11_len);
		offset += dot11_len;
		memcpy((void *) ((char *) packet+offset), (void *) &ssid_tag, sizeof(ssid_tag));
		offset += sizeof(ssid_tag);
		memcpy((void *) ((char *) packet+offset), essid, ssid_tag.len);
		offset += ssid_tag.len;

		memcpy(packet+offset, ALL_TAGS, sizeof(ALL_TAGS) -1);
		offset += sizeof(ALL_TAGS) -1;

		*len = packet_len;
	}

	return packet;
}

/* Wrapper function for Radio Tap / Dot11 / LLC */
void *build_snap_packet(size_t *len)
{
	void *packet = NULL;
	size_t rt_len = 0, dot11_len = 0, llc_len = 0, packet_len = 0;
	struct radio_tap_header rt_header;
	struct dot11_frame_header dot11_header;
	struct llc_header llc_header;

	rt_len = build_radio_tap_header(&rt_header);
        dot11_len = build_dot11_frame_header(&dot11_header, FC_STANDARD);
        llc_len = build_llc_header(&llc_header);

	packet_len = rt_len + dot11_len + llc_len;
	packet = malloc(packet_len);

	if(packet)
	{
		memset((void *) packet, 0, packet_len);
		memcpy((void *) packet, &rt_header, rt_len);
		memcpy((void *) ((char *) packet+rt_len), &dot11_header, dot11_len);
		memcpy((void *) ((char *) packet+rt_len+dot11_len), &llc_header, llc_len);

		*len = packet_len;
	}

	return packet;
}

void *build_dot1X_header(uint8_t type, uint16_t payload_len, size_t *len)
{
	struct dot1X_header *header = NULL;
	void *buf = NULL;

	buf = malloc(sizeof(struct dot1X_header));
	if(buf)
	{
		*len = sizeof(struct dot1X_header);
		memset((void *) buf, 0, sizeof(struct dot1X_header));
		header = (struct dot1X_header *) buf;

		header->version = DOT1X_VERSION;
		header->type = type;
		header->len = htons(payload_len);
	}

	return buf;
}

void *build_eap_header(uint8_t id, uint8_t code, uint8_t type, uint16_t payload_len, size_t *len)
{
	struct eap_header *header = NULL;
	void *buf = NULL;

	buf = malloc(sizeof(struct eap_header));
	if(buf)
	{
		*len = sizeof(struct eap_header);
		memset((void *) buf, 0, sizeof(struct eap_header));
		header = (struct eap_header *) buf;
		
		header->code = code;
		header->id = id;
		header->len = htons((payload_len + *len));
		header->type = type;

		id++;
	}

	return buf;
}

void *build_wfa_header(uint8_t op_code, size_t *len)
{
	void *buf = NULL;
	struct wfa_expanded_header *header = NULL;

	buf = malloc(sizeof(struct wfa_expanded_header));
	if(buf)
	{
		*len = sizeof(struct wfa_expanded_header);
		memset((void *) buf, 0, *len);
		header = (struct wfa_expanded_header *) buf;
	
		memcpy(header->id, WFA_VENDOR_ID, sizeof(header->id));
		header->type = end_htobe32(SIMPLE_CONFIG);
		header->opcode = op_code;
	}
	
	return buf;
}

/* Wrapper for SNAP / Dot1X Start */
void *build_eapol_start_packet(size_t *len)
{
	void *snap_packet = NULL, *dot1x_header = NULL, *packet = NULL;
        size_t snap_len = 0, dot1x_len = 0, packet_len = 0;

        /* Build a SNAP packet and a 802.1X START header */
        snap_packet = build_snap_packet(&snap_len);
        dot1x_header = build_dot1X_header(DOT1X_START, 0, &dot1x_len);

	if(snap_packet && dot1x_header)
	{
        	packet_len = snap_len + dot1x_len;
        	packet = malloc(packet_len);

        	if(packet)
        	{
        	        /* Build packet */
        	        memset((void *) packet, 0, packet_len);
        	        memcpy((void *) packet, snap_packet, snap_len);
        	        memcpy((void *) ((char *) packet+snap_len), dot1x_header, dot1x_len);

			*len = packet_len;
		}

		free((void *) snap_packet);
		free((void *) dot1x_header);
	}

	return packet;
}

/* Wrapper for SNAP / Dot1X / EAP / WFA / Payload */
void *build_eap_packet(const void *payload, uint16_t payload_len, size_t *len)
{
	void *buf = NULL, *snap_packet = NULL, *eap_header = NULL, *dot1x_header = NULL, *wfa_header = NULL;
	size_t buf_len = 0, snap_len = 0, eap_len = 0, dot1x_len = 0, wfa_len = 0, offset = 0, total_payload_len = 0;
	uint8_t eap_type = 0, eap_code = 0;
	struct wps_data *wps = get_wps();

	/* Decide what type of EAP packet to build based on the current WPS state */
	switch(wps->state)
	{
		case RECV_M1:
			eap_code = EAP_RESPONSE;
			eap_type = EAP_IDENTITY;
			break;
		default:
			eap_code = EAP_RESPONSE;
			eap_type = EAP_EXPANDED;
	}

	/* Total payload size may or may not be equal to payload_len depending on if we
	 * need to build and add a WFA header to the packet payload.
	 */
	total_payload_len = payload_len;

	/* If eap_type is Expanded, then we need to add a WFA header */
	if(eap_type == EAP_EXPANDED)
	{
		wfa_header = build_wfa_header(get_opcode(), &wfa_len);
		total_payload_len += wfa_len;
	}

	/* Build SNAP, EAP and 802.1x headers */
	snap_packet = build_snap_packet(&snap_len);
	eap_header = build_eap_header(get_eap_id(), eap_code, eap_type, total_payload_len, &eap_len);
	dot1x_header = build_dot1X_header(DOT1X_EAP_PACKET, (total_payload_len+eap_len), &dot1x_len);

	if(snap_packet && eap_header && dot1x_header)
	{
		buf_len = snap_len + dot1x_len + eap_len + total_payload_len;
		buf = malloc(buf_len);
		if(buf)
		{
			memset((void *) buf, 0, buf_len);

			/* Build the packet */
			memcpy((void *) buf, snap_packet, snap_len);
			offset += snap_len;
			memcpy((void *) ((char *) buf+offset), dot1x_header, dot1x_len);
			offset += dot1x_len;
			memcpy((void *) ((char *) buf+offset), eap_header, eap_len);
			offset += eap_len;
	
			if(eap_type == EAP_EXPANDED)
			{
				memcpy((void *) ((char *) buf+offset), wfa_header, wfa_len);
				offset += wfa_len;
			}

			if(payload && payload_len)
			{
				memcpy((void *) ((char *) buf+offset), payload, payload_len);
			}

			*len = (offset + payload_len);
		}

		free((void *) snap_packet);
		free((void *) eap_header);
		free((void *) dot1x_header);
		if(wfa_header) free((void *) wfa_header);
	}

	return buf;
}

void *build_eap_failure_packet(size_t *len)
{
	void *buf = NULL, *snap_packet = NULL, *eap_header = NULL, *dot1x_header = NULL;
	size_t buf_len = 0, snap_len = 0, eap_len = 0, dot1x_len = 0, offset = 0;

	/* Build SNAP, EAP and 802.1x headers */
        snap_packet = build_snap_packet(&snap_len);
        eap_header = build_eap_header(get_eap_id(), EAP_FAILURE, EAP_FAILURE, 0, &eap_len);
        dot1x_header = build_dot1X_header(DOT1X_EAP_PACKET, eap_len, &dot1x_len);

	buf_len = snap_len + eap_len + dot1x_len;

	if(snap_packet && eap_header && dot1x_header)
	{
		buf = malloc(buf_len);
		if(buf)
		{
			memset((void *) buf, 0, buf_len);
			
			memcpy((void *) buf, snap_packet, snap_len);
			offset += snap_len;
			memcpy((void *) ((char *) buf+offset), dot1x_header, dot1x_len);
			offset += dot1x_len;
			memcpy((void *) ((char *) buf+offset), eap_header, eap_len);

			*len = buf_len;
		}
	}

	if(snap_packet) free((void *) snap_packet);
	if(eap_header) free((void *) eap_header);
	if(dot1x_header) free((void *) dot1x_header);

	return buf;
}

static size_t build_tagged_parameter(struct tagged_parameter *tag, uint8_t number, uint8_t size)
{
	tag->number = number;
	tag->len = size;

	return sizeof *tag;
}

size_t build_ssid_tagged_parameter(unsigned char buf[IW_ESSID_MAX_SIZE+2], char *essid)
{
	struct tagged_parameter ssid_param;
	size_t ssid_len = strlen(essid), ssid_param_len;

	ssid_param_len = build_tagged_parameter(&ssid_param, SSID_TAG_NUMBER, ssid_len);
	assert(ssid_param_len == 2);
	assert(2 == sizeof (struct tagged_parameter));
	memcpy(buf, &ssid_param, sizeof ssid_param);
	memcpy(buf+2, essid, ssid_len);

	return 2 + ssid_len;
}

size_t build_wps_tagged_parameter(unsigned char buf[2+WPS_TAG_SIZE])
{
	size_t wps_param_len;
	struct tagged_parameter wps_param;

	wps_param_len = build_tagged_parameter(&wps_param, WPS_TAG_NUMBER, WPS_TAG_SIZE);
	assert(wps_param_len == 2);
	assert(2 == sizeof (struct tagged_parameter));

	memcpy(buf, &wps_param, sizeof wps_param);
	memcpy(buf+2, WPS_REGISTRAR_TAG, WPS_TAG_SIZE);

	return 2+WPS_TAG_SIZE;
}

size_t build_supported_rates_tagged_parameter(unsigned char *buf, size_t buflen)
{
	unsigned char *erates, *srates;
	int srates_tag_size, erates_tag_size;
        size_t i, len, srates_len, erates_len, offset = 0;

	struct tagged_parameter supported_rates, extended_rates;

	srates = get_ap_rates(&srates_tag_size);
	assert(sizeof(struct tagged_parameter) + srates_tag_size < buflen);

        srates_len = build_tagged_parameter(&supported_rates, SRATES_TAG_NUMBER, srates_tag_size);
	memcpy(buf, &supported_rates, srates_len);
	offset += srates_len;

	memcpy(buf+offset, srates, srates_tag_size);
	for(i=offset; i<offset+srates_tag_size; i++)
		buf[i] = buf[i] & 0x7f; // remove (B) bit

	offset += srates_tag_size;

	erates = get_ap_ext_rates(&erates_tag_size);
	erates_len = build_tagged_parameter(&extended_rates, ERATES_TAG_NUMBER, erates_tag_size);

	len = srates_len + erates_len + srates_tag_size + erates_tag_size;
	assert(len < buflen);

	memcpy(buf+offset, &extended_rates, erates_len);
	offset += erates_len;
	memcpy(buf+offset, erates, erates_tag_size);

	return len;
}

size_t build_htcaps_parameter(unsigned char *buf, size_t buflen)
{
	unsigned char* htcaps;
	int htlen; size_t taglen;
	if((htcaps = get_ap_htcaps(&htlen)) == NULL)
		return 0;
	struct tagged_parameter tag_htcaps;
	taglen = build_tagged_parameter(&tag_htcaps, HT_CAPS_TAG_NUMBER, htlen);
	if(taglen + htlen >= buflen)
		return 0; /* having HT caps is usually not critical, so better return nothing */

	memcpy(buf, &tag_htcaps, taglen);
	memcpy(buf + taglen, htcaps, htlen);
	return taglen + htlen;
}
