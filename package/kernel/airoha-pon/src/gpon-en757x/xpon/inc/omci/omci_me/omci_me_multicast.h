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
	omci_me_multicast.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wayne.lee	2012/7/28	Create
*/

#ifndef _OMCI_MULTICAST_ME_H_
#define _OMCI_MULTICAST_ME_H_
#include <linux/types.h>
#include "omci_types.h"
#include "omci_me.h"
#ifdef TCSUPPORT_XPON_IGMP
#include "../../../lib/xpon_igmp/xpon_igmp_omci.h"
#endif

#define OMCI_IPV4_ACTIVE_MLTICAST_GROUP_RECORD_SIZE 	24
#define OMCI_MAX_IPV4_PREFIX 								12
#define OMCI_ACTIVE_MLTICAST_GROUP_RECORD_SIZE 			60
#define OMCI_MAX_ACTIVE_MLTICAST_GROUP_NUM				20


#define OMCI_MOP_ACL_TABLE_ENTRY_CLEAR_TEST_BIT		(1<<10)
#define OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART0 				0
#define OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART1 			1
#define OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART2 			2
#define OMCI_MOP_ACL_TABLE_ENTRY_MAX_ROW_PART 		3
#define OMCI_MOP_ACL_TABLE_ENTRY_LENGTH 				24
#define OMCI_MOP_ACL_TABLE_ENTRY_SET_CTRL_FILED    		14
#define OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART_FILED     		11
#define OMCI_MOP_ACL_TABLE_ENTRY_SET_CTRL_MASK			0xc000
#define OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART_MASK		0x3800
#define OMCI_MOP_ACL_TABLE_ENTRY_ROW_ID_988_AMD1_MASK		0x3ff
#define OMCI_MOP_ACL_TABLE_ENTRY_ROW_ID_988_MASK			0x3fff



#define OMCI_MULTICAST_ONE_PORT_FLAG						1
#define OMCI_MULTICAST_ALL_PORT_FLAG						2

#define OMCI_MULTICAST_VER_TYPE							(1<<0)
#define OMCI_MULTICAST_FUNC_TYPE							(1<<1)
#define OMCI_MULTICAST_FAST_LEAVE_TYPE					(1<<2)
#define OMCI_MULTICAST_UP_TCI_TYPE						(1<<3)
#define OMCI_MULTICAST_TAG_CTRL_TYPE						(1<<4)
#define OMCI_MULTICAST_MAX_RATE_TYPE						(1<<5)
#define OMCI_MULTICAST_DYNAMIC_ACL_TYPE					(1<<6)
#define OMCI_MULTICAST_STATIC_ACL_TYPE					(1<<7)
#define OMCI_MULTICAST_ROBUSTNESS_TYPE					(1<<9)
#define OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE			(1<<10)
#define OMCI_MULTICAST_QUERY_INTERVAL_TYPE				(1<<11)
#define OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE	(1<<12)
#define OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE			(1<<13)
#define OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE			(1<<14)
#define OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE			(1<<15)

#define OMCI_MULTICAST_VER_ATTR_INDEX							1
#define OMCI_MULTICAST_FUNC_ATTR_INDEX							2
#define OMCI_MULTICAST_FAST_LEAVE_ATTR_INDEX					3
#define OMCI_MULTICAST_UP_TCI_ATTR_INDEX							4
#define OMCI_MULTICAST_TAG_CTRL_ATTR_INDEX						5
#define OMCI_MULTICAST_UP_RATE_ATTR_INDEX						6
#define OMCI_MULTICAST_DYNAMIC_ACL_ATTR_INDEX					7
#define OMCI_MULTICAST_STATIC_ACL_ATTR_INDEX					8
#define OMCI_MULTICAST_LOST_GROUP_ATTR_INDEX					9
#define OMCI_MULTICAST_ROBUSTNESS_ATTR_INDEX					10
#define OMCI_MULTICAST_QUERIER_IP_ADDRESS_ATTR_INDEX			11
#define OMCI_MULTICAST_QUERY_INTERVAL_ATTR_INDEX				12
#define OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_ATTR_INDEX	13
#define OMCI_MULTICAST_LAST_QUERY_INTERVAL_ATTR_INDEX			14
#define OMCI_MULTICAST_UNAUTHORIZED_JOIN_ATTR_INDEX			15
#define OMCI_MULTICAST_DOWN_IGMP_AND_TCI_ATTR_INDEX			16

/*IGMP version*/
#define OMCI_MULTICAST_VERSION_V2				2 //igmp v2
#define OMCI_MULTICAST_VERSION_V3				3 //igmp v3
#define OMCI_MULTICAST_VERSION_MLD_V1			16 //MLD v1
#define OMCI_MULTICAST_VERSION_MLD_V2			17 //MLD V2

/*IGMP function*/
#define OMCI_MULTICAST_FUNCTION_SNOOPING				0
#define OMCI_MULTICAST_FUNCTION_SPR						1
#define OMCI_MULTICAST_FUNCTION_PROXY					2

/*IGMP fast leave*/
#define OMCI_MULTICAST_FAST_LEAVE_DISABLE				0
#define OMCI_MULTICAST_FAST_LEAVE_ENABLE				1

/*IGMP upstream tag ctrl*/
#define OMCI_MULTICAST_TAG_CTRL_TRANSPARENT						0 //transparent
#define OMCI_MULTICAST_TAG_CTRL_ADD_TCI							1 //add vlan tag
#define OMCI_MULTICAST_TAG_CTRL_REPLACE_TCI						2 //replace tci
#define OMCI_MULTICAST_TAG_CTRL_REPLACE_VID						3 //replace vid

/*IGMP robustness*/
#define OMCI_MULTICAST_DEFAULT_ROBUSTNESS				0

/*IGMP query ip*/
#define OMCI_MULTICAST_DEFAULT_QUERY_IP					{0,0,0,0}

/*IGMP query interval*/
#define OMCI_MULTICAST_DEFAULT_QUERY_INTERVAL			125

/*IGMP max response time*/
#define OMCI_MULTICAST_DEFAULT_QUERY_MAX_RESPONSE 		100

/*IGMP Last member query interval*/
#define OMCI_MULTICAST_DEFAULT_LAST_QUERY_INTERVAL		10

/*IGMP Unauthorized join request*/
#define OMCI_MULTICAST_UNAUTHORIZED_JOIN_DISABLE		0
#define OMCI_MULTICAST_UNAUTHORIZED_JOIN_ENABLE		1

/*IGMP Downstream IGMP and TCI*/
#define OMCI_MULTICAST_DOWN_TAG_CTRL_TRANSPARENT				0 //transparent
#define OMCI_MULTICAST_DOWN_TAG_CTRL_STRIP						1 //strip
#define OMCI_MULTICAST_DOWN_TAG_CTRL_ADD_TCI					2 //add tci
#define OMCI_MULTICAST_DOWN_TAG_CTRL_REPLACE_TCI				3 //replace tci
#define OMCI_MULTICAST_DOWN_TAG_CTRL_REPLACE_VID				4 //replace vid
#define OMCI_MULTICAST_DOWN_TAG_CTRL_ADD_TCI_USE_UNI			5 //add tci using VID(UNI)
#define OMCI_MULTICAST_DOWN_TAG_CTRL_REPLACE_TCI_USE_UNI		6 //replace tci using VID(UNI)
#define OMCI_MULTICAST_DOWN_TAG_CTRL_REPLACE_VID_USE_UNI		7 //replace vid using VID(UNI)

#define OMCI_MULTICAST_IGMP_RULE_TYPE_DYN						1
#define OMCI_MULTICAST_IGMP_RULE_TYPE_STATIC					2

#define OMCI_DEFAULT_MULTICAST_FUNC_TYPE				0 // type0: transparent igmp snooping only.
#define OMCI_DEFAULT_MULTICAST_FAST_LEAVE_FLAG			0 

#define OMCI_DEFAULT_MULTICAST_TAG_CTRL					0 

#define  OMCI_IGMP_ACL_TABLE_ENTRY_WRITE_ACTION 		1
#define  OMCI_IGMP_ACL_TABLE_ENTRY_DEL_ACTION 			2
#define  OMCI_IGMP_ACL_TABLE_ENTRY_CLEAR_ACTION			3

#define OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_0			0 //9.3.28 me type attribute, pointer to 9.3.4 mac bridge port config data ME
#define OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_1			1 //9.3.28 me type attribute, pointer to 9.3.10 802.1p ME

#define OMCI_FIND_MULTICAST_BY_MAC_BRIDGE_PORT			1
#define OMCI_NO_FIND_MULTICAST_BY_MAC_BRIDGE_PORT		0
#define OMCI_MULTICAST_IP_LEN								16
#define OMCI_MULTICAST_IPV4_OFFSET						12
#define OMCI_MULTICAST_IPV4_LEN							4

#define OMCI_MULTICAST_DEFAULT_VER						16 //MLD V1
#define OMCI_MULTICAST_DEFAULT_MODE						0 //snooping
#define OMCI_MULTICAST_DEFAULT_FAST_LEAVE				0 //disable fast leave function
#define OMCI_MULTICAST_DEFAULT_UP_TCI					0 //upstream tci :0
#define OMCI_MULTICAST_DEFAULT_UP_TAG_CTRL				0 //upstream tag control:pass

#define OMCI_MULTICAST_MBPCD_TP_TYPE_VEIP		11

/*igmp rule information structure*/
typedef struct omciIgmpRuleInfo_s{
	uint16	rowKey; /*The row key distinguished rows in the tables*/
	uint16	ruleType; /*(1<<0):dynamic group record, (1<<1):static group record*/
	uint16	gemPortId; /*GEM port-ID*/
/*This field specifies the VLAN carrying the multicast group downstream. 
The VLAN ID resides in the 12 least significant bits; the remaining bits ar e set to 0 and not used.
The value 0 designates an untagged downstream flow.*/
	uint16	vid; 
	uint8 srcIp[OMCI_MULTICAST_IP_LEN]; /*Source IP address.The value 0.0.0.0 specifies that source IP address is to be ignored.*/
	uint8 startDestIp[OMCI_MULTICAST_IP_LEN]; /*Destination IP address of the start of the multicast range.*/
	uint8 endDestIp[OMCI_MULTICAST_IP_LEN]; /*Destination IP address of the end of the multicast range. */
/* Imputed group bandwidth. Expressed in bytes per second,
the imputed group bandwidth is used to  decide whether or not to honour a join request in the presence of a max multicast bandwidth limit. 
The recommended default value 0 effectively allows this table entry to avoid max bandwidth limitations*/	
	uint32	imputedBandwidth; /**/
}omciIgmpRuleInfo_t, *omciIgmpRuleInfo_ptr;

typedef struct igmpRuleInst_s{
	struct igmpRuleInst_s * next;
	omciIgmpRuleInfo_ptr value;
}igmpRuleInst_t, *igmpRuleInst_Ptr;


typedef struct omciIgmpActiveGroupInfo_s{
	uint8 srcIp[16];
	uint8 multiDestIp[16];
	uint32 bestBandWidth;
	uint8 clientIp[16];
	uint32 time;
	uint16 vid;
	uint16 reserved;
}omciIgmpActiveGroupInfo_t, *omciIgmpActiveGroupInfo_ptr;

/*******************************************************************************************************************************
general function

********************************************************************************************************************************/
int getLanPortByMeType(IN uint16 instanceId, IN uint8 meType, OUT uint8 *portFlag, OUT uint16 *portId);
int setMOPMulticastByPort(uint16 instId, uint16 port, uint16 type);
int setRealMulticastRuleByMeType(uint16 instanceId, uint8 meType, uint16 clearType);
int clearRealMulticastRuleByMeType(uint16 instanceId, uint8 meType, uint16 clearType);
int clearRealMulticastRuleByProfileType(IN uint16 currInstId, IN uint16 type);
int setRealMulticastRuleByProfile(IN uint16 currInstId, IN uint16 type, IN uint8 aclRuleAction, IN char *val_ptr, IN omciIgmpRuleInfo_t *ruleRecord);
int findMulticastInstByMacBrPortInst(uint16 macBrPortinstId);
int getMulticastPortList(IN uint16 currInstId, OUT uint16 *portList, OUT uint8 *validPortNum);


/*******************************************************************************************************************************
start
9.3.27 multicast operations profile

********************************************************************************************************************************/
int omciMeInitFor_MulticastProfile(omciManageEntity_t *omciManageEntity_p);
int omciMulticastProfileDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int setMOPMulticastAttrValue(uint16 instId, uint16 type, char *val_ptr);

int setMOPMulticastVersionValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastFuncValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastFastLeaveValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastUpstreamTCIValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastTagCtrlValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastUpstreamRateValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int getMOPMulticastAclTableValueByType(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, IN uint16 attrIndex);
int setMOPMulticastAclTableValueByType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, IN uint16 attrIndex);
int getMOPMulticastDynamicAclTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastDynamicAclTableValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int getMOPMulticastStaticAclTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastStaticAclTableValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastRobustnessValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastQuerierIPAddressValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastQueryIntervalValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastQueryMaxResponseTimeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastLastMemberQueryIntervalValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastUnauthorizedJoinRequestValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMOPMulticastDownstreamIGMPAndTCIValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
start
9.3.28 multicast subscriber config info

********************************************************************************************************************************/
int omciMeInitForMulticasSubscribertCfgInfo(omciManageEntity_t *omciManageEntity_p);
int omciMulticasSubscribertCfgInfoDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);

int setMSCmeTypeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMSCOperationProfileValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMSCMaxSimultaneousGroupValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMSCMaxBandwidthValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMSCBWEnforcementValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
start
9.3.29 multicast subscriber monitor

********************************************************************************************************************************/
int omciMeInitForMulticastSubscriberMonitor(omciManageEntity_t *omciManageEntity_p);
int32 getMSMIpv4ActiveGroupTblValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);

/*extern */
extern omciAttriDescript_t omciAttriDescriptListMulticastProfile[];
extern omciAttriDescript_t omciAttriDescriptListMulticasSubscribertCfgInfo[];
extern omciAttriDescript_t omciAttriDescriptListMulticastSubscriberMonitor[];



#endif
