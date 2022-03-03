
#ifndef	__BBU_SOC_H__
#define	__BBU_SOC_H__


//#include "rt_config.h"


#define POLLING_MODE_DETECT_INTV  300
extern void os_TCIfQuery (unsigned short query_id, void* result, void* result2);


typedef struct _BBUPollingMode{
	RALINK_TIMER_STRUCT			PollingModeDetect;	
	BOOLEAN							PollingModeDetectRunning;	
	struct work_struct					PollingDataBH;
	UINT32   							pAd_va;
	NDIS_SPIN_LOCK          			PollingModeLock;
}BBUPollingMode,*PBBUPollingMode;

extern void tc3162_enable_irq(unsigned int irq);
extern void tc3162_disable_irq(unsigned int irq);
extern VOID PeriodicPollingModeDetect(
	IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);
DECLARE_TIMER_FUNCTION(PeriodicPollingModeDetect);

extern VOID PollingModeIsr(struct work_struct *work);
/*
    ========================================================================

    Routine Description:
       Trendchip DMT Trainning status detect

    Arguments:
        data                     Point to RTMP_ADAPTER

    Return Value:
        NONE

    Note: 

    ========================================================================
*/
extern void pcieReset(void);
extern int pcieRegInitConfig(void);
extern void BBU_PCIE_Init(void);
extern VOID BBUPollingModeClose(IN RTMP_ADAPTER *pAd);
extern VOID BBUPollingModeInit(IN RTMP_ADAPTER *pAd);
extern VOID BBUPollingModeStart(IN RTMP_ADAPTER *pAd);
extern VOID BBUPrepareMAC(IN RTMP_ADAPTER *pAd, PUCHAR macaddr );
#endif

