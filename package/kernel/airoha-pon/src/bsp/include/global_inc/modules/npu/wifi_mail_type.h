/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef WIFI_MAIL_TYPE_H_
#define WIFI_MAIL_TYPE_H_

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#ifndef NULL
	#define NULL	0
#endif

#define INTERFACE_2G 0
#define INTERFACE_5G 1

#define NPU_MAILBOX_ERROR 0
#define NPU_MAILBOX_SUCCESS 1
#define MAX_SSID_PER_BAND 8

#define MAX_PER_ENTRY 128
	
typedef enum {
	WIFI_MAIL_FUNCTION_SET_WAIT_PCIE_ADDR = 0,
	WIFI_MAIL_FUNCTION_SET_WAIT_DESC,
	WIFI_MAIL_FUNCTION_SET_WAIT_NPU_INIT_DONE,
	WIFI_MAIL_FUNCTION_SET_WAIT_TRAN_TO_CPU,
	WIFI_MAIL_FUNCTION_SET_WAIT_BA_WIN_SIZE,
	WIFI_MAIL_FUNCTION_SET_WAIT_DRIVER_MODEL,
	WIFI_MAIL_FUNCTION_SET_WAIT_DEL_STA,
	WIFI_MAIL_FUNCTION_SET_WAIT_DRAM_BA_NODE_ADDR,
	WIFI_MAIL_FUNCTION_SET_WAIT_PKT_BUF_ADDR,
	WIFI_MAIL_FUNCTION_SET_WAIT_IS_TEST_NOBA,
	WIFI_MAIL_FUNCTION_SET_WAIT_FLUSHONE_TIMEOUT,
	WIFI_MAIL_FUNCTION_SET_WAIT_FLUSHALL_TIMEOUT,	
	WIFI_MAIL_FUNCTION_SET_WAIT_IS_FORCE_TO_CPU,
	WIFI_MAIL_FUNCTION_SET_WAIT_PCIE_STATE,		
	WIFI_MAIL_FUNCTION_SET_WAIT_PCIE_PORT_TYPE,	
	WIFI_MAIL_FUNCTION_SET_WAIT_ERROR_RETRY_TIMES,	
	WIFI_MAIL_FUNCTION_SET_WAIT_BAR_INFO,
	WIFI_MAIL_FUNCTION_SET_WAIT_FAST_FLAG,
	WIFI_MAIL_FUNCTION_SET_WAIT_NPU_BAND0_ONCPU,
	WIFI_MAIL_FUNCTION_SET_WAIT_TX_RING_PCIE_ADDR,		//TCSUPPORT_NPU_WIFI_TX
	WIFI_MAIL_FUNCTION_SET_WAIT_TX_DESC_HW_BASE,		//TCSUPPORT_NPU_WIFI_TX
	WIFI_MAIL_FUNCTION_SET_WAIT_TX_BUF_SPACE_HW_BASE,	//TCSUPPORT_NPU_WIFI_TX
	WIFI_MAIL_FUNCTION_SET_WAIT_RX_RING_FOR_TXDONE_HW_BASE,	//TCSUPPORT_NPU_WIFI_TX
	WIFI_MAIL_FUNCTION_SET_WAIT_TX_PKT_BUF_ADDR,		//TCSUPPORT_NPU_WIFI_TX
	WIFI_MAIL_FUNCTION_SET_WAIT_INODE_TXRX_REG_ADDR,	//TCSUPPORT_WLAN_INODE
	WIFI_MAIL_FUNCTION_SET_WAIT_INODE_DEBUG_FLAG,		//TCSUPPORT_WLAN_INODE
	WIFI_MAIL_FUNCTION_SET_WAIT_INODE_HW_CFG_INFO,		//TCSUPPORT_WLAN_INODE
	WIFI_MAIL_FUNCTION_SET_WAIT_INODE_STOP_ACTION,		//TCSUPPORT_WLAN_INODE
	WIFI_MAIL_FUNCTION_SET_WAIT_INODE_PCIE_SWAP,		//TCSUPPORT_WLAN_INODE
	WIFI_MAIL_FUNCTION_SET_WAIT_RATELIMIT_CTRL,
	WIFI_MAIL_FUNCTION_SET_WAIT_MAX_NUM
} WIFI_MAIL_Set_Wait_Func_t;

typedef enum {
	WIFI_MAIL_FUNCTION_SET_NO_WAIT_BASE = 0,

	WIFI_MAIL_FUNCTION_SET_NO_WAIT_MAX_NUM
} WIFI_MAIL_Set_noWait_Func_t;

typedef enum {
	WIFI_MAIL_FUNCTION_GET_WAIT_NPU_INFO=0,
	WIFI_MAIL_FUNCTION_GET_WAIT_LAST_RATE,
	WIFI_MAIL_FUNCTION_GET_WAIT_COUNTER,
	WIFI_MAIL_FUNCTION_GET_WAIT_DBG_COUNTER,
	WIFI_MAIL_FUNCTION_GET_WAIT_RXDESC_BASE,
	WIFI_MAIL_FUNCTION_GET_WAIT_WCID_DBG_COUNTER,
	WIFI_MAIL_FUNCTION_GET_WAIT_DMA_ADDR,		//TCSUPPORT_WLAN_INODE
	WIFI_MAIL_FUNCTION_GET_WAIT_RING_SIZE,		//TCSUPPORT_WLAN_INODE
	WIFI_MAIL_FUNCTION_GET_WAIT_MAX_NUM
} WIFI_MAIL_Get_Wait_Func_t;

typedef enum {
	WIFI_MAIL_FUNCTION_GET_NO_WAIT_BASE = 0,

	WIFI_MAIL_FUNCTION_GET_NO_WAIT_MAX_NUM
} WIFI_MAIL_Get_noWait_Func_t;

typedef enum {
	SET_WAIT = 1,
	SET_NO_WAIT,
	GET_WAIT,
	GET_NO_WAIT,

	OP_MAX_NUM 
} WIFI_MAIL_FuncType_t;

typedef struct npu_info {
	unsigned int info;
}npu_info_t;

typedef struct last_rate {
	unsigned int perSta;
	unsigned int total;
}last_rate_t;

typedef struct counter {
	unsigned int txCount;

	unsigned long long rxCount2G[MAX_SSID_PER_BAND];
	unsigned long long rxCount5G[MAX_SSID_PER_BAND];
	unsigned long long rxByteCount2G[MAX_SSID_PER_BAND];
	unsigned long long rxByteCount5G[MAX_SSID_PER_BAND];	
	unsigned char omacIdx5G[MAX_SSID_PER_BAND];
	unsigned char omacIdx2G[MAX_SSID_PER_BAND]; 
	unsigned long long rxApcliCount2G;
	unsigned long long rxApcliCount5G;
	unsigned long long rxApcliByteCount2G;
	unsigned long long rxApcliByteCount5G;
    unsigned long long rx_pkts_entry[2][MAX_PER_ENTRY];	
	unsigned long long rx_bytes_entry[2][MAX_PER_ENTRY];

//	unsigned int rxCount;

}counter_t;

typedef struct ratelimit_ctrl {
	unsigned int band_idx;
	unsigned int bssid_idx;
	unsigned int ctrl;
}ratelimit_ctrl_t;

typedef struct dbg_counter {
	unsigned int errCount;
}dbg_counter_t;

typedef struct inode_txrx_addr{
	unsigned int dir;
	unsigned int inCounterAddr;
	unsigned int outStatusAddr;
	unsigned int outCounterAddr;
}inode_txrx_addr_t;

typedef struct inode_cfg_info{
	unsigned char  ep_mask;
	unsigned char  vap_mask;
}inode_cfg_info_t;

typedef enum {
	MT7915_A = 0,
	MT7915_D,	
	MT7916_D,
	D_BOTTOM_
} _DriverModel_t;

typedef enum {
	P0 = 0,
	P1,
	P0_P0,
	P0_P1
} _PortType_t;

typedef struct WIFI_MAIL_Data {
	unsigned int interfaceID:4;
	unsigned int funcType:4;
	unsigned int funcId;
	union {
		unsigned int PCIEAddr;
		unsigned int desc;
		unsigned int npuInitDone;
		unsigned int tran2cpu;
		unsigned int BAWinSize;
	        unsigned int BARINFO;
		unsigned int DriverModel;
		unsigned int dramBaNodeAddr;
		npu_info_t npuInfo;
		last_rate_t lastRate;
		counter_t count;
		ratelimit_ctrl_t rate_limit_ctrl;
		dbg_counter_t dbgCount;		
		unsigned int wcid;
		unsigned int isForceToCpu_flag;
		unsigned int band0OnCpuFlag;
		unsigned int npuPktBufAddr;
		unsigned int isforTestNoBaFlag;
		unsigned int flushone_timeout;
		unsigned int flushall_timeout;		
		unsigned char fastFlag;
		unsigned int PciePortType;
		unsigned int retryTimes;
//#ifdef TCSUPPORT_NPU_WIFI_TX
		unsigned int phy_addr;
//#endif
//#ifdef TCSUPPORT_WLAN_INODE //i-one only but do not use compile flag
		inode_txrx_addr_t txrx_addr;
		inode_cfg_info_t cfg_info;
		unsigned int dir;
		unsigned int dma_addr;
//#endif
	} wifi_mail_private;
}WIFI_MAIL_Data_t;


#endif 


