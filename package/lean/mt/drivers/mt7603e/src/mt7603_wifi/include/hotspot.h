/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	hotspot.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

#ifndef CONFIG_DOT11V_WNM 
#error "For HOTSPOT2.0 feature, you must define the compile flag -DCONFIG_DOT11V_WNM"
#endif

#ifndef DOT11U_INTERWORKING
#error "For HOTSPOT2.0 feature, you must define the compile flag -DCONFIG_DOT11U_INTERWORKING"
#endif

#include "rtmp.h"

#define HSCTRL_MACHINE_BASE 0

enum HSCTRL_STATE {
	WAIT_HSCTRL_OFF,
	WAIT_HSCTRL_ON,
	HSCTRL_IDLE,
	MAX_HSCTRL_STATE,
};

enum HSCTRL_EVENT {
	HSCTRL_OFF,
	HSCTRL_ON,
	MAX_HSCTRL_MSG,
};

typedef struct GNU_PACKED _HSCTRL_EVENT_DATA {
	UCHAR ControlIndex;
	UCHAR EventTrigger;
	UCHAR EventType;
} HSCTRL_EVENT_DATA, *PHSCTRL_EVENT_DATA;

typedef struct _HOTSPOT_CTRL {
	UINT32 HSIndicationIELen;
	UINT32 P2PIELen;
	/*UINT32 InterWorkingIELen;
	UINT32 AdvertisementProtoIELen;
	*/
	UINT32 QosMapSetIELen;
	UINT32 RoamingConsortiumIELen;
	/*PUCHAR InterWorkingIE;
	PUCHAR AdvertisementProtoIE;
	*/
	PUCHAR QosMapSetIE;
	USHORT DscpRange[8];
	USHORT DscpException[21];	
	PUCHAR RoamingConsortiumIE;
	PUCHAR HSIndicationIE;
	PUCHAR P2PIE;
	BOOLEAN  HSDaemonReady;
	BOOLEAN HotSpotEnable;
	enum HSCTRL_STATE HSCtrlState;
	BOOLEAN IsHessid;
	UCHAR Hessid[MAC_ADDR_LEN];
	UINT8 AccessNetWorkType;
	BOOLEAN DGAFDisable;
	UINT8 L2Filter;
	BOOLEAN ICMPv4Deny;
	BOOLEAN QosMapEnable;
/* BOOLEAN bNonTX;	0:enable TX, 1:disable TX*/
	BOOLEAN bASANEnable;		/* 1:enable ASAN IE, 1:disable ASAN IE*/
	UCHAR  	QLoadTestEnable;	/* for BSS Load IE Test */
	UCHAR  	QLoadCU;			/* for BSS Load IE Test */
	USHORT 	QLoadStaCnt;		/* for BSS Load IE Test */
} HOTSPOT_CTRL, *PHOTSPOT_CTRL;

enum {
	L2FilterDisable,
	L2FilterBuiltIn,
	L2FilterExternal,
};

/*enum {
	PARAM_DGAF_DISABLED,
	PARAM_PROXY_ARP,
	PARAM_L2_FILTER,
	PARAM_ICMPV4_DENY,
	PARAM_MMPDU_SIZE,
	PARAM_EXTERNAL_ANQP_SERVER_TEST,
	PARAM_GAS_COME_BACK_DELAY,
	PARAM_WNM_NOTIFICATION,
	PARAM_QOSMAP,
}; move to wapp_cmm_type.h
*/

BOOLEAN L2FilterInspection(
			IN PRTMP_ADAPTER pAd,
			IN PHOTSPOT_CTRL pHSCtrl,	
			IN PUCHAR pData);

VOID HSCtrlStateMachineInit(
	IN	PRTMP_ADAPTER		pAd, 
	IN	STATE_MACHINE		*S, 
	OUT	STATE_MACHINE_FUNC	Trans[]);

INT Set_STAHotSpot_OnOff(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OnOff);

enum HSCTRL_STATE HSCtrlCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

BOOLEAN HotSpotEnable(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN INT Type);

VOID HSCtrlExit(
	IN PRTMP_ADAPTER pAd);

VOID HSCtrlHalt(
	IN PRTMP_ADAPTER pAd);

INT Set_HotSpot_OnOff(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OnOff,
	IN UINT8 EventTrigger,
	IN UINT8 EventType);

struct _PEER_PROBE_REQ_PARAM;

BOOLEAN ProbeReqforHSAP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR APIndex,
	IN struct _PEER_PROBE_REQ_PARAM *ProbeReqParam);

VOID Clear_Hotspot_All_IE(IN PRTMP_ADAPTER PAd);

#define isBcastEtherAddr(addr)  ((addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5])== 0xff) 

#define GAS_STATE_MESSAGES    0
#define ACTION_STATE_MESSAGES 1

void HotspotOnOffEvent(PNET_DEV net_dev, int onoff);
void HotspotAPReload(PNET_DEV net_dev);

INT Set_HotSpot_Param(
	IN PRTMP_ADAPTER pAd,
	UINT32 Param,
	UINT32 Value);

enum {
	HS_ON_OFF_BASE,
	HS_AP_RELOAD,
};

#ifdef CONFIG_AP_SUPPORT
BOOLEAN HSIPv4Check(
			IN PRTMP_ADAPTER pAd,
			PUSHORT pWcid,
			PNDIS_PACKET pPacket,
			PUCHAR pSrcBUf,
			UINT16 srcPort,
			UINT16 dscPort);
#endif

#ifdef CONFIG_HOTSPOT_R2	
struct _sta_hs_info {
	//UCHAR	addr[6];
	UCHAR	version;
	UCHAR	ppsmo_exist;
	USHORT	ppsmo_id;
};
#endif
#endif /* __HOTSPOT_H__ */

