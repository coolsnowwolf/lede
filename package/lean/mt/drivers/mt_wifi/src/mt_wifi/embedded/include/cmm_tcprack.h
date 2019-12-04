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

    Module Name: reduce_tcpack.h

    Abstract:
	The corresponding defines and data structure for RACK (Reduce TCP ACK)
    engine.

    Revision History:
    Who         When            What
    Hank Huang  2015/07/13      initial version
    --------    ----------      ----------------------------------------------
*/

#ifndef __REDUCE_TCPACK_H__
#define __REDUCE_TCPACK_H__

#define REDUCE_ACK_MAX_HASH_BUCKETS		2048
#define MAX_REDUCE_ACK_CNX_ENTRY			256					/* 256 connections at max */
#define MAX_CONSECUTIVE_DROP_CNT			5
#define MAX_CONSECUTIVE_DATA_CNT			10
#define MAX_SUPPORTED_ACK_RATIO             5                   /* max ack ratio we supported */
#define REDUCE_PROB_INC_TH					1
#define REDUCE_PROB_INC_STEP				20
#define REDUCE_ACK_CNX_POLLING_INTERVAL	(10*HZ)			/* 10 seconds */
#define REDUCE_ACK_POLLING_INTERVAL		(20*HZ/1000)		/* 20ms */
#define REDUCE_ACK_TIMEOUT					(250*HZ/1000)		/* 250ms */
#define REDUCE_ACK_CNX_TIMEOUT				(180*HZ)			/* 180 seconds */
#define REDUCE_ACK_FIN_CNX_TIMEOUT			(10*HZ)				/* 10 seconds */
#define REDUCE_ACK_ONE_PERCENT				(0x80000000/100)
#define DEFAULT_REDUCE_PERCENT				(REDUCE_ACK_ONE_PERCENT * 100)
#define REDUCE_ACK_IGNORE_CNT				(126)
#define REDUCE_ACK_UINT32_MAX				4294967295U
#define REDUCE_ACK_PERCENTAGE(prob)			({ \
		UINT32 i = 0, percentage = 0;\
		for (percentage = 0; percentage < 100; percentage++) {\
			if (i >= prob) {\
				break;\
			} \
			else {\
				i += REDUCE_ACK_ONE_PERCENT;\
			} } percentage; })
/* 0813 */
#define REDUCE_ACK_PKT_CACHE				0

enum {
	TCP_ACK = 0,
	TCP_DATA = 1,
};

enum {
	STATE_INIT = 0,
	STATE_SS = 1,
	STATE_REDUCTION = 2,
	STATE_CONGESTION = 3,
	STATE_TERMINATION = 4,
};

enum {
	REDUCE_ACK_DISABLE = 0,
	REDUCE_ACK_ENALBE = 1,
	REDUCE_ACK_ENABLE_WITOUT_DEL_CNX = 2,
	REDUCE_ACK_ENABLE_NO_DROP_MODE = 3,    /* for testing only, check algorithm CPU cost */
	REDUCE_ACK_ENABLE_LAST = 3,
};

typedef struct _rack_packet {
	UINT32 sip;
	UINT32 dip;
	UINT16 sport;
	UINT16 dport;
	UINT32 type;		/* TCP_ACK or TCP_DATA */
	UINT32 ack;
	UINT32 seq;
	UINT32 wsize;
	UINT16 opt_len;
	UINT32 data_len;
	UINT8 wscale;
	UINT8 flags;
	UINT16 mss;
	ULONG timestamp;	/* timestamp that incoming TCP packet is parsed */
	UINT32 opt_have_sack;
} rack_packet;

/* Each rack_cnx may be linked to two tables:
   1. Hash table by (sip, dip, sport, dport): pAd->ackCnxHashTbl
   2. Link list for all connections for flushing purpose: pApd->ackCnxList
 */
typedef struct _rack_cnx {
	/*
	 * Parameters which retrieved from TCP data/ack packets
	 */
	struct {
		UINT32 sip;
		UINT32 dip;
		UINT16 sport;
		UINT16 dport;
	} tuple;                /* 4-tuple */
	UINT32 ack;				/* acknowledgement # of last ACK packet */
	UINT32 seq;				/* sequence # of last DATA packet */
	UINT8  wscale;			/* window scale (WS) parsed from TCP option of SYN packet */
	UINT32 wsize;			/* window size inside last ACK packet (RWND) */
	UINT32 mss;				/* Max Segment Size parsed from TCP option of SYN packet */

	/*
	 * Parameters which derived from TCP header and connection tracking
	 */
	UINT32 ref_ack;			/* reference ack# used to calcualte relative ack# */
	UINT32 rel_ack;			/* relative ack# calculated from ref_ack */
	INT32 ss_countdown;		/* The number of ack packets we should not reduce for slow start */
	UINT32 sent_ack;		/* the last ack# we ever sent to air */
	UINT32 next_seq;		/* {seq# of DATA packet} + {data length of DATA packet}, used to calculate BIF. */
	UINT32 ref_seq;			/* reference seq# used to calculate relative seq# */
	UINT32 rel_seq;			/* relative seq# */
	UINT8 state;			/* Connection state, INIT/SS/REDUCTION/CONGESTION */
	ULONG last_tstamp;		/* The timestamp of last incoming ACK which is declared as a reduction candidate */
	ULONG fin_tstamp;		/* The timestamp of last received ACK with FIN flag. */

	/*
	 * Statistics which is used for reduction decision
	 */
	UINT32 consecutive_data;	/* consecutive TCP DATA count, reset while ACK received */
	UINT32 consecutive_ack;		/* consecutive TCP ACK count, reset while data received */
	UINT32 consecutive_drop;	/* consecutive TCP ACK which is dropped by us. */
	UINT32 bif;					/* Bytes In Flight, calculated based on our connection tracking info */
	UINT32 pushack;				/* the expected ack# of data segment with PSH flag */
	UINT32 wsize_changed;       /* the flag to indicate window size changed */
	UINT32 avg_bif;             /* the average of local max BIF */
	UINT32 max_bif;             /* max. BIF we ever calculated */
	UINT32 ack_ratio;           /* calculated ACK ratio before REDUCTION state */

	/*
	 * Statistics which is used for debugging
	 */
	struct {
		UINT32 dropped;					/* total ACK packets dropped by us */
		UINT32 total;					/* total ACK packets received by us */
		UINT32 total_data;				/* total DATA packets received by us */
		UINT32 timeout;					/* total ACK packets which are dropped but timeout */
		UINT32 retrans;					/* total DATA packets which are detected as retransmission */
		UINT32 dupack;					/* total duplicate ACKs detected by us */
		UINT32 rwnd_full;				/* total number of times which we detect receive window full */
		UINT32 max_consecutive_data;	/* the max number of consecutive DATA packets */
		UINT32 max_consecutive_ack;		/* the max number of consecutive ACK packets */
		UINT32 pass_state;				/* the number of ACK which is ignored reduction because state is not STATE_REDUCTION */
		UINT32 pass_con_drop;			/* the number of ACK which is ignored reduction because exceed MAX_CONSECUTIVE_DROP_CNT */
		UINT32 pass_con_data;			/* the number of ACK which is ignored reduction because exceed MAX_CONSECUTIVE_DATA_CNT */
		UINT32 pass_win_warning;		/* the number of ACK which is ignored reduction because of receive window warnning */
		UINT32 pass_sack;               /* the number of ACK which is ignored reduction because of SACK included */
		UINT32 pass_push_data;			/* the number of ACK which is ignored reduction because of DATA with PSH flag */
		UINT32 pass_push_data3;
		UINT32 pass_bif_warning;        /* the number of ACK which is ignored reduction because of approaching avg_bif */
		UINT32 pass_ack_ratio;          /* the number of ACK which is ignored reduction because of exceed MAX_ACK_RATIO */
		UINT32 pass_wsize_change;       /* the number of ACK which is ignored reduction because of window size changed */
		UINT32 drop_prob;				/* the number of ACK which is dropped because of consecutve ACKs */
		UINT32 data_jump;				/* the number of DATA which seq# is not equal to next_seq */
		UINT32 oldack;					/* the number of ACK which ack# is less than last ack# we ever received */
		UINT32 timeout_acks[10];        /* the ack# which is timeout and flushed by RACK engine */
	} stats;

	/*
	 * MISC
	 */
#if REDUCE_ACK_PKT_CACHE
	struct {
		PNDIS_PACKET raw_pkt;
		rack_packet rack;
	} cache;
#endif /* REDUCE_ACK_PKT_CACHE */

	struct hlist_node	hnode;
	struct list_head	list;
} rack_cnx;

/* External APIs */
BOOLEAN ReduceTcpAck(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket);
BOOLEAN ReduceAckUpdateDataCnx(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket);
VOID ReduceAckInit(RTMP_ADAPTER *pAd);
VOID ReduceAckExit(RTMP_ADAPTER *pAd);
VOID ReduceAckShow(RTMP_ADAPTER *pAd);
VOID ReduceAckSetEnable(RTMP_ADAPTER *pAd, UINT32 enable);
VOID ReduceAckSetProbability(RTMP_ADAPTER *pAd, UINT32 prob);

#endif /* __REDUCE_TCPACK_H__ */
