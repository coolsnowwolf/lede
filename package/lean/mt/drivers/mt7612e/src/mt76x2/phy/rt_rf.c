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
	rt_rf.c

	Abstract:
	Ralink Wireless driver RF related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include "rt_config.h"


#ifdef RTMP_RF_RW_SUPPORT
/*
	========================================================================
	
	Routine Description: Read RF register through MAC with specified bit mask

	Arguments:
		pAd		- pointer to the adapter structure
		regID	- RF register ID
		pValue1	- (RF value & BitMask)
		pValue2	- (RF value & (~BitMask))
		BitMask	- bit wise mask

	Return Value:
	
	Note:
	
	========================================================================
*/
VOID RTMP_ReadRF(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			RegID,
	OUT	PUCHAR			pValue1,
	OUT PUCHAR			pValue2,
	IN	UCHAR			BitMask)
{	
	UCHAR RfReg = 0;									
	RT30xxReadRFRegister(pAd, RegID, &RfReg);		
	if (pValue1 != NULL)								
		*pValue1 = RfReg & BitMask;			
	if (pValue2 != NULL)								
		*pValue2 = RfReg & (~BitMask);		
}

/*
	========================================================================
	
	Routine Description: Write RF register through MAC with specified bit mask

	Arguments:
		pAd		- pointer to the adapter structure
		regID	- RF register ID
		Value	- only write the part of (Value & BitMask) to RF register
		BitMask	- bit wise mask

	Return Value:
	
	Note:
	
	========================================================================
*/
VOID RTMP_WriteRF(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			RegID,
	IN	UCHAR			Value,
	IN	UCHAR			BitMask)
{
	UCHAR RfReg = 0;	
	RTMP_ReadRF(pAd, RegID, NULL, &RfReg, BitMask);
	RfReg |= ((Value) & BitMask);
	RT30xxWriteRFRegister(pAd, RegID, RfReg);
}

/*
	========================================================================
	
	Routine Description: Write RF register through MAC

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NDIS_STATUS RT30xxWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	UCHAR			value)
{
	RF_CSR_CFG_STRUC rfcsr = { { 0 } };
	UINT i = 0;
	NDIS_STATUS	 ret;

#ifdef RTMP_MAC_PCI
	if ((pAd->bPCIclkOff == TRUE) || (pAd->LastMCUCmd == SLEEP_MCU_CMD))
	{
		DBGPRINT_ERR(("RT30xxWriteRFRegister. Not allow to write RF 0x%x : fail\n",  regID));	
		return STATUS_UNSUCCESSFUL;
	}
#endif /* RTMP_MAC_PCI */

	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));


	ret = STATUS_UNSUCCESSFUL;
	do
	{
		RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

		if (!rfcsr.non_bank.RF_CSR_KICK)
			break;
		i++;
	}
	while ((i < MAX_BUSY_COUNT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));

	if ((i == MAX_BUSY_COUNT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s():RF Write failed(RetryCnt=%d, DevNotExistFlag=%d)\n",
						__FUNCTION__, i, RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
		goto done;
	}

	rfcsr.non_bank.RF_CSR_WR = 1;
	rfcsr.non_bank.RF_CSR_KICK = 1;
	rfcsr.non_bank.TESTCSR_RFACC_REGNUM = regID;

	if ((pAd->chipCap.RfReg17WtMethod == RF_REG_WT_METHOD_STEP_ON) && (regID == RF_R17))
	{
		UCHAR IdRf;
		UCHAR RfValue;
		BOOLEAN beAdd;

		RT30xxReadRFRegister(pAd, RF_R17, &RfValue);
		beAdd =  (RfValue < value) ? TRUE : FALSE;
		IdRf = RfValue;
		while(IdRf != value)
		{
			if (beAdd)
				IdRf++;
			else
				IdRf--;
			
				rfcsr.non_bank.RF_CSR_DATA = IdRf;
				RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);
				RtmpOsMsDelay(1);
		}
	}

	rfcsr.non_bank.RF_CSR_DATA = value;
	RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);

	ret = NDIS_STATUS_SUCCESS;

done:

	return ret;
}


/*
	========================================================================
	
	Routine Description: Read RF register through MAC

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NDIS_STATUS RT30xxReadRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue)
{
	RF_CSR_CFG_STRUC rfcsr = { { 0 } };
	UINT i=0, k=0;
	NDIS_STATUS	 ret = STATUS_UNSUCCESSFUL;


#ifdef RTMP_MAC_PCI
	if ((pAd->bPCIclkOff == TRUE) || (pAd->LastMCUCmd == SLEEP_MCU_CMD))
	{
		DBGPRINT_ERR(("RT30xxReadRFRegister. Not allow to read RF 0x%x : fail\n",  regID));	
		return STATUS_UNSUCCESSFUL;
	}
#endif /* RTMP_MAC_PCI */

	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));


	for (i=0; i<MAX_BUSY_COUNT; i++)
	{
		if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			goto done;
			
		RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

		if (rfcsr.non_bank.RF_CSR_KICK == BUSY)
				continue;
		
		rfcsr.word = 0;
		rfcsr.non_bank.RF_CSR_WR = 0;
		rfcsr.non_bank.RF_CSR_KICK = 1;
		rfcsr.non_bank.TESTCSR_RFACC_REGNUM = regID;
		RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);
		
		for (k=0; k<MAX_BUSY_COUNT; k++)
		{
			if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
				goto done;
				
			RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

			if (rfcsr.non_bank.RF_CSR_KICK == IDLE)
				break;
		}
		
		if ((rfcsr.non_bank.RF_CSR_KICK == IDLE) &&
			(rfcsr.non_bank.TESTCSR_RFACC_REGNUM == regID))
		{
			*pValue = (UCHAR)(rfcsr.non_bank.RF_CSR_DATA);
			break;
		}
	}

	if (rfcsr.non_bank.RF_CSR_KICK == BUSY)
	{
		DBGPRINT_ERR(("%s(): RF read R%d=0x%X fail, i[%d], k[%d]\n",
						__FUNCTION__, regID, rfcsr.word,i,k));
		goto done;
	}
	ret = STATUS_SUCCESS;
	
done:
	
	return ret;
}


#ifdef RT6352
NDIS_STATUS RT635xWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			bank,
	IN	UCHAR			regID,
	IN	UCHAR			value)
{
	RF_CSR_CFG_STRUC	rfcsr = { { 0 } };
	UINT				i = 0;
	NDIS_STATUS	 ret;

#ifdef RTMP_MAC_PCI
	if ((pAd->bPCIclkOff == TRUE) || (pAd->LastMCUCmd == SLEEP_MCU_CMD))
	{
		DBGPRINT_ERR(("RT635xWriteRFRegister. Not allow to write RF 0x%x : fail\n",  regID));	
		return STATUS_UNSUCCESSFUL;
	}
#endif /* RTMP_MAC_PCI */

	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));
	if (regID > pAd->chipCap.MaxNumOfRfId)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RegID=%d, pAd->chipCap.MaxNumOfRfId=%d\n", regID, pAd->chipCap.MaxNumOfRfId));
	}

	ret = STATUS_UNSUCCESSFUL;
	do
	{
		RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

		if (!rfcsr.bank_6352.RF_CSR_KICK)
			break;
		i++;
	}
	while ((i < MAX_BUSY_COUNT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));

	if ((i == MAX_BUSY_COUNT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
		goto done;
	}

	rfcsr.bank_6352.RF_CSR_WR = 1;
	rfcsr.bank_6352.RF_CSR_KICK = 1;
	rfcsr.bank_6352.TESTCSR_RFACC_REGNUM = (regID | (bank << 6));


	rfcsr.bank_6352.RF_CSR_DATA = value;
	RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);

	ret = NDIS_STATUS_SUCCESS;

done:

	return ret;
}


/*
	========================================================================
	
	Routine Description: Read RT30xx RF register through MAC

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NDIS_STATUS RT635xReadRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			bank,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue)
{
	RF_CSR_CFG_STRUC	rfcsr = { { 0 } };
	UINT				i=0, k=0;
	NDIS_STATUS	 ret = STATUS_UNSUCCESSFUL;

#ifdef RTMP_MAC_PCI
	if ((pAd->bPCIclkOff == TRUE) || (pAd->LastMCUCmd == SLEEP_MCU_CMD))
	{
		DBGPRINT_ERR(("%s():Not allow to read RF 0x%x : fail\n",  __FUNCTION__, regID));
		return STATUS_UNSUCCESSFUL;
	}
#endif /* RTMP_MAC_PCI */

	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));

	if (regID > pAd->chipCap.MaxNumOfRfId)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():RegID=%d, MaxNumOfRfId=%d\n",
					__FUNCTION__, regID, pAd->chipCap.MaxNumOfRfId));
	}
		
	for (i=0; i<MAX_BUSY_COUNT; i++)
	{
		if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			goto done;
			
		RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

		if (rfcsr.bank_6352.RF_CSR_KICK == BUSY)
			continue;
		
		rfcsr.word = 0;
		rfcsr.bank_6352.RF_CSR_WR = 0;
		rfcsr.bank_6352.RF_CSR_KICK = 1;
		rfcsr.bank_6352.TESTCSR_RFACC_REGNUM = (regID | (bank << 6));
		RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);
		
		for (k=0; k<MAX_BUSY_COUNT; k++)
		{
			if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
				goto done;
				
			RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

			if (rfcsr.bank_6352.RF_CSR_KICK == IDLE)
				break;
		}
		
		if ((rfcsr.bank_6352.RF_CSR_KICK == IDLE) &&
			((rfcsr.bank_6352.TESTCSR_RFACC_REGNUM & 0x3F) == regID))
		{
			*pValue = (UCHAR)(rfcsr.bank_6352.RF_CSR_DATA);
			break;
		}
	}

	if (rfcsr.bank_6352.RF_CSR_KICK == BUSY)
	{																	
		DBGPRINT_ERR(("%s(): RF read R%d[Bank %d]=0x%X fail, i[%d], k[%d]\n",
						__FUNCTION__, regID, bank, rfcsr.word,i,k));
		goto done;
	}
	ret = STATUS_SUCCESS;
done:

	return ret;
}

static inline BOOLEAN rtmp_rf_csr_poll_idle(RTMP_ADAPTER *pAd, UINT32 *rfcsr)
{
	RF_CSR_CFG_STRUC *csr_val;
	BOOLEAN idle = BUSY;
	INT i = 0;

	do
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			break;

		RTMP_IO_READ32(pAd, RF_CSR_CFG, rfcsr);

		csr_val = (RF_CSR_CFG_STRUC *)rfcsr;
#ifdef RT6352
		if (IS_RT6352(pAd))
			idle = csr_val->bank_6352.RF_CSR_KICK;
		else
#endif /* RT6352 */
			idle = csr_val->non_bank.RF_CSR_KICK;

		if (idle == IDLE)
			break;

		i++;
	} while (i < MAX_BUSY_COUNT);

	if ((i == MAX_BUSY_COUNT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed(cnt=%d)!\n", i));
	}

	return idle;
}


NDIS_STATUS rtmp_rf_write(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR bank,
	IN UCHAR regID,
	IN UCHAR value)
{
	RF_CSR_CFG_STRUC rfcsr = { { 0 } };
	NDIS_STATUS	 ret;


#ifdef RTMP_MAC_PCI
	if ((pAd->bPCIclkOff == TRUE) || (pAd->LastMCUCmd == SLEEP_MCU_CMD))
	{
		DBGPRINT_ERR(("rtmp_rf_write. Not allow to write RF 0x%x : fail\n",  regID));	
		return STATUS_UNSUCCESSFUL;
	}
#endif /* RTMP_MAC_PCI */

	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));

	ret = STATUS_UNSUCCESSFUL;
	if (rtmp_rf_csr_poll_idle(pAd, &rfcsr.word) != IDLE)
		goto done;

#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		rfcsr.bank_6352.RF_CSR_WR = 1;
		rfcsr.bank_6352.RF_CSR_KICK = 1;
		rfcsr.bank_6352.TESTCSR_RFACC_REGNUM = (regID | (bank << 6));
		rfcsr.bank_6352.RF_CSR_DATA = value;
	}
	else
#endif /* RT6352 */
	{
		DBGPRINT_ERR(("%s():RF write with wrong handler!\n", __FUNCTION__));
		goto done;
	}
	RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);

	ret = NDIS_STATUS_SUCCESS;

done:

	return ret;
}


/*
	========================================================================
	
	Routine Description: Read RF register through MAC

	Arguments:

	Return Value:

	IRQL = 
	
	Note:
	
	========================================================================
*/
NDIS_STATUS rtmp_rf_read(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR bank,
	IN UCHAR regID,
	IN UCHAR *pValue)
{
	RF_CSR_CFG_STRUC rfcsr = { { 0 } };
	UINT i=0, k=0;
	BOOLEAN rf_status;
	NDIS_STATUS	 ret = STATUS_UNSUCCESSFUL;


#ifdef RTMP_MAC_PCI
	if ((pAd->bPCIclkOff == TRUE) || (pAd->LastMCUCmd == SLEEP_MCU_CMD))
	{
		DBGPRINT_ERR(("rtmp_rf_read. Not allow to read RF 0x%x : fail\n",  regID));	
		return STATUS_UNSUCCESSFUL;
	}
#endif /* RTMP_MAC_PCI */


	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));

	rfcsr.word = 0;
	
	for (i=0; i<MAX_BUSY_COUNT; i++)
	{
		if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			goto done;

		rf_status = rtmp_rf_csr_poll_idle(pAd, &rfcsr.word);
		if ( rf_status == BUSY)
			break;

		rfcsr.word = 0;
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
			rfcsr.bank_6352.RF_CSR_WR = 0;
			rfcsr.bank_6352.RF_CSR_KICK = 1;
			rfcsr.bank_6352.TESTCSR_RFACC_REGNUM = (regID | (bank << 6));
		}
		else
#endif /* RT6352 */
		{
			DBGPRINT_ERR(("RF[%d] read function for non-supported chip[0x%x]\n", regID, pAd->MACVersion));
			break;
		}
		
		RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);

		rf_status = rtmp_rf_csr_poll_idle(pAd, &rfcsr.word);
		if (rf_status == IDLE)
		{
#ifdef RT6352
			if (IS_RT6352(pAd) && ((rfcsr.bank_6352.TESTCSR_RFACC_REGNUM & 0x3F) == regID))
			{
				*pValue = (UCHAR)(rfcsr.bank_6352.RF_CSR_DATA);
				break;
			}
#endif /* RT6352 */
		}
	}

	if (rf_status == BUSY)
	{																	
		DBGPRINT_ERR(("RF read R%d=0x%X fail, i[%d], k[%d]\n", regID, rfcsr.word,i,k));
		goto done;
	}
	ret = STATUS_SUCCESS;

done:

	return ret;
}
#endif /* RT6352 */

/*
    ========================================================================
    Routine Description:
        Adjust frequency offset when do channel switching or frequency calabration.
        
    Arguments:
        pAd         		- Adapter pointer
        pRefFreqOffset	in: referenced Frequency offset   out: adjusted frequency offset
        
    Return Value:
        None
        
    ========================================================================
*/
BOOLEAN RTMPAdjustFrequencyOffset(RTMP_ADAPTER *pAd,UCHAR *pRefFreqOffset)
{
	BOOLEAN RetVal = TRUE;
	UCHAR RFValue = 0; 
	UCHAR PreRFValue = 0; 
	UCHAR FreqOffset = 0;
	UCHAR HighCurrentBit = 0;
	
	RTMP_ReadRF(pAd, RF_R17, &FreqOffset, &HighCurrentBit, 0x7F);
	PreRFValue =  HighCurrentBit | FreqOffset;
	FreqOffset = min((*pRefFreqOffset & 0x7F), 0x5F);
	RFValue = HighCurrentBit | FreqOffset;
	if (PreRFValue != RFValue)
	{
		RetVal = (RT30xxWriteRFRegister(pAd, RF_R17, RFValue) == STATUS_SUCCESS ? TRUE:FALSE);
	}

	if (RetVal == FALSE)
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): Error in tuning frequency offset !!\n", __FUNCTION__));
	else
		*pRefFreqOffset = FreqOffset;

	return RetVal;

}

#endif /* RTMP_RF_RW_SUPPORT */

