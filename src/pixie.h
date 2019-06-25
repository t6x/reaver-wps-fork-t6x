#ifndef PIXIE_H
#define PIXIE_H

#define _GNU_SOURCE
#define _POSIX_SOURCE 200809L

#include <string.h>

struct pixie {
	char *authkey;
	char *pkr;
	char *pke;
	char *enonce;
	char *ehash1;
	char *ehash2;
	char *wrapper;
	char *pin;
	int do_pixie;
};

extern struct pixie pixie;

#define PIXIE_FREE(KEY) \
	do { \
		if(pixie.KEY) free(pixie.KEY); \
		pixie.KEY = 0; \
	} while(0)

#define PIXIE_SET(KEY, VALUE) \
	do { \
		if(pixie.KEY) free(pixie.KEY); \
		pixie.KEY = strdup(VALUE); \
	} while(0)

void pixie_format(const unsigned char *key, unsigned len, char *outbuf);
void pixie_attack(void);
void pixie_free();
int pixie_wrapper(char *cmd);

#endif

