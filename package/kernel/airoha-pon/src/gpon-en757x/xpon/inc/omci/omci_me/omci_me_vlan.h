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
	omci_me_vlan.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wayne.lee	2012/7/28	Create
*/

#ifndef  _OMCI_VLAN_ME_H_
#define _OMCI_VLAN_ME_H_
#include "omci_types.h"
#include "omci_me.h"
#ifdef TCSUPPORT_PON_VLAN
#include "../../../lib/pon_vlan/lib_pon_vlan.h"
#endif
/*******************************************************************************************************************************
vlan API  function 

********************************************************************************************************************************/
#ifndef TCSUPPORT_PON_VLAN
typedef struct pon_vlan_rule_s
{
	//Only used in tag_num = 2,filter outer Tag info
	uint32 filter_outer_tpid;//0:don't care, 1:0x8100 2:input TPID (downstrean 3:output TPID)
	uint8 filter_outer_pri;//0~7:pbit value  8:don't care
	uint8 filter_outer_dei;//0,1:DEI value 2:don't care
	uint16 filter_outer_vid;//0~4094:VID 4096:don't care
	
	//used in tag_num = 1 or 2.If Tag Num = 2,this field is
	//filter inner tag Info.
	uint32 filter_inner_tpid;
	uint8 filter_inner_pri;
	uint8 filter_inner_dei;
	uint16 filter_inner_vid;

	//used in downstream direction and tagNum is 4
	uint32 down_filter_outer_tpid;
	uint8 down_filter_outer_pri;
	uint8 down_filter_outer_dei;
	uint16 down_filter_outer_vid;
	
	//used in downstream direction and tagNum is 3
	uint32 down_filter_inner_tpid;
	uint8 down_filter_inner_pri;
	uint8 down_filter_inner_dei;
	uint16 down_filter_inner_vid;
	
	uint8 tag_num;//0~5 	(downstream 3,4 mean tagNum 5:ignore this entry)
	uint16 filter_ethertype;//0x0000 :Don't care. Other value mean true Ethertype.0x0800 mean IPoE frames

	/*
	0:transparent
	1:block
	2x:x=1,2. Add x Tag(s)
	3x:x=1,2. Del x Tag(s)
	4x:x=0,1,2. 40:change inner Tag. 41:change outer Tag. 42:change two tags.
	treatment_method should be set 40 if you want change tag when packet only have one tag.
	*/
	uint8 treatment_method;

	//used when treatment_method = 2 and Add_Num = 1 or 2
	//this field indicate how to add first Tag.
	uint32 add_fst_tpid;//0:0x8100 1:output TPID 2:copy from inner tag 3:copy from outer tag	(downstream 4:input TPID)
	uint8 add_fst_pri;//0~7 pbit value . 8:copy from inner tag 9:copy from outer tag 10:derive pbit from DSCP
	uint8 add_fst_dei;//0,1:DEI value 2:copy from inner tag 3:copy from outer tag
	uint16 add_fst_vid;//0~4094 VID value 4096:copy from inner tag 4097:copy from outer tag

	//used when treatment_method = 2 and Add_Num = 2
	//this field indicate how to add second Tag.
	uint32 add_sec_tpid;
	uint8 add_sec_pri;
	uint8 add_sec_dei;
	uint16 add_sec_vid;
	
	uint32 rule_priority;//0:high 1:low
}pon_vlan_rule,*pon_vlan_rule_p;

typedef struct pon_vlan_ioctl_s
{
	/*
	PortNumDefine
	00:CPE	01~09 reserved
	11~14: lan port 1~4.	10,15~19 reserved
	21~24: wlan port 1~4. 20,25~29 reserved
	30:usb?not uesd
	40:default rule
	-1 error
	*/
	int port;

	// 0:disable igmp tagging.Handle igmp packet as general packet.
	// 1:enable igmp tagging.Tagging igmp packet follow igmp_mode.
	uint8 igmp_enable;
	
	/*
	0: Pass upstream IGMP traffic transparently.
	1: Add tag.TPID=0x8100.TCI = igmp_tci.
	2: Replace TCI field with igmp_tci.If original frame is untagged.Add a tag,TPID = 0x8100 & tci = igmp_tci.
	3: Replace VID field with the VID field in igmp_tci.If original frame is untagged.Add a tag,TPID = 0x8100 & tci = igmp_tci.
	*/
	uint8 igmp_mode;
	uint16 igmp_tci;

	// 0:disable vlan tagging.
	// 1:enable vlan tagging.
	uint8 vlan_enable;
	uint8 rule_index;//vlan rule index.
	uint8 option_flag;//0:get 1:set 2:del 3: clean 4:show
	uint8 tpid_type;//flag for type of tpid.0:add special ethertype to linux kernel ethertype list. 1:set the input and output tpid in the rule
	uint16 special_tpid;//any value but the one has been used.
	uint16 input_tpid;
	uint16 output_tpid;
	uint32 dscp_map[6];//24byte map
	uint8 downstream_mode;//0:transparent 1:do inverse option 2:stripped outer tag
	uint8 default_vlan_rule_flag;//0:not use default vlan when no rule in this port. 1:use default when no rule in this port.

	uint8 if_vlan_switch;//0:disable 1:enable
	uint8 dev_name[9];//name is like nasx_y
	uint16 dev_vid;//allowed VID on Interface,VID = 4097 mean discard all packet, VID = 4096 mean forward all packet.default value is 4097.

	uint8 pcp_mode;//0:8P0D 1:7P1D   2:6P2D  3:5P3D

	uint8 mac_bind_vid_enable;
	int mac_vlan_time;

	__u8 user_group_enable;
	__u32 user_group;
	uint8 dbg_level;//0:low, only can see error message  1:medium,can see result of operation 2:high,can see lots of info when do the operation
	pon_vlan_rule rule;
}pon_vlan_ioctl, *pon_vlan_ioctl_p;
#endif

/*******************************************************************************************************************************
vlan API  function end

********************************************************************************************************************************/
/*******************************************************************************************************************************
vlan filter API start

********************************************************************************************************************************/

/*******************************************************************************************************************************
vlan filter API end

********************************************************************************************************************************/
enum omciVlanStatus {
	OMCI_VLAN_NO_FIND = 0,
	OMCI_VLAN_FIND = 1
};

/*9.3.10*/
#define  OMCI_DOT1P_TP_TYPE_BRIDGE_MAPPING	0
#define  OMCI_DOT1P_TP_TYPE_PPTP_UNI			1
#define  OMCI_DOT1P_TP_TYPE_ACTION_TYPE_0	0 //according pptp uni ME, add gem port mapping rule
#define  OMCI_DOT1P_TP_TYPE_ACTION_TYPE_1	1 //del gem port mapping rule about PPTP UNI

#define  OMCI_DOT1P_INTERWORK_TP_DISCARD	0xffff
#define  OMCI_DOT1P_GEM_PORT_DISCARD		0xffff

#define  OMCI_DOT1P_ATTR_13					13 //tp type attribute

#define  OMCI_DOT1P_UNMARK_0				0 //dscp pbit for untagged frame
#define  OMCI_DOT1P_UNMARK_1				1 //default pbit for untagged frame

#define  OMCI_UPDATE_GEMPORT_MAPP_RULE 		1
#define  OMCI_NO_UPDATE_GEMPORT_MAPP_RULE 	0


/*9.3.11*/
#define VLAN_FILTER_ACTION_TYPE_A		0
#define VLAN_FILTER_ACTION_TYPE_C		1
#define VLAN_FILTER_ACTION_TYPE_G		2
#define VLAN_FILTER_ACTION_TYPE_H		3
#define VLAN_FILTER_ACTION_TYPE_J		4


#define VLAN_FILTER_OPERATION_TYPE0		0x0 //tag:bridge, untag:bridge
#if 0//TCSUPPORT_VLAN_FILTER_EX
#define VLAN_FILTER_OPERATION_TYPE4		0x04 // tag:H(vid), untag: discard
#define VLAN_FILTER_OPERATION_TYPE10	0x10 // tag:H(vid), untag: discard

#define VLAN_FILTER_OPERATION_TYPE3		0x03 // tag:H(vid), untag: bridge
#define VLAN_FILTER_OPERATION_TYPE0F	0x0F // tag:H(vid), untag: bridge

#define VLAN_FILTER_OPERATION_TYPE8		0x08 // tag:H(pbit), untag: discard
#define VLAN_FILTER_OPERATION_TYPE12	0x12 // tag:H(pbit), untag: discard

#define VLAN_FILTER_OPERATION_TYPE7		0x07 // tag:H(pbit), untag: bridge
#define VLAN_FILTER_OPERATION_TYPE11	0x11 // tag:H(pbit), untag: discard

#define VLAN_FILTER_OPERATION_TYPE16	0x16
#endif

#define MAX_VLAN_FILTER_LIST_LEN		24 //24 bytes

//#define ADD_EBTABLE_RULE_FOR_VLAN_FILTER_SH	"/tmp/omci/add_rule_for_vlan_filter.sh"

#define VLAN_FILTER_ME_VLAN_FILTER_LIST_ATT		0
#define VLAN_FILTER_ME_FORWARD_OPERATION_ATT	1
#define VLAN_FILTER_ME_NUM_OF_ENTRY_ATT			2

#define MAX_VLAN_FILTER_TCI_LIST_NUM	12

/*9.3.12*/
#define OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_0		0 //physical path termination point ethernet UNI
#ifdef TCSUPPORT_PON_IP_HOST
#define OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_1		1 //IP host config data
#endif
#define OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_3		3 //mac bridge port configuration data
#define OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_10	10 //physical path termination point ethernet UNI

#define OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_1	1 //modify upstream vlan tagging operation mode attribute
#define OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_2	2 //modify tci attribute
#define OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_4	4 //modify associate type attribute
#define OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_5	5 //modify associate pointer attribute


#define OMCI_VLAN_TAG_OP_MODE_0	0 //upstream frame is send as is, regardless of tag
#define OMCI_VLAN_TAG_OP_MODE_1	1 //modify tag for tagged frame, add tag for untagged frame
#define OMCI_VLAN_TAG_OP_MODE_2	2 //add tag for untagged or taged frame
/*9.3.13*/
#define OMCI_EXT_VLAN_TAG_ATTR_JUST_8BYTES				8 // just to  judge delete rule byte.
#define OMCI_EXT_VLAN_TAG_ATTR_LAST_8BYTES				8 // last 8 bytes number.
#define OMCI_EXT_VLAN_TAG_DEL_ENTRY						1
#define OMCI_EXT_VLAN_TAG_NO_DEL_ENTRY					0

#define OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_1				1 //association type attribute
#define OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_7				7 //association pointer attribute
#define OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_3				3 //input TPID attribute
#define OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_4				4 //output TPID attribute
#define OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_17				17 //delete action 



#define OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_0		0 //mac bridge port configuration data
#define OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_2		2 //physical path termination point ethernet UNI
#ifdef TCSUPPORT_PON_IP_HOST
#define OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_3		3 //IP host config data
#endif
#define OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_10		10//virtual Ethernet interface point
#define OMCI_DSCP_MAP_U32_NUM							6
#define OMCI_DSCP_MAP_U8_NUM								24
#define OMCI_EXT_VLAN_TAG_OP_DOWN_MODE_0				0 //reverse
#define OMCI_EXT_VLAN_TAG_OP_DOWN_MODE_1				1 //transparent
#define OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN				16

#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_8			8 //do not filter
#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_14			14 //default filter
#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_15			15 //no tag

#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_VID_4096		4096 //do not filter

#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE0		0 //do not filter
#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE4		4 //TPID = 8100
#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE5		5 //input tpid, and do not care DEI
#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE6		6 //input tpid, DEI = 0
#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE7		7 //input tpid, DEI = 1

#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_ETHERTYPE_MODE0	0 //don't filer	
#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_ETHERTYPE_MODE1	1 //0x0800, IPoE
#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_ETHERTYPE_MODE2	2 //0x8863, 0x8864, PPPoE
#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_ETHERTYPE_MODE3	3 //0x0806, ARP
#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_ETHERTYPE_MODE4	4 //0x86dd, IPv6 IPoE

#define OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PPPoE_8863			0x8863

#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TAG_MODE0		0 //remove 0 tag
#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TAG_MODE1		1 //remove 1 tag
#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TAG_MODE2		2 //remove 2 tag
#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TAG_MODE3		3 //discard the frame

#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE0		0 //copy TPID ad DEI from inner tag of received frame
#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE1		1 //copy TPID ad DEI from outer tag of received frame
#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE2		2 //TPID=output TPID, copy DEI from inner tag of received frame
#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE3		3 //TPID=output TPID, copy DEI from outer tag of received frame
#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE4		4 //TPID = 0x8100
#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE5		5 //reserved
#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE6		6 //TPID=output TPID, DEI = 0
#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE7		7 //TPID=output TPID, DEI = 1


#define OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_PBIT_15		15 //no add tag

#define OMCI_EXT_VLAN_TAG_OP_TBL_DEFAULT_RULE_FLAG_1	1
#define OMCI_EXT_VLAN_TAG_OP_TBL_SET_SECOND_RULE_1		1 //need set the second vlan operation rule
#define OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1			1 //handle this vlan tag
#define OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_0			0  //don't handle this vlan tag

#define OMCI_EXT_VLAN_TAG_OP_TBL_SET_DEFAULT_RULE_FLAG_1	1 //need set untagged, 1 tagged, 2 tagged default rule.


/*9.3.12, 9.3.13*/
#define OMCI_VLAN_BASIC_LAN_PORT_VAL			10 //valid vlan lan port value :11~14
#ifdef TCSUPPORT_PON_IP_HOST
#define OMCI_VLAN_BASIC_IP_HOST_PORT_VAL		50 //valid vlan ip host port value :50
#endif
#define OMCI_MAX_PORT_VLAN_RULE_NUM			32 //max vlan rule number by port
#define OMCI_VLAN_TAG_OP_RULE_TPID_88A8		0x88a8
#define OMCI_VLAN_TAG_OP_RULE_TPID_8100		0x8100

#define OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_0			0 //don't care
#define OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_1	 		1//0x8100
#define OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_2	 		2//input TPID
#define OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_3	 		3//output TPID-->downstream

#define OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8	 		8//don't care
#define OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2			2//don't care
#define OMCI_VLAN_TAG_OP_RULE_FILTER_VID_4096		4096//don't care
	
#define OMCI_VLAN_TAG_OP_RULE_TAG_NUM_0	 			0 
#define OMCI_VLAN_TAG_OP_RULE_TAG_NUM_1	 			1
#define OMCI_VLAN_TAG_OP_RULE_TAG_NUM_2	 			2
#define OMCI_VLAN_TAG_OP_RULE_TAG_NUM_3	 			3 //using for downstream
#define OMCI_VLAN_TAG_OP_RULE_TAG_NUM_4	 			4 //using for downstream 
#define OMCI_VLAN_TAG_OP_RULE_TAG_NUM_5	 			5 //ignore this entry

#define OMCI_VLAN_TAG_OP_RULE_FILTER_ETHERTYPE_0	0 //don't care

#define OMCI_VLAN_TAG_OP_RULE_OP_0	 				0 //transparent
#define OMCI_VLAN_TAG_OP_RULE_OP_1	 				1 //block
#define OMCI_VLAN_TAG_OP_RULE_OP_21	 				21 //add 1 tag
#define OMCI_VLAN_TAG_OP_RULE_OP_22	 				22 //add 2 tags
#define OMCI_VLAN_TAG_OP_RULE_OP_23	 				23 //del 1 tag and add 2 tags

#define OMCI_VLAN_TAG_OP_RULE_OP_31	 				31 //del 1 tag
#define OMCI_VLAN_TAG_OP_RULE_OP_32	 				32 //del 2 tags
#define OMCI_VLAN_TAG_OP_RULE_OP_33	 				33 //del 2 tags and add 1 tag

#define OMCI_VLAN_TAG_OP_RULE_OP_40	 				40 //change inner tag
#define OMCI_VLAN_TAG_OP_RULE_OP_41	 				41 //change outer tag
#define OMCI_VLAN_TAG_OP_RULE_OP_42	 				42 //change two tags

#define OMCI_VLAN_TAG_OP_RULE_ADD_TPID_0			0 //0X8100
#define OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1			1 //output tpid
#define OMCI_VLAN_TAG_OP_RULE_ADD_TPID_2			2 //copy from inner tag
#define OMCI_VLAN_TAG_OP_RULE_ADD_TPID_3			3 //copy from outer tag
#define OMCI_VLAN_TAG_OP_RULE_ADD_TPID_4			4 //input TPID-->downstream

#define OMCI_VLAN_TAG_OP_RULE_ADD_PRI_8				8 //copy from inner tag
#define OMCI_VLAN_TAG_OP_RULE_ADD_PRI_9				9 //copy from outer tag
#define OMCI_VLAN_TAG_OP_RULE_ADD_PRI_10			10 //derive pbit from DSCP

#define OMCI_VLAN_TAG_OP_RULE_ADD_DEI_2				2 //copy from inner tag
#define OMCI_VLAN_TAG_OP_RULE_ADD_DEI_3				3 //copy from outer tag


#define OMCI_VLAN_TAG_OP_RULE_ADD_VID_4096			4096 //copy from inner tag
#define OMCI_VLAN_TAG_OP_RULE_ADD_VID_4097			4097 //copy from outer tag

#define OMCI_VLAN_TAG_OP_RULE_PRI_LOW				1 //set vlan rule to low priority 
#define OMCI_VLAN_TAG_OP_RULE_PRI_HIGH				0 //set vlan rule to high priority

#define OMCI_VLAN_TAG_OP_RULE_LAN_PORT1				11 //lan port 1
#define OMCI_VLAN_TAG_OP_RULE_LAN_PORT2				12 //lan port 2
#define OMCI_VLAN_TAG_OP_RULE_LAN_PORT3				13 //lan port 3
#define OMCI_VLAN_TAG_OP_RULE_LAN_PORT4				14 //lan port 4

#define OMCI_VLAN_TAG_OP_RULE_WLAN_PORT1			21 //wlan port 1
#define OMCI_VLAN_TAG_OP_RULE_WLAN_PORT2			22 //wlan port 2
#define OMCI_VLAN_TAG_OP_RULE_WLAN_PORT3			23 //wlan port 3
#define OMCI_VLAN_TAG_OP_RULE_WLAN_PORT4			24 //wlan port 4

#define OMCI_VLAN_TAG_OP_RULE_DEFAULT_PORT			40 //default rule port id

#define OMCI_VLAN_TAG_OP_RULE_DEFAULT_DOWNSTREAM_MODE	0 //0:transparent 
#define OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_MODE_0	0 //0:transparent 
#define OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_MODE_1	1 // 1: inverse operation to upstream
#define OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_MODE_2			2 //strip a tag
#define OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_ATT_STRIP_VAL	1 //9.3.12 downstream attribute value 1

#define OMCI_VLAN_TAG_PBIT_0							0 //min valid pbit =0
#define OMCI_VLAN_TAG_PBIT_7							7 //max valid pbit =7

#define OMCI_VLAN_TAG_VID_0							0 //min valid vid =0
#define OMCI_VLAN_TAG_VID_4094						4094 //max valid vid =4094

#define OMCI_VLAN_TAG_DEI_0							0 //min valid DEI =0
#define OMCI_VLAN_TAG_DEI_1							1 //max valid DEI =1
#define OMCI_VLAN_TAG_DEI_2							2 //copy from inner tag 
#define OMCI_VLAN_TAG_DEI_3							3 //copy from outer tag 


/*******************************************************************************************************************************
general function

********************************************************************************************************************************/
int convertDSCPMap2Pbit(IN uint8* dscp, OUT uint8 * dscp2Pbit);
int getVlanRulePortIdByInst(uint16 classId, uint16 instId, uint8 * vlanRulePortId);
int initVlanTagOpRule(pon_vlan_rule_p rule_ptr);
int setVlanTagOpRuleValByTci(uint8 upMode, uint16 tci, pon_vlan_rule_p untaggedRule_ptr, pon_vlan_rule_p taggedRule_ptr);
int findVlanTagOpRuleByRuleInfo(uint8 vlanRulePortId, pon_vlan_rule_p findRule_ptr);
int delVlanTagOpRuleByRuleInfo(uint8 vlanRulePortId, pon_vlan_rule_p delRule_ptr);
int setVlanTagRuleByInst(IN uint16 classId, IN uint16 instId, IN  uint8 vlanRulePortId);
int addVlanTagRuleByVlanPortId(IN uint16 classId, IN uint8 vlanRulePortId);
int setVlanRuleToDefaultByPort(uint8 vlanRulePortId);
int setExtVlanOpTBLDefaultRule(void);
int swapOmciVlanTag(IN pon_vlan_rule_p rule_ptr);
int setExtVlanTagRuleByPkt(OUT pon_vlan_rule_p rule_ptr, OUT uint8 * defaultRuleFlag,  IN uint8 *val_ptr, IN uint8 size);
int addGemPortMappingRuleByAniPort(IN uint16 instId);
int delGemPortMappingRuleByMbp(IN uint16 instId);
int checkMappingRuleUntagAction(int16 instanceId);
/*******************************************************************************************************************************
9.3.10 802.1p mapper service profile

********************************************************************************************************************************/
int omciMeInitFor8021pMapperProfile(omciManageEntity_t *omciManageEntity_p);
int omci8021pMapperProfileDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int updateGemMappingRuleByMB(omciManageEntity_t *me_p, omciMeInst_t *inst_p, uint16 dot1pInstId);
int updateGemMappingRuleBy8021p(uint16 dot1pInstId);
int setdot1pTPPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pTPPbit0Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pTPPbit1Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pTPPbit2Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pTPPbit3Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pTPPbit4Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pTPPbit5Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pTPPbit6Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pTPPbit7Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pUnmarkOptionValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pDscpPbitMappValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pDefaultPbitValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setdot1pTPTypeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
9.3.11 vlan tagging filter data

********************************************************************************************************************************/
int omciMeInitForVlanTagFilterData(omciManageEntity_t *omciManageEntity_p);
int omciVlanTagFilterDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int setvlanFilterVlanFilterListValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setvlanFilterForwardOPValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setvlanFilterValidNumberEntryValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);


/*******************************************************************************************************************************
9.3.12 vlan tagging operation configuration data

********************************************************************************************************************************/
int omciMeInitForVlanTagOperation(omciManageEntity_t *omciManageEntity_p);
int omciVlanTagDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int setVlanTagOpUpModeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setVlanTagOpTCIValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setVlanTagOpDownModeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setVlanTagOpAssociaTypeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setVlanTagOpAssociaPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int addGemPortMappingRuleByVlanFilter(IN uint16 currInstId,  IN uint8 * vlanList, IN uint8 maxValidNum, IN uint8 forwardOperation);



/*******************************************************************************************************************************
9.3.13 extended vlan tagging operation configuration data

********************************************************************************************************************************/
int omciMeInitForVlanTagExtendOperation(omciManageEntity_t *omciManageEntity_p);
int omciExtVlanTagDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int setExtVlanTagOpAssociaTypeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getExtVlanTagOpMaxNumValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setExtVlanTagOpInputTPIDValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setExtVlanTagOpOutputTPIDValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setExtVlanTagOpDownModeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getExtVlanTagOpTblValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setExtVlanTagOpTblValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setExtVlanTagOpAssociaPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setExtVlanTagOpDSCPMappingValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);


/*extern */
extern omciAttriDescript_t omciAttriDescriptList8021pMapperProfile[];
extern omciAttriDescript_t omciAttriDescriptListVlanTagFilterData[];
extern omciAttriDescript_t omciAttriDescriptListVlanTagOperation[];
extern omciAttriDescript_t omciAttriDescriptListVlanTagExtendOperation[];

#endif
