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
	cmm_txbf_cal_mt.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Cmm_txbf_cal_mt.h"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

#ifdef TXBF_SUPPORT

UINT_16 au2IBfCalEEPROMOffset[9] = {
	IBF_LNA_PHASE_G0_ADDR,
	IBF_LNA_PHASE_G1_ADDR,
	IBF_LNA_PHASE_G2_ADDR,
	IBF_LNA_PHASE_G3_ADDR,
	IBF_LNA_PHASE_G4_ADDR,
	IBF_LNA_PHASE_G5_ADDR,
	IBF_LNA_PHASE_G6_ADDR,
	IBF_LNA_PHASE_G7_ADDR,
	IBF_LNA_PHASE_G8_ADDR
};


VOID E2pMemWrite(IN PRTMP_ADAPTER pAd,
		 IN UINT16 u2MemAddr,
		 IN UCHAR  ucInput_L,
		 IN UCHAR  ucInput_H)
{
	UINT16 u2Value;

	if ((u2MemAddr & 0x1) != 0)
		u2Value = (ucInput_L << 8) | ucInput_H;
	else
		u2Value = (ucInput_H << 8) | ucInput_L;

	RT28xx_EEPROM_WRITE16(pAd, u2MemAddr, u2Value);
}


#if defined(MT7615)
VOID mt7615_iBFPhaseComp(IN PRTMP_ADAPTER pAd, IN UCHAR ucGroup, IN PCHAR pCmdBuf)
{
	if (ucGroup == 0)
		os_move_mem(pCmdBuf, pAd->piBfPhaseG0, sizeof(MT7615_IBF_PHASE_G0_T));
	else
		os_move_mem(pCmdBuf, &pAd->piBfPhaseGx[(ucGroup - 1) * sizeof(MT7615_IBF_PHASE_Gx_T)], sizeof(MT7615_IBF_PHASE_Gx_T));

}
#endif /* MT7615 */


#if defined(MT7622)
VOID mt7622_iBFPhaseComp(IN PRTMP_ADAPTER pAd, IN UCHAR ucGroup, IN PCHAR pCmdBuf)
{
	os_move_mem(pCmdBuf, pAd->piBfPhaseG0, sizeof(MT7622_IBF_PHASE_G0_T));
}
#endif /* MT7622 */

#if defined(MT7615)
VOID mt7615_iBFPhaseFreeMem(IN PRTMP_ADAPTER pAd)
{
	/* Group 0 */
	if (pAd->piBfPhaseG0 != NULL) {
		os_free_mem(pAd->piBfPhaseG0);
	}

	/* Group 1 ~ 8 */
	if (pAd->piBfPhaseGx != NULL) {
		os_free_mem(pAd->piBfPhaseGx);
	}
}
#endif /* MT7615 */


#if defined(MT7622)
VOID mt7622_iBFPhaseFreeMem(IN PRTMP_ADAPTER pAd)
{
	/* Group 0 */
	if (pAd->piBfPhaseG0 != NULL) {
		os_free_mem(pAd->piBfPhaseG0);
	}
}
#endif /* MT7622 */


#if defined(MT7615)
VOID mt7615_iBFPhaseCalInit(IN PRTMP_ADAPTER pAd)
{
	/* Free memory allocated by iBF phase calibration */
	mt7615_iBFPhaseFreeMem(pAd);

	os_alloc_mem(pAd, (PUCHAR *)&pAd->piBfPhaseG0, sizeof(MT7615_IBF_PHASE_G0_T));
	os_alloc_mem(pAd, (PUCHAR *)&pAd->piBfPhaseGx, sizeof(MT7615_IBF_PHASE_Gx_T) * 8);

	NdisZeroMemory(pAd->piBfPhaseG0, sizeof(MT7615_IBF_PHASE_G0_T));
	NdisZeroMemory(pAd->piBfPhaseGx, sizeof(MT7615_IBF_PHASE_Gx_T) * 8);

	pAd->fgCalibrationFail = FALSE;
	NdisZeroMemory(&pAd->fgGroupIdPassFailStatus[0], 9);
}
#endif /* MT7615 */


#if defined(MT7622)
VOID mt7622_iBFPhaseCalInit(IN PRTMP_ADAPTER pAd)
{
	mt7622_iBFPhaseFreeMem(pAd);

	os_alloc_mem(pAd, (PUCHAR *)&pAd->piBfPhaseG0, sizeof(MT7622_IBF_PHASE_G0_T));

	NdisZeroMemory(pAd->piBfPhaseG0, sizeof(MT7622_IBF_PHASE_G0_T));

	pAd->fgCalibrationFail = FALSE;
	pAd->fgGroupIdPassFailStatus[0] = FALSE;
}
#endif /* MT7622 */


#if defined(MT7615)
VOID mt7615_iBFPhaseCalE2PInit(IN PRTMP_ADAPTER pAd)
{
	UINT_16 u2BfE2pOccupiedSize;


	/* Group 0 */
	u2BfE2pOccupiedSize = sizeof(MT7615_IBF_PHASE_G0_T);
	RT28xx_EEPROM_READ_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_0], u2BfE2pOccupiedSize, pAd->piBfPhaseG0);

	/* Group 1 ~ 7 */
	u2BfE2pOccupiedSize = (sizeof(MT7615_IBF_PHASE_Gx_T) * 7) - 4;
	RT28xx_EEPROM_READ_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_1], u2BfE2pOccupiedSize, pAd->piBfPhaseGx);

	/* Group 8 */
	u2BfE2pOccupiedSize = sizeof(MT7615_IBF_PHASE_Gx_T);
	RT28xx_EEPROM_READ_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_8], u2BfE2pOccupiedSize, (PUCHAR)&pAd->piBfPhaseGx[7 * sizeof(MT7615_IBF_PHASE_G0_T)]);

	pAd->fgCalibrationFail = FALSE;
	NdisZeroMemory(&pAd->fgGroupIdPassFailStatus[0], 9);
}
#endif /* MT7615 */


#if defined(MT7622)
VOID mt7622_iBFPhaseCalE2PInit(IN PRTMP_ADAPTER pAd)
{
	UINT_16 u2BfE2pOccupiedSize;


	/* Group 0 */
	u2BfE2pOccupiedSize = sizeof(MT7622_IBF_PHASE_G0_T);
	RT28xx_EEPROM_READ_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_0], u2BfE2pOccupiedSize, pAd->piBfPhaseG0);

	pAd->fgCalibrationFail = FALSE;
	pAd->fgGroupIdPassFailStatus[0] = FALSE;
}
#endif /* MT7622 */


#if defined(MT7615)
VOID mt7615_iBFPhaseCalE2PUpdate(IN PRTMP_ADAPTER pAd,
			  IN UCHAR   ucGroup,
			  IN BOOLEAN fgSX2,
			  IN UCHAR   ucUpdateAllTye)
{
	MT7615_IBF_PHASE_Gx_T iBfPhaseGx;
	MT7615_IBF_PHASE_G0_T iBfPhaseG0;
	MT7615_IBF_PHASE_Gx_T iBfPhaseGx8[8];
	UCHAR  ucGroupIdx, ucEndLoop;
	UCHAR  ucIBfGroupSize, ucCounter;
	UCHAR  ucBuf[64];


	/* UINT16 u2Value; */
	/* IF phase calibration is for BW20/40/80/160 */
	ucGroupIdx = 0;
	NdisZeroMemory(&iBfPhaseG0, sizeof(MT7615_IBF_PHASE_G0_T));
	NdisZeroMemory(&iBfPhaseGx, sizeof(MT7615_IBF_PHASE_Gx_T));

	switch (ucUpdateAllTye) {
	case IBF_PHASE_ONE_GROUP_UPDATE:
		ucGroupIdx = ucGroup - 1;
		ucEndLoop  = 0;

		if (ucGroup == GROUP_0) {
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[ucGroup], sizeof(MT7615_IBF_PHASE_G0_T), pAd->piBfPhaseG0);
			NdisCopyMemory((PUCHAR)&iBfPhaseG0, pAd->piBfPhaseG0, sizeof(MT7615_IBF_PHASE_G0_T));
		} else if (ucGroup == GROUP_2) {
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[ucGroup], (sizeof(MT7615_IBF_PHASE_Gx_T) - 4),
							&pAd->piBfPhaseGx[sizeof(MT7615_IBF_PHASE_Gx_T)]);
			NdisCopyMemory((PUCHAR)&iBfPhaseGx, &pAd->piBfPhaseGx[ucGroupIdx * sizeof(MT7615_IBF_PHASE_Gx_T)],
							(sizeof(MT7615_IBF_PHASE_Gx_T) - 4));
		} else {
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[ucGroup], sizeof(MT7615_IBF_PHASE_Gx_T),
							&pAd->piBfPhaseGx[ucGroupIdx * sizeof(MT7615_IBF_PHASE_Gx_T)]);
			NdisCopyMemory((PUCHAR)&iBfPhaseGx, &pAd->piBfPhaseGx[ucGroupIdx * sizeof(MT7615_IBF_PHASE_Gx_T)],
							sizeof(MT7615_IBF_PHASE_Gx_T));
		}

		if (ucGroup == GROUP_0) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_L\n G%d_L_T0_H = %d\n G%d_L_T1_H = %d\n G%d_L_T2_H = %d\n",
				ucGroup,
				ucGroup, iBfPhaseG0.ucG0_L_T0_H,
				ucGroup, iBfPhaseG0.ucG0_L_T1_H,
				ucGroup, iBfPhaseG0.ucG0_L_T2_H));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_M\n G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n G%d_M_T2_H = %d\n",
				ucGroup,
				ucGroup, iBfPhaseG0.ucG0_M_T0_H,
				ucGroup, iBfPhaseG0.ucG0_M_T1_H,
				ucGroup, iBfPhaseG0.ucG0_M_T2_H));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R0_H = %d\n G%d_R0_M = %d\n G%d_R0_L = %d\n G%d_R1_H = %d\n G%d_R1_M = %d\n G%d_R1_L = %d\n",
				ucGroup, iBfPhaseG0.ucG0_R0_H,
				ucGroup, iBfPhaseG0.ucG0_R0_M,
				ucGroup, iBfPhaseG0.ucG0_R0_L,
				ucGroup, iBfPhaseG0.ucG0_R1_H,
				ucGroup, iBfPhaseG0.ucG0_R1_M,
				ucGroup, iBfPhaseG0.ucG0_R1_L));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R2_H = %d\n G%d_R2_M = %d\n G%d_R2_L = %d\n G%d_R3_H = %d\n G%d_R3_M = %d\n G%d_R3_L = %d\n",
				ucGroup, iBfPhaseG0.ucG0_R2_H,
				ucGroup, iBfPhaseG0.ucG0_R2_M,
				ucGroup, iBfPhaseG0.ucG0_R2_L,
				ucGroup, iBfPhaseG0.ucG0_R3_H,
				ucGroup, iBfPhaseG0.ucG0_R3_M,
				ucGroup, iBfPhaseG0.ucG0_R3_L));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_H\n G%d_H_T0_H = %d\n G%d_H_T1_H = %d\n G%d_H_T2_H = %d\n",
				ucGroup,
				ucGroup, iBfPhaseG0.ucG0_H_T0_H,
				ucGroup, iBfPhaseG0.ucG0_H_T1_H,
				ucGroup, iBfPhaseG0.ucG0_H_T2_H));
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_L\n G%d_L_T0_H = %d\n G%d_L_T1_H = %d\n G%d_L_T2_H = %d\n",
				ucGroup,
				ucGroup, iBfPhaseGx.ucGx_L_T0_H,
				ucGroup, iBfPhaseGx.ucGx_L_T1_H,
				ucGroup, iBfPhaseGx.ucGx_L_T2_H));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_M\n G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n G%d_M_T2_H = %d\n",
				ucGroup,
				ucGroup, iBfPhaseGx.ucGx_M_T0_H,
				ucGroup, iBfPhaseGx.ucGx_M_T1_H,
				ucGroup, iBfPhaseGx.ucGx_M_T2_H));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R0_H = %d\n G%d_R0_M = %d\n G%d_R0_L = %d\n G%d_R1_H = %d\n G%d_R1_M = %d\n G%d_R1_L = %d\n",
				ucGroup, iBfPhaseGx.ucGx_R0_H,
				ucGroup, iBfPhaseGx.ucGx_R0_M,
				ucGroup, iBfPhaseGx.ucGx_R0_L,
				ucGroup, iBfPhaseGx.ucGx_R1_H,
				ucGroup, iBfPhaseGx.ucGx_R1_M,
				ucGroup, iBfPhaseGx.ucGx_R1_L));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R2_H = %d\n G%d_R2_M = %d\n G%d_R2_L = %d\n G%d_R3_H = %d\n G%d_R3_M = %d\n G%d_R3_L = %d\n",
				ucGroup, iBfPhaseGx.ucGx_R2_H,
				ucGroup, iBfPhaseGx.ucGx_R2_M,
				ucGroup, iBfPhaseGx.ucGx_R2_L,
				ucGroup, iBfPhaseGx.ucGx_R3_H,
				ucGroup, iBfPhaseGx.ucGx_R3_M,
				ucGroup, iBfPhaseGx.ucGx_R3_L));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_H\n G%d_H_T0_H = %d\n G%d_H_T1_H = %d\n G%d_H_T2_H = %d\n",
				ucGroup,
				ucGroup, iBfPhaseGx.ucGx_H_T0_H,
				ucGroup, iBfPhaseGx.ucGx_H_T1_H,
				ucGroup, iBfPhaseGx.ucGx_H_T2_H));
		}

		break;

	case IBF_PHASE_ALL_GROUP_UPDATE:
		if (pAd->fgCalibrationFail == FALSE) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("All of groups can pass criterion and calibrated phases can be written into EEPROM\n"));

			NdisCopyMemory(ucBuf, pAd->piBfPhaseG0, sizeof(MT7615_IBF_PHASE_G0_T));
			NdisCopyMemory(&ucBuf[sizeof(MT7615_IBF_PHASE_G0_T)], pAd->piBfPhaseGx, sizeof(MT7615_IBF_PHASE_Gx_T));

			/* Write Group 0 and 1 into EEPROM */
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_0], (sizeof(MT7615_IBF_PHASE_G0_T) + sizeof(MT7615_IBF_PHASE_Gx_T)), ucBuf);
			/* Write Group 2 into EEPROM */
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_2], (sizeof(MT7615_IBF_PHASE_Gx_T) - 4),
							&pAd->piBfPhaseGx[(GROUP_2 - 1) * sizeof(MT7615_IBF_PHASE_Gx_T)]);
			/* Write Group 3 ~ 7 into EEPROM */
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_3], (sizeof(MT7615_IBF_PHASE_Gx_T) * (GROUP_7 - GROUP_2)),
							&pAd->piBfPhaseGx[(GROUP_3 - 1) * sizeof(MT7615_IBF_PHASE_Gx_T)]);
			/* Write Group 8 into EEPROM */
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_8], sizeof(MT7615_IBF_PHASE_Gx_T),
							&pAd->piBfPhaseGx[(GROUP_8 - 1) * sizeof(MT7615_IBF_PHASE_Gx_T)]);
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated phases can't be written into EEPROM because some groups can't pass criterion!!!\n"));

			for (ucCounter = GROUP_0; ucCounter <= GROUP_8; ucCounter++) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group%d = %s\n",
						 ucCounter,
						 (pAd->fgGroupIdPassFailStatus[ucCounter] == TRUE) ? "FAIL" : "PASS"));
			}
		}

		break;

	case IBF_PHASE_ALL_GROUP_ERASE:
		if (pAd->piBfPhaseG0 != NULL) {
			NdisZeroMemory(pAd->piBfPhaseG0, sizeof(MT7615_IBF_PHASE_G0_T));
		}

		if (pAd->piBfPhaseGx != NULL) {
			NdisZeroMemory(pAd->piBfPhaseGx, sizeof(MT7615_IBF_PHASE_Gx_T) * 8);
		}

		NdisZeroMemory((PUCHAR)&iBfPhaseG0, sizeof(MT7615_IBF_PHASE_G0_T));
		NdisZeroMemory((PUCHAR)&iBfPhaseGx8[0], (sizeof(MT7615_IBF_PHASE_Gx_T) * 8));

		RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_0], sizeof(MT7615_IBF_PHASE_G0_T), (PUCHAR)&iBfPhaseG0);
		RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_1], (sizeof(MT7615_IBF_PHASE_Gx_T) * 7 - 4), (PUCHAR)&iBfPhaseGx8[0]);
		RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_8], sizeof(MT7615_IBF_PHASE_Gx_T), (PUCHAR)&iBfPhaseGx8[7]);
		break;

	case IBF_PHASE_ALL_GROUP_READ_FROM_E2P:
		mt7615_iBFPhaseCalE2PInit(pAd);
		break;

	default:
		ucGroupIdx = ucGroup - 1;
		ucEndLoop  = 0;
		ucIBfGroupSize = sizeof(MT7615_IBF_PHASE_Gx_T);
		ucIBfGroupSize = (ucGroup == GROUP_2) ? (ucIBfGroupSize - 4) : ucIBfGroupSize;

		NdisCopyMemory((PUCHAR)&iBfPhaseGx, &pAd->piBfPhaseGx[ucGroupIdx * sizeof(MT7615_IBF_PHASE_Gx_T)], ucIBfGroupSize);
		NdisCopyMemory((PUCHAR)&iBfPhaseG0, pAd->piBfPhaseG0, sizeof(MT7615_IBF_PHASE_G0_T));
		break;
	}
}
#endif /* MT7615 */


#if defined(MT7622)
VOID mt7622_iBFPhaseCalE2PUpdate(IN PRTMP_ADAPTER pAd,
			  IN UCHAR   ucGroup,
			  IN BOOLEAN fgSX2,
			  IN UCHAR   ucUpdateAllTye)
{
	MT7622_IBF_PHASE_Gx_T iBfPhaseGx;
	MT7622_IBF_PHASE_G0_T iBfPhaseG0;
	UCHAR  ucGroupIdx, ucEndLoop;
	/* UINT16 u2Value; */


	/* IF phase calibration is for BW20/40/80/160 */
	ucGroupIdx = 0;
	NdisZeroMemory(&iBfPhaseG0, sizeof(MT7622_IBF_PHASE_G0_T));
	NdisZeroMemory(&iBfPhaseGx, sizeof(MT7622_IBF_PHASE_Gx_T));

	switch (ucUpdateAllTye) {
	case IBF_PHASE_ONE_GROUP_UPDATE:
		ucGroupIdx = ucGroup - 1;
		ucEndLoop  = 0;

		if (ucGroup == GROUP_0) {
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[ucGroup], sizeof(MT7622_IBF_PHASE_G0_T), pAd->piBfPhaseG0);
			NdisCopyMemory(&iBfPhaseG0, pAd->piBfPhaseG0, sizeof(MT7622_IBF_PHASE_G0_T));

			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_H\n G%d_H_T0_UH = %d\n G%d_H_T1_UH = %d\n G%d_H_T2_UH = %d\n",
				ucGroup,
				ucGroup, iBfPhaseG0.ucG0_H_T0_UH,
				ucGroup, iBfPhaseG0.ucG0_H_T1_UH,
				ucGroup, iBfPhaseG0.ucG0_H_T2_UH));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_L\n G%d_L_T0_UH = %d\n G%d_L_T1_UH = %d\n G%d_L_T2_UH = %d\n",
				ucGroup,
				ucGroup, iBfPhaseG0.ucG0_L_T0_UH,
				ucGroup, iBfPhaseG0.ucG0_L_T1_UH,
				ucGroup, iBfPhaseG0.ucG0_L_T2_UH));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_M\n G%d_M_T0_UH = %d\n G%d_M_T1_UH = %d\n G%d_M_T2_UH = %d\n",
				ucGroup,
				ucGroup, iBfPhaseG0.ucG0_M_T0_UH,
				ucGroup, iBfPhaseG0.ucG0_M_T1_UH,
				ucGroup, iBfPhaseG0.ucG0_M_T2_UH));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R0_UH = %d\n G%d_R0_H = %d\n G%d_R0_M = %d\n G%d_R0_L = %d\n G%d_R1_UH = %d\n G%d_R1_H = %d\n G%d_R1_M = %d\n G%d_R1_L = %d\n",
				ucGroup, iBfPhaseG0.ucG0_R0_UH,
				ucGroup, iBfPhaseG0.ucG0_R0_H,
				ucGroup, iBfPhaseG0.ucG0_R0_M,
				ucGroup, iBfPhaseG0.ucG0_R0_L,
				ucGroup, iBfPhaseG0.ucG0_R1_UH,
				ucGroup, iBfPhaseG0.ucG0_R1_H,
				ucGroup, iBfPhaseG0.ucG0_R1_M,
				ucGroup, iBfPhaseG0.ucG0_R1_L));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R2_UH = %d\n G%d_R2_H = %d\n G%d_R2_M = %d\n G%d_R2_L = %d\n G%d_R3_UH = %d\n G%d_R3_H = %d\n G%d_R3_M = %d\n G%d_R3_L = %d\n",
				ucGroup, iBfPhaseG0.ucG0_R2_H,
				ucGroup, iBfPhaseG0.ucG0_R2_UH,
				ucGroup, iBfPhaseG0.ucG0_R2_M,
				ucGroup, iBfPhaseG0.ucG0_R2_L,
				ucGroup, iBfPhaseG0.ucG0_R3_UH,
				ucGroup, iBfPhaseG0.ucG0_R3_H,
				ucGroup, iBfPhaseG0.ucG0_R3_M,
				ucGroup, iBfPhaseG0.ucG0_R3_L));
		}

		break;

	case IBF_PHASE_ALL_GROUP_UPDATE:
		if (pAd->fgCalibrationFail == FALSE) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("All of groups can pass criterion and calibrated phases can be written into EEPROM\n"));

			/* Write Group 0 into EEPROM */
			RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_0], sizeof(MT7622_IBF_PHASE_G0_T), pAd->piBfPhaseG0);
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated phases can't be written into EEPROM because some groups can't pass criterion!!!\n"));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group%d = %s\n",
				0,
				(pAd->fgGroupIdPassFailStatus[0] == TRUE) ? "FAIL" : "PASS"));
		}

		break;

	case IBF_PHASE_ALL_GROUP_ERASE:
		NdisZeroMemory(pAd->piBfPhaseG0, sizeof(MT7622_IBF_PHASE_G0_T));

		RT28xx_EEPROM_WRITE_WITH_RANGE(pAd, au2IBfCalEEPROMOffset[GROUP_0], sizeof(MT7622_IBF_PHASE_G0_T), pAd->piBfPhaseG0);
		break;

	case IBF_PHASE_ALL_GROUP_READ_FROM_E2P:
		mt7622_iBFPhaseCalE2PInit(pAd);
		break;

	default:
		ucGroupIdx = ucGroup - 1;
		ucEndLoop  = 0;

		NdisCopyMemory(&iBfPhaseG0, pAd->piBfPhaseG0, sizeof(MT7622_IBF_PHASE_G0_T));
		break;
	}
}
#endif


#if defined(MT7615)
VOID mt7615_iBFPhaseCalReport(IN PRTMP_ADAPTER pAd,
		   IN UCHAR   ucGroupL_M_H,
		   IN UCHAR   ucGroup,
		   IN BOOLEAN fgSX2,
		   IN UCHAR   ucStatus,
		   IN UCHAR   ucPhaseCalType,
		   IN PUCHAR  pBuf)
{
	MT7615_IBF_PHASE_G0_T   iBfPhaseG0;
	MT7615_IBF_PHASE_Gx_T   iBfPhaseGx;
	P_MT7615_IBF_PHASE_G0_T piBfPhaseG0;
	P_MT7615_IBF_PHASE_Gx_T piBfPhaseGx;
	MT7615_IBF_PHASE_OUT    iBfPhaseOut;
	UCHAR  ucGroupIdx;
	PUCHAR pucIBfPhaseG;


	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s :: Calibrated iBF phases\n", __func__));
	pucIBfPhaseG = pBuf + sizeof(MT7615_IBF_PHASE_OUT);
	NdisCopyMemory(&iBfPhaseOut, pBuf, sizeof(MT7615_IBF_PHASE_OUT));

	switch (ucPhaseCalType) {
	case IBF_PHASE_CAL_NOTHING: /* Do nothing */
		break;

	case IBF_PHASE_CAL_NORMAL_INSTRUMENT:
	case IBF_PHASE_CAL_NORMAL: /* Store calibrated phases with buffer mode */
		/* IF phase calibration is for DBDC or 80+80 */
		if ((fgSX2 == TRUE) && (ucGroup > 0)) {
			ucGroupIdx = ucGroup - 1;
			NdisCopyMemory(&iBfPhaseGx, pucIBfPhaseG, sizeof(MT7615_IBF_PHASE_Gx_T));

			piBfPhaseGx = (P_MT7615_IBF_PHASE_Gx_T)&pAd->piBfPhaseGx[ucGroupIdx * sizeof(MT7615_IBF_PHASE_Gx_T)];

			switch (ucGroupL_M_H) {
			case GROUP_L:
				piBfPhaseGx->ucGx_L_T2_H_SX2 = iBfPhaseGx.ucGx_L_T2_H_SX2;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("160 iBF phase calibration\n Group_L\n G%d_L_T2_H_SX2 = %d\n ",
						 ucGroup, iBfPhaseGx.ucGx_L_T2_H_SX2));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n",
						 iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
						 iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
						 iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
						 iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
				break;

			case GROUP_M:
				piBfPhaseGx->ucGx_M_T2_H_SX2 = iBfPhaseGx.ucGx_M_T2_H_SX2;
				piBfPhaseGx->ucGx_R2_H_SX2   = iBfPhaseGx.ucGx_R2_H_SX2;
				piBfPhaseGx->ucGx_R2_M_SX2   = iBfPhaseGx.ucGx_R2_M_SX2;
				piBfPhaseGx->ucGx_R2_L_SX2   = iBfPhaseGx.ucGx_R2_L_SX2;
				piBfPhaseGx->ucGx_R3_H_SX2   = iBfPhaseGx.ucGx_R3_H_SX2;
				piBfPhaseGx->ucGx_R3_M_SX2   = iBfPhaseGx.ucGx_R3_M_SX2;
				piBfPhaseGx->ucGx_R3_L_SX2   = iBfPhaseGx.ucGx_R3_L_SX2;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group_M\n G%d_M_T2_H_SX2 = %d\n G%d_R2_H_SX2 = %d\n G%d_R2_M_SX2 = %d\n",
						 ucGroup, iBfPhaseGx.ucGx_H_T2_H_SX2,
						 ucGroup, iBfPhaseGx.ucGx_R2_H_SX2,
						 ucGroup, iBfPhaseGx.ucGx_R2_M_SX2));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R2_L_SX2 = %d\n G%d_R3_H_SX2 = %d\n G%d_R3_M_SX2 = %d\n G%d_R3_L_SX2 = %d\n",
						 ucGroup, iBfPhaseGx.ucGx_R2_L_SX2,
						 ucGroup, iBfPhaseGx.ucGx_R3_H_SX2,
						 ucGroup, iBfPhaseGx.ucGx_R3_M_SX2,
						 ucGroup, iBfPhaseGx.ucGx_R3_L_SX2));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n",
						 iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
						 iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
						 iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
						 iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
				break;

			case GROUP_H:
				piBfPhaseGx->ucGx_H_T2_H_SX2 = iBfPhaseGx.ucGx_H_T2_H_SX2;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group_H\n G%d_H_T2_H_SX2 = %d\n",
						 ucGroup, iBfPhaseGx.ucGx_H_T2_H_SX2));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n",
						 iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
						 iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
						 iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
						 iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
				break;
			}

			return;
		}

		/* IF phase calibration is for BW20/40/80 */
		if (ucGroup == GROUP_0) {
			NdisCopyMemory(&iBfPhaseG0, pucIBfPhaseG, sizeof(MT7615_IBF_PHASE_G0_T));
			NdisCopyMemory(&iBfPhaseGx, pucIBfPhaseG, sizeof(MT7615_IBF_PHASE_Gx_T));

			piBfPhaseG0 = (P_MT7615_IBF_PHASE_G0_T)pAd->piBfPhaseG0;

			switch (ucGroupL_M_H) {
			case GROUP_L:
				piBfPhaseG0->ucG0_L_T0_H = iBfPhaseGx.ucGx_L_T0_H;
				piBfPhaseG0->ucG0_L_T1_H = iBfPhaseGx.ucGx_L_T1_H;
				piBfPhaseG0->ucG0_L_T2_H = iBfPhaseGx.ucGx_L_T2_H;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0 and Group_L\n G0_L_T0_H = %d\n G0_L_T1_H = %d\n G0_L_T2_H = %d\n",
						 iBfPhaseGx.ucGx_L_T0_H,
						 iBfPhaseGx.ucGx_L_T1_H,
						 iBfPhaseGx.ucGx_L_T2_H));
				break;

			case GROUP_M:
				piBfPhaseG0->ucG0_M_T0_H = iBfPhaseGx.ucGx_M_T0_H;
				piBfPhaseG0->ucG0_M_T1_H = iBfPhaseGx.ucGx_M_T1_H;
				piBfPhaseG0->ucG0_M_T2_H = iBfPhaseGx.ucGx_M_T2_H;
				piBfPhaseG0->ucG0_R0_H   = iBfPhaseG0.ucG0_R0_H;
				piBfPhaseG0->ucG0_R0_M   = iBfPhaseG0.ucG0_R0_M;
				piBfPhaseG0->ucG0_R0_L   = iBfPhaseG0.ucG0_R0_L;
				piBfPhaseG0->ucG0_R1_H   = iBfPhaseG0.ucG0_R1_H;
				piBfPhaseG0->ucG0_R1_M   = iBfPhaseG0.ucG0_R1_M;
				piBfPhaseG0->ucG0_R1_L   = iBfPhaseG0.ucG0_R1_L;
				piBfPhaseG0->ucG0_R2_H   = iBfPhaseG0.ucG0_R2_H;
				piBfPhaseG0->ucG0_R2_M   = iBfPhaseG0.ucG0_R2_M;
				piBfPhaseG0->ucG0_R2_L   = iBfPhaseG0.ucG0_R2_L;
				piBfPhaseG0->ucG0_R3_H   = iBfPhaseG0.ucG0_R3_H;
				piBfPhaseG0->ucG0_R3_M   = iBfPhaseG0.ucG0_R3_M;
				piBfPhaseG0->ucG0_R3_L   = iBfPhaseG0.ucG0_R3_L;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0 and Group_M\n G0_M_T0_H = %d\n G0_M_T1_H = %d\n G0_M_T2_H = %d\n",
						 iBfPhaseGx.ucGx_M_T0_H,
						 iBfPhaseGx.ucGx_M_T1_H,
						 iBfPhaseGx.ucGx_M_T2_H));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0_R0_H = %d\n G0_R0_M = %d\n G0_R0_L = %d\n G0_R1_H = %d\n G0_R1_M = %d\n G0_R1_L = %d\n",
						 iBfPhaseG0.ucG0_R0_H,
						 iBfPhaseG0.ucG0_R0_M,
						 iBfPhaseG0.ucG0_R0_L,
						 iBfPhaseG0.ucG0_R1_H,
						 iBfPhaseG0.ucG0_R1_M,
						 iBfPhaseG0.ucG0_R1_L));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0_R2_H = %d\n G0_R2_M = %d\n G0_R2_L = %d\n G0_R3_H = %d\n G0_R3_M = %d\n G0_R3_L = %d\n",
						 iBfPhaseG0.ucG0_R2_H,
						 iBfPhaseG0.ucG0_R2_M,
						 iBfPhaseG0.ucG0_R2_L,
						 iBfPhaseG0.ucG0_R3_H,
						 iBfPhaseG0.ucG0_R3_M,
						 iBfPhaseG0.ucG0_R3_L));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", 0));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n",
						 iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
						 iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
						 iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
						 iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
				break;

			case GROUP_H:
				piBfPhaseG0->ucG0_H_T0_H = iBfPhaseGx.ucGx_H_T0_H;
				piBfPhaseG0->ucG0_H_T1_H = iBfPhaseGx.ucGx_H_T1_H;
				piBfPhaseG0->ucG0_H_T2_H = iBfPhaseGx.ucGx_H_T2_H;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G1 and Group_H\n G0_H_T0_H = %d\n G0_H_T1_H = %d\n G0_H_T2_H = %d\n",
						 iBfPhaseGx.ucGx_H_T0_H,
						 iBfPhaseGx.ucGx_H_T1_H,
						 iBfPhaseGx.ucGx_H_T2_H));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", 0));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n",
						 iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
						 iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
						 iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
						 iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
				break;
			}
		} else {
			ucGroupIdx = ucGroup - 1;
			NdisCopyMemory(&iBfPhaseGx, pucIBfPhaseG, sizeof(MT7615_IBF_PHASE_Gx_T));

			piBfPhaseGx = (P_MT7615_IBF_PHASE_Gx_T)&pAd->piBfPhaseGx[ucGroupIdx * sizeof(MT7615_IBF_PHASE_Gx_T)];

			switch (ucGroupL_M_H) {
			case GROUP_L:
				piBfPhaseGx->ucGx_L_T0_H = iBfPhaseGx.ucGx_L_T0_H;
				piBfPhaseGx->ucGx_L_T1_H = iBfPhaseGx.ucGx_L_T1_H;
				piBfPhaseGx->ucGx_L_T2_H = iBfPhaseGx.ucGx_L_T2_H;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_L\n G%d_L_T0_H = %d\n G%d_L_T1_H = %d\n G%d_L_T2_H = %d\n",
						 ucGroup,
						 ucGroup, iBfPhaseGx.ucGx_L_T0_H,
						 ucGroup, iBfPhaseGx.ucGx_L_T1_H,
						 ucGroup, iBfPhaseGx.ucGx_L_T2_H));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n",
						 iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
						 iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
						 iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
						 iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
				break;

			case GROUP_M:
				piBfPhaseGx->ucGx_M_T0_H = iBfPhaseGx.ucGx_M_T0_H;
				piBfPhaseGx->ucGx_M_T1_H = iBfPhaseGx.ucGx_M_T1_H;
				piBfPhaseGx->ucGx_M_T2_H = iBfPhaseGx.ucGx_M_T2_H;
				piBfPhaseGx->ucGx_R0_H   = iBfPhaseGx.ucGx_R0_H;
				piBfPhaseGx->ucGx_R0_M   = iBfPhaseGx.ucGx_R0_M;
				piBfPhaseGx->ucGx_R0_L   = iBfPhaseGx.ucGx_R0_L;
				piBfPhaseGx->ucGx_R1_H   = iBfPhaseGx.ucGx_R1_H;
				piBfPhaseGx->ucGx_R1_M   = iBfPhaseGx.ucGx_R1_M;
				piBfPhaseGx->ucGx_R1_L   = iBfPhaseGx.ucGx_R1_L;
				piBfPhaseGx->ucGx_R2_H   = iBfPhaseGx.ucGx_R2_H;
				piBfPhaseGx->ucGx_R2_M   = iBfPhaseGx.ucGx_R2_M;
				piBfPhaseGx->ucGx_R2_L   = iBfPhaseGx.ucGx_R2_L;
				piBfPhaseGx->ucGx_R3_H   = iBfPhaseGx.ucGx_R3_H;
				piBfPhaseGx->ucGx_R3_M   = iBfPhaseGx.ucGx_R3_M;
				piBfPhaseGx->ucGx_R3_L   = iBfPhaseGx.ucGx_R3_L;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_M\n G%d_M_T0_H = %d\n G%d_M_T1_H = %d\n G%d_M_T2_H = %d\n",
						 ucGroup,
						 ucGroup, iBfPhaseGx.ucGx_M_T0_H,
						 ucGroup, iBfPhaseGx.ucGx_M_T1_H,
						 ucGroup, iBfPhaseGx.ucGx_M_T2_H));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R0_H = %d\n G%d_R0_M = %d\n G%d_R0_L = %d\n G%d_R1_H = %d\n G%d_R1_M = %d\n G%d_R1_L = %d\n",
						 ucGroup, iBfPhaseGx.ucGx_R0_H,
						 ucGroup, iBfPhaseGx.ucGx_R0_M,
						 ucGroup, iBfPhaseGx.ucGx_R0_L,
						 ucGroup, iBfPhaseGx.ucGx_R1_H,
						 ucGroup, iBfPhaseGx.ucGx_R1_M,
						 ucGroup, iBfPhaseGx.ucGx_R1_L));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d_R2_H = %d\n G%d_R2_M = %d\n G%d_R2_L = %d\n G%d_R3_H = %d\n G%d_R3_M = %d\n G%d_R3_L = %d\n",
						 ucGroup, iBfPhaseGx.ucGx_R2_H,
						 ucGroup, iBfPhaseGx.ucGx_R2_M,
						 ucGroup, iBfPhaseGx.ucGx_R2_L,
						 ucGroup, iBfPhaseGx.ucGx_R3_H,
						 ucGroup, iBfPhaseGx.ucGx_R3_M,
						 ucGroup, iBfPhaseGx.ucGx_R3_L));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n",
						 iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
						 iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
						 iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
						 iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
				break;

			case GROUP_H:
				piBfPhaseGx->ucGx_H_T0_H = iBfPhaseGx.ucGx_H_T0_H;
				piBfPhaseGx->ucGx_H_T1_H = iBfPhaseGx.ucGx_H_T1_H;
				piBfPhaseGx->ucGx_H_T2_H = iBfPhaseGx.ucGx_H_T2_H;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G%d and Group_H\n G%d_H_T0_H = %d\n G%d_H_T1_H = %d\n G%d_H_T2_H = %d\n",
						 ucGroup,
						 ucGroup, iBfPhaseGx.ucGx_H_T0_H,
						 ucGroup, iBfPhaseGx.ucGx_H_T1_H,
						 ucGroup, iBfPhaseGx.ucGx_H_T2_H));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n",
						 iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
						 iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
						 iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
						 iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
				break;
			}
		}
	break;

	case IBF_PHASE_CAL_VERIFY: /* Show calibrated result only */
	case IBF_PHASE_CAL_VERIFY_INSTRUMENT:
		NdisCopyMemory(&iBfPhaseOut, pBuf, sizeof(MT7615_IBF_PHASE_OUT));
		/* Update calibrated status */
		pAd->fgCalibrationFail |= ((ucStatus == 1) ? FALSE : TRUE);
		pAd->fgGroupIdPassFailStatus[ucGroup] = ((ucStatus == 1) ? FALSE : TRUE);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C3_M : %d, C3_L : %d\n",
				 iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
				 iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
				 iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
				 iBfPhaseOut.ucC3_M, iBfPhaseOut.ucC3_L));
		break;

	default:
		break;
	}
}
#endif /* MT7615 */


#if defined(MT7622)
VOID mt7622_iBFPhaseCalReport(IN PRTMP_ADAPTER pAd,
		   IN UCHAR   ucGroupL_M_H,
		   IN UCHAR   ucGroup,
		   IN BOOLEAN fgSX2,
		   IN UCHAR   ucStatus,
		   IN UCHAR   ucPhaseCalType,
		   IN PUCHAR  pBuf)
{
	MT7622_IBF_PHASE_G0_T   iBfPhaseG0;
	MT7622_IBF_PHASE_Gx_T   iBfPhaseGx;
	P_MT7622_IBF_PHASE_G0_T piBfPhaseG0;
	MT7622_IBF_PHASE_OUT    iBfPhaseOut;
	PUCHAR pucIBfPhaseG;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s :: Calibrated iBF phases\n", __func__));
	pucIBfPhaseG = pBuf + sizeof(MT7622_IBF_PHASE_OUT);
	NdisCopyMemory(&iBfPhaseOut, pBuf, sizeof(MT7622_IBF_PHASE_OUT));

	piBfPhaseG0 = (P_MT7622_IBF_PHASE_G0_T)pAd->piBfPhaseG0;

	switch (ucPhaseCalType) {
	case IBF_PHASE_CAL_NOTHING: /* Do nothing */
		break;

	case IBF_PHASE_CAL_NORMAL_INSTRUMENT:
	case IBF_PHASE_CAL_NORMAL: /* Store calibrated phases with buffer mode */
		/* IF phase calibration is for BW20/40/80 */
		if (ucGroup == GROUP_0) {
			NdisCopyMemory(&iBfPhaseGx, pucIBfPhaseG, sizeof(MT7622_IBF_PHASE_Gx_T));
			NdisCopyMemory(&iBfPhaseG0, pucIBfPhaseG, sizeof(MT7622_IBF_PHASE_G0_T));

			switch (ucGroupL_M_H) {
			case GROUP_L:
				piBfPhaseG0->ucG0_L_T0_UH = iBfPhaseGx.ucGx_L_T0_UH;
				piBfPhaseG0->ucG0_L_T1_UH = iBfPhaseGx.ucGx_L_T1_UH;
				piBfPhaseG0->ucG0_L_T2_UH = iBfPhaseGx.ucGx_L_T2_UH;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0 and Group_L\n G0_L_T0_UH = %d\n G0_L_T1_UH = %d\n G0_L_T2_UH = %d\n",
					iBfPhaseGx.ucGx_L_T0_UH,
					iBfPhaseGx.ucGx_L_T1_UH,
					iBfPhaseGx.ucGx_L_T2_UH));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", 0));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C0_UH : %d, C1_UH : %d, C2_UH : %d\n",
					iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
					iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
					iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
					iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C_L : %d, C_M : %d, C_H : %d, C_UH : %d\n",
					iBfPhaseOut.ucC_L, iBfPhaseOut.ucC_M, iBfPhaseOut.ucC_H, iBfPhaseOut.ucC_UH));
				break;
			case GROUP_M:
				piBfPhaseG0->ucG0_M_T0_UH = iBfPhaseGx.ucGx_M_T0_UH;
				piBfPhaseG0->ucG0_M_T1_UH = iBfPhaseGx.ucGx_M_T1_UH;
				piBfPhaseG0->ucG0_M_T2_UH = iBfPhaseGx.ucGx_M_T2_UH;
				piBfPhaseG0->ucG0_R0_UH   = iBfPhaseG0.ucG0_R0_UH;
				piBfPhaseG0->ucG0_R0_H    = iBfPhaseG0.ucG0_R0_H;
				piBfPhaseG0->ucG0_R0_M    = iBfPhaseG0.ucG0_R0_M;
				piBfPhaseG0->ucG0_R0_L    = iBfPhaseG0.ucG0_R0_L;
				piBfPhaseG0->ucG0_R1_UH   = iBfPhaseG0.ucG0_R1_UH;
				piBfPhaseG0->ucG0_R1_H    = iBfPhaseG0.ucG0_R1_H;
				piBfPhaseG0->ucG0_R1_M    = iBfPhaseG0.ucG0_R1_M;
				piBfPhaseG0->ucG0_R1_L    = iBfPhaseG0.ucG0_R1_L;
				piBfPhaseG0->ucG0_R2_UH   = iBfPhaseG0.ucG0_R2_UH;
				piBfPhaseG0->ucG0_R2_H    = iBfPhaseG0.ucG0_R2_H;
				piBfPhaseG0->ucG0_R2_M    = iBfPhaseG0.ucG0_R2_M;
				piBfPhaseG0->ucG0_R2_L    = iBfPhaseG0.ucG0_R2_L;
				piBfPhaseG0->ucG0_R3_UH   = iBfPhaseG0.ucG0_R3_UH;
				piBfPhaseG0->ucG0_R3_H    = iBfPhaseG0.ucG0_R3_H;
				piBfPhaseG0->ucG0_R3_M    = iBfPhaseG0.ucG0_R3_M;
				piBfPhaseG0->ucG0_R3_L    = iBfPhaseG0.ucG0_R3_L;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0 and Group_M\n G0_M_T0_UH = %d\n G0_M_T1_UH = %d\n G0_M_T2_UH = %d\n",
					iBfPhaseGx.ucGx_M_T0_UH,
					iBfPhaseGx.ucGx_M_T1_UH,
					iBfPhaseGx.ucGx_M_T2_UH));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0_R0_UH = %d\n G0_R0_H = %d\n G0_R0_M = %d\n G0_R0_L = %d\n G0_R1_UH = %d\n G0_R1_H = %d\n G0_R1_M = %d\n G0_R1_L = %d\n",
					iBfPhaseG0.ucG0_R0_UH,
					iBfPhaseG0.ucG0_R0_H,
					iBfPhaseG0.ucG0_R0_M,
					iBfPhaseG0.ucG0_R0_L,
					iBfPhaseG0.ucG0_R1_UH,
					iBfPhaseG0.ucG0_R1_H,
					iBfPhaseG0.ucG0_R1_M,
					iBfPhaseG0.ucG0_R1_L));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G0_R2_UH = %d\n G0_R2_H = %d\n G0_R2_M = %d\n G0_R2_L = %d\n G0_R3_UH = %d\n G0_R3_H = %d\n G0_R3_M = %d\n G0_R3_L = %d\n",
					iBfPhaseG0.ucG0_R2_UH,
					iBfPhaseG0.ucG0_R2_H,
					iBfPhaseG0.ucG0_R2_M,
					iBfPhaseG0.ucG0_R2_L,
					iBfPhaseG0.ucG0_R3_UH,
					iBfPhaseG0.ucG0_R3_H,
					iBfPhaseG0.ucG0_R3_M,
					iBfPhaseG0.ucG0_R3_L));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", 0));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C0_UH : %d, C1_UH : %d, C2_UH : %d\n",
					iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
					iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
					iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
					iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C_L : %d, C_M : %d, C_H : %d, C_UH : %d\n",
					iBfPhaseOut.ucC_L, iBfPhaseOut.ucC_M, iBfPhaseOut.ucC_H, iBfPhaseOut.ucC_UH));

				break;
			case GROUP_H:
				piBfPhaseG0->ucG0_H_T0_UH = iBfPhaseGx.ucGx_H_T0_UH;
				piBfPhaseG0->ucG0_H_T1_UH = iBfPhaseGx.ucGx_H_T1_UH;
				piBfPhaseG0->ucG0_H_T2_UH = iBfPhaseGx.ucGx_H_T2_UH;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("G1 and Group_H\n G0_H_T0_UH = %d\n G0_H_T1_UH = %d\n G0_H_T2_UH = %d\n",
					iBfPhaseGx.ucGx_H_T0_UH,
					iBfPhaseGx.ucGx_H_T1_UH,
					iBfPhaseGx.ucGx_H_T2_UH));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", 0));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C0_UH : %d, C1_UH : %d, C2_UH : %d\n",
					iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
					iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
					iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
					iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C_L : %d, C_M : %d, C_H : %d, C_UH : %d\n",
					iBfPhaseOut.ucC_L, iBfPhaseOut.ucC_M, iBfPhaseOut.ucC_H, iBfPhaseOut.ucC_UH));

				break;
		}
	}
	break;

	case IBF_PHASE_CAL_VERIFY: /* Show calibrated result only */
	case IBF_PHASE_CAL_VERIFY_INSTRUMENT:
		NdisCopyMemory(&iBfPhaseOut, pBuf, sizeof(MT7622_IBF_PHASE_OUT));
		/* Update calibrated status */
		pAd->fgCalibrationFail |= ((ucStatus == 1) ? FALSE : TRUE);
		pAd->fgGroupIdPassFailStatus[ucGroup] = ((ucStatus == 1) ? FALSE : TRUE);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group : %d\n", ucGroup));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibration == 1? or Verification == 2? : %d\n", ucPhaseCalType));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Calibrated result = %d\n", ucStatus));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0 : Means failed\n 1: means pass\n 2: means on-going\n"));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C0_H : %d, C1_H : %d, C2_H : %d\n C0_M : %d, C1_M : %d, C2_M : %d\n C0_L : %d, C1_L : %d, C2_L : %d\n C0_UH : %d, C1_UH : %d, C2_UH : %d\n",
			 iBfPhaseOut.ucC0_H, iBfPhaseOut.ucC1_H, iBfPhaseOut.ucC2_H,
			 iBfPhaseOut.ucC0_M, iBfPhaseOut.ucC1_M, iBfPhaseOut.ucC2_M,
			 iBfPhaseOut.ucC0_L, iBfPhaseOut.ucC1_L, iBfPhaseOut.ucC2_L,
			 iBfPhaseOut.ucC0_UH, iBfPhaseOut.ucC1_UH, iBfPhaseOut.ucC2_UH));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("C_L : %d, C_M : %d, C_H : %d, C_UH : %d\n",
			 iBfPhaseOut.ucC_L, iBfPhaseOut.ucC_M, iBfPhaseOut.ucC_H, iBfPhaseOut.ucC_UH));

		break;

	default:
		break;
	}
}
#endif

#endif /* TXBF_SUPPORT */
