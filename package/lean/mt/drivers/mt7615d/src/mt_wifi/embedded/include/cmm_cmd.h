/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	hw_ctrl.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __CMM_CMD_H__
#define __CMM_CMD_H__

#include "rtmp_type.h"
#include "security/wpa_cmm.h"


typedef struct _CmdQElmt {
	UINT command;
	PVOID buffer;
	ULONG bufferlength;
	BOOLEAN CmdFromNdis;
	BOOLEAN SetOperation;
#ifdef DBG_STARVATION
	struct starv_dbg starv;
#endif /*DBG_STARVATION*/
	struct _CmdQElmt *next;
} CmdQElmt, *PCmdQElmt;

typedef struct _CmdQ {
	UINT size;
	CmdQElmt *head;
	CmdQElmt *tail;
	UINT32 CmdQState;
#ifdef DBG_STARVATION
	struct starv_dbg_block block;
#endif /*DBG_STARVATION*/
} CmdQ, *PCmdQ;

#define EnqueueCmd(cmdq, cmdqelmt)		\
	{										\
		if (cmdq->size == 0)				\
			cmdq->head = cmdqelmt;			\
		else								\
			cmdq->tail->next = cmdqelmt;	\
		cmdq->tail = cmdqelmt;				\
		cmdqelmt->next = NULL;				\
		cmdq->size++;						\
	}

#define NDIS_OID	UINT


enum {
	CMDTHREAD_FIRST_CMD_ID = 0,
	/*STA related*/
	CMDTHREAD_SET_PSM_BIT = CMDTHREAD_FIRST_CMD_ID,
	CMDTHREAD_QKERIODIC_EXECUT,
	CMDTHREAD_SET_PORT_SECURED,
	/*AP related*/
	CMDTHREAD_CHAN_RESCAN,
	CMDTHREAD_802_11_COUNTER_MEASURE,
	CMDTHREAD_AP_RESTART,
	CMDTHREAD_APCLI_PBC_TIMEOUT,
	CMDTHREAD_APCLI_IF_DOWN,
	CMDTHREAD_WSC_APCLI_LINK_DOWN,
	/*CFG 802.11*/
	CMDTHREAD_REG_HINT,
	CMDTHREAD_REG_HINT_11D,
	CMDTHREAD_SCAN_END,
	CMDTHREAD_CONNECT_RESULT_INFORM,
	/*P2P*/
	CMDTHREAD_SET_P2P_LINK_DOWN,
	/*RT3593 related*/
	CMDTHREAD_UPDATE_TX_CHAIN_ADDRESS,
	/*TDLS related*/
	CMDTHREAD_TDLS_SEND_CH_SW_SETUP,
	CMDTHREAD_TDLS_AUTO_TEARDOWN,
	CMDTHREAD_TDLS_RECV_NOTIFY,
	/*Not usable*/
	CMDTHREAD_RESPONSE_EVENT_CALLBACK,
	/*BT Coexistence related*/
	RT_CMD_COEXISTENCE_DETECTION,
#ifdef WIFI_SPECTRUM_SUPPORT
	/* WIFI-SPECTRUM */
	CMDTHRED_WIFISPECTRUM_DUMP_RAW_DATA,
#endif /* WIFI_SPECTRUM_SUPPORT */
#ifdef INTERNAL_CAPTURE_SUPPORT
	/* INTERNAL CAPTURE */
	CMDTHRED_ICAP_DUMP_RAW_DATA,
#endif /* INTERNAL_CAPTURE_SUPPORT */

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
    CMDTHRED_PRECAL_TXLPF,
    CMDTHRED_PRECAL_TXIQ,
    CMDTHRED_PRECAL_TXDC,
    CMDTHRED_PRECAL_RXFI,
    CMDTHRED_PRECAL_RXFD,
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

	CMDTHRED_DOT11H_SWITCH_CHANNEL,
	CMDTHRED_MAC_TABLE_DEL,
#ifdef MT_DFS_SUPPORT
	CMDTHRED_DFS_CAC_TIMEOUT,
	CMDTHRED_DFS_AP_RESTART,
#endif
	CMDTHRED_STA_DEAUTH_ACT,
	CMDTHRED_RXV_WRITE_IN_FILE,
	CMDTHREAD_END_CMD_ID,
};



typedef struct _CMDHandler_TLV {
	USHORT Offset;
	USHORT Length;
	UCHAR DataFirst;
} CMDHandler_TLV, *PCMDHandler_TLV;



/*Tempral define before hwctrl ready*/
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)

/* ----------------- MLME Related MACRO ----------------- */
/* #define RTMP_MLME_PRE_SANITY_CHECK(pAd) */

#define RTMP_MLME_RESET_STATE_MACHINE(pAd, _wdev)	\
	MlmeRestartStateMachine(pAd, _wdev)

#define RTMP_HANDLE_COUNTER_MEASURE(_pAd, _pEntry)\
	HandleCounterMeasure(_pAd, _pEntry)

/* ----------------- Power Save Related MACRO ----------------- */
#define RTMP_PS_POLL_ENQUEUE(pAd, pStaCfg)	EnqueuePsPoll(pAd, pStaCfg)


#else



#define RTMP_MLME_RESET_STATE_MACHINE(pAd, _wdev)	\
	do {	\
		MlmeEnqueueWithWdev(pAd, MLME_CNTL_STATE_MACHINE, MT2_RESET_CONF, 0, NULL, 0, _wdev);	\
		RTMP_MLME_HANDLER(pAd);	\
	} while (0)

#define RTMP_HANDLE_COUNTER_MEASURE(_pAd, _pEntry)		\
	{	\
		RTEnqueueInternalCmd(_pAd, CMDTHREAD_802_11_COUNTER_MEASURE, _pEntry, sizeof(MAC_TABLE_ENTRY));	\
		RTMP_MLME_HANDLER(_pAd);									\
	}

#endif


/*HIF related*/


#endif
