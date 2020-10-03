
#ifndef __FPGA_CTL_H__
#define __FPGA_CTL_H__


#ifdef MT_MAC
typedef enum _MT_MAC_TXS_TYPE{
	TXS_NONE = 0x0,

	TXS_DATA = 0x1ff,
	TXS_QDATA = 0x101,
	TXS_NON_QDATA = 0x102,

	TXS_MGMT = 0x2ff,
	TXS_BCN = 0x201,
	TXS_MGMT_OTHER = 0x202,
	
	TXS_CTRL = 0x4ff,
	TXS_ALL = 0xfff,
}MT_MAC_TXS_TYPE;
#endif /* MT_MAC */

struct fpga_ctrl {
	int tx_kick_cnt;
	int phy_rates;
	int tx_data_phy;
	UINT8 tx_data_bw;
	UINT8 tx_data_ldpc;
	UINT8 tx_data_mcs;
	UINT8 tx_data_gi;
	UINT8 tx_data_stbc;
	int rx_data_phy;
	UINT8 rx_data_bw;
	UINT8 rx_data_ldpc;
	UINT8 rx_data_mcs;
	UINT8 rx_data_gi;
	UINT8 rx_data_stbc;
	UINT8 data_basize;
	UINT8 fpga_on;
	UINT8 fpga_tr_stop;
	UINT8 vco_cal;

	UINT8 dma_mode;

#ifdef MT_MAC
	MT_MAC_TXS_TYPE txs_type;
	UCHAR no_bcn;
#endif /* MT_MAC */

#ifdef CAPTURE_MODE
	BOOLEAN cap_support;	/* 0: no cap mode; 1: cap mode enable */
	UCHAR cap_type;			/* 1: ADC6, 2: ADC8, 3: FEQ */
	UCHAR cap_trigger;		/* 1: manual trigger, 2: auto trigger */
	BOOLEAN do_cap;			/* 1: start to do cap, if auto, will triggered depends on trigger condition, if manual, start immediately */
	BOOLEAN cap_done;		/* 1: capture done, 0: capture not finish yet */
	UINT32 trigger_offset;	/* in unit of bytes */
	UCHAR *cap_buf;
#endif /* CAPTURE_MODE */
};

#endif

