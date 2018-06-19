/*
 * Wash - Main and usage functions
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

#include "wpsmon.h"
#include "utils/file.h"
#include "utils/vendor.h"
#include "send.h"

#define MAX_APS 512

extern const char* get_version(void);
static void wash_usage(char *prog);

int show_all_aps = 0;
int json_mode = 0;
int show_utf8_ssid = 0;

static struct mac {
	unsigned char mac[6];
	unsigned char vendor_oui[1+3];
	unsigned char probes;
	unsigned char flags;
} seen_list[MAX_APS];
enum seen_flags {
	SEEN_FLAG_PRINTED = 1,
	SEEN_FLAG_COMPLETE = 2,
};
static unsigned seen_count;
static int list_insert(char *bssid) {
	unsigned i;
	unsigned char mac[6];
	str2mac(bssid, mac);
	for(i=0; i<seen_count; i++)
		if(!memcmp(seen_list[i].mac, mac, 6)) return i;
	if(seen_count >= MAX_APS) return -1;
	memcpy(seen_list[seen_count].mac, mac, 6);
	return seen_count++;
}
static int was_printed(char* bssid) {
	int x = list_insert(bssid);
	if(x >= 0 && x < MAX_APS) {
		unsigned f = seen_list[x].flags;
		seen_list[x].flags |= SEEN_FLAG_PRINTED;
		return f & SEEN_FLAG_PRINTED;
	}
	return 1;
}
static void mark_ap_complete(char *bssid) {
	int x = list_insert(bssid);
	if(x >= 0 && x < MAX_APS) seen_list[x].flags |= SEEN_FLAG_COMPLETE;
}
static int is_done(char *bssid) {
	int x = list_insert(bssid);
	if(x >= 0 && x < MAX_APS) return seen_list[x].flags & SEEN_FLAG_COMPLETE;
	return 1;
}
static int should_probe(char *bssid) {
	int x = list_insert(bssid);
        if(x >= 0 && x < MAX_APS) return seen_list[x].probes < get_max_num_probes();
	return 0;
}
static void update_probe_count(char *bssid) {
	int x = list_insert(bssid);
	if(x >= 0 && x < MAX_APS) seen_list[x].probes++;
}
static void set_ap_vendor(char *bssid) {
	int x = list_insert(bssid);
	if(x >= 0 && x < MAX_APS) memcpy(seen_list[x].vendor_oui, globule->vendor_oui, sizeof(seen_list[x].vendor_oui));
}
static unsigned char *get_ap_vendor(char* bssid) {
	int x = list_insert(bssid);
	if(x >= 0 && x < MAX_APS && seen_list[x].vendor_oui[0])
		return seen_list[x].vendor_oui+1;
	return 0;
}

static volatile int got_sigint;
static void sigint_handler(int x) {
	(void) x;
	got_sigint = 1;
	pcap_breakloop(get_handle());
}

int wash_main(int argc, char *argv[])
{
	int c = 0;
	FILE *fp = NULL;
	int long_opt_index = 0, i = 0, channel = 0, passive = 0, mode = 0;
	int source = INTERFACE, ret_val = EXIT_FAILURE;
	struct bpf_program bpf = { 0 };
	char *last_optarg = NULL, *target = NULL, *bssid = NULL;
	char *short_options = "i:c:n:b:25sfuFDhajU";
        struct option long_options[] = {
		{ "bssid", required_argument, NULL, 'b' },
                { "interface", required_argument, NULL, 'i' },
                { "channel", required_argument, NULL, 'c' },
		{ "probes", required_argument, NULL, 'n' },
		{ "file", no_argument, NULL, 'f' },
		{ "ignore-fcs", no_argument, NULL, 'F' },
		{ "2ghz", no_argument, NULL, '2' },
		{ "5ghz", no_argument, NULL, '5' },
		{ "scan", no_argument, NULL, 's' },
		{ "survey", no_argument, NULL, 'u' },
		{ "all", no_argument, NULL, 'a' },
		{ "json", no_argument, NULL, 'j' },
		{ "utf8", no_argument, NULL, 'U' },
                { "help", no_argument, NULL, 'h' },
                { 0, 0, 0, 0 }
        };

	globule_init();
	set_auto_channel_select(0);
	set_wifi_band(0);
	set_debug(INFO);
	set_validate_fcs(1);
	/* send all warnings, etc to stderr */
	set_log_file(stderr);
	set_max_num_probes(DEFAULT_MAX_NUM_PROBES);

	setvbuf(stdout, 0, _IONBF, 0);
	setvbuf(stderr, 0, _IONBF, 0);

	while((c = getopt_long(argc, argv, short_options, long_options, &long_opt_index)) != -1)
        {
                switch(c)
                {
			case 'f':
				source = PCAP_FILE;
				break;
			case 'i':
				set_iface(optarg);
				break;
			case 'b':
				bssid = strdup(optarg);
				break;
			case 'c':
				channel = atoi(optarg);
				set_fixed_channel(1);
				break;
			case '5':
				set_wifi_band(get_wifi_band() | AN_BAND);
				break;
			case '2':
				set_wifi_band(get_wifi_band() | BG_BAND);
				break;
			case 'n':
				set_max_num_probes(atoi(optarg));
				break;
			case 'j':
				json_mode = 1;
				break;
			case 's':
				mode = SCAN;
				break;
			case 'u':
				mode = SURVEY;
				break;
			case 'F':
				set_validate_fcs(0);
				break;
			case 'a':
				show_all_aps = 1;
				break;
			case 'U':
				show_utf8_ssid = 1;
				break;
			default:
				wash_usage(argv[0]);
				goto end;
		}

		/* Track the last optarg. This is used later when looping back through any specified pcap files. */
		if(optarg)
		{
			if(last_optarg)
			{
				free(last_optarg);
			}

			last_optarg = strdup(optarg);
		}
	}

	if(get_wifi_band() == 0) set_wifi_band(BG_BAND);

	/* The interface value won't be set if capture files were specified; else, there should have been an interface specified */
	if(!get_iface() && source != PCAP_FILE)
	{
		wash_usage(argv[0]);
		goto end;
	}
	else if(get_iface())
	{
		/* Get the MAC address of the specified interface */
		read_iface_mac();
	}

	if(get_iface() && source == PCAP_FILE)
	{
		cprintf(CRITICAL, "[X] ERROR: -i and -f options cannot be used together.\n");
		wash_usage(argv[0]);
		goto end;
	}

	/* If we're reading from a file, be sure we don't try to transmit probe requests */
	if(source == PCAP_FILE)
	{
		passive = 1;
	}

	/* 
	 * Loop through all of the specified capture sources. If an interface was specified, this will only loop once and the
	 * call to monitor() will block indefinitely. If capture files were specified, this will loop through each file specified
	 * on the command line and monitor() will return after each file has been processed.
	 */
	for(i=argc-1; i>0; i--)
	{
		/* If the source is a pcap file, get the file name from the command line */
		if(source == PCAP_FILE)
		{
			/* If we've gotten to the arguments, we're done */
			if((argv[i][0] == '-') ||
			   (last_optarg && (memcmp(argv[i], last_optarg, strlen(last_optarg)) == 0))
			)
			{
				break;
			}
			else
			{
				target = argv[i];
			}
		}
		/* Else, use the specified interface name */
		else
		{
			target = get_iface();
		}

		set_handle(capture_init(target));
		if(!get_handle())
		{
			cprintf(CRITICAL, "[X] ERROR: Failed to open '%s' for capturing\n", get_iface());
			goto end;
		}

		if(pcap_compile(get_handle(), &bpf, PACKET_FILTER, 0, 0) != 0)
		{
			cprintf(CRITICAL, "[X] ERROR: Failed to compile packet filter\n");
			cprintf(CRITICAL, "[X] PCAP: %s\n", pcap_geterr(get_handle()));
			goto end;
		}
		
		if(pcap_setfilter(get_handle(), &bpf) != 0)
		{
			cprintf(CRITICAL, "[X] ERROR: Failed to set packet filter\n");
			goto end;
		}

		/* Do it. */
		monitor(bssid, passive, source, channel, mode);
		printf("\n");
	}

	ret_val = EXIT_SUCCESS;

end:
	globule_deinit();
	if(bssid) free(bssid);
	if(wpsmon.fp) fclose(wpsmon.fp);
	return ret_val;
}

/* Monitors an interface (or capture file) for WPS data in beacon packets or probe responses */
void monitor(char *bssid, int passive, int source, int channel, int mode)
{
	struct sigaction act;
	struct itimerval timer;
	struct pcap_pkthdr header;
	static int header_printed;
        const u_char *packet = NULL;

        memset(&act, 0, sizeof(struct sigaction));
        memset(&timer, 0, sizeof(struct itimerval));

	/* If we aren't reading from a pcap file, set the interface channel */
	if(source == INTERFACE)
	{
		/* 
		 * If a channel has been specified, set the interface to that channel. 
		 * Else, set a recurring 1 second timer that will call sigalrm() and switch to 
		 * a new channel.
		 */
		if(channel > 0)
		{
			change_channel(channel);
		}
		else
		{
        		act.sa_handler = sigalrm_handler;
        		sigaction (SIGALRM, &act, 0);
			ualarm(CHANNEL_INTERVAL, CHANNEL_INTERVAL);
			int startchan = 1;
			if(get_wifi_band() == AN_BAND)
				startchan = 34;
			change_channel(startchan);
		}

		memset(&act, 0, sizeof(struct sigaction));
		sigaction (SIGINT, 0, &act);
		act.sa_flags &= ~SA_RESTART;
		act.sa_handler = sigint_handler;
		sigaction (SIGINT, &act, 0);

	}

	if(!header_printed)
	{
		if(!json_mode) {
			fprintf  (stdout, "BSSID               Ch  dBm  WPS  Lck  Vendor    ESSID\n");
			//fprintf(stdout, "00:11:22:33:44:55  104  -77  1.0  Yes  Bloatcom  0123456789abcdef0123456789abcdef\n");
			fprintf  (stdout, "--------------------------------------------------------------------------------\n");
		}
		header_printed = 1;
	}

	while(!got_sigint && (packet = next_packet(&header))) {
		parse_wps_settings(packet, &header, bssid, passive, mode, source);
		memset((void *) packet, 0, header.len);
	}

	return;
}

#define wps_active(W) (((W)->version) || ((W)->locked != 2) || ((W)->state))

void parse_wps_settings(const u_char *packet, struct pcap_pkthdr *header, char *target, int passive, int mode, int source)
{
	struct radio_tap_header *rt_header = NULL;
	struct dot11_frame_header *frame_header = NULL;
	struct libwps_data *wps = NULL;
	enum encryption_type encryption = NONE;
	char *bssid = NULL, *ssid = NULL, *lock_display = NULL;
	int wps_parsed = 0, probe_sent = 0, channel = 0, rssi = 0;
	static int channel_changed = 0;

	wps = malloc(sizeof(struct libwps_data));
	memset(wps, 0, sizeof(struct libwps_data));

	if(packet == NULL || header == NULL || header->len < MIN_BEACON_SIZE)
        {
                goto end;
        }

	rt_header = (struct radio_tap_header *) radio_header(packet, header->len);
	size_t rt_header_len = end_le16toh(rt_header->len);
	frame_header = (struct dot11_frame_header *) (packet + rt_header_len);

	/* If a specific BSSID was specified, only parse packets from that BSSID */
	if(!is_target(frame_header))
	{
		goto end;
	}

	set_ssid(NULL);
	bssid = (char *) mac2str(frame_header->addr3, ':');
	set_bssid((unsigned char *) frame_header->addr3);

	if(bssid)
	{
		if((target == NULL) ||
		   (target != NULL && strcmp(bssid, target) == 0))
		{
			channel = parse_beacon_tags(packet, header->len);
			if(channel == 0) {
				// It seems 5 GHz APs do not set channel tags.
				// FIXME: get channel by parsing radiotap header
				channel = get_channel();
			}
			rssi = signal_strength(packet, header->len);
			ssid = (char *) get_ssid();

			if(target != NULL && channel_changed == 0)
			{
				ualarm(0, 0);
				change_channel(channel);
				channel_changed = 1;
			}

			unsigned fsub_type = frame_header->fc & end_htole16(IEEE80211_FCTL_STYPE);

			int is_beacon = fsub_type == end_htole16(IEEE80211_STYPE_BEACON);
			int is_probe_resp = fsub_type == end_htole16(IEEE80211_STYPE_PROBE_RESP);

			if(is_probe_resp || is_beacon) {
				wps_parsed = parse_wps_parameters(packet, header->len, wps);
				if(is_beacon || !get_ap_vendor(bssid)) set_ap_vendor(bssid);
			}
			if(!is_done(bssid) && (get_channel() == channel || source == PCAP_FILE))
			{
				if(is_beacon && 
				   mode == SCAN && 
				   !passive && 
				   should_probe(bssid))
				{
					send_probe_request(get_bssid(), get_ssid());
					probe_sent = 1;
				}
		
				if(!json_mode && (!was_printed(bssid) && (wps_active(wps) || show_all_aps == 1)))
				{
					if(wps_active(wps)) switch(wps->locked)
					{
						case WPSLOCKED:
							lock_display = YES;
							break;
						case UNLOCKED:
						case UNSPECIFIED:
							lock_display = NO;
							break;
					} else lock_display = NO;

					char* vendor = get_vendor_string(get_ap_vendor(bssid));
					char* sane_ssid = sanitize_string(ssid);

					if(show_utf8_ssid && verifyssid(ssid))
						strcpy(sane_ssid,ssid);

					if(wps_active(wps))
						fprintf(stdout, "%17s  %3d  %.2d  %d.%d  %3s  %8s  %s\n", bssid, channel, rssi, (wps->version >> 4), (wps->version & 0x0F), lock_display, vendor ? vendor : "        ", sane_ssid);
					else
						fprintf(stdout, "%17s  %3d  %.2d            %8s  %s\n", bssid, channel, rssi, vendor ? vendor : "        ", sane_ssid);
					free(sane_ssid);
				}

				if(probe_sent)
				{
					update_probe_count(bssid);
				}

				/* 
				 * If there was no WPS information, then the AP does not support WPS and we should ignore it from here on.
				 * If this was a probe response, then we've gotten all WPS info we can get from this AP and should ignore it from here on.
				 */
				if(!wps_parsed || is_probe_resp)
				{
					mark_ap_complete(bssid);
					if(json_mode && (show_all_aps || wps_active(wps))) {
						char *json_string = wps_data_to_json(bssid, ssid, channel, rssi, get_ap_vendor(bssid), wps);
						fprintf(stdout, "%s\n", json_string);
						fflush(stdout);
						free(json_string);
					}
				}
	
			}
		}

		free(bssid);
		bssid = NULL;
	}

end:
	if(wps) free(wps);
	set_bssid((unsigned char *) NULL_MAC);

	return;
}

/* Does what it says */
void send_probe_request(unsigned char *bssid, char *essid)
{
	const void *probe = NULL;
	size_t probe_size = 0;

	probe = build_wps_probe_request(bssid, essid, &probe_size);
	if(probe)
	{
		send_packet(probe, probe_size, 0);
		free((void *) probe);
	}

	return;
}

/* Whenever a SIGALRM is thrown, go to the next 802.11 channel */
void sigalrm_handler(int x)
{
	next_channel();
}

static void print_header(void) {
	fprintf(stderr, "\nWash v%s WiFi Protected Setup Scan Tool\n", get_version());
        fprintf(stderr, "Copyright (c) 2011, Tactical Network Solutions, Craig Heffner\n\n");
}

static void wash_usage(char *prog)
{
	print_header();

	fprintf(stderr, "Required Arguments:\n");
	fprintf(stderr, "\t-i, --interface=<iface>              Interface to capture packets on\n");
	fprintf(stderr, "\t-f, --file [FILE1 FILE2 FILE3 ...]   Read packets from capture files\n");

	fprintf(stderr, "\nOptional Arguments:\n");
	fprintf(stderr, "\t-c, --channel=<num>                  Channel to listen on [auto]\n");
	fprintf(stderr, "\t-n, --probes=<num>                   Maximum number of probes to send to each AP in scan mode [%d]\n", DEFAULT_MAX_NUM_PROBES);
	fprintf(stderr, "\t-F, --ignore-fcs                     Ignore frame checksum errors\n");
	fprintf(stderr, "\t-2, --2ghz                           Use 2.4GHz 802.11 channels\n");
	fprintf(stderr, "\t-5, --5ghz                           Use 5GHz 802.11 channels\n");
	fprintf(stderr, "\t-s, --scan                           Use scan mode\n");
	fprintf(stderr, "\t-u, --survey                         Use survey mode [default]\n");
	fprintf(stderr, "\t-a, --all                            Show all APs, even those without WPS\n");
	fprintf(stderr, "\t-j, --json                           print extended WPS info as json\n");
	fprintf(stderr, "\t-U, --utf8                           Show UTF8 ESSID (does not sanitize ESSID, dangerous)\n");
	fprintf(stderr, "\t-h, --help                           Show help\n");
	
	fprintf(stderr, "\nExample:\n");
	fprintf(stderr, "\t%s -i wlan0mon\n\n", prog);

	return;
}
