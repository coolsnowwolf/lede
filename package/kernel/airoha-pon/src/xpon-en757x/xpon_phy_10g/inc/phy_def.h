#ifndef _PHY_DEF_H_
#define _PHY_DEF_H_
#include <linux/version.h>
#include <linux/mtd/rt_flash.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/jiffies.h>
#include "phy_types.h"
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
#include <linux/time64.h>
#include <linux/ktime.h>
#endif
#include <ecnt_hook/ecnt_hook_pon_phy.h>


// ********************* PON PHY Compile Option Define *****************
#define PHY_DATE_VERSION      			20240912
// PHY_DATE_VERSION		20180118	=>  EN7580_FPGA_final
// PHY_DATE_VERSION		>=20180420	=>  EN7580_ASIC_version
// PHY_DATE_VERSION		>=20190329	=>  EN7580_ASIC_PMA_module-style_version
// PHY_DATE_VERSION		>=20200226	=>  support EN7572_E2, CAN NOT SUPPORT EN7572_E1
// PHY_DATE_VERSION		>=20200609	=>  change XEPON status switch strategy
// PHY_DATE_VERSION		>=20200915	=>  change XGPON ISR status switch strategy. add digital reset when plug out/in
// PHY_DATE_VERSION		>=20210520	=>  support EN7581_FPGA
// PHY_DATE_VERSION		>=20220923	=>  support EN7581_ASIC


#define PHY_VERSION           			25
// PHY_VERSION	0  =>	EN7580_FPGA
// PHY_VERSION	1~12  =>	EN7580_ASIC_non_PMA_module-style
// PHY_VERSION	>=13  =>	EN7580_ASIC_PMA_module-style
// PHY_VERSION  >=14  =>    support EN7581_FPGA
// PHY_VERSION  =15  =>    support EN7572_E1, need workaround 7572_E1 bug
// PHY_VERSION  >=16  =>    support EN7572_E2, CAN NOT SUPPORT EN7572_E1
// PHY_VERSION  >=18  =>    support EN7571+7580 
// PHY_VERSION  >=19  =>    support kernel 5.4
// PHY_VERSION  >=20  =>    support EN7581_FGPA 
// PHY_VERSION  >=21  =>    support EN7581_ASIC 
// PHY_VERSION  >=22  =>    support EN7581 typeB 
// PHY_VERSION  >=23  =>    support EN7583 FGPA 
// PHY_VERSION  >=24  =>    support EN7583 ASIC 
// PHY_VERSION  >=25  =>    support EN7583 ASIC HTMS&HEC

#define PHY_VERSION_NOTE           		"General"


//#define XILINX_SERDES                   1
//#define A60972_SERDES                   0	// for FPGA
//#define ASIC_SERDES					    0	// for ASIC 


#ifdef CONFIG_USE_MT7520_ASIC //julia_20220922
#define XILINX_SERDES                   0
#define A60972_SERDES                   0	// for FPGA
#define ASIC_SERDES					    1	// for ASIC 
#else
#define XILINX_SERDES                   1
#define A60972_SERDES                   0	// for FPGA
#define ASIC_SERDES					    0	// for ASIC 
#endif


// ******************************************************************

#define EN7580_TEST_CODE				1
#define EN7580_EYE_SCAN					0
#define EN7580_HEC_TEST					1
#define EN7580_EFUSE					1	// for ASIC
#define EN7580_FLASH					0

#define EN7581_TEST_CODE				1
#define EN7581_EYE_SCAN					0
#define EN7581_HEC_TEST					1
#define EN7581_EFUSE					0	// for ASIC
#define EN7581_FLASH					0


//#define A60972_FPGA                   0
#define LDDLA_SUPPORT_SET_TX_MODE       1

#define SIF_DEBUG_LEVEL_CONTROL         0

// ********************* EN7580 Internal API  *****************

// SET

#define PHY_SET_XGPON_PROFILE_DELIMITER          0x2001
#define PHY_SET_XGPON_PROFILE_PREAMBLE           0x2002
#define PHY_SET_XGPON_PROFILE_LEN                0x2003
#define PHY_SET_XGPON_LASER_ON_LEN               0x2004
#define PHY_SET_XGPON_LASER_ON_PATTERN           0x2005
#define PHY_SET_ERR_CNT_EN                       0x2006
#define PHY_SET_BIP_CNT_EN                       0x2007
#define PHY_SET_FM_CNT_EN                        0x2008
#define PHY_SET_TX_FEC_EN                        0x2009
#define PHY_SET_PCS_FPGAIF_RESET				 0x200a
#define PHY_SET_PCS_RESET					 	 0x200b


// GET
#define PHY_GET_XGPON_          0x3001




// ********************* EN7580 Internal API  related structure *****************


static inline int PHY_INTERNAL_GET_API(uint cmd_id, void * in_data) 
{
    xpon_phy_api_data_t phy_api_data = {0};
    
    phy_api_data.api_type = XPON_PHY_API_TYPE_PHY_INTERNAL_GET;
    phy_api_data.cmd_id   = cmd_id  ;
    phy_api_data.raw      = in_data ;
	
	pon_phy_api_dispatch(&phy_api_data);

    return phy_api_data.ret;
}

static inline int PHY_INTERNAL_GET(uint cmd_id) 
{
    return PHY_INTERNAL_GET_API(cmd_id, NULL);
}

static inline int PHY_INTERNAL_SET_API(uint cmd_id, void * in_data) 
{
    xpon_phy_api_data_t phy_api_data = {0};
    
    phy_api_data.api_type = XPON_PHY_API_TYPE_PHY_INTERNAL_SET;
    phy_api_data.cmd_id   = cmd_id  ;
    phy_api_data.raw      = in_data ;
	
	pon_phy_api_dispatch(&phy_api_data);

    return phy_api_data.ret;
}

static inline int PHY_INTERNAL_SET(uint cmd_id)
{
    return PHY_INTERNAL_SET_API(cmd_id, NULL);
}

// ******************************************************************

//#define PON_PHY_PERIODIC_POLLING				1
//#define PON_PHY_STATUS_REPORT_BY_POLLING_ONLY	1
//  gpon_act_change_state


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
static inline s64 ktime_to_sec(const ktime_t kt){	return ktime_divns(kt, NSEC_PER_SEC);}
#endif

#define MAX(a, b)          (((a)>(b))?(a):(b))

/* del_timer_sync cannot be used from interrupt context. */
#define PHY_STOP_TIMER(timer) \
			{ \
				if(in_interrupt()) { \
					del_timer(&timer) ; \
				} else {\
					del_timer_sync(&timer) ; \
				} \
			}
//#define PHY_START_TIMER(timer)			{ mod_timer(&timer, (jiffies + ((timer.data*HZ)/1000))) ; }//{ mod_timer(&timer, (jiffies + ((timer.data)/HZ))) ; }
#define PHY_START_TIMER(timer,para)	{ timer.expires = para;mod_timer(&timer, (jiffies + ((timer.expires*HZ)/1000))) ; }//julia_20201017


#ifndef TIMER_FUN_PAAM //julia_20201017
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
#define TIMER_FUN_PAAM unsigned long
#else
#define TIMER_FUN_PAAM struct timer_list *
#endif
#endif



typedef void (*phy_timer_callback)(TIMER_FUN_PAAM); //julia_20201017
int phy_create_timer(struct timer_list *timer, phy_timer_callback callback, unsigned long param); ////julia_20201017
#define PHY_CREATE_TIMER(timer,func,para) phy_create_timer(timer,func,para)//julia_20201027

#define PHY_DEFAULT	                            0xFF

#define ENABLE	1
#define DISABLE	0

#define PHY_TX_LOCK_2_REF						0x1000000
#define PHY_TX_LOCK_2_DATA						0xFEFFFFFF	//by HC 20150522

#define GPON_MAC_RESET	1
#define GPON_MAC_PHY_RESET	2
#define GPON_PHY_TASK_FLAG	3


/*error cnt offset and setting*/
#define PHY_ERR_LATCH							1
#define PHY_ERR_CLR								(1<<1)
#define PHY_BIP_LATCH							(1<<2)
#define PHY_BIP_CLR								(1<<3)
#define PHY_RXFRAME_LATCH						(1<<4)
#define PHY_RXFRAME_CLR 						(1<<5)
#define PHY_ERR_CNT_OFFSET						0
#define PHY_BIP_CNT_OFFSET						1
#define PHY_FRAME_CNT_OFFSET					2
#define PHY_ERRCNT_MASK							0xfffffffe
#define PHY_BIPCNT_MASK							0xfffffffd
#define PHY_FRAMECNT_MASK						0xfffffffb
#define PHY_BER_UNKNOWN							0
/*Tx counter setting*/
#define PHY_TX_CNT_CLR							(1<<2)
#define PHY_TX_CNT_LCH							(1<<3)

//PHY INT mode
#define PHY_INT_EN_MODE_DEFAULT			       	0
#define PHY_INT_EN_MODE_MANUAL					1
#define PHY_INT_EN_MODE_DEBUG					2

//PHY INT type
#define PHY_INT_EN_DISABLE						0
#define PHY_INT_EN_ALL							0xffffffff

// #define PON_PHY_INT_NUM							(27+1)              //clash with XPON_PHY_INTR in tc3182_int_source.h 

/*Tx Burst mode*/
#define PHY_TX_CONT_MODE_MASK					0xffffff5f
#define PHY_TX_CONT_MODE_OFFSET					0xa0
#define PHY_TX_EPON_CONT_MODE					0xffffffdf

/*PBUS control*/
#define PHY_PBUS_CTRL_DISABLE						(1<<2)

/* PHY RX CDR mode */
#define PHY_RX_CDR_DEFAULT						0x0
#define PHY_RX_CDR_DISABLE						0x1
#define PHY_RX_CDR_ENABLE						0x2

#define PHY_TX_DIS_ON_HW_ONLY                   0x2  //turn on TX_DIS only, not change corresponding sw variable  //tx_dis_reset_protection
#define PHY_TX_DIS_RESTORE_BY_SW                0x3  //restore TX_DIS setting based on saved sw variable          //tx_dis_reset_protection

/*GPON RX control*/
typedef enum {
	PHY_GPON_M1_LMT_EN = 0x01,//set by PHY_GponRxCtl_T->mask to modify gpon_m1_lmt
 	PHY_GPON_M2_LMT_EN = 0x02,//set by PHY_GponRxCtl_T->mask to modify gpon_m2_lmt1_lmt
 	PHY_PRESYNC_MIS_LMT_EN = 0x04,//set by PHY_GponRxCtl_T->mask to modify presync_mismatch_lmt
 	PHY_INSYNC_MIS_LMT_EN = 0x08,//set by PHY_GponRxCtl_T->mask to modify insync_mismatch_lmt
	PHY_GPON_GSYNC_PROT_EN = 0x10//set by PHY_GponRxCtl_T->mask to modify gpon_gsync_prot	
} ENUM_GponRxCtl_T;


typedef struct
{
	unchar 			gpon_m1_lmt; //[3 bits] presync state to insync state after m1 consecutive correct Psync
	unchar 			gpon_m2_lmt; //[3 bits ] insync state to hunt state after m2 consecutive incorrect Psync
	unchar 			presync_mismatch_lmt; //[3 bits ] in presync state, the limit of error bits can be ignored in Psync field 
	unchar 			insync_mismatch_lmt; //[3 bits ] in insync state, the limit of error bits can be ignored in Psync field 
	unchar 			gpon_gsync_prot; //[1 bits ] enforce the Psync value are right during insync state
	uint 			mask;//use ENUM_GponRxCtl_T  to define it
}PHY_GponRxCtl_T,*PPHY_GponRxCtl_T;

/*transceiver alarm threshold*/
typedef struct
{
	ushort 			rx_power_high_alarm_thod;//alarm threshold
	ushort 			rx_power_low_alarm_thod;//alarm threshold
	ushort 			tx_power_high_alarm_thod;//alarm threshold
	ushort 			tx_power_low_alarm_thod;//alarm threshold
}PHY_TransAlarm_T, *PPHY_TransAlarm_T;

//dyma_20170206 add define for EN7580 

typedef struct
{
	UINT32 id;
	char   name[48];
	UINT32 addr;	
	UINT32 def;
	UINT32 cur;
} PHY_REG_ENTRY, *PPHY_REG_ENTRY;  


typedef int (*ponPhyFuncTbl)(char *buf);

typedef enum {
	PHY_RESET_FUNC,
	PHY_DUMP_FUNC,
	PHY_DEVICE_INIT_FUNC,
	PHY_INT_CONFIG_FUNC,
	PHY_GET_API_DISPATCH_FUNC,
	PHY_SET_API_DISPATCH_FUNC,
	PHY_ISR_FUNC,
	PHY_EVENT_POLL_FUNC,
	PHY_DBG_FUNC,
	PHY_PMA_RESET_FUNC,

	PHY_TOTAL_FUNC
}PHY_FUNC_LIST;

typedef enum {
	xpon_10p3125g,
	xpon_9p95328g,
	xpon_2p48832g,
	xpon_1p25g,
	xfi_10p3125g,
	xfi_5p15625g,
	xfi_3p125g,
	xfi_1p25g
}PHY_EYESCAN_MODE_LIST;

typedef enum{
	lcpll_2p48832G_1p24416G=0,
	lcpll_1p25G_1p25G=1,
	lcpll_10p3125G_1p25G=2,
	lcpll_10p3125G_10p3125G=3,
	lcpll_9p95328G_2p48832G=4,
	lcpll_9p95328G_9p95328G=5,
}XPON_PMA_LCPLL_Mode_t;

typedef enum{
	fm_2p48832G_1p24416G=0,
	fm_1p25G_1p25G=1,
	fm_10p3125G_1p25G=2,
	fm_10p3125G_10p3125G=3,
	fm_9p95328G_2p48832G=4,
	fm_9p95328G_9p95328G=5,
}XPON_PMA_FM_Mode_t;

typedef enum{
	RX_Rate_10p3125G=0,
	RX_Rate_9p95328G=1,
	Rx_Rate_2p48832G=2,
	Rx_Rate_1p25G=3

}XPON_PMA_RX_Mode_t;


typedef enum{
	TX_Rate_10p3125G=0,
	TX_Rate_9p95328G=1,
	Tx_Rate_2p48832G=2,
	Tx_Rate_1p25G_RX_1p25G=3,
	Tx_Rate_1p25G_RX_10p3125G=4,
	Tx_Rate_1p24416G=5
}XPON_PMA_TX_Mode_t;


typedef enum{
	PATTERN_DISABLE=0,
	PRBS7_PATTERN=1,
	PRBS9_PATTERN=2,
	PRBS15_PATTERN=3,
	PRBS23_PATTERN=4,
	PRBS31_PATTERN=5,
	HFTP_PATTERN=6,
	MFTP_PATTERN=7,
	PROGRAMMED_IF_TX_TEST_PATTERN=16,
	ALL1_PATTERN=17,
	ALL0_PATTERN=18,
	PRBS11_PATTERN=19	
}XPON_PMA_TX_Bist_Pattern_t;

typedef enum{
	PRBS7_ALIGN_PATTERN=1,
	PRBS9_ALIGN_PATTERN=2,
	PRBS15_ALIGN_PATTERN=3,
	PRBS23_ALIGN_PATTERN=4,
	PRBS31_ALIGN_PATTERN=5,
	PRBS11_ALIGN_PATTERN=19	
}XPON_PMA_RX_Bist_Check_t;


typedef enum{
	normal_mode=0,
	t2r_mode=1,
	pma_t2r_mode=2,
	r2t_mode=3,
}XPON_PMA_LoopBack_Path_t;

typedef enum{
	HIGH_ACTIVE=0,
	LOW_ACTIVE=1
}XPON_PMA_Trans_Ben_Active_Level_t;


typedef struct {
    XPON_PMA_LCPLL_Mode_t				LCPLL_Mode;
	XPON_PMA_FM_Mode_t					FM_Mode;	//added by ASIC period
    XPON_PMA_RX_Mode_t					RX_Mode;
    XPON_PMA_TX_Mode_t					TX_Mode;
    XPON_PMA_TX_Bist_Pattern_t			TX_Bist_Pattern;
    XPON_PMA_LoopBack_Path_t			LoopBack_Path;
	XPON_PMA_Trans_Ben_Active_Level_t	Trans_Ben_Active_Level;
} XPON_PMA_Param_T ;


static inline int phy_dummy_func(char *buf)
{
	printk("phy_dummy_func !\n");
    return 0;
}

#endif /* _PHY_DEF_H_ */
