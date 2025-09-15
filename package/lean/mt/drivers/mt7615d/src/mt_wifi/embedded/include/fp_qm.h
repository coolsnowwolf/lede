/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#ifndef __FP_QM_H__

struct FastPathTxQueElement {
	DL_LIST List;
	CHAR QueIdx;
	CHAR que_type;
	PNDIS_PACKET pPacket;
};

VOID fp_tx_pkt_deq_func(struct _RTMP_ADAPTER *pAd);

#endif


