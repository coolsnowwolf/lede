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

typedef struct eponMacRegHwTest_s{
	__u32 addr;	
	__u32 value;	/*value*/
	__u32 regMask;/*mask*/
}eponMacRegHwTest_t;

#ifdef TCSUPPORT_CPU_ARMV8
#define READ_FE_REG(reg)              get_frame_engine_data(reg)
#define WRITE_FE_REG(reg, val)        set_frame_engine_data(reg,val)
#define READ_REG_WORD(reg)            get_xpon_data(reg)
#define WRITE_REG_WORD(reg, val)      set_xpon_data(reg,val)
#else
#define READ_FE_REG(reg)              ioread32((void __iomem *)(reg))
#define WRITE_FE_REG(reg, val)        iowrite32(val, (void __iomem *)(reg))
#define READ_REG_WORD(reg)            ioread32((void __iomem *)(reg)) /* GetReg((uint)reg) */
#define WRITE_REG_WORD(reg, val)      iowrite32(val, (void __iomem *)(reg)) /* SetReg((uint)reg, val) */
#endif
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
#ifdef __LITTLE_ENDIAN
    __u32 llidValue:16;//15:0  LLID0_VALUE
    __u32  llidValid:1;//16 	 LLID0_VALID
    __u32  reserved1:7;   //23:17		Reserved
    __u32 rgstrFlgSts:2;//25:24 RGSTR_FLG_STS
    __u32 reserved: 4;//29:26		Reserved
    __u32  llidDscvrySts: 2;//	LLID0_DSCVRY_STS
#else		
    __u32  llidDscvrySts: 2;//  LLID0_DSCVRY_STS
    __u32 reserved: 4;//29:26		Reserved
    __u32 rgstrFlgSts:2;//25:24 RGSTR_FLG_STS
    __u32  reserved1:7;   //23:17		Reserved
    __u32  llidValid:1;//16		 LLID0_VALID
    __u32 llidValue:16;//15:0 	LLID0_VALUE
#endif 
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

#endif /* _EPON_REG_H_ */




