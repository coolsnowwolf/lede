/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36 Taiyuan St., Jhubei City,
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	dot11u_hotspot.h
	
	Revision History:
	Who 			When			What
	--------		----------		----------------------------------------------
	
*/

#ifndef	__DOT11U_HOTSPOT_H__
#define	__DOT11U_HOTSPOT_H__

#include "rtmp_type.h"

/* unit ms */
#define dot11GASResponseTimeout	5120
#define dotGASResponseBufferingTime 5120


enum GAS_ACTION_FIELD {
	ACTION_GAS_INIT_REQ = 10,
	ACTION_GAS_INIT_RSP = 11,
	ACTION_GAS_CB_REQ = 12,
	ACTION_GAS_CB_RSP = 13,
};

typedef struct GNU_PACKED _GAS_FRAME
{
	HEADER_802_11 Hdr;
	UCHAR Category;
	union {
		struct {
			UCHAR Action;
			UCHAR DialogToken;
			/* 
 			 * Following are advertisement protocol element, 
 			 * query request length, and query request
 			 */
			UCHAR Variable[0];
		} GNU_PACKED GAS_INIT_REQ;
		struct {
			UCHAR Action;
			UCHAR DialogToken;
			UINT16 StatusCode;
			UINT16 GASComebackDelay;
			/*
 			 *  Following are advertisement protocol element,
 			 *  query response length, and query response(optional)
 			 */
			UCHAR Variable[0];
		} GNU_PACKED GAS_INIT_RSP;
		struct {
			UCHAR Action;
			UCHAR DialogToken;
		} GNU_PACKED GAS_CB_REQ;
		struct {
			UCHAR Action;
			UCHAR DialogToken;
			UINT16 StatusCode;
			UCHAR GASRspFragID;
			UINT16 GASComebackDelay;
			/*
 			 * Following are advertisment protocol element,
 			 * query response length, and query response(optional)
 			 */
			UCHAR Variable[0]; 
		} GNU_PACKED GAS_CB_RSP;
	}u;
}GAS_FRAME, *PGAS_FRAME;


/* Status Code */
enum DOT11U_STATUS_CODE {
	ADVERTISEMENT_PROTOCOL_NOT_SUPPORTED = 59,	
	UNSPECIFIED_FAILURE = 60,
	RESPONSE_NOT_RECEIVED_FROM_SERVER = 61,
	TIMEOUT = 62,
	QUERY_RESPONSE_TOO_LARGE = 63,
	REQUEST_REFUSED_HOME_NETWORK_NOT_SUPPORT = 64,
	SERVER_UNREACHABLE = 65,
	REQUEST_REFUSED_PERMISSIONS_RECEIVED_FROM_SSPN = 67,
	REQUEST_REFUSED_AP_NOT_SUPPORT_UNAUTH_ACCESS = 68,
	TRANSMISSION_FAILURE = 79,	
};

/* Advertismenet Protocol ID definitions */
enum DOT11U_ADVERTISMENT_PROTOCOL_ID {
	ACCESS_NETWORK_QUERY_PROTOCOL = 0,
	MIH_INFORMATION_SERVICE = 1,
	MIH_COMMAND_AND_EVENT_SERVICES_CAPBILITY_DISCOVERY = 2,
	EMERGENCY_ALERT_SYSTEM = 3,
	VENDOR_SPECIFIC = 221
};

struct anqp_frame {
	UINT16 info_id;
	UINT16 length;
	char variable[0];
} __packed;
#endif /* __DOT11U_HOTSPOT_H__ */

