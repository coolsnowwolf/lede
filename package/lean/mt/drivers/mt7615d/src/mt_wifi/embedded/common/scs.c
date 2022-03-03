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
	scs.c
*/
#ifdef SMART_CARRIER_SENSE_SUPPORT
#include "rt_config.h"
#include "scs.h"
/*
***************************************************************************
*SmartCarrierSense_Gen3
*Base on MT7622 HW Design
***************************************************************************
*/
VOID SmartCarrierSense_Gen3(RTMP_ADAPTER *pAd)
{
	PSMART_CARRIER_SENSE_CTRL    pSCSCtrl;
	UCHAR	BandIndex;
	UCHAR	BssIndex;
	UINT32 MaxRtsRtyCount = 0;
	UINT32 MaxRtsCount = 0;
	UINT32	CrValue = 0;
	UINT32 TempValue = 0;
	UINT32	PdCount = 0;
	UINT32	MdrdyCount = 0;
	INT32	CckPdBlkBundry = 0;
	INT32	OfdmPdBlkBundry = 0;
	BOOL	Main_Tx = TRUE;
	BOOL	WriteCCKCr = FALSE;
	BOOL	WriteOFDMCr = FALSE;
	BOOL	WriteDefault = FALSE;

	pSCSCtrl = &pAd->SCSCtrl;
	for (BandIndex = 0; BandIndex < BandNum; BandIndex++) { /* Currently only supported Band0 */
		if (pSCSCtrl->SCSEnable[BandIndex] == SCS_ENABLE) {
			for (BssIndex = 0; BssIndex < BssNum; BssIndex++) {
				HW_IO_READ32(pAd, MIB_MB0SDR0 + (BssIndex * BssOffset) + (BandIndex * BandOffset), &CrValue);
			TempValue = (CrValue >> RtsRtyCountOffset) & RtsCountMask;
			if (TempValue > MaxRtsRtyCount) {
				MaxRtsRtyCount = TempValue;
				MaxRtsCount = CrValue & RtsCountMask;
			}
		}
			pSCSCtrl->RtsCount[BandIndex] = MaxRtsCount;
			pSCSCtrl->RtsRtyCount[BandIndex] = MaxRtsRtyCount;
			PdCount = pAd->MsMibBucket.PdCount[BandIndex][pAd->MsMibBucket.CurIdx];
			MdrdyCount = pAd->MsMibBucket.MdrdyCount[BandIndex][pAd->MsMibBucket.CurIdx];
			pSCSCtrl->CckFalseCcaCount[BandIndex] = (PdCount & 0xffff) - (MdrdyCount & 0xffff);
			pSCSCtrl->OfdmFalseCcaCount[BandIndex] = ((PdCount & 0xffff0000) >> 16) - ((MdrdyCount & 0xffff0000) >> 16);
			if (pSCSCtrl->OneSecTxByteCount[BandIndex] * TxTrafficTh <  pSCSCtrl->OneSecRxByteCount[BandIndex] || pSCSCtrl->OneSecTxByteCount[BandIndex] == 0)
				Main_Tx = FALSE;
			/*Update Drop Count for Develope SCS Gen3 , Only Support Band0  */
			HW_IO_READ32(pAd, MIB_MPDU_SR0, &CrValue);
			TempValue = (CrValue >> RtsDropCountOffset) & RTSDropCountMask;
			pAd->SCSCtrl.RTS_MPDU_DROP_CNT = TempValue;
			pAd->SCSCtrl.Retry_MPDU_DROP_CNT = CrValue & RTSDropCountMask;
			HW_IO_READ32(pAd, MIB_MPDU_SR1, &CrValue);
			pAd->SCSCtrl.LTO_MPDU_DROP_CNT = CrValue & RTSDropCountMask;
			if (Main_Tx == TRUE && (pSCSCtrl->RtsCount[BandIndex] > 0 || pSCSCtrl->RtsRtyCount[BandIndex] > 0)) {
				/* >UpBound  RTS PER < 40% Decrease coverage */
				if (pSCSCtrl->CckFalseCcaCount[BandIndex] > pSCSCtrl->CckFalseCcaUpBond[BandIndex] && MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {
					if (pAd->SCSCtrl.CckPdBlkTh[BandIndex] == PdBlkCckThDefault) {
						pAd->SCSCtrl.CckPdBlkTh[BandIndex] = FastInitTh;
						WriteCCKCr = TRUE;
					} else {
						pSCSCtrl->CckPdBlkTh[BandIndex] += OneStep;
						WriteCCKCr = TRUE;
						}
				/* <LowBond or RTS PER >60% Increase coverage */
				} else if (pSCSCtrl->CckFalseCcaCount[BandIndex] < pSCSCtrl->CckFalseCcaLowBond[BandIndex] || MaxRtsRtyCount > (MaxRtsCount + (MaxRtsCount >> 1))) {
						if (pSCSCtrl->CckPdBlkTh[BandIndex] - OneStep >= PdBlkCckThDefault) {
							pSCSCtrl->CckPdBlkTh[BandIndex] -= OneStep;
							WriteCCKCr = TRUE;
						} else {
							WriteCCKCr = FALSE;
						}
				} else {
					WriteCCKCr = FALSE;
					}
				/* Tracking the Farthest STA(Min RSSI)*/
				CckPdBlkBundry = min(((pSCSCtrl->SCSMinRssi[BandIndex] - pSCSCtrl->SCSMinRssiTolerance[BandIndex]) + 256), pSCSCtrl->CckFixedRssiBond[BandIndex]);
				if (pSCSCtrl->CckPdBlkTh[BandIndex] > CckPdBlkBundry) {
					pSCSCtrl->CckPdBlkTh[BandIndex] = CckPdBlkBundry;
					WriteCCKCr = TRUE;
					}
				if (WriteCCKCr) {
					HW_IO_READ32(pAd, PHY_RXTD_CCKPD_7, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset);
					CrValue |= (pSCSCtrl->CckPdBlkTh[BandIndex]  << PdBlkCckThOffset);
					HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_7, CrValue);
					HW_IO_READ32(pAd, PHY_RXTD_CCKPD_8, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset);
					CrValue |= (pSCSCtrl->CckPdBlkTh[BandIndex]  << PdBlkCck1RThOffset);
					HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_8, CrValue);
				}
				/* >UpBound  RTS PER < 40% Decrease coverage */
				if ((pSCSCtrl->OfdmFalseCcaCount[BandIndex] > pSCSCtrl->OfdmFalseCcaUpBond[BandIndex]) && MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {
					if (pAd->SCSCtrl.OfdmPdBlkTh[BandIndex] == PdBlkOfmdThDefault) {
						pAd->SCSCtrl.OfdmPdBlkTh[BandIndex] = FastInitThOfdm;
						WriteOFDMCr = TRUE;
					} else {
						pSCSCtrl->OfdmPdBlkTh[BandIndex] += OneStep;
						WriteOFDMCr = TRUE;
						}
				/* <LowBond or RTS PER >60% Increase coverage */
				} else if (pSCSCtrl->OfdmFalseCcaCount[BandIndex] < pSCSCtrl->OfdmFalseCcaLowBond[BandIndex] || MaxRtsRtyCount > (MaxRtsCount + (MaxRtsCount >> 1))) {
						if (pSCSCtrl->OfdmPdBlkTh[BandIndex] - OneStep >= PdBlkOfmdThDefault) {
							pSCSCtrl->OfdmPdBlkTh[BandIndex] -= OneStep;
							WriteOFDMCr = TRUE;
						} else {
							WriteOFDMCr = FALSE;
						}
				} else {
					WriteOFDMCr = FALSE;
					}
				/* Tracking the Farthest STA(Min RSSI)*/
				OfdmPdBlkBundry = min(((pSCSCtrl->SCSMinRssi[BandIndex] - pSCSCtrl->SCSMinRssiTolerance[BandIndex]) * 2 + 512),  pSCSCtrl->OfdmFixedRssiBond[BandIndex]);
				if (pSCSCtrl->OfdmPdBlkTh[BandIndex] > OfdmPdBlkBundry) {
					pSCSCtrl->OfdmPdBlkTh[BandIndex] = OfdmPdBlkBundry;
					WriteOFDMCr = TRUE;
				}
				if (WriteOFDMCr) {
					if (BandIndex == 0) {
						HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
						CrValue |= (pSCSCtrl->OfdmPdBlkTh[BandIndex] << PdBlkOfmdThOffset);
						HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);
					} else if (BandIndex == 1) {
						HW_IO_READ32(pAd, BAND1_PHY_MIN_PRI_PWR, &CrValue);
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
						CrValue |= (pSCSCtrl->OfdmPdBlkTh[BandIndex] << PdBlkOfmdThOffsetB1);
						HW_IO_WRITE32(pAd, BAND1_PHY_MIN_PRI_PWR, CrValue);
					}
				}
			} else {
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Disable SCS due to RtsCount=%d RtsRtyCount=%d Main_Tx=%d\n", pSCSCtrl->RtsCount[BandIndex], pSCSCtrl->RtsRtyCount[BandIndex], Main_Tx));
					WriteDefault = TRUE;
				}
		} else {
			WriteDefault = TRUE;
		}
		if (WriteDefault) {
			if (pSCSCtrl->CckPdBlkTh[BandIndex] != PdBlkCckThDefault) {
				pSCSCtrl->CckPdBlkTh[BandIndex] = PdBlkCckThDefault;
					HW_IO_READ32(pAd, PHY_RXTD_CCKPD_7, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset);
					CrValue |= (PdBlkCckThDefault << PdBlkCckThOffset);
					HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_7, CrValue);
					HW_IO_READ32(pAd, PHY_RXTD_CCKPD_8, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset);
					CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset);
					HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_8, CrValue);
				}
			if (pSCSCtrl->OfdmPdBlkTh[BandIndex] != PdBlkOfmdThDefault) {
				if (BandIndex == 0) {
					pSCSCtrl->OfdmPdBlkTh[BandIndex] = PdBlkOfmdThDefault;
						HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
						CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffset);
						HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);
			}
				if (BandIndex == 1) {
					pSCSCtrl->OfdmPdBlkTh[BandIndex] = PdBlkOfmdThDefault;
					HW_IO_READ32(pAd, BAND1_PHY_MIN_PRI_PWR, &CrValue);
					CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
					CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffsetB1);
					HW_IO_WRITE32(pAd, BAND1_PHY_MIN_PRI_PWR, CrValue);
				}
			}
		}
	}
}

/*
***************************************************************************
*SmartCarrierSense_Gen2
*Base on MT7615 HW Design
***************************************************************************
*/
VOID SmartCarrierSense_Gen2(RTMP_ADAPTER *pAd)
{
	PSMART_CARRIER_SENSE_CTRL    pSCSCtrl;
	BOOL	RxOnly = FALSE;
	UINT32 TotalTP = 0, CrValue = 0;
	INT32	 CckPdBlkBundry = 0, OfdmPdBlkBundry = 0;
	UCHAR i;
	UCHAR idx;
	UINT32 MaxRtsRtyCount = 0;
	UINT32 MaxRtsCount = 0;
	UINT32 TempValue = 0;
	BOOL WriteCr = FALSE;
	UINT32 PdCount = 0, MdrdyCount = 0;

	pSCSCtrl = &pAd->SCSCtrl;

	/* 2. Tx/Rx */
	/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, */
	/* ("%s Band0:Tx/Rx=%d/%d MinRSSI=%d, Band1:Tx/Rx=%d/%d, MinRSSI=%d\n", */
	/* __FUNCTION__, pAd->SCSCtrl.OneSecTxByteCount[0], pAd->SCSCtrl.OneSecRxByteCount[0], pAd->SCSCtrl.SCSMinRssi[0], */
	/* pAd->SCSCtrl.OneSecTxByteCount[1], pAd->SCSCtrl.OneSecRxByteCount[1], pAd->SCSCtrl.SCSMinRssi[1])); */

	/* 3. based on minRssi to adjust PD_BLOCK_TH */
	for (i = 0; i < 1; i++) { /* NO DBDC support. */
		for (idx = 0; idx < 4; idx++) {
			HW_IO_READ32(pAd, MIB_MB0SDR0 + (idx * BssOffset) + (i * BandOffset), &CrValue);
			TempValue = (CrValue >> RtsRtyCountOffset) & RtsCountMask;

			if (TempValue > MaxRtsRtyCount) {
				MaxRtsRtyCount = TempValue;
				MaxRtsCount = CrValue & RtsCountMask;
			}
		}

		pSCSCtrl->RtsCount[i] = MaxRtsCount;
		pSCSCtrl->RtsRtyCount[i] = MaxRtsRtyCount;
		PdCount = pAd->MsMibBucket.PdCount[i][pAd->MsMibBucket.CurIdx];
		MdrdyCount = pAd->MsMibBucket.MdrdyCount[i][pAd->MsMibBucket.CurIdx];
		/* printk("PD_count=%x, MDRSY_count=%x\n", CrValue, CrValue2); */
		pSCSCtrl->CckFalseCcaCount[i] = (PdCount & 0xffff) - (MdrdyCount & 0xffff);
		pSCSCtrl->OfdmFalseCcaCount[i] = ((PdCount & 0xffff0000) >> 16) - ((MdrdyCount & 0xffff0000) >> 16);

		if (pSCSCtrl->SCSEnable[i] == SCS_ENABLE) {
			TotalTP = (pSCSCtrl->OneSecTxByteCount[i] + pSCSCtrl->OneSecRxByteCount[i]);

			if ((pSCSCtrl->OneSecTxByteCount[i]) * 9 <  pSCSCtrl->OneSecRxByteCount[i])
				RxOnly = TRUE;

			/* if (1 TotalTP > pSCSCtrl->SCSTrafficThreshold[i]) {*/ /* default 2M */
			if ((pSCSCtrl->RtsCount[i] > 0 || pSCSCtrl->RtsRtyCount[i] > 0) && RxOnly == FALSE) {
				/* Set PD_BLOCKING_BOUNDARY */
				CckPdBlkBundry = min(((pSCSCtrl->SCSMinRssi[i] - pSCSCtrl->SCSMinRssiTolerance[i]) + 256), pSCSCtrl->CckFixedRssiBond[i]);

				/* CCK part */
				if ((pSCSCtrl->CckFalseCcaCount[i] > pSCSCtrl->CckFalseCcaUpBond[i])) { /* Decrease coverage */
					if (MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {/* RTS PER < 40% */
						if (pAd->SCSCtrl.CckPdBlkTh[i] == PdBlkCckThDefault && CckPdBlkBundry > FastInitTh) {
							pAd->SCSCtrl.CckPdBlkTh[i] = FastInitTh;
							WriteCr = TRUE;
						}
						/* pSCSCtrl->CckPdBlkTh[i] += 2; //One step is 2dB. */
						else if ((pSCSCtrl->CckPdBlkTh[i] + OneStep) <= CckPdBlkBundry) {
							pSCSCtrl->CckPdBlkTh[i] += OneStep;
							/* Write to CR */
							WriteCr = TRUE;
						} else  if (pSCSCtrl->CckPdBlkTh[i] > CckPdBlkBundry) {
							pSCSCtrl->CckPdBlkTh[i] = CckPdBlkBundry;
							/* Write to CR */
							WriteCr = TRUE;
						}
					}
				} else if (pSCSCtrl->CckFalseCcaCount[i] < pSCSCtrl->CckFalseCcaLowBond[i] || (MaxRtsCount + (MaxRtsCount >> 1)) < MaxRtsRtyCount) { /* Increase coverage */
					if (pSCSCtrl->CckPdBlkTh[i] - OneStep >= PdBlkCckThDefault) {
						pSCSCtrl->CckPdBlkTh[i] -= OneStep;

						if (pSCSCtrl->CckPdBlkTh[i] > CckPdBlkBundry) /* Tracking mini RSSI to prevent out of service rage. */
							pSCSCtrl->CckPdBlkTh[i] = CckPdBlkBundry;

						/* Write to CR */
						WriteCr = TRUE;
					}
				} else { /* Stable stat */
					if (pSCSCtrl->CckPdBlkTh[i] > CckPdBlkBundry) {/* Tracking mini RSSI to prevent out of service rage. */
						pSCSCtrl->CckPdBlkTh[i] = CckPdBlkBundry;
						WriteCr = TRUE;
					}
				}

				if (WriteCr) { /* Write for CCK PD blocking */
					HW_IO_READ32(pAd, PHY_RXTD_CCKPD_7, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset); /* Bit[8:1] */
					CrValue |= (pSCSCtrl->CckPdBlkTh[i]  << PdBlkCckThOffset);
					HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_7, CrValue);
					HW_IO_READ32(pAd, PHY_RXTD_CCKPD_8, &CrValue);
					CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset); /* Bit[31:24] */
					CrValue |= (pSCSCtrl->CckPdBlkTh[i]  << PdBlkCck1RThOffset);
					HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_8, CrValue);
				}

				WriteCr = FALSE; /* Clear */
				/* OFDM part */
				/* Set PD_BLOCKING_BOUNDARY */
				OfdmPdBlkBundry = min(((pSCSCtrl->SCSMinRssi[i] - pSCSCtrl->SCSMinRssiTolerance[i]) * 2 + 512),  pSCSCtrl->OfdmFixedRssiBond[i]);

				if (pSCSCtrl->OfdmFalseCcaCount[i] > pSCSCtrl->OfdmFalseCcaUpBond[i]) { /* Decrease coverage */
					if (MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {/* RTS PER < 40% */
						if (pAd->SCSCtrl.OfdmPdBlkTh[i] == PdBlkOfmdThDefault && OfdmPdBlkBundry > FastInitThOfdm) {
							pAd->SCSCtrl.OfdmPdBlkTh[i] = FastInitThOfdm;
							WriteCr = TRUE;
						}

						if ((pSCSCtrl->OfdmPdBlkTh[i] + OneStep) <= OfdmPdBlkBundry) {
							pSCSCtrl->OfdmPdBlkTh[i] += OneStep;
							/* Write to CR */
							WriteCr = TRUE;
						} else  if (pSCSCtrl->OfdmPdBlkTh[i] > OfdmPdBlkBundry) {
							pSCSCtrl->OfdmPdBlkTh[i] = OfdmPdBlkBundry;
							/* Write to CR */
							WriteCr = TRUE;
						}
					}
				} else if (pSCSCtrl->OfdmFalseCcaCount[i] < pSCSCtrl->OfdmFalseCcaLowBond[i] || (MaxRtsCount + (MaxRtsCount >> 1)) < MaxRtsRtyCount) {
					/* Increase coverage */
					if (pSCSCtrl->OfdmPdBlkTh[i] - OneStep >= PdBlkOfmdThDefault) {
						pSCSCtrl->OfdmPdBlkTh[i] -= OneStep;

						if (pSCSCtrl->OfdmPdBlkTh[i] > OfdmPdBlkBundry)/* Tracking mini RSSI to prevent out of service rage. */
							pSCSCtrl->OfdmPdBlkTh[i] = OfdmPdBlkBundry;

						/* Write to CR */
						WriteCr = TRUE;
					}
				} else { /* Stable stat */
					if (pSCSCtrl->OfdmPdBlkTh[i] > OfdmPdBlkBundry) {/* Tracking mini RSSI to prevent out of service rage. */
						pSCSCtrl->OfdmPdBlkTh[i] = OfdmPdBlkBundry;
						WriteCr = TRUE;
					}
				}

				if (WriteCr) {/* Write for OFDM PD blocking */
					if (i == 0) {
						HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);
						/* OFDM PD BLOCKING TH */
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
						CrValue |= (pSCSCtrl->OfdmPdBlkTh[i] << PdBlkOfmdThOffset);
						HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);
					} else if (i == 1) { /* DBDC */
						HW_IO_READ32(pAd, BAND1_PHY_MIN_PRI_PWR, &CrValue);
						/* OFDM PD BLOCKING TH */
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
						CrValue |= (pSCSCtrl->OfdmPdBlkTh[i] << PdBlkOfmdThOffsetB1);
						HW_IO_WRITE32(pAd, BAND1_PHY_MIN_PRI_PWR, CrValue);
					}
				}
			} else { /* Disable SCS  No traffic */
				if (pSCSCtrl->CckPdBlkTh[i] != PdBlkCckThDefault) {
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Disable SCS due to RtsCount=%d RxOnly=%d\n", pSCSCtrl->RtsCount[i], RxOnly));
					pSCSCtrl->CckPdBlkTh[i] = PdBlkCckThDefault;
					HW_IO_READ32(pAd, PHY_RXTD_CCKPD_7, &CrValue);

					CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset);
					/* 0x92 is default value -110dBm */
					CrValue |= (PdBlkCckThDefault << PdBlkCckThOffset);
					HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_7, CrValue);
					HW_IO_READ32(pAd, PHY_RXTD_CCKPD_8, &CrValue);
					/* Bit[31:24] */
					CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset);
					/* 0x92 is default value -110dBm */
					CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset);
					HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_8, CrValue);
				}

				if (pSCSCtrl->OfdmPdBlkTh[i] != PdBlkOfmdThDefault) {
					if (i == 0) {
						pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
						HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);
						/* OFDM PD BLOCKING TH */
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
						CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffset);
						HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);
					} else if (i == 1) {
						pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
						HW_IO_READ32(pAd, BAND1_PHY_MIN_PRI_PWR, &CrValue);
						/* OFDM PD BLOCKING TH */
						CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
						CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffsetB1);
						HW_IO_WRITE32(pAd, BAND1_PHY_MIN_PRI_PWR, CrValue);
					}
				}
			}
		} else {
			if (pSCSCtrl->CckPdBlkTh[i] != PdBlkCckThDefault) {
				pSCSCtrl->CckPdBlkTh[i] = PdBlkCckThDefault;
				HW_IO_READ32(pAd, PHY_RXTD_CCKPD_7, &CrValue)
				/* Bit[8:1] */;
				CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset);
				CrValue |= (PdBlkCckThDefault << PdBlkCckThOffset);
				 /* 0x92 is default value -110dBm */
				HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_7, CrValue);
				HW_IO_READ32(pAd, PHY_RXTD_CCKPD_8, &CrValue);
				/* Bit[31:24] */
				CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset);
				/* 0x92 is default value -110dBm */
				CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset);
				HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_8, CrValue);
			}

			if (pSCSCtrl->OfdmPdBlkTh[i] != PdBlkOfmdThDefault) {
				if (i == 0) {
					pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
					HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);
					/* OFDM PD BLOCKING TH */
					CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);
					CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffset);
					HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);
				} else if (i == 1) {
					pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
					HW_IO_READ32(pAd, BAND1_PHY_MIN_PRI_PWR, &CrValue);
					CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);
					 /* OFDM PD BLOCKING TH */
					CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffsetB1);
					HW_IO_WRITE32(pAd, BAND1_PHY_MIN_PRI_PWR, CrValue);
				}
			}
		}
	}
}

VOID SetSCS(RTMP_ADAPTER *pAd, UCHAR BandIdx, UINT32 value)
{
	UINT32 CrValue;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): BandIdx=%d, SCSEnable=%d\n", __func__, BandIdx, value));

	if (value > 500) /* traffic threshold.*/
		pAd->SCSCtrl.SCSTrafficThreshold[BandIdx] = value;
	else if (value == SCS_DISABLE) {
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_DISABLE;
		pAd->SCSCtrl.SCSStatus[BandIdx] = PD_BLOCKING_OFF;
		/* Disable PD blocking and reset related CR */
		HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);
		/* CrValue &= ~(0x1 << PdBlkEnabeOffset);  Bit[19] */
		CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);  /* OFDM PD BLOCKING TH */
		CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffset);
		HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);
		HW_IO_READ32(pAd, PHY_RXTD_CCKPD_7, &CrValue);
		CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset); /* Bit[8:1] */
		CrValue |= (PdBlkCckThDefault << PdBlkCckThOffset); /* 0x92 is default value -110dBm */
		HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_7, CrValue);
		HW_IO_READ32(pAd, PHY_RXTD_CCKPD_8, &CrValue);
		CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset); /* Bit[31:24] */
		CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset); /* 0x92 is default value -110dBm */
		HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_8, CrValue);
	} else if (value == SCS_ENABLE)
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_ENABLE;
}

VOID SCS_init(RTMP_ADAPTER *pAd)
{
	UINT32 CrValue;
		/* Enable Band0 PD_BLOCKING */
		HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);
		CrValue |= (0x1 << PdBlkEnabeOffset);/* Bit[19] */
		HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);
		/* Enable Band1 PD_BLOCKING & initail PD_BLOCKING_TH */
		HW_IO_READ32(pAd, BAND1_PHY_MIN_PRI_PWR, &CrValue);
		CrValue |= (0x1 << PdBlkEnabeOffsetB1);/* Bit[25] */
		CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);/* OFDM PD BLOCKING TH */
		CrValue |= (PdBlkOfmdThDefault << PdBlkOfmdThOffsetB1);
		HW_IO_WRITE32(pAd, BAND1_PHY_MIN_PRI_PWR, CrValue);
		if (pAd->SCSCtrl.SCSGeneration == SCS_Gen3) {
			/* Enable Band0 RTS Drop Count */
			HW_IO_READ32(pAd, M0_MISC_CR, &CrValue);
			CrValue |= (0x7 << RTSDropRdClrEnabeOffset);/* Bit[10:8] */
			CrValue |= (0x7 << RTSDropCntEnabeOffset);/* Bit[2:0] */
			HW_IO_WRITE32(pAd, M0_MISC_CR, CrValue);
		}
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */
