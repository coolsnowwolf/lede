/*

*/

#define MIN_NET_DEVICE_FOR_MBSSID		0x00		/*0x00,0x10,0x20,0x30 */
#define MIN_NET_DEVICE_FOR_WDS			0x10		/*0x40,0x50,0x60,0x70 */
#define MIN_NET_DEVICE_FOR_APCLI		0x20
#define MIN_NET_DEVICE_FOR_MESH			0x30


#define NET_DEVICE_REAL_IDX_MASK		0x0f		/* for each operation mode, we maximum support 15 entities. */


/******************************************************************************
	Packet Meta info fields
	NOTE: the meta info may stored in some os-dependent packet buffer and may
			have extra limitations.
		1. Linux:
			a).re-use sk_budd->cb[]
			b).Max space of skb->cb = 48B = [CB_OFF+38]
		2. VxWorks:
			a).
			b).Max space: 32!!
******************************************************************************/
/* [CB_OFF + 0]  */
/* User Priority */
#define RTMP_SET_PACKET_UP(_p, _prio)			(PACKET_CB(_p, 0) = _prio)
#define RTMP_GET_PACKET_UP(_p)					(PACKET_CB(_p, 0))

/* [CB_OFF + 1]  */
/* Fragment # */
#define RTMP_SET_PACKET_FRAGMENTS(_p, _num)	(PACKET_CB(_p, 1) = _num)
#define RTMP_GET_PACKET_FRAGMENTS(_p)		(PACKET_CB(_p, 1))

/* [CB_OFF + 2]  */
/*
	0x0 ~ 0xff: index mapping to MAC_TABLE_ENTRY
				for Tx, search by eth_hdr.addr1
				for Rx, get from ASIC(RxWI)/search by wlan_hdr.addr2
*/
#define RTMP_SET_PACKET_WCID(_p, _wdsidx)		(PACKET_CB(_p, 2) = _wdsidx)
#define RTMP_GET_PACKET_WCID(_p)				(PACKET_CB(_p, 2))


/* [CB_OFF + 3]  */
/* From which Interface, wdev index */
#define RTMP_SET_PACKET_WDEV(_p, _wdev_idx)		(PACKET_CB(_p, 3) = _wdev_idx)
#define RTMP_GET_PACKET_WDEV(_p)					(PACKET_CB(_p, 3))



/* [CB_OFF + 4]  */
/* If this flag is set, it indicates that this EAPoL frame MUST be clear. */
#define RTMP_SET_PACKET_CLEAR_EAP_FRAME(_p, _flg)   (PACKET_CB(_p, 4) = _flg)
#define RTMP_GET_PACKET_CLEAR_EAP_FRAME(_p)         (PACKET_CB(_p, 4))


/* [CB_OFF + 5]  */
#define RTMP_SET_PACKET_MOREDATA(_p, _morebit)		(PACKET_CB(_p, 5) = _morebit)
#define RTMP_GET_PACKET_MOREDATA(_p)					(PACKET_CB(_p, 5))


/* [CB_OFF + 6]  */
/*
	Sepcific Pakcet Type definition
*/
#define RTMP_PACKET_SPECIFIC_CB_OFFSET	6

#define RTMP_PACKET_SPECIFIC_DHCP		0x01
#define RTMP_PACKET_SPECIFIC_EAPOL	0x02
#define RTMP_PACKET_SPECIFIC_IPV4		0x04
#define RTMP_PACKET_SPECIFIC_WAI		0x08
#define RTMP_PACKET_SPECIFIC_VLAN		0x10
#define RTMP_PACKET_SPECIFIC_LLCSNAP	0x20
#define RTMP_PACKET_SPECIFIC_TDLS		0x40
#define RTMP_PACKET_SPECIFIC_DISASSOC	0x80

/* Specific */
#define RTMP_SET_PACKET_SPECIFIC(_p, _flg)	   	(PACKET_CB(_p, 6) = _flg)

/* DHCP */
#define RTMP_SET_PACKET_DHCP(_p, _flg)   								\
			do{															\
				if (_flg)												\
					PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_DHCP);	\
				else													\
					PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_DHCP);	\
			}while(0)
#define RTMP_GET_PACKET_DHCP(_p)\
			(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_DHCP)

/* EAPOL */
#define RTMP_SET_PACKET_EAPOL(_p, _flg)   								\
			do{															\
				if (_flg)												\
					PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_EAPOL);	\
				else													\
					PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_EAPOL);	\
			}while(0)
#define RTMP_GET_PACKET_EAPOL(_p)\
			(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_EAPOL)

/* WAI */
#define RTMP_SET_PACKET_WAI(_p, _flg)   								\
			do{															\
				if (_flg)												\
					PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_WAI);	\
				else													\
					PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_WAI);	\
			}while(0)
#define RTMP_GET_PACKET_WAI(_p) \
			(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_WAI)

#define RTMP_GET_PACKET_FORCE_TX(_p)\
		(PACKET_CB(_p, 6) & (RTMP_PACKET_SPECIFIC_EAPOL | \
							RTMP_PACKET_SPECIFIC_WAI))

#define RTMP_GET_PACKET_LOWRATE(_p) \
			(PACKET_CB(_p, 6) & (RTMP_PACKET_SPECIFIC_EAPOL |\
								RTMP_PACKET_SPECIFIC_DHCP |\
								RTMP_PACKET_SPECIFIC_WAI))

/* VLAN */
#define RTMP_SET_PACKET_VLAN(_p, _flg)   								\
			do{															\
				if (_flg)												\
					PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_VLAN);	\
				else													\
					PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_VLAN);	\
			}while(0)
#define RTMP_GET_PACKET_VLAN(_p) \
			(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_VLAN)

/* LLC/SNAP */
#define RTMP_SET_PACKET_LLCSNAP(_p, _flg)   								\
			do{																\
				if (_flg)													\
					PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_LLCSNAP);	\
				else														\
					PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_LLCSNAP);	\
			}while(0)
			
#define RTMP_GET_PACKET_LLCSNAP(_p) \
			(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_LLCSNAP)

/* IP */
#define RTMP_SET_PACKET_IPV4(_p, _flg)									\
			do{															\
				if (_flg)												\
					PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_IPV4);	\
				else													\
					PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_IPV4);	\
			}while(0)
			
#define RTMP_GET_PACKET_IPV4(_p) \
			(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_IPV4)

/* TDLS */
#define RTMP_SET_PACKET_TDLS_MMPDU(_p, _flg)									\
			do{															\
				if (_flg)												\
					PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_TDLS);	\
				else														\
					PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_TDLS);	\
			}while(0)
			
#define RTMP_GET_PACKET_TDLS_MMPDU(_p) \
			(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_TDLS)

/* DISASSOC */
#define RTMP_SET_PACKET_DISASSOC(_p, _flg)                                                      \
            do{                                                                             \
                if (_flg)                                                                   \
                    (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_DISASSOC);     \
                else                                                                        \
                    (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (~RTMP_PACKET_SPECIFIC_DISASSOC);    \
            }while(0)
#define RTMP_GET_PACKET_DISASSOC(_p)   \
			(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_DISASSOC)

#define RTMP_SET_PACKET_ICMP(_p, _flg)   (PACKET_CB(_p, 7) = _flg)
#define RTMP_GET_PACKET_ICMP(_p)         (PACKET_CB(_p, 7))

/* [CB_OFF + 7]  */


/* [CB_OFF + 8 ~ 9]  */
#ifdef UAPSD_SUPPORT
/* if we queue a U-APSD packet to any software queue, we will set the U-APSD
   flag and its physical queue ID for it */
#define RTMP_SET_PACKET_UAPSD(_p, _flg_uapsd, _que_id) \
                    (PACKET_CB(_p, 8) = ((_flg_uapsd<<7) | _que_id))

#define RTMP_SET_PACKET_QOS_NULL(_p)			(PACKET_CB(_p, 8) = 0xff)
#define RTMP_GET_PACKET_QOS_NULL(_p)			(PACKET_CB(_p, 8))
#define RTMP_SET_PACKET_NON_QOS_NULL(_p)		(PACKET_CB(_p, 8) = 0x00)
#define RTMP_GET_PACKET_UAPSD_Flag(_p)		((PACKET_CB(_p, 8) & 0x80) >> 7)
#define RTMP_GET_PACKET_UAPSD_QUE_ID(_p)		(PACKET_CB(_p, 8) & 0x7f)

#define RTMP_SET_PACKET_EOSP(_p, _flg)			(PACKET_CB(_p, 9) = _flg)
#define RTMP_GET_PACKET_EOSP(_p)				(PACKET_CB(_p, 9))
#endif /* UAPSD_SUPPORT */


/* [CB_OFF + 10 ~ 11]  */
#define RTMP_SET_PACKET_PROTOCOL(_p, _protocol) {\
	(PACKET_CB(_p, 10) = (UINT8)((_protocol) & 0x00ff)); \
	(PACKET_CB(_p, 11) = (UINT8)(((_protocol) & 0xff00) >> 8)); \
}

#define RTMP_GET_PACKET_PROTOCOL(_p) \
	((((UINT16)PACKET_CB(_p, 11)) << 8) \
	| ((UINT16)PACKET_CB(_p, 10)))


/* [CB_OFF + 12]  */
#define RTMP_SET_PACKET_TXTYPE(_p, _mode)		(PACKET_CB(_p, 12) = _mode)
#define RTMP_GET_PACKET_TXTYPE(_p)			(PACKET_CB(_p, 12))

/* [CB_OFF + 13]  */
/* TX rate index */
#define RTMP_SET_PACKET_TXRATE(_p, _rate)		(PACKET_CB(_p, 13) = _rate)
#define RTMP_GET_PACKET_TXRATE(_p)			(PACKET_CB(_p, 13))


/* [CB_OFF + 14]  */
#ifdef MAC_REPEATER_SUPPORT
#define RTMP_SET_PKT_MAT_FREE(_p, _flg)	(PACKET_CB(_p, 14) = (_flg))
#define RTMP_GET_PKT_MAT_FREE(_p)			(PACKET_CB(_p, 14))
#endif /* MAC_REPEATER_SUPPORT */


/* [CB_OFF + 15 ~ 19]  */

/* use bit3 of cb[CB_OFF+16] */
#define RTMP_SET_PACKET_MGMT_PKT(_p, _flg)	        \
        PACKET_CB(_p, 16) = (PACKET_CB(_p, 16) & 0xF7) | ((_flg & 0x01) << 3);
#define RTMP_GET_PACKET_MGMT_PKT(_p)				\
		((PACKET_CB(_p, 16) & 0x08) >> 3)

/* use bit0 of cb[CB_OFF+20] */
#define RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(_p, _flg)	\
        PACKET_CB(_p, 20) = (PACKET_CB(_p, 20) & 0xFE) | (_flg & 0x01);
#define RTMP_GET_PACKET_MGMT_PKT_DATA_QUE(_p)		\
		(PACKET_CB(_p, 20) & 0x01)


/* [CB_OFF+21 ~ 22]  */


/* [CB_OFF + 23]  */
/*
 *	TDLS Sepcific Pakcet Type definition
*/
#define RTMP_TDLS_SPECIFIC_WAIT_ACK		0x01
#define RTMP_TDLS_SPECIFIC_NOACK			0x02
#define RTMP_TDLS_SPECIFIC_PKTQ_HCCA		0x04
#define RTMP_TDLS_SPECIFIC_PKTQ_EDCA		0x08

#define RTMP_SET_PACKET_TDLS_WAIT_ACK(_p, _flg)						\
			do{															\
				if (_flg)													\
					PACKET_CB(_p, 23) |= (RTMP_TDLS_SPECIFIC_WAIT_ACK);	\
				else														\
					PACKET_CB(_p, 23) &= (~RTMP_TDLS_SPECIFIC_WAIT_ACK);	\
			}while(0)
			
#define RTMP_GET_PACKET_TDLS_WAIT_ACK(_p)		(PACKET_CB(_p, 23) & RTMP_TDLS_SPECIFIC_WAIT_ACK)

#define RTMP_SET_PACKET_TDLS_NO_ACK(_p, _flg)						\
			do{														\
				if (_flg)												\
					PACKET_CB(_p, 23) |= (RTMP_TDLS_SPECIFIC_NOACK);	\
				else													\
					PACKET_CB(_p, 23) &= (~RTMP_TDLS_SPECIFIC_NOACK);	\
			}while(0)
			
#define RTMP_GET_PACKET_TDLS_NO_ACK(_p)		(PACKET_CB(_p, 23) & RTMP_TDLS_SPECIFIC_NOACK)

#define RTMP_SET_TDLS_SPECIFIC_PACKET(_p, _flg)   (PACKET_CB(_p, 23) = _flg)
#define RTMP_GET_TDLS_SPECIFIC_PACKET(_p)         (PACKET_CB(_p, 23))


/* [CB_OFF + 24]  */
#ifdef INF_AMAZON_SE
/* [CB_OFF+25], 1B, Iverson patch for WMM A5-T07 ,WirelessStaToWirelessSta do not bulk out aggregate */
#define RTMP_SET_PACKET_NOBULKOUT(_p, _morebit)			(PACKET_CB(_p, 24) = _morebit)
#define RTMP_GET_PACKET_NOBULKOUT(_p)					(PACKET_CB(_p, 24))
#else
#define RTMP_SET_PACKET_ETHTYPE(_p, _morebit)			(PACKET_CB(_p, 24) = _morebit)
#define RTMP_GET_PACKET_ETHTYPE(_p)						(PACKET_CB(_p, 24))
#endif /* INF_AMAZON_SE */
/* Max skb->cb = 48B = [CB_OFF+38] */


/* [CB_OFF + 25]  */
/* ATTENTION PLEASE:
[CB_OFF + 25]  have been used for macro MAX_CONTINUOUS_TX_CNT,
this is a feature for AP to continously transmit 21 packets!!!*/


/* [CB_OFF + 26]  */


/* [CB_OFF + 27 ~ 31]  */


/* [CB_OFF + 32]  */
/* RTS/CTS-to-self protection method */
#define RTMP_SET_PACKET_RTS(_p, _num)			(PACKET_CB(_p, 32) = _num)
#define RTMP_GET_PACKET_RTS(_p)				(PACKET_CB(_p, 32))
/* see RTMP_S(G)ET_PACKET_EMACTAB */

/* [CB_OFF + 33]  */
#ifdef CONFIG_5VT_ENHANCE
#define RTMP_SET_PACKET_5VT(_p, _flg)   (PACKET_CB(_p, 33) = _flg)
#define RTMP_GET_PACKET_5VT(_p)         (PACKET_CB(_p, 33))
#endif /* CONFIG_5VT_ENHANCE */

/* [CB_OFF + 34]  */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
/* set link cover packet send by 5G or 2G */
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
#define PACKET_BAND_CB	36
#endif
#define RTMP_PACKET_SPECIFIC_2G		0x1
#define RTMP_PACKET_SPECIFIC_5G		0x2
#define RTMP_PACKET_SPECIFIC_ETHER	0x4
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
#define RTMP_PACKET_SPECIFIC_5G_H	0x10

#define RTMP_CLEAN_PACKET_BAND(_p)   (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+36] = 0)
#endif

#if (MT7615_MT7603_COMBO_FORWARDING == 1)
#define RTMP_SET_PACKET_BAND(_p, _flg)   	\
	do{										\
		if (_flg)								\
			PACKET_CB(_p, 36) |= (_flg);		\
		else									\
			PACKET_CB(_p, 36) &= (~_flg);		\
	}while(0)

#define RTMP_GET_PACKET_BAND(_p)	(PACKET_CB(_p, 33))
/* [CB_OFF + 35]: tag the packet received from which net device */
#define RECV_FROM_CB			37
#define H_CHANNEL_BIGGER_THAN   100

#else
#define RTMP_SET_PACKET_BAND(_p, _flg)   	\
	do{										\
		if (_flg)								\
			PACKET_CB(_p, 34) |= (_flg);		\
		else									\
			PACKET_CB(_p, 34) &= (~_flg);		\
	}while(0)

#define RTMP_GET_PACKET_BAND(_p)	(PACKET_CB(_p, 34))
#endif
/* [CB_OFF + 35]: tag the packet received from which net device */
#if (MT7615_MT7603_COMBO_FORWARDING == 1)

#define RECV_FROM_CB			37
#define H_CHANNEL_BIGGER_THAN   100
#define RTMP_PACKET_RECV_FROM_5G_H_CLIENT   0x10
#define RTMP_PACKET_RECV_FROM_5G_H_AP     	0x20

#define RTMP_CLEAN_PACKET_RECV_FROM(_p)   (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+37] = 0)
#endif
#define RTMP_PACKET_RECV_FROM_2G_CLIENT 	0x1
#define RTMP_PACKET_RECV_FROM_5G_CLIENT 	0x2
#define RTMP_PACKET_RECV_FROM_2G_AP			0x4
#define RTMP_PACKET_RECV_FROM_5G_AP     		0x8

#if (MT7615_MT7603_COMBO_FORWARDING == 1)
#define RTMP_SET_PACKET_RECV_FROM(_p, _flg)	\
	do{                 	                        				\
		if (_flg)\
		PACKET_CB(_p, 37) |= (_flg);\
		else\
		PACKET_CB(_p, 37) &= (~_flg);\
	} while (0)

#define RTMP_GET_PACKET_RECV_FROM(_p)        (PACKET_CB(_p, 37))
#define RTMP_IS_PACKET_APCLI(_p)\
		((RTPKT_TO_OSPKT(_p)->cb[CB_OFF+37]) &\
		(RTMP_PACKET_RECV_FROM_2G_CLIENT | RTMP_PACKET_RECV_FROM_5G_CLIENT | RTMP_PACKET_RECV_FROM_5G_H_CLIENT))
#define RTMP_IS_PACKET_AP_APCLI(_p)	((RTPKT_TO_OSPKT(_p)->cb[CB_OFF+37]) != 0)

#else
#define RTMP_SET_PACKET_RECV_FROM(_p, _flg)	\
	do{                 	                        				\
          	if (_flg)                               				\
                	PACKET_CB(_p, 35) |= (_flg);    		\
                else                                   	 			\
                        PACKET_CB(_p, 35) &= (~_flg);   	\
          }while(0)

#define RTMP_GET_PACKET_RECV_FROM(_p)        (PACKET_CB(_p, 35))
#endif

#endif /* CONFIG_WIFI_PKT_FWD */
#ifdef MAX_CONTINUOUS_TX_CNT
#define RTMP_SET_PACKET_SWQINDEX_LOW(_p, _flg)   (PACKET_CB(_p, 25) = _flg)
#define RTMP_GET_PACKET_SWQINDEX_LOW(_p)         (PACKET_CB(_p, 25))
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
#define RTMP_SET_PACKET_SWQINDEX_HIGH(_p, _flg)   (PACKET_CB(_p, 34) = _flg)
#define RTMP_GET_PACKET_SWQINDEX_HIGH(_p)         (PACKET_CB(_p, 34))
#else
#define RTMP_SET_PACKET_SWQINDEX_HIGH(_p, _flg)   (PACKET_CB(_p, 37) = _flg)
#define RTMP_GET_PACKET_SWQINDEX_HIGH(_p)         (PACKET_CB(_p, 37))
#endif
#endif
