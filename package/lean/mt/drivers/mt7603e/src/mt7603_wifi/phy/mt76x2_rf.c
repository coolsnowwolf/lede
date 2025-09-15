/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt76x2_rf.c
*/

#include "rt_config.h"

INT32 MT76x2ShowPartialRF(RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End)
{
	UINT32 RFIdx, Offset, Value;

	for (RFIdx = 0; RFIdx < pAd->Antenna.field.TxPath; RFIdx++)
	{
		for (Offset = Start; Offset <= End; Offset = Offset + 4)
		{
			mt76x2_rf_read(pAd, RFIdx, Offset, &Value);
			DBGPRINT(RT_DEBUG_TRACE, ("%s():%d 0x%04x 0x%08x\n", __FUNCTION__, RFIdx, Offset, Value));
			Value = 0;
		}
	}
}


INT mt76x2_rf_write(RTMP_ADAPTER *pAd, UINT8 rf_idx, UINT16 offset, UINT32 data)
{
	UINT32 i = 0;
	UINT32 value;
	int ret = 0;


	/* rf data */
	RTMP_IO_WRITE32(pAd, W_RFDATA, data);

	/* rf control */	
	RTMP_IO_READ32(pAd, RF_CTRL, &value);
	
	/* rf address */
	value &= ~RF_ADDR_MASK;
	value |= RF_ADDR(offset);
	
	/* write control */
	value |= RF_R_W_CTRL;

	/* rf index */
	value &= ~RF_IDX_MASK;

	value |= RF_IDX(rf_idx);
			
	RTMP_IO_WRITE32(pAd, RF_CTRL, value);

	do {
		RTMP_IO_READ32(pAd, RF_CTRL, &value);

		if (RF_READY(value))
			break;
		i++;
		RtmpOsMsDelay(1);
	} while ((i < MAX_BUSY_COUNT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
		
	if ((i == MAX_BUSY_COUNT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) {
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
		ret = STATUS_UNSUCCESSFUL;
	}


	return ret;
}


INT mt76x2_rf_read(RTMP_ADAPTER *pAd, UINT8 rf_idx, UINT16 offset, UINT32 *data)
{
	UINT32 i = 0;
	UINT32 value;
	int ret;


	/* rf control */	
	RTMP_IO_READ32(pAd, RF_CTRL, &value);
	
	/* rf address */
	value &= ~RF_ADDR_MASK;
	value |= RF_ADDR(offset);
	
	/* read control */
	value &= ~RF_R_W_CTRL;

	/* rf index */
	value &= ~RF_IDX_MASK;
	value |= RF_IDX(rf_idx);

	RTMP_IO_WRITE32(pAd, RF_CTRL, value);
	
	do {/* wait for ready */
	
		RTMP_IO_READ32(pAd, RF_CTRL, &value);

		if (RF_READY(value))
			break;
		i++;
		RtmpOsMsDelay(1);
	} while ((i < MAX_BUSY_COUNT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
	
	if ((i == MAX_BUSY_COUNT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) {
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
		ret = STATUS_UNSUCCESSFUL;
		goto done;
	}

	/* rf data */
	RTMP_IO_READ32(pAd, R_RFDATA, data);

done:

	return ret;
}

