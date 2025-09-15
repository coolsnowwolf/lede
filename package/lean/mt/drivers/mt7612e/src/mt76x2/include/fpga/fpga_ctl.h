
#ifndef __FPGA_CTL_H__
#define __FPGA_CTL_H__

struct fpga_ctrl {
	int tx_kick_cnt;
	int phy_rates;
	int tx_data_phy;
	u8 tx_data_bw;
	u8 tx_data_ldpc;
	u8 tx_data_mcs;
	u8 tx_data_gi;
	u8 tx_data_stbc;
	int rx_data_phy;
	u8 rx_data_bw;
	u8 rx_data_ldpc;
	u8 rx_data_mcs;
	u8 rx_data_gi;
	u8 rx_data_stbc;
	u8 data_basize;
	u8 fpga_on;
	u8 fpga_tr_stop;
	u8 vco_cal;
};

#endif

