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
#ifndef _ECNT_HOOK_FE_VIP_H_
#define _ECNT_HOOK_FE_VIP_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include "ecnt_hook_ifc.h"
#include "ecnt_hook_fe.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/
typedef enum
{
	FE_VIP_PROTO_TCP = 0,
	FE_VIP_PROTO_UDP,
}FE_VIP_PROTO;

/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
static inline int FE_VIP_ADD_ETYPE(unsigned short ether_type)
{
	#if defined(TCSUPPORT_CPU_EN7580)
		return IFC_API_SET_VIP_BY_ETYPE(ether_type);
	#else
		return FE_API_ADD_VIP_ETHER(ether_type);
	#endif
}

static inline int FE_VIP_ADD_IPV4_PROTO(unsigned short proto)
{
	#if defined(TCSUPPORT_CPU_EN7580)
		return IFC_API_SET_VIP_BY_IPV4_PROTO(proto);
	#else
		return FE_API_ADD_VIP_IP(proto);
	#endif
}

static inline int FE_VIP_ADD_IPV6_NXTHDR(unsigned short nxthdr)
{	
	#if defined(TCSUPPORT_CPU_EN7580)
		return IFC_API_SET_VIP_BY_IPV6_NXTHDR(nxthdr);
	#else		
		return FE_API_ADD_VIP_IP(nxthdr);
	#endif
}

/*
parameter:
		sport/dport -- TCP/UDP port,0 means not care
		mainly for VOIP
*/
static inline int FE_VIP_ADD_PORT(FE_VIP_PROTO proto,unsigned int sport,unsigned int dport)
{
	unsigned int mode = 0;
	
	if(isEN7580)	//only EN7580 do not have VIP
		return IFC_API_SET_VIP_BY_PORT(sport,dport);

	if(sport)
		mode |= 0x1;
	if(dport)
		mode |= 0x2;

	if(FE_VIP_PROTO_TCP == proto)
		return FE_API_ADD_VIP_TCP(sport,dport,mode);
	else if(FE_VIP_PROTO_UDP == proto)
		return FE_API_ADD_VIP_UDP(sport,dport,mode);

	return ECNT_HOOK_ERROR;
}

/*
parameter:
		sip -- format is hex,ex 192.168.1.1->0xc0a80101,0 means not care
		sport -- TCP/UDP sport,0 means not care
		mainly for TR069
*/
static inline int FE_VIP_ADD_SIP_AND_SPORT(unsigned int sip,unsigned short sport)
{
	#if defined(TCSUPPORT_CPU_EN7580)
		return IFC_API_SET_VIP_BY_SIP_AND_SPORT(sip,sport);
	#else
		return ECNT_HOOK_ERROR;
	#endif
}

static inline int FE_VIP_DEL_ETYPE(unsigned short ether_type)
{
	#if defined(TCSUPPORT_CPU_EN7580)
		return IFC_API_DEL_VIP_BY_ETYPE(ether_type);
	#else
		return FE_API_DEL_VIP_ETHER(ether_type);
	#endif
}

static inline int FE_VIP_DEL_IPV4_PROTO(unsigned short proto)
{
	#if defined(TCSUPPORT_CPU_EN7580)
		return IFC_API_DEL_VIP_BY_IPV4_PROTO(proto);
	#else
		return FE_API_DEL_VIP_IP(proto);
	#endif
}

static inline int FE_VIP_DEL_IPV6_NXTHDR(unsigned short nxthdr)
{
	#if defined(TCSUPPORT_CPU_EN7580)
		return IFC_API_DEL_VIP_BY_IPV6_NXTHDR(nxthdr);
	#else
		return FE_API_DEL_VIP_IP(nxthdr);
	#endif
}


/*
parameter:
		sport/dport -- TCP/UDP port,0 means not care
*/
static inline int FE_VIP_DEL_PORT(FE_VIP_PROTO proto,unsigned short sport,unsigned short dport)
{
	unsigned int mode = 0;
	
	#if defined(TCSUPPORT_CPU_EN7580)
		return IFC_API_DEL_VIP_BY_PORT(sport,dport);
	#else
		if(sport)
			mode |= 0x1;
		if(dport)
			mode |= 0x2;

		if(FE_VIP_PROTO_TCP == proto)
			return FE_API_DEL_VIP_TCP(sport,dport,mode);
		else if(FE_VIP_PROTO_UDP == proto)
			return FE_API_DEL_VIP_UDP(sport,dport,mode);
		else
			return ECNT_HOOK_ERROR;
	#endif
}

/*
parameter:
		dip -- format is hex,ex 192.168.1.1->0xc0a80101,0 means not care
		dport -- TCP/UDP dport,0 means not care
		mainly for TR069
*/
static inline int FE_VIP_DEL_SIP_AND_SPORT(unsigned int sip,unsigned short sport)
{
	#if defined(TCSUPPORT_CPU_EN7580)
		return IFC_API_DEL_VIP_BY_SIP_AND_SPORT(sip,sport);
	#else
		return ECNT_HOOK_ERROR;
	#endif
}

#endif /* _ECNT_HOOK_IFC_H_ */

