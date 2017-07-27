# `fag` (Fork After Grep)

```
fag(1)                           User Commands                          fag(1)

NAME
       fag - daemonize program after a string was found (ForkAfterGrep)

SYNOPSIS
       fag [OPTIONS] PATTERN PROGRAM [ARGUMENTS...]

DESCRIPTION
       fag  uses  grep  to  search  the  output  of  PROGRAM  for  the regular
       expression PATTERN and daemonizes it when a match is found. The PID  is
       then returned on stdout.

OPTIONS
   Behaviour Changing Options
       -t SECONDS
              Set a timeout of SECONDS seconds.

       -k [SIGNAL]
              If  given, send a signal to PROGRAM.  SIGNAL defaults to SIGTERM
              (15).  Right now, only decimal notation is implemented.

       -e     Search PATTERN on stderr instead of stdout.

       -V     Be verbose; print program's stdout (or stderr if -e is  set)  to
              stderr.

   Generic Program Information
       -h     Output a short usage message and exit.

       -v     Display version and copyright information and exit.

EXIT STATUS
       If PATTERN was found, 0 is returned. Otherwise, the exit status follows
       the BSD guideline  outlined  in  #include  <sysexits.h>  if  the  error
       occured  from within fag or in case the chid process exits prematurely,
       its exit code is inherited. Notably, 69 is returned when the timeout is
       reached.

BUGS
   Known Bugs
       SIGNAL needs to be given as an integer; mnemonic should be supported in
       the future.

       Sometimes, stdin behaves strange after the program terminates.

   Reporting Bugs
       Please  report   bugs   and   patches   to   the   issue   tracker   at
       https://github.com/girst/forkaftergrep/.

NOTES
       Some might find the name of this program offensive. Feel free to create
       a symlink or alias on your system.

COPYRIGHT
       Copyright  2017  Tobias  Girstmair.  This   is   free   software;   see
       https://www.gnu.org/licenses/gpl-3.0.html for conditions.

AUTHOR
       Tobias Girstmair (http://isticktoit.net)

1.0                              28 July 2017                           fag(1)
```

## Notes

I've written this program for the [`tzap`/`szap`](https://linuxtv.org/wiki/index.php/Zap) utilities. They take a few seconds until the TV card/stick is tuned, and won't fork off when they are ready (If you terminate them, the tuning will end). Instead of waiting a few seconds, and hoping for the best, this does the exactly right thing. 
