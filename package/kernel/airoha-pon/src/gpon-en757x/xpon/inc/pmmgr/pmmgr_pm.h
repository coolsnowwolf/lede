/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	omci.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	andy.Yi		2012/7/5	Create
*/

#ifndef _PMMGR_PM_H_
#define _PMMGR_PM_H_
#include "pmmgr.h"

/*******************************************************************************************************************************
genera function

********************************************************************************************************************************/
int findRowBufFirstStr(char * rowBuf, uint32 * fromLen, char * firstStr);


int pmmgrPmInitForFecPM(pmmgrPmMe_t *me_p);
int pmmgrPmInitForGemPortCtpPM(pmmgrPmMe_t *me_p);
int pmmgrPmInitForEnergyConsumPM(pmmgrPmMe_t *me_p);
int getDataFromApi(char *data, uint16 portId);
int getAllDataFromApiForFecPM(char *data, uint16 portId);
int getAllDataFromApiForGemPortCtpPM(char *data, uint16 portId);
int getAllDataFromApiForEnergyConsumPM(char *data, uint16 portId);

/*******************************************************************************************************************************
start
9.3.3 MAC bridge performance monitoring history data ME

********************************************************************************************************************************/
int pmmgrPmInitForMACBridgePM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForMACBridgePM(char *data, uint16 portId);

/*******************************************************************************************************************************
start
9.3.9 mac bridge port performance monitoring history data

********************************************************************************************************************************/

int pmmgrPmInitForMACBridgePortPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForMACBridgePortPM(char *data, uint16 portId);

/*******************************************************************************************************************************
start
9.3.30 ethernet frame performance monitoring history data upstream

********************************************************************************************************************************/
int pmmgrPmInitForEthernetFrameStreamPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForEthernetFrameUpPM(char *data, uint16 portId);

/*******************************************************************************************************************************
start
9.3.31 ethernet frame performance monitoring history data downstream

********************************************************************************************************************************/
int getAllDataFromApiForEthernetFrameDownPM(char *data, uint16 portId);

extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptFecPM[];
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptGemPortCtpPM[];
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEnergyConsumPM[];
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptMACBridgePM[];
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptMACBridgePortPM[];
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEthernetFrameUpPM[];
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEthernetFrameDownPM[];

//9.2.6 GEM port performance monitoring PM
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptGemPortPM[];
int	pmmgrPmInitForGemPortPM(pmmgrPmMe_t * me_p);
int getAllDataFromApiForGemPortPM(char *data, uint16 portId);

#ifdef TCSUPPORT_OMCI_ALCATEL
//ALCATEL Vendor Specific ME:   Total GEM Port PM
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptTotalGemPortPM[];
int pmmgrPmInitForTotalGemPortPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForTotalGemPortPM(char *data, uint16 portId);

//ALCATEL Vendor Specific ME:   Ethernet Traffic PM
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEthernetTrafficPM[];
int pmmgrPmInitForEthernetTrafficPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForEthernetTrafficPM(char *data, uint16 portId);

extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptGEMPortProtocolMonitoringHDPart2PM[];
int pmmgrPmInitForGEMPortProtocolMonitoringHDPart2PM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForGEMPortProtocolMonitoringHDPart2PM(char *data, uint16 portId);
#endif
//9.3.2 Ethernet frame extended PM
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEthernetFrameExtendedPM[];
int pmmgrPmInitForEthernetFrameExtendedPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForEthernetFrameExtendedPM(char *data, uint16 portId);
//9.3.34 Ethernet frame extended PM 64-bit
#define ETHERNET_PM_64_BIT_DIRECTION_DOWM	1
#define ETHERNET_PM_64_BIT_DIRECTION_UP		0
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEthernetFrameExtendedPM64bit[];
int pmmgrPmInitForEthernetFrameExtendedPM64bit(pmmgrPmMe_t *me_p);
int getAllDataFromApiForEthernetFrameExtendedPM64bit(char *data, uint16 portId);
void getEthernetFrameExtendedPmmeDatafromSwitchApi(uint8 validPortId,EthernetFrameExtendedPmcounters_t *pm);


//9.5.2 Ethernet performance monitoring history data
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptPptpEthernetUniPM[];
int pmmgrPmInitForPptpEthernetUniPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForPptpEthernetUniPM(char *data, uint16 portId);

//9.5.3 Ethernet performance monitoring history data 2
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptPptpEthernetUni2PM[];
int pmmgrPmInitForPptpEthernetUni2PM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForPptpEthernetUni2PM(char *data, uint16 portId);

//9.5.4 Ethernet performance monitoring history data 3
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptPptpEthernetUni3PM[];
int pmmgrPmInitForPptpEthernetUni3PM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForPptpEthernetUni3PM(char *data, uint16 portId);

#ifdef TCSUPPORT_VOIP
//9.9.12 call control  performance monitoring history data
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define MAX_PM_ATTRI_STRING_NUM        32
typedef struct PMTcapiNode_s
{
	char attri[MAX_PM_ATTRI_STRING_NUM];
}PMTcapiNode_t;
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptCallCtrlPM[];
int pmmgrPmInitForCallCtrlPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForCallCtrlPM(char *data, uint16 portId);

//9.9.13 rtp performance monitoring history data
#define RTP_TIMESTAMP_UNIT 		80
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptRtpDataPM[];
int pmmgrPmInitForRTPDataPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForRTPDataPM(char *data, uint16 portId);

//9.9.14 SIP agent performance monitoring history data
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptSipAgentPM[];
int pmmgrPmInitForSipAgentPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForSipAgentPM(char *data, uint16 portId);

//9.9.15 SIP call initiation performance monitoring history data
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptSipCallInitPM[];
int pmmgrPmInitForSipCallInitPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForSipCallInitPM(char *data, uint16 portId);
#endif

//9.4.2 IP host config data PM
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptIpHostCfgDataPM[];
int pmmgrPmInitForIpHostCfgDataPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForIpHostCfgDataPM(char *data, uint16 portId);

//9.4.4 TCP/UDP config data PM
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptTcpUdpCfgDataPM[];
int pmmgrPmInitForTcpUdpCfgDataPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForTcpUdpCfgDataPM(char *data, uint16 portId);

//9.2.8 GAL ethernet PM
extern pmmgrPmAttriDescript_t pmmgrPmAttriDescriptGalEthernetPM[];
int pmmgrPmInitForGalEthernetPM(pmmgrPmMe_t *me_p);
int getAllDataFromApiForGalEthernetPM(char *data, uint16 portId);

#endif
