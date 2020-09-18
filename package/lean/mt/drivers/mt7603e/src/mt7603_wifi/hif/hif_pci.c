
#include "rt_config.h"


#ifdef RTMP_MAC_PCI
INT NICInitPwrPinCfg(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	UINT32 mac_val = 0;
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));

		return FALSE;
	}

#if defined(RTMP_MAC) || defined(RLT_MAC)
	/*@!Release
		For MT76x0 series, 
		PWR_PIN_CFG[2:0]: obsolete, no function
		Don't need to change PWR_PIN_CFG here.
	*/
		mac_val = 0x3;	/* To fix driver disable/enable hang issue when radio off*/
	RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, mac_val);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

	return TRUE;
}



#endif /* RTMP_MAC_PCI */


