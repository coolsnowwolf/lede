/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    wfa_p2p.h
 
    Abstract:
	Defined status code, IE and frame structures that WiFi Direct needed.
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    
 */


#ifndef __WFA_P2P_H
#define __WFA_P2P_H

#include "rtmp_type.h"


#if defined(P2P_SUPPORT) || defined(RT_CFG80211_SUPPORT)

typedef	struct	_P2P_PUBLIC_FRAME	{
	HEADER_802_11   p80211Header;
	UCHAR          Category;
	UCHAR           Action;
	UCHAR           OUI[3];
	UCHAR		OUIType;
	UCHAR		Subtype;
	UCHAR		Token;
	UCHAR            ElementID;
	UCHAR            Length;
	UCHAR            OUI2[3];
	UCHAR            OUIType2;
	UCHAR            Octet[1];
}
	P2P_PUBLIC_FRAME, *PP2P_PUBLIC_FRAME;

typedef	struct	_P2P_ACTION_FRAME	{
	HEADER_802_11			p80211Header;
	UCHAR					Category;
	UCHAR					OUI[3];
	UCHAR					OUIType;
	UCHAR					Subtype;
	UCHAR					Token;
	UCHAR					Octet[1];
}	P2P_ACTION_FRAME, *PP2P_ACTION_FRAME;

typedef struct _FRAME_P2P_ACTION {
	HEADER_802_11			Hdr;
	UCHAR					Category;
	UCHAR					OUI[3];
	UCHAR					OUIType;	/* 1 */
	UCHAR					OUISubType;	/* 1 */
	UCHAR					Token;	/* 1 */
}   FRAME_P2P_ACTION, *PFRAME_P2P_ACTION;

typedef struct _MLME_P2P_ACTION_STRUCT {
	UCHAR					TabIndex;	/* sometimes it's Mactable index, sometime it's P2P table index. depend on the command. */
	UCHAR					Addr[MAC_ADDR_LEN];
} MLME_P2P_ACTION_STRUCT, *PMLME_P2P_ACTION_STRUCT;

typedef	struct	_P2P_NOA_DESC	{
	UCHAR					Count;
	UCHAR					Duration[4];
	UCHAR					Interval[4];
	UCHAR					StartTime[4];
}	P2P_NOA_DESC, *PP2P_NOA_DESC;


/* General format of P2P attribut(4.1.1) */
#define SUBID_P2P_STATUS				0
#define SUBID_P2P_MINOR_REASON			1
#define SUBID_P2P_CAP					2
#define SUBID_P2P_DEVICE_ID				3
#define SUBID_P2P_OWNER_INTENT			4
#define SUBID_P2P_CONFIG_TIMEOUT		5
#define SUBID_P2P_LISTEN_CHANNEL		6
#define SUBID_P2P_GROUP_BSSID			7
#define SUBID_P2P_EXT_LISTEN_TIMING		8
#define SUBID_P2P_INTERFACE_ADDR		9
#define SUBID_P2P_MANAGEABILITY			10
#define SUBID_P2P_CHANNEL_LIST			11
#define SUBID_P2P_NOA					12
#define SUBID_P2P_DEVICE_INFO			13
#define SUBID_P2P_GROUP_INFO			14
#define SUBID_P2P_GROUP_ID				15
#define SUBID_P2P_INTERFACE				16
#define SUBID_P2P_OP_CHANNEL			17
#define SUBID_P2P_INVITE_FLAG			18
#define SUBID_P2P_VENDOR				221

/* Status Code definitions(4.1.2) */
#define P2PSTATUS_SUCCESS					0
#define P2PSTATUS_PASSED					1
#define P2PSTATUS_IMCOMPA_PARM				2
#define P2PSTATUS_LIMIT						3
#define P2PSTATUS_INVALID_PARM				4
#define P2PSTATUS_UNABLE					5
#define P2PSTATUS_NO_CHANNEL				7
#define P2PSTATUS_UNKNOWN_GROUP				8
#define P2PSTATUS_BOTH_INTENT15				9
#define P2PSTATUS_INCOMPATIBLE_PROVISION	10
#define P2PSTATUS_REJECT_BY_USER			11

//bitmap of P2P Capability/ 1st byte is DEVCAP, 2nd byte is GRPCAP
/* Device Capability Bitmap definitio(4.1.4) */
#define DEVCAP_SD					0x1	//Service discovery
#define DEVCAP_CLIENT_DISCOVER		0x2
#define DEVCAP_CLIENT_CONCURRENT	0x4
#define DEVCAP_INFRA_MANAGED		0x8
#define DEVCAP_DEVICE_LIMIT			0x10
#define DEVCAP_INVITE				0x20

/*Group Capability Bitmap definition(4.1.4) */
#define GRPCAP_OWNER				0x1
#define GRPCAP_PERSISTENT			0x2
#define GRPCAP_LIMIT				0x4
#define GRPCAP_INTRA_BSS			0x8
#define GRPCAP_CROSS_CONNECT		0x10
#define GRPCAP_PERSISTENT_RECONNECT	0x20
#define GRPCAP_GROUP_FORMING		0x40


/* P2P public action frame typ(4.2.8) */
#define GO_NEGOCIATION_REQ			0
#define GO_NEGOCIATION_RSP			1
#define GO_NEGOCIATION_CONFIRM		2
#define P2P_INVITE_REQ				3
#define P2P_INVITE_RSP				4
#define P2P_DEV_DIS_REQ				5
#define P2P_DEV_DIS_RSP				6
#define P2P_PROVISION_REQ			7
#define P2P_PROVISION_RSP			8

/* P2P action frame typ(4.2.9) */
#define P2PACT_NOA					0
#define P2PACT_PERSENCE_REQ			1
#define P2PACT_PERSENCE_RSP			2
#define P2PACT_GO_DISCOVER_REQ		3

/* Service Protocol Type */
#define SERVICE_PROTOCOL_TYPE_ALL				0
#define SERVICE_PROTOCOL_TYPE_BONJOUR			1
#define SERVICE_PROTOCOL_TYPE_UPNP				2
#define SERVICE_PROTOCOL_TYPE_WS_DISCOVERY		3
#define SERVICE_PROTOCOL_TYPE_WFD				4
#define SERVICE_PROTOCOL_TYPE_VENDOR_SPECIFIC	255

#endif /* P2P_SUPPORT */

#endif /* __WFA_P2P_H */

