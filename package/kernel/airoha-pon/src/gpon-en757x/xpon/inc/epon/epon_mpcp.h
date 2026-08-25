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
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    epon_mpcp.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    JQ.Zhu  2012-06-06      Initial version
*/

#ifndef __EPON_MPCP
#define __EPON_MPCP

/***********************
MPCP discover
***************************/
/*  refer to 802.3ah 64.3.3.2 */
/* mpcpDiscState */
#define MPCP_STATE_WAIT				0
#define MPCP_STATE_REGISTERING			1
#define MPCP_STATE_REGISTER_REQUEST	2
#define MPCP_STATE_REGISTER_PENDING	3
#define MPCP_STATE_RETRY				4
#define MPCP_STATE_DENIED				5
#define MPCP_STATE_REGISTER_ACK		6
#define MPCP_STATE_NACK				7
#define MPCP_STATE_REGISTERED			8
#define MPCP_STATE_REMOTE_DEREGISTER			9
#define MPCP_STATE_LOCAL_DEREGISTER			10

/* epon mpcp state max number */
#define MAX_MPCP_STATE_NUM           11
#define MAX_MPCP_STATE_STR_LEN       20


#ifdef TCSUPPORT_CUC
#define MPCP_RGST_AGAIN_TIMEOUT (50)
#else
#define MPCP_RGST_AGAIN_TIMEOUT (60)
#endif

#define MPCP_SYNC_TIME_HANDLER(__MPCPSyncTime)		\
{								\
	REG_e_sync_time eSyncTime;		\
	eSyncTime.Raw = READ_REG_WORD(e_sync_time);	\
	if (eSyncTime.Bits.sync_time > MPCP_SYNC_TIME_MAX)		\
	{						\
		WRITE_REG_WORD(e_sync_time, MPCP_SYNC_TIME_MAX);		\
		WRITE_REG_WORD(e_trx_adjust_time1, 0x008efff1);			\
	}							\
	else if (eSyncTime.Bits.sync_time == 0)							\
	{							\
		WRITE_REG_WORD(e_sync_time, __MPCPSyncTime);				\
		eSyncTime.Raw = READ_REG_WORD(e_sync_time);			\
		WRITE_REG_WORD(e_trx_adjust_time1, (0x002ffff1 + ((__MPCPSyncTime) << 16)));	\
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter %s:sync_time == 0, set __MPCPSyncTime = 0x%x", __FUNCTION__, __MPCPSyncTime);	\
	}								\
	else									\
	{									\
		WRITE_REG_WORD(e_trx_adjust_time1, (0x002ffff1 + (eSyncTime.Bits.sync_time << 16)));		\
		if (__MPCPSyncTime != eSyncTime.Bits.sync_time)					\
		{									\
			__MPCPSyncTime = eSyncTime.Bits.sync_time;					\
		}									\
	}												\
}

typedef struct eponMpcpDiscFsm_s{
	__u8 mpcpDiscState;
	__u8 mpcpDiscMacr_RgstrAck;//ACK; NACK
	__u8 mpcpReqRetryFlag;
	
	//variables
	bool begin;
	__u8 *dataRx;
	__u8 *dataTx;
	__u32 grantEndTime;
	bool insideDiscoveryWindow;
	__u32 localTime;
	__u16 opcodeRx;
	__u16 opcodeTx;
	__u16 pendingGrants;
	bool registered;
	__u16 syncTime;
	bool timestampDrift;	
#ifdef TCSUPPORT_EPON_RGST_SILENT
	__u16 rgstAgainTimeout;
#endif
	//struct timer_list eponMpcpTimer;
	
}eponMpcpDiscFsm_t, *eponMpcpDiscFsm_p;

typedef struct eponMpcp_s{
	eponMpcpDiscFsm_t eponDiscFsm;
}eponMpcp_t, *eponMpcp_p;


int eponMpcpDscvFsmWaitHandler(__u8 llidIndex);
void eponTimeDrftIntHandler(__u32 data);
void eponMpcpTmOutIntHandler(__u32 data);
void eponTimeDrftIntHandler(__u32 data);
void eponMpcpDiscvGateIntHandler(unsigned long data);
int eponMpcpLocalDergstr(__u8 llidIndex);  
int eponMpcpSetDiscvRgstAck(__u8 llidIndex , __u8 rgstAckFlag);
int eponMpcpRgstReqIntHandler(void);
int eponMpcpRgstAckIntHandler(void);
int eponMpcpRgstIntHandler(__u8 llidIndex);
int eponMpcpGntOvrRunIntHandler(void);
int eponMpcpRptOvrIntvalIntHandler(void);
int max_dscv_gate_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data);
int max_dscv_gate_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data);
#endif//end of __EPON_MPCP
