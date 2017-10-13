# `fag` (Fork After Grep)

```
fag(1)                           User Commands                          fag(1)

NAME
       fag  - daemonize program after a regular expression pattern was matched
       (ForkAfterGrep)

SYNOPSIS
       fag [OPTIONS] PATTERN PROGRAM [ARGUMENTS...]

DESCRIPTION
       fag uses  grep  to  search  the  output  of  PROGRAM  for  the  regular
       expression  PATTERN and daemonizes it when a match is found. The PID is
       then returned on stdout.

OPTIONS
   Behaviour Changing Options
       -t SECONDS
              Set a timeout of SECONDS seconds.

       -k [SIGNAL]
              If given, send a signal to PROGRAM.  SIGNAL defaults to  SIGTERM
              (15).  Right now, only decimal notation is implemented.

       -e     Search PATTERN on stderr instead of stdout.

       -V     Be  verbose;  print program's stdout (or stderr if -e is set) to
              stderr.

   Generic Program Information
       -h     Output a short usage message and exit.

       -v     Display version and copyright information and exit.

EXIT STATUS
       If PATTERN was found, 0 is returned. Otherwise, the exit status follows
       the  BSD  guideline  outlined  in  #include  <sysexits.h>  if the error
       occured from within fag or in case the chid process exits  prematurely,
       its exit code is inherited. Notably, 69 is returned when the timeout is
       reached.

BUGS
   Known Bugs
       if grep gets killed (e.g. `killall grep'), fag should terminate.

   Reporting Bugs
       Please  report   bugs   and   patches   to   the   issue   tracker   at
       https://github.com/girst/forkaftergrep/.

NOTES
       SIGNAL needs to be given as an integer. implementing mnemonics is dirty
       and not very portable; maybe support in the future.

       Since 1.1, if fag gets interrupted or terminated, SIGTERM  is  sent  to
       PROGRAM.

COPYRIGHT
       Copyright   2017   Tobias   Girstmair.   This  is  free  software;  see
       https://www.gnu.org/licenses/gpl-3.0.html for conditions.

AUTHOR
       Tobias Girstmair (https://gir.st)

1.1                             13 October 2017                         fag(1)
```

## Notes

I've written this program for the [`tzap`/`szap`](https://linuxtv.org/wiki/index.php/Zap) utilities. They take a few seconds until the TV card/stick is tuned, and won't fork off when they are ready (If you terminate them, the tuning will end). Instead of waiting a few seconds, and hoping for the best, this does the exactly right thing.    
It also comes handy for `mopidy`, which takes a while to start up and before one can connect to it.
