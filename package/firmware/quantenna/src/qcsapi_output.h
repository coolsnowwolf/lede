/*SH1
*******************************************************************************
**                                                                           **
**         Copyright (c) 2009 - 2011 Quantenna Communications Inc            **
**                                                                           **
**  File        : qcsapi.h                                                   **
**  Description :                                                            **
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
EH1*/

#ifndef _QCSAPI_OUTPUT_H
#define _QCSAPI_OUTPUT_H

#include <sys/types.h>
#include <stdarg.h>

enum qcsapi_print_type {
	OUT,
	ERR,
};

struct qcsapi_output_bufinfo {
	char	**buf;
	ssize_t	bufsize;
	ssize_t	bytes_written;
};

typedef struct qcsapi_output {
	int (*func)(struct qcsapi_output*, enum qcsapi_print_type, const char *, va_list args);
	struct qcsapi_output_bufinfo out;
	struct qcsapi_output_bufinfo err;
#define QCSAPI_OUTPUT_REALLOC	0x1
	int flags;
} qcsapi_output;

extern struct qcsapi_output qcsapi_output_stdio_adapter(void);
extern struct qcsapi_output qcsapi_output_buf_adapter(char **stdout_buf, size_t stdout_bufsize,
		char **stderr_buf, size_t stderr_bufsize, int realloc_allowed);

static inline int print_out(struct qcsapi_output *output, const char *format, ...)
{
	int ret;
	va_list args;

	va_start(args, format);
	ret = output->func(output, OUT, format, args);
	va_end(args);

	return ret;
}

static inline int print_err(struct qcsapi_output *output, const char *format, ...)
{
	int ret;
	va_list args;

	va_start(args, format);
	ret = output->func(output, ERR, format, args);
	va_end(args);

	return ret;
}

#endif	/* _QCSAPI_OUTPUT_H */
