/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	igmp_snoop.h
 
    Abstract:
    
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
 

#ifndef __RTMP_IGMP_SNOOP_H__
#define __RTMP_IGMP_SNOOP_H__

#include "link_list.h"

#define IP_UDP 0x11
#define IGMP_PROTOCOL_DESCRIPTOR	0x02
#define IGMP_V1_MEMBERSHIP_REPORT	0x12
#define IGMP_V2_MEMBERSHIP_REPORT	0x16
#define IGMP_LEAVE_GROUP			0x17
#define IGMP_V3_MEMBERSHIP_REPORT	0x22

#define MLD_V1_LISTENER_REPORT		131
#define MLD_V1_LISTENER_DONE		132
#define MLD_V2_LISTERNER_REPORT		143

#define IGMPMAC_TB_ENTRY_AGEOUT_TIME (120 * OS_HZ)

#define MULTICAST_ADDR_HASH_INDEX(Addr)      (MAC_ADDR_HASH(Addr) & (MAX_LEN_OF_MULTICAST_FILTER_HASH_TABLE - 1))

#define IS_IPV6_MULTICAST_MAC_ADDR(Addr)	((((Addr[0]) & 0x01) == 0x01) && ((Addr[0]) == 0x33))

#define IGMP_NONE		0
#define IGMP_PKT		1
#define IGMP_IN_GROUP	2


VOID MulticastFilterTableInit(
	IN PRTMP_ADAPTER pAd,
	IN PMULTICAST_FILTER_TABLE *ppMulticastFilterTable);

VOID MultiCastFilterTableReset(
	IN PMULTICAST_FILTER_TABLE *ppMulticastFilterTable);

BOOLEAN MulticastFilterTableInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev,
	IN MulticastFilterEntryType type);

BOOLEAN MulticastFilterTableDeleteEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev);

PMULTICAST_FILTER_TABLE_ENTRY MulticastFilterTableLookup(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PUCHAR pAddr,
	IN PNET_DEV dev);

BOOLEAN isIgmpPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);

VOID IGMPSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN PNET_DEV pDev);

BOOLEAN isMldPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader,
	OUT UINT8 *pProtoType,
	OUT PUCHAR *pMldHeader);

BOOLEAN IPv6MulticastFilterExcluded(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);

VOID MLDSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN PNET_DEV pDev);

UCHAR IgmpMemberCnt(
	IN PLIST_HEADER pList);

VOID IgmpGroupDelMembers(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV pDev);

INT Set_IgmpSn_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_IgmpSn_AddEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_IgmpSn_DelEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_IgmpSn_TabDisplay_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

void rtmp_read_igmp_snoop_from_file(
	IN  PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer);

NDIS_STATUS IgmpPktInfoQuery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pSrcBufVA,
	IN PNDIS_PACKET pPacket,
	IN struct wifi_dev *wdev,
	OUT INT *pInIgmpGroup,
	OUT PMULTICAST_FILTER_TABLE_ENTRY *ppGroupEntry);

NDIS_STATUS IgmpProtocolPktClone(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	IN INT IgmpPktInGroup,
	IN PMULTICAST_FILTER_TABLE_ENTRY pGroupEntry,
	IN UCHAR QueIdx,
	IN UINT8 UserPriority,
	IN PNET_DEV pNetDev);
	
NDIS_STATUS IgmpPktClone(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	IN INT IgmpPktInGroup,
	IN PMULTICAST_FILTER_TABLE_ENTRY pGroupEntry,
	IN UCHAR QueIdx,
	IN UINT8 UserPriority,
	IN PNET_DEV pNetDev);

#endif /* __RTMP_IGMP_SNOOP_H__ */

