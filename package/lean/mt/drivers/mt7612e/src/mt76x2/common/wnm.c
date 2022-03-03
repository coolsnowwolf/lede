/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wnm.c

	Abstract:
	Wireless Network Management(WNM)

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"
static UCHAR ZERO_IP_ADDR[4] = {0x00, 0x00, 0x00, 0x00};

static char SolicitedMulticastAddr[] = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 
									  0x00, 0x00, 0x00, 0x00, 0x01, 0xff};  
static char AllNodeLinkLocalMulticastAddr[] = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00,
											 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											 0x00, 0x00, 0x00, 0x01};

static char link_local[] = {0xfe, 0x80};
#define IP_PROTO_HOPOPTS        0       /* IP6 hop-by-hop options - RFC1883 */
#define IP_PROTO_ROUTING        43      /* IP6 routing header */
#define IP_PROTO_FRAGMENT       44      /* IP6 fragmentation header */
#define IP_PROTO_AH             51      /* Authentication Header for IPv6 - RFC2402*/
#define IP_PROTO_DSTOPTS        60      /* IP6 destination options - RFC1883 */

#ifndef MAT_SUPPORT
#define IS_UNSPECIFIED_IPV6_ADDR(_addr)	\
		(!((_addr).ipv6_addr32[0] | (_addr).ipv6_addr32[1] | (_addr).ipv6_addr32[2] | (_addr).ipv6_addr32[3]))
#endif

#ifdef CONFIG_AP_SUPPORT
void wext_send_btm_query_event(PNET_DEV net_dev, const char *peer_mac_addr,
							   const char *btm_query, u16 btm_query_len)
{
	struct btm_query_data *query_data;
	u16 buflen = 0;
	char *buf;	

	buflen = sizeof(*query_data) + btm_query_len;
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	query_data = (struct btm_query_data *)buf;
	query_data->ifindex = RtmpOsGetNetIfIndex(net_dev);
	memcpy(query_data->peer_mac_addr, peer_mac_addr, 6);
	query_data->btm_query_len	= btm_query_len;
	memcpy(query_data->btm_query, btm_query, btm_query_len);

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM, 
					OID_802_11_WNM_BTM_QUERY, NULL, (PUCHAR)buf, buflen);

	os_free_mem(NULL, buf);
}

void SendBTMQueryEvent(PNET_DEV net_dev, const char *peer_mac_addr,
					   const char *btm_query, UINT16 btm_query_len, UINT8 ipc_type)
{
	if (ipc_type == RA_WEXT) {
		wext_send_btm_query_event(net_dev,
								  peer_mac_addr,
								  btm_query,
								  btm_query_len);
	}
}


void wext_send_btm_cfm_event(PNET_DEV net_dev, const char *peer_mac_addr,
							 const char *btm_rsp, u16 btm_rsp_len)
{
	
	struct btm_rsp_data *rsp_data;
	u16 buflen = 0;
	char *buf;


	buflen = sizeof(*rsp_data) + btm_rsp_len;
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	rsp_data = (struct btm_rsp_data *)buf;
	rsp_data->ifindex = RtmpOsGetNetIfIndex(net_dev);
	memcpy(rsp_data->peer_mac_addr, peer_mac_addr, 6);
	rsp_data->btm_rsp_len	= btm_rsp_len;
	memcpy(rsp_data->btm_rsp, btm_rsp, btm_rsp_len);

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM, 
						OID_802_11_WNM_BTM_RSP, NULL, (PUCHAR)buf, buflen);

	os_free_mem(NULL, buf);
}

void SendBTMConfirmEvent(PNET_DEV net_dev, const char *peer_mac_addr,
						 const char *btm_rsp, UINT16 btm_rsp_len, UINT8 ipc_type)
{
	if (ipc_type == RA_WEXT) {
		wext_send_btm_cfm_event(net_dev,
								peer_mac_addr,
								btm_rsp,
								btm_rsp_len);
	}
}

void wext_send_proxy_arp_event(PNET_DEV net_dev,
							   const char *source_mac_addr,
							   const char *source_ip_addr,
							   const char *target_mac_addr,
							   const char *target_ip_addr, 
							   u8 ip_type,							   
							   u8 from_ds,
							   unsigned char IsDAD)
							   
{
	struct proxy_arp_entry *arp_entry;
	u16 varlen = 0, buflen = 0;
	char *buf;

	if (ip_type == IPV4)
		varlen += 8;
	else if (ip_type == IPV6) 
		varlen += 32;
	
	//for IsDAD, add one more byte
	varlen++;

	buflen = sizeof(*arp_entry) + varlen;

	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	arp_entry = (struct proxy_arp_entry *)buf;

	arp_entry->ifindex = RtmpOsGetNetIfIndex(net_dev);
	arp_entry->ip_type = ip_type;
	arp_entry->from_ds = from_ds;
	arp_entry->IsDAD = IsDAD;
	memcpy(arp_entry->source_mac_addr, source_mac_addr, 6);
	memcpy(arp_entry->target_mac_addr, target_mac_addr, 6);
	
	if (ip_type == IPV4) {
		memcpy(arp_entry->ip_addr, source_ip_addr, 4);
		memcpy(arp_entry->ip_addr + 4, target_ip_addr, 4);
	} else if (ip_type == IPV6) {
		memcpy(arp_entry->ip_addr, source_ip_addr, 16);
		memcpy(arp_entry->ip_addr + 16, target_ip_addr, 16);
	} else
		printk("error not such ip type packet\n");

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
						OID_802_11_WNM_PROXY_ARP, NULL, (PUCHAR)buf, buflen);

	os_free_mem(NULL, buf);
}


void SendProxyARPEvent(PNET_DEV net_dev,
					   const char *source_mac_addr,
					   const char *source_ip_addr, 
					   const char *target_mac_addr,
					   const char *target_ip_addr,
				  	   u8 ip_type,
				  	   u8 from_ds,
				  	   unsigned char IsDAD)
					   
{
	wext_send_proxy_arp_event(net_dev,
							  source_mac_addr,
							  source_ip_addr,
							  target_mac_addr,
							  target_ip_addr,
							  ip_type,
							  from_ds,
							  IsDAD);
}


BOOLEAN IsGratuitousARP(IN RTMP_ADAPTER *pAd,
						IN UCHAR *pData,
						IN UCHAR *DAMacAddr,
						IN struct _MULTISSID_STRUCT *pMbss)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	UCHAR *SenderIP;
	UCHAR *TargetIP;
	
	UCHAR BroadcastMac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	UINT16 ARPOperation;
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	PUCHAR SourceMACAddr; 
	BOOLEAN IsDrop = FALSE;
	INT32 Ret;


	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_ARP)
	{
		/* 
 		 * Check if Gratuitous ARP, Sender IP equal Target IP
 		 */
 		SourceMACAddr = Pos + 8;
		SenderIP = Pos + 14;
		TargetIP = Pos + 24;
		
		if ((NdisCmpMemory(SenderIP, TargetIP, 4) == 0) && (NdisCmpMemory(DAMacAddr, BroadcastMac, 6) == 0))
		
		{
			DBGPRINT(RT_DEBUG_TRACE, ("The Packet is GratuitousARP\n"));
			return TRUE;
		}
		
#ifdef CONFIG_HOTSPOT_R2
		Pos += 6;
		NdisMoveMemory(&ARPOperation, Pos, 2);
		ARPOperation = OS_NTOHS(ARPOperation);
		Pos += 2;

		if (ARPOperation == 0x0002)
		{
			RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
			DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
			{
				if ((IPV4_ADDR_EQUAL(ProxyARPEntry->TargetIPAddr, SenderIP)) && (MAC_ADDR_EQUAL(ProxyARPEntry->TargetMACAddr, SourceMACAddr) == FALSE))
				{
					IsDrop = TRUE;
					break;
				}
			}
			RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
		
			if (IsDrop == TRUE)
			{
				printk("Drop pkt, ip not qeual mac\n");
				return TRUE;
			}			
		}
#endif		
	}

	return FALSE;
}


BOOLEAN IsUnsolicitedNeighborAdver(PRTMP_ADAPTER pAd,
								   PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;

	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_IPV6)
	{
		Pos += 24;	
		
		if (RTMPEqualMemory(Pos, AllNodeLinkLocalMulticastAddr, 16))
		{
			Pos += 16;

			/* Check if neighbor advertisement type */
			if (*Pos == 0x88)
			{
				Pos += 4;

				/* Check if solicited flag set to 0 */
				if ((*Pos & 0x40) == 0x00)
				{
					DBGPRINT(RT_DEBUG_OFF, ("The Packet is UnsolicitedNeighborAdver\n"));
					Pos += 4;
					return TRUE;
				}
			}
		}

	}

	return FALSE;
}


BOOLEAN IsIPv4ProxyARPCandidate(IN PRTMP_ADAPTER pAd,
						   		IN PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	UINT16 ARPOperation;
	UCHAR *SenderIP;
	UCHAR *TargetIP;


	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_ARP)
	{
		Pos += 6;
		NdisMoveMemory(&ARPOperation, Pos, 2);
		ARPOperation = OS_NTOHS(ARPOperation);
		Pos += 2;

		if (ARPOperation == 0x0001)
		{
			SenderIP = Pos + 6;
			TargetIP = Pos + 16;
			/* ARP Request */
			if (NdisCmpMemory(SenderIP, TargetIP, 4) != 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("IPv4ProxyARPCandidate\n"));
				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOLEAN IsIpv6DuplicateAddrDetect(PRTMP_ADAPTER pAd,								  
										PUCHAR pData,
										PUCHAR pOffset)								  
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	RT_IPV6_ADDR *pIPv6Addr;
	UCHAR	*pData_offset = Pos;

	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_IPV6)
	{
	
		INT32	PayloadLen = 0;//((*(Pos+5) & 0xff) << 8) | (*(Pos+4) & 0xff);
		UCHAR	NextHeader = *(Pos+6);
		UCHAR	IsExtenHeader = 0;
		
		NdisMoveMemory(&PayloadLen, (Pos+4), 2);
		PayloadLen = OS_NTOHS(PayloadLen);
	
		Pos += 8;
		pIPv6Addr = (RT_IPV6_ADDR *)Pos;

		if (IS_UNSPECIFIED_IPV6_ADDR(*pIPv6Addr))
		{
			Pos += 16;
			if (RTMPEqualMemory(Pos, SolicitedMulticastAddr, 13))
			{
				Pos += 16;
				
				if ((NextHeader == IP_PROTO_HOPOPTS) || (NextHeader == IP_PROTO_ROUTING) || (NextHeader == IP_PROTO_FRAGMENT) || (NextHeader == IP_PROTO_AH) || (NextHeader == IP_PROTO_DSTOPTS))             
				{
					IsExtenHeader = 1;
			
					do
					{
						printk("IsIpv6DuplicateAddrDetect: nextheader=0x%x, %d, %d\n", NextHeader, PayloadLen, IsExtenHeader);
						switch (NextHeader)
						{
							case IP_PROTO_HOPOPTS:
							case IP_PROTO_ROUTING:
							case IP_PROTO_DSTOPTS:	
								{
									UCHAR HdrExtLen = *(Pos+1);
								
									NextHeader = *Pos;
									PayloadLen -= ((HdrExtLen+1)<<3);
									Pos += ((HdrExtLen+1)<<3);
								}
								break;
							case IP_PROTO_FRAGMENT:
								{
									NextHeader = *Pos;
									PayloadLen -= 8;
									Pos += 8;
								}
								break;	
							case IP_PROTO_AH:
								{
									UCHAR AHPayloadLen = *(Pos+1);
									UCHAR AHLen = (8+(AHPayloadLen<<2));
							
									NextHeader = *Pos;
									PayloadLen -= AHLen;
									Pos += AHLen;
								}
								break;	
							default:
								IsExtenHeader = 0;
								break;
						}
					}
					while((PayloadLen > 0) && (IsExtenHeader == 1));
			
					if (PayloadLen <= 0)
						return FALSE;
				}
				

				/* Check if neighbor solicitation */
				if (*Pos == 0x87)
				{
					DBGPRINT(RT_DEBUG_OFF, ("THe Packet is for Ipv6DuplicateAddrDetect\n"));					
					*pOffset = Pos-pData_offset+8;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOLEAN IsIPv6ProxyARPCandidate(IN PRTMP_ADAPTER pAd,
								IN PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
		
	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_IPV6)
	{
	
		INT32	PayloadLen = 0; //((*(Pos+5) & 0xff) << 8) | (*(Pos+4) & 0xff);
		UCHAR	NextHeader = *(Pos+6);
		UCHAR	IsExtenHeader = 0;
		
		NdisMoveMemory(&PayloadLen, (Pos+4), 2);
		PayloadLen = OS_NTOHS(PayloadLen);
	
		Pos += 8;		
		
		//if (!IS_UNSPECIFIED_IPV6_ADDR(*pIPv6Addr))
		//{
			Pos += 16;
			if (RTMPEqualMemory(Pos, SolicitedMulticastAddr, 13))
			{
				Pos+= 16;
				if ((NextHeader == IP_PROTO_HOPOPTS) || (NextHeader == IP_PROTO_ROUTING) || (NextHeader == IP_PROTO_FRAGMENT) || (NextHeader == IP_PROTO_AH) || (NextHeader == IP_PROTO_DSTOPTS))             
				{
					IsExtenHeader = 1;
			
					do
					{
						printk("IsIPv6ProxyARPCandidate: nextheader=0x%x, %d, %d\n", NextHeader, PayloadLen, IsExtenHeader);
						switch (NextHeader)
						{
							case IP_PROTO_HOPOPTS:
							case IP_PROTO_ROUTING:
							case IP_PROTO_DSTOPTS:	
								{
									UCHAR HdrExtLen = *(Pos+1);
								
									NextHeader = *Pos;
									PayloadLen -= ((HdrExtLen+1)<<3);
									Pos += ((HdrExtLen+1)<<3);
								}
								break;
							case IP_PROTO_FRAGMENT:
								{
									NextHeader = *Pos;
									PayloadLen -= 8;
									Pos += 8;
								}
								break;	
							case IP_PROTO_AH:
								{
									UCHAR AHPayloadLen = *(Pos+1);
									UCHAR AHLen = (8+(AHPayloadLen<<2));
							
									NextHeader = *Pos;
									PayloadLen -= AHLen;
									Pos += AHLen;
								}
								break;	
							default:
								IsExtenHeader = 0;
								break;
						}
					}
					while((PayloadLen > 0) && (IsExtenHeader == 1));
			
					if (PayloadLen <= 0)
						return FALSE;
				}
				

				/* Check if neighbor solicitation */
				if (*Pos == 0x87)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("The Packet is IPv6ProxyARPCandidate\n")); 
					return TRUE;
				}
			}
		//}
	}

	return FALSE;
}

//JERRY
BOOLEAN IsIPv6DHCPv6Solicitation(IN PRTMP_ADAPTER pAd,
                                 IN PUCHAR pData)
{
    UCHAR *Pos = pData;
    UINT16 ProtoType, SrcPort, DstPort;

    NdisMoveMemory(&ProtoType, pData, 2);
    ProtoType = OS_NTOHS(ProtoType);
    Pos += 2;

    if (ProtoType == ETH_P_IPV6)
    {
        INT32   PayloadLen = 0; //((*(Pos+5) & 0xff) << 8) | (*(Pos+4) & 0xff);
        UCHAR   NextHeader = *(Pos+6);
        UCHAR   IsExtenHeader = 0;

        NdisMoveMemory(&PayloadLen, (Pos+4), 2);
        PayloadLen = OS_NTOHS(PayloadLen);

        if ((NextHeader != IP_PROTO_HOPOPTS) && (NextHeader != IP_PROTO_ROUTING) &&  (NextHeader != IP_PROTO_FRAGMENT) &&  (NextHeader != IP_PROTO_AH) && (NextHeader != IP_PROTO_DSTOPTS))
        {
			if (NextHeader == 0x11)
			{
            	Pos += 40;
			}
			else
				return FALSE;
        }
        else
        {
            IsExtenHeader = 1;
            Pos += 40;

            do
            {
                printk("IsIPv6DHCPv6Solicitation: nextheader=0x%x, %d, %d\n", NextHeader, PayloadLen, IsExtenHeader);
                switch (NextHeader)
                {
                    case IP_PROTO_HOPOPTS:
                    case IP_PROTO_ROUTING:
                    case IP_PROTO_DSTOPTS:
						{
                            UCHAR HdrExtLen = *(Pos+1);

                            NextHeader = *Pos;
                            PayloadLen -= ((HdrExtLen+1)<<3);
                            Pos += ((HdrExtLen+1)<<3);
                        }
                        break;
                    case IP_PROTO_FRAGMENT:
                        {
                            NextHeader = *Pos;
                            PayloadLen -= 8;
                            Pos += 8;
                        }
                        break;
                    case IP_PROTO_AH:
                        {
                            UCHAR AHPayloadLen = *(Pos+1);
                            UCHAR AHLen = (8+(AHPayloadLen<<2));

                            NextHeader = *Pos;
                            PayloadLen -= AHLen;
                            Pos += AHLen;
                        }
                        break;
                    default:
                        IsExtenHeader = 0;
                        break;
                }
            }
            while((PayloadLen > 0) && (IsExtenHeader == 1));

            if (PayloadLen <= 0)
                return FALSE;
        }

        /* Check if DHCPv6 solicitation */
		{
			unsigned char *type = (unsigned char *)(Pos+8);
			
			if ((*type == 1) || (*type == 4))
			{
				NdisMoveMemory(&SrcPort, Pos, 2);
			    SrcPort = OS_NTOHS(SrcPort);
				NdisMoveMemory(&DstPort, Pos+2, 2);
			    DstPort = OS_NTOHS(DstPort);

				if ((SrcPort == 546) && (DstPort == 547))
				{
					DBGPRINT(RT_DEBUG_OFF, ("The Packet is DHCPv6 Solicitation,msg type=%d\n", *type));
					return TRUE;
				}
			}
        }
    }

    return FALSE;
}

BOOLEAN IsIPv6RouterSolicitation(IN PRTMP_ADAPTER pAd,
								 IN PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;

	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;
	
	if (ProtoType == ETH_P_IPV6)
	{		
		INT32 	PayloadLen = 0; //((*(Pos+5) & 0xff) << 8) | (*(Pos+4) & 0xff);
		UCHAR 	NextHeader = *(Pos+6);
		UCHAR 	IsExtenHeader = 0;
		
		NdisMoveMemory(&PayloadLen, (Pos+4), 2);
		PayloadLen = OS_NTOHS(PayloadLen);
		
		if ((NextHeader != IP_PROTO_HOPOPTS) && (NextHeader != IP_PROTO_ROUTING) &&  (NextHeader != IP_PROTO_FRAGMENT) &&  (NextHeader != IP_PROTO_AH) && (NextHeader != IP_PROTO_DSTOPTS))             
		{
			Pos += 40;
		}
		else
		{
			IsExtenHeader = 1;
		Pos += 40;

			do
			{
				printk("IsIPv6RouterSolicitation: nextheader=0x%x, %d, %d\n", NextHeader, PayloadLen, IsExtenHeader);
				switch (NextHeader)
				{
					case IP_PROTO_HOPOPTS:
					case IP_PROTO_ROUTING:
					case IP_PROTO_DSTOPTS:	
						{
							UCHAR HdrExtLen = *(Pos+1);
							
							NextHeader = *Pos;
							PayloadLen -= ((HdrExtLen+1)<<3);
							Pos += ((HdrExtLen+1)<<3);
						}
						break;
					case IP_PROTO_FRAGMENT:
						{
							NextHeader = *Pos;
							PayloadLen -= 8;
							Pos += 8;
						}
						break;	
					case IP_PROTO_AH:
						{
							UCHAR AHPayloadLen = *(Pos+1);
							UCHAR AHLen = (8+(AHPayloadLen<<2));
							
							NextHeader = *Pos;
							PayloadLen -= AHLen;
							Pos += AHLen;
						}
						break;	
					default:
						IsExtenHeader = 0;
						break;
				}
			}
			while((PayloadLen > 0) && (IsExtenHeader == 1));
			
			if (PayloadLen <= 0)
				return FALSE;
		}
		

		/* Check if router solicitation */
		if (*Pos == 0x85)
		{
			DBGPRINT(RT_DEBUG_OFF, ("The Packet is IPv6 Router Solicitation\n"));
			return TRUE;
		}
	}

	return FALSE;
}


BOOLEAN IsIPv6RouterAdvertisement(IN PRTMP_ADAPTER pAd,								  
									IN PUCHAR pData,
									IN PUCHAR pOffset)								  
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	UCHAR	*pData_offset = NULL;

	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_IPV6)
	{
		
		INT32 	PayloadLen = 0;
		UCHAR 	NextHeader = *(Pos+6);
		UCHAR 	IsExtenHeader = 0;
		
		NdisMoveMemory(&PayloadLen, (Pos+4), 2);
		PayloadLen = OS_NTOHS(PayloadLen);
		
		if ((NextHeader != IP_PROTO_HOPOPTS) && 
			(NextHeader != IP_PROTO_ROUTING) &&  
			(NextHeader != IP_PROTO_FRAGMENT) &&  
			(NextHeader != IP_PROTO_AH) && 
			(NextHeader != IP_PROTO_DSTOPTS))             
		{
			Pos += 40;
			pData_offset = Pos;
		}
		else
		{	
			IsExtenHeader = 1;
			Pos += 40;
			pData_offset = Pos;
			
			do
			{
				DBGPRINT(RT_DEBUG_TRACE, ("IsIPv6RouterAdvertisement: nextheader=0x%x, %d, %d\n", NextHeader, PayloadLen, IsExtenHeader));
				switch (NextHeader)
				{
					case IP_PROTO_HOPOPTS:
					case IP_PROTO_ROUTING:
					case IP_PROTO_DSTOPTS:	
						{
							UCHAR HdrExtLen = *(Pos+1);
							
							NextHeader = *Pos;
							PayloadLen -= ((HdrExtLen+1)<<3);
							Pos += ((HdrExtLen+1)<<3);
						}
						break;
					case IP_PROTO_FRAGMENT:
						{
							NextHeader = *Pos;
							PayloadLen -= 8;
							Pos += 8;
						}
						break;	
					case IP_PROTO_AH:
						{
							UCHAR AHPayloadLen = *(Pos+1);
							UCHAR AHLen = (8+(AHPayloadLen<<2));
							
							NextHeader = *Pos;
							PayloadLen -= AHLen;
							Pos += AHLen;
						}
						break;	
					default:
						IsExtenHeader = 0;
						break;
				}
			}
			while((PayloadLen > 0) && (IsExtenHeader == 1));
			
			if (PayloadLen <= 0)
				return FALSE;
		}
		
		
		/* Check if router advertisement */
		if (*Pos == 0x86)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("The Packet is IPv6 Router Advertisement\n"));			
			*pOffset = Pos-pData_offset;
			return TRUE;
		}
	}

	return FALSE;
}


BOOLEAN IsTDLSPacket(IN PRTMP_ADAPTER pAd,
					 IN PUCHAR pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	
	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == 0x890d)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("THe Packet is TDLS\n"));
		return TRUE;
	}

	return FALSE;
}


UINT32 IPv4ProxyARPTableLen(IN PRTMP_ADAPTER pAd,
							IN struct _MULTISSID_STRUCT *pMbss)
{

	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	UINT32 TableLen = 0;
	INT32 Ret;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
		TableLen += sizeof(PROXY_ARP_IPV4_UNIT);
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	return TableLen;
}

UINT32 IPv6ProxyARPTableLen(IN PRTMP_ADAPTER pAd,
							IN struct _MULTISSID_STRUCT *pMbss)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
	UINT32 TableLen = 0;
	INT32 Ret;

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
		TableLen += sizeof(PROXY_ARP_IPV6_UNIT);
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);

	return TableLen;
}

BOOLEAN GetIPv4ProxyARPTable(IN PRTMP_ADAPTER pAd,
						 	 IN struct _MULTISSID_STRUCT *pMbss,
						 	 PUCHAR *ProxyARPTable)
{

	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	PROXY_ARP_IPV4_UNIT *ProxyARPUnit = (PROXY_ARP_IPV4_UNIT *)(*ProxyARPTable);
	INT32 Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
			NdisMoveMemory(ProxyARPUnit->TargetMACAddr, ProxyARPEntry->TargetMACAddr, MAC_ADDR_LEN);
			NdisMoveMemory(ProxyARPUnit->TargetIPAddr, ProxyARPEntry->TargetIPAddr, 4);		
			ProxyARPUnit++; 
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	return TRUE;
}

BOOLEAN GetIPv6ProxyARPTable(IN PRTMP_ADAPTER pAd,
						 	 IN struct _MULTISSID_STRUCT *pMbss,
						 	 PUCHAR *ProxyARPTable)
{

	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
	PROXY_ARP_IPV6_UNIT *ProxyARPUnit = (PROXY_ARP_IPV6_UNIT *)(*ProxyARPTable);
	INT32 Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
			NdisMoveMemory(ProxyARPUnit->TargetMACAddr, ProxyARPEntry->TargetMACAddr, MAC_ADDR_LEN);
			ProxyARPUnit->TargetIPType = ProxyARPEntry->TargetIPType;
			NdisMoveMemory(ProxyARPUnit->TargetIPAddr, ProxyARPEntry->TargetIPAddr, 16);
			ProxyARPUnit++; 
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);

	return TRUE;
}

UINT32 AddIPv4ProxyARPEntry(IN PRTMP_ADAPTER pAd,
					   		IN MULTISSID_STRUCT *pMbss,
							PUCHAR pTargetMACAddr,
							PUCHAR pTargetIPAddr)
{
	
	int i = 0, find_list = 0;	
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	INT32 Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	if ((pTargetIPAddr[0] == 0) && (pTargetIPAddr[1] == 0)) {
		DBGPRINT(RT_DEBUG_ERROR, ("Drop invalid IP Addr:%d.%d.%d.%d\n", pTargetIPAddr[0], pTargetIPAddr[1], pTargetIPAddr[2], pTargetIPAddr[3]));
		return FALSE;
	}

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(ProxyARPEntry->TargetMACAddr, pTargetMACAddr))
		{			
			//RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
			//return FALSE;
			find_list = 1;
			break;			
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
	
	if (find_list == 0)
		os_alloc_mem(NULL, (UCHAR **)&ProxyARPEntry, sizeof(*ProxyARPEntry));
	
	if (!ProxyARPEntry)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return FALSE;
	}

	NdisMoveMemory(ProxyARPEntry->TargetMACAddr, pTargetMACAddr, 6);
	NdisMoveMemory(ProxyARPEntry->TargetIPAddr, pTargetIPAddr, 4);

	for (i = 0; i < 4; i++)
		printk("pTargetIPv4Addr[%i] = %x\n", i, pTargetIPAddr[i]);
	
	/* Add ProxyARP Entry to list */	
	if (find_list == 0) 
	{
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
		DlListAddTail(&pWNMCtrl->IPv4ProxyARPList, &ProxyARPEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
	}
	

	return TRUE;
}

VOID RemoveIPv4ProxyARPEntry(IN PRTMP_ADAPTER pAd,
					   		IN MULTISSID_STRUCT *pMbss,
							PUCHAR pTargetMACAddr)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry, *ProxyARPEntryTmp;
	INT32 Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	
	DlListForEachSafe(ProxyARPEntry, ProxyARPEntryTmp, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
		if (!ProxyARPEntry)
			break;
			
		if (MAC_ADDR_EQUAL(ProxyARPEntry->TargetMACAddr, pTargetMACAddr))
		{
			//RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
			//return FALSE;
			DlListDel(&ProxyARPEntry->List);
			os_free_mem(NULL, ProxyARPEntry);
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
}

UINT32 AddIPv6ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							IN MULTISSID_STRUCT *pMbss,
							PUCHAR pTargetMACAddr,
							PUCHAR pTargetIPAddr)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
	INT32 Ret;
	UINT8 i;
	BOOLEAN IsDAD = FALSE;
	PNET_DEV NetDev = pMbss->wdev.if_dev;
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(ProxyARPEntry->TargetMACAddr, pTargetMACAddr) &&
			IPV6_ADDR_EQUAL(ProxyARPEntry->TargetIPAddr, pTargetIPAddr))
		{
			
			DBGPRINT(RT_DEBUG_OFF, ("%s, the Mac address and IPv6 Address exactly same as the one in List already!\n", __FUNCTION__));
			RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);
			return FALSE;
		}
			
		if ((MAC_ADDR_EQUAL(ProxyARPEntry->TargetMACAddr, pTargetMACAddr) == FALSE) &&
			IPV6_ADDR_EQUAL(ProxyARPEntry->TargetIPAddr, pTargetIPAddr))
		{
			DBGPRINT(RT_DEBUG_OFF, ("%s, different Mac address use IPv6 address which already in List!\n", __func__));
			RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);
			IsDAD = TRUE;
		
			/*we got IPv6 DAD here, AP shall issue Neighbor Advertisement back to sender in below format */
			/* SenderMAC is the mac which in List, */
			/* DestMAC is multicast address e.g, 33:33:00:00:00:01 */
			/* SourceIP = TargetIP = TentativeIP, in this case is the IP which in List */
			/* DestIP = FF02::1 */
			SendProxyARPEvent(NetDev,
						  pTargetMACAddr,
						  ProxyARPEntry->TargetIPAddr,
						  ProxyARPEntry->TargetMACAddr,
						  ProxyARPEntry->TargetIPAddr,
						  IPV6,
						  FALSE,
						  IsDAD);
						  
			return FALSE;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);
	
	os_alloc_mem(NULL, (UCHAR **)&ProxyARPEntry, sizeof(*ProxyARPEntry));
	
	if (!ProxyARPEntry)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return FALSE;
	}

	NdisMoveMemory(ProxyARPEntry->TargetMACAddr, pTargetMACAddr, 6);

	if (NdisEqualMemory(link_local, pTargetIPAddr, 2))
		ProxyARPEntry->TargetIPType = IPV6_LINK_LOCAL; 
	else
		ProxyARPEntry->TargetIPType = IPV6_GLOBAL;
 
	NdisMoveMemory(ProxyARPEntry->TargetIPAddr, pTargetIPAddr, 16);

	for (i = 0; i < 6; i++)
		printk("pTargetMACAddr[%i] = %x\n", i, pTargetMACAddr[i]);

	for (i = 0; i < 16; i++)
		printk("pTargetIPv6Addr[%i] = %x\n", i, pTargetIPAddr[i]);
	
	/* Add ProxyARP Entry to list */
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	DlListAddTail(&pWNMCtrl->IPv6ProxyARPList, &ProxyARPEntry->List);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);

	return TRUE;
}

VOID RemoveIPv6ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							IN MULTISSID_STRUCT *pMbss,
							PUCHAR pTargetMACAddr)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry, *ProxyARPEntryTmp;
	INT32 Ret;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);

	DlListForEachSafe(ProxyARPEntry, ProxyARPEntryTmp, &pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
		if (!ProxyARPEntry)
			break;
			
		if (MAC_ADDR_EQUAL(ProxyARPEntry->TargetMACAddr, pTargetMACAddr))
		{
			DlListDel(&ProxyARPEntry->List);
			os_free_mem(NULL, ProxyARPEntry);
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);
}

BOOLEAN IPv4ProxyARP(IN PRTMP_ADAPTER pAd,
				 	 IN MULTISSID_STRUCT *pMbss,
				 	 IN PUCHAR pData,
					 IN BOOLEAN FromDS)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PNET_DEV NetDev = pMbss->wdev.if_dev;	
	BOOLEAN IsFound = FALSE, InTable = FALSE;	
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;	

	PUCHAR SourceMACAddr = pData + 10; 
	PUCHAR SourceIPAddr = pData + 16;
	PUCHAR TargetIPAddr = pData + 26;
	INT32 Ret;	
	BOOLEAN IsDAD = FALSE;
	PUCHAR TargetMACAddr = pData + 20;
	UCHAR ALL_ZERO_BROADCAST_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
		if (IPV4_ADDR_EQUAL(ProxyARPEntry->TargetIPAddr, TargetIPAddr))
		{
			IsFound = TRUE;
			if (
			(MAC_ADDR_EQUAL(ProxyARPEntry->TargetMACAddr, SourceMACAddr) == FALSE) &&
			((MAC_ADDR_EQUAL(TargetMACAddr, BROADCAST_ADDR) == TRUE) ||
			(MAC_ADDR_EQUAL(TargetMACAddr, ALL_ZERO_BROADCAST_ADDR) == TRUE)) &&
			(IPV4_ADDR_EQUAL(SourceIPAddr, ZERO_IP_ADDR) == TRUE)
			) {
				//Mac address is not equal to the one which already  in List.
				//it's a DAD arp.
				IsDAD = TRUE;
				DBGPRINT(RT_DEBUG_TRACE, ("%s, Found DAD!!!!\n", __FUNCTION__));
				printk("found dad...\n");
			}
			
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);	
	
	if (IsFound)
	{
		/* ARP Probe and ARP Entry already Build and not DAD */
		if ((IsDAD == FALSE) && (IPV4_ADDR_EQUAL(SourceIPAddr, ZERO_IP_ADDR) == TRUE))
			return IsFound;

		/* Send proxy arp indication to daemon */
		SendProxyARPEvent(NetDev,
						  SourceMACAddr,
						  SourceIPAddr,
						  ProxyARPEntry->TargetMACAddr,
						  ProxyARPEntry->TargetIPAddr,
						  IPV4,
						  FromDS,
						  IsDAD);

		if ((IsDAD == FALSE) && (FromDS == FALSE)) {
			RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
            DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
            {
                if (IPV4_ADDR_EQUAL(ProxyARPEntry->TargetIPAddr, SourceIPAddr))
                {
                    InTable = TRUE;
    	            break;
	            }
            }
            RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
            if (InTable == FALSE) {
				AddIPv4ProxyARPEntry(pAd, pMbss, SourceMACAddr, SourceIPAddr);
                DBGPRINT(RT_DEBUG_ERROR,
                ("%s, New Station take arp request, Learning ARP Entry from it\n", __func__));
			}
		}
	}
	else 
	{
		if (
		((MAC_ADDR_EQUAL(TargetMACAddr, BROADCAST_ADDR) == TRUE) ||
		(MAC_ADDR_EQUAL(TargetMACAddr, ALL_ZERO_BROADCAST_ADDR) == TRUE)) && (FromDS == FALSE)
		) {
		//waht if there is a new mac for List, and take BOARDCAST and ZERO_IP, 
		//it's a station take DAD packet to ask the network.
		//In this case, AP shall learn the mac/ip mapping from it.
			if (IPV4_ADDR_EQUAL(SourceIPAddr, ZERO_IP_ADDR) == TRUE) {
				AddIPv4ProxyARPEntry(pAd, pMbss, SourceMACAddr, TargetIPAddr);
				DBGPRINT(RT_DEBUG_ERROR, 
				("%s, New Station take DAD to detect, Learning ARP Entry from it\n", __func__));
			}
			else {
				RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
				DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
			    {
        			if (IPV4_ADDR_EQUAL(ProxyARPEntry->TargetIPAddr, SourceIPAddr))
        			{
            			InTable = TRUE;
						break;
					}
				}
				RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
				if (InTable == FALSE) {
					AddIPv4ProxyARPEntry(pAd, pMbss, SourceMACAddr, SourceIPAddr);
					DBGPRINT(RT_DEBUG_ERROR, 
					("%s, New Station take arp request to detect, Learning ARP Entry from it\n", __func__));
				}
			}	
		}
	}	
	
	return IsFound;
}

BOOLEAN IPv6ProxyARP(IN PRTMP_ADAPTER pAd,
					 IN MULTISSID_STRUCT *pMbss,
					 IN PUCHAR pData,
					 IN BOOLEAN FromDS)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PNET_DEV NetDev = pMbss->wdev.if_dev;
	BOOLEAN IsFound = FALSE;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
	PUCHAR SourceMACAddr = pData + 68; 
	PUCHAR SourceIPAddr = pData + 10;
	PUCHAR TargetIPAddr = pData + 50;
	INT32 Ret;

	
	BOOLEAN IsDAD = FALSE;

	//DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
		if (IPV6_ADDR_EQUAL(ProxyARPEntry->TargetIPAddr, TargetIPAddr))
		{
			IsFound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	if (IsFound)
	{
	
		DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
		/* Send proxy arp indication to daemon */
		SendProxyARPEvent(NetDev,
						  SourceMACAddr,
						  SourceIPAddr,
						  ProxyARPEntry->TargetMACAddr,
						  ProxyARPEntry->TargetIPAddr,
						  IPV6,
						  FromDS,
						  IsDAD);
	}

	return IsFound;
}


VOID WNMIPv4ProxyARPCheck(
			IN PRTMP_ADAPTER pAd,
			PNDIS_PACKET pPacket,
			USHORT srcPort,
			USHORT dstPort,
			PUCHAR pSrcBuf)
{
	UCHAR apidx = RTMP_GET_PACKET_NET_DEVICE(pPacket);
	MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];	
	MAC_TABLE_ENTRY  *pEntry;	
	
	if (srcPort  == 0x43 && dstPort == 0x44)
	{
		UCHAR *pTargetIPAddr = pSrcBuf + 24;
		/* Client hardware address */
		UCHAR *pTargetMACAddr = pSrcBuf + 36;						
		
		if ((pMbss->WNMCtrl.ProxyARPEnable) && (pEntry = MacTableLookup(pAd, pTargetMACAddr)))
		{
			printk("entry apidx=%d,%d,%d\n",pEntry->apidx, apidx, pMbss->WNMCtrl.ProxyARPEnable);

			if ((pEntry->apidx == apidx) && pMbss->WNMCtrl.ProxyARPEnable)
			{
				/* Proxy MAC address/IP mapping */
				AddIPv4ProxyARPEntry(pAd, pMbss, pTargetMACAddr, pTargetIPAddr);
			}
		}
		
	}
}


VOID WNMIPv6ProxyARPCheck(
			IN PRTMP_ADAPTER pAd,
			PNDIS_PACKET pPacket,
			PUCHAR pSrcBuf)
{
	UCHAR apidx = RTMP_GET_PACKET_NET_DEVICE(pPacket);
	MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	UCHAR Offset = 0;
	
	if (pMbss->WNMCtrl.ProxyARPEnable)
	{
		/* Check if router advertisement, and add proxy entry */
		
		if (IsIPv6RouterAdvertisement(pAd, pSrcBuf - 2, &Offset))		
		{
			UCHAR *Pos = pSrcBuf + 4;
			UCHAR TargetIPAddr[16];			
			INT32 PayloadLen; 			
			DBGPRINT(RT_DEBUG_OFF, ("This packet is router advertisement\n"));

			NdisMoveMemory(&PayloadLen, Pos, 2);
			PayloadLen = OS_NTOHS(PayloadLen);

			printk("WNMIPv6ProxyARPCheck: offset=%d\n", Offset);
			/* IPv6 options */
			Pos += 52 + Offset;
			PayloadLen -= (16 + Offset);


			while (PayloadLen > 0)
			{
				UINT8 OptionsLen = (*(Pos + 1)) * 8;

				/* Prefix information */
				if (*Pos == 0x03)
				{
					UCHAR *Prefix;
					INT32 Ret;
					PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
					PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;

					/* Prefix */
					Prefix = (Pos + 16);
								
					/* Copy global address prefix */
					NdisMoveMemory(TargetIPAddr, Prefix, 8);

					RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
					DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv6ProxyARPList, 
										PROXY_ARP_IPV6_ENTRY, List)
					{
						if (ProxyARPEntry->TargetIPType == IPV6_LINK_LOCAL)
						{

							/* Copy host ipv6 interface identifier */
							NdisMoveMemory(&TargetIPAddr[8], 
											&ProxyARPEntry->TargetIPAddr[8], 8);

							/* Proxy MAC address/IPv6 mapping for global address */
							AddIPv6ProxyARPEntry(pAd, pMbss, ProxyARPEntry->TargetMACAddr, 
														TargetIPAddr);
						}
					}	
					RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
				}

				Pos += OptionsLen;
				PayloadLen -= OptionsLen;
			}
		}
	}
}

static VOID ReceiveBTMQuery(IN PRTMP_ADAPTER pAd,
							IN MLME_QUEUE_ELEM *Elem)
{
	BTM_EVENT_DATA *Event;	
	WNM_FRAME *WNMFrame = (WNM_FRAME *)Elem->Msg;
	BTM_PEER_ENTRY *BTMPeerEntry;
	PWNM_CTRL pWNMCtrl = NULL;
	UCHAR APIndex, *Buf;
	UINT16 VarLen;
	UINT32 Len = 0;
	INT32 Ret;
	BOOLEAN IsFound = FALSE;

	printk("%s\n", __FUNCTION__);

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		if (MAC_ADDR_EQUAL(WNMFrame->Hdr.Addr3, pAd->ApCfg.MBSSID[APIndex].wdev.bssid))
		{
			pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
			break;
		}
	}

	if (!pWNMCtrl)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Can not find Peer Control\n", __FUNCTION__));
		return;
	}
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, WNMFrame->Hdr.Addr2))
			IsFound = TRUE;
		
		break;
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
	
	if (IsFound) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s Find peer address in BTMPeerList already\n", __FUNCTION__));
		return;
	}
	
	os_alloc_mem(NULL, (UCHAR **)&BTMPeerEntry, sizeof(*BTMPeerEntry));

	if (!BTMPeerEntry)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return;
	}

	NdisZeroMemory(BTMPeerEntry, sizeof(*BTMPeerEntry));
	BTMPeerEntry->CurrentState = WAIT_PEER_BTM_QUERY;

	NdisMoveMemory(BTMPeerEntry->PeerMACAddr, WNMFrame->Hdr.Addr2, MAC_ADDR_LEN);
	BTMPeerEntry->DialogToken = WNMFrame->u.BTM_QUERY.DialogToken;
	BTMPeerEntry->Priv = pAd;

	RTMPInitTimer(pAd, &BTMPeerEntry->WaitPeerBTMRspTimer,
				 GET_TIMER_FUNCTION(WaitPeerBTMRspTimeout), BTMPeerEntry, FALSE);

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListAddTail(&pWNMCtrl->BTMPeerList, &BTMPeerEntry->List);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	VarLen = *(WNMFrame->u.BTM_QUERY.Variable + 2);
	VarLen += 3;
	
	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + VarLen);

	if (!Buf)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		goto error;
	}

	NdisZeroMemory(Buf, sizeof(*Event) + VarLen);

	Event = (BTM_EVENT_DATA *)Buf;
	
	Event->ControlIndex = APIndex;
	Len += 1;
	
	NdisMoveMemory(Event->PeerMACAddr, WNMFrame->Hdr.Addr2, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;
	
	Event->EventType = PEER_BTM_QUERY;
	Len += 2;

	Event->u.PEER_BTM_QUERY_DATA.DialogToken = WNMFrame->u.BTM_QUERY.DialogToken;
 	Len += 1;

	/* FIXME */
	//Event->u.PEER_BTM_QUERY_DATA.BTMQueryLen;
	Len += 2;

	NdisMoveMemory(Event->u.PEER_BTM_QUERY_DATA.BTMQuery, WNMFrame->u.BTM_QUERY.Variable, 
							VarLen);
	Len += VarLen;
	
	MlmeEnqueue(pAd, BTM_STATE_MACHINE, PEER_BTM_QUERY, Len, Buf,0); 
	
	os_free_mem(NULL, Buf);

	return;

error:
	DlListDel(&BTMPeerEntry->List);
	os_free_mem(NULL, BTMPeerEntry);
}


static VOID ReceiveBTMRsp(IN PRTMP_ADAPTER pAd,
						  IN MLME_QUEUE_ELEM *Elem)
{

	BTM_EVENT_DATA *Event;	
	WNM_FRAME *WNMFrame = (WNM_FRAME *)Elem->Msg;
	BTM_PEER_ENTRY *BTMPeerEntry;
	PWNM_CTRL pWNMCtrl = NULL;
	UCHAR APIndex, *Buf;
	UINT16 VarLen = 0;
	UINT32 Len = 0;
	INT32 Ret;
	BOOLEAN IsFound = FALSE, Cancelled;

	printk("%s\n", __FUNCTION__);

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		if (MAC_ADDR_EQUAL(WNMFrame->Hdr.Addr3, pAd->ApCfg.MBSSID[APIndex].wdev.bssid))
		{
			pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
			break;
		}
	}

	if (!pWNMCtrl)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Can not find Peer Control\n", __FUNCTION__));
		return;
	}
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, WNMFrame->Hdr.Addr2))
			IsFound = TRUE;
		
		break;
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	if (!IsFound) {
		DBGPRINT(RT_DEBUG_ERROR, ("Not found peer entry in list\n"));
		return;
	}

	/* Cancel Wait peer wnm response frame */
	RTMPCancelTimer(&BTMPeerEntry->WaitPeerBTMRspTimer, &Cancelled);
	RTMPReleaseTimer(&BTMPeerEntry->WaitPeerBTMRspTimer, &Cancelled);

	VarLen = 2;

	if (*WNMFrame->u.BTM_RSP.Variable == 0)
		VarLen += 6;

	VarLen += *(WNMFrame->u.BTM_RSP.Variable + VarLen + 1);
	VarLen += 2;
	
	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + VarLen);

	if (!Buf)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return;
	}

	NdisZeroMemory(Buf, sizeof(*Event) + VarLen);

	Event = (BTM_EVENT_DATA *)Buf;
	
	Event->ControlIndex = APIndex;
	Len += 1;
	
	NdisMoveMemory(Event->PeerMACAddr, WNMFrame->Hdr.Addr2, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;
	
	Event->EventType = PEER_BTM_RSP;
	Len += 2;

	Event->u.PEER_BTM_RSP_DATA.DialogToken = WNMFrame->u.BTM_RSP.DialogToken;
 	Len += 1;

	/* FIXME */
	//Event->u.PEER_BTM_RSP_DATA.BTMRspLen;
	Len += 2;

	NdisMoveMemory(Event->u.PEER_BTM_RSP_DATA.BTMRsp, WNMFrame->u.BTM_RSP.Variable, 
							VarLen);
	Len += VarLen;
	
	MlmeEnqueue(pAd, BTM_STATE_MACHINE, PEER_BTM_RSP, Len, Buf,0); 
	
	os_free_mem(NULL, Buf);

	return;
}


VOID BTMSetPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN enum BTM_STATE State)
{
	PWNM_CTRL pWNMCtrl;
	PBTM_PEER_ENTRY BTMPeerEntry;
	PBTM_EVENT_DATA Event = (PBTM_EVENT_DATA)Elem->Msg;
	INT32 Ret;
	
#ifdef CONFIG_AP_SUPPORT
	pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList,
							BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			BTMPeerEntry->CurrentState = State;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
}

static VOID SendBTMQueryIndication(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem)
{

	BTM_EVENT_DATA *Event = (BTM_EVENT_DATA *)Elem->Msg;
	PNET_DEV NetDev = pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.if_dev;

	printk("%s\n", __FUNCTION__);

	/* Send BTM query indication to daemon */
	SendBTMQueryEvent(NetDev,
					  Event->PeerMACAddr,
					  Event->u.PEER_BTM_QUERY_DATA.BTMQuery,
					  Event->u.PEER_BTM_QUERY_DATA.BTMQueryLen,
					  RA_WEXT);
	
	BTMSetPeerCurrentState(pAd, Elem, WAIT_BTM_REQ); 
}


VOID WaitPeerBTMRspTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	BTM_PEER_ENTRY *BTMPeerEntry = (BTM_PEER_ENTRY *)FunctionContext;
	PRTMP_ADAPTER pAd;
	PWNM_CTRL pWNMCtrl;
	INT32 Ret;
	BOOLEAN Cancelled;
	
	printk("%s\n", __FUNCTION__);

	if (!BTMPeerEntry)
		return;

	pAd = BTMPeerEntry->Priv;

	RTMPReleaseTimer(&BTMPeerEntry->WaitPeerBTMRspTimer, &Cancelled);
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS
							| fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	pWNMCtrl = &pAd->ApCfg.MBSSID[BTMPeerEntry->ControlIndex].WNMCtrl;
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListDel(&BTMPeerEntry->List);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
	os_free_mem(NULL, BTMPeerEntry);
}
BUILD_TIMER_FUNCTION(WaitPeerBTMRspTimeout);


static VOID SendBTMReq(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM  *Elem)
{
	BTM_EVENT_DATA *Event = (BTM_EVENT_DATA *)Elem->Msg;
	UCHAR *Buf;
	WNM_FRAME *WNMFrame;
	BTM_PEER_ENTRY *BTMPeerEntry = NULL;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
	UINT32 FrameLen = 0, VarLen = Event->u.BTM_REQ_DATA.BTMReqLen;
	INT32 Ret;

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList,
						BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*WNMFrame) + VarLen);
	
	if (!Buf)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return;
	}

	NdisZeroMemory(Buf, sizeof(*WNMFrame) + VarLen);

	WNMFrame = (WNM_FRAME *)Buf;

	ActHeaderInit(pAd, &WNMFrame->Hdr, Event->PeerMACAddr, 
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid);

	FrameLen += sizeof(HEADER_802_11);
	
	WNMFrame->Category = CATEGORY_WNM;
	FrameLen += 1;

	WNMFrame->u.BTM_REQ.Action = BSS_TRANSITION_REQ;
	FrameLen += 1;

	WNMFrame->u.BTM_REQ.DialogToken = Event->u.BTM_REQ_DATA.DialogToken;
	FrameLen += 1;

	NdisMoveMemory(WNMFrame->u.BTM_REQ.Variable, Event->u.BTM_REQ_DATA.BTMReq,
					Event->u.BTM_REQ_DATA.BTMReqLen);
	FrameLen += Event->u.BTM_REQ_DATA.BTMReqLen;

	BTMSetPeerCurrentState(pAd, Elem, WAIT_PEER_BTM_RSP);
	
	MiniportMMRequest(pAd, 0, Buf, FrameLen);

	RTMPSetTimer(&BTMPeerEntry->WaitPeerBTMRspTimer, WaitPeerBTMRspTimeoutVale);

#ifdef CONFIG_HOTSPOT_R2
	{
		MAC_TABLE_ENTRY  *pEntry;
		
		if ((pEntry = MacTableLookup(pAd, Event->PeerMACAddr)) != NULL)
    	{	
    		UINT8 *BTMData = (UINT8 *)Event->u.BTM_REQ_DATA.BTMReq;
    		
    		pEntry->BTMDisassocCount = (((*(BTMData+2) << 8) | (*(BTMData+1)))*pAd->CommonCfg.BeaconPeriod)/1000;
    		printk("bss discount sec=%d\n", pEntry->BTMDisassocCount);
    		if (pEntry->BTMDisassocCount < 1)
    			pEntry->BTMDisassocCount = 1;
    	}
	}
#endif	

	os_free_mem(NULL, Buf);
}


static VOID SendBTMConfirm(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem)
{

	PBTM_PEER_ENTRY BTMPeerEntry, BTMPeerEntryTmp;
	BTM_EVENT_DATA *Event = (BTM_EVENT_DATA *)Elem->Msg;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
	PNET_DEV NetDev = pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.if_dev;
	INT32 Ret;

	printk("%s\n", __FUNCTION__);

	/* Send BTM confirm to daemon */
	SendBTMConfirmEvent(NetDev,
						Event->PeerMACAddr,
						Event->u.PEER_BTM_RSP_DATA.BTMRsp,
						Event->u.PEER_BTM_RSP_DATA.BTMRspLen,
						RA_WEXT);

	/* Delete BTM peer entry */
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEachSafe(BTMPeerEntry, BTMPeerEntryTmp, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			
			DlListDel(&BTMPeerEntry->List);
			os_free_mem(NULL, BTMPeerEntry);
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
static VOID ReceiveBTMReq(IN PRTMP_ADAPTER pAd,
						  IN MLME_QUEUE_ELEM *Elem)
{



}


static VOID SendBTMQuery(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem)
{


}


static VOID SendBTMIndication(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem)
{


}


static VOID SendBTMRsp(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem)
{


}
#endif /* CONFIG_STA_SUPPORT */

enum BTM_STATE BTMPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PWNM_CTRL pWNMCtrl;
	PBTM_PEER_ENTRY BTMPeerEntry;
	PBTM_EVENT_DATA Event = (PBTM_EVENT_DATA)Elem->Msg;
	INT32 Ret;

#ifdef CONFIG_AP_SUPPORT
	pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			
			RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
			return BTMPeerEntry->CurrentState;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	return BTM_UNKNOWN;
}



void PeerWNMAction(IN PRTMP_ADAPTER pAd,
				   IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

	switch(Action)
	{	
#ifdef CONFIG_AP_SUPPORT
		case BSS_TRANSITION_QUERY: 
			ReceiveBTMQuery(pAd, Elem);
			break;
		case BSS_TRANSITION_RSP:
			ReceiveBTMRsp(pAd, Elem);
			break;
#ifdef CONFIG_HOTSPOT_R2
		case WNM_NOTIFICATION_RSP:
			ReceiveWNMNotifyRsp(pAd, Elem);
			break;		
#endif
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		case BSS_TRANSITION_REQ:
			ReceiveBTMReq(pAd, Elem);
			break;
#endif /* CONFIG_STA_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_TRACE, ("Invalid action field = %d\n", Action));
			break;
	}
}

VOID WNMCtrlInit(IN PRTMP_ADAPTER pAd)
{
	PWNM_CTRL pWNMCtrl;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
		NdisZeroMemory(pWNMCtrl, sizeof(*pWNMCtrl));
		RTMP_SEM_EVENT_INIT(&pWNMCtrl->BTMPeerListLock, &pAd->RscSemMemList);
		RTMP_SEM_EVENT_INIT(&pWNMCtrl->ProxyARPListLock, &pAd->RscSemMemList);
		RTMP_SEM_EVENT_INIT(&pWNMCtrl->ProxyARPIPv6ListLock, &pAd->RscSemMemList);
		DlListInit(&pWNMCtrl->BTMPeerList);
		DlListInit(&pWNMCtrl->IPv4ProxyARPList);
		DlListInit(&pWNMCtrl->IPv6ProxyARPList);
#ifdef CONFIG_HOTSPOT_R2		
		RTMP_SEM_EVENT_INIT(&pWNMCtrl->WNMNotifyPeerListLock, &pAd->RscSemMemList);
		DlListInit(&pWNMCtrl->WNMNotifyPeerList);
#endif		

	}
#endif 
}


static VOID WNMCtrlRemoveAllIE(PWNM_CTRL pWNMCtrl)
{
	if (pWNMCtrl->TimeadvertisementIELen)
	{
		pWNMCtrl->TimeadvertisementIELen = 0;
		os_free_mem(NULL, pWNMCtrl->TimeadvertisementIE);
	}
	
	if (pWNMCtrl->TimezoneIELen)
	{
		pWNMCtrl->TimezoneIELen = 0;
		os_free_mem(NULL, pWNMCtrl->TimezoneIE);
	}
}


VOID WNMCtrlExit(IN PRTMP_ADAPTER pAd)
{
	PWNM_CTRL pWNMCtrl;
	UINT32 Ret;	
	BTM_PEER_ENTRY *BTMPeerEntry, *BTMPeerEntryTmp;
#ifdef CONFIG_AP_SUPPORT
	PROXY_ARP_IPV4_ENTRY *ProxyARPIPv4Entry, *ProxyARPIPv4EntryTmp;
	PROXY_ARP_IPV6_ENTRY *ProxyARPIPv6Entry, *ProxyARPIPv6EntryTmp;
	UCHAR APIndex;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
	WNM_NOTIFY_PEER_ENTRY *WNMNotifyPeerEntry, *WNMNotifyPeerEntryTmp;
#endif	


#ifdef CONFIG_AP_SUPPORT
	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
		
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);

		/* Remove all btm peer entry */
		DlListForEachSafe(BTMPeerEntry, BTMPeerEntryTmp,
							&pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List)
		{
			DlListDel(&BTMPeerEntry->List);
			os_free_mem(NULL, BTMPeerEntry);
		}

		DlListInit(&pWNMCtrl->BTMPeerList);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
		RTMP_SEM_EVENT_DESTORY(&pWNMCtrl->BTMPeerListLock);

		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
		/* Remove all proxy arp entry */
		DlListForEachSafe(ProxyARPIPv4Entry, ProxyARPIPv4EntryTmp,
							&pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
		{
			DlListDel(&ProxyARPIPv4Entry->List);
			os_free_mem(NULL, ProxyARPIPv4Entry);
		}
		DlListInit(&pWNMCtrl->IPv4ProxyARPList);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
		DlListForEachSafe(ProxyARPIPv6Entry, ProxyARPIPv6EntryTmp,
							&pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
		{
			DlListDel(&ProxyARPIPv6Entry->List);
			os_free_mem(NULL, ProxyARPIPv6Entry);
		}
		DlListInit(&pWNMCtrl->IPv6ProxyARPList);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);

		RTMP_SEM_EVENT_DESTORY(&pWNMCtrl->ProxyARPListLock);
		RTMP_SEM_EVENT_DESTORY(&pWNMCtrl->ProxyARPIPv6ListLock);
#ifdef CONFIG_HOTSPOT_R2
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
		
		/* Remove all wnm notify peer entry */
		DlListForEachSafe(WNMNotifyPeerEntry, WNMNotifyPeerEntryTmp,
							&pWNMCtrl->WNMNotifyPeerList, WNM_NOTIFY_PEER_ENTRY, List)
		{
			DlListDel(&WNMNotifyPeerEntry->List);
			os_free_mem(NULL, WNMNotifyPeerEntry);
		}

		DlListInit(&pWNMCtrl->WNMNotifyPeerList);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);
		RTMP_SEM_EVENT_DESTORY(&pWNMCtrl->WNMNotifyPeerListLock);
#endif

		/* Remove all WNM IEs */
		WNMCtrlRemoveAllIE(pWNMCtrl);
	}
#endif /* CONFIG_AP_SUPPORT */

}


#ifdef CONFIG_AP_SUPPORT
VOID Clear_All_PROXY_TABLE(IN PRTMP_ADAPTER pAd)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl;
	UINT32 Ret;
	PROXY_ARP_IPV4_ENTRY *ProxyARPIPv4Entry, *ProxyARPIPv4EntryTmp;
	PROXY_ARP_IPV6_ENTRY *ProxyARPIPv6Entry, *ProxyARPIPv6EntryTmp;
	
	pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	/* Remove all proxy arp entry */
	DlListForEachSafe(ProxyARPIPv4Entry, ProxyARPIPv4EntryTmp,
						&pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
		DlListDel(&ProxyARPIPv4Entry->List);
		os_free_mem(NULL, ProxyARPIPv4Entry);
	}
	DlListInit(&pWNMCtrl->IPv4ProxyARPList);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	DlListForEachSafe(ProxyARPIPv6Entry, ProxyARPIPv6EntryTmp,
						&pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
		DlListDel(&ProxyARPIPv6Entry->List);
		os_free_mem(NULL, ProxyARPIPv6Entry);
	}
	DlListInit(&pWNMCtrl->IPv6ProxyARPList);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);
}
#endif


VOID BTMStateMachineInit(
			IN	PRTMP_ADAPTER pAd, 
			IN	STATE_MACHINE *S, 
			OUT STATE_MACHINE_FUNC	Trans[])
{

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));


	StateMachineInit(S,	(STATE_MACHINE_FUNC*)Trans, MAX_BTM_STATE, MAX_BTM_MSG, (STATE_MACHINE_FUNC)Drop, BTM_UNKNOWN, BTM_MACHINE_BASE);

#ifdef CONFIG_AP_SUPPORT
	StateMachineSetAction(S, WAIT_PEER_BTM_QUERY, PEER_BTM_QUERY, (STATE_MACHINE_FUNC)SendBTMQueryIndication);
	StateMachineSetAction(S, WAIT_BTM_REQ, BTM_REQ, (STATE_MACHINE_FUNC)SendBTMReq);
	StateMachineSetAction(S, WAIT_PEER_BTM_RSP, PEER_BTM_RSP, (STATE_MACHINE_FUNC)SendBTMConfirm);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	StateMachineSetAction(S, WAIT_BTM_QUERY, BTM_QUERY, (STATE_MACHINE_FUNC)SendBTMQuery);
	StateMachineSetAction(S, WAIT_PEER_BTM_REQ, PEER_BTM_REQ, (STATE_MACHINE_FUNC)SendBTMIndication);
	StateMachineSetAction(S, WAIT_BTM_RSP,BTM_RSP, (STATE_MACHINE_FUNC)SendBTMRsp);
#endif /* CONFIG_STA_SUPPORT */
}

#ifdef CONFIG_HOTSPOT_R2
#ifdef CONFIG_AP_SUPPORT
VOID WNMSetPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN enum WNM_NOTIFY_STATE State)
{
	PWNM_CTRL pWNMCtrl;
	PWNM_NOTIFY_PEER_ENTRY WNMNotifyPeerEntry;
	PWNM_NOTIFY_EVENT_DATA Event = (PWNM_NOTIFY_EVENT_DATA)Elem->Msg;
	INT32 Ret;
	
#ifdef CONFIG_AP_SUPPORT
	pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
	DlListForEach(WNMNotifyPeerEntry, &pWNMCtrl->WNMNotifyPeerList,
							WNM_NOTIFY_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(WNMNotifyPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			WNMNotifyPeerEntry->CurrentState = State;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);
}

enum WNM_NOTIFY_STATE WNMNotifyPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PWNM_CTRL pWNMCtrl;
	PWNM_NOTIFY_PEER_ENTRY WNMNotifyPeerEntry;
	PWNM_NOTIFY_EVENT_DATA Event = (PWNM_NOTIFY_EVENT_DATA)Elem->Msg;
	INT32 Ret;

#ifdef CONFIG_AP_SUPPORT
	pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
	DlListForEach(WNMNotifyPeerEntry, &pWNMCtrl->WNMNotifyPeerList, WNM_NOTIFY_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(WNMNotifyPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			
			RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);
			return WNMNotifyPeerEntry->CurrentState;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);

	return WNM_NOTIFY_UNKNOWN;
}

VOID WaitPeerWNMNotifyRspTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	WNM_NOTIFY_PEER_ENTRY *WNMNotifyPeerEntry = (WNM_NOTIFY_PEER_ENTRY *)FunctionContext;
	PRTMP_ADAPTER pAd;
	PWNM_CTRL pWNMCtrl;
	INT32 Ret;
	BOOLEAN Cancelled;
	
	printk("%s\n", __FUNCTION__);

	if (!WNMNotifyPeerEntry)
		return;

	pAd = WNMNotifyPeerEntry->Priv;

	RTMPReleaseTimer(&WNMNotifyPeerEntry->WaitPeerWNMNotifyRspTimer, &Cancelled);
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS
							| fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	pWNMCtrl = &pAd->ApCfg.MBSSID[WNMNotifyPeerEntry->ControlIndex].WNMCtrl;
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
	DlListDel(&WNMNotifyPeerEntry->List);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);
	os_free_mem(NULL, WNMNotifyPeerEntry);
}
BUILD_TIMER_FUNCTION(WaitPeerWNMNotifyRspTimeout);
 
VOID ReceiveWNMNotifyRsp(IN PRTMP_ADAPTER pAd,
						  IN MLME_QUEUE_ELEM *Elem)
{

	WNM_NOTIFY_EVENT_DATA *Event;	
	WNM_FRAME *WNMFrame = (WNM_FRAME *)Elem->Msg;
	WNM_NOTIFY_PEER_ENTRY *WNMNotifyPeerEntry;
	PWNM_CTRL pWNMCtrl = NULL;
	UCHAR APIndex, *Buf;
	UINT16 VarLen = 0;
	UINT32 Len = 0;
	INT32 Ret;
	BOOLEAN IsFound = FALSE, Cancelled;

	printk("%s\n", __FUNCTION__);

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		if (MAC_ADDR_EQUAL(WNMFrame->Hdr.Addr3, pAd->ApCfg.MBSSID[APIndex].wdev.bssid))
		{
			pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
			break;
		}
	}

	if (!pWNMCtrl)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Can not find Peer Control\n", __FUNCTION__));
		return;
	}
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
	DlListForEach(WNMNotifyPeerEntry, &pWNMCtrl->WNMNotifyPeerList, WNM_NOTIFY_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(WNMNotifyPeerEntry->PeerMACAddr, WNMFrame->Hdr.Addr2))
			IsFound = TRUE;
		
		break;
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);

	if (!IsFound) {
		DBGPRINT(RT_DEBUG_ERROR, ("Not found peer entry in list\n"));
		{
			unsigned char *tmp1 = (unsigned char *)WNMFrame->Hdr.Addr2;
			unsigned char *tmp2;
			printk("client mac:%02x:%02x:%02x:%02x:%02x:%02x\n",*(tmp1),*(tmp1+1),*(tmp1+2),*(tmp1+3),*(tmp1+4),*(tmp1+5));
			DlListForEach(WNMNotifyPeerEntry, &pWNMCtrl->WNMNotifyPeerList, WNM_NOTIFY_PEER_ENTRY, List)
			{
				tmp2 = (unsigned char *)WNMNotifyPeerEntry->PeerMACAddr;
				printk("list=> %02x:%02x:%02x:%02x:%02x:%02x\n",*(tmp2),*(tmp2+1),*(tmp2+2),*(tmp2+3),*(tmp2+4),*(tmp2+5));
			}
			printk("\n");
		}
		return;
	}

	/* Cancel Wait peer wnm response frame */
	RTMPCancelTimer(&WNMNotifyPeerEntry->WaitPeerWNMNotifyRspTimer, &Cancelled);
	RTMPReleaseTimer(&WNMNotifyPeerEntry->WaitPeerWNMNotifyRspTimer, &Cancelled);

	VarLen = 1;
	
	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + VarLen);

	if (!Buf)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return;
	}

	NdisZeroMemory(Buf, sizeof(*Event) + VarLen);

	Event = (WNM_NOTIFY_EVENT_DATA *)Buf;
	
	Event->ControlIndex = APIndex;
	Len += 1;
	
	NdisMoveMemory(Event->PeerMACAddr, WNMFrame->Hdr.Addr2, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;
	
	Event->EventType = WNM_NOTIFY_RSP;
	Len += 2;

	Event->u.WNM_NOTIFY_RSP_DATA.DialogToken = WNMFrame->u.WNM_NOTIFY_RSP.DialogToken;
 	Len += 1;

	Event->u.WNM_NOTIFY_RSP_DATA.WNMNotifyRspLen = 1;//  = WNMFrame->u.WNM_NOTIFY_RSP.DialogToken;
	Len += 2;

	//NdisMoveMemory(Event->u.WNM_NOTIFY_RSP_DATA.WNMNotifyRsp, WNMFrame->u.WNM_NOTIFY_RSP.Variable, 
	//						VarLen);
	Event->u.WNM_NOTIFY_RSP_DATA.WNMNotifyRsp[0] = WNMFrame->u.WNM_NOTIFY_RSP.RespStatus;
	Len += VarLen;
	
	MlmeEnqueue(pAd, WNM_NOTIFY_STATE_MACHINE, WNM_NOTIFY_RSP, Len, Buf,0); 
	
	os_free_mem(NULL, Buf);

	return;
}

static VOID SendWNMNotifyReq(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM  *Elem)
{
	WNM_NOTIFY_EVENT_DATA *Event = (WNM_NOTIFY_EVENT_DATA *)Elem->Msg;
	UCHAR *Buf;
	WNM_FRAME *WNMFrame;
	WNM_NOTIFY_PEER_ENTRY *WNMNotifyPeerEntry = NULL;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
	UINT32 FrameLen = 0, VarLen = Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen;
	INT32 Ret;
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
	DlListForEach(WNMNotifyPeerEntry, &pWNMCtrl->WNMNotifyPeerList,
						WNM_NOTIFY_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(WNMNotifyPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*WNMFrame) + VarLen + 7);
	
	if (!Buf)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return;
	}

	NdisZeroMemory(Buf, sizeof(*WNMFrame) + VarLen);

	WNMFrame = (WNM_FRAME *)Buf;

	ActHeaderInit(pAd, &WNMFrame->Hdr, Event->PeerMACAddr, 
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid);

	FrameLen += sizeof(HEADER_802_11);
	
	WNMFrame->Category = CATEGORY_WNM;
	FrameLen += 1;

	WNMFrame->u.WNM_NOTIFY_REQ.Action = WNM_NOTIFICATION_REQ;
	FrameLen += 1;

	WNMFrame->u.WNM_NOTIFY_REQ.DialogToken = Event->u.WNM_NOTIFY_REQ_DATA.DialogToken;
	FrameLen += 1;
	
	WNMFrame->u.WNM_NOTIFY_REQ.Type = 1;
	FrameLen += 1;

	if (Event->EventType == 0) //remediation
	{
		printk("remediation\n");
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[0] = 0xdd;
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[1] = 5+Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen;
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[2] = 0x50;
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[3] = 0x6f;
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[4] = 0x9a;
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[5] = 0x00;
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[6] = Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen;
		FrameLen += 7;
		NdisMoveMemory(&WNMFrame->u.WNM_NOTIFY_REQ.Variable[7], Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReq,
					Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen);
		FrameLen += Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen;
	}
	else if (Event->EventType == 2) //remediation+service method
    {
        printk("remediation with method\n");
        WNMFrame->u.WNM_NOTIFY_REQ.Variable[0] = 0xdd;
        WNMFrame->u.WNM_NOTIFY_REQ.Variable[1] = 5+Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen;
        WNMFrame->u.WNM_NOTIFY_REQ.Variable[2] = 0x50;
        WNMFrame->u.WNM_NOTIFY_REQ.Variable[3] = 0x6f;
        WNMFrame->u.WNM_NOTIFY_REQ.Variable[4] = 0x9a;
        WNMFrame->u.WNM_NOTIFY_REQ.Variable[5] = 0x00;
        WNMFrame->u.WNM_NOTIFY_REQ.Variable[6] = Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen-1;
        FrameLen += 7;
        NdisMoveMemory(&WNMFrame->u.WNM_NOTIFY_REQ.Variable[7], Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReq,
                    Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen);
        FrameLen += Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen;
    }
	else if (Event->EventType == 1) //deauth imminent notice
	{
		MAC_TABLE_ENTRY  *pEntry;
		
		printk("deauth imminent: %d\n", Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen);
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[0] = 0xdd;
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[1] = 5+Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen;
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[2] = 0x50;
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[3] = 0x6f;
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[4] = 0x9a;
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[5] = 0x01;
		NdisMoveMemory(&WNMFrame->u.WNM_NOTIFY_REQ.Variable[6], Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReq,
					3);
		WNMFrame->u.WNM_NOTIFY_REQ.Variable[9] = Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen-3;			
		FrameLen += 10;
		if (WNMFrame->u.WNM_NOTIFY_REQ.Variable[9] != 0)
		{
			NdisMoveMemory(&WNMFrame->u.WNM_NOTIFY_REQ.Variable[10], &Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReq[3],
					Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen-3);
			FrameLen += (Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen-3);
		}

		if ((pEntry = MacTableLookup(pAd, Event->PeerMACAddr)) != NULL)
    	{	
    		pEntry->BTMDisassocCount = 40; //20;
    	}		
	}
	else
	{
		printk("no match event type:%d\n", Event->EventType);
		os_free_mem(NULL, Buf);
	}
	WNMSetPeerCurrentState(pAd, Elem, WAIT_WNM_NOTIFY_RSP);
	
	MiniportMMRequest(pAd, 0, Buf, FrameLen);

	RTMPSetTimer(&WNMNotifyPeerEntry->WaitPeerWNMNotifyRspTimer, WaitPeerWNMNotifyRspTimeoutVale);

	os_free_mem(NULL, Buf);
}

VOID SendWNMNotifyConfirm(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem)
{

	PWNM_NOTIFY_PEER_ENTRY WNMNotifyPeerEntry, WNMNotifyPeerEntryTmp;
	WNM_NOTIFY_EVENT_DATA *Event = (WNM_NOTIFY_EVENT_DATA *)Elem->Msg;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
	INT32 Ret;

	printk("%s\n", __FUNCTION__);

	printk("Receive WNM Notify Response Status:%d\n", Event->u.WNM_NOTIFY_RSP_DATA.WNMNotifyRsp[0]);
	/* Delete BTM peer entry */
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
	DlListForEachSafe(WNMNotifyPeerEntry, WNMNotifyPeerEntryTmp, &pWNMCtrl->WNMNotifyPeerList, WNM_NOTIFY_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(WNMNotifyPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			
			DlListDel(&WNMNotifyPeerEntry->List);
			os_free_mem(NULL, WNMNotifyPeerEntry);
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);

}
#endif /* CONFIG_AP_SUPPORT */

VOID WNMNotifyStateMachineInit(
			IN	PRTMP_ADAPTER pAd, 
			IN	STATE_MACHINE *S, 
			OUT STATE_MACHINE_FUNC	Trans[])
{

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));


	StateMachineInit(S,	(STATE_MACHINE_FUNC*)Trans, MAX_WNM_NOTIFY_STATE, MAX_WNM_NOTIFY_MSG, (STATE_MACHINE_FUNC)Drop, WNM_NOTIFY_UNKNOWN, WNM_NOTIFY_MACHINE_BASE);

#ifdef CONFIG_AP_SUPPORT
	StateMachineSetAction(S, WAIT_WNM_NOTIFY_REQ, WNM_NOTIFY_REQ, (STATE_MACHINE_FUNC)SendWNMNotifyReq);
	StateMachineSetAction(S, WAIT_WNM_NOTIFY_RSP, WNM_NOTIFY_RSP, (STATE_MACHINE_FUNC)SendWNMNotifyConfirm);
#endif /* CONFIG_AP_SUPPORT */

}
#endif /* CONFIG_HOTSPOT_R2 */

