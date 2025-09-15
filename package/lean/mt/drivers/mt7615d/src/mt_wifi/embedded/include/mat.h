/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    mat.h

    Abstract:
    Support AP-Client function.

    Revision History:
    Who               When            What
    --------------    ----------      ----------------------------------------------
    Shiang		      02-26-2007      created
*/

#ifndef _MAT_H_
#define _MAT_H_




/*#if defined(LINUX) || defined (VXWORKS) */

/*#else */
/*Currently support upper layer protocols */
#ifndef ETH_P_IP
#define ETH_P_IP        0x0800          /* Internet Protocol packet     */
#endif
#ifndef ETH_P_ARP
#define ETH_P_ARP       0x0806          /* Address Resolution packet    */
#endif
#ifndef ETH_P_PPP_DISC
#define ETH_P_PPP_DISC  0x8863          /* PPPoE discovery messages     */
#endif
#ifndef ETH_P_PPP_SES
#define ETH_P_PPP_SES   0x8864          /* PPPoE session messages       */
#endif
#ifndef ETH_P_VLAN
#define ETH_P_VLAN   0x8100         	/* VLAN packet       */
#endif

/* ARP protocol HARDWARE identifiers. */
#ifndef ARPHRD_ETHER
#define ARPHRD_ETHER    1               /* Ethernet 10Mbps              */
#endif

/* ARP protocol opcodes. */
#ifndef ARPOP_REQUEST
#define ARPOP_REQUEST   1               /* ARP request                  */
#endif
#ifndef ARPOP_REPLY
#define ARPOP_REPLY     2               /* ARP reply                    */
#endif

typedef struct _NET_PRO_ARP_HDR {
	unsigned short  ar_hrd;         /* format of hardware address   */
	unsigned short  ar_pro;         /* format of protocol address   */
	unsigned char   ar_hln;         /* length of hardware address   */
	unsigned char   ar_pln;         /* length of protocol address   */
	unsigned short  ar_op;          /* ARP opcode (command)         */
} NET_PRO_ARP_HDR;

typedef struct _NET_PRO_IP_HDR {
#ifndef RT_BIG_ENDIAN
	UCHAR	ihl:4;
	UCHAR	version:4;
#else
	UCHAR	version:4;
	UCHAR	ihl:4;
#endif
	UCHAR	tos;
	UINT16	tot_len;
	UINT16	id;
	UINT16	frag_off;
	UCHAR	ttl;
	UCHAR	protocol;
	UINT16	check;
	UINT16	saddr;
	UINT32	daddr;
} NET_PRO_IP_HDR;
/*#endif  //endif of __LINUX__ */

/* #ifndef MAT_SUPPORT */
/* #error "You should define MAT_SUPPORT if you want to compile MAT related functions!" */
/* #endif */


/* MAT relate definition */
#define MAT_MAX_HASH_ENTRY_SUPPORT		64
#define MAT_TB_ENTRY_AGEOUT_TIME			(5 * 60 * OS_HZ)	/* 30000, 5min. MAT convert table entry age-out time interval. now set it as 5min. */


/* 802.3 Ethernet related definition */
#define MAT_ETHER_HDR_LEN		14							/* dstMac(6) + srcMac(6) + protoType(2) */
#define MAT_VLAN_ETH_HDR_LEN	(MAT_ETHER_HDR_LEN + 4)		/* 4 for h_vlan_TCI and h_vlan_encapsulated_proto */

#define MAT_MAC_ADDR_HASH(Addr)       (Addr[0] ^ Addr[1] ^ Addr[2] ^ Addr[3] ^ Addr[4] ^ Addr[5])
#define MAT_MAC_ADDR_HASH_INDEX(Addr) (MAT_MAC_ADDR_HASH(Addr) % MAT_MAX_HASH_ENTRY_SUPPORT)

#define isMcastEtherAddr(addr)	(addr[0] & 0x1)
#define isBcastEtherAddr(addr)	((addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5]) == 0xff)
#define isZeroEtherAddr(addr)	(!(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]))

#define IS_GROUP_MAC(Addr)		((Addr[0]) & 0x01)
#define IS_UCAST_MAC(addr)		(!(isMcastEtherAddr(addr) || isZeroEtherAddr(addr))) /* isUcastMac = !(00:00:00:00:00:00 || mcastMac); */
#define IS_EQUAL_MAC(a, b)		(((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2]) | (a[3] ^ b[3]) | (a[4] ^ b[4]) | (a[5] ^ b[5])) == 0)

#define IS_VLAN_PACKET(pkt)		((((pkt)[12] << 8) | (pkt)[13]) == 0x8100)

/* IPv4 related definition */
#define IPMAC_TB_HASH_ENTRY_NUM			(MAT_MAX_HASH_ENTRY_SUPPORT+1)	/* One entry for broadcast address */
#define IPMAC_TB_HASH_INDEX_OF_BCAST	MAT_MAX_HASH_ENTRY_SUPPORT		/* cause hash index start from 0. */

#define MAT_IP_ADDR_HASH(Addr)		(((Addr>>24)&0xff)^((Addr>>16) & 0xff) ^ ((Addr>>8) & 0xff) ^ (Addr & 0xff))
#define MAT_IP_ADDR_HASH_INDEX(Addr)	(MAT_IP_ADDR_HASH(Addr) % MAT_MAX_HASH_ENTRY_SUPPORT)

#define IS_GOOD_IP(IP)	(IP != 0)
#define IS_MULTICAST_IP(IP)	(((UINT32)(IP) & 0xf0000000) == 0xe0000000)

/* IPv6 related definition */
#define IPV6MAC_TB_HASH_ENTRY_NUM		(MAT_MAX_HASH_ENTRY_SUPPORT+1)	/* One entry for broadcast address */
#define IPV6MAC_TB_HASH_INDEX_OF_BCAST	MAT_MAX_HASH_ENTRY_SUPPORT		/* cause hash index start from 0. */

/*We just use byte 10,13,14,15 to calculate the IPv6 hash, because the byte 11,12 usually are 0xff, 0xfe for link-local address. */
#define MAT_IPV6_ADDR_HASH(Addr)    ((Addr[10]&0xff) ^ (Addr[13] & 0xff) ^ (Addr[14] & 0xff) ^ (Addr[15] & 0xff))
#define MAT_IPV6_ADDR_HASH_INDEX(Addr)	(MAT_IPV6_ADDR_HASH(Addr) % MAT_MAX_HASH_ENTRY_SUPPORT)

#define IS_UNSPECIFIED_IPV6_ADDR(_addr)	\
	(!((_addr).ipv6_addr32[0] | (_addr).ipv6_addr32[1] | (_addr).ipv6_addr32[2] | (_addr).ipv6_addr32[3]))

#define IS_LOOPBACK_IPV6_ADDR(_addr) \
	(NdisEqualMemory((UCHAR *)(&((_addr).ipv6_addr[0])), &IPV6_LOOPBACKADDR[0], IPV6_ADDR_LEN))
#define IS_MULTICAST_IPV6_ADDR(_addr) \
	(((_addr).ipv6_addr[0] & 0xff) == 0xff)

/* The MAT_TABLE used for MacAddress <-> UpperLayer Address Translation. */
typedef struct _MAT_TABLE_ {
	VOID	*IPMacTable;		/* IPv4 Address, Used for IP, ARP protocol */
	VOID	*IPv6MacTable;		/* IPv6 Address, Used for IPv6 related protocols */
	VOID	*SesMacTable;		/* PPPoE Session */
	VOID	*UidMacTable;		/* PPPoE Discovery */
} MAT_TABLE, *PMAT_TABLE;


typedef enum _MAT_ENGINE_STATUS_ {
	MAT_ENGINE_STAT_UNKNOWN = 0,
	MAT_ENGINE_STAT_INITED = 1,
	MAT_ENGINE_STAT_EXITED = 2,
} MAT_ENGINE_STATUS;


typedef struct _MAT_STRUCT_ {
	MAT_ENGINE_STATUS	status;
	NDIS_SPIN_LOCK		MATDBLock;
	MAT_TABLE			MatTableSet;
#ifdef KMALLOC_BATCH
	UCHAR				*pMATNodeEntryPoll;
#endif
#ifdef MAC_REPEATER_SUPPORT
	BOOLEAN				bMACRepeaterEn;
#endif /* MAC_REPEATER_SUPPORT */
	UINT32				nodeCount;		/* the number of nodes which connect to Internet via us. */
	VOID				*pPriv;
} MAT_STRUCT;


typedef struct _MATProtoOps {
	NDIS_STATUS(*init)(MAT_STRUCT * pMatCfg);
	PUCHAR			(*tx)(MAT_STRUCT * pMatCfg, PNDIS_PACKET pSkb, PUCHAR pLayerHdr, PUCHAR pMacAddr);
	PUCHAR			(*rx)(MAT_STRUCT * pMatCfg, PNDIS_PACKET pSkb, PUCHAR pLayerHdr, PUCHAR pMacAddr);
	NDIS_STATUS(*exit)(MAT_STRUCT * pMatCfg);
} MATProtoOps, *PMATProtoOps;


typedef struct _MATProtoTable {
	USHORT			protocol;
	MATProtoOps		*pHandle;
} MATProtoTable, *PMATProtoTable;


VOID dumpPkt(PUCHAR pHeader, int len);

/*#define KMALLOC_BATCH */

PUCHAR MATDBEntryAlloc(
	IN MAT_STRUCT * pMatStruct,
	IN UINT32		size);

NDIS_STATUS MATDBEntryFree(
	IN MAT_STRUCT * pMatStruct,
	IN PUCHAR		NodeEntry);


#endif /* _MAT_H_ */

