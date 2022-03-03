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
	meta_agent.c
*/
#include "rt_config.h"

enum _WIFI_SUB_TEST_STATE {
	WIFI_SUB_TEST_STANDBY,
	WIFI_SUB_TEST_RESET,
	WIFI_SUB_TEST_TX,
	WIFI_SUB_TEST_RX,
	WIFI_SUB_TEST_OUTPUT_PWR,
	WIFI_SUB_TEST_LOCAL_FREQ,
	WIFI_SUB_TEST_CARR_SUPPRESSION,
	WIFI_SUB_TEST_CONTI_WAVE,
	NUM_STATE
};
#define _DUMP_HEX_ 1
static enum _WIFI_SUB_TEST_STATE g_SUB_TEST_STATE = WIFI_SUB_TEST_STANDBY;
static UINT32 g_MT_META_ATCMD_TBL_VERSION = MT_META_WIFI_TEST_TABLE_VER;

static BOOLEAN valid_ch(UCHAR Ch)
{
	int chIdx;

	for (chIdx = 0; chIdx < CH_HZ_ID_MAP_NUM; chIdx++) {
		if ((Ch) == CH_HZ_ID_MAP[chIdx].channel)
			return true;
	}

	return false;
}

static INT32 resp_to_meta(INT32 ioctl_cmd, struct _META_CMD_HDR *rsp, RTMP_IOCTL_INPUT_STRUCT *wrq, INT32 len, INT32 status)
{
	if (ioctl_cmd == MTPRIV_IOCTL_META_SET)
		rsp->len_in = len;
	else if (ioctl_cmd == MTPRIV_IOCTL_META_QUERY)
		rsp->len_out = len;

	wrq->u.data.length = sizeof(rsp->oid) + sizeof(rsp->len_in) + sizeof(rsp->len_out) + len;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,   ("WRQ->u.data.length = %u, oid: %lu, rsp: %lu, rsp->len: 0x%x\n", wrq->u.data.length, sizeof(rsp->oid), sizeof(rsp->len_in), len));

	if (copy_to_user(wrq->u.data.pointer, (UCHAR *)(rsp), wrq->u.data.length)) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("copy_to_user() fail in %s\n", __func__));
		return -EFAULT;
	}

#ifdef _DUMP_HEX_
	{
		UCHAR *pSrcBufVA = wrq->u.data.pointer;
		UINT32 SrcBufLen = wrq->u.data.length;
		unsigned char *pt;
		INT32 x;

		pt = pSrcBufVA;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %p, len = %d\n", "META_IOCTL Ret Content", pSrcBufVA, SrcBufLen));

		for (x = 0; x < SrcBufLen; x++) {
			if (x % 16 == 0)
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x%04x : ", x));

			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x ", ((unsigned char)pt[x])));

			if (x % 16 == 15)
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n", __func__));
		}

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	}
#endif
	return 0;
}

static INT32 wifi_sub_test_stop(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = 0;
	INT32 val = 0;
	UINT32 Mode;

	memcpy((PUCHAR)&val, &param->data, 4);

	switch (g_SUB_TEST_STATE) {
	case WIFI_SUB_TEST_STANDBY:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, WIFI_SUB_TEST_STANDBY\n", __func__));
		break;

	case WIFI_SUB_TEST_RESET:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, WIFI_SUB_TEST_RESET\n", __func__));
		break;

	case WIFI_SUB_TEST_TX:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, WIFI_SUB_TEST_TX STOP\n", __func__));
		Mode = ATECtrl->Mode;
		ATECtrl->Mode &= ATE_TXSTOP;
		ATECtrl->bQATxStart = FALSE;
		ret = ATEOp->StopTx(pAd, Mode);
		ATECtrl->TxStatus = 0;
		break;

	case WIFI_SUB_TEST_RX:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, WIFI_SUB_TEST_RX STOP\n", __func__));
		ATECtrl->Mode &= ATE_RXSTOP;
		ATECtrl->bQARxStart = FALSE;
		ret = ATEOp->StopRx(pAd);
		break;

	case WIFI_SUB_TEST_OUTPUT_PWR:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Conti. Tx stop\n", __func__));
		ret = ATEOp->StopContinousTx(pAd, TESTMODE_BAND0);
		break;

	case WIFI_SUB_TEST_LOCAL_FREQ:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Tx Tone DC stop\n", __func__));
		ret = ATEOp->StopTxTone(pAd);
		break;

	case WIFI_SUB_TEST_CARR_SUPPRESSION:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Tx Carrier Suppression stop\n", __func__));
		ret = ATEOp->StopTxTone(pAd);
		break;

	case WIFI_SUB_TEST_CONTI_WAVE:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Conti. Tx stop\n", __func__));
		ret = ATEOp->StopContinousTx(pAd, TESTMODE_BAND0);
		break;

	default:
		break;
	}

	g_SUB_TEST_STATE = WIFI_SUB_TEST_STANDBY;
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_sub_test_start_tx(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	ret = ATEOp->StartTx(pAd);
	g_SUB_TEST_STATE =	WIFI_SUB_TEST_TX;

	if (ATECtrl->bQATxStart == TRUE)
		ATECtrl->TxStatus = 1;

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_sub_test_start_rx(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	g_SUB_TEST_STATE =	WIFI_SUB_TEST_RX;
	ret = ATEOp->StartRx(pAd);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_sub_test_reset(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));

	if (ioctl_cmd == MTPRIV_IOCTL_META_SET) {
		/* 1. Table Version */
		g_MT_META_ATCMD_TBL_VERSION = 0x01000002;
		/* 3. Rate */
		ATECtrl->PhyMode = MODE_CCK;
		ATECtrl->Mcs = 1;
		/* 4. Preamble, follow PhyMode, and always using long preamble currently, in mac/mt_mac.c line. 754 */
		/* 6. Packet Length */
		ATECtrl->TxLength = 1024;
		/* 7. Packet Count */
		ATECtrl->TxCount = 1000;
		/* 15. BW */
		ATECtrl->BW = BW_20;
		/* 16. GI */
		ATECtrl->Sgi = 0;
		/* 18. CH FREQ */
		ATEOp->SetTxFreqOffset(pAd, 0);
		/* 23. TODO: Slow Clk Mode (?)*/
		/* 29. TODO: TSSI */
		/* 31. Tx Power mode - always dBm */
		/* 65. TODO: CW Mode (?) */
		/* 71. CH BW */
		ATECtrl->BW = BW_20;
		/* 72. Data BW */
		/* ATECtrl->PerPktBW = BW_20; */
		/* 73. Primary setting */
		ATECtrl->ControlChl = 1;
		ATECtrl->Channel = 1;
		/* 74. J mode Setting (tone) */
		g_SUB_TEST_STATE =	WIFI_SUB_TEST_STANDBY;
	} else {
		if (g_SUB_TEST_STATE != WIFI_SUB_TEST_STANDBY)
			ret =  NDIS_STATUS_FAILURE;

		resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	}

	return ret;
}

static INT32 wifi_sub_test_output_pwr(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	/* TODO: Correct band selection */
	ret = ATEOp->StartContinousTx(pAd, ATECtrl->TxAntennaSel, TESTMODE_BAND0);
	g_SUB_TEST_STATE = WIFI_SUB_TEST_OUTPUT_PWR;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, val:0x%x\n", __func__, val));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_sub_test_local_freq(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_FAILURE;
	INT32 val = 0;
	INT32 mode = 8;
	INT32 pwr1 = 0xf;
	INT32 pwr2 = 0;
	CHAR txpwr = 0;

	memcpy((PUCHAR)&val, &param->data, 4);

	/* TxAntennaSel, 0: All 1:TX0 2:TX1 */
	if (ATECtrl->TxAntennaSel == 1) {
		mode = WF0_TX_ONE_TONE_DC;
		txpwr = ATECtrl->TxPower0;
	} else if (ATECtrl->TxAntennaSel == 2) {
		mode = WF1_TX_ONE_TONE_DC;
		txpwr = ATECtrl->TxPower1;
	} else
		goto meta_tx_suppress_done;

	if (txpwr > 30)
		pwr2 = (txpwr - 30) << 1;
	else {
		pwr1 = (txpwr & 0x1e) >> 1;
		pwr2 = (txpwr & 0x01) << 1;
	}

	ret = ATEOp->StartTxTone(pAd, mode);
	ret = ATEOp->SetTxTonePower(pAd, pwr1, pwr2);
	g_SUB_TEST_STATE = WIFI_SUB_TEST_LOCAL_FREQ;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: TX Tone DC, TxPower:0x%x, pwr1:0x%x, pwr2:0x%x\n", __func__, txpwr, pwr1, pwr2));
meta_tx_suppress_done:
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_sub_test_rf_suppression(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_FAILURE;
	INT32 val = 0;
	INT32 mode = 8;
	INT32 pwr1 = 0xf;
	INT32 pwr2 = 0;
	CHAR txpwr = 0;

	memcpy((PUCHAR)&val, &param->data, 4);

	/* TxAntennaSel, 0: All 1:TX0 2:TX1 */
	if (ATECtrl->TxAntennaSel == 1) {
		mode = WF0_TX_TWO_TONE_5M;
		txpwr = ATECtrl->TxPower0;
	} else if (ATECtrl->TxAntennaSel == 2) {
		mode = WF1_TX_TWO_TONE_5M;
		txpwr = ATECtrl->TxPower1;
	} else
		goto meta_tx_suppress_done;

	if (txpwr > 30)
		pwr2 = (txpwr - 30) << 1;
	else {
		pwr1 = (txpwr & 0x1e) >> 1;
		pwr2 = (txpwr & 0x01) << 1;
	}

	ret = ATEOp->StartTxTone(pAd, mode);
	ret = ATEOp->SetTxTonePower(pAd, pwr1, pwr2);
	g_SUB_TEST_STATE = WIFI_SUB_TEST_CARR_SUPPRESSION;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: TXCARR, TxPower:0x%x, pwr1:0x%x, pwr2:0x%x\n", __func__, txpwr, pwr1, pwr2));
meta_tx_suppress_done:
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_sub_test_trx_iq_cali(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	/* UINT8 Action = 0, Mode = 0, CalItem = 0; */
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	/*
	 * Enter RF test mode
	 * TODO: Don't know when to go back to normal mode,  Mode = OPERATION_NORMAL_MODE;
	 */
	/* CmdRfTest(pAd, ACTION_SWITCH_TO_RFTEST, OPERATION_RFTEST_MODE, 0); */
	MtCmdRfTestSwitchMode(pAd, OPERATION_RFTEST_MODE, 0);
	/* Sec, operation
	 * 6: Tx IQ Cal
	 * 10: Rx FIIIQ Cal
	 * 11: Rx FDIQ Cal
	 */
	/* CmdRfTest(pAd, ACTION_IN_RFTEST, OPERATION_RFTEST_MODE, 6); */
	/* CmdRfTest(pAd, ACTION_IN_RFTEST, OPERATION_RFTEST_MODE, 10); */
	/* CmdRfTest(pAd, ACTION_IN_RFTEST, OPERATION_RFTEST_MODE, 11); */
	MtCmdDoCalibration(pAd, 6);
	MtCmdDoCalibration(pAd, 10);
	MtCmdDoCalibration(pAd, 11);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_sub_test_tssi_cali(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	/*
	 * Enter RF test mode
	 * TODO: Don't know when to go back to normal mode,  Mode = OPERATION_NORMAL_MODE;
	 */
	/* CmdRfTest(pAd, ACTION_SWITCH_TO_RFTEST, OPERATION_RFTEST_MODE, 0); */
	MtCmdRfTestSwitchMode(pAd, OPERATION_RFTEST_MODE, 0);
	/* Sec, operation
	 * 6: Tx IQ Cal
	 */
	/* CmdRfTest(pAd, ACTION_IN_RFTEST, OPERATION_RFTEST_MODE, 7); */
	MtCmdDoCalibration(pAd, 7);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_sub_test_dpd_cali(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	/*
	 * Enter RF test mode
	 * TODO: Don't know when to go back to normal mode,  Mode = OPERATION_NORMAL_MODE;
	 */
	/* CmdRfTest(pAd, ACTION_SWITCH_TO_RFTEST, OPERATION_RFTEST_MODE, 0); */
	MtCmdRfTestSwitchMode(pAd, OPERATION_RFTEST_MODE, 0);
	/* Sec, operation
	 * 6: Tx IQ Cal
	 */
	/* CmdRfTest(pAd, ACTION_IN_RFTEST, OPERATION_RFTEST_MODE, 9); */
	MtCmdDoCalibration(pAd, 9);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_sub_test_conti_waveform(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	/* TODO: Correct band selection */
	ret = ATEOp->StartContinousTx(pAd, ATECtrl->TxAntennaSel, TESTMODE_BAND0);
	g_SUB_TEST_STATE = WIFI_SUB_TEST_CONTI_WAVE;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, val:0x%x\n", __func__, val));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_sub_test_start_icap(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static META_CMD_HANDLER WIFI_TEST_CMD_SUBTEST[] = {
	wifi_sub_test_stop,
	wifi_sub_test_start_tx,
	wifi_sub_test_start_rx,
	wifi_sub_test_reset,
	wifi_sub_test_output_pwr,
	wifi_sub_test_local_freq,
	wifi_sub_test_rf_suppression,
	wifi_sub_test_trx_iq_cali,
	wifi_sub_test_tssi_cali,
	wifi_sub_test_dpd_cali,
	wifi_sub_test_conti_waveform,
	wifi_sub_test_start_icap,
};

static INT32 wifi_test_version(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	INT32 status = NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("\n%s: version:%04x !\n", __func__, g_MT_META_ATCMD_TBL_VERSION));
	memcpy(&param->data, &g_MT_META_ATCMD_TBL_VERSION, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(struct _PARAM_MTK_WIFI_TEST), status);
	return status;
}

static INT32 wifi_test_cmd(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT32 op = param->data;

	if (op > (sizeof(WIFI_TEST_CMD_SUBTEST) / sizeof(META_CMD_HANDLER))) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("%s:op_idx: %04x, size of handle tbl: %lu\n", __func__, op, sizeof(WIFI_TEST_CMD_SUBTEST) / sizeof(META_CMD_HANDLER)));
		return NDIS_STATUS_INVALID_DATA;
	}

	ret = (*WIFI_TEST_CMD_SUBTEST[op])(ioctl_cmd, pAd, WRQ, cmd_hdr);
	return ret;
}

static INT32 wifi_test_pwr_cfg(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, sizeof(val));
	/* val = OS_NTOHS(val); */
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, val:0x%x\n", __func__, val));
	/* TODO Add in the setting of another Tx Power, txpwr1*/
	pAd->ATECtrl.TxPower0 = val;
	ret = ATEOp->SetTxPower0(pAd, val, 0);
	/* ret = ATEOp->SetTxPower1(pAd, val); */
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_rate_cfg(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, sizeof(val));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, val:0x%x\n", __func__, val));

	if ((val >> 31) & 0x00000001) {
		/* b31(msb) set(1) - 11n MCS:0~15, 32 */
		val &= 0x7FFFFFF;

		if (((val > 15) && (val < 32))
			|| (val > 32))
			return -1;

		if (val == 32)
			val = 17; /* for corresponding to idx of OFDMRateTable */

		/*	case MODE_HTMIX / case MODE_HTGREENFIELD */
		ATECtrl->PhyMode = MODE_HTMIX;
	} else {
		/* b31(msb) set(0) - CCK:0~3, OFDM:4~11 */
		val &= 0x7FFFFFF;

		if ((val < 0) || (val > 11))
			return -1;
		else if ((val < 4) && (val >= 0)) {
			/*KOKO: Not sure why there are 8 entries in CCKRateTable */
			ATECtrl->PhyMode = MODE_CCK;
		} else {
			ATECtrl->PhyMode = MODE_OFDM;
			val -= 4;
		}
	}

	ATECtrl->Mcs = (UCHAR)val;
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_preamble_cfg(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Val: %x\n", __func__, val));

	switch (val) {
	case 0: /* Normal: don't know what is for normal, anyway choose OFDM */
		val = MODE_OFDM;
		break;

	case 1: /* Have to check if its short preamble by default in CCK */
		val = MODE_CCK;
		break;

	case 2:
		val = MODE_HTMIX;
		break;

	case 3:
		val = MODE_HTGREENFIELD;
		break;

	case 4:
		val = MODE_VHT;
		break;

	default:
		break;
	}

	ATECtrl->PhyMode = (UCHAR)val;
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_antenna_cfg(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Val: %x\n", __func__, val));
	ret = ATEOp->SetTxAntenna(pAd, (CHAR)val);
	ret = ATEOp->SetRxAntenna(pAd, (CHAR)val);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_pkt_len_cfg(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);

	if ((val < 24) || (val > (MAX_FRAME_SIZE - 34/* == 2312 */))) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Out of range (%d), it should be in range of 24~%d.\n", __func__, val,
				 (MAX_FRAME_SIZE - 34/* == 2312 */)));
		return -1;
	}

	ATECtrl->TxLength = val;
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxLength = %x\n", __func__, ATECtrl->TxLength));
	return ret;
}

static INT32 wifi_test_set_pkt_cnt(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, val: %u\n", __func__, val));

	if (val == 0)
		ATECtrl->TxCount = 0xFFFFFFFF;
	else
		ATECtrl->TxCount = val;

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}
/*
 *	Since different chip has different wifi Tx off time duration, meta tool do not set it
 */
static INT32 wifi_test_set_pkt_interval(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT32 val = 0;
	UINT32 slot_t = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	/* TODO: "Val will be round-up to (19+9n)us", quoted from doc. Ref. 6630 FW */
	slot_t = (val - 19 + SLOT_TIME_SHORT - 1) / SLOT_TIME_SHORT;
	ATEOp->SetAIFS(pAd, (UINT32)slot_t);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_tmp_comp(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	return ret;
}

static INT32 wifi_test_txop_en(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	return ret;
}

static INT32 wifi_test_set_ack(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	return ret;
}

static INT32 wifi_test_set_pkt_content(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	return ret;
}

static INT32 wifi_test_set_retry_cnt(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	return ret;
}

static INT32 wifi_test_set_qos_q(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);

	if (val > 4)
		return NDIS_STATUS_INVALID_DATA;

	/* ATECtrl->q_idx = (UCHAR)val; */
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_bw(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, val: 0x%x\n", __func__, val));

	if (val > 3) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, Invalid Parameter(%x)\n", __func__, val));
		ret =  -1;
		goto _meta_set_bw_err;
	}

	if (val == 0) {
		ATECtrl->BW = BW_20;
		ATECtrl->ControlChl = ATECtrl->Channel;
	} else {
		ATECtrl->BW = BW_40;
		ATECtrl->ControlChl = ATECtrl->Channel;

		switch (val) {
		case 1:
		case 2:	/* Upper 20MHz of a 40MHz Ch, ex. Ch6 upper, then control ch will be Ch4 */
			ATECtrl->ControlChl -= 2;

			if (!valid_ch(ATECtrl->ControlChl)) {
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s-case1/2, Invalid ControlChl(%x)\n", __func__, ATECtrl->ControlChl));
				ATECtrl->ControlChl = ATECtrl->Channel;
				ATECtrl->Channel += 2;
			}

			break;

		case 3:	/* Lower 20MHz of a 40MHz Ch, ex. Ch6 lower, then control ch will be Ch8 */
			ATECtrl->Channel += 2;

			if (!valid_ch(ATECtrl->Channel)) {
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s-case3, Invalid ControlChl(%x)\n", __func__, ATECtrl->ControlChl));
				ATECtrl->Channel =	ATECtrl->ControlChl;
				ATECtrl->ControlChl -= 2;
			}

			break;

		default:
			break;
		}
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Channel = %d, Control ch: 0x%x, BW = %d\n", __func__, ATECtrl->Channel, ATECtrl->ControlChl, ATECtrl->BW));
	ret = ATEOp->SetBW(pAd, ATECtrl->BW);
_meta_set_bw_err:
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

/*
 *	param:
 *		0: normal GI
 *		1: short GI
 */
static INT32 wifi_test_set_gi(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, val:0x%x\n", __func__, val));

	if (val > 1 || val < 0)
		return -1;

	ATECtrl->Sgi = (UCHAR)val;
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_stbc(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	ATECtrl->Stbc = (UCHAR)val;
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_ch_freq(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;
	INT32 ch = 1;

	memcpy((PUCHAR)&val, &param->data, 4);
	/* TODO: Need to check the value passed from host */
	RTMP_MapKHZ2ChannelID(val / 1000, &ch);
	ATECtrl->Channel = ch;
	ATECtrl->ControlChl = ATECtrl->Channel;

	/* Set Control channel according to BW */
	switch (ATECtrl->BW) {
	case BW_20:
		break;

	case BW_40:
	case BW_80:
	default:

		/* Lower 20MHz of a 40MHz Ch, ex. Ch6 lower, then control ch will be Ch8 */
		if (ATECtrl->ControlChl > 2)
			ATECtrl->ControlChl -= 2;
		else
			ATECtrl->ControlChl += 2;

		break;
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, val:%d, ch: 0x%x, Control ch: 0x%x\n", __func__, val, ATECtrl->Channel, ATECtrl->ControlChl));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_rifs(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_tr_switch(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_rf_sx_en(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_pll_en(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_slow_clk_en(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_adc_clk_mode(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_measure_mode(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_vlt_comp(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_get_dpd_tx_gain(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_dpd_en(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_tssi_en(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;
	INT32 wf_sel = 0;

	/* TODO: Plan to use upper 16 bits for antenna selection, lower 16 bits for on/off */
	memcpy((PUCHAR)&val, &param->data, 4);
	val = val & 0x0000ffff;
	memcpy((PUCHAR)&wf_sel, &param->data, 4);
	wf_sel = ((wf_sel & 0xffff0000) >> 16) & 0x0000ffff;
	ATEOp->SetTSSI(pAd, (CHAR)wf_sel, (CHAR)val);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_get_tx_gain_code(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_tx_pwr_mode(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static META_CMD_HANDLER WIFI_TEST_CMD_SET1[] = {
	wifi_test_version,
	wifi_test_cmd,
	wifi_test_pwr_cfg,
	wifi_test_rate_cfg,
	wifi_test_preamble_cfg,
	wifi_test_antenna_cfg,
	wifi_test_pkt_len_cfg,
	wifi_test_set_pkt_cnt,
	wifi_test_set_pkt_interval,
	wifi_test_tmp_comp,
	wifi_test_txop_en,
	wifi_test_set_ack,
	wifi_test_set_pkt_content,
	wifi_test_set_retry_cnt,
	wifi_test_set_qos_q,
	wifi_test_set_bw,
	wifi_test_set_gi,
	wifi_test_set_stbc,
	wifi_test_set_ch_freq,
	wifi_test_set_rifs,
	wifi_test_tr_switch,
	wifi_test_rf_sx_en,
	wifi_test_pll_en,
	wifi_test_slow_clk_en,
	wifi_test_adc_clk_mode,
	wifi_test_measure_mode,
	wifi_test_vlt_comp,
	wifi_test_get_dpd_tx_gain,
	wifi_test_dpd_en,
	wifi_test_tssi_en,
	wifi_test_get_tx_gain_code,
	wifi_test_tx_pwr_mode,
};

static INT32 wifi_test_get_tx_cnt(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;

	memcpy(&param->data, &ATECtrl->TxDoneCount, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*param), ret);
	return ret;
}

static INT32 wifi_test_get_tx_ok_cnt(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_rx_ok_cnt(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)(&cmd_hdr->data);
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT32 mcs_err = 0;
	UINT32 mac_mdrdy = 0;
	UINT32 value = 0;

	/* Get latest FCSErr. sync iwpriv & HQA same FCSErr result */
	NICUpdateRawCounters(pAd);
	mcs_err = ATECtrl->RxMacFCSErrCount;
	RTMP_IO_READ32(pAd, MIB_MSDR10, &value);
	ATECtrl->RxMacMdrdyCount += value;
	mac_mdrdy  = ATECtrl->RxMacMdrdyCount;
	value = mac_mdrdy - mcs_err;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, rx_ok: %u, err: %u\n", __func__, value, ATECtrl->RxMacFCSErrCount));
	memcpy(&param->data, &value, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*param), ret);
	return ret;
}

static INT32 wifi_test_rx_err_cnt(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)(&cmd_hdr->data);
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT32 mcs_err = 0;

	/* Get latest FCSErr. sync iwpriv & HQA same FCSErr result */
	NICUpdateRawCounters(pAd);
	mcs_err = ATECtrl->RxMacFCSErrCount;
	memcpy(&param->data, &mcs_err, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, mcs_err: %u\n", __func__, mcs_err));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*param), ret);
	return ret;
}

static INT32 wifi_test_get_result_len(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_get_trx_iq_cal(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_get_tssi_cal(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_get_dpd_cal(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_rxv_dump(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_get_rx_stat(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_get_rpi_ipi(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_get_tmp_sensor(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*param), ret);
	return ret;
}

static INT32 wifi_test_get_vlt_sensor(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*param), ret);
	return ret;
}

static INT32 wifi_test_read_efuse(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*param), ret);
	return ret;
}

static INT32 wifi_test_get_rx_rssi(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT32 value = 0;

	/* HqaRxStat.DriverRxCount = OS_NTOHL(ATECtrl->RxTotalCnt); */
	/* HqaRxStat.RCPI0 = OS_NTOHL(ATECtrl->RCPI0); */
	/* HqaRxStat.RCPI1 = OS_NTOHL(ATECtrl->RCPI1); */
	/* HqaRxStat.FreqOffsetFromRX = OS_NTOHL(ATECtrl->FreqOffsetFromRx); */
	value |= (0x000000ff & ATECtrl->LastRssi0);
	value = value << 8;
	value |= (0x000000ff & ATECtrl->MaxRssi0);
	value = value << 8;
	value |= (0x000000ff & ATECtrl->MinRssi0);
	value = value << 8;
	value |= (0x000000ff & ATECtrl->AvgRssi0);
	memcpy(&param->data, &value, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, value:0x%x, last:0x%x,max:0x%x, min:0x%x, avg:0x%x\n",
			 __func__, value, ATECtrl->LastRssi0, ATECtrl->MaxRssi0, ATECtrl->MinRssi0, ATECtrl->AvgRssi0));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*param), ret);
	return ret;
}

static INT32 wifi_test_get_fw_info(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0x0001;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	memcpy(&param->data, &val, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*param), ret);
	return ret;
}

static INT32 wifi_test_get_dri_info(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0x7636;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	memcpy(&param->data, &val, 4);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*param), ret);
	return ret;
}

static INT32 wifi_test_get_pwr_detector(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_get_phy_rssi(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT32 IBRssi0, IBRssi1, WBRssi0, WBRssi1;
	UINT32 value;

	RTMP_IO_READ32(pAd, RO_AGC_DEBUG_2, &value);
	IBRssi0 = (value & 0xFF000000) >> 24;

	if (IBRssi0 > 128)
		IBRssi0 -= 256;

	WBRssi0 = (value & 0x00FF0000) >> 16;

	if (WBRssi0 > 128)
		WBRssi0 -= 256;

	/* Rx1 Rssi */
	IBRssi1 = (value & 0x0000FF00) >> 8;

	if (IBRssi1 > 128)
		IBRssi1 -= 256;

	WBRssi1 = (value & 0x000000FF);

	if (WBRssi1 > 128)
		WBRssi1 -= 256;

	value = 0;
	value |= (0x000000ff & WBRssi0);
	value <<= 16;
	value |= (0x000000ff & IBRssi0);
	memcpy(&param->data, &value, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, Return: 0x%x, WBRssi:0x%x, IBRssi:0x%x\n", __func__, value, WBRssi0, IBRssi0));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*param), ret);
	return ret;
}

static INT32 wifi_test_get_rx_rssi1(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT32 value = 0;

	/* HqaRxStat.DriverRxCount = OS_NTOHL(ATECtrl->RxTotalCnt); */
	/* HqaRxStat.RCPI0 = OS_NTOHL(ATECtrl->RCPI0); */
	/* HqaRxStat.RCPI1 = OS_NTOHL(ATECtrl->RCPI1); */
	/* HqaRxStat.FreqOffsetFromRX = OS_NTOHL(ATECtrl->FreqOffsetFromRx); */
	value |= (0x000000ff & ATECtrl->LastRssi1);
	value = value << 8;
	value |= (0x000000ff & ATECtrl->MaxRssi1);
	value = value << 8;
	value |= (0x000000ff & ATECtrl->MinRssi1);
	value = value << 8;
	value |= (0x000000ff & ATECtrl->AvgRssi1);
	memcpy(&param->data, &value, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, value:0x%x, last:0x%x,max:0x%x, min:0x%x, avg:0x%x\n",
			 __func__, value, ATECtrl->LastRssi1, ATECtrl->MaxRssi1, ATECtrl->MinRssi1, ATECtrl->AvgRssi1));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*param), ret);
	return ret;
}

static INT32 wifi_test_pwr_cfg_tx1(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	UINT32 value = 0;
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, sizeof(val));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, val:0x%x\n", __func__, val));
	pAd->ATECtrl.TxPower1 = val;
	/* TODO Add in the setting of another Tx Power, txpwr1*/
	ret = ATEOp->SetTxPower0(pAd, val, 0);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return value;
}

static INT32 wifi_test_tx_path_sel(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Val: %x\n", __func__, val));
	ret = ATEOp->SetTxAntenna(pAd, (CHAR)val);
	return ret;
}

static INT32 wifi_test_rx_path_sel(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Val: %x\n", __func__, val));
	ret = ATEOp->SetRxAntenna(pAd, (CHAR)val);
	return ret;
}

static META_CMD_HANDLER WIFI_TEST_CMD_SET2[] = {
	wifi_test_get_tx_cnt,	/* 32 */
	wifi_test_get_tx_ok_cnt,
	wifi_test_rx_ok_cnt,
	wifi_test_rx_err_cnt,
	wifi_test_get_result_len, /* For next 6 command */
	wifi_test_get_trx_iq_cal,
	wifi_test_get_tssi_cal,
	wifi_test_get_dpd_cal,
	wifi_test_rxv_dump,
	wifi_test_get_rx_stat,
	wifi_test_get_rpi_ipi,
	wifi_test_get_tmp_sensor,
	wifi_test_get_vlt_sensor,
	wifi_test_read_efuse,
	wifi_test_get_rx_rssi,
	wifi_test_get_fw_info,
	wifi_test_get_dri_info,
	wifi_test_get_pwr_detector,
	wifi_test_get_phy_rssi, /* 50 */
	wifi_test_pwr_cfg_tx1,
	wifi_test_tx_path_sel,
	wifi_test_rx_path_sel,
	wifi_test_get_rx_rssi1,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

static INT32 wifi_test_set_dpd(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_cw_mode(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, val: %d, Ant: 0x%x\n", __func__, val, ATECtrl->TxAntennaSel));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_jp_ch_flt_en(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_write_efuse(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */
	UINT32 offset = 0;

	FUNC_IDX_GET_SUBFIELD(param->idx, offset);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_ra(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_ta(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_rx_match_rule(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_ch_bw(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);

	switch (val) {
	case 0:
		ATECtrl->BW = BW_20;
		break;

	case 1:
		ATECtrl->BW = BW_40;
		break;

	case 2:
		ATECtrl->BW = BW_80;
		break;

	case 3:
	default:
		/* Not support yet */
		return -1;
	}

	ATECtrl->ControlChl = ATECtrl->Channel;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Channel = %d, ContrlChl: 0x%x, BW = %d\n", __func__, ATECtrl->Channel, ATECtrl->ControlChl, ATECtrl->BW));
	ret = ATEOp->SetBW(pAd, ATECtrl->BW);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_data_bw(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	/* struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp; */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, val:0x%x\n", __func__, val));

	switch (val) {
	case 0:
		/* ATECtrl->PerPktBW = BW_20; */
		break;

	case 1:
		/* ATECtrl->PerPktBW = BW_40; */
		break;

	case 2:
		/* ATECtrl->PerPktBW = BW_80; */
		break;

	case 3:
	default:
		/* Not support yet */
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, Invalid Paramter(%x)\n", __func__, val));
		ret = -1;
		break;
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Channel = %d, BW = %d\n", __func__, ATECtrl->Channel, ATECtrl->BW));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_primary(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	/* struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp; */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	ATECtrl->ControlChl = val;
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_encode_mode(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	memcpy((PUCHAR)&val, &param->data, 4);
	/* ATECtrl->Ldpc = val; */
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_jmode(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _PARAM_MTK_WIFI_TEST *param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = NDIS_STATUS_FAILURE;
	INT32 val = 0;
	INT32 mode = 8;
	INT32 pwr1 = 0xf;
	INT32 pwr2 = 0;
	CHAR txpwr = 0;

	memcpy((PUCHAR)&val, &param->data, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, val: %d, Ant: 0x%x\n", __func__, val, ATECtrl->TxAntennaSel));

	/* TxAntennaSel, 0: All 1:TX0 2:TX1 */
	/* val, 0: disable, 1.5M single tone, 2.10M single tone */
	switch (val) {
	case 1:
		if (ATECtrl->TxAntennaSel == 1) {
			mode = WF0_TX_ONE_TONE_5M;
			txpwr = ATECtrl->TxPower0;
		} else if (ATECtrl->TxAntennaSel == 2) {
			mode = WF1_TX_ONE_TONE_5M;
			txpwr = ATECtrl->TxPower1;
		} else
			goto meta_jmode_done;

		ret = ATEOp->StartTxTone(pAd, mode);
		break;

	case 2:
		if (ATECtrl->TxAntennaSel == 1) {
			mode = WF0_TX_ONE_TONE_10M;
			txpwr = ATECtrl->TxPower0;
		} else if (ATECtrl->TxAntennaSel == 2) {
			mode = WF1_TX_ONE_TONE_10M;
			txpwr = ATECtrl->TxPower1;
		} else
			goto meta_jmode_done;

		ret = ATEOp->StartTxTone(pAd, mode);
		break;

	default:
		ret = ATEOp->StopTxTone(pAd);
		goto meta_jmode_done;
	}

	if (txpwr > 30)
		pwr2 = (txpwr - 30) << 1;
	else {
		pwr1 = (txpwr & 0x1e) >> 1;
		pwr2 = (txpwr & 0x01) << 1;
	}

	ret = ATEOp->SetTxTonePower(pAd, pwr1, pwr2);
meta_jmode_done:
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_inter_cap_cont(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_inter_cap_trig(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_inter_cap_size(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* INT32 val = 0; */

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static INT32 wifi_test_set_inter_cap_trig_offset(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static META_CMD_HANDLER WIFI_TEST_CMD_SET3[] = {
	wifi_test_set_dpd,	/* 64 */
	wifi_test_set_cw_mode,
	wifi_test_jp_ch_flt_en,
	wifi_test_write_efuse,
	wifi_test_set_ra,
	wifi_test_set_ta,
	wifi_test_set_rx_match_rule,
	wifi_test_set_ch_bw,
	wifi_test_set_data_bw,
	wifi_test_set_primary,
	wifi_test_set_encode_mode,
	wifi_test_set_jmode,
	wifi_test_set_inter_cap_cont,
	wifi_test_set_inter_cap_trig,
	wifi_test_set_inter_cap_size,
	wifi_test_set_inter_cap_trig_offset,/* 84 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL, /* 95 */
};

static INT32 wifi_test_set_mod_patch(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	/* struct _PARAM_MTK_WIFI_TEST* param = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data; */
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32 ret = NDIS_STATUS_SUCCESS;

	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, ret);
	return ret;
}

static META_CMD_HANDLER WIFI_TEST_CMD_SET5[] = {
	wifi_test_set_mod_patch,	/* 128 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};


/*
 *	Command Set for OID_CUSTOM_MTK_WIFI_TEST
 */
static META_CMD_HANDLER *WIFI_TEST_CMD_SETS[] = {
	WIFI_TEST_CMD_SET1,	/* 0~31 */
	WIFI_TEST_CMD_SET2, /* 32~63 */
	WIFI_TEST_CMD_SET3,	/* 64~95 */
	NULL,	/* 96~127 */
	WIFI_TEST_CMD_SET5,	/* 128~149 */
};
/*
 *	Buffer Length: 4
 *	Return value: Chip Number, ex:0x00006620
 */
static INT32 oid_if_version(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _META_CMD_HDR *cmd = cmd_hdr;
	INT32 status = NDIS_STATUS_SUCCESS;
	UINT32 value = 0x00006620;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("%s: oid = 0x%04x, len:0x%x!\n", __func__, cmd_hdr->oid, cmd_hdr->len_in));
	memcpy(&cmd->data, &value, 4);
	status =  resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 4, status);
	return status;
}

/*
 *	Both SET/QUERY, using struct _PARAM_CUSTOM_MCR_RW for payload
 */
static INT32 oid_mcr_rw(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	PARAM_CUSTOM_MCR_RW *cmd = (PARAM_CUSTOM_MCR_RW *)cmd_hdr->data;
	INT32 status = NDIS_STATUS_SUCCESS;
	UINT32 value = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("%s: MCR Access Offset = 0x%x\n", __func__, cmd->offset));

	/* Check if access F/W Domain MCR (due to WiFiSYS is placed from 0x6000-0000*/
	if (ioctl_cmd == MTPRIV_IOCTL_META_SET) {
		if (cmd->offset & 0xFFFF0000) {
			RTMP_IO_WRITE32(pAd, cmd->offset, cmd->data);
			RTMP_IO_READ32(pAd, cmd->offset, &value);
		} else {
		}

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: MCR Set Offset 0x%x, write in data: 0x%x, read after write: 0x%x\n", __func__, cmd->offset, cmd->data, value));
	} else if (ioctl_cmd == MTPRIV_IOCTL_META_QUERY) {
		if (cmd->offset & 0xFFFF0000)
			RTMP_IO_READ32(pAd, cmd->offset, &value);
		else {
		}

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("%s: MCR Query Offset = 0x%02x, Val: 0x%x\n", __func__, cmd->offset, value));
	} else {
		status = NDIS_STATUS_INVALID_DATA;
		goto _mcr_rw_err;
	}

	memcpy(&cmd->data, &value, 4);
_mcr_rw_err:
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*cmd), status);
	return status;
}

/*
 *	Both SET/QUERY, using struct _PARAM_CUSTOM_EEPROM_RW for payload
 */
static INT32 oid_eeprom_rw(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	PARAM_CUSTOM_EEPROM_RW *cmd = (PARAM_CUSTOM_EEPROM_RW *)cmd_hdr->data;
	INT32 status = NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("\n%s: EEPROM Write Data = 0x%02x !\n", __func__, cmd->data));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*cmd), status);
	return status;
}

/*
 *	QUERY, using struct _PARAM_CUSTOM_EFUSE_RW for payload
 */
static INT32 oid_efuse_rw(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	PARAM_CUSTOM_EFUSE_RW *cmd = (PARAM_CUSTOM_EFUSE_RW *)cmd_hdr->data;
	INT32 status = NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("\n%s: EFUSE Write Data = 0x%02x !\n", __func__, cmd->data));
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*cmd), status);
	return status;
}

/*
 *	Like ATEStart (?)
 */
static INT32 oid_test_mode(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	INT32 status = NDIS_STATUS_SUCCESS;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("%s: oid = 0x%04x, len:0x%x\n", __func__, cmd_hdr->oid, cmd_hdr->len_in));
	ATECtrl->bQAEnabled = TRUE;
	/* Prepare feedback as soon as we can to avoid QA timeout. */
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, status);
	/* Initialization */
	g_SUB_TEST_STATE = WIFI_SUB_TEST_STANDBY;
	ATECtrl->Channel = 1;
	ATECtrl->ControlChl = 1;
	status = ATEOp->ATEStart(pAd);
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, status);
	return status;
}

/*
 *	Like ATEStop (?)
 */
static INT32 oid_abort_test_mode(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	INT32 status = NDIS_STATUS_SUCCESS;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("%s: oid = 0x%04x, len:0x%x!\n", __func__, cmd_hdr->oid, cmd_hdr->len_in));
	ATECtrl->bQAEnabled = FALSE;
	/* Prepare feedback as soon as we can to avoid QA timeout. */
	resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 0, status);
	status = ATEOp->ATEStop(pAd);
	return status;
}

/*
 *	Bunch of further commands
 */
static INT32 oid_mt_wifi_test(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	INT32 status = NDIS_STATUS_SUCCESS;
	struct _PARAM_MTK_WIFI_TEST *cmd = (struct _PARAM_MTK_WIFI_TEST *)cmd_hdr->data;
	INT32 idx = 0;
	INT32 set_idx = 0;
	INT32 handle_idx = 0;

	FUNC_IDX_GET_IDX(cmd->idx, idx);
	set_idx = ((idx & ~MT_META_WIFI_TEST_CMD_MASK) >> 5)&MT_META_WIFI_TEST_SET_MASK;
	handle_idx = idx & MT_META_WIFI_TEST_CMD_MASK;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: cmd->idx:0x%x ,set_idx:0x%04x, handle_idx:0x%04x\n", __func__, cmd->idx, set_idx, handle_idx));

	if (!WIFI_TEST_CMD_SETS[set_idx])
		goto err_handle;

	if (!(*WIFI_TEST_CMD_SETS[set_idx][handle_idx]))
		goto err_handle;

	(*WIFI_TEST_CMD_SETS[set_idx][handle_idx])(ioctl_cmd, pAd, WRQ, cmd_hdr);
	return status;
err_handle:
	return NDIS_STATUS_INVALID_DATA;
}

static INT32 oid_cfg_src_type(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _META_CMD_HDR *cmd = cmd_hdr;
	INT32 status = NDIS_STATUS_SUCCESS;
	UINT32 value = 1; /* 1 For NVRAM */

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("%s: len:0x%x!\n", __func__, cmd->len_in));
	memcpy(&cmd->data, &value, 4);
	status =  resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 4, status);
	return status;
}

static INT32 oid_eeprom_type(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	struct _META_CMD_HDR *cmd = cmd_hdr;
	INT32 status = NDIS_STATUS_SUCCESS;
	UINT32 value = 0; /* 0 For EEPROM Not Present */

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("%s: len:0x%x!\n", __func__, cmd_hdr->len_in));
	memcpy(&cmd->data, &value, 4);
	status =  resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, 4, status);
	return status;
}

static INT32 oid_nvram_rw(INT32 ioctl_cmd, PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _META_CMD_HDR *cmd_hdr)
{
	PARAM_CUSTOM_NVRAM_RW *cmd = (PARAM_CUSTOM_NVRAM_RW *)cmd_hdr->data;
	INT32 status = NDIS_STATUS_SUCCESS;
	UINT16 value = 0;
	UINT8 idx = cmd->idx << 1;

	if (ioctl_cmd == MTPRIV_IOCTL_META_SET) {
		memcpy((PUCHAR)&value, &cmd->data, 4);
		pAd->EEPROMImage[idx] = (0xff00 & value) >> 8;
		pAd->EEPROMImage[idx + 1] = (0x00ff & value);
		rtmp_ee_write_to_bin(pAd);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s-SET, method:0x%x ,offset:0x%x, valu:0x%x, EEPROMImage:0x%x\n", __func__, cmd->method, idx, value, pAd->EEPROMImage[idx]));
	} else if (ioctl_cmd == MTPRIV_IOCTL_META_QUERY) {
		if (cmd->method == PARAM_EEPROM_READ_METHOD_READ) {
			value |= pAd->EEPROMImage[idx];
			value <<= 8;
			value |= pAd->EEPROMImage[idx + 1];
		} else if (cmd->method == PARAM_EEPROM_READ_METHOD_GETSIZE)
			value = EEPROM_SIZE;

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,  ("%s-QUREY, method:0x%x ,offset:0x%x, valu:0x%x\n", __func__, cmd->method, idx, value));
	} else {
	}

	memcpy(&cmd->data, &value, 4);
	status =  resp_to_meta(ioctl_cmd, cmd_hdr, WRQ, sizeof(*cmd), status);
	return status;
}

/*
 *	Handler for Each OID
 */
static META_CMD_HANDLER OID_HANDLE[] = {
	oid_if_version,
	oid_mcr_rw,
	oid_eeprom_rw,
	oid_efuse_rw,
	oid_test_mode,
	oid_abort_test_mode,
	oid_mt_wifi_test,
	oid_cfg_src_type,
	oid_eeprom_type,
	oid_nvram_rw,
};

INT32 do_meta_cmd(INT32 ioctl_cmd, PRTMP_ADAPTER	pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, RTMP_STRING *wrq_name)
{
	INT32 Status = NDIS_STATUS_SUCCESS;
	struct _META_CMD_HDR *cmd_hdr;
	UINT32 oid = 0;
	INT32 ret = 0;

	os_alloc_mem_suspend(pAd, (UCHAR **)&cmd_hdr, sizeof(*cmd_hdr));

	if (!cmd_hdr) {
		Status = -ENOMEM;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,   ("%s, cmd_hdr NULL\n", __func__));
		goto ERROR0;
	}

	if (!WRQ) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,   ("%s. wrq is null\n", __func__));
		goto ERROR0;
	}

	os_zero_mem(cmd_hdr, sizeof(*cmd_hdr));
	Status = copy_from_user((PUCHAR)cmd_hdr, WRQ->u.data.pointer, WRQ->u.data.length);
#ifdef _DUMP_HEX_
	{
		UCHAR *pSrcBufVA = WRQ->u.data.pointer;
		UINT32 SrcBufLen = WRQ->u.data.length;
		unsigned char *pt;
		INT32 x;

		pt = pSrcBufVA;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %p, len = %d\n",
				 "META_IOCTL Content", pSrcBufVA, SrcBufLen));

		for (x = 0; x < SrcBufLen; x++) {
			if (x % 16 == 0)
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x%04x : ", x));

			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x ", (unsigned char)pt[x]));

			if (x % 16 == 15)
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}
#endif

	if (Status) {
		Status = -EFAULT;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, copy_from_user error\n", __func__));
		goto ERROR1;
	}

	oid = cmd_hdr->oid;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() ==>,OID:0x%x, len:0x%x\n", __func__, oid, WRQ->u.data.length));

	switch (oid) {
	case OID_CUSTOM_IF_VER:
		ret = (*OID_HANDLE[0])(ioctl_cmd, pAd, WRQ, cmd_hdr);
		break;

	case OID_CUSTOM_MCR_RW:
		ret = (*OID_HANDLE[1])(ioctl_cmd, pAd, WRQ, cmd_hdr);
		break;

	case OID_CUSTOM_EEPROM_RW:
		ret = (*OID_HANDLE[2])(ioctl_cmd, pAd, WRQ, cmd_hdr);
		break;

	case OID_CUSTOM_EFUSE_RW:
		ret = (*OID_HANDLE[3])(ioctl_cmd, pAd, WRQ, cmd_hdr);
		break;

	case OID_CUSTOM_TEST_MODE:
		ret = (*OID_HANDLE[4])(ioctl_cmd, pAd, WRQ, cmd_hdr);
		break;

	case OID_CUSTOM_ABORT_TEST_MODE:
		ret = (*OID_HANDLE[5])(ioctl_cmd, pAd, WRQ, cmd_hdr);
		break;

	case OID_CUSTOM_MTK_WIFI_TEST:
		ret = (*OID_HANDLE[6])(ioctl_cmd, pAd, WRQ, cmd_hdr);
		break;

	case OID_CUSTOM_CFG_SRC_TYPE:
		ret = (*OID_HANDLE[7])(ioctl_cmd, pAd, WRQ, cmd_hdr);
		break;

	case OID_CUSTOM_EEPROM_TYPE:
		ret = (*OID_HANDLE[8])(ioctl_cmd, pAd, WRQ, cmd_hdr);
		break;

	case OID_CUSTOM_MTK_NVRAM_RW:
		ret = (*OID_HANDLE[9])(ioctl_cmd, pAd, WRQ, cmd_hdr);
		break;

	default:
		Status = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknown oid number of AT command = %x\n", oid));
		goto ERROR1;
	}

ERROR1:
	os_free_mem(cmd_hdr);
ERROR0:
	return Status;
}
