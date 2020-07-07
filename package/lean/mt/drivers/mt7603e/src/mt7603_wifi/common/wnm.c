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
/*#ifndef CONFIG_HOTSPOT_R2 #ifdef WNM_NEW_API*/
#ifndef WAPP_SUPPORT
void wext_send_btm_query_event_newapi(PNET_DEV net_dev, const char *peer_mac_addr,
							   const char *btm_query, UINT16 btm_query_len)
{
	struct btm_query_data *query_data = NULL;
	struct wnm_event *event_data = NULL;
	UINT16 buflen = 0;
	char *buf;

	buflen = sizeof(struct wnm_event) + sizeof(struct btm_query_data) + btm_query_len;
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	event_data = (struct wnm_event *)buf;
	event_data->event_id = OID_802_11_WNM_EVT_BTM_QUERY;
	event_data->event_len = sizeof(*query_data) + btm_query_len;

	query_data = (struct btm_query_data *)event_data->event_body;
	query_data->ifindex = RtmpOsGetNetIfIndex(net_dev);
	memcpy(query_data->peer_mac_addr, peer_mac_addr, 6);
	query_data->btm_query_len	= btm_query_len;
	memcpy(query_data->btm_query, btm_query, btm_query_len);

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
					OID_802_11_WNM_EVENT, NULL, (PUCHAR)buf, buflen);

	os_free_mem(NULL, buf);
}

#endif
const UCHAR wfa_oui[3] = {0x50, 0x6F, 0x9A};

void wext_send_btm_query_event(PNET_DEV net_dev, const char *peer_mac_addr,
							   const char *btm_query, UINT16 btm_query_len)
{
	struct btm_query_data *query_data;
	UINT16 buflen = 0;
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
/*#ifndef CONFIG_HOTSPOT_R2 #ifdef WNM_NEW_API*/
#ifndef WAPP_SUPPORT
		if (1) {
			wext_send_btm_query_event_newapi(net_dev,
											peer_mac_addr,
											btm_query,
											btm_query_len);

		} else
#endif
		wext_send_btm_query_event(net_dev,
								  peer_mac_addr,
								  btm_query,
								  btm_query_len);
	}
}

/*#ifndef CONFIG_HOTSPOT_R2 #ifdef WNM_NEW_API*/
#ifndef WAPP_SUPPORT
void wext_send_btm_cfm_event_newapi(PNET_DEV net_dev, const char *peer_mac_addr,
							 const char *btm_rsp, UINT16 btm_rsp_len)
{
	struct btm_rsp_data *rsp_data = NULL;
	struct wnm_event *event_data = NULL;
	UINT16 buflen = 0;
	char *buf;


	buflen = sizeof(struct wnm_event) + sizeof(*rsp_data) + btm_rsp_len;
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	event_data = (struct wnm_event *)buf;
	event_data->event_id = OID_802_11_WNM_EVT_BTM_RSP;
	event_data->event_len = sizeof(struct btm_rsp_data) + btm_rsp_len;


	rsp_data = (struct btm_rsp_data *)event_data->event_body;
	rsp_data->ifindex = RtmpOsGetNetIfIndex(net_dev);
	memcpy(rsp_data->peer_mac_addr, peer_mac_addr, 6);
	rsp_data->btm_rsp_len	= btm_rsp_len;
	memcpy(rsp_data->btm_rsp, btm_rsp, btm_rsp_len);

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
						OID_802_11_WNM_EVENT, NULL, (PUCHAR)buf, buflen);

	os_free_mem(NULL, buf);
}

#endif

void wext_send_btm_cfm_event(PNET_DEV net_dev, const char *peer_mac_addr,
							 const char *btm_rsp, UINT16 btm_rsp_len)
{
	struct btm_rsp_data *rsp_data;
	UINT16 buflen = 0;
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
/*#ifndef CONFIG_HOTSPOT_R2 #ifdef WNM_NEW_API*/
#ifndef WAPP_SUPPORT
		if (1) {
			wext_send_btm_cfm_event_newapi(net_dev,
											peer_mac_addr,
											btm_rsp,
											btm_rsp_len);
		} else
#endif
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
							   UINT8 ip_type,
							   UINT8 from_ds,
							   unsigned char IsDAD)
{
	struct proxy_arp_entry *arp_entry;
	UINT16 varlen = 0, buflen = 0;
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
				  	   UINT8 ip_type,
					   UINT8 from_ds,
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
						IN struct _BSS_STRUCT *pMbss)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	UCHAR *SenderIP;
	UCHAR *TargetIP;
	UCHAR BroadcastMac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
#ifdef CONFIG_HOTSPOT_R2
	UINT16 ARPOperation;
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	BOOLEAN IsDrop = FALSE;
	INT32 Ret;
#endif
	PUCHAR SourceMACAddr;

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
			if(Ret != 0)
				DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
		INT32 	PayloadLen = 0;//((*(Pos+5) & 0xff) << 8) | (*(Pos+4) & 0xff);
		UCHAR 	NextHeader = *(Pos+6);
		UCHAR 	IsExtenHeader = 0;
		
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
	//RT_IPV6_ADDR *pIPv6Addr;
	
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
		
		Pos += 8;
		//pIPv6Addr = (RT_IPV6_ADDR *)Pos;		
		
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
							IN struct _BSS_STRUCT *pMbss)
{

	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	UINT32 TableLen = 0;
	INT32 Ret;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv4ProxyARPList, PROXY_ARP_IPV4_ENTRY, List)
	{
		TableLen += sizeof(PROXY_ARP_IPV4_UNIT);
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);

	return TableLen;
}

UINT32 IPv6ProxyARPTableLen(IN PRTMP_ADAPTER pAd,
							IN struct _BSS_STRUCT *pMbss)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
	UINT32 TableLen = 0;
	INT32 Ret;

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
	DlListForEach(ProxyARPEntry, &pWNMCtrl->IPv6ProxyARPList, PROXY_ARP_IPV6_ENTRY, List)
	{
		TableLen += sizeof(PROXY_ARP_IPV6_UNIT);
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);

	return TableLen;
}

BOOLEAN GetIPv4ProxyARPTable(IN PRTMP_ADAPTER pAd,
						 	 IN struct _BSS_STRUCT *pMbss,
						 	 PUCHAR *ProxyARPTable)
{

	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	PROXY_ARP_IPV4_UNIT *ProxyARPUnit = (PROXY_ARP_IPV4_UNIT *)(*ProxyARPTable);
	INT32 Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
						 	 IN struct _BSS_STRUCT *pMbss,
						 	 PUCHAR *ProxyARPTable)
{

	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
	PROXY_ARP_IPV6_UNIT *ProxyARPUnit = (PROXY_ARP_IPV6_UNIT *)(*ProxyARPTable);
	INT32 Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
					   		IN BSS_STRUCT *pMbss,
							PUCHAR pTargetMACAddr,
							PUCHAR pTargetIPAddr)
{

	int i = 0, find_list = 0;
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry;
	INT32 Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s ====>\n", __FUNCTION__));

	if ((pTargetIPAddr[0] == 0) && (pTargetIPAddr[1] == 0)) {
		DBGPRINT(RT_DEBUG_ERROR, ("Drop invalid IP Addr:%d.%d.%d.%d\n", pTargetIPAddr[0], pTargetIPAddr[1], pTargetIPAddr[2], pTargetIPAddr[3]));
		return FALSE;
	}
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
		DBGPRINT(RT_DEBUG_OFF,("pTargetIPv4Addr[%i] = %d\n", i, pTargetIPAddr[i]));
	
	/* Add ProxyARP Entry to list */
	if (find_list == 0) 
	{
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
		if(Ret != 0)
			DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
		DlListAddTail(&pWNMCtrl->IPv4ProxyARPList, &ProxyARPEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPListLock);
	}

	return TRUE;
}

VOID RemoveIPv4ProxyARPEntry(IN PRTMP_ADAPTER pAd,
					   		IN BSS_STRUCT *pMbss,
							PUCHAR pTargetMACAddr)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV4_ENTRY *ProxyARPEntry, *ProxyARPEntryTmp;
	INT32 Ret;

	DBGPRINT(RT_DEBUG_OFF, ("%s ====>\n", __FUNCTION__));
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPListLock, Ret);
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
	
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
							IN BSS_STRUCT *pMbss,
							PUCHAR pTargetMACAddr,
							PUCHAR pTargetIPAddr)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry;
	INT32 Ret;
	UINT8 i;
	BOOLEAN IsDAD = FALSE;
	PNET_DEV NetDev = pMbss->wdev.if_dev;
	DBGPRINT(RT_DEBUG_OFF, ("%s ====>\n", __FUNCTION__));

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
		DBGPRINT(RT_DEBUG_OFF,("pTargetMACAddr[%i] = %x\n", i, pTargetMACAddr[i]));

	for (i = 0; i < 16; i++)
		DBGPRINT(RT_DEBUG_OFF,("pTargetIPv6Addr[%i] = %d\n", i, pTargetIPAddr[i]));
	
	/* Add ProxyARP Entry to list */
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
	DlListAddTail(&pWNMCtrl->IPv6ProxyARPList, &ProxyARPEntry->List);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->ProxyARPIPv6ListLock);

	return TRUE;
}

VOID RemoveIPv6ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							IN BSS_STRUCT *pMbss,
							PUCHAR pTargetMACAddr)
{
	PWNM_CTRL pWNMCtrl = &pMbss->WNMCtrl;
	PROXY_ARP_IPV6_ENTRY *ProxyARPEntry, *ProxyARPEntryTmp;
	INT32 Ret;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->ProxyARPIPv6ListLock, Ret);
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));

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
				 	 IN BSS_STRUCT *pMbss,
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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
			if(Ret != 0)
				DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
				if(Ret != 0)
					DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
					 IN BSS_STRUCT *pMbss,
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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
	struct wifi_dev *wdev;
	UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);
	BSS_STRUCT *pMbss;
	MAC_TABLE_ENTRY  *pEntry;	

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Invalid wdev_idx(%d)\n", __FUNCTION__, wdev_idx));
		return;
	}

	wdev = pAd->wdev_list[wdev_idx];
	ASSERT(wdev->func_idx < pAd->ApCfg.BssidNum);
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	if (srcPort  == 0x43 && dstPort == 0x44)
	{
		UCHAR *pTargetIPAddr = pSrcBuf + 24;
		/* Client hardware address */
		UCHAR *pTargetMACAddr = pSrcBuf + 36;
						
		if ((pMbss->WNMCtrl.ProxyARPEnable) && (pEntry = MacTableLookup(pAd, pTargetMACAddr)))
		{
			printk("entry apidx=%d,%d,%d\n",pEntry->func_tb_idx, wdev_idx, pMbss->WNMCtrl.ProxyARPEnable);

			if ((pEntry->func_tb_idx == wdev_idx) && pMbss->WNMCtrl.ProxyARPEnable)
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
	struct wifi_dev *wdev;
	UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);
	BSS_STRUCT *pMbss;
	UCHAR Offset = 0;

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Invalid wdev_idx(%d)\n", __FUNCTION__, wdev_idx));
		return;
	}
	
	wdev = pAd->wdev_list[wdev_idx];
	ASSERT(wdev->func_idx < pAd->ApCfg.BssidNum);
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
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
					if(Ret != 0)
						DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
	WNM_FRAME *WNMFrame = (WNM_FRAME *)Elem->Msg;
	BTM_PEER_ENTRY *BTMPeerEntry;
	PWNM_CTRL pWNMCtrl = NULL;
	UCHAR APIndex;
	UINT16 VarLen;
	INT32 Ret;
	BOOLEAN IsFound = FALSE;
	PNET_DEV NetDev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __func__));

	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++) {
		if (MAC_ADDR_EQUAL(WNMFrame->Hdr.Addr3, pAd->ApCfg.MBSSID[APIndex].wdev.bssid)) {
			pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
			break;
		}
	}

	if (!pWNMCtrl) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s Can not find Peer Control\n", __FUNCTION__));
		return;
	}

	if (pWNMCtrl->WNMBTMEnable == 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s BTM Not Supported Drop!!\n", __func__));
		return;
	}

	NetDev = pAd->ApCfg.MBSSID[APIndex].wdev.if_dev;

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);

	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List) {
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

	if (!BTMPeerEntry) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return;
	}

	NdisZeroMemory(BTMPeerEntry, sizeof(*BTMPeerEntry));
	BTMPeerEntry->CurrentState = WAIT_BTM_REQ;

	NdisMoveMemory(BTMPeerEntry->PeerMACAddr, WNMFrame->Hdr.Addr2, MAC_ADDR_LEN);
	BTMPeerEntry->DialogToken = WNMFrame->u.BTM_QUERY.DialogToken;
	BTMPeerEntry->Priv = pAd;

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListAddTail(&pWNMCtrl->BTMPeerList, &BTMPeerEntry->List);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	VarLen = Elem->MsgLen -
		(sizeof(HEADER_802_11) + 1 + sizeof(WNMFrame->u.BTM_QUERY)) + 1;
	SendBTMQueryEvent(NetDev,
						WNMFrame->Hdr.Addr2,
						(PUCHAR)&(WNMFrame->u.BTM_QUERY.DialogToken),
						VarLen,
						RA_WEXT);

	RTMPInitTimer(pAd, &BTMPeerEntry->WaitPeerBTMReqTimer,
					GET_TIMER_FUNCTION(WaitPeerBTMReqTimeout), BTMPeerEntry, FALSE);
	RTMPSetTimer(&BTMPeerEntry->WaitPeerBTMReqTimer, WaitPeerBTMReqTimeoutVale);
}


static VOID ReceiveBTMRsp(IN PRTMP_ADAPTER pAd,
						  IN MLME_QUEUE_ELEM *Elem)
{
	WNM_FRAME *WNMFrame = (WNM_FRAME *)Elem->Msg;
	BTM_PEER_ENTRY *BTMPeerEntry;
	PWNM_CTRL pWNMCtrl = NULL;
	UCHAR APIndex;
	UINT16 VarLen = 0;
	INT32 Ret;
	BOOLEAN IsFound = FALSE, Cancelled;
	PNET_DEV NetDev = NULL;

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

	if (pWNMCtrl->WNMBTMEnable == 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s BTM Not Supported Drop!!\n", __func__));
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
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListDel(&BTMPeerEntry->List);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
	os_free_mem(NULL, BTMPeerEntry);
	
	NetDev = pAd->ApCfg.MBSSID[APIndex].wdev.if_dev;
	/* Send BTM confirm to daemon */
	VarLen = Elem->MsgLen -
		(sizeof(HEADER_802_11) + 1 + sizeof(WNMFrame->u.BTM_RSP)) + 1;
	SendBTMConfirmEvent(NetDev,
					WNMFrame->Hdr.Addr2,
					(PUCHAR)&(WNMFrame->u.BTM_RSP.DialogToken),
					VarLen,
					RA_WEXT);
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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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


VOID WaitPeerBTMReqTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	BTM_PEER_ENTRY *BTMPeerEntry = (BTM_PEER_ENTRY *)FunctionContext;
	PRTMP_ADAPTER pAd = NULL;
	BTM_EVENT_DATA event;

	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s BTMPeerEntry:%p\n", __func__, BTMPeerEntry));
	if (!BTMPeerEntry)
		return;

	pAd = BTMPeerEntry->Priv;
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS
							| fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;
	memset(&event, 0, sizeof(event));
	memcpy(event.PeerMACAddr, BTMPeerEntry->PeerMACAddr, MAC_ADDR_LEN);
	event.ControlIndex = BTMPeerEntry->ControlIndex;
	event.u.BTM_REQ_DATA.DialogToken = BTMPeerEntry->DialogToken;

	MlmeEnqueue(pAd, BTM_STATE_MACHINE, BTM_REQ_TIMEOUT,
		sizeof(BTM_EVENT_DATA), &event, 0);
}
BUILD_TIMER_FUNCTION(WaitPeerBTMReqTimeout);


VOID WaitPeerBTMRspTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PBTM_PEER_ENTRY BTMPeerEntry = (PBTM_PEER_ENTRY)FunctionContext;
	PRTMP_ADAPTER pAd = NULL;
	BTM_EVENT_DATA event;
	
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s BTMPeerEntry:%p\n", __func__, BTMPeerEntry));

	if (!BTMPeerEntry)
		return;

	pAd = BTMPeerEntry->Priv;
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS
							| fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	memset(&event, 0, sizeof(event));
	memcpy(event.PeerMACAddr, BTMPeerEntry->PeerMACAddr, MAC_ADDR_LEN);
	event.ControlIndex = BTMPeerEntry->ControlIndex;
	event.u.PEER_BTM_RSP_DATA.DialogToken = BTMPeerEntry->DialogToken;

	MlmeEnqueue(pAd, BTM_STATE_MACHINE, PEER_BTM_RSP_TIMEOUT,
		sizeof(BTM_EVENT_DATA), &event, 0);

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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
	
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), Buf, FrameLen);

	RTMPSetTimer(&BTMPeerEntry->WaitPeerBTMRspTimer, WaitPeerBTMRspTimeoutVale);

#if (defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM))
	{
		MAC_TABLE_ENTRY  *pEntry;

		pEntry = MacTableLookup(pAd, Event->PeerMACAddr);
		if (pEntry != NULL) {
			UINT8 *BTMData = (UINT8 *)Event->u.BTM_REQ_DATA.BTMReq;
			/*if (*BTMData & 0x04) {*/
			if ((*(BTMData) & 0x1C) != 0) {
				pEntry->BTMDisassocCount = (((*(BTMData+2) << 8) | (*(BTMData+1)))*pAd->CommonCfg.BeaconPeriod)/1000;
				DBGPRINT(RT_DEBUG_OFF, ("[%s] bss DisAssocCount sec=%d\n", __func__, pEntry->BTMDisassocCount));
				if (pEntry->BTMDisassocCount < 1)
					pEntry->BTMDisassocCount = 1;
			}
		}
	}
#endif	

	os_free_mem(NULL, Buf);
}


static VOID SendBTMReqIE(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PBTM_EVENT_DATA Event = (PBTM_EVENT_DATA)Elem->Msg;
	PBTM_PEER_ENTRY BTMPeerEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
	PUCHAR p_btm_req_data = Event->u.BTM_REQ_DATA.BTMReq;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	UINT32 btm_reqinfo_len = Event->u.BTM_REQ_DATA.BTMReqLen;
	INT32 Ret;
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	BOOLEAN isfound = FALSE, Cancelled;
	HEADER_802_11 ActHdr;

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList,
						BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
			isfound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	if (!isfound) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s() BTMPeerEntry is already deleted\n",
		__func__));
		return;
	}

	pEntry = MacTableLookup(pAd, Event->PeerMACAddr);
	if (!pEntry || !((Ndis802_11AuthModeAutoSwitch >=
			pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.AuthMode) ||
			((pEntry->WpaState == AS_PTKINITDONE) &&
			(pEntry->GTKState == REKEY_ESTABLISHED)))) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s() STA(%02x:%02x:%02x:%02x:%02x:%02x)not associates with AP!\n",
			__func__, PRINT_MAC(Event->PeerMACAddr)));
		goto error;
	}

	/*allocate a buffer prepare for btm req frame*/
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s() allocate memory for btm req frame failed\n",
			__func__));
		goto error;
	}

	/*compose 80211 header*/
	ActHeaderInit(pAd, &ActHdr, Event->PeerMACAddr,
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid);
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(ActHdr));
	FrameLen = sizeof(ActHdr);
	/*compose the  Neighbor report header
	*   category, action and token
	*/
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen,
		CATEGORY_WNM, BSS_TRANSITION_REQ);
	InsertDialogToken(pAd, (pOutBuffer + FrameLen),
		&FrameLen, Event->u.BTM_REQ_DATA.DialogToken);
	memcpy((pOutBuffer + FrameLen), p_btm_req_data, btm_reqinfo_len);
	FrameLen += btm_reqinfo_len;

	BTMPeerEntry->CurrentState = WAIT_PEER_BTM_RSP;
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);
	RTMPCancelTimer(&BTMPeerEntry->WaitPeerBTMReqTimer, &Cancelled);
	RTMPReleaseTimer(&BTMPeerEntry->WaitPeerBTMReqTimer, &Cancelled);
	RTMPInitTimer(pAd, &BTMPeerEntry->WaitPeerBTMRspTimer,
				GET_TIMER_FUNCTION(WaitPeerBTMRspTimeout), BTMPeerEntry, FALSE);
	RTMPSetTimer(&BTMPeerEntry->WaitPeerBTMRspTimer,
		(BTMPeerEntry->WaitPeerBTMRspTime == 0) ? WaitPeerBTMRspTimeoutVale : BTMPeerEntry->WaitPeerBTMRspTime);

	if (p_btm_req_data[0] & 0x04) {
		pEntry->BTMDisassocCount =
			(p_btm_req_data[1] | p_btm_req_data[2]<<8) * pAd->CommonCfg.BeaconPeriod / 1000;
		DBGPRINT(RT_DEBUG_OFF, ("bss discount sec=%d\n", pEntry->BTMDisassocCount));
	if (pEntry->BTMDisassocCount < 1)
		pEntry->BTMDisassocCount = 1;
	}

	os_free_mem(NULL, pOutBuffer);
	return;

error:
	if (BTMPeerEntry->WaitPeerBTMReqTimer.Valid) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s()  BTMReqTimer is valid, wait timeout to delete BTMPeerEntry\n",
			__func__));
	} else {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s()  BTMReqTimer is not valid, delete BTMPeerEntry now\n",
			__func__));
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
		DlListDel(&BTMPeerEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
		os_free_mem(NULL, BTMPeerEntry);
	}
}

static VOID SendBTMReqParam(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PBTM_EVENT_DATA Event = (PBTM_EVENT_DATA)Elem->Msg;
	PBTM_PEER_ENTRY BTMPeerEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
	p_btm_reqinfo_t p_btm_req_data =
		(p_btm_reqinfo_t)Event->u.BTM_REQ_DATA.BTMReq;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	UINT32 TmpLen = 0;
	UINT32 btm_reqinfo_len = Event->u.BTM_REQ_DATA.BTMReqLen;
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	INT32 Ret;
	BOOLEAN isfound = FALSE, Cancelled;
	HEADER_802_11 ActHdr;

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList,
						BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
			isfound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	if (!isfound) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s() BTMPeerEntry is already deleted\n",
		__func__));
		return;
	}

	/*check the parameters*/
	NStatus = check_btm_custom_params(pAd, p_btm_req_data, btm_reqinfo_len);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s() check customer params failed\n", __func__));
		goto error;
	}

	/*allocate a buffer prepare for btm req frame*/
	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s() allocate memory for btm req frame failed\n",
			__func__));
		goto error;
	}
	/*compose 80211 header*/
	ActHeaderInit(pAd, &ActHdr, p_btm_req_data->sta_mac,
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid,
					pAd->ApCfg.MBSSID[Event->ControlIndex].wdev.bssid);
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(ActHdr));
	FrameLen = sizeof(ActHdr);

	/*compose the  Neighbor report header
	 *  category, action and token
	 */
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen,
		CATEGORY_WNM, BSS_TRANSITION_REQ);
	InsertDialogToken(pAd, (pOutBuffer + FrameLen),
		&FrameLen, p_btm_req_data->dialogtoken);

	/*compose the  Neighbor report IE*/
	compose_btm_req_ie(pAd, (pOutBuffer + FrameLen), &TmpLen,
		p_btm_req_data, btm_reqinfo_len);
	FrameLen += TmpLen;

	BTMPeerEntry->CurrentState = WAIT_PEER_BTM_RSP;
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);

	RTMPCancelTimer(&BTMPeerEntry->WaitPeerBTMReqTimer, &Cancelled);
	RTMPReleaseTimer(&BTMPeerEntry->WaitPeerBTMReqTimer, &Cancelled);
	RTMPInitTimer(pAd, &BTMPeerEntry->WaitPeerBTMRspTimer,
				GET_TIMER_FUNCTION(WaitPeerBTMRspTimeout), BTMPeerEntry, FALSE);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
		("HZ=%d, WaitPeerBTMRspTime=%d\n", HZ, BTMPeerEntry->WaitPeerBTMRspTime));
	RTMPSetTimer(&BTMPeerEntry->WaitPeerBTMRspTimer,
		(BTMPeerEntry->WaitPeerBTMRspTime == 0) ? WaitPeerBTMRspTimeoutVale : BTMPeerEntry->WaitPeerBTMRspTime);

	pEntry = MacTableLookup(pAd, Event->PeerMACAddr);
	if (pEntry != NULL) {
		if (p_btm_req_data->reqmode & 0x04) {
			pEntry->BTMDisassocCount =
				p_btm_req_data->disassoc_timer * pAd->CommonCfg.BeaconPeriod / 1000;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
				("bss discount sec=%d\n", pEntry->BTMDisassocCount));
		if (pEntry->BTMDisassocCount < 1)
			pEntry->BTMDisassocCount = 1;
		}
	}

	os_free_mem(NULL, pOutBuffer);
	return;

error:
	if (BTMPeerEntry->WaitPeerBTMReqTimer.Valid) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s()  BTMReqTimer is valid, wait timeout to delete BTMPeerEntry\n",
			__func__));
	} else {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s()  BTMReqTimer is not valid, delete BTMPeerEntry now\n",
			__func__));
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
		DlListDel(&BTMPeerEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
		os_free_mem(NULL, BTMPeerEntry);
	}
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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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

static VOID BTMReqTimeout(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	BTM_EVENT_DATA event;
	BTM_PEER_ENTRY *BTMPeerEntry = NULL, *BTMPeerEntryTmp = NULL;
	PWNM_CTRL pWNMCtrl = NULL;
	INT32 Ret;
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_OFF,
		("%s\n", __func__));

	RTMPMoveMemory(&event, Elem->Msg, sizeof(event));

	pWNMCtrl = &pAd->ApCfg.MBSSID[event.ControlIndex].WNMCtrl;
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEachSafe(BTMPeerEntry, BTMPeerEntryTmp, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, &event.PeerMACAddr[0]))
			break;
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	/*
	 * no other place to delete BTMPeerEntry when receive btm query;
	 * so here must exist BTMPeerEntry
	 */
	if (BTMPeerEntry->WaitPeerBTMRspTimer.Valid) {
		DBGPRINT(RT_DEBUG_OFF,
				("%s receive btm req and set btm rsp timer no need to delete BTMPeerEntry\n",
				__func__));
	} else {
		/*timeout; need delete BTMPeerEntry*/
		DBGPRINT(RT_DEBUG_OFF,
				("%s receive btm req timeout the uplayer does not send btm req in time\n",
				 __func__));
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
		DlListDel(&BTMPeerEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
		RTMPReleaseTimer(&BTMPeerEntry->WaitPeerBTMReqTimer, &Cancelled);
		os_free_mem(NULL, BTMPeerEntry);
	}
}


static VOID ReceiveBTMRspTimeout(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	BTM_EVENT_DATA event;
	BTM_PEER_ENTRY *BTMPeerEntry = NULL, *BTMPeerEntryTmp = NULL;
	PWNM_CTRL pWNMCtrl = NULL;
	INT32 Ret;
	BOOLEAN Cancelled;
	PNET_DEV NetDev = NULL;
	BOOLEAN isfound = FALSE;

	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s\n", __func__));

	memcpy(&event, Elem->Msg, sizeof(BTM_EVENT_DATA));

	pWNMCtrl = &pAd->ApCfg.MBSSID[event.ControlIndex].WNMCtrl;
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEachSafe(BTMPeerEntry, BTMPeerEntryTmp, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, &event.PeerMACAddr[0])) {
			isfound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
	if (isfound == FALSE) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_OFF,
		("%s receive btm rsp;BTMPeerEntry is already deleted\n", __func__));
		return;
	}

	NetDev = pAd->ApCfg.MBSSID[event.ControlIndex].wdev.if_dev;
	/* Send BTM RSP timeout indication to daemon */
	SendBTMConfirmEvent(NetDev,
						event.PeerMACAddr,
						(PUCHAR)&(event.u.PEER_BTM_RSP_DATA.DialogToken),
						1,
						RA_WEXT);

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListDel(&BTMPeerEntry->List);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
	RTMPReleaseTimer(&BTMPeerEntry->WaitPeerBTMRspTimer, &Cancelled);
	os_free_mem(NULL, BTMPeerEntry);

}

#ifndef WAPP_SUPPORT/*CONFIG_HOTSPOT_R2 #ifdef WNM_NEW_API*/
NDIS_STATUS wnm_handle_command(IN PRTMP_ADAPTER pAd, IN struct wnm_command *pCmd_data)
{

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
	int status = NDIS_STATUS_SUCCESS;

	switch (pCmd_data->command_id) {
	case OID_802_11_WNM_CMD_ENABLE:
	{

		if (pCmd_data->command_body[0] == 0) {
			/*Disable All Features*/
			pWNMCtrl->WNMBTMEnable = 0;
			APUpdateBeaconFrame(pAd, APIndex);
		} else {
			DBGPRINT(RT_DEBUG_OFF, ("%s OID_802_11_WNM_CMD_ENABLE, do nothing\n",
				__func__));
		}
	}
		break;
	case OID_802_11_WNM_CMD_CAP:
	{

		if (pCmd_data->command_body[0] & BTM_ENABLE_OFFSET)
			pWNMCtrl->WNMBTMEnable = 1;
		else
			pWNMCtrl->WNMBTMEnable = 0;

		APUpdateBeaconFrame(pAd, APIndex);
	}
		break;
	case OID_802_11_WNM_CMD_SEND_BTM_REQ_IE:
	{
		status = send_btm_req_ie(pAd,
			(p_btm_req_ie_data_t)pCmd_data->command_body,
				pCmd_data->command_len);
	}
		break;
	case OID_802_11_WNM_CMD_SET_BTM_REQ_PARAM:
	{
		status = send_btm_req_param(pAd,
			(p_btm_reqinfo_t)pCmd_data->command_body,
				pCmd_data->command_len);
	}
		break;
	default:
		DBGPRINT(RT_DEBUG_OFF, ("%s Invalid Command %d\n",
			__func__, pCmd_data->command_id));
		status = NDIS_STATUS_INVALID_DATA;
		break;

	}
	return status;
}
#endif
void WNM_ReadParametersFromFile(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	INT loop;
	RTMP_STRING *macptr;

	if (RTMPGetKeyParameter("WNMEnable", tmpbuf, 255, buffer, TRUE)) {
		for (loop = 0, macptr = rstrtok(tmpbuf, ";");
				(macptr && loop < MAX_MBSSID_NUM(pAd));
					macptr = rstrtok(NULL, ";"), loop++) {
			LONG Enable;

			kstrtol(macptr, 10, &Enable);
			pAd->ApCfg.MBSSID[loop].WNMCtrl.WNMBTMEnable =
				(Enable > 0) ? TRUE : FALSE;
			DBGPRINT(RT_DEBUG_TRACE, ("%s::(bDot11vWNMEnable[%d]=%d\n",
				__func__, loop,
				pAd->ApCfg.MBSSID[loop].WNMCtrl.WNMBTMEnable));
		}
	} else {
		for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++)
			pAd->ApCfg.MBSSID[loop].WNMCtrl.WNMBTMEnable = FALSE;
	}
}
#endif /* CONFIG_AP_SUPPORT */



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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
		case WNM_NOTIFICATION_RSP:
			ReceiveWNMNotifyRsp(pAd, Elem);
			break;		
		case WNM_NOTIFICATION_REQ:
			ReceiveWNMNotifyReq(pAd, Elem);
			break;
#endif /* CONFIG_AP_SUPPORT */

		default:
			DBGPRINT(RT_DEBUG_TRACE, ("Invalid action field = %d\n", Action));
			break;
	}
}

VOID WNMCtrlInit(IN PRTMP_ADAPTER pAd)
{
	PWNM_CTRL pWNMCtrl;
	BOOLEAN WNMBTMEnable = TRUE;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
#ifndef CONFIG_HOTSPOT_R2
		if (pWNMCtrl->WNMBTMEnable)
			WNMBTMEnable = pWNMCtrl->WNMBTMEnable;
#endif
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
		pWNMCtrl->WNMBTMEnable = WNMBTMEnable;
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
		if(Ret != 0)
			DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));

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
		if(Ret != 0)
			DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
		if(Ret != 0)
			DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
		if(Ret != 0)
			DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
		
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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
	StateMachineSetAction(S, WAIT_BTM_REQ, BTM_REQ_IE, (STATE_MACHINE_FUNC)SendBTMReqIE);
	StateMachineSetAction(S, WAIT_BTM_REQ, BTM_REQ_PARAM, (STATE_MACHINE_FUNC)SendBTMReqParam);
	StateMachineSetAction(S, WAIT_BTM_REQ, BTM_REQ_TIMEOUT, (STATE_MACHINE_FUNC)BTMReqTimeout);
	StateMachineSetAction(S, WAIT_PEER_BTM_RSP, PEER_BTM_RSP, (STATE_MACHINE_FUNC)SendBTMConfirm);
	StateMachineSetAction(S, WAIT_PEER_BTM_RSP, PEER_BTM_RSP_TIMEOUT, (STATE_MACHINE_FUNC)ReceiveBTMRspTimeout);
#endif /* CONFIG_AP_SUPPORT */

}


int send_btm_req_ie(
	IN PRTMP_ADAPTER pAd,
	IN p_btm_req_ie_data_t p_btm_req_data,
	IN UINT32 btm_req_data_len)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	BSS_STRUCT *pMbss = NULL;
	PWNM_CTRL pWNMCtrl = NULL;
	PBTM_EVENT_DATA Event = NULL;
	PBTM_PEER_ENTRY BTMPeerEntry = NULL;
	PUCHAR Buf = NULL;
	UINT32 Len = 0;

	static UINT8 j;
	INT32 Ret;
	UCHAR APIndex = pObj->ioctl_if;
	BOOLEAN IsFound = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __func__));

	pMbss = &pAd->ApCfg.MBSSID[APIndex];
	pWNMCtrl = &pMbss->WNMCtrl;
	if (pWNMCtrl->WNMBTMEnable == FALSE) {
		DBGPRINT(RT_DEBUG_TRACE,
			("%s() btm off\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	if (btm_req_data_len > 1000) {
		DBGPRINT(RT_DEBUG_TRACE,
			("%s BTM Req ie len(%d) is too long", __func__,
			  btm_req_data_len));
		return NDIS_STATUS_FAILURE;
	}

	Len = sizeof(*Event) + btm_req_data_len;
	os_alloc_mem(NULL, (UCHAR **)&Buf, Len);
	if (!Buf) {
		DBGPRINT(RT_DEBUG_TRACE,
			("%s Not available memory for btm req msg\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, p_btm_req_data->peer_mac_addr)) {
			IsFound = TRUE;
			break;
		}
	}
	if (IsFound) {
		DBGPRINT(RT_DEBUG_TRACE,
			("%s() found BTMPeerEntry in BTMEntrylist that say receive btm query before\n",
			__func__));
		BTMPeerEntry->CurrentState = WAIT_BTM_REQ;
		BTMPeerEntry->WaitPeerBTMRspTime = p_btm_req_data->timeout * 1000;
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	if (!IsFound) {
		os_alloc_mem(NULL, (UCHAR **)&BTMPeerEntry, sizeof(*BTMPeerEntry));

		if (!BTMPeerEntry) {
			DBGPRINT(RT_DEBUG_TRACE,
				("%s Not available memory for BTMPeerEntry\n", __func__));
			os_free_mem(NULL, Buf);
			return NDIS_STATUS_FAILURE;
		}

		NdisZeroMemory(BTMPeerEntry, sizeof(*BTMPeerEntry));

		BTMPeerEntry->CurrentState = WAIT_BTM_REQ;
		BTMPeerEntry->ControlIndex = APIndex;
		NdisMoveMemory(BTMPeerEntry->PeerMACAddr,
			p_btm_req_data->peer_mac_addr, MAC_ADDR_LEN);
		if (p_btm_req_data->dialog_token) {
			BTMPeerEntry->DialogToken = p_btm_req_data->dialog_token;
		} else {
			j++;
			if (j == 0)
				j = 1;
			BTMPeerEntry->DialogToken = j;
			p_btm_req_data->dialog_token = j;
		}
		BTMPeerEntry->Priv = pAd;
		BTMPeerEntry->WaitPeerBTMRspTime = p_btm_req_data->timeout * 1000;
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
		DlListAddTail(&pWNMCtrl->BTMPeerList, &BTMPeerEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
	}

	NdisZeroMemory(Buf, Len);
	Event = (BTM_EVENT_DATA *)Buf;
	Event->ControlIndex = APIndex;
	NdisMoveMemory(Event->PeerMACAddr,
		p_btm_req_data->peer_mac_addr, MAC_ADDR_LEN);
	Event->EventType = BTM_REQ_IE;
	/*
	*   if we receive btm query and btm req is sent in query session
	 *  then we think the btm req sent for btm query
	 */
	Event->u.BTM_REQ_DATA.DialogToken = p_btm_req_data->dialog_token;
	Event->u.BTM_REQ_DATA.BTMReqLen = p_btm_req_data->btm_req_len;

	NdisMoveMemory(Event->u.BTM_REQ_DATA.BTMReq,
		p_btm_req_data->btm_req, p_btm_req_data->btm_req_len);

	if (!MlmeEnqueue(pAd, BTM_STATE_MACHINE, BTM_REQ_IE, Len, Buf, 0)) {
		DBGPRINT(RT_DEBUG_TRACE,
			("%s() mlme enqueue failed\n", __func__));
		if (!IsFound) {
			RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
			DlListDel(&BTMPeerEntry->List);
			RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
			os_free_mem(NULL, BTMPeerEntry);
		}
	}
	os_free_mem(NULL, Buf);

	return NDIS_STATUS_SUCCESS;
}


/*
*	this function is used to compose the btm request frame with
*	parameters sent by the up-layer
*	return value: error reason or success
*/
int send_btm_req_param(
	IN PRTMP_ADAPTER pAd,
	IN p_btm_reqinfo_t p_btm_req_data,
	IN UINT32 btm_req_data_len)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_STRUCT *pMbss = NULL;
	PWNM_CTRL pWNMCtrl = NULL;
	PBTM_EVENT_DATA Event = NULL;
	PBTM_PEER_ENTRY BTMPeerEntry = NULL;
	PUCHAR pBuf = NULL;
	UINT32 Len = 0;
	static UINT8 i;
	INT32 Ret;
	UCHAR ifIndex = pObj->ioctl_if;
	BOOLEAN IsFound = FALSE;

	pMbss = &pAd->ApCfg.MBSSID[ifIndex];
	pWNMCtrl = &pMbss->WNMCtrl;
	if (pWNMCtrl->WNMBTMEnable == FALSE) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s() btm off\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	Len = sizeof(*p_btm_req_data) +
		p_btm_req_data->num_candidates * sizeof(struct nr_info);
	if (btm_req_data_len != Len) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s() length check failed btm_req_data_len=%d, Len=%d\n",
			__func__, btm_req_data_len, Len));
		return NDIS_STATUS_FAILURE;
	}

	if (p_btm_req_data->num_candidates > MAX_CANDIDATE_NUM) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s the num of candidate(%d) excceed %d", __func__,
			  p_btm_req_data->num_candidates, MAX_CANDIDATE_NUM));
		return NDIS_STATUS_FAILURE;
	}

	/*send btm req msg to mlme*/
	Len = sizeof(*Event) + btm_req_data_len;
	os_alloc_mem(NULL, (UCHAR **)&pBuf, Len);
	if (!pBuf) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s Not available memory for btm req msg\n", __func__));
		return NDIS_STATUS_RESOURCES;
	}

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, p_btm_req_data->sta_mac)) {
			IsFound = TRUE;
			break;
		}
	}
	if (IsFound) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s() found BTMPeerEntry in BTMEntrylist that say receive btm query before\n",
			__func__));
		BTMPeerEntry->CurrentState = WAIT_BTM_REQ;
		BTMPeerEntry->WaitPeerBTMRspTime = p_btm_req_data->timeout * 1000;
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	if (!IsFound) {
		os_alloc_mem(NULL, (UCHAR **)&BTMPeerEntry, sizeof(*BTMPeerEntry));

		if (!BTMPeerEntry) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s Not available memory for BTMPeerEntry\n", __func__));
			os_free_mem(NULL, pBuf);
			return NDIS_STATUS_RESOURCES;
		}

		NdisZeroMemory(BTMPeerEntry, sizeof(*BTMPeerEntry));
		BTMPeerEntry->CurrentState = WAIT_BTM_REQ;
		BTMPeerEntry->ControlIndex = ifIndex;
		NdisMoveMemory(BTMPeerEntry->PeerMACAddr, p_btm_req_data->sta_mac,
			MAC_ADDR_LEN);
		if (p_btm_req_data->dialogtoken) {
			BTMPeerEntry->DialogToken = p_btm_req_data->dialogtoken;
		} else {
			i++;
			if (i == 0)
				i = 1;
			BTMPeerEntry->DialogToken = i;
			p_btm_req_data->dialogtoken = i;
		}
		BTMPeerEntry->Priv = pAd;
		BTMPeerEntry->WaitPeerBTMRspTime = p_btm_req_data->timeout * 1000;
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
		DlListAddTail(&pWNMCtrl->BTMPeerList, &BTMPeerEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
	}

	NdisZeroMemory(pBuf, Len);
	Event = (PBTM_EVENT_DATA)pBuf;
	Event->ControlIndex = ifIndex;
	NdisMoveMemory(Event->PeerMACAddr,
		p_btm_req_data->sta_mac, MAC_ADDR_LEN);
	Event->EventType = BTM_REQ_PARAM;
	/*
	*if we receive btm query and btm req is sent in query session
	* then we think the btm req sent for btm query
	*/
	Event->u.BTM_REQ_DATA.DialogToken = p_btm_req_data->dialogtoken;
	Event->u.BTM_REQ_DATA.BTMReqLen = btm_req_data_len;
	NdisMoveMemory(Event->u.BTM_REQ_DATA.BTMReq,
		(PUCHAR)p_btm_req_data, btm_req_data_len);

	if (!MlmeEnqueue(pAd, BTM_STATE_MACHINE, BTM_REQ_PARAM, Len, pBuf, 0)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s() mlme enqueue failed", __func__));
		if (!IsFound) {
			RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
			DlListDel(&BTMPeerEntry->List);
			RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
			os_free_mem(NULL, BTMPeerEntry);
		}
	}

	/*free the memory*/
	os_free_mem(NULL, pBuf);

	return NDIS_STATUS_SUCCESS;
}

int check_btm_custom_params(
	IN PRTMP_ADAPTER pAd,
	IN p_btm_reqinfo_t p_btm_req_data,
	IN UINT32 btm_req_data_len)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	struct nr_info *pinfo = NULL;
	INT32 Len = 0;
	UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	pEntry = MacTableLookup(pAd, p_btm_req_data->sta_mac);
	if (!pEntry || !((Ndis802_11AuthModeAutoSwitch >=
		pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.AuthMode) ||
		((pEntry->WpaState == AS_PTKINITDONE) &&
		(pEntry->GTKState == REKEY_ESTABLISHED)))) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() STA(%02x:%02x:%02x:%02x:%02x:%02x) not associates with AP!\n",
			__func__, PRINT_MAC(p_btm_req_data->sta_mac)));
		goto error;
	}


	if (!pEntry->BssTransitionManmtSupport) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s() STA(%02x:%02x:%02x:%02x:%02x:%02x) not support btm!\n",
			__func__, PRINT_MAC(p_btm_req_data->sta_mac)));
		goto error;
	}

	if (p_btm_req_data->num_candidates == 0) {
		DBGPRINT(RT_DEBUG_WARN,
			("%s() candidate count equals to 0; btm request is meaningless\n",
			__func__));
		return NDIS_STATUS_SUCCESS;
	}

	Len = p_btm_req_data->num_candidates * sizeof(struct nr_info);
	pinfo = p_btm_req_data->candidates;

	/*
	*need check the validity of each neighbor report candidates
	*check bssid field of each neighbor report candidates
	*/
	while (Len > 0) {
		if (MAC_ADDR_EQUAL(pinfo->bssid, ZERO_MAC_ADDR)) {
			DBGPRINT(RT_DEBUG_ERROR,
				("%s() bssid check failed\n", __func__));
			goto error;
		}
		if (pinfo->channum == 0) {
			DBGPRINT(RT_DEBUG_ERROR,
				("%s() channel check failed\n", __func__));
			goto error;
		}

		Len -= sizeof(struct nr_info);
		pinfo++;
	}

	return NDIS_STATUS_SUCCESS;

error:
	return NDIS_STATUS_FAILURE;
}

int compose_btm_req_ie(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR p_btm_req_ie,
	OUT PUINT32 p_btm_req_ie_len,
	IN p_btm_reqinfo_t p_btm_req_data,
	IN UINT32 btm_req_data_len)
{
	struct nr_info *p_info = p_btm_req_data->candidates;
	BOOLEAN nr_flag = FALSE;
	ULONG bss_index = BSS_NOT_FOUND;
	BSS_ENTRY *pBssEntry = NULL;
	RRM_NEIGHBOR_REP_INFO NeighborRepInfo;
	RRM_BSSID_INFO BssidInfo;
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	PUCHAR pos = p_btm_req_ie;
	UINT32 TmpLen = 0, i = 0;
	UINT16 disassoc_timer = 0;

	DBGPRINT(RT_DEBUG_TRACE,
		("%s\n", __func__));

	/*set Request Mode*/
	if (p_btm_req_data->disassoc_timer)
		p_btm_req_data->reqmode |= BTM_DISASSOC_OFFSET;
	else
		p_btm_req_data->reqmode &= ~BTM_DISASSOC_OFFSET;

	if (p_btm_req_data->duration)
		p_btm_req_data->reqmode |= BTM_TERMINATION_OFFSET;
	else
		p_btm_req_data->reqmode &= ~BTM_TERMINATION_OFFSET;

	if (p_btm_req_data->url_len)
		p_btm_req_data->reqmode |= BTM_URL_OFFSET;
	else
		p_btm_req_data->reqmode &= ~BTM_URL_OFFSET;

	p_btm_req_data->reqmode &= ~BTM_CANDIDATE_OFFSET;

	/*fill request mode into ie buffer*/
	memcpy(pos+TmpLen, &p_btm_req_data->reqmode, 1);
	TmpLen += 1;

	disassoc_timer = cpu2le16(p_btm_req_data->disassoc_timer);

	/*fill disassociation timer into ie buffer*/
	memcpy(pos+TmpLen, &disassoc_timer, 2);
	TmpLen += 2;
	/*fill validity interval into ie buffer*/
	memcpy(pos+TmpLen, &p_btm_req_data->valint, 1);
	TmpLen += 1;
	/*fill bss termination duration into ie buffer; optional*/
	if (p_btm_req_data->reqmode & BTM_TERMINATION_OFFSET) {
		WNM_InsertBSSTerminationSubIE(pAd, pos+TmpLen, &TmpLen,
			p_btm_req_data->TSF,
			p_btm_req_data->duration);
	}
	if (p_btm_req_data->reqmode & BTM_URL_OFFSET) {
		memcpy(pos + TmpLen, &p_btm_req_data->url_len, 1);
		TmpLen += 1;
		memcpy(pos + TmpLen, &p_btm_req_data->url,
			p_btm_req_data->url_len);
		TmpLen += p_btm_req_data->url_len;
	}
	/*before add neighbor report ie*/
	*p_btm_req_ie_len = TmpLen;
	/*after add neighbor report ie*/
	for (i = 0; i < p_btm_req_data->num_candidates; i++, p_info++) {
		/*
		*first, check if need get info from scan table to compose
		*neighbor report response ie
		*/
		if (!(p_info->phytype &&
			p_info->regclass &&
			p_info->capinfo &&
			p_info->is_ht &&
			p_info->is_vht &&
			p_info->mobility)) {
			nr_flag = TRUE;
		}
		/*
		*if nr_flag equals to TRUE, it means that the up-layer need driver to
		*help with the build of neighbor report ie. so need find info to complete
		*the ie from scan table matched by bssid. if do not find info in
		*scan table, just skip this candidate
		*/
		if (nr_flag) {
			bss_index =
				BssTableSearch(&pAd->ScanTab, p_info->bssid, p_info->channum);
			if (bss_index == BSS_NOT_FOUND) {
				DBGPRINT(RT_DEBUG_WARN,
				("%s() bss not found\n", __func__));
				continue;
			}
			pBssEntry = &pAd->ScanTab.BssEntry[bss_index];
			if (p_info->regclass == 0)
				p_info->regclass = pBssEntry->RegulatoryClass;
			if (p_info->capinfo == 0)
				p_info->capinfo = pBssEntry->CapabilityInfo;
			if (p_info->is_ht == 0)
				p_info->is_ht = (pBssEntry->HtCapabilityLen != 0)?1:0;
#ifdef DOT11_VHT_AC
			if (p_info->is_vht == 0)
				p_info->is_vht = (pBssEntry->vht_cap_len != 0)?1:0;
#endif /* DOT11_VHT_AC */
			if (p_info->is_ht == 0)
				p_info->is_ht = (pBssEntry->HtCapabilityLen != 0)?1:0;
			if (p_info->mobility == 0)
				p_info->mobility = (pBssEntry->bHasMDIE)?1:0;

			if (pBssEntry->Channel > 14) {
				if (pBssEntry->HtCapabilityLen != 0) {
#ifdef DOT11_VHT_AC
					if (pBssEntry->vht_cap_len != 0)
						pBssEntry->CondensedPhyType = 9;
					else
#endif /* DOT11_VHT_AC */
						pBssEntry->CondensedPhyType = 7;
				} else { /* OFDM case */
					pBssEntry->CondensedPhyType = 4;
				}
			} else {
				if (pBssEntry->HtCapabilityLen != 0) /* HT case */
					pBssEntry->CondensedPhyType = 7;
				else if (ERP_IS_NON_ERP_PRESENT(pBssEntry->Erp)) /* ERP case */
					pBssEntry->CondensedPhyType = 6;
				else if (pBssEntry->SupRateLen > 4)	/* OFDM case (1,2,5.5,11 for CCK 4 Rates) */
					pBssEntry->CondensedPhyType = 4;
				/* no CCK's definition in spec. */
			}
			if (p_info->phytype == 0)
				p_info->phytype = pBssEntry->CondensedPhyType;
		}

		/*compose neighbor report ie buffer*/
		BssidInfo.word = 0;
		BssidInfo.field.APReachAble =
			(p_info->ap_reachability == 0)?3:p_info->ap_reachability;
		BssidInfo.field.Security = p_info->security;
		BssidInfo.field.KeyScope = p_info->key_scope;
		BssidInfo.field.SepctrumMng = (p_info->capinfo & (1 << 8))?1:0;
		BssidInfo.field.Qos = (p_info->capinfo & (1 << 9))?1:0;
		BssidInfo.field.APSD = (p_info->capinfo & (1 << 11))?1:0;
		BssidInfo.field.RRM = (p_info->capinfo & RRM_CAP_BIT)?1:0;
		BssidInfo.field.DelayBlockAck = (p_info->capinfo & (1 << 14))?1:0;
		BssidInfo.field.ImmediateBA = (p_info->capinfo & (1 << 15))?1:0;
		BssidInfo.field.MobilityDomain = p_info->mobility;
		BssidInfo.field.HT = p_info->is_ht;
#ifdef DOT11_VHT_AC
		BssidInfo.field.VHT = p_info->is_vht;
#endif /* DOT11_VHT_AC */
		COPY_MAC_ADDR(NeighborRepInfo.Bssid, p_info->bssid);
		NeighborRepInfo.BssidInfo = cpu2le32(BssidInfo.word);
		NeighborRepInfo.RegulatoryClass = p_info->regclass;
		NeighborRepInfo.ChNum = p_info->channum;
		NeighborRepInfo.PhyType = p_info->phytype;

		RRM_InsertNeighborRepIE_New(pAd, (pos+TmpLen), (PULONG)&TmpLen,
				sizeof(RRM_NEIGHBOR_REP_INFO), &NeighborRepInfo);
		if (p_info->preference)
			RRM_InsertPreferenceSubIE(pAd, (pos+TmpLen), &TmpLen,
				p_info->preference);
	}
	if (TmpLen > *p_btm_req_ie_len) {
		*p_btm_req_ie_len = TmpLen;
		pos[0] |= BTM_CANDIDATE_OFFSET;
	}

	return NStatus;
}

VOID WNM_InsertBSSTerminationSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PUINT32 pFrameLen,
	IN UINT64 TSF,
	IN UINT16 Duration)
{
	ULONG TempLen = 0;
	UINT8 Len = 10;
	UINT8 SubId = WNM_BSS_TERMINATION_SUBIE;

#ifdef RT_BIG_ENDIAN
	TSF = cpu2le64(TSF);
	Duration = cpu2le16(Duration);
#endif

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&SubId,
						1,				&Len,
						8,				&TSF,
						2,				&Duration,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

VOID RRM_InsertPreferenceSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PUINT32 pFrameLen,
	IN UINT8 preference)
{
	ULONG TempLen = 0;
	UINT8 Len = 1;
	UINT8 SubId = WNM_BSS_PERFERENCE_SUBIE;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&SubId,
						1,				&Len,
						1,				&preference,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}


#ifdef CONFIG_AP_SUPPORT
VOID WNMSetPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem,
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
	if (Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n", __func__, Ret));

	DlListForEach(WNMNotifyPeerEntry, &pWNMCtrl->WNMNotifyPeerList,
							WNM_NOTIFY_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(WNMNotifyPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
			WNMNotifyPeerEntry->CurrentState = State;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);
}

enum WNM_NOTIFY_STATE WNMNotifyPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PWNM_CTRL pWNMCtrl;
	PWNM_NOTIFY_PEER_ENTRY WNMNotifyPeerEntry;
	PWNM_NOTIFY_EVENT_DATA Event = (PWNM_NOTIFY_EVENT_DATA)Elem->Msg;
	INT32 Ret;

#ifdef CONFIG_AP_SUPPORT
	pWNMCtrl = &pAd->ApCfg.MBSSID[Event->ControlIndex].WNMCtrl;
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n", __func__, Ret));

	DlListForEach(WNMNotifyPeerEntry, &pWNMCtrl->WNMNotifyPeerList, WNM_NOTIFY_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(WNMNotifyPeerEntry->PeerMACAddr, Event->PeerMACAddr)) {
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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
	DlListDel(&WNMNotifyPeerEntry->List);
	RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);
	os_free_mem(NULL, WNMNotifyPeerEntry);
}
BUILD_TIMER_FUNCTION(WaitPeerWNMNotifyRspTimeout);
VOID ReceiveWNMNotifyReq(IN PRTMP_ADAPTER pAd,
						IN MLME_QUEUE_ELEM *Elem)
{

	WNM_FRAME *WNMFrame = (WNM_FRAME *)Elem->Msg;
	UINT	pos = 0;
	/* skip  category, action, DialogToken, type */
	UINT	OptionalElementLen = (UINT)Elem->MsgLen - sizeof(HEADER_802_11) - 4;
	UINT	ElementID = 0, ElementLen = 0;
#ifdef MBO_SUPPORT
	struct wifi_dev *pWdev = wdev_search_by_address(pAd, WNMFrame->Hdr.Addr1);

	MBO_STA_CH_PREF_CDC_INFO MboStaInfoNPC;
	MBO_STA_CH_PREF_CDC_INFO MboStaInfoCDC;
	BOOLEAN bIndicateNPC = FALSE;
#endif /* MBO_SUPPORT */

#ifdef MBO_SUPPORT
	NdisZeroMemory(&MboStaInfoNPC, sizeof(MBO_STA_CH_PREF_CDC_INFO));
	COPY_MAC_ADDR(MboStaInfoNPC.mac_addr, WNMFrame->Hdr.Addr2);
	NdisZeroMemory(&MboStaInfoCDC, sizeof(MBO_STA_CH_PREF_CDC_INFO));
	COPY_MAC_ADDR(MboStaInfoCDC.mac_addr, WNMFrame->Hdr.Addr2);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s sta mac %02x:%02x:%02x:%02x:%02x:%02x\n",
		__func__, PRINT_MAC(WNMFrame->Hdr.Addr2)));
#endif /* MBO_SUPPORT */


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s MsgLen %ld MBSS %02x:%02x:%02x:%02x:%02x:%02x\n",
		__func__, Elem->MsgLen, PRINT_MAC(WNMFrame->Hdr.Addr1)));

	while ((pos+1) <= OptionalElementLen) {
		ElementID = WNMFrame->u.WNM_NOTIFY_REQ.Variable[pos];
		ElementLen = WNMFrame->u.WNM_NOTIFY_REQ.Variable[pos+1];

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s, %u pos %d OptionalElementLen %d ElementID %d ElementLen %d\n",
		__func__, __LINE__, pos, OptionalElementLen, ElementID, ElementLen));

		pos += 2;

		switch (ElementID) {
		case IE_VENDOR_SPECIFIC:
			if (NdisEqualMemory(wfa_oui, &WNMFrame->u.WNM_NOTIFY_REQ.Variable[pos], 3)) {
				UINT8 ouiType = WNMFrame->u.WNM_NOTIFY_REQ.Variable[pos+3];

				switch (ouiType) {
#ifdef MBO_SUPPORT
/* Non-Preferred Channel Report : NPC Report */
				case MBO_OUI_NON_PREFERRED_CHANNEL_REPORT:
					MboParseStaNPCElement(pAd, pWdev, &WNMFrame->u.WNM_NOTIFY_REQ.Variable[pos+4]
					, ElementLen, &MboStaInfoNPC, MBO_FRAME_TYPE_WNM_REQ);
					bIndicateNPC = TRUE;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s, %u npc_num %d mac_addr %02X:%02X:%02X:%02X:%02X:%02X bIndicateNPC %d\n"
					, __func__, __LINE__, MboStaInfoNPC.npc_num,
					PRINT_MAC(MboStaInfoNPC.mac_addr), bIndicateNPC));
					break;
				case MBO_OUI_CELLULAR_DATA_CAPABILITY:
					MboStaInfoCDC.cdc = WNMFrame->u.WNM_NOTIFY_REQ.Variable[pos+4];
					MboStaInfoCDC.npc_num = 0;
					MboIndicateStaInfoToDaemon(pAd, &MboStaInfoCDC, MBO_MSG_CDC_UPDATE);
					break;
#endif/* MBO_SUPPORT */
				default:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("\033[1;32m %s, %u vendor specific, but unknown OUI type, please check \033[0m\n",
			__func__, __LINE__));
				}
			} else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s, %u vendor specific, but unknown OUI, please check\n",
					__func__, __LINE__));
			}
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s, UNKNOWN ElementID 0x%X , break parsing\n", __func__, ElementID));
		}

		pos += ElementLen; /* forward to next element */
	}


#ifdef MBO_SUPPORT
	if (bIndicateNPC)
		MboIndicateStaInfoToDaemon(pAd, &MboStaInfoNPC, MBO_MSG_STA_PREF_UPDATE);
#endif /* MBO_SUPPORT */

	/*return;*/
}
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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
	if(Ret != 0)
		DBGPRINT(RT_DEBUG_OFF, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,Ret));
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
