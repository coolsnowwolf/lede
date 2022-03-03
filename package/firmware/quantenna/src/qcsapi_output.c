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

#include "qcsapi_output.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int qcsapi_output_stdio_fn(struct qcsapi_output* qo,
		enum qcsapi_print_type out_type, const char * format, va_list args)
{
	FILE *file = stderr;
	int rv;

	if (out_type == OUT) {
		file = stdout;
	} else {
		file = stderr;
	}

	rv = vfprintf(file, format, args);

	return rv;
}

int qcsapi_output_buf_fn(struct qcsapi_output* qo,
		enum qcsapi_print_type out_type, const char * format, va_list args)
{
	const ssize_t realloc_threshold = 512;
	ssize_t limit;
	int ret;
	struct qcsapi_output_bufinfo *bi = NULL;

	if (out_type == OUT) {
		bi = &qo->out;
	} else {
		bi = &qo->err;
	}

	limit = bi->bufsize - bi->bytes_written - 1;

	if ((qo->flags & QCSAPI_OUTPUT_REALLOC) &&
			(*bi->buf == NULL ||
			 limit < realloc_threshold)) {
		char *newbuf;
		ssize_t newbufsize;

		newbufsize = bi->bufsize;
		if (newbufsize < realloc_threshold)
			newbufsize = realloc_threshold;
		newbufsize <<= 1;

		newbuf = realloc(*bi->buf, newbufsize);
		if (newbuf == NULL) {
			return -ENOMEM;
		}
		*bi->buf = newbuf;
		bi->bufsize = newbufsize;

		limit = bi->bufsize - bi->bytes_written - 1;
	}

	if (limit <= 0) {
		ret = 0;
	} else {
		ret = vsnprintf(&(*bi->buf)[bi->bytes_written], limit, format, args);
		bi->bytes_written += ret;
		(*bi->buf)[bi->bytes_written] = '\0';
	}

	return ret;
}

struct qcsapi_output qcsapi_output_stdio_adapter(void)
{
	struct qcsapi_output qo = {0};

	qo.func = qcsapi_output_stdio_fn;

	return qo;
}

struct qcsapi_output qcsapi_output_buf_adapter(
		char **outbuf, size_t outbufsize,
		char **errbuf, size_t errbufsize,
		int realloc_allowed)
{
	struct qcsapi_output qo;

	qo.func = qcsapi_output_buf_fn;
	qo.flags = realloc_allowed ? QCSAPI_OUTPUT_REALLOC : 0;

	qo.out.buf = outbuf;
	qo.out.bufsize = outbufsize;
	qo.out.bytes_written = 0;

	qo.err.buf = errbuf;
	qo.err.bufsize = errbufsize;
	qo.err.bytes_written = 0;

	return qo;
}

