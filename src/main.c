#include <string.h>

extern int reaver_main(int argc, char **argv);
extern int wash_main(int argc, char **argv);

#define C_REAVER 0
#define C_WASH 1

int main(int argc, char** argv) {
	char *e = strrchr(argv[0], '/');
	if(!e) e = argv[0];
	else e++;
	int command = C_REAVER;
	if(!strcmp(e, "reaver")) command = C_REAVER;
	else
	if(!strcmp(e, "wash")) command = C_WASH;
	else
	if(strstr(e, "wash")) command = C_WASH;
	else
	if(strstr(e, "reaver")) command = C_REAVER;

	if(command == C_WASH)
		return wash_main(argc, argv);
	else
		return reaver_main(argc, argv);
}
