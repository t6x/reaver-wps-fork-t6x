/*
 * Reaver - Misc functions
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

#include "misc.h"

/* Converts a raw MAC address to a colon-delimited string */
char *mac2str(unsigned char *mac, char delim)
{
	char nyu[6*3];
#define PAT "%.2X%c"
#define PRT(X) mac[X], delim
#define PBT "%.2X"
	if(delim)
		snprintf(nyu, sizeof nyu, PAT PAT PAT PAT PAT PBT, PRT(0), PRT(1), PRT(2), PRT(3), PRT(4), mac[5]);
	else
		snprintf(nyu, sizeof nyu, PBT PBT PBT PBT PBT PBT, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return strdup(nyu);
}

/* Converts a colon-delimited string to a raw MAC address */
void str2mac(char *str, unsigned char *mac)
{
	char *delim_ptr = NULL, *num_ptr = NULL, *tmp_str = NULL;
	char delim = ':';
	int count = 0;

	tmp_str = strdup(str);
	delim_ptr = num_ptr = tmp_str;

	while((delim_ptr = strchr(delim_ptr, delim)) && count < (MAC_ADDR_LEN-1))
	{
		memset(delim_ptr, 0, 1);
		mac[count] = strtol(num_ptr, NULL, 16);
		delim_ptr++;
		count++;
		num_ptr = delim_ptr;
	}
	mac[count] = strtol(num_ptr, NULL, 16);
	
	free(tmp_str);
	return;
}

/* Conditional printf wrapper */
void cprintf(enum debug_level level, const char *fmt, ...)
{
	va_list arg;

	if(level <= get_debug())
	{
		va_start(arg, fmt);
		vfprintf(get_log_file(), fmt, arg);
		va_end(arg);
	}

	fflush(get_log_file());
}

/* Closes libpcap during sleep period to avoid stale packet data in pcap buffer */
void pcap_sleep(int seconds)
{
	if(seconds > 0)
	{
		pcap_close(get_handle());
		set_handle(NULL);
		sleep(seconds);
        	set_handle(capture_init(get_iface()));

		if(!get_handle())
		{
			cprintf(CRITICAL, "[-] Failed to re-initialize interface '%s'\n", get_iface());
		}
	}
}

