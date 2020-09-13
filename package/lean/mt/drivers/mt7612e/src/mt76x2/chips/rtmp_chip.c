/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rt_chip.c

	Abstract:
	Ralink Wireless driver CHIP related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include "rt_config.h"


/*
========================================================================
Routine Description:
	write high memory.
	if firmware do not support auto high/low memory switching, we should switch to high memory by ourself.

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Memory offsets
	Value			- Written value
	Unit				- Unit in "Byte"

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipWriteHighMemory(
	IN RTMP_ADAPTER *pAd,
	IN USHORT Offset,
	IN UINT32 Value,
	IN UINT8 Unit)
{
#ifdef RTMP_MAC_PCI
#ifdef SPECIFIC_BCN_BUF_SUPPORT
unsigned long irqFlag = 0;
	RTMP_MAC_SHR_MSEL_LOCK(pAd, HIGHER_SHRMEM, irqFlag);
	RtmpChipWriteMemory(pAd, Offset, Value, Unit);
	RTMP_MAC_SHR_MSEL_UNLOCK(pAd, LOWER_SHRMEM, irqFlag);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
#endif /* RTMP_MAC_PCI */
}


/*
========================================================================
Routine Description:
	write memory

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Memory offsets
	Value			- Written value
	Unit				- Unit in "Byte"
Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipWriteMemory(
	IN	RTMP_ADAPTER	*pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value,
	IN	UINT8			Unit)
{
	switch(Unit)
	{
		case 1:
			RTMP_IO_WRITE8(pAd, Offset, Value);
			break;
		case 2:
			RTMP_IO_WRITE16(pAd, Offset, Value);
			break;
		case 4:
			RTMP_IO_WRITE32(pAd, Offset, Value);
		default:
			break;
	}
}


/*
========================================================================
Routine Description:
	Initialize specific beacon frame architecture.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipBcnSpecInit(RTMP_ADAPTER *pAd)
{
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;


	pChipCap->FlgIsSupSpecBcnBuf = TRUE;
	pChipCap->BcnMaxHwNum = 16;
	pChipCap->WcidHwRsvNum = 255;

/* 	In 16-MBSS support mode, if AP-Client is enabled, 
	the last 8-MBSS would be occupied for AP-Client using. */
#ifdef APCLI_SUPPORT
	pChipCap->BcnMaxNum = (8 - MAX_MESH_NUM);
#else
	pChipCap->BcnMaxNum = (16 - MAX_MESH_NUM);
#endif /* APCLI_SUPPORT */

	pChipCap->BcnMaxHwSize = 0x2000;

	/* It's allowed to use the higher(secordary) 8KB shared memory */
	pChipCap->BcnBase[0] = 0x4000;
	pChipCap->BcnBase[1] = 0x4200;
	pChipCap->BcnBase[2] = 0x4400;
	pChipCap->BcnBase[3] = 0x4600;
	pChipCap->BcnBase[4] = 0x4800;
	pChipCap->BcnBase[5] = 0x4A00;
	pChipCap->BcnBase[6] = 0x4C00;
	pChipCap->BcnBase[7] = 0x4E00;
	pChipCap->BcnBase[8] = 0x5000;
	pChipCap->BcnBase[9] = 0x5200;
	pChipCap->BcnBase[10] = 0x5400;
	pChipCap->BcnBase[11] = 0x5600;
	pChipCap->BcnBase[12] = 0x5800;
	pChipCap->BcnBase[13] = 0x5A00;
	pChipCap->BcnBase[14] = 0x5C00;
	pChipCap->BcnBase[15] = 0x5E00;

	pAd->chipOps.BeaconUpdate = RtmpChipWriteHighMemory;
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
}


/*
========================================================================
Routine Description:
	Initialize normal beacon frame architecture.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipBcnInit(
	IN RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;


	pChipCap->FlgIsSupSpecBcnBuf = FALSE;
	pChipCap->BcnMaxHwNum = 8;
	pChipCap->BcnMaxNum = (pChipCap->BcnMaxHwNum - MAX_MESH_NUM - MAX_APCLI_NUM);
	pChipCap->BcnMaxHwSize = 0x1000;

	pChipCap->BcnBase[0] = 0x7800;
	pChipCap->BcnBase[1] = 0x7A00;
	pChipCap->BcnBase[2] = 0x7C00;
	pChipCap->BcnBase[3] = 0x7E00;
	pChipCap->BcnBase[4] = 0x7200;
	pChipCap->BcnBase[5] = 0x7400;
	pChipCap->BcnBase[6] = 0x5DC0;
	pChipCap->BcnBase[7] = 0x5BC0;

	/*
		If the MAX_MBSSID_NUM is larger than 6,
		it shall reserve some WCID space(wcid 222~253) for beacon frames.
		-	these wcid 238~253 are reserved for beacon#6(ra6).
		-	these wcid 222~237 are reserved for beacon#7(ra7).
	*/
	if (pChipCap->BcnMaxNum == 8)
		pChipCap->WcidHwRsvNum = 222;
	else if (pChipCap->BcnMaxNum == 7)
		pChipCap->WcidHwRsvNum = 238;
	else
		pChipCap->WcidHwRsvNum = 255;

	pAd->chipOps.BeaconUpdate = RtmpChipWriteMemory;
}


#ifdef RLT_MAC
/*
========================================================================
Routine Description:
	Initialize specific beacon frame architecture.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID rlt_bcn_buf_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	pChipCap->FlgIsSupSpecBcnBuf = FALSE;
#if defined(MT7601) || defined(MT76x2)
	if (IS_MT7601(pAd) || IS_MT76x2(pAd)) 
	{
		pChipCap->BcnMaxHwNum = 8;
		pChipCap->WcidHwRsvNum = 127;
	}
	else
#endif /* MT7601 || MT76x2 */
	{
		pChipCap->BcnMaxHwNum = 16;
		pChipCap->WcidHwRsvNum = 255;
	}

/*
	In 16-MBSS support mode, if AP-Client is enabled, 
	the last 8-MBSS would be occupied for AP-Client using.
*/
#ifdef APCLI_SUPPORT
	pChipCap->BcnMaxNum = (8 - MAX_MESH_NUM);
#else
	pChipCap->BcnMaxNum = (pChipCap->BcnMaxHwNum - MAX_MESH_NUM);
#endif /* APCLI_SUPPORT */

	pChipCap->BcnMaxHwSize = 0x2000;

	pChipCap->BcnBase[0] = 0xc000;
	pChipCap->BcnBase[1] = 0xc200;
	pChipCap->BcnBase[2] = 0xc400;
	pChipCap->BcnBase[3] = 0xc600;
	pChipCap->BcnBase[4] = 0xc800;
	pChipCap->BcnBase[5] = 0xca00;
	pChipCap->BcnBase[6] = 0xcc00;
	pChipCap->BcnBase[7] = 0xce00;
	pChipCap->BcnBase[8] = 0xd000;
	pChipCap->BcnBase[9] = 0xd200;
	pChipCap->BcnBase[10] = 0xd400;
	pChipCap->BcnBase[11] = 0xd600;
	pChipCap->BcnBase[12] = 0xd800;
	pChipCap->BcnBase[13] = 0xda00;
	pChipCap->BcnBase[14] = 0xdc00;
	pChipCap->BcnBase[15] = 0xde00;

#ifdef CONFIG_MULTI_CHANNEL
	/* Record HW Null Frame offset */
	//pAd->NullBufOffset[0] = 0xd000;
	pAd->NullBufOffset[0] = 0xd000;
	pAd->NullBufOffset[1] = 0xd200;
#endif /* CONFIG_MULTI_CHANNEL */

	pAd->chipOps.BeaconUpdate = RtmpChipWriteMemory;
}
#endif /* RLT_MAC */




#ifdef HW_ANTENNA_DIVERSITY_SUPPORT
UINT32 SetHWAntennaDivsersity(
	IN PRTMP_ADAPTER		pAd,
	IN BOOLEAN				Enable)
{
	if (Enable == TRUE)
	{
		UINT8 BBPValue = 0, RFValue = 0;
		USHORT value;

		// RF_R29 bit7:6
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_GAIN, value);
		
		RT30xxReadRFRegister(pAd, RF_R29, &RFValue);
		RFValue &= 0x3f; // clear bit7:6
		RFValue |= (value << 6);
		RT30xxWriteRFRegister(pAd, RF_R29, RFValue);

		// BBP_R47 bit7=1
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
		BBPValue |= 0x80;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);
	
		BBPValue = 0xbe;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, BBPValue);
		BBPValue = 0xb0;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, BBPValue);
		BBPValue = 0x23;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);
		BBPValue = 0x3a;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R153, BBPValue);
		BBPValue = 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, BBPValue);
		BBPValue = 0x3b;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R155, BBPValue);
		BBPValue = 0x04;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R253, BBPValue);

		DBGPRINT(RT_DEBUG_TRACE, ("HwAnDi> Enable!\n"));
	}
	else
	{
		UINT8 BBPValue = 0;

		/*
			main antenna: BBP_R152 bit7=1
			aux antenna: BBP_R152 bit7=0
		 */
		if (pAd->FixDefaultAntenna == 0)
		{
			/* fix to main antenna */
			/* do not care BBP R153, R155, R253 */
			BBPValue = 0x3e;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, BBPValue);
			BBPValue = 0x30;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, BBPValue);
			BBPValue = 0x23;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);
			BBPValue = 0x00;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, BBPValue);
		}
		else
		{
			/* fix to aux antenna */
			/* do not care BBP R153, R155, R253 */
			BBPValue = 0x3e;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, BBPValue);
			BBPValue = 0x30;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, BBPValue);
			BBPValue = 0xa3;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);
			BBPValue = 0x00;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, BBPValue);
		}

		DBGPRINT(RT_DEBUG_TRACE, ("HwAnDi> Disable!\n"));
	}

	return 0;
}
#endif // HW_ANTENNA_DIVERSITY_SUPPORT // 




INT WaitForAsicReady(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val = 0, reg = MAC_CSR0;
	int idx = 0;

	do
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return FALSE;
		
		RTMP_IO_READ32(pAd, reg, &mac_val);
		if ((mac_val != 0x00) && (mac_val != 0xFFFFFFFF))
			return TRUE;

		RtmpOsMsDelay(5);
	} while (idx++ < 500);

	DBGPRINT(RT_DEBUG_ERROR,
				("%s(0x%x):AsicNotReady!\n",
				__FUNCTION__, mac_val));
	
	return FALSE;
}


INT AsicGetMacVersion(RTMP_ADAPTER *pAd)
{
	UINT32 reg = MAC_CSR0;


#ifdef RT65xx
	if (IS_RT65XX(pAd))
		RTMP_IO_READ32(pAd, ASIC_VERSION, &pAd->ChipID);
#endif /* RT65xx */

	if (WaitForAsicReady(pAd) == TRUE)
	{
		RTMP_IO_READ32(pAd, reg, &pAd->MACVersion);
		DBGPRINT(RT_DEBUG_OFF, ("MACVersion[Ver:Rev]=0x%08x : 0x%08x\n",
					pAd->MACVersion, pAd->ChipID));
		return TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s() failed!\n", __FUNCTION__));
		return FALSE;
	}
}


/*
========================================================================
Routine Description:
	Initialize chip related information.

Arguments:
	pCB				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
int RtmpChipOpsHook(VOID *pCB)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pCB;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	UINT32 MacValue;
	int ret = 0;
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

	/* sanity check */
	if (WaitForAsicReady(pAd) == FALSE)
		return -1;

	RTMP_IO_READ32(pAd, MAC_CSR0, &MacValue);
	pAd->MACVersion = MacValue;
	
	if (pAd->MACVersion == 0xffffffff)
		return -1;

#ifdef RT65xx
	RTMP_IO_READ32(pAd, ASIC_VERSION, &MacValue);
	pAd->ChipID = MacValue;

	if (pAd->ChipID == 0xffffffff)
		return -1;
#endif /* RT65xx */

	/* default init */
	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_LEGACY);

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
		RTMP_SYS_IO_READ32(0xb000000c, &pAd->CommonCfg.CID);
		RTMP_SYS_IO_READ32(0xb0000000, &pAd->CommonCfg.CN);

#ifdef RT6352
		if (IS_RT6352(pAd)) {
			pAd->CommonCfg.PKG_ID = (UCHAR)((pAd->CommonCfg.CID >> 16) & 0x0001);
			pAd->CommonCfg.Chip_VerID = (UCHAR)((pAd->CommonCfg.CID >> 8) & 0x0f);
			pAd->CommonCfg.Chip_E_Number = (UCHAR)((pAd->CommonCfg.CID) & 0x0f);
		}
#endif /* RT6352 */

		DBGPRINT(RT_DEBUG_TRACE, ("CN: %lx\tCID = %lx\n",
				pAd->CommonCfg.CN, pAd->CommonCfg.CID));
	}
#endif /* RTMP_RBUS_SUPPORT */

	/* EDCCA */
	pChipOps->ChipSetEDCCA= NULL;



#ifdef RT8592
	if (IS_RT8592(pAd)) {
		RT85592_Init(pAd);
		goto done;
	}
#endif /* RT8592 */


#ifdef MT76x2
	if (IS_MT76x2(pAd)) {
		mt76x2_init(pAd);
		goto done;
	}
#endif


#ifdef RT6352
	if (IS_RT6352(pAd)) {
		RT6352_Init(pAd);
		goto done;
	}
#endif /* RT6352 */

#ifdef RTMP_MAC
	// TODO: default settings for rest of the chips!! change this to really default chip.
	RTxx_default_Init(pAd);
#endif /* RTMP_MAC */

	/* We depends on RfICType and MACVersion to assign the corresponding operation callbacks. */



done:
	DBGPRINT(RT_DEBUG_TRACE, ("Chip specific bbpRegTbSize=%d!\n", pChipCap->bbpRegTbSize));
	DBGPRINT(RT_DEBUG_TRACE, ("Chip VCO calibration mode = %d!\n", pChipCap->FlgIsVcoReCalMode));
#ifdef DOT11W_PMF_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("[PMF] Encryption mode = %d\n", pChipCap->FlgPMFEncrtptMode));
#endif /* DOT11W_PMF_SUPPORT */

	return ret;
}

#ifdef RT65xx
BOOLEAN isExternalPAMode(RTMP_ADAPTER *ad, INT channel)
{
	BOOLEAN pa_mode = FALSE;

        if (channel > 14) {
	        if (ad->chipCap.PAType == EXT_PA_2G_5G)
                	pa_mode = TRUE;
                else if (ad->chipCap.PAType == EXT_PA_5G_ONLY)
                        pa_mode = TRUE;
                else
        	        pa_mode = FALSE;
        } else {
                if (ad->chipCap.PAType == EXT_PA_2G_5G)
                        pa_mode = TRUE;
                else if ((ad->chipCap.PAType == EXT_PA_5G_ONLY) ||
                         (ad->chipCap.PAType == INT_PA_2G_5G))
                        pa_mode = FALSE;
                else if (ad->chipCap.PAType == EXT_PA_2G_ONLY)
                        pa_mode = TRUE;
        }

	return pa_mode;
}

BOOLEAN is_external_lna_mode(RTMP_ADAPTER *ad, INT channel)
{
	BOOLEAN lna_mode = FALSE;

	/* b'00: 2.4G+5G external LNA, b'01: 5G external LNA, b'10: 2.4G external LNA, b'11: Internal LNA */
	if (channel > 14) {
		if ((ad->chipCap.LNA_type == 0x0) || (ad->chipCap.LNA_type == 0x1))
	            	lna_mode = TRUE;
	    	else
	        	lna_mode = FALSE;
	} else {
	    	if ((ad->chipCap.LNA_type == 0x0) || (ad->chipCap.LNA_type == 0x10))
	            	lna_mode = TRUE;
	    	else
	            	lna_mode = FALSE;
	}
	
	return lna_mode;
}
#endif /* RT65xx */

