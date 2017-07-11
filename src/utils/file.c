#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

char *gettempfilename(void) {
	char buf[256];
	strcpy(buf, "/tmp/reaver-XXXXXX");
	char *s;
	if(!(s = mkdtemp(buf))) return 0;
	else assert(s == buf);
	strcat(buf, ".tmp");
	return strdup(buf);
}

void writefile(const char* fn, const char* contents) {
	FILE *f = fopen(fn, "w");
	size_t l = strlen(contents);
	fwrite(contents, l, 1, f);
	fclose(f);
}
