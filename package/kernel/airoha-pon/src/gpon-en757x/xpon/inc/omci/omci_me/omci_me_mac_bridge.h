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
	omciMacBridgeME.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wayne.lee	2012/7/28	Create
*/

#ifndef _OMCI_MAC_BRIDGE_ME_H_
#define _OMCI_MAC_BRIDGE_ME_H_
#include "omci_types.h"
#include "omci_me.h"

#ifdef TCSUPPORT_PON_MAC_FILTER
#include "../../../lib/pon_mac_filter/lib_pon_mac_filter.h"
#endif


#ifndef TCSUPPORT_PON_MAC_FILTER
/*******************************************************************************************************************************
MAC FILTER API  function  Start

********************************************************************************************************************************/
typedef struct pon_mac_filter_rule_s
{
         uint8 filter_type;//0:forward 1:discard
         uint8 mac_type;//0:des mac 1:src mac
         uint16 ethertype;//0:don't care
         uint8 start_mac[6];//0:don'tcare
         uint8 end_mac[6];//0:don't care
 }pon_mac_filter_rule, * pon_mac_filter_rule_p;
/*******************************************************************************************************************************
MAC FILTER API  End 

********************************************************************************************************************************/
#endif

/*******************************************************************************************************************************
ANI port list

********************************************************************************************************************************/
#define OMCI_VALID_MAC_BRIDGE_PORT_FLAG		1
#define OMCI_INVALID_MAC_BRIDGE_PORT_FLAG	0

#define OMCI_ADD_MAC_BRIDGE_ANI_PORT		0
#define OMCI_DEL_MAC_BRIDGE_ANI_PORT		1
#define OMCI_MOD_MAC_BRIDGE_ANI_PORT		2 //modify
#define OMCI_RECFG_MAC_BRIDGE_ANI_PORT		4 //uplink to link (modify interfane match rule with gemport )

/*9.3.1*/
#define MAX_OMCI_LAN_PORT_NUM			4
#define MAX_OMCI_LAN_PORT_MASK			0x0f
#define OMCI_LAN_PORT1_OTHER_MASK		0xf0
#define MAX_OMCI_LAN_PORT_DEFAULT_MASK	0xff

#define OMCI_MAC_BRIDGE_LAN_PORT1		0 
#define OMCI_MAC_BRIDGE_LAN_PORT2		1 
#define OMCI_MAC_BRIDGE_LAN_PORT3		2 
#define OMCI_MAC_BRIDGE_LAN_PORT4		3 
#define OMCI_MAC_BRIDGE_ENABLE_DISCARD	0
#define OMCI_MAC_BRIDGE_DISABLE_DISCARD	1



/*9.3.4*/
#define OMCI_MBP_UNI_ISOLATE_DEFAULT_VAL	0xffffffff

#define OMCI_MAC_BRIDGE_PORT_TP_TYPE_1	1 //physical path termination point ethernet uni
#define OMCI_MAC_BRIDGE_PORT_TP_TYPE_3	3 //802.1p mapper service profile
#define OMCI_MAC_BRIDGE_PORT_TP_TYPE_4	4 //IP host config data
#define OMCI_MAC_BRIDGE_PORT_TP_TYPE_5	5 //GEM interworking termination point
#define OMCI_MAC_BRIDGE_PORT_TP_TYPE_6	6 //multicast gem interworking termination point
#define OMCI_MAC_BRIDGE_PORT_TP_TYPE_11	11 //virtual ethernet interface point

#define OMCI_MAC_BRIDGE_PORT_TP_TYPE_ATTR 		1 //9.3.4 TP type attribute
#define OMCI_MAC_BRIDGE_PORT_TP_POINTE_ATTR 	2 //9.3.4 TP pointer attribute

#define OMCI_MAC_BRIDGE_PORT_CFG_DATA_ATTR_11	11	//outbound TD pointer attribute 	
#define OMCI_MAC_BRIDGE_PORT_CFG_DATA_ATTR_12	12	//inbound TD pointer attribute

#define OMCI_MAC_BRIDGE_PORT_LAN_IFC		0 //lan interface
#define OMCI_MAC_BRIDGE_PORT_WAN_IFC	1 //wan interface

#define OMCI_MAC_BRIDGE_PORT_MULTICAST_TP	1
#define OMCI_MAC_BRIDGE_PORT_GEM_TP			2
#define OMCI_MAC_BRIDGE_PORT_DOT1P			3
#define OMCI_MAC_BRIDGE_PORT_PPTP_UNI		4
#ifdef TCSUPPORT_PON_IP_HOST
#define OMCI_MAC_BRIDGE_PORT_IP_CONFIG_HOST		5
#endif
#define MAX_GEM_PORT_NUM					256 //max gem port number(1~256)
#define MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM	8 //max gem port in a mac bridge port

#ifdef TCSUPPORT_PON_IP_HOST
#define IP_HOST_CONFIG_PORT_BASIC 10
#endif

/*constant define*/
/*spanning tree protocol*/
#if 1//def CZ_OLT
#define MIN_STP_MAX_AGE			6 /*6 seconds*/
#define MAX_STP_MAX_AGE			40 /*40 seconds*/
#define MIN_STP_HELLO_TIME 		1 /*1 seconds*/
#define MAX_STP_HELLO_TIME	 	10 /*10 seconds*/
#define MIN_STP_FORWARD_DELAY 	4 /*4 seconds*/
#define MAX_STP_FORWARD_DELAY 	30 /*30 seconds*/
#else
#define MIN_STP_MAX_AGE	0x0600 /*6 seconds*/
#define MAX_STP_MAX_AGE	0x2800 /*40 seconds*/
#define MIN_STP_HELLO_TIME 	0x0100 /*1 seconds*/
#define MAX_STP_HELLO_TIME	 	0x0a00 /*10 seconds*/
#define MIN_STP_FORWARD_DELAY 	0x0400 /*4 seconds*/
#define MAX_STP_FORWARD_DELAY 	0x1e00 /*30 seconds*/
#endif
#define MIN_DYNAMIC_FILTER_AGEING_TIME 	10 /*10 seconds*/
#define MAX_DYNAMIC_FILTER_AGEING_TIME 	1000000 /*1000000 seconds*/

/*performance monitoring history data*/
#define MAX_INTERVAL_END_TIME	255 /*interval end time */

/*mac bridge port information , store releation PON interface with gem port list*/
#define OMCI_VALID_MB_PORT_GEM_PORT		1
#define OMCI_INVALID_MB_PORT_GEM_PORT	0

#define OMCI_UPDATE_UNI_MAPPING_BY_VLAN_FILTER_ME	1
#define OMCI_UPDATE_UNI_MAPPING_BY_DOT1P_ME			2
#define OMCI_UPDATE_UNI_MAPPING_BY_USERPORT_ME		3

typedef struct mb_port_info_s{
	struct mb_port_info_s *next;
	uint16 instId;//9.3.4 instance id
	uint8 type;//0:LAN, 1:WAN
	uint8 reserve;
	uint16 gemlist[MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM][2]; //element 0:valid/invalid , element 1:gem port id	
}mb_port_info_t, *mb_port_info_ptr;


/*9.3.5*/
#define OMCI_MAC_BRIDGE_PORT_STATE_0		0 //disable
#define OMCI_MAC_BRIDGE_PORT_STATE_6		6 //STP_OFF

/*9.3.6 and 9.3.7*/
#define OMCI_MAC_FILTER_TYPE				0 //9.3.6
#define OMCI_PKT_FILTER_TYPE				1 //9.3.7

#define MAX_OMCI_MAC_BRIDGE_FILTER_SIMULTANEOUS_RULE_NUM	4 //max rule number in a attribute

#define OMCI_MAC_BRIDGE_FILTER_LAN_PORT_BASIC			11
#define OMCI_MAC_BRIDGE_FILTER_WAN_PORT_BASIC			51

#define  OMCI_FIND_RULE						1
#define  OMCI_NO_FIND_RULE					0

#define OMCI_MAC_BRIDGE_PORT_MAC_FILTER_TBL_TYPE_DEFAULT	2
#define OMCI_MAC_BRIDGE_PORT_MAC_FILTER_TBL_TYPE_FILTER	1 //blacklist
#define OMCI_MAC_BRIDGE_PORT_MAC_FILTER_TBL_TYPE_FORWARD	0 //white list

#define OMCI_MAC_BRIDGE_PORT_MAC_FILTER_DEL_ENTRY		0 //remove this entry
#define OMCI_MAC_BRIDGE_PORT_MAC_FILTER_CLEAR_ENTRY		1 //clear entry table
#define OMCI_MAC_BRIDGE_PORT_MAC_FILTER_ADD_ENTRY		2 //add this entry


#define OMCI_MAC_LENGTH 6

#define MAC_FILTER_RECORD_LENGTH 8
/*  	filterType: 
	Bit 	Name			Setting
	1	filter/forward		0:forward,1:filter
	2					0:dest mac, 1:src mac
	3~6	reserved
	7~8	add/remove		00:remove this entry, 01:addthis entry, 10:clear entire table
	*/	
#define MAC_FILTER_TYPE_80						0x80 // only consider add, dest bits, don't consider bit 0
#define MAC_FILTER_TYPE_80_VALID_BITS			0xc2


#define OMCI_MAC_FILTER_ETHERTYPE_ENABLED 	1
#define OMCI_MAC_FILTER_ETHERTYPE_DISABLED 	0

#define MAC_FILTER_TABLE_ENTRY		0 //9.3.6
#define MAC_FILTER_IPV4_MULTICAST		1
#define MAC_FILTER_IPV6_MULTICAST		2
#define MAC_FILTER_IPV4_BROADCAST		3
#define MAC_FILTER_RARP				4
#define MAC_FILTER_IPX					5
#define MAC_FILTER_NETBEUL				6
#define MAC_FILTER_AARP				7
#define MAC_FILTER_MANAGE_INFO		8
#define MAC_FILTER_ARP					9
#define MAC_FILTER_PPPOE_BROADCAST	10

#define OMCI_MAC_FILTER_RULE_TYPE_FORWARD			0
#define OMCI_MAC_FILTER_RULE_TYPE_DISCARD			1
#define OMCI_MAC_FILTER_RULE_MAC_TYPE_DST			0 //dest mac
#define OMCI_MAC_FILTER_RULE_MAC_TYPE_SRC			1 //src mac

#define OMCI_MAC_FILTER_RULE_ETHERTYPE_0				0 //don't care ethertype
#define OMCI_MAC_FILTER_RULE_ETHERTYPE_0800			0x0800 //ipv4 broadcast 
#define OMCI_MAC_FILTER_RULE_ETHERTYPE_8035			0x8035 //RARP
#define OMCI_MAC_FILTER_RULE_ETHERTYPE_8137			0x8137 //IPX
#define OMCI_MAC_FILTER_RULE_ETHERTYPE_809b			0x809b //appletalk
#define OMCI_MAC_FILTER_RULE_ETHERTYPE_80f3			0x80f3 //appletalk
#define OMCI_MAC_FILTER_RULE_ETHERTYPE_0806			0x0806 //ARP
#define OMCI_MAC_FILTER_RULE_ETHERTYPE_8863			0x8863 //PPPoE broadcast

 #define MAX_OMCI_MAC_FILTER_RULE_ONE_PORT			32 //max mac filter rule number in a port
 #define MAX_OMCI_MAC_FILTER_9_3_7_RULE_ONE_PORT	15 //max mac filter rule number of port in 9.3.7
 #define MAX_OMCI_MAC_FILTER_9_3_6_RULE_ONE_PORT	17 //max mac filter rule number of port in 9.3.6

/*9.3.8*/
 
#define ETH_PORT_STS    "/proc/tc3162/eth_port_status"

#define MAX_MAC_BRIDGE_RECORD_BUF 40
#define MAX_OMCI_IFC_NAME_LEN				16
#define  OMCI_MAC_FILTER_FLAG_0			0 //forward
#define  OMCI_MAC_FILTER_FLAG_1			1 //filter



/*******************************************************************************************************************************
general function

********************************************************************************************************************************/
void getOmciEtherAddr(unsigned char *mac_addr);
int getMacBridgePortCnt(uint16 instanceId, uint8 * portCnt);
int getMacBridgePortInstType(uint16 instId,  uint8 * ifcType);
int getMacBridgeLanPortId(uint16 instId, uint8 *lanPortId);
int getMacBridgeWanPortId(uint16 instId, uint8 *wanPortId);
int getMacBridgePortIfcByInst(IN uint16 instanceId, OUT char * ifcName);
int getPortFromMacBridgeByInstId(OUT uint8 *portId, OUT uint8 * ifaceType, IN uint16 instId);
int getPortFromMacBridgeSkipUNIFilterRuleByInstId(OUT uint8 * skip_filter_flag, IN uint16 instId);
int setMacBridgePortInfoToRomfile(IN uint16 instanceId);
//int getPortInfoByTraficDesInst(OUT uint8 *portId, OUT uint8 * ifaceType, OUT uint8 type, IN uint16 instId);
int getGemPortByMACBridgePortInst(IN uint16 instanceId, OUT uint16 * gemPort, OUT uint8 * validGemPortNum);
int updateMACBridgePortInfoByAssociaAttr(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 attrType);
int addGemPortRuleByMbPortUNI(uint16 instId);
int updateGemMappingRuleByMbPortUNI(uint8 mapType, uint16 instId);
int findMacBridgeFirstStr(char * rowBuf, uint32 * fromLen, char * firstStr);
int getMacFormMacStr(char delimiter, char * macStr, uint8 * mac);
int getSearchFilterMacFormFilterTbl(IN uint16 instanceId,IN  uint8 *mac, OUT uint8 *find, OUT uint8 * filterFlag);

int getMacFilterRulePortIdByInst(uint16 instId, uint8 * macFilterRulePortId);
int setMacFilterRuleByType(IN uint8 type,  IN uint8 * macFilterEntry_ptr,  IN  uint8 macFilterRulePortId);
int addMacFilterRuleByInst(IN uint16 classId, IN uint16 instId, IN uint8 macFilterRulePortId);
int findMacFilterRuleByRuleInfo(IN pon_mac_filter_rule_p macFilterRule_ptr, IN uint8 macFilterRulePortId, IN uint8 macFilterType);

mb_port_info_ptr findMBPortInfo(uint16 instId);
int addMBPortInfo(IN mb_port_info_ptr  info_ptr);
int delMBPortInfo(IN uint16 instId);
int clearMBPortInfo(void);
void displayMBPortInfo();
int clearOmciMBInfo(void);

/*******************************************************************************************************************************
ANI port list

********************************************************************************************************************************/
int getFreeOmciAniPortId(OUT uint16 * omcAniPortId);
int findOmciAniPortId(uint16 instId, uint8 *findFlag, uint16 * omcAniPortId);
int setMacBridgePortDeviceId(IN uint16 instId, IN uint8 type);
int updateAniInfoBy8021p(uint16 instId);
int omciRecfgIfc(void);

/*******************************************************************************************************************************
start
9.3.1 MAC bridge Service Profile ME

********************************************************************************************************************************/
int omciMeInitForMACBridgeServiceProfile(omciManageEntity_t *omciManageEntity_p);
//int8 getMBSP_spanningTreeIndValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBSPSpanningTreeIndValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBSPLearningIndValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMBSPPortBridingIndValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBSPPortBridingIndValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBSPPriorityValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBSPMaxAgeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBSPHelloTimeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBSPForwardDelayValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMBSPUnKnownMacAddrDiscardValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBSPUnKnownMacAddrDiscardValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBSPMacAddrLimitValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
start
9.3.2 MAC bridge configuration data ME

********************************************************************************************************************************/
int omciMeInitForMACBridgeConData(omciManageEntity_t *omciManageEntity_p);
//int8 getMBCD_managedEntityIdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMBCDBridgeMACAddrValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMBCDBridgePriorityValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMBCDDesignatedRootValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMBCDRootPathCostValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMBCDBridgePortCountValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMBCDRootPortNumValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
start
9.3.3 MAC bridge performance monitoring history data

********************************************************************************************************************************/
int omciMeInitForMACBridgePMHD(omciManageEntity_t *omciManageEntity_p);
int32 setMBPMHDThresholdDataIdValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
start
9.3.4 MAC bridge port configuration data

********************************************************************************************************************************/
int omciMeInitForMACBridgePortConData(omciManageEntity_t *omciManageEntity_p);
int omciMACBridgePortConDataDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t *  omciPayLoad, uint8 msgSrc);
int setMBPCDTPTypeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPCDTPPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMBPCDportMacAddressValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPCDoutTDPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPCDinTDPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPCDMacAddrLimitValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);


/*******************************************************************************************************************************
start
9.3.5 MAC bridge port designation data ME

********************************************************************************************************************************/
int omciMeInitForMACBridgePortDesignData(omciManageEntity_t *omciManageEntity_p);
int omciInternalCreateActionForMACBridge(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int32 getMBPDDPortStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
/*******************************************************************************************************************************
start
9.3.6 mac bridge port filter table table

********************************************************************************************************************************/
int omciMeInitForMACBridgePortFilterData(omciManageEntity_t *omciManageEntity_p);
int omciMACBridgePortFilterDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int32 getMBPFTmacFilterTableValue(char *value, uint8 *meInstantPtr,  omciAttriDescript_ptr omciAttribute, uint8 flag);
int setMBPFTmacFilterTableValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
start
9.3.7 mac bridge port filter preassign table

********************************************************************************************************************************/
int omciMeInitForMACBridgePortFilterPreData(omciManageEntity_t *omciManageEntity_p);
int omciMACBridgePortFilterPreDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int setMBPFPTFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 type);
int setMBPFPTipv4MulticastFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPFPTipv6MulticastFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPFPTipv4BroadcastFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPFPTRARPFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPFPTIPXFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPFPTNetBEULFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPFPTappleTalkFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPFPTmanageInfoFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPFPTARPFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMBPFPTPPPoEBroadcastFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
start
9.3.8 mac bridge port bridge table data

********************************************************************************************************************************/
int omciMeInitForMACBridgePortData(omciManageEntity_t *omciManageEntity_p);
int32 getBPBTDBridgeTableValue(char *value, uint8 *meInstantPtr,  omciAttriDescript_ptr omciAttribute, uint8 flag);

/*******************************************************************************************************************************
start
9.3.9 mac bridge port performance monitoring history data

********************************************************************************************************************************/
int omciMeInitForMACBridgePortPMHD(omciManageEntity_t *omciManageEntity_p);
int setThresholdDataForMACBridgePort(IN uint16 currClassId, IN uint16 currDeviceId, IN uint16 thresholdInstId);
int32 setMBPPMHDThresholdDataValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
start
9.3.30 ethernet frame performance monitoring history data upstream

********************************************************************************************************************************/
int omciMeInitForEthFrameUpPMHD(omciManageEntity_t *omciManageEntity_p);
int setThresholdDataForEthFrame(IN uint16 currClassId, IN uint16 currDeviceId, IN uint16 thresholdInstId);
int32 setEFPMHDThresholdDataValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
start
9.3.31 ethernet frame performance monitoring history data downstream

********************************************************************************************************************************/
int omciMeInitForEthFrameDownPMHD(omciManageEntity_t *omciManageEntity_p);


/*extern */
extern omciAttriDescript_t omciAttriDescriptListMBServiceProfile[];
extern omciAttriDescript_t omciAttriDescriptListMBConnData[];
extern omciAttriDescript_t omciAttriDescriptListMBPMHD[];
extern omciAttriDescript_t omciAttriDescriptListMBPortConData[];
extern omciAttriDescript_t omciAttriDescriptListMBPortDesignData[];
extern omciAttriDescript_t omciAttriDescriptListMBPortFilterData[];
extern omciAttriDescript_t omciAttriDescriptListMBPortFilterPreData[];
extern omciAttriDescript_t omciAttriDescriptListMBPortData[];
extern omciAttriDescript_t omciAttriDescriptListMBPortPMHD[];
extern omciAttriDescript_t omciAttriDescriptList_EthFrameUpPMHD[];
extern omciAttriDescript_t omciAttriDescriptListEthFrameDownPMHD[];

extern uint16 validAniPortFlag[MAX_OMCI_ANI_PORT_NUM][MAX_OMCI_ANI_PORT_VALID_NUM];

#endif

