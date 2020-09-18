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
	mt7622.c
*/

#include "rt_config.h"
#include "chip/mt7622_cr.h"
#include "mcu/mt7622_firmware_e2.h"
#ifdef NEED_ROM_PATCH
#include "mcu/mt7622_rom_patch_e2.h"
#endif /* NEED_ROM_PATCH */

/* ePAeLNA shall always be included as default */
#include "eeprom/mt7622_e2p_ePAeLNA.h"

#if defined(CONFIG_FIRST_IF_IPAILNA)
#include "eeprom/mt7622_e2p_iPAiLNA.h"
#endif
#if defined(CONFIG_FIRST_IF_IPAELNA)
#include "eeprom/mt7622_e2p_iPAeLNA.h"
#endif

#ifdef CONFIG_AP_SUPPORT
#define DEFAULT_BIN_FILE "/etc_ro/wlan/MT7622_EEPROM.bin"
#else
#define DEFAULT_BIN_FILE "/etc/MT7622_EEPROM.bin"
#endif /* CONFIG_AP_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
extern RBIST_DESC_T MT7622_ICAP_DESC[];
extern UINT8 MT7622_ICapBankNum;
#endif /* INTERNAL_CAPTURE_SUPPORT */

#ifdef WIFI_SPECTRUM_SUPPORT
extern RBIST_DESC_T MT7622_SPECTRUM_DESC[];
extern UINT8 MT7622_SpectrumBankNum;
#endif /* WIFI_SPECTRUM_SUPPORT */

UCHAR mt7622_ba_range[] = {4, 8, 12, 24, 36, 48, 54, 64};

static UINT32 UNMASK[] = {0x30, 0x31, 0x32, 0x33, 0x38, 0x39, 0x3d, 0x3e, 0x48, 0x49,
			0x53, 0x54, 0x55, 0x56, 0x57, 0x5c, 0x5d, 0x62, 0x63, 0x68, 0x69, 0xf4, 0xf7,
			0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x10a, 0x10b, 0x10c, 0x10d, 0x10e, 0x10f,
			0x110, 0x111, 0x112, 0x113, 0x114, 0x115, 0x116, 0x117, 0x118, 0x119, 0x11a, 0x11b,
			0x140, 0x141, 0x142, 0x143, 0x144, 0x145, 0x146, 0x147,
			0x155, 0x156, 0x157, 0x158, 0x159, 0x15a, 0x15b, 0x15c, 0x15d,
			0x1d0, 0x1d1, 0x1d2, 0x1d3, 0x1d4, 0x1d5, 0x1d6, 0x1d7, 0x1d8, 0x1d9, 0x1da, 0x1db, 0x1dc, 0x1dd, 0x1de, 0x1df,
			0x1e0, 0x1e1, 0x1e2, 0x1e3, 0x1e4, 0x1e5, 0x1e6, 0x1e7, 0x1e8, 0x1e9, 0x1ea, 0x1eb, 0x1ec, 0x1ed, 0x1ee, 0x1ef,
			0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5, 0x1f6, 0x1f7,
			0x358, 0x359, 0x35a, 0x35b, 0x35c, 0x35d, 0x35e, 0x35f,
			0x360, 0x361, 0x362, 0x363, 0x364, 0x365, 0x366, 0x367, 0x368, 0x369, 0x36a, 0x36b, 0x36c, 0x36d, 0x36e, 0x36f,
			0x370, 0x371, 0x372, 0x373, 0x374, 0x375, 0x376, 0x377, 0x378, 0x379, 0x37a, 0x37b, 0x37c, 0x37d, 0x37e, 0x37f,
			0x380, 0x381, 0x382, 0x383, 0x384, 0x385, 0x386, 0x387, 0x388, 0x389, 0x38a, 0x38b, 0x38c, 0x38d, 0x38e, 0x38f,
			0x390, 0x391, 0x392, 0x393, 0x394, 0x395, 0x396, 0x397, 0x398, 0x399, 0x39a, 0x39b, 0x39c, 0x39d, 0x39e, 0x39f,
			0x3a0, 0x3a1, 0x3a2, 0x3a3, 0x3a4, 0x3a5, 0x3a6, 0x3a7, 0x3a8, 0x3a9, 0x3aa, 0x3ab, 0x3ac, 0x3ad, 0x3ae, 0x3af,
			0x3b0, 0x3b1, 0x3b2, 0x3b3, 0x3b4, 0x3b5, 0x3b6, 0x3b7, 0x3b8, 0x3b9, 0x3ba, 0x3bb, 0x3bc, 0x3bd, 0x3be, 0x3bf,
			0x3c0, 0x3c1, 0x3c2, 0x3c3, 0x3c4, 0x3c5, 0x3c6, 0x3c7, 0x3c8, 0x3c9, 0x3ca, 0x3cb, 0x3cc, 0x3cd, 0x3ce, 0x3cf,
			0x3d0, 0x3d1, 0x3d2, 0x3d3, 0x3d4, 0x3d5, 0x3d6, 0x3d7, 0x3d8, 0x3d9, 0x3da, 0x3db
			}; /* keep efuse field only */

#ifdef MT7622_FPGA
REG_CHK_PAIR hif_dft_cr[] = {
	{HIF_BASE + 0x00, 0xffffffff, 0x76030001},
	{HIF_BASE + 0x04, 0xffffffff, 0x1b},
	{HIF_BASE + 0x10, 0xffffffff, 0x3f01},
	{HIF_BASE + 0x20, 0xffffffff, 0xe01001e0},
	{HIF_BASE + 0x24, 0xffffffff, 0x1e00000f},

	{HIF_BASE + 0x200, 0xffffffff, 0x0},
	{HIF_BASE + 0x204, 0xffffffff, 0x0},
	{HIF_BASE + 0x208, 0xffffffff, 0x10001870},
	{HIF_BASE + 0x20c, 0xffffffff, 0x0},
	{HIF_BASE + 0x210, 0xffffffff, 0x0},
	{HIF_BASE + 0x214, 0xffffffff, 0x0},
	{HIF_BASE + 0x218, 0xffffffff, 0x0},
	{HIF_BASE + 0x21c, 0xffffffff, 0x0},
	{HIF_BASE + 0x220, 0xffffffff, 0x0},
	{HIF_BASE + 0x224, 0xffffffff, 0x0},
	{HIF_BASE + 0x234, 0xffffffff, 0x0},
	{HIF_BASE + 0x244, 0xffffffff, 0x0},
	{HIF_BASE + 0x300, 0xffffffff, 0x0},
	{HIF_BASE + 0x304, 0xffffffff, 0x0},
	{HIF_BASE + 0x308, 0xffffffff, 0x0},
	{HIF_BASE + 0x30c, 0xffffffff, 0x0},
	{HIF_BASE + 0x310, 0xffffffff, 0x0},
	{HIF_BASE + 0x314, 0xffffffff, 0x0},
	{HIF_BASE + 0x318, 0xffffffff, 0x0},
	{HIF_BASE + 0x31c, 0xffffffff, 0x0},
	{HIF_BASE + 0x320, 0xffffffff, 0x0},
	{HIF_BASE + 0x324, 0xffffffff, 0x0},
	{HIF_BASE + 0x328, 0xffffffff, 0x0},
	{HIF_BASE + 0x32c, 0xffffffff, 0x0},
	{HIF_BASE + 0x330, 0xffffffff, 0x0},
	{HIF_BASE + 0x334, 0xffffffff, 0x0},
	{HIF_BASE + 0x338, 0xffffffff, 0x0},
	{HIF_BASE + 0x33c, 0xffffffff, 0x0},

	{HIF_BASE + 0x400, 0xffffffff, 0x0},
	{HIF_BASE + 0x404, 0xffffffff, 0x0},
	{HIF_BASE + 0x408, 0xffffffff, 0x0},
	{HIF_BASE + 0x40c, 0xffffffff, 0x0},
	{HIF_BASE + 0x410, 0xffffffff, 0x0},
	{HIF_BASE + 0x414, 0xffffffff, 0x0},
	{HIF_BASE + 0x418, 0xffffffff, 0x0},
	{HIF_BASE + 0x41c, 0xffffffff, 0x0},
};


INT mt7622_chk_hif_default_cr_setting(RTMP_ADAPTER *pAd)
{
	UINT32 val;
	INT i;
	BOOLEAN match = TRUE;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Default CR Setting Checking for HIF!\n", __func__));

	for (i = 0; i < sizeof(hif_dft_cr) / sizeof(REG_CHK_PAIR); i++) {
		RTMP_IO_READ32(pAd, hif_dft_cr[i].Register, &val);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t Reg(%x): Current=0x%x(0x%x), Default=0x%x, Mask=0x%x, Match=%s\n",
				  hif_dft_cr[i].Register, val, (val & hif_dft_cr[i].Mask),
				  hif_dft_cr[i].Value, hif_dft_cr[i].Mask,
				  ((val & hif_dft_cr[i].Mask) != hif_dft_cr[i].Value) ? "No" : "Yes"));

		if ((val & hif_dft_cr[i].Mask) != hif_dft_cr[i].Value)
			match = FALSE;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Checking Done, Result=> %s match!\n",
			 __func__, match == TRUE ? "All" : "No"));
	return match;
}


REG_CHK_PAIR top_dft_cr[] = {
	{TOP_CFG_BASE + 0x1000, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1004, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1008, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1010, 0xffffffff, 0x0},

	{TOP_CFG_BASE + 0x1100, 0xffffffff, 0x26110310},
	{TOP_CFG_BASE + 0x1108, 0x0000ff00, 0x1400},
	{TOP_CFG_BASE + 0x110c, 0x00000000, 0x0},
	{TOP_CFG_BASE + 0x1110, 0x0f0f00ff, 0x02090040},
	{TOP_CFG_BASE + 0x1124, 0xf000f00f, 0x00000008},
	{TOP_CFG_BASE + 0x1130, 0x000f0000, 0x0},
	{TOP_CFG_BASE + 0x1134, 0x00000000, 0x0},
	{TOP_CFG_BASE + 0x1140, 0x00ff00ff, 0x0},

	{TOP_CFG_BASE + 0x1200, 0x00000000, 0x0},
	{TOP_CFG_BASE + 0x1204, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1208, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x120c, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1210, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1214, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1218, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x121c, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1220, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1224, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1228, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x122c, 0x000fffff, 0x0},
	{TOP_CFG_BASE + 0x1234, 0x00ffffff, 0x0},
	{TOP_CFG_BASE + 0x1238, 0x00ffffff, 0x0},
	{TOP_CFG_BASE + 0x123c, 0xffffffff, 0x5c1fee80},
	{TOP_CFG_BASE + 0x1240, 0xffffffff, 0x6874ae05},
	{TOP_CFG_BASE + 0x1244, 0xffffffff, 0x00fb89f1},

	{TOP_CFG_BASE + 0x1300, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1304, 0xffffffff, 0x8f020006},
	{TOP_CFG_BASE + 0x1308, 0xffffffff, 0x18010000},
	{TOP_CFG_BASE + 0x130c, 0xffffffff, 0x0130484f},
	{TOP_CFG_BASE + 0x1310, 0xffffffff, 0xff000004},
	{TOP_CFG_BASE + 0x1314, 0xffffffff, 0xf0000084},
	{TOP_CFG_BASE + 0x1318, 0x00000000, 0x0},
	{TOP_CFG_BASE + 0x131c, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1320, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1324, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1328, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x132c, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1330, 0xffffffff, 0x00007800},
	{TOP_CFG_BASE + 0x1334, 0x00000000, 0x0},
	{TOP_CFG_BASE + 0x1338, 0xffffffff, 0x0000000a},
	{TOP_CFG_BASE + 0x1400, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1404, 0xffffffff, 0x00005180},
	{TOP_CFG_BASE + 0x1408, 0xffffffff, 0x00001f00},
	{TOP_CFG_BASE + 0x140c, 0xffffffff, 0x00000020},
	{TOP_CFG_BASE + 0x1410, 0xffffffff, 0x0000003a},
	{TOP_CFG_BASE + 0x141c, 0xffffffff, 0x0},

	{TOP_CFG_BASE + 0x1500, 0xffffffff, 0x0},
	{TOP_CFG_BASE + 0x1504, 0xffffffff, 0x0},
};

INT mt7622_chk_top_default_cr_setting(RTMP_ADAPTER *pAd)
{
	UINT32 val;
	INT i;
	BOOLEAN match = TRUE;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Default CR Setting Checking for TOP!\n", __func__));

	for (i = 0; i < sizeof(top_dft_cr) / sizeof(REG_CHK_PAIR); i++) {
		RTMP_IO_READ32(pAd, top_dft_cr[i].Register, &val);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\t Reg(%x): Current=0x%x(0x%x), Default=0x%x, Mask=0x%x, Match=%s\n",
				  top_dft_cr[i].Register, val, (val & top_dft_cr[i].Mask),
				  top_dft_cr[i].Value, top_dft_cr[i].Mask,
				  ((val & top_dft_cr[i].Mask) != top_dft_cr[i].Value) ? "No" : "Yes"));

		if ((val & top_dft_cr[i].Mask) != top_dft_cr[i].Value)
			match = FALSE;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Checking Done, Result=> %s match!\n",
			 __func__, match == TRUE ? "All" : "No"));
	return match;
}
#endif /* MT7622_FPGA */


static VOID mt7622_bbp_adjust(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	/*do nothing, change to use radio_resource control*/
	/*here should do bbp setting only, bbp is full-offload to fw*/
}


/* TODO: Star */
static void mt7622_switch_channel(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	MtCmdChannelSwitch(pAd, SwChCfg);
	MtCmdSetTxRxPath(pAd, SwChCfg);
	pAd->LatchRfRegs.Channel = SwChCfg.CentralChannel;
#ifdef SINGLE_SKU_V2
#ifdef TXBF_SUPPORT
#ifdef MT_MAC
#if defined(MT7615) || defined(MT7622)
	TxPowerBfBackoffParaCtrl(pAd, SwChCfg.Channel_Band, SwChCfg.ControlChannel, SwChCfg.BandIdx);
#endif /* defined(MT7615) || defined(MT7622) */
#endif /*MT_MAC*/
#endif /*TXBF_SUPPORT*/
#endif /*SINGLE_SKU_V2*/
}

#ifdef NEW_SET_RX_STREAM
static INT mt7622_set_RxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
	UINT32 path = 0;
	UINT i;

	if (StreamNums > 4) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():illegal StreamNums(%d)\n",
				  __func__, StreamNums));
		StreamNums = 4;
	}

	for (i = 0; i < StreamNums; i++)
		path |= 1 << i;

	return MtCmdSetRxPath(pAd, path, BandIdx);
}
#endif

static inline VOID bufferModeFieldSet(RTMP_ADAPTER *pAd, EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd, UINT16 addr)
{
	UINT32 i = pCmd->ucCount;
	pCmd->BinContent[i] = pAd->EEPROMImage[addr];
	pCmd->ucCount++;
}


static VOID mt7622_bufferModeEfuseFill(RTMP_ADAPTER *pAd, EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd)
{
	UINT16 i = 0;
	pCmd->ucCount = 0;

	for (i = 0x34; i <= 0x3DB; i++)
		bufferModeFieldSet(pAd, pCmd, i);

	/*must minus last add*/
	pCmd->ucCount--;
}


static VOID mt7622_keep_efuse_field_only(RTMP_ADAPTER *pAd, UCHAR *buffer)
{
	UINT16	i = 0;
	UINT16 offset = 0;

	for (i = 0; i < (sizeof(UNMASK)/sizeof(UINT32)); i++) {
		offset = UNMASK[i];
		buffer[offset] = pAd->EEPROMImage[offset];
	}
}


#ifdef CAL_FREE_IC_SUPPORT
static UINT32 ICAL[] = {0x53, 0x54, 0x55, 0xf4, 0xf7, 0x144, 0x156, 0x15b};
static UINT32 ICAL_NUM = (sizeof(ICAL) / sizeof(UINT32));
static inline BOOLEAN check_valid(RTMP_ADAPTER *pAd, UINT16 Offset)
{
	UINT16 Value = 0;
	BOOLEAN NotValid;

	if ((Offset % 2) != 0) {
		NotValid = rtmp_ee_efuse_read16(pAd, Offset - 1, &Value);

		if (NotValid == FALSE)
			return TRUE;
	} else {
		NotValid = rtmp_ee_efuse_read16(pAd, Offset, &Value);

		if (NotValid == FALSE)
			return TRUE;
	}

	return FALSE;
}
static BOOLEAN mt7622_is_cal_free_ic(RTMP_ADAPTER *pAd)
{
	UINT32 i;

	for (i = 0; i < ICAL_NUM; i++)
		if (check_valid(pAd, ICAL[i]) == TRUE)
			return TRUE;

	return FALSE;
}

static inline VOID cal_free_data_get_from_addr(RTMP_ADAPTER *ad, UINT16 Offset)
{
	UINT16 value;
	BOOLEAN NotValid;

	if ((Offset % 2) != 0) {
		NotValid = rtmp_ee_efuse_read16(ad, Offset - 1, &value);

		if (NotValid == FALSE && ((value & 0xff00) != 0))
			ad->EEPROMImage[Offset] = (value >> 8) & 0xFF;
	} else {
		NotValid = rtmp_ee_efuse_read16(ad, Offset, &value);

		if (NotValid == FALSE && ((value & 0xff) != 0))
			ad->EEPROMImage[Offset] =  value & 0xFF;
	}
}

static VOID mt7622_cal_free_data_get(RTMP_ADAPTER *ad)

{
	UINT32 i;

	for (i = 0; i < ICAL_NUM; i++)
		cal_free_data_get_from_addr(ad, ICAL[i]);
}
#endif /* CAL_FREE_IC_SUPPORT */

static INT32 mt7622_dma_shdl_init(RTMP_ADAPTER *pAd)
{
	UINT32 value;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	HIF_DMASHDL_IO_READ32(pAd, MT_HIF_DMASHDL_PKT_MAX_SIZE, &value);
	value &= ~(PLE_PKT_MAX_SIZE_MASK | PSE_PKT_MAX_SIZE_MASK);
	value |= PLE_PKT_MAX_SIZE_NUM(0x1);
	value |= PSE_PKT_MAX_SIZE_NUM(0x8);
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_PKT_MAX_SIZE, value);

	/* only enable group 0, 1, 2, 4, 5 */
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_REFILL_CTRL, 0xffc80000);
	value = DMASHDL_MIN_QUOTA_NUM(0x10);
	value |= DMASHDL_MAX_QUOTA_NUM(0x800);

	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_GROUP0_CTRL, value);
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_GROUP1_CTRL, value);
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_GROUP2_CTRL, value);
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_GROUP4_CTRL, value);
	HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_GROUP5_CTRL, value);

	if ((cap->qm == GENERIC_QM) || (cap->qm == GENERIC_FAIR_QM)) {
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP0, 0x42104210);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP1, 0x42104210);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP2, 0x00000005);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_Q_MAP3, 0x0);

		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SHDL_SET0, 0x6012345f);
		HIF_DMASHDL_IO_WRITE32(pAd, MT_HIF_DMASHDL_SHDL_SET1, 0xedcba987);
	}

	return TRUE;
}



#ifdef CFG_SUPPORT_MU_MIMO
#ifdef MANUAL_MU
INT mu_update_profile_tb(RTMP_ADAPTER *pAd, INT profile_id, UCHAR wlan_id)
{
}

INT mu_update_grp_table(RTMP_ADAPTER *pAd, INT grp_id)
{
	return TRUE;
}


INT mu_update_cluster_tb(RTMP_ADAPTER *pAd, UCHAR c_id, UINT32 *m_ship, UINT32 *u_pos)
{
	UINT32 entry_base, mac_val, offset;
	ASSERT(c_id <= 31);
	MAC_IO_READ32(pAd, MU_MUCR1, &mac_val);

	if (c_id < 16)
		mac_val &= (~MUCR1_CLUSTER_TAB_REMAP_CTRL_MASK);
	else
		mac_val |= MUCR1_CLUSTER_TAB_REMAP_CTRL_MASK;

	MAC_IO_WRITE32(pAd, MU_MUCR1, mac_val);
	entry_base = MU_CLUSTER_TABLE_BASE  + (c_id & (~0x10)) * 24;
	/* update membership */
	MAC_IO_WRITE32(pAd, entry_base + 0x0, m_ship[0]);
	MAC_IO_WRITE32(pAd, entry_base + 0x4, m_ship[1]);
	/* Update user position */
	MAC_IO_WRITE32(pAd, entry_base + 0x8, u_pos[0]);
	MAC_IO_WRITE32(pAd, entry_base + 0xc, u_pos[1]);
	MAC_IO_WRITE32(pAd, entry_base + 0x10, u_pos[2]);
	MAC_IO_WRITE32(pAd, entry_base + 0x14, u_pos[3]);
	return TRUE;
}


INT mu_get_wlanId_ac_len(RTMP_ADAPTER *pAd, UINT32 wlan_id, UINT ac)
{
	return TRUE;
}


INT mu_get_mu_tx_retry_cnt(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT mu_get_pfid_tx_stat(RTMP_ADAPTER *pAd)
{
}

INT mu_get_gpid_rate_per_stat(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT mt7622_mu_init(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;
	/****************************************************************************
		MU Part
	****************************************************************************/
	/* After power on initial setting,  AC legnth clear */
	MAC_IO_READ32(pAd, MU_MUCR4, &mac_val);
	mac_val = 0x1;
	MAC_IO_WRITE32(pAd, MU_MUCR4, mac_val); /* 820fe010= 0x0000_0001 */
	/* PFID table */
	MAC_IO_WRITE32(pAd, MU_PROFILE_TABLE_BASE + 0x0, 0x1e000);  /* 820fe780= 0x0001_e000 */
	MAC_IO_WRITE32(pAd, MU_PROFILE_TABLE_BASE + 0x4, 0x1e103);  /* 820fe784= 0x0001_e103 */
	MAC_IO_WRITE32(pAd, MU_PROFILE_TABLE_BASE + 0x8, 0x1e205);  /* 820fe788= 0x0001_e205 */
	MAC_IO_WRITE32(pAd, MU_PROFILE_TABLE_BASE + 0xc, 0x1e306);  /* 820fe78c= 0x0001_e306 */
	/* Cluster table */
	MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x0, 0x0);  /* 820fe400= 0x0000_0000 */
	MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x8, 0x0);  /* 820fe408= 0x0000_0000 */
	MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x20, 0x2);  /* 820fe420= 0x0000_0002 */
	MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x28, 0x0);  /* 820fe428= 0x0000_0000 */
	MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x40, 0x2);  /* 820fe440= 0x0000_0002 */
	MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x48, 0x4);  /* 820fe448= 0x0000_0004 */
	MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x60, 0x0);  /* 820fe460= 0x0000_0000 */
	MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x68, 0x0);  /* 820fe468= 0x0000_0000 */
	/* Group rate table */
	MAC_IO_WRITE32(pAd, MU_GRP_TABLE_RATE_MAP + 0x0, 0x4109);  /* 820ff000= 0x0000_4109 */
	MAC_IO_WRITE32(pAd, MU_GRP_TABLE_RATE_MAP + 0x4, 0x99);  /* 820ff004= 0x0000_0099 */
	MAC_IO_WRITE32(pAd, MU_GRP_TABLE_RATE_MAP + 0x8, 0x800000f0);  /* 820ff008= 0x8000_00f0 */
	MAC_IO_WRITE32(pAd, MU_GRP_TABLE_RATE_MAP + 0xc, 0x99);  /* 820ff00c= 0x0000_0099 */
	/* SU Tx minimum setting */
	MAC_IO_WRITE32(pAd, MU_MUCR2, 0x10000001);  /* 820fe008= 0x1000_0001 */
	/* MU max group search entry = 1 group entry */
	MAC_IO_WRITE32(pAd, MU_MUCR1, 0x0);  /* 820fe004= 0x0000_0000 */
	/* MU enable */
	MAC_IO_READ32(pAd, MU_MUCR0, &mac_val);
	mac_val |= 1;
	MAC_IO_WRITE32(pAd, MU_MUCR0, 0x1);  /* 820fe000= 0x1000_0001 */
	/****************************************************************************
		M2M Part
	****************************************************************************/
	/* Enable M2M MU temp mode */
	MAC_IO_READ32(pAd, RMAC_M2M_BAND_CTRL, &mac_val);
	mac_val |= (1 << 16);
	MAC_IO_WRITE32(pAd, RMAC_M2M_BAND_CTRL, mac_val);
	/****************************************************************************
		AGG Part
	****************************************************************************/
	/* 820f20e0[15] = 1 or 0 all need to be verified, because
	    a). if primary is the fake peer, and peer will not ACK to us, cannot setup the TxOP
	    b). Or can use CTS2Self to setup the TxOP
	*/
	MAC_IO_READ32(pAd, AGG_MUCR, &mac_val);
	mac_val &= (~MUCR_PRIM_BAR_MASK);
	/* mac_val |= (1 << MUCR_PRIM_BAR_BIT); */
	MAC_IO_WRITE32(pAd, AGG_MUCR, mac_val);  /* 820fe000= 0x1000_0001 */
	return TRUE;
}
#endif /* MANUAL_MU */
#endif /* CFG_SUPPORT_MU_MIMO */

#ifndef MAC_INIT_OFFLOAD
#endif /* MAC_INIT_OFFLOAD */

/* need to confirm with DE, wilsonl */
static VOID mt7622_init_mac_cr(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

#ifndef MAC_INIT_OFFLOAD
	/* need to confirm with DE, wilsonl */
	/* done, confirmed by Austin */
	/* Set TxFreeEvent packet only go through CR4 */
	HW_IO_READ32(pAd, PLE_HIF_REPORT, &mac_val);
	mac_val |= 0x1;
	HW_IO_WRITE32(pAd, PLE_HIF_REPORT, mac_val);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s(): Set TxRxEventPkt path 0x%0x = 0x%08x\n",
			  __func__, PLE_HIF_REPORT, mac_val));

	HW_IO_READ32(pAd, PP_PAGECTL_2, &mac_val);
	mac_val &= ~(PAGECTL_2_CUT_PG_CNT_MASK);
	mac_val |= 0x30;
	HW_IO_WRITE32(pAd, PP_PAGECTL_2, mac_val);

#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
#else
	/* TxS Setting */
	InitTxSTypeTable(pAd);
#endif
	MtAsicSetTxSClassifyFilter(pAd, TXS2HOST, TXS2H_QID1, TXS2HOST_AGGNUMS, 0x00, 0);
#endif /*MAC_INIT_OFFLOAD*/
	/* MAC D0 2x / MAC D0 1x clock enable */
	MAC_IO_READ32(pAd, CFG_CCR, &mac_val);
	mac_val |= (BIT31 | BIT25);
	MAC_IO_WRITE32(pAd, CFG_CCR, mac_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: MAC D0 2x 1x initial(val=%x)\n", __func__, mac_val));
	/*  Disable RX Header Translation */
	MAC_IO_READ32(pAd, DMA_DCR0, &mac_val);
	mac_val &= ~(DMA_DCR0_RX_HDR_TRANS_EN_BIT | DMA_DCR0_RX_HDR_TRANS_MODE_BIT |
				 DMA_DCR0_RX_RM_VLAN_BIT | DMA_DCR0_RX_INS_VLAN_BIT |
				 DMA_DCR0_RX_HDR_TRANS_CHK_BSSID);
#ifdef HDR_TRANS_RX_SUPPORT

	if (IS_ASIC_CAP(pAd, fASIC_CAP_RX_HDR_TRANS)) {
		UINT32 mac_val2;
		mac_val |= DMA_DCR0_RX_HDR_TRANS_EN_BIT | DMA_DCR0_RX_RM_VLAN_BIT |
				   DMA_DCR0_RX_HDR_TRANS_CHK_BSSID;
		/* TODO: UnifiedSW, take care about Windows for translation mode! */
		/* mac_val |= DMA_DCR0_RX_HDR_TRANS_MODE_BIT; */
		MAC_IO_READ32(pAd, DMA_DCR1, &mac_val2);
		mac_val2 |= RHTR_AMS_VLAN_EN;
		MAC_IO_WRITE32(pAd, DMA_DCR1, mac_val2);
	}

#endif /* HDR_TRANS_RX_SUPPORT */
	MAC_IO_WRITE32(pAd, DMA_DCR0, mac_val);
	/* CCA Setting */
	MAC_IO_READ32(pAd, TMAC_TRCR0, &mac_val);
	mac_val &= ~CCA_SRC_SEL_MASK;
	mac_val |= CCA_SRC_SEL(0x2);
	mac_val &= ~CCA_SEC_SRC_SEL_MASK;
	mac_val |= CCA_SEC_SRC_SEL(0x0);
	MAC_IO_WRITE32(pAd, TMAC_TRCR0, mac_val);
	MAC_IO_READ32(pAd, TMAC_TRCR0, &mac_val);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s(): TMAC_TRCR0=0x%x\n", __func__, mac_val));
	/* ---Add by shiang for MT7615 RFB ED issue */
	/* Set BAR rate as 0FDM 6M default, remove after fw set */
	MAC_IO_WRITE32(pAd, AGG_ACR0, 0x04b10496);
	/*Add by Star for zero delimiter*/
	MAC_IO_READ32(pAd, TMAC_CTCR0, &mac_val);
	mac_val &= ~INS_DDLMT_REFTIME_MASK;
	mac_val |= INS_DDLMT_REFTIME(0x3f);
	mac_val |= DUMMY_DELIMIT_INSERTION;
	mac_val |= INS_DDLMT_DENSITY(3);
	MAC_IO_WRITE32(pAd, TMAC_CTCR0, mac_val);

	/* Temporary setting for RTS */
	/*if no protect should enable for CTS-2-Self, WHQA_00025629*/
	MAC_IO_WRITE32(pAd, AGG_PCR1, 0x0400092b);/* sync MT7615 MP2.1 */
	MAC_IO_READ32(pAd, AGG_SCR, &mac_val);
	mac_val |= NLNAV_MID_PTEC_DIS;
	MAC_IO_WRITE32(pAd, AGG_SCR, mac_val);

	/*Default disable rf low power beacon mode*/
#define WIFI_SYS_PHY 0x10000
#define RF_LOW_BEACON_BAND0 (WIFI_SYS_PHY+0x1900)
#define RF_LOW_BEACON_BAND1 (WIFI_SYS_PHY+0x1d00)
	PHY_IO_READ32(pAd, RF_LOW_BEACON_BAND0, &mac_val);
	mac_val &= ~(0x3 << 8);
	mac_val |= (0x2 << 8);
	PHY_IO_WRITE32(pAd, RF_LOW_BEACON_BAND0, mac_val);
	PHY_IO_READ32(pAd, RF_LOW_BEACON_BAND1, &mac_val);
	mac_val &= ~(0x3 << 8);
	mac_val |= (0x2 << 8);
	PHY_IO_WRITE32(pAd, RF_LOW_BEACON_BAND1, mac_val);
}





static VOID MT7622BBPInit(RTMP_ADAPTER *pAd)
{
	/* Disable PHY shaping filter for Japan Region */
	if (pAd->CommonCfg.RDDurRegion == JAP)
		MtCmdPhyShapingFilterDisable(pAd);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo\n", __func__));
}


static void mt7622_init_rf_cr(RTMP_ADAPTER *ad)
{
}

/* Read power per rate */
void mt7622_get_tx_pwr_per_rate(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo\n", __func__));
}


void mt7622_get_tx_pwr_info(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo\n", __func__));
}


static UCHAR get_subid(void)
{
	UCHAR subid = 0;
	ULONG sysEfuseAddr = 0;
	UINT32 sysEfuseVal = 0;

	sysEfuseAddr = (ULONG)ioremap(0x1020682c, 4);
	sysEfuseVal = (UINT32)ioread32((void *)sysEfuseAddr);
	iounmap((void *)sysEfuseAddr);

	if (((sysEfuseVal & 0xf000) >> 12) != 0)
		subid = (sysEfuseVal & 0xf000) >> 12;
	else if (((sysEfuseVal & 0xf00) >> 8) != 0)
		subid = (sysEfuseVal & 0xf00) >> 8;
	else if (((sysEfuseVal & 0xf0) >> 4) != 0)
		subid = (sysEfuseVal & 0xf0) >> 4;
	else
		subid = 2;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("read sub id = %x\n", subid));
	return subid;
}


static void mt7622_antenna_default_reset(
	struct _RTMP_ADAPTER *pAd,
	EEPROM_ANTENNA_STRUC *pAntenna)
{
	pAntenna->word = 0;
	pAd->RfIcType = RFIC_7622;
#ifndef TXBF_SUPPORT
	pAntenna->field.TxPath = 4;
	pAntenna->field.RxPath = 4;
#else
	pAntenna->field.TxPath = (pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] >> 4) & 0x0F;
	pAntenna->field.RxPath = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] & 0x0F;
#endif /* TXBF_SUPPORT */

	if (get_subid() == 0x4) {
		pAntenna->field.TxPath = 2;
		pAntenna->field.RxPath = 2;
	}
}


static VOID mt7622_fw_prepare(RTMP_ADAPTER *pAd)
{
	struct fwdl_ctrl *ctrl = &pAd->MCUCtrl.fwdl_ctrl;

#ifdef NEED_ROM_PATCH
	/* Use E2 rom patch as default */
	ctrl->patch_profile[WM_CPU].source.header_ptr = mt7622_rom_patch_e2;
	ctrl->patch_profile[WM_CPU].source.header_len = sizeof(mt7622_rom_patch_e2);
	ctrl->patch_profile[WM_CPU].source.bin_name = MT7622_ROM_PATCH_BIN_FILE_NAME_E2;
#endif /* NEED_ROM_PATCH */

	ctrl->fw_profile[WM_CPU].source.header_ptr = MT7622_FirmwareImage_E2;
	ctrl->fw_profile[WM_CPU].source.header_len = sizeof(MT7622_FirmwareImage_E2);
	ctrl->fw_profile[WM_CPU].source.bin_name = MT7622_RAM_BIN_FILE_NAME_E2;
}

static VOID mt7622_fwdl_datapath_setup(RTMP_ADAPTER *pAd, BOOLEAN init)
{
	WPDMA_GLO_CFG_STRUC GloCfg;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (init == TRUE) {
		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);
		GloCfg.MT7622_field.fw_ring_bp_tx_sch = 1;
		HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, GloCfg.word);

		ops->pci_kick_out_cmd_msg = AndesMTPciKickOutCmdMsgFwDlRing;
	} else {
		ops->pci_kick_out_cmd_msg = AndesMTPciKickOutCmdMsg;

		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);
		GloCfg.MT7622_field.fw_ring_bp_tx_sch = 0;
		HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, GloCfg.word);
	}
}




#ifdef TXBF_SUPPORT
void mt7622_setETxBFCap(
	IN  RTMP_ADAPTER *pAd,
	IN  TXBF_STATUS_INFO * pTxBfInfo)
{
	HT_BF_CAP *pTxBFCap = pTxBfInfo->pHtTxBFCap;

	if (pTxBfInfo->cmmCfgETxBfEnCond > 0) {
		switch (pTxBfInfo->cmmCfgETxBfEnCond) {
		case SUBF_ALL:
		default:
			pTxBFCap->RxNDPCapable         = TRUE;
			pTxBFCap->TxNDPCapable         = (pTxBfInfo->ucRxPathNum > 1) ? TRUE : FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF           = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF             =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping          = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup     = 3;
			pTxBFCap->TxSoundCapable       = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation       = pTxBfInfo->ucRxPathNum - 1;
			break;

		case SUBF_BFER:
			pTxBFCap->RxNDPCapable         = FALSE;
			pTxBFCap->TxNDPCapable         = (pTxBfInfo->ucRxPathNum > 1) ? TRUE : FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF           = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF             =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping          = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup     = 3;
			pTxBFCap->TxSoundCapable       = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation       = pTxBfInfo->ucRxPathNum - 1;
			break;

		case SUBF_BFEE:
			pTxBFCap->RxNDPCapable         = TRUE;
			pTxBFCap->TxNDPCapable         = FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF           = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF             =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping          = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup     = 3;
			pTxBFCap->TxSoundCapable       = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation       = pTxBfInfo->ucRxPathNum - 1;
			break;
		}
	} else
		memset(pTxBFCap, 0, sizeof(*pTxBFCap));
}

#ifdef VHT_TXBF_SUPPORT
void mt7622_setVHTETxBFCap(
	IN  RTMP_ADAPTER *pAd,
	IN  TXBF_STATUS_INFO * pTxBfInfo)
{
	VHT_CAP_INFO *pTxBFCap = pTxBfInfo->pVhtTxBFCap;

	if (pTxBfInfo->cmmCfgETxBfIncapable) {
		pTxBFCap->num_snd_dimension  = 0;
		pTxBFCap->bfee_cap_mu        = 0;
		pTxBFCap->bfee_cap_su        = 0;
		pTxBFCap->bfer_cap_mu        = 0;
		pTxBFCap->bfer_cap_su        = 0;
		pTxBFCap->bfee_sts_cap       = 0;
	} else {
		pTxBFCap->bfee_cap_su       = 1;
		pTxBFCap->bfer_cap_su       = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
		pTxBFCap->bfee_cap_mu       = 1;
		pTxBFCap->bfer_cap_mu       = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
		pTxBFCap->bfee_sts_cap      = 3;
		pTxBFCap->num_snd_dimension = pTxBfInfo->ucTxPathNum - 1;
	}
}
#endif /* VHT_TXBF_SUPPORT */
#endif /* TXBF_SUPPORT */

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
static void mt7622_host_resume_done_ack(
	struct _RTMP_ADAPTER *pAd)
{
	mt_cmd_host_resume_done_ack(pAd);
}
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */

#ifdef RF_LOCKDOWN
static UINT32 ICAL_JUST_MERGE[] = {0x118, 0x1b5, 0x1b6, 0x1b7, 0x3ac, 0x3ad, 0x3ae, 0x3af, 0x3b0, 0x3b1, 0x3b2}; /* merge but nott check */
static UINT32 ICAL_JUST_MERGE_NUM = (sizeof(ICAL_JUST_MERGE) / sizeof(UINT32));

static UINT32 RFLOCK[] = { 0x03F, 0x040, 0x041, 0x056, 0x057, 0x058, 0x059, 0x05A, 0x05B, 0x05C, 0x05D, 0x05E, 0x05F, 0x060, 0x061, 0x062,
						   0x063, 0x064, 0x065, 0x066, 0x067, 0x068, 0x069, 0x06A, 0x06B, 0x06C, 0x06D, 0x06E, 0x06F, 0x070, 0x071, 0x072,
						   0x073, 0x074, 0x075, 0x076, 0x077, 0x078, 0x079, 0x07A, 0x07B, 0x07C, 0x07D, 0x07E, 0x07F, 0x080, 0x081, 0x082,
						   0x083, 0x084, 0x085, 0x086, 0x087, 0x088, 0x089, 0x08A, 0x08B, 0x08C, 0x08D, 0x08E, 0x08F, 0x090, 0x091, 0x092,
						   0x093, 0x094, 0x095, 0x096, 0x097, 0x098, 0x099, 0x09A, 0x09B, 0x09C, 0x09D, 0x09E, 0x09F, 0x0A0, 0x0A1, 0x0A2,
						   0x0A3, 0x0A4, 0x0A5, 0x0A6, 0x0A7, 0x0A8, 0x0A9, 0x0AA, 0x0AB, 0x0AC, 0x0AD, 0x0AE, 0x0AF, 0x0B0, 0x0B1, 0x0B2,
						   0x0B3, 0x0B4, 0x0B5, 0x0B6, 0x0B7, 0x0B8, 0x0B9, 0x0BA, 0x0BB, 0x0BC, 0x0BD, 0x0BE, 0x0BF, 0x0C0, 0x0C1, 0x0C2,
						   0x0C3, 0x0C4, 0x0C5, 0x0C6, 0x0C7, 0x0C8, 0x0C9, 0x0CA, 0x0CB, 0x0CC, 0x0CD, 0x0CE, 0x0CF, 0x0D0, 0x0D1, 0x0D2,
						   0x0D3, 0x0D4, 0x0D5, 0x0D6, 0x0D7, 0x0D8, 0x0D9, 0x0DA, 0x0DB, 0x0DC, 0x0DD, 0x0DE, 0x0DF, 0x0E0, 0x0E1, 0x0E2,
						   0x0E3, 0x0E4, 0x0E5, 0x0E6, 0x0E7, 0x0E8, 0x0E9, 0x0EA, 0x0EB, 0x0EC, 0x0ED, 0x0EE, 0x0EF, 0x0F0, 0x0F2, 0x0F3,
						   0x118, 0x11C, 0x11D, 0x11E, 0x11F, 0x12C, 0x12D, 0x140, 0x141, 0x142, 0x143, 0x144, 0x145, 0x146, 0x147, 0x148,
						   0x149, 0x14A, 0x14B, 0x14C, 0x14D, 0x14E, 0x14F, 0x150, 0x151, 0x152, 0x153, 0x154, 0x155, 0x156, 0x157, 0x158,
						   0x159, 0x15A, 0x15B, 0x15C, 0x15D, 0x15E, 0x15F, 0x160, 0x161, 0x162, 0x163, 0x164, 0x165, 0x166, 0x167, 0x168,
						   0x169, 0x16A, 0x16B, 0x16C, 0x16D, 0x16E, 0x16F, 0x170, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0x177, 0x178,
						   0x179, 0x17A, 0x17B, 0x17C, 0x17D, 0x17E, 0x17F, 0x180, 0x181, 0x182, 0x183, 0x184, 0x185, 0x186, 0x187, 0x188,
						   0x189, 0x18A, 0x18B, 0x18C, 0x18D, 0x18E, 0x18F, 0x190, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196, 0x197, 0x198,
						   0x199, 0x19A, 0x19B, 0x19C, 0x19D, 0x19E, 0x19F, 0x1A0, 0x1A2, 0x1A3, 0x1A4, 0x1A5, 0x1A6, 0x1A7, 0x1A8, 0x1A9,
						   0x1AA, 0x1AB, 0x1AC, 0x1AD, 0x1AE, 0x1AF, 0x1B0, 0x1B1, 0x1B2, 0x3AC, 0x3AD, 0x3AE, 0x3AF
						 };

static UINT32 RFLOCK_NUM = (sizeof(RFLOCK) / sizeof(UINT32));

static BOOLEAN mt7622_check_RF_lock_down(RTMP_ADAPTER *pAd)
{
	UCHAR block[EFUSE_BLOCK_SIZE] = "";
	USHORT offset = 0;
	UINT isVaild = 0;
	BOOL RFlockDown;
	/* RF lock down column (0x12C) */
	offset = RF_LOCKDOWN_EEPROME_BLOCK_OFFSET;
	MtCmdEfuseAccessRead(pAd, offset, &block[0], &isVaild);

	if (((block[RF_LOCKDOWN_EEPROME_COLUMN_OFFSET] & RF_LOCKDOWN_EEPROME_BIT) >> RF_LOCKDOWN_EEPROME_BIT_OFFSET) == TRUE)
		RFlockDown = TRUE;
	else
		RFlockDown = FALSE;

	return RFlockDown;
}

static BOOLEAN mt7622_write_RF_lock_parameter(RTMP_ADAPTER *pAd, USHORT offset)
{
	BOOLEAN RFParaWrite;
	BOOLEAN fgRFlock = FALSE;
	UINT16  RFlock_index = 0;
#ifdef CAL_FREE_IC_SUPPORT
	BOOLEAN fgCalFree = FALSE;
	UINT16  CalFree_index = 0;
#endif /* CAL_FREE_IC_SUPPORT */

	/* Priority rule 1: RF lock paramter or not?    */
	/* Priority rule 2: Apply cal free or not?      */
	/* Priority rule 3: Cal free parameter or not?  */

	/* Check whether the offset exist in RF Lock Table or not */
	for (RFlock_index = pAd->RFlockTempIdx; RFlock_index < RFLOCK_NUM; RFlock_index++) {
		if (RFLOCK[RFlock_index] == offset) {
			fgRFlock = TRUE;
			pAd->RFlockTempIdx = RFlock_index;
			break;
		}
	}

#ifdef CAL_FREE_IC_SUPPORT

	/* Check whether the offset exist in Cal Free Table or not */
	for (CalFree_index = pAd->CalFreeTempIdx; CalFree_index < ICAL_NUM; CalFree_index++) {
		if (ICAL[CalFree_index] == offset) {
			fgCalFree = TRUE;
			pAd->CalFreeTempIdx = CalFree_index;
			break;
		}
	}

	/* Check whether the offset exist in Cal Free (Merge but not check) Table or not */
	for (CalFree_index = 0; CalFree_index < ICAL_JUST_MERGE_NUM; CalFree_index++) {
		if (ICAL_JUST_MERGE[CalFree_index] == offset) {
			fgCalFree = TRUE;
			break;
		}
	}

#endif /* CAL_FREE_IC_SUPPORT */

	/* Determine whether this offset needs to be written or not when RF lockdown */
	if (fgRFlock) {
#ifdef CAL_FREE_IC_SUPPORT

		if (pAd->fgCalFreeApply) {
			if (fgCalFree)
				RFParaWrite = FALSE;
			else
				RFParaWrite = TRUE;
		} else
			RFParaWrite = TRUE;

#else
		RFParaWrite = TRUE;
#endif
	} else
		RFParaWrite = FALSE;

	return RFParaWrite;
}

static BOOLEAN mt7622_merge_RF_lock_parameter(RTMP_ADAPTER *pAd)
{
	UCHAR   block[EFUSE_BLOCK_SIZE] = "";
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	USHORT  length = cap->EEPROM_DEFAULT_BIN_SIZE;
	UCHAR   *ptr = pAd->EEPROMImage;
	UCHAR   index;
	USHORT  offset = 0;
	UINT    isVaild = 0;
	BOOL    WriteStatus;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* Merge RF parameters in Effuse to E2p buffer */
	if (chip_check_rf_lock_down(pAd)) {
		/* Check Effuse Content block by block */
		for (offset = 0; offset < length; offset += EFUSE_BLOCK_SIZE) {
			MtCmdEfuseAccessRead(pAd, offset, &block[0], &isVaild);

			/* Check the Needed contents are different and update the E2p content by Effuse */
			for (index = 0; index < EFUSE_BLOCK_SIZE; index++) {
				/* Obtain the status of this E2p column need to write or not */
				WriteStatus = ops->write_RF_lock_parameter(pAd, offset + index);
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Effuse[0x%04x]: Write(%d)\n", offset + index, WriteStatus));

				if ((block[index] != ptr[index]) && (WriteStatus))
					ptr[index] = block[index];
				else
					continue;

				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("index 0x%04x: ", offset + index));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("orignal E2p value=0x%04x, write value=0x%04x\n", ptr[index],
						 block[index]));
			}

			ptr += EFUSE_BLOCK_SIZE;
		}
	}

	return TRUE;
}

static UCHAR mt7622_Read_Effuse_parameter(RTMP_ADAPTER *pAd, USHORT offset)
{
	UCHAR   block[EFUSE_BLOCK_SIZE] = "";
	UINT    isVaild = 0;
	UINT16  BlockOffset, IndexOffset;
	UCHAR   RFUnlock = 0xFF;
	/* Obtain corresponding BlockOffset and IndexOffset for Effuse contents access */
	IndexOffset = offset % EFUSE_BLOCK_SIZE;
	BlockOffset = offset - IndexOffset;

	/* Merge RF parameters in Effuse to E2p buffer */
	if (chip_check_rf_lock_down(pAd)) {
		/* Check Effuse Content block by block */
		MtCmdEfuseAccessRead(pAd, BlockOffset, &block[0], &isVaild);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Read Effuse[0x%x]: 0x%x ", offset, block[IndexOffset]));
		return block[IndexOffset];
	}

	return RFUnlock;
}

static BOOLEAN mt7622_Config_Effuse_Country(RTMP_ADAPTER *pAd)
{
	UCHAR   Buffer0, Buffer1;
	UCHAR   CountryCode[2];
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* Read Effuse Content */
	if (ops->Read_Effuse_parameter != NULL) {
		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		/* Country Region 2G */
		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		Buffer0 = ops->Read_Effuse_parameter(pAd, COUNTRY_REGION_2G_EEPROME_OFFSET);

		/* Check the RF lock status */
		if (Buffer0 != 0xFF) {
			/* Check Validation bit for content */
			if (((Buffer0) & (COUNTRY_REGION_VALIDATION_MASK)) >> (COUNTRY_REGION_VALIDATION_OFFSET))
				pAd->CommonCfg.CountryRegion = ((Buffer0) & (COUNTRY_REGION_CONTENT_MASK));
		}

		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		/* Country Region 5G */
		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		Buffer1 = ops->Read_Effuse_parameter(pAd, COUNTRY_REGION_5G_EEPROME_OFFSET);

		/* Check the RF lock status */
		if (Buffer1 != 0xFF) {
			/* Check Validation bit for content */
			if (((Buffer1) & (COUNTRY_REGION_VALIDATION_MASK)) >> (COUNTRY_REGION_VALIDATION_OFFSET))
				pAd->CommonCfg.CountryRegionForABand = ((Buffer1) & (COUNTRY_REGION_CONTENT_MASK));
		}

		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		/* Country Code */
		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		CountryCode[0] = ops->Read_Effuse_parameter(pAd, COUNTRY_CODE_BYTE0_EEPROME_OFFSET);
		CountryCode[1] = ops->Read_Effuse_parameter(pAd, COUNTRY_CODE_BYTE1_EEPROME_OFFSET);

		/* Check the RF lock status */
		if ((CountryCode[0] != 0xFF) && (CountryCode[1] != 0xFF)) {
			/* Check Validation for content */
			if ((CountryCode[0] != 0x00) && (CountryCode[1] != 0x00)) {
				pAd->CommonCfg.CountryCode[0] = CountryCode[0];
				pAd->CommonCfg.CountryCode[1] = CountryCode[1];
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->CommonCfg.CountryCode[0]: 0x%x, %c ",
						 pAd->CommonCfg.CountryCode[0], pAd->CommonCfg.CountryCode[0]));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->CommonCfg.CountryCode[1]: 0x%x, %c ",
						 pAd->CommonCfg.CountryCode[1], pAd->CommonCfg.CountryCode[1]));
			}
		}
	}

	return TRUE;
}
#endif /* RF_LOCKDOWN */

UCHAR *mt7622_get_default_bin_image(VOID)
{
#if defined(CONFIG_FIRST_IF_IPAILNA)
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Use 1st iPAiLNA default bin.\n"));
	return MT7622_E2PImage_iPAiLNA;
#elif defined(CONFIG_FIRST_IF_IPAELNA)
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Use 1st iPAeLNA default bin.\n"));
	return MT7622_E2PImage_iPAeLNA;
#else
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Use 1st ePAeLNA default bin.\n"));
	return MT7622_E2PImage_ePAeLNA;
#endif

	return NULL;
}

UCHAR *mt7622_get_default_bin_image_file(VOID)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Use 1st %s default bin.\n", DEFAULT_BIN_FILE));
	return DEFAULT_BIN_FILE;
}

static INT hif_set_WPDMA(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable, UINT8 WPDMABurstSIZE)
{
	WPDMA_GLO_CFG_STRUC GloCfg;
#ifdef CONFIG_DELAY_INT
	UINT32 Value;
#endif
	HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);
#ifdef CONFIG_DELAY_INT
	HIF_IO_READ32(pAd, MT_DELAY_INT_CFG, &Value);
#endif

	switch (TxRx) {
	case PDMA_TX:
		if (enable == TRUE) {
			GloCfg.MT7622_field.EnableTxDMA = 1;
			GloCfg.MT7622_field.EnTXWriteBackDDONE = 1;
			GloCfg.MT7622_field.WPDMABurstSIZE = WPDMABurstSIZE;
			GloCfg.MT7622_field.multi_dma_en = MULTI_DMA_EN_FEATURE_2_PREFETCH;
#ifdef CONFIG_DELAY_INT
			Value |= TX_DLY_INT_EN;
			Value &= ~TX_MAX_PINT_MASK;
			Value |= TX_MAX_PINT(TX_PENDING_INT_NUMS);
			Value &= ~TX_MAX_PTIME_MASK;
			Value |= TX_MAX_PTIME(TX_PENDING_INT_TIME);
#endif
		} else {
			GloCfg.MT7622_field.EnableTxDMA = 0;
#ifdef CONFIG_DELAY_INT
			Value &= ~TX_DLY_INT_EN;
			Value &= ~TX_MAX_PINT_MASK;
			Value &= ~TX_MAX_PTIME_MASK;
#endif
		}

		break;

	case PDMA_RX:
		if (enable == TRUE) {
			GloCfg.MT7622_field.EnableRxDMA = 1;
			GloCfg.MT7622_field.WPDMABurstSIZE = WPDMABurstSIZE;
			GloCfg.MT7622_field.multi_dma_en = MULTI_DMA_EN_FEATURE_2_PREFETCH;
#ifdef CONFIG_DELAY_INT
			Value |= RX_DLY_INT_EN;
			Value &= ~RX_MAX_PINT_MASK;
			Value |= RX_MAX_PINT(RX_PENDING_INT_NUMS);
			Value &= ~RX_MAX_PTIME_MASK;
			Value |= RX_MAX_PTIME(RX_PENDING_INT_TIME);
#endif
		} else {
			GloCfg.MT7622_field.EnableRxDMA = 0;
#ifdef CONFIG_DELAY_INT
			Value &= ~RX_DLY_INT_EN;
			Value &= ~RX_MAX_PINT_MASK;
			Value &= ~RX_MAX_PTIME_MASK;
#endif
		}

		break;

	case PDMA_TX_RX:
		if (enable == TRUE) {
			GloCfg.MT7622_field.EnableTxDMA = 1;
			GloCfg.MT7622_field.EnableRxDMA = 1;
			GloCfg.MT7622_field.EnTXWriteBackDDONE = 1;
			GloCfg.MT7622_field.WPDMABurstSIZE = WPDMABurstSIZE;
			GloCfg.MT7622_field.multi_dma_en = MULTI_DMA_EN_FEATURE_2_PREFETCH;

#ifdef CONFIG_DELAY_INT
			Value |= TX_DLY_INT_EN;
			Value &= ~TX_MAX_PINT_MASK;
			Value |= TX_MAX_PINT(TX_PENDING_INT_NUMS);
			Value &= ~TX_MAX_PTIME_MASK;
			Value |= TX_MAX_PTIME(TX_PENDING_INT_TIME);
			Value |= RX_DLY_INT_EN;
			Value &= ~RX_MAX_PINT_MASK;
			Value |= RX_MAX_PINT(RX_PENDING_INT_NUMS);
			Value &= ~RX_MAX_PTIME_MASK;
			Value |= RX_MAX_PTIME(RX_PENDING_INT_TIME);
#endif
		} else {
			GloCfg.MT7622_field.EnableRxDMA = 0;
			GloCfg.MT7622_field.EnableTxDMA = 0;
#ifdef CONFIG_DELAY_INT
			Value &= ~TX_DLY_INT_EN;
			Value &= ~TX_MAX_PINT_MASK;
			Value &= ~TX_MAX_PTIME_MASK;
			Value &= ~RX_DLY_INT_EN;
			Value &= ~RX_MAX_PINT_MASK;
			Value &= ~RX_MAX_PTIME_MASK;
#endif
		}

		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown path (%d\n", __func__, TxRx));
		break;
	}

	HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, GloCfg.word);
#ifdef CONFIG_DELAY_INT
	HIF_IO_WRITE32(pAd, MT_DELAY_INT_CFG, Value);
#endif

#define WPDMA_DISABLE -1

	if (!enable)
		TxRx = WPDMA_DISABLE;

	WLAN_HOOK_CALL(WLAN_HOOK_DMA_SET, pAd, &TxRx);
	return TRUE;
}

static BOOLEAN hif_wait_WPDMA_idle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us)
{
	INT i = 0;

	WPDMA_GLO_CFG_STRUC GloCfg;
	/* TODO: shiang-MT7615 */
	do {
		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);

		if ((GloCfg.MT7622_field.TxDMABusy == 0)  && (GloCfg.MT7622_field.RxDMABusy == 0)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>  DMAIdle, GloCfg=0x%x\n", GloCfg.word));
			return TRUE;
		}

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return FALSE;

		RtmpusecDelay(wait_us);
	} while ((i++) < round);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>  DMABusy\n"));
	return FALSE;
}

static BOOLEAN hif_reset_WPDMA(RTMP_ADAPTER *pAd)
{
	UINT32 value = 0;

	/* pdma0 hw reset (w/ dma scheduler)
		activate: toggle (active low)
		scope: PDMA + DMASCH + Tx/Rx FIFO
		PDMA:
			logic reset: Y
			register reset: N (but DMA_IDX will be reset to 0)
		DMASCH:
			logic reset: Y
			register reset: Y
	*/
	HIF_PDMA_IO_READ32(pAd, MT_HIF_SYS_SW_RST, &value);
	value &= ~(HIF_DATA_PATH_RESET_N);
	HIF_PDMA_IO_WRITE32(pAd, MT_HIF_SYS_SW_RST, value);
	HIF_PDMA_IO_READ32(pAd, MT_HIF_SYS_SW_RST, &value);
	value |= (HIF_DATA_PATH_RESET_N);
	HIF_PDMA_IO_WRITE32(pAd, MT_HIF_SYS_SW_RST, value);

	return TRUE;
}

static INT32 get_fw_sync_value(RTMP_ADAPTER *pAd)
{
	UINT32 value;

	RTMP_IO_READ32(pAd, TOP_OFF_RSV, &value);
	value = (value & 0x00070000) >> 16;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: current sync CR = 0x%x\n", __func__, value));
	return value;
}

VOID mt7622_trigger_intr_to_mcu(BOOLEAN enable)
{
	unsigned long addr;
	unsigned int value;

	addr = (unsigned long)ioremap(HIF_INTR_BASE, HIF_REMAP_SIZE);
	RTMP_SYS_IO_READ32(addr, &value);

	/* write 0 to assert interrupt */
	if (enable == TRUE)
		value &= ~HIF_INTR_VALUE;
	else
		value |= HIF_INTR_VALUE;

	RTMP_SYS_IO_WRITE32(addr, value);
	iounmap((void *)addr);
}

static VOID fw_own(RTMP_ADAPTER *pAd)
{
	UINT32 counter, value;

	if (pAd->bDrvOwn == FALSE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()::Return since already in Fw Own...\n", __func__));
		return;
	}

	/* trigger hif interrupt to MCU */
	mt7622_trigger_intr_to_mcu(TRUE);

	/* Write any value to HIF_FUN_CAP to set FW own */
	HIF_IO_WRITE32(pAd, MT_CFG_LPCR_HOST, MT_HOST_SET_OWN);

	/* Poll driver own status */
	counter = 0;

	while (counter < FW_OWN_POLLING_COUNTER) {
		RtmpusecDelay(1000);
		HIF_IO_READ32(pAd, MT_CFG_LPCR_HOST, &value);

		if (value & MT_HOST_SET_OWN) {
			pAd->bDrvOwn = FALSE;
			break;
		}

		counter++;
	}

	mt7622_trigger_intr_to_mcu(FALSE);

	if (!pAd->bDrvOwn)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()::Success to set FW Own\n", __func__));
	else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()::Fail to set FW Own (%d)\n", __func__, counter));
}

static INT32 driver_own(RTMP_ADAPTER *pAd)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UINT32 counter, value;

	if (pAd->bDrvOwn == TRUE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()::Return since already in Driver Own...\n", __func__));
		return Ret;
	}

	/* trigger hif interrupt to MCU */
	mt7622_trigger_intr_to_mcu(TRUE);

	/* Write CR to get driver own */
	HIF_IO_WRITE32(pAd, MT_CFG_LPCR_HOST, MT_HOST_CLR_OWN);

	/* Poll driver own status */
	counter = 0;

	while (counter < FW_OWN_POLLING_COUNTER) {
		RtmpusecDelay(1000);

		/* polling case */
		HIF_IO_READ32(pAd, MT_CFG_LPCR_HOST, &value);
		if (!(value & MT_HOST_SET_OWN)) {
			pAd->bDrvOwn = TRUE;
			break;
		}

		/* interrupt case */
		if (pAd->bDrvOwn)
			break;

		counter++;
	}

	mt7622_trigger_intr_to_mcu(FALSE);

	if (pAd->bDrvOwn)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()::Success to clear Fw Own\n", __func__));
	else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()::Fail to clear Fw Own (%d)\n", __func__, counter));
		Ret = NDIS_STATUS_FAILURE;
	}

	return Ret;
}

void mt7622_heart_beat_check(RTMP_ADAPTER *pAd)
{
#define HEART_BEAT_CHECK_PERIOD 30
#define N9_HEART_BEAT_ADDR 0x2300 /* TOP_CONFIG_DUMMY_SER_RECOVER_RECORD, dummy CR, defined in N9 FW */
	UINT32 mac_val;
	UINT8 n9_detect = FALSE;
	RTMP_STRING *str = NULL;

	if ((pAd->Mlme.PeriodicRound % HEART_BEAT_CHECK_PERIOD) == 0) {

		if (pAd->heart_beat_stop == TRUE)
			return;

		MAC_IO_READ32(pAd, N9_HEART_BEAT_ADDR, &mac_val);

		if (mac_val == pAd->pre_n9_heart_beat_cnt)
			pAd->pre_n9_heart_beat_cnt = ~mac_val;
		else if (~mac_val == pAd->pre_n9_heart_beat_cnt)
			n9_detect = TRUE;
		else
			pAd->pre_n9_heart_beat_cnt = mac_val;

		if (n9_detect)
			str = "N9 heart beat stop!!\n";

		if (str != NULL) {
			pAd->heart_beat_stop = TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("[%s]:%s", RtmpOsGetNetDevName(pAd->net_dev), str));
#ifdef MT_FDB
			show_fdb_n9_log(pAd, NULL);
#endif /* MT_FDB */
#ifdef ERR_RECOVERY
			ser_sys_reset(str);
#endif
		}
	}
}


#ifdef CONFIG_PROPRIETARY_DRIVER
#define EFUSE_NIC_CONFIG_1 0x36
#define EFUSE_TSSI_MASK 0x20
#define EFUSE_WIFI_CONFIG 0x3E
#define EFUSE_PA_LNA_MASK 0xC
#define EFUSE_PA_MASK 0x8
#define EFUSE_PA_LNA_SHIFT 2
#define EFUSE_PA_SHIFT 3

static INT32 mt7622_tssi_set(struct _RTMP_ADAPTER *ad, UCHAR *efuse)
{
	UCHAR *nic_cfg1 = &efuse[EFUSE_NIC_CONFIG_1 + 1];
	UCHAR mode = (*nic_cfg1 & EFUSE_TSSI_MASK) ? 1 : 0;
	UCHAR *wifi_cfg = &efuse[EFUSE_WIFI_CONFIG];
	UCHAR pa_mode = (*wifi_cfg & EFUSE_PA_MASK) >> EFUSE_PA_SHIFT;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): nic_cfg1=%x, mode=%d\n", __func__, *nic_cfg1, mode));
	if (pa_mode && mode)
		return rbus_tssi_set(ad, mode);
	else
		return 0;
}

static INT32 mt7622_pa_lna_set(struct _RTMP_ADAPTER *ad, UCHAR *efuse)
{
	UINT8 antCtrl  = 0;
	UINT8 polCtrl  = 0;
	UINT8 wifiConfig1, wifiConfig2, coexType, wifiPolarity1, wifiPolarity2;
	UCHAR wifiStreams;
	UINT32 efuse_settings = 0;

	wifiConfig1 = efuse[EFUSE_ANTENNA_CONFIG_POS1];
	wifiConfig2 = efuse[EFUSE_ANTENNA_CONFIG_POS2];
	coexType = efuse[EFUSE_ANTENNA_COEX_TYPE_POS];
	wifiPolarity1 = efuse[EFUSE_ANTENNA_POLARITY_POS1];
	wifiPolarity2 = efuse[EFUSE_ANTENNA_POLARITY_POS2];
	wifiStreams = efuse[EFUSE_ANTENNA_STREAMS_POS];

	antCtrl |= GetGbandPaLna(wifiConfig1);
	antCtrl |= ((GetFEMPowerSave(wifiConfig2)) << FEM_PWR_SAV_OFFSET);
	antCtrl |= (GetStreams(wifiConfig2) << 2);
	antCtrl |= ((GetCoex(coexType)) << COEX_TYPE_OFFSET);
	antCtrl |= ((GetSmartAntMode(wifiConfig1)) << SMART_ANT_OFFSET);
	antCtrl |= ((GetPIP(wifiConfig2)) << BT_PIP_OFFSET);
	polCtrl |= (GetPolTRSWN(wifiPolarity1));
	polCtrl |= ((GetPolTRSWP(wifiPolarity1)) << POLARITY_TRSW_P_OFFSET);
	polCtrl |= ((GetPolLNA(wifiPolarity1)) << POLARITY_LNA_OFFSET);
	polCtrl |= ((GetPolPA(wifiPolarity1)) << POLARITY_PA_OFFSET);
	polCtrl |= ((GetSAntpol(wifiPolarity2)) << SMART_ANT_POLARITY_OFFSET);
	polCtrl |= ((GetPIPPol(wifiPolarity2)) << BT_PIP_POLARITY_OFFSET);
	polCtrl |= ((GetVLINPol(wifiPolarity2)) << VLIN_LINEARITY_OFFSET);
	polCtrl |= ((GetSharedPin(wifiPolarity2)) << SHARED_PIN_OFFSET);
	efuse_settings |= (antCtrl | (polCtrl << 8) | (wifiStreams << 16));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): read antCtrl = %d, polCtrl = %d, wifiStreams = %02x\n", __func__, antCtrl, polCtrl, wifiStreams));
	return rbus_pa_lna_set(ad, efuse_settings);
}

#endif /*CONFIG_PROPRIETARY_DRIVER*/

static struct dly_ctl_cfg mt7622_rx_dly_ctl_ul_tbl[] = {
	{0, 0x811c},
	{300, 0xa01c},
	{500, 0xc01f},
};

static struct dly_ctl_cfg mt7622_rx_dly_ctl_dl_tbl[] = {
	{0, 0x811c},
};

static RTMP_CHIP_OP MT7622_ChipOp = {0};
static RTMP_CHIP_CAP MT7622_ChipCap = {0};

static VOID mt7622_chipCap_init(void)
{
#ifdef DOT11_VHT_AC
	MT7622_ChipCap.max_vht_mcs = VHT_MCS_CAP_9;
	MT7622_ChipCap.max_mpdu_len = MPDU_3895_OCTETS;
	MT7622_ChipCap.vht_max_ampdu_len_exp = 7;
#ifdef G_BAND_256QAM
	MT7622_ChipCap.g_band_256_qam = TRUE;
#endif
#endif /* DOT11_VHT_AC */
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	MT7622_ChipCap.max_v2_bcn_num = 16;
#endif
	MT7622_ChipCap.TXWISize = sizeof(TMAC_TXD_L);
	MT7622_ChipCap.RXWISize = 28;
	MT7622_ChipCap.tx_hw_hdr_len = MT7622_ChipCap.TXWISize;
	MT7622_ChipCap.rx_hw_hdr_len = MT7622_ChipCap.RXWISize;
	MT7622_ChipCap.num_of_tx_ring = 6;
	MT7622_ChipCap.num_of_rx_ring = 2;
	MT7622_ChipCap.tx_ring_size = 512;
	MT7622_ChipCap.rx0_ring_size = 512;
	MT7622_ChipCap.rx1_ring_size = 512;
	MT7622_ChipCap.asic_caps = (fASIC_CAP_PMF_ENC | fASIC_CAP_MCS_LUT
								| fASIC_CAP_CT | fASIC_CAP_HW_DAMSDU);
#ifdef RX_CUT_THROUGH
	MT7622_ChipCap.asic_caps |= fASIC_CAP_BA_OFFLOAD;
#endif
#ifdef HDR_TRANS_TX_SUPPORT
	MT7622_ChipCap.asic_caps |= fASIC_CAP_TX_HDR_TRANS;
#endif
#ifdef HDR_TRANS_RX_SUPPORT
	MT7622_ChipCap.asic_caps |= fASIC_CAP_RX_HDR_TRANS;
#endif
#ifdef CONFIG_CSO_SUPPORT
	MT7622_ChipCap.asic_caps |= fASIC_CAP_CSO;
#endif
#ifdef RX_SCATTER
	MT7622_ChipCap.asic_caps |= fASIC_CAP_RX_DMA_SCATTER;
#endif

	MT7622_ChipCap.asic_caps |= fASIC_CAP_RX_DLY;

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	MT7622_ChipCap.asic_caps &= ~fASIC_CAP_PCIE_ASPM_DYM_CTRL;
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

	MT7622_ChipCap.phy_caps = (fPHY_CAP_24G | fPHY_CAP_HT | \
							   fPHY_CAP_TXBF | fPHY_CAP_LDPC | \
							   fPHY_CAP_BW40);
	MT7622_ChipCap.hw_ops_ver = HWCTRL_OP_TYPE_V2;
	MT7622_ChipCap.hif_type = HIF_MT;
	MT7622_ChipCap.mac_type = MAC_MT;
	MT7622_ChipCap.MCUType = ANDES;
	MT7622_ChipCap.rf_type = RF_MT;
	MT7622_ChipCap.pRFRegTable = NULL;
	MT7622_ChipCap.pBBPRegTable = NULL;
	MT7622_ChipCap.bbpRegTbSize = 0;
	MT7622_ChipCap.MaxNumOfRfId = MAX_RF_ID;
	MT7622_ChipCap.MaxNumOfBbpId = 200;
	MT7622_ChipCap.WtblHwNum = MT7622_MT_WTBL_SIZE;
	MT7622_ChipCap.FlgIsHwWapiSup = TRUE;
	MT7622_ChipCap.FlgIsHwAntennaDiversitySup = FALSE;
#ifdef STREAM_MODE_SUPPORT
	MT7622_ChipCap.FlgHwStreamMode = FALSE;
#endif
#ifdef TXBF_SUPPORT
	MT7622_ChipCap.FlgHwTxBfCap = TXBF_HW_CAP;
#endif
	MT7622_ChipCap.SnrFormula = SNR_FORMULA4;
	if (get_subid() == 0x4) {
		MT7622_ChipCap.max_nss = 2;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("7622D found!\n"));
	} else {
		MT7622_ChipCap.max_nss = 4;
	}
	/* todo Ellis */
#ifdef RTMP_EFUSE_SUPPORT
	MT7622_ChipCap.EFUSE_USAGE_MAP_START = 0x3c0;
	MT7622_ChipCap.EFUSE_USAGE_MAP_END = 0x3fb;
	MT7622_ChipCap.EFUSE_USAGE_MAP_SIZE = 60;
	MT7622_ChipCap.EFUSE_RESERVED_SIZE = 59;	/* Cal-Free is 22 free block */
#endif
	MT7622_ChipCap.EEPROM_DEFAULT_BIN = mt7622_get_default_bin_image();
	MT7622_ChipCap.EEPROM_DEFAULT_BIN_FILE = mt7622_get_default_bin_image_file();
	MT7622_ChipCap.EEPROM_DEFAULT_BIN_SIZE = sizeof(MT7622_E2PImage_ePAeLNA);
	MT7622_ChipCap.EFUSE_BUFFER_CONTENT_SIZE = EFUSE_CONTENT_SIZE;
#ifdef CARRIER_DETECTION_SUPPORT
	MT7622_ChipCap.carrier_func = TONE_RADAR_V2;
#endif
#ifdef RTMP_MAC_PCI
	MT7622_ChipCap.WPDMABurstSIZE = 3;
#endif
	MT7622_ChipCap.ProbeRspTimes = 2;
#ifdef NEW_MBSSID_MODE
#ifdef ENHANCE_NEW_MBSSID_MODE
	MT7622_ChipCap.MBSSIDMode = MBSSID_MODE4;
#else
	MT7622_ChipCap.MBSSIDMode = MBSSID_MODE1;
#endif /* ENHANCE_NEW_MBSSID_MODE */
#else
	MT7622_ChipCap.MBSSIDMode = MBSSID_MODE0;
#endif /* NEW_MBSSID_MODE */
#ifdef DOT11W_PMF_SUPPORT
	/* sync with Ellis, wilsonl */
	MT7622_ChipCap.FlgPMFEncrtptMode = PMF_ENCRYPT_MODE_2;
#endif /* DOT11W_PMF_SUPPORT */
#ifdef CONFIG_ANDES_SUPPORT
#ifdef NEED_ROM_PATCH
	MT7622_ChipCap.need_load_patch = BIT(WM_CPU);
#else
	MT7622_ChipCap.need_load_patch = 0;
#endif
	MT7622_ChipCap.need_load_fw = BIT(WM_CPU);
	MT7622_ChipCap.load_patch_flow = PATCH_FLOW_V1;
	MT7622_ChipCap.load_fw_flow = FW_FLOW_V1;
	MT7622_ChipCap.patch_format = PATCH_FORMAT_V1;
	MT7622_ChipCap.fw_format = FW_FORMAT_V2;
	MT7622_ChipCap.load_fw_method = BIT(HEADER_METHOD);
	MT7622_ChipCap.load_patch_method = BIT(BIN_METHOD) | BIT(HEADER_METHOD);
	MT7622_ChipCap.rom_patch_offset = MT7622_ROM_PATCH_START_ADDRESS;
#endif
#ifdef UNIFY_FW_CMD /* todo wilsonl */
	MT7622_ChipCap.cmd_header_len = sizeof(FW_TXD) + sizeof(TMAC_TXD_L);
#else
	MT7622_ChipCap.cmd_header_len = 12; /* sizeof(FW_TXD) */
#endif
	MT7622_ChipCap.cmd_padding_len = 0;
	MT7622_ChipCap.TxAggLimit = 64;
	MT7622_ChipCap.RxBAWinSize = 64;
	MT7622_ChipCap.ht_max_ampdu_len_exp = 3;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	/* enabled later, wilsonl */
	MT7622_ChipCap.fgRateAdaptFWOffload = TRUE;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	MT7622_ChipCap.WmmHwNum = MT7622_MT_WMM_SIZE; /* for multi-wmm */
	MT7622_ChipCap.PDA_PORT = MT7622_PDA_PORT;
	MT7622_ChipCap.SupportAMSDU = TRUE;
	/* sync with Pat, wilsonl */
	MT7622_ChipCap.APPSMode = APPS_MODE2;
	MT7622_ChipCap.CtParseLen = MT7622_CT_PARSE_LEN;
	MT7622_ChipCap.qm = GENERIC_QM;
	MT7622_ChipCap.qm_tm = TASKLET_METHOD;
	MT7622_ChipCap.hif_tm = TASKLET_METHOD;
	MT7622_ChipCap.wmm_detect_method = WMM_DETECT_METHOD1;

	/* for interrupt enable mask */
	MT7622_ChipCap.int_enable_mask = MT_CoherentInt | MT_MacInt | MT_INT_RX_DLY |
						MT_INT_T3_DONE | MT_INT_T15_DONE | MT_FW_CLR_OWN_INT;

	MT7622_ChipCap.hif_pkt_type[HIF_TX_IDX0] = TX_DATA;
	MT7622_ChipCap.hif_pkt_type[HIF_TX_IDX1] = TX_DATA;
	MT7622_ChipCap.hif_pkt_type[HIF_TX_IDX2] = TX_DATA;
	MT7622_ChipCap.hif_pkt_type[HIF_TX_IDX3] = TX_FW_DL;
	MT7622_ChipCap.hif_pkt_type[HIF_TX_IDX4] = TX_DATA;
	MT7622_ChipCap.hif_pkt_type[HIF_TX_IDX5] = TX_ALTX;
	MT7622_ChipCap.hif_pkt_type[HIF_TX_IDX15] = TX_CMD;
#if defined(ERR_RECOVERY) || defined(CONFIG_FWOWN_SUPPORT)
	MT7622_ChipCap.int_enable_mask |= MT_McuCommand;
#endif /* ERR_RECOVERY || CONFIG_FWOWN_SUPPORT */
#ifdef INTERNAL_CAPTURE_SUPPORT
	MT7622_ChipCap.pICapDesc = MT7622_ICAP_DESC;
	MT7622_ChipCap.ICapBankNum = MT7622_ICapBankNum;
	MT7622_ChipCap.ICapADCIQCnt = MT7622_ICAP_FOUR_WAY_ADC_IQ_DATA_CNT;
	MT7622_ChipCap.ICapIQCIQCnt = MT7622_ICAP_FOUR_WAY_IQC_IQ_DATA_CNT;
	MT7622_ChipCap.ICapBankSmplCnt = MT7622_ICAP_BANK_SAMPLE_CNT;
	MT7622_ChipCap.ICapFourWayADC = MT7622_CAP_FOUR_WAY_ADC;
	MT7622_ChipCap.ICapFourWayFIIQ = MT7622_CAP_FOUR_WAY_FIIQ;
	MT7622_ChipCap.ICapFourWayFDIQ = MT7622_CAP_FOUR_WAY_FDIQ;
#endif /* INTERNAL_CAPTURE_SUPPORT */
#ifdef WIFI_SPECTRUM_SUPPORT
	MT7622_ChipCap.pSpectrumDesc = MT7622_SPECTRUM_DESC;
	MT7622_ChipCap.SpectrumBankNum = MT7622_SpectrumBankNum;
	MT7622_ChipCap.SpectrumWF0ADC = MT7622_CAP_WF0_ADC;
	MT7622_ChipCap.SpectrumWF1ADC = MT7622_CAP_WF1_ADC;
	MT7622_ChipCap.SpectrumWF2ADC = MT7622_CAP_WF2_ADC;
	MT7622_ChipCap.SpectrumWF3ADC = MT7622_CAP_WF3_ADC;
	MT7622_ChipCap.SpectrumWF0FIIQ = MT7622_CAP_WF0_FIIQ;
	MT7622_ChipCap.SpectrumWF1FIIQ = MT7622_CAP_WF1_FIIQ;
	MT7622_ChipCap.SpectrumWF2FIIQ = MT7622_CAP_WF2_FIIQ;
	MT7622_ChipCap.SpectrumWF3FIIQ = MT7622_CAP_WF3_FIIQ;
	MT7622_ChipCap.SpectrumWF0FDIQ = MT7622_CAP_WF0_FDIQ;
	MT7622_ChipCap.SpectrumWF1FDIQ = MT7622_CAP_WF1_FDIQ;
	MT7622_ChipCap.SpectrumWF2FDIQ = MT7622_CAP_WF2_FDIQ;
	MT7622_ChipCap.SpectrumWF3FDIQ = MT7622_CAP_WF3_FDIQ;
#endif /* WIFI_SPECTRUM_SUPPORT */
	MT7622_ChipCap.band_cnt = 1;
#ifdef COEX_SUPPORT
	MT7622_ChipCap.TxBAWinSize = 64;
#endif /* COEX_SUPPORT*/
	MT7622_ChipCap.ba_range = mt7622_ba_range;
	MT7622_ChipCap.txd_type = TXD_V1;
	MT7622_ChipCap.tx_delay_support = TRUE;

	MT7622_ChipCap.asic_caps |= fASIC_CAP_ADV_SECURITY;
}

static VOID mt7622_chipOp_init(void)
{
	MT7622_ChipOp.AsicRfInit = mt7622_init_rf_cr;
	MT7622_ChipOp.AsicBbpInit = MT7622BBPInit;
	MT7622_ChipOp.AsicMacInit = mt7622_init_mac_cr;
	MT7622_ChipOp.AsicReverseRfFromSleepMode = NULL;
	MT7622_ChipOp.AsicHaltAction = NULL;
	/* BBP adjust */
	MT7622_ChipOp.ChipBBPAdjust = mt7622_bbp_adjust;
	/* AGC */
	MT7622_ChipOp.ChipSwitchChannel = mt7622_switch_channel;
#ifdef NEW_SET_RX_STREAM
	MT7622_ChipOp.ChipSetRxStream = mt7622_set_RxStream;
#endif
	MT7622_ChipOp.AsicAntennaDefaultReset = mt7622_antenna_default_reset;
#ifdef CAL_FREE_IC_SUPPORT
	/* do not need, turn off compile flag, wilsonl */
	MT7622_ChipOp.is_cal_free_ic = mt7622_is_cal_free_ic;
	MT7622_ChipOp.cal_free_data_get = mt7622_cal_free_data_get;
#endif /* CAL_FREE_IC_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	MT7622_ChipOp.ToneRadarProgram = ToneRadarProgram_v2;
#endif
	MT7622_ChipOp.DisableTxRx = NULL; /* 302 */
#ifdef RTMP_PCI_SUPPORT
	/* sync with Pat, Hammin, wilsonl */
	/* MT7622_ChipOp.AsicRadioOn = RT28xxPciAsicRadioOn; */
	/* MT7622_ChipOp.AsicRadioOff = RT28xxPciAsicRadioOff; */
#endif
#ifdef RF_LOCKDOWN
	MT7622_ChipOp.check_RF_lock_down = mt7622_check_RF_lock_down;
	MT7622_ChipOp.write_RF_lock_parameter = mt7622_write_RF_lock_parameter;
	MT7622_ChipOp.merge_RF_lock_parameter = mt7622_merge_RF_lock_parameter;
	MT7622_ChipOp.Read_Effuse_parameter = mt7622_Read_Effuse_parameter;
	MT7622_ChipOp.Config_Effuse_Country = mt7622_Config_Effuse_Country;
#endif /* RF_LOCKDOWN */
#ifdef MT_WOW_SUPPORT
	/* do not need, turn off compile flag, wilsonl */
	MT7622_ChipOp.AsicWOWEnable = MT76xxAndesWOWEnable;
	MT7622_ChipOp.AsicWOWDisable = MT76xxAndesWOWDisable;
	/* MT7622_ChipOp.AsicWOWInit = MT76xxAndesWOWInit, */
#endif /* MT_WOW_SUPPORT */
	MT7622_ChipOp.show_pwr_info = NULL;
	MT7622_ChipOp.bufferModeEfuseFill = mt7622_bufferModeEfuseFill;
	/* For MT7622 keep efuse field only */
	MT7622_ChipOp.keep_efuse_field_only = mt7622_keep_efuse_field_only;
	MT7622_ChipOp.MtCmdTx = MtCmdSendMsg;
	MT7622_ChipOp.prepare_fwdl_img = mt7622_fw_prepare;
	MT7622_ChipOp.fwdl_datapath_setup = mt7622_fwdl_datapath_setup;
#ifdef TXBF_SUPPORT
	MT7622_ChipOp.TxBFInit                 = mt_WrapTxBFInit;
	MT7622_ChipOp.ClientSupportsETxBF      = mt_WrapClientSupportsETxBF;
	MT7622_ChipOp.iBFPhaseComp             = mt7622_iBFPhaseComp;
	MT7622_ChipOp.iBFPhaseCalInit          = mt7622_iBFPhaseCalInit;
	MT7622_ChipOp.iBFPhaseFreeMem          = mt7622_iBFPhaseFreeMem;
	MT7622_ChipOp.iBFPhaseCalE2PUpdate     = mt7622_iBFPhaseCalE2PUpdate;
	MT7622_ChipOp.iBFPhaseCalReport        = mt7622_iBFPhaseCalReport;
	MT7622_ChipOp.setETxBFCap              = mt7622_setETxBFCap;
	MT7622_ChipOp.BfStaRecUpdate           = mt_AsicBfStaRecUpdate;
	MT7622_ChipOp.BfStaRecRelease          = mt_AsicBfStaRecRelease;
	MT7622_ChipOp.BfPfmuMemAlloc           = CmdPfmuMemAlloc;
	MT7622_ChipOp.BfPfmuMemRelease         = CmdPfmuMemRelease;
	MT7622_ChipOp.TxBfTxApplyCtrl          = CmdTxBfTxApplyCtrl;
	MT7622_ChipOp.BfApClientCluster        = CmdTxBfApClientCluster;
	MT7622_ChipOp.BfHwEnStatusUpdate       = CmdTxBfHwEnableStatusUpdate;
	MT7622_ChipOp.BfeeHwCtrl               = CmdTxBfeeHwCtrl;
	MT7622_ChipOp.BfModuleEnCtrl           = NULL;
#endif /* TXBF_SUPPORT */
#ifdef GREENAP_SUPPORT
	MT7622_ChipOp.EnableAPMIMOPS = enable_greenap;
	MT7622_ChipOp.DisableAPMIMOPS = disable_greenap;
#endif /* GREENAP_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	MT7622_ChipOp.pcie_aspm_dym_ctrl = NULL;
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef INTERNAL_CAPTURE_SUPPORT
	MT7622_ChipOp.ICapStart = MtCmdRfTestICapStart;
	MT7622_ChipOp.ICapStatus = MtCmdRfTestGen2ICapStatus;
	MT7622_ChipOp.ICapCmdRawDataProc = MtCmdRfTestICapRawDataProc;
	MT7622_ChipOp.ICapGetIQData = Get_RBIST_IQ_Data;
	MT7622_ChipOp.ICapEventRawDataHandler = ExtEventICap96BitRawDataHandler;
#endif /* INTERNAL_CAPTURE_SUPPORT */
#ifdef WIFI_SPECTRUM_SUPPORT
	MT7622_ChipOp.SpectrumStart = MtCmdWifiSpectrumStart;
	MT7622_ChipOp.SpectrumStatus = MtCmdWifiSpectrumGen2Status;
	MT7622_ChipOp.SpectrumCmdRawDataProc = MtCmdWifiSpectrumRawDataProc;
	MT7622_ChipOp.SpectrumEventRawDataHandler = ExtEventWifiSpectrumRawDataHandler;
#endif /* WIFI_SPECTRUM_SUPPORT */
	MT7622_ChipOp.dma_shdl_init = mt7622_dma_shdl_init;
#ifdef SMART_CARRIER_SENSE_SUPPORT
	MT7622_ChipOp.SmartCarrierSense = SmartCarrierSense_Gen3;
	MT7622_ChipOp.ChipSetSCS = SetSCS;
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef MT7622_FPGA
	MT7622_ChipOp.chk_hif_default_cr_setting = mt7622_chk_hif_default_cr_setting;
	MT7622_ChipOp.chk_top_default_cr_setting = mt7622_chk_top_default_cr_setting
#endif
#ifdef HOST_RESUME_DONE_ACK_SUPPORT
	MT7622_ChipOp.HostResumeDoneAck = mt7622_host_resume_done_ack;
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
	MT7622_ChipOp.hif_set_dma = hif_set_WPDMA;
	MT7622_ChipOp.hif_wait_dma_idle = hif_wait_WPDMA_idle;
	MT7622_ChipOp.hif_reset_dma = hif_reset_WPDMA;
	MT7622_ChipOp.get_fw_sync_value = get_fw_sync_value;
	MT7622_ChipOp.read_chl_pwr = NULL;
	MT7622_ChipOp.parse_RXV_packet = parse_RXV_packet_v2;
	MT7622_ChipOp.txs_handler = txs_handler_v2;
	MT7622_ChipOp.driver_own = driver_own;
	MT7622_ChipOp.fw_own = fw_own;
#ifdef CONFIG_PROPRIETARY_DRIVER
	MT7622_ChipOp.tssi_set = mt7622_tssi_set;
	MT7622_ChipOp.pa_lna_set = mt7622_pa_lna_set;
#endif /*CONFIG_PROPRIETARY_DRIVER*/
	MT7622_ChipOp.heart_beat_check = mt7622_heart_beat_check;
}


VOID mt7622_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
	struct tr_delay_control *tr_delay_ctl = &pAd->tr_ctl.tr_delay_ctl;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()-->\n", __func__));
	mt7622_chipCap_init();
	mt7622_chipOp_init();
	NdisMoveMemory(pChipCap, &MT7622_ChipCap, sizeof(RTMP_CHIP_CAP));
	hc_register_chip_ops(pAd->hdev_ctrl, &MT7622_ChipOp);
#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
#else
	Mt7622AsicArchOpsInit(pAd);
#endif
	mt7622_chip_dbg_init(chip_dbg);
	mt_phy_probe(pAd);
	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_AGBS);
	/* Following function configure beacon related parameters in pChipCap
	 * FlgIsSupSpecBcnBuf / BcnMaxHwNum / WcidHwRsvNum / BcnMaxHwSize / BcnBase[]
	 */
	/* sync with Cater, wilsonl */
	mt_chip_bcn_parameter_init(pAd);
	/* sync with Cater, wilsonl */
	pChipCap->OmacNums = 5;
	pChipCap->BssNums = 4;
	pChipCap->ExtMbssOmacStartIdx = 0x10;
	pChipCap->RepeaterStartIdx = 0x20;
#ifdef AIR_MONITOR
	pChipCap->MaxRepeaterNum = 16;
#else
	pChipCap->MaxRepeaterNum = 32;
#endif /* AIR_MONITOR */
#ifdef BCN_OFFLOAD_SUPPORT
	pChipCap->fgBcnOffloadSupport = TRUE;
	pChipCap->fgIsNeedPretbttIntEvent = FALSE;
#endif
	/* TMR HW version */
	pChipCap->TmrHwVer = TMR_VER_2_0;

	/* For calibration log buffer size limitation issue */
	pAd->fgQAtoolBatchDumpSupport = TRUE;
#ifdef RED_SUPPORT
	pAd->red_have_cr4 = FALSE;
#endif /* RED_SUPPORT */
	pAd->cp_have_cr4 = FALSE;
#ifdef CONFIG_AP_SUPPORT
	/*VOW CR Address offset - Gen_2*/
	pAd->vow_gen.VOW_GEN = VOW_GEN_2;
#endif /* #ifdef CONFIG_AP_SUPPORT */
	tr_delay_ctl->ul_rx_dly_ctl_tbl = mt7622_rx_dly_ctl_ul_tbl;
	tr_delay_ctl->ul_rx_dly_ctl_tbl_size = (sizeof(mt7622_rx_dly_ctl_ul_tbl) / sizeof(mt7622_rx_dly_ctl_ul_tbl[0]));
	tr_delay_ctl->dl_rx_dly_ctl_tbl = mt7622_rx_dly_ctl_dl_tbl;
	tr_delay_ctl->dl_rx_dly_ctl_tbl_size = (sizeof(mt7622_rx_dly_ctl_dl_tbl) / sizeof(mt7622_rx_dly_ctl_dl_tbl[0]));

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--%s()\n", __func__));
}

#ifdef COEX_SUPPORT
void mt7622_antenna_sel_cfg(RTMP_ADAPTER *pAd)
{

	UINT8 antCtrl  = 0;
	UINT8 polCtrl  = 0;
	UINT8 wifiConfig1, wifiConfig2, coexType, wifiPolarity1, wifiPolarity2;
	UINT16 value = 0;
	UCHAR wifiStreams;

	if (pAd->eeprom_type == EEPROM_EFUSE) {
		/* WiFi_Config/Polarity Setting read for Antenna Selection Configuraiton*/
		rtmp_ee_efuse_read16(pAd, EFUSE_ANTENNA_CONFIG_POS1, &value);
		wifiConfig1 = (UCHAR)(value & 0x00FF);

		rtmp_ee_efuse_read16(pAd, (EFUSE_ANTENNA_CONFIG_POS2-1), &value);
		wifiConfig2 = (UCHAR)((value >> 8) & 0x00FF);

		rtmp_ee_efuse_read16(pAd, EFUSE_ANTENNA_COEX_TYPE_POS, &value);
		coexType = (UCHAR)(value & 0x00FF);

		rtmp_ee_efuse_read16(pAd, EFUSE_ANTENNA_POLARITY_POS1, &value);
		wifiPolarity1 = (UCHAR)(value & 0x00FF);
		wifiPolarity2 = (UCHAR)((value >> 8) & 0x00FF); /* EFUSE_ANTENNA_POLARITY_POS2 */

		rtmp_ee_efuse_read16(pAd, EFUSE_ANTENNA_STREAMS_POS, &value);
		wifiStreams = (UCHAR)(value & 0x00FF);
	} else if (pAd->eeprom_type == EEPROM_FLASH) {
		wifiConfig1 = (pAd->EEPROMImage[EFUSE_ANTENNA_CONFIG_POS1]);
		wifiConfig2 = (pAd->EEPROMImage[EFUSE_ANTENNA_CONFIG_POS2]);
		coexType = (pAd->EEPROMImage[EFUSE_ANTENNA_COEX_TYPE_POS]);
		wifiPolarity1 = (pAd->EEPROMImage[EFUSE_ANTENNA_POLARITY_POS1]);
		wifiPolarity2 = (pAd->EEPROMImage[EFUSE_ANTENNA_POLARITY_POS2]);
		wifiStreams = (pAd->EEPROMImage[EFUSE_ANTENNA_STREAMS_POS]);
	}

	antCtrl |= GetStreams(wifiConfig2);
	antCtrl |= ((GetFEMPowerSave(wifiConfig2)) << FEM_PWR_SAV_OFFSET);
	antCtrl |= ((GetGbandPaLna(wifiConfig1)) << PA_LNA_CFG_OFFSET);
	antCtrl |= ((GetCoex(coexType)) << COEX_TYPE_OFFSET);
	antCtrl |= ((GetSmartAntMode(wifiConfig1)) << SMART_ANT_OFFSET);
	antCtrl |= ((GetPIP(wifiConfig2)) << BT_PIP_OFFSET);
	polCtrl |= (GetPolTRSWN(wifiPolarity1));
	polCtrl |= ((GetPolTRSWP(wifiPolarity1)) << POLARITY_TRSW_P_OFFSET);
	polCtrl |= ((GetPolLNA(wifiPolarity1)) << POLARITY_LNA_OFFSET);
	polCtrl |= ((GetPolPA(wifiPolarity1)) << POLARITY_PA_OFFSET);
	polCtrl |= ((GetSAntpol(wifiPolarity2)) << SMART_ANT_POLARITY_OFFSET);
	polCtrl |= ((GetPIPPol(wifiPolarity2)) << BT_PIP_POLARITY_OFFSET);
	polCtrl |= ((GetVLINPol(wifiPolarity2)) << VLIN_LINEARITY_OFFSET);
	polCtrl |= ((GetSharedPin(wifiPolarity2)) << SHARED_PIN_OFFSET);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("read antCtrl = %d, polCtrl = %d \n", antCtrl, polCtrl));
	mt7622_pin_mux_control_setup(pAd, antCtrl, polCtrl, wifiStreams);
}


/****************************************************************************************************************
 * Function: _mt7668_pin_mux_control_setup
 * Purpose:
 *    1. This function select pin mux for specified properties and used antenna pins.
 ****************************************************************************************************************/
void mt7622_pin_mux_control_setup(RTMP_ADAPTER *pAd, UCHAR antCtrl, UCHAR polCtrl, UCHAR wifiStreams)
{
	UINT32 usedPins;
	UINT32 regAonVal;
	/* Get used pin. */
	usedPins = mt7622_antsel_get_used_pin(pAd, antCtrl, polCtrl, wifiStreams);

	/* Set pin mux for ANTSEL_0, ANTSEL_1, ANTSEL_2 , ANT_SEL_3, ANT_SEL_7, ANT_SEL_8, ANT_SEL_9, ANT_SEL_10 */

	regAonVal = mt7622_gpio_get(GPIO_GPIO_MODE9);

	if (usedPins & BIT(ANT_SEL_0)) {
		regAonVal &= ~GPIO91_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO91_MODE_OFFSET) & GPIO91_MODE);
	}
	if (usedPins & BIT(ANT_SEL_1)) {
		regAonVal &= ~GPIO92_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO92_MODE_OFFSET) & GPIO92_MODE);
	}
	if (usedPins & BIT(ANT_SEL_2)) {
		regAonVal &= ~GPIO93_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO93_MODE_OFFSET) & GPIO93_MODE);
	}
	if (usedPins & BIT(ANT_SEL_3)) {
		regAonVal &= ~GPIO94_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO94_MODE_OFFSET) & GPIO94_MODE);
	}

	if (usedPins & BIT(ANT_SEL_7)) {
		regAonVal &= ~GPIO98_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO98_MODE_OFFSET) & GPIO98_MODE);
	}
	if (usedPins & BIT(ANT_SEL_8)) {
		regAonVal &= ~GPIO99_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO99_MODE_OFFSET) & GPIO99_MODE);
	}
	if (usedPins & BIT(ANT_SEL_9)) {
		regAonVal &= ~GPIO100_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO100_MODE_OFFSET) & GPIO100_MODE);
	}
	if (usedPins & BIT(ANT_SEL_10)) {
		regAonVal &= ~GPIO101_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO101_MODE_OFFSET) & GPIO101_MODE);
	}

	mt7622_gpio_set(GPIO_GPIO_MODE9, regAonVal);

	/* Set pin mux for ANTSEL_4, ANTSEL_5, ANTSEL_6  */

	regAonVal = mt7622_gpio_get(GPIO_GPIO_MODE8);
	if (usedPins & BIT(ANT_SEL_4)) {
		regAonVal &= ~GPIO95_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO95_MODE_OFFSET) & GPIO95_MODE);
	}
	if (usedPins & BIT(ANT_SEL_5)) {
		regAonVal &= ~GPIO96_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO96_MODE_OFFSET) & GPIO96_MODE);
	}
	if (usedPins & BIT(ANT_SEL_6)) {
		regAonVal &= ~GPIO97_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO97_MODE_OFFSET) & GPIO97_MODE);
	}

	mt7622_gpio_set(GPIO_GPIO_MODE8, regAonVal);

	/* Set pin mux for ANTSEL_12, ANTSEL_13, ANTSEL_14, ANTSEL_15, ANTSEL_26, ANTSEL_27, ANTSEL_28, ANTSEL_29 */

	regAonVal = mt7622_gpio_get(GPIO_GPIO_MODE1);

	if (usedPins & BIT(ANT_SEL_12)) {
		regAonVal &= ~GPIO73_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO73_MODE_OFFSET) & GPIO73_MODE);
	}
	if (usedPins & BIT(ANT_SEL_13)) {
		regAonVal &= ~GPIO74_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO74_MODE_OFFSET) & GPIO74_MODE);
	}
	if (usedPins & BIT(ANT_SEL_14)) {
		regAonVal &= ~GPIO75_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO75_MODE_OFFSET) & GPIO75_MODE);
	}
	if (usedPins & BIT(ANT_SEL_15)) {
		regAonVal &= ~GPIO76_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO76_MODE_OFFSET) & GPIO76_MODE);
	}
	if (usedPins & BIT(ANT_SEL_26)) {
		regAonVal &= ~GPIO18_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO18_MODE_OFFSET) & GPIO18_MODE);
	}
	if (usedPins & BIT(ANT_SEL_27)) {
		regAonVal &= ~GPIO19_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO19_MODE_OFFSET) & GPIO19_MODE);
	}
	if (usedPins & BIT(ANT_SEL_28)) {
		regAonVal &= ~GPIO20_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO20_MODE_OFFSET) & GPIO20_MODE);
	}
	if (usedPins & BIT(ANT_SEL_29)) {
		regAonVal &= ~GPIO21_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO21_MODE_OFFSET) & GPIO21_MODE);
	}

	mt7622_gpio_set(GPIO_GPIO_MODE1, regAonVal);

	/* Set pin mux for ANTSEL_22, ANTSEL_23, ANTSEL_17, ANTSEL_24, ANTSEL_25, ANTSEL_16  */
	regAonVal = mt7622_gpio_get(GPIO_GPIO_MODE2);
	if (usedPins & BIT(ANT_SEL_22)) {
		regAonVal &= ~GPIO34_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO34_MODE_OFFSET) & GPIO34_MODE);
	}
	if (usedPins & BIT(ANT_SEL_23)) {
		regAonVal &= ~GPIO35_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO35_MODE_OFFSET) & GPIO35_MODE);
	}
	if (usedPins & BIT(ANT_SEL_17)) {
		regAonVal &= ~GPIO78_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO78_MODE_OFFSET) & GPIO78_MODE);
	}
	if (usedPins & BIT(ANT_SEL_24)) {
		regAonVal &= ~GPIO36_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO36_MODE_OFFSET) & GPIO36_MODE);
	}
	if (usedPins & BIT(ANT_SEL_25)) {
		regAonVal &= ~GPIO37_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO37_MODE_OFFSET) & GPIO37_MODE);
	}
	if (usedPins & BIT(ANT_SEL_16)) {
		regAonVal &= ~GPIO77_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO77_MODE_OFFSET) & GPIO77_MODE);
	}
	mt7622_gpio_set(GPIO_GPIO_MODE2, regAonVal);

	/* Set pin mux for ANTSEL_18, ANTSEL_19, ANTSEL_20, ANTSEL_21 */

	regAonVal = mt7622_gpio_get(GPIO_GPIO_MODE10);
	if (usedPins & BIT(ANT_SEL_18)) {
		regAonVal &= ~GPIO79_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO79_MODE_OFFSET) & GPIO79_MODE);
	}
	if (usedPins & BIT(ANT_SEL_19)) {
		regAonVal &= ~GPIO80_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO80_MODE_OFFSET) & GPIO80_MODE);
	}
	if (usedPins & BIT(ANT_SEL_20)) {
	regAonVal &= ~GPIO81_MODE;
	regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO81_MODE_OFFSET) & GPIO81_MODE);
	}
	if (usedPins & BIT(ANT_SEL_21)) {
		regAonVal &= ~GPIO82_MODE;
		regAonVal |= ((PINMUX_FUCTION_ANTSEL << GPIO82_MODE_OFFSET) & GPIO82_MODE);
	}

	mt7622_gpio_set(GPIO_GPIO_MODE10, regAonVal);
}



UINT32 mt7622_antsel_get_used_pin(RTMP_ADAPTER *pAd, UCHAR antCtrl, UCHAR polCtrl, UCHAR wifiStreams)
{
	UCHAR  G_bandPaLna = GetGbandPaLna(antCtrl);
	UINT32 usedPins    = 0;
	UCHAR  sharePin    = GetSharePinSupport(polCtrl);

	if (G_bandPaLna)  /* ePA or eLNA Enable Case */ {

		switch (wifiStreams) {
		case CONFIG_4x4:
			/* Check 2.4G LNA. */
			if (G_bandPaLna == CONFIG_EPA_ELNA)    /* ePAeLNA */
				usedPins |= USED_PIN_EPA_ELNA_4x4;
			else if (G_bandPaLna == CONFIG_EPA_ILNA) /* ePAiLNA */
				usedPins |= USED_PIN_EPA_ILNA_4x4;
			else if (G_bandPaLna == CONFIG_IPA_ELNA) /* iPAeLNA */
				usedPins |= USED_PIN_IPA_ELNA_4x4;

			if (!sharePin)
				usedPins |= (USED_PIN_WF3_TRSW_P | USED_PIN_WF3_TRSW_N);


		case CONFIG_3x3:
			/* Check 2.4G LNA. */
			if (G_bandPaLna == CONFIG_EPA_ELNA)    /* ePAeLNA */
				usedPins |= USED_PIN_EPA_ELNA_3x3;
			else if (G_bandPaLna == CONFIG_EPA_ILNA) /* ePAiLNA */
				usedPins |= USED_PIN_EPA_ILNA_3x3;
			else if (G_bandPaLna == CONFIG_IPA_ELNA) /* iPAeLNA */
				usedPins |= USED_PIN_IPA_ELNA_3x3;

			if (!sharePin)
				usedPins |= (USED_PIN_WF2_TRSW_P | USED_PIN_WF2_TRSW_N);


		case CONFIG_2x2:
			/* Check 2.4G LNA. */
			if (G_bandPaLna == CONFIG_EPA_ELNA)    /* ePAeLNA */
				usedPins |= USED_PIN_EPA_ELNA_2x2;
			else if (G_bandPaLna == CONFIG_EPA_ILNA) /* ePAiLNA */
				usedPins |= USED_PIN_EPA_ILNA_2x2;
			else if (G_bandPaLna == CONFIG_IPA_ELNA) /* iPAeLNA */
				usedPins |= USED_PIN_IPA_ELNA_2x2;

			if (!sharePin)
				usedPins |= (USED_PIN_WF1_TRSW_P | USED_PIN_WF1_TRSW_N);

		case CONFIG_1x1:
			/* Check 2.4G LNA. */
			if (G_bandPaLna == CONFIG_EPA_ELNA)    /* ePAeLNA */
				usedPins |= USED_PIN_EPA_ELNA_1x1;
			else if (G_bandPaLna == CONFIG_EPA_ILNA) /* ePAiLNA */
				usedPins |= USED_PIN_EPA_ILNA_1x1;
			else if (G_bandPaLna == CONFIG_IPA_ELNA) /* iPAeLNA */
				usedPins |= USED_PIN_IPA_ELNA_1x1;

			usedPins |= (USED_PIN_WF0_TRSW_P | USED_PIN_WF0_TRSW_N);

		default:
		break;
		}
	}
	return usedPins;
}

UINT32 mt7622_gpio_get(UINT32 offset)
{
	UINT32 gpioVal;
	ULONG gpioVirt;

	gpioVirt = (ULONG)ioremap(GPIO_BASE_ADDR + offset, 0x20);
	gpioVal = (UINT32)ioread32((void *)gpioVirt);
	iounmap((void *)gpioVirt);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("read gpioVal = %x\n", gpioVal));
	return gpioVal;
}

void mt7622_gpio_set(UINT32 offset, UINT32 gpioVal)
{
	ULONG gpioVirt;

	gpioVirt = (ULONG)ioremap(GPIO_BASE_ADDR + offset, 0x20);
	iowrite32(gpioVal, (void *) gpioVirt);
	iounmap((void *) gpioVirt);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Write gpioVal = %x\n", gpioVal));

}
#endif

#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
#else
INT Mt7622AsicArchOpsInit(RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;
	arch_ops->archGetCrcErrCnt = MtAsicGetCrcErrCnt;
	arch_ops->archGetCCACnt = MtAsicGetCCACnt;
	arch_ops->archGetChBusyCnt = MtAsicGetChBusyCnt;
	arch_ops->archSetAutoFallBack = MtAsicSetAutoFallBack;
	arch_ops->archAutoFallbackInit = MtAsicAutoFallbackInit;
	arch_ops->archUpdateProtect = MtAsicUpdateProtectByFw;
	arch_ops->archUpdateRtsThld = MtAsicUpdateRtsThldByFw;
	arch_ops->archSwitchChannel = MtAsicSwitchChannel;
	arch_ops->archSetRDG = NULL;
	arch_ops->archResetBBPAgent = MtAsicResetBBPAgent;
	arch_ops->archSetDevMac = MtAsicSetDevMacByFw;
	arch_ops->archSetBssid = MtAsicSetBssidByFw;
	arch_ops->archSetStaRec = MtAsicSetStaRecByFw;
	arch_ops->archUpdateStaRecBa = MtAsicUpdateStaRecBaByFw;
	arch_ops->asic_rts_on_off = NULL; /* mt_asic_rts_on_off; */
#ifdef CONFIG_AP_SUPPORT
	arch_ops->archSetMbssMode = MtAsicSetMbssMode;
	arch_ops->archSetMbssWdevIfAddr = MtAsicSetMbssWdevIfAddrGen2;
	arch_ops->archSetMbssHwCRSetting = MtDmacSetMbssHwCRSetting;
	arch_ops->archSetExtTTTTHwCRSetting = MtDmacSetExtTTTTHwCRSetting;
	arch_ops->archSetExtMbssEnableCR = MtDmacSetExtMbssEnableCR;
#endif /* CONFIG_AP_SUPPORT */
	arch_ops->archDelWcidTab = MtAsicDelWcidTabByFw;
#ifdef HTC_DECRYPT_IOT
	arch_ops->archSetWcidAAD_OM = MtAsicSetWcidAAD_OMByFw;
#endif /* HTC_DECRYPT_IOT */
#ifdef MBSS_AS_WDS_AP_SUPPORT
	arch_ops->archSetWcid4Addr_HdrTrans = MtAsicSetWcid4Addr_HdrTransByFw;
#endif

	arch_ops->archAddRemoveKeyTab = MtAsicAddRemoveKeyTabByFw;
#ifdef BCN_OFFLOAD_SUPPORT
	/* sync with Carter, wilsonl */
	arch_ops->archEnableBeacon = NULL;
	arch_ops->archDisableBeacon = NULL;
	arch_ops->archUpdateBeacon = MtUpdateBcnAndTimToMcu;
#else
	arch_ops->archEnableBeacon = MtDmacAsicEnableBeacon;
	arch_ops->archDisableBeacon = MtDmacAsicDisableBeacon;
	arch_ops->archUpdateBeacon = MtUpdateBeaconToAsic;
#endif
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	arch_ops->archSetReptFuncEnable = MtAsicSetReptFuncEnableByFw;
	arch_ops->archInsertRepeaterEntry = MtAsicInsertRepeaterEntryByFw;
	arch_ops->archRemoveRepeaterEntry = MtAsicRemoveRepeaterEntryByFw;
	arch_ops->archInsertRepeaterRootEntry = MtAsicInsertRepeaterRootEntryByFw;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
	arch_ops->archSetPiggyBack = MtAsicSetPiggyBack;
	arch_ops->archSetPreTbtt = NULL;/* offload to BssInfoUpdateByFw */
	arch_ops->archSetGPTimer = MtAsicSetGPTimer;
	arch_ops->archSetChBusyStat = MtAsicSetChBusyStat;
	arch_ops->archGetTsfTime = MtAsicGetTsfTimeByFirmware;
	arch_ops->archDisableSync = NULL;/* MtAsicDisableSyncByDriver; */
	arch_ops->archSetSyncModeAndEnable = NULL;/* MtAsicEnableBssSyncByDriver; */
	arch_ops->archSetWmmParam = MtAsicSetWmmParam;
	arch_ops->archGetWmmParam = MtAsicGetWmmParam;
	arch_ops->archSetEdcaParm = MtAsicSetEdcaParm;
	arch_ops->archSetRetryLimit = MtAsicSetRetryLimit;
	arch_ops->archGetRetryLimit = MtAsicGetRetryLimit;
	arch_ops->archSetSlotTime = MtAsicSetSlotTime;
	arch_ops->archGetTxTsc = MtAsicGetTxTscByDriver;
	arch_ops->archAddSharedKeyEntry = MtAsicAddSharedKeyEntry;
	arch_ops->archRemoveSharedKeyEntry = MtAsicRemoveSharedKeyEntry;
	arch_ops->archAddPairwiseKeyEntry = MtAsicAddPairwiseKeyEntry;
	arch_ops->archSetBW = MtAsicSetBW;
	arch_ops->archSetCtrlCh = mt_mac_set_ctrlch;
	arch_ops->archWaitMacTxRxIdle = MtAsicWaitMacTxRxIdle;
#ifdef MAC_INIT_OFFLOAD
	arch_ops->archSetMacTxRx = MtAsicSetMacTxRxByFw;
	arch_ops->archSetRxvFilter = MtAsicSetRxvFilter;
	arch_ops->archSetMacMaxLen = NULL;
	arch_ops->archSetTxStream = NULL;
	arch_ops->archSetRxFilter = NULL;/* MtAsicSetRxFilter; */
#else
	arch_ops->archSetMacTxRx = MtAsicSetMacTxRx;
	arch_ops->archSetMacMaxLen = MtAsicSetMacMaxLen;
	arch_ops->archSetTxStream = MtAsicSetTxStream;
	arch_ops->archSetRxFilter = MtAsicSetRxFilter;
#endif /*MAC_INIT_OFFLOAD*/
	arch_ops->archSetMacWD = MtAsicSetMacWD;
#ifdef MAC_APCLI_SUPPORT
	arch_ops->archSetApCliBssid = MtAsicSetApCliBssid;
#endif /* MAC_APCLI_SUPPORT */
	arch_ops->archTOPInit = MtAsicTOPInit;
	arch_ops->archSetTmrCR = MtSetTmrCRByFw;
	arch_ops->archUpdateRxWCIDTable = MtAsicUpdateRxWCIDTableByFw;
#ifdef TXBF_SUPPORT
	arch_ops->archUpdateClientBfCap = mt_AsicClientBfCap;
#endif /* TXBF_SUPPORT */
	arch_ops->archUpdateBASession = MtAsicUpdateBASessionByFw;
	arch_ops->archGetTidSn = MtAsicGetTidSnByDriver;
	arch_ops->archSetSMPS = MtAsicSetSMPSByDriver;
	arch_ops->archRxHeaderTransCtl = MtAsicRxHeaderTransCtl;
	arch_ops->archRxHeaderTaranBLCtl = MtAsicRxHeaderTaranBLCtl;
	arch_ops->rx_pkt_process = mt_rx_pkt_process;
	arch_ops->archSetRxStream = NULL;/* MtAsicSetRxStream; */
#ifdef IGMP_SNOOP_SUPPORT
	arch_ops->archMcastEntryInsert = MulticastFilterTableInsertEntry;
	arch_ops->archMcastEntryDelete = MulticastFilterTableDeleteEntry;
#ifdef IGMP_TVM_SUPPORT
	arch_ops->archMcastConfigAgeout = MulticastFilterConfigAgeOut;
	arch_ops->archMcastGetMcastTable = MulticastFilterGetMcastTable;
#endif /* IGMP_TVM_SUPPORT */
#endif
	arch_ops->write_txp_info = mtd_write_txp_info_by_host;
	arch_ops->write_tmac_info_fixed_rate = mtd_write_tmac_info_fixed_rate;
	arch_ops->write_tmac_info = mtd_write_tmac_info_by_host;
	arch_ops->write_tx_resource = mtd_pci_write_tx_resource;
	arch_ops->write_frag_tx_resource = mt_pci_write_frag_tx_resource;
	arch_ops->kickout_data_tx = pci_kickout_data_tx;
	arch_ops->get_pkt_from_rx_resource = mtd_pci_get_pkt_from_rx_resource;
	arch_ops->get_pkt_from_rx1_resource = mtd_pci_get_pkt_from_rx_resource;
	arch_ops->get_resource_idx = mtd_pci_get_resource_idx;
	arch_ops->get_tx_resource_free_num = pci_get_tx_resource_free_num;
	arch_ops->check_hw_resource = mtd_check_hw_resource;
	arch_ops->inc_resource_full_cnt = pci_inc_resource_full_cnt;
	arch_ops->get_resource_state = pci_get_resource_state;
	arch_ops->set_resource_state = pci_set_resource_state;
	arch_ops->check_resource_state = pci_check_resource_state;
	arch_ops->get_hif_buf = mt_pci_get_hif_buf;
	arch_ops->hw_tx = mtd_hw_tx;
	arch_ops->mlme_hw_tx = mtd_mlme_hw_tx;
#ifdef CONFIG_ATE
	arch_ops->ate_hw_tx = mtd_ate_hw_tx;
#endif
	arch_ops->rx_done_handle = mtd_rx_done_handle;
	arch_ops->tx_dma_done_handle = mtd_tx_dma_done_handle;
	arch_ops->cmd_dma_done_handle = mt_cmd_dma_done_handle;
	arch_ops->fwdl_dma_done_handle = mt_fwdl_dma_done_handle;
	arch_ops->hw_isr = mtd_isr;
#ifdef RED_SUPPORT
	arch_ops->archRedMarkPktDrop = RedMarkPktDrop;
	arch_ops->archRedRecordCP = RedRecordCP;
	arch_ops->archRedEnqueueFail = RedEnqueueFail;
#endif
	return TRUE;
}
#endif


