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
	ponvlancmd.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Reid.Ma		2013/2/4	Create
*/

#ifndef _PONVLANCMD_H_
#define _PONVLANCMD_H_

#define PONVLAN_IOC_MAGIC 'j'
#define PONVLAN_MAJOR 215
#define PONVLAN_IOC_SWITCH_OPT _IOR(PONVLAN_IOC_MAGIC,0,unsigned long)
#define PONVLAN_IOC_DOWNSTREAM_MODE_OPT _IOR(PONVLAN_IOC_MAGIC,1,unsigned long)
#define PONVLAN_IOC_TPID_OPT _IOR(PONVLAN_IOC_MAGIC,2,unsigned long)
#define PONVLAN_IOC_DEFAULT_RULE_FLAG_OPT _IOR(PONVLAN_IOC_MAGIC,3,unsigned long)
#define PONVLAN_IOC_RULE_OPT _IOR(PONVLAN_IOC_MAGIC,4,unsigned long)
#define PONVLAN_IOC_DSCP_MAP_OPT _IOR(PONVLAN_IOC_MAGIC,5,unsigned long)

#define PONVLAN_IOC_IGMP_VLAN_SWITCH_OPT _IOR(PONVLAN_IOC_MAGIC,6,unsigned long)
#define PONVLAN_IOC_IGMP_VLAN_MODE_OPT _IOR(PONVLAN_IOC_MAGIC,7,unsigned long)
#define PONVLAN_IOC_IGMP_VLAN_TCI_OPT _IOR(PONVLAN_IOC_MAGIC,8,unsigned long)

#define PONVLAN_IOC_IF_VLAN_SWITCH_OPT _IOR(PONVLAN_IOC_MAGIC,9,unsigned long)
#define PONVLAN_IOC_IF_VLAN_RULE_OPT _IOR(PONVLAN_IOC_MAGIC,10,unsigned long)

#define PONVLAN_IOC_DBG_LEVEL_OPT _IOR(PONVLAN_IOC_MAGIC,11,unsigned long)

#define PONVLAN_IOC_PCP_MODE_OPT _IOR(PONVLAN_IOC_MAGIC,12,unsigned long)

#define PONVLAN_IOC_MAC_BIND_VID_SWITCH_OPT _IOR(PONVLAN_IOC_MAGIC,13,unsigned long)
#define PONVLAN_IOC_MAC_BIND_VID_TIME_OPT _IOR(PONVLAN_IOC_MAGIC,14,unsigned long)

#define PONVLAN_IOC_USER_GROUP_SWITCH_OPT _IOR(PONVLAN_IOC_MAGIC,15,unsigned long)
#define PONVLAN_IOC_USER_GROUP_RULE_OPT _IOR(PONVLAN_IOC_MAGIC,16,unsigned long)

#ifdef TCSUPPORT_PON_VLAN_FILTER
/*vlan filter*/
#define PONVLAN_IOC_ADD_VLAN_FILTER_ENTRY _IOR(PONVLAN_IOC_MAGIC,20,unsigned long)
#define PONVLAN_IOC_GET_VLAN_FILTER_ENTRY _IOR(PONVLAN_IOC_MAGIC,21,unsigned long)
#define PONVLAN_IOC_DEL_VLAN_FILTER_ENTRY _IOR(PONVLAN_IOC_MAGIC,22,unsigned long)
#define PONVLAN_IOC_DISP_VLAN_FILTER_ENTRY _IOR(PONVLAN_IOC_MAGIC,23,unsigned long) //display all vlan filter rule in list
#define PONVLAN_IOC_VLAN_FILTER_DBG_LEVEL _IOR(PONVLAN_IOC_MAGIC,24,unsigned long)
#endif
#define PONVLAN_IOC_VEIP_SWITCH_OPT _IOR(PONVLAN_IOC_MAGIC,25,unsigned long)


#define __u8 unsigned char
#define __u16 unsigned short
#define __u32 unsigned int

#define OPT_GET 0
#define OPT_SET 1
#define OPT_DEL 2
#define OPT_CLEAN 3
#define OPT_CLEAN_ALL 4
#define OPT_SHOW 5


#define ENABLE 1
#define DISABLE 0

#define UPSTREAM 1
#define DOWNSTREAM 2

#define ADD_FIRST_TAG 1
#define ADD_SECOND_TAG 2

#define FILTER_FIRST_TAG 1
#define FILTER_SECONE_TAG 2

#define CHANGE_INNER_TAG 0
#define CHANGE_OUTER_TAG 1

#define MATCH_FIRST_TAG 1
#define MATCH_SECOND_TAG 2
#define MATCH_THIRD_TAG 3
#define MATCH_FOURTH_TAG 4

#define FILTER_TPID_DO_NOT_CARE 0
#define FILTER_TPID_8100 1
#define FILTER_TPID_EQUAL_TO_INPUT_TPID 2
#define FILTER_TPID_EQUAL_TO_OUTPUT_TPID 3

#define FILTER_PRI_DO_NOT_CARE 8

#define FILTER_DEI_DO_NOT_CARE 2

#define FILTER_VID_DO_NOT_CARE 4096

#define TREAT_TPID_8100 0
#define TREAT_TPID_OUTPUT_TPID 1
#define TREAT_TPID_COPY_FROM_INNER 2
#define TREAT_TPID_COPY_FROM_OUTER 3
#define TREAT_TPID_INPUT_TPID 4


#define TREAT_PRI_COPY_FROM_INNER 8
#define TREAT_PRI_COPY_FROM_OUTER 9
#define TREAT_PRI_BASED_ON_DSCP 10

#define TREAT_DEI_COPY_FROM_INNER 2
#define TREAT_DEI_COPY_FROM_OUTER 3

#define TREAT_VID_COPY_FROM_INNER 4096
#define TREAT_VID_COPY_FROM_OUTER 4097

#define METHOD_TRANSPARENT 0
#define METHOD_BLOCK 1
#define METHOD_ADD_TAG 2
#define METHOD_DEL_TAG 3
#define METHOD_CHANGE_TAG 4


typedef struct pon_vlan_rule_s
{
	//Only used in tag_num = 2,filter outer Tag info
	__u32 filter_outer_tpid;//0:don't care, 1:0x8100 2:input TPID (downstrean 3:output TPID)
	__u8 filter_outer_pri;//0~7:pbit value  8:don't care
	__u8 filter_outer_dei;//0,1:DEI value 2:don't care
	__u16 filter_outer_vid;//0~4095:VID 4096:don't care
	
	//used in tag_num = 1 or 2.If Tag Num = 2,this field is
	//filter inner tag Info.
	__u32 filter_inner_tpid;
	__u8 filter_inner_pri;
	__u8 filter_inner_dei;
	__u16 filter_inner_vid;

	//used in downstream direction and tagNum is 4
	__u32 down_filter_outer_tpid;
	__u8 down_filter_outer_pri;
	__u8 down_filter_outer_dei;
	__u16 down_filter_outer_vid;
	
	//used in downstream direction and tagNum is 3
	__u32 down_filter_inner_tpid;
	__u8 down_filter_inner_pri;
	__u8 down_filter_inner_dei;
	__u16 down_filter_inner_vid;
	
	__u8 tag_num;//0~5 	(downstream 3,4 mean tagNum 5:ignore this entry)
	__u16 filter_ethertype;//0x0000 :Don't care. Other value mean true Ethertype.0x0800 mean IPoE frames

	/*
	0:transparent
	1:block
	2x:x=1,2. Add x Tag(s) or 23:add a Tag and modify original outer tag(use add_sec field to store change info)
	3x:x=1,2. Del x Tag(s) or 33:del a Tag and modify original inner tag(use add_sec field to store change info)
	4x:x=0,1,2. 40:change inner Tag. 41:change outer Tag. 42:change two tags.
	treatment_method should be set 40 if you want change tag when packet only have one tag.
	If packet has 2 tags and you only want to change outer tag,you should config Add_Sec_XXX to support this func.
	*/
	__u8 treatment_method;

	//used when treatment_method = 2 and Add_Num = 1 or 2
	//this field indicate how to add first Tag.
	__u32 add_fst_tpid;//0:0x8100 1:output TPID 2:copy from inner tag 3:copy from outer tag	(downstream 4:input TPID)
	__u8 add_fst_pri;//0~7 pbit value . 8:copy from inner tag 9:copy from outer tag 10:derive pbit from DSCP
	__u8 add_fst_dei;//0,1:DEI value 2:copy from inner tag 3:copy from outer tag
	__u16 add_fst_vid;//0~4095 VID value 4096:copy from inner tag 4097:copy from outer tag

	//used when treatment_method = 2 and Add_Num = 2
	//this field indicate how to add second Tag.
	__u32 add_sec_tpid;
	__u8 add_sec_pri;
	__u8 add_sec_dei;
	__u16 add_sec_vid;
	
	__u32 rule_priority;//0:high 1:low
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
	__u8 igmp_enable;
	
	/*
	0: Pass upstream IGMP traffic transparently.
	1: Add tag.TPID=0x8100.TCI = igmp_tci.
	2: Replace TCI field with igmp_tci.If original frame is untagged.Add a tag,TPID = 0x8100 & tci = igmp_tci.
	3: Replace VID field with the VID field in igmp_tci.If original frame is untagged.Add a tag,TPID = 0x8100 & tci = igmp_tci.
	*/
	__u8 igmp_mode;
	__u16 igmp_tci;

	// 0:disable vlan tagging.
	// 1:enable vlan tagging.
	__u8 vlan_enable;
	__u8 veip_vlan_enable;
	__u8 rule_index;//vlan rule index.
	__u8 option_flag;//0:get 1:set 2:del 3: clean 4:show
	__u8 tpid_type;//flag for type of tpid.0:add special ethertype to linux kernel ethertype list. 1:set the input and output tpid in the rule
	__u16 special_tpid;//any value but the one has been used.
	__u16 input_tpid;
	__u16 output_tpid;
	__u32 dscp_map[6];//24byte map
	__u8 downstream_mode;//0:transparent 1:do inverse option 2:stripped outer tag
	__u8 default_vlan_rule_flag;//0:not use default vlan when no rule in this port. 1:use default when no rule in this port.

	__u8 if_vlan_switch;//0:disable 1:enable
	__u8 dev_name[9];//name is like nasx_y
	__u16 dev_vid;//allowed VID on Interface,VID = 4097 mean discard all packet, VID = 4096 mean forward all packet.default value is 4097.

	__u8 pcp_mode;//0:8P0D 1:7P1D   2:6P2D  3:5P3D

	__u8 mac_bind_vid_enable;
	int mac_vlan_time;

	__u8 user_group_enable;
	__u32 user_group;
	__u8 dbg_level;//0:low, only can see error message  1:medium,can see result of operation 2:high,can see lots of info when do the operation
	pon_vlan_rule rule;
}pon_vlan_ioctl, *pon_vlan_ioctl_p;

#ifdef TCSUPPORT_PON_VLAN_FILTER

#define GPON_VLAN_FILTR_PORT_TYPE_LAN	0
#define GPON_VLAN_FILTR_PORT_TYPE_ANI	1

#define GPON_VLAN_FILTR_TYPE_UNTAGGED	(1<<0)
#define GPON_VLAN_FILTR_TYPE_TAGGED		(1<<1)

#define GPON_VLAN_FILTER_ACTION_BRIDGE	0
#define GPON_VLAN_FILTER_ACTION_DISCARD	1
#define GPON_VLAN_FILTER_ACTION_G_VID	21
#define GPON_VLAN_FILTER_ACTION_G_PBIT	22
#define GPON_VLAN_FILTER_ACTION_G_TCI	23
#define GPON_VLAN_FILTER_ACTION_H_VID	31
#define GPON_VLAN_FILTER_ACTION_H_PBIT	32
#define GPON_VLAN_FILTER_ACTION_H_TCI	33
#define GPON_VLAN_FILTER_ACTION_J_VID		41
#define GPON_VLAN_FILTER_ACTION_J_PBIT	42
#define GPON_VLAN_FILTER_ACTION_J_TCI		43

#define MAX_GPON_VLAN_FILTER_LIST 		12
#define MAX_GPON_VLAN_FILTER_LIST_BYTES	24

#define	OMCI_MAC_BRIDGE_PORT_PON_PORT_BASIC 		13
#define 	OMCI_MAC_BRIDGE_PORT_LAN_PORT_BASIC		1
/*
**********************************************************************************
vlan filter  data structure.
**********************************************************************************
*/
typedef struct gponVlanFilterIoctl_s{
	__u16 port; //LAN port 0~3, ANI port: 0~31
	__u8 portType;//0:LAN port, 1:ANI port
	__u8 type; //bit0::set for untagged frame, bit1:set for tagged frame
	__u8 untaggedAction; //0:bridge, 1:discard
	/*
		0:bridge, 
		1:discard, 		
		21: when egress frame match vid in vlan list, then filter, others is forward.(Action g)
		22: when egress frame match pbit in vlan list, then filter, others is forward.(Action g)
		23: when egress frame match TCI in vlan list, then filter, others is forward.(Action g)		
		31: when ingress frame match vid in vlan list, then bridge, others is filter.(Action h)
		     when egress frame match vid in vlan list, then bridge, others is filter.
		32:when ingress frame match pbit in vlan list, then bridge, others is filter.(Action h)
		     when egress frame match pbit in vlan list, then bridge, others is filter.
		33:when ingress frame match TCI in vlan list, then bridge, others is filter.(Action h)
		     when egress frame match TCI in vlan list, then bridge, others is filter.
		41:when egress frame match vid in vlan list, then forward, others is filter.(Action j)
		42:when egress frame match pbit in vlan list, then forward, others is filter.(Action j)
		43:when egress frame match TCI in vlan list, then forward, others is filter.(Action j)
	*/
	__u8 taggedAction;
	__u8 maxValidVlanListNum; //max valid number in vlan list.
	__u8 cleanFlag;
	__u16 vlanList[MAX_GPON_VLAN_FILTER_LIST];
}gponVlanFilterIoctl_t, *gponVlanFilterIoctl_ptr;
#endif
#endif
