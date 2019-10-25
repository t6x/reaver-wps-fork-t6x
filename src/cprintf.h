#ifndef CPRINTF_H
#define CPRINTF_H

enum debug_level
{
	CRITICAL = 0,
	INFO = 1,
	WARNING = 2,
	VERBOSE = 3,
	DEBUG = 4
};

void cprintf(enum debug_level level, const char *fmt, ...);
void cprintf_mute();
void cprintf_unmute();
int cprintf_ismuted();

#endif
