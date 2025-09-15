/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2003-2005 H. Peter Anvin - All Rights Reserved
 *
 *   Permission is hereby granted, free of charge, to any person
 *   obtaining a copy of this software and associated documentation
 *   files (the "Software"), to deal in the Software without
 *   restriction, including without limitation the rights to use,
 *   copy, modify, merge, publish, distribute, sublicense, and/or
 *   sell copies of the Software, and to permit persons to whom
 *   the Software is furnished to do so, subject to the following
 *   conditions:
 *
 *   The above copyright notice and this permission notice shall
 *   be included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 *
 * ----------------------------------------------------------------------- */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <paths.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>

#define PACKAGE_STRING "util-linux-ng 2.18"
#define _(x) (x)

static const struct option long_options[] = {
  { "shared",       0, NULL, 's' },
  { "exclusive",    0, NULL, 'x' },
  { "unlock",       0, NULL, 'u' },
  { "nonblocking",  0, NULL, 'n' },
  { "nb",           0, NULL, 'n' },
  { "timeout",      1, NULL, 'w' },
  { "wait",         1, NULL, 'w' },
  { "close",        0, NULL, 'o' },
  { "help",         0, NULL, 'h' },
  { "version",      0, NULL, 'V' },
  { 0, 0, 0, 0 }
};

const char *program;

static void usage(int ex)
{
  fputs("flock (" PACKAGE_STRING ")\n", stderr);
  fprintf(stderr,
	_("Usage: %1$s [-sxun][-w #] fd#\n"
	  "       %1$s [-sxon][-w #] file [-c] command...\n"
	  "       %1$s [-sxon][-w #] directory [-c] command...\n"
	  "  -s  --shared     Get a shared lock\n"
	  "  -x  --exclusive  Get an exclusive lock\n"
	  "  -u  --unlock     Remove a lock\n"
	  "  -n  --nonblock   Fail rather than wait\n"
	  "  -w  --timeout    Wait for a limited amount of time\n"
	  "  -o  --close      Close file descriptor before running command\n"
	  "  -c  --command    Run a single command string through the shell\n"
	  "  -h  --help       Display this text\n"
	  "  -V  --version    Display version\n"),
	  program);
  exit(ex);
}


static sig_atomic_t timeout_expired = 0;

static void timeout_handler(int sig)
{
  (void)sig;

  timeout_expired = 1;
}


static char * strtotimeval(const char *str, struct timeval *tv)
{
  char *s;
  long fs;			/* Fractional seconds */
  int i;

  tv->tv_sec = strtol(str, &s, 10);
  fs = 0;

  if ( *s == '.' ) {
    s++;

    for ( i = 0 ; i < 6 ; i++ ) {
      if ( !isdigit(*s) )
	break;

      fs *= 10;
      fs += *s++ - '0';
    }

    for ( ; i < 6; i++ )
      fs *= 10;

    while ( isdigit(*s) )
      s++;
  }

  tv->tv_usec = fs;
  return s;
}

int main(int argc, char *argv[])
{
  struct itimerval timeout, old_timer;
  int have_timeout = 0;
  int type = LOCK_EX;
  int block = 0;
  int fd = -1;
  int opt, ix;
  int do_close = 0;
  int err;
  int status;
  int open_flags = 0;
  char *eon;
  char **cmd_argv = NULL, *sh_c_argv[4];
  const char *filename = NULL;
  struct sigaction sa, old_sa;

  program = argv[0];

  if ( argc < 2 )
    usage(EX_USAGE);

  memset(&timeout, 0, sizeof timeout);

  optopt = 0;
  while ( (opt = getopt_long(argc, argv, "+sexnouw:hV?", long_options, &ix)) != EOF ) {
    switch(opt) {
    case 's':
      type = LOCK_SH;
      break;
    case 'e':
    case 'x':
      type = LOCK_EX;
      break;
    case 'u':
      type = LOCK_UN;
      break;
    case 'o':
      do_close = 1;
      break;
    case 'n':
      block = LOCK_NB;
      break;
    case 'w':
      have_timeout = 1;
      eon = strtotimeval(optarg, &timeout.it_value);
      if ( *eon )
	usage(EX_USAGE);
      break;
    case 'V':
      printf("flock (%s)\n", PACKAGE_STRING);
      exit(0);
    default:
      /* optopt will be set if this was an unrecognized option, i.e. *not* 'h' or '?' */
      usage(optopt ? EX_USAGE : 0);
      break;
    }
  }

  if ( argc > optind+1 ) {
    /* Run command */

    if ( !strcmp(argv[optind+1], "-c") ||
	 !strcmp(argv[optind+1], "--command") ) {

      if ( argc != optind+3 ) {
	fprintf(stderr, _("%s: %s requires exactly one command argument\n"),
		program, argv[optind+1]);
	exit(EX_USAGE);
      }

      cmd_argv = sh_c_argv;

      cmd_argv[0] = getenv("SHELL");
      if ( !cmd_argv[0] || !*cmd_argv[0] )
	cmd_argv[0] = _PATH_BSHELL;

      cmd_argv[1] = "-c";
      cmd_argv[2] = argv[optind+2];
      cmd_argv[3] = 0;
    } else {
      cmd_argv = &argv[optind+1];
    }

    filename = argv[optind];
    fd = open(filename, O_RDONLY|O_NOCTTY|O_CREAT, 0666);
    /* Linux doesn't like O_CREAT on a directory, even though it should be a
       no-op */
    if (fd < 0 && errno == EISDIR)
        fd = open(filename, O_RDONLY|O_NOCTTY);

    if ( fd < 0 ) {
      err = errno;
      fprintf(stderr, _("%s: cannot open lock file %s: %s\n"),
	      program, argv[optind], strerror(err));
      exit((err == ENOMEM||err == EMFILE||err == ENFILE) ? EX_OSERR :
	   (err == EROFS||err == ENOSPC) ? EX_CANTCREAT :
	   EX_NOINPUT);
    }

  } else if (optind < argc) {
    /* Use provided file descriptor */

    fd = (int)strtol(argv[optind], &eon, 10);
    if ( *eon || !argv[optind] ) {
      fprintf(stderr, _("%s: bad number: %s\n"), program, argv[optind]);
      exit(EX_USAGE);
    }

  } else {
    /* Bad options */

    fprintf(stderr, _("%s: requires file descriptor, file or directory\n"),
		program);
    exit(EX_USAGE);
  }


  if ( have_timeout ) {
    if ( timeout.it_value.tv_sec == 0 &&
	 timeout.it_value.tv_usec == 0 ) {
      /* -w 0 is equivalent to -n; this has to be special-cased
	 because setting an itimer to zero means disabled! */

      have_timeout = 0;
      block = LOCK_NB;
    } else {
      memset(&sa, 0, sizeof sa);

      sa.sa_handler = timeout_handler;
      sa.sa_flags   = SA_RESETHAND;
      sigaction(SIGALRM, &sa, &old_sa);

      setitimer(ITIMER_REAL, &timeout, &old_timer);
    }
  }

  while ( flock(fd, type|block) ) {
    switch( (err = errno) ) {
    case EWOULDBLOCK:		/* -n option set and failed to lock */
      exit(1);
    case EINTR:			/* Signal received */
      if ( timeout_expired )
	exit(1);		/* -w option set and failed to lock */
      continue;			/* otherwise try again */
    case EBADF:			/* since Linux 3.4 (commit 55725513) */
      /* Probably NFSv4 where flock() is emulated by fcntl().
       * Let's try to reopen in read-write mode.
       */
      if (!(open_flags & O_RDWR) &&
          type != LOCK_SH &&
          filename &&
          access(filename, R_OK | W_OK) == 0) {

              close(fd);
              open_flags = O_RDWR;
              fd = open(filename, open_flags);
              break;
      }
      /* go through */
    default:			/* Other errors */
      if ( filename )
	fprintf(stderr, "%s: %s: %s\n", program, filename, strerror(err));
      else
	fprintf(stderr, "%s: %d: %s\n", program, fd, strerror(err));
      exit((err == ENOLCK||err == ENOMEM) ? EX_OSERR : EX_DATAERR);
    }
  }

  if ( have_timeout ) {
    setitimer(ITIMER_REAL, &old_timer, NULL); /* Cancel itimer */
    sigaction(SIGALRM, &old_sa, NULL); /* Cancel signal handler */
  }

  status = 0;

  if ( cmd_argv ) {
    pid_t w, f;

    /* Clear any inherited settings */
    signal(SIGCHLD, SIG_DFL);
    f = fork();

    if ( f < 0 ) {
      err = errno;
      fprintf(stderr, _("%s: fork failed: %s\n"), program, strerror(err));
      exit(EX_OSERR);
    } else if ( f == 0 ) {
      if ( do_close )
	close(fd);
      err = errno;
      execvp(cmd_argv[0], cmd_argv);
      /* execvp() failed */
      fprintf(stderr, "%s: %s: %s\n", program, cmd_argv[0], strerror(err));
      _exit((err == ENOMEM) ? EX_OSERR: EX_UNAVAILABLE);
    } else {
      do {
	w = waitpid(f, &status, 0);
	if (w == -1 && errno != EINTR)
	  break;
      } while ( w != f );

      if (w == -1) {
	err = errno;
	status = EXIT_FAILURE;
	fprintf(stderr, "%s: waitpid failed: %s\n", program, strerror(err));
      } else if ( WIFEXITED(status) )
	status = WEXITSTATUS(status);
      else if ( WIFSIGNALED(status) )
	status = WTERMSIG(status) + 128;
      else
	status = EX_OSERR;	/* WTF? */
    }
  }

  return status;
}

