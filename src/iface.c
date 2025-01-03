/*
 * Reaver - Wireless interface functions
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
#include "globule.h"
#include <net/if.h>
#include <netinet/in.h>
#ifdef LIBNL3
#include <net/ethernet.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>
#else
#include "lwe/iwlib.h"
#endif
#include <sys/ioctl.h>
#include <stdlib.h>

#if defined(__FreeBSD__) || defined(__APPLE__)
#include <ifaddrs.h>
#include <net/if_dl.h>
int read_iface_mac() {
	struct ifaddrs* iflist;
	int found = 0;
	if (getifaddrs(&iflist) == 0) {
		struct ifaddrs* cur;
		for (cur = iflist; cur; cur = cur->ifa_next) {
			if ((cur->ifa_addr->sa_family == AF_LINK) &&
				(strcmp(cur->ifa_name, get_iface()) == 0) &&
				cur->ifa_addr) {
				struct sockaddr_dl* sdl = (struct sockaddr_dl*)cur->ifa_addr;
				set_mac(LLADDR(sdl));
				found = 1;
				break;
			}
		}
		freeifaddrs(iflist);
	}
	return found;
}
#else
/* Populates globule->mac with the MAC address of the interface globule->iface */
int read_iface_mac()
{
	struct ifreq ifr;
	struct ether_addr *eth = NULL;
	int sock = 0, ret_val = 0;

	/* Need a socket for the ioctl call */
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(sock != -1)
	{
		eth = malloc(sizeof(struct ether_addr));
		if(eth)
		{
			memset(eth, 0, sizeof(struct ether_addr));

			/* Prepare request */
			memset(&ifr, 0, sizeof(struct ifreq));
			strncpy(ifr.ifr_name, get_iface(), IFNAMSIZ);

			/* Do it */
			if(ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
			{
				set_mac((unsigned char *) &ifr.ifr_hwaddr.sa_data);
				ret_val = 1;
			}

			free(eth);
		}

		close(sock);
	}

	return ret_val;
}
#endif

/*
 * Goes to the next 802.11 channel.
 * This is mostly required for APs that hop channels, which usually hop between channels 1, 6, and 11.
 * We just hop channels until we successfully associate with the AP.
 * The AP's actual channel number is parsed and set by parse_beacon_tags() in 80211.c.
 */
int next_channel()
{
#define BG_CHANNELS	14, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
#define AN_CHANNELS	16, 34, 36, 38, 40, 42, 44, 46, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, 183, 184, 185, 187, 188, 189, 192, 196

        static int i;
        static const short bg_channels[] = {BG_CHANNELS};
	static const short an_channels[] = {AN_CHANNELS};
	static const short bgan_channels[] = {BG_CHANNELS, AN_CHANNELS};
	static const int band_chan_count[] = {
		[BG_BAND] = sizeof(bg_channels)/sizeof(bg_channels[0]),
		[AN_BAND] = sizeof(an_channels)/sizeof(an_channels[0]),
		[BG_BAND|AN_BAND] = sizeof(bgan_channels)/sizeof(bgan_channels[0]),
	};
	static const short* band_select[] = {
		[BG_BAND] = bg_channels,
		[AN_BAND] = an_channels,
		[BG_BAND|AN_BAND] = bgan_channels,
	};

	int band = get_wifi_band();
	const short *channels = band_select[band];
	int n = band_chan_count[band];

	/* Only switch channels if fixed channel operation is disabled */
	if(!get_fixed_channel())
	{
		i++;
		if((i >= n) || i < 0) i = 0;
		return change_channel(channels[i]);
	}

	return 0;
}

/* Sets the 802.11 channel for the selected interface */
#ifdef __APPLE__
int change_channel(int channel)
{
	cprintf(VERBOSE, "[+] Switching %s to channel %d\n", get_iface(), channel);
	// Unfortunately, there is no API to change the channel
	pid_t pid = fork();
	if (!pid) {
		char chan_arg[32];
		sprintf(chan_arg, "-c%d", channel);
		char* argv[] = {"/System/Library/PrivateFrameworks/Apple80211.framework/Resources/airport", chan_arg, NULL};
		execve("/System/Library/PrivateFrameworks/Apple80211.framework/Resources/airport", argv, NULL);
	}
	int status;
	waitpid(pid,&status,0);
	set_channel(channel);
	return 0;
}
#else
#ifdef LIBNL3
/* took from the Aircrack-ng */
static int ieee80211_channel_to_frequency(int chan)
{
	if (chan < 14) return 2407 + chan * 5;

	if (chan == 14) return 2484;

	/* FIXME: dot11ChannelStartingFactor (802.11-2007 17.3.8.3.2) */
	return (chan + 1000) * 5;
}

int change_channel(int channel)
{
	int skfd = 0, ret_val = 0;
	unsigned int freq;

	cprintf(VERBOSE, "[+] Switching %s to channel %d\n", get_iface(), channel);

/* Modified example from the stackoverflow probably inspired by the Aircrack-ng code */
/* https://stackoverflow.com/questions/21846965/set-wireless-channel-using-netlink-api */
	freq = ieee80211_channel_to_frequency(channel);
	/* Create the socket and connect to it. */
	struct nl_sock *sckt = nl_socket_alloc();
	genl_connect(sckt);

	/* Allocate a new message. */
	struct nl_msg *mesg = nlmsg_alloc();

	/* Check /usr/include/linux/nl80211.h for a list of commands and attributes. */
	enum nl80211_commands command = NL80211_CMD_SET_WIPHY;

	/* Create the message so it will send a command to the nl80211 interface. */
	genlmsg_put(mesg, 0, 0, genl_ctrl_resolve(sckt, "nl80211"), 0, 0, command, 0);

	/* Add specific attributes to change the frequency of the device. */
	NLA_PUT_U32(mesg, NL80211_ATTR_IFINDEX, if_nametoindex(get_iface()));
	NLA_PUT_U32(mesg, NL80211_ATTR_WIPHY_FREQ, freq);

	/* Finally send it and receive the amount of bytes sent. */
	int ret = nl_send_auto_complete(sckt, mesg);

	ret_val = 1;

nla_put_failure:
	nlmsg_free(mesg);

	return ret_val;
}
#else // !LIBNL3
int change_channel(int channel)
{
        int skfd = 0, ret_val = 0;
        struct iwreq wrq;

        memset((void *) &wrq, 0, sizeof(struct iwreq));

        /* Open NET socket */
        if((skfd = iw_sockets_open()) < 0)
        {
                perror("iw_sockets_open");
        }
        else if(get_iface())
        {
                /* Convert channel to a frequency */
                iw_float2freq((double) channel, &(wrq.u.freq));

                /* Fixed frequency */
                wrq.u.freq.flags = IW_FREQ_FIXED;

        	cprintf(VERBOSE, "[+] Switching %s to channel %d\n", get_iface(), channel);

                /* Set frequency */
                if(iw_set_ext(skfd, get_iface(), SIOCSIWFREQ, &wrq) >= 0)
                {
			set_channel(channel);
                        ret_val = 1;
                }

                iw_sockets_close(skfd);
        }

        return ret_val;
}
#endif // LIBNL3
#endif
