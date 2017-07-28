/* forkaftergrep (C) 2017 Tobias Girstmair, GPLv3 */
//TODO: if grep exits with an error, fag thinks a match was found

#define _XOPEN_SOURCE 500
#define _DEFAULT_SOURCE
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

int main (int argc, char** argv) {
	struct opt opts = {0, 0, 0, NULL, NULL, STDOUT_FILENO};
	struct grepopt optg = {basic_regexp, 0, 0, 0, 0};
	int opt;
	opterr = 0;

	/* the `+' forces getopt to stop at the first non-option */
	while ((opt = getopt (argc, argv, "+t:k::eVhvEFGPiwxyU")) != -1) {
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
		case 'V':
			opts.verbose = 1;
			break;
		case 'h':
			fprintf (stderr, VERTEXT USAGE
				"Options:\n"
				"\t-t N\ttimeout after N seconds\n"
				"\t-k [M]\tsend signal M to child after timeout (default: 15/SIGTERM)\n"
				"\t-e\tgrep on stderr instead of stdout\n"
				"\t-V\tbe verbose; print PROGRAM's stdout/stderr to stderr\n"
				"\t-[EFGPiwxyU]\t grep options\n", argv[0]);
			return EX_OK;
		case 'v':
			fprintf (stderr, VERTEXT);
			return EX_OK;
		/* `grep' options (Note: missing `-e:', `-f:') */
		case 'E': optg.regex = extended_regexp; break;
		case 'F': optg.regex = fixed_strings  ; break;
		case 'G': optg.regex = basic_regexp   ; break; /* grep default */
		case 'P': optg.regex = perl_regexp    ; break;
		case 'i': /* fall thru */
		case 'y': optg.ignore_case = 1; break;
		case 'w': optg.word_regexp = 1; break;
		case 'x': optg.line_regexp = 1; break;
		case 'U': optg.binary      = 1; break;

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

	int retval = fork_after_grep (opts, optg);

	return retval;
}

int fork_after_grep (struct opt opts, struct grepopt optg) {
	int pipefd[2];
	pid_t cpid;
	int status;

	char buf[BUF_SIZE];
	int nbytes;

	struct timeval begin, now, diff;

	if (pipe(pipefd) == -1) {
		fprintf (stderr, "pipe error (userprog)\n");
		return EX_OSERR;
	}

	if ((cpid = fork()) == -1) {
		fprintf (stderr, "fork error (userprog): %s", strerror (errno));
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
			fprintf (stderr, "setsid error (userprog): %s", strerror (errno));
			_exit (EX_OSERR);
		}

		execvp (opts.argv[0], opts.argv);
		fprintf (stderr, "exec error (userprog): %s", strerror (errno));
		_exit (EX_UNAVAILABLE);
	} else {
		pid_t grep_cpid;
		int grep_pipefd[2];
		int grep_status;

		close (pipefd[1]);
		fcntl (pipefd[0], F_SETFL, fcntl (pipefd[0], F_GETFL, 0) | O_NONBLOCK);

		gettimeofday (&begin, NULL); /* for timeout */

		if (pipe(grep_pipefd) == -1) {
			fprintf (stderr, "pipe error (grep)\n");
			close (pipefd[0]);
			close (pipefd[1]);
			return EX_OSERR;
		}

		if ((grep_cpid = fork()) == -1) {
			fprintf (stderr, "fork error (grep): %s", strerror (errno));
			close (pipefd[0]);
			close (pipefd[1]);
			close (grep_pipefd[0]);
			close (grep_pipefd[1]);
			return EX_OSERR;
		}

		if (grep_cpid == 0) {
			close (grep_pipefd[1]);
			dup2 (grep_pipefd[0], STDIN_FILENO);
			close (grep_pipefd[0]);

			close (STDERR_FILENO);
			close (STDOUT_FILENO);

			/* generate argument list on the fly (TODO: ugly) */
			/* `-q': don't print anything; exit with 0 on match; with 1 on error */
			char grep_options[16] = "-q";
			char* p = grep_options+2;
			if (optg.regex == extended_regexp) *(p++)='E';
			if (optg.regex == fixed_strings)   *(p++)='F';
			if (optg.regex == basic_regexp)    *(p++)='G';
			if (optg.regex == perl_regexp)     *(p++)='P';
			if (optg.ignore_case)              *(p++)='i';
			if (optg.word_regexp)              *(p++)='w';
			if (optg.line_regexp)              *(p++)='x';
			if (optg.binary)                   *(p++)='U';
			*p = '\0';

			execlp ("grep", "grep", grep_options, opts.pattern, NULL);
			fprintf (stderr, "exec error (grep): %s", strerror (errno));
			_exit (EX_SOFTWARE);
		} else {
			close (grep_pipefd[0]);
			for (;;) {
				usleep (20000);
				nbytes = read (pipefd[0], buf, BUF_SIZE);
				if (nbytes == -1) {
					switch (errno) {
					case EAGAIN:
						break;
					default:
						fprintf (stderr, "read error (userprog): %s", strerror (errno));
						close (pipefd[0]);
						close (pipefd[1]);
						close (grep_pipefd[1]);
						//TODO: kill grep?
						return EX_IOERR;
					}
				} else if (nbytes == 0) {
					fprintf (stderr, "Child program exited prematurely (userprog).\n");
					close (pipefd[0]);
					close (pipefd[1]);
					close (grep_pipefd[1]);
					//TODO: kill grep?
					if (waitpid (cpid, &status, WNOHANG) > 0 && WIFEXITED (status)) {
						return WEXITSTATUS (status);
					}
					return EX_UNAVAILABLE;
				} else {
					/* have new userprog-data, send it to grep */
					if (opts.verbose) {
						fputs (buf, stderr);
						write(STDERR_FILENO, buf, nbytes);
					}

					write(grep_pipefd[1], buf, nbytes);
				}

				// TODO: exits with `0' even if `grep' exits with code > 0 !
				if (waitpid (grep_cpid, &grep_status, WNOHANG) > 0 && WIFEXITED (grep_status)) {
					close (grep_pipefd[1]);

					if (WEXITSTATUS(grep_status) == 0) {
						/* grep exited with match found */
						printf ("%d\n", cpid);

						/* create a new child to keep pipe alive (will exit with exec'd program) */
						if (!fork ()) {
							while (kill(cpid, 0) != -1 && errno != ESRCH ) sleep (1);
							close (pipefd[0]);
							close (pipefd[1]);
							_exit(0);
						}
						close (pipefd[0]);
						close (pipefd[1]);
						return EX_OK;
					} else {
						/* grep exited due to an error */
						fprintf (stderr, "grep exited due to an error.");
						close (pipefd[0]);
						close (pipefd[1]);
						close (grep_pipefd[1]);
						return EX_IOERR;
					}
				}

				if (opts.timeout > 0) {
					gettimeofday (&now, NULL);
					timersub (&now, &begin, &diff);
					if (diff.tv_sec >= opts.timeout) {
						fprintf (stderr, "Timeout reached. \n");
						if (opts.kill_sig > 0) kill (cpid, opts.kill_sig);
						close (pipefd[0]);
						close (pipefd[1]);
						close (grep_pipefd[1]);
						//TODO: kill grep?
						return EX_UNAVAILABLE;
					}
				}
			}
		}
	}
}
