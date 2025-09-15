/*SH1
*******************************************************************************
**                                                                           **
**         Copyright (c) 2009 - 2011 Quantenna Communications Inc            **
**                                                                           **
**  File        : c_rpc_qcsapi_sample.c                                      **
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>

#include "qcsapi_output.h"
#include "../qcsapi_rpc_common/client/find_host_addr.h"

#include "qcsapi.h"
#include "../qcsapi_rpc/client/qcsapi_rpc_client.h"
#include "./qcsapi_rpc/generated/qcsapi_rpc.h"
#include "qcsapi_driver.h"
#include "call_qcsapi.h"

#define MAX_RETRY_TIMES 15
#define WIFINAME "wifi0"

static int s_c_rpc_use_udp = 0;

/*=============================================================================
FUNCTION:		c_rpc_qcsapi_wps_push_button
DESCRIPTION:		Start the WPS by QCSAPI
ARGUMENTS PASSED:
RETURN VALUE:		0:success, other:error
=============================================================================*/
int c_rpc_qcsapi_wps_push_button()
{
	int ret;
	qcsapi_mac_addr bssid;
	memset(bssid, 0, MAC_ADDR_SIZE);
	ret = qcsapi_wps_enrollee_report_button_press(WIFINAME, bssid);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wps_enrollee_report_button_press error, return: %d\n", ret);
		return -1;
	}
	printf("WPS push button started\n");
	return 0;
}

/*=============================================================================
FUNCTION:		c_rpc_qcsapi_get_rssi
DESCRIPTION:		1.Check if the association established.
			2.If associtied, get the rssi value by QCSAPI
ARGUMENTS PASSED:
RETURN VALUE:		0:success, other:error
=============================================================================*/
int c_rpc_qcsapi_get_rssi()
{
	int ret;
	int rssi=0;
	qcsapi_unsigned_int assoc_cnt;

	//Get the association count
	//if assoc_cnt is 0, it means not associated
	ret = qcsapi_wifi_get_count_associations(WIFINAME, &assoc_cnt);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_count_associations error, return: %d\n", ret);
		return -1;
	}
	//Has the association
	if ( assoc_cnt == 0){
		printf("Device not associated\n");
	} else {
		ret = qcsapi_wifi_get_rssi_in_dbm_per_association(WIFINAME, 0, &rssi);
		if (ret < 0) {
			printf("enrollee report button press return %d\n", ret);
			return -1;
		}
		printf("RSSI: %d dbm\n",rssi);
	}
	return 0;
}

/*=============================================================================
FUNCTION:		c_rpc_qcsapi_get_ssid
DESCRIPTION:		Get the current ssid.
			If the device is not associated, the ssid could be empty.
ARGUMENTS PASSED:
RETURN VALUE:		0:success, other:error
=============================================================================*/
int c_rpc_qcsapi_get_ssid()
{
	int ret;
	qcsapi_SSID ssid;
	ret = qcsapi_wifi_get_SSID(WIFINAME, ssid);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_SSID error, return: %d\n", ret);
		return -1;
	}
	printf("Current SSID: %s\n",ssid);
	return 0;
}

/*=============================================================================
FUNCTION:		c_rpc_qcsapi_start_scan
DESCRIPTION:		Start the scan.
ARGUMENTS PASSED:
RETURN VALUE:		0:success, other:error
=============================================================================*/
int c_rpc_qcsapi_start_scan()
{
	int ret;
	ret = qcsapi_wifi_start_scan(WIFINAME);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_start_scan error, return: %d\n", ret);
		return -1;
	}
	printf("Scan started\n");
	return 0;
}

/*=============================================================================
FUNCTION:		c_rpc_qcsapi_get_ap_properties
DESCRIPTION:		Get the scaned AP properties and print.
ARGUMENTS PASSED:
RETURN VALUE:		0:success, other:error
=============================================================================*/
int c_rpc_qcsapi_get_ap_properties()
{
	int ret,i;
	unsigned int ap_count = 0;
	qcsapi_ap_properties ap_current;

	//Get the scaned AP count
	ret = qcsapi_wifi_get_results_AP_scan(WIFINAME, &ap_count);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_results_AP_scan error, return: %d\n", ret);
		return -1;
	}
	if (ap_count == 0) {
		printf("Scaned ap count is 0\n");
		return -1;
	}
	for (i = 0; i < ap_count; i++) {
		ret = qcsapi_wifi_get_properties_AP(WIFINAME, i, &ap_current);
		if (ret < 0) {
			printf("Qcsapi qcsapi_wifi_get_properties_AP error, return: %d\n", ret);
			return -1;
		}
		printf
		    ("AP %02d:\tSSID:%30s\tMAC:%02X:%02X:%02X:%02X:%02X:%02X\tSecurity:%d\tRSSI:%02d\tChannel:%02d\tWPS:%d\n",
		     i, ap_current.ap_name_SSID, ap_current.ap_mac_addr[0], ap_current.ap_mac_addr[1],
		     ap_current.ap_mac_addr[2], ap_current.ap_mac_addr[3], ap_current.ap_mac_addr[4],
		     ap_current.ap_mac_addr[5], ap_current.ap_flags, (ap_current.ap_RSSI-90),
		     ap_current.ap_channel, ap_current.ap_wps);
	}
	return 0;
}

/*=============================================================================
FUNCTION:		print_help
DESCRIPTION:		Print the supported option list
ARGUMENTS PASSED:
RETURN VALUE:
=============================================================================*/
void print_help()
{
	printf("RPC Qcsapi Sample:\n");
	printf("\t-h: Help\n");
	printf("\t-w: Wps push button\n");
	printf("\t-r: get Rssi\n");
	printf("\t-c: get Current ssid\n");
	printf("\t-s: start Scan\n");
	printf("\t-g: Get ap properties\n");
	printf("\t-u: Use UDP as the transport layer for RPC (default to TCP)\n");
	return;
}

/*=============================================================================
FUNCTION:		process_option
DESCRIPTION:		Process all the options with corresponding functions
ARGUMENTS PASSED:	int argc, char **argv
RETURN VALUE:
=============================================================================*/
void process_option(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, "hwrcsg")) != -1){
		switch (c) {
		case 'h':	//Help print
			print_help();
			break;
		case 'w':	//WPS push button
			c_rpc_qcsapi_wps_push_button();
			break;
		case 'r':	//get rssi
			c_rpc_qcsapi_get_rssi();
			break;
		case 'c':	//get the current ssid
			c_rpc_qcsapi_get_ssid();
			break;
		case 's':	//start scan
			c_rpc_qcsapi_start_scan();
			break;
		case 'g':	//get the ap properties list
			c_rpc_qcsapi_get_ap_properties();
			break;
		case 'u':	//use UDP as the transport. Default is TCP
			s_c_rpc_use_udp = 1;
			break;
		default:
			print_help();
			break;
		}
	}
}

int main(int argc, char **argv)
{
	int retry = 0;
	const char *host;
	CLIENT *clnt;

	/* print help if no arguments */
	if (argc == 1) {
		print_help();
		exit(1);
	}

	/* setup RPC based on udp protocol */
	while (retry++ < MAX_RETRY_TIMES) {

		host = client_qcsapi_find_host_addr(&argc, &argv);
		if (!host) {
			client_qcsapi_find_host_errmsg(argv[0]);
			sleep(1);
			continue;
		}

		if (!s_c_rpc_use_udp) {
			clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
		} else {
			clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
		}
		if (clnt == NULL) {
			clnt_pcreateerror(host);
			sleep(1);
			continue;
		} else {
			client_qcsapi_set_rpcclient(clnt);
			break;
		}
	}

	/* could not find host or create a client, exit */
	if (retry >= MAX_RETRY_TIMES)
		exit(1);

	process_option(argc, argv);

	clnt_destroy(clnt);

	return 0;
}
