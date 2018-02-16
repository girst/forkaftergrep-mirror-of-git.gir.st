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

       -k[SIGNAL]
              If given, send a signal to PROGRAM.  SIGNAL defaults to  SIGTERM
              (15).  Right now, only decimal notation is implemented.

       -r     Search PATTERN on stderr instead of stdout.

       -l FILE
              Log PROGRAM's stdout to FILE.  The file will be opened in append
              mode and created with permissions 0600 if it doesn't exist.

       -L FILE
              Same as -l but logs PROGRAM's stderr.

       -V     Be verbose; print program's stdout (or stderr if -r is  set)  to
              stderr.

   Generic Program Information
       -h     Output a short usage message and exit.

       -v     Display version and copyright information and exit.

   Supported grep Options
       -E, -F, -G, -P
              Matcher  selection  switches  for  extended regular expressions,
              fixed strings, basic  regular  expressions  (default)  or  Perl-
              compatible  regular  expressions.  At  most  one  of them may be
              supplied.

       -i, -w, -x, -U
              Matching control switches for ignore  case  distinctions,  whole
              words only, whole lines only and treat as binary.

       -Z, -J Decompression  switches  for  gzip(1)  and bzip2(1).  Not widely
              supported; check your grep's capabilities.

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
       Usually,  fag uses the grep supplied in the path. This behaviour can be
       overridden with the environment variable GREP_OVERRIDE.

       Since 1.2, if fag gets interrupted or  terminated  before  a  match  is
       found  (or  the  timeout  has  been  reached), this signal is passed to
       PROGRAM.

       In version 1.2 the command line switch -e was renamed to  -r  to  avoid
       overloading  grep's own switches. An error will be thrown when -e or -f
       is supplied as an argument.

COPYRIGHT
       Copyright 2017-2018 Tobias Girstmair. This is  free  software  released
       under  the  terms  of  the  GNU  General  Public License Version 3; see
       https://www.gnu.org/licenses/gpl-3.0.html for conditions.

AUTHOR
       Tobias Girstmair (https://gir.st/)

1.2                            16 February 2018                         fag(1)
```

## Notes

I've written this program for the [`tzap`/`szap`](https://linuxtv.org/wiki/index.php/Zap) utilities. They take a few seconds until the TV card/stick is tuned, and won't fork off when they are ready (If you terminate them, the tuning will end). Instead of waiting a few seconds, and hoping for the best, this does the exactly right thing.    
It also comes handy for `mopidy`, which takes a while to start up and before one can connect to it.
