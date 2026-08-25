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
	ctc/oam_ext_cmn.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/
#ifndef OAM_COMMON_H
#define OAM_COMMON_H
#include <stdio.h>
#include "epon_oam_types.h"
#include "ctc/oam_ext_buff.h"


/* OAM DEBUG LEVEL DEFINE */
#define DBG_OAM_L0	0     /* Info Must be displayed */
#define DBG_OAM_L1	1     /* Error Info */
#define DBG_OAM_L2	2	/* Recv OAM PDU Info */
#define DBG_OAM_L3	3	/* Reply OAM PDU Info */
#define DBG_OAM_L4 4     /* Code Trace */

/* OAM CODE DEFINE */
#define OAM_CODE_ORG_EXT     	0xFE

/* OAM INFORMATION DEFINE */
#define OAM_INFO_ORG_SPEC    	0xFE

/* OAM EVENT DEFINE */
#define OAM_EVENT_ORG_SPEC   	0xFE

#define OUI_LEN		3
#define OAM_TLV_MAX_VALUE_LEN    1496

#define PTR_MOVE(ptr, len) ((ptr) += (len))
#define MAX(a, b)          (((a)>(b))?(a):(b))
#define MIN(a, b)          (((a)>(b))?(b):(a))

#pragma pack(push, 1)
/* ETHERNET OAM HEADER */
typedef struct pOamHdr_s
{
	u_char  subtype;
	u_short flags;
	u_char  code;
}OamHdr_t, *OamHdr_Ptr;

/* OAM CTC Ext discovery information TLV */
typedef struct oamCtcInfoHdr_s
{
	u_char	type;
	u_char	length;
	u_char	oui[OUI_LEN];
	u_char	extSupport;
	u_char	version;
}OamCtcInfoHdr_t, *OamCtcInfoHdr_Ptr;

typedef struct oamCtcOuiVersion_s
{
	u_char	oui[OUI_LEN];
	u_char	version;
}OamCtcOuiVersion_t, *OamCtcOuiVersion_Ptr;


/* OAM Ext Event TLV: Orgnization Specific Event TLV
 * CTC Alarm Info TLV
 */
typedef struct oamOrgEventTlv_s
{
	u_char	type;	// 0xFE
	u_char	length;
	u_char	oui[OUI_LEN];
	u_short	objectType;
	u_int		instaneNum;	// if object is ONU, = 0xFFFFFFFF
	u_short	alarmID;
	u_short	timeStamp;	// ONU's local time
	u_char	alarmState;	//0x01 = alarm; 0x00 = clear alarm
//	u_char	*alarmInfo;
}OamOrgEventTlv_t, *OamOrgEventTlv_Ptr;


/*	Organization Specific OAM PDU Data Header	*/
typedef struct oamOrgHdr_s
{
	u_char	oui[OUI_LEN];
	u_char	opCode;
}OamOrgHdr_t, *OamOrgHdr_Ptr;

#pragma pack(pop)

#define GetData(type, pData)  (*((type*)(pData)))
#define GetU8(pData) ((*((u_char*)pData)))
#define GetU16(pData) (ntohs(*((u_short*)(pData))))
#define GetU32(pData) (ntohl(*((u_int*)(pData))))

#ifdef CONFIG_DEBUG
extern int eponOamCtcDbgLvl;
#define eponOamExtDbg(L, fmt, args...) {if (eponOamCtcDbgLvl >=L) tcdbg_printf(fmt, ##args);}
void eponOamDumpHexString(int level, u_char *pData, int len);
#else
#define eponOamDumpHexString(L, pData, len) {}
#define eponOamExtDbg(L, fmt, args...) {}
#endif

#define OamTrace(fmt, args...)  eponOamExtDbg(DBG_OAM_L4, fmt, ##args)

#define OAM_FUNC_TRACE_ENTER \
    OamTrace("===Enter Function: %s===\n",  __FUNCTION__)

#define OAM_FUNC_TRACE_LEAVE \
    OamTrace("===Leave Function: %s===\n",  __FUNCTION__)


void setU8(u_char *dp, u_char value);
void setU16(u_char *dp, u_short value);
void setU24(u_char *dp, u_char *value);
void setU32(u_char *dp, u_int value);

int match(u_char *dp1, u_char *dp2, int length);
int lowAlignStrcpy(char *to, char *from, int lenTo);


int eponOamCtcGetDbgLevel(void);
void eponOamCtcSetDbgLevel(int lvl);

int isSameOui(const u_char *oui1, const u_char *oui2);

void handleBuffNotEnough(INOUT Buff_Ptr bfp, IN int revLen);


#endif
