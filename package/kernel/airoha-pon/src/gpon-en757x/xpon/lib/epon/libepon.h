/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	libepon.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	JQ.zhu		2012/7/20	Create
*/


#ifndef _EPON_LIB_H_
#define _EPON_LIB_H_
#include <sys/types.h>
#include <asm/ioctl.h>


#define EPON_LLID_MAX_NUM		8

#define EPON_MAC_IOC_MAGIC 'j'
#define EPON_MAC_MAJOR 221

#define EPON_IOCTL_SET_LLID_ENABLE_MASK     	_IOW(EPON_MAC_IOC_MAGIC, 0,  unsigned long)
#define EPON_IOCTL_LLID_DEREGISTER    			_IOW(EPON_MAC_IOC_MAGIC, 1,  unsigned long) 
#define EPON_IOCTL_LLID_DISCV_RGSTACK		_IOW(EPON_MAC_IOC_MAGIC, 2,  unsigned long) 
#define EPON_IOCTL_SET_LLID_KEY				_IOW(EPON_MAC_IOC_MAGIC, 3,  unsigned long) 
#define EPON_IOCTL_GET_LLID_KEY				_IOW(EPON_MAC_IOC_MAGIC, 4,  unsigned long) 
#define EPON_IOCTL_SET_LLID_FEC				_IOW(EPON_MAC_IOC_MAGIC, 5,  unsigned long) 
#define EPON_IOCTL_GET_LLID_FEC				_IOR(EPON_MAC_IOC_MAGIC, 6,  unsigned long) 
#define EPON_IOCTL_MAC_RST				_IOR(EPON_MAC_IOC_MAGIC, 7,  unsigned long)  //johnma
#define EPON_IOCTL_SET_LLID_DBA_THRSHLD_NUM		_IOW(EPON_MAC_IOC_MAGIC, 8,  unsigned long) 
#define EPON_IOCTL_GET_LLID_DBA_THRSHLD_NUM		_IOR(EPON_MAC_IOC_MAGIC, 9,  unsigned long) 
#define EPON_IOCTL_SET_LLID_DBA_THROD		_IOW(EPON_MAC_IOC_MAGIC, 10,  unsigned long) 
#define EPON_IOCTL_GET_LLID_DBA_THROD		_IOR(EPON_MAC_IOC_MAGIC, 11,  unsigned long) 
#define EPON_IOCTL_SET_MPCP_TIME		_IOW(EPON_MAC_IOC_MAGIC, 12,  unsigned long) 
#define EPON_IOCTL_GET_MPCP_TIME		_IOR(EPON_MAC_IOC_MAGIC, 13,  unsigned long) 
#define EPON_IOCTL_SET_HOLDOVER_CFG		_IOW(EPON_MAC_IOC_MAGIC, 14,  unsigned long) 
#define EPON_IOCTL_GET_HOLDOVER_CFG		_IOR(EPON_MAC_IOC_MAGIC, 15,  unsigned long) 
#define EPON_IOCTL_GET_TX_ETH_MIB		_IOR(EPON_MAC_IOC_MAGIC, 16,  unsigned long) 
#define EPON_IOCTL_GET_RX_ETH_MIB		_IOR(EPON_MAC_IOC_MAGIC, 17,  unsigned long) 
#define EPON_IOCTL_CLR_TX_RX_MIB		_IOW(EPON_MAC_IOC_MAGIC, 18,  unsigned long) 
#define EPON_IOCTL_POWER_CTL               _IOW(EPON_MAC_IOC_MAGIC, 19,  unsigned long)
#define EPON_IOCTL_CTL_PS 				_IOW(EPON_MAC_IOC_MAGIC, 20,  unsigned long)  //ltm
#define EPON_IOCTL_AUTH_FAIL_CTL        _IOW(EPON_MAC_IOC_MAGIC, 21,  unsigned long)


#define EPON_PS_CMD_SLP_CTL  0
#define EPON_PS_CMD_CFG_GET  1
#define EPON_PS_CMD_CFG_SET  2

typedef struct eponPsIoctl_s{
	u_char cmdType;
	u_int  startTime;  // timestamp(TQ)
	u_int  sleepDuration;  // the duration of the sleep period
	u_int  wakeDuration;
	u_char sleepFlag;
	u_char sleepMode;
	u_char earlyWakeup;
	u_char sleepDurationMax[6];
}eponPsIoctl_t, *eponPsIoctl_Ptr;
int eponApiCtlPs(eponPsIoctl_Ptr cscp);


#define EPON_MAC_DEV "/dev/epon_mac"

typedef struct eponTxCnt_s{
	u_int txFrameCnt;
	u_int txFrameLen;
	u_int txDropCnt;
	u_int txBroadcastCnt;
	u_int txMulticastCnt;
	u_int tx_less64_Cnt;
	u_int tx_more1518_Cnt;
	u_int tx_64_Cnt;
	u_int tx_65_to_127_Cnt;
	u_int tx_128_to_255_Cnt;
	u_int tx_256_to_511_Cnt;
	u_int tx_512_to_1023_Cnt;
	u_int tx_1024_to_1518_Cnt;
}eponTxCnt_t, *eponTxCnt_p;

typedef struct eponRxCnt_s{
	u_int rxFrameCnt;
	u_int rxFrameLen;
	u_int rxDropCnt;
	u_int rxBroadcastCnt;
	u_int rxMulticastCnt;
	u_int rxCrcCnt;
	u_int rxFragFameCnt;
	u_int rxJabberFameCnt;
	u_int rx_less64_Cnt;
	u_int rx_more1518_Cnt;
	u_int rx_64_Cnt;
	u_int rx_65_to_127_Cnt;
	u_int rx_128_to_255_Cnt;
	u_int rx_256_to_511_Cnt;
	u_int rx_512_to_1023_Cnt;
	u_int rx_1024_to_1518_Cnt;
}eponRxCnt_t, *eponRxCnt_p;

typedef struct {
	u_char llidIdx;
	u_char param0;
	u_short param1;
	u_int param2;
	u_char info[128];
} eponMacIoctl_t ;

/*
**********************************************************************************
epon mac ioctl data structure.
**********************************************************************************
*/
typedef struct eponMacMpcpIoctl_MACR_s{
	u_char destMac[6];
	u_short registerCode;
	u_char llidIdx;
	u_char status;
}eponMacMpcpIoctl_MACR_t;


typedef struct eponMpcpMib_s{
	u_short ioctlParamIndex;
	u_char llidIdx;
	u_int mpcpId;// index =1
	u_int mpcpAdminState;
	u_int mpcpMode;
	u_int mpcpLinkId;
	u_char aMPCPRemoteMACAddress[6];
	u_int aMPCPRegistrationState;
	u_int aMPCPMACCtrlFramesTransmitted;
	u_int aMPCPMACCtrlFramesReceived;
	u_int aMPCPTxGate;
	u_int aMPCPTxRegAck;
	u_int aMPCPTxRegister;
	u_int aMPCPTxRegRequest;
	u_int aMPCPTxReport;
	u_int aMPCPRxGate;
	u_int aMPCPRxRegAck;
	u_int aMPCPRxRegister;
	u_int aMPCPRxRegRequest;
	u_int aMPCPRxReport;
	u_int aMPCPTransmitElapsed;
	u_int aMPCPReceiveElapsed;
	u_int aMPCPRoundTripTime;
	u_int aMPCPDiscoveryWindowsSent;
	u_int aMPCPDiscoveryTimeout;
	u_int aMPCPMaximumPendingGrants;	
}eponMpcpMib_t;

int eponMacIoctl(int cmd, void *data, int datalen);
int eponApiSetOnuMpcpTime(u_short time );
int eponApiGetOnuMpcpTime(u_short *time );
int eponApiSetOnuHoldoverCfg(u_char enable,u_short time );
int eponApiGetOnuHoldoverCfg(u_char *enable, u_short *time );
int eponApiClearOnuTxRxCnt(u_char clearTx, u_char clearRx );
int eponApiGetOnuTxCnt(eponTxCnt_t *txCnt );
int eponApiGetOnuRxCnt(eponRxCnt_t *rxCnt );

int eponApiGetLlidkey(u_char llidIdx, u_char keyIndex, u_char *key);
int eponApiSetLlidkey(u_char llidIdx, u_char keyIndex, u_char *key);
int eponCmdSetLlidMask(u_int mask);
int eponApiGetLlidFec(u_char llidIdx);
int eponApiSetLlidFec(u_char llidIdx, u_char fecFlag);
int eponApiSetLlidDBAThrshldNum(u_char llidIdx, u_char num);
int eponApiGetLlidDBAThrshldNum(u_char llidIdx, u_char *num);
int eponApiSetLlidDBAThrshld(u_char llidIdx, u_char thrshldIndex, u_char mask,u_short *threshold );
int eponApiGetLlidDBAThrshld(u_char llidIdx, u_char thrshldIndex, u_char mask,u_short *threshold );
int eponApiSetPhyPower(unsigned int time);
int eponApiSetAuthFail(u_char isAuthFail);



#ifdef TCSUPPORT_EPON_OAM
#define EPONOAM_MQ_FLAG_PATH "/tmp/epon_oam/epon_oam_cmd_queue"
#define EPONOAM_PROJID 10
#define EPONOAM_CMD_INFO_LEN 128

#define EPONOAM_CMD_TYPE_DEBUGLEVEL			1
#define EPONOAM_CMD_TYPE_INIT					2
#define EPONOAM_CMD_TYPE_STOP				3
#define EPONOAM_CMD_TYPE_DESTORY			4
#define EPONOAM_CMD_TYPE_SHOW_STATUS		5
#define EPONOAM_CMD_TYPE_STOPPDUTM			6
#define EPONOAM_CMD_TYPE_SEND_NORMAL_PDU	7
#define EPONOAM_CMD_TYPE_SEND_CRITICAL_PDU	8
#define EPONOAM_CMD_TYPE_LOCALSATISFY		9

#define EPONOAM_CMD_TYPE_CTC_DBG_LVL 			10
#define EPONOAM_CMD_TYPE_CTC_SET_DSCVY_STATE 	11
#define EPONOAM_CMD_TYPE_OAM_ALARM   			12

#define EPONOAM_CMD_TYPE_LAN_DBG		15
#define EPONOAM_CMD_TYPE_TMSENDINFO		16
#define EPONOAM_CMD_TYPE_SEND_DYGASP	17
#define EPONOAM_CMD_TYPE_LOSTTMOUT         18
#define EPONOAM_CMD_TYPE_MAXPDUNUMPERSEC  19
#define EPONOAM_CMD_DRIVER_EVENT 20 // ltm new
#define EPONOAM_CMD_TYPE_RESTART 21
#define EPONOAM_CMD_TYPE_SAVECFG  22
#define EPONOAM_CMD_TYPE_LOADCFG   23
#define EPONOAM_CMD_TYPE_UPDATACFG  24
#define EPONOAM_CMD_TYPE_ONUTYPE  25


typedef struct eponOamCmdMsg_s{
	u_int cmdType;
	u_int cmdSeq;
	u_char cmdInfo[EPONOAM_CMD_INFO_LEN];
}eponOamCmdMsg_t;

typedef struct eponOamIpcCmdMsg_s{
	long msgType;
	eponOamCmdMsg_t msg;
}eponOamIpcCmdMsg_t;

int sendEponOamCmdMsg(u_int msgType, u_char *buf, u_int len , u_int *cmdSeqOut);
#endif // TCSUPPORT_EPON_OAM

#endif /* _EPON_LIB_H_ */

