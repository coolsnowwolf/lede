/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

#ifndef __UAPI_ECNT_EVENT_DBUS_H_
#define __UAPI_ECNT_EVENT_DBUS_H_

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/


/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

#define	CTC_GW_SERVICE_NAME_SYSTEM			"SYSTEM"
#define CTC_GW_SERVICE_NAME_HTTP			"http"
#define	CTC_GW_SERVICE_NAME_VOICE			"voice"
#define	CTC_GW_SERVICE_NAME_WIFI			"wifi"
#define	CTC_GW_SERVICE_NAME_DBUS			"igddbus"
#define CTC_GW_SERVICE_NAME_SAMBA			"samba"
#define	CTC_GW_SERVICE_NAME_APPMGR			"appmgr"
#define	CTC_GW_SERVICE_NAME_CLOUDCLI		"cloudclient"
#define	CTC_GW_SERVICE_NAME_SAF				"saf"
#define	CTC_GW_SERVICE_NAME_TR069C			"tr069c"
typedef enum {
	ECNT_EVENT_DBUS_INFORM,	
	ECNT_EVENT_DBUS_SYSCPUTEMPALARM,			/*SysCPUTempAlarm*/
	ECNT_EVENT_DBUS_SYSPONTEMPALARM,			/*SysPONTempAlarm*/
	ECNT_EVENT_DBUS_SYSWRITABLEFLASHALARM,		/*SysWritableFlashAlarm*/
	ECNT_EVENT_DBUS_SYSCONNNUMALARM,			/*SysConnNumAlarm*/
	ECNT_EVENT_DBUS_SYSMEMCONTROL,				/*SysMemControl*/
	ECNT_EVENT_DBUS_DNSSPEEDLIMIT,				/*DnsSpeedLimitAlarm*/
	ECNT_EVENT_DBUS_ITVLINESTATUS,				/*ITVLineStatus*/
	ECNT_EVENT_DBUS_SYSNONEEDREBOOT,        	/*SysNoNeedReboot*/
	ECNT_EVENT_DBUS_PROC_STAT,
	ECNT_EVENT_DBUS_VOICE_POORQL,
	ECNT_EVENT_DBUS_QOS,
	ECNT_EVENT_DBUS_RESOURCE,
	ECNT_EVENT_DBUS_NOTIFYDESTADDRESS,			/* Traffic Monitor */
	ECNT_EVENT_DBUS_NOTIFYHTTPTRAFFIC,
	ECNT_EVENT_DBUS_IPPINGRESPONSE,				/* IPPingDiagnostics */
	ECNT_EVENT_DBUS_TRACEROUTERESPONSE,			/* TraceRouteDiagnostics */
	ECNT_EVENT_DBUS_NOTIFYLANDEVICEACCESS,      /* LanDeviceAccess */
	ECNT_EVENT_DBUS_CHECKHARDWARESTATUS,		/* Checkhardwarestatus */
	ECNT_EVENT_DBUS_APPFILTEREVENT,
	
	ECNT_EVENT_DBUS_MAX=255
}ECNT_EVENT_DBUS_SubType_t;

#define TR069C_SEND_FLAG				"/tmp/tr069c_send_flag"
#define	DBUS_SEND_FLAG					"/tmp/dbus_send_flag"
#define WIFI_SEND_FLAG					"/tmp/wifi_send_flag"

/* dbus name */
#if defined(TCSUPPORT_CUC)
#define IGD_NAME						"com.cuc.igd1"
#define IGD_PATH						"/com/cuc/igd1"
#define GDBUS_SAF						"com.cuc.ufw1"
#define GDBUS_SAF_PATH					"/com/cuc/ufw1"
#define OPERATOR_NAME					"Unicom"
#define GW_SERVICE_NAME					"ufwmg"
#define GW_SERVICE_PID_FILE				"/var/run/cuc_"GW_SERVICE_NAME".pid"
#else
#define IGD_NAME						"com.ctc.igd1"
#define IGD_PATH						"/com/ctc/igd1"
#define GDBUS_SAF						"com.ctc.saf1"
#define GDBUS_SAF_PATH					"/com/ctc/saf1"
#define OPERATOR_NAME					"Telecom"
#define ECNT_GW_SERVICE_NAME			"saf"
#define GW_SERVICE_NAME					"saf"
#define GW_SERVICE_PID_FILE				"/var/run/ctc_"GW_SERVICE_NAME".pid"
#endif

#define GDBUS_IFACE_PROPERTIES			IGD_NAME".Properties"


/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
struct traffic_monitor_detect
{
	int state;
	char destAddress[64];
	char wanIPAddress[64];
	char pppoeName[128];
};

struct traffic_http_process
{
	char reqLine[120];
	char statusCode[8];
	char hdrFields[888];
};

#if defined(TCSUPPORT_CUC) || defined(TCSUPPORT_CT_UBUS)
struct hardware_status_info
{
	char CPUStatus[16];
	char WIFIStatus[16];
	char LSWStatus[16];
	char CodecStatus[16];
	char OpticStatus[16];
	char LAN1Status[16];
	char LAN2Status[16];
	char LAN3Status[16];
	char LAN4Status[16];
};

typedef struct lan_notify_info_s
{
	char DevName[256];
	char DevType[24];
	char DevMAC[24];
	char DevIP[24];
	unsigned char ConnectType;
	unsigned char Port;
	char Brand[64];
	char OS[64];
	char Action[64];
}lan_notify_info_t, *lan_notify_info_ptr;
#endif
/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/


/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
#endif

