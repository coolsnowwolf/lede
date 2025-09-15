
#ifndef _TESTMODE_IOCTL_H
#define _TESTMODE_IOCTL_H

#define TM_STATUS_NOTSUPPORT 0x01

#if defined(COMPOS_TESTMODE_WIN)
#define PKTL_TRAN_TO_NET(_Val) _Val
#define PKTL_TRAN_TO_HOST(_Val) _Val
#define PKTS_TRAN_TO_NET(_Val) _Val
#define PKTS_TRAN_TO_HOST(_Val) _Val
#define PKTLA_TRAN_TO_HOST(_len, _byte_array) _byte_array
#define PKTLA_TRAN_TO_NET(_len, _byte_array) _byte_array
#define PKTLA_DUMP(_lvl, _len, _byte_arrary) _byte_array
#define PKTUC_DUMP(_lvl, _len, _byte_arrary) _byte_array
#else
#define PKTL_TRAN_TO_NET(_Val) OS_HTONL(_Val)
#define PKTL_TRAN_TO_HOST(_Val) OS_NTOHL(_Val)
#define PKTS_TRAN_TO_NET(_Val) OS_HTONS(_Val)
#define PKTS_TRAN_TO_HOST(_Val) OS_NTOHS(_Val)
#define PKTLA_TRAN_TO_HOST(_len, _byte_array) ({		\
		INT _cnt = 0;										\
		UINT32 *_arr = (UINT32 *)_byte_array;				\
		for (_cnt = 0; _cnt < _len; _cnt++)						\
			_arr[_cnt] = OS_NTOHL(_arr[_cnt]);				\
		_byte_array;										\
	});
#define PKTLA_TRAN_TO_NET(_len, _byte_array) ({		\
		INT _cnt = 0;										\
		UINT32 *_arr = (UINT32 *)_byte_array;				\
		for (_cnt = 0; _cnt < _len; _cnt++)						\
			_arr[_cnt] = OS_HTONL(_arr[_cnt]);				\
		_byte_array;										\
	});
#define PKTLA_DUMP(_lvl, _len, _byte_array)({			\
		INT _cnt = 0;										\
		UINT32 *_arr = (UINT32 *)_byte_array;				\
		for (_cnt = 0; _cnt < _len; _cnt++)						\
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, _lvl, ("DWORD%d:%08x\n", _cnt, _arr[_cnt]));	\
		_len;												\
	});
#define PKTUC_DUMP(_lvl, _len, _byte_array)({			\
		INT _cnt = 0;										\
		UCHAR *_arr = (UCHAR *)_byte_array;				\
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, _lvl, ("PKTUC_DUMP(%x): ", _len));	\
		for (_cnt = 0; _cnt < _len; _cnt++)						\
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, _lvl, ("%02x", _arr[_cnt]));	\
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, _lvl, ("\n"));	\
		_len;												\
	});
#endif

#if defined(COMPOS_TESTMODE_WIN)
/***********************************************************************************
 * IOCTL related definitions and data structures.
 **********************************************************************************/
typedef struct __RTMP_IOCTL_INPUT_STRUCT {
	UINT32 InBufLen;
	PVOID InBuf;
	UINT32 OutBufLen;
	PVOID OutBuf;
	UINT32 *BytesRet;
} RTMP_IOCTL_INPUT_STRUCT;

BOOLEAN mt_mac_cr_range_mapping(struct _RTMP_ADAPTER *pAd, UINT32 *mac_addr);
UINT32 mt_physical_addr_map(UINT32 addr);

#define RTMP_IO_EFAULT							30002
#define EFAULT					(-RTMP_IO_EFAULT)
#endif /* TODO: Add lack of structure for Linux temporarily, fix during merge */
VOID HQA_GetThermalValue_CB(struct cmd_msg *msg, char *Data, UINT16 Len);
#ifdef TXBF_SUPPORT
VOID HQA_BF_INFO_CB(RTMP_ADAPTER *pAd, unsigned char *data, UINT32 len);
#endif
/*
 * #if defined(COMPOS_TESTMODE_WIN)
 * #define OS_ENDIANSWAP(_Val) _Val;
 * #else
 * //#define OS_ENDIANSWAP(_Val) PKTL_TRAN_TO_HOST(_Val);
 * #endif
 */
/* Common Part, TODO: Only common part after merge */
/* Structure for TxBF*/
#define HQA_BF_STR_SIZE 512
#pragma pack(1)
struct GNU_PACKED _HQA_HIFTestParam {
	UINT32 start_len;
	UINT32 stop_len;
	UINT32 repeat_time;
	UINT32 is_def_pattern;
	UINT32 bulkout_num;
	UINT32 bulkin_num;
	UINT32 txagg_num;
	UINT32 rxagg_limit;
	UINT32 rxagg_lm;
	UINT32 rxagg_to;
	UINT32 enable_rxagg;
};

#ifdef TXBF_SUPPORT
struct GNU_PACKED _HQA_BF_TAG_ALLOC {
	UINT32 col_idx0;
	UINT32 row_idx0;
	UINT32 col_idx1;
	UINT32 row_idx1;
	UINT32 col_idx2;
	UINT32 row_idx2;
	UINT32 col_idx3;
	UINT32 row_idx3;
};

struct GNU_PACKED _HQA_BF_TAG_MATRIX {
	UINT32 nrow;
	UINT32 ncol;
	UINT32 ngroup;
	UINT32 LM;
	UINT32 code_book;
	UINT32 htc_exist;
};

struct GNU_PACKED _HQA_BF_TAG_SNR {
	UINT32 snr_sts0;
	UINT32 snr_sts1;
	UINT32 snr_sts2;
	UINT32 snr_sts3;
};

struct GNU_PACKED _HQA_BF_TAG_MCS_THRD {
	UINT32 mcs_lss0;
	UINT32 mcs_sss0;
	UINT32 mcs_lss1;
	UINT32 mcs_sss1;
	UINT32 mcs_lss2;
	UINT32 mcs_sss2;
};

struct GNU_PACKED _HQA_BF_TAG_INFO {
	/* Tag 1 */
	UINT32 profile;			/* [6:0]		: 0 ~ 63 */
	UINT32 tx_bf;			/* [7]		: 0: iBF, 1: eBF */
	UINT32 dbw;			/* [9:8]     : 0/1/2/3: DW20/40/80/160NC */
	UINT32 su_mu;			/* [10]      : 0:SU, 1: MU */
	UINT32 invalid;			/* [11]      : 0:default, 1: This profile number is invalid by SW */
	UINT32 rmsd;			/* [14:12]   : RMSD value from CE */
	UINT32 addr1_col_idx;		/* [17 : 15] : column index : 0 ~ 5 */
	UINT32 addr1_row_idx;		/* [23 : 18] : row index : 0 ~ 63 */
	UINT32 addr2_col_idx;		/* [26 : 24] : column index : 0 ~ 5 */
	UINT32 addr2_row_idx;		/* [31 : 27] : row index : 0 ~ 63, [32]: MSB of row index */
	UINT32 addr3_col_idx;		/* [35 : 33] : column index : 0 ~ 5 */
	UINT32 addr3_row_idx;		/* [41 : 36] : row index : 0 ~ 63 */
	UINT32 addr4_col_idx;		/* [44 : 42] : column index : 0 ~ 5 */
	UINT32 addr4_row_idx;		/* [50 : 45] : row index : 0 ~ 63 */
	UINT32 reserved1;		/* [51]      : Reserved */
	UINT32 nrow;			/* [53 : 52] : Nrow */
	UINT32 ncol;			/* [55 : 54] : Ncol */
	UINT32 ngroup;			/* [57 : 56] : Ngroup */
	UINT32 LM;			/* [59 : 58] : 0/1/2 */
	UINT32 code_book;		/* [61:60]   : Code book */
	UINT32 htc_exist;		/* [62]      : HtcExist */
	UINT32 reserved2;		/* [63]      : Reserved */
	UINT32 snr_sts0;		/* [71:64]   : SNR_STS0 */
	UINT32 snr_sts1;		/* [79:72]   : SNR_STS1 */
	UINT32 snr_sts2;		/* [87:80]   : SNR_STS2 */
	UINT32 snr_sts3;		/* [95:88]   : SNR_STS3 */
	UINT32 ibf_lanidx;		/* [103:96]  : iBF LNA index */
	/* Tag 2 */
	UINT32 smt_ant;			/* [11:0]   : Smart Ant config */
	UINT32 reserved3;		/* [14:12]  : Reserved */
	UINT32 se_idx;			/* [19:15]  : SE index */
	UINT32 rmsd_thrd;		/* [22:20]  : RMSD Threshold */
	UINT32 reserved4;		/* [23]     : Reserved */
	UINT32 mcs_thrd_l1ss;		/* [27:24]  : MCS TH long 1SS */
	UINT32 mcs_thrd_s1ss;		/* [31:28]  : MCS TH short 1SS */
	UINT32 mcs_thrd_l2ss;		/* [35:32]  : MCS TH long 2SS */
	UINT32 mcs_thrd_s2ss;		/* [39:36]  : MCS TH short 2SS */
	UINT32 mcs_thrd_l3ss;		/* [43:40]  : MCS TH long 3SS */
	UINT32 mcs_thrd_s3ss;		/* [47:44]  : MCS TH short 3SS */
	UINT32 bf_tout;			/* [55:48]  : iBF timeout limit */
	UINT32 reserved5;		/* [63:56]  : Reserved */
	UINT32 reserved6;		/* [71:64]  : Reserved */
	UINT32 reserved7;		/* [79:72]  : Reserved */
	UINT32 ibf_dbw;			/* [81:80]  : iBF desired DBW 0/1/2/3 : BW20/40/80/160NC */
	UINT32 ibf_ncol;		/* [83:82]  : iBF desired Ncol = 1 ~ 3 */
	UINT32 ibf_nrow;		/* [85:84]  : iBF desired Nrow = 1 ~ 4 */
	UINT32 reserved8;		/* [95:86]  : Reserved */
};

struct GNU_PACKED _HQA_BF_STA_CMM_REC {
	UINT32 wlan_idx;
	UINT32 bss_idx;
	UINT32 aid;
	UCHAR mac[MAC_ADDR_LEN];
};

enum _HQA_BF_CMD_ID {
	ATE_TXBF_INIT = 1,
	ATE_CHANNEL,
	ATE_TX_MCS,
	ATE_TX_POW0,
	ATE_TX_ANT,
	ATE_RX_FRAME,
	ATE_RX_ANT,
	ATE_TXBF_LNA_GAIN,
	ATE_IBF_PHASE_COMP,
	ATE_IBF_TX,
	ATE_IBF_PROF_UPDATE,
	ATE_EBF_PROF_UPDATE,
	ATE_IBF_INST_CAL,
	ATE_IBF_INST_VERIFY,
	ATE_TXBF_GD_INIT,
	ATE_IBF_PHASE_E2P_UPDATE
};

struct GNU_PACKED _HQA_BF_STA_REC {
	UINT32 wlan_idx;
	UINT32 bss_idx;
	UINT32 PfmuId;
	UINT32 su_mu;
	UINT32 etxbf_cap;
	UINT32 ndpa_rate;
	UINT32 ndp_rate;
	UINT32 report_poll_rate;
	UINT32 tx_mode;
	UINT32 nc;
	UINT32 nr;
	UINT32 cbw;
	UINT32 spe_idx;
	UINT32 tot_mem_req;
	UINT32 mem_req_20m;
	UINT32 mem_row0;
	UINT32 mem_col0;
	UINT32 mem_row1;
	UINT32 mem_col1;
	UINT32 mem_row2;
	UINT32 mem_col2;
	UINT32 mem_row3;
	UINT32 mem_col3;
};

struct GNU_PACKED _HQA_BF_STA_PROFILE {
	UINT32 pfmuid;
	UINT32 subcarrier;
	UINT32 phi11;
	UINT32 psi21;
	UINT32 phi21;
	UINT32 psi31;
	UINT32 phi31;
	UINT32 psi41;
	UINT32 phi22;
	UINT32 psi32;
	UINT32 phi32;
	UINT32 psi42;
	UINT32 phi33;
	UINT32 psi43;
	UINT32 snr00;
	UINT32 snr01;
	UINT32 snr02;
	UINT32 snr03;
};

struct GNU_PACKED _HQA_BF_SOUNDING {
	UINT32 su_mu;
	UINT32 mu_num;
	UINT32 snd_interval;
	UINT32 wlan_id0;
	UINT32 wlan_id1;
	UINT32 wlan_id2;
	UINT32 wlan_id3;
	UINT32 band_idx;
};

struct GNU_PACKED _HQA_BF_MANUAL_CONN {
	UCHAR mac[MAC_ADDR_LEN];
	UINT32 type;
	UINT32 wtbl_idx;
	UINT32 ownmac_idx;
	UINT32 phymode;
	UINT32 bw;
	UINT32 pfmuid;
	UINT32 marate_mode;
	UINT32 marate_mcs;
	UINT32 nss;
	UINT32 spe_idx;
	UINT32  rca2;
	UINT32  rv;
	UINT32  aid;
};

struct GNU_PACKED _HQA_PWR_MANUAL {
	UINT32 u4PwrManCtrl;
	UINT32 u4TxPwrModeManual;
	UINT32 u4TxPwrBwManual;
	UINT32 u4TxPwrRateManual;
	UINT32 i4TxPwrValueManual;
	UINT32 u4Band;
};

#ifdef CFG_SUPPORT_MU_MIMO
/* Structure for MU-MIMO */
struct _HQA_MU_STA_PARAM {
	UINT32 gid[2];
	UINT32 up[4];
};

struct _HQA_MU_TBL {
	UINT32 su_mu;
	UCHAR tbl[0];
};

struct  GNU_PACKED _HQA_MU_USR_INIT_MCS {
	UINT32 user0;
	UINT32 user1;
	UINT32 user2;
	UINT32 user3;
};

struct  GNU_PACKED _HQA_MU_USR_LQ {
	UINT32 usr0_bpsk;
	UINT32 usr0_qpsk;
	UINT32 usr0_16qam;
	UINT32 usr0_64qam;
	UINT32 usr0_256qam;
	UINT32 usr1_bpsk;
	UINT32 usr1_qpsk;
	UINT32 usr1_16qam;
	UINT32 usr1_64qam;
	UINT32 usr1_256qam;
	UINT32 usr2_bpsk;
	UINT32 usr2_qpsk;
	UINT32 usr2_16qam;
	UINT32 usr2_64qam;
	UINT32 usr2_256qam;
	UINT32 usr3_bpsk;
	UINT32 usr3_qpsk;
	UINT32 usr3_16qam;
	UINT32 usr3_64qam;
	UINT32 usr3_256qam;
};

struct  GNU_PACKED _HQA_MU_GRP_INFO {
	UINT32 grp_idx;
	UINT32 num_usr;
	UINT32 usr0_ldpc;
	UINT32 usr1_ldpc;
	UINT32 usr2_ldpc;
	UINT32 usr3_ldpc;
	UINT32 usr0_nss;
	UINT32 usr1_nss;
	UINT32 usr2_nss;
	UINT32 usr3_nss;
	UINT32 gid;
	UINT32 usr0_up;
	UINT32 usr1_up;
	UINT32 usr2_up;
	UINT32 usr3_up;
	UINT32 usr0_pfmu_id;
	UINT32 usr1_pfmu_id;
	UINT32 usr2_pfmu_id;
	UINT32 usr3_pfmu_id;
	UINT32 initMCS0;
	UINT32 initMCS1;
	UINT32 initMCS2;
	UINT32 initMCS3;
	UCHAR usr0_mac[MAC_ADDR_LEN];
	UCHAR usr1_mac[MAC_ADDR_LEN];
	UCHAR usr2_mac[MAC_ADDR_LEN];
	UCHAR usr3_mac[MAC_ADDR_LEN];
};
#endif/* CFG_SUPPORT_MU_MIMO */
#endif /* TXBF_SUPPORT */
#pragma pack()

/* DBDC Tx Ext */
#pragma pack(1)
struct  GNU_PACKED _HQA_EXT_SET_CH {
	UINT32 ext_id;
	UINT32 num_param;
	UINT32 band_idx;
	UINT32 central_ch0;
	UINT32 central_ch1;
	UINT32 sys_bw;
	UINT32 perpkt_bw;
	UINT32 pri_sel;
	UINT32 reason;
	UINT32 ch_band;
    UINT32 out_band_freq;
};

struct GNU_PACKED _HQA_EXT_TX_CONTENT {
	UINT32 ext_id;
	UINT32 num_param;
	UINT32 band_idx;
	UINT32 FC;
	UINT32 dur;
	UINT32 seq;
	UINT32 gen_payload_rule;	/* Normal:0,Repeat:1,Random:2 */
	UINT32 txlen;
	UINT32 payload_len;
	UCHAR addr1[MAC_ADDR_LEN];
	UCHAR addr2[MAC_ADDR_LEN];
	UCHAR addr3[MAC_ADDR_LEN];
	UCHAR payload[0];
};

struct GNU_PACKED _HQA_EXT_TXV {
	UINT32 ext_id;
	UINT32 num_param;
	UINT32 band_idx;
	UINT32 pkt_cnt;
	UINT32 phymode;
	UINT32 rate;
	UINT32 pwr;
	UINT32 stbc;
	UINT32 ldpc;
	UINT32 ibf;
	UINT32 ebf;
	UINT32 wlan_id;
	UINT32 aifs;
	UINT32 gi;
	UINT32 tx_path;
	UINT32 nss;
};

struct GNU_PACKED _HQA_CMD_FRAME {
	UINT32		MagicNo;
	UINT16		Type;
	UINT16		Id;
	UINT16		Length;
	UINT16		Sequence;
	UINT8		Data[IOCTLBUFF];
};
#pragma pack()

typedef INT32(*HQA_CMD_HANDLER)(
	struct _RTMP_ADAPTER *pAd,
	struct __RTMP_IOCTL_INPUT_STRUCT *wrq,
	struct _HQA_CMD_FRAME *HqaCmdFrame);

struct _HQA_CMD_TABLE {
	HQA_CMD_HANDLER *CmdSet;
	UINT32	CmdSetSize;
	UINT32	CmdOffset;
};

struct GNU_PACKED _HQA_RX_STAT {
	UINT32 mac_rx_fcs_err_cnt;
	UINT32 mac_rx_mdrdy_cnt;
	UINT32 phy_rx_fcs_err_cnt_cck;
	UINT32 phy_rx_fcs_err_cnt_ofdm;
	UINT32 phy_rx_pd_cck;
	UINT32 phy_rx_pd_ofdm;
	UINT32 phy_rx_sig_err_cck;
	UINT32 phy_rx_sfd_err_cck;
	UINT32 phy_rx_sig_err_ofdm;
	UINT32 phy_rx_tag_err_ofdm;
	UINT32 WB_RSSSI0;
	UINT32 IB_RSSSI0;
	UINT32 WB_RSSSI1;
	UINT32 IB_RSSSI1;
	UINT32 phy_rx_mdrdy_cnt_cck;
	UINT32 phy_rx_mdrdy_cnt_ofdm;
	UINT32 DriverRxCount;
	UINT32 RCPI0;
	UINT32 RCPI1;
	INT32 FreqOffsetFromRX;
	UINT32 RSSI0;
	UINT32 RSSI1;
	UINT32 rx_fifo_full;
	/* #ifdef MT7615 */
	UINT32 mac_rx_len_mismatch;
	UINT32 mac_rx_fcs_err_cnt_band1;
	UINT32 mac_rx_mdrdy_cnt_band1;
	UINT32 FAGC_IB_RSSSI[4];
	UINT32 FAGC_WB_RSSSI[4];
	UINT32 Inst_IB_RSSSI[4];
	UINT32 Inst_WB_RSSSI[4];
	UINT32 ACIHitLow;
	UINT32 ACIHitHigh;
	UINT32 DriverRxCount1;
	UINT32 RCPI2;
	UINT32 RCPI3;
	UINT32 RSSI2;
	UINT32 RSSI3;
	UINT32 SNR0;
	UINT32 SNR1;
	UINT32 SNR2;
	UINT32 SNR3;
	UINT32 rx_fifo_full_band1;
	UINT32 mac_rx_len_mismatch_band1;
	UINT32 phy_rx_pd_cck_band1;
	UINT32 phy_rx_pd_ofdm_band1;
	UINT32 phy_rx_sig_err_cck_band1;
	UINT32 phy_rx_sfd_err_cck_band1;
	UINT32 phy_rx_sig_err_ofdm_band1;
	UINT32 phy_rx_tag_err_ofdm_band1;
	UINT32 phy_rx_mdrdy_cnt_cck_band1;
	UINT32 phy_rx_mdrdy_cnt_ofdm_band1;
	UINT32 phy_rx_fcs_err_cnt_cck_band1;
	UINT32 phy_rx_fcs_err_cnt_ofdm_band1;
	UINT32 MuPktCount;
	UINT32 SIG_MCS;
	UINT32 SINR;
	UINT32 RXVRSSI;
	/* #endif */
	UINT32 mac_rx_fcs_ok_cnt;	/* MT7622 only */
};

UINT32 HQA_CMDHandler(struct _RTMP_ADAPTER *pAd, struct __RTMP_IOCTL_INPUT_STRUCT *Wrq, struct _HQA_CMD_FRAME *HqaCmdFrame);

#define TM_CMDREQ			0x0008
#define TM_CMDRSP			0x8008
#define HQA_CMD_MAGIC_NO		0x18142880

#define	HQA_CMD_REQ			0x0005
#define	HQA_CMD_RSP			0x8005
#define TMR_HW_VER_100			100
#define TMR_HW_VER_150			150
#define TMR_HW_VER_200			200


#define HQA_CMD_MAGIC_NO		0x18142880
#define EFUSE_ADDR_FREQUENCY_OFFSET_MT7637 0xF4/* MT7637 */

#endif /* _TESTMODE_IOCTL_H */
