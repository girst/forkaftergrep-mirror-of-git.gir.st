#ifndef __FAG_H__
#define __FAG_H__

#define PROGRNAME "fag"
#define VERSION "1.0"
#define AUTHOR "Tobias Girstmair"
#define YEAR "2017"
#define LICENSE "GNU GPLv3"

#define BUF_SIZE 4096

#define USAGE "Usage: %s [OPTION]... PATTERN PROGRAM [ARG]...\n"
#define VERTEXT ""PROGRNAME" "VERSION"\t(C) "YEAR" "AUTHOR", "LICENSE"\n"

struct opt {
	int timeout;
	int kill_sig;
	int verbose;
	char* pattern;
	char** argv;
	int stream;
	char grepopt[16];
};

int fork_after_grep (struct opt opts);

#endif
