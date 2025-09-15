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

        Abstract:

        Revision History:
        Who             When                    What
        --------        ----------              ----------------------------------------------
*/


#ifdef RLT_RF

#include "rt_config.h"


static inline BOOLEAN rf_csr_poll_idle(RTMP_ADAPTER *pAd, UINT32 *rfcsr)
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


NDIS_STATUS rlt_rf_write(
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
		DBGPRINT_ERR(("rlt_rf_write. Not allow to write RF 0x%x : fail\n",  regID));	
		return STATUS_UNSUCCESSFUL;
	}
#endif /* RTMP_MAC_PCI */

#ifdef RLT_MAC
	// TODO: shiang-usw, why we need to check this for MT7601?? Get these code from MT7601!
	if (pAd->chipCap.hif_type == HIF_RLT) {
		if (pAd->WlanFunCtrl.field.WLAN_EN == 0)
		{
			DBGPRINT_ERR(("rlt_rf_write. Not allow to write RF 0x%x : fail\n",  regID));	
			return STATUS_UNSUCCESSFUL;
		}
	}
#endif /* RLT_MAC */


	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));

	ret = STATUS_UNSUCCESSFUL;
	if (rf_csr_poll_idle(pAd, &rfcsr.word) != IDLE)
		goto done;

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
NDIS_STATUS rlt_rf_read(
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
		DBGPRINT_ERR(("%s():Not allow to read RF 0x%x : fail\n",  __FUNCTION__, regID));
		return STATUS_UNSUCCESSFUL;
	}
#endif /* RTMP_MAC_PCI */


	ASSERT((regID <= pAd->chipCap.MaxNumOfRfId));

	rfcsr.word = 0;
	
	for (i=0; i<MAX_BUSY_COUNT; i++)
	{
		if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			goto done;

		rf_status = rf_csr_poll_idle(pAd, &rfcsr.word);
		if ( rf_status == BUSY)
			break;

		rfcsr.word = 0;
		{
			DBGPRINT_ERR(("RF[%d] read function for non-supported chip[0x%x]\n", regID, pAd->MACVersion));
			break;
		}
		
		RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);

		rf_status = rf_csr_poll_idle(pAd, &rfcsr.word);
		if (rf_status == IDLE)
		{
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

#endif /* RLT_RF */

