# `fag` (Fork After Grep)

```
fag(1)                           User Commands                          fag(1)

NAME
       fag - daemonize program after a pattern was matched (ForkAfterGrep)

SYNOPSIS
       fag [OPTIONS] PATTERN PROGRAM [ARGUMENTS...]

DESCRIPTION
       fag  uses  grep  to  search  the  output  of  PROGRAM  for  the regular
       expression PATTERN and daemonizes it when a match is found. The PID  is
       then returned on stdout.

OPTIONS
   Behaviour Changing Options
       -t SECONDS
              Abort  matching  after  SECONDS seconds and print PROGRAM's PID.
              Unless -k is given, PROGRAM is kept running and daemonized.

       -k[SIGNAL]
              If the timeout (-t) has been reached, send a signal to  PROGRAM.
              SIGNAL  defaults  to SIGTERM (15).  The signal may only be given
              as a number.

       -r     Search for PATTERN on stderr instead of stdout.

       -l FILE
              Log PROGRAM's stdout to FILE.  The file will be opened in append
              mode.  If  the file does not exist, it will be created with file
              mode 0600.

       -L FILE
              Same as -l but logs PROGRAM's stderr.

       -V     Be verbose; print PROGRAM's monitored stream to stderr.

   Generic Program Information
       -h     Output a short usage message and exit.

       -v     Display version and copyright information and exit.

   Supported grep Options
       -E, -F, -G, -P
              Matcher selection switches  for  extended  regular  expressions,
              fixed  strings,  basic  regular  expressions  (default) or Perl-
              compatible regular expressions. At  most  one  of  them  may  be
              supplied.

       -i, -w, -x, -U
              Matching  control  switches  for ignore case distinctions, whole
              words only, whole lines only and treat as binary.

       -Z, -J Decompression switches for gzip(1)  and  bzip2(1).   Not  widely
              supported; check your grep's capabilities.

EXIT STATUS
       If PATTERN was found, 0 is returned. Otherwise, the exit status follows
       the BSD guideline  outlined  in  #include  <sysexits.h>  if  the  error
       occurred   from   within  fag  or  in  case  the  child  process  exits
       prematurely, its exit code is inherited. Notably, 69 is  returned  when
       the timeout is reached.

BUGS
   Known Bugs
       logging stops when a timeout is reached.

       if grep gets killed (e.g. `killall grep'), fag should terminate.

   Reporting Bugs
       Please   report   bugs   and   patches   to   the   issue   tracker  at
       https://github.com/girst/forkaftergrep/.

NOTES
       Usually, fag uses the grep supplied in the path. This behaviour can  be
       overridden with the environment variable GREP_OVERRIDE.

       fag  works best when PROGRAM's output is line-buffered.  stdbuf(1) from
       the GNU coreutils can adjust buffering options. If a program  is  still
       too clever, script(1) creates a pty to wrap around a program.

       Since  1.2,  if  fag  gets  interrupted or terminated before a match is
       found (or the timeout has been  reached),  this  signal  is  passed  to
       PROGRAM.

       In  version  1.2  the command line switch -e was renamed to -r to avoid
       overloading grep's own switches. An error will be thrown when -e or  -f
       is supplied as an argument.

COPYRIGHT
       Copyright  2017-2018  Tobias  Girstmair. This is free software released
       under the terms of the  GNU  General  Public  License  Version  3;  see
       https://www.gnu.org/licenses/gpl-3.0.html for conditions.

AUTHOR
       Tobias Girstmair (https://gir.st/)

1.2                            16 February 2018                         fag(1)
```

## Installation

Compile the program by issuing `make`. Targets `install` and `uninstall` should work as expected.

## Notes

I've written this program for the [`tzap`/`szap`](https://linuxtv.org/wiki/index.php/Zap) utilities. They take a few seconds until the TV card/stick is tuned, and won't fork off when they are ready (If you terminate them, the tuning will end). Instead of waiting a few seconds, and hoping for the best, this does the exactly right thing.    
It also comes handy for `mopidy`, which takes a while to start up and before one can connect to it.

## Examples

Wait for `tzap-t2` (DVB-T2 version of `tzap`) to tune into a channel or abort after 1 minute:

    fag -t 60 -k FE_HAS_LOCK ./tzap-t2 -a0 -f1 -V -c channels.vdr "ORF1;ORF"

---

Start [mopidy](https://www.mopidy.com/) and wait for the MPD service to have started up:

    fag -rV -L /tmp/mopidy.log "MPD server running at" mopidy


### On Buffering

Some programs will detect that their output is not going to a terminal and switch to using a large buffer size. Most of the time it is sufficient to wrap such a program with `stdbuf`(1) like so:

    fag PATTTERN stdbuf -oL PROGRAM

Stubborn programs can also be coaxed into line-buffering by executing them in a pty, for example with `script`(1): [via](https://stackoverflow.com/a/55655115)

    script -qfc "$(printf "%q " "$@")" /dev/null
