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
#include <errno.h>

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
	int add, p1_tried, p2_tried;

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

	if(stat(file, &wpstat) != 0) goto out;

	/* If the user explicitly specified a session file, don't prompt them */
	if(answer == 0)
	{
		bssid = mac2str(get_bssid(), ':');

		/* Don't use cprintf here; else, if the output is sent to a file via -o, the user won't see this prompt. */
		fprintf(stderr, "[?] Restore previous session for %s? [n/Y] ", bssid);
		answer = getc(stdin);
		free(bssid);
	}

	if(!(answer == 'y' || answer == 'Y' || answer == '\n'))
		goto out;

	if(!(fp = fopen(file, "r"))) {
		perror("fopen");
		goto out;
	}
	/* Get the key1 index value */
	if(fgets(line, MAX_LINE_SIZE, fp) == NULL) goto fout;
	set_p1_index(atoi(line));

	/* Get the key2 index value */
	if(fgets(line, MAX_LINE_SIZE, fp) == NULL) goto fout;
	set_p2_index(atoi(line));

	/* Get the key status value */
	if(fgets(line, MAX_LINE_SIZE, fp) == NULL) goto fout;
	set_key_status(atoi(line));

	/* Read in all p1 values */
	add = p1_tried = 0;
	for(i=0; i<P1_SIZE; i++)
	{
		memset(temp, 0, P1_READ_LEN);

		if(fgets(temp, P1_READ_LEN, fp) != NULL)
		{
			/* NULL out the new line character */
			temp[P1_STR_LEN] = 0;
			/* check has first half pin was specified and yet is KEY1_WIP */
			if (get_static_p1() && get_key_status() < KEY2_WIP)
			{
				if (i < get_p1_index())
				{
					/* Check the first half has been already tried */
					if (strcmp(get_static_p1(), temp) == 0)
					{
						p1_tried = 1;
					}
				}
				else if (i == get_p1_index())
				{
					/* Check current index of first half is the specified pin
					 * Yes: do nothing
					 * No: insert into current index and set add to 1
					 */
					if (!p1_tried && strcmp(get_static_p1(), temp) != 0)
					{
						set_p1(i, get_static_p1());
						add = 1;
					}
				}
				else
				{
					/* Check former index of first half
					 * Yes: set add to 0 and continue to next loop;
					 * No: do nothing
					 */
					if (strcmp(get_static_p1(), temp) == 0)
					{
						add = 0;
						continue;
					}
				}
			}
			set_p1(i+add, temp);
		}
	}

	/* Read in all p2 values */
	add = p2_tried = 0;
	for(i=0; i<P2_SIZE; i++)
	{
		memset(temp, 0, P1_READ_LEN);

		if(fgets(temp, P2_READ_LEN, fp) != NULL)
		{
			/* NULL out the new line character */
			temp[P2_STR_LEN] = 0;
			/* check has second half pin was specified and yet not KEY_DONE */
			if (get_static_p2() && get_key_status() != KEY_DONE)
			{
				if (i < get_p2_index())
				{
					/* Check the second half has been already tried */
					if (strcmp(get_static_p2(), temp) == 0)
					{
						p2_tried = 1;
					}
				}
				else if (i == get_p2_index())
				{
					/* Check current index of second half is the specified pin
					 * Yes: do nothing
					 * No: insert into current index and set add to 1
					 */
					if (!p2_tried && strcmp(get_static_p2(), temp) != 0)
					{
						set_p2(i, get_static_p2());
						add = 1;
					}
				}
				else
				{
					/* Check former index of second half
					 * Yes: set add to 0 and continue to next loop;
					 * No: do nothing
					 */
					if (strcmp(get_static_p2(), temp) == 0)
					{
						add = 0;
						continue;
					}
				}
			}
			set_p2(i+add, temp);
		}
	}

	ret_val = 1;

	/* Print warning message if the specified first or second half PIN was ignored */
	if (get_static_p1())
	{
		/* Check the specified 4/8 digit WPS PIN has been already tried */
		if (p1_tried || p2_tried)
		{
			ret_val = -1;
		}
		/* Print message what first half pin ignored if former key status >= KEY2_WIP */
		if (get_key_status() >= KEY2_WIP && strcmp(get_static_p1(), get_p1(get_p1_index())) != 0)
		{
			cprintf(INFO, "[!] First half PIN ignored, it was cracked\n");
		}
		/* Print message what second half pin ignored if former key status == KEY_DONE */
		if (get_key_status() == KEY_DONE && strcmp(get_static_p2(), get_p2(get_p2_index())) != 0)
		{
			cprintf(INFO, "[!] Second half PIN ignored, it was cracked\n");
		}
	}
	
fout:
	fclose(fp);

out:
	if(!ret_val)
	{
		set_p1_index(0);
		set_p2_index(0);
		set_key_status(KEY1_WIP);
	}
	else if(ret_val == -1)
	{
		cprintf(CRITICAL, "[!] The PIN has already been tested\n");
	} else {
		cprintf(INFO, "[+] Restored previous session\n");
	}

	return ret_val;
}

int save_session()
{
	if(get_pin_string_mode()) {
		cprintf(VERBOSE, "[*] String pin was specified, nothing to save.\n");
		return 0;
	}

	/* Don't bother saving anything if nothing has been done */
	/* Save .wpc file if the first half of first pin is correct */
	if(!((get_p1_index() > 0) || (get_p2_index() > 0) || (get_key_status() >= KEY2_WIP))) {
		cprintf(VERBOSE, "[+] Nothing done, nothing to save.\n");
		return 0;
	}

	/*
	 * If a session file was explicitly specified, use that; else, check for the
	 * default session file name for this BSSID.
	 */
        char file_name[FILENAME_MAX];
	if(get_session())
	{
		strcpy(file_name, get_session());
	}
	else
	{
		char bssid[6*3];
		mac2str_buf(get_bssid(), '\0', bssid);
		gen_sessionfile_name(bssid, file_name);
	}

        FILE *fp;

	if(!(fp = fopen(file_name, "w"))) {
		dprintf(2, "errror: fopen %s: %s\n", file_name, strerror(errno));
		return 0;
	}
	/* Save key1 index value */
	fprintf(fp, "%d\n", get_p1_index());

	/* Save key2 index value */
	fprintf(fp, "%d\n", get_p2_index());

	/* Save key status value */
	fprintf(fp, "%d\n", get_key_status());

	int i;
	/* Save all the p1 values */
	for(i=0; i<P1_SIZE; i++) fprintf(fp, "%s\n", get_p1(i));

	/* Save all the p2 values */
	for(i=0; i<P2_SIZE; i++) fprintf(fp, "%s\n", get_p2(i));

	fclose(fp);
	return 1;
}

/**
 * Return the percentage of crack progress
 * 
 * @params unsigned char* mac The MAC address
 * 
 * @return char*              x.xx, xx.xx or xxx.x
 */
char *get_crack_progress(unsigned char *mac)
{
	struct stat wpstat = { 0 };
	FILE *fp = NULL;
	char file[FILENAME_MAX];
	int p1_idx, p2_idx, num, attempts;
	char *bssid = NULL, *crack_progress = NULL;
	enum key_state key_status;

	bssid = mac2str(mac, '\0');

	if (bssid) {
		gen_sessionfile_name(bssid, file);

		if(stat(file, &wpstat) == 0) {
			crack_progress = malloc(10);
			if((fp = fopen(file, "r")) && crack_progress) {
				crack_progress[0] = '\0';
				fscanf(fp, "%d", &p1_idx);
				fscanf(fp, "%d", &p2_idx);
				fscanf(fp, "%d", &num);
				key_status = num;

				fclose(fp);

				if (key_status == KEY1_WIP) {
					attempts = p1_idx;
					sprintf(crack_progress, "%.2lf", (attempts*100.0)/(P1_SIZE + P2_SIZE));
				} else if (key_status == KEY2_WIP) {
					attempts = P1_SIZE + p2_idx;
					sprintf(crack_progress, "%.2lf", (attempts*100.0)/(P1_SIZE + P2_SIZE));
				} else {
					sprintf(crack_progress, "%.1lf", 100.0);
				}
			} else {
				if (crack_progress) {
					free(crack_progress);
					crack_progress = NULL;
				}
			}
		}
	}
	if (bssid) free(bssid);

	return crack_progress;
}


/**
 * Insert the value to current p1 index (jump the queue)
 * Return value:
 * -2 = value tried because KEY2_WIP/KEY_DONE, array nothing done
 * -1 = value tried, array nothing done
 *  0 = value is the same as current index, nothing done
 *  1 = array reorganized
 * 
 * @params char* value The first half WPS pin
 * 
 * @return int
 */
int jump_p1_queue(char* value)
{
	int i, index, v_index, found;
	int ret_val = 0;
	char *pch;

	if (!value || strcmp(value, get_p1(get_p1_index())) == 0) {
		return 0;
	}

	if(get_key_status() < KEY2_WIP) {
		found = 0;
		/* Get the p1 index */
		index = get_p1_index();
		/* Check the value was tried */
		for(i=0; i < P1_SIZE && i < index; ++i) {
			if (strcmp(get_p1(i), value) == 0) {
				found = 1;
				ret_val = -1;
				break;
			}
		}
		if (!found) {
			/* Find the p1 index of the value */
			for(i=index; i < P1_SIZE; ++i) {
				if (strcmp(get_p1(i), value) == 0) {
					found = 1;
					v_index = i;
					if (v_index != index) {
						ret_val = 1;
					}
					break;
				}
			}
			if (found) {
				/* Reorganize p1 array */
				pch = globule->p1[v_index];
				for (i=v_index; i > index; --i) {
					globule->p1[i] = globule->p1[i-1];
				}
				globule->p1[index] = pch;
			}
		}
	}
	else if (strcmp(value, get_p1(get_p1_index())) != 0) {
		ret_val = -2;
	}

	return ret_val;
}

/**
 * Insert the value to current p2 index (jump the queue)
 * Return value:
 * -2 = value tried because KEY_DONE, array nothing done
 * -1 = value tried, array nothing done
 *  0 = value is the same as current index, nothing done
 *  1 = array reorganized
 * 
 * @params char* value The second half WPS pin
 * 
 * @return int
 */
int jump_p2_queue(char* value)
{
	int i, index, v_index, found;
	int ret_val = 0;
	char *pch;

	if (!value || strcmp(value, get_p2(get_p2_index())) == 0) {
		return 0;
	}

	if(get_key_status() < KEY_DONE) {
		found = 0;
		/* Get the p2 index */
		index = get_p2_index();
		/* Check the value was tried */
		for(i=0; i < P2_SIZE && i < index; ++i) {
			if (strcmp(get_p2(i), value) == 0) {
				found = 1;
				ret_val = -1;
				break;
			}
		}
		if (!found) {
			/* Find the p2 index of the value */
			for(i=index; i < P2_SIZE; ++i) {
				if (strcmp(get_p2(i), value) == 0) {
					found = 1;
					v_index = i;
					if (v_index != index) {
						ret_val = 1;
					}
					break;
				}
			}
			if (found) {
				/* Reorganize p2 array */
				pch = globule->p2[v_index];
				for (i=v_index; i > index; --i) {
					globule->p2[i] = globule->p2[i-1];
				}
				globule->p2[index] = pch;
			}
		}
	}
	else if (strcmp(value, get_p2(get_p2_index())) != 0) {
		ret_val = -2;
	}

	return ret_val;
}
