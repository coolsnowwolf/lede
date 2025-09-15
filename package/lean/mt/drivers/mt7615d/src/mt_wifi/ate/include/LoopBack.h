
#ifndef _LOOPBACK_H_
#define _LOOPBACK_H_

#define LOOPBACK_SIZE (10240*6)


enum _LOOPBACK_STATUS {
	RUNNING = 0,
	PASS,
	FAIL
};

enum _LOOPBACK_FAIL {
	NO_ERROR = 0,
	TX_TIMEOUT,
	RX_TIMEOUT,
	BIT_TRUE_FAIL
};

struct _LOOPBACK_SETTING {
	UINT32 StartLen;
	UINT32 StopLen;
	UINT32 RepeatTimes; /* 0 = infinite */
	UINT32 IsDefaultPattern;
	/* #ifdef RTMP_USB_SUPPORT */
	UINT32 BulkOutNumber;
	UINT32 BulkInNumber;
	UINT32 TxAggNumber;
	UINT32 RxAggPktLmt;/* pkt numbers */
	UINT32 RxAggLmt;/* pkt size */
	UINT32 RxAggTO;/* timeout (us) */
	UINT32 RxAggEnable;
	/* #endif */
};
/* TODO:Unify */
#if !defined(COMPOS_TESTMODE_WIN)
#define MAX_TX_BULK_PIPE_NUM 6
#define MAX_RX_BULK_PIPE_NUM 2
#endif
struct _LOOPBACK_RESULT {
	UINT32 Status; /* 0 running, 1 pass , 2 fail */
	UINT32 FailReason; /* 0 no error, 1 TX timeout, 2 RX timeout, 3 bit true fail */
	UINT32 TxPktCount;
	UINT32 RxPktCount;
	UINT32 TxByteCount; /* 0 = infinite */
	UINT32 RxByteCount;
	/* #ifdef RTMP_USB_SUPPORT */
	UINT32 LastBulkOut;
	/* #endif */
};
struct _LOOPBACK_CTRL {
	BOOLEAN				DebugMode;
	BOOLEAN				LoopBackRunning;
	BOOLEAN				LoopBackWaitRx;
	struct _LOOPBACK_RESULT		LoopBackResult;
	struct _LOOPBACK_SETTING	LoopBackSetting;
	UINT8				LoopBackBulkoutNumber;
	UCHAR				LoopBackTxRaw[LOOPBACK_SIZE];
	UCHAR				LoopBackRxRaw[LOOPBACK_SIZE];
	UINT32				LoopBackTxRawLen;
	UINT32				LoopBackRxRawLen;
	UINT32				LoopBackExpectTxLen;
	UINT32				LoopBackExpectRxLen;
	UCHAR				LoopBackExpectTx[LOOPBACK_SIZE];
	UCHAR				LoopBackExpectRx[LOOPBACK_SIZE];
	NDIS_SPIN_LOCK		LoopBackLock;
	BOOLEAN				LoopBackDefaultPattern;
	RTMP_OS_TASK		LoopBackTxTask;
	RTMP_OS_COMPLETION	LoopBackEvent;
	BOOLEAN				LoopBackUDMA;
#ifdef RTMP_PCI_SUPPORT
	RTMP_OS_COMPLETION LoopBackPCITxEvent;
#endif
};

INT32 CheckFWROMWiFiSysOn(struct _RTMP_ADAPTER *pAd);

void LoopBack_Start(struct _RTMP_ADAPTER *pAd, struct _LOOPBACK_SETTING *pSetting);
void LoopBack_Stop(struct _RTMP_ADAPTER *pAd);
void LoopBack_Status(struct _RTMP_ADAPTER *pAd, struct _LOOPBACK_RESULT *pResult);
void LoopBack_RawData(struct _RTMP_ADAPTER *pAd, UINT32 *pLength, BOOLEAN IsTx, UINT8 *pRawData);
void LoopBack_ExpectRx(struct _RTMP_ADAPTER *pAd, UINT32 Length, UINT8 *pRawData);
void LoopBack_ExpectTx(struct _RTMP_ADAPTER *pAd, UINT32 Length, UINT8 *pRawData);
void LoopBack_Run(struct _RTMP_ADAPTER *pAd, struct _LOOPBACK_SETTING *pSetting, UINT32 length);
void LoopBack_BitTrueCheck(struct _RTMP_ADAPTER *pAd);
void LoopBack_Fail(struct _RTMP_ADAPTER *pAd,  enum _LOOPBACK_FAIL FailNum);
#ifdef COMPOS_TESTMODE_WIN	/* TODO::Unify thread parameter */
INT LoopBack_TxThread(IN OUT PVOID Context);
#else
INT LoopBack_TxThread(ULONG Context);
#endif
void LoopBack_Rx(struct _RTMP_ADAPTER *pAd, UINT32 pktlen, UINT8 *pData);
#ifdef RTMP_PCI_SUPPORT
void PCILoopBack_Run(struct _RTMP_ADAPTER *pAd, struct _LOOPBACK_SETTING *pSetting, UINT32 length);
INT32 ATECheckFWROMWiFiSysOn(struct _RTMP_ADAPTER *pAd);
#endif

#endif /* _LOOPBACK_H_ */
