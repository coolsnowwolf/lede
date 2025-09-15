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

#include <stdio.h>
#include <call_qcsapi.h>
#include <qcsapi_output.h>
#include <qcsapi_rpc/client/qcsapi_rpc_client.h>
#include <qcsapi_rpc/generated/qcsapi_rpc.h>
#include <qcsapi_rpc_common/common/rpc_pci.h>

int main(int argc, char **argv)
{
	int ret;
	char *host;
	CLIENT *clnt;
	struct qcsapi_output output;

	output = qcsapi_output_stdio_adapter();

	host = "localhost";

	clnt = clnt_pci_create(host, QCSAPI_PROG, QCSAPI_VERS, NULL);
	if (clnt == NULL) {
		clnt_pcreateerror(host);
		exit (1);
	}

	client_qcsapi_set_rpcclient(clnt);

	ret = qcsapi_main(&output, argc, argv);

	clnt_destroy(clnt);

	return ret;
}

