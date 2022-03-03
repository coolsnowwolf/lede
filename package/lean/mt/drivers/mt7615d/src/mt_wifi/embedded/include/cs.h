#ifndef __CS_H__
#define __CS_H__

#ifdef CARRIER_DETECTION_SUPPORT
#define CARRIER_DETECT_RECHECK_TIME			3
#define CARRIER_GONE_TRESHOLD				10 /* Radar tone count in 1 sec */
#define CARRIER_DETECT_THRESHOLD			0x0fffffff
#define CARRIER_DETECT_STOP_RATIO			2
#define CARRIER_DETECT_STOP_RECHECK_TIME		4
#define CARRIER_DETECT_DEFAULT_MASK			20
#define CARRIER_DETECT_DELTA				7
#define CARRIER_DETECT_DIV_FLAG				0
#define CD_INT_POLLING_CMD			0x67
#define CARRIER_DETECT_CRITIRIA_COMMON			7000
#define CARRIER_DETECT_CRITIRIA_RBUS			280
#if defined(RTMP_PCI_SUPPORT) && defined(RTMP_RBUS_SUPPORT)
#define CARRIER_DETECT_CRITIRIA(_pAd)			((IS_RBUS_INF(_pAd)) ? CARRIER_DETECT_CRITIRIA_RBUS:CARRIER_DETECT_CRITIRIA_COMMON)
#else
#ifdef RTMP_RBUS_SUPPORT
#define CARRIER_DETECT_CRITIRIA(_pAd)			CARRIER_DETECT_CRITIRIA_RBUS
#else
#define CARRIER_DETECT_CRITIRIA(_pAd)			CARRIER_DETECT_CRITIRIA_COMMON
#endif /* RTMP_RBUS_SUPPORT */
#endif /* defined(RTMP_PCI_SUPPORT) && defined(RTMP_RBUS_SUPPORT) */

typedef enum CD_STATE_n {
	CD_NORMAL,
	CD_SILENCE,
	CD_MAX_STATE
} CD_STATE;

typedef enum _TONE_RADAR_VERSION {
	DISABLE_TONE_RADAR = 0,
	TONE_RADAR_V1,
	TONE_RADAR_V2,
	TONE_RADAR_V3
} TONE_RADAR_VERSION;

typedef struct CARRIER_DETECTION_s {
	BOOLEAN Enable;
	BOOLEAN bCsInit;
	UINT8 CDSessionTime;
	UINT8 CDPeriod;
	CD_STATE CD_State;
	UINT8 delta;
	UINT8 SymRund;
	UINT8 div_flag;
	UINT32 threshold;
	UINT8 recheck;
	UINT8 recheck1;
	UINT32 TimeStamp; /*unit:16us*/
	UINT32 criteria;
	ULONG idle_time;
	ULONG busy_time;
	ULONG Debug;
	ULONG OneSecIntCount;
	UINT8 CarrierGoneThreshold;
	UCHAR VGA_Mask;
	UCHAR Packet_End_Mask;
	UCHAR Rx_PE_Mask;
} CARRIER_DETECTION_STRUCT, *PCARRIER_DETECTION_STRUCT;


INT isCarrierDetectExist(
	IN PRTMP_ADAPTER pAd);

INT CarrierDetectReset(
	IN PRTMP_ADAPTER pAd);

extern VOID RtmpOsMsDelay(
	IN	ULONG	msec);

INT Set_CarrierCriteria_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CarrierReCheck_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CarrierGoneThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CarrierStopCheck_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_CarrierDebug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_CarrierDelta_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_CarrierDivFlag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_CarrierThrd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* v2 functions */
INT	Set_CarrierSymRund_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CarrierMask_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID CSInit(
	IN PRTMP_ADAPTER pAd);

VOID CarrierDetectionStart(
	IN PRTMP_ADAPTER pAd);

VOID RTMPHandleRadarInterrupt(
	IN PRTMP_ADAPTER  pAd);

VOID CarrierDetectionStop(
	IN PRTMP_ADAPTER	pAd);

VOID ToneRadarProgram_v1(
	IN PRTMP_ADAPTER pAd,
	IN ULONG threshold);

VOID ToneRadarProgram_v2(
	IN PRTMP_ADAPTER pAd,
	IN ULONG threshold);

VOID ToneRadarProgram_v3(
	IN PRTMP_ADAPTER pAd,
	IN ULONG threshold);

VOID CarrierDetectionPeriodicStateCtrl(
	IN PRTMP_ADAPTER pAd);

#ifdef CONFIG_AP_SUPPORT
INT Set_CarrierDetect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /*CONFIG_AP_SUPPORT*/
#endif /* CARRIER_DETECTION_SUPPORT */

#endif /*__CS_H__*/
