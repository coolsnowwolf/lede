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
	trace.c
*/

#include "rt_config.h"

#ifdef CONFIG_TRACE_SUPPORT
#define CREATE_TRACE_POINTS
#include "os/trace.h"
#endif

static VOID TraceCrPseFrameInfo(RTMP_ADAPTER *pAd, UINT8 PID, UINT8 QID)
{
	UINT32 FirstFID, CurFID, NextFID, FrameNums = 0;
	UINT32 Value;
	UCHAR PseData[32];
	UINT32 DwIndex;

	RTMP_IO_READ32(pAd, C_GFF, &Value);
	Value &= ~GET_QID_MASK;
	Value |= SET_GET_QID(QID);
	Value &= ~GET_PID_MASK;
	Value |= SET_GET_PID(PID);
	RTMP_IO_WRITE32(pAd, C_GFF, Value);
	RTMP_IO_READ32(pAd, C_GFF, &Value);
	FirstFID = GET_FIRST_FID(Value);

	if (FirstFID == 0xfff)
		return;

	CurFID = FirstFID;
	FrameNums++;

	for (DwIndex = 0; DwIndex < 8; DwIndex++) {
		RTMP_IO_READ32(pAd, (MT_PCI_REMAP_ADDR_1 + (((CurFID) * 128)) + (DwIndex * 4)),
					   &PseData[DwIndex * 4]);
	}

	TRACE_CR_PSE_FRAME_INFO(PID, QID, CurFID);
	TRACE_TX_MAC_LINFO((TMAC_TXD_L *)PseData);

	while (1) {
		RTMP_IO_READ32(pAd, C_GF, &Value);
		Value &= ~CURR_FID_MASK;
		Value |= SET_CURR_FID(CurFID);
		RTMP_IO_WRITE32(pAd, C_GF, Value);
		RTMP_IO_READ32(pAd, C_GF, &Value);
		NextFID = GET_RETURN_FID(Value);

		if (NextFID == 0xfff)
			return;

		CurFID = NextFID;

		for (DwIndex = 0; DwIndex < 8; DwIndex++) {
			RTMP_IO_READ32(pAd, (MT_PCI_REMAP_ADDR_1 + (((CurFID) * 128)) + (DwIndex * 4)),
						   &PseData[DwIndex * 4]);
		}

		TRACE_CR_PSE_FRAME_INFO(PID, QID, CurFID);
		TRACE_TX_MAC_LINFO((TMAC_TXD_L *)PseData);
		FrameNums++;
	}
}


VOID TraceCrPseInfo(RTMP_ADAPTER *pAd)
{
	TRACE_CR_PSE_INFO(pAd);
	/* HIF port frame information */
	TraceCrPseFrameInfo(pAd, 0, 0);
	TraceCrPseFrameInfo(pAd, 0, 1);
	TraceCrPseFrameInfo(pAd, 0, 2);
	/* MCU port frame information */
	TraceCrPseFrameInfo(pAd, 1, 0);
	TraceCrPseFrameInfo(pAd, 1, 1);
	TraceCrPseFrameInfo(pAd, 1, 2);
	TraceCrPseFrameInfo(pAd, 1, 3);
	/* WLAN port frame information */
	TraceCrPseFrameInfo(pAd, 2, 0);
	TraceCrPseFrameInfo(pAd, 2, 1);
	TraceCrPseFrameInfo(pAd, 2, 2);
	TraceCrPseFrameInfo(pAd, 2, 3);
	TraceCrPseFrameInfo(pAd, 2, 4);
	TraceCrPseFrameInfo(pAd, 2, 5);
	TraceCrPseFrameInfo(pAd, 2, 6);
	TraceCrPseFrameInfo(pAd, 2, 7);
	TraceCrPseFrameInfo(pAd, 2, 8);
	TraceCrPseFrameInfo(pAd, 2, 9);
	TraceCrPseFrameInfo(pAd, 2, 10);
	TraceCrPseFrameInfo(pAd, 2, 11);
	TraceCrPseFrameInfo(pAd, 2, 12);
	TraceCrPseFrameInfo(pAd, 2, 13);
}


#define MT_WTBL_SIZE			20
VOID TraceWtblInfo(RTMP_ADAPTER *pAd, UINT32 wtbl_idx)
{
	INT idx, start_idx, end_idx, tok;
	UINT32 addr, val[16];
	struct wtbl_1_struc wtbl_1;
	struct wtbl_2_struc wtbl_2;
	union wtbl_3_struc wtbl_3;
	struct wtbl_4_struc wtbl_4;

	if (wtbl_idx == RESERVED_WCID) {
		start_idx = 0;
		end_idx = (MT_WTBL_SIZE - 1);
	} else if (wtbl_idx < MT_WTBL_SIZE)
		start_idx = end_idx = wtbl_idx;
	else
		return;

	for (idx = start_idx; idx <= end_idx; idx++) {
		/* Read WTBL 1 */
		os_zero_mem((UCHAR *)&wtbl_1, sizeof(struct wtbl_1_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];
		RTMP_IO_READ32(pAd, addr, &wtbl_1.wtbl_1_d0.word);
		RTMP_IO_READ32(pAd, addr + 4, &wtbl_1.wtbl_1_d1.word);
		RTMP_IO_READ32(pAd, addr + 8, &wtbl_1.wtbl_1_d2.word);
		RTMP_IO_READ32(pAd, addr + 12, &wtbl_1.wtbl_1_d3.word);
		RTMP_IO_READ32(pAd, addr + 16, &wtbl_1.wtbl_1_d4.word);
		TRACE_CR_WTBL1_INFO(pAd, &wtbl_1);
		/* Read WTBL 2 */
		os_zero_mem((UCHAR *)&wtbl_2, sizeof(struct wtbl_2_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[1] + idx * pAd->mac_ctrl.wtbl_entry_size[1];

		for (tok = 0; tok < sizeof(struct wtbl_2_struc) / 4; tok++)
			RTMP_IO_READ32(pAd, addr + tok * 4, &val[tok]);

		TRACE_CR_WTBL2_INFO(pAd, (struct wtbl_2_struc *)&val[0]);
		/* Read WTBL 3 */
		os_zero_mem((UCHAR *)&wtbl_3, sizeof(union wtbl_3_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[2] + idx * pAd->mac_ctrl.wtbl_entry_size[2];

		for (tok = 0; tok < sizeof(union wtbl_3_struc) / 4; tok++)
			RTMP_IO_READ32(pAd, addr + tok * 4, &val[tok]);

		/* dump_wtbl_3_info(pAd, (union wtbl_3_struc *)&val[0]); */
		/* Read WTBL 4 */
		os_zero_mem((UCHAR *)&wtbl_4, sizeof(struct wtbl_4_struc));
		addr = pAd->mac_ctrl.wtbl_base_addr[3] + idx * pAd->mac_ctrl.wtbl_entry_size[3];
		RTMP_IO_READ32(pAd, addr, &wtbl_4.ac0.word[0]);
		RTMP_IO_READ32(pAd, addr + 4, &wtbl_4.ac0.word[1]);
		RTMP_IO_READ32(pAd, addr + 8, &wtbl_4.ac1.word[0]);
		RTMP_IO_READ32(pAd, addr + 12, &wtbl_4.ac1.word[1]);
		RTMP_IO_READ32(pAd, addr + 16, &wtbl_4.ac2.word[0]);
		RTMP_IO_READ32(pAd, addr + 20, &wtbl_4.ac2.word[1]);
		RTMP_IO_READ32(pAd, addr + 24, &wtbl_4.ac3.word[0]);
		RTMP_IO_READ32(pAd, addr + 28, &wtbl_4.ac3.word[1]);
		/* dump_wtbl_4_info(pAd, &wtbl_4); */
	}
}


INT32 TracePSTable(RTMP_ADAPTER *pAd, UINT32 ent_type, BOOLEAN bReptCli)
{
	INT i, j;
	UINT32 RegValue;
	ULONG DataRate = 0;
	struct wtbl_entry tb_entry;
	union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
	UINT32  rPseRdTabAccessReg;
	BOOLEAN pfgForce;
	UCHAR pucPort, pucQueue;
	INT Total_Packet_Number = 0;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];

		Total_Packet_Number = 0;

		if (ent_type == ENTRY_NONE) {
			/* dump all MacTable entries */
			if (pEntry->EntryType == ENTRY_NONE)
				continue;
		} else {
			/* dump MacTable entries which match the EntryType */
			if (pEntry->EntryType != ent_type)
				continue;

			if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry))
				&& (pEntry->Sst != SST_ASSOC))
				continue;
		}

		os_zero_mem(&tb_entry, sizeof(tb_entry));

		if (mt_wtbl_get_entry234(pAd, pEntry->wcid, &tb_entry) == FALSE)
			return FALSE;

		RTMP_IO_READ32(pAd, tb_entry.wtbl_addr[0] + 12, &dw3->word);
		/* get PSE register */
		/* rPseRdTabAccessReg.field.rd_kick_busy=1; */
		/* rPseRdTabAccessReg.field.rd_tag=pEntry->wcid; */
		rPseRdTabAccessReg = PSE_RTA_RD_KICK_BUSY | PSE_RTA_TAG(pEntry->wcid);
		RTMP_IO_WRITE32(pAd, PSE_RTA, rPseRdTabAccessReg);

		do {
			RTMP_IO_READ32(pAd, PSE_RTA, &rPseRdTabAccessReg);
			pfgForce = (BOOLEAN) GET_PSE_RTA_RD_RULE_F(rPseRdTabAccessReg);
			pucPort  = (UCHAR)  GET_PSE_RTA_RD_RULE_PID(rPseRdTabAccessReg);
			pucQueue = (UCHAR)  GET_PSE_RTA_RD_RULE_QID(rPseRdTabAccessReg);
		} while (GET_PSE_RTA_RD_KICK_BUSY(rPseRdTabAccessReg) == 1);

		Total_Packet_Number = Total_Packet_Number + tr_entry->ps_queue.Number;

		for (j = 0; j < WMM_QUE_NUM; j++)
			Total_Packet_Number = Total_Packet_Number + tr_entry->tx_queue[j].Number;

		TRACE_PS_INFO(pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2], pEntry->Addr[3],
					  pEntry->Addr[4], pEntry->Addr[5],
					  pEntry->EntryType, pEntry->Aid, pEntry->func_tb_idx,
					  pEntry->PsMode, tr_entry->ps_state, dw3->field.i_psm,
					  dw3->field.du_i_psm, dw3->field.skip_tx, pfgForce,
					  pucPort, pucQueue, Total_Packet_Number, tr_entry->ps_queue.Number);
	}

	return TRUE;
}


INT32 TraceTRInfo(RTMP_ADAPTER *pAd)
{
	return TRUE;
}
