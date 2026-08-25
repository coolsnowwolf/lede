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
	omci_me_gemport_mapping.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wayne.lee	2012/7/28	Create
*/

#ifndef  _OMCI_GEM_PORT_MAPPING_H_
#define _OMCI_GEM_PORT_MAPPING_H_
#include "omci_general_func.h"
//#include "omci_types.h"
//#include "omci_me.h"
#ifndef TCSUPPORT_GPON_MAPPING //gem port API
enum gponmap_status {
	GPONMAP_SUCCESS=0,
	GPONMAP_FAIL=1,
	GPONMAP_ENTRY_NOT_FOUND=2,
	GPONMAP_ENTRY_EXIST=3
};
/*
**********************************************************************************
gpon traffic class to GEM port mapping entry ioctl data structure.
**********************************************************************************
*/
typedef struct gemPortMappingIoctl_s{
/*tagctl: if the bit value is 1, then this bit function is valid in this rule.
	bit		description
	0		tag flag, 0:untagged, 1:tagged
	1		user port
	2		ani port
	3		vid
	4		dscp
	5		pbit
	6		gem port
	7~15	reserved
*/	

	uint16 tagCtl;
	uint8 tagFlag;//0:untagged,1:tagged
	uint8 userPort;//0xff: this value indicates all user ports.
	uint16 aniPort;	
	uint16 vid;
	uint8 dscp;
	uint8 pbit;
	uint16 gemPort;	
}gemPortMappingIoctl_t, *gemPortMappingIoctl_ptr;
/* -----------------------------OUT Interfaces----------------------------- */
int addGemPortMappingRule(gemPortMappingIoctl_t *tableEntry);
int delGemPortMappingRule(gemPortMappingIoctl_t *tableEntry);
int displayAllGemPortMappingRule(void);
#endif


//#define OMCI_GEM_PORT_MAPPING_DEBUG  1

#define MAX_GEM_PORT_MAPPING_RULE_NUM	8192 //max rule num in gem port mapping 

/*9.3.10 */
#define OMCI_GEMPORT_MAPPING_UNTAGGED_FLAG		0
#define OMCI_GEMPORT_MAPPING_TAGGED_FLAG		1

#define MAX_OMCI_DSCP_PBIT_GPORT_ITEM_LEN		4
#define MAX_OMCI_DSCP_PBIT_GPORT_LEN				256

#define MAX_OMCI_PBIT_GPORT_ITEM_LEN			3
#define MAX_OMCI_PBIT_GPORT_LEN					3

#define MAX_OMCI_DSCP_KINDS						64
#define MAX_OMCI_PBIT_NUM							8	



#define OMCI_GEMPORT_MAPPING_TAGFLAG			(1<<0)
#define OMCI_GEMPORT_MAPPING_USERPORT			(1<<1)
#define OMCI_GEMPORT_MAPPING_ANI_PORT			(1<<2)
#define OMCI_GEMPORT_MAPPING_VID					(1<<3)
#define OMCI_GEMPORT_MAPPING_DSCP				(1<<4)
#define OMCI_GEMPORT_MAPPING_PBIT				(1<<5)
#define OMCI_GEMPORT_MAPPING_GEMPORT 			(1<<6)
#define OMCI_GEMPORT_MAPPING_CLASSID_INSTID		(1<<7)
#define OMCI_GEMPORT_MAPPING_TAGCTL				(1<<8)

#define MAX_GEMPORT_VALID_MEID_NUM				3

#define OMCI_GEMPORT_RULE_FREE_FLAG1 			1
#define OMCI_GEMPORT_RULE_FREE_FLAG0 			0

typedef struct omciGemPortMapping_s{
	struct omciGemPortMapping_s *next;
	uint16 index;
/*tagctl: if the bit value is 1, then this bit function is valid in this rule.
	bit		description
	0		tag flag, 0:untagged, 1:tagged
	1		user port
	2		ani port
	3		vid
	4		dscp
	5		pbit
	6		gem port
	7		ME id (class id+instance id)
	8~15	reserved
*/	
	uint16 tagctl;
	uint8 tagFlag;//0:untagged,1:tagged
	uint8 userPort;//0xff: this value indicates all user ports.
	uint16 aniPort;	
	uint16 vid;
	uint8 dscp;
	uint8 pbit;
	uint16 gemPort;	
	uint32 classIdInstId;//classid+instance id
}omciGemPortMapping_t, *omciGemPortMapping_ptr;

/*******************************************************************************************************************************
general function

********************************************************************************************************************************/
int convertGemPortRule(IN omciGemPortMapping_ptr srcRule_ptr, OUT gemPortMappingIoctl_ptr destRule_ptr);
int addConvertedGemPortRule(IN omciGemPortMapping_ptr  rule_ptr);
int delConvertedGemPortRule(IN omciGemPortMapping_ptr  rule_ptr);
int findOmciGemPortMappingRule(omciGemPortMapping_ptr rule_ptr, uint16 * index);
int modifyOmciGemPortMappingRule(uint16 modifyFlag, omciGemPortMapping_ptr oldRule_ptr, omciGemPortMapping_ptr newRule_ptr);
int getOmciGemPortMappingRuleMaxIndex(uint16 * maxIndex);
int addOmciGemPortMappingRule(omciGemPortMapping_ptr rule_ptr);
int addOmciGemPortMappingRuleByUni(uint8 userPort, uint32 classIdInstId, uint32 *meIdArr, uint8 validNum);
int delOmciGemPortMappingRuleByMatchFlag(uint16 delMatchFlag, omciGemPortMapping_ptr currMappingRule);
//int delGemPortMappingRule(uint16 index);	
//int clearGemPortMappingRuleByInstId(uint32 classIdInstId);
int modifyOmciGemPortMappingRuleByUnmarkOpt(uint8 flag, uint16 length, uint8 *val, uint32 classIdInstId);
void displayOmciCurrGemPortMappingRule();
void clearOmciGemportRule(void);

extern omciGemPortMapping_ptr gemPortMapping_ptr;
#endif
