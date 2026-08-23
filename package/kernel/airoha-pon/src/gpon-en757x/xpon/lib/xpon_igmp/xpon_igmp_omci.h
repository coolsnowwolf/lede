/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
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
	xpon_igmp_omci.h
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name				Date			Modification logs
	lidong.hu		2012/7/28	Create
*/
#ifndef _XPON_IGMP_OMCI_H_
#define _XPON_IGMP_OMCI_H_


#define uint8  unsigned char 
#define uint16 unsigned short
#define uint32 unsigned int

#define NULL ((void*)0)

////////////////////////////////////////////////////////////////////////////////////
typedef struct igmpRuleInfo_s
{
	uint16 index;
	uint16 type;
	uint16 gem;
	uint16 vid;
	uint8 srcip[16];
	uint8 startip[16];
	uint8 endip[16];
	uint32 band;
}igmpRuleInfo_t, *igmpRuleInfo_Ptr;

typedef struct igmpLostGroupInfo_s
{
	uint16 vid;
	uint8 srcip[16];
	uint8 grpip[16];
}igmpLostGroupInfo_t, *igmpLostGroupInfo_Ptr;


typedef struct igmpActiveGroupInfo_s
{
	uint8 srcip[16];
	uint8 grpip[16];
	uint32 bandwidth;
	uint8 clientip[16];
	uint32 times;
	uint16 vid;
	uint16 reserve;
}igmpActiveGroupInfo_t, *igmpActiveGroupInfo_Ptr;

////////////////////////////////////////////////////////////////////////////////////////
int igmpONUGetVer(uint16 port);

int igmpONUSetVer (uint16 port,uint8 ver);

int igmpONUGetFunc(uint16 port);

int igmpONUSetFunc(uint16 port,uint8 func);

int igmpONUGetFastLeave(uint16 port);

int igmpONUSetFastLeave(uint16 port,uint8 fast);

int igmpONUGetUpstreamTCI(uint16 port);

int igmpONUSetUpstreamTCI(uint16 port,uint16 tci);

int igmpONUGetTagCtrl(uint16 port);

int igmpONUSetTagCtrl (uint16 port,uint8 tag);

int igmpONUGetMaxRate (uint16 por,uint32* rate);
 
int igmpONUSetMaxRate (uint16 port,uint8 rate);
 
int igmpONUClearMaxRate(uint16 port);

int igmpONUGetDyList(uint16 port,uint16 maxlen,uint8 *ruleNum,uint8* val );

int igmpONUSetDyCtrList(uint16 port, igmpRuleInfo_t* rule);

int igmpONUDelDyCtrList(uint16 port, uint16 rowKey);

int igmpONUClearDyCtrList(uint16 port);

int igmpONUGetStaticList(uint16 port,uint16 maxlen,uint8 *ruleNum,uint8* val );

int igmpONUSetStaticCtrList(uint16 port, igmpRuleInfo_t* rule);

int igmpONUDelStaticCtrList(uint16 port, uint16 rowKey);

int igmpONUClearStaticCtrList(uint16 port);

int igmpONUGetLostList(uint16 port,uint16 maxlen,uint8 *ruleNum,uint8* val );

int igmpGetRobustness(uint16 port,uint8* robust);

int igmpSetRobustness(uint16 port,uint8 robust);

int igmpGetQueryIp(uint16 port,uint8* ip);

int igmpSetQueryIp(uint16 port,uint8* ip);

int igmpGetQueryInterval(uint16 port,uint32* interval);

int igmpSetQueryInterval(uint16 port,uint32 interval);

int igmpGetQueryMaxRespTime(uint16 port,uint32* respTime);

int igmpSetQueryMaxRespTime(uint16 port,uint32 respTime);

int igmpGetLastQueryInterval(uint16 port,uint32* interval);

int igmpSetLastQueryInterval(uint16 port,uint32 interval);

int igmpGetUnauthorized(uint16 port,uint8* author);

int igmpSetUnauthorized(uint16 port,uint8 author);

int igmpGetDownTCIType(uint16 port, uint8* tyoe,uint16* tci);

int igmpSetDownTCIType(uint16 port, uint8 tyoe,uint16 tci);

int igmpONUGetActiveGroupList(uint16 port,uint16 maxlen,uint8 type,uint8 *ruleNum,igmpActiveGroupInfo_t* rulePtr );

int igmpONUSetGponMode(void);

int igmpONUGetMaxBW(uint16 port,uint32* bw);
int igmpONUSetMaxBW(uint16 port,uint32 bw);
int igmpONUGetBWEnforcement(uint16 port,uint8* bwe);
int igmpONUSetMaxEnforcement(uint16 port,uint8 bwe);
//////////////////////////////////////////////////////////////////////////////////////////
#endif
