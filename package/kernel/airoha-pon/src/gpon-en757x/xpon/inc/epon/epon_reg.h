#ifndef _EPON_REG_H_
#define _EPON_REG_H_

#include "epon_mac_reg_c_header.h"

#define CONFIG_EPON_BASE_ADDR					(0x1FB60000)
#define CONFIG_EPON_REG_RANGE					(sizeof(EPON_MAC_REGS))



#define REG_E_SW_RST     0xBFB00834  //bit31:for reset epon mac

typedef struct eponMacHwtestReg_s{
	__u32 addr;	
	__u32 def_value;	/*default value*/
	__u32 rwmask;/*bit:0-read only*/
}eponMacHwtestReg_t;


#define READ_REG_WORD(reg)		ioread32((void __iomem *)(reg)) /* GetReg((uint)reg) */
#define WRITE_REG_WORD(reg, val)      iowrite32(val, (void __iomem *)(reg)) /* SetReg((uint)reg, val) */

#define PAUSE(x)		mdelay(x)



/* e_glb_cfg */
#define SNIFFER_MODE	(1<<26)
#define MPCP_FWD	(1<<22)
#define BCST_LLID_0xFFFF_DROP 	(1<<21)
#define BCST_LLID_0x7FFF_DROP (1<<20)
#define MCST_LLID_DROP (1<<19)
#define ALL_UNICAST_LLID_PKT_FWD (1<<18)
#define FCS_ERR_FWD (1<<17)
#define LLID_CRC8_ERR_FWD (1<<16)
#define PHY_PWR_DOWN (1<<11)
#define RX_NML_GATE_FWD (1<<10)
#define RXMBI_STOP (1<<9)
#define TXMBI_STOP (1<<8)
#define CHK_ALL_GNT_MODE (1<<7)
#define LOC_CNT_SYNC_METHOD (1<<6)
#define TX_DEFAULT_RPT (1<<5)
#define EPON_MAC_SW_RST (1<<4)
#define EPON_OAM_CAL_IN_ETH (1<<3)
#define EPON_MAC_LPBK_EN	(1<<2)
#define RPT_TXPRI_CTRL	(1<<1)
#define MODE_SEL	(1<<0)




/* e_int_status */
#define REG_ACK_DONE_INT	(1<<25)
#define REG_REQ_DONE_INT	(1<<24)
#ifdef TCSUPPORT_EPON_POWERSAVING
#define PS_EARLY_WAKEUP_INT	(1<<20)
#define RX_SLEEP_ALLOW_INT		(1<<19)
#define PS_WAKEUP_INT			(1<<18)
#define PS_SLEEP_INT			(1<<17)
#endif
#define RPT_OVERINTVL_INT		(1<<15)
#define MPCP_TIMEOUT_INT		(1<<14)
#define TIMEDRFT_INT			(1<<13)
#define TOD_1PPS_INT			(1<<12)
#define TOD_UPDT_INT			(1<<11)
#define PTP_MSG_TX_INT			(1<<10)
#define GNT_BUF_OVRRUN_INT	(1<<9)
#define LLID7_RCV_RGST_INT		(1<<8)
#define LLID6_RCV_RGST_INT		(1<<7)
#define LLID5_RCV_RGST_INT		(1<<6)
#define LLID4_RCV_RGST_INT		(1<<5)
#define LLID3_RCV_RGST_INT		(1<<4)
#define LLID2_RCV_RGST_INT		(1<<3)
#define LLID1_RCV_RGST_INT		(1<<2)
#define LLID0_RCV_RGST_INT		(1<<1)
#define RCV_DSCVRY_GATE_INT	(1<<0)

// ---e_llid0_dscvry_sts  -- e_llid7_dscvry_sts
typedef  union
{
  struct
  {
    __u32  llidDscvrySts: 2;//  LLID0_DSCVRY_STS
    __u32 reserved: 4;//29:26		Reserved
    __u32 rgstrFlgSts:2;//25:24 RGSTR_FLG_STS
    __u32  reserved1:7;   //23:17		Reserved
    __u32  llidValid:1;//16		 LLID0_VALID
    __u32 llidValue:16;//15:0 	LLID0_VALUE
  } Bits;
  __u32 Raw;
} eponLlidDscvStsReg_t;	//REG_e_llid0_dscvry_sts


//e_rpt_mpcp_timeout_llid_idx
typedef  union
{
  struct
  {
    __u8  reserve1;
    __u8  reserve2;
    __u8  mpcpTmoutLlid;
    __u8  rptOverIntvlLlid;
  } Bits;
  __u32 Raw;
} eponRptMpcpLlidReg_t;	//REG_e_rpt_mpcp_timeout_llid_idx

#ifdef TCSUPPORT_CPU_EN7521
typedef struct {
	__u8 			channel ;
	__u8			queue ;
	__u8			thrIdx ;
	__u16			value ;
} eponQueueThreshold_t ;

#define EPON_QTHRESHLD_RWCMD						(1<<31)
#define EPON_QTHRESHLD_DONE							(1<<30)

#define EPON_QTHRESHLD_VALUE_SHIFT					(8)
#define EPON_QTHRESHLD_VALUE_MASK					(0xFFFF<<EPON_QTHRESHLD_VALUE_SHIFT)
#define EPON_QTHRESHLD_TYPE_SHIFT					(6)
#define EPON_QTHRESHLD_TYPE_MASK					(0x3<<EPON_QTHRESHLD_TYPE_SHIFT)
#define EPON_QTHRESHLD_CHANNEL_SHIFT				(3)
#define EPON_QTHRESHLD_CHANNEL_MASK					(0x7<<EPON_QTHRESHLD_CHANNEL_SHIFT)
#define EPON_QTHRESHLD_QUEUE_SHIFT					(0)
#define EPON_QTHRESHLD_QUEUE_MASK					(0x7<<EPON_QTHRESHLD_QUEUE_SHIFT)

#define SNIFFER_SP_TAG_SHIFT         (16)
#define SNIFFER_SP_TAG_MASK        (0x3F)

#endif


#if 0
// --- e_llid_dscvry_ctrl ---
typedef  union
{
  struct
  {
    __u32 mpcp_cmd  : 2;  
    __u32 reserved: 13;
    __u32 mpcp_cmd_done:1;
    __u32 reserved2:3;
    __u32 rgstr_ack_flg:1;
    __u32 reserved3:3;
    __u32 rgstr_req_flg:1;
    __u32 reserved4:5;
    __u32 tx_mpcp_llid_idx:3;
  } bits;
  __u32 value;
} REG_e_llid_dscvry_ctrl;


//e_mac_addr_cfg
typedef  union
{
  struct
  {
    __u32 mac_addr_rwcmd:1;
    __u32 reserved0:14;
    __u32 mac_addr_rwcmd_done:1;
    __u32 reserved1:12;
    __u32 mac_addr_llid_indx:3;
    __u32 mac_addr_dw_idx:1;
  } Bits;
  __u32 Raw;
} REG_e_mac_addr_cfg;


//e_security_key_cfg
typedef  union
{
  struct
  {
    __u32 key_rwcmd:1;
    __u32 reserved0:14;
    __u32 key_rwcmd_done:1;
    __u32 reserved1:10;
    __u32 key_llid_index:3;
    __u32 key_idx:1;
	__u32 key_dw_indx:2;
  } Bits;
  __u32 Raw;
} REG_e_security_key_cfg;



/* EPON MAC register define */
//#define REG_EPON_BASE		0xbfae0000//63365
#define REG_EPON_BASE		0xbfb66000//7510 base

#define e_glb_cfg    	(REG_EPON_BASE +0x00 )
#define e_int_status     (REG_EPON_BASE +0x04 )
#define e_int_en		(REG_EPON_BASE +0x08 )
#define e_rpt_mpcp_timeout_llid_idx		(REG_EPON_BASE +0x0C )
#define e_dyinggsp_cfg		(REG_EPON_BASE +0x10 )
#define e_pending_gnt_num		(REG_EPON_BASE +0x14 )
#define e_llid0_3_cfg		 (REG_EPON_BASE +0x20 )
#define e_llid4_7_cfg		(REG_EPON_BASE +0x24 )
#define e_llid_dscvry_ctrl		(REG_EPON_BASE +0x28 )
#define e_llid0_dscvry_sts		(REG_EPON_BASE +0x2c )
#define e_llid1_dscvry_sts		(REG_EPON_BASE +0x30 )
#define e_llid2_dscvry_sts		(REG_EPON_BASE +0x34 )
#define e_llid3_dscvry_sts		(REG_EPON_BASE +0x38 )
#define e_llid4_dscvry_sts		 (REG_EPON_BASE +0x3c )
#define e_llid5_dscvry_sts		(REG_EPON_BASE +0x40 )
#define e_llid6_dscvry_sts		 (REG_EPON_BASE +0x44 )
#define e_llid7_dscvry_sts		(REG_EPON_BASE +0x48 )
#define e_mac_addr_cfg		(REG_EPON_BASE +0x50 )
#define e_mac_addr_value		(REG_EPON_BASE +0x54 )
#define e_security_key_cfg	(REG_EPON_BASE +0x58 )
#define e_key_value			(REG_EPON_BASE +0x5c)

#define e_rpt_data		(REG_EPON_BASE +0x60)
#define e_rpt_len			(REG_EPON_BASE +0x64)

#define e_local_time			(REG_EPON_BASE +0x80 )
#define e_tod_sync_x			(REG_EPON_BASE +0x84 )
#define e_tod_ltncy			(REG_EPON_BASE +0x88 )
#define p2p_tx_tag1			(REG_EPON_BASE +0x8c )
#define p2p_tx_tag2			(REG_EPON_BASE +0x90 )	
#define e_new_tod_p2p_offset_sec_l32		(REG_EPON_BASE +0x94 )
#define e_new_tod_p2p_tod_offset_nsec	(REG_EPON_BASE +0x98 )
#define e_tod_p2p_tod_sec_l32	(REG_EPON_BASE +0x9c )
#define e_tod_p2p_tod_nsec		(REG_EPON_BASE +0xa0 )
#define e_tod_period			(REG_EPON_BASE +0xa4 )
//#define 						(REG_EPON_BASE +0xa8 )
#if 0
#define REG_P2P_TOD_OFFSET_SEC_L32 				(REG_EPON_BASE +0x94 )
#define REG_P2P_TOD_OFFSET_NSEC		(REG_EPON_BASE +0x98 )
#define REG_P2P_TOD_SEC_L32			(REG_EPON_BASE +0x9C )
//#define REG_P2P_TOD_SEC_H16			(REG_EPON_BASE +0xa0 )
#define REG_P2P_TOD_NSEC				(REG_EPON_BASE +0xa0 )
#endif
#define e_txfetch_cfg				(REG_EPON_BASE +0xd0 )
#define e_sync_time				(REG_EPON_BASE +0xd4 )
#define e_tx_cal_cnst				(REG_EPON_BASE +0xd8 )
#define e_laser_onoff_time		(REG_EPON_BASE +0xdc )
#define e_grd_thrshld				(REG_EPON_BASE +0xe0 )
#define e_mpcp_timeout_intvl		(REG_EPON_BASE +0xe4 )
#define e_rpt_timeout_intvl		(REG_EPON_BASE +0xe8 )
#define e_max_future_gnt_time	(REG_EPON_BASE +0xec )
#define e_min_proc_time			(REG_EPON_BASE +0xf0 )
#define e_trx_adjust_time1			(REG_EPON_BASE +0xf4 )
#define e_trx_adjust_time2			(REG_EPON_BASE +0xf8 )
#define e_dbg_prb_sel			(REG_EPON_BASE +0x100 )
#define e_dbg_prb_h32			(REG_EPON_BASE +0x104 )
#define e_dbg_prb_l32			(REG_EPON_BASE +0x108 )
#define e_rxmbi_eth_cnt			(REG_EPON_BASE +0x10C )
#define e_rxmpi_eth_cnt			(REG_EPON_BASE +0x110 )
#define e_txmbi_eth_cnt			(REG_EPON_BASE +0x114 )
#define e_txmpi_eth_cnt			(REG_EPON_BASE +0x118 )
#define e_oam_stat			(REG_EPON_BASE +0x11C )
#define e_mpcp_stat			(REG_EPON_BASE +0x120 )
#define e_mpcp_rgst_stat			(REG_EPON_BASE +0x124 )
#define e_gnt_pending_stat			(REG_EPON_BASE +0x128 )
#define e_gnt_length_stat			(REG_EPON_BASE +0x12C )
#define e_gnt_type_stat			(REG_EPON_BASE +0x130 )
#define e_time_drft_stat			(REG_EPON_BASE +0x134 )
#endif


#endif /* _EPON_REG_H_ */




