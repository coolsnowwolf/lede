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

#ifndef NOTIFY_H
#define NOTIFY_H

/**
* \file  notify.h  
* \brief This file is notify header file that will be exported for others to use.
* \author Lei.Zhang
* \date     2020-09-24
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

#include <stdlib.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>

#define DBUS_MQ_FLAG_PATH "/tmp/dbus_mq"
#define DBUS_PROJ_ID 0x05

#define NOTIFY_TCAPI_SAVE_CALLED 	1 /*message queue for tcapi_save been called*/
#define NOTIFY_OBJECT_ADD		 	2 /*message queue for adding object*/
#define NOTIFY_OBJECT_DEL		 	3 /*message queue for deleteing object*/
#define NOTIFY_OBJECT_INITALL		4 /*message queue for init notify tab*/

typedef struct _dbus_notify_msg_data_s
{
	int notify_type;
	char object_path[64];
	char interface_name[64];
}dbus_notify_msg_data_t;

typedef struct _dbus_notify_msg_s
{
	long msgq_type;
	dbus_notify_msg_data_t text;
}dbus_notify_msg_t;

static inline int ctc_notify2dbus(int notify_type, char* object_path, char* interface_name)
{
	int ret;
	key_t mqkey;
	int mqid;
	dbus_notify_msg_t notify_msg;

	memset(&notify_msg, 0, sizeof(dbus_notify_msg_t));

	notify_msg.msgq_type = 1;
	notify_msg.text.notify_type = notify_type;
	if (NULL != object_path)
	{
		strncpy(notify_msg.text.object_path, object_path, 
			sizeof(notify_msg.text.object_path) - 1);
	}

	if (NULL != interface_name)
	{
		strncpy(notify_msg.text.interface_name, interface_name, 
			sizeof(notify_msg.text.interface_name) - 1);
	}
	
	mqkey = ftok(DBUS_MQ_FLAG_PATH, DBUS_PROJ_ID);
	mqid = msgget(mqkey, 0);   
	if(mqid < 0)
	{
		printf("\r\n open message queue fail!");
		return -1;
	}
	ret = msgsnd(mqid, &notify_msg, sizeof(dbus_notify_msg_data_t), IPC_NOWAIT);
	if(ret < 0)
	{
		printf("\r\n write message fail!");
		return -1;
	}
	
	return ret;	
}

#define CTCAPD_MQ_FLAG_PATH "/tmp/ctcapd_mq"
#define CTCAPD_PROJ_ID 0x06

#define CTCAPD_MAX_MSG_LEN	(256)

#define NOTIFY_TCAPI_SAVE_CALLED 	1 /*message queue for tcapi_save been called*/
#define NOTIFY_WANINFO_IP_CHANGE	2 /*message queue for wan ip changed*/
#define NOTIFY_LED_STATUS_CHANGE	3 /*message queue for led status changed*/
#if 0

#define NOTIFY_OBJECT_DEL		 	3 /*message queue for deleteing object*/
#define NOTIFY_OBJECT_INITALL		4 /*message queue for init notify tab*/
#endif
#define NOTIFY_LANINFO_NEWDEVICE 4 /*message queue for new device*/
#define NOTIFY_LANINFO_DEVICESTATUS_CHANGE 5/*message queue for laninfo devicestatus changed*/
#define NOTIFY_DIAG_PING 6
#define NOTIFY_DIAG_TRACE 7
#define NOTIFY_WIFI_STATUS_CHANGE          8 /*message queue for wifi status changed*/ 
#define NOTIFY_WIFI_PSWERR_STATUS   	   9 /*message queue for wifi passworderror*/ 
#define NOTIFY_WIFI_MESHMAP_STATUS   	   10 /*message queue for mesh mapinfo*/ 
#define NOTIFY_WIFI_ROAM_STATUS   		   11 /*message queue for wifi roaming notification*/ 
#define NOTIFY_WLAN24G_CHANNEL_CHANGE      12/*message queue for wifi 2.4G channel changed*/
#define NOTIFY_WLAN5G_CHANNEL_CHANGE       13/*message queue for wifi 5G channel changed*/
#define NOTIFY_WLAN24G_SSID_CHANGE         14/*message queue for wifi 2.4G ssid changed*/
#define NOTIFY_WLAN5G_SSID_CHANGE          15/*message queue for wifi 5G ssid changed*/
#define NOTIFY_PROBERX_VSIE_RECORD         16/*message queue for wifi proberx vsie record received*/

typedef struct waninfo_msg_s{
	char ipaddr[16];
	char status[16];
}waninfo_msg_t;
typedef struct ledstatus_msg_s{
	char status[4];
}ledstatus_msg_t;

typedef struct wifistatus_msg_s{
	char wifiswitch[4];
	char wpsswitch[4];
	char elinksync[4];
}wifistatus_msg_t;

typedef struct wifipswerr_msg_s{
	char stamac[16];
}wifipswerr_msg_t;

typedef struct wifimeshinfo_msg_s{
	char almac[16];
	char mode[10];
	char ssid[32];
	char key[32];
	char auth[32];
	char encrypt[32];
	char ssid11ac[32];
	char key11ac[32];
	char auth11ac[32];
	char encrypt11ac[32];
}wifimeshinfo_msg_t;

typedef struct wifiroamnotifi_msg_s{
	char mac[16];
	char connectime[32];
	char rssi[32];
	char band[4];
	char channel[32];
	char bssid[16];
	char support11k[4];
	char support11v[4];
	
}wifiroamnotifi_msg_t;

typedef struct _ctcapd_notify_msg_data_s
{
	int notify_type;
	union
	{
		char reserved[CTCAPD_MAX_MSG_LEN];
	}msg_data;
}ctcapd_notify_msg_data_t;
typedef struct laninfo_msg_s{
	char mac[24];
	char vmac[24];
	char hostname[100];
	unsigned int connectionType;
	unsigned int port;
	unsigned int active;
}laninfo_msg_t;

typedef struct diagnostics_msg_s{
	unsigned int received;
	unsigned int loss;
	float avg;
	float min;
	float max;
	unsigned int status;
	char traceresult[1000];
}diagnostics_msg_t;
typedef struct wlan_msg_s{
	int channel;
}wlan_msg_t;

typedef struct wlan_ap_msg_s{
	int idx;
	char ssid[64];
}wlan_ap_msg_t;

typedef struct proberx_vsie_record_msg_s{
	int recordIdx;
	int band;
	char mac[32];
	char vsie[256];
}proberx_vsie_record_msg_t;

typedef struct _ctcapd_notify_msg_s
{
	int msgq_type;
	ctcapd_notify_msg_data_t text;
}ctcapd_notify_msg_t;

static inline int ctc_notify2ctcapd(int notify_type, void *data, int data_len)
{
	int ret;
	int cplen = 0;
	key_t mqkey;
	int mqid;
	ctcapd_notify_msg_t notify_msg;

	memset(&notify_msg, 0, sizeof(ctcapd_notify_msg_t));

	notify_msg.msgq_type = 1;
	memset(notify_msg.text.msg_data.reserved, 0, sizeof(notify_msg.text.msg_data.reserved));
	if ( NULL != data && data_len > 0 )
	{
		if ( data_len < sizeof(notify_msg.text.msg_data.reserved) )
		{
			cplen = data_len;
		}
		else
		{
			printf("\r\n ctcapd:input message length is too large! notify_type is [%d].\n", notify_type);
			cplen = sizeof(notify_msg.text.msg_data.reserved) - 1;
		}
		memcpy(notify_msg.text.msg_data.reserved, (void *)data, cplen);
	}
	notify_msg.text.notify_type = notify_type;
	
	mqkey = ftok(CTCAPD_MQ_FLAG_PATH, CTCAPD_PROJ_ID);
	mqid = msgget(mqkey, 0);   
	if ( mqid < 0 )
	{
		printf("\r\n ctcapd:open message queue fail!");
		return -1;
	}
	ret = msgsnd(mqid, &notify_msg, sizeof(ctcapd_notify_msg_data_t), IPC_NOWAIT);
	if ( ret < 0 )
	{
		printf("\r\n ctcapd:write message fail!");
		return -1;
	}
	
	return ret;	
}

#endif /* NOTIFY_H */
