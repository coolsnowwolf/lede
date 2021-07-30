/*
*******************************************************************************
**                                                                           **
**         Copyright (c) 2012 Quantenna Communications Inc                   **
**                            All Rights Reserved                            **
**                                                                           **
**  Author      : Quantenna Communications, Inc.                             **
**  File        : qvsp_common.h                                              **
**  Description : Video Screen Protection                                    **
**                                                                           **
*******************************************************************************
*/

#ifndef _QVSP_COMMON_H_
#define _QVSP_COMMON_H_

/*
 * Default stream airtime cost in msec per sec to send or receive at 8 Mbps.
 * Constants are binary for efficiency and do not need to be accurate.  They only need to
 * scale so that stream cost roughly equates to used airtime, in order to estimate the
 * affect of disabling or re-enabling a stream.
 */
#define BYTES_PER_KIB			(1024)		/* Kibibytes */
#define BYTES_PER_MIB			(1024 * 1024)	/* Mebibytes */
#define QVSP_STRM_COST_UNIT_MIB		(8)		/* arbitrary (optimised) cost unit */
#define QVSP_STRM_COST_UNIT_BYTES	(QVSP_STRM_COST_UNIT_MIB * BYTES_PER_MIB)
#define QVSP_NODE_COST_DFLT		(1000)

struct qtn_per_tid_stats {
	uint32_t tx_throt_pkts;
	uint32_t tx_throt_bytes;
	uint32_t tx_sent_pkts;
	uint32_t tx_sent_bytes;
};

#endif
