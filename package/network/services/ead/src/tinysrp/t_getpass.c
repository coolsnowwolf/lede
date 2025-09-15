/*
 * Copyright 1990 - 1995, Julianne Frances Haugh
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Julianne F. Haugh nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JULIE HAUGH AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL JULIE HAUGH OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "t_defines.h"
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif /* _WIN32 */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <signal.h>
#include <stdio.h>

static  int     sig_caught;
#ifdef HAVE_SIGACTION
static  struct  sigaction sigact;
#endif

/*ARGSUSED*/
static RETSIGTYPE
sig_catch (sig)
int     sig;
{
	sig_caught = 1;
}

_TYPE( int )
t_getpass (buf, maxlen, prompt)
	char *buf;
	unsigned maxlen;
	const char *prompt;
{
	char    *cp;
#ifdef _WIN32
    HANDLE handle = (HANDLE) _get_osfhandle(_fileno(stdin));
    DWORD  mode;

    GetConsoleMode( handle, &mode );
    SetConsoleMode( handle, mode & ~ENABLE_ECHO_INPUT );

    if(fputs(prompt, stdout) == EOF ||
	fgets(buf, maxlen, stdin) == NULL) {
	SetConsoleMode(handle,mode);
	return -1;
    }
    cp = buf + strlen(buf) - 1;
    if ( *cp == 0x0a )
	*cp = '\0';
    printf("\n");
    SetConsoleMode(handle,mode);
#else
	FILE    *fp;
	int     tty_opened = 0;

#ifdef HAVE_SIGACTION
	struct  sigaction old_sigact;
#else
	RETSIGTYPE      (*old_signal)();
#endif
	TERMIO  new_modes;
	TERMIO  old_modes;

	/*
	 * set a flag so the SIGINT signal can be re-sent if it
	 * is caught
	 */

	sig_caught = 0;

	/*
	 * if /dev/tty can't be opened, getpass() needs to read
	 * from stdin instead.
	 */

	if ((fp = fopen ("/dev/tty", "r")) == 0) {
		fp = stdin;
		setbuf (fp, (char *) 0);
	} else {
		tty_opened = 1;
	}

	/*
	 * the current tty modes must be saved so they can be
	 * restored later on.  echo will be turned off, except
	 * for the newline character (BSD has to punt on this)
	 */

	if (GTTY (fileno (fp), &new_modes))
		return -1;

	old_modes = new_modes;

#ifdef HAVE_SIGACTION
	sigact.sa_handler = sig_catch;
	(void) sigaction (SIGINT, &sigact, &old_sigact);
#else
	old_signal = signal (SIGINT, sig_catch);
#endif

#ifdef USE_SGTTY
	new_modes.sg_flags &= ~ECHO;
#else
	new_modes.c_iflag &= ~IGNCR;
	new_modes.c_iflag |= ICRNL;
	new_modes.c_oflag |= OPOST|ONLCR;
	new_modes.c_lflag &= ~(ECHO|ECHOE|ECHOK);
	new_modes.c_lflag |= ICANON|ECHONL;
#endif

	if (STTY (fileno (fp), &new_modes))
		goto out;

	/*
	 * the prompt is output, and the response read without
	 * echoing.  the trailing newline must be removed.  if
	 * the fgets() returns an error, a NULL pointer is
	 * returned.
	 */

	if (fputs (prompt, stdout) == EOF)
		goto out;

	(void) fflush (stdout);

	if (fgets (buf, maxlen, fp) == buf) {
		if ((cp = strchr (buf, '\n')))
			*cp = '\0';
		else
			buf[maxlen - 1] = '\0';

#ifdef USE_SGTTY
		putc ('\n', stdout);
#endif
	}
	else buf[0] = '\0';
out:
	/*
	 * the old SIGINT handler is restored after the tty
	 * modes.  then /dev/tty is closed if it was opened in
	 * the beginning.  finally, if a signal was caught it
	 * is sent to this process for normal processing.
	 */

	if (STTY (fileno (fp), &old_modes))
	{ memset (buf, 0, maxlen); return -1; }

#ifdef HAVE_SIGACTION
	(void) sigaction (SIGINT, &old_sigact, NULL);
#else
	(void) signal (SIGINT, old_signal);
#endif

	if (tty_opened)
		(void) fclose (fp);

	if (sig_caught) {
		kill (getpid (), SIGINT);
		memset (buf, 0, maxlen);
		return -1;
	}
#endif

	return 0;
}
