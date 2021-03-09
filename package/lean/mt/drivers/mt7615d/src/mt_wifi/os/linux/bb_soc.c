#include "rt_config.h"

#include "os/bb_soc.h"
/* #include "rtmp_timer.h" */
/* #include "rt_config.h" */
#ifdef WSC_AP_SUPPORT
extern int wscTimerRunning;
extern int wscStatus;
extern int statusprobe;
extern unsigned short wsc_done;
#endif

/*
 *  ========================================================================
 *  Routine Description:
 *     Trendchip DMT Trainning status detect
 *
 *  Arguments:
 *	data                     Point to RTMP_ADAPTER
 *
 *  Return Value:
 *	NONE
 *  ========================================================================
 */
static UCHAR dslStateChg;
VOID PeriodicPollingModeDetect(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	unsigned long irqFlags;
	UCHAR modem_status = 0;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	POS_COOKIE _pObj = (POS_COOKIE)(pAd->OS_Cookie);

	os_TCIfQuery(0x0002, &modem_status, NULL);

	if ((modem_status == 0x08) || (modem_status == 0x10)) {
		if (!(dslStateChg & (1 << 0))) {
			RTMP_INT_LOCK(&pAd->irq_lock, irqFlags);
			dslStateChg |= (1 << 0);
			RTMP_INT_UNLOCK(&pAd->irq_lock, irqFlags);
			/* disable enterrupt */
			tc3162_disable_irq(_pObj->pci_dev->irq);
		}

		schedule_work(&pAd->Pollingmode.PollingDataBH);
		/* slow down the POLLING MODE DETECT while the dmt in wait init state */
		/* pAd->PollingModeDetect.expires = jiffies + POLLING_MODE_DETECT_INTV; */
		RTMPModTimer(&pAd->Pollingmode.PollingModeDetect, 10);
		pAd->Pollingmode.PollingModeDetectRunning = TRUE;
		return;
	}

	if (dslStateChg & (1 << 0)) {
		tc3162_enable_irq(_pObj->pci_dev->irq);
		RTMP_INT_LOCK(&pAd->irq_lock, irqFlags);
		dslStateChg &= ~(1 << 0);
		RTMP_INT_UNLOCK(&pAd->irq_lock, irqFlags);
	}

	RTMPModTimer(&pAd->Pollingmode.PollingModeDetect, POLLING_MODE_DETECT_INTV);
	pAd->Pollingmode.PollingModeDetectRunning = TRUE;
#ifdef WSC_AP_SUPPORT
	{
		static int lastWscStatus;
		PWSC_CTRL  pWscControl = &pAd->ApCfg.MBSSID[MAIN_MBSSID].WscControl;

		wscTimerRunning = pWscControl->Wsc2MinsTimerRunning;
		wscStatus = pWscControl->WscStatus;

		if ((wscStatus == STATUS_WSC_CONFIGURED) && (lastWscStatus != STATUS_WSC_CONFIGURED)) {
#ifndef LED_WPSSPEC_COMPLY
			wsc_done = 10;
#else
			wsc_done = 600;
#endif
			/* ledTurnOff(LED_WLAN_WPS_ACT_STATUS);//xyyou??? */
			/* ledTurnOn(LED_WLAN_WPS_STATUS); */
			/* ledTurnOn(LED_WLAN_WPS_NOACT_STATUS); */
		}

		lastWscStatus = wscStatus;
	}
#endif
}


VOID PollingModeIsr(struct work_struct *work)
{
	PBBUPollingMode pPollingmode = container_of(work, BBUPollingMode, PollingDataBH);
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pPollingmode->pAd_va;
	struct net_device *net_dev = pAd->net_dev;

	rt2860_interrupt(0, net_dev);
}


VOID BBUPollingModeClose(IN RTMP_ADAPTER *pAd)
{
	BOOLEAN		Cancelled;

	pAd->Pollingmode.PollingModeDetectRunning = FALSE;
	RTMPCancelTimer(&pAd->Pollingmode.PollingModeDetect, &Cancelled);
}

BUILD_TIMER_FUNCTION(PeriodicPollingModeDetect);


VOID BBUPollingModeInit(IN RTMP_ADAPTER *pAd)
{
	NdisAllocateSpinLock(&pAd->Pollingmode.PollingModeLock);/* for polling mode */
	RTMPInitTimer(pAd, &pAd->Pollingmode.PollingModeDetect, GET_TIMER_FUNCTION(PeriodicPollingModeDetect), pAd, FALSE);
	pAd->Pollingmode.PollingModeDetectRunning = FALSE;
}

VOID BBUPollingModeStart(IN RTMP_ADAPTER *pAd)
{
	if (pAd->Pollingmode.PollingModeDetectRunning == FALSE) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("jiffies=%08lx, POLLING_MODE_DETECT_INTV=%d\r\n", jiffies, POLLING_MODE_DETECT_INTV));
		RTMPSetTimer(&pAd->Pollingmode.PollingModeDetect, POLLING_MODE_DETECT_INTV);
	}

	/* init a BH task here */
	INIT_WORK(&(pAd->Pollingmode.PollingDataBH), PollingModeIsr);
}

VOID BBU_PCIE_Init(void)
{
	pcieReset();
	pcieRegInitConfig();
}

VOID BBUPrepareMAC(IN RTMP_ADAPTER *pAd, PUCHAR macaddr)
{
	UCHAR FourByteOffset = 0;
	UCHAR NWlanExt = 0;

	FourByteOffset = macaddr[5] % 4;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("\r\nFourByteOffset is %d", FourByteOffset));
	NWlanExt = pAd->ApCfg.BssidNum;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("\r\nNWlanExt is %d", NWlanExt));

	switch (NWlanExt) {
	case 1:
		break;

	case 2:
		switch (FourByteOffset) {
		case 1:
		case 3:
			macaddr[5]--;
			break;

		case 0:
		case 2:
			break;
		}

		break;

	case 3:
	case 4:
		switch (FourByteOffset) {
		case 0:
			break;

		case 1:
			macaddr[5]--;
			break;

		case 2:
			macaddr[5] -= 2;
			break;

		case 3:
			macaddr[5] -= 3;
			break;
		}

		break;

	default:
		break;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("current MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
			 macaddr[0], macaddr[1],
			 macaddr[2], macaddr[3],
			 macaddr[4], macaddr[5]));
	/*generate bssid from cpe mac address end, merge from linos, 20100208*/
}
