/* forkaftergrep (C) 2017 Tobias Girstmair, GPLv3 */

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include "fag.h"

struct opt opts = {0, 0, NULL, NULL, STDOUT_FILENO};

int main (int argc, char** argv) {
	int opt;
	opterr = 0;

	/* the `+' forces getopt to stop at the first non-option */
	while ((opt = getopt (argc, argv, "+t:k::ehv")) != -1) {
		switch (opt) {
		case 't':
			opts.timeout = atoi (optarg);
			break;
		case 'k':
			opts.kill_sig = optarg ? atoi (optarg) : SIGTERM;
			break;
		case 'e':
			opts.stream = STDERR_FILENO;
			break;
		case 'h':
			fprintf (stderr, VERTEXT USAGE
				"Options:\n"
				"\t-t N\ttimeout after N seconds\n"
				"\t-k [M]\tsend signal M to child after timeout (default: 15/SIGTERM)\n"
				"\t-e\tgrep on stderr instead of stdout\n", argv[0]);
			return EX_OK;
		case 'v':
			fprintf (stderr, VERTEXT);
			return EX_OK;
		default: 
			fprintf (stderr, "Unrecognized option: %c\n" USAGE, optopt, argv[0]);
			return EX_USAGE;
		}
	}

	/* the first non-option argument is the search string */
	if (optind < argc) {
		opts.pattern = argv[optind++];
	} else {
		fprintf (stderr, USAGE "(Missing PATTERN)\n", argv[0]);
		return EX_USAGE;
	}

	/* the remaining are the program to be run */
	if (optind < argc) {
		opts.argv = &(argv[optind]);
	} else {
		fprintf (stderr, USAGE "(Missing PROGRAM)\n", argv[0]);
		return EX_USAGE;
	}

	int retval = fork_after_grep (opts);

	return retval;
}

int fork_after_grep (struct opt opts) {
        printf ("timeout:\t%d\n" "kill_sig:\t%d\n" "pattern: \t%s\n" "stream:  \t%d\n" "program: \t"
                , opts.timeout, opts.kill_sig, opts.pattern, opts.stream);
        for (char** p = opts.argv; *p;) printf ("%s ", *p++); putchar ('\n');

	int pipefd[2];
	pid_t cpid;
	int status;

	char buf[BUF_SIZE];
	int nbytes;

	struct timeval begin, now, diff;

	if (pipe(pipefd) == -1) {
		fprintf (stderr, "pipe error\n");
		return EX_OSERR;
	}

	if ((cpid = fork()) == -1) {
		fprintf (stderr, "fork error: %s", strerror (errno));
		close (pipefd[0]);
		close (pipefd[1]);
		return EX_OSERR;
	}

	if (cpid == 0) {
		close (pipefd[0]);
		dup2 (pipefd[1], opts.stream);
		close (pipefd[1]);
		close (opts.stream==STDOUT_FILENO?STDERR_FILENO:STDOUT_FILENO);

		if (setsid () == -1) {
			fprintf (stderr, "setsid error: %s", strerror (errno));
			_exit (EX_OSERR);
		}

		execvp (opts.argv[0], opts.argv);
		fprintf (stderr, "exec error: %s", strerror (errno));
		_exit (EX_UNAVAILABLE);
	} else {
		close (pipefd[1]);
		fcntl (pipefd[0], F_SETFL, fcntl (pipefd[0], F_GETFL, 0) | O_NONBLOCK);

		gettimeofday (&begin, NULL);

		for (;;) {
			usleep (20000);
			nbytes = read (pipefd[0], buf, BUF_SIZE);
			if (nbytes == -1) {
				switch (errno) {
				case EAGAIN:
					continue;
				default:
					fprintf (stderr, "read error: %s", strerror (errno));
					close (pipefd[0]);
					close (pipefd[1]);
					return EX_IOERR;
				}
			} else if (nbytes == 0) {
				fprintf (stderr, "Child program exited prematurely.\n");
				close (pipefd[0]);
				close (pipefd[1]);
				if (waitpid (cpid, &status, WNOHANG) > 0 && WIFEXITED (status)) {
					return WEXITSTATUS (status);
				}
				return EX_UNAVAILABLE;
			}
			if (strstr (buf, opts.pattern) != NULL) {
				printf ("%d\n", cpid);
				/* create a new child to keep pipe alive (will exit with exec'd program) */
				if (!fork ()) {
					while (kill(cpid, 0) != -1 && errno != ESRCH ) sleep (1);
					close (pipefd[0]);
					close (pipefd[1]);
//close(0);close(1);close(2);
					_exit(0);
				}
				close (pipefd[0]);
				close (pipefd[1]);
//close(0);close(1);close(2);
				return EX_OK;
			}

			if (opts.timeout > 0) {
				gettimeofday (&now, NULL);
				timersub (&now, &begin, &diff);
				if (diff.tv_sec >= opts.timeout) {
					fprintf (stderr, "Timeout reached. \n");
					if (opts.kill_sig > 0) kill (cpid, opts.kill_sig);
					close (pipefd[0]);
					close (pipefd[1]);
					return EX_UNAVAILABLE;
				}
			}
		}
	}
}
