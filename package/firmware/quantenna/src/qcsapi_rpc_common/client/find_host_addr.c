/*SH0
*******************************************************************************
**                                                                           **
**         Copyright (c) 2009 - 2011 Quantenna Communications Inc            **
**                                                                           **
**  File        : call_qcsapi_local.c                                        **
**  Description : tiny wrapper to invoke call_qcsapi locally, from main()    **
**                                                                           **
*******************************************************************************
**                                                                           **
**  Redistribution and use in source and binary forms, with or without       **
**  modification, are permitted provided that the following conditions       **
**  are met:                                                                 **
**  1. Redistributions of source code must retain the above copyright        **
**     notice, this list of conditions and the following disclaimer.         **
**  2. Redistributions in binary form must reproduce the above copyright     **
**     notice, this list of conditions and the following disclaimer in the   **
**     documentation and/or other materials provided with the distribution.  **
**  3. The name of the author may not be used to endorse or promote products **
**     derived from this software without specific prior written permission. **
**                                                                           **
**  Alternatively, this software may be distributed under the terms of the   **
**  GNU General Public License ("GPL") version 2, or (at your option) any    **
**  later version as published by the Free Software Foundation.              **
**                                                                           **
**  In the case this software is distributed under the GPL license,          **
**  you should have received a copy of the GNU General Public License        **
**  along with this software; if not, write to the Free Software             **
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  **
**                                                                           **
**  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR       **
**  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES**
**  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  **
**  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,         **
**  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT **
**  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,**
**  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    **
**  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      **
**  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF **
**  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.        **
**                                                                           **
*******************************************************************************
EH0*/

#include <qcsapi_rpc_common/client/find_host_addr.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define QCSAPI_HOST_ENV_VAR	"QCSAPI_RPC_TARGET"

static const char * const cfg_file_paths[] = {
	"/mnt/jffs2/rmt_ip.conf",
	"/etc/qcsapi_target_ip.conf",
	NULL	/* last entry must be null */
};

#define MAX_HOSTNAME_SIZE	128


void client_qcsapi_find_host_errmsg(const char *progname)
{
	int i;
	fprintf(stderr, "No remote host configured! Remote host config is\n");
	fprintf(stderr, "evaluated in the following order:\n");
	fprintf(stderr, " 1) Command line parameter:\n");
	fprintf(stderr, "	%s --host <host> <args>\n", progname);
	fprintf(stderr, " 2) Environment variable:\n");
	fprintf(stderr, "	export %s=<host>\n", QCSAPI_HOST_ENV_VAR);
	fprintf(stderr, "	%s <args>\n", progname);
	fprintf(stderr, " 3) Configuration files, in order:\n");
	for (i = 0; cfg_file_paths[i]; i++) {
		fprintf(stderr, "    %s\n", cfg_file_paths[i]);
	}
}

static void trim_trailing_space(char *buf)
{
	int i;
	for (i = strlen(buf) - 1; isspace(buf[i]); i--) {
		buf[i] = '\0';
	}
}

static const char *first_nonspace(const char *buf)
{
	while (*buf && isspace(*buf)) {
		buf++;
	}
	return buf;
}

static const char * client_qcsapi_find_host_read_file(const char * const filename)
{
	static char hostbuf[MAX_HOSTNAME_SIZE];
	const char* host = NULL;
	char* fret;

	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		/* files may legitimately not exist */
		return NULL;
	}

	/* assume the file contains the target host on the first line */
	fret = fgets(hostbuf, MAX_HOSTNAME_SIZE, file);
	if (fret || feof(file)) {
		trim_trailing_space(hostbuf);
		host = first_nonspace(hostbuf);
	} else {
		fprintf(stderr, "%s: error reading file '%s': %s\n",
				__FUNCTION__, filename, strerror(errno));
	}

	fclose(file);

	return host;
}

const char* client_qcsapi_find_host_addr(int *argc, char ***argv)
{
	int i;
	const char *host;

	/* check for command line arguments */
	if (argc && argv && *argc >= 2 && strcmp((*argv)[1], "--host") == 0) {
		host = (*argv)[2];

		/* move program argv[0] */
		(*argv)[2] = (*argv)[0];

		/* skip over --host <arg> args */
		*argc = *argc - 2;
		*argv = &(*argv)[2];

		return host;
	}

	/* check for environment variables */
	host = getenv(QCSAPI_HOST_ENV_VAR);
	if (host) {
		return host;
	}

	/* check for config files */
	for (i = 0; cfg_file_paths[i]; i++) {
		host = client_qcsapi_find_host_read_file(cfg_file_paths[i]);
		if (host) {
			return host;
		}
	}

	return NULL;
}

