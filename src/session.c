/*
 * Reaver - Session save/restore functions
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

#include "session.h"

/* Does the configuration directory exist? Returns 1 for yes, 0 for no. */
static int configuration_directory_exists()
{
	struct stat dirstat;
	return (stat(CONF_DIR, &dirstat) == 0);
}

static void gen_sessionfile_name(const char* bssid, char* outbuf) {
#ifdef SAVETOCURRENT
	snprintf(outbuf, FILENAME_MAX, "%s.%s", bssid, CONF_EXT);
#else
	int cde = configuration_directory_exists();
	snprintf(outbuf, FILENAME_MAX, "%s%s%s.%s",
	         cde?CONF_DIR:"", cde?"/":"", bssid, CONF_EXT);
#endif
}

int restore_session()
{
	struct stat wpstat = { 0 };
	char line[MAX_LINE_SIZE] = { 0 };
	char temp[P1_READ_LEN] = { 0 };
	char file[FILENAME_MAX];
	char *bssid = NULL;
	char answer = 0;
	FILE *fp = NULL;
	int ret_val = 0, i = 0;

	/* 
	 * If a session file was explicitly specified, use that; else, check for the 
	 * default session file name for this BSSID.
	 */
	if(get_session())
	{
		strcpy(file, get_session());
	}
	else
	{
		bssid = mac2str(get_bssid(), '\0');
		gen_sessionfile_name(bssid, file);
		free(bssid);
	}

	/*
	 * If a session was explicitly specified, or if the auto detect option was specified,
	 * then the answer to any of the following questions will be 'yes'.
	 */
	if(get_session())
	{
		answer = 'y';
	}

	/*
	 * Do not restore the session when arbitrary string pin is specified.
	 */
	if(get_pin_string_mode())
	{
		answer = 'n';
	}

	if(stat(file, &wpstat) == 0)
	{
		/* If the user explicitly specified a session file, don't prompt them */
		if(answer == 0)
		{
			bssid = mac2str(get_bssid(), ':');

			/* Don't use cprintf here; else, if the output is sent to a file via -o, the user won't see this prompt. */
			fprintf(stderr, "[?] Restore previous session for %s? [n/Y] ", bssid);
			answer = getc(stdin);
			free(bssid);
		}
	
		if(answer == 'y' || answer == 'Y' || answer == '\n')
		{
			if((fp = fopen(file, "r")))
			{
				/* Get the key1 index value */
				if(fgets(line, MAX_LINE_SIZE, fp) != NULL)
				{
					set_p1_index(atoi(line));
					memset(line, 0, MAX_LINE_SIZE);
	
					/* Get the key2 index value */
					if(fgets(line, MAX_LINE_SIZE, fp) != NULL)
					{
						set_p2_index(atoi(line));
						memset(line, 0, MAX_LINE_SIZE);
				
						/* Get the key status value */
						if(fgets(line, MAX_LINE_SIZE, fp) != NULL)
						{
							set_key_status(atoi(line));

							/* Read in all p1 values */
							for(i=0; i<P1_SIZE; i++)
							{
								memset(temp, 0, P1_READ_LEN);

								if(fgets(temp, P1_READ_LEN, fp) != NULL)
								{
									/* NULL out the new line character */
									temp[P1_STR_LEN] = 0;
									set_p1(i, temp);
								}
							}

							/* Read in all p2 values */
							for(i=0; i<P2_SIZE; i++)
							{
								memset(temp, 0, P1_READ_LEN);

								if(fgets(temp, P2_READ_LEN, fp) != NULL)
								{
									/* NULL out the new line character */
									temp[P2_STR_LEN] = 0;
									set_p2(i, temp);
								}
							}

							ret_val = 1;
						}
					}
				}
		
				fclose(fp);
			}
			else
			{
				perror("fopen");
			}
		}
	}

	if(!ret_val)
	{
		set_p1_index(0);
		set_p2_index(0);
		set_key_status(KEY1_WIP);
	} else {
		cprintf(INFO, "[+] Restored previous session\n");
	}

	return ret_val;
}

int save_session()
{
	char *bssid = NULL;
	char *wpa_key = NULL, *essid = NULL, *pretty_bssid = NULL;
        char file_name[FILENAME_MAX] = { 0 };
        char line[MAX_LINE_SIZE] = { 0 };
        FILE *fp = NULL;
	size_t write_size = 0;
        int attempts = 0, ret_val = 0, i = 0;
	struct wps_data *wps = NULL;
	int pin_string;

	wps = get_wps();
	bssid = mac2str(get_bssid(), '\0');
	pretty_bssid = mac2str(get_bssid(), ':');
	pin_string = get_pin_string_mode();

	if(wps)
	{
		wpa_key = wps->key;
		essid = wps->essid;
	}
	
	if(!bssid || !pretty_bssid || pin_string)
	{
		if (pin_string)
		{
			cprintf(VERBOSE, "[*] String pin was specified, nothing to save.\n");
		}
		else
		{
			cprintf(CRITICAL, "[X] ERROR: Failed to save session data (memory error).\n");
		}
	}
	else
	{
		/* 
		 * If a session file was explicitly specified, use that; else, check for the 
		 * default session file name for this BSSID.
		 */
		if(get_session())
		{
			strcpy(file_name, get_session());
		}
		else
		{
			gen_sessionfile_name(bssid, file_name);
		}

		/* Don't bother saving anything if nothing has been done */
		if((get_p1_index() > 0) || (get_p2_index() > 0))
		{
			if((fp = fopen(file_name, "w")))
			{
				snprintf(line, MAX_LINE_SIZE, "%d\n", get_p1_index());
				write_size = strlen(line);

				/* Save key1 index value */
				if(fwrite(line, 1, write_size, fp) == write_size)
				{
					memset(line, 0, MAX_LINE_SIZE);
					snprintf(line, MAX_LINE_SIZE, "%d\n", get_p2_index());
					write_size = strlen(line);

					/* Save key2 index value */
					if(fwrite(line, 1, write_size, fp) == write_size)
					{
						memset(line, 0, MAX_LINE_SIZE);
        		                	snprintf(line, MAX_LINE_SIZE, "%d\n", get_key_status());
        		                	write_size = strlen(line);
	
						/* Save key status value */
						if(fwrite(line, 1, write_size, fp) == write_size)
						{
							/* Save all the p1 values */
							for(i=0; i<P1_SIZE; i++)
							{
								fwrite(get_p1(i), 1, strlen(get_p1(i)), fp);
								fwrite("\n", 1, 1, fp);
							}

							/* Save all the p2 values */
							for(i=0; i<P2_SIZE; i++)
							{
								fwrite(get_p2(i), 1, strlen(get_p2(i)), fp);
								fwrite("\n", 1, 1, fp);
							}

							/* If we have the WPA key, then we've exhausted all attempts, and the UI should reflect that */
							if(wpa_key && strlen(wpa_key) > 0)
							{
								attempts = P1_SIZE + P2_SIZE;
							}
							else
							{
								if(get_key_status() == KEY1_WIP)
								{
									attempts = get_p1_index() + get_p2_index();
								}
								else if(get_key_status() == KEY2_WIP)
								{
									attempts = P1_SIZE + get_p2_index();
								}
							}

							/* If we got an SSID from the WPS data, then use that; else, use whatever was used to associate with the AP */
							if(!essid || strlen(essid) == 0)
							{
								essid = get_ssid();
							}

							ret_val = 1;
						}
					}
				}
				
				fclose(fp);
			}
		}
		else
		{
			cprintf(VERBOSE, "[+] Nothing done, nothing to save.\n");
		}
		
		free(bssid);
		free(pretty_bssid);
	}

	return ret_val;
}

