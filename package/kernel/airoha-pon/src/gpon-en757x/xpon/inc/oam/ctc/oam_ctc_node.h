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
	oam_ctc_node.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#ifndef INCLUDE_OAM_NODE_H
#define INCLUDE_OAM_NODE_H
#include "../epon_oam_types.h"
//#include "oam_ctc_sleep.h"

/* used for perforemance get/set with same api port&PON IF
 * ETHER port id: 0~3;  PON IF id: 4 (PONIF id + 4)
 * TODO: need change with the system has.
 */
#define PORT_ETHER_NUM     4
#define PORT_VOIP_NUM      2
#define PONIF_NUM          1


/* OAM CTC V3 UNI Port type */
#define PORT_PON   0x00
#define PORT_ETHER 0x01
#define PORT_VOIP  0x02
#define PORT_ADSL2 0x03
#define PORT_VDSL2 0x04
#define PORT_E1    0x05
#define PORT_LLID  0xFD
#define PORT_OTHER 0xFE
#define PORT_ALL   0xFF

/* OAM CTC object type, other type will be skiped! */
#define OBJ_BIT_UNKOWN 0x00
#define OBJ_BIT_PORT  (1<<0)
#define OBJ_BIT_LLID  (1<<1)
#define OBJ_BIT_PONIF (1<<2)
#define OBJ_BIT_ONU   (1<<3)
#define OBJ_BIT_ALL   0x0F

#define OBJ_LF_UNKOWN 0x0000
#define OBJ_LF_PORT   0x0001
#define OBJ_LF_LLID   0x0003
#define OBJ_LF_PONIF  0x0004
#define OBJ_LF_ONU    0xFFFF


#define OAM_OBJECT_BRANCH_V1    	0x36
#define OAM_OBJECT_LEAF_WIDTH_V1	0x01
#define OAM_OBJECT_LENGTH_V1		5

#define OBJ_LF_PON_START_V1 		0x00
#define OBJ_LF_ETH_START_V1 		0x01
#define OBJ_LF_VOIP_START_V1 		0x50
#define OBJ_LF_E1_START_V1 			0x90
#define OBJ_LF_OTHER_START_V1		0xA0
#define OBJ_LF_ALL_START_V1			0xFF

#define OAM_OBJECT_BRANCH_V2    	0x37
#define OAM_OBJECT_LEAF_WIDTH_V2	0x04
#define OAM_OBJECT_LENGTH_V2		8

#define OBJ_LF_PON_START_V2 		0x00
#define OBJ_LF_ETH_START_V2 		0x01
#define OBJ_LF_VOIP_START_V2 		0x01  //TODO: need to test epon system
#define OBJ_LF_E1_START_V2 		0x01  //TODO: need to test epon system
#define OBJ_LF_OTHER_START_V2		0xA0
#define OBJ_LF_ALL_START_V2			0xFF


#define OAM_TLV_MAX_VALUE_LENGTH	128
#define OAM_MAX_PARAM_NAME_LEN		64

#define GET_PORT_ALL  0xFFFFFFFF
#define GET_PORT_ALL_V1 0xFF
#define GET_PORT_ALL_V2 0xFFFF

/*	The Attribute/Action's branch value	*/
#define OAM_END_TLV	0x00
#define OAM_STD_ATTR  	0x07
#define OAM_STD_ACT		0x09
#define OAM_EXT_ATTR  	0xC7
#define OAM_EXT_ACT		0xC9

/*	here to define the leaf of OAM Parameter Nodes	*/
/*	The Attribute/Action's leaf value		*/
/* OAM Standard attributes 0x07 */
#define OAM_LF_PHYADMINSTATE              	0x0025
#define OAM_LF_AUTONEGADMINSTATE          	0x004F
#define OAM_LF_AUTONEGLOCALTECHABILITY    	0x0052
#define OAM_LF_AUTONEGADVTECHABILITY      	0x0053
#define OAM_LF_FECABILITY                 	0x0139
#define OAM_LF_FECMODE                    	0x013A

/* OAM External attributes 0xC7 */
#define OAM_LF_ONU_SN                     	0x0001
#define OAM_LF_FIRMWAREVER                	0x0002
#define OAM_LF_CHIPSET_ID                 	0x0003
#define OAM_LF_ONU_CAPABILITIES_1         	0x0004
#define OAM_LF_OPTICALTRANSCEIVERDIAGNOSIS	0x0005
#define OAM_LF_SERVICE_SLA                	0x0006
#define OAM_LF_ONU_CAPABILITIES_2         	0x0007
#define OAM_LF_HOLDOVERCONFIG             	0x0008
#define OAM_LF_MXUMNGGLOBALPARAMETER      	0x0009
#define OAM_LF_MXUMNGSNMPPARAMETER        	0x000A
#define OAM_LF_ACTIVE_PON_IFADMINSTATE    	0x000B
#define OAM_LF_ONU_CAPABILITIES_3         	0x000C
#define OAM_LF_POWERSAVINGCAPABILITIES    	0x000D
#define OAM_LF_POWERSAVINGCONFIG	    	0x000E
#define OAM_LF_PROTECTIONPARAMETERS	    	0x000F

#define OAM_LF_ETHLINKSTATE               	0x0011
#define OAM_LF_ETHPORT_PAUSE              	0x0012
#define OAM_LF_ETHPORTUS_POLICING         	0x0013
#define OAM_LF_VOIP_PORT                  	0x0014
#define OAM_LF_E1_PORT                    	0x0015
#define OAM_LF_ETHPORTDS_RATELIMITING     	0x0016
#define OAM_LF_PORTLOOPDETECT             	0x0017
#define OAM_LF_PORTDISABLELOOPED           	0x0018 // v3.0
#define OAM_LF_VLAN                       	0x0021
#define OAM_LF_CLASSIFICATION_MARKING     	0x0031
#define OAM_LF_ADD_DEL_MULTICAST_VLAN     	0x0041
#define OAM_LF_MULTICASTTAGOPER           	0x0042
#define OAM_LF_MULTICASTSWITCH            	0x0043
#define OAM_LF_MULTICASTCONTROL           	0x0044
#define OAM_LF_GROUP_NUM_MAX              	0x0045
#define OAM_LF_FASTLEAVEABILITY           	0x0046
#define OAM_LF_FASTLEAVEADMINSTATE        	0x0047
#define OAM_LF_LLIDQUEUECONFIG_V3            	0x0051
#define OAM_LF_LLIDQUEUECONFIG_V2            	0x0071

#define OAM_LF_IADINFO                    	0x0061
#define OAM_LF_GLOBALPARAMETERCONFIG      	0x0062
#define OAM_LF_H248_PARAMETERCONFIG       	0x0063
#define OAM_LF_H248_USERTIDINFO           	0x0064
#define OAM_LF_H248_RTP_TIDCONFIG         	0x0065
#define OAM_LF_H248_RTP_TIDINFO           	0x0066
#define OAM_LF_SIPPARAMETERCONFIG         	0x0067
#define OAM_LF_SIPUSERPARAMETERCONFIG     	0x0068
#define OAM_LF_FAX_MODEM_CONFIG           	0x0069
#define OAM_LF_H248IADOPERATIONSTATUS     	0x006A
#define OAM_LF_POTSSTATUS                 	0x006B
#define OAM_LF_ALARMADMINSTATE	           	0x0081 // only v2.1
#define OAM_LF_ALARMTHRESHOLD              	0x0082 // only v2.1
#define OAM_LF_ONUTXPOWERSUPPLYCONTROL    	0x00A1
#define OAM_LF_PORTMACAGINGTIME				0x00A4 // v3.0

#define OAM_LF_PFMCSTATUS					0x00B1 // v3.0
#define OAM_LF_PFMCDATACURR					0x00B2 // v3.0
#define OAM_LF_PFMCDATAHSTRY				0x00B3 // v3.0

/* OAM Standard Actions 0x09 */
#define OAM_LF_PHYADMINCONTROL            	0x0005
#define OAM_LF_AUTONEGRESTARTAUTOCONFIG   	0x000B
#define OAM_LF_AUTONEGADMINCONTROL        	0x000C

/* OAM External Actions 0xC9 */
#define OAM_LF_RESETONU                   	0x0001
#define OAM_LF_SLEEPCONTROL					0x0002 // v3.0
#define OAM_LF_FASTLEAVEADMINCONTROL      	0x0048
#define OAM_LF_MULTILLIDADMINCONTROL      	0x0202
#define OAM_LF_IADOPERATION               	0x006C
#define OAM_LF_SIPDIGITMAP                	0x006D

#ifdef TCSUPPORT_VOIP
#define OAM_LF_CUC_TIMER_CONFIG				0x007B
#endif

#ifdef TCSUPPORT_EPON_OAM_CUC
/* Support CUC version: C1 & C2 with OUI=0x111111 */
#define OAM_LF_CU_ONUMACAGINGTIME		0x000E
#define OAM_LF_CU_POWERSAVINGCONFIG	    	0x00A5
#define OAM_LF_CU_PROTECTIONPARAMETERS	0x00A6
#define OAM_LF_CU_ONUMACLIMIT				0x001C
#define OAM_LF_CU_ONUPORTMODECONFIG		0x001D
#define OAM_LF_CU_ONUPORTMODESTATUS		0x001E
#define OAM_LF_CU_CFGCNT					0x000F
#define OAM_LF_CU_RMSGLBPRMCFG1			0x00A3
#define OAM_LF_CU_RMSGLBPRMCFG2			0x00A4
#define OAM_LF_CU_CFGPRMMNGMT			0x0002
#define OAM_LF_CU_SLEEPCONTROL 			0x0003
#define OAM_LF_CU_PINGTEST				0x007c
#define OAM_LF_CU_PINGTEST_RESULT		0x007d
#endif


typedef struct oamObject_s{
	u_char   branch; // the object's branch
	u_short	leaf;	// the object's type
	u_int	index;	// the object's index
}OamObject_t, *OamObject_Ptr;


/*------------------------------------------------------------------------------------------*/
/* RetCode */
#define SET_OK           	0x80  // everything is ok
#define VAR_BAD_PARAMETERS	0x86  // the system not support this node or the object node doesn't exist.
#define VAR_NO_RESOURCE   	0x87  // the current system's setting is no resource to process this node 
#define RET_NOT_REPLY       0x00  // needn't to reply anything to olt 
//@TODO
#define RET_ONLY_SKIP       0x80  // the param need skip (if =0x80, reply set_ok, if =0x86, reply error_code)
	
/*	The most important OAM parameter node structure.
	Each Attribute or Action has one structure.
*/
#define DIRECT_COPY	2  // the result of getfunc is variable container
typedef struct oamParameterNode{
	char	name[OAM_MAX_PARAM_NAME_LEN];	// The Attribute/Action's name
	u_char	objectType;
	u_char	branch;		// The Attribute/Action's branch value:
	u_short	leaf;		// The Attribute/Action's leaf value
	u_char   length;		// The node's length, if the length is dynamic, the value is 0xFF.

	/* If get success, return SUCCESS; else return error code. */
	u_char	 (*oamGetFunc)(
				IN  u_char llidIdx,
				IN  OamObject_Ptr pObj, // the object which the node needs.
				OUT int	*pLength,		// The length of the value returned.
				OUT u_char  *pValue		// The value returned.
			);							// get func (GET Request).
	u_char	 (*oamSetFunc)(
				IN  u_char llidIdx,
				IN  OamObject_Ptr pObj,
				IN  int	length,
				IN  u_char  *pValue
			);					// set func(SET Request), if the node needn't to reply, return RET_NOT_REPLY
}OamParamNode_t, *OamParamNode_Ptr;

/*------------------------------------------------------------------------------------------*/

#pragma pack(push, 1)

typedef struct oamCtcBranchLeaf
{
	u_char  branch;
	u_short leaf;
}OamCtcBranchLeaf_t, *OamCtcBranchLeaf_Ptr;

/*	if the OAM parameter node's value contains more than one param to define the value's struct.
	Attribute of each node
*/
#define DISACTIVE	0x00
#define ACTIVE   	0x01

#define POWER_ON   	0x00
#define POWER_DOWN   	0x01



#define DISABLED 	0x01
#define ENABLED  	0x02

/*	OAM Attribute leaf = OAM_LF_AUTONEGLOCALTECHABILITY	*/

#define NEG_10BASE5         8

#define NEG_10BASE_T        14
#define NEG_10BASE_THD      141
#define NEG_10BASE_TFD      142

#define NEG_1000BASE_T      40
#define NEG_1000BASE_THD    401
#define NEG_1000BASE_TFD    402

#define NEG_100BASE_T2      32
#define NEG_100BASE_T2HD    321
#define NEG_100BASE_T2FD    322

typedef struct oamAutoNegLocalTechAbility_s
{
	u_int	enumNum;	//the number of AutoNegTechnology
	u_int	enumTable[];	//the first point
}OamAutoNegLocalTechAbility_t, *OamAutoNegLocalTechAbility_Ptr;

/*	OAM Attribute leaf = OAM_LF_AUTONEGLOCALTECHABILITY	*/
typedef OamAutoNegLocalTechAbility_t OamAutoNegAdvertisedTechAbility_t;
typedef OamAutoNegLocalTechAbility_Ptr OamAutoNegAdvertisedTechAbility_Ptr;

#define FEC_ABILITY_UNKNOWN    	0x00000001
#define FEC_ABILITY_SUPPORTED  	0x00000002
#define FEC_ABILITY_UNSUPPORTED	0x00000003

#define FEC_MODE_UNKNOWN       	0x00000001
#define FEC_MODE_ENABLED       	0x00000002
#define FEC_MODE_DISABLED      	0x00000003

/*	OAM Attribute leaf = OAM_LF_ONU_SN	*/
typedef struct oamOnuSN_s
{
	char	vendorID[4];
	char	onuModel[4];
	u_char	onuID[6];
	char	hwVersion[8];
	char	swVersion[16];
}OamOnuSN_t, *OamOnuSN_Ptr;

/*	OAM Attribute leaf = OAM_LF_CHIPSETID	*/
typedef struct oamChipsetID_s
{
	u_short	vendorID;
	u_short	chipModel;
	u_char	revision;
	u_char   versionDate[3];
}OamChipsetID_t, *OamChipsetID_Ptr;


/*	OAM Attribute leaf = OAM_LF_ONU_CAPABILITIES_1	*/
#define SUPPORT_ETH_GE	(1<<0)  // support GE
#define SUPPORT_ETH_FE	(1<<1)  // support FE highest
#define SUPPORT_VOIP	(1<<2)  // support VOIP
#define SUPPORT_TDMCES	(1<<3)  // support TDM CES

#define BATTERY_BACKUP_NO		0x00
#define BATTERY_BACKUP_YES		0x01

typedef struct oamOnuCapabilities_1_s
{
	u_char	serviceSupported;
	u_char	GEPortNum;
	u_char	GEPortBitMap[8];
	u_char	FEPortNum;
	u_char	FEPortBitMap[8];

	u_char	POTSPortNum;
	u_char	E1PortNum;

	u_char	USQueuesNum;
	u_char	queueMaxPerPortUS;
	u_char	DSQueuesNum;
	u_char	queueMaxPerPortDS;
	u_char	batteryBackup;
}OamOnuCapabilities_1_t, *OamOnuCapabilities_1_Ptr;


/*	OAM Attribute leaf = OAM_LF_ONU_CAPABILITIES_2	*/
typedef struct Interface_s
{
	u_int	interfaceType;
	u_short	portNum;
}Interface_t, *Interface_Ptr;

#define ONU_TYPE_SFU               	0x00000000
#define ONU_TYPE_HGU               	0x00000001
#define ONU_TYPE_SBU               	0x00000002
#define ONU_TYPE_MDU_BOX           	0x00000003
#define ONU_TYPE_MDU_SMALL_ETH     	0x00000004
#define ONU_TYPE_MDU_SMALL_DSL     	0x00000005
#define ONU_TYPE_MDU_MID_DSL       	0x00000006
#define ONU_TYPE_MDU_MIX_ETH_DSL   	0x00000007
#define ONU_TYPE_MTU               	0x00000008

#define LLID_SINGLE                	0x01
#define LLID_INVAILD               	0x00

#define PROTECTION_TYPE_NOSUPPORTED	0x00
#define PROTECTION_TYPE_C          	0x01
#define PROTECTION_TYPE_D          	0x02

#define IF_TYPE_GE                 	0x00000000
#define IF_TYPE_FE                 	0x00000001
#define IF_TYPE_VOIP               	0x00000002
#define IF_TYPE_TDM                	0x00000003
#define IF_TYPE_ADSL2              	0x00000004
#define IF_TYPE_VDSL2              	0x00000005
#define IF_TYPE_WLAN               	0x00000006
#define IF_TYPE_USB                	0x00000007
#define IF_TYPE_CATVRF             	0x00000008
#define IF_TYPE_MAX             	0x00000009

typedef struct oamOnuCapabilities_2
{
	u_int		onuType;
	u_char		multiLLD;
	u_char		protectionType;
	u_char		PONIFNum;
	u_char		slotNum;
	u_char		ifTypeNum;
	Interface_t	interfaces[];
//	u_char		batteryBackup;
}OamOnuCapabilities_2_t, *OamOnuCapabilities_2_Ptr;


/*	OAM Attribute leaf = OAM_LF_ONU_CAPABILITIES_3	*/
#define IPV6_NOSUPPORTED     	0x00
#define IPV6_SUPPORTED       	0x01

#define POWER_CTL_NOSUPPORTED	0x00
#define POWER_CTL_ONLY_TX    	0x01
#define POWER_CTL_TX_RX      	0x02

typedef struct oamOnuCapabilities_3
{
	u_char	IPv6Supported;
	u_char	OnuPowerSupplyCtrl;
#ifndef TCSUPPORT_EPON_OAM_CUC
	u_char   serviceSLA;	// 0x01 don't support SLA; 0x02-0x08 support SLA and = service num
#endif
}OamOnuCapabilities_3_t, *OamOnuCapabilities_3_Ptr;


/*	OAM Attribute leaf = OAM_LF_POWERSAVINGCAPABILITIES	*/
#define SLEEP_MODE_NONE      0x00
#define SLEEP_MODE_TX        0x01
#define SLEEP_MODE_TRX       0x02 // default value
#define SLEEP_MODE_TXTRX     0x03

#define EARLY_WAKE_SUPPORTED     0x00
#define EARLY_WAKE_NOT_SUPPORTED 0x01

typedef struct oamPowerSavingCapabilities_s
{
	u_char sleepMode;
	u_char supportEarlyWake;
}OamPowerSavingCapabilities_t, *OamPowerSavingCapabilities_Ptr;

/*	OAM Attribute leaf = OAM_LF_POWERSAVINGCONFIG	*/
typedef struct oamPowerSavingConfig_s
{
	u_char earlyWakeEnable; // if not support, is 0xFF; 0x00 = enable; 0x01 = disable
	u_char maxSleepDuration[6];
}OamPowerSavingConfig_t, *OamPowerSavingConfig_Ptr;

/*	OAM Attribute leaf = OAM_LF_POWERSAVINGCONFIG	*/
typedef struct oamProtectionParameters_s
{
	u_short TlosOptical; // default 2ms
	u_short TlosMAC;		// default 55ms
}OamProtectionParameters_t, *OamProtectionParameters_Ptr;

/*	OAM Attribute leaf = OAM_LF_OPTICALTRANSCEIVERDIAGNOSIS	*/
typedef struct oamOpticalTransceiverDiagnosis_s
{
	u_short	transceiverTemperature;
	u_short	supplyVoltage;
	u_short	txBiasCurrent;
	u_short	txPower;
	u_short	rxPower;
}OamOpticalTransceiverDiagnosis_t, *OamOpticalTransceiverDiagnosis_Ptr;



/*	OAM Attribute leaf = OAM_LF_SERVICE_SLA	*/
#define SLA_SP		0
#define SLA_WRR		1
#define SLA_SP_WRR	2
typedef struct oamSLAQueueService_s
{
	u_char	queueNum;			// possible values: 0-7
	u_short	fixedPktSize;		// if 	 0x0000, it is no limited.
	u_short	fixedBandwidth;		// if  0x0000, it is not a fixed num
	u_short	guaranteedBandwidth;// assured bandwidth in 256Kbps units
	u_short	bestEffortBandwidth;	// best effort bandwidth in 256Kbps units
	u_char	WRRWeight;			// possible values:0(SP),1-100(WRR)
}OamSLAQueueService_t, *OamSLAQueueService_Ptr;

typedef struct oamServiceSLA_s
{
	u_char	operateDBA;
	u_char	bestEffortSchedulingScheme;	// SP/WRR/SP+WRR
	u_char	highPriorityBoundary;
	u_int		cycleLength;
	u_char	servicesNum;	// possible values: 1-8
	OamSLAQueueService_t services[];
}OamServiceSLA_t, *OamServiceSLA_Ptr;


#define ETHLINKSTATE_DOWN    	0x00
#define ETHLINKSTATE_UP      	0x01

#define ETHPORTPAUSE_DISABLED	0x00
#define ETHPORTPAUSE_ENABLED 	0x01

/*	OAM Attribute leaf = OAM_LF_ETHPORTUS_POLICING	*/
typedef struct oamEthPortUSPolicing_s
{
	u_char	actived;// default value is disactiveated
	u_char	portCIR[3];
	u_char	portCBS[3];
	u_char	portEBS[3];
}OamEthPortUSPolicing_t, *OamEthPortUSPolicing_Ptr;


/*	OAM Attribute leaf = OAM_LF_ETHPORTDS_RATELIMITING	*/
typedef struct oamEthPortDSRateLimiting_s
{
	u_char	actived;
	u_char	portCIR[3];
	u_char	portPIR[3];
}OamEthPortDSRateLimiting_t, *OamEthPortDSRateLimiting_Ptr;


/*	OAM Attribute leaf = OAM_LF_VLAN	*/
#define MAX_VLAN_RULE_NUM_PER_PORT 16

typedef struct oamVlanTranlationEntry_s
{
	u_int	oldTag;
	u_int	newTag;
}OamVlanTranlationEntry_t, *OamVlanTranlationEntry_Ptr;

typedef struct oamVlanTranlation_s
{
	u_int	defaultTag;
	OamVlanTranlationEntry_t	entries[];
}OamVlanTranlation_t, *OamVlanTranlation_Ptr;

typedef struct oamVlanTrunk_s
{
	u_int	vlanTag;
	u_int	entries[];
}OamVlanTrunk_t, OamVlanTrunk_Ptr;


#define VLAN_MODE_TRANSPARENT  	0x00
#define VLAN_MODE_TAG          	0x01
#define VLAN_MODE_TRANSLATION  	0x02
#define VLAN_MODE_N1AGGREGATION	0x03
#define VLAN_MODE_TRUNK        	0x04
typedef struct oamVlan_s
{
	u_char	mode;
}OamVlan_t, OamVlan_Ptr;


#define RULE_FIELD_DST_MAC         	0x00
#define RULE_FIELD_SRC_MAC         	0x01
#define RULE_FIELD_ETH_PRIORITY    	0x02
#define RULE_FIELD_VLAN_ID         	0x03
#define RULE_FIELD_ETH_TYPE        	0x04
#define RULE_FIELD_DST_IPV4        	0x05
#define RULE_FIELD_SRC_IPV4        	0x06
#define RULE_FIELD_IP_PROTOCOL     	0x07
#define RULE_FIELD_IP_TOS          	0x08
#define RULE_FIELD_IP_TRAFFIC_CLASS	0x09
#define RULE_FIELD_SRC_L4          	0x0A
#define RULE_FIELD_DST_L4          	0x0B
#define RULE_FIELD_IPVERSION       	0x0C
#define RULE_FIELD_IP_FLOW_LABEL   	0x0D
#define RULE_FIELD_DST_IPV6        	0x0E
#define RULE_FIELD_SRC_IPV6        	0x0F
#define RULE_FIELD_DST_IPV6_PREFIX 	0x10
#define RULE_FIELD_SRC_IPV6_PREFIX 	0x11
#define RULE_FIELD_NEXT_HEADER     	0x12

#define RULE_OPRT_NEVER_MATCH      	0x00
#define RULE_OPRT_EQUAL            	0x01
#define RULE_OPRT_NOT_EQUAL        	0x02
#define RULE_OPRT_LESS_EQUAL       	0x03
#define RULE_OPRT_GRATER_EQUAL     	0x04
#define RULE_OPRT_EXIST            	0x05
#define RULE_OPRT_NOT_EXIST        	0x06
#define RULE_OPRT_ALWAYS           	0x07
typedef struct oamRuleEntry4_s
{
	u_char	field;
	u_char	matchs[6];
	u_char	validOperator;
}OamQosRuleEntry4_t, *OamQosRuleEntry4_Ptr;

typedef struct oamRuleEntry6_s
{
	u_char	field;
	u_char	matchs[16];
	u_char	validOperator;
}OamQosRuleEntry6_t, *OamQosRuleEntry6_Ptr;


typedef struct oamQosRule_s
{
	u_char	precedence; // the index of this rule in ruleTable
	u_char	length;     // the length of from queueMapped to end
	u_char	queueMapped;// the number of queue to be mapped
	u_char	etherPriorityMark;
	u_char	entriesNum;
	// OamRuleEntry_t	*entries[];
}OamQosRule_t, *OamQosRule_Ptr;


#define ACTION_DELETE	0x00
#define ACTION_ADD   	0x01
#define ACTION_CLEAR 	0x02
#define ACTION_LIST  	0x03
typedef struct oamCM_s
{
	u_char	action;
	u_char	rulesNum;
	// OamRule_t	rules[];
}OamCM_t, *OamCM_Ptr;


typedef struct oamMulticastVlan_s
{
	u_char	action;
	u_short	vids[];
}OamMulticastVlan_t, *OamMulticastVlan_Ptr;


typedef struct oamVlanSwitch_s
{
	u_short	mcVlanId;
	u_short	iptvVlanId;
}OamVlanSwitch_t, *OamVlanSwitch_Ptr;

#define TAG_STRIPED_NO    	0x00
#define TAG_STRIPED_YES   	0x01
#define TAG_STRIPED_SWITCH	0x02
typedef struct oamMulticastTagOper_s
{
	u_char	tagStriped;
	u_char	switchNum;	// max value is 8
	OamVlanSwitch_t	switchs[];
}OamMulticastTagOper_t, *OamMulticastTagOper_Ptr;

#define MC_CTRL_TYPE_SNOOPING 0x00
#define MC_CTRL_TYPE_CTC      0x01

typedef struct oamMCC1Entry_s
{
	u_short	portId;
	u_short	vid;
	u_char	mac[6];	// multicast GDA mac/ ipv4
}OamMCC1Entry_t, *OamMCC1Entry_Ptr;

typedef struct oamMCC2Entry_s
{
	u_short	portId;
	u_char	mac[6];	// mac addr
	u_int	srcIp;	// dst ip addr
}OamMCC2Entry_t, *OamMCC2Entry_Ptr;

typedef struct oamMCC3Entry_s
{
	u_short	portId;
	u_short	vid;
	u_char	ip6[16];	// ipv6 addr
}OamMCC3Entry_t, *OamMCC3Entry_Ptr;

typedef struct oamMCC4Entry_s
{
	u_short	portId;
	u_char	mac[6];  // multicast mac addr 
	u_char	srcIp[16];	// ipv6 addr
}OamMCC4Entry_t, *OamMCC4Entry_Ptr;

#define MULITCAST_TYPE_MAC        	0x00
#define MULITCAST_TYPE_MAC_VLANID 	0x01
#define MULITCAST_TYPE_MAC_SAIPV4  	0x02
#define MULITCAST_TYPE_IPV4_VLANID	0x03
#define MULITCAST_TYPE_IPV6_VLANID	0x04
#define MULITCAST_TYPE_MAC_SAIPV6  	0x05


typedef struct oamMulticastControl_s
{
	u_char	action;
	u_char	type;
	u_char	entriesNum;
//	u_char 	entries[];	// OamMCC(1-4)_t
}OamMulticastControl_t,*OamMulticastControl_Ptr;


#define FAST_LEAVE_IGMP_NON	0x00000000
#define FAST_LEAVE_IGMP_YES	0x00000001
#define FAST_LEAVE_CTL_NON 	0x00000010
#define FAST_LEAVE_CTL_YES 	0x00000011
#define FAST_LEAVE_MLD_NON 	0x00000002
#define FAST_LEAVE_MLD_YES 	0x00000003
typedef struct oamFastLeaveAbility_s
{
	u_int	enumNum;
	u_int	mode[];
}OamFastLeaveAbility_t, *OamFastLeaveAbility_Ptr;


typedef struct oamLLIDQueueConfigEntry_s
{
	u_short	queueIdx;
	u_short	WRRWeight;
}OamLLIDQueueConfigEntry_t, *OamLLIDQueueConfigEntry_Ptr;


typedef struct oamLLIDQueueConfig_s
{
	u_char	num;
	OamLLIDQueueConfigEntry_t	entries[];
}OamLLIDQueueConfig_t, *OamLLIDQueueConfig_Ptr;

/* For MTU/MDU SNMP remote manager structures */
/*
typedef struct oamMxUMngGlobalParameterV4_s
{
	u_char	mngIPAddr[4];
	u_char	mngIPMask[4];
	u_char	mngIPGW[4];
	u_short	mngDataCVlan;
	u_short	mngDataSVlan;
	u_char	mngDataPriority;
}OamMxUMngGlobalParameterV4_t, *OamMxUMngGlobalParameterV4_Ptr;

typedef struct oamMxUMngGlobalParameterV6_s
{
	u_char	mngIPAddr[16];
	u_char	prefix[4];
	u_char	mngIPGW[16];
	u_short	mngDataCVlan;
	u_short	mngDataSVlan;
	u_char	mngDataPriority;
}OamMxUMngGlobalParameterV6_t, *OamMxUMngGlobalParameterV6_Ptr;

typedef struct oamMxUMngSNMPParameterV4_s
{
	u_char	snmpVersion;
	u_char	trapHostIPAddr[4];
	u_short	trapPort;
	u_short	snmpPort;
	char	securityName[32];
	char	communityForRead[32];
	char	communityForWrite[32];
}OamMxUMngSNMPParameterV4_t, *OamMxUMngSNMPParameterV4_Ptr;

typedef struct oamMxUMngSNMPParameterV6_s
{
	u_char	snmpVersion;
	u_char	trapHostIPAddr[16];
	u_short	trapPort;
	u_short	snmpPort;
	char	securityName[32];
	char	communityForRead[32];
	char	communityForWrite[32];
}OamMxUMngSNMPParameterV6_t, *OamMxUMngSNMPParameterV6_Ptr;
*/

#define HOLDOVER_DISACTIVATED		0x00000001
#define HOLDOVER_ACTIVATED			0x00000002
typedef struct oamHoldoverConfig_s
{
	u_int	state;
	u_int	time;	//50-1000 ms
}OamHoldoverConfig_t;

#define TXPOWER_REENABLE		0
#define TXPOWER_SHUTDOWN		65535
typedef struct oamOnuTxPowerSupplyControlDraft_s
{
	u_int    action;
	u_char  onuID[6];
}OamOnuTxPowerSupplyControlDraft_t, *OamOnuTxPowerSupplyControlDraft_Ptr;


typedef struct oamOnuTxPowerSupplyControl_s
{
	u_short	action;
	u_char	onuID[6];
	u_int	opticalTransmitterID;
}OamOnuTxPowerSupplyControl_t, *OamOnuTxPowerSupplyControl_Ptr;

#ifdef TCSUPPORT_VOIP
#define IAD_SUPPORT_H248  0x00
#define IAD_SUPPORT_SIP   0x01
#define IAD_SUPPORT_MGCP	0x02
#define IAD_SUPPORT_IMSSIP	0x03
typedef struct oamIADInfo_s
{
	u_char	mac[6];
	u_char	protocolSupported;
	char	iadSoftwareVersion[32];
	char 	iadSoftwareTime[32];
	u_char	voipUserCount;
}OamIADInfo_t, *OamIADInfo_Ptr;


#define IP_MODE_STATIC    0x00
#define IP_MODE_DHCP      0x01
#define IP_MODE_PPPOE     0x02
#define IP_MODE_UNDEFINED 0xFF


#define PPPOE_AUTH_AUTO 0x00
#define PPPOE_AUTH_CHAP 0x01
#define PPPOE_AUTH_PAP  0x02

typedef struct oamGlobalParamConfig_s
{
	u_char	voiceIpMode;
	u_int	iadIpAddr;
	u_int	iadNetMask;
	u_int	iadDefaultGW;
	u_char	PPPoEMode;
	char	PPPoEUsername[32];
	char	PPPoEPassword[32];
	u_char	taggedFlag;
	u_short	voiceCVlan;
	u_short	voiceSVlan;
	u_char	voicePriority;	//default value is 5
}OamGlobalParamConfig_t, *OamGlobalParamConfig_Ptr;

typedef struct oamH248RTPTIDInfo_s
{
	u_char	num;
	char	name[32];
}OamH248RTPTIDInfo_t;


#define HEARTBEAT_ENABLE  0x00
#define HEARTBEAT_DISABLE 0x01
typedef struct oamSIPParamConfig_s
{
	u_short	mgPort;
	u_int	sipProxyServIp;
	u_short	sipProxyServPort;
	u_int	backupSipProxyServIp;
	u_short	backupSipProxyServPort;
	u_int	activeSipProxyServIp;
	u_int	sipRegServIp;
	u_short	sipRegServPort;
	u_int	backupSipRegServIp;
	u_short	backupSipRegServPort;
	u_int	outBoundServIp;
	u_short	outBoundServPort;
	u_int	sipRegInterval;
	u_char	heartbeatSwitch;
	u_short	heartbeatCycle;
	u_short	heartbeatCount;
}OamSIPParamConfig_t, *OamSIPParamConfig_Ptr;


typedef struct oamSIPUserParamConfig_s
{
	char	userAccount[16];
	char	userName[32];
	char	userPassword[16];
}OamSIPUserParamConfig_t, *OamSIPUserParamConfig_Ptr;


#define VOICE_T30	0x00
#define VOICE_T38	0x01
#define CONSULT  	0x00
#define AUTOVBD  	0x01

typedef struct oamFaxModemConfig_s
{
	u_char	voiceT38Enable;	//default is T30
	u_char	voiceControl;
}OamFaxModemConfig_t, *OamFaxModemConfig_Ptr;


#define IAD_OPER_STATUS_ONGOING    	0x00000000
#define IAD_OPER_STATUS_SUCCESS    	0x00000001
#define IAD_OPER_STATUS_IADERROR   	0x00000002
#define IAD_OPER_STATUS_LOGOUT     	0x00000003
#define IAD_OPER_STATUS_REBOOTING  	0x00000004


#define IAD_PORT_STATUS_REGISTING  	0x00000000
#define IAD_PORT_STATUS_IDLE       	0x00000001
#define IAD_PORT_STATUS_OFFHOOK    	0x00000002
#define IAD_PORT_STATUS_DIALUP     	0x00000003
#define IAD_PORT_STATUS_RINGING    	0x00000004
#define IAD_PORT_STATUS_RINGBACK   	0x00000005
#define IAD_PORT_STATUS_CONNECTING 	0x00000006
#define IAD_PORT_STATUS_CONNECTED  	0x00000007
#define IAD_PORT_STATUS_RELEASECONN	0x00000008
#define IAD_PORT_STATUS_REG_FAILED 	0x00000009
#define IAD_PORT_STATUS_NOTACTIVED 	0x0000000A

#define IAD_PORT_SERVICE_ENDLOCAL  	0x00000000
#define IAD_PORT_SERVICE_ENDREMOTE 	0x00000001
#define IAD_PORT_SERVICE_ENDAUTO   	0x00000002
#define IAD_PORT_SERVICE_NORMAL    	0x00000003

#define IAD_PORT_CODEC_MODE_G711A  	0x00000000
#define IAD_PORT_CODEC_MODE_G729   	0x00000001
#define IAD_PORT_CODEC_MODE_G711U  	0x00000002
#define IAD_PORT_CODEC_MODE_G723   	0x00000003
#define IAD_PORT_CODEC_MODE_G726   	0x00000004
#define IAD_PORT_CODEC_MODE_T38    	0x00000005

#define IAD_OPER_REREGISTER  0x00000000
#define IAD_OPER_LOGOUT      0x00000001
#define IAD_OPER_RESET       0x00000002

typedef struct oamPOTSStatus_s
{
	u_int	iadPortStatus;
	u_int	iadPortServiceStatus;
	u_int	iadPortCodecMode;
}OamPOTSStatus_t, *OamPOTSStatus_Ptr;

typedef struct oamSIPDigitMap_s
{
	u_char blockNum;
	u_char blockNo;
	u_char block[0];
}OamSIPDigitMap_t, *OamSIPDigitMap_Ptr;
#endif // TCSUPPORT_VOIP

typedef struct oamAlarmState_s
{
	u_short alarmId;
	u_int   status;
}OamAlarmState_t, *OamAlarmState_Ptr;

typedef struct oamAlarmControl_s
{
	u_char  branch;
	u_short leaf;
	u_char  len;
	u_short alarmId;
	u_int   status;
}OamAlarmControl_t, *OamAlarmControl_Ptr;

typedef struct oamAlarmThreshold_s
{
	u_short alarmId;
	u_int   alarm;
	u_int   clear;
}OamAlarmThreshold_t, *OamAlarmThreshold_Ptr;

typedef struct oamAlarmThresholdControl_s
{
	u_char  branch;
	u_short leaf;
	u_char  len;
	u_short alarmId;
	u_int   alarm;
	u_int   clear;
}OamAlarmThresholdControl_t, *OamAlarmThresholdControl_Ptr;


#ifdef TCSUPPORT_EPON_CTC_POWERSAVING
typedef struct OamCtcSleepControl_s
{
	u_int  startTime;  // timestamp(TQ)
	u_int  sleepDuration;  // the duration of the sleep period
	u_int  wakeDuration;
	u_char sleepFlag;
	u_char sleepMode;
}OamCtcSleepControl_t, *OamCtcSleepControl_Ptr;
#endif

typedef struct pmStatus_s
{
	u_short enable;
	u_int   period;
}PMStatus_t, *PMStatus_Ptr;

typedef struct pmCounts_s
{
    long long downstreamDropEvents;
    long long upstreamDropEvents;
    long long downstreamOctets;
    long long upstreamOctets;
    long long downstreamFrames;
    long long upstreamFrames;
    long long downstreamBroadcastFrames;
    long long upstreamBroadcastFrames;
    long long downstreamMulticastFrames;
    long long upstreamMulticastFrames;
    long long downstreamCrcErroredFrames;
    long long upstreamCrcErroredFrames;
    long long downstreamUndersizeFrames;
    long long upstreamUndersizeFrames;
    long long downstreamOversizeFrames;
    long long upstreamOversizeFrames;
    long long downstreamFragments;
    long long upstreamFragments;
    long long downstreamJabbers;
    long long upstreamJabbers;
    long long downstream64Frames;
    long long upstream64Frames;
    long long downstream127Frames;
    long long upstream127Frames;
    long long downstream255Frames;
    long long upstream255Frames;
    long long downstream511Frames;
    long long upstream511Frames;
    long long downstream1023Frames;
    long long upstream1023Frames;
    long long downstream1518Frames;
    long long upstream1518Frames;
    long long downstreamDiscards;
    long long upstreamDiscards;
    long long downstreamErrors;
    long long upstreamErrors;
    long long statusChangeTimes;
}PMCounters_t, *PMCounters_Ptr;
#ifdef TCSUPPORT_VOIP
typedef struct oamH248ParamConfig_s
{
	u_short MGPortNo;
	u_int  MGCIP;
	u_short MgcComPortNo;
	u_int   BackupMgcIp;
	u_short  BackupMgcComPortNo;
	u_char ActiveMGC;
	u_char RegMode;
	u_char MID[64];
	u_char HeartbeatMode;
	u_short HeartbeatCycle;
	u_char HeartbeatCount;
}OamH248ParamConfig_t, *OamH248ParamConfig_Ptr;

typedef struct oamH248UserTIDInfo_s
{
	u_char UserTIDName[32];
}OamH248UserTIDInfo_t, *OamH248UserTIDInfo_Ptr;


typedef struct oamH248RTPTIDConfig_s
{
	u_char NumOfRTPTID;
	u_char RTPTIDPrefix[16];
	u_char RTPTIDDigitBegin[8];
	u_char RTPTIDMode;
	u_char RTPTIDDigitLen;
}OamH248RTPTIDConfig_t, *OamH248RTPTIDConfig_Ptr;
#endif

#ifdef TCSUPPORT_VOIP
#define TIME_CFG_DML_NAME	"InterDigitTimerLong"
#define TIME_CFG_DMS_NAME	"InterDigitTimerShort"
#define TIME_CFG_DMLS_MIN_VAL	1
#define TIME_CFG_DMLS_MAX_VAL	40
#define TIME_CFG_DML_DEF_VAL	20
#define TIME_CFG_DMS_DEF_VAL	5

typedef struct oamTimeCfg_s{
	u_char dml;
	u_char dms;
}oamTimeCfg_t, *oamTimeCfg_Ptr;
#endif
#pragma pack(pop)

void htonpm(PMCounters_Ptr pCnt);
void ntohpm(PMCounters_Ptr pCnt);

char* nodeTypeName(u_char type);
char* objTypeName(OamObject_Ptr pObj);
u_char sysObjType(OamObject_Ptr pObj);

/* Get port real index from the port code defined in ctc epon spec */
#define PORT_ERROR   0xFF
u_char sysObjIdx(OamObject_Ptr pObj, u_char objType);

int isOamAttribActBranch(u_char branch);
int isOamObjBranch(u_char branch);
int isOamPortAll(OamObject_Ptr oop);

int isNeedObject(OamParamNode_Ptr opnp);
int isObjectBranchLeaf(OamObject_Ptr oop, OamParamNode_Ptr opnp);
OamParamNode_Ptr findOamParamNodeByBranchLeaf(u_char branch, u_short leaf);
void oamClearSelfConfig();
int oamResetSystemParam(IN u_char llid);

/*------------------------------------------------------------------------------------------*/

/*	here to define the GET/SET functions which oam parameters use.
*/
u_char oamGetPhyAdminState(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamGetAutoNegAdminState(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamGetAutoNegLocalTechAbility(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamGetAutoNegAdvTechAbility(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamGetFecAbility(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);

u_char oamGetFecMode(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetFecMode(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetOnuSN(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamGetFirmwareVer(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamGetChipsetID(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamGetOnuCapabilities1(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamGetOnuCapabilities2(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamGetOnuCapabilities3(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamGetPowerSavingCapabilities(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamGetOpticalTransceiverDiagnosis(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);

u_char oamGetServiceSLA(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetServiceSLA(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetEthPortPause(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetEthPortPause(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetEthLinkState(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);

u_char oamGetEthPortUSPolicing(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetEthPortUSPolicing(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

#ifdef TCSUPPORT_VOIP
u_char oamGetVoIPPort(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetVoIPPort(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);
#endif //TCSUPPORT_VOIP

u_char oamGetE1Port(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetE1Port(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetEthPortDSRateLimiting(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetEthPortDSRateLimiting(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetVlan(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetVlan(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetClassificationMarking(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetClassificationMarking(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetAddDelMulticastVlan(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetAddDelMulticastVlan(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetMulticastTagOper(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetMulticastTagOper(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetMulticastSwitch(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetMulticastSwitch(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetMulticastControl(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetMulticastControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetGroupNumMax(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetGroupNumMax(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetFastLeaveAbility(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);

u_char oamGetFastLeaveAdminState(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);

u_char oamGetLlidQueueConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetLlidQueueConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetMxUMngGlobalParameter(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetMxUMngGlobalParameter(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetMxUMngSNMPParameter(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetMxUMngSNMPParameter(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamSetPortLoopDetect(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetHoldoverConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetHoldoverConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetActivePONIFAdminstate(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetActivePONIFAdminstate(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamSetONUTxPowerSupplyControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

#ifdef TCSUPPORT_VOIP
u_char oamGetIADInfo(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);

u_char oamGetGlobalParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetGlobalParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetSIPParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetSIPParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetSIPUserParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetSIPUserParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetFaxModemConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetFaxModemConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetH248ParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetH248ParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetH248UserTIDInfo(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetH248UserTIDInfo(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamSetH248RTPTIDConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetTimerCfg(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetTimerCfg(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetH248RTPTIDInfo(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);


u_char oamGetH248IADOperationStatus(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);

u_char oamGetPOTSStatus(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
#endif // TCSUPPORT_VOIP

u_char oamSetPhyAdminControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);
u_char oamSetAutoNegRestartAutoConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);
u_char oamSetAutoNegAdminControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamSetResetOnu(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamSetFastLeaveAdminControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamSetMultiLlidAdminControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

#ifdef TCSUPPORT_VOIP
u_char oamSetIADOperation(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamSetSIPDigitMap(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);
#endif //TCSUPPORT_VOIP


u_char oamGetPowerSavingConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetPowerSavingConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetProtectionParameters(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetProtectionParameters(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamSetPortDisableLooped(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetAlarmAdminState(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetAlarmAdminState(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetAlarmThresholds(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetAlarmThresholds(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetPortMacAgingTime(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetPortMacAgingTime(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

#ifdef TCSUPPORT_PMMGR
u_char oamGetPerformanceStatus(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetPerformanceStatus(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetPerformanceDataCurrent(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetPerformanceDataCurrent(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

u_char oamGetPerformanceDataHistory(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
#endif

u_char oamSetSleepControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

#ifdef TCSUPPORT_EPON_OAM_CUC
#define OAM_CUC     "Oam_CUC"
#define OAM_CUC_SLA     "Oam_CUC_SLA"
#define OAM_CUC_LLID    "Oam_CUC_LLID%d"
#define OAM_CUC_PORT	"Oam_CUC_Port%d"
// CU OAM Version C1 & C2
#define oamIsCucVersn(ver) (((ver)==0xc1) || ((ver)==0xc2))
OamParamNode_Ptr findCucOamNodeByBranchLeaf(u_char branch, u_short leaf);
u_char oamGetPortMacLimit(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetPortMacLimit(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);
u_char oamGetPortModeStatus(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetPortModeCfg(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);
u_char oamGetCfgCnt(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetCfgCnt(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);
u_char oamSetCfgParaMngmt(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

#ifdef TCSUPPORT_VOIP
u_char oamSetPingTest(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);
u_char oamGetPingTestResult(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);

#pragma pack(push, 1)
typedef struct oamPingTest_s
{
	u_int		destIP;
	u_char	pingAction;
	u_short	pingTimes;
	u_short   	pingSize;
	u_short	timeout;
}OamPingTest_t, *OamPingTest_Ptr;

typedef struct oamPingTestResult_s
{
	u_char	testResult;
	u_short	txPkt;
	u_short	rxPkt;
	u_short	lostPkt;
	u_char	lostPktRatio;
	u_short	minDelay;
	u_short	maxDelay;
	u_short	avgDelay;
}OamPingTestResult_t, *OamPingTestResult_Ptr;
#pragma pack(pop)
#endif // TCSUPPORT_VOIP

u_char oamGetRMSGlobalParameterConfig1(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetRMSGlobalParameterConfig1(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);
u_char oamGetRMSGlobalParameterConfig2(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue);
u_char oamSetRMSGlobalParameterConfig2(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);
#pragma pack(push, 1)
typedef struct oamRMSGlbParaCfg1_s
{
	u_char	RMSIpMode;
	u_int		RMSIpAddr;
	u_int		RMSNetMask;
	u_int   	RMSDefaultGW;
	u_char	pppoeMode;
	u_char 	pppoeUsername[32];
	u_char 	pppoePassword[32];
	u_char	taggedFlag;
	u_short	CVlan;
	u_short	SVLAN;
	u_char	priority;
}oamRMSGlbParaCfg1_t, *oamRMSGlbParaCfg1_Ptr;

typedef struct oamRMSGlbParaCfg2_s
{
	u_char	numOfURLBlocks;
	u_char	seqOfURLBlock;
	u_char	RMS_URL[125];
}oamRMSGlbParaCfg2_t, *oamRMSGlbParaCfg2_Ptr;
#pragma pack(pop)
#endif /*TCSUPPORT_EPON_OAM_CUC */
/*------------------------------------------------------------------------------------------*/

#if 1 //def TCSUPPORT_RESET
/* For Support Delay To Take Affect */
#include "oam_ctc_var.h"

/* The node enums have priority as spec */
enum{
	FAST_LEAVE,
	MAX_LAST_WORK_NODES
};

typedef struct lastWorkNode_s
{
	u_char  flag;	// if TRUE: the node must be called.

	u_char llidIdx;
	OamObject_t obj;
	OamCtcBranchLeaf_t branchLeaf;
	int  length;
	u_char *pData;

	u_char  (*resetFunc)(
				IN  u_char llidIdx,
				IN  OamObject_Ptr pObj,
				IN  int	length,
				IN  u_char  *pValue
				);
}LastWorkNode_t, *LastWorkNode_Ptr;

void procLastWorkTable(Buff_Ptr bfp);
u_char oamResetFastLeaveAdminControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue);

#endif
#endif
