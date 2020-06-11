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
	smartant.c

	Abstract:
	Smart Antenna related functions.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"


#ifdef SMART_ANTENNA


/*
	Following register offset definition used for GPIO95_72
*/
#define BIT(_X) (_X) /* (1 << (_X)) */
#define SA_HDR_PIN(_X, _Y) (((_X) << 8) | (_Y))

#define SA_REG_GPIO_BASE			0xb0000600
#define SA_REG_GPIO_MODE			0xb0000060

#define SA_REG_GPIO_95_72_DATA		0x98
#define SA_REG_GPIO_OFFSET_DIR		0x4	/* When access the GPIO_XX_XX_DIR, need to add GPIO_XX_XX_DATA first */
#define SA_REG_GPIO_OFFSET_SET		0xc	/* When access the GPIO_XX_XX_SET, need to add GPIO_XX_XX_DATA first */
#define SA_REG_GPIO_OFFSET_RESET	0x10/* When access the GPIO_XX_XX_RESET, need to add GPIO_XX_XX_DATA first */
#define SA_REG_GPIO_DIR_OUTPUT_MASK	0xfff

#define SA_GPIO_72	BIT(0)   /* 1 */
#define SA_GPIO_73	BIT(1)   /* 2 */
#define SA_GPIO_74	BIT(2)   /* 4 */
#define SA_GPIO_75	BIT(3)   /* 8 */
#define SA_GPIO_76	BIT(4)   /* 10 */
#define SA_GPIO_77	BIT(5)   /* 20 */
#define SA_GPIO_78	BIT(6)   /* 40 */
#define SA_GPIO_79	BIT(7)   /* 80 */
#define SA_GPIO_80	BIT(8)   /* 100 */
#define SA_GPIO_81	BIT(9)   /* 200 */
#define SA_GPIO_82	BIT(10)  /* 400 */
#define SA_GPIO_83	BIT(11)  /* 800 */


#define GE2_TXD0	SA_GPIO_72  /* 1 */
#define GE2_TXD1	SA_GPIO_73  /* 2 */
#define GE2_TXD2	SA_GPIO_74  /* 4 */
#define GE2_TXD3	SA_GPIO_75  /* 8 */

#define GE2_TXEN	SA_GPIO_76  /* 10 */
#define GE2_TXCLK	SA_GPIO_77  /* 20 */

#define GE2_RXD0	SA_GPIO_78  /* 40 */
#define GE2_RXD1	SA_GPIO_79  /* 80 */
#define GE2_RXD2	SA_GPIO_80  /* 100 */
#define GE2_RXD3	SA_GPIO_81  /* 200 */

#define GE2_RXDV	SA_GPIO_82  /* 400 */
#define GE2_RXCLK	SA_GPIO_83  /* 800 */

#define SA_GPIO_VALUE_MASK	(GE2_TXD1 | GE2_TXD3 | GE2_TXEN | GE2_RXD3 | GE2_RXD2 | GE2_RXD1 | GE2_RXD0 | GE2_RXDV | GE2_RXCLK)

#define GND			0
#define VC			0

/* The NULL terminator for the agsp list */
static RTMP_SA_AGSP_MAP AGSP_ENTRY_END = {0, 0, 0};
static UINT32 AGSP_TRAIN_SEQ_END;

/* The default entry table for the agsp list */
static RTMP_SA_AGSP_MAP DefaultAGSP[] = {
	{1, 0, GND},
	{2, SA_REG_GPIO_95_72_DATA, GE2_RXDV},
	{3, SA_REG_GPIO_95_72_DATA, GE2_RXD1},
	{4, SA_REG_GPIO_95_72_DATA, GE2_RXD3},
	{5, SA_REG_GPIO_95_72_DATA, GE2_TXEN},
	{6, SA_REG_GPIO_95_72_DATA, GE2_TXD1},
	{7, SA_REG_GPIO_95_72_DATA, GE2_TXD3},
	{0, 0, 0}
};


/*
	By default configuration, RT3662 only use two txAntenna(tx0, tx1)
*/
static UINT32 DefaultTrainSeq[] = {
	SA_HDR_PIN(2, 5)/* 0x410 */, SA_HDR_PIN(2, 6) /* 0x402 */, SA_HDR_PIN(2, 7) /* 0x408 */,
	SA_HDR_PIN(3, 5)/* 0x090 */, SA_HDR_PIN(3, 6) /* 0x082 */, SA_HDR_PIN(3, 7) /* 0x88 */,
	SA_HDR_PIN(4, 5)/* 0x210 */, SA_HDR_PIN(4, 6) /* 0x202 */, SA_HDR_PIN(4, 7) /* 0x208 */,
	0
};

static char *SA_MODE[] = {"None", "Manual", "OneShot", "Auto"};
static char *phyMode[4] = {"CCK", "OFDM", "HTMIX", "GF"};
static char *saStage[4] = {"None", "Init", "Confirm", "Monitor"};


VOID sa_switch_exec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);


DECLARE_TIMER_FUNCTION(sa_switch_exec);
BUILD_TIMER_FUNCTION(sa_switch_exec);


/*
	When use this function
		1. User shall guarantee the input "pSrc" is not NULL
		1. This tokenParser will break the original content of the pSrc
		2. If the token is parsing finished, the pSrc shall return as NULL
		3. This function always return a non-NULL pointer to indicate some
			position of the pSrc
		4. The return pointer may be a string with strlen() as 0
*/
char *tokenParser(char **pSrc, char delim)
{
	char *token, *pos;
	int i, len = strlen(*pSrc);
	token = pos = *pSrc;

	for (i = 0; i < len; i++) {
		if (pos[i] == delim) {
			pos[i] = '\0';
			break;
		}
	}

	*pSrc = ((i != len) ? (*pSrc + i + 1) : NULL);
	return token;
}


int rtstrtomac(RTMP_STRING *macStr, UCHAR *macAddr, char delimiter)
{
	int mac_len, i;
	RTMP_STRING *ptr = macStr;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(macStr);
	if (mac_len < 17) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR, ("%s : invalid length (%d)\n", __func__, mac_len));
		return FALSE;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++) {
		AtoH(ptr, &macAddr[i], 1);
		ptr = ptr + 3;
	}

	return TRUE;
}


BOOLEAN is_valid_agsp_entry(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_SA_AGSP_MAP *pAgspEntry)
{
	return TRUE;
	/*
		if (pAgspEntry->gpioBit <= 32)
			return FALSE;

		if (pAgspEntry->hdrPin > 255)
			return FALSE;

		if (pAgspEntry->regOffset)
			return FALSE;
	*/
}


BOOLEAN is_valid_train_entry(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT * pSAParam,
	IN RTMP_SA_TRAINING_PARAM * pSAStaEntry)
{
	MAC_TABLE_ENTRY *pMacEntry;

	/* check mac address */
	if (NdisEqualMemory(&pSAStaEntry->macAddr[0], ZERO_MAC_ADDR, MAC_ADDR_LEN)) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("%s():pSAStaEntry->macAddr is all zero!\n", __func__));
		return FALSE;
	}

	/* check mac entry */
	pMacEntry = MacTableLookup(pAd, &pSAStaEntry->macAddr[0]);

	if (pMacEntry == NULL) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("%s():Cannot found Sta(%02x:%02x:%02x:%02x:%02x:%02x) in MacTable\n",
				  __func__, PRINT_MAC(pSAStaEntry->macAddr)));
		return FALSE;
	}

	/* check antPattern */
	if ((pSAParam->saMode == SA_MODE_MANUAL) &&
		(pSAStaEntry->curAntPattern == 0)) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("%s():SATrainMode is manual but not set antPattern\n",
				  __func__));
		return FALSE;
	}

	/* TODO: Other check?? */
	pSAStaEntry->pMacEntry = pMacEntry;
	return TRUE;
}


BOOLEAN is_ant_equal(UINT32 ant1, UINT32 ant2)
{
	UINT32 swapAnt1;
	swapAnt1 = ((ant1 & 0xff) << 8) | ((ant1 >> 8) & 0xff);
	return ((ant1 == ant2) || (swapAnt1 == ant2));
}


VOID reset_mac_entry_stats(
	IN MAC_TABLE_ENTRY *pEntry)
{
	/* NdisZeroMemory(pEntry->baseAvgRSSI, 3); */
	NdisZeroMemory(pEntry->curRSSI, 3 * sizeof(UINT32));
	NdisZeroMemory(pEntry->cntRSSI, 3 * sizeof(UINT32));
	NdisZeroMemory(pEntry->rssi_zero_cnt, 3 * sizeof(UINT32));
	NdisZeroMemory(pEntry->sumSNR, 3 * sizeof(INT32));
	NdisZeroMemory(pEntry->cntSNR, 3 * sizeof(INT32));
	NdisZeroMemory(pEntry->sumPreSNR, 3 * sizeof(INT32));
	NdisZeroMemory(pEntry->cntPreSNR, 3 * sizeof(INT32));
	pEntry->hwTxSucCnt = 0;
	pEntry->hwTxRtyCnt = 0;
	pEntry->saTxCnt = 0;
	pEntry->saRxCnt = 0;
	NdisZeroMemory(&pEntry->mcsUsage[0], 33 * sizeof(ULONG));
	NdisGetSystemUpTime(&pEntry->calcTime);

	if (pEntry->HTPhyMode.field.MCS <= 32) {
		pEntry->mcsInUse = pEntry->HTPhyMode.field.MCS;
		pEntry->curMcsApplyTime = pEntry->calcTime;
		/* printk("%s(): change the pEntry->curMcsApplyTime=0x%lx\n", __func__, pEntry->curMcsApplyTime); */
	}
}


VOID sa_calc_rx_signal_info(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY *pEntry)
{
	if (pEntry->saRxCnt) {
		int i;
		INT32 avgRssi, avgSNR, avgPreSNR;
#error "Need to fix ConvertToRssi() before compile this feature"

		for (i = 0; i < 3; i++) {
			if (pEntry->cntRSSI[i]) {
				pEntry->curAvgRSSI[i] = ConvertToRssi(pAd, pEntry->curRSSI[i] / pEntry->cntRSSI[i], i);
				pEntry->avgRssi[i] = pEntry->curAvgRSSI[i];
			} else
				pEntry->avgRssi[i] = pEntry->curAvgRSSI[i] = 0;

			if (pEntry->cntSNR[i]) {
				avgSNR = pEntry->sumSNR[i] / pEntry->cntSNR[i];
				pEntry->avgSNR[i] = (avgSNR * 1881) / 10000;
			} else
				pEntry->avgSNR[i] = 0;

			if (pEntry->cntPreSNR[i]) {
				avgPreSNR = pEntry->sumPreSNR[i] / pEntry->cntPreSNR[i];
				pEntry->avgPreSNR[i] = ((avgPreSNR / 4) + 22);
			} else
				pEntry->avgPreSNR[i] = 0;
		}

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
				 ("Wcid:%d RSSI=%d,%d,%d cntRSSI=%d,%d,%d, prevRssi=%d\n",
				  pEntry->wcid,
				  pEntry->avgRssi[0], pEntry->avgRssi[1], pEntry->avgRssi[2],
				  pEntry->cntRSSI[0], pEntry->cntRSSI[1], pEntry->cntRSSI[2],
				  pEntry->prevAvgRssi[0]));
	}
}


VOID sa_dump_stat_result(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY *pEntry)
{
	INT32 HwPER = -1;
	int mcsIdx;
	unsigned long tv, iv;

	if (pEntry->hwTxSucCnt)
		HwPER = (pEntry->hwTxRtyCnt * 1000) / pEntry->hwTxSucCnt;

	printk("%02x:%02x:%02x:%02x:%02x:%02x    ", PRINT_MAC(pEntry->Addr));
	printk("%6d  %6d  %6d  %6d\t%d",
		   pEntry->saTxCnt, pEntry->hwTxSucCnt,
		   pEntry->hwTxRtyCnt, pEntry->saRxCnt, HwPER);
	printk("\t%2d,%2d  %2d,%2d\t",
		   pEntry->avgSNR[0], pEntry->avgSNR[1],
		   pEntry->avgRssi[0], pEntry->avgRssi[1]);

	/* Update current mcs usage cnt first */
	if ((pEntry->mcsInUse >= 0) && (pEntry->mcsInUse <= 32)) {
		NdisGetSystemUpTime(&tv);

		/* if smaller than 1 jiffies, we give it one. */
		if (tv >= pEntry->curMcsApplyTime)
			iv = tv - pEntry->curMcsApplyTime;
		else
			iv = 0xffffffff - pEntry->curMcsApplyTime + tv;

		if (iv == 0)
			iv++;

		pEntry->mcsUsage[pEntry->mcsInUse] += iv;
		/* printk("%s(): Update the mcsUsage[%d]=%ld, TimeInterval=0x%lx-0x%lx!\n", */
		/* __func__, pEntry->mcsInUse, pEntry->mcsUsage[pEntry->mcsInUse], */
		/* pEntry->curMcsApplyTime, tv); */
	}

	/* dump the result out */
	for (mcsIdx = 0; mcsIdx < 33; mcsIdx++) {
		if (pEntry->mcsUsage[mcsIdx] != 0) {
			tv = (pEntry->mcsUsage[mcsIdx] * 1000 / OS_HZ);
			printk("%d:%ld,", mcsIdx, tv);
		}
	}

	printk("\n");
	printk("\n");
}


VOID sa_dump_agsp(RTMP_SA_AGSP_MAP *pAgspList)
{
	RTMP_SA_AGSP_MAP *pAgspEntry;
	int cnt;
	pAgspEntry = pAgspList;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Start Dump AGSP List:\n"));
	cnt = 0;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tHeaderPin  GPIO RegOffset  GPIOBit\n"));

	while (memcmp(pAgspEntry, &AGSP_ENTRY_END, sizeof(RTMP_SA_AGSP_MAP)) !=  0) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t%d\t0x%x\t%d\n",
				 pAgspEntry->hdrPin, pAgspEntry->regOffset, pAgspEntry->gpioBit));
		pAgspEntry++;
	}
}


VOID sa_dump_train_seq(UINT32 *pTrainSeq)
{
	UINT32 *pAntPair = pTrainSeq;
	int dbgcnt = 0;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Start Dump SA TrainingSequence:\n"));

	while (*pAntPair != AGSP_TRAIN_SEQ_END) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t0x%x(%d, %d)\n", *pAntPair,
				 (*pAntPair & 0xff), ((*pAntPair >> 8) & 0xff)));
		pAntPair++;
		dbgcnt++;

		if (dbgcnt > 255)
			break;
	}
}


VOID sa_dump_train_log(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_SA_TRAINING_PARAM * pTrainEntry,
	IN RTMP_SA_TRAIN_LOG_ELEMENT * pTrainLog,
	IN UCHAR txNss)
{
	MAC_TABLE_ENTRY *pMacEntry;
	int i, j;
	int avgRssi, avgSNR, avgPreSNR;
	pMacEntry = pTrainEntry->pMacEntry;

	if (pMacEntry) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("SmartAntTrainLogDump(Ant:0x%x-%d,TimeInterval:0x%lx-0x%lx)\n",
				 pTrainLog->antPattern, pTrainLog->patternOffset,
				 pTrainLog->srtTime, pTrainLog->endTime));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tBW:MCS=%d:%d, txMcs=%d\n",
				 pMacEntry->HTPhyMode.field.BW, pMacEntry->HTPhyMode.field.MCS,
				 pTrainLog->txMcs));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\ttxCnt=%d,noRtyCnt=%d,reTry=%d,rtyFail=%d,PER=%d!\n",
				 pTrainLog->txCnt, pTrainLog->txNoRtyCnt, pTrainLog->txRtyCnt, pTrainLog->txRtyFailCnt, pTrainLog->PER));

		if (pTrainLog->rxCnt) {
			int i;
#error "Need to fix ConvertToRssi() before compile this feature"
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tRxCnt=%d\n", pTrainLog->rxCnt));

			for (i = 0; i < txNss; i++) {
				avgRssi = avgSNR = avgPreSNR = 0;

				if (pTrainLog->sumRSSI[i])
					avgRssi = pTrainLog->sumRSSI[i] / pTrainLog->cntRSSI[i];

				if (pTrainLog->cntSNR[i])
					avgSNR = pTrainLog->sumSNR[i] / pTrainLog->cntSNR[i];

				if (pTrainLog->cntPreSNR[i])
					avgPreSNR = pTrainLog->sumPreSNR[i] / pTrainLog->cntPreSNR[i];

				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tAnt%d:Rssi=%d(%d), SNR=%d(%d), preSNR=%d(%d)\n",
						 i, avgRssi, ConvertToRssi(pAd, avgRssi, i),
						 avgSNR, ((avgSNR * 1881) / 10000),
						 avgPreSNR, ((avgPreSNR / 4) + 22)));
			}
		} else
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tNo valid RxCnt!\n"));

#ifdef SA_DBG

		if (pAd->smartAntDbgOn) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tRssi Distribution:\n"));

			for (i = 0; i < txNss; i++) {
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tRSSI-%d:\n\t\t\t", i));

				for (j  = 0; j < 33; j++) {
					if (j == 16)
						MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\n\t\t\t"));

					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%6d", pTrainLog->rssiDist[i][j]));
				}

				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\n"));
			}

			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tSNR Distribution:\n"));

			for (i = 0; i < txNss; i++) {
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tSNR-%d:\n\t\t\t", i));

				for (j  = 0; j < 33; j++) {
					if (j == 16)
						MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\n\t\t\t"));

					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%6d", pTrainLog->SNRDist[i][j]));
				}

				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\n"));
			}

			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tBF_SNR Distribution:\n"));

			for (i = 0; i < txNss; i++) {
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tBF_SNR-%d:\n\t\t\t", i));

				for (j  = 0; j < 33; j++) {
					if (j == 16)
						MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\n\t\t\t"));

					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%6d", pTrainLog->preSNRDist[i][j]));
				}

				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\n"));
			}
		}

#endif /* SA_DBG // */
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\n\n"));
	}
}


VOID reset_trainlog_stats_record(
	IN RTMP_SA_TRAIN_LOG_ELEMENT * pTrainLog)
{
	unsigned int offset, cleanLen;
	offset = (unsigned int)(&(((RTMP_SA_TRAIN_LOG_ELEMENT *)NULL)->txCnt));
	cleanLen = sizeof(RTMP_SA_TRAIN_LOG_ELEMENT) - offset;
	NdisZeroMemory((UCHAR *)(&pTrainLog->txCnt), cleanLen);
}


int sa_rssi_chk_variant(
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN UCHAR antCnt,
	IN UCHAR diffVal)
{
	return TRUE;
}


BOOLEAN get_gpio_by_hdr_pins(
	IN RTMP_SA_AGSP_MAP *pAgspList,
	IN UCHAR hdrPin,
	OUT SA_GPIO_PAIR * gpioPair)
{
	RTMP_SA_AGSP_MAP *pAgspEntry = pAgspList;

	while (pAgspEntry->hdrPin != 0) {
		if (pAgspEntry->hdrPin == hdrPin) {
			gpioPair->regAddr = pAgspEntry->regOffset;
			gpioPair->bitMask = (1 <<  pAgspEntry->gpioBit);
			return TRUE;
		}

		pAgspEntry++;
	}

	return FALSE;
}


INT32 sa_get_offset_by_antp(
	IN SMART_ANTENNA_STRUCT * pSAParam,
	IN UINT32 antPattern)
{
	UINT32 *pTrainSeq;
	UINT32 revPattern = 0, tmpAnt;
	INT32 offset;
	UCHAR pinVal;
	pTrainSeq = pSAParam->pTrainSeq;

	if (pTrainSeq == NULL)
		return -1;

	revPattern = 0;
	tmpAnt = antPattern;

	do {
		pinVal = (tmpAnt & 0xff);
		revPattern = ((revPattern << 8) | pinVal);
		tmpAnt >>= 8;
	} while (tmpAnt);

	offset = 0;

	do {

		if ((*(pTrainSeq + offset) == antPattern) ||
			(*(pTrainSeq + offset) == revPattern)) {
			/* printk("\t%s():Found it at offset(%d)\n", __func__, offset); */
			return offset;
		}

		offset++;
	} while (offset < pSAParam->trainSeqCnt);

	return -1;
}


int sa_gpio_write(
	IN RTMP_ADAPTER * pAd,
	IN UINT32 oldAntCfg,
	IN UINT32 newAntCfg)
{
	UINT32 regVal = 0, regAddr;
	/* Check the gpio setting first */
	RTMP_SYS_IO_READ32(SA_REG_GPIO_MODE, &regVal);

	if ((regVal & 0x400) != 0x400) {
		regVal |= 0x400;
		RTMP_SYS_IO_WRITE32(SA_REG_GPIO_MODE, regVal);
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_INFO, ("Change as GPIO Mode(0x%x)\n", regVal));
		RTMP_SYS_IO_READ32(SA_REG_GPIO_MODE, &regVal);
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_INFO, ("After Change, now GPIO_MODE value is 0x%x\n", regVal));
	}

	/* Check the gpio direction */
	regAddr = SA_REG_GPIO_BASE + SA_REG_GPIO_95_72_DATA + SA_REG_GPIO_OFFSET_DIR;
	RTMP_SYS_IO_READ32(regAddr, &regVal);

	if ((regVal & SA_REG_GPIO_DIR_OUTPUT_MASK) != SA_REG_GPIO_DIR_OUTPUT_MASK) {
		RTMP_SYS_IO_WRITE32(regAddr, (regVal | SA_REG_GPIO_DIR_OUTPUT_MASK));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_INFO, ("The GPIO output direction value is 0x%x, change to 0x%x!\n",
				 regVal, regVal | SA_REG_GPIO_DIR_OUTPUT_MASK));
	}

	/* Read the original GPIO value first */
	regAddr = SA_REG_GPIO_BASE + SA_REG_GPIO_95_72_DATA;
	RTMP_SYS_IO_READ32(regAddr, &regVal);
	/* MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("OldGPIOVal=0x%x, oldAntCfg=0x%x, newAntCfg=0x%x!\n",regVal, oldAntCfg, newAntCfg)); */
	regVal &= (~SA_REG_GPIO_DIR_OUTPUT_MASK);
	newAntCfg &= SA_REG_GPIO_DIR_OUTPUT_MASK;
	regVal |= newAntCfg;
	RTMP_SYS_IO_WRITE32(regAddr, regVal);
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_INFO,
			 ("Write the AntPattern(addr=0x%x, val=0x%x)\n", regAddr, regVal));
	RTMP_SYS_IO_READ32(regAddr, &regVal);
	/* MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("After Cfg, GPIOVal=0x%x!\n",regVal)); */
	return TRUE;
}


/*
	When use this function, please make sure the pSAParam is valid!
*/
UINT32 sa_get_ant_by_gpio(
	IN SMART_ANTENNA_STRUCT * pSAParam)
{
	RTMP_SA_AGSP_MAP *pAgspEntry;
	UINT32 regVal, regAddr, antPattern, bitMask;
	int antCnt;

	if (pSAParam->agsp == NULL)
		return 0;

	/* Check the gpio setting first */
	RTMP_SYS_IO_READ32(SA_REG_GPIO_MODE, &regVal);

	if ((regVal & 0x400) != 0x400) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("%s():Error- GPIOMode(0x%x) not set as GPIO mode\n",
				  __func__, regVal));
		return 0;
	}

	/* Check the gpio direction */
	regAddr = SA_REG_GPIO_BASE + SA_REG_GPIO_95_72_DATA + SA_REG_GPIO_OFFSET_DIR;
	RTMP_SYS_IO_READ32(regAddr, &regVal);

	if ((regVal & SA_REG_GPIO_DIR_OUTPUT_MASK) != SA_REG_GPIO_DIR_OUTPUT_MASK) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("%s():Error- GPIODirection(0x%x) not set as output\n",
				  __func__, regVal));
		return 0;
	}

	/* Write the new cfg first */
	regAddr = SA_REG_GPIO_BASE + SA_REG_GPIO_95_72_DATA;
	RTMP_SYS_IO_READ32(regAddr, &regVal);
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
			 ("%s(): The GPIO output is 0x%x\n",
			  __func__, regVal));
	antCnt = antPattern = 0;
	pAgspEntry = pSAParam->agsp;

	while ((antCnt < pSAParam->txNss) && (pAgspEntry->hdrPin != 0)) {
		bitMask = 1 <<  (pAgspEntry->gpioBit);

		if (regVal & bitMask) {
			antPattern |= (pAgspEntry->hdrPin) << (antCnt * 8);
			antCnt++;
		}

		pAgspEntry++;
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
			 ("%s(): The antPattern is 0x%x\n",
			  __func__, antPattern));
	return antPattern;
}


/*
	when call this function, please make sure following parameters are valid
	@pSAParm
	@pTrainEntry
*/
BOOLEAN sa_get_curAntPattern(
	IN SMART_ANTENNA_STRUCT * pSAParam,
	IN RTMP_SA_TRAINING_PARAM * pTrainEntry)
{
	UINT32 antPattern;
	INT32 ret;
	antPattern = sa_get_ant_by_gpio(pSAParam);

	if (antPattern != 0) {
		ret = sa_get_offset_by_antp(pSAParam, antPattern);

		if (ret  >= 0) {
			pTrainEntry->patternOffset = ret;
			pTrainEntry->curAntPattern = antPattern;
			return TRUE;
		} else {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
					 ("%s():Cannot found antPattern(0x%x) in TrainSeq\n",
					  __func__, antPattern));
		}
	} else {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("%s(): The antPattern get from GPIO is invalid\n",
				  __func__));
	}

	pTrainEntry->curAntPattern = 0;
	pTrainEntry->patternOffset = 0;
	return FALSE;
}


/*
	when call this function, please make sure following parameters are valid
	@pSAParm
	@pTrainEntry
*/
BOOLEAN sa_get_nextAntPattern(
	IN SMART_ANTENNA_STRUCT * pSAParam,
	IN RTMP_SA_TRAINING_PARAM * pTrainEntry,
	IN UCHAR weight,
	IN UINT32 selectRule)
{
	UINT32 *pTrainSeq;
	RTMP_SA_TRAIN_LOG_ELEMENT *pCurLog, *pBaseInfo = &pTrainEntry->antBaseInfo;
	pTrainSeq = pSAParam->pTrainSeq;

	if (pTrainSeq == NULL) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("%s():TrainSeq is NULL\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR,
			 ("%s():CurAnt(0x%x-%d),GetNextAntBy(Rule:%d, Weight:%d)\n",
			  __func__, pTrainEntry->curAntPattern,
			  pTrainEntry->patternOffset, selectRule, weight));
check:

	/* If the pTrainEntry->curAntPattern is 0, use the first one */
	if (pTrainEntry->curAntPattern)
		pTrainEntry->patternOffset++;
	else
		pTrainEntry->patternOffset = 0;

	if (pTrainEntry->patternOffset >= pSAParam->trainSeqCnt) {
		pTrainEntry->curAntPattern = 0;
		pTrainEntry->patternOffset = 0;
	} else
		pTrainEntry->curAntPattern = *(pTrainSeq + pTrainEntry->patternOffset);

	/* check weight when weight > 0 */
	pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + pTrainEntry->patternOffset);
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR, ("\tCheck Ant(0x%x-%d), AntTrainLog->antWeight(%d)\n",
			 pTrainEntry->curAntPattern, pTrainEntry->patternOffset, pCurLog->antWeight));

	if (weight && (pTrainEntry->curAntPattern != 0)) {
		if (pCurLog->antWeight > weight) {
			if (is_ant_equal(pTrainEntry->curAntPattern, pCurLog->antPattern) == FALSE) {
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR, ("\t\tGPIO of CandAnt(0x%x-%d) in TSeq(0x%x) and TLog(0x%x) are mismatch\n",
						 pTrainEntry->curAntPattern, pTrainEntry->patternOffset,
						 pCurLog->antPattern, pCurLog->patternOffset));
				sa_dump_train_seq(pSAParam->pTrainSeq);
				/* sa_dump_train_log(pAd, pTrainEntry, pTrainEntry->pTrainInfo, pSAParam->txNss); */
				pTrainEntry->curAntPattern = 0;
				pTrainEntry->patternOffset = 0;
			}

			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR, ("\t\tAnt(0x%x-%d) weight(%d) > required(%d), ignore\n",
					 pTrainEntry->curAntPattern, pTrainEntry->patternOffset,
					 pCurLog->antWeight, weight));
			goto check;
		} else {
			BOOLEAN bTryNext = FALSE;

			switch (selectRule) {
			case ANT_SELECT_IGNORE_BASE:
				if (is_ant_equal(pBaseInfo->antPattern, pCurLog->antPattern)) {
					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR, ("\t\tByRule(%d), Ant(0x%x-%d) == BaseAnt(0x%x-%d), ignore\n",
							 selectRule, pTrainEntry->curAntPattern, pTrainEntry->patternOffset,
							 pBaseInfo->antPattern, pBaseInfo->patternOffset));
					bTryNext = TRUE;
				}

				break;

			case ANT_SELECT_BASE:
				if (is_ant_equal(pBaseInfo->antPattern, pCurLog->antPattern) == FALSE) {
					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR, ("\t\tByRule(%d), Ant(0x%x-%d) != BaseAnt(0x%x-%d), ignore\n",
							 selectRule, pTrainEntry->curAntPattern, pTrainEntry->patternOffset,
							 pBaseInfo->antPattern, pBaseInfo->patternOffset));
					bTryNext = TRUE;
				}

				break;

			default:
				/* No limition */
				break;
			}

			if (bTryNext)
				goto check;
		}
	}

	if ((pTrainEntry->curAntPattern == 0) && (selectRule == ANT_SELECT_BASE))
		goto check;

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
			 ("\t=>SelectedAnt=0x%x-%d, weight=%d!\n",
			  pTrainEntry->curAntPattern, pTrainEntry->patternOffset,
			  weight));
	return TRUE;
}


INT sa_set_antPattern(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT * pSAParam,
	IN UINT32 prevAnt,
	IN UINT32 newAnt)
{
	int i, cnt = 0;
	UCHAR pinVal[SA_TX_NSS_MAX_NUM];
	SA_GPIO_PAIR gpioPair[SA_TX_NSS_MAX_NUM];
	UINT32 oldCfg = 0, newCfg = 0;

	while (newAnt > 0) {
		pinVal[cnt] = (newAnt & 0xff);
		cnt++;
		newAnt >>= (cnt * 8);
	}

	NdisZeroMemory(&gpioPair[0], sizeof(gpioPair));

	for (i = 0; i < cnt; i++) {
		/* TODO: We need to take care if the previous GPIO register is not the same with current one */
		/* Here we allow the antPattern is 0-0, which it means disable all antennas */
		if (get_gpio_by_hdr_pins(pSAParam->agsp, pinVal[i], &gpioPair[i]) == TRUE)
			newCfg |= gpioPair[i].bitMask;

		pinVal[i] = ((prevAnt >> (i * 8)) & 0xff);

		if (get_gpio_by_hdr_pins(pSAParam->agsp, pinVal[i], &gpioPair[i]) == TRUE)
			oldCfg |= gpioPair[i].bitMask;
	}

	/* MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("newCfg=0x%x, oldCfg=0x%x\n", newCfg, oldCfg)); */
	sa_gpio_write(pAd, oldCfg, newCfg);
	return TRUE;
}


INT sa_set_ant_weight(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT * pSAParam,
	IN RTMP_SA_TRAINING_PARAM * pTrainEntry,
	IN UINT32 antPattern,
	IN UCHAR weight)
{
	RTMP_SA_TRAIN_LOG_ELEMENT *pCurLog;
	BOOLEAN bSetAll = FALSE;
	int cnt;

	if (antPattern == 0)
		bSetAll = TRUE;

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_INFO,
			 ("%s():set weight(%d) for antenna pattern(0x%x)\n",
			  __func__, weight, antPattern));

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);

		if (is_ant_equal(pCurLog->antPattern, antPattern) || (bSetAll == TRUE)) {
			pCurLog->antWeight = weight;

			if (bSetAll == FALSE)
				break;
		}
	}

	if ((DebugSubCategory[DebugLevel][1] & CATHW_SA) == CATHW_SA) {
		for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
			pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tantPattern:0x%x, weight:%d\n", pCurLog->antPattern, pCurLog->antWeight));
		}
	}

	return TRUE;
}


INT sa_init_train_log(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT * pSAParam,
	IN RTMP_SA_TRAINING_PARAM * pTrainEntry,
	IN UCHAR weight)
{
	RTMP_SA_TRAIN_LOG_ELEMENT *pCurLog;
	int cnt;
	UINT32 *pTrainSeq;

	if (pTrainEntry->pTrainInfo == NULL) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR, ("%s():pTrainInfo is NULL\n", __func__));
		return FALSE;
	}

	NdisZeroMemory(pTrainEntry->pTrainInfo, sizeof(RTMP_SA_TRAIN_LOG_ELEMENT) * pSAParam->trainSeqCnt);

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);
		pTrainSeq = (UINT32 *)(pSAParam->pTrainSeq + cnt);
		pCurLog->antPattern = *pTrainSeq;
		pCurLog->patternOffset = cnt;
		pCurLog->antWeight = weight;
		pCurLog->candWeight = weight;
	}

	return TRUE;
}


/*
	@pAd: Device control block pointer
	@pMacAddr: the mac address used for condition to find the entry
		If pMacAddr == BROADCAST_ADDR, find the first non-static assigned train entry
		If pMacAddr == ZERO, find first empty train entry
		If pMacAddr == Unicast Addr, find the entry which has the same macAddr

	return value:
		If found, return the entry pointer, else return NULL
*/
RTMP_SA_TRAINING_PARAM *sa_find_train_entry(
	IN RTMP_ADAPTER * pAd,
	IN UCHAR *pMacAddr)
{
	RTMP_SA_TRAINING_PARAM *pTrainEntry = NULL;
	SMART_ANTENNA_STRUCT *pSAParam = pAd->pSAParam;
	int idx;
	BOOLEAN bAllowDynamic = FALSE;

	if (NdisEqualMemory(pMacAddr, &BROADCAST_ADDR[0], MAC_ADDR_LEN))
		bAllowDynamic = TRUE;

	for (idx = 0; idx < SA_ENTRY_MAX_NUM; idx++) {
		pTrainEntry = &pSAParam->trainEntry[idx];

		if (NdisEqualMemory(pMacAddr, &pTrainEntry->macAddr[0], MAC_ADDR_LEN))
			break;

		if ((bAllowDynamic == TRUE) && (pTrainEntry->bStatic == FALSE))
			break;
	}

	return ((idx < SA_ENTRY_MAX_NUM) ? pTrainEntry : NULL);
}


INT sa_init_train_entry(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT *pSAParam,
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry,
	IN RTMP_SA_TRAIN_LOG_ELEMENT *pTrainLog,
	IN UCHAR *pMacAddr)
{
	unsigned int cleanLen, offset;
	MAC_TABLE_ENTRY *pMacEntry;
	/*
		Here we don't clean following fields:
			macAddr, bStatic, pTrainInfo
	*/
	offset = (unsigned int)(&(((RTMP_SA_TRAINING_PARAM *)NULL)->pMacEntry));
	cleanLen = sizeof(RTMP_SA_TRAINING_PARAM) - offset;
	NdisZeroMemory((PUCHAR)(&pTrainEntry->pMacEntry), cleanLen);
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():sizeof(RTMP_SA_TRAINING_PARAM)=%d, offset=%d, cleanLen=%d!\n",
			 __func__, sizeof(RTMP_SA_TRAINING_PARAM), offset, cleanLen));
	/* some necessary defualt setting for TrainEntry */
	pTrainEntry->trainStage = SA_INVALID_STAGE;

	if (pMacAddr)
		NdisCopyMemory(&pTrainEntry->macAddr[0], pMacAddr, MAC_ADDR_LEN);

	/* find and update associated mac table entry */
	pMacEntry = MacTableLookup(pAd, &pTrainEntry->macAddr[0]);

	if (pMacEntry) {
		pTrainEntry->pMacEntry = pMacEntry;
		pMacEntry->pTrainEntry = (void *)pTrainEntry;
	}

	/* init the train log related data structures */
	sa_init_train_log(pAd, pSAParam, pTrainEntry, ANT_WEIGHT_SCAN_ALL);
	return TRUE;
}


RTMP_SA_TRAINING_PARAM *sa_add_train_entry(
	IN RTMP_ADAPTER * pAd,
	IN UCHAR *pMacAddr,
	IN BOOLEAN bAddByUser)
{
	RTMP_SA_TRAINING_PARAM *pTrainEntry = NULL;
	/* check if existing entry already has this mac */
	pTrainEntry = sa_find_train_entry(pAd, pMacAddr);

	/* If no existing entry for this mac, find an empty train entry */
	if (pTrainEntry == NULL)
		pTrainEntry = sa_find_train_entry(pAd, &ZERO_MAC_ADDR[0]);

	/* if no emptry entry, then select first non-static */
	if ((pTrainEntry == NULL) && (bAddByUser == TRUE))
		pTrainEntry = sa_find_train_entry(pAd, &BROADCAST_ADDR[0]);

	if (pTrainEntry) {
		sa_init_train_entry(pAd, pAd->pSAParam, pTrainEntry,
							pTrainEntry->pTrainInfo, pMacAddr);

		if (bAddByUser)
			pTrainEntry->bStatic = bAddByUser;
	}

	return pTrainEntry;
}


BOOLEAN sa_del_train_entry(
	IN RTMP_ADAPTER * pAd,
	IN UCHAR *pMacAddr,
	IN BOOLEAN bForced)
{
	int idx;
	SMART_ANTENNA_STRUCT *pSAParam = pAd->pSAParam;
	RTMP_SA_TRAINING_PARAM *pTrainEntry;
	MAC_TABLE_ENTRY *pMacEntry;
	BOOLEAN bDeleted = FALSE;

	if (pMacAddr) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
				 ("%s():Try to del mac(%02x:%02x:%02x:%02x:%02x:%02x), bForced=%d!\n",
				  __func__, PRINT_MAC(pMacAddr), bForced));
	}

	for (idx = 0; idx < SA_ENTRY_MAX_NUM; idx++) {
		pTrainEntry = &pSAParam->trainEntry[idx];

		if (NdisEqualMemory(&pTrainEntry->macAddr[0], pMacAddr, MAC_ADDR_LEN)) {
			unsigned int cleanLen, offset;
			PUCHAR cleanPtr;
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
					 ("%s():find matched trainEntry(Idx=%d), bTraining=%d!\n",
					  __func__, idx, pTrainEntry->bTraining));

			if (pTrainEntry->pMacEntry) {
				pMacEntry = pTrainEntry->pMacEntry;
				pMacEntry->pTrainEntry = NULL;
			}

			if (pTrainEntry->bTraining == TRUE) {
				/* recovery to original antenna */
				sa_get_curAntPattern(pSAParam, pTrainEntry);
				sa_set_antPattern(pAd, pSAParam, pTrainEntry->curAntPattern, pTrainEntry->antBaseInfo.antPattern);
			}

			/*  Here we don't clean following fields: pTrainInfo */
			if ((bForced == TRUE) || (pTrainEntry->bStatic == FALSE)) {
				offset = (unsigned int)(&(((RTMP_SA_TRAINING_PARAM *)NULL)->macAddr));
				cleanPtr = (PUCHAR)(&pTrainEntry->macAddr[0]);
			} else {
				offset = (unsigned int)(&(((RTMP_SA_TRAINING_PARAM *)NULL)->pMacEntry));
				cleanPtr = (PUCHAR)(&pTrainEntry->pMacEntry);
			}

			cleanLen = sizeof(RTMP_SA_TRAINING_PARAM) - offset;
			NdisZeroMemory(cleanPtr, cleanLen);
			bDeleted = TRUE;
			break;
		}
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s(): delete %s\n",
			 __func__, (bDeleted == TRUE ? "success" : "failed")));
	return bDeleted;
}


BOOLEAN sa_select_target_train_entry(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT *pSAParam)
{
	MAC_TABLE_ENTRY *pEntry, *pCandEntry;
	RTMP_SA_TRAINING_PARAM *pTrainEntry;
	int macIdx;

	if (pSAParam == NULL) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR, ("%s():pSAParam is NULL\n", __func__));
		return FALSE;
	}

	pTrainEntry = &pSAParam->trainEntry[0];

	if (pTrainEntry->bStatic == TRUE) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
				 ("%s():static target,ignore the selection!\n", __func__));
		return FALSE;
	}

	pCandEntry = NULL;

	for (macIdx = 0; VALID_UCAST_ENTRY_WCID(pAd, macIdx); macIdx++) {
		pEntry = &pAd->MacTab.Content[macIdx];

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
					 ("STA(%02x:%02x:%02x:%02x:%02x:%02x): RSSI=%d\n",
					  PRINT_MAC(pEntry->Addr), pEntry->avgRssi[0]));

			/* ignore the station which has no RSSI result yet */
			if (pEntry->avgRssi[0] == 0)
				continue;

			/* Check if need to change the candidate */
			if (pCandEntry) {
				if (pCandEntry->avgRssi[0] < pSAParam->rssiThreshold) {
					/* We only care the stations which RSSI larger than threshold */
					if (pEntry->avgRssi[0] > pCandEntry->avgRssi[0])
						pCandEntry = pEntry;
				} else {
					if (pEntry->avgRssi[0] >= pSAParam->rssiThreshold) {
						/* both sta has rssi larger than thold, select the smaller one. */
						if (pCandEntry->avgRssi[0] > pEntry->avgRssi[0])
							pCandEntry = pEntry;
					}
				}
			} else
				pCandEntry = pEntry;
		}
	}

	if (pCandEntry) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("After Selection, target TrainEntry is %02x:%02x:%02x:%02x:%02x:%02x(RSSI=%d)\n",
				  PRINT_MAC(pCandEntry->Addr), pCandEntry->avgRssi[0]));
		sa_init_train_entry(pAd, pSAParam, pTrainEntry, pTrainEntry->pTrainInfo, &pCandEntry->Addr[0]);
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("After Selection, Cannot find target TrainEntry\n"));
		return FALSE;
	}
}


UCHAR sa_get_trainup_per_by_mcs(UCHAR *pRateTb, UCHAR mcs)
{
	int tbIdx, tbSize, entryStep;
	RTMP_RA_LEGACY_TB *pCurrTxRate;

	if (!pRateTb)
		return 0;

#ifdef NEW_RATE_ADAPT_SUPPORT

	if (ADAPT_RATE_TABLE(pRateTb))
		entryStep = 10;
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
	{
		entryStep = 5;
	}

	tbSize = pRateTb[0];

	for (tbIdx = 0; tbIdx < tbSize; tbIdx++) {
		pCurrTxRate = (RTMP_RA_LEGACY_TB *) &pRateTb[(tbIdx + 1) * entryStep];

		if (pCurrTxRate->CurrMCS == mcs) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
					 ("The trainUp PER of MCS(%d) is %d\n",
					  mcs, pCurrTxRate->TrainUp));
			return pCurrTxRate->TrainUp;
		}
	}

	return 0;
}


int sa_train_db_exit(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT *pSAParam)
{
	int i;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s()--->\n", __func__));

	/* Clean the old debug related structure first */
	for (i = 0; i < SA_ENTRY_MAX_NUM; i++)
		pSAParam->trainEntry[i].pTrainInfo = NULL;

	if (pSAParam->pTrainMem) {
		os_free_mem(pSAParam->pTrainMem);
		pSAParam->pTrainMem = NULL;
	}

	/* Clean the TrainSeq list */
	pSAParam->trainSeqCnt = 0;

	if (pSAParam->pTrainSeq && (pSAParam->pTrainSeq != &DefaultTrainSeq[0]))
		os_free_mem(pSAParam->pTrainSeq);

	pSAParam->pTrainSeq = NULL;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("<---%s():\n", __func__));
	return TRUE;
}


int sa_train_db_init(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT *pSAParam,
	IN UINT32 *pTrainSeqLst)
{
	/* RTMP_SA_TRAINING_PARAM *pTrainEntry; */
	UINT32 *ptr;
	int memSize, cnt = 0;
	ASSERT(pTrainSeqLst);
	ptr = pTrainSeqLst;

	while (*ptr != AGSP_TRAIN_SEQ_END) {
		cnt++;
		ptr++;
	}

	if (cnt == 0) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Error:TrainSeqCnt is zero!\n"));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():TrainCnt=%d\n", __func__, cnt));
	/* Clean the old debug related structure first */
	sa_train_db_exit(pAd, pSAParam);
	/* Allocate memory for TrainMem */
	memSize = sizeof(RTMP_SA_TRAIN_LOG_ELEMENT) * cnt;
	os_alloc_mem(pAd, (UCHAR **)&pSAParam->pTrainMem, memSize * SA_ENTRY_MAX_NUM);

	if (pSAParam->pTrainMem == NULL) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():malloc failed\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("AllocTrainMemDone(addr=0x%lx,entrySize=%d,totalSize=%d)\n",
			 (ULONG)pSAParam->pTrainMem,
			 sizeof(RTMP_SA_TRAIN_LOG_ELEMENT),
			 memSize * SA_ENTRY_MAX_NUM));
	/* Set new parameters */
	pSAParam->pTrainSeq = pTrainSeqLst;
	pSAParam->trainSeqCnt = cnt;

	/* Init the TrainEntry associated TrainLogInfo space */
	for (cnt = 0; cnt < SA_ENTRY_MAX_NUM; cnt++)
		pSAParam->trainEntry[cnt].pTrainInfo = pSAParam->pTrainMem + (pSAParam->trainSeqCnt * cnt);

	/* dump it out for debug */
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tTrainMemoryLayout:\n"));

	for (cnt = 0; cnt < SA_ENTRY_MAX_NUM; cnt++) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\ttrainInfo[%d]=%p\n", cnt,
				 pSAParam->trainEntry[cnt].pTrainInfo));
	}

	return TRUE;
}


INT sa_cand_method_2(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT *pSAParam,
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry)
{
	RTMP_SA_TRAIN_LOG_ELEMENT *pCurLog, *pCanLog2;
	INT32 PER[9] = {0};
	UINT32 txCnt[9] = {0}, txRtyCnt[9] = {0};
	UINT32 curAnt;
	int cnt, majorPin;
	UCHAR pinVal;
	/* RTMP_SA_AGSP_MAP *pAgspEntry; */
	/* Method 2: */
	/* Use group concept do the antenna selection */
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Method 2:\n"));
	pCanLog2 = NULL;
	memset(&PER[0], 0xff, sizeof(PER));
	memset(&txCnt[0], 0, sizeof(txCnt));
	memset(&txRtyCnt[0], 0, sizeof(txRtyCnt));

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);
		curAnt = pCurLog->antPattern;

		while (curAnt > 0) {
			pinVal = (curAnt & 0xff);

			if (pinVal < 9) {
				txCnt[pinVal] += pCurLog->txCnt;
				txRtyCnt[pinVal] += pCurLog->txRtyCnt;
			} else {
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
						 ("\t\t%s():Wrong pinVal!\n", __func__));
				return FALSE;
			}

			curAnt >>= 8;
		}
	}

	/* After we collect all info, now get the PER of each antenna */
	/* MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,("Now calculate the PER of each antenna!\n")); */
	majorPin = -1;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tStage 1:\n"));

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		if (txCnt[cnt])
			PER[cnt] = (txRtyCnt[cnt] * 1000) / txCnt[cnt];
		else
			PER[cnt] = 0xffff;

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("\t\tpin[%d].txCnt=%d,rtyCnt=%d, PER=%d!\n",
				  cnt, txCnt[cnt], txRtyCnt[cnt], PER[cnt]));

		if ((PER[cnt] >= 0) && (txCnt[cnt] > 0)) {
			if (majorPin < 0)
				majorPin = cnt;
			else {
				if ((PER[cnt] < PER[majorPin]) ||
					((PER[majorPin] == PER[cnt]) && (txCnt[cnt] > txCnt[majorPin])))
					majorPin = cnt;
			}
		}
	}

	if (majorPin >= 0) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("\t\tThe Major antenna pin is %d!\n", majorPin));
	}

	/* Check all pattern which including this major pin */
	if (majorPin >= 0) {
		UINT32 mask, majorVal;
		majorVal = ((majorPin > 4) ? majorPin : (majorPin << 8));
		mask = ((majorPin > 4) ? 0xff : 0xff00);
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("\tStage 2(mask=0x%x):\n", mask));
		pCanLog2 = pTrainEntry->pTrainInfo;

		for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
			pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);

			if ((pCurLog->antPattern & mask) == majorVal) {
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
						 ("\t\tCheck antPattern(0x%x-%d)\n",
						  pCurLog->antPattern, pCurLog->patternOffset));

				if (pCanLog2 == NULL)
					pCanLog2 = pCurLog;
				else {
					if ((pCurLog->PER < pCanLog2->PER) ||
						((pCurLog->PER == pCanLog2->PER) && (pCurLog->txCnt > pCanLog2->txCnt))
					   )
						pCanLog2 = pCurLog;
				}
			}
		}
	}

	if (pCanLog2) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("Method %d:\n", pSAParam->candMethod));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("\tAntCandidate is Ant:0x%x-%d(PER:%d,txCnt:%d)\n",
				  pCanLog2->antPattern, pCanLog2->patternOffset,
				  pCanLog2->PER, pCanLog2->txCnt));
	}

	return TRUE;
}


/* Method 3 */
/* We compare PER first, if PER < 8, then find the max txCnt as the cand! */
RTMP_SA_TRAIN_LOG_ELEMENT *sa_cand_method_3(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT *pSAParam,
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry)
{
	RTMP_SA_TRAIN_LOG_ELEMENT *pCanLog = NULL, *pCurLog;
	int cnt;
	/* pass 1, check all candidates and get with low PER(i.e., PER < 8) */
	pCanLog = pTrainEntry->pTrainInfo;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_INFO,
			 ("\tPhase 1(PER threshold=%d):\n", pSAParam->trainCond));

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);

		if (pCurLog->PER < pSAParam->trainCond) {
			pCanLog = pCurLog;
			pCanLog->candWeight = 1;
		} else {
			/* when PER > threshold, directly use PER to check the value */
			if ((pCurLog->PER < pCanLog->PER) ||
				((pCurLog->PER == pCanLog->PER) && (pCurLog->txCnt > pCanLog->txCnt))
			   ) {
				pCanLog->candWeight = ANT_WEIGHT_SCAN_ALL;
				pCanLog = pCurLog;
				pCurLog->candWeight = 2;
			} else
				pCurLog->candWeight = ANT_WEIGHT_SCAN_ALL;
		}

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
				 ("\t\tAntPattern:0x%x,txCnt:%d,PER:%d,weight:%d\n",
				  pCurLog->antPattern, pCurLog->txCnt, pCurLog->PER,
				  pCurLog->candWeight));
	}

	/* phase 2, get the real candidate depends on weight and txCnt */
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_INFO, ("\tPhase 2:\n"));
	pCanLog = NULL;

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);

		if (pCurLog->candWeight  <= 2) {
			if (pCanLog) {
				if (pCanLog->candWeight > pCurLog->candWeight)
					pCanLog = pCurLog;
				else if (pCanLog->candWeight < pCurLog->candWeight) {
					/* do nothing */
				} else if (pCanLog->candWeight == pCurLog->candWeight) {
					/* it must be 1, because only weight 1 may have multiple cand. */
					if (pCurLog->txCnt > pCanLog->txCnt)
						pCanLog = pCurLog;
				}
			} else
				pCanLog = pCurLog;
		}

		if (pCanLog) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_INFO,
					 ("\t\tpCurLog(0x%x-%d), pCanLog(0x%x-%d)\n",
					  pCurLog->antPattern, pCurLog->candWeight,
					  pCanLog->antPattern, pCanLog->candWeight));
		} else {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_INFO,
					 ("\t\tpCurLog(0x%x-%d), ignore\n",
					  pCurLog->antPattern, pCurLog->candWeight));
		}
	}

	if (pCanLog) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("Method %d:\n", pSAParam->candMethod));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("\tAntCandidate is Ant:0x%x-%d(PER:%d,txCnt:%d)\n",
				  pCanLog->antPattern, pCanLog->patternOffset,
				  pCanLog->PER, pCanLog->txCnt));
	}

	return pCanLog;
}


/* Method 4 */
/* We compare PER first, if PER < 8, then find the max txCnt as the cand */
RTMP_SA_TRAIN_LOG_ELEMENT *sa_cand_method_4(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT *pSAParam,
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry)
{
	RTMP_SA_TRAIN_LOG_ELEMENT *pCanLog = NULL, *pCurLog;
	UINT32 totalTxNoRtyCnt = 0, avgTxNoRtyCnt, tpCnt = 0, PerUBnd = 0xffffffff, mcsPerBnd = 0;
	int cnt, txNss, NoRtyRatio;
	INT32 candSNR, curSNR;
	/* phase 1, check all candidates and get with average txCnt */
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():phase 1, get totalTxNoRtyCnt(trainWeight=%d)\n",
			 __func__, pTrainEntry->trainWeight));
	pCanLog = pTrainEntry->pTrainInfo;

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);
		pCurLog->candWeight = ANT_WEIGHT_CAND_INIT;

		if (pCurLog->antWeight <= pTrainEntry->trainWeight) {
			tpCnt++;
			totalTxNoRtyCnt += pCurLog->txNoRtyCnt;
			NoRtyRatio = -1;

			if (pCurLog->txCnt)
				NoRtyRatio = (pCurLog->txNoRtyCnt * 100) / pCurLog->txCnt;

			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
					 ("\tantPattern(0x%x-%d),AWeight=%3d,CWeight=%3d,txCnt=%d(S:%d,R:%d,F:%d,OSR:%d)\n",
					  pCurLog->antPattern, pCurLog->patternOffset,
					  pCurLog->antWeight, pCurLog->candWeight,
					  pCurLog->txCnt, pCurLog->txNoRtyCnt, pCurLog->txRtyCnt, pCurLog->txRtyFailCnt,
					  NoRtyRatio));
		}
	}

	avgTxNoRtyCnt = totalTxNoRtyCnt / tpCnt;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_INFO,
			 ("\tPhase 1, AvgTxNoRtyCnt=%d(total=%d,cnt=%d)\n",
			  avgTxNoRtyCnt, totalTxNoRtyCnt, tpCnt));
	/*
		phase 2,
			(a).filter the antenna pattern whose txCnt lower than average txcnt
			(b).get the lowest PER
	*/
	avgTxNoRtyCnt = avgTxNoRtyCnt * 3 / 4;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
			 ("%s():phase 2, set CWeight by Weighted AvgTxNoRtyCnt(%d)\n",
			  __func__, avgTxNoRtyCnt));

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);

		if ((pCurLog->antWeight <= pTrainEntry->trainWeight) &&
			(((pTrainEntry->trainWeight == ANT_WEIGHT_SCAN_ALL) && (pCurLog->txNoRtyCnt >= avgTxNoRtyCnt)) ||
			 ((pTrainEntry->trainWeight != ANT_WEIGHT_SCAN_ALL))
			)
		   ) {
			pCurLog->candWeight = ANT_WEIGHT_CAND_LOW;

			if (pCurLog->PER < PerUBnd)
				PerUBnd = pCurLog->PER;

			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
					 ("\tantPattern(0x%x-%d),AWeight=%3d,CWeight=%3d,PER=%4d,PERUB=%4d,txCnt=%d(S:%d,R:%d,F:%d)\n",
					  pCurLog->antPattern, pCurLog->patternOffset, pCurLog->antWeight,
					  pCurLog->candWeight, pCurLog->PER, PerUBnd,
					  pCurLog->txCnt, pCurLog->txNoRtyCnt, pCurLog->txRtyCnt, pCurLog->txRtyFailCnt));
		}
	}

	/*
		phase 3,
			(a).find the candidate by candWeight and SNR
			(b).if SNR euqal, then check PER
			(c).if SNR and PER both equal, then check totalTxNoRtyCnt
	*/
	pCanLog = NULL;
	candSNR = 0;
	PerUBnd += pSAParam->trainCond;

	if (pTrainEntry->pMacEntry) {
		MAC_TABLE_ENTRY *pMacEntry;
		pMacEntry = pTrainEntry->pMacEntry;

		if (pMacEntry->pRateTable)
			mcsPerBnd = sa_get_trainup_per_by_mcs(pMacEntry->pRateTable, pMacEntry->HTPhyMode.field.MCS);

		mcsPerBnd *= 10;
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():phase 3, find candidate by PER(< max[PerUBnd=%d, McsTrainUpBnd=%d]) and Weight(<%d)\n",
			 __func__, PerUBnd, mcsPerBnd, ANT_WEIGHT_CAND_LOW));

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);
		curSNR = 0;

		/* calcuate the SNR first */
		for (txNss = 0; txNss < pSAParam->txNss; txNss++) {
			if (pCurLog->cntSNR[txNss])
				curSNR += (((pCurLog->sumSNR[txNss] / pCurLog->cntSNR[txNss]) * 1881) / 10000);
		}

		if ((pCurLog->candWeight == ANT_WEIGHT_CAND_LOW) &&
			((pCurLog->PER <= PerUBnd) || (pCurLog->PER <= mcsPerBnd))) {
			BOOLEAN bChange = FALSE;
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tCheck Ant(0x%x-%d)=>AWeight=%3d,CWeight=%3d,PER=%4d,SNR=%2d,txCnt=%d(S:%d,R:%d,F:%d)\n",
					 pCurLog->antPattern, pCurLog->patternOffset,
					 pCurLog->antWeight, pCurLog->candWeight, pCurLog->PER, curSNR,
					 pCurLog->txCnt, pCurLog->txNoRtyCnt, pCurLog->txRtyCnt, pCurLog->txRtyFailCnt));

			/* TODO: move SNR calculation to here! */
			/* Check the curSNR with the selected candidate antenna pattern */
			if (pCanLog) {
				if (curSNR > candSNR) {
					/* SNR first! */
					bChange = TRUE;
				} else if (curSNR == candSNR) {
					/* check PER */
					if (pCurLog->PER < pCanLog->PER)
						bChange = TRUE;
					else if (pCurLog->PER == pCanLog->PER) {
						/* check txCnt */
						if (pCurLog->txCnt >= pCanLog->txCnt)
							bChange = TRUE;
					}
				}
			} else
				bChange = TRUE;

			if (bChange == TRUE) {
				pCanLog = pCurLog;
				candSNR = curSNR;
			}
		}

		if (pCanLog) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
					 ("\t\tpCurLog(0x%x-%d,PER=%d,SNR=%d,txCnt=%d),pCanLog(0x%x-%d,PER=%d,SNR=%d,txCnt=%d)\n",
					  pCurLog->antPattern, pCurLog->candWeight, pCurLog->PER, curSNR, pCurLog->txCnt,
					  pCanLog->antPattern, pCanLog->candWeight, pCanLog->PER, candSNR, pCanLog->txCnt));
		} else {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
					 ("\t\tpCurLog(0x%x-%d,PER=%d,SNR=%d,txCnt=%d), no candidate!\n",
					  pCurLog->antPattern, pCurLog->candWeight,
					  pCurLog->PER, curSNR, pCurLog->txCnt));
		}
	}

	/* phase 4, depends on the PER of pCanLog, find the final candidate via SNR */
	if (pCanLog) {
		UINT32 PerUBnd;
		PerUBnd = pCanLog->PER + 50;
	}

	if (pCanLog) {
		pCanLog->candWeight = ANT_WEIGHT_CAND_HIGH;
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("\tAntCandidate is Ant:0x%x-%d(PER:%d,txCnt:%d)\n",
				  pCanLog->antPattern, pCanLog->patternOffset,
				  pCanLog->PER, pCanLog->txCnt));
	}

	return pCanLog;
}


/* Method 1: */
RTMP_SA_TRAIN_LOG_ELEMENT *sa_cand_method_1(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT *pSAParam,
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry)
{
	RTMP_SA_TRAIN_LOG_ELEMENT *pCanLog = NULL, *pCurLog;
	int cnt;
	/* check if we can switch this one as the new candidate */
	pCanLog = pTrainEntry->pTrainInfo;

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);

		if ((pCurLog->PER < pCanLog->PER) ||
			((pCurLog->PER == pCanLog->PER) && (pCurLog->txCnt > pCanLog->txCnt))
		   )
			pCanLog = pCurLog;
	}

	if (pCanLog) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("Method %d:\n", pSAParam->candMethod));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("\tAntCandidate is Ant:0x%x-%d(PER:%d,txCnt:%d)\n",
				  pCanLog->antPattern, pCanLog->patternOffset,
				  pCanLog->PER, pCanLog->txCnt));
	}

	return pCanLog;
}


INT sa_update_ant_weight(
	IN SMART_ANTENNA_STRUCT *pSAParam,
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry,
	IN UCHAR candWeight,
	IN UCHAR antWeight)
{
	int cnt;
	RTMP_SA_TRAIN_LOG_ELEMENT *pTrainLog;

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pTrainLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);

		if (pTrainLog->candWeight <= candWeight)
			pTrainLog->antWeight = antWeight;
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Dump antWeight after adjust antenna weight!\n"));

	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pTrainLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tantPattern(0x%x-%d), antWeight=%d, candWeight=%d\n",
				 pTrainLog->antPattern, pTrainLog->patternOffset,
				 pTrainLog->antWeight, pTrainLog->candWeight));
	}

	return TRUE;
}


BOOLEAN sa_can_do_fast_tune(
	IN SMART_ANTENNA_STRUCT *pSAParam,
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry)
{
	int i;
	MAC_TABLE_ENTRY *pMacEntry = pTrainEntry->pMacEntry;
	BOOLEAN bCanDoFT = TRUE;

	/* check RSSI */
	if (pMacEntry) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():dump RSSI result\n", __func__));

		for (i = 0; i < pSAParam->txNss; i++) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tRSSI[%d]: prev=%d, cur=%d\n",
					 i, pMacEntry->baseAvgRSSI[i], pMacEntry->curAvgRSSI[i]));
		}

		sa_rssi_chk_variant(pMacEntry, pSAParam->txNss, pSAParam->rssiVar);

		for (i = 0; i < pSAParam->txNss; i++) {
			if (pMacEntry->baseAvgRSSI[i] != 0) {
				if (((pMacEntry->curAvgRSSI[i] - pMacEntry->baseAvgRSSI[i]) > pSAParam->rssiVar) ||
					((pMacEntry->baseAvgRSSI[i] - pMacEntry->curAvgRSSI[i]) > pSAParam->rssiVar)) {
					bCanDoFT = FALSE;
					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
							 ("%s(Rssi[%d]):Hit!!Cannot do FastTuning(OrgRssi=%d,curRssi=%d,rssiVar=%d)\n",
							  __func__, i, pMacEntry->baseAvgRSSI[i],
							  pMacEntry->curAvgRSSI[i], pSAParam->rssiVar));
				}
			}
		}
	}

	return bCanDoFT;
}


BOOLEAN sa_chk_train_consistence(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT *pSAParam,
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry)
{
	RTMP_SA_TRAIN_LOG_ELEMENT *pBaseInfo, *pCurLog;
	BOOLEAN bIsConsisant = TRUE;
	int i;
	pBaseInfo = &pTrainEntry->antBaseInfo;
	pCurLog = pTrainEntry->pCurTrainInfo;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():Check consistence of training environment(BaseInfo:0x%x-%d,CurLog:0x%x-%d)\n",
			 __func__, pBaseInfo->antPattern, pBaseInfo->patternOffset,
			 pCurLog->antPattern, pCurLog->patternOffset));

	for (i = 0; i < pSAParam->txNss; i++) {
#error "Need to fix ConvertToRssi() before compile this feature"
		if (pBaseInfo->cntRSSI[i] != 0)
			pBaseInfo->avgRSSI[i] = ConvertToRssi(pAd, pBaseInfo->sumRSSI[i] / pBaseInfo->cntRSSI[i], i);
		else
			pBaseInfo->avgRSSI[i] = 0;

		if (pCurLog->cntRSSI[i] != 0)
			pCurLog->avgRSSI[i] = ConvertToRssi(pAd, pCurLog->sumRSSI[i] / pCurLog->cntRSSI[i], i);
		else
			pCurLog->avgRSSI[i] = 0;

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tBase-RSSI[%d](cnt:%d,avgRSSI:%d), Current-RSSI[%d](cnt:%d,avgRSSI[%d])\n",
				 i, pBaseInfo->cntRSSI[i], pBaseInfo->avgRSSI[i],
				 i, pCurLog->cntRSSI[i], pCurLog->avgRSSI[i]));
	}

	if (!is_ant_equal(pBaseInfo->antPattern, pCurLog->antPattern)) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():ERROR!BaseInfo(0x%x-%d) not match CurLog(0x%x-%d)\n",
				 __func__, pBaseInfo->antPattern, pBaseInfo->patternOffset,
				 pCurLog->antPattern, pCurLog->patternOffset));
		bIsConsisant = FALSE;
	}

	for (i = 0; i < pSAParam->txNss; i++) {
		if (pBaseInfo->avgRSSI[i] != 0) {
			if (((pCurLog->avgRSSI[i] - pBaseInfo->avgRSSI[i]) > pSAParam->rssiVar) ||
				((pBaseInfo->avgRSSI[i] - pCurLog->avgRSSI[i]) > pSAParam->rssiVar)) {
				bIsConsisant = FALSE;
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
						 ("%s(Rssi[%d]):Hit!!Environment not consistant!(OrgRssi=%d,curRssi=%d, rssiVar=%d)\n",
						  __func__, i, pBaseInfo->avgRSSI[i],
						  pCurLog->avgRSSI[i], pSAParam->rssiVar));
			}
		}
	}

	return bIsConsisant;
}


VOID sa_read_clean_train_result(
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry,
	IN RTMP_SA_TRAIN_LOG_ELEMENT *pCurLog)
{
#ifdef SA_LUMP_SUM
	pCurLog->txRtyFailCnt =  pTrainEntry->sumTxFailCnt;
	pCurLog->txRtyCnt = pTrainEntry->sumTxRtyCnt + pTrainEntry->sumTxFailCnt;
	pCurLog->txNoRtyCnt = pTrainEntry->sumTxCnt - pCurLog->txRtyCnt;
	pCurLog->txCnt = pTrainEntry->sumTxCnt;
	pTrainEntry->sumTxCnt = 0;
	pTrainEntry->sumTxRtyCnt = 0;
	pTrainEntry->sumTxFailCnt = 0;
#else
	MAC_TABLE_ENTRY *pMacEntry = pTrainEntry->pMacEntry;
	pCurLog->txRtyFailCnt = pMacEntry->saLstTxFailCnt;
	pCurLog->txRtyCnt = pMacEntry->saLstTxRtyCnt +
						pMacEntry->saLstTxFailCnt;
	pCurLog->txNoRtyCnt = pMacEntry->saLstTxNoRtyCnt;
	pCurLog->txCnt = pCurLog->txRtyCnt +
					 pMacEntry->saLstTxNoRtyCnt;
	pMacEntry->saLstTxRtyCnt = 0;
	pMacEntry->saLstTxFailCnt = 0;
	pMacEntry->saLstTxNoRtyCnt = 0;
#endif /* SA_LUMP_SUM // */
}


VOID sa_trainInfo_update(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry,
	IN RTMP_SA_TRAIN_LOG_ELEMENT *pCurLog,
	IN UCHAR antCnt)
{
	TX_STA_CNT0_STRUC staTxCnt0;
	TX_STA_CNT1_STRUC staTxCnt1;
	/* MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE, ("\tUpdate TrainInfo of Ant(0x%x-%d)=>\n", */
	/* pTrainEntry->curAntPattern, pTrainEntry->patternOffset)); */
	NicGetTxRawCounters(pAd, &staTxCnt0, &staTxCnt1);
	sa_read_clean_train_result(pTrainEntry, pCurLog);
	/* sa_rssi_update(pAd, pTrainEntry->pMacEntry, antCnt); */
	/* MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE, ("\t<=Update TrainInfo of Ant(0x%x-%d)\n", */
	/* pTrainEntry->curAntPattern, pTrainEntry->patternOffset)); */
}


INT sa_get_cand(
	IN RTMP_ADAPTER * pAd,
	IN SMART_ANTENNA_STRUCT *pSAParam,
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry)
{
	RTMP_SA_TRAIN_LOG_ELEMENT *pCurLog, *pCanLog;
	int cnt, agspCnt;
	RTMP_SA_AGSP_MAP *pAgspEntry;

	if (pTrainEntry->pTrainInfo == NULL) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("Error:pTrainEntry->pTrainInfo is NULL\n"));
		return FALSE;
	}

	if (pSAParam->agsp == NULL) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("Error: AGSP is not initialized!\n"));
		return FALSE;
	}

	agspCnt = 0;
	pAgspEntry = pSAParam->agsp;

	while (memcmp(pAgspEntry, &AGSP_ENTRY_END, sizeof(RTMP_SA_AGSP_MAP)) !=  0) {
		pAgspEntry++;
		agspCnt++;
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
			 ("agspCnt=%d!\n", agspCnt));

	/* first of all, dump all info */
	for (cnt = 0; cnt < pSAParam->trainSeqCnt; cnt++) {
		pCurLog = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + cnt);

		if ((pTrainEntry->trainWeight == 0) || (pCurLog->antWeight <= pTrainEntry->trainWeight))
			sa_dump_train_log(pAd, pTrainEntry, pCurLog, pSAParam->txNss);
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
			 ("Now decide the candidate by Method %d!\n",
			  pSAParam->candMethod));

	switch (pSAParam->candMethod) {
	case 1:
		pCanLog = sa_cand_method_1(pAd, pSAParam, pTrainEntry);
		break;

	case 2:
		pCanLog = NULL;
		/* sa_cand_method_2(pAd, pSAParam, pTrainEntry); */
		break;

	case 3:
		pCanLog = sa_cand_method_3(pAd, pSAParam, pTrainEntry);
		break;

	case 4:
		pCanLog = sa_cand_method_4(pAd, pSAParam, pTrainEntry);
		break;

	default:
		pCanLog = NULL;
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR,
				 ("%s():Invalid candMethod(%d)\n",
				  __func__, pSAParam->candMethod));
		break;
	}

	if (pCanLog) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("FinalDecision with Method %d(stage=%d):\n",
				  pSAParam->candMethod, pTrainEntry->trainStage));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("\tAntCandidate is Ant:0x%x-%d(PER:%d,txCnt:%d)\n",
				  pCanLog->antPattern, pCanLog->patternOffset,
				  pCanLog->PER, pCanLog->txCnt));
		pTrainEntry->pCanTrainInfo = pCanLog;
		pTrainEntry->canAntPattern = pCanLog->antPattern;
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR,
				 ("%s():Cannot found candidate by method %d!\n",
				  __func__, pSAParam->candMethod));
		return FALSE;
	}
}


INT sa_ant_adaptation(
	IN RTMP_ADAPTER * pAd)
{
	SMART_ANTENNA_STRUCT *pSAParam = NULL;
	RTMP_SA_TRAINING_PARAM *pTrainEntry;
	RTMP_SA_TRAIN_LOG_ELEMENT *pCurLog, *pCanLog = NULL;
	MAC_TABLE_ENTRY *pMacEntry;
	UINT32 curAnt, bTryNext = FALSE;
	unsigned long irqFlag;
	BOOLEAN bConsistantEnv = TRUE;
	/* UCHAR antWeight; */
	RTMP_IRQ_LOCK(&pAd->smartAntLock, irqFlag);

	if (!RTMP_SA_WORK_ON(pAd)) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("%s():SA status unsync(SAEnable=%d,SAParam=0x%lx)\n",
				  __func__, pAd->smartAntEnable, (ULONG)pAd->pSAParam));
		RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlag);
		return FALSE;
	}

	pSAParam = pAd->pSAParam;
	pTrainEntry = &pSAParam->trainEntry[0];
	pMacEntry = pTrainEntry->pMacEntry;

	if ((pMacEntry == NULL) || (pTrainEntry->pTrainInfo == NULL)) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("%s():MacEntry(0x%p) or pTrainInfo(0x%p) is NULL\n",
				  __func__, pMacEntry, pTrainEntry->pTrainInfo));
		RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlag);
		return FALSE;
	}

	if (IS_ENTRY_CLIENT(pMacEntry)) {
		RTMP_SA_TRAIN_LOG_ELEMENT *tmpLogPtr;
#ifdef SA_LUMP_SUM
		TX_STA_CNT0_STRUC		TxStaCnt0;
		TX_STA_CNT1_STRUC		StaTx1;
#endif /* SA_LUMP_SUM // */
		/* phase 1. save previously info to the TrainInfo */
#ifdef SA_LUMP_SUM
		/* Update statistic counter */
		NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);
#else
		NICUpdateFifoStaCounters(pAd);
#endif /* SA_LUMP_SUM // */
		/* update all rx signals */
		RtmpSAUpdateRxSignal(pAd);
		tmpLogPtr = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + pTrainEntry->patternOffset);

		if ((pTrainEntry->pCurTrainInfo != tmpLogPtr) || (pTrainEntry->pCurTrainInfo == NULL)) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR, ("%s():ERR!!pCurTrainInfo(0x%p) and PatternOffset(0x%p-%d) not equal!\n",
					 __func__, pTrainEntry->pCurTrainInfo, tmpLogPtr, pTrainEntry->patternOffset));
			pTrainEntry->pCurTrainInfo = tmpLogPtr;
		}

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():curAnt=0x%x-%d, TStage=%d,TWeight=%d!\n",
				 __func__, pTrainEntry->curAntPattern, pTrainEntry->patternOffset,
				 pTrainEntry->trainStage, pTrainEntry->trainWeight));
		curAnt = pTrainEntry->curAntPattern;
		pCurLog = pTrainEntry->pCurTrainInfo;
		NdisGetSystemUpTime(&pCurLog->endTime);
		/* save the statistics and clean for netx round */
		pCurLog->antPattern = pTrainEntry->curAntPattern;
		pCurLog->patternOffset = pTrainEntry->patternOffset;
		sa_trainInfo_update(pAd, pTrainEntry, pCurLog, pSAParam->txNss);

		if (pCurLog->txCnt)
			pCurLog->PER = (pCurLog->txRtyCnt * 1000) / pCurLog->txCnt;
		else
			pCurLog->PER = 0xffffffff;

		/* sa_dump_train_log(pAd, pTrainEntry, pCurLog, pSAParam->txNss); */

		if (pTrainEntry->bLastRnd == 1) {
			ULONG nowTime;
			pTrainEntry->mcsStableCnt = 0;
			bTryNext = FALSE;
			pTrainEntry->bLastRnd = 0;
			/*
				check the RSSI of pBaseInfo and pCurLog info to make sure the
				whole training progress is in a consistant environment
			*/
			bConsistantEnv = sa_chk_train_consistence(pAd, pSAParam, pTrainEntry);

			if (bConsistantEnv == FALSE) {
				pTrainEntry->trainStage = SA_INIT_STAGE;
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("EndOfTraining:\n"));
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
						 ("Ignore this round, env not consistent!\n"));
				goto ignoreIt;
			}

			if (sa_get_cand(pAd, pSAParam, pTrainEntry) == TRUE) {
				pCanLog = pTrainEntry->pCanTrainInfo;
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("EndOfTraining:\n"));
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
						 ("The Candidate Ant is:0x%x\n", pTrainEntry->canAntPattern));
				sa_dump_train_log(pAd, pTrainEntry, pCanLog, pSAParam->txNss);
				/* Apply this new antenna pattern */
				sa_set_antPattern(pAd, pSAParam, curAnt, pTrainEntry->canAntPattern);
				sa_get_curAntPattern(pSAParam, pTrainEntry);
				pTrainEntry->pCurTrainInfo = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + pTrainEntry->patternOffset);
			} else {
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("EndOfTraining:\n"));
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
						 ("Cannot found Candidate Ant, use origianl Ant :0x%x\n",
						  pTrainEntry->antBaseInfo.antPattern));
				/* go back to original antenna pattern */
				sa_set_antPattern(pAd, pSAParam, curAnt, pTrainEntry->antBaseInfo.antPattern);
				sa_get_curAntPattern(pSAParam, pTrainEntry);
				pTrainEntry->pCurTrainInfo = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + pTrainEntry->patternOffset);
				pTrainEntry->trainStage = SA_INIT_STAGE;
				goto ignoreIt;
			}

			/* If mode is one-shot, change the mode as NONE */
			if (pSAParam->saMode == SA_MODE_ONESHOT) {
				pSAParam->saMode = SA_MODE_NONE;
				pTrainEntry->trainStage = SA_INVALID_STAGE;
			}

			if (pSAParam->saMode == SA_MODE_AUTO) {
				MAC_TABLE_ENTRY *pEntry = pTrainEntry->pMacEntry;
				/* int cnt; */
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
						 ("Modify trainStage for AutoMode\n"));
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
						 ("\tBefore:trainStage=%s,initAnt=0x%x,confirmAnt=0x%x!\n",
						  saStage[pTrainEntry->trainStage],
						  pTrainEntry->ant_init_stage,
						  pTrainEntry->ant_confirm_stage));

				if (pTrainEntry->trainStage == SA_INIT_STAGE) {
					if (pSAParam->bSkipConfStage) {
						MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
								 ("\t\tskip confirm stage, Change from Init to Monitor stage:\n"));
						pTrainEntry->ant_init_stage = pTrainEntry->canAntPattern;
						pTrainEntry->ant_confirm_stage = pTrainEntry->canAntPattern;
						pTrainEntry->trainStage = SA_MONITOR_STAGE;
					} else {
						MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
								 ("\t\tChange from Init to Confirm stage:\n"));
						pTrainEntry->ant_init_stage = pTrainEntry->canAntPattern;
						pTrainEntry->trainStage = SA_CONFIRM_STAGE;
						pTrainEntry->ant_confirm_stage = 0;
					}

					/* update trianing info for antenna patterns which has txCnt larger than average TxCnt */
					sa_update_ant_weight(pSAParam, pTrainEntry, ANT_WEIGHT_CAND_LOW, ANT_WEIGHT_SCAN_AVG);
					/* We need to  record the RSSI sample here for fast tunning */
					reset_mac_entry_stats(pEntry);

					if (pCanLog) {
						int i;
						NdisZeroMemory(&pEntry->baseAvgRSSI[0], 3 * sizeof(CHAR));

						for (i = 0; i < pSAParam->txNss; i++) {
							if (pCanLog->sumRSSI[i]) {
#error "Need to fix ConvertToRssi() before compile this feature"
								pEntry->baseAvgRSSI[i] = ConvertToRssi(pAd, pCanLog->sumRSSI[i] / pCanLog->cntRSSI[i], i);
							}

							MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
									 ("\t\t\tAnt%d:Rssi=%d(cnt=%d, sum=%d)\n",
									  i, pEntry->baseAvgRSSI[i],
									  pCanLog->cntRSSI[i], pCanLog->sumRSSI[i]));
						}
					}
				} else if (pTrainEntry->trainStage == SA_CONFIRM_STAGE) {
					if (pTrainEntry->canAntPattern != pTrainEntry->ant_init_stage) {
						if (pTrainEntry->trainWeight == ANT_WEIGHT_SCAN_ALL) {
							/* no change, we keep in the confirm stage now! */
							pTrainEntry->trainStage = SA_CONFIRM_STAGE;
							pTrainEntry->ant_init_stage = pTrainEntry->canAntPattern;
							pTrainEntry->ant_confirm_stage = 0;
						} else {
							/* we go back to init stage, because previously we just scan the sub-group */
							pTrainEntry->trainStage = SA_INIT_STAGE;
						}
					} else {
						pTrainEntry->ant_confirm_stage = pTrainEntry->canAntPattern;
						pTrainEntry->trainStage = SA_MONITOR_STAGE;
						/* reset original RSSI and copy the pCanLog->RSSI to baseRSSI[] */
						reset_mac_entry_stats(pEntry);
						MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE, ("\t\tChange from Confirm to Monitor stage:\n"));

						if (pCanLog) {
							int i;
							NdisZeroMemory(&pEntry->baseAvgRSSI[0], 3 * sizeof(CHAR));

							for (i = 0; i < pSAParam->txNss; i++) {
								if (pCanLog->sumRSSI[i])
									pEntry->baseAvgRSSI[i] = ConvertToRssi(pAd, pCanLog->sumRSSI[i] / pCanLog->cntRSSI[i], i);

								MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
										 ("\t\t\tAnt%d:Rssi=%d(cnt=%d, sum=%d)\n",
										  i, pEntry->baseAvgRSSI[i],
										  pCanLog->cntRSSI[i], pCanLog->sumRSSI[i]));
							}
						}
					}
				}

				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
						 ("\tAfter:trainStage=%d,initAnt=0x%x,confirmAnt=0x%x!\n",
						  pTrainEntry->trainStage,
						  pTrainEntry->ant_init_stage,
						  pTrainEntry->ant_confirm_stage));
			}

ignoreIt:
			NdisGetSystemUpTime(&nowTime);
			pTrainEntry->time_to_start = nowTime + pSAParam->trainDelay * OS_HZ;
		} else {
			unsigned int offset, cleanLen;
			/* Select the antPattern from TrainingSeq and set to GPIO */
			sa_get_nextAntPattern(pSAParam, pTrainEntry, pTrainEntry->trainWeight, ANT_SELECT_IGNORE_BASE);

			if (pTrainEntry->curAntPattern == 0)
				sa_get_nextAntPattern(pSAParam, pTrainEntry, pTrainEntry->trainWeight, ANT_SELECT_BASE);

			/* ready for next antenna training procedure */
			pTrainEntry->pCurTrainInfo = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + pTrainEntry->patternOffset);
			pCurLog = pTrainEntry->pCurTrainInfo;
			offset = (unsigned int)(&(((RTMP_SA_TRAIN_LOG_ELEMENT *)NULL)->srtTime));
			cleanLen = sizeof(RTMP_SA_TRAIN_LOG_ELEMENT) - offset;
			NdisZeroMemory((PUCHAR)(&pCurLog->srtTime), cleanLen);
			NdisGetSystemUpTime(&pCurLog->srtTime);
			pCurLog->txMcs = pMacEntry->HTPhyMode.field.MCS;
			sa_set_antPattern(pAd, pSAParam, curAnt, pTrainEntry->curAntPattern);

			if (is_ant_equal(pTrainEntry->antBaseInfo.antPattern, pTrainEntry->curAntPattern))
				pTrainEntry->bLastRnd = 1;

			bTryNext = TRUE;
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
					 ("StartToTestAntPattern:0x%x(time=0x%lx!)\n",
					  pTrainEntry->curAntPattern,
					  pTrainEntry->pCurTrainInfo->srtTime));
		}

		/* update trainging status of the TrainEntry */
		pTrainEntry->bTraining = bTryNext;
	}

	RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlag);

	if ((bTryNext == TRUE) && (pSAParam))
		RTMPSetTimer(&pSAParam->saSwitchTimer, pSAParam->chkPeriod);

	return TRUE;
}


VOID sa_switch_exec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;;
	sa_ant_adaptation(pAd);
	return;
}



/******************************************************************************

	Public functions

******************************************************************************/

INT RtmpSAChkAndGo(
	IN RTMP_ADAPTER *pAd)
{
	SMART_ANTENNA_STRUCT *pSAParam;
	RTMP_SA_TRAINING_PARAM *pTrainEntry;
	MAC_TABLE_ENTRY *pMacEntry;
	ULONG nowTime;
#ifndef SA_LUMP_SUM
	UINT32 totalTxCnt;
#endif /* SA_LUMP_SUM // */
	BOOLEAN bNeedTune = FALSE;
	unsigned long irqFlag;
	/* UCHAR antWeight = ANT_WEIGHT_SCAN_ALL; */
	RTMP_IRQ_LOCK(&pAd->smartAntLock, irqFlag);

	if (!RTMP_SA_WORK_ON(pAd)) {
		RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlag);
		return FALSE;
	}

	pSAParam = pAd->pSAParam;
	pTrainEntry = &pSAParam->trainEntry[0];
	pMacEntry = pTrainEntry->pMacEntry;

	/* update Rx signals */
	if (pTrainEntry->bTraining == FALSE)
		RtmpSAUpdateRxSignal(pAd);

	if (pSAParam->bStaChange || pSAParam->bRssiChange) {
		BOOLEAN bCancel;
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
				 ("%s():StaChange=%d,RssiChange=%d!\n",
				  __func__, pSAParam->bStaChange,
				  pSAParam->bRssiChange));

		if (pTrainEntry->bTraining == TRUE)
			RTMPCancelTimer(&pSAParam->saSwitchTimer, &bCancel);

		sa_select_target_train_entry(pAd, pSAParam);
		pTrainEntry->trainStage = SA_INIT_STAGE;
		pTrainEntry->trainWeight = ANT_WEIGHT_SCAN_ALL;
		/* get current physically applied antenna pattern */
		sa_get_curAntPattern(pSAParam, pTrainEntry);

		if (pTrainEntry->curAntPattern == 0)
			sa_get_nextAntPattern(pSAParam, pTrainEntry, pTrainEntry->trainWeight, ANT_SELECT_FIRST);

		pTrainEntry->pCurTrainInfo = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + pTrainEntry->patternOffset);
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("%s(): The SmartAnt StaChanged with antPattern(0x%x-%d), pCurLog(0x%p, 0x%x-%d)\n",
				  __func__, pTrainEntry->curAntPattern, pTrainEntry->patternOffset,
				  pTrainEntry->pCurTrainInfo, pTrainEntry->pCurTrainInfo->antPattern,
				  pTrainEntry->pCurTrainInfo->patternOffset));
		pTrainEntry->mcsStableCnt = 0;
		NdisGetSystemUpTime(&nowTime);
		pTrainEntry->time_to_start = nowTime + pSAParam->trainDelay * OS_HZ;
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():set time_to_start=0x%lx\n",
				 __func__, pTrainEntry->time_to_start));
		pSAParam->bStaChange = FALSE;
		pSAParam->bRssiChange = FALSE;
		/* the pMacEntry may changed, re-assign it here */
		pMacEntry = pTrainEntry->pMacEntry;
	}

	if ((pMacEntry == NULL) || (pSAParam->saMode < SA_MODE_ONESHOT)) {
		RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlag);
		return FALSE;
	}

	if (IS_ENTRY_CLIENT(pMacEntry)) {
		BOOLEAN doTrain = FALSE;
		int i;

		if (pTrainEntry->bTraining == TRUE) {
			RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlag);
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_INFO,
					 ("%s():Entry in SA Training, Skip the check!\n",
					  __func__));
			return FALSE;
		}

		/* Update the train log and calculate the RSSI first */
		if (pTrainEntry->trainStage != SA_INVALID_STAGE) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
					 ("%s(0x%lx):wcid:%d,TS(%s),cAnt(0x%x-%d),bAnt(0x%x-%d),get RSSI Values!\n",
					  __func__, nowTime, pMacEntry->wcid, saStage[pTrainEntry->trainStage],
					  pTrainEntry->curAntPattern, pTrainEntry->patternOffset,
					  pTrainEntry->antBaseInfo.antPattern, pTrainEntry->antBaseInfo.patternOffset));
			sa_trainInfo_update(pAd, pTrainEntry, pTrainEntry->pCurTrainInfo, pSAParam->txNss);
		}

		NdisGetSystemUpTime(&nowTime);

		/* Depends on training stage, do different check */
		if (pTrainEntry->trainStage == SA_MONITOR_STAGE) {
			BOOLEAN bValidBase = FALSE, bValidRssi = FALSE, bRssiChanged = FALSE;

			for (i = 0; i < pSAParam->txNss; i++) {
				bValidRssi |= (pMacEntry->curAvgRSSI[i] != 0);

				if (pMacEntry->baseAvgRSSI[i]) {
					bValidBase = TRUE;

					if (((pMacEntry->curAvgRSSI[i] - pMacEntry->baseAvgRSSI[i]) > pSAParam->rssiVar) ||
						((pMacEntry->baseAvgRSSI[i] - pMacEntry->curAvgRSSI[i]) > pSAParam->rssiVar)
					   )
						bRssiChanged = TRUE;
				}
			}

			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
					 ("\tMonitorStage,Check RSSI variance only(prevRSSI=%d,%d,%d,cur=%d,%d,%d),bVB:%d,bVR:%d,bRC:%d!\n",
					  pMacEntry->baseAvgRSSI[0], pMacEntry->baseAvgRSSI[1], pMacEntry->baseAvgRSSI[2],
					  pMacEntry->curAvgRSSI[0], pMacEntry->curAvgRSSI[1], pMacEntry->curAvgRSSI[2],
					  bValidBase, bValidRssi, bRssiChanged));

			if (bValidRssi && bRssiChanged) {
				doTrain = FALSE; /* here we just set the stage as init, and waiting for next time to do it! */
				pTrainEntry->time_to_start = nowTime + pSAParam->trainDelay * OS_HZ;
				pTrainEntry->mcsStableCnt = 0;
				pTrainEntry->trainStage = SA_INIT_STAGE;
				pTrainEntry->ant_init_stage = pTrainEntry->ant_confirm_stage = 0;
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
						 ("\tStageChange(0x%lx, %s -> %s): Condition RSSI variance Hit(rssiVar=%d)!!\n",
						  nowTime, saStage[SA_MONITOR_STAGE], saStage[SA_INIT_STAGE], pSAParam->rssiVar));
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
						 ("\t\tOrigRssi(%d,%d,%d), curRssi(%d,%d,%d)\n",
						  pMacEntry->baseAvgRSSI[0], pMacEntry->baseAvgRSSI[1], pMacEntry->baseAvgRSSI[2],
						  pMacEntry->curAvgRSSI[0], pMacEntry->curAvgRSSI[1], pMacEntry->curAvgRSSI[2]));
			}

			if ((bValidBase == FALSE) && (bValidRssi == TRUE) &&
				(pTrainEntry->trainStage == SA_MONITOR_STAGE)) {
				/* TODO: Shall we consider the case if any RSSI is zero? */
				NdisMoveMemory(&pMacEntry->baseAvgRSSI[0], &pMacEntry->curAvgRSSI[0], 3);
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
						 ("\tMonitorStage:Init RSSI as (%d,%d,%d)\n",
						  pMacEntry->baseAvgRSSI[0], pMacEntry->baseAvgRSSI[1],
						  pMacEntry->baseAvgRSSI[2]));
			}

			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
					 ("\t(doTrain=%d):STA(wcid=%d) prevRssi=%d,%d,%d, curRssi=%d,%d,%d!\n",
					  doTrain, pMacEntry->wcid,
					  pMacEntry->baseAvgRSSI[0], pMacEntry->baseAvgRSSI[1], pMacEntry->baseAvgRSSI[2],
					  pMacEntry->curAvgRSSI[0], pMacEntry->curAvgRSSI[1], pMacEntry->curAvgRSSI[2]));
		}

		if ((pTrainEntry->trainStage == SA_INIT_STAGE) ||
			(pTrainEntry->trainStage == SA_CONFIRM_STAGE)
		   ) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
					 ("\tCheck mcsStableCnt(nowT=0x%lx,TtoStart=0x%lx,BW:MCS=%d:%d,MSC:Bnd=%d:%d)\n",
					  nowTime, pTrainEntry->time_to_start,
					  pMacEntry->HTPhyMode.field.BW,
					  pMacEntry->HTPhyMode.field.MCS,
					  pTrainEntry->mcsStableCnt, pSAParam->saMcsBound));

			if ((pTrainEntry->mcsStableCnt >= pSAParam->saMsc) &&
				(pMacEntry->HTPhyMode.field.MCS <= pSAParam->saMcsBound) &&
				RTMP_TIME_AFTER(nowTime, pTrainEntry->time_to_start)
			   )
				doTrain = TRUE;

			/* New factor used for weighted selection for antenna training */
			pTrainEntry->trainWeight = ANT_WEIGHT_SCAN_ALL;

			if (doTrain == TRUE) {
				if (pTrainEntry->trainStage == SA_CONFIRM_STAGE) {
					BOOLEAN bGoFastTune;
					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tIn confirm stage, check need to do fast tunning\n"));
					/* check if the RSSI, false CCA, no New STA connected */
					bGoFastTune = sa_can_do_fast_tune(pSAParam, pTrainEntry);

					if (bGoFastTune)
						pTrainEntry->trainWeight = ANT_WEIGHT_SCAN_AVG;

					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tbGoFastTune=%d!trainWeight=%d!\n", bGoFastTune, pTrainEntry->trainWeight));
				}

				if (pTrainEntry->trainStage == SA_INIT_STAGE) {
					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\ttrainStage is INIT stage, reset all trainWeight!\n"));
					/* update trianing info for antenna patterns which has txCnt larger than average TxCnt */
					sa_update_ant_weight(pSAParam, pTrainEntry, ANT_WEIGHT_CAND_INIT, ANT_WEIGHT_SCAN_ALL);
				}
			}
		}

		if (doTrain &&
			/*(totalTxCnt > SA_DEFAULT_TX_CNT) &&*/
			(pSAParam->saMode >= SA_MODE_ONESHOT)) {
			RTMP_SA_TRAIN_LOG_ELEMENT *pBaseInfo = &pTrainEntry->antBaseInfo;
			RTMP_SA_TRAIN_LOG_ELEMENT *pCurInfo = pTrainEntry->pCurTrainInfo;
			pMacEntry->orgTxRateCol = pMacEntry->CurrTxRate;
			sa_get_curAntPattern(pSAParam, pTrainEntry);
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
					 ("\n\nGO!HAWK!GO!(%d-%02x:%02x:%02x:%02x:%02x:%02x, antPattern=0x%x-%d)\n",
					  pMacEntry->wcid, PRINT_MAC(pMacEntry->Addr),
					  pTrainEntry->curAntPattern, pTrainEntry->patternOffset));
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
					 ("\tTxMcs(BW:MCS=%s:%d),mcsStableCnt=%d,TP=%d ms,TStage=%d,TWeight=%d\n",
					  phyMode[pMacEntry->HTPhyMode.field.MODE],
					  pMacEntry->HTPhyMode.field.MCS,
					  pTrainEntry->mcsStableCnt,
					  pSAParam->chkPeriod,
					  pTrainEntry->trainStage,
					  pTrainEntry->trainWeight));

			/* dump the MCS of each involved station */
			for (i = 0; i < SA_ENTRY_MAX_NUM; i++) {
				RTMP_SA_TRAINING_PARAM *pTmpTrainEntry;
				MAC_TABLE_ENTRY *pTmpEntry;
				pTmpTrainEntry = &pSAParam->trainEntry[i];
				pTmpEntry = pTmpTrainEntry->pMacEntry;

				if (pTmpEntry) {
					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
							 ("\tSTA(%02x:%02x:%02x:%02x:%02x:%02x):TxMcs(PhyMode:BW:MCS=%s:%d:%d), mcsStableCnt=%d\n",
							  PRINT_MAC(pTmpEntry->Addr), phyMode[pTmpEntry->HTPhyMode.field.MODE],
							  pTmpEntry->HTPhyMode.field.BW, pTmpEntry->HTPhyMode.field.MCS,
							  pTmpTrainEntry->mcsStableCnt));
				} else {
					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
							 ("\tSTA(%02x:%02x:%02x:%02x:%02x:%02x):No Associated MacEntry!\n",
							  PRINT_MAC(pTmpEntry->Addr)));
				}
			}

			/* save the info to original and zero candidate info */
			NdisZeroMemory(pBaseInfo, sizeof(RTMP_SA_TRAIN_LOG_ELEMENT));

			if (pCurInfo) {
				if (is_ant_equal(pTrainEntry->curAntPattern, pCurInfo->antPattern) == FALSE) {
					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
							 ("ERROR! Info of pCurTrainInfo(0x%x-%d) and pTrainEntry(0x%x-%d) not match,Reset BaseInfo!\n",
							  pCurInfo->antPattern, pCurInfo->patternOffset,
							  pTrainEntry->curAntPattern, pTrainEntry->patternOffset));
				}

				NdisMoveMemory(pBaseInfo, pTrainEntry->pCurTrainInfo, sizeof(RTMP_SA_TRAIN_LOG_ELEMENT));
				pBaseInfo->txMcs = pMacEntry->HTPhyMode.field.MCS;
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
						 ("\tUpdate CurLogInfo(Ant:0x%x-%d, RSSI:%d,%d,%d) to baseInfo(Ant:0x%x-%d, RSSI:%d,%d,%d)\n",
						  pCurInfo->antPattern, pCurInfo->patternOffset,
						  pCurInfo->avgRSSI[0], pCurInfo->avgRSSI[1], pCurInfo->avgRSSI[2],
						  pBaseInfo->antPattern, pBaseInfo->patternOffset,
						  pBaseInfo->avgRSSI[0], pBaseInfo->avgRSSI[1], pBaseInfo->avgRSSI[2]));
			} else {
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
						 ("ERROR! the TrainInfo of pTrainEntry->pCurTrainInfo is NULL!\n"));
			}

			/* Calculate the PER/txRtyCnt base on current info */
#ifdef SA_LUMP_SUM
			pTrainEntry->sumTxCnt = 0;
			pTrainEntry->sumTxRtyCnt = 0;
			pTrainEntry->sumTxFailCnt = 0;
#else
			totalTxCnt = pMacEntry->saLstTxNoRtyCnt +
						 pMacEntry->saLstTxRtyCnt +
						 pMacEntry->saLstTxFailCnt;
			pBaseInfo->txNoRtyCnt = pMacEntry->saLstTxNoRtyCnt;
			pBaseInfo->txRtyCnt = pMacEntry->saLstTxRtyCnt + pMacEntry->saLstTxFailCnt;
			pBaseInfo->txRtyFailCnt = pMacEntry->saLstTxFailCnt;
			pBaseInfo->txCnt = totalTxCnt;

			if (pBaseInfo->txCnt != 0)
				pBaseInfo->PER = (pBaseInfo->txRtyCnt * 100) / pBaseInfo->txCnt;
			else
				pBaseInfo->PER = 0xffffffff;

			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
					 ("BaseAntPattern=0x%x,offset=%d,txCnt=(T:%d,S:%d,R:%d,F:%d), PER=%d\n",
					  pBaseInfo->antPattern, pBaseInfo->patternOffset, pBaseInfo->txCnt,
					  pBaseInfo->txNoRtyCnt, pBaseInfo->txRtyCnt,
					  pBaseInfo->txRtyFailCnt, pBaseInfo->PER));
#endif /* SA_LUMP_SUM // */
			/* Select the antPattern from TrainingSeq and set to GPIO */
			pTrainEntry->curAntPattern = 0;
			pTrainEntry->patternOffset = 0;
			pTrainEntry->bLastRnd = 0;
			sa_get_nextAntPattern(pSAParam, pTrainEntry, pTrainEntry->trainWeight, ANT_SELECT_IGNORE_BASE);
			sa_set_antPattern(pAd, pSAParam, pBaseInfo->antPattern, pTrainEntry->curAntPattern);
			pTrainEntry->pCurTrainInfo = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + pTrainEntry->patternOffset);
			reset_trainlog_stats_record(pTrainEntry->pCurTrainInfo);
			NdisGetSystemUpTime(&pTrainEntry->pCurTrainInfo->srtTime);
			pTrainEntry->pCurTrainInfo->txMcs = pMacEntry->HTPhyMode.field.MCS;
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
					 ("StartToTestAntPattern:0x%x(time=0x%lx!)\n",
					  pTrainEntry->curAntPattern, pTrainEntry->pCurTrainInfo->srtTime));
			bNeedTune = TRUE;
		}

		/* block the rate adaptation when we start to do antenna training! */
		pTrainEntry->bTraining = bNeedTune;
	}

	RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlag);

	if (bNeedTune) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Call SA SwitchTimer!\n\n"));
		RTMPSetTimer(&pSAParam->saSwitchTimer, pSAParam->chkPeriod);
	}

	return TRUE;
}


INT RtmpSATrainInfoUpdate(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_SA_TRAINING_PARAM *pTrainEntry,
	IN RXWI_STRUC * pRxWI,
	IN UCHAR txNss)
{
	RTMP_SA_TRAIN_LOG_ELEMENT *pTrainLog;
	UCHAR idx1, idx2, idx3;
	pTrainLog = pTrainEntry->pCurTrainInfo;

	if (pTrainLog == NULL)
		return -1;

#ifdef SA_DBG
	/* record the Rssi distribution */
	idx1 = (pRxWI->RXWI_O.RSSI0 >> 2);

	if (idx1 < 32)
		pTrainLog->rssiDist[0][idx1]++;
	else
		pTrainLog->rssiDist[0][32]++;

	idx2 = (pRxWI->RXWI_O.RSSI1 >> 2);

	if (idx2 < 32)
		pTrainLog->rssiDist[0][idx2]++;
	else
		pTrainLog->rssiDist[0][32]++;

	idx3 = (pRxWI->RXWI_O.RSSI2 >> 2);

	if (idx3 < 32)
		pTrainLog->rssiDist[0][idx3]++;
	else
		pTrainLog->rssiDist[0][32]++;

	/* record the SNR dirstibution */
	idx1 = (pRxWI->RXWI_O.SNR0 >> 3);

	if (idx1 < 32)
		pTrainLog->SNRDist[0][idx1]++;
	else
		pTrainLog->SNRDist[0][32]++;

	idx2 = (pRxWI->RXWI_O.SNR1 >> 3);

	if (idx2 < 32)
		pTrainLog->SNRDist[1][idx2]++;
	else
		pTrainLog->SNRDist[1][32]++;

#endif /* SA_DBG // */

	/* RSSI */
	if (pRxWI->RXWI_O.RSSI0) {
		pTrainLog->sumRSSI[0] += (pRxWI->RXWI_O.RSSI0 & 0xff);
		pTrainLog->cntRSSI[0]++;
	}

	if (pRxWI->RXWI_O.RSSI1) {
		pTrainLog->sumRSSI[1] += (pRxWI->RXWI_O.RSSI1 & 0xff);
		pTrainLog->cntRSSI[1]++;
	}

	if (pRxWI->RXWI_O.RSSI2) {
		pTrainLog->sumRSSI[2] += (pRxWI->RXWI_O.RSSI2 & 0xff);
		pTrainLog->cntRSSI[2]++;
	}

	/* SNR */
	if (pRxWI->RXWI_O.SNR0) {
		pTrainLog->sumSNR[0] += (pRxWI->RXWI_O.SNR0 & 0xff);
		pTrainLog->cntSNR[0]++;
	}

	if (pRxWI->RXWI_O.SNR1) {
		pTrainLog->sumSNR[1] += (pRxWI->RXWI_O.SNR1 & 0xff);
		pTrainLog->cntSNR[1]++;
	}

	pTrainLog->rxCnt++;
	return 0;
}


INT sa_pkt_radio_info_update(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	SMART_ANTENNA_STRUCT *pSAParam = pAd->pSAParam;
	RTMP_SA_TRAINING_PARAM *pTrainEntry = &pSAParam->trainEntry[0];

	if (pRxBlk->rx_signal.raw_rssi[0]) {
		pEntry->curRSSI[0] += (pRxBlk->rx_signal.raw_rssi[0] & 0xff);
		pEntry->cntRSSI[0]++;
	} else {
		pEntry->rssi_zero_cnt[0]++;
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE, ("Err!Receive A frame with RSSI0=%d!TotalZeroCnt=%d!\n",
				 pRxBlk->rx_signal.raw_rssi[0], pEntry->rssi_zero_cnt[0]));
	}

	if (pRxBlk->rx_signal.raw_rssi[1]) {
		pEntry->curRSSI[1] += (pRxBlk->rx_signal.raw_rssi[1] & 0xff);
		pEntry->cntRSSI[1]++;
	} else {
		pEntry->rssi_zero_cnt[1]++;
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE, ("Err!Receive A frame with RSSI1=%d!TotalZeroCnt=%d!\n",
				 pRxBlk->rx_signal.raw_rssi[1], pEntry->rssi_zero_cnt[1]));
	}

	/* SNR */
	if (pRxBlk->rx_signal.raw_snr[0]) {
		pEntry->sumSNR[0] += (pRxBlk->rx_signal.raw_snr[0] & 0xff);
		pEntry->cntSNR[0]++;
	}

	if (pRxBlk->rx_signal.raw_snr[1]) {
		pEntry->sumSNR[1] += (pRxBlk->rx_signal.raw_snr[1] & 0xff);
		pEntry->cntSNR[1]++;
	}

	pEntry->saRxCnt++;

	if (pTrainEntry == pEntry->pTrainEntry)
		RtmpSATrainInfoUpdate(pAd, pTrainEntry, pRxBlk->pRxWI, pSAParam->txNss);
}


VOID RtmpSAUpdateRxSignal(
	IN RTMP_ADAPTER *pAd)
{
	unsigned long saflags;
	BOOLEAN bLocked = FALSE;
	MAC_TABLE_ENTRY *pEntry;
	INT macIdx;

	if (!in_interrupt()) {
		bLocked = TRUE;
		RTMP_IRQ_LOCK(pAd, saflags);
	}

	if (!RTMP_SA_WORK_ON(pAd)) {
		if (bLocked)
			RTMP_IRQ_UNLOCK(pAd, saflags);

		return;
	}

	/* TODO:Carter, check why start from 1 */
	for (macIdx = 1; VALID_UCAST_ENTRY_WCID(pAd, macIdx); macIdx++) {
		pEntry = &pAd->MacTab.Content[macIdx];
		if (pEntry == NULL)
			continue;

		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

		/* calculate the RSSI/SNR/PreSNR */
		sa_calc_rx_signal_info(pAd, pEntry);

		if ((pEntry->cntRSSI[0] >= 15) && (pEntry->avgRssi[0] != 0)) {
			if (pEntry->prevAvgRssi[0] != 0) {
				CHAR oldRssi, newRssi;
				oldRssi = pEntry->prevAvgRssi[0];
				newRssi = pEntry->avgRssi[0];

				if (((oldRssi > newRssi) && ((oldRssi - newRssi) > 15)) ||
					((oldRssi < newRssi) && ((newRssi - oldRssi) > 15))
				   ) {
					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
							 ("wcid:%d, RSSI variant(O:%d,N:%d) > 15, set bRssiChange!\n",
							  pEntry->wcid, oldRssi, pEntry->avgRssi[0]));
					pEntry->prevAvgRssi[0] = pEntry->avgRssi[0];
					pAd->pSAParam->bRssiChange = TRUE;
				}
			} else
				pAd->pSAParam->bRssiChange = TRUE;

			if (pAd->pSAParam->bRssiChange == TRUE) {
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
						 ("wcid:%d, Update RSSI0(O:%d,N:%d)\n",
						  pEntry->wcid, pEntry->prevAvgRssi[0],
						  pEntry->avgRssi[0]));
				pEntry->prevAvgRssi[0] = pEntry->avgRssi[0];
			}
		}

		/* clean all counters for next record */
		reset_mac_entry_stats(pEntry);
	}

	if (bLocked)
		RTMP_IRQ_UNLOCK(pAd, saflags);
}


/*
	Call to start the SmartAntenna mechanism
*/
int RtmpSAStop(
	IN RTMP_ADAPTER *pAd)
{
	SMART_ANTENNA_STRUCT *pSAParam;
	unsigned long irqFlags;
	int retVal = FALSE;
	BOOLEAN canceled = FALSE;
	RTMP_IRQ_LOCK(&pAd->smartAntLock, irqFlags);
	pSAParam = pAd->pSAParam;

	if (pSAParam) {
		pSAParam->trainEntry[0].trainStage = SA_INVALID_STAGE;
		pSAParam->bStaChange = FALSE;
		RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlags);
		RTMPCancelTimer(&pSAParam->saSwitchTimer, &canceled);
		RTMP_IRQ_LOCK(&pAd->smartAntLock, irqFlags);
	}

	pAd->smartAntEnable = FALSE;
	RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlags);
	return retVal;
}


/*
	Call to start the SmartAntenna mechanism, it's mainly enable the
	flag "smartAntEnable"
*/
int RtmpSAStart(RTMP_ADAPTER *pAd)
{
	int retVal = TRUE, i;
	unsigned long irqFlags;
	SMART_ANTENNA_STRUCT *pSAParam;
	RTMP_SA_TRAINING_PARAM *pSAStaEntry;
	/* RTMP_SA_TRAIN_LOG_ELEMENT *pTrainLog; */
	RTMP_IRQ_LOCK(&pAd->smartAntLock, irqFlags);
	pSAParam = pAd->pSAParam;

	/*
		sanity check
	*/
	if (!RTMP_SA_WORK_ON(pAd)) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():pAd->smartAntEnable=%d, pSAParam=0x%x\n",
				 __func__, pAd->smartAntEnable, (UINT32)pSAParam));
		pAd->smartAntEnable = FALSE;
		RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlags);
		return FALSE;
	}

	/* allocate related resource for SA */
	if ((pSAParam->agsp == NULL) || (pSAParam->pTrainSeq == NULL)) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():AGSP(0x%x)/TrainSeq(0x%x) not configured, use default!\n",
				 __func__, (UINT32)pSAParam->agsp, (UINT32)pSAParam->pTrainSeq));
		sa_train_db_exit(pAd, pSAParam);

		if (pSAParam->agsp && (pSAParam->agsp != DefaultAGSP)) {
			os_free_mem(pSAParam->agsp);
			pSAParam->agsp = NULL;
		}

		pSAParam->agsp = &DefaultAGSP[0];
		sa_train_db_init(pAd, pSAParam, &DefaultTrainSeq[0]);
	}

	if (pSAParam->pTrainMem == NULL) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Allocate memory for TrainLog failed!\n"));
		pAd->smartAntEnable = FALSE;
		RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlags);
		return FALSE;
	}

	/*
		Init pSAParam parameters
	*/
	/* set the default candidate selection method */
	if (pSAParam->candMethod == 0)
		pSAParam->candMethod = 4;

	/* If the candidate selection method is 3, set the default PER threshold as 80 */
	if ((pSAParam->candMethod == 3) && (pSAParam->trainCond == 0))
		pSAParam->trainCond = 80;

	/* If the candidate selection method is 4, set the default PER Upper bound as 50, i.e, 5% */
	if ((pSAParam->candMethod == 4) && (pSAParam->trainCond == 0))
		pSAParam->trainCond = 100;

	/*
		Start Smart Antenna algorithm for each supported peer
	*/
	for (i = 0; i < SA_ENTRY_MAX_NUM; i++) {
		/* Init the TrainEntry and associated TrainLogInfo space */
		pSAStaEntry = &pSAParam->trainEntry[i];
		retVal = sa_init_train_entry(pAd, pSAParam, pSAStaEntry, pSAStaEntry->pTrainInfo, NULL);

		if (retVal == FALSE) {
			RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlags);
			goto done;
		}

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():Entry[%d]: pTrainLog=0x%lx,Addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
				 __func__, i, (ULONG)pSAStaEntry->pTrainInfo,
				 PRINT_MAC(pSAStaEntry->macAddr)));
	}

	sa_select_target_train_entry(pAd, pSAParam);

	/* Init each specific train station entries */
	for (i = 0; i < SA_ENTRY_MAX_NUM; i++) {
		pSAStaEntry = &pSAParam->trainEntry[i];

		/* rtmp_sa_cfg_train_entry(pAd, pSAStaEntry); */
		if (NdisEqualMemory(&pSAStaEntry->macAddr[0], ZERO_MAC_ADDR, MAC_ADDR_LEN)) {
			int j;

			/* find first valid sta as the target //TODO:Carter, check why start from 1 */
			for (j = 1; VALID_UCAST_ENTRY_WCID(pAd, j); j++) {
				MAC_TABLE_ENTRY *pEntry = NULL;
				pEntry = &pAd->MacTab.Content[j];

				if ((NdisEqualMemory(&pEntry->Addr[0], ZERO_MAC_ADDR, MAC_ADDR_LEN) != TRUE) &&
					IS_ENTRY_CLIENT(pEntry) &&
					(pEntry->pTrainEntry == NULL)) {
					NdisMoveMemory(&pSAStaEntry->macAddr[0], &pEntry->Addr[0], MAC_ADDR_LEN);
					pSAStaEntry->pMacEntry = pEntry;
					pEntry->pTrainEntry = (void *)pSAStaEntry;
					break;
				}
			}
		}

		if (is_valid_train_entry(pAd, pSAParam, pSAStaEntry)) {
			ULONG nowTime;
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():Ready to do SA training for %02x:%02x:%02x:%02x:%02x:%02x!\n",
					 __func__, PRINT_MAC(pSAStaEntry->macAddr)));
			pSAStaEntry->trainStage = SA_INIT_STAGE;
			pSAStaEntry->trainWeight = ANT_WEIGHT_SCAN_ALL;
			/* get current physically applied antenna pattern */
			sa_get_curAntPattern(pSAParam, pSAStaEntry);

			if (pSAStaEntry->curAntPattern == 0)
				sa_get_nextAntPattern(pSAParam, pSAStaEntry, pSAStaEntry->trainWeight, ANT_SELECT_FIRST);

			pSAStaEntry->pCurTrainInfo = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pSAStaEntry->pTrainInfo + pSAStaEntry->patternOffset);
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s(): The SmartAnt start with antPattern(0x%x-%d), pCurLog(0x%p, 0x%x-%d)\n",
					 __func__, pSAStaEntry->curAntPattern, pSAStaEntry->patternOffset,
					 pSAStaEntry->pCurTrainInfo, pSAStaEntry->pCurTrainInfo->antPattern,
					 pSAStaEntry->pCurTrainInfo->patternOffset));
			NdisGetSystemUpTime(&nowTime);
			pSAStaEntry->time_to_start = nowTime + pSAParam->trainDelay * OS_HZ;
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():set time_to_start=0x%lx\n", __func__, pSAStaEntry->time_to_start));
			retVal = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():TrainEntry(%02x:%02x:%02x:%02x:%02x:%02x) is not valid\n",
					 __func__, PRINT_MAC(pSAStaEntry->macAddr)));
			/* retVal = FALSE; */
			/* pAd->smartAntEnable = FALSE; */
		}
	}

	RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlags);
done:

	if (retVal == FALSE)
		RtmpSAStop(pAd);

	return retVal;
}


/*
	Global functions used to hook to the driver
*/
int RtmpSAInit(RTMP_ADAPTER *pAd)
{
	SMART_ANTENNA_STRUCT *pSAParam;
	unsigned long irqFlags;
	int status;
	/* allocate/init resources and structures */
	os_alloc_mem(pAd, (UCHAR **)&pAd->pSAParam, sizeof(SMART_ANTENNA_STRUCT));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Allocate SAParam memory(0x%p) with size(0x%x)\n",
			 pAd->pSAParam, sizeof(SMART_ANTENNA_STRUCT)));

	if (pAd->pSAParam) {
		/* initailize the lock structure */
		NdisAllocateSpinLock(&pAd->smartAntLock);
		RTMP_IRQ_LOCK(&pAd->smartAntLock, irqFlags);
		pSAParam = pAd->pSAParam;
		NdisZeroMemory(pSAParam, sizeof(SMART_ANTENNA_STRUCT));
		RTMPInitTimer(pAd, &pSAParam->saSwitchTimer,
					  GET_TIMER_FUNCTION(sa_switch_exec), pAd, FALSE);
		pSAParam->saMode = SA_MODE_AUTO; /* SA_MODE_NONE; */
		pSAParam->txNss = pAd->Antenna.field.TxPath; /* SA_DEFAULT_TX_NSS */
		pSAParam->saMsc = SA_DEFAULT_MSC;
		pSAParam->saMcsBound = (pAd->Antenna.field.TxPath * 8) - 1;	/* SA_DEFAULT_MCS_BOUND */
		pSAParam->chkPeriod = SA_DEFAULT_CHK_PERIOD;
		pSAParam->maxAntTry = SA_DEFAULT_ANT_TRIAL;
		pSAParam->rssiThreshold = SA_DEFAULT_RSSI_THRESHOLD;
		pSAParam->trainDelay = 0;
		pSAParam->rssiVar = SA_DEFAULT_RSSI_VAR;
		pSAParam->bSkipConfStage = TRUE;
		pSAParam->bStaChange = FALSE;
		pSAParam->agsp = &DefaultAGSP[0];
#ifdef SA_TRAIN_SBS
		pSAParam->trainCond = SA_TRAIN_GRADUAL_APPROACH;
#endif /* SA_TRAIN_SBS // */
		status = sa_train_db_init(pAd, pSAParam, &DefaultTrainSeq[0]);

		if (status == FALSE)
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():DBInit failed!\n", __func__));

		/* set the first TrainSeq as the default antenna pattern */
		sa_set_antPattern(pAd, pSAParam, 0, DefaultTrainSeq[0]);
		/* kick to go */
		/* pAd->smartAntEnable = TRUE; */
		pAd->smartAntEnable = FALSE;
		RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlags);
	} else {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR,
				 ("Allocate Memory for SAParam failed\n"));
	}

	/* RtmpSAStart(pAd); */
	return TRUE;
}


int RtmpSAExit(RTMP_ADAPTER *pAd)
{
	SMART_ANTENNA_STRUCT *pSAParam;
	unsigned long irqFlags;
	BOOLEAN Cancelled = FALSE;
	pSAParam = pAd->pSAParam;

	if (pSAParam) {
		/* stop it now */
		RtmpSAStop(pAd);
		/* free resources */
		RTMP_IRQ_LOCK(&pAd->smartAntLock, irqFlags);
		sa_train_db_exit(pAd, pSAParam);

		if (pSAParam->agsp && (pSAParam->agsp != &DefaultAGSP[0]))
			os_free_mem(pSAParam->agsp);

		pSAParam->agsp = NULL;
		os_free_mem(pAd->pSAParam);
		pAd->pSAParam = NULL;
		RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqFlags);
		RTMPReleaseTimer(pAd, &pSAParam->saSwitchTimer,
						 &Cancelled);
		/* free the lock structure */
		NdisFreeSpinLock(pAd->smartAntLock);
	}

	return 0;
}


/*
	iwpriv ra0 set sa_enable=0~1
*/
void show_sa_cmd_usage(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("iwpriv usage for SmartAntenna Related Sub Commands:\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tiwpriv ra0 set sa\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tshow this help message out\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tiwpriv ra0 set sa=0~1\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t0: disable the SA mechanism\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t1: enable the SA mechanism\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tiwpirv ra0 set sa_msc=0~255\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tset the condition of stable MCS count for trigger the SA training algorithm\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\t0: run SA algorithm no matter MCS is in stable stage or not\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\t1<= x <=254: run SA algorithm when MCS stay in stable stage for x times\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\t255: reserved and should not use now\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tiwpriv ra0 set sa_sta=xx:xx:xx:xx:xx:xx\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tSet the target station used to support Smart Antenna\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tiwpriv ra0 set sa_agsp\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tiwpriv ra0 set sa_maxAntennaTry\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tiwpriv ra0 set sa_dbg\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tiwpriv ra0 set sa_tp\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tiwpriv ra0 set sa_ant=T0-T1\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tSet the static tx antenna as Antenna HeaderPin T0 and T1\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\t 1<= LowestPinBoundaryInAGSP <= T0, T1 <= HighestPinBoundayInAGSP <=255\n"));
	return;
}


/******************************************************************************

	IOCTL cmds used to set Smart Antenna related paramters

******************************************************************************/

INT Show_SA_DbgInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


/* TODO: */
/* TODO: Following codes is finished!!!!! */
/* TODO: */



INT	Set_DbgLogOn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	UINT Value = 0;
	ULONG bit = GET_BIT(DBG_CAT_HW);
	Value = (UINT) os_str_tol(arg, 0, 10);
	/* pAd->smartAntDbgOn = (Value == 1? TRUE : FALSE); */
	DebugCategory |= DBG_CAT_HW;
	DebugSubCategory[DebugLevel][bit] |= CATHW_SA;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
			 ("%s():Set DebugCategory=0x%lx, bit=%d, SubCat=0x%lx\n",
			  __func__, DebugCategory, bit, DebugSubCategory[DebugLevel][bit]));

	return TRUE;
}


/*
	iwpriv ra0 set sa_tc=1~4
	iwpriv ra0 set sa_tp=x ("x" in unit of ms)

	Note: sa_tc * sa_tp shall small than 400ms
*/
INT	Set_MaxAntennaTry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->pSAParam->maxAntTry = (UCHAR)os_str_tol(arg, 0, 10);
	return TRUE;
}


INT	Set_TestPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->pSAParam->chkPeriod = (int)os_str_tol(arg, 0, 10);
	return TRUE;
}


/*
	iwpriv ra0 show sainfo
*/
INT Show_SA_CfgInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	SMART_ANTENNA_STRUCT *pSAParam;
	RTMP_SA_TRAINING_PARAM *pStaEntry;
	RTMP_SA_AGSP_MAP *pAgspEntry;
	UINT32 *pTrainingId;
	int i;
	pSAParam = pAd->pSAParam;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Dump the SmartAntenna Configuration Info:\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tSAEnable=%s\n", pAd->smartAntEnable == TRUE ? "TRUE" : "FALSE"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tSADbg=%s\n", pAd->smartAntDbgOn == TRUE ? "ON" : "OFF"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\tpSAParam:\n"));

	if (pSAParam == NULL) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tThe SMART_ANTENNA_STRUCT is not initialized!\n\n"));
		return 0;
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tsaMode=%s\n", SA_MODE[pAd->pSAParam->saMode]));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tTxNss=%d\n", pSAParam->txNss));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tsaMcsStableCnt=%d\n", pSAParam->saMsc));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tsaMcsBound=%d\n", pSAParam->saMcsBound));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tmaxAntTry=%d\n", pSAParam->maxAntTry));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tTrainCond=%d\n", pSAParam->trainCond));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tcandMethod=%d\n", pSAParam->candMethod));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\ttrainDelay=%d\n", pSAParam->trainDelay));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tTrainingPeriod=%d\n", pSAParam->chkPeriod));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\trssiVar=%d\n", pSAParam->rssiVar));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tbSkipConfStage=%d\n", pSAParam->bSkipConfStage));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tbStaChange=%d\n", pSAParam->bStaChange));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tagspCnt=%d\n", pSAParam->agspCnt));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\ttrainSeqCnt=%d\n", pSAParam->trainSeqCnt));

	for (i = 0; i < SA_ENTRY_MAX_NUM; i++) {
		pStaEntry = &pSAParam->trainEntry[i];
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\tTrainEntry:\n"));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\tbStatic=%d\n", pStaEntry->bStatic));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\tRealTimeCntOfMcsInStableStage=%d\n", pStaEntry->mcsStableCnt));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\ttrainStage=%d\n", pStaEntry->trainStage));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\tbTraining=%s\n", pStaEntry->bTraining ? "TRUE" : "FALSE"));

		if (pStaEntry->curAntPattern == 0)
			sa_get_curAntPattern(pSAParam, pStaEntry);

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\tcurAntPattern=0x%x-%d\n",
				 pStaEntry->curAntPattern, pStaEntry->patternOffset));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\tMacAddr=%02x:%02x:%02x:%02x:%02x:%02x\n",
				 PRINT_MAC(pStaEntry->macAddr)));
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\tMapped MacTableEntry=0x%x\n", (UINT32)pStaEntry->pMacEntry));

		if (pStaEntry->pMacEntry) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\t\tWCID=%d\n", pStaEntry->pMacEntry->wcid));
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\t\tBW:MCS=%d:%d\n", pStaEntry->pMacEntry->HTPhyMode.field.BW,
					 pStaEntry->pMacEntry->HTPhyMode.field.MCS));
		}

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\tAGSP:\n"));
		pAgspEntry = pSAParam->agsp;

		if (pAgspEntry == NULL) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\t\tThe AGSP is not initialized, dump default one!\n"));
			pAgspEntry = &DefaultAGSP[0];
		}

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\t\tHeaderPin  GpioRegOffset  GpioBit\n"));

		while (memcmp(pAgspEntry, &AGSP_ENTRY_END, sizeof(*pAgspEntry)) !=  0) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\t\t%d\t0x%x\t%d\n",
					 pAgspEntry->hdrPin, pAgspEntry->regOffset, pAgspEntry->gpioBit));
			pAgspEntry++;
		}

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\tTrainingSequence(cnt=%d):\n", pSAParam->trainSeqCnt));
		pTrainingId = pSAParam->pTrainSeq;

		if (pTrainingId == NULL) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\t\tThe TrainingSequence is not initialized, dump default one!\n"));
			pTrainingId = &DefaultTrainSeq[0];
		}

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t\t\t"));

		while (*pTrainingId != 0) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, (" 0x%x", *pTrainingId));
			pTrainingId++;
		}

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\n"));
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\n"));
	return TRUE;
}


/*
	iwpriv ra0 set sa_tseq=0102:0304:0506
*/
int Set_SA_TrainSeq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *args)
{
	SMART_ANTENNA_STRUCT *pSAParam;
	UINT32 antPair, *pTrainSeq, *pTrainSeqLst;
	int entryCnt, i, bUseDefault = FALSE;
	char *ptr, *pPos, *token;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Input string is:%s\n", args));
	pSAParam = pAd->pSAParam;

	if (pSAParam == NULL)
		return FALSE;

	if (strcmp(args, "default") == 0) {
		bUseDefault = TRUE;

		if (pSAParam->agsp != &DefaultAGSP[0]) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR,
					 ("Error:The AGSP list is not specified yet!\n"));
			return FALSE;
		}

		pTrainSeqLst = &DefaultTrainSeq[0];
	} else {
		entryCnt = 1; /* reserve one for AGSP_TRAIN_SEQ_END */
		ptr = args;

		while ((pPos = strchr(ptr, ':')) !=  NULL) {
			entryCnt++;
			ptr = (pPos + 1);
		}

		if (strlen(ptr))
			entryCnt += 1;

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("entryCnt=%d\n", entryCnt));

		if (entryCnt < 2)
			return FALSE;

		/*
			Now it's time to allocate memory for the TrainSeq list and insert those
			entries to the list
		*/
		i = entryCnt * sizeof(UINT32);

		if (os_alloc_mem(pAd, (UCHAR **)&pTrainSeqLst, i) == NDIS_STATUS_FAILURE) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR,
					 ("Alloc mem for pTrainSeq failed\n"));
			return FALSE;
		}

		NdisZeroMemory(pTrainSeqLst, i);
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
				 ("Allocate memory for pTrainSeq is %d\n", i));
		pTrainSeq = pTrainSeqLst;
		pPos = args;
		i = 0;

		do {
			token = tokenParser(&pPos, ':');

			if (strlen(token)) {
				A2Hex(antPair, token);
				/* move to next entry tuple */
				*pTrainSeq = antPair;
				pTrainSeq++;
			}

			i++;

			if (i >= (entryCnt - 1)) {
				MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF,
						 ("Parsing whole TrainSeq list down, handle cnt=%d!\n", i));
				break;
			}
		} while (pPos);
	}

	if (pTrainSeqLst) {
		sa_train_db_init(pAd, pSAParam, pTrainSeqLst);
		sa_dump_train_seq(pSAParam->pTrainSeq);
	}

	return TRUE;
}


/*
	iwpriv ra0 set sa_tdelay=x
	set the training delay
*/
int Set_SA_TrainDelay_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *args)
{
	UINT32 delayTime;
	SMART_ANTENNA_STRUCT *pSAParam;
	delayTime = os_str_tol(args, NULL, 10);
	pSAParam = pAd->pSAParam;

	if (pSAParam == NULL || delayTime < 0)
		return FALSE;

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
			 ("SA trainDelay(0x%x) change to(0x%x)\n",
			  pSAParam->trainDelay, delayTime));
	pSAParam->trainDelay = delayTime;
	return TRUE;
}


int Set_SA_AntCand_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *args)
{
	UINT32 candMethod;
	SMART_ANTENNA_STRUCT *pSAParam;
	candMethod = os_str_tol(args, NULL, 10);

	if ((candMethod < 1) || (candMethod > 4))
		candMethod = 0;

	pSAParam = pAd->pSAParam;

	if (pSAParam) {
		pSAParam->candMethod = candMethod;
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():Set AntennaCandidateSelectMethod=%d!\n", __func__, pSAParam->candMethod));
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():SmartAntenna feature is not enabled yet\n", __func__));
		return FALSE;
	}
}


/*
	0x1: trainUpperMCS instead of train CurrentMCS
*/
int Set_SA_TrainCond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *args)
{
	UINT32 trainCond;
	SMART_ANTENNA_STRUCT *pSAParam;
	trainCond = os_str_tol(args, NULL, 10);
	pSAParam = pAd->pSAParam;

	if (pSAParam == NULL)
		return FALSE;

	/*
		// If the trainCond changed, we stop the on-going training sequence
		if (trainCond != pSAParam->trainCond)
		{
			RtmpSAStop(pAd);
		}
	*/
	pSAParam->trainCond = trainCond;
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
			 ("SA TrainCond(0x%x) change to(0x%x)\n",
			  trainCond, pSAParam->trainCond));
	return TRUE;
}


INT Set_SA_RssiVariance_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *args)
{
	UCHAR rssiVar;
	SMART_ANTENNA_STRUCT *pSAParam;
	rssiVar = os_str_tol(args, NULL, 10);
	pSAParam = pAd->pSAParam;

	if (pSAParam == NULL)
		return FALSE;

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
			 ("SA RSSI variance threshold (0x%x) change to(0x%x)\n",
			  pSAParam->rssiVar, rssiVar));
	pSAParam->rssiVar = rssiVar;
	return TRUE;
}


INT Set_SA_RssiThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *args)
{
	int rssiThold;
	SMART_ANTENNA_STRUCT *pSAParam;
	rssiThold = os_str_tol(args, NULL, 10);
	pSAParam = pAd->pSAParam;

	if (pSAParam == NULL)
		return FALSE;

	if (rssiThold <= -99 && rssiThold > -1)
		rssiThold = SA_DEFAULT_RSSI_THRESHOLD;

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
			 ("SA RSSI variance threshold (0x%x) change to(0x%x)\n",
			  pSAParam->rssiThreshold, rssiThold));
	pSAParam->rssiThreshold = rssiThold;
	return TRUE;
}



INT Set_SA_SkipConfirmStage_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *args)
{
	BOOLEAN bSkipConfStage;
	SMART_ANTENNA_STRUCT *pSAParam;
	bSkipConfStage = os_str_tol(args, NULL, 10);
	pSAParam = pAd->pSAParam;

	if (pSAParam == NULL)
		return FALSE;

	bSkipConfStage = ((bSkipConfStage == 1) ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_TRACE,
			 ("SA skip Confirm stage (0x%x) change to(0x%x)\n",
			  pSAParam->bSkipConfStage, bSkipConfStage));
	pSAParam->bSkipConfStage = bSkipConfStage;
	return TRUE;
}


/*
	The AGSP pin table cmd show set in following format:
		iwpriv ra0 set sa_agsp=HeadrPin-GPIO_Register_Offset-GPIO_Register_BIT

		for example: iwpriv ra0 set sa_agsp=1-a4-2:2-a4-3:5-a4-8

*/
int Set_SA_AGSP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *args)
{
	char *ptr, *pPos, *token, *subToken;
	int entryCnt, subCnt, i;
	SMART_ANTENNA_STRUCT *pSAParam;
	RTMP_SA_AGSP_MAP *pAgspEntry, *pAgspList = NULL;

	if (!args) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR,
				 ("Input without arguments\n"));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Input string is:\n"));
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("\t\t%s\n", args));
	pSAParam = pAd->pSAParam;

	if (pSAParam == NULL)
		return FALSE;

	if (strcmp(args, "default") == 0)
		pAgspList = &DefaultAGSP[0];
	else {
		/* Find how many entries need to added to the agsp list */
		entryCnt = 1; /* reserve one for AGSP_ENTRY_END */
		ptr = args;

		while ((pPos = strchr(ptr, ':')) !=  NULL) {
			entryCnt++;
			ptr = (pPos + 1);
		}

		if (strlen(ptr))
			entryCnt += 1;

		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("entryCnt=%d\n", entryCnt));

		if (entryCnt < 2)
			return FALSE;

		/*
			Now it's time to allocate memory for the agsp list and insert those
			entries to the list
		*/
		i = entryCnt * sizeof(RTMP_SA_AGSP_MAP);

		if (os_alloc_mem(pAd, (UCHAR **)&pAgspList, i) != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Allocate Memory for AGSP failed\n"));
			return FALSE;
		}

		NdisZeroMemory(pAgspList, i);
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Allocate memory for AGSP success(addr=0x%lx, size=%d)\n",
				 (ULONG)pAgspList, i));
		/*
			Now parsing each tokens and insert to the agsp list
		*/
		i = 0;
		pPos = args;
		pAgspEntry = pAgspList;

		do {
			token = tokenParser(&pPos, ':');

			if (strlen(token)) {
				subCnt = 0;
				ptr = token;

				do {
					subToken = tokenParser(&ptr, '-');

					if (strlen(subToken)) {
						switch (subCnt) {
						case 0:
							pAgspEntry->hdrPin = (UINT8)os_str_tol(subToken, 0, 10);
							break;

						case 1:
							A2Hex(pAgspEntry->regOffset, subToken);
							break;

						case 2:
							pAgspEntry->gpioBit = (UINT32)os_str_tol(subToken, 0, 10);
							break;

						default:
							break;
						}

						subCnt++;
					}
				} while ((ptr != NULL) && (subCnt < 3));

				if ((subCnt == 3) && is_valid_agsp_entry(pAd, pAgspEntry)) {
					/* move to next entry tuple */
					pAgspEntry++;
				}

				i++;

				if (i >= (entryCnt - 1)) {
					MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Parsing whole AGSP list down, handle cnt=%d!\n", i));
					break;
				}
			}
		} while (pPos);
	}

	/* Remove old one and insert new AGSP */
	if (pAgspList) {
		sa_train_db_exit(pAd, pSAParam);

		if (pSAParam->agsp && (pSAParam->agsp != &DefaultAGSP[0]))
			os_free_mem(pSAParam->agsp);

		pSAParam->agsp = pAgspList;

		if (pAgspList == &DefaultAGSP[0])
			sa_train_db_init(pAd, pSAParam, &DefaultTrainSeq[0]);
	}

	if (pSAParam->agsp)
		sa_dump_agsp(pSAParam->agsp);

	return TRUE;
}


/*
	iwpriv ra0 set sa_ant=T0-T1
*/
INT	Set_SA_StaticAntPair_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR pinVal[SA_TX_NSS_MAX_NUM];
	SMART_ANTENNA_STRUCT *pSAParam;
	RTMP_SA_AGSP_MAP *pAgspEntry;
	RTMP_SA_TRAINING_PARAM *pTrainEntry;
	char *token, *pPos;
	int i, cnt = 0;
	UINT32 antPattern = 0;
	RTMP_SA_TRAIN_LOG_ELEMENT *pBaseInfo;
	pSAParam = pAd->pSAParam;

	if (pSAParam == NULL) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Smart Antenna is not initialized!\n"));
		return FALSE;
	}

	/* Parsing the pin Header */
	pPos = arg;

	do {
		token = tokenParser(&pPos, '-');

		if (strlen(token) > 0) {
			pinVal[cnt] = (UCHAR)os_str_tol(token, 0, 10);
			cnt++;
		}

		if (cnt == SA_TX_NSS_MAX_NUM)
			break;
	} while (pPos);

	/* Check if the antPair is one element of the AGSP table */
	if (cnt != pSAParam->txNss) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Error: AntPairSetting(%d) not equal txNss(%d)!!!\n", cnt, pSAParam->txNss));
		return FALSE;
	}

	if (pSAParam->agsp) {
		pAgspEntry = pSAParam->agsp;

		while (0) { /* pAgspEntry->hdrPin != 0) */
			/* TODO: check if the AntPair is valid in the AGSP list!! */
		}
	} else {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():Error!! AGSP(0x%lx) or TrainSeq(0x%lx) is not set yet!",
				 __func__, (ULONG)pSAParam->agsp, (ULONG)pSAParam->pTrainSeq));
		return FALSE;
	}

	pTrainEntry = &pSAParam->trainEntry[0];

	for (i = 0; i < cnt; i++)
		antPattern |= pinVal[i] << (i * 8);

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("prevAntPattern=0x%x, newAntPattern=0x%x\n",
			 pTrainEntry->curAntPattern, antPattern));
	sa_set_antPattern(pAd, pSAParam, pTrainEntry->curAntPattern, antPattern);
	/* update antenna related data structures in pTrainEntry */
	sa_get_curAntPattern(pSAParam, pTrainEntry);

	if (pTrainEntry->pTrainInfo) {
		pTrainEntry->pCurTrainInfo = (RTMP_SA_TRAIN_LOG_ELEMENT *)(pTrainEntry->pTrainInfo + pTrainEntry->patternOffset);
		NdisZeroMemory(pTrainEntry->pCurTrainInfo, sizeof(RTMP_SA_TRAIN_LOG_ELEMENT));
		pTrainEntry->pCurTrainInfo->antPattern = pTrainEntry->curAntPattern;
		pTrainEntry->pCurTrainInfo->patternOffset = pTrainEntry->patternOffset;
	}

	pBaseInfo = &pTrainEntry->antBaseInfo;
	NdisZeroMemory(pBaseInfo, sizeof(RTMP_SA_TRAIN_LOG_ELEMENT));
	pBaseInfo->antPattern = antPattern;
	pBaseInfo->patternOffset = pTrainEntry->patternOffset;

	if (pTrainEntry->curAntPattern) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Static AntPattern=0x%x, AGSP Offset=%d!\n", pTrainEntry->curAntPattern, pTrainEntry->patternOffset));
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Static Set AntPattern(0x%x) failed!\n", pTrainEntry->curAntPattern));
		return FALSE;
	}
}


/*
	iwpriv ra0 set sa_txNss=1~4
*/
INT	set_SA_txNss_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG value;
	value = os_str_tol(arg, 0, 10);

	if ((value > SA_TX_NSS_MAX_NUM) || (value == 0)) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Error: Request txNss(%ld) out of range(1~%d) of hardware limitation\n",
				 value, SA_TX_NSS_MAX_NUM));
		return TRUE;
	} else {
		if (pAd->pSAParam)
			pAd->pSAParam->txNss = (UCHAR)value;
		else
			MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Error: Smart Antenna not properly initialized!\n"));
	}

	return TRUE;
}


/*
	iwpriv ra0 set sa_mode = 0~3
		{
			SA_MODE_NONE = 0,
			SA_MODE_MANUAL = 1,
			SA_MODE_ONESHOT = 2,
			SA_MODE_AUTO = 3,
		}
*/
INT	Set_SA_Mode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value = 0;
	SMART_ANTENNA_STRUCT *pSAParam;
	Value = (ULONG)os_str_tol(arg, 0, 10);

	if ((Value < 1) || (Value > 3))
		Value = SA_MODE_NONE;

	pSAParam = pAd->pSAParam;

	if (pSAParam) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():Disable SA before change the saMode!\n", __func__));
		RtmpSAStop(pAd);
		pSAParam->saMode = Value;
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():saMode=%s!\n", __func__, SA_MODE[pSAParam->saMode]));
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():SmartAntenna feature is not enabled yet\n", __func__));
		return FALSE;
	}
}


/*
	iwpriv ra0 set sa_sta=add-xx:xx:xx:xx:xx:xx
	iwpriv ra0 set sa_sta=del-xx:xx:xx:xx:xx:xx
*/
INT Set_SA_Station_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR macAddr[MAC_ADDR_LEN];
	RTMP_SA_TRAINING_PARAM *pSAStaEntry;
	BOOLEAN bDel = FALSE;

	if (strncmp(arg, "del-", 4) == 0)
		bDel = TRUE;

	if (pAd->pSAParam == NULL) {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():The SmartAntenna funciton is not enabled\n", __func__));
		return FALSE;
	}

	if (rtstrtomac(arg, &macAddr[0], ':') == FALSE)
		return FALSE;

	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():MacAddr=%02x:%02x:%02x:%02x:%02x:%02x\n",
			 __func__, PRINT_MAC(macAddr)));
	pSAStaEntry = sa_add_train_entry(pAd, &macAddr[0], TRUE);
	return TRUE;
}


INT Set_SA_StationCandRule_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	SMART_ANTENNA_STRUCT *pSAParam;
	pSAParam = pAd->pSAParam;

	if (pSAParam) {
		if (strncmp(arg, "rssi", 4) == 0)
			pSAParam->candStaRule = SA_STA_BY_RSSI;

		if (strncmp(arg, "default", 7) == 0)
			pSAParam->candStaRule = SA_STA_BY_DEFAULT;
	} else {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s():The SmartAntenna funciton is not enabled\n", __func__));
		return FALSE;
	}

	return TRUE;
}


INT Set_SA_McsBound_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	SMART_ANTENNA_STRUCT *pSAParam;
	UCHAR txNssUpper;
	Value = (UINT) os_str_tol(arg, 0, 10);
	pSAParam = pAd->pSAParam;

	if (pSAParam) {
		if (pSAParam->txNss)
			txNssUpper = pSAParam->txNss * 8;
		else
			txNssUpper = SA_TX_NSS_MAX_NUM * 8;

		txNssUpper -= 1;
		pSAParam->saMcsBound = (Value > txNssUpper ? txNssUpper : Value);
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s(): saMCSBound=%d\n", __func__, pSAParam->saMcsBound));
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR,
				 ("The SmartAntenna feature is not enabled yet!\n"));
		return FALSE;
	}
}


/*
	iwpirv ra0 set sa_msc=0~255
*/
INT	Set_McsStableCnt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	SMART_ANTENNA_STRUCT *pSAParam;
	Value = (UINT) os_str_tol(arg, 0, 10);
	pSAParam = pAd->pSAParam;

	if (pSAParam) {
		pSAParam->saMsc = (Value > 255 ? 255 : Value);
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("%s(): saMsc=%d\n", __func__, pSAParam->saMsc));
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_ERROR,
				 ("The SmartAntenna feature is not enabled yet!\n"));
		return FALSE;
	}
}


/*
	iwpriv ra0 set sa=0~1
*/
INT	Set_SmartAnt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	if (arg == NULL) {
		show_sa_cmd_usage(pAd);
		return TRUE;
	}

	Value = (UINT)os_str_tol(arg, 0, 10);
	pAd->smartAntEnable = ((Value == 1) ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_HW, CATHW_SA, DBG_LVL_OFF, ("Change the SmartAntenna feature as %s\n",
			 (pAd->smartAntEnable == TRUE ? "TRUE" : "FALSE")));

	if (pAd->smartAntEnable == TRUE) {
		/*  Shall we disable it first? */
		RtmpSAStart(pAd);
	} else {
		if (pAd->pSAParam)
			RtmpSAStop(pAd);
	}

	return TRUE;
}


void pilot_system(
	IN RTMP_ADAPTER *pAd)
{
	/*
		For pilot system, we periodically send the Null frame out with highest
		MCS to measure the success mcs, depends on the successful data rate,
		we estimate the goodness of antenna.

		Procedures:
		0. When system is idle and no traffic for Tx/Rx.
		1. After switch the antenna
		2. Send the piolt frame out in some period and fixed MCS
		3. Check the statistic FIFO to get the successfully MCS rate.
		4. Recode the successfully MCS rate table.
		5. Depends on the rule of SNR/MCS and successfully MCS to estimate the possible
			maximum MCS capability of this antenna

		Questions:
		1. Which kinds of the frames can be treated as the pilot frames.
		2. How many pilot frames should be send out in a trainning period.
		3. What's the relationship between SNR and TxMCS
		4. Others?

		Implementation:
		1. A pilot packet template for each frames.
		2. A timer to trigger the pilot frames to delivery to the hardware queue.
		3. A table to record the tx fifo info of pilot frames.
		4. A table to record the SNR/MCS relationship
		5. After training done, checnk pilot-MCS-table and SNR-MCS table to get a conclusion.

	*/
}



#endif /* SMART_ANTENNA // */


