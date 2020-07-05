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

#include "common/link_list.h"

#define IGMP_PROTOCOL_DESCRIPTOR	0x02
#define IGMP_MEMBERSHIP_QUERY		0x11	/*same for IGMP v1, v2 & v3*/
#define IGMP_V1_MEMBERSHIP_REPORT	0x12
#define IGMP_V2_MEMBERSHIP_REPORT	0x16
#define IGMP_LEAVE_GROUP			0x17
#define IGMP_V3_MEMBERSHIP_REPORT	0x22

#define MLD_LISTENER_QUERY			130		/*same for MLD v1 & v2*/
#define MLD_V1_LISTENER_REPORT		131
#define MLD_V1_LISTENER_DONE		132
#define MLD_V2_LISTERNER_REPORT		143

#define IGMPMAC_TB_ENTRY_AGEOUT_TIME (120 * OS_HZ)

#define MULTICAST_ADDR_HASH_INDEX(Addr)      (MAC_ADDR_HASH(Addr) & (MAX_LEN_OF_MULTICAST_FILTER_HASH_TABLE - 1))

#define IS_IPV6_MULTICAST_MAC_ADDR(Addr)	((((Addr[0]) & 0x01) == 0x01) && ((Addr[0]) == 0x33))

#define IGMP_NONE		0
#define IGMP_PKT		1
#define IGMP_IN_GROUP	2
#ifdef VENDOR_FEATURE6_SUPPORT
#ifndef ETH_TYPE_VLAN
#define ETH_TYPE_VLAN   0X8100
#endif
#endif
#define IGMP_CFG_BAND0		(1 << 0)
#define IGMP_CFG_BAND1		(1 << 1)


VOID MulticastFilterTableInit(
	IN PRTMP_ADAPTER pAd,
	IN PMULTICAST_FILTER_TABLE * ppMulticastFilterTable);

VOID MultiCastFilterTableReset(
	PRTMP_ADAPTER pAd,
	IN PMULTICAST_FILTER_TABLE * ppMulticastFilterTable);

BOOLEAN MulticastFilterTableInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	UINT8 BssIdx,
	IN UINT8 type,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev,
	IN UINT8 WlanIndex);

BOOLEAN MulticastFilterTableDeleteEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	UINT8 BssIdx,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev,
	IN UINT8 WlanIndex);

PMULTICAST_FILTER_TABLE_ENTRY MulticastFilterTableLookup(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PUCHAR pAddr,
	IN PNET_DEV dev);

BOOLEAN isIgmpPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);

#ifdef IGMP_TVM_SUPPORT
INT IgmpSnEnableTVMode(
	IN RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 IsTVModeEnable,
	UINT8 TVModeType);

VOID ConvertUnicastMacToMulticast(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev,
	IN RX_BLK * pRxBlk);

VOID MakeTVMIE(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev,
	IN OUT PUCHAR pOutBuffer,
	IN OUT PULONG pFrameLen);

INT Set_IgmpSn_BlackList_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg);

INT Show_IgmpSn_BlackList_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg);

BOOLEAN isIgmpMldExemptPkt(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PUCHAR pGroupIpAddr,
	IN UINT16 ProtoType);

INT Set_IgmpSn_AgeOut_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_IgmpSn_McastTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

BOOLEAN IgmpSnoopingGetMulticastTable(
	RTMP_ADAPTER *pAd,
	UINT8 ucOwnMacIdx,
	P_IGMP_MULTICAST_TABLE pMcastTable);

VOID IgmpSnoopingShowMulticastTable(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

BOOLEAN MulticastFilterConfigAgeOut(RTMP_ADAPTER *pAd, UINT8 AgeOutTime, UINT8 ucOwnMacIdx);

BOOLEAN MulticastFilterInitMcastTable(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN bActive);

BOOLEAN MulticastFilterGetMcastTable(RTMP_ADAPTER *pAd, UINT8 ucOwnMacIdx, struct wifi_dev *wdev);

#endif /* IGMP_TVM_SUPPORT */

VOID IGMPSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN MAC_TABLE_ENTRY *pEntry,
	UINT8 Wcid);

#ifdef A4_CONN
/* Indicate if Specific Pkt is an IGMP query message*/
BOOLEAN isIGMPquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);
#endif

BOOLEAN isMldPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader,
	OUT UINT8 *pProtoType,
	OUT PUCHAR * pMldHeader);

BOOLEAN IPv6MulticastFilterExcluded(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);

VOID MLDSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN MAC_TABLE_ENTRY *pEntry,
	UINT8 Wcid);

#ifdef A4_CONN
/* Indicate if Specific Pkt is an MLD query message*/
BOOLEAN isMLDquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);
#endif


UCHAR IgmpMemberCnt(
	IN PLIST_HEADER pList);

VOID IgmpGroupDelMembers(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMemberAddr,
	IN struct wifi_dev *wdev,
	UINT8 Wcid);

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

NDIS_STATUS IgmpPktClone(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pPacket,
	INT IgmpPktInGroup,
	PMULTICAST_FILTER_TABLE_ENTRY pGroupEntry,
	UCHAR QueIdx,
	UINT8 UserPriority,
	PNET_DEV pNetDev);

#ifdef A4_CONN
/* Indicate if Specific Pkt is an IGMP query message*/
BOOLEAN isIGMPquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader);
#endif



#ifdef A4_CONN

#define QUERY_SEND_PERIOD 6 /* 60 seconds */
#define QUERY_HOLD_PERIOD 15 /* 150 seconds*/

/* Send an IGMP query message on particular AP interface*/
void send_igmpv3_gen_query_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN  PMAC_TABLE_ENTRY pMacEntry);

/* Send a MLD query message on particular AP interface*/
void send_mldv2_gen_query_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN  PMAC_TABLE_ENTRY pMacEntry);

/* For specifed MBSS, compute & store IPv6 format checksum for MLD query message to be sent on that interface*/
void calc_mldv2_gen_query_chksum(
	IN	PRTMP_ADAPTER	pAd,
	IN  BSS_STRUCT *pMbss);
#endif


#endif /* __RTMP_IGMP_SNOOP_H__ */

