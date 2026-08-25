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
	omci_types.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/6/28	Create
*/



#ifndef _OMCI_TYPES_H_
#define _OMCI_TYPES_H_

#include <xpon_global/private/xpon_if.h>

/**debug level ***/
#define OMCI_DEBUG_LEVEL_NO_MSG		0
#define OMCI_DEBUG_LEVEL_ERROR		1
#define OMCI_DEBUG_LEVEL_WARN		2
#define OMCI_DEBUG_LEVEL_TRACE		3
#define OMCI_DEBUG_LEVEL_DEBUG		4


/* ME attribute Access type */
#define ATTR_ACCESS_R						1
#define ATTR_ACCESS_W					2
#define ATTR_ACCESS_R_W						3
#define ATTR_ACCESS_R_SET_CREATE	   5	
#define ATTR_ACCESS_W_SET_CREATE	   6		
#define ATTR_ACCESS_R_W_SET_CREATE		7	

/*attribute format */
#define ATTR_FORMAT_POINTER    1
#define ATTR_FORMAT_BIT_FIELD    2
#define ATTR_FORMAT_SIGNED_INT    3
#define ATTR_FORMAT_UNSIGNED_INT    4	
#define ATTR_FORMAT_STRING    5
#define ATTR_FORMAT_ENUM    6
#define ATTR_FORMAT_TABLE    7


/*attribute supported define */
#define ATTR_FULLY_SUPPORT    1
#define ATTR_UNSUPPORT    2
#define ATTR_PART_SUPPORT    3
#define ATTR_IGNORED    4
 

#define OMCI_CREATE_ACTION_SET_VAL		0
#define OMCI_SET_ACTION_SET_VAL			1
#define OMCI_SET_TABLE_ACTION_SET_VAL	2



#define NO_ALARM	0x00
#define NEED_ALARM	0x01


#define MT_OMCI_ALARM_BITMAP_LENGTH		28

#define OMCI_INTERNAL_END_TALBE	255
	
#define OMCI_ME_DEFAULT_ATTR_LEN	24
/*9.3.11*/
#define OMCI_PON_INTERFACE_PREFIX_STR	"nas"
#define OMCI_LAN_INTERFACE_PREFIX_STR	"eth0."
/*9.3.27*/
#define OMCI_LAN_PORT_NUM	4

/*9.3.10*/
#define MAX_OMCI_PBIT_NUM	8 //8 kinds pbit

/*9.3.4*/
#define OMCI_IN_BOUND			1
#define OMCI_OUT_BOUND			0

#define MAX_OMCI_ANI_PORT_NUM	GPON_MAX_ANI_INTERFACE
#define MAX_OMCI_ANI_PORT_VALID_NUM	2
#define OMCI_ANI_PORT_INST_ID_INDEX	0
#define OMCI_ANI_PORT_VALID_INDEX	1
#define OMCI_PONMGR_CMD	"/userfs/bin/ponmgr "
#define OMCI_BRCTL_CMD	"/usr/bin/brctl "
#define OMCI_IFCONFIG_CMD	"/sbin/ifconfig "

#if 0
#define INIT_GPON_IFC					"initGponIfc"
#define INIT_GPON_IFC_ENABLE			1
#define INIT_GPON_IFC_DISABLE			0
#endif
#define GPON_IFC						"interface"
#define GPON_GEMPORT_LIST				"gemportlist"
#define GPON_HANDLE_PON_IFC				"handleifc" //0:default, 1:create, 2:del, 3:modify, 4:unassign gem port
#define GPON_OMCI_ENTRY					"OMCI_Entry"
#define GPON_HANDLE_PON_IFC_DEF			0 //default
#define GPON_HANDLE_PON_IFC_CREATE		1
#define GPON_HANDLE_PON_IFC_DEL			2
#define GPON_HANDLE_PON_IFC_MODIFY		3
#define GPON_HANDLE_PON_IFC_UNASSIGN	4 //unassign gemport in this inteface

#define GPON_GEMPORT_LIST_SEP			'_'

#define GPON_QOS_CFG 				"QOSCfg"
#define GPON_CONGEST_CFG 			"CongestCfg"
#define GPON_TRTCM_CFG 				"TrtcmCfg"

#define PON_QOS						"QOS" 
#define PON_CONGEST					"Congest"
#define PON_TRTCM_PARAMS			"TrtcmParam" 

#define PON_QOS_MASK				"QOSMask" 
#define PON_CONGEST_MASK			"CongestMask"
#define PON_TRTCM_PARAMS_MASK		"TrtcmMask" 

#define PON_QOS_TYPE				"QOSType"
#define PON_QOS_TYPE_DEFAULT_VALUE	'X'

#define ATTR_QOS_WEIGHT_TYPE		"WeightType"
#define ATTR_QOS_WEIGHT_SCALE		"WeightScale"
#define ATTR_QOS_VALUE				"QOSValue"

#define ATTR_CONGEST_TRTCM_MODE		"CongestTrtcmMode"
#define ATTR_CONGEST_DEI_DROP_MODE	"CongestDeiDropMode"
#define ATTR_CONGEST_THRED_MODE		"CongestThredMode"
#define ATTR_CONGEST_SCALE			"CongestScale"
#define ATTR_CONGEST_DROP_P			"CongestDropProbability"
#define ATTR_CONGEST_THRED_VALUE	"CongestThredValue"

#define ATTR_TRTCM_SCALE			"TrtcmScale"
#define ATTR_TRTCM_VALUE			"TrtcmValue"


#define GPON_HANDLE_DEF			0 //default
#define GPON_HANDLE_SET			1 

#define MASK_DEF			0 //default

#define MASK_QOS_WEIGHT_PARAM 		(1<<0) 
#define MASK_QOS_CONFIG				(1<<1) 

#define MASK_TRTCM_SCALE 		(1<<0) 
#define MASK_TRTCM_PARAMETERS			(1<<1) 

#define MASK_CONGEST_TRTCM_MODE 		(1<<0) 
#define MASK_CONGEST_DEI_DROP_MODE		(1<<1) 
#define MASK_CONGEST_THRED_MODE 		(1<<2) 
#define MASK_CONGEST_SCALE				(1<<3) 
#define MASK_CONGEST_DROP_PROBILITY		(1<<4) 
#define MASK_CONGEST_THRED		(1<<5) 


enum omciMacBridgePort_status{
	OMCI_MAC_BRIDGE_PORT_FAIL= -1,
	OMCI_MAC_BRIDGE_PORT_SUCCESS=0,		
	OMCI_MAC_BRIDGE_PORT_ENTRY_NOT_FOUND=2,
	OMCI_MAC_BRIDGE_PORT_ENTRY_EXIST=3
};


#define OMCI_MAC_BRIDGE_PORT_PON_PORT_BASIC 13
#define OMCI_MAC_BRIDGE_PORT_LAN_PORT_BASIC 1



#if 1//def LINUX
// Put platform dependent declaration here
// For example, linux type definition
#ifndef _int8_defined
#define _int8_defined
typedef signed char 			int8;
#endif

#ifndef _uint8_defined
#define _uint8_defined
typedef unsigned char			uint8;
#endif

#ifndef _int16_defined
#define _int16_defined
typedef signed short 			int16;
#endif
#ifndef _uint16_defined
#define _uint16_defined
typedef unsigned short			uint16;
#endif
#ifndef _int32_defined
#define _int32_defined
typedef int				int32;
#endif
#ifndef _uint32_defined
#define _uint32_defined
typedef unsigned int			uint32;
#endif


#ifndef _int64_defined
#define _int64_defined
typedef long long 			int64;

#endif

#ifndef _uint64_defined
#define _uint64_defined
typedef unsigned long long		uint64;
#endif

typedef unsigned char			uchar;
typedef unsigned short			ushort;
typedef unsigned int			uint;
typedef unsigned long			ulong;
#endif // LINUX //

/* OMCI device ID define */
#define MT_OMCI_DEVICE_ID_BASELINE	0x0A
#define MT_OMCI_DEVICE_ID_EXTENDED	0x0B

/*OMCI ME access define*/
#define OMCI_ME_ACCESS_ONU	1
#define OMCI_ME_ACCESS_OLT	2
#define OMCI_ME_ACCESS_OLT_ONU	3

/* OMCI ME support define */
#define OMCI_ME_SUPPORT		1
#define OMCI_ME_UNSUPPORT	2
#define OMCI_ME_PARTIAL_SUPPORT	3
#define OMCI_ME_IGNORED		4

/* result and reason code */
#define OMCI_CMD_SUCCESS							0x00
#define OMCI_CMD_ERROR								0x01
#define OMCI_CMD_NOT_SUPPORT							0x02
#define OMCI_PARAM_ERROR							0x03
#define OMCI_UNKNOW_ME								0x04
#define OMCI_UNKNOW_ME_INST							0x05
#define OMCI_DEVICE_BUSY							0x06
#define OMCI_INST_EXIST								0x07
#define OMCI_ATTR_FAIL_OR_UNKNOW						0x09


/* OMCI message source indicator */
#define EXTERNAL_MSG						0x0000	
/* FROM OLT */
#define INTERNAL_MSG						0x0001    
/* FROM ONU */


#define ETH_P_IP  0x0800

#define OMCI_MODE_GPON  0
#define OMCI_MODE_EPON  1


#define IN
#define OUT
#define IN_OUT

#define TRUE 1
#define FALSE 0

#define SFU 1
#define HGU 2

#define	COMMITTED		1
#define	UNCOMMITTED	0
#define	ACTIVE			1
#define	INACTIVE		0
#define	VALID			1
#define	INVALID			0
 
#define OMCI_ATTR_ADMIN_STATE		("admin state")


#define OMCI_ERROR(args...) omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, args)
#define OMCI_WARN(args...)  omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN,  args)
#define OMCI_TRACE(args...) omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, args)
#define OMCI_DEBUG(args...) omcidbgPrintf(OMCI_DEBUG_LEVEL_DEBUG, args)


#define OMCI_FUNC_TRACE_ENTER \
    OMCI_TRACE("===Enter Function: %s===\n",  __FUNCTION__)

#define OMCI_FUNC_TRACE_LEAVE \
    OMCI_TRACE("===Leave Function: %s===\n",  __FUNCTION__)

#ifndef TCAPI_SUCCESS
#define TCAPI_SUCCESS (0)
#endif


#define 		CODE_POINT_END			0xffff
#endif // _OMCI_TYPES_H_




