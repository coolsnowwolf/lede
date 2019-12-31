/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2015, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

    Module Name: reduce_tcpack.c

    Abstract:
	TCP ACK is accummulative such that one could drop TCP ACKs without
    retransmission. On the shared wireless medium, one could reduce TCP ACKs
    to save airtime of TCP ACKs. The saved airtime could be used for TCP DATA
    transmission and higher throughput will be expected, especially on low
    wireless MAC efficiency (i.e. non-aggregated) cases.

    Revision History:
    Who         When            What
    Hank Huang  2015/07/13      initial version
    --------    ----------      ----------------------------------------------
*/


#include "rt_config.h"
#include <linux/jhash.h>
#include <net/tcp.h>

/* Un-mark to turn on verbose debugging messages */
/* #define REDUCE_ACK_DEBUG */

#define REMOVE_REDUCE_PROB

#ifdef REDUCE_ACK_DEBUG
#define CNX_DPRINT(pAd, cnx, string)                           \
	do {                                                    \
		if ((pAd) == NULL || (pAd)->CommonCfg.ReduceAckEnable != REDUCE_ACK_ENABLE_WITOUT_DEL_CNX) {\
			break;                                         \
		}                                                  \
		if ((cnx) == NULL) {                               \
			printk("[rack] cnx is NULL (%s)\n", string);   \
			break;                                         \
		}                                                  \
		printk("[cnx] %u.%u.%u.%u:%u --> %u.%u.%u.%u:%u, seq=%u, rel_seq=%u, ack=%u, rel_ack=%u, win=%u, bif=%u (%s)\n",\
			   ((cnx)->tuple.sip >> 24) & 0xFF,           \
			   ((cnx)->tuple.sip >> 16) & 0xFF,           \
			   ((cnx)->tuple.sip >> 8) & 0xFF,            \
			   ((cnx)->tuple.sip >> 0) & 0xFF,            \
			   (cnx)->tuple.sport,                        \
			   ((cnx)->tuple.dip >> 24) & 0xFF,           \
			   ((cnx)->tuple.dip >> 16) & 0xFF,           \
			   ((cnx)->tuple.dip >> 8) & 0xFF,            \
			   ((cnx)->tuple.dip >> 0) & 0xFF,            \
			   (cnx)->tuple.dport,                        \
			   (cnx)->seq,                                \
			   (cnx)->rel_seq,                            \
			   (cnx)->sent_ack,                           \
			   (cnx)->rel_ack,                            \
			   (cnx)->wsize,                              \
			   (cnx)->bif,                                \
			   string);                                   \
	} while (0)

#define PKT_DPRINT(pAd, pkt, string)                           \
	do {                                                    \
		if ((pAd) == NULL || (pAd)->CommonCfg.ReduceAckEnable != REDUCE_ACK_ENABLE_WITOUT_DEL_CNX) {\
			break;                                         \
		}                                                  \
		if ((pkt) == NULL) {                               \
			printk("[pkt] pkt is NULL (%s)\n", string);   \
		}                                                  \
		printk("[pkt] %u.%u.%u.%u:%u --> %u.%u.%u.%u:%u, len=%u, opt_len=%u, flags=%x (%s)\n",\
			   ((pkt)->sip >> 24) & 0xFF,                 \
			   ((pkt)->sip >> 16) & 0xFF,                 \
			   ((pkt)->sip >> 8) & 0xFF,                  \
			   ((pkt)->sip >> 0) & 0xFF,                  \
			   (pkt)->sport,                              \
			   ((pkt)->dip >> 24) & 0xFF,                 \
			   ((pkt)->dip >> 16) & 0xFF,                 \
			   ((pkt)->dip >> 8) & 0xFF,                  \
			   ((pkt)->dip >> 0) & 0xFF,                  \
			   (pkt)->dport,                              \
			   (pkt)->data_len,                           \
			   (pkt)->opt_len,                            \
			   (pkt)->flags,                              \
			   string);                                   \
	} while (0)

#define MY_DPRINT(format, ...)                               \
	do {                                                 \
		printk("[rack] "format, ##__VA_ARGS__);          \
	} while (0)
#else
#define CNX_DPRINT(pAd, cnx, string) do {} while (0)
#define PKT_DPRINT(pAd, pkt, string) do {} while (0)
#define MY_DPRINT(format, ...) do {} while (0)
#endif

#define MY_LOCK(pAd) do {RTMP_SEM_LOCK(&pAd->ReduceAckLock); } while (0)
#define MY_UNLOCK(pAd) do {RTMP_SEM_UNLOCK(&pAd->ReduceAckLock); } while (0)

/*
========================================================================
Routine Description:
    Bypass RACK if incoming protocol is in the black list

Arguments:
    rack_packet  *pkt       The incoming TCP packet.

Return Value:
    FALSE if we could perform RACK

Note:
    None
========================================================================
*/
static BOOLEAN match_black_list(rack_packet *pkt)
{
	if (pkt->sport == 21 || pkt->dport == 21)
		return TRUE;

	if (pkt->sport == 23 || pkt->dport == 23)
		return TRUE;

	if (pkt->sport == 22 || pkt->dport == 22)
		return TRUE;

	return FALSE;
}

/*
========================================================================
Routine Description:
    The main decision logic

Arguments:
    RTMP_ADAPTER     *pAd       Pointer refer to the device handle.
    rack_packet      *pkt       The info of incoming ACK packet.
    rack_cnx         *cnx       The connection info of the incoming ACK packet.

Return Value:
    TRUE - The decision logic declared this incoming ACK is reduced.
    FALSE - RACK engine should send this incoming ACK immediately

Note:

========================================================================
*/
static BOOLEAN decide_to_drop(RTMP_ADAPTER *pAd, rack_packet *pkt, rack_cnx *cnx)
{
	UINT32 reduceProbability = pAd->CommonCfg.ReduceAckProbability;

	/* YC 0812 return FALSE, always for testing. */
	/* return FALSE; */

	/* Decision Check #1 */
	if (cnx->state != STATE_REDUCTION) {
		cnx->stats.pass_state++;
		return FALSE;
	}

	/* Decision Check #2 */
	if (cnx->ack_ratio > MAX_SUPPORTED_ACK_RATIO) {
		cnx->stats.pass_ack_ratio++;
		return FALSE;
	}

	/* Decision Check #3 */
	if (cnx->wsize_changed) {
		cnx->stats.pass_wsize_change++;
		return FALSE;
	}

	/* Decision Check #4 */
	if (cnx->consecutive_drop > MAX_CONSECUTIVE_DROP_CNT) {
		cnx->stats.pass_con_drop++;
		return FALSE;
	}

	/* Decision Check #5 */
	if (cnx->consecutive_data > MAX_CONSECUTIVE_DATA_CNT) {
		cnx->stats.pass_con_data++;
		return FALSE;
	}

	/* Decision Check #6 */
	if (cnx->bif + (cnx->ack_ratio) * (cnx->mss) >= cnx->avg_bif) {
		cnx->stats.pass_bif_warning++;
		return FALSE;
	}

	/* Decision Check #7 */
	if (pkt->ack == cnx->pushack) {
		cnx->stats.pass_push_data++;
		return FALSE;
	}

	/* Decision Check #8 */
	if (after(pkt->ack, cnx->pushack) && before(cnx->sent_ack, cnx->pushack)) {
		cnx->stats.pass_push_data3++;
		return FALSE;
	}

#ifdef REMOVE_REDUCE_PROB

	if (reduceProbability == 0)
		return FALSE;

	return TRUE;
#else

	if (cnx->consecutive_ack > REDUCE_PROB_INC_TH) {
		reduceProbability += REDUCE_ACK_ONE_PERCENT * REDUCE_PROB_INC_STEP * cnx->consecutive_ack;

		if (reduceProbability > 0x7FFFFFFF)
			cnx->stats.drop_prob++;
	}

	/* try to reduce ACK with specified prob (ReduceAckProbability divided by 0x80000000). */
	if ((random32() & 0x7FFFFFFF) < reduceProbability)
		return TRUE;

	return FALSE;
#endif
}

/*
========================================================================
Routine Description:
    The hash function for incoming pkt

Arguments:
    rack_packet      *pkt       The info of incoming ACK packet.

Return Value:
    The calculated index of hash table

Note:

========================================================================
*/
static UINT32 hash_cnx(rack_packet *pkt)
{
	UINT32 n, h, index;

	if (pkt->type == TCP_ACK) {
		n = jhash((void *)&(pkt->dip), sizeof(pkt->dip),
				  (pkt->dport << 16));
		h = jhash((void *)&(pkt->sip), sizeof(pkt->sip),
				  (pkt->sport | n));
	} else {
		n = jhash((void *)&(pkt->sip), sizeof(pkt->sip),
				  (pkt->sport << 16));
		h = jhash((void *)&(pkt->dip), sizeof(pkt->dip),
				  (pkt->dport | n));
	}

	index = ((u64)h * MAX_REDUCE_ACK_CNX_ENTRY) >> 32;
	index = index % REDUCE_ACK_MAX_HASH_BUCKETS;
	return index;
}

/*
========================================================================
Routine Description:
    The utility function find corresponding connection given a pkt

Arguments:
    RTMP_ADAPTER     *pAd       Pointer refer to the device handle.
    rack_packet      *pkt       The info of incoming ACK packet.

Return Value:
    The corresponding connection entry or NULL if not found.

Note:

========================================================================
*/
static rack_cnx *find_cnx(RTMP_ADAPTER *pAd, rack_packet *pkt)
{
#if LINUX_VERSION_CODE  < KERNEL_VERSION(3, 9, 0)
	struct hlist_node *tmp;
#endif
	rack_cnx *cnx = NULL;
	BOOLEAN bFound = FALSE;
	UINT32 hashIndex = hash_cnx(pkt);
#if LINUX_VERSION_CODE  < KERNEL_VERSION(3, 9, 0)
	hlist_for_each_entry(cnx, tmp, &(pAd->ackCnxHashTbl[hashIndex]), hnode) {
#else
	hlist_for_each_entry(cnx, &(pAd->ackCnxHashTbl[hashIndex]), hnode) {
#endif

		if (pkt->type == TCP_ACK && cnx->tuple.sip == pkt->sip &&
			cnx->tuple.dip == pkt->dip &&
			cnx->tuple.sport == pkt->sport &&
			cnx->tuple.dport == pkt->dport) {
			bFound = TRUE;
			break;
		}

		if (pkt->type == TCP_DATA && cnx->tuple.dip == pkt->sip &&
			cnx->tuple.sip == pkt->dip &&
			cnx->tuple.dport == pkt->sport &&
			cnx->tuple.sport == pkt->dport) {
			bFound = TRUE;
			break;
		}
	}
	return bFound ? cnx : NULL;
}

/*
========================================================================
Routine Description:
    The utility function to update BIF of a connection. Besides, the local
    max. of average BIF will be also calculated.

Arguments:
    rack_cnx         *cnx       The connection info we are going to calculate

Return Value:
    VOID

Note:
    One should update all necessary variables needed for calculation before
    invoking this fuction. You should update
    1. cnx.next_seq
    2. cnx.sent_ack
    3. cnx.consecutive_ack
    4. cnx.consecutive_drop
========================================================================
*/
static VOID calculate_bif(rack_cnx *cnx)
{
	if (cnx->next_seq != 0 && cnx->sent_ack != 0 && after(cnx->next_seq, cnx->sent_ack)) {
		if (cnx->consecutive_ack == 1 && cnx->consecutive_drop == 0) {
			if (cnx->avg_bif == 0)
				cnx->avg_bif = cnx->bif;

			cnx->avg_bif = (cnx->avg_bif + cnx->bif) / 2;
		}

		if (cnx->next_seq > cnx->sent_ack)
			cnx->bif = cnx->next_seq - cnx->sent_ack;
		else {
			/* handle wrap around case */
			cnx->bif = REDUCE_ACK_UINT32_MAX - cnx->sent_ack + cnx->next_seq;
		}

		if (cnx->bif > cnx->max_bif)
			cnx->max_bif = cnx->bif;
	}

	if (cnx->bif >= cnx->wsize)
		cnx->stats.rwnd_full++;
}

/*
========================================================================
Routine Description:
    The utility function to update BIF of a connection. Besides, the local
    max. of average BIF will be also calculated.

Arguments:
    UCHAR         *ptr       The pointer to buffer of TCP options
    UINT32        length     The length of TCP options
    rack_packet   *pkt       The parsed results will be set to pkt

Return Value:
    VOID

Note:
    1. This API is copied/modified from tcp_parse_options() of tcp_input.c
    2. Only the following options will be updated to pkt so far
	a. MSS (Max Segment Size)
	b. WS (Window Scale)
	c. SACK
	We only set a flag in pkt if SACK is appended.
========================================================================
*/
static VOID parse_tcp_options(UCHAR *ptr, UINT32 length, rack_packet *pkt)
{
	while (length > 0) {
		int opcode = *ptr++;
		int opsize;

		switch (opcode) {
		case TCPOPT_EOL:
			return;

		case TCPOPT_NOP:    /* Ref: RFC 793 section 3.1 */
			length--;
			continue;

		default:
			opsize = *ptr++;

			if (opsize < 2) /* "silly options" */
				return;

			if (opsize > length)
				return; /* don't parse partial options */

			switch (opcode) {
			case TCPOPT_MSS:
				if (opsize == TCPOLEN_MSS)
					pkt->mss = OS_NTOHS(*((UINT16 *)(ptr)));

				break;

			case TCPOPT_WINDOW:
				if (opsize == TCPOLEN_WINDOW) {
					UINT8 snd_wscale = *ptr;

					if (snd_wscale > 14)
						snd_wscale = 14;

					pkt->wscale = snd_wscale;
				}

				break;

			/*
			case TCPOPT_TIMESTAMP:
			    if ((opsize == TCPOLEN_TIMESTAMP) &&
				((estab && opt_rx->tstamp_ok) ||
				 (!estab && sysctl_tcp_timestamps))) {
				opt_rx->saw_tstamp = 1;
				opt_rx->rcv_tsval = get_unaligned_be32(ptr);
				opt_rx->rcv_tsecr = get_unaligned_be32(ptr + 4);
			    }
			    break;
			case TCPOPT_SACK_PERM:
			    if (opsize == TCPOLEN_SACK_PERM && th->syn &&
				!estab && sysctl_tcp_sack) {
				opt_rx->sack_ok = 1;
				tcp_sack_reset(opt_rx);
			    }
			    break;
			*/
			case TCPOPT_SACK:
				if ((opsize >= (TCPOLEN_SACK_BASE + TCPOLEN_SACK_PERBLOCK)) &&
					!((opsize - TCPOLEN_SACK_BASE) % TCPOLEN_SACK_PERBLOCK))
					pkt->opt_have_sack = 1;

				break;
			}

			ptr += opsize - 2;
			length -= opsize;
		}
	}

	return;
}

/*
========================================================================
Routine Description:
    The utility function to parse incoming TCP DATA/ACK packet and then
    set corresponding values into rack_packet structure

Arguments:
    PNDIS_PACKET  pPacket    The incoming packet used in WiFi driver
    rack_packet   *pkt       The parsed results will be set to pkt

Return Value:
    TRUE - Incoming pPacket is a TCP DATA or ACK and corresponding
	   values are set to pkt
    FALSE - others

Note:

========================================================================
*/
static BOOLEAN parse_tcp_packet(PNDIS_PACKET pPacket, rack_packet *pkt)
{
	PUCHAR pSrcBuf;
	BOOLEAN bRet = FALSE;
	UINT32 pktLen, tcpdata_len;
	USHORT TypeLen = 0;
	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
	pktLen = GET_OS_PKT_LEN(pPacket);
	tcpdata_len = pktLen;
	tcpdata_len -= 14;/* minus Ethernet header */
	/* get Ethernet protocol field*/
	TypeLen = OS_NTOHS(*((UINT16 *)(pSrcBuf + 12)));

	/* bypass VALN 802.1Q field */
	if (TypeLen == 0x8100) {
		pSrcBuf += 4;
		tcpdata_len -= 4; /* minus VLAN header */
	} else if ((TypeLen == 0x9100) || (TypeLen == 0x9200) || (TypeLen == 0x9300)) {
		pSrcBuf += 8;
		tcpdata_len -= 8; /* minus VLAN header */
	}

	if ((TypeLen == 0x0800) /* Type: IP (0x0800) */
		&& (pSrcBuf[23] == 0x06)) { /* Protocol: TCP (0x06) */
		UINT16 headerLen = (*((UINT8 *)(pSrcBuf + 46))) / 4;
		UINT16 opt_len = headerLen - 20;
		tcpdata_len -= 20; /* minus IP header */
		tcpdata_len -= headerLen; /* minus TCP header */
		pkt->sip = OS_NTOHL(*((UINT32 *)(pSrcBuf + 26)));
		pkt->dip = OS_NTOHL(*((UINT32 *)(pSrcBuf + 30)));
		pkt->sport = OS_NTOHS(*((UINT16 *)(pSrcBuf + 34)));
		pkt->dport = OS_NTOHS(*((UINT16 *)(pSrcBuf + 36)));
		pkt->seq = OS_NTOHL(*((UINT32 *)(pSrcBuf + 38)));
		pkt->ack = OS_NTOHL(*((UINT32 *)(pSrcBuf + 42)));
		pkt->flags = (*((UINT8 *)(pSrcBuf + 47)));
		pkt->wsize = OS_NTOHS(*((USHORT *)(pSrcBuf + 48)));
		pkt->opt_len = opt_len;
		pkt->opt_have_sack = 0;

		if (opt_len > 0 /*&& (pkt->flags & TCPHDR_SYN)*/)
			parse_tcp_options((UCHAR *)(pSrcBuf + 54), opt_len, pkt);

		pkt->mss = 1460;/* / Use 1460 here to avoid tracking TCP handshake process. */
		pkt->data_len = tcpdata_len;
		RTMP_GetCurrentSystemTick(&(pkt->timestamp));

		if ((pkt->flags & TCPHDR_ACK)) {
			if (pkt->data_len > 6)
				pkt->type = TCP_DATA;
			else
				pkt->type = TCP_ACK;

			bRet = TRUE;
		} else if (pkt->data_len > 6) {
			pkt->type = TCP_DATA;
			bRet = TRUE;
		}
	}

	return bRet;
}

/*
========================================================================
Routine Description:
    The utility function to add a connection entry for incoming_pkt

Arguments:
    RTMP_ADAPTER     *pAd       Pointer refer to the device handle.
    rack_packet  *incmoing_pkt  The parsed results will be set to pkt

Return Value:
    The new added connection entry

Note:

========================================================================
*/
static rack_cnx *add_cnx(RTMP_ADAPTER *pAd, rack_packet *incoming_pkt)
{
	rack_cnx *cnx = kmalloc(sizeof(rack_cnx), GFP_KERNEL);
	UINT32 hashIndex = hash_cnx(incoming_pkt);

	if (cnx == NULL)
		return NULL;

	memset(cnx, 0, sizeof(rack_cnx));
	/* search Window Scale optoin if any */
	cnx->wscale = incoming_pkt->wscale;
	/* setup CNX */
	cnx->tuple.sip = incoming_pkt->sip;
	cnx->tuple.dip = incoming_pkt->dip;
	cnx->tuple.sport = incoming_pkt->sport;
	cnx->tuple.dport = incoming_pkt->dport;
	cnx->ack = incoming_pkt->ack;
	cnx->ref_ack = cnx->ack - 1;

	if (cnx->ack > cnx->ref_ack)
		cnx->rel_ack = cnx->ack - cnx->ref_ack;
	else
		cnx->rel_ack = REDUCE_ACK_UINT32_MAX - cnx->ack + cnx->ref_ack;

	cnx->ss_countdown = REDUCE_ACK_IGNORE_CNT;
	cnx->state = STATE_INIT;
	cnx->mss = incoming_pkt->mss;
	/* add CNX into hlist and connection list */
	hlist_add_head(&(cnx->hnode), &(pAd->ackCnxHashTbl[hashIndex]));
	list_add(&(cnx->list), &(pAd->ackCnxList));
	pAd->ReduceAckConnections++;
	return cnx;
}

/*
========================================================================
Routine Description:
    The utility function to delete a connection entry

Arguments:
    RTMP_ADAPTER     *pAd       Pointer refer to the device handle.
    rack_cnx         *cnx       The CNX to be deleted

Return Value:
    VOID

Note:

========================================================================
*/
static VOID delete_cnx(RTMP_ADAPTER *pAd, rack_cnx *cnx)
{
	hlist_del(&(cnx->hnode));
	list_del(&(cnx->list));

	if (cnx) {
#if REDUCE_ACK_PKT_CACHE

		if (cnx->cache.raw_pkt != NULL) {
			RELEASE_NDIS_PACKET(pAd, cnx->cache.raw_pkt, NDIS_STATUS_FAILURE);
			cnx->cache.raw_pkt = NULL;
			memset(&(cnx->cache.rack), 0, sizeof(cnx->cache.rack));
		}

#endif
		kfree(cnx);
	}

	if (pAd->ReduceAckConnections > 0)
		pAd->ReduceAckConnections--;
}

/*
========================================================================
Routine Description:
    The utility function to update ack ratio for a connection

Arguments:
    rack_cnx         *cnx       The CNX to be updated

Return Value:
    VOID

Note:

========================================================================
*/
static VOID update_ack_ratio(rack_cnx *cnx)
{
	UINT32 step = (cnx->stats.total) / 2;
	UINT32 i = 1;

	while (step != 0 && step < cnx->stats.total_data) {
		step += (cnx->stats.total) / 2;
		i++;
	}

	if (i % 2 != 0)
		cnx->ack_ratio = (i - 1) / 2;
	else
		cnx->ack_ratio = (i) / 2;
}

/*
========================================================================
Routine Description:
    The utility function to update a connection

Arguments:
    rack_cnx         *cnx       The CNX to be updated
    rack_packet  *incoming_pkt  The incoming packet to be updated into CNX

Return Value:
    VOID

Note:
    The state transition is performed inside this function
========================================================================
*/
static VOID update_cnx(rack_cnx *cnx, rack_packet *incoming_pkt)
{
	if (incoming_pkt->type == TCP_ACK) {
		BOOLEAN updateCNX = TRUE;

		if (incoming_pkt->ack == cnx->ack) {
			if (incoming_pkt->wsize != cnx->wsize)
				cnx->wsize_changed = 1;
			else {
				cnx->stats.dupack++;
				cnx->state = STATE_CONGESTION;
				/* reset ss_countdown for SS or CA */
				cnx->ss_countdown = REDUCE_ACK_IGNORE_CNT;
			}
		} else if (before(incoming_pkt->ack, cnx->ack)) {
			cnx->stats.oldack++;
			updateCNX = FALSE;
		} else {
			cnx->ack = incoming_pkt->ack;
			cnx->wsize_changed = 0;
			cnx->rel_ack = cnx->ack - cnx->ref_ack;

			if (cnx->ss_countdown > 0)
				cnx->ss_countdown--;
		}

		if (updateCNX) {
			/* receiver may send duplicate ACK with window update */
			cnx->wsize = incoming_pkt->wsize;

			if (cnx->wscale > 0)
				cnx->wsize <<= cnx->wscale;

			/* Update connection state here */
			if (cnx->state != STATE_REDUCTION && cnx->ss_countdown <= 0) {
				cnx->state = STATE_REDUCTION;
				update_ack_ratio(cnx);
			}

			/* We should reset consecutive count after checking reduction. */
		}
	} else {
		if (cnx->seq == 0)
			cnx->ref_seq = incoming_pkt->seq - 1;

		/* update this connection */
		if (cnx->seq != 0 && (before(incoming_pkt->seq, cnx->seq) || incoming_pkt->seq == cnx->seq)) {
			cnx->stats.retrans++;
			cnx->state = STATE_CONGESTION;
			/* reset ss_countdown because of congestion */
			cnx->ss_countdown = REDUCE_ACK_IGNORE_CNT;
		} else if (cnx->seq != 0 && (incoming_pkt->seq != cnx->next_seq)) {
			cnx->stats.data_jump++;
		} else
			cnx->seq = incoming_pkt->seq;

		if (incoming_pkt->data_len > 0)
			cnx->next_seq = incoming_pkt->seq + incoming_pkt->data_len;
		else
			cnx->next_seq = incoming_pkt->seq;

		if (incoming_pkt->flags & TCPHDR_PSH)
			cnx->pushack = cnx->next_seq;

		cnx->rel_seq = cnx->seq - cnx->ref_seq;
		/* reset consecutive ack count */
		cnx->consecutive_data++;
		cnx->consecutive_ack = 0;

		if (cnx->consecutive_data > cnx->stats.max_consecutive_data)
			cnx->stats.max_consecutive_data = cnx->consecutive_data;
	}
}

/*
========================================================================
Routine Description:
    The external function used in WiFi driver to drop TCP ACK by RACK

Arguments:
    RTMP_ADAPTER    *pAd       Pointer refer to the device handle.
    PNDIS_PACKET    pPacket    The incoming packet from WiFi driver

Return Value:
    TRUE - the RACK engine decided to drop this TCP ACK
    FALSE - the RACK engine bypass this TCP ACK

Note:
    This routine also update statistics of TCP ACKs used in RACK engine
========================================================================
*/
BOOLEAN ReduceTcpAck(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket)
{
	BOOLEAN dropped = FALSE, bReduceCandidate = FALSE;

	if (pAd->CommonCfg.ReduceAckEnable) {
		rack_packet incomingPkt;
		rack_cnx *cnx = NULL, *new_cnx = NULL;
		BOOLEAN bResult = parse_tcp_packet(pPacket, &incomingPkt);

		if (bResult && match_black_list(&incomingPkt) == FALSE) {
			if (incomingPkt.type == TCP_ACK) {
				PKT_DPRINT(pAd, &incomingPkt, "incoming ack");
				/* incoming an ACK */
				MY_LOCK(pAd);
				cnx = find_cnx(pAd, &incomingPkt);

				if (cnx != NULL) {
					if (incomingPkt.flags & TCPHDR_FIN) {/* FIN is set */
						/* the "ack reducing" connection is going to close. */
						cnx->fin_tstamp = incomingPkt.timestamp;
						cnx->state = STATE_TERMINATION;
						/* clear previous cached ACK */
#if REDUCE_ACK_PKT_CACHE

						if (cnx->cache.raw_pkt != NULL) {
							RELEASE_NDIS_PACKET(pAd, cnx->cache.raw_pkt, NDIS_STATUS_FAILURE);
							cnx->cache.raw_pkt = NULL;
							memset(&(cnx->cache.rack), 0, sizeof(cnx->cache.rack));
						}

#endif /* REDUCE_ACK_PKT_CACHE */
						/* calculate ACK ratio */
						update_ack_ratio(cnx);
					} else {
						update_cnx(cnx, &incomingPkt);
						CNX_DPRINT(pAd, cnx, "ack update");

						/* Declare this incoming ACK as a candidate. */
						/* Note that ACK with options will not be a candidate */
						if ((incomingPkt.flags & TCPHDR_ACK) == TCPHDR_ACK && incomingPkt.data_len <= 6) {
							if (incomingPkt.opt_have_sack == 0)
								bReduceCandidate = TRUE;
							else
								cnx->stats.pass_sack++;
						} else
							PKT_DPRINT(pAd, &incomingPkt, "not a candidate");
					}
				} else {
					if (pAd->ReduceAckConnections < MAX_REDUCE_ACK_CNX_ENTRY) {
						new_cnx = add_cnx(pAd, &incomingPkt);
						CNX_DPRINT(pAd, cnx, "ack initial");
					} else
						PKT_DPRINT(pAd, &incomingPkt, "exceed max connections");
				}

				if (cnx != NULL) {
					/* Calculate if we need to reduce this incoming ACK */
					if (bReduceCandidate && decide_to_drop(pAd, &incomingPkt, cnx)) {
						CNX_DPRINT(pAd, cnx, "ack dropped");
						dropped = TRUE;
						cnx->stats.dropped++;
						cnx->consecutive_drop++;
						/* reset consecutive data count after checking ACK reduction */
						cnx->consecutive_data = 0;
						cnx->consecutive_ack++;
#if REDUCE_ACK_PKT_CACHE

						/* clear previous cached ACK */
						if (cnx->cache.raw_pkt != NULL) {
							RELEASE_NDIS_PACKET(pAd, cnx->cache.raw_pkt, NDIS_STATUS_FAILURE);
							cnx->cache.raw_pkt = NULL;
							memset(&(cnx->cache.rack), 0, sizeof(cnx->cache.rack));
						}

						/* cached incoming ACK */
						cnx->cache.raw_pkt = pPacket;
						memcpy(&(cnx->cache.rack), &incomingPkt, sizeof(cnx->cache.rack));
#endif /* REDUCE_ACK_PKT_CACHE */

						if (pAd->CommonCfg.ReduceAckEnable != REDUCE_ACK_ENABLE_NO_DROP_MODE)
							RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
					} else {
						/* Update the ack# we ever sent to air such that we could calculate correct BIF. */
						/* Note that ack of CnxInfo is updated in the update_cnx() invoked above. */
						cnx->sent_ack = cnx->ack;
						cnx->consecutive_drop = 0;
						/* reset consecutive data count after checking ACK reduction */
						cnx->consecutive_data = 0;
						cnx->consecutive_ack++;
						/* calculate BIF./ */
						calculate_bif(cnx);
						CNX_DPRINT(pAd, cnx, "ack update(bif)");
					}

					/* update timestamp after checking ACK reduction */
					cnx->stats.total++;
					cnx->last_tstamp = incomingPkt.timestamp;

					if (cnx->consecutive_ack > cnx->stats.max_consecutive_ack)
						cnx->stats.max_consecutive_ack = cnx->consecutive_ack;
				} else if (new_cnx != NULL) {
					new_cnx->stats.total++;
					new_cnx->last_tstamp = incomingPkt.timestamp;
					new_cnx->consecutive_data = 0;
					new_cnx->consecutive_ack++;
					new_cnx->sent_ack = new_cnx->ack;
					new_cnx->consecutive_drop = 0;
					calculate_bif(new_cnx);
					CNX_DPRINT(pAd, new_cnx, "new cnx");
				}

				MY_UNLOCK(pAd);
			}
		}
	}

	if (pAd->CommonCfg.ReduceAckEnable == REDUCE_ACK_ENABLE_NO_DROP_MODE)
		dropped = FALSE;

	return dropped;
}

/*
========================================================================
Routine Description:
    The external function to update statistics of TCP DATA used on RACK engine

Arguments:
    RTMP_ADAPTER    *pAd       Pointer refer to the device handle.
    PNDIS_PACKET    pPacket    The incoming packet from WiFi driver

Return Value:
    TRUE - no error
    FALSE - others

Note:

========================================================================
*/
BOOLEAN ReduceAckUpdateDataCnx(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket)
{
	if (pAd->CommonCfg.ReduceAckEnable) {
		rack_packet incomingPkt;
		rack_cnx *cnx = NULL;
		BOOLEAN bResult = parse_tcp_packet(pPacket, &incomingPkt);

		if (bResult && match_black_list(&incomingPkt) == FALSE) {
			if (incomingPkt.type == TCP_DATA) {
				/* incoming an DATA */
				MY_LOCK(pAd);
				cnx = find_cnx(pAd, &incomingPkt);

				if (cnx != NULL) {
					PKT_DPRINT(pAd, &incomingPkt, "incoming data");
					update_cnx(cnx, &incomingPkt);
					calculate_bif(cnx);
					cnx->stats.total_data++;
					CNX_DPRINT(pAd, cnx, "data-seq update");
				}

				MY_UNLOCK(pAd);
			}
		}
	}

	return TRUE;
}

/*
========================================================================
Routine Description:
    The function to delete/flush existing CNX if no activities for a long time

Arguments:
    struct work_struct    *work       Pointer refer to the work.

Return Value:
    VOID

Note:

========================================================================
*/
static VOID cnx_flush_task(struct work_struct *work)
{
	RTMP_ADAPTER *pAd = container_of(work, RTMP_ADAPTER, cnxFlushWork.work);
	COMMON_CONFIG *pComCfg = &pAd->CommonCfg;
	ULONG curTimestamp;
	RTMP_GetCurrentSystemTick(&curTimestamp);

	if (pComCfg->ReduceAckEnable && pAd->ReduceAckConnections > 0) {
		rack_cnx *cnx = NULL, *cnxTmp = NULL;
		MY_LOCK(pAd);
		list_for_each_entry_safe(cnx, cnxTmp, &(pAd->ackCnxList), list) {
			if (RTMP_TIME_AFTER(curTimestamp, (cnx->last_tstamp + pAd->CommonCfg.ReduceAckCnxTimeout))) {
				/* The CNX has no activity for pAd->CommonCfg.ReduceAckCnxTimeout milli-seconds */
				delete_cnx(pAd, cnx);
				CNX_DPRINT(pAd, cnx, "free CnxInfo by cnx_flush_task()");
			} else if (cnx->state == STATE_TERMINATION &&
					   RTMP_TIME_AFTER(curTimestamp, (cnx->fin_tstamp + REDUCE_ACK_FIN_CNX_TIMEOUT))) {
				if (pAd->CommonCfg.ReduceAckEnable != REDUCE_ACK_ENABLE_WITOUT_DEL_CNX) {
					delete_cnx(pAd, cnx);
					CNX_DPRINT(pAd, cnx, "Delete CnxInfo by FIN");
				}
			}
		}
		MY_UNLOCK(pAd);
	}

	schedule_delayed_work(&(pAd->cnxFlushWork), REDUCE_ACK_CNX_POLLING_INTERVAL);
}

/*
========================================================================
Routine Description:
    The function to flush cached ACK if no incoming ACK for a while

Arguments:
    struct work_struct    *work       Pointer refer to the work.

Return Value:
    VOID

Note:

========================================================================
*/
static VOID ack_flush_task(struct work_struct *work)
{
	RTMP_ADAPTER *pAd = container_of(work, RTMP_ADAPTER, ackFlushWork.work);
	COMMON_CONFIG *pComCfg = &pAd->CommonCfg;
	ULONG curTimestamp;
	RTMP_GetCurrentSystemTick(&curTimestamp);

	if (pComCfg->ReduceAckEnable && pAd->ReduceAckConnections > 0) {
		rack_cnx *cnx = NULL;
		MY_LOCK(pAd);
		list_for_each_entry(cnx, &(pAd->ackCnxList), list) {
			if (cnx->state != STATE_TERMINATION &&
				RTMP_TIME_AFTER(curTimestamp, (cnx->last_tstamp + pAd->CommonCfg.ReduceAckTimeout))) {
#if REDUCE_ACK_PKT_CACHE

				if (cnx != NULL && cnx->cache.raw_pkt != NULL) {
					ap_data_pkt_enq(pAd, cnx->cache.raw_pkt);
					cnx->cache.raw_pkt = NULL;

					if (cnx->stats.timeout < 10)
						cnx->stats.timeout_acks[cnx->stats.timeout++] = cnx->cache.rack.ack;

					memset(&(cnx->cache.rack), 0, sizeof(cnx->cache.rack));
				}

#endif /* REDUCE_ACK_PKT_CACHE */
			}
		}
		MY_UNLOCK(pAd);
	}

	schedule_delayed_work(&(pAd->ackFlushWork), REDUCE_ACK_POLLING_INTERVAL);
}

/*
========================================================================
Routine Description:
    The function to display statistics of all connections

Arguments:
    RTMP_ADAPTER    *pAd       Pointer refer to the device handle.

Return Value:
    VOID

Note:

========================================================================
*/
static VOID rack_show_ex(PRTMP_ADAPTER   pAdapter)
{
	UINT32 j = 0;
#if REDUCE_ACK_PKT_CACHE
	k = 0;
#endif /* REDUCE_ACK_PKT_CACHE */
	{
		UINT32 total_data_received = 0, total_ack_dropped = 0, total_ack_received = 0;
		UINT32 total_ack_timeout = 0;
		rack_cnx *cnx = NULL;
		j = 0;
		list_for_each_entry(cnx, &(pAdapter->ackCnxList), list) {
			total_data_received += cnx->stats.total_data;
			total_ack_dropped += cnx->stats.dropped;
			total_ack_received += cnx->stats.total;
			total_ack_timeout += cnx->stats.timeout;
			printk("(conn#%02d) %u.%u.%u.%u:%u --> %u.%u.%u.%u:%u (%u)\n",
				   j, (cnx->tuple.sip >> 24) & 0xFF,
				   (cnx->tuple.sip >> 16) & 0xFF,
				   (cnx->tuple.sip >> 8) & 0xFF,
				   (cnx->tuple.sip >> 0) & 0xFF,
				   cnx->tuple.sport,
				   (cnx->tuple.dip >> 24) & 0xFF,
				   (cnx->tuple.dip >> 16) & 0xFF,
				   (cnx->tuple.dip >> 8) & 0xFF,
				   (cnx->tuple.dip >> 0) & 0xFF,
				   cnx->tuple.dport,
				   cnx->state);
			printk("    dropped/total/sent ack count = %u/%u/%u\n", cnx->stats.dropped, cnx->stats.total, cnx->stats.total - cnx->stats.dropped);
			printk("    total data count = %u\n", cnx->stats.total_data);
			printk("    window size = %u\n", cnx->wsize);
			printk("    BIF = %u (max: %u, avg: %u)\n", cnx->bif, cnx->max_bif, cnx->avg_bif);
			printk("    MSS = %u\n", cnx->mss);
			printk("    last received ack timestamp = %lu\n", cnx->last_tstamp);
			printk("    last received FIN timestamp = %lu\n", cnx->fin_tstamp);
			printk("    ACK ratio = ~%u:1\n", cnx->ack_ratio);
			printk("    retransmission count = %u\n", cnx->stats.retrans);
			printk("    duplicate ack count = %u\n", cnx->stats.dupack);
			printk("    ack flush timeout count = %u\n", cnx->stats.timeout);
#if REDUCE_ACK_PKT_CACHE

			for (k = 0; k < 10 && cnx->stats.timeout_acks[k] != 0; k++)
				printk("    timeout_acks[%u] = %u\n", k, cnx->stats.timeout_acks[k]);

#endif /* REDUCE_ACK_PKT_CACHE */
			printk("    RWND full count = %u\n", cnx->stats.rwnd_full);
			printk("    max consecutive data count = %u\n", cnx->stats.max_consecutive_data);
			printk("    max consecutive ack count = %u\n", cnx->stats.max_consecutive_ack);
			printk("    old ack count = %u\n", cnx->stats.oldack);
			printk("    data jump count = %u\n", cnx->stats.data_jump);
			printk("    passed by state (not in reduction state) = %u\n", cnx->stats.pass_state);
			printk("    passed by consecutive data = %u\n", cnx->stats.pass_con_data);
			printk("    passed by consecutive dropped ack = %u\n", cnx->stats.pass_con_drop);
			printk("    passed by pushed data = %u\n", cnx->stats.pass_push_data);
			printk("    passed by pushed data3 = %u\n", cnx->stats.pass_push_data3);
			printk("    passed by selective ack = %u\n", cnx->stats.pass_sack);
			printk("    passed by BIF warning = %u\n", cnx->stats.pass_bif_warning);
			printk("    passed by RWND warning = %u\n", cnx->stats.pass_win_warning);
			printk("    passed by wsize changed = %u\n", cnx->stats.pass_wsize_change);
			printk("    passed by ack ratio = %u\n", cnx->stats.pass_ack_ratio);
			j++;
		}
		printk("Total DATA received = %u\n", total_data_received);
		printk("Total ACK dropped = %u\n", total_ack_dropped);
		printk("Total ACK received = %u\n", total_ack_received);
		printk("Total ACK sent = %u\n", total_ack_received - total_ack_dropped);
		printk("Total timeout count = %u\n", total_ack_timeout);
	}
	return;
}

/*
========================================================================
Routine Description:
    The function to init RACK engine

Arguments:
    RTMP_ADAPTER    *pAd       Pointer refer to the device handle.

Return Value:
    VOID

Note:

========================================================================
*/
VOID ReduceAckInit(RTMP_ADAPTER *pAd)
{
	int i;

	for (i = 0; i < REDUCE_ACK_MAX_HASH_BUCKETS; i++)
		INIT_HLIST_HEAD(&(pAd->ackCnxHashTbl[i]));

	INIT_LIST_HEAD(&(pAd->ackCnxList));
	/* allocate a lock resource for SMP environment */
	NdisAllocateSpinLock(pAd, &pAd->ReduceAckLock);
	pAd->ReduceAckConnections = 0;
	pAd->CommonCfg.ReduceAckEnable = 0;
	pAd->CommonCfg.ReduceAckProbability = DEFAULT_REDUCE_PERCENT;
	pAd->CommonCfg.ReduceAckTimeout = REDUCE_ACK_TIMEOUT;
	pAd->CommonCfg.ReduceAckCnxTimeout = REDUCE_ACK_CNX_TIMEOUT;
	/* init works for CNX refresh and ACK flush */
	INIT_DELAYED_WORK(&(pAd->cnxFlushWork), cnx_flush_task);
	INIT_DELAYED_WORK(&(pAd->ackFlushWork), ack_flush_task);
	schedule_delayed_work(&(pAd->cnxFlushWork), REDUCE_ACK_CNX_POLLING_INTERVAL);
	schedule_delayed_work(&(pAd->ackFlushWork), REDUCE_ACK_POLLING_INTERVAL);
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s, ReduceAckInit, inf=%s\n", __func__, pAd->net_dev->name));
	printk("INIT REDUCE TCP ACK, %s\n", pAd->net_dev->name);
}

/*
========================================================================
Routine Description:
    The function to de-init RACK engine

Arguments:
    RTMP_ADAPTER    *pAd       Pointer refer to the device handle.

Return Value:
    VOID

Note:

========================================================================
*/
VOID ReduceAckExit(RTMP_ADAPTER *pAd)
{
	rack_cnx *cnx = NULL, *cnxTmp = NULL;
	/* stop CNX and ACK flush works */
	cancel_delayed_work_sync(&(pAd->cnxFlushWork));
	cancel_delayed_work_sync(&(pAd->ackFlushWork));
	/* free CnxInfos */
	list_for_each_entry_safe(cnx, cnxTmp, &(pAd->ackCnxList), list) {
		delete_cnx(pAd, cnx);
		CNX_DPRINT(pAd, cnx, "free CnxInfo by ReduceAckExit()");
	}
	/* free the lock resource for SMP environment */
	NdisFreeSpinLock(&pAd->ReduceAckLock);
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s, ReduceAckExit, inf=%s)\n", __func__, pAd->net_dev->name));
	printk("EXIT REDUCE TCP ACK, %s\n", pAd->net_dev->name);
}

/*
========================================================================
Routine Description:
    The function to enable/disable RACK engine

Arguments:
    PRTMP_ADAPTER   pAdapte    Pointer refer to the device handle.
    UINT32          enable     enable/disable

Return Value:
    VOID

Note:
    Used on "iwpriv raiX set ReduceAckEnable=1/0"
========================================================================
*/
VOID ReduceAckSetEnable(PRTMP_ADAPTER pAdapter, UINT32 enable)
{
	COMMON_CONFIG *pComCfg = &pAdapter->CommonCfg;
	BOOLEAN prvEnable = pComCfg->ReduceAckEnable;

	if (enable < REDUCE_ACK_DISABLE || enable > REDUCE_ACK_ENABLE_LAST) {
		printk("ERROR!! - Valid range is 0 ~ %d", REDUCE_ACK_ENABLE_LAST);
		return;
	}

	pComCfg->ReduceAckEnable = enable;

	if (prvEnable > REDUCE_ACK_DISABLE && pComCfg->ReduceAckEnable == REDUCE_ACK_DISABLE) {
		rack_cnx *cnx = NULL, *cnxTmp = NULL;
		MY_LOCK(pAdapter);
		list_for_each_entry_safe(cnx, cnxTmp, &(pAdapter->ackCnxList), list) {
			delete_cnx(pAdapter, cnx);
			CNX_DPRINT(pAdapter, cnx, "free CnxInfo by ReduceAckSetEnable()");
		}
		MY_UNLOCK(pAdapter);
	}

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s, ReduceAckEnable=%d\n",  __func__, pComCfg->ReduceAckEnable));
}

/*
========================================================================
Routine Description:
    The function to set drop probability of RACK engine

Arguments:
    PRTMP_ADAPTER   pAdapter      Pointer refer to the device handle.
    UINT32          percentage    0~100

Return Value:
    VOID

Note:
    1. Used on "iwpriv raiX set ReduceAckProb=100"
    2. You could set percentage to 0 such that all DATA/ACK are processed
       but not dropped.
========================================================================
*/
VOID ReduceAckSetProbability(PRTMP_ADAPTER pAdapter, UINT32 percentage)
{
	COMMON_CONFIG *pComCfg = &pAdapter->CommonCfg;
#ifdef REMOVE_REDUCE_PROB

	if (percentage != 0) {
		printk("You could only set 0%% to disable reduction\n");
		return;
	}

	pComCfg->ReduceAckProbability = 0;
#else

	if (percentage < 0 || percentage > 100) {
		printk("Out of range - please try 0~100");
		return;
	}

	pComCfg->ReduceAckProbability = REDUCE_ACK_ONE_PERCENT * percentage;
#endif
}

/*
========================================================================
Routine Description:
    The function to show statistics of RACK engine

Arguments:
    RTMP_ADAPTER    *pAd       Pointer refer to the device handle.

Return Value:
    VOID

Note:
    Used on "iwpriv raiX set ReduceAckShow=1"
========================================================================
*/
VOID ReduceAckShow(PRTMP_ADAPTER   pAdapter)
{
	COMMON_CONFIG *pComCfg = &pAdapter->CommonCfg;
	ULONG curTimestamp;
	RTMP_GetCurrentSystemTick(&curTimestamp);
	printk("Reduced TCP ACK Info (current: %d, max:%d entries)\n", pAdapter->ReduceAckConnections, MAX_REDUCE_ACK_CNX_ENTRY);
	printk("Current Timestamp = %lu\n", curTimestamp);
	printk("Enable = %d\n", pComCfg->ReduceAckEnable);
	printk("Probability = %u%% (%u)\n", REDUCE_ACK_PERCENTAGE(pComCfg->ReduceAckProbability), pComCfg->ReduceAckProbability);
	printk("ACK Timeout = %u (ms)\n", pComCfg->ReduceAckTimeout * 1000 / HZ);
	printk("CNX Timeout = %u (ms)\n", pComCfg->ReduceAckCnxTimeout * 1000 / HZ);
	/* printk("sizeof(CnxInfo) = %u bytes\n", (sizeof(rack_cnx))); */
	printk("sizeof(CnxInfo) = %u bytes\n", (UINT)sizeof(rack_cnx));
	printk("SS Ignore Pkts = %u packets\n", REDUCE_ACK_IGNORE_CNT);
	MY_LOCK(pAdapter);
	rack_show_ex(pAdapter);
	MY_UNLOCK(pAdapter);
}
