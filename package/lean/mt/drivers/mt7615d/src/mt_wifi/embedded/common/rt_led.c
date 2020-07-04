/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rt_led.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include	"rt_config.h"

#ifdef VENDOR_FEATURE7_SUPPORT
#ifdef WSC_INCLUDED
#include "arris_wps_gpio_handler.h"
#endif
#endif
#ifdef LED_CONTROL_SUPPORT

#ifdef CONFIG_ANDES_SUPPORT
INT LED_Array[16][12] = {
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ LED_SOLID_ON,   LED_TX_BLINKING,    LED_SOLID_OFF,    LED_SOLID_ON,   LED_SOLID_OFF,  -1,  LED_SOLID_ON, -1,   LED_BLINKING_170MS_ON_170MS_OFF, -1, -1, LED_WPS_5S_ON_3S_OFF_THEN_BLINKING},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{  3,  2,   -1,   -1,   -1, -1, 16,   1,  5,  -1, -1, 17},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,   2,     1,   -1,   -1, -1,  3, -1,   6, -1, -1,   0},
	{ 1,   2,     1,   -1,   -1, -1, -1,  1,   4, -1, -1, 18}
};
#endif /* CONFIG_ANDES_SUPPORT */

/*
	========================================================================

	Routine Description:
		Set LED Status

	Arguments:
		pAd						Pointer to our adapter
		Status					LED Status

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPSetLEDStatus(RTMP_ADAPTER *pAd, UCHAR Status)
{
	/*ULONG			data; */
	UCHAR			LinkStatus = 0;
	UCHAR			LedMode;
	UCHAR			MCUCmd = 0;
	BOOLEAN		bIgnored = FALSE;
	UCHAR			Channel = 0;
	/* #ifdef MT76x0 */
	INT				LED_CMD = -1;
	/* #endif */
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	PWSC_CTRL		pWscControl = NULL;
#ifdef CONFIG_AP_SUPPORT
	pWscControl = &pAd->ApCfg.MBSSID[MAIN_MBSSID].WscControl;
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef CONFIG_ATE

	/*
		In ATE mode of RT2860 AP/STA, we have erased 8051 firmware.
		So LED mode is not supported when ATE is running.
	*/
	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */
	LedMode = LED_MODE(pAd);

	/* #ifdef MT76x0 */
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7615(pAd) || IS_MT7622(pAd)) {
		LedMode = 1;
#if defined(WSC_INCLUDED) && defined(WSC_LED_SUPPORT)

		if (Status > LED_WPS_SUCCESS)
#else
		if (Status > LED_POWER_UP)
#endif /* defined(WSC_INCLUDED) && defined(WSC_LED_SUPPORT) */
			return;
		else
			LED_CMD = LED_Array[LedMode][Status];
	}

	/* #endif */
	switch (Status) {
	case LED_LINK_DOWN:
		LinkStatus = LINK_STATUS_LINK_DOWN;
		pAd->LedCntl.LedIndicatorStrength = 0;
		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_LINK_UP:
		Channel = HcGetRadioChannel(pAd);

		if (Channel > 14)
			LinkStatus = LINK_STATUS_ABAND_LINK_UP;
		else
			LinkStatus = LINK_STATUS_GBAND_LINK_UP;

		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_RADIO_ON:
		LinkStatus = LINK_STATUS_RADIO_ON;
		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_HALT:
		LedMode = 0; /* Driver sets MAC register and MAC controls LED */

	case LED_RADIO_OFF:
		LinkStatus = LINK_STATUS_RADIO_OFF;
		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_WPS:
		LinkStatus = LINK_STATUS_WPS;
		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_ON_SITE_SURVEY:
		LinkStatus = LINK_STATUS_ON_SITE_SURVEY;
		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_POWER_UP:
		LinkStatus = LINK_STATUS_POWER_UP;
		MCUCmd = MCU_SET_LED_MODE;
		break;
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT

	case LED_WPS_IN_PROCESS:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_IN_PROCESS;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_IN_PROCESS;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: LED_WPS_IN_PROCESS\n", __func__));
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_ERROR:
		if (WscSupportWPSLEDMode(pAd)) {
			/* In the case of LED mode 9, the error LED should be turned on only after WPS walk time expiration. */
			if ((pWscControl->bWPSWalkTimeExpiration == FALSE) &&
				(LED_MODE(pAd) == WPS_LED_MODE_9)) {
				/* do nothing. */
			} else {
				LinkStatus = LINK_STATUS_WPS_ERROR;
				MCUCmd = MCU_SET_WPS_LED_MODE;
			}

			pWscControl->WscLEDMode = LED_WPS_ERROR;
			pWscControl->WscLastWarningLEDMode = LED_WPS_ERROR;
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_SESSION_OVERLAP_DETECTED:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_SESSION_OVERLAP_DETECTED;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_SESSION_OVERLAP_DETECTED;
			pWscControl->WscLastWarningLEDMode = LED_WPS_SESSION_OVERLAP_DETECTED;
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_SUCCESS:
		if (WscSupportWPSLEDMode(pAd)) {
			if ((LED_MODE(pAd) == WPS_LED_MODE_7) ||
				(LED_MODE(pAd) == WPS_LED_MODE_11) ||
				(LED_MODE(pAd) == WPS_LED_MODE_12)
			   ) {
				/* In the WPS LED mode 7, 11 and 12, the blue LED would last 300 seconds regardless of the AP's security settings. */
				LinkStatus = LINK_STATUS_WPS_SUCCESS_WITH_SECURITY;
				MCUCmd = MCU_SET_WPS_LED_MODE;
				pWscControl->WscLEDMode = LED_WPS_SUCCESS;
				/* Turn off the WPS successful LED pattern after 300 seconds. */
				RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
			} else if (LED_MODE(pAd) == WPS_LED_MODE_8) { /* The WPS LED mode 8 */
				if (WscAPHasSecuritySetting(pAd, pWscControl)) { /* The WPS AP has the security setting. */
					LinkStatus = LINK_STATUS_WPS_SUCCESS_WITH_SECURITY;
					MCUCmd = MCU_SET_WPS_LED_MODE;
					pWscControl->WscLEDMode = LED_WPS_SUCCESS;
					/* Turn off the WPS successful LED pattern after 300 seconds. */
					RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
				} else { /* The WPS AP does not have the secuirty setting. */
					LinkStatus = LINK_STATUS_WPS_SUCCESS_WITHOUT_SECURITY;
					MCUCmd = MCU_SET_WPS_LED_MODE;
					pWscControl->WscLEDMode = LED_WPS_SUCCESS;
					/* Turn off the WPS successful LED pattern after 300 seconds. */
					RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
				}
			} else if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* The WPS LED mode 9. */
				/* Always turn on the WPS blue LED for 300 seconds. */
				LinkStatus = LINK_STATUS_WPS_BLUE_LED;
				MCUCmd = MCU_SET_WPS_LED_MODE;
				pWscControl->WscLEDMode = LED_WPS_SUCCESS;
				/* Turn off the WPS successful LED pattern after 300 seconds. */
				RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
			} else {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: LED_WPS_SUCCESS (Incorrect LED mode = %d)\n",
						 __func__, LED_MODE(pAd)));
				ASSERT(FALSE);
			}
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_TURN_LED_OFF:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_TURN_LED_OFF;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_TURN_LED_OFF;
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_TURN_ON_BLUE_LED:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_BLUE_LED;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		} else
			bIgnored = TRUE;

		break;

	case LED_NORMAL_CONNECTION_WITHOUT_SECURITY:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_NORMAL_CONNECTION_WITHOUT_SECURITY;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		} else
			bIgnored = TRUE;

		break;

	case LED_NORMAL_CONNECTION_WITH_SECURITY:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_NORMAL_CONNECTION_WITH_SECURITY;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		} else
			bIgnored = TRUE;

		break;

	/*WPS LED Mode 10 */
	case LED_WPS_MODE10_TURN_ON:
		if (WscSupportWPSLEDMode10(pAd)) {
			LinkStatus = LINK_STATUS_WPS_MODE10_TURN_ON;
			MCUCmd = MCU_SET_WPS_LED_MODE;
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_MODE10_FLASH:
		if (WscSupportWPSLEDMode10(pAd)) {
			LinkStatus = LINK_STATUS_WPS_MODE10_FLASH;
			MCUCmd = MCU_SET_WPS_LED_MODE;
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_MODE10_TURN_OFF:
		if (WscSupportWPSLEDMode10(pAd)) {
			LinkStatus = LINK_STATUS_WPS_MODE10_TURN_OFF;
			MCUCmd = MCU_SET_WPS_LED_MODE;
		} else
			bIgnored = TRUE;

		break;
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("RTMPSetLED::Unknown Status 0x%x\n", Status));
		break;
	}

	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7615(pAd) || IS_MT7622(pAd)) {
#if defined(MT7615) || defined(MT7622)
		AndesLedEnhanceOP(pAd, LED_ID_WLAN_OD, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE, LED_BAND_0, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
#else
		AndesLedEnhanceOP(pAd, LED_ID_WLAN_OD, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE, LED_ALL_TX_FRAMES, 0, 0, LED_CMD);
#endif
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: LED_CMD:0x%x, LED Mode:0x%x, LinkStatus:0x%x\n", __func__, LED_CMD, LedMode, LinkStatus));
	} else if (MCUCmd) {
		AsicSendCommandToMcu(pAd, MCUCmd, 0xff, LedMode, LinkStatus, FALSE);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: MCUCmd:0x%x, LED Mode:0x%x, LinkStatus:0x%x\n", __func__, MCUCmd, LedMode, LinkStatus));
	}

	/*
		Keep LED status for LED SiteSurvey mode.
		After SiteSurvey, we will set the LED mode to previous status.
	*/
	if ((Status != LED_ON_SITE_SURVEY) && (Status != LED_POWER_UP) && (bIgnored == FALSE))
		pAd->LedCntl.LedStatus = Status;
}


/*
	========================================================================

	Routine Description:
		Set LED Signal Stregth

	Arguments:
		pAd						Pointer to our adapter
		Dbm						Signal Stregth

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		Can be run on any IRQL level.

		According to Microsoft Zero Config Wireless Signal Stregth definition as belows.
		<= -90  No Signal
		<= -81  Very Low
		<= -71  Low
		<= -67  Good
		<= -57  Very Good
		 > -57  Excellent
	========================================================================
*/
VOID RTMPSetSignalLED(RTMP_ADAPTER *pAd, NDIS_802_11_RSSI Dbm)
{
	UCHAR		nLed = 0;

	if (pAd->LedCntl.MCULedCntl.field.LedMode == LED_MODE_SIGNAL_STREGTH) {
		if (Dbm <= -90)
			nLed = 0;
		else if (Dbm <= -81)
			nLed = 1;
		else if (Dbm <= -71)
			nLed = 3;
		else if (Dbm <= -67)
			nLed = 7;
		else if (Dbm <= -57)
			nLed = 15;
		else
			nLed = 31;

		/* */
		/* Update Signal Stregth to firmware if changed. */
		/* */
		if (pAd->LedCntl.LedIndicatorStrength != nLed) {
			AsicSendCommandToMcu(pAd, MCU_SET_LED_GPIO_SIGNAL_CFG, 0xff, nLed, pAd->LedCntl.MCULedCntl.field.Polarity, FALSE);
			pAd->LedCntl.LedIndicatorStrength = nLed;
		}
	}
}


#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT
/*
	LED indication for normal connection start.
*/
VOID LEDConnectionStart(RTMP_ADAPTER *pAd)
{
	/* LED indication. */
	/*if (pAd->StaCfg[0].WscControl.bWPSSession == FALSE) */
	/*if (pAd->StaCfg[0].WscControl.WscConfMode != WSC_DISABLE && pAd->StaCfg[0].WscControl.bWscTrigger) */
	if (pAd->StaCfg[0].WscControl.WscConfMode == WSC_DISABLE) {
		if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* LED mode 9. */
			UCHAR WPSLEDStatus = 0;

			/* The AP uses OPEN-NONE. */
			if ((pAd->StaCfg[0].AuthMode == Ndis802_11AuthModeOpen)
				&& (pAd->StaCfg[0].WepStatus == Ndis802_11WEPDisabled))
				WPSLEDStatus = LED_WPS_TURN_LED_OFF;
			else /* The AP uses an encryption algorithm. */
				WPSLEDStatus = LED_WPS_IN_PROCESS;

			RTMPSetLED(pAd, WPSLEDStatus);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: %d\n", __func__, WPSLEDStatus));
		}
	}
}


/*
	LED indication for normal connection completion.
*/
VOID LEDConnectionCompletion(RTMP_ADAPTER *pAd, BOOLEAN bSuccess)
{
	/* LED indication. */
	/*if (pAd->StaCfg[0].WscControl.bWPSSession == FALSE) */
	if (pAd->StaCfg[0].WscControl.WscConfMode == WSC_DISABLE) {
		if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* LED mode 9. */
			UCHAR WPSLEDStatus = 0;

			if (bSuccess == TRUE) { /* Successful connenction. */
				/* The AP uses OPEN-NONE. */
				if ((pAd->StaCfg[0].AuthMode == Ndis802_11AuthModeOpen) && (pAd->StaCfg[0].WepStatus == Ndis802_11WEPDisabled))
					WPSLEDStatus = LED_NORMAL_CONNECTION_WITHOUT_SECURITY;
				else /* The AP uses an encryption algorithm. */
					WPSLEDStatus = LED_NORMAL_CONNECTION_WITH_SECURITY;
			} else /* Connection failure. */
				WPSLEDStatus = LED_WPS_TURN_LED_OFF;

			RTMPSetLED(pAd, WPSLEDStatus);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: %d\n", __func__, WPSLEDStatus));
		}
	}
}
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_STA_SUPPORT */


void rtmp_read_led_setting_from_eeprom(RTMP_ADAPTER *pAd)
{
	USHORT Value;
	PLED_CONTROL pLedCntl = &pAd->LedCntl;
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, Value);
		pLedCntl->MCULedCntl.word = (Value >> 8);
		RT28xx_EEPROM_READ16(pAd, EEPROM_LEDAG_CONF_OFFSET, Value);
		pLedCntl->LedAGCfg = Value;
		RT28xx_EEPROM_READ16(pAd, EEPROM_LEDACT_CONF_OFFSET, Value);
		pLedCntl->LedACTCfg = Value;
		RT28xx_EEPROM_READ16(pAd, EEPROM_LED_POLARITY_OFFSET, Value);
		pLedCntl->LedPolarity = Value;
	}
}


void RTMPStartLEDMode(IN RTMP_ADAPTER *pAd)
{
}


void RTMPInitLEDMode(IN RTMP_ADAPTER *pAd)
{
	PLED_CONTROL pLedCntl = &pAd->LedCntl;

	if (pLedCntl->MCULedCntl.word == 0xFF) {
		pLedCntl->MCULedCntl.word = 0x01;
		pLedCntl->LedAGCfg = 0x5555;
		pLedCntl->LedACTCfg = 0x2221;
#ifdef RTMP_MAC_PCI
		pLedCntl->LedPolarity = 0xA9F8;
#endif /* RTMP_MAC_PCI */
	}

	AsicSendCommandToMcu(pAd, MCU_SET_LED_AG_CFG, 0xff, (UCHAR)pLedCntl->LedAGCfg, (UCHAR)(pLedCntl->LedAGCfg >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_ACT_CFG, 0xff, (UCHAR)pLedCntl->LedACTCfg, (UCHAR)(pLedCntl->LedACTCfg >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_POLARITY, 0xff, (UCHAR)pLedCntl->LedPolarity, (UCHAR)(pLedCntl->LedPolarity >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_GPIO_SIGNAL_CFG, 0xff, 0, pLedCntl->MCULedCntl.field.Polarity, FALSE);
	pAd->LedCntl.LedIndicatorStrength = 0xFF;
	RTMPSetSignalLED(pAd, -100);	/* Force signal strength Led to be turned off, before link up */
	RTMPStartLEDMode(pAd);
}


inline void RTMPExitLEDMode(IN RTMP_ADAPTER *pAd)
{
	RTMPSetLED(pAd, LED_RADIO_OFF);
	return;
}

#endif /* LED_CONTROL_SUPPORT */


INT RTMPSetLED(RTMP_ADAPTER *pAd, UCHAR Status)
{
#ifdef RTMP_MAC_PCI

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
#ifdef LED_CONTROL_SUPPORT
#ifdef CONFIG_INIT_RADIO_ONOFF	//avoid setting LED status in case radio is off
	// PEGATRON MODIFY START
	if((pAd->ApCfg.bRadioOn) || ((Status == LED_FORCE_ON) || (Status == LED_FORCE_OFF)))
#endif
		RTMPSetLEDStatus(pAd, Status);
#endif /* LED_CONTROL_SUPPORT */
	}

#endif /* RTMP_MAC_PCI */
	return TRUE;
}


