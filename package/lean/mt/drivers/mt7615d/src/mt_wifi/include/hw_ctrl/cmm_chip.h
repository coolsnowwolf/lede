/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_chip.h

	Abstract:
	Ralink Wireless Chip HW related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __CMM_CHIP_H__
#define __CMM_CHIP_H__

struct _EXT_CMD_EFUSE_BUFFER_MODE_T;
struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _CIPHER_KEY;
struct _MT_TX_COUNTER;
struct _EDCA_PARM;
struct _RTMP_REG_PAIR;
struct _BANK_RF_REG_PAIR;
struct _R_M_W_REG;
struct _RF_R_M_W_REG;
struct _CR_REG;
struct _REG_PAIR;
struct MT_TX_PWR_CAP;
struct _EXT_CMD_CHAN_SWITCH_T;
struct wifi_dev;
struct hdev_ctrl;
struct _RX_BLK;

#ifdef COMPOS_WIN
/* TODO: Star-usw, fix it */
typedef char RTMP_STRING;
#endif /* COMPOS_WIN */

enum ASIC_CAP {
	fASIC_CAP_RX_SC = (1 << 0),
	fASIC_CAP_CSO = (1 << 1),
	fASIC_CAP_TSO = (1 << 2),
	fASIC_CAP_MCS_LUT = (1 << 3),
	fASIC_CAP_PMF_ENC = (1 << 4),
	fASIC_CAP_DBDC = (1 << 5),
	fASIC_CAP_TX_HDR_TRANS = (1 << 6),
	fASIC_CAP_RX_HDR_TRANS = (1 << 7),
	fASIC_CAP_HW_DAMSDU = (1 << 8),
	fASIC_CAP_RX_DMA_SCATTER =  (1 << 9),
	fASIC_CAP_MCU_OFFLOAD =  (1 << 10),
	fASIC_CAP_CT = (1 << 11),
	fASIC_CAP_HW_TX_AMSDU = (1 << 12),
	fASIC_CAP_WHNAT = (1 << 13),
	fASIC_CAP_RDG = (1 << 14),
	fASIC_CAP_RX_DLY = (1 << 15),
	fASIC_CAP_WMM_PKTDETECT_OFFLOAD = (1 << 16),
	fASIC_CAP_PCIE_ASPM_DYM_CTRL = (1 << 17),
	fASIC_CAP_ADV_SECURITY = (1 << 18),
};

enum PHY_CAP {
	fPHY_CAP_24G = 0x1,
	fPHY_CAP_5G = 0x2,
	fPHY_CAP_HT = 0x10,
	fPHY_CAP_VHT = 0x20,
	fPHY_CAP_HE = 0x40, /* it might be different from the definition in trunk.
			     * need to follow trunk's definition once merge back.
			     */
	fPHY_CAP_TXBF = 0x100,
	fPHY_CAP_LDPC = 0x200,
	fPHY_CAP_MUMIMO = 0x400,
	fPHY_CAP_BW40 = 0x1000,
	fPHY_CAP_BW80 = 0x2000,
	fPHY_CAP_BW160NC = 0x4000,
	fPHY_CAP_BW160C = 0x8000,
};

enum HIF_TYPE {
	HIF_RTMP = 0x0,
	HIF_RLT = 0x1,
	HIF_MT = 0x2,
	HIF_MAX = HIF_MT,
};

enum MAC_TYPE {
	MAC_RTMP = 0x0,
	MAC_MT = 0x1,
};

enum RF_TYPE {
	RF_RT,
	RF_RLT,
	RF_MT76x2,
	RF_MT,
};

enum BBP_TYPE {
	BBP_RTMP = 0x0,
	BBP_RLT = 0x1,
	BBP_MT = 0x2,
};

#define PHY_CAP_2G(_x)		(((_x) & fPHY_CAP_24G) == fPHY_CAP_24G)
#define PHY_CAP_5G(_x)		(((_x) & fPHY_CAP_5G) == fPHY_CAP_5G)
#define PHY_CAP_N(_x)		(((_x) & fPHY_CAP_HT) == fPHY_CAP_HT)
#define PHY_CAP_AC(_x)		(((_x) & fPHY_CAP_VHT) == fPHY_CAP_VHT)
#define GET_NUM_OF_TX_RING(_chipCap)	((_chipCap)->num_of_tx_ring)
#define GET_NUM_OF_RX_RING(_chipCap)	((_chipCap)->num_of_rx_ring)
#define GET_TX_RING_SIZE(_chipCap)    ((_chipCap)->tx_ring_size)
#define GET_RX0_RING_SIZE(_chipCap)    ((_chipCap)->rx0_ring_size)
#define GET_RX1_RING_SIZE(_chipCap)    ((_chipCap)->rx1_ring_size)

enum EFUSE_TYPE {
	EFUSE_MT,
	EFUSE_MAX,
};

enum TXD_TYPE {
	TXD_V1,
	TXD_V2,
};

#define MBSSID_MODE0	0
#define MBSSID_MODE1	1	/* Enhance NEW MBSSID MODE mapping to mode 0 */
#ifdef ENHANCE_NEW_MBSSID_MODE
#define MBSSID_MODE2	2	/* Enhance NEW MBSSID MODE mapping to mode 1 */
#define MBSSID_MODE3	3	/* Enhance NEW MBSSID MODE mapping to mode 2 */
#define MBSSID_MODE4	4	/* Enhance NEW MBSSID MODE mapping to mode 3 */
#define MBSSID_MODE5	5	/* Enhance NEW MBSSID MODE mapping to mode 4 */
#define MBSSID_MODE6	6	/* Enhance NEW MBSSID MODE mapping to mode 5 */
#endif /* ENHANCE_NEW_MBSSID_MODE */

enum APPS_MODE {
	APPS_MODE0 = 0x0,	/* MT7603, host handle APPS */
	APPS_MODE1 = 0x1,	/* MT7637 */
	APPS_MODE2 = 0x2,	/* MT7615, FW handle APPS */
	APPS_MODEMAX = 0x3,
};

#ifdef MT_MAC
/*
    these functions is common setting and could be used by sMAC or dMAC.
    move to here to common use.
*/
#ifdef CONFIG_AP_SUPPORT
VOID MtAsicSetMbssWdevIfAddrGen1(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode);
VOID MtAsicSetMbssWdevIfAddrGen2(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode);
#endif /*CONFIG_AP_SUPPROT */
#endif /*MT_MAC*/

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
VOID mt_asic_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

typedef struct _RTMP_CHIP_OP {
	int (*sys_onoff)(struct _RTMP_ADAPTER *pAd, BOOLEAN on, BOOLEAN reser);

	/*  Calibration access related callback functions */
	int (*eeinit)(struct _RTMP_ADAPTER *pAd);
	BOOLEAN (*eeread)(struct _RTMP_ADAPTER *pAd, UINT16 offset, UINT16 *pValue);
	int (*eewrite)(struct _RTMP_ADAPTER *pAd, USHORT offset, USHORT value);
	BOOLEAN (*eeread_range)(struct _RTMP_ADAPTER *pAd, UINT16 start, UINT16 length, UCHAR *pbuf);
	int (*eewrite_range)(struct _RTMP_ADAPTER *pAd, USHORT start, USHORT length, UCHAR *pbuf);

	/* ITxBf calibration */
	int (*fITxBfDividerCalibration)(struct _RTMP_ADAPTER *pAd, int calFunction, int calMethod, UCHAR *divPhase);
	void (*fITxBfLNAPhaseCompensate)(struct _RTMP_ADAPTER *pAd);
	int (*fITxBfCal)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	int (*fITxBfLNACalibration)(struct _RTMP_ADAPTER *pAd, int calFunction, int calMethod, BOOLEAN gBand);

	void (*AsicRfInit)(struct _RTMP_ADAPTER *pAd);
	void (*AsicBbpInit)(struct _RTMP_ADAPTER *pAd);
	void (*AsicMacInit)(struct _RTMP_ADAPTER *pAd);
	void (*AsicReverseRfFromSleepMode)(struct _RTMP_ADAPTER *pAd, BOOLEAN FlgIsInitState);
	void (*AsicHaltAction)(struct _RTMP_ADAPTER *pAd);

	/* Power save */
#ifdef GREENAP_SUPPORT
	VOID (*EnableAPMIMOPS)(struct _RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off);
	VOID (*DisableAPMIMOPS)(struct _RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off);
#endif /* GREENAP_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	VOID (*pcie_aspm_dym_ctrl)(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
	/* MAC */
	VOID (*BeaconUpdate)(struct _RTMP_ADAPTER *pAd, USHORT Offset, UINT32 Value, UINT8 Unit);

	/* BBP adjust */
	VOID (*ChipBBPAdjust)(IN struct _RTMP_ADAPTER *pAd, UCHAR Channel);

	/* AGC */
	VOID (*BbpInitFromEEPROM)(struct _RTMP_ADAPTER *pAd);
	VOID (*ChipSwitchChannel)(struct _RTMP_ADAPTER *pAd, struct _MT_SWITCH_CHANNEL_CFG SwChCfg);

#ifdef NEW_SET_RX_STREAM
	INT (*ChipSetRxStream)(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx);
#endif

	VOID (*AsicTxAlcGetAutoAgcOffset)(
		IN struct _RTMP_ADAPTER	*pAd,
		IN PCHAR				pDeltaPwr,
		IN PCHAR				pTotalDeltaPwr,
		IN PCHAR				pAgcCompensate,
		IN PCHAR				pDeltaPowerByBbpR1,
		IN UCHAR				Channel);

	VOID (*AsicGetTxPowerOffset)(struct _RTMP_ADAPTER *pAd, ULONG *TxPwr);
	VOID (*AsicExtraPowerOverMAC)(struct _RTMP_ADAPTER *pAd);

	VOID (*AsicAdjustTxPower)(struct _RTMP_ADAPTER *pAd);

	/* Antenna */
	VOID (*AsicAntennaDefaultReset)(struct _RTMP_ADAPTER *pAd, union _EEPROM_ANTENNA_STRUC *pAntenna);
	VOID (*SetRxAnt)(struct _RTMP_ADAPTER *pAd, UCHAR Ant);

	/* EEPROM */
	VOID (*NICInitAsicFromEEPROM)(IN struct _RTMP_ADAPTER *pAd);

	/* Temperature Compensation */
	VOID (*InitTemperCompensation)(IN struct _RTMP_ADAPTER *pAd);
	VOID (*TemperCompensation)(IN struct _RTMP_ADAPTER *pAd);

	/* high power tuning */
	VOID (*HighPowerTuning)(struct _RTMP_ADAPTER *pAd, struct _RSSI_SAMPLE *pRssi);

	/* Others */
	VOID (*NetDevNickNameInit)(IN struct _RTMP_ADAPTER *pAd);
#ifdef CAL_FREE_IC_SUPPORT
	BOOLEAN (*is_cal_free_ic)(IN struct _RTMP_ADAPTER *pAd);
	VOID (*cal_free_data_get)(IN struct _RTMP_ADAPTER *pAd);
	BOOLEAN (*check_is_cal_free_merge)(IN struct _RTMP_ADAPTER *pAd);
#endif /* CAL_FREE_IC_SUPPORT */

#ifdef RF_LOCKDOWN
	BOOLEAN (*check_RF_lock_down)(IN struct _RTMP_ADAPTER *pAd);
	BOOLEAN (*write_RF_lock_parameter)(IN struct _RTMP_ADAPTER *pAd, IN USHORT offset);
	BOOLEAN (*merge_RF_lock_parameter)(IN struct _RTMP_ADAPTER *pAd);
	UCHAR   (*Read_Effuse_parameter)(IN struct _RTMP_ADAPTER *pAd, IN USHORT offset);
	BOOLEAN (*Config_Effuse_Country)(IN struct _RTMP_ADAPTER *pAd);
#endif /* RF_LOCKDOWN */

	/* The chip specific function list */
	VOID (*AsicResetBbpAgent)(IN struct _RTMP_ADAPTER *pAd);

#ifdef CARRIER_DETECTION_SUPPORT
	VOID (*ToneRadarProgram)(struct _RTMP_ADAPTER *pAd, ULONG  threshold);
#endif /* CARRIER_DETECTION_SUPPORT */
	VOID (*CckMrcStatusCtrl)(struct _RTMP_ADAPTER *pAd);
	VOID (*RadarGLRTCompensate)(struct _RTMP_ADAPTER *pAd);
	VOID (*SecondCCADetection)(struct _RTMP_ADAPTER *pAd);

	/* MCU */
	void (*MCUCtrlInit)(struct _RTMP_ADAPTER *ad);
	void (*MCUCtrlExit)(struct _RTMP_ADAPTER *ad);
	VOID (*FwInit)(struct _RTMP_ADAPTER *pAd);
	VOID (*FwExit)(struct _RTMP_ADAPTER *pAd);
	VOID (*fwdl_datapath_setup)(struct _RTMP_ADAPTER *pAd, BOOLEAN init);
	int (*DisableTxRx)(struct _RTMP_ADAPTER *ad, UCHAR Level);
	void (*AsicRadioOn)(struct _RTMP_ADAPTER *ad, UCHAR Stage);
	void (*AsicRadioOff)(struct _RTMP_ADAPTER *ad, UINT8 Stage);
#ifdef CONFIG_ANDES_SUPPORT
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	int (*pci_kick_out_cmd_msg)(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);
#endif

	void (*andes_fill_cmd_header)(struct cmd_msg *msg, VOID *net_pkt);
#endif /* CONFIG_ANDES_SUPPORT */
	void (*rx_event_handler)(struct _RTMP_ADAPTER *ad, UCHAR *data);

#ifdef MICROWAVE_OVEN_SUPPORT
	VOID (*AsicMeasureFalseCCA)(IN struct _RTMP_ADAPTER *pAd);

	VOID (*AsicMitigateMicrowave)(IN struct _RTMP_ADAPTER *pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	VOID (*AsicWOWEnable)(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg);
	VOID (*AsicWOWDisable)(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg);
	VOID (*AsicWOWInit)(struct _RTMP_ADAPTER *ad);
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

	void (*usb_cfg_read)(struct _RTMP_ADAPTER *ad, UINT32 *value);
	void (*usb_cfg_write)(struct _RTMP_ADAPTER *ad, UINT32 value);
	void (*show_pwr_info)(struct _RTMP_ADAPTER *ad);
	void (*cal_test)(struct _RTMP_ADAPTER *ad, UINT32 type);
	void (*bufferModeEfuseFill)(struct _RTMP_ADAPTER *ad, struct _EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd);
	void (*keep_efuse_field_only)(struct _RTMP_ADAPTER *ad, UCHAR *buffer);
	INT32 (*MtCmdTx)(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
	void (*prepare_fwdl_img)(struct _RTMP_ADAPTER *pAd);
#ifdef DBDC_MODE
	UCHAR (*BandGetByIdx)(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
#endif

#ifdef TXBF_SUPPORT
	VOID (*TxBFInit)(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *pEntry, struct _IE_lists *ie_list, BOOLEAN supportsETxBF);
	BOOLEAN (*ClientSupportsVhtETxBF)(struct _RTMP_ADAPTER *ad, struct _VHT_CAP_INFO *pTxBFCap);
	BOOLEAN (*ClientSupportsETxBF)(struct _RTMP_ADAPTER *ad, struct _HT_BF_CAP *pTxBFCap);
	VOID (*setETxBFCap)(struct _RTMP_ADAPTER *ad, struct _TXBF_STATUS_INFO  *pTxBfInfo);
	VOID (*setVHTETxBFCap)(struct _RTMP_ADAPTER *ad, struct _TXBF_STATUS_INFO  *pTxBfInfo);
#ifdef MT_MAC
	INT32 (*BfStaRecUpdate)(struct _RTMP_ADAPTER *ad, UCHAR ucPhyMode, UCHAR ucBssIdx, UINT8 ucWlanIdx);
	INT32 (*BfStaRecRelease)(struct _RTMP_ADAPTER *ad, UCHAR ucBssIdx, UINT8 ucWlanIdx);
	INT32 (*BfPfmuMemAlloc)(struct _RTMP_ADAPTER *ad, UCHAR ucSu_Mu, UCHAR ucWlanId);
	INT32 (*BfPfmuMemRelease)(struct _RTMP_ADAPTER *ad, UCHAR ucWlanId);
	INT32 (*BfHwEnStatusUpdate)(struct _RTMP_ADAPTER *ad, BOOLEAN fgETxBf, BOOLEAN fgITxBf);
	INT32 (*TxBfTxApplyCtrl)(struct _RTMP_ADAPTER *ad, UCHAR ucWlanId, BOOLEAN fgETxBf, BOOLEAN fgITxBf, BOOLEAN fgMuTxBf, BOOLEAN fgPhaseCali);
	INT32 (*archSetAid)(struct _RTMP_ADAPTER *ad, UINT16 Aid);
	INT32 (*BfApClientCluster)(struct _RTMP_ADAPTER *ad, UCHAR ucWlanId, UCHAR ucCmmWlanId);
	INT32 (*BfReptClonedStaToNormalSta)(struct _RTMP_ADAPTER *ad, UCHAR ucWlanId, UCHAR ucCliIdx);
	INT32 (*BfeeHwCtrl)(struct _RTMP_ADAPTER *pAd, BOOLEAN fgBfeeEn);
	INT32 (*BfModuleEnCtrl)(struct _RTMP_ADAPTER *ad, UINT8 u1BfNum, UINT8 u1BfBitmap, UINT8 u1BfSelBand[]);
	VOID (*iBFPhaseCalInit)(struct _RTMP_ADAPTER *ad);
	VOID (*iBFPhaseFreeMem)(struct _RTMP_ADAPTER *ad);
	VOID (*iBFPhaseCalE2PUpdate)(struct _RTMP_ADAPTER *ad, UCHAR ucGroup, BOOLEAN fgSX2, UCHAR ucUpdateAllTye);
	VOID (*iBFPhaseCalReport)(struct _RTMP_ADAPTER *ad, UCHAR ucGroupL_M_H, UCHAR ucGroup, BOOLEAN fgSX2, UCHAR ucStatus, UCHAR ucPhaseCalType, PUCHAR pBuf);
	VOID (*iBFPhaseComp)(struct _RTMP_ADAPTER *ad, UCHAR ucGroup, PCHAR pCmdBuf);
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
#ifdef SMART_CARRIER_SENSE_SUPPORT
	VOID (*SmartCarrierSense)(struct _RTMP_ADAPTER *pAd);
	VOID (*ChipSetSCS)(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx, UINT32 value);
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
	INT32 (*ICapStart)(struct _RTMP_ADAPTER *pAd, UINT8 *pData);
	INT32 (*ICapStatus)(struct _RTMP_ADAPTER *pAd);
	INT32 (*ICapCmdRawDataProc)(struct _RTMP_ADAPTER *pAd);
	INT32 (*ICapGetIQData)(struct _RTMP_ADAPTER *pAd, PINT32 pData, PINT32 pDataLen, UINT32 IQ_Type, UINT32 WF_Num);
	VOID  (*ICapEventRawDataHandler)(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
#endif /* INTERNAL_CAPTURE_SUPPORT */

#ifdef WIFI_SPECTRUM_SUPPORT
	INT32 (*SpectrumStart)(struct _RTMP_ADAPTER *pAd, UINT8 *pData);
	INT32 (*SpectrumStatus)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SpectrumCmdRawDataProc)(struct _RTMP_ADAPTER *pAd);
	VOID  (*SpectrumEventRawDataHandler)(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
#endif /* WIFI_SPECTRUM_SUPPORT */

	INT32 (*hif_io_read32)(void *cookie, UINT32 addr, UINT32 *value);
	INT32 (*hif_io_write32)(void *cookie, UINT32 addr, UINT32 value);
	VOID (*heart_beat_check)(struct _RTMP_ADAPTER *ad);
	INT32 (*dma_shdl_init)(struct _RTMP_ADAPTER *pAd);
	VOID (*irq_init)(struct _RTMP_ADAPTER *ad);
	INT (*chk_hif_default_cr_setting)(struct _RTMP_ADAPTER *pAd);
	INT (*chk_top_default_cr_setting)(struct _RTMP_ADAPTER *pAd);
#ifdef HOST_RESUME_DONE_ACK_SUPPORT
	void (*HostResumeDoneAck)(struct _RTMP_ADAPTER *pAd);
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
	VOID (*hif_set_pcie_read_params)(struct _RTMP_ADAPTER *pAd);
	INT (*hif_set_dma)(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable, UINT8 WPDMABurstSIZE);
	BOOLEAN (*hif_wait_dma_idle)(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us);
	BOOLEAN (*hif_reset_dma)(struct _RTMP_ADAPTER *pAd);
	INT32 (*get_fw_sync_value)(struct _RTMP_ADAPTER *pAd);
	INT32 (*read_chl_pwr)(struct _RTMP_ADAPTER *pAd);
	VOID (*parse_RXV_packet)(struct _RTMP_ADAPTER *pAd, UINT32 Type, struct _RX_BLK *RxBlk, UCHAR *Data);
	INT32 (*txs_handler)(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, VOID *rx_packet);
	INT32 (*driver_own)(struct _RTMP_ADAPTER *pAd);
	VOID (*fw_own)(struct _RTMP_ADAPTER *pAd);
	INT32 (*tssi_set) (struct _RTMP_ADAPTER *ad, UCHAR *efuse);
	INT32 (*pa_lna_set) (struct _RTMP_ADAPTER *ad, UCHAR *efuse);
} RTMP_CHIP_OP;

typedef struct _RTMP_CHIP_DBG {
	INT32 (*dump_ps_table)(struct hdev_ctrl *ctrl, UINT32 ent_type, BOOLEAN bReptCli);
	INT32 (*dump_mib_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_tmac_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_agg_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_dmasch_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_pse_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_pse_data)(struct hdev_ctrl *ctrl, UINT32 StartFID, UINT32 FrameNums);
	INT32 (*show_ple_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_protect_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_cca_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*set_cca_en)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_txv_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	VOID (*show_bcn_info)(struct hdev_ctrl *ctrl, UCHAR bandidx);
} RTMP_CHIP_DBG;

typedef struct _RTMP_CHIP_CAP {
	/* ------------------------ packet --------------------- */
	UINT8 TXWISize;	/* TxWI or LMAC TxD max size */
	UINT8 RXWISize; /* RxWI or LMAC RxD max size */
	UINT8 tx_hw_hdr_len;	/* Tx Hw meta info size which including all hw info fields */
	UINT8 rx_hw_hdr_len;	/* Rx Hw meta info size */
	UINT8 num_of_tx_ring;
	UINT8 num_of_rx_ring;
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	UINT8 max_v2_bcn_num;
#endif
	UINT16 tx_ring_size;
	UINT16 rx0_ring_size;
	UINT16 rx1_ring_size;
	enum ASIC_CAP asic_caps;
	enum PHY_CAP phy_caps;
	enum HIF_TYPE hif_type;
	enum MAC_TYPE mac_type;
	enum BBP_TYPE bbp_type;
	enum MCU_TYPE MCUType;
	enum RF_TYPE rf_type;
	enum EFUSE_TYPE efuse_type;
	enum TXD_TYPE txd_type;

	/* register */
	struct _REG_PAIR *pRFRegTable;
	struct _REG_PAIR *pBBPRegTable;
	UCHAR bbpRegTbSize;

	UINT32 MaxNumOfRfId;
	UINT32 MaxNumOfBbpId;

#define RF_REG_WT_METHOD_NONE			0
#define RF_REG_WT_METHOD_STEP_ON		1
	UCHAR RfReg17WtMethod;

	/* beacon */
	BOOLEAN FlgIsSupSpecBcnBuf;	/* SPECIFIC_BCN_BUF_SUPPORT */
	UINT8 BcnMaxNum;	/* software use */
	UINT8 BcnMaxHwNum;	/* hardware limitation */
	UINT8 WcidHwRsvNum;	/* hardware available WCID number */
	UINT32 WtblHwNum;		/* hardware WTBL number */
	UINT32 WtblPseAddr;		/* */
	UINT16 BcnMaxHwSize;	/* hardware maximum beacon size */
	UINT16 BcnBase[HW_BEACON_MAX_NUM];	/* hardware beacon base address */

	/* function */
	/* use UINT8, not bit-or to speed up driver */
	BOOLEAN FlgIsHwWapiSup;

	/* VCO calibration mode */
	UINT8 VcoPeriod; /* default 10s */
#define VCO_CAL_DISABLE		0	/* not support */
#define VCO_CAL_MODE_1		1	/* toggle RF7[0] */
#define VCO_CAL_MODE_2		2	/* toggle RF3[7] */
#define VCO_CAL_MODE_3		3	/* toggle RF4[7] or RF5[7] */
	UINT8	FlgIsVcoReCalMode;

	BOOLEAN FlgIsHwAntennaDiversitySup;
	BOOLEAN Flg7662ChipCap;
#ifdef STREAM_MODE_SUPPORT
	BOOLEAN FlgHwStreamMode;
#endif /* STREAM_MODE_SUPPORT */
#ifdef TXBF_SUPPORT
	BOOLEAN FlgHwTxBfCap;
	BOOLEAN FlgITxBfBinWrite;
#endif /* TXBF_SUPPORT */

	UCHAR ba_max_cnt;

#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */

	BOOLEAN temp_tx_alc_enable;
	INT32 temp_25_ref; /* a quantification value of temperature, but not J */
	INT32 current_temp; /* unit J */
#ifdef RTMP_TEMPERATURE_TX_ALC
	UINT32 high_temp_slope_a_band; /* slope with unit (J /dB) */
	UINT32 low_temp_slope_a_band; /* slope with unit (J /dB) */
	UINT32 high_temp_slope_g_band; /* slope with unit (J /dB) */
	UINT32 low_temp_slope_g_band; /* slope with unit (J /dB) */
	INT32 tc_upper_bound_a_band; /* unit dB */
	INT32 tc_lower_bound_a_band; /* unit dB */
	INT32 tc_upper_bound_g_band; /* unit dB */
	INT32 tc_lower_bound_g_band; /* unit dB */
#endif /* RTMP_TEMPERATURE_TX_ALC */


#ifdef DYNAMIC_VGA_SUPPORT
	BOOLEAN dynamic_vga_support;
	INT32 compensate_level;
	INT32 avg_rssi_0;
	INT32 avg_rssi_1;
	INT32 avg_rssi_all;
	UCHAR dynamic_chE_mode;
	BOOLEAN dynamic_chE_trigger;
#endif /* DYNAMIC_VGA_SUPPORT */

	/* ---------------------------- signal ---------------------------------- */
#define SNR_FORMULA1		0	/* ((0xeb     - pAd->StaCfg[0].wdev.LastSNR0) * 3) / 16; */
#define SNR_FORMULA2		1	/* (pAd->StaCfg[0].wdev.LastSNR0 * 3 + 8) >> 4; */
#define SNR_FORMULA3		2	/* (pAd->StaCfg[0].wdev.LastSNR0) * 3) / 16; */
#define SNR_FORMULA4		3	/* for MT7603 */
	UINT8 SnrFormula;

	UINT8 max_nss;			/* maximum Nss, 3 for 3883 or 3593 */
	UINT8 max_vht_mcs;		/* Maximum Vht MCS */

#ifdef DOT11_VHT_AC
	UINT8 ac_off_mode;		/* 11AC off mode */
#endif /* DOT11_VHT_AC */

	BOOLEAN bTempCompTxALC;
	BOOLEAN rx_temp_comp;
	BOOLEAN bLimitPowerRange; /* TSSI compensation range limit */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(SINGLE_SKU_V2)
	INT16	PAModeCCK[4];
	INT16	PAModeOFDM[8];
	INT16	PAModeHT[16];
#ifdef DOT11_VHT_AC
	INT16	PAModeVHT[10];
#endif /* DOT11_VHT_AC */
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(SINGLE_SKU_V2) */

	/* ---------------------------- others ---------------------------------- */
#ifdef RTMP_EFUSE_SUPPORT
	UINT16 EFUSE_USAGE_MAP_START;
	UINT16 EFUSE_USAGE_MAP_END;
	UINT8 EFUSE_USAGE_MAP_SIZE;
	UINT8 EFUSE_RESERVED_SIZE;
#endif /* RTMP_EFUSE_SUPPORT */

	UCHAR *EEPROM_DEFAULT_BIN;
	UCHAR *EEPROM_DEFAULT_BIN_FILE;
	UINT16 EEPROM_DEFAULT_BIN_SIZE;
	UINT16 EFUSE_BUFFER_CONTENT_SIZE;

#ifdef RTMP_FLASH_SUPPORT
	BOOLEAN ee_inited;
#endif /* RTMP_FLASH_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	UCHAR carrier_func;
#endif /* CARRIER_DETECTION_SUPPORT */

	/*
		Define the burst size of WPDMA of PCI
		0 : 4 DWORD (16bytes)
		1 : 8 DWORD (32 bytes)
		2 : 16 DWORD (64 bytes)
		3 : 32 DWORD (128 bytes)
	*/
	UINT8 WPDMABurstSIZE;
	/*
	 * Define the Probe Response Times
	 *	1: Multi-Sta Suopport better
	 *	2: Default
	 */
	UINT8 ProbeRspTimes;

	/*
	 * 0: MBSSID_MODE0
	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit2:bit0] of byte5)
	 * 1: MBSSID_MODE1
	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit4:bit2] of byte0)
	 */
	UINT8 MBSSIDMode;

#ifdef DOT11W_PMF_SUPPORT
#define PMF_ENCRYPT_MODE_0      0	/* All packets must software encryption. */
#define PMF_ENCRYPT_MODE_1      1	/* Data packets do hardware encryption, management packet do software encryption. */
#define PMF_ENCRYPT_MODE_2      2	/* Data and management packets do hardware encryption. */
	UINT8	FlgPMFEncrtptMode;
#endif /* DOT11W_PMF_SUPPORT */

#ifdef RT5592EP_SUPPORT
	UINT32 Priv; /* Flag for RT5592 EP */
#endif /* RT5592EP_SUPPORT */

#ifdef CONFIG_ANDES_SUPPORT
	UINT32 need_load_patch;
	UINT32 need_load_fw;
	enum load_patch_flow load_patch_flow;
	enum load_fw_flow load_fw_flow;
	UINT32 patch_format;
	UINT32 fw_format;
	UINT32 load_patch_method;
	UINT32 load_fw_method;
	UINT32 rom_patch_offset;
	UINT32 decompress_tmp_addr;
#endif

	UINT8 cmd_header_len;
	UINT8 cmd_padding_len;

#ifdef SINGLE_SKU_V2
	CHAR	Apwrdelta;
	CHAR	Gpwrdelta;
#endif /* SINGLE_SKU_V2 */


#ifdef CONFIG_SWITCH_CHANNEL_OFFLOAD
	UINT16 ChannelParamsSize;
	UCHAR *ChannelParam;
	INT XtalType;
#endif

	UCHAR *MACRegisterVer;
	UCHAR *BBPRegisterVer;
	UCHAR *RFRegisterVer;

#ifdef CONFIG_WIFI_TEST
	UINT32 MemMapStart;
	UINT32 MemMapEnd;
	UINT32 BBPMemMapOffset;
	UINT16 BBPStart;
	UINT16 BBPEnd;
	UINT16 RFStart;
	UINT16 RFEnd;
#ifdef MT_RF
	UINT8 RFIndexNum;
	struct RF_INDEX_OFFSET *RFIndexOffset;
#endif
	UINT32 MacMemMapOffset;
	UINT32 MacStart;
	UINT32 MacEnd;
	UINT16 E2PStart;
	UINT16 E2PEnd;
	BOOLEAN pbf_loopback;
	BOOLEAN pbf_rx_drop;
#endif /* CONFIG_WIFI_TEST */

	BOOLEAN tssi_enable;
	BOOLEAN ed_cca_enable;

#ifdef MT_MAC
	struct MT_TX_PWR_CAP MTTxPwrCap;
	UCHAR TmrEnable;
	UINT8 OmacNums;
	UINT8 BssNums;
	UINT8 MBSSStartIdx;
	UINT8 MaxRepeaterNum;
	UINT8 ExtMbssOmacStartIdx;
	UINT8 RepeaterStartIdx;
#endif
	BOOLEAN fgBcnOffloadSupport;
	BOOLEAN fgIsNeedPretbttIntEvent;
	UCHAR TmrHwVer;
	UINT8 TxAggLimit;
	UINT8 TxBAWinSize;
	UINT8 RxBAWinSize;
#define MPDU_3895_OCTETS    0x0
#define MPDU_7991_OCTETS    0x1
	UINT8 max_amsdu_len;
	UINT8 ht_max_ampdu_len_exp;
#ifdef DOT11_VHT_AC
#define MPDU_11454_OCTETS   0x2
	UINT8 max_mpdu_len;
	UINT8 vht_max_ampdu_len_exp; /* 2^(13+Exp) - 1 */
#endif /* DOT11_VHT_AC*/

	BOOLEAN g_band_256_qam;

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	BOOLEAN fgRateAdaptFWOffload;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */


	UINT32  TxOPScenario;

	UINT32  CurrentTxOP;
	UINT32  default_txop;
	/*Multi-WMM Control*/
	UCHAR WmmHwNum;
	/* specific PDA Port HW Address */
	UINT16 PDA_PORT;
	BOOLEAN SupportAMSDU;

	UINT8 APPSMode;

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	UINT8   nWakeupInterface;
#endif /* MT_WOW_SUPPORT */

	/* the length of partial payload delivered to MCU for further processing */
	UINT16 CtParseLen;
	UCHAR qm;
#define TASKLET_METHOD 1
#define CMWQ_METHOD 2
#define KTHREAD_METHOD 3
	UCHAR qm_tm;
	UCHAR hif_tm;
#define WMM_DETECT_METHOD1 1
#define WMM_DETECT_METHOD2 2
	UCHAR wmm_detect_method;
	UCHAR hw_ops_ver;
	UINT32 int_enable_mask;
	UINT8 hif_pkt_type[20];

#ifdef INTERNAL_CAPTURE_SUPPORT
	RBIST_DESC_T *pICapDesc;
	UINT8 ICapBankNum;
	UINT32 ICapADCIQCnt;
	UINT32 ICapIQCIQCnt;
	UINT32 ICapBankSmplCnt;
	UINT32 ICapFourWayADC;
	UINT32 ICapFourWayFIIQ;
	UINT32 ICapFourWayFDIQ;
#endif /* INTERNAL_CAPTURE_SUPPORT */
#ifdef WIFI_SPECTRUM_SUPPORT
	RBIST_DESC_T *pSpectrumDesc;
	UINT8 SpectrumBankNum;
	UINT32 SpectrumWF0ADC;
	UINT32 SpectrumWF1ADC;
	UINT32 SpectrumWF2ADC;
	UINT32 SpectrumWF3ADC;
	UINT32 SpectrumWF0FIIQ;
	UINT32 SpectrumWF1FIIQ;
	UINT32 SpectrumWF2FIIQ;
	UINT32 SpectrumWF3FIIQ;
	UINT32 SpectrumWF0FDIQ;
	UINT32 SpectrumWF1FDIQ;
	UINT32 SpectrumWF2FDIQ;
	UINT32 SpectrumWF3FDIQ;
#endif /* WIFI_SPECTRUM_SUPPORT */
	UCHAR band_cnt;
	UCHAR *ba_range;
	BOOLEAN tx_delay_support;
} RTMP_CHIP_CAP;

/*
  *   EEPROM operation related marcos
  */
BOOLEAN chip_eeprom_read16(struct _RTMP_ADAPTER *pAd, USHORT offset, USHORT *value);
BOOLEAN chip_eeprom_read_with_range(struct _RTMP_ADAPTER *pAd, USHORT start, USHORT length, UCHAR *pbuf);


#define RT28xx_EEPROM_READ16(_pAd, _offset, _val)   chip_eeprom_read16(_pAd, _offset, &(_val))

#define RT28xx_EEPROM_WRITE16(_pAd, _offset, _val)		\
	do {\
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
		if (ops->eewrite) \
			ops->eewrite(_pAd, (_offset), (USHORT)(_val));\
	} while (0)

#define RT28xx_EEPROM_READ_WITH_RANGE(_pAd, _start, _length, _pbuf)   chip_eeprom_read_with_range(_pAd, _start, _length, _pbuf)

#define RT28xx_EEPROM_WRITE_WITH_RANGE(_pAd, _start, _length, _pbuf)		\
	do { \
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
		if (ops->eewrite_range) \
			ops->eewrite_range(_pAd, _start, _length, _pbuf);\
	} while (0)

#define RTMP_CHIP_ENABLE_GREENAP(__pAd, __greenap_on_off)	\
			do { \
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->EnableAPMIMOPS != NULL)	\
					ops->EnableAPMIMOPS(__pAd, __greenap_on_off); \
			} while (0)

#define RTMP_CHIP_DISABLE_GREENAP(__pAd, __greenap_on_off)	\
			do { \
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->DisableAPMIMOPS != NULL) \
					ops->DisableAPMIMOPS(__pAd, __greenap_on_off);	\
			} while (0)

#define RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(__pAd, __pCfgOfTxPwrCtrlOverMAC)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->AsicGetTxPowerOffset != NULL)	\
					ops->AsicGetTxPowerOffset(__pAd, __pCfgOfTxPwrCtrlOverMAC);	\
			} while (0)

#define RTMP_CHIP_ASIC_AUTO_AGC_OFFSET_GET(	\
				__pAd, __pDeltaPwr, __pTotalDeltaPwr, __pAgcCompensate, __pDeltaPowerByBbpR1, __Channel)	\
		do {	\
			struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
			if (ops->AsicTxAlcGetAutoAgcOffset != NULL)	\
				ops->AsicTxAlcGetAutoAgcOffset(	\
						__pAd, __pDeltaPwr, __pTotalDeltaPwr, __pAgcCompensate, __pDeltaPowerByBbpR1, __Channel);	\
		} while (0)

#define RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(__pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->AsicExtraPowerOverMAC != NULL)	\
					ops->AsicExtraPowerOverMAC(__pAd);	\
			} while (0)

#define RTMP_CHIP_ASIC_ADJUST_TX_POWER(__pAd)					\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->AsicAdjustTxPower != NULL)					\
					ops->AsicAdjustTxPower(__pAd);	\
			} while (0)

#define RTMP_CHIP_HIGH_POWER_TUNING(__pAd, __pRssi)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->HighPowerTuning != NULL) \
					ops->HighPowerTuning(__pAd, __pRssi); \
			} while (0)

#define RTMP_CHIP_ANTENNA_INFO_DEFAULT_RESET(__pAd, __pAntenna)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->AsicAntennaDefaultReset != NULL) \
					ops->AsicAntennaDefaultReset(__pAd, __pAntenna);	\
			} while (0)

#define RTMP_NET_DEV_NICKNAME_INIT(__pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->NetDevNickNameInit != NULL)	\
					ops->NetDevNickNameInit(__pAd);	\
			} while (0)

#ifdef CAL_FREE_IC_SUPPORT
#define RTMP_CAL_FREE_IC_CHECK(__pAd, __is_cal_free)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->is_cal_free_ic != NULL)	\
					__is_cal_free = ops->is_cal_free_ic(__pAd);	\
				else		\
					__is_cal_free = FALSE;	\
			} while (0)

#define RTMP_CAL_FREE_DATA_GET(__pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->cal_free_data_get != NULL) { \
					ops->cal_free_data_get(__pAd);	\
					__pAd->E2pCtrl.e2pSource |= E2P_SRC_FROM_EFUSE; \
				} \
			} while (0)
#endif /* CAL_FREE_IC_SUPPORT */

#define RTMP_EEPROM_ASIC_INIT(__pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->NICInitAsicFromEEPROM != NULL)	\
					ops->NICInitAsicFromEEPROM(__pAd);	\
			} while (0)

#define RTMP_CHIP_ASIC_INIT_TEMPERATURE_COMPENSATION(__pAd)								\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->InitTemperCompensation != NULL)					\
					ops->InitTemperCompensation(__pAd);	\
			} while (0)

#define RTMP_CHIP_ASIC_TEMPERATURE_COMPENSATION(__pAd)						\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->TemperCompensation != NULL)					\
					ops->TemperCompensation(__pAd);	\
			} while (0)

#define RTMP_CHIP_UPDATE_BEACON(__pAd, Offset, Value, Unit)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->BeaconUpdate != NULL)	\
					ops->BeaconUpdate(__pAd, Offset, Value, Unit);	\
			} while (0)

#ifdef CARRIER_DETECTION_SUPPORT
#define	RTMP_CHIP_CARRIER_PROGRAM(__pAd, threshold)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->ToneRadarProgram != NULL)	\
					ops->ToneRadarProgram(__pAd, threshold);	\
			} while (0)
#endif /* CARRIER_DETECTION_SUPPORT */

#define RTMP_CHIP_CCK_MRC_STATUS_CTRL(__pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->CckMrcStatusCtrl != NULL)	\
					ops->CckMrcStatusCtrl(__pAd); \
			} while (0)

#define RTMP_CHIP_RADAR_GLRT_COMPENSATE(__pAd) \
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->RadarGLRTCompensate != NULL) \
					ops->RadarGLRTCompensate(__pAd);	\
			} while (0)

/*check here before*/
#define RTMP_SECOND_CCA_DETECTION(__pAd) \
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->SecondCCADetection != NULL)	\
					ops->SecondCCADetection(__pAd);	\
			} while (0)

#define DISABLE_TX_RX(_pAd, _Level)	\
			do { \
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->DisableTxRx != NULL)	\
					ops->DisableTxRx(_pAd, _Level);	\
			} while (0)

#define ASIC_RADIO_ON(_pAd, _Stage)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicRadioOn != NULL)	\
					ops->AsicRadioOn(_pAd, _Stage);	\
			} while (0)

#define ASIC_RADIO_OFF(_pAd, _Stage)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicRadioOff != NULL) \
					ops->AsicRadioOff(_pAd, _Stage);	\
			} while (0)

#ifdef MICROWAVE_OVEN_SUPPORT
#define ASIC_MEASURE_FALSE_CCA(_pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicMeasureFalseCCA != NULL)	\
					ops->AsicMeasureFalseCCA(_pAd);	\
			} while (0)

#define ASIC_MITIGATE_MICROWAVE(_pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicMitigateMicrowave != NULL)	\
					ops->AsicMitigateMicrowave(_pAd);	\
			} while (0)
#endif /* MICROWAVE_OVEN_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
#define ASIC_WOW_ENABLE(_pAd, _pStaCfg)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicWOWEnable != NULL)	\
					ops->AsicWOWEnable(_pAd, _pStaCfg);	\
			} while (0)

#define ASIC_WOW_DISABLE(_pAd, _pStaCfg)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicWOWDisable != NULL)	\
					ops->AsicWOWDisable(_pAd, _pStaCfg);	\
			} while (0)

#define ASIC_WOW_INIT(_pAd) \
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicWOWInit != NULL)	\
					ops->AsicWOWInit(_pAd);	\
			} while (0)

#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

#define MCU_CTRL_INIT(_pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->MCUCtrlInit != NULL)	\
					ops->MCUCtrlInit(_pAd);	\
			} while (0)

#define MCU_CTRL_EXIT(_pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->MCUCtrlExit != NULL)	\
					ops->MCUCtrlExit(_pAd);	\
			} while (0)

#define USB_CFG_READ(_ad, _pvalue)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_ad->hdev_ctrl);\
				if (ops->usb_cfg_read != NULL)	\
					ops->usb_cfg_read(_ad, _pvalue);	\
				else {\
					MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("%s(): usb_cfg_read not inited!\n", __func__));\
				} \
			} while (0)

#define USB_CFG_WRITE(_ad, _value)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_ad->hdev_ctrl);\
				if (ops->usb_cfg_write != NULL) \
					ops->usb_cfg_write(_ad, _value);	\
				else {\
					MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("%s(): usb_cfg_write not inited!\n", __func__));\
				} \
			} while (0)

#ifdef SMART_CARRIER_SENSE_SUPPORT
#define RTMP_CHIP_ASIC_SET_SCS(__pAd, __BandIdx, __value)			\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->ChipSetSCS != NULL)						\
					ops->ChipSetSCS(__pAd, __BandIdx, __value);	\
			} while (0)
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
INT32 AsicBfStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
	UCHAR                ucPhyMode,
	UCHAR                ucBssIdx,
	UCHAR                ucWlanIdx);

INT32 AsicBfStaRecRelease(
	struct _RTMP_ADAPTER *pAd,
	UCHAR                ucBssIdx,
	UCHAR                ucWlanIdx);

INT32 AsicBfPfmuMemAlloc(
	struct _RTMP_ADAPTER *pAd,
	UCHAR                ucSu_Mu,
	UCHAR                ucWlanId);

INT32 AsicBfPfmuMemRelease(
	struct _RTMP_ADAPTER *pAd,
	UCHAR                ucWlanId);

INT32 AsicTxBfTxApplyCtrl(
	struct _RTMP_ADAPTER *pAd,
	UCHAR                ucWlanId,
	BOOLEAN              fgETxBf,
	BOOLEAN              fgITxBf,
	BOOLEAN              fgMuTxBf,
	BOOLEAN              fgPhaseCali);

INT32 AsicTxBfeeHwCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN              fgBfeeHwCtrl);

INT32 AsicTxBfApClientCluster(
	struct _RTMP_ADAPTER *pAd,
	UCHAR                ucWlanId,
	UCHAR                ucCmmWlanId);

INT32 AsicTxBfReptClonedStaToNormalSta(
	RTMP_ADAPTER *pAd,
	UCHAR   ucWlanId,
	UCHAR   ucCliIdx);

INT32 AsicTxBfHwEnStatusUpdate(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN              fgETxBf,
	BOOLEAN              fgITxBf);

INT32 AsicTxBfModuleEnCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BfNum,
	UINT8 u1BfBitmap,
	UINT8 u1BfSelBand[]);

#endif /* MT_MAC && TXBF_SUPPORT */

VOID AsicSetRxAnt(struct _RTMP_ADAPTER *pAd, UCHAR	Ant);

#ifdef MICROWAVE_OVEN_SUPPORT
VOID AsicMeasureFalseCCA(
	struct _RTMP_ADAPTER *pAd
);

VOID AsicMitigateMicrowave(
	struct _RTMP_ADAPTER *pAd
);
#endif /* MICROWAVE_OVEN_SUPPORT */
VOID AsicBbpInitFromEEPROM(struct _RTMP_ADAPTER *pAd);



#ifdef TXBF_SUPPORT
VOID chip_tx_bf_init(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *pEntry, struct _IE_lists *ie_list, BOOLEAN supportsETxBF);
#endif /*TXBF_SUPPORT*/

#ifdef RF_LOCKDOWN
BOOLEAN chip_check_rf_lock_down(struct _RTMP_ADAPTER *pAd);
#endif /*RF_LOCKDOWN*/

INT32 chip_cmd_tx(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
VOID chip_spectrum_start(struct _RTMP_ADAPTER *ad, UINT8 *info);
VOID chip_fw_init(struct _RTMP_ADAPTER *ad);
VOID chip_parse_rxv_packet(struct _RTMP_ADAPTER *ad, UINT32 Type, struct _RX_BLK *RxBlk, UCHAR *Data);
INT32 chip_txs_handler(struct _RTMP_ADAPTER *ad, struct _RX_BLK *rx_blk, VOID *rx_packet);
VOID chip_rx_event_handler(struct _RTMP_ADAPTER *ad, VOID *rx_packet);
INT chip_show_pwr_info(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
VOID chip_arch_set_aid(struct _RTMP_ADAPTER *ad, USHORT aid);
INT32 chip_tssi_set(struct _RTMP_ADAPTER *ad, char *efuse);
INT32 chip_pa_lna_set(struct _RTMP_ADAPTER *ad, char *efuse);

#endif
