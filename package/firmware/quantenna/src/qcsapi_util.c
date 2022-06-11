/*SH0
*******************************************************************************
**                                                                           **
**           Copyright (c) 2015 Quantenna Communications, Inc.               **
**                                                                           **
**  File        : qcsapi_util.h                                              **
**  Description : utility functions to be used by qcsapi_* and call_qcsapi   **
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

#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "qcsapi_util.h"

/*
 * verify function return negative value when the parameter_value is not valid
 */
int qcsapi_verify_numeric(const char *parameter_value)
{
	while (*parameter_value != '\0') {
		if (!isdigit(*parameter_value))
			return -1;
		parameter_value++;
	}
	return 0;
}

/*
 * Conversion from string to unsigned integer.
 * Handles invalid strings and integer overflows.
 * return:
 *  0 - on success
 *  -1 - on error
 */
int qcsapi_str_to_uint32(const char *str, uint32_t *result)
{
	char *endptr = NULL;
	uint32_t res;

	while (isspace(*str)) {
		str++;
	}

	if (!isdigit(*str)) {
		return -1;
	}

	errno = 0;
	res = strtoul(str, &endptr, 10);
	if (errno != 0) {
		return -1;
	}

	if (!endptr || endptr == str) {
		return -1;
	}

	while (isspace(*endptr)) {
		endptr++;
	}

	if (*endptr != '\0') {
		return -1;
	}

	*result = res;
	return 0;
}

#define QCSAPI_MAX_ETHER_STRING 17

int parse_mac_addr(const char *mac_addr_as_str, qcsapi_mac_addr mac_addr)
{
	int i;
	int mac_len = strnlen(mac_addr_as_str, QCSAPI_MAX_ETHER_STRING + 1);
	unsigned int tmp[sizeof(qcsapi_mac_addr)];
	int retval;

	if (mac_addr_as_str == NULL)
		return -qcsapi_invalid_mac_addr;

	if (mac_len > QCSAPI_MAX_ETHER_STRING) {
		return -qcsapi_invalid_mac_addr;
	}

	for (i = 0; i < mac_len; i++) {
		if (!(isxdigit(mac_addr_as_str[i]) || (mac_addr_as_str[i] == ':')))
			return -qcsapi_invalid_mac_addr;
	}

	retval = sscanf(mac_addr_as_str, "%x:%x:%x:%x:%x:%x",
			&tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]);
	if (retval != sizeof(qcsapi_mac_addr))
		return -qcsapi_invalid_mac_addr;

	for (i = 0; i < sizeof(qcsapi_mac_addr); i++) {
		if (tmp[i] > 0xff)
			return -qcsapi_invalid_mac_addr;
	}

	mac_addr[0] = (uint8_t) tmp[0];
	mac_addr[1] = (uint8_t) tmp[1];
	mac_addr[2] = (uint8_t) tmp[2];
	mac_addr[3] = (uint8_t) tmp[3];
	mac_addr[4] = (uint8_t) tmp[4];
	mac_addr[5] = (uint8_t) tmp[5];

	return 0;
}

