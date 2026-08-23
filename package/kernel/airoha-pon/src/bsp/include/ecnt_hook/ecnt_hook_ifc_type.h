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
#ifndef _ECNT_HOOK_IFC_TYPE_H_
#define _ECNT_HOOK_IFC_TYPE_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
	
/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/

typedef enum {
    ECNT_IFC_API,
}IFC_Api_SubType_t;

typedef enum {
	IFC_DISABLE = 0 ,
	IFC_ENABLE
} IFC_Mode_t ;

typedef enum {
	IFC_SET_GLOBAL_ENABLE = 0,
	IFC_GET_GLOBAL_ENABLE,
	IFC_SET_LUT_RULE_AUTO,
	IFC_DEL_LUT_RULE_AUTO,
	IFC_SET_LUT0_RULE,
	IFC_SET_LUT1_RULE,
	IFC_SET_ACTION,
	IFC_SET_VIP,
	IFC_DEL_VIP,
	IFC_SET_GENERAL,
	IFC_DEL_GENERAL,
	IFC_UDF_RULE,
	IFC_GET_IDX,
	IFC_SET_RULE,
	IFC_SET_ACT,
	IFC_SET_MIB,
	IFC_GET_STAT,
	IFC_SET_QDMA_FTP,
	IFC_GET_HIT_IDX_ACTION,
	IFC_FUNCTION_MAX_NUM,
} IFC_HookFunctionID_t ;

typedef enum{
	IFC_PROTO_UDP= 0,
	IFC_PROTO_TCP,
	IFC_PROTO_ALL,
}IFC_PROTO;

struct ecnt_ifc_param {
    unsigned int field[48];
    unsigned int command[48];
    unsigned int mask[48];
    unsigned int mask_high[48];
    unsigned int key[48];
    unsigned int key_high[48];
};

struct ecnt_mib_cnt {
	unsigned int byte_low;
	unsigned int byte_high;
	unsigned int pkt_low;
	unsigned int pkt_high;
};

struct ecnt_ifc_data {
	unsigned char  ifcEnable;
	IFC_HookFunctionID_t  function_id;
	unsigned int  ifcIndex;
	unsigned int  actIdx;
	unsigned int  enMode;
	unsigned int  endFlag;
	unsigned int  field;
	unsigned int  command;
	unsigned int  mask_low;
    unsigned int  mask_high;
    unsigned int  key_low;
    unsigned int  key_high;
    unsigned int  value0;
    unsigned int  value1;
    unsigned int  value2;
    unsigned int  value3;
	struct ecnt_ifc_param ifc_param;
	union{
		struct{
			unsigned char fport;
			unsigned int nbq;
			unsigned int channel;
			unsigned int queue;
			unsigned int gemport;
			unsigned int fqos;
			unsigned int vlan;
		}tls_dmac_data;
		struct{
			unsigned char tcp_type;
			unsigned int tcp_dip;
		}tcp_data;
	}api_data;
	int   retValue;
};

enum IFC_FIELDS {
    DMAC = 1,
    SMAC,
    DIPv4,
    SIPv4,
    DIPv6_31_0,
    DIPv6_63_32,
    DIPv6_95_64,
    DIPv6_127_96,
    SIPv6_31_0,
    SIPv6_63_32,
    SIPv6_95_64,
    SIPv6_127_96,
    
    FLOW_LABEL,	//13
	SINF,
	FRAME_TYPE,
	VPM0,
	VPM1,
	
	VID0,		//18
	PBIT0,
	VID1,
	PBIT1,
	
	ETYPE,
	GEMPORT,
	
    IPv4_PROTOCOL,	//24
    IPv6_NEXT_HEADER,
	IPv4_DSCP,
	IPv6_TRAFFIC_CLASS,
	
    DPORT,
    SPORT,
    
    PPPOE_SESSION,
	DMAC_TYPE,
    
	UDF0 = 32,
	UDF15 = 47,

	TCP_FLAGS,
	L4_PROTOCOL_VLD,
	L4_PROTOCOL,
	L4_PORT_VLD,
	
	OUTTER_IPv4,
	OUTTER_IPv6,
	IPv4_VLD,
	IPv6_VLD,
	
	LLC_OTHER,
	IS_PPPOE,
	VLAN0_VLD,
	VLAN1_VLD,
	LUT1_HIT,
	SIP_EQ_DIP,
	SPORT_EQ_DPORT,
	DIP_EQ_ATK,
	SIP_EQ_ATK,
	RTSP_CTRL,
	SIP6_EQ_DIP6,
	SA_ID,
	FIELD_IDX_MAX
};

enum IFC_ACTIONS {
	ACT_CHN = 0,
	ACT_ForcePort,
	ACT_SP_TAG,
	ACT_MeterGRP,
	ACT_AcntGRP,
	ACT_DSCP,
	ACT_TrafficClass,
	ACT_VLAN0,
	ACT_VLAN1,
	ACT_FQOS,
	ACT_FAST,
	ACT_MultiCast,
	ACT_KPCP,
	ACT_DPI,
	ACT_ForceCPU,
	ACT_MIB,
	ACT_RateLimit,
	ACT_RateLimit_ONLY,
	ACT_OWF,
	ACTION_IDX_MAX
};

enum IFC_COMMAND {
	IFC_COMMAND_EQUAL = 0,
	IFC_COMMAND_NOT_EQUAL,
	IFC_COMMAND_IN_RANGE,
	IFC_COMMAND_NOT_IN_RANGE,
};

typedef enum
{
	IFC_VIP_FIELD_ETYPE = 0,
	IFC_VIP_FIELD_IPV4_PROTO,
	IFC_VIP_FIELD_IPV6_NXTHDR,
	IFC_VIP_FIELD_PORT,
	IFC_VIP_FIELD_SIP_SPORT,
	IFC_VIP_FIELD_DIP_DPORT,
	IFC_VIP_FIELD_PBIT,
	IFC_VIP_FIELD_DSCP,
	IFC_VIP_FIELD_TRAFFIC_CLASS,
}IFC_VIP_FIELD;

typedef enum
{
	IFC_GENERAL_FIELD_IPV4_PROTO,
	IFC_GENERAL_FIELD_DMAC,
	IFC_GENERAL_FIELD_IGMPTYPE,
}IFC_GENERAL_FIELD;


typedef enum 
{
	PPPOE_HEADER_LENGTH_TRANSLATE = 1,   
}HIT_IFC_IDX_ACTION;
/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#endif /* _ECNT_HOOK_IFC_TYPE_H_ */

