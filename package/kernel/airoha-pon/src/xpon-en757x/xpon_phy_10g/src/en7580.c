//******************************************************************************
//
// Copyright (C) 2017 ECONET
//
// Abstract : EN7580 PON PHY
//
//******************************************************************************

// -- includes -----------------------------------------------------------------
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/delay.h>

#include <linux/timer.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36) 
#include <linux/time64.h>
#endif

#include "phy_global.h"
#include "phy_types.h"
#include "phy_debug.h"
#include "phy.h"
#include "phy_init.h"
#include "phy_tx.h"
#include "phy_reg.h"
#include "phy_reg_util.h"
#include "a60972_reg.h"
#include "i2c.h"
#include <ecnt_hook/ecnt_hook_pon_phy.h>

#include "en7580.h"
#include "en7580_pma.h"
#include "en7580_reg.h"


#if EN7580_EFUSE
#if defined(TCSUPPORT_CPU_EN7580)
#include <boot/packageInfo.h>
#endif
#endif
// -- defines ------------------------------------------------------------------

#define PHY_REG_READ_WRITE_TEST  0
#define PHY_REG_ALL_ZEROS        0x0
#define PHY_REG_ALL_ONES         0xFFFFFFFF

static uint bist_check_err_cnt;  
static uint bist_err_cnt_clear_times;
extern uint pma_hi_rate_opt_val;

#define reversebit(x,y) 		x^=(1<<y)

// -- typedefs -----------------------------------------------------------------


// -- external reference functions--------------------------------------------------


// -- local forward declarations ---------------------------------------------------
#define _I_AM_PMA_INIT_SPLIT_LINE_  
// en7580_pma_func
//===============================================================
int scu_mode_sel(int SCU_Mode);
void SCU_RST(void);
//-----------------------------------------------------------------------

#if ASIC_SERDES
void xpon_rx_bist_recheck_result(uint check_onoff,  uint duration);	//add by david 20180525
int xpon_bist_check_pattern_sel(XPON_PMA_RX_Bist_Check_t align_pattern);	//add by david 20180525
void t2r_rx_bist_check(uint BIST_check_switch, XPON_PMA_RX_Bist_Check_t align_pattern);		//add by david 20180525
void normal_rx_bist_check(uint BIST_check_switch, XPON_PMA_RX_Bist_Check_t align_pattern);		//add by david 20180525
uint rx_bist_check_cnt_calculator (void);	//add by david 20180606
void bist_check_cnt_add(int ms);	//add by david 20190122
#endif
//-----------------------------------------------------------------------
void freq_check (void);	//add by david 20180904
void set_pma_fir(void);	
void get_pma_fir(void);	
void tdc_bw(uint bandwidth_level);	//add by david 20190306
void xpon_pma_param_opt (void);		//add by david 20190315
void pma_fifo_check (uint ncpo_check_cnt);	//add by ang_20190130
//===============================================================
int pma_dbg_reg_dump(void);
int pma_dbg_mode_sel_dump(void);
int pma_dbg_pi_cal_dump(void);
int pma_dbg_rx_cal_dump(void);
int pma_dbg_tx_cal_dump(void);	
//========Subroutines declarations=====================================
#define _I_AM_PMA_INIT_SPLIT_LINE_   


// en7580_gepon_func
int en7580_gepon_pon_phy_reset(char* buf);
int en7580_gepon_phy_dump(char* buf);
int en7580_gepon_phy_dev_init(char* buf);
int en7580_gepon_phy_int_config(char* buf);
int en7580_gepon_phy_get_api_dispatch(char* buf);
int en7580_gepon_phy_set_api_dispatch(char* buf);
int en7580_gepon_phy_isr(char* buf);
int en7580_gepon_phy_event_poll(char* buf);
int en7580_gepon_phy_dbg(char* buf);
int en7580_gepon_phy_pma_reset(char* buf);



// en7580_xgpon_func
int en7580_xgpon_pon_phy_reset(char* buf);
int en7580_xgpon_phy_dump(char* buf);
int en7580_xgpon_phy_dev_init(char* buf);
int en7580_xgpon_phy_int_config(char* buf);
int en7580_xgpon_phy_get_api_dispatch(char* buf);
int en7580_xgpon_phy_set_api_dispatch(char* buf);
int en7580_xgpon_phy_isr(char* buf);
int en7580_xgpon_phy_event_poll(char* buf);
int en7580_xgpon_phy_dbg(char* buf);
int en7580_xgpon_phy_pma_reset(char* buf);




// en7580_xepon_func
int en7580_xepon_pon_phy_reset(char* buf);
int en7580_xepon_phy_dump(char* buf);
int en7580_xepon_phy_dev_init(char* buf);
int en7580_xepon_phy_int_config(char* buf);
int en7580_xepon_phy_get_api_dispatch(char* buf);
int en7580_xepon_phy_set_api_dispatch(char* buf);
int en7580_xepon_phy_isr(char* buf);
int en7580_xepon_phy_event_poll(char* buf);
int en7580_xepon_phy_dbg(char* buf);
int en7580_xepon_phy_pma_reset(char* buf);





// --  variables ----------------------------------------------------------------
#if ASIC_SERDES
XPON_PMA_Param_T xpon_pma_param;
#endif

ponPhyFuncTbl en7580_gepon_func[]={
	en7580_gepon_pon_phy_reset,
	en7580_gepon_phy_dump,
	en7580_gepon_phy_dev_init,
	en7580_gepon_phy_int_config,
	en7580_gepon_phy_get_api_dispatch,
	en7580_gepon_phy_set_api_dispatch,
	en7580_gepon_phy_isr,
	en7580_gepon_phy_event_poll,
	en7580_gepon_phy_dbg,
	en7580_gepon_phy_pma_reset
};

ponPhyFuncTbl en7580_xgpon_func[]={
	en7580_xgpon_pon_phy_reset,
	en7580_xgpon_phy_dump,
	en7580_xgpon_phy_dev_init,
	en7580_xgpon_phy_int_config,
	en7580_xgpon_phy_get_api_dispatch,
	en7580_xgpon_phy_set_api_dispatch,
	en7580_xgpon_phy_isr,
	en7580_xgpon_phy_event_poll,
	en7580_xgpon_phy_dbg,
	en7580_xgpon_phy_pma_reset
};

ponPhyFuncTbl en7580_xepon_func[]={
	en7580_xepon_pon_phy_reset,
	en7580_xepon_phy_dump,
	en7580_xepon_phy_dev_init,
	en7580_xepon_phy_int_config,
	en7580_xepon_phy_get_api_dispatch,
	en7580_xepon_phy_set_api_dispatch,
	en7580_xepon_phy_isr,
	en7580_xepon_phy_event_poll,
	en7580_xepon_phy_dbg,
	en7580_xepon_phy_pma_reset
};

static PHY_REG_ENTRY gepon_phy_reg_all[]={
	//	 id,  name									    ,  addr 									  ,default ,current

    { 0   ,"GEPON_CSR_PHYSET2"                         , GEPON_CSR_PHYSET2                             , 0 , 0 }, 
    { 1   ,"GEPON_CSR_PHYSET3"                         , GEPON_CSR_PHYSET3                             , 0 , 0 }, 
	{ 2   ,"GEPON_CSR_PHYSET5"						   , GEPON_CSR_PHYSET5							   , 0 , 0 },     
    { 3   ,"GEPON_CSR_PHYSET10"                        , GEPON_CSR_PHYSET10                            , 0 , 0 }, 
    { 4   ,"GEPON_CSR_PHYSTA1"                         , GEPON_CSR_PHYSTA1                             , 0 , 0 }, 
    { 5   ,"GEPON_CSR_EPON_SYNC_CTL"                   , GEPON_CSR_EPON_SYNC_CTL                       , 0 , 0 }, 
    { 6   ,"GEPON_CSR_EPON_FRAM_SRH_CTL"               , GEPON_CSR_EPON_FRAM_SRH_CTL                   , 0 , 0 }, 
    { 7   ,"GEPON_CSR_GPON_PSYNC_CTL"                  , GEPON_CSR_GPON_PSYNC_CTL                      , 0 , 0 }, 
    { 8   ,"GEPON_CSR_GPON_INDENT_CTL"                 , GEPON_CSR_GPON_INDENT_CTL                     , 0 , 0 }, 
    { 9   ,"GEPON_CSR_RS_CTL"                          , GEPON_CSR_RS_CTL                              , 0 , 0 }, 
    { 10  ,"GEPON_CSR_PHYRX_TEST_CTL"                  , GEPON_CSR_PHYRX_TEST_CTL                      , 0 , 0 }, 
    { 11  ,"GEPON_CSR_PHYRX_STATUS"                    , GEPON_CSR_PHYRX_STATUS                        , 0 , 0 }, 
    { 12  ,"GEPON_CSR_PHYRX_MISC_TRIG"                 , GEPON_CSR_PHYRX_MISC_TRIG                     , 0 , 0 }, 
    { 13  ,"GEPON_CSR_PHYRX_TEST_DBG_TRIG"             , GEPON_CSR_PHYRX_TEST_DBG_TRIG                 , 0 , 0 }, 
    { 14  ,"GEPON_CSR_PHYRX_EPON_FRAME_ERR"            , GEPON_CSR_PHYRX_EPON_FRAME_ERR                , 0 , 0 }, 
    { 15  ,"GEPON_CSR_RX_SRAM_BIST_STATUS"             , GEPON_CSR_RX_SRAM_BIST_STATUS                 , 0 , 0 }, 
    { 16  ,"GEPON_CSR_XP_ERRCNT_EN"                    , GEPON_CSR_XP_ERRCNT_EN                        , 0 , 0 }, 
    { 17  ,"GEPON_CSR_XP_ERRCNT_CTL"                   , GEPON_CSR_XP_ERRCNT_CTL                       , 0 , 0 }, 
    { 18  ,"GEPON_CSR_ERR_BYTE_CNT"                    , GEPON_CSR_ERR_BYTE_CNT                        , 0 , 0 }, 
    { 19  ,"GEPON_CSR_ERR_CODE_CNT"                    , GEPON_CSR_ERR_CODE_CNT                        , 0 , 0 }, 
    { 20  ,"GEPON_CSR_NOSOL_CODE_CNT"                  , GEPON_CSR_NOSOL_CODE_CNT                      , 0 , 0 }, 
    { 21  ,"GEPON_CSR_RX_CODE_CNT"                     , GEPON_CSR_RX_CODE_CNT                         , 0 , 0 }, 
    { 22  ,"GEPON_CSR_FEC_SECONDS"                     , GEPON_CSR_FEC_SECONDS                         , 0 , 0 }, 
    { 23  ,"GEPON_CSR_BIP_CNT"                         , GEPON_CSR_BIP_CNT                             , 0 , 0 }, 
    { 24  ,"GEPON_CSR_FRAME_CNT_L"                     , GEPON_CSR_FRAME_CNT_L                         , 0 , 0 }, 
    { 25  ,"GEPON_CSR_FRAME_CNT_H"                     , GEPON_CSR_FRAME_CNT_H                         , 0 , 0 }, 
    { 26  ,"GEPON_CSR_LOF_CNT"                         , GEPON_CSR_LOF_CNT                             , 0 , 0 }, 
    { 27  ,"GEPON_CSR_FECDEC_TESTCTL"                  , GEPON_CSR_FECDEC_TESTCTL                      , 0 , 0 }, 
    { 28  ,"GEPON_CSR_FECRS_TESTCTL"                   , GEPON_CSR_FECRS_TESTCTL                       , 0 , 0 }, 
    { 29  ,"GEPON_CSR_FECDEC_CTL"                      , GEPON_CSR_FECDEC_CTL                          , 0 , 0 }, 
    { 30  ,"GEPON_CSR_FECDEC_SRAMCTL"                  , GEPON_CSR_FECDEC_SRAMCTL                      , 0 , 0 }, 
    { 31  ,"GEPON_CSR_SRAM_TEST_RX_CTRL"               , GEPON_CSR_SRAM_TEST_RX_CTRL                   , 0 , 0 }, 
    { 32  ,"GEPON_CSR_SRAM_TEST_RX_WR_DATA"            , GEPON_CSR_SRAM_TEST_RX_WR_DATA                , 0 , 0 }, 
    { 33  ,"GEPON_CSR_SRAM_TEST_RX_RD_DATA"            , GEPON_CSR_SRAM_TEST_RX_RD_DATA                , 0 , 0 }, 
    { 34  ,"GEPON_CSR_SRAM_TEST_RX_RW_TRIG"            , GEPON_CSR_SRAM_TEST_RX_RW_TRIG                , 0 , 0 }, 
    { 35  ,"GEPON_CSR_DUMMY_REG_0290"                  , GEPON_CSR_DUMMY_REG_0290                      , 0 , 0 }, 
    { 36  ,"GEPON_CSR_PHYRX_RESET"                     , GEPON_CSR_PHYRX_RESET                         , 0 , 0 }, 
    { 37  ,"GEPON_CSR_PHYRX_EPON_CNT_CTL"              , GEPON_CSR_PHYRX_EPON_CNT_CTL                  , 0 , 0 }, 
    { 38  ,"GEPON_CSR_PHYRX_EPON_SYNC_STATUS"          , GEPON_CSR_PHYRX_EPON_SYNC_STATUS              , 0 , 0 }, 
    { 39  ,"GEPON_CSR_PRBS_ALIGN_PATTERN"              , GEPON_CSR_PRBS_ALIGN_PATTERN                  , 0 , 0 }, 
    { 40  ,"GEPON_CSR_PRBS_ALIGN_MASK"                 , GEPON_CSR_PRBS_ALIGN_MASK                     , 0 , 0 }, 
    { 41  ,"GEPON_CSR_PRBS_ERR_CNT"                    , GEPON_CSR_PRBS_ERR_CNT                        , 0 , 0 }, 
    { 42  ,"GEPON_CSR_PRBS_LOOPBACK_STATUS"            , GEPON_CSR_PRBS_LOOPBACK_STATUS                , 0 , 0 }, 
    { 43  ,"GEPON_CSR_RPMA_SRAM_CTRL"                  , GEPON_CSR_RPMA_SRAM_CTRL                      , 0 , 0 }, 
    { 44  ,"GEPON_CSR_ROUND_TRIP_DELAY_CTRL"           , GEPON_CSR_ROUND_TRIP_DELAY_CTRL               , 0 , 0 }, 
    { 45  ,"GEPON_CSR_ROUND_TRIP_CAL_MASK_CTRL"        , GEPON_CSR_ROUND_TRIP_CAL_MASK_CTRL            , 0 , 0 }, 
    { 46  ,"GEPON_CSR_ROUND_TRIP_DELAY_VALUE"          , GEPON_CSR_ROUND_TRIP_DELAY_VALUE              , 0 , 0 }, 
    { 47  ,"GEPON_CSR_ROUND_TRIP_DELAY_VALUE_STATIC"   , GEPON_CSR_ROUND_TRIP_DELAY_VALUE_STATIC       , 0 , 0 }, 
    { 48  ,"GEPON_CSR_PSYNC_DET_ALIGN_PHASE"           , GEPON_CSR_PSYNC_DET_ALIGN_PHASE               , 0 , 0 }, 
    { 49  ,"GEPON_CSR_RX_TX_HEAD_TO_HEAD_DELAY"        , GEPON_CSR_RX_TX_HEAD_TO_HEAD_DELAY            , 0 , 0 }, 
    { 50  ,"GEPON_CSR_ROUND_TRIP_CAL_MASK_CTRL_1"      , GEPON_CSR_ROUND_TRIP_CAL_MASK_CTRL_1          , 0 , 0 }, 
    { 51  ,"GEPON_CSR_ROUND_TRIP_DELAY_VALUE_1"        , GEPON_CSR_ROUND_TRIP_DELAY_VALUE_1            , 0 , 0 }, 
    { 52  ,"GEPON_CSR_ROUND_TRIP_DELAY_VALUE_2"        , GEPON_CSR_ROUND_TRIP_DELAY_VALUE_2            , 0 , 0 }, 
    { 53  ,"GEPON_CSR_ROUND_TRIP_DELAY_VALUE_STATIC_1" , GEPON_CSR_ROUND_TRIP_DELAY_VALUE_STATIC_1     , 0 , 0 }, 
    { 54  ,"GEPON_CSR_ROUND_TRIP_DELAY_VALUE_STATIC_2" , GEPON_CSR_ROUND_TRIP_DELAY_VALUE_STATIC_2     , 0 , 0 }, 
    { 55  ,"GEPON_CSR_RX_TX_HEAD_TO_HEAD_DELAY_1"      , GEPON_CSR_RX_TX_HEAD_TO_HEAD_DELAY_1          , 0 , 0 }, 
    { 56  ,"GEPON_CSR_RX_TX_HEAD_TO_HEAD_DELAY_2"      , GEPON_CSR_RX_TX_HEAD_TO_HEAD_DELAY_2          , 0 , 0 }, 
    { 57  ,"GEPON_CSR_PSYNC_UPDATE_CTRL"               , GEPON_CSR_PSYNC_UPDATE_CTRL                   , 0 , 0 }, 
    { 58  ,"GEPON_CSR_PSYNC_FORCE_VALUE"               , GEPON_CSR_PSYNC_FORCE_VALUE                   , 0 , 0 }, 
    { 59  ,"GEPON_CSR_PSYNC_DET_TINMEOUT"              , GEPON_CSR_PSYNC_DET_TINMEOUT                  , 0 , 0 }, 
    { 60  ,"GEPON_CSR_PSYNC_DIFF_TIMER"                , GEPON_CSR_PSYNC_DIFF_TIMER                    , 0 , 0 }, 
    { 61  ,"GEPON_CSR_FOUND_PSYNC_TIMER"               , GEPON_CSR_FOUND_PSYNC_TIMER                   , 0 , 0 }, 
    { 62  ,"GEPON_CSR_ABS_TIMER"                       , GEPON_CSR_ABS_TIMER                           , 0 , 0 }, 
    { 63  ,"GEPON_CSR_PSYNC_DET_CNT"                   , GEPON_CSR_PSYNC_DET_CNT                       , 0 , 0 }, 
    { 64  ,"GEPON_CSR_MGMII_PHY_DELAY"                 , GEPON_CSR_MGMII_PHY_DELAY                     , 0 , 0 }, 
    { 65  ,"GEPON_CSR_RX_FIFO_EMPTY_CNT"               , GEPON_CSR_RX_FIFO_EMPTY_CNT                   , 0 , 0 }, 
    { 66  ,"GEPON_CSR_GPON_PREAMBLE"                   , GEPON_CSR_GPON_PREAMBLE                       , 0 , 0 }, 
    { 67  ,"GEPON_CSR_GPON_DELIMITER_GUARD"            , GEPON_CSR_GPON_DELIMITER_GUARD                , 0 , 0 }, 
    { 68  ,"GEPON_CSR_GPON_EXTENDED_PREAMBLE"          , GEPON_CSR_GPON_EXTENDED_PREAMBLE              , 0 , 0 }, 
    { 69  ,"GEPON_CSR_PHYTX_STATUS"                    , GEPON_CSR_PHYTX_STATUS                        , 0 , 0 }, 
    { 70  ,"GEPON_CSR_EPON_TX_CTL"                     , GEPON_CSR_EPON_TX_CTL                         , 0 , 0 }, 
    { 71  ,"GEPON_CSR_PHYTX_TEST_CTL"                  , GEPON_CSR_PHYTX_TEST_CTL                      , 0 , 0 }, 
    { 72  ,"GEPON_CSR_PHYTX_TEST_TRIG"                 , GEPON_CSR_PHYTX_TEST_TRIG                     , 0 , 0 }, 
    { 73  ,"GEPON_CSR_EPON_LOOPBAK_MODE"               , GEPON_CSR_EPON_LOOPBAK_MODE                   , 0 , 0 }, 
    { 74  ,"GEPON_CSR_PHYTX_FECENC_SW_RST"             , GEPON_CSR_PHYTX_FECENC_SW_RST                 , 0 , 0 }, 
    { 75  ,"GEPON_CSR_PHYTX_MISC"                      , GEPON_CSR_PHYTX_MISC                          , 0 , 0 }, 
    { 76  ,"GEPON_CSR_TX_FRAME_COUNTER"                , GEPON_CSR_TX_FRAME_COUNTER                    , 0 , 0 }, 
    { 77  ,"GEPON_CSR_TX_BURST_COUNTER"                , GEPON_CSR_TX_BURST_COUNTER                    , 0 , 0 }, 
    { 78  ,"GEPON_CSR_TX_FEC_FRAME_COUNTER"            , GEPON_CSR_TX_FEC_FRAME_COUNTER                , 0 , 0 }, 
    { 79  ,"GEPON_CSR_SRAM_TEST_TX_CTRL"               , GEPON_CSR_SRAM_TEST_TX_CTRL                   , 0 , 0 }, 
    { 80  ,"GEPON_CSR_SRAM_TEST_TX_RD_DATA"            , GEPON_CSR_SRAM_TEST_TX_RD_DATA                , 0 , 0 }, 
    { 81  ,"GEPON_CSR_SRAM_TEST_TX_RW_TRIG"            , GEPON_CSR_SRAM_TEST_TX_RW_TRIG                , 0 , 0 }, 
    { 82  ,"GEPON_CSR_FECENC_STATUS"                   , GEPON_CSR_FECENC_STATUS                       , 0 , 0 }, 
    { 83  ,"GEPON_CSR_DUMMY_REG_0460"                  , GEPON_CSR_DUMMY_REG_0460                      , 0 , 0 }, 
    { 84  ,"GEPON_CSR_TX_SRAM_BIST_STATUS"             , GEPON_CSR_TX_SRAM_BIST_STATUS                 , 0 , 0 }, 
    { 85  ,"GEPON_CSR_TX_MULT2"                        , GEPON_CSR_TX_MULT2                            , 0 , 0 }, 
    { 86  ,"GEPON_CSR_BISTCTL_LOOPBACK_SEL"            , GEPON_CSR_BISTCTL_LOOPBACK_SEL                , 0 , 0 }, 
    { 87  ,"GEPON_CSR_BISTCTL_PRBS_TX_EN"              , GEPON_CSR_BISTCTL_PRBS_TX_EN                  , 0 , 0 }, 
    { 88  ,"GEPON_CSR_CID_LENGTH_SEL"                  , GEPON_CSR_CID_LENGTH_SEL                      , 0 , 0 }, 
    { 89  ,"GEPON_CSR_TPMA_SRAM_CTRL"                  , GEPON_CSR_TPMA_SRAM_CTRL                      , 0 , 0 }, 
    { 90  ,"GEPON_CSR_TEST_FRAME_EN"                   , GEPON_CSR_TEST_FRAME_EN                       , 0 , 0 }, 
    { 91  ,"GEPON_CSR_TEST_FRAME_NUM"                  , GEPON_CSR_TEST_FRAME_NUM                      , 0 , 0 }, 
    { 92  ,"GEPON_CSR_TEST_FRAME_MAX_LENGTH"           , GEPON_CSR_TEST_FRAME_MAX_LENGTH               , 0 , 0 }, 
    { 93  ,"GEPON_CSR_TEST_FRAME_MIN_LENGTH"           , GEPON_CSR_TEST_FRAME_MIN_LENGTH               , 0 , 0 }, 
    { 94  ,"GEPON_CSR_TF_PAT_SEL"                      , GEPON_CSR_TF_PAT_SEL                          , 0 , 0 }, 
    { 95  ,"GEPON_CSR_TEST_FRAME_ERR_CNT"              , GEPON_CSR_TEST_FRAME_ERR_CNT                  , 0 , 0 }, 
    { 96  ,"GEPON_CSR_TEST_FRAME_TX_INFO"              , GEPON_CSR_TEST_FRAME_TX_INFO                  , 0 , 0 }, 
    { 97  ,"GEPON_CSR_TEST_FRAME_DONE"                 , GEPON_CSR_TEST_FRAME_DONE                     , 0 , 0 }, 
    { 98  ,"GEPON_CSR_EPON_SPD"                        , GEPON_CSR_EPON_SPD                            , 0 , 0 }, 
    { 99  ,"GEPON_CSR_EPON_PREAMBLE0"                  , GEPON_CSR_EPON_PREAMBLE0                      , 0 , 0 }, 
    { 100 ,"GEPON_CSR_EPON_PREAMBLE1"                  , GEPON_CSR_EPON_PREAMBLE1                      , 0 , 0 }, 
    { 101 ,"GEPON_CSR_EPON_SFD"                        , GEPON_CSR_EPON_SFD                            , 0 , 0 }, 
    { 102 ,"GEPON_CSR_EPON_TF_DST0"                    , GEPON_CSR_EPON_TF_DST0                        , 0 , 0 }, 
    { 103 ,"GEPON_CSR_EPON_TF_DST1"                    , GEPON_CSR_EPON_TF_DST1                        , 0 , 0 }, 
    { 104 ,"GEPON_CSR_EPON_TF_SRC0"                    , GEPON_CSR_EPON_TF_SRC0                        , 0 , 0 },
    { 105 ,"GEPON_CSR_EPON_TF_SRC1"                    , GEPON_CSR_EPON_TF_SRC1                        , 0 , 0 },
    { 106 ,"GEPON_CSR_EPON_TF_LEN_TYPE"                , GEPON_CSR_EPON_TF_LEN_TYPE                    , 0 , 0 },
    { 107 ,"GEPON_CSR_EPON_TF_FIRST_DATA0"             , GEPON_CSR_EPON_TF_FIRST_DATA0                 , 0 , 0 },
    { 108 ,"GEPON_CSR_EPON_TF_FIRST_DATA1"             , GEPON_CSR_EPON_TF_FIRST_DATA1                 , 0 , 0 },
    { 109 ,"GEPON_CSR_EPON_TF_FIRST_DATA2"             , GEPON_CSR_EPON_TF_FIRST_DATA2                 , 0 , 0 },
    { 110 ,"GEPON_CSR_EPON_TF_FIRST_DATA3"             , GEPON_CSR_EPON_TF_FIRST_DATA3                 , 0 , 0 },
    { 111 ,"GEPON_CSR_EPON_TF_FIRST_DATA4"             , GEPON_CSR_EPON_TF_FIRST_DATA4                 , 0 , 0 },
    { 112 ,"GEPON_CSR_EPON_TF_FIRST_DATA5"             , GEPON_CSR_EPON_TF_FIRST_DATA5                 , 0 , 0 },
    { 113 ,"GEPON_CSR_EPON_TF_FIRST_DATA6"             , GEPON_CSR_EPON_TF_FIRST_DATA6                 , 0 , 0 },
    { 114 ,"GEPON_CSR_EPON_TF_FIRST_DATA7"             , GEPON_CSR_EPON_TF_FIRST_DATA7                 , 0 , 0 },
    { 115 ,"GEPON_CSR_XPON_STA"                        , GEPON_CSR_XPON_STA                            , 0 , 0 },
    { 116 ,"GEPON_CSR_GIO1_SETTING"                    , GEPON_CSR_GIO1_SETTING                        , 0 , 0 },
    { 117 ,"GEPON_CSR_GIO2_SETTING"                    , GEPON_CSR_GIO2_SETTING                        , 0 , 0 },
    { 118 ,"GEPON_CSR_XPON_INT_EN"                     , GEPON_CSR_XPON_INT_EN                         , 0 , 0 },
    { 119 ,"GEPON_CSR_XPON_INT_STA_CLR"                , GEPON_CSR_XPON_INT_STA_CLR                    , 0 , 0 },
    { 120 ,"GEPON_CSR_XPON_INT_STA"                    , GEPON_CSR_XPON_INT_STA                        , 0 , 0 },
    { 121 ,"GEPON_CSR_EPON_TF_ASYNC_FIFO"              , GEPON_CSR_EPON_TF_ASYNC_FIFO                  , 0 , 0 } 

};

static PHY_REG_ENTRY xgpon_phy_reg_all[]={
	// id	,	name									,  addr 								,default ,current
	{ 0   , "XGPON_PHY_XG_PON_RX_SYNC_CTRL" 			,XGPON_PHY_XG_PON_RX_SYNC_CTRL				, 0 , 0},
	{ 1   , "XGPON_PHY_XG_PON_TX_CTRL"					,XGPON_PHY_XG_PON_TX_CTRL					, 0 , 0},
	{ 2   , "XGPON_PHY_XG_PHY_RST_N"					,XGPON_PHY_XG_PHY_RST_N 					, 0 , 0},
	{ 3   , "XGPON_PHY_XG_PON_INT_STA"					,XGPON_PHY_XG_PON_INT_STA					, 0 , 0},
	{ 4   , "XGPON_PHY_XG_PON_INT_EN"					,XGPON_PHY_XG_PON_INT_EN					, 0 , 0},
	{ 5   , "XGPON_PHY_PREAMBLE1_UPPER" 				,XGPON_PHY_PREAMBLE1_UPPER					, 0 , 0},
	{ 6   , "XGPON_PHY_PREAMBLE1_LOWER" 				,XGPON_PHY_PREAMBLE1_LOWER					, 0 , 0},
	{ 7   , "XGPON_PHY_PREAMBLE2_UPPER" 				,XGPON_PHY_PREAMBLE2_UPPER					, 0 , 0},
	{ 8   , "XGPON_PHY_PREAMBLE2_LOWER" 				,XGPON_PHY_PREAMBLE2_LOWER					, 0 , 0},
	{ 9   , "XGPON_PHY_PREAMBLE3_UPPER" 				,XGPON_PHY_PREAMBLE3_UPPER					, 0 , 0},
	{ 10  , "XGPON_PHY_PREAMBLE3_LOWER" 				,XGPON_PHY_PREAMBLE3_LOWER					, 0 , 0},
	{ 11  , "XGPON_PHY_PREAMBLE4_UPPER" 				,XGPON_PHY_PREAMBLE4_UPPER					, 0 , 0},
	{ 12  , "XGPON_PHY_PREAMBLE4_LOWER" 				,XGPON_PHY_PREAMBLE4_LOWER					, 0 , 0},
	{ 13  , "XGPON_PHY_DELIMITER1_UPPER"				,XGPON_PHY_DELIMITER1_UPPER 				, 0 , 0},
	{ 14  , "XGPON_PHY_DELIMITER1_LOWER"				,XGPON_PHY_DELIMITER1_LOWER 				, 0 , 0},
	{ 15  , "XGPON_PHY_DELIMITER2_UPPER"				,XGPON_PHY_DELIMITER2_UPPER 				, 0 , 0},
	{ 16  , "XGPON_PHY_DELIMITER2_LOWER"				,XGPON_PHY_DELIMITER2_LOWER 				, 0 , 0},
	{ 17  , "XGPON_PHY_DELIMITER3_UPPER"				,XGPON_PHY_DELIMITER3_UPPER 				, 0 , 0},
	{ 18  , "XGPON_PHY_DELIMITER3_LOWER"				,XGPON_PHY_DELIMITER3_LOWER 				, 0 , 0},
	{ 19  , "XGPON_PHY_DELIMITER4_UPPER"				,XGPON_PHY_DELIMITER4_UPPER 				, 0 , 0},
	{ 20  , "XGPON_PHY_DELIMITER4_LOWER"				,XGPON_PHY_DELIMITER4_LOWER 				, 0 , 0},
	{ 21  , "XGPON_PHY_XG_TX_FEC_EN_CTRL"				,XGPON_PHY_XG_TX_FEC_EN_CTRL				, 0 , 0},
	{ 22  , "XGPON_PHY_PSBU_INFO1"						,XGPON_PHY_PSBU_INFO1						, 0 , 0},
	{ 23  , "XGPON_PHY_PSBU_INFO2"						,XGPON_PHY_PSBU_INFO2						, 0 , 0},
	{ 24  , "XGPON_PHY_PSBU_INFO3"						,XGPON_PHY_PSBU_INFO3						, 0 , 0},
	{ 25  , "XGPON_PHY_PSBU_INFO4"						,XGPON_PHY_PSBU_INFO4						, 0 , 0},
	{ 26  , "XGPON_PHY_LASER_ON_LEN"					,XGPON_PHY_LASER_ON_LEN 					, 0 , 0},
	{ 27  , "XGPON_PHY_LASER_ON_PATTERN"				,XGPON_PHY_LASER_ON_PATTERN 				, 0 , 0},
	{ 28  , "XGPON_PHY_XG_TX_IDLE_CTRL" 				,XGPON_PHY_XG_TX_IDLE_CTRL					, 0 , 0},
	{ 29  , "XGPON_PHY_XG_CONTINUE_CTRL"				,XGPON_PHY_XG_CONTINUE_CTRL 				, 0 , 0},
	{ 30  , "XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER"	,XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER	, 0 , 0},
	{ 31  , "XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER"	,XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER	, 0 , 0},
	{ 32  , "XGPON_PHY_DBG_CTRL"						,XGPON_PHY_DBG_CTRL 						, 0 , 0},
	{ 33  , "XGPON_PHY_DBG_RX_CW_START_CNT" 			,XGPON_PHY_DBG_RX_CW_START_CNT				, 0 , 0},
	{ 34  , "XGPON_PHY_DBG_RX_CW_END_CNT"				,XGPON_PHY_DBG_RX_CW_END_CNT				, 0 , 0},
	{ 35  , "XGPON_PHY_DBG_RX_FRAME2PHYD_CNT"			,XGPON_PHY_DBG_RX_FRAME2PHYD_CNT			, 0 , 0},
	{ 36  , "XGPON_PHY_DBG_RX_SOF2MAC_CNT"				,XGPON_PHY_DBG_RX_SOF2MAC_CNT				, 0 , 0},
	{ 37  , "XGPON_PHY_DBG_RX_EOF2MAC_CNT"				,XGPON_PHY_DBG_RX_EOF2MAC_CNT				, 0 , 0},
	{ 38  , "XGPON_PHY_DBG_TX_SOF2PHYD_CNT" 			,XGPON_PHY_DBG_TX_SOF2PHYD_CNT				, 0 , 0},
	{ 39  , "XGPON_PHY_DBG_TX_EOF2PHYD_CNT" 			,XGPON_PHY_DBG_TX_EOF2PHYD_CNT				, 0 , 0},
	{ 40  , "XGPON_PHY_DBG_TX_START2PHYA_CNT"			,XGPON_PHY_DBG_TX_START2PHYA_CNT			, 0 , 0},
	{ 41  , "XGPON_PHY_DBG_TX_END2PHYA_CNT" 			,XGPON_PHY_DBG_TX_END2PHYA_CNT				, 0 , 0},
	{ 42  , "XGPON_PHY_DBG_TX_PHY_RDY2MAC_DLY"			,XGPON_PHY_DBG_TX_PHY_RDY2MAC_DLY			, 0 , 0},
	{ 43  , "XGPON_PHY_DBG_PSYNC_MISMATCH_CNT"			,XGPON_PHY_DBG_PSYNC_MISMATCH_CNT			, 0 , 0},
	{ 44  , "XGPON_PHY_DBG_SFC_HEC_ERR_CNT" 			,XGPON_PHY_DBG_SFC_HEC_ERR_CNT				, 0 , 0},
	{ 45  , "XGPON_PHY_DBG_LOF_CNT" 					,XGPON_PHY_DBG_LOF_CNT						, 0 , 0},
	{ 46  , "XGPON_PHY_DBG_RX_SYNC_ST"					,XGPON_PHY_DBG_RX_SYNC_ST					, 0 , 0},
	{ 47  , "XGPON_PHY_DBG_PON_ID_HEC_ERR_CNT"			,XGPON_PHY_DBG_PON_ID_HEC_ERR_CNT			, 0 , 0},
	{ 48  , "XGPON_PHY_FEC_UNCORRECTED_CW_CNT"			,XGPON_PHY_FEC_UNCORRECTED_CW_CNT			, 0 , 0},
	{ 49  , "XGPON_PHY_FEC_CORRECTED_CW_CNT"			,XGPON_PHY_FEC_CORRECTED_CW_CNT 			, 0 , 0},
	{ 50  , "XGPON_PHY_DBG_TX_ERR_FLAG" 				,XGPON_PHY_DBG_TX_ERR_FLAG					, 0 , 0},
	{ 51  , "XGPON_PHY_DBG_XG_PHYD_PROBE_SEL"			,XGPON_PHY_DBG_XG_PHYD_PROBE_SEL			, 0 , 0},
	{ 52  , "XGPON_PHY_DBG_XG_PHYD_UPPER_PROBE" 		,XGPON_PHY_DBG_XG_PHYD_UPPER_PROBE			, 0 , 0},
	{ 53  , "XGPON_PHY_DBG_XG_PHYD_LOWER_PROBE" 		,XGPON_PHY_DBG_XG_PHYD_LOWER_PROBE			, 0 , 0},
	{ 54  , "XGPON_PHY_DBG_PHYA_STA"					,XGPON_PHY_DBG_PHYA_STA 					, 0 , 0},
	{ 55  , "XGPON_PHY_PHYA_INT_STA"					,XGPON_PHY_PHYA_INT_STA 					, 0 , 0},
	{ 56  , "XGPON_PHY_PHYA_INT_EN" 					,XGPON_PHY_PHYA_INT_EN						, 0 , 0},
	{ 57  , "XGPON_PHY_SFP_VLD_LEVEL"					,XGPON_PHY_SFP_VLD_LEVEL					, 0 , 0},
	{ 58  , "XGPON_PHY_SFP_STA" 						,XGPON_PHY_SFP_STA							, 0 , 0},
	{ 59  , "XGPON_PHY_SFP_TX_SD_TH"					,XGPON_PHY_SFP_TX_SD_TH 					, 0 , 0},
	{ 60  , "XGPON_PHY_XG_PHY_STA"						,XGPON_PHY_XG_PHY_STA						, 0 , 0},
	{ 61  , "XGPON_PHY_AFIFO_OUT_D1"					,XGPON_PHY_AFIFO_OUT_D1 					, 0 , 0},
	{ 62  , "XGPON_PHY_AFIFO_OUT_D2"					,XGPON_PHY_AFIFO_OUT_D2 					, 0 , 0},
	{ 63  , "XGPON_PHY_AFIFO_OUT_D3"					,XGPON_PHY_AFIFO_OUT_D3 					, 0 , 0},
	{ 64  , "XGPON_PHY_AFIFO_OUT_D4"					,XGPON_PHY_AFIFO_OUT_D4 					, 0 , 0},
	{ 65  , "XGPON_PHY_AFIFO_OUT_D5"					,XGPON_PHY_AFIFO_OUT_D5 					, 0 , 0},
	{ 66  , "XGPON_PHY_AFIFO_OUT_D6"					,XGPON_PHY_AFIFO_OUT_D6 					, 0 , 0},
	{ 67  , "XGPON_PHY_MAC_TX_D1"						,XGPON_PHY_MAC_TX_D1						, 0 , 0},
	{ 68  , "XGPON_PHY_MAC_TX_D2"						,XGPON_PHY_MAC_TX_D2						, 0 , 0},
	{ 69  , "XGPON_PHY_MAC_TX_D3"						,XGPON_PHY_MAC_TX_D3						, 0 , 0},
	{ 70  , "XGPON_PHY_MAC_TX_D4"						,XGPON_PHY_MAC_TX_D4						, 0 , 0},
	{ 71  , "XGPON_PHY_MAC_TX_D5"						,XGPON_PHY_MAC_TX_D5						, 0 , 0},
	{ 72  , "XGPON_PHY_MAC_TX_D6"						,XGPON_PHY_MAC_TX_D6						, 0 , 0},
	{ 73  , "XGPON_PHY_XG_PON_SERDES_CTR"				,XGPON_PHY_XG_PON_SERDES_CTR				, 0 , 0},
	{ 74  , "XGPON_PHY_DBG_SFC_HEC_1BIT_CORRECT_CNT"	,XGPON_PHY_DBG_SFC_HEC_1BIT_CORRECT_CNT 	, 0 , 0},
	{ 75  , "XGPON_PHY_DBG_SFC_HEC_2BIT_CORRECT_CNT"	,XGPON_PHY_DBG_SFC_HEC_2BIT_CORRECT_CNT 	, 0 , 0},
	{ 76  , "XGPON_PHY_DBG_PONID_HEC_1BIT_CORRECT_CNT"	,XGPON_PHY_DBG_PONID_HEC_1BIT_CORRECT_CNT	, 0 , 0},
	{ 77  , "XGPON_PHY_DBG_PONID_HEC_2BIT_CORRECT_CNT"	,XGPON_PHY_DBG_PONID_HEC_2BIT_CORRECT_CNT	, 0 , 0},
	{ 78  , "XGPON_PHY_DBG_PHYD_DLY_STA"				,XGPON_PHY_DBG_PHYD_DLY_STA 				, 0 , 0},
	{ 79  , "XGPON_PHY_TX_BURST_ADJUST" 				,XGPON_PHY_TX_BURST_ADJUST					, 0 , 0},
	{ 80  , "XGPON_PHY_FEC_TOTAL_CW_CNT"				,XGPON_PHY_FEC_TOTAL_CW_CNT 				, 0 , 0},
	{ 81  , "XGPON_PHY_FEC_CORRECTED_BYTE_CNT"			,XGPON_PHY_FEC_CORRECTED_BYTE_CNT			, 0 , 0},
	{ 82  , "XGPON_PHY_PSBD_HEC_ERR_CNT"				,XGPON_PHY_PSBD_HEC_ERR_CNT 				, 0 , 0},
	{ 83  , "XGPON_PHY_PHY_PM_CNT_CLR"					,XGPON_PHY_PHY_PM_CNT_CLR					, 0 , 0},
	{ 84  , "XGPON_PHY_PON_ID_UPPER"					,XGPON_PHY_PON_ID_UPPER 					, 0 , 0},
    { 85  , "XGPON_PHY_PON_ID_LOWER"                    ,XGPON_PHY_PON_ID_LOWER                     , 0 , 0},
    { 86  , "XGPON_PHY_FEC_ERR_SECONDS"                 ,XGPON_PHY_FEC_ERR_SECONDS                  , 0 , 0},
    { 87  , "XGPON_PHY_SFP_DEGLITCH_FF_CASCADE"         ,XGPON_PHY_SFP_DEGLITCH_FF_CASCADE          , 0 , 0},
    { 88  , "XGPON_PHY_DBG_SUPPOSED_LASER_ON"           ,XGPON_PHY_DBG_SUPPOSED_LASER_ON            , 0 , 0},
    { 89  , "XGPON_PHY_DBG_ACTUAL_LASER_ON"             ,XGPON_PHY_DBG_ACTUAL_LASER_ON              , 0 , 0},
    { 90  , "XGPON_PHY_DBG_TX_FEC_STA"                  ,XGPON_PHY_DBG_TX_FEC_STA                   , 0 , 0} 

};


static PHY_REG_ENTRY xepon_phy_reg_all[]={
	// id   ,	name										,  addr 										,default ,current
	{ 0   , "XEPON_PCS_TX_SP0"							, XEPON_PCS_TX_SP0							, 0 , 0},
    { 1   , "XEPON_PCS_TX_SP1"                          , XEPON_PCS_TX_SP1                           , 0 , 0},
	{ 2   , "XEPON_PCS_TX_START_BST_D0" 				, XEPON_PCS_TX_START_BST_D0					, 0 , 0},
	{ 3   , "XEPON_PCS_TX_START_BST_D1" 				, XEPON_PCS_TX_START_BST_D1					, 0 , 0},
	{ 4   , "XEPON_PCS_TX_END_BST_D0"					, XEPON_PCS_TX_END_BST_D0					, 0 , 0},
	{ 5   , "XEPON_PCS_TX_END_BST_D1"					, XEPON_PCS_TX_END_BST_D1					, 0 , 0},
	{ 6   , "XEPON_PCS_TX_CTRL_CFG" 					, XEPON_PCS_TX_CTRL_CFG						, 0 , 0},
	{ 7   , "XEPON_PCS_RX_CTRL_CFG" 					, XEPON_PCS_RX_CTRL_CFG						, 0 , 0},
	{ 8   , "XEPON_PCS_INT_STATUS"						, XEPON_PCS_INT_STATUS						, 0 , 0},
	{ 9   , "XEPON_PCS_INT_EN"							, XEPON_PCS_INT_EN							, 0 , 0},
	{ 10  , "XEPON_PCS_CONTINUE_CTRL"					, XEPON_PCS_CONTINUE_CTRL					, 0 , 0},
	{ 11  , "XEPON_PCS_CONTINUE_USER_CFG_DATA0" 		, XEPON_PCS_CONTINUE_USER_CFG_DATA0			, 0 , 0},
	{ 12  , "XEPON_PCS_CONTINUE_USER_CFG_DATA1" 		, XEPON_PCS_CONTINUE_USER_CFG_DATA1			, 0 , 0},
	{ 13  , "XEPON_PCS_LOGIC_RST"						, XEPON_PCS_LOGIC_RST						, 0 , 0},
	{ 14  , "XEPON_PCS_BER_MONITOR_THRE"				, XEPON_PCS_BER_MONITOR_THRE 				, 0 , 0},
	{ 15  , "XEPON_PCS_BER_MONITOR_INTERVAL"			, XEPON_PCS_BER_MONITOR_INTERVAL 			, 0 , 0},
	{ 16  , "XEPON_PCS_BER_START_MONITOR"				, XEPON_PCS_BER_START_MONITOR				, 0 , 0},
	{ 17  , "XEPON_PCS_BER_CNT" 						, XEPON_PCS_BER_CNT							, 0 , 0},
	{ 18  , "XEPON_PCS_BER_6466DEC_ERROR_CNT"			, XEPON_PCS_BER_6466DEC_ERROR_CNT			, 0 , 0},
	{ 19  , "XEPON_PCS_BER_FEC_DEC_ERROR_CNT"			, XEPON_PCS_BER_FEC_DEC_ERROR_CNT			, 0 , 0},
	{ 20  , "XEPON_PCS_CLR_ALL_NUM" 					, XEPON_PCS_CLR_ALL_NUM						, 0 , 0},
	{ 21  , "XEPON_PCS_RX_SOF_NUM"						, XEPON_PCS_RX_SOF_NUM						, 0 , 0},
	{ 22  , "XEPON_PCS_RX_EOF_NUM"						, XEPON_PCS_RX_EOF_NUM						, 0 , 0},
	{ 23  , "XEPON_PCS_TX_MAC_SOF_NUM"					, XEPON_PCS_TX_MAC_SOF_NUM					, 0 , 0},
	{ 24  , "XEPON_PCS_TX_MAC_EOF_NUM"					, XEPON_PCS_TX_MAC_EOF_NUM					, 0 , 0},
	{ 25  , "XEPON_PCS_TX_GB_SOF_NUM"					, XEPON_PCS_TX_GB_SOF_NUM					, 0 , 0},
	{ 26  , "XEPON_PCS_TX_GB_EOF_NUM"					, XEPON_PCS_TX_GB_EOF_NUM					, 0 , 0},
	{ 27  , "XEPON_PCS_RX_SYNC_STATUS"					, XEPON_PCS_RX_SYNC_STATUS					, 0 , 0},
	{ 28  , "XEPON_PCS_PROBE_SEL"						, XEPON_PCS_PROBE_SEL						, 0 , 0},
	{ 29  , "XEPON_PCS_PROBE_H" 						, XEPON_PCS_PROBE_H							, 0 , 0},
	{ 30  , "XEPON_PCS_PROBE_L" 						, XEPON_PCS_PROBE_L							, 0 , 0},
	{ 31  , "XEPON_PCS_TX_TEST_CTRL_CFG"				, XEPON_PCS_TX_TEST_CTRL_CFG 				, 0 , 0},
    { 32  , "XEPON_PCS_RX_TEST_DONE"                    ,XEPON_PCS_RX_TEST_DONE                     , 0 , 0},
    { 33  , "XEPON_PCS_SFP_STATUS"                      ,XEPON_PCS_SFP_STATUS                       , 0 , 0},
    { 34  , "XEPON_PCS_TX_BST_INVLD_DATA"               ,XEPON_PCS_TX_BST_INVLD_DATA                , 0 , 0},
    { 35  , "XEPON_PCS_TX_LASER_ON_PATTERN0"            ,XEPON_PCS_TX_LASER_ON_PATTERN0             , 0 , 0},
    { 36  , "XEPON_PCS_TX_LASER_ON_PATTERN1"            ,XEPON_PCS_TX_LASER_ON_PATTERN1             , 0 , 0},
    { 37  , "XEPON_PCS_TX_LASER_ON_HEAD"                ,XEPON_PCS_TX_LASER_ON_HEAD                 , 0 , 0},
    { 38  , "XEPON_PCS_TX_RX_GB_ALERT_THRE"             ,XEPON_PCS_TX_RX_GB_ALERT_THRE              , 0 , 0},
    { 39  , "XEPON_PCS_TX_EN_DLY_SEL"                   ,XEPON_PCS_TX_EN_DLY_SEL                    , 0 , 0},
    { 40  , "XEPON_PCS_ALL_CDWD"                        ,XEPON_PCS_ALL_CDWD                         , 0 , 0},
    { 41  , "XEPON_PCS_ALL_NO_ERR_CDWD"                 ,XEPON_PCS_ALL_NO_ERR_CDWD                  , 0 , 0}, 
    { 42  , "XEPON_PCS_ALL_ERR_CORR_CDWD"               ,XEPON_PCS_ALL_ERR_CORR_CDWD                , 0 , 0}, 
    { 43  , "XEPON_PCS_ALL_ERR_NO_CORR_CDWD"            ,XEPON_PCS_ALL_ERR_NO_CORR_CDWD             , 0 , 0}, 
    { 44  , "XEPON_PCS_ALL_ERR_BYTE"                    ,XEPON_PCS_ALL_ERR_BYTE                     , 0 , 0}, 
    { 45  , "XEPON_PCS_ALL_ERR_BIT"                     ,XEPON_PCS_ALL_ERR_BIT                      , 0 , 0},
    { 46  , "XEPON_PCS_FEC_ERR_SECOND"                  ,XEPON_PCS_FEC_ERR_SECOND                   , 0 , 0},
    { 47  , "XEPON_PCS_XE_RX_INFO_CSR"                  ,XEPON_PCS_XE_RX_INFO_CSR                   , 0 , 0},
    { 48  , "XEPON_PCS_RX_GB_RD_THRE"                   ,XEPON_PCS_RX_GB_RD_THRE                    , 0 , 0},
    { 49  , "XEPON_PCS_RX_SYNC_OK_CNT"                  ,XEPON_PCS_RX_SYNC_OK_CNT                   , 0 , 0},
    { 50  , "XEPON_PCS_RX_SYNC_LOSS_CNT"                ,XEPON_PCS_RX_SYNC_LOSS_CNT                 , 0 , 0},
    { 51  , "XEPON_PCS_LPBK_TIME_CNT"                   ,XEPON_PCS_LPBK_TIME_CNT                    , 0 , 0},
    { 52  , "XEPON_PCS_MAC_GB_SOB_CNT"                  ,XEPON_PCS_MAC_GB_SOB_CNT                   , 0 , 0},
    { 53  , "XEPON_1G_PHYSET2"                          ,XEPON_1G_PHYSET2                           , 0 , 0},                                          
    { 54  , "XEPON_1G_PHYSET3"                          ,XEPON_1G_PHYSET3                           , 0 , 0},                                                   
    { 55  , "XEPON_1G_PHYSTA1"                          ,XEPON_1G_PHYSTA1                           , 0 , 0},                                                       
    { 56  , "XEPON_1G_EPON_SYNC_CTL"                    ,XEPON_1G_EPON_SYNC_CTL                     , 0 , 0},                                                        
    { 57  , "XEPON_1G_EPON_FRAM_SRH_CTL"                ,XEPON_1G_EPON_FRAM_SRH_CTL                 , 0 , 0},                                                            
    { 58  , "XEPON_1G_GPON_PSYNC_CTL"                   ,XEPON_1G_GPON_PSYNC_CTL                    , 0 , 0},
    { 59  , "XEPON_1G_GPON_INDENT_CTL"                  ,XEPON_1G_GPON_INDENT_CTL                   , 0 , 0},
    { 60  , "XEPON_1G_RS_CTL"                           ,XEPON_1G_RS_CTL                            , 0 , 0},
    { 61  , "XEPON_1G_PHYRX_TEST_CTL"                   ,XEPON_1G_PHYRX_TEST_CTL                    , 0 , 0},
    { 62  , "XEPON_1G_PHYRX_STATUS"                     ,XEPON_1G_PHYRX_STATUS                      , 0 , 0},
    { 63  , "XEPON_1G_PHYRX_MISC_TRIG"                  ,XEPON_1G_PHYRX_MISC_TRIG                   , 0 , 0},
    { 64  , "XEPON_1G_PHYRX_TEST_DBG_TRIG"              ,XEPON_1G_PHYRX_TEST_DBG_TRIG               , 0 , 0},
    { 65  , "XEPON_1G_PHYRX_EPON_FRAME_ERR"             ,XEPON_1G_PHYRX_EPON_FRAME_ERR              , 0 , 0},
    { 66  , "XEPON_1G_RX_SRAM_BIST_STATUS"              ,XEPON_1G_RX_SRAM_BIST_STATUS               , 0 , 0},
    { 67  , "XEPON_1G_XP_ERRCNT_EN"                     ,XEPON_1G_XP_ERRCNT_EN                      , 0 , 0},
    { 68  , "XEPON_1G_XP_ERRCNT_CTL"                    ,XEPON_1G_XP_ERRCNT_CTL                     , 0 , 0},
    { 69  , "XEPON_1G_ERR_BYTE_CNT"                     ,XEPON_1G_ERR_BYTE_CNT                      , 0 , 0},
    { 70  , "XEPON_1G_ERR_CODE_CNT"                     ,XEPON_1G_ERR_CODE_CNT                      , 0 , 0},
    { 71  , "XEPON_1G_NOSOL_CODE_CNT"                   ,XEPON_1G_NOSOL_CODE_CNT                    , 0 , 0},
    { 72  , "XEPON_1G_RX_CODE_CNT"                      ,XEPON_1G_RX_CODE_CNT                       , 0 , 0},
    { 73  , "XEPON_1G_FEC_SECONDS"                      ,XEPON_1G_FEC_SECONDS                       , 0 , 0},
    { 74  , "XEPON_1G_BIP_CNT"                          ,XEPON_1G_BIP_CNT                           , 0 , 0},
    { 75  , "XEPON_1G_FRAME_CNT_L"                      ,XEPON_1G_FRAME_CNT_L                       , 0 , 0},
    { 76  , "XEPON_1G_FRAME_CNT_H"                      ,XEPON_1G_FRAME_CNT_H                       , 0 , 0},
    { 77  , "XEPON_1G_LOF_CNT"                          ,XEPON_1G_LOF_CNT                           , 0 , 0},
    { 78  , "XEPON_1G_FECDEC_TESTCTL"                   ,XEPON_1G_FECDEC_TESTCTL                    , 0 , 0},
    { 79  , "XEPON_1G_FECRS_TESTCTL"                    ,XEPON_1G_FECRS_TESTCTL                     , 0 , 0},
    { 80  , "XEPON_1G_FECDEC_CTL"                       ,XEPON_1G_FECDEC_CTL                        , 0 , 0},
    { 81  , "XEPON_1G_FECDEC_SRAMCTL"                   ,XEPON_1G_FECDEC_SRAMCTL                    , 0 , 0},
    { 82  , "XEPON_1G_SRAM_TEST_RX_CTRL"                ,XEPON_1G_SRAM_TEST_RX_CTRL                 , 0 , 0},
    { 83  , "XEPON_1G_SRAM_TEST_RX_WR_DATA"             ,XEPON_1G_SRAM_TEST_RX_WR_DATA              , 0 , 0},
    { 84  , "XEPON_1G_SRAM_TEST_RX_RD_DATA"             ,XEPON_1G_SRAM_TEST_RX_RD_DATA              , 0 , 0},
    { 85  , "XEPON_1G_SRAM_TEST_RX_RW_TRIG"             ,XEPON_1G_SRAM_TEST_RX_RW_TRIG              , 0 , 0},
    { 86  , "XEPON_1G_DUMMY_REG_0290"                   ,XEPON_1G_DUMMY_REG_0290                    , 0 , 0},
    { 87  , "XEPON_1G_PHYRX_RESET"                      ,XEPON_1G_PHYRX_RESET                       , 0 , 0},
    { 88  , "XEPON_1G_PHYRX_EPON_CNT_CTL"               ,XEPON_1G_PHYRX_EPON_CNT_CTL                , 0 , 0},
    { 89  , "XEPON_1G_PHYRX_EPON_SYNC_STATUS"           ,XEPON_1G_PHYRX_EPON_SYNC_STATUS            , 0 , 0},
    { 90  , "XEPON_1G_PRBS_ALIGN_PATTERN"               ,XEPON_1G_PRBS_ALIGN_PATTERN                , 0 , 0},
    { 91  , "XEPON_1G_PRBS_ALIGN_MASK"                  ,XEPON_1G_PRBS_ALIGN_MASK                   , 0 , 0},
    { 92  , "XEPON_1G_PRBS_ERR_CNT"                     ,XEPON_1G_PRBS_ERR_CNT                      , 0 , 0},
    { 93  , "XEPON_1G_PRBS_LOOPBACK_STATUS"             ,XEPON_1G_PRBS_LOOPBACK_STATUS              , 0 , 0},
    { 94  , "XEPON_1G_RPMA_SRAM_CTRL"                   ,XEPON_1G_RPMA_SRAM_CTRL                    , 0 , 0},
    { 95  , "XEPON_1G_ROUND_TRIP_DELAY_CTRL"            ,XEPON_1G_ROUND_TRIP_DELAY_CTRL             , 0 , 0},
    { 96  , "XEPON_1G_ROUND_TRIP_CAL_MASK_CTRL"         ,XEPON_1G_ROUND_TRIP_CAL_MASK_CTRL          , 0 , 0},
    { 97  , "XEPON_1G_ROUND_TRIP_DELAY_VALUE"           ,XEPON_1G_ROUND_TRIP_DELAY_VALUE            , 0 , 0},
    { 98  , "XEPON_1G_ROUND_TRIP_DELAY_VALUE_STATIC"    ,XEPON_1G_ROUND_TRIP_DELAY_VALUE_STATIC     , 0 , 0},
    { 99  , "XEPON_1G_PSYNC_DET_ALIGN_PHASE"            ,XEPON_1G_PSYNC_DET_ALIGN_PHASE             , 0 , 0},
    { 100 , "XEPON_1G_RX_TX_HEAD_TO_HEAD_DELAY"         ,XEPON_1G_RX_TX_HEAD_TO_HEAD_DELAY          , 0 , 0},
    { 101 , "XEPON_1G_ROUND_TRIP_CAL_MASK_CTRL_1"       ,XEPON_1G_ROUND_TRIP_CAL_MASK_CTRL_1        , 0 , 0},
    { 102 , "XEPON_1G_ROUND_TRIP_DELAY_VALUE_1"         ,XEPON_1G_ROUND_TRIP_DELAY_VALUE_1          , 0 , 0},
    { 103 , "XEPON_1G_ROUND_TRIP_DELAY_VALUE_2"         ,XEPON_1G_ROUND_TRIP_DELAY_VALUE_2          , 0 , 0},
    { 104 , "XEPON_1G_ROUND_TRIP_DELAY_VALUE_STATIC_1"  ,XEPON_1G_ROUND_TRIP_DELAY_VALUE_STATIC_1   , 0 , 0},
    { 105 , "XEPON_1G_ROUND_TRIP_DELAY_VALUE_STATIC_2"  ,XEPON_1G_ROUND_TRIP_DELAY_VALUE_STATIC_2   , 0 , 0},
    { 106 , "XEPON_1G_RX_TX_HEAD_TO_HEAD_DELAY_1"       ,XEPON_1G_RX_TX_HEAD_TO_HEAD_DELAY_1        , 0 , 0},
    { 107 , "XEPON_1G_RX_TX_HEAD_TO_HEAD_DELAY_2"       ,XEPON_1G_RX_TX_HEAD_TO_HEAD_DELAY_2        , 0 , 0},
    { 108 , "XEPON_1G_PSYNC_UPDATE_CTRL"                ,XEPON_1G_PSYNC_UPDATE_CTRL                 , 0 , 0},
    { 109 , "XEPON_1G_PSYNC_FORCE_VALUE"                ,XEPON_1G_PSYNC_FORCE_VALUE                 , 0 , 0},
    { 110 , "XEPON_1G_PSYNC_DET_TINMEOUT"               ,XEPON_1G_PSYNC_DET_TINMEOUT                , 0 , 0},
    { 111 , "XEPON_1G_PSYNC_DIFF_TIMER"                 ,XEPON_1G_PSYNC_DIFF_TIMER                  , 0 , 0},
    { 112 , "XEPON_1G_FOUND_PSYNC_TIMER"                ,XEPON_1G_FOUND_PSYNC_TIMER                 , 0 , 0},
    { 113 , "XEPON_1G_ABS_TIMER"                        ,XEPON_1G_ABS_TIMER                         , 0 , 0},
    { 114 , "XEPON_1G_PSYNC_DET_CNT"                    ,XEPON_1G_PSYNC_DET_CNT                     , 0 , 0},
    { 115 , "XEPON_1G_MGMII_PHY_DELAY"                  ,XEPON_1G_MGMII_PHY_DELAY                   , 0 , 0},
    { 116 , "XEPON_1G_RX_FIFO_EMPTY_CNT"                ,XEPON_1G_RX_FIFO_EMPTY_CNT                 , 0 , 0},
    { 117 , "XEPON_1G_GPON_PREAMBLE"                    ,XEPON_1G_GPON_PREAMBLE                     , 0 , 0},
    { 118 , "XEPON_1G_GPON_DELIMITER_GUARD"             ,XEPON_1G_GPON_DELIMITER_GUARD              , 0 , 0},
    { 119 , "XEPON_1G_GPON_EXTENDED_PREAMBLE"           ,XEPON_1G_GPON_EXTENDED_PREAMBLE            , 0 , 0},
    { 120 , "XEPON_1G_PHYTX_STATUS"                     ,XEPON_1G_PHYTX_STATUS                      , 0 , 0},
    { 121 , "XEPON_1G_EPON_TX_CTL"                      ,XEPON_1G_EPON_TX_CTL                       , 0 , 0},
    { 122 , "XEPON_1G_PHYTX_TEST_CTL"                   ,XEPON_1G_PHYTX_TEST_CTL                    , 0 , 0},
    { 123 , "XEPON_1G_PHYTX_TEST_TRIG"                  ,XEPON_1G_PHYTX_TEST_TRIG                   , 0 , 0},
    { 124 , "XEPON_1G_EPON_LOOPBAK_MODE"                ,XEPON_1G_EPON_LOOPBAK_MODE                 , 0 , 0},
    { 125 , "XEPON_1G_PHYTX_FECENC_SW_RST"              ,XEPON_1G_PHYTX_FECENC_SW_RST               , 0 , 0},
    { 126 , "XEPON_1G_PHYTX_MISC"                       ,XEPON_1G_PHYTX_MISC                        , 0 , 0},
    { 127 , "XEPON_1G_TX_FRAME_COUNTER"                 ,XEPON_1G_TX_FRAME_COUNTER                  , 0 , 0},
    { 128 , "XEPON_1G_TX_BURST_COUNTER"                 ,XEPON_1G_TX_BURST_COUNTER                  , 0 , 0},
    { 129 , "XEPON_1G_TX_FEC_FRAME_COUNTER"             ,XEPON_1G_TX_FEC_FRAME_COUNTER              , 0 , 0},
    { 130 , "XEPON_1G_SRAM_TEST_TX_CTRL"                ,XEPON_1G_SRAM_TEST_TX_CTRL                 , 0 , 0},
    { 131 , "XEPON_1G_SRAM_TEST_TX_RD_DATA"             ,XEPON_1G_SRAM_TEST_TX_RD_DATA              , 0 , 0},
    { 132 , "XEPON_1G_SRAM_TEST_TX_RW_TRIG"             ,XEPON_1G_SRAM_TEST_TX_RW_TRIG              , 0 , 0},
    { 133 , "XEPON_1G_FECENC_STATUS"                    ,XEPON_1G_FECENC_STATUS                     , 0 , 0},
    { 134 , "XEPON_1G_DUMMY_REG_0460"                   ,XEPON_1G_DUMMY_REG_0460                    , 0 , 0},
    { 135 , "XEPON_1G_TX_SRAM_BIST_STATUS"              ,XEPON_1G_TX_SRAM_BIST_STATUS               , 0 , 0},
    { 136 , "XEPON_1G_TX_MULT2"                         ,XEPON_1G_TX_MULT2                          , 0 , 0},
    { 137 , "XEPON_1G_BISTCTL_LOOPBACK_SEL"             ,XEPON_1G_BISTCTL_LOOPBACK_SEL              , 0 , 0},
    { 138 , "XEPON_1G_BISTCTL_PRBS_TX_EN"               ,XEPON_1G_BISTCTL_PRBS_TX_EN                , 0 , 0},
    { 139 , "XEPON_1G_CID_LENGTH_SEL"                   ,XEPON_1G_CID_LENGTH_SEL                    , 0 , 0},
    { 140 , "XEPON_1G_TPMA_SRAM_CTRL"                   ,XEPON_1G_TPMA_SRAM_CTRL                    , 0 , 0},
    { 141 , "XEPON_1G_TEST_FRAME_EN"                    ,XEPON_1G_TEST_FRAME_EN                     , 0 , 0},
    { 142 , "XEPON_1G_TEST_FRAME_NUM"                   ,XEPON_1G_TEST_FRAME_NUM                    , 0 , 0},
    { 143 , "XEPON_1G_TEST_FRAME_MAX_LENGTH"            ,XEPON_1G_TEST_FRAME_MAX_LENGTH             , 0 , 0},
    { 144 , "XEPON_1G_TEST_FRAME_MIN_LENGTH"            ,XEPON_1G_TEST_FRAME_MIN_LENGTH             , 0 , 0},
    { 145 , "XEPON_1G_TF_PAT_SEL"                       ,XEPON_1G_TF_PAT_SEL                        , 0 , 0},
    { 146 , "XEPON_1G_TEST_FRAME_ERR_CNT"               ,XEPON_1G_TEST_FRAME_ERR_CNT                , 0 , 0},
    { 147 , "XEPON_1G_TEST_FRAME_TX_INFO"               ,XEPON_1G_TEST_FRAME_TX_INFO                , 0 , 0},
    { 148 , "XEPON_1G_TEST_FRAME_DONE"                  ,XEPON_1G_TEST_FRAME_DONE                   , 0 , 0},
    { 149 , "XEPON_1G_EPON_SPD"                         ,XEPON_1G_EPON_SPD                          , 0 , 0},
    { 150 , "XEPON_1G_EPON_PREAMBLE0"                   ,XEPON_1G_EPON_PREAMBLE0                    , 0 , 0},
    { 151 , "XEPON_1G_EPON_PREAMBLE1"                   ,XEPON_1G_EPON_PREAMBLE1                    , 0 , 0},
    { 152 , "XEPON_1G_EPON_SFD"                         ,XEPON_1G_EPON_SFD                          , 0 , 0},
    { 153 , "XEPON_1G_EPON_TF_DST0"                     ,XEPON_1G_EPON_TF_DST0                      , 0 , 0},
    { 154 , "XEPON_1G_EPON_TF_DST1"                     ,XEPON_1G_EPON_TF_DST1                      , 0 , 0},
    { 155 , "XEPON_1G_EPON_TF_SRC0"                     ,XEPON_1G_EPON_TF_SRC0                      , 0 , 0},
    { 156 , "XEPON_1G_EPON_TF_SRC1"                     ,XEPON_1G_EPON_TF_SRC1                      , 0 , 0},
    { 157 , "XEPON_1G_EPON_TF_LEN_TYPE"                 ,XEPON_1G_EPON_TF_LEN_TYPE                  , 0 , 0},
    { 158 , "XEPON_1G_EPON_TF_FIRST_DATA0"              ,XEPON_1G_EPON_TF_FIRST_DATA0               , 0 , 0},
    { 159 , "XEPON_1G_EPON_TF_FIRST_DATA1"              ,XEPON_1G_EPON_TF_FIRST_DATA1               , 0 , 0},
    { 160 , "XEPON_1G_EPON_TF_FIRST_DATA2"              ,XEPON_1G_EPON_TF_FIRST_DATA2               , 0 , 0},
    { 161 , "XEPON_1G_EPON_TF_FIRST_DATA3"              ,XEPON_1G_EPON_TF_FIRST_DATA3               , 0 , 0},
    { 162 , "XEPON_1G_EPON_TF_FIRST_DATA4"              ,XEPON_1G_EPON_TF_FIRST_DATA4               , 0 , 0},
    { 163 , "XEPON_1G_EPON_TF_FIRST_DATA5"              ,XEPON_1G_EPON_TF_FIRST_DATA5               , 0 , 0},
    { 164 , "XEPON_1G_EPON_TF_FIRST_DATA6"              ,XEPON_1G_EPON_TF_FIRST_DATA6               , 0 , 0},
    { 165 , "XEPON_1G_EPON_TF_FIRST_DATA7"              ,XEPON_1G_EPON_TF_FIRST_DATA7               , 0 , 0},
    { 166 , "XEPON_1G_XPON_STA"                         ,XEPON_1G_XPON_STA                          , 0 , 0},
    { 167 , "XEPON_1G_GIO1_SETTING"                     ,XEPON_1G_GIO1_SETTING                      , 0 , 0},
    { 168 , "XEPON_1G_GIO2_SETTING"                     ,XEPON_1G_GIO2_SETTING                      , 0 , 0},
    { 169 , "XEPON_1G_XPON_INT_EN"                      ,XEPON_1G_XPON_INT_EN                       , 0 , 0},
    { 170 , "XEPON_1G_XPON_INT_STA_CLR"                 ,XEPON_1G_XPON_INT_STA_CLR                  , 0 , 0},
    { 171 , "XEPON_1G_XPON_INT_STA"                     ,XEPON_1G_XPON_INT_STA                      , 0 , 0},
    { 172 , "XEPON_1G_EPON_TF_ASYNC_FIFO"               ,XEPON_1G_EPON_TF_ASYNC_FIFO                , 0 , 0}
                                                                               
};                                                                             

#if ASIC_SERDES
static PHY_REG_ENTRY xpon_pma_reg_all[]={

	
	{	0	,	"XPON_PMA_RG_XPON_CMN_EN_0" 						,	XPON_PMA_RG_XPON_CMN_EN_0							,	0	,	0	}	,
	{	1	,	"XPON_PMA_RG_XPON_BENTX_BIAS_EN_0"					,	XPON_PMA_RG_XPON_BENTX_BIAS_EN_0					,	0	,	0	}	,
	{	2	,	"XPON_PMA_RG_XPON_RX_MPXSEL_0"						,	XPON_PMA_RG_XPON_RX_MPXSEL_0						,	0	,	0	}	,
	{	3	,	"XPON_PMA_RG_XPON_RX_REV_0_0"						,	XPON_PMA_RG_XPON_RX_REV_0_0 						,	0	,	0	}	,
	{	4	,	"XPON_PMA_RG_XPON_RX_PHYCK_DIV_0"					,	XPON_PMA_RG_XPON_RX_PHYCK_DIV_0 					,	0	,	0	}	,
	{	5	,	"XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0"			,	XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0			,	0	,	0	}	,
	{	6	,	"XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0" 				,	XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0					,	0	,	0	}	,
	{	7	,	"XPON_PMA_RG_XPON_CDR_LPF_SETVALUE_0"				,	XPON_PMA_RG_XPON_CDR_LPF_SETVALUE_0 				,	0	,	0	}	,
	{	8	,	"XPON_PMA_RG_XPON_CDR_PR_INJ_FORCE_OFF_0"			,	XPON_PMA_RG_XPON_CDR_PR_INJ_FORCE_OFF_0 			,	0	,	0	}	,
	{	9	,	"XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0"				,	XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0 				,	0	,	0	}	,
	{	10	,	"XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0"				,	XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0					,	0	,	0	}	,
	{	11	,	"XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0"				,	XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0				,	0	,	0	}	,
	{	12	,	"XPON_PMA_RG_XPON_PLL_IC_0" 						,	XPON_PMA_RG_XPON_PLL_IC_0							,	0	,	0	}	,
	{	13	,	"XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0"				,	XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0					,	0	,	0	}	,
	{	14	,	"XPON_PMA_RG_XPON_PLL_SDM_ORD_0"					,	XPON_PMA_RG_XPON_PLL_SDM_ORD_0						,	0	,	0	}	,
	{	15	,	"XPON_PMA_RG_XPON_PLL_RESERVE0_0"					,	XPON_PMA_RG_XPON_PLL_RESERVE0_0 					,	0	,	0	}	,
	{	16	,	"XPON_PMA_RGS_XPON_PLL_AUTOK_BAND_0"				,	XPON_PMA_RGS_XPON_PLL_AUTOK_BAND_0					,	0	,	0	}	,
	{	17	,	"XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0"				,	XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0 				,	0	,	0	}	,
	{	18	,	"XPON_PMA_RG_XPON_TX_EN_0"							,	XPON_PMA_RG_XPON_TX_EN_0							,	0	,	0	}	,
	{	19	,	"XPON_PMA_RG_XPON_TX_LSDATA_EN_0"					,	XPON_PMA_RG_XPON_TX_LSDATA_EN_0 					,	0	,	0	}	,
	{	20	,	"XPON_PMA_RG_XPON_TX_RESERVED_0"					,	XPON_PMA_RG_XPON_TX_RESERVED_0						,	0	,	0	}	,
	{	21	,	"XPON_PMA_SS_TX_DA_1"								,	XPON_PMA_SS_TX_DA_1 								,	0	,	0	}	,
	{	22	,	"XPON_PMA_SS_LCPLL_PWCTL_SETTING_0" 				,	XPON_PMA_SS_LCPLL_PWCTL_SETTING_0					,	0	,	0	}	,
	{	23	,	"XPON_PMA_SS_LCPLL_PWCTL_SETTING_1" 				,	XPON_PMA_SS_LCPLL_PWCTL_SETTING_1					,	0	,	0	}	,
	{	24	,	"XPON_PMA_SS_LCPLL_PWCTL_SETTING_2" 				,	XPON_PMA_SS_LCPLL_PWCTL_SETTING_2					,	0	,	0	}	,
	{	25	,	"XPON_PMA_SS_LCPLL_PWCTL_DBG_SETTING"				,	XPON_PMA_SS_LCPLL_PWCTL_DBG_SETTING 				,	0	,	0	}	,
	{	26	,	"XPON_PMA_SS_LCPLL_TDC_PW_0"						,	XPON_PMA_SS_LCPLL_TDC_PW_0							,	0	,	0	}	,
	{	27	,	"XPON_PMA_SS_LCPLL_TDC_PW_1"						,	XPON_PMA_SS_LCPLL_TDC_PW_1							,	0	,	0	}	,
	{	28	,	"XPON_PMA_SS_LCPLL_TDC_PW_2"						,	XPON_PMA_SS_LCPLL_TDC_PW_2							,	0	,	0	}	,
	{	29	,	"XPON_PMA_SS_LCPLL_TDC_PW_3"						,	XPON_PMA_SS_LCPLL_TDC_PW_3							,	0	,	0	}	,
	{	30	,	"XPON_PMA_SS_LCPLL_TDC_PW_4"						,	XPON_PMA_SS_LCPLL_TDC_PW_4							,	0	,	0	}	,
	{	31	,	"XPON_PMA_SS_LCPLL_TDC_PW_5"						,	XPON_PMA_SS_LCPLL_TDC_PW_5							,	0	,	0	}	,
	{	32	,	"XPON_PMA_SS_LCPLL_TDC_FLT_0"						,	XPON_PMA_SS_LCPLL_TDC_FLT_0 						,	0	,	0	}	,
	{	33	,	"XPON_PMA_SS_LCPLL_TDC_FLT_1"						,	XPON_PMA_SS_LCPLL_TDC_FLT_1 						,	0	,	0	}	,
	{	34	,	"XPON_PMA_SS_LCPLL_TDC_FLT_2"						,	XPON_PMA_SS_LCPLL_TDC_FLT_2 						,	0	,	0	}	,
	{	35	,	"XPON_PMA_SS_LCPLL_TDC_FLT_3"						,	XPON_PMA_SS_LCPLL_TDC_FLT_3 						,	0	,	0	}	,
	{	36	,	"XPON_PMA_SS_LCPLL_TDC_FLT_4"						,	XPON_PMA_SS_LCPLL_TDC_FLT_4 						,	0	,	0	}	,
	{	37	,	"XPON_PMA_SS_LCPLL_TDC_FLT_5"						,	XPON_PMA_SS_LCPLL_TDC_FLT_5 						,	0	,	0	}	,
	{	38	,	"XPON_PMA_SS_LCPLL_TDC_FLT_6"						,	XPON_PMA_SS_LCPLL_TDC_FLT_6 						,	0	,	0	}	,
	{	39	,	"XPON_PMA_SS_LCPLL_TDC_FLT_7"						,	XPON_PMA_SS_LCPLL_TDC_FLT_7 						,	0	,	0	}	,
	{	40	,	"XPON_PMA_SS_LCPLL_TDC_PCW_1"						,	XPON_PMA_SS_LCPLL_TDC_PCW_1 						,	0	,	0	}	,
	{	41	,	"XPON_PMA_SS_LCPLL_TDC_PCW_2"						,	XPON_PMA_SS_LCPLL_TDC_PCW_2 						,	0	,	0	}	,
	{	42	,	"XPON_PMA_SS_LCPLL_TDC_RO_1"						,	XPON_PMA_SS_LCPLL_TDC_RO_1							,	0	,	0	}	,
	{	43	,	"XPON_PMA_SS_LCPLL_TDC_RO_2"						,	XPON_PMA_SS_LCPLL_TDC_RO_2							,	0	,	0	}	,
	{	44	,	"XPON_PMA_SS_LCPLL_TDC_RO_3"						,	XPON_PMA_SS_LCPLL_TDC_RO_3							,	0	,	0	}	,
	{	45	,	"XPON_PMA_SS_LCPLL_TDC_RO_4"						,	XPON_PMA_SS_LCPLL_TDC_RO_4							,	0	,	0	}	,
	{	46	,	"XPON_PMA_SS_LCPLL_TDC_RO_5"						,	XPON_PMA_SS_LCPLL_TDC_RO_5							,	0	,	0	}	,
	{	47	,	"XPON_PMA_SS_LCPLL_TDC_CTRL_0"						,	XPON_PMA_SS_LCPLL_TDC_CTRL_0						,	0	,	0	}	,
	{	48	,	"XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0"				,	XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0 				,	0	,	0	}	,
	{	49	,	"XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1"				,	XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1 				,	0	,	0	}	,
	{	50	,	"XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2"				,	XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2 				,	0	,	0	}	,
	{	51	,	"XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3"				,	XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3 				,	0	,	0	}	,
	{	52	,	"XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0" 			,	XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0				,	0	,	0	}	,
	{	53	,	"XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1" 			,	XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1				,	0	,	0	}	,
	{	54	,	"XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0" 				,	XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0					,	0	,	0	}	,
	{	55	,	"XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1" 				,	XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1					,	0	,	0	}	,
	{	56	,	"XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2" 				,	XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2					,	0	,	0	}	,
	{	57	,	"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0"					,	XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0					,	0	,	0	}	,
	{	58	,	"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1"					,	XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1					,	0	,	0	}	,
	{	59	,	"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2"					,	XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2					,	0	,	0	}	,
	{	60	,	"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3"					,	XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3					,	0	,	0	}	,
	{	61	,	"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4"					,	XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4					,	0	,	0	}	,
	{	62	,	"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5"					,	XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5					,	0	,	0	}	,
	{	63	,	"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6"					,	XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6					,	0	,	0	}	,
	{	64	,	"XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0" 			,	XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0				,	0	,	0	}	,
	{	65	,	"XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1" 			,	XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1				,	0	,	0	}	,
	{	66	,	"XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0"			,	XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0				,	0	,	0	}	,
	{	67	,	"XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1"			,	XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1				,	0	,	0	}	,
	{	68	,	"XPON_PMA_PHY_EQ_CTRL_0"							,	XPON_PMA_PHY_EQ_CTRL_0								,	0	,	0	}	,
	{	69	,	"XPON_PMA_PHY_EQ_CTRL_1"							,	XPON_PMA_PHY_EQ_CTRL_1								,	0	,	0	}	,
	{	70	,	"XPON_PMA_PHY_EQ_CTRL_2"							,	XPON_PMA_PHY_EQ_CTRL_2								,	0	,	0	}	,
	{	71	,	"XPON_PMA_PHY_EQ_CTRL_3"							,	XPON_PMA_PHY_EQ_CTRL_3								,	0	,	0	}	,
	{	72	,	"XPON_PMA_PHY_EQ_CTRL_4"							,	XPON_PMA_PHY_EQ_CTRL_4								,	0	,	0	}	,
	{	73	,	"XPON_PMA_PHY_EQ_CTRL_5"							,	XPON_PMA_PHY_EQ_CTRL_5								,	0	,	0	}	,
	{	74	,	"XPON_PMA_PHY_EQ_CTRL_6"							,	XPON_PMA_PHY_EQ_CTRL_6								,	0	,	0	}	,
	{	75	,	"XPON_PMA_PHY_EQ_CTRL_7"							,	XPON_PMA_PHY_EQ_CTRL_7								,	0	,	0	}	,
	{	76	,	"XPON_PMA_PHY_EQ_CTRL_8"							,	XPON_PMA_PHY_EQ_CTRL_8								,	0	,	0	}	,
	{	77	,	"XPON_PMA_PHY_EQ_CTRL_9"							,	XPON_PMA_PHY_EQ_CTRL_9								,	0	,	0	}	,
	{	78	,	"XPON_PMA_PHY_EQ_CTRL_10"							,	XPON_PMA_PHY_EQ_CTRL_10 							,	0	,	0	}	,
	{	79	,	"XPON_PMA_SS_RX_FEOS"								,	XPON_PMA_SS_RX_FEOS 								,	0	,	0	}	,
	{	80	,	"XPON_PMA_SS_RX_BLWC"								,	XPON_PMA_SS_RX_BLWC 								,	0	,	0	}	,
	{	81	,	"XPON_PMA_SS_RX_FREQ_DET_1" 						,	XPON_PMA_SS_RX_FREQ_DET_1							,	0	,	0	}	,
	{	82	,	"XPON_PMA_SS_RX_FREQ_DET_2" 						,	XPON_PMA_SS_RX_FREQ_DET_2							,	0	,	0	}	,
	{	83	,	"XPON_PMA_SS_RX_FREQ_DET_3" 						,	XPON_PMA_SS_RX_FREQ_DET_3							,	0	,	0	}	,
	{	84	,	"XPON_PMA_SS_RX_FREQ_DET_4" 						,	XPON_PMA_SS_RX_FREQ_DET_4							,	0	,	0	}	,
	{	85	,	"XPON_PMA_SS_RX_PI_CAL" 							,	XPON_PMA_SS_RX_PI_CAL								,	0	,	0	}	,
	{	86	,	"XPON_PMA_SS_RX_CAL_1"								,	XPON_PMA_SS_RX_CAL_1								,	0	,	0	}	,
	{	87	,	"XPON_PMA_SS_RX_CAL_2"								,	XPON_PMA_SS_RX_CAL_2								,	0	,	0	}	,
	{	88	,	"XPON_PMA_SS_RX_SIGDET_0"							,	XPON_PMA_SS_RX_SIGDET_0 							,	0	,	0	}	,
	{	89	,	"XPON_PMA_SS_RX_SIGDET_1"							,	XPON_PMA_SS_RX_SIGDET_1 							,	0	,	0	}	,
	{	90	,	"XPON_PMA_SS_RX_FLL_0"								,	XPON_PMA_SS_RX_FLL_0								,	0	,	0	}	,
	{	91	,	"XPON_PMA_SS_RX_FLL_1"								,	XPON_PMA_SS_RX_FLL_1								,	0	,	0	}	,
	{	92	,	"XPON_PMA_SS_RX_FLL_2"								,	XPON_PMA_SS_RX_FLL_2								,	0	,	0	}	,
	{	93	,	"XPON_PMA_SS_RX_FLL_3"								,	XPON_PMA_SS_RX_FLL_3								,	0	,	0	}	,
	{	94	,	"XPON_PMA_SS_RX_FLL_4"								,	XPON_PMA_SS_RX_FLL_4								,	0	,	0	}	,
	{	95	,	"XPON_PMA_SS_RX_FLL_5"								,	XPON_PMA_SS_RX_FLL_5								,	0	,	0	}	,
	{	96	,	"XPON_PMA_SS_RX_FLL_6"								,	XPON_PMA_SS_RX_FLL_6								,	0	,	0	}	,
	{	97	,	"XPON_PMA_SS_RX_FLL_7"								,	XPON_PMA_SS_RX_FLL_7								,	0	,	0	}	,
	{	98	,	"XPON_PMA_SS_RX_FLL_8"								,	XPON_PMA_SS_RX_FLL_8								,	0	,	0	}	,
	{	99	,	"XPON_PMA_SS_RX_FLL_9"								,	XPON_PMA_SS_RX_FLL_9								,	0	,	0	}	,
	{	100 ,	"XPON_PMA_SS_RX_FLL_a"								,	XPON_PMA_SS_RX_FLL_a								,	0	,	0	}	,
	{	101 ,	"XPON_PMA_SS_RX_FLL_b"								,	XPON_PMA_SS_RX_FLL_b								,	0	,	0	}	,
	{	102 ,	"XPON_PMA_RX_PDOS_CTRL_0"							,	XPON_PMA_RX_PDOS_CTRL_0 							,	0	,	0	}	,
	{	103 ,	"XPON_PMA_RX_RESET_0"								,	XPON_PMA_RX_RESET_0 								,	0	,	0	}	,
	{	104 ,	"XPON_PMA_RX_RESET_1"								,	XPON_PMA_RX_RESET_1 								,	0	,	0	}	,
	{	105 ,	"XPON_PMA_RX_DEBUG_0"								,	XPON_PMA_RX_DEBUG_0 								,	0	,	0	}	,
	{	106 ,	"XPON_PMA_BISTCTL_CONTROL"							,	XPON_PMA_BISTCTL_CONTROL							,	0	,	0	}	,
	{	107 ,	"XPON_PMA_BISTCTL_ALIGN_PAT"						,	XPON_PMA_BISTCTL_ALIGN_PAT							,	0	,	0	}	,
	{	108 ,	"XPON_PMA_BISTCTL_PROGRAM_PAT_0"					,	XPON_PMA_BISTCTL_PROGRAM_PAT_0						,	0	,	0	}	,
	{	109 ,	"XPON_PMA_BISTCTL_PROGRAM_PAT_1"					,	XPON_PMA_BISTCTL_PROGRAM_PAT_1						,	0	,	0	}	,
	{	110 ,	"XPON_PMA_BISTCTL_POLLUTION"						,	XPON_PMA_BISTCTL_POLLUTION							,	0	,	0	}	,
	{	111 ,	"XPON_PMA_BISTCTL_PRBS_INITIAL_SEED"				,	XPON_PMA_BISTCTL_PRBS_INITIAL_SEED					,	0	,	0	}	,
	{	112 ,	"XPON_PMA_BISTCTL_PRBS_EVENT"						,	XPON_PMA_BISTCTL_PRBS_EVENT 						,	0	,	0	}	,
	{	113 ,	"XPON_PMA_BISTCTL_PRBS_ERRCNT"						,	XPON_PMA_BISTCTL_PRBS_ERRCNT						,	0	,	0	}	,
	{	114 ,	"XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD"				,	XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD				,	0	,	0	}	,
	{	115 ,	"XPON_PMA_RX_TORGS_DEBUG_0" 						,	XPON_PMA_RX_TORGS_DEBUG_0							,	0	,	0	}	,
	{	116 ,	"XPON_PMA_RX_TORGS_DEBUG_1" 						,	XPON_PMA_RX_TORGS_DEBUG_1							,	0	,	0	}	,
	{	117 ,	"XPON_PMA_RX_TORGS_DEBUG_2" 						,	XPON_PMA_RX_TORGS_DEBUG_2							,	0	,	0	}	,
	{	118 ,	"XPON_PMA_RX_TORGS_DEBUG_3" 						,	XPON_PMA_RX_TORGS_DEBUG_3							,	0	,	0	}	,
	{	119 ,	"XPON_PMA_RX_TORGS_DEBUG_4" 						,	XPON_PMA_RX_TORGS_DEBUG_4							,	0	,	0	}	,
	{	120 ,	"XPON_PMA_RX_TORGS_DEBUG_5" 						,	XPON_PMA_RX_TORGS_DEBUG_5							,	0	,	0	}	,
	{	121 ,	"XPON_PMA_RX_TORGS_DEBUG_6" 						,	XPON_PMA_RX_TORGS_DEBUG_6							,	0	,	0	}	,
	{	122 ,	"XPON_PMA_RX_TORGS_DEBUG_7" 						,	XPON_PMA_RX_TORGS_DEBUG_7							,	0	,	0	}	,
	{	123 ,	"XPON_PMA_RX_TORGS_DEBUG_8" 						,	XPON_PMA_RX_TORGS_DEBUG_8							,	0	,	0	}	,
	{	124 ,	"XPON_PMA_RX_TORGS_DEBUG_9" 						,	XPON_PMA_RX_TORGS_DEBUG_9							,	0	,	0	}	,
	{	125 ,	"XPON_PMA_RX_TORGS_DEBUG_10"						,	XPON_PMA_RX_TORGS_DEBUG_10							,	0	,	0	}	,
	{	126 ,	"XPON_PMA_SS_TX_RST_B"								,	XPON_PMA_SS_TX_RST_B								,	0	,	0	}	,
	{	127 ,	"XPON_PMA_SS_TX_CALIB_0"							,	XPON_PMA_SS_TX_CALIB_0								,	0	,	0	}	,
	{	128 ,	"XPON_PMA_SS_TX_CALIB_1"							,	XPON_PMA_SS_TX_CALIB_1								,	0	,	0	}	,
	{	129 ,	"XPON_PMA_SS_TX_CALIB_2"							,	XPON_PMA_SS_TX_CALIB_2								,	0	,	0	}	,
	{	130 ,	"XPON_PMA_XPON_SETTING_0"							,	XPON_PMA_XPON_SETTING_0 							,	0	,	0	}	,
	{	131 ,	"XPON_PMA_XPON_SETTING_1"							,	XPON_PMA_XPON_SETTING_1 							,	0	,	0	}	,
	{	132 ,	"XPON_PMA_XPON_SETTING_2"							,	XPON_PMA_XPON_SETTING_2 							,	0	,	0	}	,
	{	133 ,	"XPON_PMA_XPON_STA" 								,	XPON_PMA_XPON_STA									,	0	,	0	}	,
	{	134 ,	"XPON_PMA_XPON_INT_EN_0"							,	XPON_PMA_XPON_INT_EN_0								,	0	,	0	}	,
	{	135 ,	"XPON_PMA_XPON_INT_EN_1"							,	XPON_PMA_XPON_INT_EN_1								,	0	,	0	}	,
	{	136 ,	"XPON_PMA_XPON_INT_STA_0"							,	XPON_PMA_XPON_INT_STA_0 							,	0	,	0	}	,
	{	137 ,	"XPON_PMA_XPON_INT_STA_1"							,	XPON_PMA_XPON_INT_STA_1 							,	0	,	0	}	,
	{	138 ,	"XPON_PMA_RX_TORGS_DEBUG_11"						,	XPON_PMA_RX_TORGS_DEBUG_11							,	0	,	0	}	,
	{	139 ,	"XPON_PMA_RX_FORCE_MODE_0"							,	XPON_PMA_RX_FORCE_MODE_0							,	0	,	0	}	,
	{	140 ,	"XPON_PMA_RX_FORCE_MODE_1"							,	XPON_PMA_RX_FORCE_MODE_1							,	0	,	0	}	,
	{	141 ,	"XPON_PMA_RX_FORCE_MODE_2"							,	XPON_PMA_RX_FORCE_MODE_2							,	0	,	0	}	,
	{	142 ,	"XPON_PMA_RX_DISB_MODE_0"							,	XPON_PMA_RX_DISB_MODE_0 							,	0	,	0	}	,
	{	143 ,	"XPON_PMA_RX_DISB_MODE_1"							,	XPON_PMA_RX_DISB_MODE_1 							,	0	,	0	}	,
	{	144 ,	"XPON_PMA_RX_DISB_MODE_2"							,	XPON_PMA_RX_DISB_MODE_2 							,	0	,	0	}	,
	{	145 ,	"XPON_PMA_RX_FORCE_MODE_3"							,	XPON_PMA_RX_FORCE_MODE_3							,	0	,	0	}	,
	{	146 ,	"XPON_PMA_RX_FORCE_MODE_4"							,	XPON_PMA_RX_FORCE_MODE_4							,	0	,	0	}	,
	{	147 ,	"XPON_PMA_RX_FORCE_MODE_5"							,	XPON_PMA_RX_FORCE_MODE_5							,	0	,	0	}	,
	{	148 ,	"XPON_PMA_RX_FORCE_MODE_6"							,	XPON_PMA_RX_FORCE_MODE_6							,	0	,	0	}	,
	{	149 ,	"XPON_PMA_RX_DISB_MODE_3"							,	XPON_PMA_RX_DISB_MODE_3 							,	0	,	0	}	,
	{	150 ,	"XPON_PMA_RX_DISB_MODE_4"							,	XPON_PMA_RX_DISB_MODE_4 							,	0	,	0	}	,
	{	151 ,	"XPON_PMA_RX_DISB_MODE_5"							,	XPON_PMA_RX_DISB_MODE_5 							,	0	,	0	}	,
	{	152 ,	"XPON_PMA_RX_FORCE_MODE_7"							,	XPON_PMA_RX_FORCE_MODE_7							,	0	,	0	}	,
	{	153 ,	"XPON_PMA_RX_FORCE_MODE_8"							,	XPON_PMA_RX_FORCE_MODE_8							,	0	,	0	}	,
	{	154 ,	"XPON_PMA_RX_FORCE_MODE_9"							,	XPON_PMA_RX_FORCE_MODE_9							,	0	,	0	}	,
	{	155 ,	"XPON_PMA_RX_DISB_MODE_6"							,	XPON_PMA_RX_DISB_MODE_6 							,	0	,	0	}	,
	{	156 ,	"XPON_PMA_RX_DISB_MODE_7"							,	XPON_PMA_RX_DISB_MODE_7 							,	0	,	0	}	,
	{	157 ,	"XPON_PMA_RX_DISB_MODE_8"							,	XPON_PMA_RX_DISB_MODE_8 							,	0	,	0	}	,
	{	158 ,	"XPON_PMA_SS_BIST_0"								,	XPON_PMA_SS_BIST_0									,	0	,	0	}	,
	{	159 ,	"XPON_PMA_SS_BIST_1"								,	XPON_PMA_SS_BIST_1									,	0	,	0	}	,
	{	160 ,	"XPON_PMA_SS_BIST_2"								,	XPON_PMA_SS_BIST_2									,	0	,	0	}	,
	{	161 ,	"XPON_PMA_SS_DA_XPON_PWDB_0"						,	XPON_PMA_SS_DA_XPON_PWDB_0							,	0	,	0	}	,
	{	162 ,	"XPON_PMA_SS_DA_XPON_PWDB_1"						,	XPON_PMA_SS_DA_XPON_PWDB_1							,	0	,	0	}	,
	{	163 ,	"XPON_PMA_SS_LCPLL_0"								,	XPON_PMA_SS_LCPLL_0 								,	0	,	0	}	,
	{	164 ,	"XPON_PMA_SS_LCPLL_1"								,	XPON_PMA_SS_LCPLL_1 								,	0	,	0	}	,
	{	165 ,	"XPON_PMA_SS_LCPLL_2"								,	XPON_PMA_SS_LCPLL_2 								,	0	,	0	}	,
	{	166 ,	"XPON_PMA_ADD_DIG_RESERVE_0"						,	XPON_PMA_ADD_DIG_RESERVE_0							,	0	,	0	}	,
	{	167 ,	"XPON_PMA_ADD_DIG_RESERVE_1"						,	XPON_PMA_ADD_DIG_RESERVE_1							,	0	,	0	}	,
	{	168 ,	"XPON_PMA_ADD_DIG_RESERVE_2"						,	XPON_PMA_ADD_DIG_RESERVE_2							,	0	,	0	}	,
	{	169 ,	"XPON_PMA_ADD_DIG_RESERVE_3"						,	XPON_PMA_ADD_DIG_RESERVE_3							,	0	,	0	}	,
	{	170 ,	"XPON_PMA_ADD_DIG_RESERVE_4"						,	XPON_PMA_ADD_DIG_RESERVE_4							,	0	,	0	}	,
	{	171 ,	"XPON_PMA_RG_XPON_RX_RESERVED_1"					,	XPON_PMA_RG_XPON_RX_RESERVED_1						,	0	,	0	}	,
	{	172 ,	"XPON_PMA_ADD_DIG_RO_RESERVE_0" 					,	XPON_PMA_ADD_DIG_RO_RESERVE_0						,	0	,	0	}	,
	{	173 ,	"XPON_PMA_ADD_DIG_RO_RESERVE_1" 					,	XPON_PMA_ADD_DIG_RO_RESERVE_1						,	0	,	0	}	,
	{	174 ,	"XPON_PMA_ADD_DIG_RO_RESERVE_2" 					,	XPON_PMA_ADD_DIG_RO_RESERVE_2						,	0	,	0	}	,
	{	175 ,	"XPON_PMA_ADD_DIG_RO_RESERVE_3" 					,	XPON_PMA_ADD_DIG_RO_RESERVE_3						,	0	,	0	}	,
	{	176 ,	"XPON_PMA_ADD_DIG_RO_RESERVE_4" 					,	XPON_PMA_ADD_DIG_RO_RESERVE_4						,	0	,	0	}	,
	{	177 ,	"XPON_PMA_ADD_RX_SYS_EN_SEL_0"						,	XPON_PMA_ADD_RX_SYS_EN_SEL_0						,	0	,	0	}	,
	{	178 ,	"XPON_PMA_PLL_TDC_FREQDET_0"						,	XPON_PMA_PLL_TDC_FREQDET_0							,	0	,	0	}	,
	{	179 ,	"XPON_PMA_PLL_TDC_FREQDET_1"						,	XPON_PMA_PLL_TDC_FREQDET_1							,	0	,	0	}	,
	{	180 ,	"XPON_PMA_PLL_TDC_FREQDET_2"						,	XPON_PMA_PLL_TDC_FREQDET_2							,	0	,	0	}	,
	{	181 ,	"XPON_PMA_PLL_TDC_FREQDET_3"						,	XPON_PMA_PLL_TDC_FREQDET_3							,	0	,	0	}	,
	{	182 ,	"XPON_PMA_DA_XPON_TX_FORCE_0"						,	XPON_PMA_DA_XPON_TX_FORCE_0 						,	0	,	0	}	,
	{	183 ,	"XPON_PMA_DA_XPON_TX_FORCE_1"						,	XPON_PMA_DA_XPON_TX_FORCE_1 						,	0	,	0	}	,
	{	184 ,	"XPON_PMA_DA_XPON_TX_FORCE_2"						,	XPON_PMA_DA_XPON_TX_FORCE_2 						,	0	,	0	}	,
	{	185 ,	"XPON_PMA_RX_FORCE_MODE_10" 						,	XPON_PMA_RX_FORCE_MODE_10							,	0	,	0	}	,
	{	186 ,	"XPON_PMA_ADD_CLKPATH_RST_0"						,	XPON_PMA_ADD_CLKPATH_RST_0							,	0	,	0	}	,
	{	187 ,	"XPON_PMA_ADD_XPON_MODE_1"							,	XPON_PMA_ADD_XPON_MODE_1							,	0	,	0	}	,
	{	188 ,	"XPON_PMA_ADD_R2T_MODE_1"							,	XPON_PMA_ADD_R2T_MODE_1 							,	0	,	0	}	,
	{	189 ,	"XPON_PMA_ADD_T2R_MODE_1"							,	XPON_PMA_ADD_T2R_MODE_1 							,	0	,	0	}	,
	{	190 ,	"XPON_PMA_ADD_LCPLL_RO_1"							,	XPON_PMA_ADD_LCPLL_RO_1 							,	0	,	0	}	,
	{	191 ,	"XPON_PMA_ADD_RO_RX2ANA_1"							,	XPON_PMA_ADD_RO_RX2ANA_1							,	0	,	0	}	,
	{	192 ,	"XPON_PMA_ADD_RO_RX2ANA_2"							,	XPON_PMA_ADD_RO_RX2ANA_2							,	0	,	0	}	,
	{	193 ,	"XPON_PMA_ADD_RO_RX2ANA_3"							,	XPON_PMA_ADD_RO_RX2ANA_3							,	0	,	0	}	,
	{	194 ,	"XPON_PMA_ADD_RO_R2TMODE_1" 						,	XPON_PMA_ADD_RO_R2TMODE_1							,	0	,	0	}	,
	{	195 ,	"XPON_PMA_RG_LCPLL_XPON_CMN_STB_CTRL_0" 			,	XPON_PMA_RG_LCPLL_XPON_CMN_STB_CTRL_0				,	0	,	0	}	,
	{	196 ,	"XPON_PMA_RG_LCPLL_XPON_CMN_STB_CTRL_1" 			,	XPON_PMA_RG_LCPLL_XPON_CMN_STB_CTRL_1				,	0	,	0	}	,
	{	197 ,	"XPON_PMA_RG_TX_HSDATA_EN_EXT_CNT_0"				,	XPON_PMA_RG_TX_HSDATA_EN_EXT_CNT_0					,	0	,	0	}	,
	{	198 ,	"XPON_PMA_PON_TX_COUNTER_0" 						,	XPON_PMA_PON_TX_COUNTER_0							,	0	,	0	}	,
	{	199 ,	"XPON_PMA_PON_TX_COUNTER_1" 						,	XPON_PMA_PON_TX_COUNTER_1							,	0	,	0	}	,
	{	200 ,	"XPON_PMA_PON_TX_COUNTER_2" 						,	XPON_PMA_PON_TX_COUNTER_2							,	0	,	0	}	,
	{	201 ,	"XPON_PMA_PON_TX_COUNTER_3" 						,	XPON_PMA_PON_TX_COUNTER_3							,	0	,	0	}	,
	{	202 ,	"XPON_PMA_PON_CK_SET"								,	XPON_PMA_PON_CK_SET 								,	0	,	0	}	,
	{	203 ,	"XPON_PMA_TX_FIFO_MODE_SEL" 						,	XPON_PMA_TX_FIFO_MODE_SEL							,	0	,	0	}	,
	{	204 ,	"XPON_PMA_XPON_PLL_STB_CNT" 						,	XPON_PMA_XPON_PLL_STB_CNT							,	0	,	0	}	,
	{	205 ,	"XPON_PMA_XPON_PLL_STOP_CNT"						,	XPON_PMA_XPON_PLL_STOP_CNT							,	0	,	0	}	,
	{	206 ,	"XPON_PMA_SW_RST_SET"								,	XPON_PMA_SW_RST_SET 								,	0	,	0	}	,
	{	207 ,	"XPON_PMA_ADD_RO_TX2ANA_1"							,	XPON_PMA_ADD_RO_TX2ANA_1							,	0	,	0	}	,
	{	208 ,	"XPON_PMA_TX_DLY_CTRL"								,	XPON_PMA_TX_DLY_CTRL								,	0	,	0	}	,
	{	209 ,	"XPON_PMA_MEM_WRAPPER_CTRL" 						,	XPON_PMA_MEM_WRAPPER_CTRL							,	0	,	0	}	,
	{	210 ,	"XPON_PMA_XPON_INT_EN_2"							,	XPON_PMA_XPON_INT_EN_2								,	0	,	0	}	,
	{	211 ,	"XPON_PMA_XPON_INT_EN_3"							,	XPON_PMA_XPON_INT_EN_3								,	0	,	0	}	,
	{	212 ,	"XPON_PMA_XPON_INT_STA_2"							,	XPON_PMA_XPON_INT_STA_2 							,	0	,	0	}	,
	{	213 ,	"XPON_PMA_XPON_INT_STA_3"							,	XPON_PMA_XPON_INT_STA_3 							,	0	,	0	}	,
	{	214 ,	"XPON_PMA_XPON_SD_BEHAVIOR_SETTING_0"				,	XPON_PMA_XPON_SD_BEHAVIOR_SETTING_0 				,	0	,	0	}	,
	{	215 ,	"XPON_PMA_XPON_SD_BEHAVIOR_SETTING_1"				,	XPON_PMA_XPON_SD_BEHAVIOR_SETTING_1 				,	0	,	0	}	,
	{	216 ,	"XPON_PMA_XPON_SD_BEHAVIOR_STA" 					,	XPON_PMA_XPON_SD_BEHAVIOR_STA						,	0	,	0	}	,
	{	217 ,	"XPON_PMA_RX_EXTRAL_CTRL"							,	XPON_PMA_RX_EXTRAL_CTRL 							,	0	,	0	}	,
	{	218 ,	"XPON_PMA_RX_LEQ_DISB_CTRL0"						,	XPON_PMA_RX_LEQ_DISB_CTRL0							,	0	,	0	}	,
	{	219 ,	"XPON_PMA_RX_LEQ_DISB_CTRL1"						,	XPON_PMA_RX_LEQ_DISB_CTRL1							,	0	,	0	}	,
	{	220 ,	"XPON_PMA_RX_LEQ_FORCE_CTRL0"						,	XPON_PMA_RX_LEQ_FORCE_CTRL0 						,	0	,	0	}	,
	{	221 ,	"XPON_PMA_RX_LEQ_FORCE_CTRL1"						,	XPON_PMA_RX_LEQ_FORCE_CTRL1 						,	0	,	0	}	,
	{	222 ,	"XPON_PMA_RX_LEQ_ENTIME_CTRL0"						,	XPON_PMA_RX_LEQ_ENTIME_CTRL0						,	0	,	0	}	,
	{	223 ,	"XPON_PMA_RX_LEQ_ENTIME_CTRL1"						,	XPON_PMA_RX_LEQ_ENTIME_CTRL1						,	0	,	0	}	,
	{	224 ,	"XPON_PMA_RX_LEQ_ENTIME_CTRL2"						,	XPON_PMA_RX_LEQ_ENTIME_CTRL2						,	0	,	0	}	,
	{	225 ,	"XPON_PMA_RX_LEQ_CTRL0" 							,	XPON_PMA_RX_LEQ_CTRL0								,	0	,	0	}	,
	{	226 ,	"XPON_PMA_RX_LEQ_CTRL1" 							,	XPON_PMA_RX_LEQ_CTRL1								,	0	,	0	}	,
	{	227 ,	"XPON_PMA_RX_LEQ_CTRL2" 							,	XPON_PMA_RX_LEQ_CTRL2								,	0	,	0	}	,
	{	228 ,	"XPON_PMA_RX_LEQ_RO0"								,	XPON_PMA_RX_LEQ_RO0 								,	0	,	0	}	,
	{	229 ,	"XPON_PMA_RX_LEQ_RO1"								,	XPON_PMA_RX_LEQ_RO1 								,	0	,	0	}	,
	{	230 ,	"XPON_PMA_PLL_FT_FREQDET_0" 						,	XPON_PMA_PLL_FT_FREQDET_0							,	0	,	0	}	,
	{	231 ,	"XPON_PMA_PLL_FT_FREQDET_1" 						,	XPON_PMA_PLL_FT_FREQDET_1							,	0	,	0	}	,
	{	232 ,	"XPON_PMA_PLL_FT_FREQDET_2" 						,	XPON_PMA_PLL_FT_FREQDET_2							,	0	,	0	}	,
	{	233 ,	"XPON_PMA_PLL_FT_FREQDET_3" 						,	XPON_PMA_PLL_FT_FREQDET_3							,	0	,	0	}	,
	{	234 ,	"XPON_PMA_RO_RX_FREQDET"							,	XPON_PMA_RO_RX_FREQDET								,	0	,	0	}	,
	{	235 ,	"XPON_PMA_RO_PLL_FREQDET"							,	XPON_PMA_RO_PLL_FREQDET 							,	0	,	0	}	,
	{	236 ,	"XPON_PMA_RO_PLL_FT_FREQDET"						,	XPON_PMA_RO_PLL_FT_FREQDET							,	0	,	0	}	,
	{	237 ,	"XPON_PMA_RO_PMA_FREQDET"							,	XPON_PMA_RO_PMA_FREQDET 							,	0	,	0	}	,
	{	238 ,	"XPON_PMA_RG_PMA_FREQDET"							,	XPON_PMA_RG_PMA_FREQDET 							,	0	,	0	}	,
	{	239 ,	"XPON_PMA_RG_EXT_BEN_DATA"							,	XPON_PMA_RG_EXT_BEN_DATA							,	0	,	0	}	,
	{	240 ,	"XPON_PMA_RG_PRE_BEN_DATA"							,	XPON_PMA_RG_PRE_BEN_DATA							,	0	,	0	}	,
	{	241 ,	"XPON_PMA_RX_TORGS_DEBUG_12"						,	XPON_PMA_RX_TORGS_DEBUG_12							,	0	,	0	}	,
	{	242 ,	"XPON_PMA_RO_FLL_ADC_0" 							,	XPON_PMA_RO_FLL_ADC_0								,	0	,	0	}	,
	{	243 ,	"XPON_PMA_RO_FLL_ADC_1" 							,	XPON_PMA_RO_FLL_ADC_1								,	0	,	0	}	,
	{	244 ,	"XPON_PMA_RO_FLL_ADC_2" 							,	XPON_PMA_RO_FLL_ADC_2								,	0	,	0	}	,
	{	245 ,	"XPON_PMA_RO_FLL_ADC_3" 							,	XPON_PMA_RO_FLL_ADC_3								,	0	,	0	}	,
	{	246 ,	"XPON_PMA_RO_FLL_ADC_4" 							,	XPON_PMA_RO_FLL_ADC_4								,	0	,	0	}	,
	{	247 ,	"XPON_PMA_RG_AD_XPON_PLL_FT_CK_MON_MUX_SEL" 		,	XPON_PMA_RG_AD_XPON_PLL_FT_CK_MON_MUX_SEL			,	0	,	0	}	,
	{	248 ,	"XPON_PMA_XPON_INT_EN_4"							,	XPON_PMA_XPON_INT_EN_4								,	0	,	0	}	,
	{	249 ,	"XPON_PMA_XPON_INT_STA_4"							,	XPON_PMA_XPON_INT_STA_4 							,	0	,	0	}	,
	{	250 ,	"XPON_PMA_XPON_PMA_AFIFO_REACH_TH"					,	XPON_PMA_XPON_PMA_AFIFO_REACH_TH					,	0	,	0	}	,
	{	251 ,	"XPON_PMA_XPON_MD32_PBUS_CSR_CTRL"					,	XPON_PMA_XPON_MD32_PBUS_CSR_CTRL					,	0	,	0	}	,
	{	252 ,	"XPON_PMA_BENOFF_BIST_SEED" 						,	XPON_PMA_BENOFF_BIST_SEED							,	0	,	0	}	,
	{	253 ,	"XPON_PMA_PWR_PLL_CTRL" 							,	XPON_PMA_PWR_PLL_CTRL								,	0	,	0	}	,
	{	254 ,	"XPON_PMA_XPON_TX_RATE_CTRL"						,	XPON_PMA_XPON_TX_RATE_CTRL							,	0	,	0	}	,
	{	255 ,	"XPON_PMA_BENOFF_DATA0" 							,	XPON_PMA_BENOFF_DATA0								,	0	,	0	}	,
	{	256 ,	"XPON_PMA_BENOFF_DATA1" 							,	XPON_PMA_BENOFF_DATA1								,	0	,	0	}	,
	{	257 ,	"XPON_PMA_BENOFF_DATA2" 							,	XPON_PMA_BENOFF_DATA2								,	0	,	0	}	,
	{	258 ,	"XPON_PMA_BENOFF_DATA3" 							,	XPON_PMA_BENOFF_DATA3								,	0	,	0	}	,
	{	259 ,	"XPON_PMA_BENOFF_CTRL"								,	XPON_PMA_BENOFF_CTRL								,	0	,	0	}	,
	{	260 ,	"XPON_PMA_HS_DATA_EN_SEL"							,	XPON_PMA_HS_DATA_EN_SEL 							,	0	,	0	}	,
	{	261 ,	"XPON_PMA_FIFO_CK_STATUS"							,	XPON_PMA_FIFO_CK_STATUS 							,	0	,	0	}	,
	{	262 ,	"XPON_PMA_TDC_DLF_GAIN_STATUS"						,	XPON_PMA_TDC_DLF_GAIN_STATUS						,	0	,	0	}	,
	{	263 ,	"XPON_PMA_TDC_DLF_MODE_SETTING" 					,	XPON_PMA_TDC_DLF_MODE_SETTING						,	0	,	0	}	,
	{	264 ,	"XPON_PMA_MD32_ISR_CTRL"							,	XPON_PMA_MD32_ISR_CTRL								,	0	,	0	}	,
	{	265 ,	"XPON_PMA_MD32_MEM_CLK_CTRL"						,	XPON_PMA_MD32_MEM_CLK_CTRL							,	0	,	0	}	,
	{	266 ,	"XPON_PMA_MD32_MEM_CLK_CG_CTRL" 					,	XPON_PMA_MD32_MEM_CLK_CG_CTRL						,	0	,	0	}	,
	{	267 ,	"XPON_PMA_XG_EQD_STA"								,	XPON_PMA_XG_EQD_STA 								,	0	,	0	}	,
	{	268 ,	"XPON_PMA_XG_EQD_CTRL"								,	XPON_PMA_XG_EQD_CTRL								,	0	,	0	}	,
	{	269 ,	"XPON_PMA_RAWDATA_DUMP_CTRL_0"						,	XPON_PMA_RAWDATA_DUMP_CTRL_0						,	0	,	0	}	,
	{	270 ,	"XPON_PMA_RAWDATA_DUMP_CTRL_1"						,	XPON_PMA_RAWDATA_DUMP_CTRL_1						,	0	,	0	}	,
	{	271 ,	"XPON_PMA_RAWDATA_DUMP_CTRL_2"						,	XPON_PMA_RAWDATA_DUMP_CTRL_2						,	0	,	0	}	,
	{	272 ,	"XPON_PMA_RAWDATA_DUMP_CTRL_3"						,	XPON_PMA_RAWDATA_DUMP_CTRL_3						,	0	,	0	}	,
	{	273 ,	"XPON_PMA_RAWDATA_DUMP_STA_0"						,	XPON_PMA_RAWDATA_DUMP_STA_0 						,	0	,	0	}	,
	{	274 ,	"XPON_PMA_RAWDATA_DUMP_STA_1"						,	XPON_PMA_RAWDATA_DUMP_STA_1 						,	0	,	0	}	,
	{	275 ,	"XPON_PMA_RAWDATA_DUMP_STA_2"						,	XPON_PMA_RAWDATA_DUMP_STA_2 						,	0	,	0	}	,
	{	276 ,	"XPON_PMA_MD32MEM_DIV2_CNT" 						,	XPON_PMA_MD32MEM_DIV2_CNT							,	0	,	0	}	,
	{	277 ,	"XPON_PMA_AD_XPON_CDR_LPF_SV"						,	XPON_PMA_AD_XPON_CDR_LPF_SV 						,	0	,	0	}	,
	{	278 ,	"XPON_PMA_K_VALUE_RD"								,	XPON_PMA_K_VALUE_RD 								,	0	,	0	}	,
	{	279 ,	"XPON_PMA_XPONPLL_CTRL" 							,	XPON_PMA_XPONPLL_CTRL								,	0	,	0	}	,

};
#endif



#define _definition_SPLIT_LINE_ 
                                                                               
        
// -- functions  ----------------------------------------------------------------

void phy_delay1ms(int ms)
{
#ifdef TCSUPPORT_CPU_ARMV8

	delay1ms(ms);

#else
	volatile uint32 timer_now, timer_last;
	volatile uint32 tick_acc;

	uint32 one_tick_unit = SYS_HCLK * 500;// 1 * SYS_HCLK * 1000 / 2

	volatile uint32 tick_wait = ms * one_tick_unit; 
	volatile uint32 timer1_ldv = regRead32(CR_TIMER1_LDV);

	tick_acc = 0;
	timer_last = regRead32(CR_TIMER1_VLR);
	do {
		timer_now = regRead32(CR_TIMER1_VLR);
	  	if (timer_last >= timer_now) 
	  		tick_acc += timer_last - timer_now;
		else
			tick_acc += timer1_ldv - timer_now + timer_last;
		timer_last = timer_now;
	} while (tick_acc < tick_wait);

#endif	
}

#if ASIC_SERDES  
int pma_dbg_reg_dump(void){
	
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;
	
	phy_reg_total_num=279;
	phy_reg_all=xpon_pma_reg_all;
	
	printk("\r\n%-44s	 address=value\r\n","_____ XPON PMA REG DUMP _____");
	for(i=0;i<=phy_reg_total_num;i++)
	{
		(phy_reg_all+i)->def=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
	}

	return PHY_SUCCESS;
}
int scu_mode_sel(int SCU_Mode){
	//  IO_SPHYA_REG_BITS(0xBFB00098, 3, 3, 0x01);  

	// Step_1  **************** Mode select ****************
	IO_SPHYA_REG_BITS(SCU_WAN_CONF_REG, 31, 0, SCU_Mode);	// bfb00070 default = 0x00000000							
						// IO_SPHYA_REG_BITS(0xBFB00070, 7, 0, 0x7));	

						// 0x00 = GPON	 DS_2.48832G	/ US_1.24416G
						// 0x01 = EPON	 DS_1.25G	/ US_1.25G
						// 0x06 = XEPON  DS_10.3125G	/ US_1.25G
						// 0x07 = XEPON  DS_10.3125G	/ US_10.3125G
						// 0x09 = XGPON  DS_9.95328G	/ US_2.48832G
						// 0x0b = XGSPON DS_9.95328G	/ US_9.95328G						
	// = = = = END Step_1 = = = = = = = = = = = = = = = = = = = = = //
	return 0;
}

void SCU_RST(void){
	// SCU_RST will clean all xpon settings include pma, pcs and mac!! 
	IO_SPHYA_REG_BITS(TOP_RST_CTRL_SW2, 0, 0, TOP_RST_CTRL_SW2_HOLD); //bfb00830[0]  ponphy_sw_rst_n => 1	

	//delay for a while
	udelay(5);
	
	IO_SPHYA_REG_BITS(TOP_RST_CTRL_SW2, 0, 0, TOP_RST_CTRL_SW2_RELEASE); //bfb00830[0]  ponphy_sw_rst_n => 0
}

void pon_phy_clear_rogueonu(void)
{
	uint read_data=0;
	
	//clear rogue onu cnt
	read_data=IO_GPHYREG(XPON_PMA_XPON_SD_BEHAVIOR_STA);
	printk("XPON_PMA_XPON_SD_BEHAVIOR_STA= %x\n", read_data);
	IO_SPHYA_REG_BITS(XPON_PMA_XPON_SD_BEHAVIOR_SETTING_1 , 8 , 8 , 1 );
	IO_SPHYA_REG_BITS(XPON_PMA_XPON_SD_BEHAVIOR_SETTING_1 , 8 , 8 , 0 );
	read_data=IO_GPHYREG(XPON_PMA_XPON_SD_BEHAVIOR_STA);
	printk("XPON_PMA_XPON_SD_BEHAVIOR_STA= %x\n", read_data);

	//clear rogue onu int
	read_data=IO_GPHYREG(XPON_PMA_XPON_INT_STA_0);
	printk("XPON_PMA_XPON_INT_STA_0= %x\n", read_data);
	IO_SPHYREG(XPON_PMA_XPON_INT_STA_0, read_data);
	read_data=IO_GPHYREG(XPON_PMA_XPON_INT_STA_0);
	printk("XPON_PMA_XPON_INT_STA_0= %x\n", read_data);


}

void pon_phy_rogueonu_int_en(unchar rogue_int_en)
{	
	uint read_data=0, write_data=0;


	read_data = IO_GPHYREG(XPON_PMA_XPON_INT_EN_0);
	PON_PHY_PRINT(PHY_MSG_TRACE,"XPON_PMA_XPON_INT_EN_0 read     :0x%.8x\n", read_data);

	if(PHY_ENABLE == rogue_int_en)
	{
		write_data = read_data | XPON_PMA_TRANS_ROGUE_ONU_INT_EN;
		IO_SPHYREG(XPON_PMA_XPON_INT_EN_0, write_data);
		read_data = IO_GPHYREG(XPON_PMA_XPON_INT_EN_0);
		PON_PHY_PRINT(PHY_MSG_TRACE,"XGPON XPON_PMA_XPON_INT_EN_0 write    :0x%.8x\n", read_data);	
	}
	else
	{
		write_data = read_data & (~(XPON_PMA_TRANS_ROGUE_ONU_INT_EN));
		IO_SPHYREG(XPON_PMA_XPON_INT_EN_0, write_data);
		read_data = IO_GPHYREG(XPON_PMA_XPON_INT_EN_0);
		PON_PHY_PRINT(PHY_MSG_TRACE,"XPON_PMA_XPON_INT_EN_0 write    :0x%.8x\n", read_data);	
	}


}

#endif


int en7580_get_wan_sel(void)  // (ponPhyFuncTbl** pPonPhyFunc)   * pPonPhyFunc=mt752x_func
{

	uint read_data = 0;
	uint write_data = 0;
	
#ifdef TCSUPPORT_CPU_ARMV8
		read_data=GET_WAN_CONF();
		gpPhyPriv->wan_sel=read_data&SCU_WAN_CONF_REG_WAN_SEL_BITS;
		printk("SCU_WAN_CONF_REG_WAN_SEL=%d\n",gpPhyPriv->wan_sel);
#else
		read_data=IO_GPHYREG(SCU_WAN_CONF_REG);
		gpPhyPriv->wan_sel=read_data&SCU_WAN_CONF_REG_WAN_SEL_BITS;
		PON_PHY_PRINT(PHY_MSG_ERR,"SCU_WAN_CONF_REG_WAN_SEL=%d\n",gpPhyPriv->wan_sel);
		
#endif


	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"EPON mode\n");
					break;
		
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"GPON mode\n");
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"XGPON mode\n");

			break;	
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"XGSPON mode\n");
		
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			PON_PHY_PRINT(PHY_MSG_ERR,"XEPON_10G_1G mode\n");
			
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			PON_PHY_PRINT(PHY_MSG_ERR,"XEPON_10G_10G mode\n");
			
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
			PON_PHY_PRINT(PHY_MSG_ERR,"XEPON_1G_1G mode\n");
			
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
			PON_PHY_PRINT(PHY_MSG_ERR,"NGPON2_10G_10G mode\n");
			
			break;

		default:
			PON_PHY_PRINT(PHY_MSG_ERR,"UNKNOWN mode\n");

			return PHY_FAILURE;
	}


	return PHY_SUCCESS;
}

int en7580_func_init(void)  // (ponPhyFuncTbl** pPonPhyFunc)   * pPonPhyFunc=mt752x_func
{

	uint read_data = 0;

	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x  0x%8x  *=0x%8x  **=0x%8x  pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);

	PON_PHY_PRINT(PHY_MSG_ERR,"%s mode %d, product id = 0x%x\n",__FUNCTION__,gpPhyPriv->phyCfg.flags.mode,gpPhyPriv->scu_hir_np_sys_hw_id);

	//wan conf // should be marked later since it should be configured by sw

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
			switch(gpPhyPriv->scu_hir_np_sys_hw_id)
			{
				case 0x5:
				case 0x7:
					PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] NOT support MT752X EPON mode\n",__FUNCTION__,__LINE__);
					break;
		
				case 0x8:
				case 0xa:
					PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7580 EPON mode\n",__FUNCTION__,__LINE__);
					ponPhyFunc=en7580_gepon_func;
					break;			

				default:
					PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EPON UNKNOWN scu_hir_np_sys_hw_id = 0x%x\n",__FUNCTION__,__LINE__,gpPhyPriv->scu_hir_np_sys_hw_id);
					return 0;	//ang_20180208
			}
			
			break;
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			switch(gpPhyPriv->scu_hir_np_sys_hw_id)
			{
				case 0x5:
				case 0x7:
					PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] NOT support MT752X GPON mode\n",__FUNCTION__,__LINE__);
					break;
		
				case 0x8:
				case 0xa:
					PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7580 GPON mode\n",__FUNCTION__,__LINE__);
					ponPhyFunc=en7580_gepon_func;
					break;			

				default:
					PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] GPON UNKNOWN scu_hir_np_sys_hw_id = 0x%x\n",__FUNCTION__,__LINE__,gpPhyPriv->scu_hir_np_sys_hw_id);
					return 0;	//ang_20180208
			}
			
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7580 XGPON mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=en7580_xgpon_func;

			break;	
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7580 XGSPON mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=en7580_xgpon_func;
		
			break;	
			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7580 NGPON2_10G_10G mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=en7580_xgpon_func;
		
			break;	

		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7580 XEPON_10G_1G mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=en7580_xepon_func;
			
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7580 XEPON_10G_10G mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=en7580_xepon_func;
			
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7580 XEPON_1G_1G mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=en7580_gepon_func;
			
			break;

		default:
			printk("[%s:%d] UNKNOWN mode = %d\n",__FUNCTION__,__LINE__,gpPhyPriv->wan_sel);

			return PHY_FAILURE;
	}
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x  0x%8x  *=0x%8x  **=0x%8x	pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);

	return PHY_SUCCESS;
}


int en7580_set_trans_setting(void)
{
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	int ret;
	
    ret = PHY_SUCCESS;

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{
				#if ASIC_SERDES //EN7580_ASIC
				read_data=IO_GPHYREG(GEPON_CSR_XPON_SETTING);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read GEPON_CSR_XPON_SETTING(0x%x)=0x%x \n",GEPON_CSR_XPON_SETTING,read_data);
				IO_SPHYREG(GEPON_CSR_XPON_SETTING,phy_trans_iot_list[gpPhyPriv->trans_index].gepon_csr_xpon_setting) ;
				read_data=IO_GPHYREG(GEPON_CSR_XPON_SETTING);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set GEPON_CSR_XPON_SETTING(0x%x)=0x%x \n",GEPON_CSR_XPON_SETTING,read_data);

				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_0,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);

				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_1,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_1) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);
				
				#endif

				#if A60972_SERDES
				read_data=IO_GPHYREG(GEPON_CSR_XPON_SETTING);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read GEPON_CSR_XPON_SETTING(0x%x)=0x%x \n",GEPON_CSR_XPON_SETTING,read_data);
				IO_SPHYREG(GEPON_CSR_XPON_SETTING, phy_trans_iot_list[gpPhyPriv->trans_index].gepon_csr_xpon_setting);
				read_data=IO_GPHYREG(GEPON_CSR_XPON_SETTING);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set GEPON_CSR_XPON_SETTING(0x%x)=0x%x \n",GEPON_CSR_XPON_SETTING,read_data);

				if (!(phy_trans_iot_list[gpPhyPriv->trans_index].gepon_csr_xpon_setting & GEPON_CSR_XPON_SETTING_BURST_EN_INV))
				{
					gpPhyPriv->trans_tx_ben_level=HIGH_ACTIVE;
				}
				#endif
				
			}
			break;
		
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
            if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{
				#if ASIC_SERDES
				read_data=IO_GPHYREG(XEPON_PCS_SFP_STATUS);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read XEPON_PCS_SFP_STATUS(0x%x)=0x%x \n",XEPON_PCS_SFP_STATUS,read_data);
				IO_SPHYREG(XEPON_PCS_SFP_STATUS,phy_trans_iot_list[gpPhyPriv->trans_index].xepon_pcs_sfp_status) ;
				read_data=IO_GPHYREG(XEPON_PCS_SFP_STATUS);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set XEPON_PCS_SFP_STATUS(0x%x)=0x%x \n",XEPON_PCS_SFP_STATUS,read_data);
				
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_0,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);

				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_1,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_1) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);
				#endif

				#if A60972_SERDES				
				read_data=IO_GPHYREG(XEPON_PCS_SFP_STATUS);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read XEPON_PCS_SFP_STATUS(0x%x)=0x%x \n",XEPON_PCS_SFP_STATUS,read_data);
				IO_SPHYREG(XEPON_PCS_SFP_STATUS,phy_trans_iot_list[gpPhyPriv->trans_index].xepon_pcs_sfp_status) ;
				read_data=IO_GPHYREG(XEPON_PCS_SFP_STATUS);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set XEPON_PCS_SFP_STATUS(0x%x)=0x%x \n",XEPON_PCS_SFP_STATUS,read_data);
				#endif
            }
            break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G:
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
			
            if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{
				// SFP RX valid level

				#if ASIC_SERDES
				read_data=IO_GPHYREG(XGPON_PHY_SFP_VLD_LEVEL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",XGPON_PHY_SFP_VLD_LEVEL,read_data);
				IO_SPHYREG(XGPON_PHY_SFP_VLD_LEVEL,phy_trans_iot_list[gpPhyPriv->trans_index].xgpon_phy_sfp_vld_level) ;
				read_data=IO_GPHYREG(XGPON_PHY_SFP_VLD_LEVEL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",XGPON_PHY_SFP_VLD_LEVEL,read_data);

				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_0,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);

				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_1,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_1) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);
				#endif

				#if A60972_SERDES
				read_data=IO_GPHYREG(XGPON_PHY_SFP_VLD_LEVEL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",XGPON_PHY_SFP_VLD_LEVEL,read_data);
				IO_SPHYREG(XGPON_PHY_SFP_VLD_LEVEL,phy_trans_iot_list[gpPhyPriv->trans_index].xgpon_phy_sfp_vld_level) ;
				read_data=IO_GPHYREG(XGPON_PHY_SFP_VLD_LEVEL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",XGPON_PHY_SFP_VLD_LEVEL,read_data);
				#endif

            }
            break;

		default:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

			ret = PHY_FAILURE;
			break;
	}
	
	return ret;
}


int en7580_phy_cnt_enable(int errcnt_en, int bipcnt_en, int fmcnt_en)
{
	UINT32 read_data = 0;
	UINT32 write_data = 0;

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			read_data = IO_GPHYREG(GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_ERRCNT_MASK;
			if((errcnt_en == PHY_ENABLE) || (errcnt_en == PHY_DISABLE))
			{
				write_data = read_data | (errcnt_en << PHY_ERR_CNT_OFFSET);
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_EN, write_data);
			}
			read_data = IO_GPHYREG(GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_BIPCNT_MASK;
			if((bipcnt_en == PHY_ENABLE) || (bipcnt_en == PHY_DISABLE))
			{
				write_data = read_data | (bipcnt_en << PHY_BIP_CNT_OFFSET);
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_EN, write_data);
			}
			read_data = IO_GPHYREG(GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_FRAMECNT_MASK;
			if((fmcnt_en == PHY_ENABLE) || (fmcnt_en == PHY_DISABLE))
			{
				write_data = read_data | (fmcnt_en << PHY_FRAME_CNT_OFFSET);
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_EN, write_data);
			}
			break;
		
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G:
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
			break;

		default:
			printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

			break;
	}
	
	return PHY_SUCCESS;
}

int en7580_phy_reset_counter(void) 
	{
		UINT32 read_data = 0;
	
		switch(gpPhyPriv->wan_sel)
		{
			case SCU_WAN_CONF_REG_WAN_SEL_EPON:
			case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:			
			case SCU_WAN_CONF_REG_WAN_SEL_GPON:
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, PHY_ERR_CLR);//clear
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, PHY_BIP_CLR);//clear
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, PHY_RXFRAME_CLR);//clear

				break;
			
			case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
				IO_SPHYREG(XEPON_PCS_CLR_ALL_NUM, XEPON_PCS_CLR_ALL_NUM_CLR);
				break;
	
			case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
			case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G:
			case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:			
			case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
				read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
				IO_SPHYREG(XGPON_PHY_DBG_CTRL,read_data|XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
				read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
				IO_SPHYREG(XGPON_PHY_DBG_CTRL,read_data&~XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
				break;
	
			default:
				printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);
	
				break;
		}
		
		return PHY_SUCCESS;
	}

int en7580_phy_clear_int(void)
{
	uint phyIntStatus = 0;
	PON_PHY_PRINT(PHY_MSG_TRACE,"%s mode %d, product id = 0x%x\n",__FUNCTION__,gpPhyPriv->phyCfg.flags.mode,gpPhyPriv->scu_hir_np_sys_hw_id);

#if ASIC_SERDES
	// clear PMA rogue int before trans_power_ON
	phyIntStatus=IO_GPHYREG(XPON_PMA_XPON_INT_STA_0);
	PON_PHY_PRINT(PHY_MSG_INT,"XPON_PMA_XPON_INT_STA_0= %x\n", phyIntStatus);
	IO_SPHYREG(XPON_PMA_XPON_INT_STA_0, phyIntStatus);
	phyIntStatus=IO_GPHYREG(XPON_PMA_XPON_INT_STA_0);
	PON_PHY_PRINT(PHY_MSG_INT,"XPON_PMA_XPON_INT_STA_0= %x\n", phyIntStatus);
#endif
	
	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:			
			phyIntStatus=IO_GPHYREG(GEPON_CSR_XPON_INT_STA);
			IO_SPHYREG(GEPON_CSR_XPON_INT_STA_CLR, phyIntStatus);
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			phyIntStatus=IO_GPHYREG(GEPON_CSR_XPON_INT_STA);
			IO_SPHYREG(GEPON_CSR_XPON_INT_STA_CLR, phyIntStatus);
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:			
			phyIntStatus=IO_GPHYREG(XGPON_PHY_XG_PON_INT_STA);	//by ang_20180502
			IO_SPHYREG(XGPON_PHY_XG_PON_INT_STA, phyIntStatus);
			break;	
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:
			phyIntStatus=IO_GPHYREG(XGPON_PHY_XG_PON_INT_STA);	//by ang_20180502
			IO_SPHYREG(XGPON_PHY_XG_PON_INT_STA, phyIntStatus);
			break;	

		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			/*get interrupt status and clear*/
			phyIntStatus=IO_GPHYREG(XEPON_PCS_INT_STATUS);
			IO_SPHYREG(XEPON_PCS_INT_STATUS, phyIntStatus);
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			/*get interrupt status and clear*/
			phyIntStatus=IO_GPHYREG(XEPON_PCS_INT_STATUS);
			IO_SPHYREG(XEPON_PCS_INT_STATUS, phyIntStatus);
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
			phyIntStatus=IO_GPHYREG(GEPON_CSR_XPON_INT_STA);
			IO_SPHYREG(GEPON_CSR_XPON_INT_STA_CLR, phyIntStatus);
			break;

		default:			
			return PHY_FAILURE;
	}
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x  0x%8x  *=0x%8x  **=0x%8x	pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);

	return PHY_SUCCESS;
}

/*****************************************************************************
//function :
//		en7580_phy_gpon_mode
//description : 
//		this function is used to read which mode the system work at 
//input :	
//		N/A
//output :
//		1 : means GPON mode ; 0 : means EPON mode
******************************************************************************/
unchar en7580_phy_gpon_mode(void)
{
	uint read_data ;
	
	read_data = IO_GPHYREG(GEPON_CSR_PHYSET10) ;
	return ((read_data&GEPON_PHY_GPON_MODE) ? 1 : 0) ;
}

/*****************************************************************************
//function :
//		en7580_phy_set_epon_ts_continue_mode
//description : 
//		this function is used to set burst mode or continue mode
//input :	
//		mode : PHY_ENABLE meas continue mode(phy_burst_en | mac_burst_en); PHY_DISABLE means burst_mode(phy_burst_en)
//output :
//		PHY_SUCCESS
******************************************************************************/
int en7580_phy_set_epon_ts_continue_mode(uint mode)
{
	uint read_data = 0;

#if A60972_SERDES || ASIC_SERDES//EN7580_ASIC
	if (mode == PHY_ENABLE)
	{
		read_data = IO_GPHYREG(GEPON_CSR_PHYSET10);
		read_data |= (0x01 << 26);
		IO_SPHYREG(GEPON_CSR_PHYSET10, read_data);
	}
	else if (mode == PHY_DISABLE)
	{
		read_data = IO_GPHYREG(GEPON_CSR_PHYSET10);
		read_data &= ~(0x01 << 26);
		IO_SPHYREG(GEPON_CSR_PHYSET10, read_data);
	}
#endif

	return PHY_SUCCESS;
}


/*****************************************************************************
//function :
//		en7580_phy_round_trip_delay_sof
//description : 
//		
//input :	
//		void
//output :
//		ushort round_trip_delay_sof
******************************************************************************/

ushort en7580_phy_round_trip_delay_sof(void) 
{
	  uint read_data = 0;
	  ushort round_trip_delay_sof=0;

	  read_data = IO_GPHYREG(GEPON_CSR_ROUND_TRIP_DELAY_VALUE);	  
	  round_trip_delay_sof=read_data&0x0000ffff;	
	  return round_trip_delay_sof;
}


int en7580_fw_ready(int fwrdy_en)
{
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	int ret;
	
    ret = PHY_SUCCESS;

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
            read_data = IO_GPHYREG(GEPON_CSR_PHYSET2);
			if(fwrdy_en == PHY_ENABLE)
				write_data = read_data & (GEPON_PHY_FW_RDY_MASK) | (GEPON_PHY_FW_RDY_EN);
			else if(fwrdy_en == PHY_DISABLE)
				write_data = read_data & (GEPON_PHY_FW_RDY_MASK);
			else
				ret = PHY_FAILURE;
			IO_SPHYREG(GEPON_CSR_PHYSET2,write_data);
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
			
			printk("[%s:%d] XPON PHY do noting\n",__FUNCTION__,__LINE__);
			break;

		default:
			printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

			ret = PHY_FAILURE;
			break;
	}
	
	return ret;
}


int en7580_disable_pcs_tdc(void)
{

	uint read_data, write_data;

	PON_PHY_PRINT(PHY_MSG_DBG,"%s mode %d, product id = 0x%x\n",__FUNCTION__,gpPhyPriv->phyCfg.flags.mode,gpPhyPriv->scu_hir_np_sys_hw_id);


	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
			
			break;
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:

			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:			
			#if ASIC_SERDES
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
			//disable rx
			read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data & (~XGPON_PHY_RX_ENABLE); //disable rx
			IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			break;	
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:
			#if ASIC_SERDES
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
			//disable rx
			read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data & (~XGPON_PHY_RX_ENABLE); //disable rx
			IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			break;	

		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			#if ASIC_SERDES
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
			//disable rx
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_DISABLE);
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			#if ASIC_SERDES
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
			//disable rx
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_DISABLE);
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
		
			break;

		default:
			
			return PHY_FAILURE;
	}
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x  0x%8x  *=0x%8x  **=0x%8x	pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);

	return PHY_SUCCESS;
}

// for eyescan varification in kernel
// by david 20180424
void phy_eyescan_test(PHY_EYESCAN_MODE_LIST mode, int Ovr_sel, int start_p, int sweep_r)
{
	uint pical_data_out=0;
    //uint ro_dac_eye , eye_offset=0;
    int EYE_X_FW, EYE_Y_FW=0;
    uint EYE_X_HW, EYE_Y_HW=0;            
    uint eyecnt = 0;
    int i,j,k=0;
           
    //ro_dac_eye = Convert.ToUInt32(Yoffset.Text, 16);
    //eye_offset = Convert.ToUInt32(Yoffset.Text, 16);
    int Start_Point = start_p;
    int Sweep_Range = sweep_r;
	int Ovr = Ovr_sel; 	//need to modify for different RX Rate
    printk("Start_Point = %d	Sweep_Range = %d	Ovr = %d \n", Start_Point, Sweep_Range, Ovr);
	
	if (mode <= 3){
		//	 uint step = 5;
		XPON_eyescan_setting(mode);
			
		IO_SPHYA_REG_BITS(0xbfaf3334, 8, 8, 0x1);	//rg_disb_eyedur_init_b, 1'b0			   
		IO_SPHYA_REG_BITS(0xbfaf3674, 24, 24, 0x1);  //rg_disb_eyecnt_rx_rst_b, 1'b0				
		IO_SPHYA_REG_BITS(0xbfaf3334, 0, 0, 0x1);	//rg_disb_eyedur_en, 1'b0			   
			
		XPON_eyescan_Cal();
						   
		IO_SPHYA_REG_BITS(0xbfaf349c, 24, 24, 0x0); 	//toggle to generate latch signal
		IO_SPHYA_REG_BITS(0xbfaf349c, 24, 24, 0x1); 	//toggle to generate latch signal
//		ro_dac_eye = IO_GPHYA_REG_BITS(0xbfaf3528, 6, 0); //ro_dac_eye 
		pical_data_out = IO_GPHYA_REG_BITS(0xbfaf352c, 22, 16); //ro_pi_cal_data_out
	
	}
	else{
		XFI_eyescan_setting(mode);
			
		IO_SPHYA_REG_BITS(0xbfaf6334, 8, 8, 0x1);	//rg_disb_eyedur_init_b, 1'b0			  
		IO_SPHYA_REG_BITS(0xbfaf6674, 24, 24, 0x1); //rg_disb_eyecnt_rx_rst_b, 1'b0 			   
		IO_SPHYA_REG_BITS(0xbfaf6334, 0, 0, 0x1);	//rg_disb_eyedur_en, 1'b0	 
			
		XFI_eyescan_Cal();
			
		IO_SPHYA_REG_BITS(0xbfaf649c, 24, 24, 0x0);    //toggle to generate latch signal
		IO_SPHYA_REG_BITS(0xbfaf649c, 24, 24, 0x1);    //toggle to generate latch signal 
//		ro_dac_eye = IO_GPHYA_REG_BITS(0xbfaf6528, 6, 0); //ro_dac_eye 
		pical_data_out = IO_GPHYA_REG_BITS(0xbfaf652c, 22, 16); //ro_pi_cal_data_out 
	}

	//ro_dac_eye = eye_offset + ro_dac_eye;

	
	//EYE_X_HW = 0;
	EYE_X_HW = pical_data_out;
//	EYE_Y_HW = eye_offset + ro_dac_eye;
	EYE_X_FW = EYE_X_HW;

	EYE_Y_HW = 64;	
	EYE_Y_FW = -64; 

	//printk("Yoffset = %d \n", ro_dac_eye);
	printk("pical_data_out = %d \n", pical_data_out);		 
				   	
	for (i = 0; i < Start_Point*Ovr; i++){
		if (mode <= 3){
			// X index
			IO_SPHYA_REG_BITS(0xbfaf3638, 30, 24, EYE_X_HW); //rg_force_da_xpon_cdr_pr_pieye
			IO_SPHYA_REG_BITS(0xbfaf3644, 24, 24, 0x0);		//rg_disb_da_xpon_cdr_pr_pieye					  
		}
		else{
			IO_SPHYA_REG_BITS(0xbfaf6638, 30, 24, EYE_X_HW); //rg_force_da_xpon_cdr_pr_pieye
			IO_SPHYA_REG_BITS(0xbfaf6644, 24, 24, 0x0);		//rg_disb_da_xpon_cdr_pr_pieye					  
		}
			EYE_X_HW--;
			EYE_X_FW--;
	}	

	printk("eyecnt		EYE_X_FW		EYE_Y_FW \n");
	//snack sequence sweep full eye scan																							  
	for (k = 0; k < (Sweep_Range/2) ; k++)																							  
	{																																  
		for (i = 0; i < 65; i++)																									  
		{																															  		
			eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, mode);
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);
				 	  
			EYE_Y_HW++; 																											  
			EYE_Y_FW++; 																											  
		}																															  
		EYE_Y_HW = 1;																												  
		EYE_Y_FW = 1;																												  
																																		  
		for (j = 0; j < 63; j++)																									  
		{																															  
			eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, mode);																				  
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	  
			EYE_Y_HW++; 																											  
			EYE_Y_FW++; 																											  
		}			
		
			EYE_X_HW = eyescan_moveX(EYE_X_FW, EYE_Y_FW, Ovr, mode);																					  
			EYE_X_FW = EYE_X_FW + Ovr;																									  
																																		  
			EYE_Y_HW--; 																												  
			EYE_Y_FW--; 																												  
																																		  
			for (j = 0; j < 63; j++)																									  
			{																															  
				eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, mode);																				  
				printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	 	  
				EYE_Y_HW--; 																											  
				EYE_Y_FW--; 																											  
			}																															  
			EYE_Y_HW = 128; 																											  
			EYE_Y_FW = 0;																												  
																																		  
			for (i = 0; i < 65; i++)																									  
			{																															  
				eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, mode);																				  
				printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	  	  
				EYE_Y_HW--; 																											  
				EYE_Y_FW--; 																											  
			}																															  
			EYE_X_HW = eyescan_moveX(EYE_X_FW, EYE_Y_FW, Ovr, mode);																					  
			EYE_X_FW = EYE_X_FW + Ovr;																									  
																																		  
			EYE_Y_HW++; 																												  
			EYE_Y_FW++; 																												  
		}																																  
																																		  
		//last time bottom-up sweep Y index 																							  
		for (i = 0; i < 65; i++)																										  
		{																																  
			eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, mode);																				  
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	 				  
			EYE_Y_HW++; 																												  
			EYE_Y_FW++; 																												  
		}																																  
		EYE_Y_HW = 1;																													  
		EYE_Y_FW = 1;																													  
		for (j = 0; j < 63; j++)																										  
		{																																  
			eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, mode);																				  
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	 				  
			EYE_Y_HW++; 																												  
			EYE_Y_FW++; 																												  
		}

}

void XPON_eyescan_setting(PHY_EYESCAN_MODE_LIST mode){
	switch(mode){
		case xpon_10p3125g:
			IO_SPHYA_REG_BITS(0xbfaf3118, 25, 24, 0x0);          //rg_xpon_cdr_lpf_ratio,0x0
	    	//IO_SPHYA_REG_BITS(0x1330, 31, 24, 0xFF);         //rg_eye_mask, 8'hFF
	    	IO_SPHYA_REG_BITS(0xbfaf3330, 31, 24, 0x01);         //rg_eye_mask, 8'hFF hugo
	    	IO_SPHYA_REG_BITS(0xbfaf3300, 10, 0, 0x1C0);         //rg_x_min, 11'h1F0
	    	IO_SPHYA_REG_BITS(0xbfaf3300, 26, 16, 0x234);        //rg_x_max, 11'h234	
	    	break;
	    case xpon_9p95328g:
			IO_SPHYA_REG_BITS(0xbfaf3118, 25, 24, 0x0);          //rg_xpon_cdr_lpf_ratio,0x2
	   		 //IO_SPHYA_REG_BITS(0x1330, 31, 24, 0xFF);         //rg_eye_mask, 8'hFF
	    	IO_SPHYA_REG_BITS(0xbfaf3330, 31, 24, 0x01);         //rg_eye_mask, 8'h55
	    	IO_SPHYA_REG_BITS(0xbfaf3300, 10, 0, 0x1C0);         //rg_x_min, 11'h1C0
	    	IO_SPHYA_REG_BITS(0xbfaf3300, 26, 16, 0x240);        //rg_x_max, 11'h240
	    	break;
	    case xpon_2p48832g:
			IO_SPHYA_REG_BITS(0xbfaf3118, 25, 24, 0x2);          //rg_xpon_cdr_lpf_ratio,0x2
	    	//IO_SPHYA_REG_BITS(0x1330, 31, 24, 0x55);         //rg_eye_mask, 8'h55
	    	IO_SPHYA_REG_BITS(0xbfaf3330, 31, 24, 0x01);         //rg_eye_mask, 8'h55
	    	IO_SPHYA_REG_BITS(0xbfaf3300, 10, 0, 0x19c);         //rg_x_min, 11'h100
	    	IO_SPHYA_REG_BITS(0xbfaf3300, 26, 16, 0x330);        //rg_x_max, 11'h240
	    	break;
	    case xpon_1p25g:
			IO_SPHYA_REG_BITS(0xbfaf3118, 25, 24, 0x3);          //rg_xpon_cdr_lpf_ratio,0x3
	    	//IO_SPHYA_REG_BITS(0x1330, 31, 24, 0x22);         //rg_eye_mask, 8'h22
	    	IO_SPHYA_REG_BITS(0xbfaf3330, 31, 24, 0x01);         //rg_eye_mask, 8'h22
	    	IO_SPHYA_REG_BITS(0xbfaf3300, 10, 0, 0x100);         //rg_x_min, 11'h100
	    	IO_SPHYA_REG_BITS(0xbfaf3300, 26, 16, 0x340);        //rg_x_max, 11'h340
	    	break;
	    default:
			break;
	}

	IO_SPHYA_REG_BITS(0xbfaf3330, 9, 0, 0x0F8);              //rg_cntlen, 10'h0F8
	IO_SPHYA_REG_BITS(0xbfaf3330, 16, 16, 0x0);              //rg_cntforever, 1'b0
	IO_SPHYA_REG_BITS(0xbfaf3338, 8, 8, 0x0);                //rg_data_shift, 1'b0
	                                                        
	IO_SPHYA_REG_BITS(0xbfaf3304, 16, 16, 0x0);              //rg_index_mode, 1'b0
	IO_SPHYA_REG_BITS(0xbfaf3308, 19, 0, 0x0FFF8);           //rg_eyedur, 20'hFFFF8
	                                                        
	IO_SPHYA_REG_BITS(0xbfaf330c, 0, 0, 0x0);                //rg_eye_nextpts_sel, 1'b0
	IO_SPHYA_REG_BITS(0xbfaf330c, 8, 8, 0x0);                //rg_eye_nextpts_toggle, 1'b0
	IO_SPHYA_REG_BITS(0xbfaf330c, 16, 16, 0x1);              //rg_eye_nextpts, 1'b1
	                                                        
	IO_SPHYA_REG_BITS(0xbfaf3320, 7, 0, 0x4);                //rg_eyecnt_hth, 8'd2
	IO_SPHYA_REG_BITS(0xbfaf3320, 15, 8, 0x4);               //rg_eyecnt_vth, 8'd2
	IO_SPHYA_REG_BITS(0xbfaf3324, 10, 0, 0x4);               //rg_eo_hth, 11'd10
	IO_SPHYA_REG_BITS(0xbfaf3324, 22, 16, 0x4);              //rg_eo_vth, 7'd10
	                                                        
	IO_SPHYA_REG_BITS(0xbfaf3374, 16, 16, 0x0);              //rg_heo_emphasis=1'b0
	IO_SPHYA_REG_BITS(0xbfaf3374, 0, 0, 0x0);                //rg_a_lgain=1'b0
	IO_SPHYA_REG_BITS(0xbfaf3374, 8, 8, 0x0);                //rg_a_mgain=1'b0
	IO_SPHYA_REG_BITS(0xbfaf3378, 1, 0, 0x1);                //rg_a_sel=2'b01
	IO_SPHYA_REG_BITS(0xbfaf3374, 24, 24, 0x0);              //rg_b_zero_sel=1'b0

}

void XFI_eyescan_setting(PHY_EYESCAN_MODE_LIST mode){
	switch(mode){
			case xfi_10p3125g:
				IO_SPHYA_REG_BITS(0xbfaf6030, 14, 13, 0x0); 		 //rg_xpon_cdr_lpf_ratio,0x0			  
				//IO_SPHYA_REG_BITS(0x2330, 31, 24, 0xFF);		   //rg_eye_mask, 8'hFF 				  
				IO_SPHYA_REG_BITS(0xbfaf6330, 31, 24, 0x01);		 //rg_eye_mask, 8'hFF					  
				IO_SPHYA_REG_BITS(0xbfaf6300, 10, 0, 0x1C0);		 //rg_x_min, 11'h1C0					  
				IO_SPHYA_REG_BITS(0xbfaf6300, 26, 16, 0x240);		 //rg_x_max, 11'h240		
				break;
				
			case xfi_5p15625g:				
				IO_SPHYA_REG_BITS(0xbfaf6030, 14, 13, 0x1); 		 //rg_xfi_cdr_lpf_ratio,0x2 	  
				//IO_SPHYA_REG_BITS(0x2330, 31, 24, 0x01);		   //rg_eye_mask, 8'h55 		  
				IO_SPHYA_REG_BITS(0xbfaf6330, 31, 24, 0x01);		 //rg_eye_mask, 8'h55			  
				IO_SPHYA_REG_BITS(0xbfaf6300, 10, 0, 0x100);		 //rg_x_min, 11'h100			  
				IO_SPHYA_REG_BITS(0xbfaf6300, 26, 16, 0x240);		 //rg_x_max, 11'h240			  				
				break;
				
			case xfi_3p125g:			
				IO_SPHYA_REG_BITS(0xbfaf6030, 14, 13, 0x1); 		 //rg_xpon_cdr_lpf_ratio,0x3   
				//IO_SPHYA_REG_BITS(0x2330, 31, 24, 0xFF);		   //rg_eye_mask, 8'hFF 	   
				IO_SPHYA_REG_BITS(0xbfaf6330, 31, 24, 0x01);		 //rg_eye_mask, 8'h22		   
				IO_SPHYA_REG_BITS(0xbfaf6300, 10, 0, 0x11C);		 //rg_x_min, 11'h11C		   
				IO_SPHYA_REG_BITS(0xbfaf6300, 26, 16, 0x280);		 //rg_x_max, 11'h280		   				
				break;
				
			case xfi_1p25g:
				IO_SPHYA_REG_BITS(0xbfaf6030, 14, 13, 0x3); 		 //rg_xpon_cdr_lpf_ratio,0x3			  
				//IO_SPHYA_REG_BITS(0x2330, 31, 24, 0x22);		   //rg_eye_mask, 8'h22 				  
				IO_SPHYA_REG_BITS(0xbfaf6330, 31, 24, 0x01);		 //rg_eye_mask, 8'h22					  
				IO_SPHYA_REG_BITS(0xbfaf6300, 10, 0, 0x0);		   //rg_x_min, 11'h100						  
				IO_SPHYA_REG_BITS(0xbfaf6300, 26, 16, 0x400);		 //rg_x_max, 11'h340	
				break;
				
			default:
				break;
		}
																																													  
	IO_SPHYA_REG_BITS(0xbfaf6330, 9, 0, 0x0F8); 			 //rg_cntlen, 10'h0F8					  
	IO_SPHYA_REG_BITS(0xbfaf6330, 16, 16, 0x0); 			 //rg_cntforever, 1'b0					  
	IO_SPHYA_REG_BITS(0xbfaf6338, 8, 8, 0x0);				 //rg_data_shift, 1'b0					  
																								  
	IO_SPHYA_REG_BITS(0xbfaf6304, 16, 16, 0x0); 			 //rg_index_mode, 1'b0					  
	IO_SPHYA_REG_BITS(0xbfaf6308, 19, 0, 0x0FFF8);			 //rg_eyedur, 20'hFFFF8 				  
																								  
	IO_SPHYA_REG_BITS(0xbfaf630c, 0, 0, 0x0);				 //rg_eye_nextpts_sel, 1'b0 			  
	IO_SPHYA_REG_BITS(0xbfaf630c, 8, 8, 0x0);				 //rg_eye_nextpts_toggle, 1'b0			  
	IO_SPHYA_REG_BITS(0xbfaf630c, 16, 16, 0x1); 			 //rg_eye_nextpts, 1'b1 				  
																								  
	IO_SPHYA_REG_BITS(0xbfaf6320, 7, 0, 0x4);				 //rg_eyecnt_hth, 8'd2					  
	IO_SPHYA_REG_BITS(0xbfaf6320, 15, 8, 0x4);				 //rg_eyecnt_vth, 8'd2					  
	IO_SPHYA_REG_BITS(0xbfaf6324, 10, 0, 0x4);				 //rg_eo_hth, 11'd10					  
	IO_SPHYA_REG_BITS(0xbfaf6324, 22, 16, 0x4); 			 //rg_eo_vth, 7'd10 					  
																								  
	IO_SPHYA_REG_BITS(0xbfaf6374, 16, 16, 0x0); 			 //rg_heo_emphasis=1'b0 				  
	IO_SPHYA_REG_BITS(0xbfaf6374, 0, 0, 0x0);				 //rg_a_lgain=1'b0						  
	IO_SPHYA_REG_BITS(0xbfaf6374, 8, 8, 0x0);				 //rg_a_mgain=1'b0						  
	IO_SPHYA_REG_BITS(0xbfaf6378, 1, 0, 0x1);				 //rg_a_sel=2'b01						  
	IO_SPHYA_REG_BITS(0xbfaf6374, 24, 24, 0x0); 			 //rg_b_zero_sel=1'b0					  

}

void XPON_eyescan_Cal(void){
	
	IO_SPHYA_REG_BITS(0xbfaf3644, 24, 24, 0x1); 	 //rg_disb_da_xpon_cdr_pr_pieye 		   
	//pical redo																		   
	//reset block																		   
	IO_SPHYA_REG_BITS(0xbfaf3370, 7, 0, 0x80);		 //rg_eq_en_delay, 8'h80				   
	IO_SPHYA_REG_BITS(0xbfaf3430, 10, 8, 0x4);		 //rg_kpgain, 3'h4						   
	IO_SPHYA_REG_BITS(0xbfaf3494, 16, 16, 0x0); 	 //rg_eq_pi_cal_rst_b, 1'b0 			   
																						   
	IO_SPHYA_REG_BITS(0xbfaf3670, 8, 8, 0x0);		 //rg_disb_rx_and_pical_rstb, 1'b0		   
	IO_SPHYA_REG_BITS(0xbfaf3664, 8, 8, 0x0);		 //rg_force_rx_and_pical_rstb, 1'b0 	   
																						   
	IO_SPHYA_REG_BITS(0xbfaf3670, 0, 0, 0x0);		 //rg_disb_ref_and_pical_rstb, 1'b0 	   
	IO_SPHYA_REG_BITS(0xbfaf3664, 0, 0, 0x0);		 //rg_force_ref_and_pical_rstb, 1'b0	   
																						   
	//enable																			   
	IO_SPHYA_REG_BITS(0xbfaf3660, 24, 24, 0x0); 	   //rg_disb_rx_or_pical_en, 1'b0		   
	IO_SPHYA_REG_BITS(0xbfaf3654, 8, 8, 0x0);	   //rg_force_rx_or_pical_en, 1'b0			   
																						   
	IO_SPHYA_REG_BITS(0xbfaf335C, 8, 8, 0x0);	   //rg_disb_rx_pical_en					   
	IO_SPHYA_REG_BITS(0xbfaf3364, 8, 8, 0x0);	   //rg_force_rx_pical_en					   
																						   
	//release reset 																	   
	IO_SPHYA_REG_BITS(0xbfaf3494, 16, 16, 0x1); 	 //rg_eq_pi_cal_rst_b, 1'b1 			   
	IO_SPHYA_REG_BITS(0xbfaf3664, 8, 8, 0x1);		 //rg_force_rx_and_pical_rstb, 1'b1 	   
	IO_SPHYA_REG_BITS(0xbfaf3664, 0, 0, 0x1);		 //rg_force_ref_and_pical_rstb, 1'b1	   
	IO_SPHYA_REG_BITS(0xbfaf3654, 8, 8, 0x1);	   //rg_force_rx_or_pical_en, 1'b1			   
																						   
	//delay for 1m seconds																   
	mdelay(1);																	   
																						   
	IO_SPHYA_REG_BITS(0xbfaf3654, 8, 8, 0x0);	   //rg_force_rx_or_pical_en, 1'b1			   
	IO_SPHYA_REG_BITS(0xbfaf3658, 0, 0, 0x0);		 //rg_disb_eq_pi_cal_rdy, 1'b0			   
	IO_SPHYA_REG_BITS(0xbfaf3648, 0, 0, 0x1);		 //rg_force_eq_pi_cal_rdy, 1'b1 		   
}

void XFI_eyescan_Cal(void){
	
	IO_SPHYA_REG_BITS(0xbfaf6644, 24, 24, 0x1); 	 //rg_disb_da_xfi_cdr_pr_pieye			   
	//pical redo																		   
	//reset block																		   
	IO_SPHYA_REG_BITS(0xbfaf6370, 7, 0, 0x80);		 //rg_eq_en_delay, 8'h80				   
	IO_SPHYA_REG_BITS(0xbfaf6430, 10, 8, 0x4);		 //rg_kpgain, 3'h4						   
	IO_SPHYA_REG_BITS(0xbfaf6494, 16, 16, 0x0); 	 //rg_eq_pi_cal_rst_b, 1'b0 			   
	IO_SPHYA_REG_BITS(0xbfaf6670, 8, 8, 0x0);		 //rg_disb_rx_and_pical_rstb, 1'b0		   
	IO_SPHYA_REG_BITS(0xbfaf6664, 8, 8, 0x0);		 //rg_force_rx_and_pical_rstb, 1'b0 	   
	IO_SPHYA_REG_BITS(0xbfaf6670, 0, 0, 0x0);		 //rg_disb_ref_and_pical_rstb, 1'b0 	   
	IO_SPHYA_REG_BITS(0xbfaf6664, 0, 0, 0x0);		 //rg_force_ref_and_pical_rstb, 1'b0	   
																						   
	//enable																			   
	IO_SPHYA_REG_BITS(0xbfaf6660, 24, 24, 0x0); 	 //rg_disb_rx_or_pical_en, 1'b0 		   
	IO_SPHYA_REG_BITS(0xbfaf6654, 8, 8, 0x0);		 //rg_force_rx_or_pical_en, 1'b0		   
	IO_SPHYA_REG_BITS(0xbfaf635C, 8, 8, 0x0);		 //rg_disb_rx_pical_en					   
	IO_SPHYA_REG_BITS(0xbfaf6364, 8, 8, 0x0);		 //rg_force_rx_pical_en 				   
																						   
	//release reset 																	   
	IO_SPHYA_REG_BITS(0xbfaf6494, 16, 16, 0x1); 	 //rg_eq_pi_cal_rst_b, 1'b1 			   
	IO_SPHYA_REG_BITS(0xbfaf6664, 8, 8, 0x1);		 //rg_force_rx_and_pical_rstb, 1'b1 	   
	IO_SPHYA_REG_BITS(0xbfaf6664, 0, 0, 0x1);		 //rg_force_ref_and_pical_rstb, 1'b1	   
	IO_SPHYA_REG_BITS(0xbfaf6654, 8, 8, 0x1);		 //rg_force_rx_or_pical_en, 1'b1		   
																						   
	//delay for 1m seconds																   
	mdelay(1);																	   
																						   
	IO_SPHYA_REG_BITS(0xbfaf6654, 8, 8, 0x0);	   //rg_force_rx_or_pical_en, 1'b1			   
	IO_SPHYA_REG_BITS(0xbfaf6658, 0, 0, 0x0);		 //rg_disb_eq_pi_cal_rdy, 1'b0			   
	IO_SPHYA_REG_BITS(0xbfaf6648, 0, 0, 0x1);		 //rg_force_eq_pi_cal_rdy, 1'b1 	
	
}

int eyescan_countPoint(uint EYE_X_FW, uint EYE_Y_FW, PHY_EYESCAN_MODE_LIST mode){
	uint eyecnt = 0;																			
	int eyecnt_rdy = 0;																		
																								
	if (mode <= 3)
    {
         //Set the eyecnt count point (EYE_X_FW, EYE_Y_FW)
         // X index
         IO_SPHYA_REG_BITS(0xbfaf3638, 30, 24, EYE_X_FW); //rg_force_da_xpon_cdr_pr_pieye
         IO_SPHYA_REG_BITS(0xbfaf3644, 24, 24, 0x0);      //rg_disb_da_xpon_cdr_pr_pieye
         
         // Y index
         IO_SPHYA_REG_BITS(0xbfaf3638, 14, 8, EYE_Y_FW);  //rg_force_da_xpon_rx_dac_eye
         IO_SPHYA_REG_BITS(0xbfaf3644, 8, 8, 0x0);        //rg_disb_da_xpon_rx_dac_eye                

         // EYE cnt enable 
         IO_SPHYA_REG_BITS(0xbfaf3334, 8, 8, 0x0);   //rg_disb_eyedur_init_b, 1'b0
         IO_SPHYA_REG_BITS(0xbfaf3334, 24, 24, 0x0);   //rg_force_eyedur_init_b, 1'b0

         IO_SPHYA_REG_BITS(0xbfaf3674, 24, 24, 0x0);   //rg_disb_eyecnt_rx_rst_b, 1'b0
         IO_SPHYA_REG_BITS(0xbfaf3668, 24, 24, 0x0);   //rg_force_eyecnt_rx_rst_b, 1'b0

         IO_SPHYA_REG_BITS(0xbfaf3334, 0, 0, 0x0);   //rg_disb_eyedur_en, 1'b0
         IO_SPHYA_REG_BITS(0xbfaf3334, 16, 16, 0x0);   //rg_force_eyedur_en, 1'b0

         IO_SPHYA_REG_BITS(0xbfaf3334, 24, 24, 0x1);   //rg_force_eyedur_init_b, 1'b1
         IO_SPHYA_REG_BITS(0xbfaf3668, 24, 24, 0x1);  //rg_force_eyecnt_rx_rst_b, 1'b1

         IO_SPHYA_REG_BITS(0xbfaf3334, 16, 16, 0x1);   //rg_force_eyedur_en, 1'b1
         mdelay(1);

         IO_SPHYA_REG_BITS(0xbfaf349c, 24, 24, 0x0);      //toggle to generate latch signal
         IO_SPHYA_REG_BITS(0xbfaf349c, 24, 24, 0x1);      //toggle to generate latch signal

         eyecnt_rdy = IO_GPHYA_REG_BITS(0xbfaf3534, 24, 24);
         if (eyecnt_rdy == 1)   // if eyecnt_rdy
         {
             eyecnt = IO_GPHYA_REG_BITS(0xbfaf3540, 19, 00);   //eyecnt
         }
         else
         {
             printk("eyecnt_rdy = %d \n", eyecnt_rdy);
         }
     }

     else
     {
         //Set the eyecnt count point (EYE_X_FW, EYE_Y_FW)
         // X index
         IO_SPHYA_REG_BITS(0xbfaf6638, 30, 24, EYE_X_FW); //rg_force_da_xpon_cdr_pr_pieye
         IO_SPHYA_REG_BITS(0xbfaf6644, 24, 24, 0x0);      //rg_disb_da_xpon_cdr_pr_pieye
        
         // Y index
         IO_SPHYA_REG_BITS(0xbfaf6638, 14, 8, EYE_Y_FW);  //rg_force_da_xpon_rx_dac_eye
         IO_SPHYA_REG_BITS(0xbfaf6644, 8, 8, 0x0);        //rg_disb_da_xpon_rx_dac_eye                

         // EYE cnt enable 
         IO_SPHYA_REG_BITS(0xbfaf6334, 8, 8, 0x0);     //rg_disb_eyedur_init_b, 1'b0
         IO_SPHYA_REG_BITS(0xbfaf6334, 24, 24, 0x0);   //rg_force_eyedur_init_b, 1'b0

         IO_SPHYA_REG_BITS(0xbfaf6674, 24, 24, 0x0);   //rg_disb_eyecnt_rx_rst_b, 1'b0
         IO_SPHYA_REG_BITS(0xbfaf6668, 24, 24, 0x0);   //rg_force_eyecnt_rx_rst_b, 1'b0

         IO_SPHYA_REG_BITS(0xbfaf6334, 0, 0, 0x0);   //rg_disb_eyedur_en, 1'b0
         IO_SPHYA_REG_BITS(0xbfaf6334, 16, 16, 0x0);   //rg_force_eyedur_en, 1'b0

         IO_SPHYA_REG_BITS(0xbfaf6334, 24, 24, 0x1);   //rg_force_eyedur_init_b, 1'b1
         IO_SPHYA_REG_BITS(0xbfaf6668, 24, 24, 0x1);  //rg_force_eyecnt_rx_rst_b, 1'b1

         IO_SPHYA_REG_BITS(0xbfaf6334, 16, 16, 0x1);   //rg_force_eyedur_en, 1'b1
         mdelay(1);

         IO_SPHYA_REG_BITS(0xbfaf649c, 24, 24, 0x0);      //toggle to generate latch signal
         IO_SPHYA_REG_BITS(0xbfaf649c, 24, 24, 0x1);      //toggle to generate latch signal

         eyecnt_rdy = IO_GPHYA_REG_BITS(0xbfaf6534, 24, 24);
         if (eyecnt_rdy == 1)   // if eyecnt_rdy
         {
             eyecnt = IO_GPHYA_REG_BITS(0xbfaf6540, 19, 00);   //eyecnt
         }
         else
         {
             printk("eyecnt_rdy = %d\n", eyecnt_rdy);
         }
     }
     return eyecnt;																	
}

uint eyescan_moveX(uint EYE_X_HW, uint EYE_Y_HW, int Ovr_sel, PHY_EYESCAN_MODE_LIST mode){
	int i;
	if (mode <= 3){               
       for (i = 0; i < Ovr_sel; i++)
       {
           // X index
           IO_SPHYA_REG_BITS(0xbfaf3638, 30, 24, EYE_X_HW); //rg_force_da_xpon_cdr_pr_pieye
           IO_SPHYA_REG_BITS(0xbfaf3644, 24, 24, 0x0);      //rg_disb_da_xpon_cdr_pr_pieye                
           EYE_X_HW++;
       }
    }
    else{
       for (i = 0; i < Ovr_sel; i++)
       {
           // X index
           IO_SPHYA_REG_BITS(0xbfaf6638, 30, 24, EYE_X_HW); //rg_force_da_xpon_cdr_pr_pieye
           IO_SPHYA_REG_BITS(0xbfaf6644, 24, 24, 0x0);      //rg_disb_da_xpon_cdr_pr_pieye                
           EYE_X_HW++;
       }
    }
    return EYE_X_HW;

}

                                                                          
#define _I_AM_GEPON_SPLIT_LINE_                                                
                                                                               
/////////////////////////////////////////////////////////////////////////////////////
// en7580_gepon_func                                                           
/////////////////////////////////////////////////////////////////////////////////////

int en7580_gepon_pon_phy_reset(char* buf)
{
	uint val = 0;
	uint wan_cfg=0;

	printk("\r\nEN7580 << GEPON >> pon_phy_reset\r\n");

	//gating PHY-D ck out
	IO_SPHYREG(XPON_PMA_PON_CK_SET, 0x0);    //0x1fa8b450=0
	udelay(1);

	//switch wan mode
	wan_cfg=IO_GPHYREG(SCU_WAN_CONF_REG); 
	IO_SPHYREG(SCU_WAN_CONF_REG,((wan_cfg&0xffffff00)|0x11));

	val = IO_GPHYREG(TOP_RST_CTRL_SW2);
	val |=0x01;
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
	udelay(1);
	//release xpon phy top reset 
	val &=~(0x01);
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);

	udelay(1);	
	IO_SPHYREG(SCU_WAN_CONF_REG,wan_cfg);


	return PHY_SUCCESS;
}

int en7580_gepon_phy_dump(char* buf)
{
	
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;
	UINT32 temp=0;
	phy_reg_total_num=121;
	phy_reg_all=gepon_phy_reg_all;

	printk("\r\n%-44s	 address=value\r\n","_____ GEPON REG DUMP _____");
	for(i=0;i<=phy_reg_total_num;i++)
	{
		(phy_reg_all+i)->def=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
	}

#if	PHY_REG_READ_WRITE_TEST
	//Register Read/Write test
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ZEROS ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);
		mdelay(10);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_0");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ONES ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
        temp = IO_GPHYREG((phy_reg_all+i)->addr);
        mdelay(10);
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);
        mdelay(10);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_1");
		}
		else
		{
			printk("=Pass");
		}
        IO_SPHYREG((phy_reg_all+i)->addr,temp);
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ END ___\r\n");
#endif

	return PHY_SUCCESS;
}

int en7580_gepon_phy_dev_init(char* buf)
{
	uint read_data = 0;
	uint write_data = 0;

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d]\r\n",__FUNCTION__,__LINE__);


	/* switch GPIO to XPON mode*/
#ifdef TCSUPPORT_CPU_ARMV8 
	/* switch GPIO to XPON mode*/		
	read_data = GET_IOMUX_CTRL_3(); //GPIO_PON_MODE_7523	
	write_data =(read_data | 0x01); //bit [0]: 1	
	SET_IOMUX_CTRL_3(write_data); //0-->1 //GPIO_PON_MODE_7523
#else 

	/* switch GPIO to XPON mode*/
	read_data = IO_GREG(IOMUX_Control_1_register);
	read_data = read_data | (RG_GPIO_PON_MODE) | (RG_PON_I2C_MODE) ;
	IO_SREG(IOMUX_Control_1_register, read_data);
#endif 	

	

	/*sigdet*/
	//read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	//read_data = read_data & ~(PHY_SQUELCH_DEGLITCH);
	//IO_SPHYREG(PHY_CSR_PHYSET3, read_data);

#ifdef TCSUPPORT_CPU_ARMV8 //julia_asic
	/*after sw reset, register can be modify by PBUS*/
	read_data = GET_SCU_RST_ACC_CHK_BMA(); //TOP_CSR_PBUS_CTRL
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	SET_SCU_RST_ACC_CHK_BMA(read_data); //TOP_CSR_PBUS_CTRL
	/* transceiver power initial setting*/
	read_data = GET_GPIO_SSR(); //TOP_CSR_GPIO_SHARE
	read_data = read_data & ~(TOP_LED1_MODE);
	SET_GPIO_SSR(read_data); //TOP_CSR_GPIO_SHARE
#else	
	/*after sw reset, register can be modify by PBUS*/
	read_data = IO_GREG(TOP_CSR_PBUS_CTRL);
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	IO_SREG(TOP_CSR_PBUS_CTRL, read_data);
	/* transceiver power initial setting*/
	read_data = IO_GREG(TOP_CSR_GPIO_SHARE);
	read_data = read_data & ~(TOP_LED1_MODE);
	IO_SREG(TOP_CSR_GPIO_SHARE, read_data);
#endif

	
	
	read_data = IO_GPHYREG(GEPON_CSR_PHYSET10);

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:			
			write_data = (read_data & ~GEPON_PHY_GPON_MODE);
			if(gpPhyPriv->scu_hir_np_sys_hw_id == 0xa)
			{
				IO_SPHYREG(GEPON_CSR_FECDEC_CTL,0x0);
			}
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			write_data = (read_data | GEPON_PHY_GPON_MODE);
			break;

		default:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

			return PHY_FAILURE;
	}

	IO_SPHYREG(GEPON_CSR_PHYSET10, write_data);
	
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x	0x%8x  *=0x%8x	**=0x%8x	pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);

	
	/* change the guard time pattern */
	gpPhyPriv->phyGuardBitDelm = 0xAAAB5983;
	//IO_SPHYREG(PHY_CSR_GPON_DELIMITER_GUARD, 0xAAAB5983);

	//read_data = IO_GPHYREG(GEPON_CSR_PHYSET3);
	//write_data = read_data |GEPON_CSR_PHYSET3_PLL_RST;  // |GEPON_CSR_PHYSET3_PHY_RST|GEPON_CSR_PHYSET3_SOFTWARE_RST;  //GEPON_CSR_PHYSET3_LOS_RST
	//IO_SPHYREG(GEPON_CSR_PHYSET3, write_data) ;
	phy_fw_ready(PHY_DISABLE);

	return PHY_SUCCESS ;

}

int en7580_gepon_phy_int_config(char* buf)
{
	UINT32 *mode = (UINT32 *)buf;
	UINT32 *int_type = (UINT32 *)buf+1;

	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 phyIntStatus;
	
	if(PHY_INT_EN_MODE_DEFAULT==(*mode))
	{
		write_data=(GEPON_CSR_XPON_PHYRDY_INT_EN|GEPON_CSR_XPON_LOF_INT_EN|GEPON_CSR_XPON_TRANS_LOS_INT_EN|GEPON_CSR_XPON_NO_LOS_INT_EN);
		PON_PHY_PRINT(PHY_MSG_TRACE,"GEPON Phy use default int_type:0x%.8x\n", write_data);
	}
	else
	{
		write_data=(*int_type);
		PON_PHY_PRINT(PHY_MSG_TRACE,"GEPON Phy_Int_Config type     :0x%.8x\n", write_data);
	}

	//NOT Clear PHY ISR Status, Let PHY_ISR() function deal with the ISR Status state machine
	//Otherwise, we may miss the ISR status after PHY init and before register ISR to system

	read_data = IO_GPHYREG(GEPON_CSR_XPON_INT_EN);	
	PON_PHY_PRINT(PHY_MSG_TRACE,"GEPON Phy_Int_Config read     :0x%.8x\n", read_data);
	
	IO_SPHYREG(GEPON_CSR_XPON_INT_EN, write_data);	
	read_data = IO_GPHYREG(GEPON_CSR_XPON_INT_EN);
	PON_PHY_PRINT(PHY_MSG_TRACE,"GEPON Phy_Int_Config write    :0x%.8x\n", read_data);


	return PHY_SUCCESS;
}

int en7580_gepon_phy_get_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	int phyReadyStatus,phyLosStatus;

    api_data->ret = PHY_SUCCESS;

    switch(api_data->cmd_id)
    {
        case PON_GET_PHY_LOS_STATUS:
			read_data = IO_GPHYREG(GEPON_CSR_XPON_STA);
			if( (read_data & GEPON_CSR_XPON_STA_LOS) == GEPON_CSR_XPON_STA_LOS) //it is transceiver sfp rx LOSS no PHY digital
			{
				api_data->ret = PHY_LOS_HAPPEN;
			}
			else
			{
				api_data->ret = PHY_NO_LOS_HAPPEN;
			}
            break;
			
        case PON_GET_PHY_READY_STATUS:
			read_data = IO_GPHYREG(GEPON_CSR_PHYSTA1);//read bit[18:20]
			if(((read_data >> GEPON_PHYRDY_OFFSET) & GEPON_PHYRDY_MASK) == GEPON_PHYRDY_STATUS)
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Ready. \n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy not Ready. \n");
				api_data->ret = PHY_FALSE;
			}
            break; 
			
        case PON_GET_PHY_IS_SYNC:
            //api_data->ret = is_phy_sync(); // for this API, in_data is NULL
            
			read_data = IO_GPHYREG(GEPON_CSR_XPON_STA);
			if( (read_data & GEPON_CSR_XPON_STA_LOS) == GEPON_CSR_XPON_STA_LOS) //it is transceiver sfp rx LOSS no PHY digital
			{
				phyLosStatus= PHY_LOS_HAPPEN;
			}
			else
			{
				phyLosStatus = PHY_NO_LOS_HAPPEN;
			}
			
			read_data = IO_GPHYREG(GEPON_CSR_PHYSTA1);//read bit[18:20]
			if(((read_data >> GEPON_PHYRDY_OFFSET) & GEPON_PHYRDY_MASK) == GEPON_PHYRDY_STATUS)
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Ready. \n");
				phyReadyStatus= PHY_TRUE;
			}
			else
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy not Ready. \n");
				phyReadyStatus = PHY_FALSE;
			}
			
			api_data->ret = ((phyReadyStatus == PHY_TRUE) && (phyLosStatus == PHY_NO_LOS_HAPPEN) );
            break;

        case PON_GET_PHY_MODE:
            api_data->ret = gpPhyPriv->phyCfg.flags.mode;
            break;
            
        case PON_GET_PHY_TX_LONG_FLAG:
            api_data->ret = gpPhyPriv->phyCfg.flags.txLongFlag;
            break;

        case PON_GET_PHY_TRANS_STATUS:
			phy_trans_param_status_real(&api_data->trans_status->params);
			api_data->trans_status->alarms = phy_tx_alarm() | phy_rx_power_alarm();
            break;

        case PON_GET_PHY_GET_TX_POWER_EN_FLAG:
            api_data->ret = gpPhyPriv->phyCfg.flags.txPowerEnFlag;
            break;

        case PON_GET_PHY_BIP_COUNTER:
			  IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, (PHY_BIP_LATCH)); /* latch */
			  api_data->ret = IO_GPHYREG(GEPON_CSR_BIP_CNT);
			  /* clear bip error counter */
			  IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, PHY_BIP_CLR);
            break;

        case PON_GET_PHY_RX_FEC_COUNTER:
			IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, (PHY_ERR_LATCH));//latch
			api_data->rx_fec_cnt->correct_bytes = IO_GPHYREG(GEPON_CSR_ERR_BYTE_CNT);
			api_data->rx_fec_cnt->correct_codewords = IO_GPHYREG(GEPON_CSR_ERR_CODE_CNT);
			api_data->rx_fec_cnt->uncorrect_codewords = IO_GPHYREG(GEPON_CSR_NOSOL_CODE_CNT);
			api_data->rx_fec_cnt->total_rx_codewords = IO_GPHYREG(GEPON_CSR_RX_CODE_CNT);
			api_data->rx_fec_cnt->fec_seconds = IO_GPHYREG(GEPON_CSR_FEC_SECONDS);
            break;

        case PON_GET_PHY_RX_FRAME_COUNTER:
            IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, (PHY_RXFRAME_LATCH));//latch 
			api_data->rx_frame_cnt->frame_count_high = IO_GPHYREG(GEPON_CSR_FRAME_CNT_H);
			api_data->rx_frame_cnt->frame_count_low = IO_GPHYREG(GEPON_CSR_FRAME_CNT_L);
			if(en7580_phy_gpon_mode())//LOF is only used in GPON mode
				api_data->rx_frame_cnt->lof_counter = IO_GPHYREG(GEPON_CSR_LOF_CNT);
	
            break;

        case PON_GET_PHY_RX_FEC_GETTING:
            api_data->ret = IO_GPHYREG(GEPON_CSR_FECDEC_CTL);
            break;

        case PON_GET_PHY_RX_FEC_STATUS:
           	read_data = IO_GPHYREG(GEPON_CSR_PHYRX_STATUS);
			if((read_data & GEPON_PHY_RX_FEC) == GEPON_PHY_RX_FEC)
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "RX FEC on.\n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "RX FEC off.\n");
				api_data->ret = PHY_FALSE;
			}
            break;

        case PON_GET_PHY_TX_FEC_STATUS:
            read_data = IO_GPHYREG(GEPON_CSR_PHYTX_STATUS);
			if((read_data & GEPON_PHY_TX_FEC) == GEPON_PHY_TX_FEC)
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC on. \n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC off. \n");
				api_data->ret = PHY_FALSE;
			}
            break;

        case PON_GET_PHY_TX_BURST_GETTING:
#if ASIC_SERDES
           	read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1);
			if( (read_data & XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK)== XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK )
			{
				api_data->ret = PHY_TX_BURST_MODE;
			}
			else
			{
	           	read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2);
				
				if( (read_data & XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)== XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)
				{
				api_data->ret = PHY_TX_CONT_MODE;
			}
				else
				{
					api_data->ret = PHY_TX_BURST_MODE;
				}
			}
#endif
            break;

        case PON_GET_PHY_TRANS_TX:
			#if A60972_SERDES //EN7580_ASIC
			read_data = IO_GPHYREG(GEPON_CSR_XPON_SETTING);
			if((read_data & ~(GEPON_PHY_TX_SD_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_DISABLE;
			if((read_data & ~(GEPON_PHY_BURST_EN_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_DISABLE;
			if((read_data & ~(GEPON_PHY_TX_FAULT_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_DISABLE;
			#endif
			
			#if ASIC_SERDES //EN7580_ASIC
			read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
			if((read_data & ~(XPON_PMA_XPON_SETTING_1_TX_SD_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_DISABLE;
			
			read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
			if((read_data & ~(XPON_PMA_XPON_SETTING_0_BURST_EN_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_DISABLE;
			if((read_data & ~(XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_DISABLE;
			#endif
            break;

        case PON_GET_PHY_TRANS_RX_GETTING:
			
#if ASIC_SERDES
            read_data = IO_GPHYREG(GEPON_CSR_XPON_SETTING);
			if((read_data & GEPON_PHY_RX_SD_INV) == 0)
				api_data->ret = PHY_DISABLE;
			else
				api_data->ret = PHY_ENABLE;
			#endif
			
			#if 0 //EN7580_ASIC
            read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
			if((read_data & ~(XPON_PMA_XPON_SETTING_0_RX_SD_INV_MASK)) != 0)
				api_data->ret = PHY_ENABLE;
			else
				api_data->ret = PHY_DISABLE;
			#endif
            break;

        case PON_GET_PHY_ROUND_TRIP_DELAY:
			read_data = IO_GPHYREG(GEPON_CSR_ROUND_TRIP_DELAY_VALUE);	  
	  		api_data->ret=read_data&0x0000ffff;

			break;
			
        case PON_GET_PHY_LOF_STATUS:
			//read_data = IO_GPHYREG(LOF);	  
	  		api_data->ret=0;

			break;
			
		case PON_GET_PHY_TX_FRAME_COUNTER:
			read_data = IO_GPHYREG(GEPON_CSR_PHYTX_TEST_TRIG);
			write_data = read_data | PHY_TX_CNT_LCH;//latch
			IO_SPHYREG(GEPON_CSR_PHYTX_TEST_TRIG,write_data);
			read_data = IO_GPHYREG(GEPON_CSR_TX_FRAME_COUNTER);
			api_data->ret = read_data;
			//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Tx Frame Counter : %.8x\n", read_data);
			break;
			
		case PON_GET_PHY_INIT_STATUS:
			api_data->ret = gpPhyPriv->phy_init_done;
			break;
			
        default:
            PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
            api_data->ret = PHY_NO_API;
            break;
    }
	return PHY_SUCCESS;	//ang_20180208
}

int en7580_gepon_phy_set_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;

    api_data->ret = PHY_SUCCESS;
    
    switch(api_data->cmd_id)
    {
        case PON_SET_PHY_MODE_CONFIG:
            api_data->ret = phy_mode_config(api_data->phy_mode_cfg->mode, api_data->phy_mode_cfg->txEnable);

		#if ASIC_SERDES
			if(FALSE==gpPhyPriv->pma_init_done){		// incase of pma_tx_ben keeping high
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
				gpPhyPriv->first_plugin_flag = FALSE;
				gpPhyPriv->pma_init_done= TRUE;
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
			//	printk("==========API: PON_SET_PHY_MODE_CONFIG========= \n");
			}	
		#endif
		
            break;

		case PHY_SET_ERR_CNT_EN:
			read_data = IO_GPHYREG(GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_ERRCNT_MASK;
			if(((*api_data->data) == PHY_ENABLE) || ((*api_data->data) == PHY_DISABLE))
			{
				read_data = read_data | ((*api_data->data) << PHY_ERR_CNT_OFFSET);
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_EN, read_data);
			}
			break;
			
		case PHY_SET_BIP_CNT_EN:
			read_data = IO_GPHYREG(GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_BIPCNT_MASK;
			if(((*api_data->data) == PHY_ENABLE) || ((*api_data->data) == PHY_DISABLE))
			{
				read_data = read_data | ((*api_data->data) << PHY_BIP_CNT_OFFSET);
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_EN, read_data);
			}
			break;

		case PHY_SET_FM_CNT_EN:
			read_data = IO_GPHYREG(GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_FRAMECNT_MASK;
			if(((*api_data->data) == PHY_ENABLE) || ((*api_data->data) == PHY_DISABLE))
			{
				read_data = read_data | ((*api_data->data) << PHY_FRAME_CNT_OFFSET);
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_EN, read_data);
			}
			break;

        case PON_SET_PHY_RESET_COUNTER:
            IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, PHY_ERR_CLR);//clear
			IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, PHY_BIP_CLR);//clear
			IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, PHY_RXFRAME_CLR);//clear
				
            break;

        case PON_SET_PHY_START:
            api_data->ret = xpon_phy_start();
            break;

        case PON_SET_PHY_STOP:
            api_data->ret = xpon_phy_stop();
            break;

        case PON_SET_PHY_FW_READY:
            read_data = IO_GPHYREG(GEPON_CSR_PHYSET2);
			if((*api_data->data) == PHY_ENABLE)
				write_data = read_data & (GEPON_PHY_FW_RDY_MASK) | (GEPON_PHY_FW_RDY_EN);
			else if((*api_data->data) == PHY_DISABLE)
				write_data = read_data & (GEPON_PHY_FW_RDY_MASK);
			else
				api_data->ret = PHY_FAILURE;
			IO_SPHYREG(GEPON_CSR_PHYSET2,write_data);
            break;
				
		case PON_SET_PHY_COUNTER_CLEAR:
			if(((*api_data->data) & PHY_ERR_CNT_CLR)== PHY_ERR_CNT_CLR)
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, PHY_ERR_CLR);//clear
			if(((*api_data->data) & PHY_BIP_CNT_CLR)== PHY_BIP_CNT_CLR)
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, PHY_BIP_CLR);//clear
			if(((*api_data->data) & PHY_RXFRAME_CNT_CLR)== PHY_RXFRAME_CNT_CLR)
				IO_SPHYREG(GEPON_CSR_XP_ERRCNT_CTL, PHY_RXFRAME_CLR);//clear
		
			break;

        case PON_SET_EPONMODE_PHY_RESET:

            break;

        case PON_SET_PHY_BIT_DELAY:
			#if A60972_SERDES || ASIC_SERDES //EN7580_ASIC
            if(((*api_data->data) & GEPON_PHY_BIT_DELAY_LEN_MASK) == 0x00)
			{
				read_data = IO_GPHYREG(GEPON_CSR_PHYSET5);
				write_data = (read_data & GEPON_PHY_BIT_DELAY_MASK) | ((*api_data->data) << GEPON_PHY_DIG_BIT_DELAY_OFFSET) | GEPON_PHY_TX_BIT_DEL_SEL;
				IO_SPHYREG(GEPON_CSR_PHYSET5,write_data);
				return PHY_SUCCESS;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : delay_value. \n");
				return PHY_FAILURE;
			}
			#endif
            break;

        case PON_SET_PHY_TX_POWER_CONFIG:
            phy_tx_power_config(*api_data->data);
            break;

        case PON_SET_PHY_ROGUE_PRBS_CONFIG:
			PON_PHY_PRINT(PHY_MSG_ERR,"rogue_mode=%d, rogue_pattern=%d, tx_d_in_timeslot=%d\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);
            #if A60972_SERDES
			if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_ROGUE_MODE)
			{
        		gpPhyPriv->phyCfg.flags.rogue = 1;
				PON_PHY_PRINT(PHY_MSG_ERR, "Phy_Rogue_PRBS mode\n");
				if(SCU_WAN_CONF_REG_WAN_SEL_GPON==gpPhyPriv->wan_sel)
				{
					//#if A60972_SERDES //EN7580_FPGA
					read_data = IO_GPHYREG(GEPON_CSR_MISC) ;
					write_data = read_data | (0x01<<28);
					IO_SPHYREG(GEPON_CSR_MISC,write_data);
					//#endif

					read_data = IO_GPHYREG(GEPON_CSR_BISTCTL_LOOPBACK_SEL) ;
					write_data = read_data | 0x05;
					IO_SPHYREG(GEPON_CSR_BISTCTL_LOOPBACK_SEL,write_data);
            
					read_data = IO_GPHYREG(GEPON_CSR_BISTCTL_PRBS_TX_EN) ;
					write_data = read_data | 0x01;				
					IO_SPHYREG(GEPON_CSR_BISTCTL_PRBS_TX_EN,write_data);
				}
				else if (SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G==gpPhyPriv->wan_sel)
				{
					//#if A60972_SERDES //EN7580_FPGA
					read_data = IO_GPHYREG(GEPON_CSR_PHYSET3) ;
					write_data = read_data | 0x80;
					IO_SPHYREG(GEPON_CSR_PHYSET3,write_data);
					//#endif
				}
			} 
			else if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_NORMAL_MODE)
			{
				gpPhyPriv->phyCfg.flags.rogue = 0;
				PON_PHY_PRINT(PHY_MSG_ERR, "Phy_NO_Rogue_PRBS mode\n");
				if(SCU_WAN_CONF_REG_WAN_SEL_GPON==gpPhyPriv->wan_sel)
				{
					//#if A60972_SERDES //EN7580_FPGA
					read_data = IO_GPHYREG(GEPON_CSR_MISC) ;
					write_data = read_data & ~(0x01<<28);
					IO_SPHYREG(GEPON_CSR_MISC,write_data);			
					//#endif

					read_data = IO_GPHYREG(GEPON_CSR_BISTCTL_LOOPBACK_SEL) ;
					write_data = read_data & ~0x05;
					IO_SPHYREG(GEPON_CSR_BISTCTL_LOOPBACK_SEL,write_data);

					read_data = IO_GPHYREG(GEPON_CSR_BISTCTL_PRBS_TX_EN) ;
					write_data = read_data & ~0x01;				
					IO_SPHYREG(GEPON_CSR_BISTCTL_PRBS_TX_EN,write_data);
				}
				else if (SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G==gpPhyPriv->wan_sel)
				{
					//#if A60972_SERDES //EN7580_FPGA
					read_data = IO_GPHYREG(GEPON_CSR_PHYSET3) ;
					write_data = read_data & ~0x80;
					IO_SPHYREG(GEPON_CSR_PHYSET3,write_data);
					//#endif
				}			
				
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d\n",api_data->phy_rogue_cfg->rogue_onoff);
			}            
			#endif
			
			#if ASIC_SERDES
			if(gpPhyPriv->first_plugin_flag == TRUE)
			{
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
				gpPhyPriv->first_plugin_flag = FALSE;
				gpPhyPriv->pma_init_done= TRUE;
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
				mdelay(350);	//add by david 20190508  
				phy_trans_power_switch(PHY_ENABLE);	
			}	// by david 20190522	incase onu startup without OLT DS Signal, RX & TX could L2R lock.
			
			if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_ROGUE_MODE)
			{
        		gpPhyPriv->phyCfg.flags.rogue = 1;
				if (api_data->phy_rogue_cfg->tx_d_in_timeslot == 1)
				{
					if(api_data->phy_rogue_cfg->rogue_pattern<=3)
					{
				read_data = IO_GPHYREG(XPON_PMA_BISTCTL_CONTROL) ;
						write_data = (read_data & XPON_PMA_BISTCTL_CONTROL_PAT_SEL_MASK)|(api_data->phy_rogue_cfg->rogue_pattern+1);
				write_data = (write_data & XPON_PMA_BISTCTL_CONTROL_PAT_TX_EN_MASK)|0x100;
				IO_SPHYREG(XPON_PMA_BISTCTL_CONTROL,write_data);
        
				read_data = IO_GPHYREG(XPON_PMA_ADD_XPON_MODE_1);
				write_data = (read_data & XPON_PMA_ADD_XPON_MODE_1_R2T_MODE_MASK);
				write_data = (write_data & XPON_PMA_ADD_XPON_MODE_1_TX_BIST_MASK)|0x10000;				
				IO_SPHYREG(XPON_PMA_ADD_XPON_MODE_1,write_data);				

						PON_PHY_PRINT(PHY_MSG_ERR,"Rogue ON\n");
						PON_PHY_PRINT(PHY_MSG_ERR,"Rogue pattern %d (0=PRBS7,1=PRBS9,2=PRBS15,3=PRBS23)\n",api_data->phy_rogue_cfg->rogue_pattern);
						PON_PHY_PRINT(PHY_MSG_ERR,"In_timeslot Rogue\n");
					}
					else
					{
						PON_PHY_PRINT(PHY_MSG_ERR, "error input: pattern %d(must <=3) while Rogue mode 1 and in_timeslot 1\n",api_data->phy_rogue_cfg->rogue_pattern);
						api_data->ret = PHY_FAILURE;
					}
				}
				else if (api_data->phy_rogue_cfg->tx_d_in_timeslot == 0)
				{
					if(api_data->phy_rogue_cfg->rogue_pattern<=1)
					{
						read_data = IO_GPHYREG(XPON_PMA_MD32_MEM_CLK_CTRL) ;
						write_data = (read_data & XPON_PMA_MD32_MEM_CLK_CTRL_CLK_SEL_MASK)|0x3;
						IO_SPHYREG(XPON_PMA_MD32_MEM_CLK_CTRL,write_data);

						read_data = IO_GPHYREG(XPON_PMA_TX_DLY_CTRL);
						write_data = (read_data & XPON_PMA_TX_DLY_CTRL_OUTBEN_DATA_MASK)|((api_data->phy_rogue_cfg->rogue_pattern+2)<<28);
						IO_SPHYREG(XPON_PMA_TX_DLY_CTRL,write_data);	


						read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1);
						write_data = read_data &( ~XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK);
						IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1, write_data);

						read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2);
						write_data = read_data |(XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK);
						IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2, write_data);

						PON_PHY_PRINT(PHY_MSG_ERR,"Rogue ON\n");
						PON_PHY_PRINT(PHY_MSG_ERR,"Rogue pattern %d (0=PRBS9,1=PRBS31)\n",api_data->phy_rogue_cfg->rogue_pattern);
						PON_PHY_PRINT(PHY_MSG_ERR,"In_timeslot Normal\n");
					}
					else
					{
						PON_PHY_PRINT(PHY_MSG_ERR, "error input: pattern %d(must <=1) while Rogue mode 1 and in_timeslot 0\n",api_data->phy_rogue_cfg->rogue_pattern);
						api_data->ret = PHY_FAILURE;
					}
				}
				else
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input: in_timeslot %d(must <=1)\n",api_data->phy_rogue_cfg->tx_d_in_timeslot);
					api_data->ret = PHY_FAILURE;
				}
			} 
			else if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_NORMAL_MODE)
			{
				gpPhyPriv->phyCfg.flags.rogue = 0;
				PON_PHY_PRINT(PHY_MSG_ERR, "Phy_NO_Rogue_PRBS mode\n");
				read_data = IO_GPHYREG(XPON_PMA_BISTCTL_CONTROL) ;
				write_data = (read_data & XPON_PMA_BISTCTL_CONTROL_PAT_SEL_MASK);
				write_data = (write_data & XPON_PMA_BISTCTL_CONTROL_PAT_TX_EN_MASK);
				IO_SPHYREG(XPON_PMA_BISTCTL_CONTROL,write_data);
        
				read_data = IO_GPHYREG(XPON_PMA_ADD_XPON_MODE_1);
				write_data = (read_data & XPON_PMA_ADD_XPON_MODE_1_R2T_MODE_MASK);
				write_data = (write_data & XPON_PMA_ADD_XPON_MODE_1_TX_BIST_MASK);				
				IO_SPHYREG(XPON_PMA_ADD_XPON_MODE_1,write_data);

				read_data = IO_GPHYREG(XPON_PMA_MD32_MEM_CLK_CTRL) ;
				write_data = (read_data & XPON_PMA_MD32_MEM_CLK_CTRL_CLK_SEL_MASK)|0x3;
				IO_SPHYREG(XPON_PMA_MD32_MEM_CLK_CTRL,write_data);

				read_data = IO_GPHYREG(XPON_PMA_TX_DLY_CTRL);
				write_data = (read_data & XPON_PMA_TX_DLY_CTRL_OUTBEN_DATA_MASK)|(1<<28);
				IO_SPHYREG(XPON_PMA_TX_DLY_CTRL,write_data);	


				read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1);
				write_data = read_data |(XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK);
				IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1, write_data);

				read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2);
				write_data = read_data &(~XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK);
				IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2, write_data);
			}
			else
			{
				//PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1)\n",api_data->phy_rogue_cfg->rogue_onoff);
				api_data->ret = PHY_FAILURE;
			}
			#endif
            break;

        case PON_SET_PHY_DEV_INIT:
            api_data->ret = phy_dev_init();
            break;

        case PON_SET_PHY_TRANS_POWER_SWITCH:
            api_data->ret = phy_trans_power_switch(*api_data->data);
            break;

        case PON_SET_PHY_RESTORE_PREAMBLE_AND_GUARD_BIT:
            break;

        case PON_SET_PHY_TRANS_MODEL_SETTING:
			if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{
				#if A60972_SERDES //EN7580_ASIC
            	IO_SPHYREG(GEPON_CSR_XPON_SETTING, phy_trans_iot_list[gpPhyPriv->trans_index].gepon_csr_xpon_setting);
				#endif
				
				#if ASIC_SERDES //EN7580_ASIC
				read_data=IO_GPHYREG(GEPON_CSR_XPON_SETTING);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Read GEPON_CSR_XPON_SETTING(0x%x)=0x%x \n",GEPON_CSR_XPON_SETTING,read_data);
				IO_SPHYREG(GEPON_CSR_XPON_SETTING,phy_trans_iot_list[gpPhyPriv->trans_index].gepon_csr_xpon_setting) ;
				read_data=IO_GPHYREG(GEPON_CSR_XPON_SETTING);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Set GEPON_CSR_XPON_SETTING(0x%x)=0x%x \n",GEPON_CSR_XPON_SETTING,read_data);

				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Read XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_0,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Set XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);

				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Read XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_1,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_1) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Set XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);	
				#endif

			}
            break;

		case PON_SET_PHY_SCU_RST:
			#if ASIC_SERDES
			phy_trans_power_switch(PHY_TX_DIS_ON_HW_ONLY); //tx_dis_reset_protection
			gpPhyPriv->debugLevel_backup=gpPhyPriv->debugLevel;
			gpPhyPriv->debugLevel=PHY_MSG_OFF;
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			//PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
			xPON_Phy_Reset();
			gpPhyPriv->phy_init_done=FALSE;
			gpPhyPriv->is_phy_start = FALSE;
			gpPhyPriv->phy_init_done = FALSE; 
			gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
			gpPhyPriv->trans_tx_enable = PHY_DISABLE;
			gpPhyPriv->phy_unexpected_isr_flag=FALSE;
			gpPhyPriv->first_plugin_flag = TRUE; //ang_20180926
			gpPhyPriv->pma_init_done = FALSE;	//ang_20190108
			gpPhyPriv->trans_msg_print_cnt = 95;
			pon_phy_mod_init();
			if(gpPhyPriv->rx_fec_setting == PHY_ENABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_ENABLE;
				if((gpPhyPriv->scu_hir_np_sys_hw_id == 0xa)&&
				((gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_EPON)||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G)))
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "EN7580 auto FEC\n");
				}
				else
				{
				PON_PHY_PRINT(PHY_MSG_DBG, "Rx FEC Enable\n");
					IO_SPHYREG(GEPON_CSR_FECDEC_CTL,0x1);
				}
			}
			else if(gpPhyPriv->rx_fec_setting == PHY_DISABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_DISABLE;
				if((gpPhyPriv->scu_hir_np_sys_hw_id == 0xa)&&
				((gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_EPON)||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G)))
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "EN7580 auto FEC\n");
				}
				else
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "Rx FEC Disable\n");
					IO_SPHYREG(GEPON_CSR_FECDEC_CTL,0x0);
				}
			}
			xpon_phy_start();
			gpPhyPriv->debugLevel=gpPhyPriv->debugLevel_backup;
			phy_trans_power_switch(PHY_TX_DIS_RESTORE_BY_SW);  //tx_dis_reset_protection
			PON_PHY_PRINT(PHY_MSG_ERR,"PHY_SCU_RESET\n");
			#endif
			break;


        case PON_SET_PHY_RX_FEC_SETTING:
			if(*api_data->data == PHY_ENABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_ENABLE;
				if((gpPhyPriv->scu_hir_np_sys_hw_id == 0xa)&&
				((gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_EPON)||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G)))
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "EN7580 auto FEC\n");
				}
				else
				{
				PON_PHY_PRINT(PHY_MSG_DBG, "Rx FEC Enable\n");
					IO_SPHYREG(GEPON_CSR_FECDEC_CTL,0x1);
				}
				api_data->ret = PHY_SUCCESS;
			}
			else if(*api_data->data == PHY_DISABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_DISABLE;
				if((gpPhyPriv->scu_hir_np_sys_hw_id == 0xa)&&
				((gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_EPON)||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G)))
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "EN7580 auto FEC\n");
				}
				else
				{
				PON_PHY_PRINT(PHY_MSG_DBG, "Rx FEC Disable\n");
					IO_SPHYREG(GEPON_CSR_FECDEC_CTL,0x0);
				}
				api_data->ret = PHY_SUCCESS;
			}
			else
			{
				gpPhyPriv->rx_fec_setting=PHY_DEFAULT;
			}
            break;

        case PON_SET_PHY_TX_BURST_CONFIG:
			
#if ASIC_SERDES
			PON_PHY_PRINT(PHY_MSG_ERR, "TX_BURST_CONFIG=%d\n",(*api_data->data));
			if (*api_data->data == PHY_TX_CONT_MODE)
			{
				read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1);
				write_data = read_data &( ~XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config read_data:0x%.8x\n", read_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config write_data:0x%.8x\n", write_data);
				IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1, write_data);
				
				read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2);
				write_data = read_data |(XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config read_data:0x%.8x\n", read_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config write_data:0x%.8x\n", write_data);
				IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2, write_data);

								//sw workaround
				//read_data = IO_GPHYREG(GEPON_CSR_XPON_STA);
				//if( (read_data & GEPON_CSR_XPON_STA_LOS) == GEPON_CSR_XPON_STA_LOS) //it is transceiver sfp rx LOSS no PHY digital
				//{
					/*reset phy*/
				//	fiber_plug_reset();
				//}
				
				gpPhyPriv->phyCfg.flags.txLongFlag = ((*api_data->data==PHY_TX_CONT_MODE)?PHY_TRUE:PHY_FALSE);
			
				if (*api_data->data == PHY_TX_BURST_MODE){
					phy_tx_power_config(PHY_ENABLE);
					phy_trans_power_switch(PHY_ENABLE);
				}
				
				api_data->ret = PHY_SUCCESS;
			}
			else if (*api_data->data == PHY_TX_BURST_MODE)
			{
				read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1);
				write_data = read_data |(XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config read_data:0x%.8x\n", read_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config write_data:0x%.8x\n", write_data);
				IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1, write_data);
				
				read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2);
				write_data = read_data &(~XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config read_data:0x%.8x\n", read_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config write_data:0x%.8x\n", write_data);
				IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2, write_data);
				//sw workaround
				//read_data = IO_GPHYREG(GEPON_CSR_XPON_STA);
				//if( (read_data & GEPON_CSR_XPON_STA_LOS) == GEPON_CSR_XPON_STA_LOS) //it is transceiver sfp rx LOSS no PHY digital
				//{
					/*reset phy*/
				//	fiber_plug_reset();
				//}
				
				gpPhyPriv->phyCfg.flags.txLongFlag = ((*api_data->data==PHY_TX_CONT_MODE)?PHY_TRUE:PHY_FALSE);
			
				if (*api_data->data == PHY_TX_BURST_MODE){
					phy_tx_power_config(PHY_ENABLE);
					phy_trans_power_switch(PHY_ENABLE);
				}
				
				api_data->ret = PHY_SUCCESS;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : burst_mode = %d\n",(*api_data->data));
				api_data->ret = PHY_FAILURE;
			}
#endif
            break;

        case PON_SET_PHY_GPON_DELIMITER_GUARD:
			if((api_data->delimiter_guard->delimiter & GEPON_PHY_GPON_DELIM_PAT_LEN_MASK) != 0)
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "error input : delimiter pattern. \n");
				api_data->ret = PHY_FAILURE;
			}
			else
			{
				write_data = (api_data->delimiter_guard->guard_time << GEPON_PHY_GPON_GUARD_PAT_OFFSET) | api_data->delimiter_guard->delimiter;
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Gpon_Delimiter_Guard write_data:0x%.8x\n", write_data);
				gpPhyPriv->phyGuardBitDelm = write_data;
				IO_SPHYREG(GEPON_CSR_GPON_DELIMITER_GUARD, write_data);
				api_data->ret = PHY_SUCCESS;
			}

            break;

        case PON_SET_PHY_TRANS_TX_SETTINGS:
			#if A60972_SERDES //EN7580_ASIC
            read_data = IO_GPHYREG(GEPON_CSR_XPON_SETTING);
			if((api_data->tx_trans_cfg->trans_tx_sd_inv_status == PHY_ENABLE) || (api_data->tx_trans_cfg->trans_tx_sd_inv_status == PHY_DISABLE) )//tx_sd
				read_data = (read_data & GEPON_PHY_TX_SD_INV_MASK) | (api_data->tx_trans_cfg->trans_tx_sd_inv_status << GEPON_PHY_TX_SD_INV_OFFSET);
			else 
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : tx_sd_inv.\n");

			if((api_data->tx_trans_cfg->trans_burst_en_inv_status == PHY_ENABLE) || (api_data->tx_trans_cfg->trans_burst_en_inv_status == PHY_DISABLE) )//burst_en
				read_data = (read_data & GEPON_PHY_BURST_EN_INV_MASK) | (api_data->tx_trans_cfg->trans_burst_en_inv_status<< GEPON_PHY_BURST_EN_INV_OFFSET);
			else 
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : burst_en_inv.\n");

			if((api_data->tx_trans_cfg->trans_tx_fault_inv_status == PHY_ENABLE) || (api_data->tx_trans_cfg->trans_tx_fault_inv_status == PHY_DISABLE) )//tx_fault
				read_data = (read_data & GEPON_PHY_TX_FAULT_INV_MASK) | (api_data->tx_trans_cfg->trans_tx_fault_inv_status << GEPON_PHY_TX_FAULT_INV_OFFSET);
			else 
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : tx_fault_inv.\n");

			write_data = read_data;
			IO_SPHYREG(GEPON_CSR_XPON_SETTING,write_data);
			#endif
			
			#if ASIC_SERDES //EN7580_ASIC
            read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
			if((api_data->tx_trans_cfg->trans_tx_sd_inv_status == PHY_ENABLE) || (api_data->tx_trans_cfg->trans_tx_sd_inv_status == PHY_DISABLE) )//tx_sd
				write_data = (read_data & XPON_PMA_XPON_SETTING_1_TX_SD_INV_MASK) | (api_data->tx_trans_cfg->trans_tx_sd_inv_status << XPON_PMA_XPON_SETTING_1_TX_SD_INV_OFFSET);
			else 
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : tx_sd_inv.\n");
			IO_SPHYREG(XPON_PMA_XPON_SETTING_1,write_data);

            read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
			if((api_data->tx_trans_cfg->trans_burst_en_inv_status == PHY_ENABLE) || (api_data->tx_trans_cfg->trans_burst_en_inv_status == PHY_DISABLE) )//burst_en
				read_data = (read_data & XPON_PMA_XPON_SETTING_0_BURST_EN_INV_MASK) | (api_data->tx_trans_cfg->trans_burst_en_inv_status<< XPON_PMA_XPON_SETTING_0_BURST_EN_INV_OFFSET);
			else 
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : burst_en_inv.\n");

			if((api_data->tx_trans_cfg->trans_tx_fault_inv_status == PHY_ENABLE) || (api_data->tx_trans_cfg->trans_tx_fault_inv_status == PHY_DISABLE) )//tx_fault
				read_data = (read_data & XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_MASK) | (api_data->tx_trans_cfg->trans_tx_fault_inv_status << XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_OFFSET);
			else 
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : tx_fault_inv.\n");

			write_data = read_data;
			IO_SPHYREG(XPON_PMA_XPON_SETTING_0,write_data);
			#endif
            break;

        case PON_SET_PHY_TRANS_RX_SETTING:
			#if 1 //EN7580_ASIC
            read_data = IO_GPHYREG(GEPON_CSR_XPON_SETTING);//rx_sd
	
			if(((*api_data->data) == PHY_ENABLE))//rx_sd
				write_data = (read_data & ~(GEPON_PHY_RX_SD_INV)) | (GEPON_PHY_RX_SD_INV);
			else if(((*api_data->data) == PHY_DISABLE))
				write_data = (read_data & ~(GEPON_PHY_RX_SD_INV)) | (GEPON_PHY_RX_SD);
			else 
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "error input : rx_sd_inv.\n");
				return PHY_FAILURE;
			}
			PON_PHY_PRINT(PHY_MSG_DBG, "rx_sd_inv success \n");
			IO_SPHYREG(GEPON_CSR_XPON_SETTING,write_data);
			#endif
			
#if 0 //EN7580_ASIC
            read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
			if(((*api_data->data) == PHY_ENABLE) || ((*api_data->data) == PHY_DISABLE) )//tx_sd
				write_data = (read_data & XPON_PMA_XPON_SETTING_0_RX_SD_INV_MASK) | ((*api_data->data) << XPON_PMA_XPON_SETTING_0_RX_SD_INV_OFFSET);
			else 
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "error input : rx_sd_inv.\n");
				return PHY_FAILURE;
			}
			PON_PHY_PRINT(PHY_MSG_DBG, "rx_sd_inv success \n");
			IO_SPHYREG(XPON_PMA_XPON_SETTING_0,write_data);

#endif
            break;

        case PON_SET_PHY_GPON_PREAMBLE:
            read_data = IO_GPHYREG(GEPON_CSR_GPON_PREAMBLE);
			//PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Gpon_Preamble read_data:0x%.8x\n", read_data);
			if((api_data->gpon_preamble->mask & PHY_GUARD_BIT_NUM_EN) == PHY_GUARD_BIT_NUM_EN )
			{
				read_data = (read_data & GEPON_PHY_GUARD_BIT_NUM_MASK) | api_data->gpon_preamble->guard_bit_num;
			}
			if((api_data->gpon_preamble->mask & PHY_PRE_T1_NUM_EN) == PHY_PRE_T1_NUM_EN)
			{
				read_data = (read_data & GEPON_PHY_PRE_T1_NUM_MAKS) | (api_data->gpon_preamble->preamble_t1_num << GEPON_PHY_PRE_T1_NUM_OFFSET);
			}
			if((api_data->gpon_preamble->mask & PHY_PRE_T2_NUM_EN) == PHY_PRE_T2_NUM_EN)
			{
				read_data = (read_data & GEPON_PHY_PRE_T2_NUM_MASK) | (api_data->gpon_preamble->preamble_t2_num << GEPON_PHY_PRE_T2_NUM_OFFSET);
			}
			if((api_data->gpon_preamble->mask & PHY_PRE_T3_PAT_EN) == PHY_PRE_T3_PAT_EN)
			{
				read_data = (read_data & GEPON_PHY_PRE_T3_PAT_MASK) | (api_data->gpon_preamble->preamble_t3_pat << GEPON_PHY_PRE_T3_PAT_OFFSET);
			}
			write_data = read_data;
			//PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Gpon_Preamble write_data:0x%.8x\n", write_data);
		    gpPhyPriv->phyPreamble = write_data;
			IO_SPHYREG(GEPON_CSR_GPON_PREAMBLE, write_data);
            break;

        case PON_SET_PHY_GPON_EXTEND_PREAMBLE:
            read_data = IO_GPHYREG(GEPON_CSR_GPON_EXTENDED_PREAMBLE);
			//PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Gpon_Extend_Preamble read_data:0x%.8x\n", read_data);

			if((api_data->gpon_preamble->mask & PHY_T3_O4_PRE_EN) == PHY_T3_O4_PRE_EN)
				read_data = (read_data & GEPON_PHY_T3_O4_PRE_MASK) | api_data->gpon_preamble->t3_O4_preamble;
			if((api_data->gpon_preamble->mask & PHY_T3_O5_PRE_EN) == PHY_T3_O5_PRE_EN)
				read_data = (read_data & GEPON_PHY_T3_O5_PRE_MASK) | (api_data->gpon_preamble->t3_O5_preamble << GEPON_PHY_T3_O5_PRE_OFFSET);
			if((api_data->gpon_preamble->mask & PHY_EXT_BUR_MODE_EN) == PHY_EXT_BUR_MODE_EN)
			{
				if((api_data->gpon_preamble->extend_burst_mode != PHY_ENABLE) && (api_data->gpon_preamble->extend_burst_mode!= PHY_DISABLE))
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input : extend_burst_mode. \n");
					api_data->ret = PHY_FAILURE;
				}
				read_data = (read_data & GEPON_PHY_EXT_BUR_MODE_MASK) | (api_data->gpon_preamble->extend_burst_mode << GEPON_PHY_EXT_BUR_MODE_OFFSET);
			}
			if((api_data->gpon_preamble->mask & PHY_OPER_RANG_EN) == PHY_OPER_RANG_EN)
			{
				if( (api_data->gpon_preamble->oper_ranged_st & GEPON_PHY_OPER_RANG_LEG_MASK) != 0x00)
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input : oper_ranged_st. \n");
					api_data->ret = PHY_FAILURE;
				}
				read_data = (read_data & GEPON_PHY_OPER_RANG_MASK) | (api_data->gpon_preamble->oper_ranged_st<< GEPON_PHY_OPER_RANG_OFFSET);
			}
			if((api_data->gpon_preamble->mask & PHY_EXTB_LENG_SEL_EN) == PHY_EXTB_LENG_SEL_EN)
			{
				if( (api_data->gpon_preamble->extb_length_sel & GEPON_PHY_EXTB_LENG_SEL_LEN_MASK) != 0x00)
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input : oper_ranged_st. \n");
					api_data->ret = PHY_FAILURE;
				}
				read_data = (read_data & GEPON_PHY_EXTB_LENG_SEL_MASK) | (api_data->gpon_preamble->extb_length_sel<< GEPON_PHY_EXTB_LENG_SEL_OFFSET);
			}
			if((api_data->gpon_preamble->mask & PHY_DIS_SCRAM_EN) == PHY_DIS_SCRAM_EN)
			{
				if((api_data->gpon_preamble->dis_scramble != PHY_ENABLE) && (api_data->gpon_preamble->dis_scramble!= PHY_DISABLE))
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input : dis_scramble. \n");
					api_data->ret = PHY_FAILURE;
				}	
				read_data = (read_data & GEPON_PHY_DIS_SCRAM_MASK) | (api_data->gpon_preamble->dis_scramble<< GEPON_PHY_DIS_SCRAM_OFFSET);
			}
			write_data = read_data;

			//PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Gpon_Extend_Preamble write_data:0x%.8x\n", write_data);
			gpPhyPriv->phyExtPreamble = write_data;
			IO_SPHYREG(GEPON_CSR_GPON_EXTENDED_PREAMBLE, write_data);
            break;
			
		case PHY_SET_TX_FEC_EN:
			if((*api_data->data) == PHY_ENABLE)
			{
				read_data = IO_GPHYREG(GEPON_CSR_PHYTX_MISC);
				write_data = (read_data & GEPON_PHY_TX_FEC_MANUAL_MASK) | GEPON_PHY_TX_FEC_MANUAL | GEPON_PHY_TX_FEC_EN;//clear and set
				IO_SPHYREG(GEPON_CSR_PHYTX_MISC,write_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Tx FEC manual enable setting success. \n");
				api_data->ret = PHY_SUCCESS;
			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				read_data = IO_GPHYREG(GEPON_CSR_PHYTX_MISC);
				write_data = (read_data & GEPON_PHY_TX_FEC_MANUAL_MASK);//clear setting
				IO_SPHYREG(GEPON_CSR_PHYTX_MISC,write_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Tx FEC manual disable setting success. \n");
				api_data->ret = PHY_SUCCESS;
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			break;


#ifdef TCSUPPORT_CPU_EN7521
		case PON_SET_PHY_EPON_TS_CONTINUE_MODE:
            api_data->ret = en7580_phy_set_epon_ts_continue_mode(*api_data->data);
            break;
#endif       
#if (defined(TCSUPPORT_WAN_GPON) || defined(TCSUPPORT_WAN_EPON))&& defined(TCSUPPORT_XPON_HAL_API_EXT)
        case PON_SET_PHY_NOTIFY_EVENT:
            if(gpPhyPriv->phyXponStateNotifyHandler){
                gpPhyPriv->phyXponStateNotifyHandler(*api_data->data);
            }
            break;
#endif
		case PON_SET_PHY_TRAFFIC_STATUS:
			api_data->ret = phy_set_traffic_status(*api_data->data);
				break;
#ifdef TCSUPPORT_MT7570
		case PON_SET_PHY_EN7571_INIT_DONE:
			if((*api_data->data) == PHY_TRUE)
			{
				gpPhyPriv->en7571_init_done = TRUE;
			}
			else if((*api_data->data) == PHY_FALSE)
			{
				gpPhyPriv->en7571_init_done = FALSE;
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			PON_PHY_PRINT(PHY_MSG_ERR, "EN7571 Init %s\n",(gpPhyPriv->en7571_init_done==TRUE)?"Done":"Fail");

			break;

		case PON_SET_PHY_RX_CDR:
#if ASIC_SERDES
			if((*api_data->data) == PHY_ENABLE)
			{
				xpon_phy_stop();
				gpPhyPriv->phy_init_done=FALSE;
				gpPhyPriv->first_plugin_flag = TRUE; 
				pon_phy_mod_init();		// xpon_phy_10g module reset (the most complete reset cmd)
				xpon_phy_start();
			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				xpon_phy_stop();
				xpon_init(gpPhyPriv->wan_sel);
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel); 							// reset pma and first_plug_in
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	// force RX L2R lock
				phy_int_config(PHY_INT_EN_MODE_MANUAL, PHY_INT_EN_DISABLE);		//disable PON_PHY isr mask
				PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);	// stop event_poll_timer	
				mdelay(350);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
#endif
			break;
#endif

#ifdef LDDLA_SUPPORT_SET_TX_MODE
		case PON_SET_PHY_LDDLA_AUTO_MODE:
			if((*api_data->data) == PHY_ENABLE)
			{
				gpPhyPriv->lddla_auto_mode= TRUE; 

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				gpPhyPriv->lddla_auto_mode= FALSE;
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			PON_PHY_PRINT(PHY_MSG_ERR, "MAC set lddla_auto_mode = %s\n",(gpPhyPriv->lddla_auto_mode==TRUE)?"TRUE":"FALSE");
			break;
#endif
		case PON_SET_PHY_LOGIC_RESET:
			phy_pma_reset();
			PON_PHY_PRINT(PHY_MSG_ERR,"PHY_LOGIC_RESET.\n");
			break;

        default:
            PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
            api_data->ret = PHY_NO_API;
            break;
    }
	return PHY_SUCCESS;	//ang_20180208
}



int en7580_gepon_phy_isr(char* buf)
{
	ulong flags;
	uint phyIntStatus, phyIntEn ;
    PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_HW_IRQ };
	static uint isr_cnt=0,phy_rdy_cnt=0,phy_los_cnt=0,phy_rx_rdy_cnt=0,phy_lof_cnt=0,phy_other_cnt=0;

	static uint trans_laser_detected = FALSE;
	UINT32 read_data = 0;
	UINT32 write_data = 0;

	isr_cnt++;
	
	spin_lock_irqsave(&gpPhyPriv->event_handle_lock, flags);
	
	/*get interrupt status and clear*/
	phyIntStatus=IO_GPHYREG(GEPON_CSR_XPON_INT_STA);
	IO_SPHYREG(GEPON_CSR_XPON_INT_STA_CLR, phyIntStatus);
	phyIntEn=IO_GPHYREG(GEPON_CSR_XPON_INT_EN);
	phy_print_time();

	#if ASIC_SERDES
	if(trans_laser_detected==TRUE)
	{
		if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & GEPON_CSR_XPON_PHYRDY_INT)) 
		{
			phy_rdy_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
		#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
		#endif
			trans_laser_detected=FALSE;
		}
		else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & GEPON_CSR_XPON_TRANS_LOS_INT)) 
		{
			phy_los_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS when laser on ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			trans_laser_detected=FALSE;
			phy_fw_ready(PHY_DISABLE);
			#if ASIC_SERDES	// for ASIC
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
		} 
		else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & GEPON_CSR_XPON_TRANS_LOS_INT)) 
		{
			phy_los_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS when ready ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
			#endif
			trans_laser_detected=FALSE;

			
			//read_data = IO_GPHYREG(GEPON_CSR_PHYSET3);
			//write_data = read_data |GEPON_CSR_PHYSET3_PLL_RST;	// |GEPON_CSR_PHYSET3_PHY_RST|GEPON_CSR_PHYSET3_SOFTWARE_RST;  //GEPON_CSR_PHYSET3_LOS_RST
			//IO_SPHYREG(GEPON_CSR_PHYSET3, write_data) ;
			phy_fw_ready(PHY_DISABLE);
			#if ASIC_SERDES	// for ASIC
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
		} 
		else
		{
			phy_other_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected when laser on fw=0x%x, EN=0x%08x GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",gpPhyPriv->phy_status,phyIntEn,isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
        }
	}
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & GEPON_CSR_XPON_TRANS_LOS_INT))
	{
		phy_los_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
		#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
		#endif
		
		//read_data = IO_GPHYREG(GEPON_CSR_PHYSET3);
		//write_data = read_data |GEPON_CSR_PHYSET3_PLL_RST;	// |GEPON_CSR_PHYSET3_PHY_RST|GEPON_CSR_PHYSET3_SOFTWARE_RST;  //GEPON_CSR_PHYSET3_LOS_RST
		//IO_SPHYREG(GEPON_CSR_PHYSET3, write_data) ;
		
		phy_fw_ready(PHY_DISABLE);
		#if ASIC_SERDES	// for ASIC
		fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
		#endif
	}
	else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & GEPON_CSR_XPON_NO_LOS_INT))
	{
		phy_rx_rdy_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_NO_LOS Reset__Serdes GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		#if (A60972_SERDES)||(ASIC_SERDES)
		//mdelay(10);
		phy_pma_reset();
		#else
		IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_SERDES);
		IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_DISABLE);
		#endif
		trans_laser_detected = TRUE;
		phy_print_time();
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_NO_LOS Reset__Serdes Done\r\n");
	}
	else if(phyIntStatus & GEPON_CSR_XPON_LOF_INT)  // LOF may happen after LOS
	{
		phy_lof_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_LOF GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x,phyrx_status=0x%x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus,IO_GPHYREG(GEPON_CSR_PHYRX_STATUS));
	}
	else
	{
		phy_other_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected! laser=0x%x, fw=0x%x, EN=0x%08x GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",trans_laser_detected,gpPhyPriv->phy_status,phyIntEn,isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
	}
	
#endif

#if A60972_SERDES

	if(trans_laser_detected==TRUE)
	{
		if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & GEPON_CSR_XPON_PHYRDY_INT)) 
		{
			phy_rdy_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
		#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
		#endif
			trans_laser_detected=FALSE;
		}
		else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & GEPON_CSR_XPON_TRANS_LOS_INT)) 
		{
			phy_los_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS when laser on ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			trans_laser_detected=FALSE;
		} 
		else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & GEPON_CSR_XPON_TRANS_LOS_INT)) 
		{
			phy_los_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS when ready ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
			#endif
			trans_laser_detected=FALSE;
		} 
		else
		{
			phy_other_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected when laser on fw=0x%x, EN=0x%08x GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",gpPhyPriv->phy_status,phyIntEn,isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
        }
	}
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & GEPON_CSR_XPON_TRANS_LOS_INT))
	{
		phy_los_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
		#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
		#endif
	}
	else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & GEPON_CSR_XPON_NO_LOS_INT))
	{
		phy_rx_rdy_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_NO_LOS Reset__Serdes GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		#if A60972_SERDES
		mdelay(10);
		phy_pma_reset();
		#else
		IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_SERDES);
		IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_DISABLE);
		#endif
		trans_laser_detected = TRUE;
		phy_print_time();
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_NO_LOS Reset__Serdes Done\r\n");
	}
	else if(phyIntStatus & GEPON_CSR_XPON_LOF_INT)  // LOF may happen after LOS
	{
		phy_lof_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_LOF GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x,phyrx_status=0x%x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus,IO_GPHYREG(GEPON_CSR_PHYRX_STATUS));
	}
	else
    {
		phy_other_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected! laser=0x%x, fw=0x%x, EN=0x%08x GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",trans_laser_detected,gpPhyPriv->phy_status,phyIntEn,isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
	}
	
#endif


#ifndef PON_PHY_PERIODIC_POLLING
	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
#endif	

	spin_unlock(&gpPhyPriv->event_handle_lock); 

	return PHY_SUCCESS;

}

int en7580_gepon_phy_event_poll(char* buf)
{
	ulong flags;
	PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_SW_POLL};
	int phyReadyStatus=0,phyLosStatus=0;
	static UINT32 poll_set_los_cnt=0,poll_set_ready_cnt=0,poll_no_ready_no_los_cnt=0,poll_los_cnt=0,poll_ready_cnt=0;
	static PHY_Los_Status_t  pre_phy_status=PHY_LINK_STATUS_UNKNOWN;
	UINT32 read_data = 0;

	if(FALSE == gpPhyPriv->is_phy_start){
		return PHY_FAILURE;	//ang_20180208
	}

	gpPhyPriv->event_poll_timer_value = 1500;

	spin_lock_irqsave(&gpPhyPriv->event_handle_lock, flags);

	//PON_PHY_PRINT(PHY_MSG_INT," - POLLING - __START__ \r\n");

	phyReadyStatus=phy_ready_status();
	phyLosStatus=phy_los_status();

	
	read_data = IO_GPHYREG(GEPON_CSR_PHYSTA1);//read bit[18:20]
	if(((read_data >> GEPON_PHYRDY_OFFSET) & GEPON_PHYRDY_MASK) == GEPON_PHYRDY_STATUS)
	{
		phyReadyStatus = PHY_TRUE;
	}
	else
	{
		phyReadyStatus = PHY_FALSE;
	}
	
	read_data = IO_GPHYREG(GEPON_CSR_XPON_STA);
	if( (read_data & GEPON_CSR_XPON_STA_LOS) == GEPON_CSR_XPON_STA_LOS) //it is transceiver sfp rx LOSS no PHY digital
	{
		phyLosStatus = PHY_LOS_HAPPEN;
	}
	else
	{
		phyLosStatus = PHY_NO_LOS_HAPPEN;
	}
	
	if(PHY_LINK_STATUS_READY == gpPhyPriv->phy_status) 
	{
		poll_no_ready_no_los_cnt=0;
		//if(!is_phy_sync())  /* LOS */
		if(!(phyReadyStatus&&(!phyLosStatus)))  /* LOS */
		{
			poll_set_los_cnt++;
			#if (A60972_SERDES)||(ASIC_SERDES)
			phy_print_time();
			PON_PHY_PRINT(PHY_MSG_INT," POLLING => LOS(%d) from ready, ReadyStatus=%d, LosStatus=%d \r\n",poll_set_los_cnt,phyReadyStatus,phyLosStatus);
			//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;
			#endif
			//gpPhyPriv->event_poll_timer.data = 2000;

			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			phy_event.id = PHY_EVENT_TRANS_LOS_INT;
			phy_event_handler(&phy_event);
			#endif
		}
		else /* READY */
		{
			PON_PHY_PRINT(PHY_MSG_INT," POLLING => READY(%d) from ready ReadyStatus=%d,LosStatus=%d\r\n",poll_set_ready_cnt,phyReadyStatus,phyLosStatus);
		}
	}
	else
	{		  
		//if(phy_no_ready_no_los())
		if((!phyReadyStatus)&&(!phyLosStatus))  /* NO LOS NO READY */
		{
			poll_no_ready_no_los_cnt++;

			#if ASIC_SERDES
			phy_print_time();
			if(poll_no_ready_no_los_cnt%10 ==0)
			{
				PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)SCU Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
				PHY_INTERNAL_SET_API(PON_SET_PHY_SCU_RST,NULL);
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)Reset pma,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);// ang_20190307			//disable rx			
				phy_pma_reset();	
			}

			gpPhyPriv->event_poll_timer_value = 3500;
			PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4

			#endif
			
			#if A60972_SERDES
			xpon_pma_init();
			phy_pma_reset();
			gpPhyPriv->event_poll_timer_value = 3500;
			PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
			gpPhyPriv->event_poll_timer_value = 1500;
			#endif
			
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			#ifdef LDDLA_SUPPORT_SET_TX_MODE
			phy_event.id = PHY_EVENT_NO_LOS_NO_READY;
			phy_event_handler(&phy_event);
			#endif
			#endif
		}													
		else if(phyReadyStatus&&(!phyLosStatus)) /* READY */
		{
			poll_set_ready_cnt++;
			poll_no_ready_no_los_cnt=0;
			phy_print_time();
			PON_PHY_PRINT(PHY_MSG_INT," POLLING => READY(%d) ReadyStatus=%d,LosStatus=%d\r\n",poll_set_ready_cnt,phyReadyStatus,phyLosStatus);

			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
			
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			phy_event.id = PHY_EVENT_PHYRDY_INT;
			phy_event_handler(&phy_event);
			#endif
		}
		else  /* LOS */
		{
			PON_PHY_PRINT(PHY_MSG_INT," POLLING => LOS(%d) ReadyStatus=%d, LosStatus=%d \r\n",poll_set_los_cnt,phyReadyStatus,phyLosStatus);
		}
	}

#ifdef PON_PHY_PERIODIC_POLLING
	if(pre_phy_status!=gpPhyPriv->phy_status)
	{
		phy_print_time();
		PON_PHY_PRINT(PHY_MSG_INT," - POLLING - __END__ (%d)(%d)(%d) phy_status = %d\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status);
	}
	pre_phy_status=gpPhyPriv->phy_status;

	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
#else
	if(PHY_LINK_STATUS_LOS==gpPhyPriv->phy_status)
	{
		phy_event.id = PHY_EVENT_TRANS_LOS_INT;
		poll_los_cnt++;
	}
	else if(PHY_LINK_STATUS_READY==gpPhyPriv->phy_status)
	{
		phy_event.id = PHY_EVENT_PHYRDY_INT;
		poll_ready_cnt++;
	}
#ifdef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
	if(pre_phy_status!=gpPhyPriv->phy_status)
	{
		phy_event_handler(&phy_event);
	}
	pre_phy_status=gpPhyPriv->phy_status;
#endif
	PON_PHY_PRINT(PHY_MSG_DBG," - POLLING - __END__ (%d)(%d)(%d) Get phy_status = %d (%d)(%d)\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status,poll_los_cnt,poll_ready_cnt);
#endif

	spin_unlock_irqrestore(&gpPhyPriv->event_handle_lock, flags);

	return PHY_SUCCESS;

}

int en7580_gepon_phy_dbg(char* buf)
{
	char dbg_id = *buf;
	char dbg_option1 = *(buf+1);
	char dbg_option2 = *(buf+2);

	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;

	phy_reg_total_num=121;
	phy_reg_all=gepon_phy_reg_all;

	
	printk("[%s:%d] id = %d\n",__FUNCTION__,__LINE__,dbg_id);
	switch(dbg_id)
	{
		case 1:
			printk("\r\n%-44s	 address=value\r\n","_____ GEPON REG DBG _____");
			for(i=36;i<=42;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}			

			break;

		case 2:
			
			printk("\r\n%-44s	 address=value\r\n","_____ GEPON REG DBG _____");
			for(i=43;i<=51;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}

			break;

		case 3:
			
			printk("\r\n%-44s	 address=value\r\n","_____ GEPON REG DBG _____");
			i=115;
			(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
			printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			i=11;
			(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
			printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			i=4;
			(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
			printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			i=3;
			(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
			printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);

			break;			

		default:
			printk("[%s:%d] UNKNOWN ID\n",__FUNCTION__,__LINE__);

			return PHY_FAILURE;
	}

	return PHY_SUCCESS;

}


int en7580_gepon_phy_pma_reset(char* buf)
{
#if A60972_SERDES	// for FPGA
	UINT32 read_data = 0;
	UINT32 write_data = 0;

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d]\r\n",__FUNCTION__,__LINE__);

	//Hold GEPON PHY-D Reset
	read_data = IO_GPHYREG(GEPON_CSR_PHYSET3);
	write_data = read_data |GEPON_CSR_PHYSET3_PLL_RST;  // |GEPON_CSR_PHYSET3_PHY_RST|GEPON_CSR_PHYSET3_SOFTWARE_RST;  //GEPON_CSR_PHYSET3_LOS_RST
	IO_SPHYREG(GEPON_CSR_PHYSET3, write_data) ;
	printk("EN7580 << GEPON >> PLL & Software reset hold 0x%x = 0x%x\r\n",GEPON_CSR_PHYSET3,write_data);

	//reset A60972
	printk("reset A60972\r\n");
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_ON);
	mdelay(1);
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_OFF);

	//init A60972
	a60972_init();

	//FPGA interface Reset
	read_data = EN7580_IF_RG_A60972_CTRL_IF_SW_RESET_ONLY;
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, read_data);
	mdelay(1);
	read_data = EN7580_IF_RG_A60972_CTRL_NORMAL_OPERATION;
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, read_data);
	printk("EN7580_IF_RG_A60972_CTRL_IF_SW_RESET_ONLY\r\n");

	//Release GEPON PHY-D Reset
	write_data = IO_GPHYREG(GEPON_CSR_PHYSET3);
	write_data = write_data&~(GEPON_CSR_PHYSET3_PLL_RST);
	//write_data = write_data&~(GEPON_CSR_PHYSET3_LOS_RST);
	//write_data = write_data&~(GEPON_CSR_PHYSET3_PHY_RST);
	//write_data = write_data&~(GEPON_CSR_PHYSET3_SOFTWARE_RST);
	printk("EN7580 << GEPON >> PLL & Software reset release 0x%x = 0x%x\r\n",GEPON_CSR_PHYSET3,write_data);
	IO_SPHYREG(GEPON_CSR_PHYSET3, write_data);	

#endif
	
#if ASIC_SERDES	// for ASIC
	UINT32 read_data = 0;
	UINT32 write_data = 0;

	read_data = IO_GPHYREG(GEPON_CSR_PHYSET3);
	write_data = read_data |GEPON_CSR_PHYSET3_PLL_RST;  // |GEPON_CSR_PHYSET3_PHY_RST|GEPON_CSR_PHYSET3_SOFTWARE_RST;  //GEPON_CSR_PHYSET3_LOS_RST
	IO_SPHYREG(GEPON_CSR_PHYSET3, write_data) ;
	phy_fw_ready(PHY_DISABLE);

	if(gpPhyPriv->first_plugin_flag == TRUE)
	{
		fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
		gpPhyPriv->first_plugin_flag = FALSE;
		gpPhyPriv->pma_init_done= TRUE;
	}
	else
	{
		fiber_plug_reset(PLUG_IN,gpPhyPriv->wan_sel);
	}
	
	write_data = IO_GPHYREG(GEPON_CSR_PHYSET3);
	write_data = write_data&~(GEPON_CSR_PHYSET3_PLL_RST);	
	IO_SPHYREG(GEPON_CSR_PHYSET3, write_data) ;
	
	phy_fw_ready(PHY_ENABLE);
#endif

	return PHY_SUCCESS;
}




#define _I_AM_XGPON_SPLIT_LINE_

/////////////////////////////////////////////////////////////////////////////////////
// en7580_xgpon_func
/////////////////////////////////////////////////////////////////////////////////////

int en7580_xgpon_pon_phy_reset(char* buf)
{
	uint val = 0;
	uint wan_cfg=0;

	printk("\r\nEN7580 << XGPON >> pon_phy_reset\r\n");

	//gating PHY-D ck out
	IO_SPHYREG(XPON_PMA_PON_CK_SET, 0x0);    //0x1fa8b450=0
	udelay(1);

	//switch wan mode
	wan_cfg=IO_GPHYREG(SCU_WAN_CONF_REG); 
	IO_SPHYREG(SCU_WAN_CONF_REG,((wan_cfg&0xffffff00)|0x11));
			
	val = IO_GPHYREG(TOP_RST_CTRL_SW2);
	val |=0x01;
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
	udelay(1);
	//release xpon phy top reset 
	val &=~(0x01);
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);

	IO_SPHYREG(SCU_WAN_CONF_REG,wan_cfg);
	//wan_cfg=IO_GPHYREG(SCU_WAN_CONF_REG);
	//PON_PHY_PRINT(PHY_MSG_ERR,"\r\nSET_WAN_CONF=0x%x\r\n",wan_cfg);
	
	IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_ON);
	udelay(1);	
	IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_OFF);

	return PHY_SUCCESS;
}


int en7580_xgpon_phy_dump(char* buf)
{
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;

	phy_reg_total_num=90;
	phy_reg_all=xgpon_phy_reg_all;

	printk("\r\n%-44s	 address=value\r\n","_____ XGPON REG DUMP _____");
	for(i=0;i<=phy_reg_total_num;i++)
	{
		(phy_reg_all+i)->def=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
	}
	
#if	PHY_REG_READ_WRITE_TEST
	//Register Read/Write test
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ZEROS ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_0");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ONES ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_1");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ END ___\r\n");
#endif
	

	return PHY_SUCCESS;
}


int en7580_xgpon_phy_dev_init(char* buf)
{
	uint read_data = 0;
	uint write_data = 0;

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d]\r\n",__FUNCTION__,__LINE__);


#ifdef TCSUPPORT_CPU_ARMV8 //julia_asic
	/*after sw reset, register can be modify by PBUS*/
	read_data = GET_SCU_RST_ACC_CHK_BMA(); //TOP_CSR_PBUS_CTRL
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	SET_SCU_RST_ACC_CHK_BMA(read_data); //TOP_CSR_PBUS_CTRL
	/* transceiver power initial setting*/
	read_data = GET_GPIO_SSR(); //TOP_CSR_GPIO_SHARE
	read_data = read_data & ~(TOP_LED1_MODE);
	SET_GPIO_SSR(read_data); //TOP_CSR_GPIO_SHARE
#else	
		/*after sw reset, register can be modify by PBUS*/
	read_data = IO_GREG(TOP_CSR_PBUS_CTRL);
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	IO_SREG(TOP_CSR_PBUS_CTRL, read_data);
	/* transceiver power initial setting*/
	read_data = IO_GREG(TOP_CSR_GPIO_SHARE);
	read_data = read_data & ~(TOP_LED1_MODE);
	IO_SREG(TOP_CSR_GPIO_SHARE, read_data);
#endif

	//enable PCS RX control 	david 20170124
	read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
	printk("Read XGPON_RX_SYNC_CTRL=0x%x \n",read_data);
	IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL,read_data|XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE) ;
	read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
	printk("Set XGPON_RX_SYNC_CTRL=0x%x \n",read_data);


	//phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_ALL);

	/* Enable BIP error counter */
	
	/* change the guard time pattern */
	gpPhyPriv->phyGuardBitDelm = 0xAAAB5983;
	//IO_SPHYREG(PHY_CSR_GPON_DELIMITER_GUARD, 0xAAAB5983);


	//for xgspon/ngpon2, rx_fec refer to oc body
	if((SCU_WAN_CONF_REG_WAN_SEL_NGPON2_2G_2G==gpPhyPriv->wan_sel)\
		||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G==gpPhyPriv->wan_sel)\
		||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G==gpPhyPriv->wan_sel))
	//	||(SCU_WAN_CONF_REG_WAN_SEL_XGSPON==gpPhyPriv->wan_sel))  //default set xgspon rx fec force on (for supporting OLT:HW5800)  by david 20220916
	{
		read_data = IO_GPHYREG(XGPON_PHY_DBG_CTRL);
		//printk("XGPON_PHY_DBG_CTRL = %x\n",read_data);
		write_data = read_data | XGPON_PHY_DBG_CTRL_DBG_RX_FEC_OC_REF_EN;
		IO_SPHYREG(XGPON_PHY_DBG_CTRL,write_data);
		read_data = IO_GPHYREG(XGPON_PHY_DBG_CTRL);
		//printk("XGPON_PHY_DBG_CTRL = %x\n",read_data);
	}

	return PHY_SUCCESS ;

}

int en7580_xgpon_phy_int_config(char* buf)
{
	UINT32 *mode = (UINT32 *)buf;
	UINT32 *int_type = (UINT32 *)buf+1;

	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 phyIntStatus = 0;

	if(PHY_INT_EN_MODE_DEFAULT==(*mode))
	{
		//write_data=(XGPON_PHY_RX_RDY_INT_EN|XGPON_PHY_RX_LOF_INT_EN|XGPON_PHY_RX_SYNC_OK_INT_EN|XGPON_PHY_RX_LOS_INT_EN);
		write_data= XGPON_PHY_TX_FAULT_INT_EN\
					|XGPON_PHY_TX_BURST_SPACE_ERR_INT_EN\
					|XGPON_PHY_TX_MPI_ERR_INT_EN\
					|XGPON_PHY_TX_PSBU_INFO_ERR_INT_EN\
					|XGPON_PHY_TX_INFO_FIFO_INT_EN\
					|XGPON_PHY_RX_RDY_INT_EN\
					|XGPON_PHY_RX_INFO_FIFO_INT_EN\
					|XGPON_PHY_RX_NGPON2_OC_ERR_INT_EN\
					|XGPON_PHY_RX_BER_HIGH_INT_EN\
					|XGPON_PHY_RX_LOF_INT_EN\
					|XGPON_PHY_RX_SYNC_OK_INT_EN\
					|XGPON_PHY_RX_LOS_INT_EN;

					//XGPON_PHY_XG_PHYA_RDY_INT_EN\		
					//|XGPON_PHY_TX_SFP_CONTINUE_INT_EN\
					//|XGPON_PHY_TX_SFP_ABNORMAL_INT_EN\
					//|XGPON_PHY_RX_CW_CNT_ERR_INT_EN\
					//|XGPON_PHY_RX_FEC_ERR_INT_EN\

		PON_PHY_PRINT(PHY_MSG_TRACE,"XGPON Phy use default int_type:0x%.8x\n", write_data);
	}
	else
	{
		write_data=(*int_type);
		PON_PHY_PRINT(PHY_MSG_TRACE,"XGPON Phy_Int_Config type     :0x%.8x\n", write_data);
	}
	
	//NOT Clear PHY ISR Status, Let PHY_ISR() function deal with the ISR Status state machine
	//Otherwise, we may miss the ISR status after PHY init and before register ISR to system
	
	read_data = IO_GPHYREG(XGPON_PHY_XG_PON_INT_EN);	
	PON_PHY_PRINT(PHY_MSG_TRACE,"XGPON Phy_Int_Config read     :0x%.8x\n", read_data);

	IO_SPHYREG(XGPON_PHY_XG_PON_INT_EN, write_data);
	read_data = IO_GPHYREG(XGPON_PHY_XG_PON_INT_EN);
	PON_PHY_PRINT(PHY_MSG_TRACE,"XGPON Phy_Int_Config write    :0x%.8x\n", read_data);


	return PHY_SUCCESS;
}


int en7580_xgpon_phy_get_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	int phyReadyStatus,phyLosStatus;

	UINT32 i=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;
	phy_reg_all=xgpon_phy_reg_all;


	//////by ang_20180129
	UINT32 xgpon_preamble_Up=0 ;
	UINT32 xgpon_preamble_Low=0;

	UINT32 xgpon_delimiter_Up=0;
	UINT32 xgpon_delimiter_Low=0;

	REG_PHY_FEC_INDICATION phyFec_indication;
	REG_PHY_PSBU_INFO psbuInfo;
	//////
	

	api_data->ret = PHY_SUCCESS;

	switch(api_data->cmd_id)
	{
		case PON_GET_PHY_LOS_STATUS:
			read_data = IO_GPHYREG(XGPON_PHY_SFP_STA);
			if(XGPON_PHY_SFP_RX_LOS_ST==(read_data & XGPON_PHY_SFP_RX_LOS_ST)) //it is transceiver sfp rx LOSS no PHY digital
			{
				api_data->ret = PHY_LOS_HAPPEN;
			}
			else
			{
				api_data->ret = PHY_NO_LOS_HAPPEN;
			}

			break;
			
		case PON_GET_PHY_READY_STATUS:
			read_data = IO_GPHYREG(XGPON_PHY_DBG_RX_SYNC_ST);//read bit[18:20]
			if(XGPON_PHY_DBG_RX_SYNC_ST_SYNC==(read_data& XGPON_PHY_DBG_RX_SYNC_ST_SYNC))
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Ready. \n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy not Ready. \n");
				api_data->ret = PHY_FALSE;
			}

			break; 
			
		case PON_GET_PHY_IS_SYNC:
			//api_data->ret = is_phy_sync(); // for this API, in_data is NULL
			
				read_data = IO_GPHYREG(XGPON_PHY_SFP_STA);
				if(XGPON_PHY_SFP_RX_LOS_ST==(read_data & XGPON_PHY_SFP_RX_LOS_ST)) //it is transceiver sfp rx LOSS no PHY digital
				{
					phyLosStatus = PHY_LOS_HAPPEN;
				}
				else
				{
					phyLosStatus = PHY_NO_LOS_HAPPEN;
				}
			
				read_data = IO_GPHYREG(XGPON_PHY_DBG_RX_SYNC_ST);//read bit[18:20]
				if(XGPON_PHY_DBG_RX_SYNC_ST_SYNC==(read_data& XGPON_PHY_DBG_RX_SYNC_ST_SYNC))
				{
					//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Ready. \n");
					phyReadyStatus = PHY_TRUE;
				}
				else
				{
					//PON_PHY_PRINT(PHY_MSG_DBG, "Phy not Ready. \n");
					phyReadyStatus = PHY_FALSE;
				}

				api_data->ret = ((phyReadyStatus == PHY_TRUE) && (phyLosStatus == PHY_NO_LOS_HAPPEN) );
			break;

		case PON_GET_PHY_MODE:
			api_data->ret = gpPhyPriv->phyCfg.flags.mode;

			break;
			
		case PON_GET_PHY_TX_LONG_FLAG:
			api_data->ret = gpPhyPriv->phyCfg.flags.txLongFlag;

			break;

		case PON_GET_PHY_TRANS_STATUS:
			phy_trans_param_status_real(&api_data->trans_status->params);
			api_data->trans_status->alarms = phy_tx_alarm() | phy_rx_power_alarm();
			break;

		case PON_GET_PHY_GET_TX_POWER_EN_FLAG:
			api_data->ret = gpPhyPriv->phyCfg.flags.txPowerEnFlag;

			break;

		case PON_GET_PHY_BIP_COUNTER:
			
			break;

		case PON_GET_PHY_RX_FEC_COUNTER:
			
			api_data->rx_fec_cnt->correct_bytes=IO_GPHYREG(XGPON_PHY_FEC_CORRECTED_BYTE_CNT);
			api_data->rx_fec_cnt->correct_codewords=IO_GPHYREG(XGPON_PHY_FEC_CORRECTED_CW_CNT);
			api_data->rx_fec_cnt->uncorrect_codewords=IO_GPHYREG(XGPON_PHY_FEC_UNCORRECTED_CW_CNT);
			api_data->rx_fec_cnt->total_rx_codewords=IO_GPHYREG(XGPON_PHY_FEC_TOTAL_CW_CNT);
			//XGPON_PHY_FEC_ERR_SECONDS
			api_data->rx_fec_cnt->fec_seconds=IO_GPHYREG(XGPON_PHY_FEC_ERR_SECONDS); // by ang_20170808

			break;

		case PON_GET_PHY_RX_FRAME_COUNTER:
			api_data->rx_frame_cnt->frame_count_high = 0x0;
			api_data->rx_frame_cnt->frame_count_low = IO_GPHYREG(XGPON_PHY_DBG_RX_FRAME2PHYD_CNT);
			api_data->rx_frame_cnt->lof_counter = IO_GPHYREG(XGPON_PHY_DBG_LOF_CNT);
			break;

		case PON_GET_PHY_RX_FEC_GETTING:
			read_data = IO_GPHYREG(XGPON_PHY_DBG_CTRL);
			if(read_data&XGPON_PHY_DBG_CTRL_DBG_RX_FEC_DISABLE)
			{
				api_data->ret = PHY_FALSE;
			}
			else
			{
				api_data->ret = PHY_TRUE;
			}

			break;

		case PON_GET_PHY_RX_FEC_STATUS:
			read_data = IO_GPHYREG(XGPON_PHY_DBG_CTRL);
			if(read_data&XGPON_PHY_DBG_CTRL_DBG_RX_FEC_DISABLE)
			{
				api_data->ret = PHY_FALSE;
			}
			else if((SCU_WAN_CONF_REG_WAN_SEL_NGPON2_2G_2G==gpPhyPriv->wan_sel)
					||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G==gpPhyPriv->wan_sel)
					||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G==gpPhyPriv->wan_sel)
					||((SCU_WAN_CONF_REG_WAN_SEL_XGSPON==gpPhyPriv->wan_sel)&&(read_data&XGPON_PHY_DBG_CTRL_DBG_RX_FEC_OC_REF_EN)))
			{
				read_data = IO_GPHYREG(XGPON_PHY_PON_ID_UPPER);
				if(read_data&XGPON_PHY_PON_ID_UPPER_DS_FEC)
				{
					api_data->ret = PHY_TRUE;
				}
				else
				{
					api_data->ret = PHY_FALSE;
				}
			}
			else
			{
				api_data->ret = PHY_TRUE;
			}

			break;

		case PON_GET_PHY_TX_FEC_STATUS:		//by ang_20180115
			read_data = IO_GPHYREG(XGPON_PHY_DBG_TX_FEC_STA);
			if((read_data & XGPON_PHY_TX_FEC) == XGPON_PHY_TX_FEC)
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC on. \n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC off. \n");
				api_data->ret = PHY_FALSE;
			}

			break;

		case PON_GET_PHY_TX_BURST_GETTING:
			
#if ASIC_SERDES
           	read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1);
			if( (read_data & XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK)== XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK )
			{
				api_data->ret = PHY_TX_BURST_MODE;
			}
			else
			{
	           	read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2);

				if( (read_data & XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)== XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)
				{
				api_data->ret = PHY_TX_CONT_MODE;
			}
				else
				{
					api_data->ret = PHY_TX_BURST_MODE;
				}
			}
#endif
			break;

		case PON_GET_PHY_TRANS_TX:
			#if ASIC_SERDES //EN7580_ASIC
			read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
			if((read_data & ~(XPON_PMA_XPON_SETTING_1_TX_SD_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_DISABLE;

			read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
			if((read_data & ~(XPON_PMA_XPON_SETTING_0_BURST_EN_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_DISABLE;
			if((read_data & ~(XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_DISABLE;
			#endif
			break;

		case PON_GET_PHY_TRANS_RX_GETTING:

#if ASIC_SERDES
            read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
			if((read_data & XPON_PMA_XPON_SETTING_0_RX_SD_INV) == 0)
				api_data->ret = PHY_DISABLE;
			else
				api_data->ret = PHY_ENABLE;
#endif
			break;

		case PON_GET_PHY_ROUND_TRIP_DELAY:

			break;

        case PON_GET_PHY_LOF_STATUS:
			//read_data = IO_GPHYREG(LOF);	  
	  		api_data->ret=0;

			break;
		case PON_GET_PHY_XGPON_PROFILE:

			phyFec_indication.Raw = IO_GPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL);	

			if(api_data->xgpon_profile->profile_index==0)	// profile index is 0
			{
				//get preamble
				xgpon_preamble_Up = IO_GPHYREG(XGPON_PHY_PREAMBLE1_UPPER);
				xgpon_preamble_Low = IO_GPHYREG(XGPON_PHY_PREAMBLE1_LOWER);	
				//get delimiter
				xgpon_delimiter_Up = IO_GPHYREG(XGPON_PHY_DELIMITER1_UPPER);
				xgpon_delimiter_Low = IO_GPHYREG(XGPON_PHY_DELIMITER1_LOWER);
	
				//get fec status
				api_data->xgpon_profile->fec_indication = phyFec_indication.Bits.xg_tx_fec_en1;
					
				//get preamble repeat count & preableb length & delimiter length
				psbuInfo.Raw = IO_GPHYREG(XGPON_PHY_PSBU_INFO1);
			}
			else if(api_data->xgpon_profile->profile_index==1)	// profile index is 1
			{	
				xgpon_preamble_Up = IO_GPHYREG(XGPON_PHY_PREAMBLE2_UPPER);
				xgpon_preamble_Low = IO_GPHYREG(XGPON_PHY_PREAMBLE2_LOWER);	

				xgpon_delimiter_Up = IO_GPHYREG(XGPON_PHY_DELIMITER2_UPPER);
				xgpon_delimiter_Low = IO_GPHYREG(XGPON_PHY_DELIMITER2_LOWER);
	
				api_data->xgpon_profile->fec_indication = phyFec_indication.Bits.xg_tx_fec_en2;
					
				psbuInfo.Raw = IO_GPHYREG(XGPON_PHY_PSBU_INFO2);
			}
			else if(api_data->xgpon_profile->profile_index==2)	// profile index is 2
			{	
				xgpon_preamble_Up = IO_GPHYREG(XGPON_PHY_PREAMBLE3_UPPER);
				xgpon_preamble_Low = IO_GPHYREG(XGPON_PHY_PREAMBLE3_LOWER);	

				xgpon_delimiter_Up = IO_GPHYREG(XGPON_PHY_DELIMITER3_UPPER);
				xgpon_delimiter_Low = IO_GPHYREG(XGPON_PHY_DELIMITER3_LOWER);
	
				api_data->xgpon_profile->fec_indication = phyFec_indication.Bits.xg_tx_fec_en3;
					
				psbuInfo.Raw = IO_GPHYREG(XGPON_PHY_PSBU_INFO3);
			}
			else if(api_data->xgpon_profile->profile_index==3)	// profile index is 3
			{	
				xgpon_preamble_Up = IO_GPHYREG(XGPON_PHY_PREAMBLE4_UPPER);
				xgpon_preamble_Low = IO_GPHYREG(XGPON_PHY_PREAMBLE4_LOWER);	

				xgpon_delimiter_Up = IO_GPHYREG(XGPON_PHY_DELIMITER4_UPPER);
				xgpon_delimiter_Low = IO_GPHYREG(XGPON_PHY_DELIMITER4_LOWER);
	
				api_data->xgpon_profile->fec_indication = phyFec_indication.Bits.xg_tx_fec_en4;
					
				psbuInfo.Raw = IO_GPHYREG(XGPON_PHY_PSBU_INFO4);
			}
			else
			{	
				xgpon_preamble_Up = 0;
				xgpon_preamble_Low = 0;	
				xgpon_delimiter_Up = 0;
				xgpon_delimiter_Low = 0;	
				api_data->xgpon_profile->fec_indication = phyFec_indication.Bits.xg_tx_fec_en4;
				psbuInfo.Raw = 0;
				PON_PHY_PRINT(PHY_MSG_ERR, "PON_PHY ERROR! profile_index is not between 0 and 3 ! \n");
			}

			api_data->xgpon_profile->preamble[0] = (xgpon_preamble_Up >> 24)  & XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[1] = (xgpon_preamble_Up >> 16)  & XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[2] = (xgpon_preamble_Up >>  8)  & XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[3] = (xgpon_preamble_Up >>  0)  & XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[4] = (xgpon_preamble_Low >> 24) & XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[5] = (xgpon_preamble_Low >> 16) & XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[6] = (xgpon_preamble_Low >>  8) & XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[7] = (xgpon_preamble_Low >>  0) & XGPON_PHY_PREAMBLE_MASK;

			api_data->xgpon_profile->delimiter[0] = (xgpon_delimiter_Up >> 24)  & XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[1] = (xgpon_delimiter_Up >> 16)  & XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[2] = (xgpon_delimiter_Up >>  8)  & XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[3] = (xgpon_delimiter_Up >>  0)  & XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[4] = (xgpon_delimiter_Low >> 24) & XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[5] = (xgpon_delimiter_Low >> 16) & XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[6] = (xgpon_delimiter_Low >>  8) & XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[7] = (xgpon_delimiter_Low >>  0) & XGPON_PHY_DELIMITER_MASK;

			api_data->xgpon_profile->preamble_repeat_count = psbuInfo.Bits.prmb_rpt_num;	
			api_data->xgpon_profile->preamble_length = psbuInfo.Bits.ptmb_len;
			api_data->xgpon_profile->delimiter_length = psbuInfo.Bits.dlmt_len;	

			PON_PHY_PRINT(PHY_MSG_TRACE,"profile index  		= %d\r\n", api_data->xgpon_profile->profile_index);
			PON_PHY_PRINT(PHY_MSG_TRACE,"xgpon_preamble_Up 		= 0x%x\r\n", xgpon_preamble_Up);
			PON_PHY_PRINT(PHY_MSG_TRACE,"xgpon_preamble_Low 	= 0x%x\r\n", xgpon_preamble_Low);
			PON_PHY_PRINT(PHY_MSG_TRACE,"xgpon_delimiter_Up 	= 0x%x\r\n", xgpon_delimiter_Up);
			PON_PHY_PRINT(PHY_MSG_TRACE,"xgpon_delimiter_Low 	= 0x%x\r\n", xgpon_delimiter_Low);
			PON_PHY_PRINT(PHY_MSG_TRACE,"preamble repeat number = %d\r\n", psbuInfo.Bits.prmb_rpt_num);
			PON_PHY_PRINT(PHY_MSG_TRACE,"preamble length 		= %d\r\n", psbuInfo.Bits.ptmb_len);
			PON_PHY_PRINT(PHY_MSG_TRACE,"delimiter length  		= %d\r\n", psbuInfo.Bits.dlmt_len);

			break;
			
		case PON_GET_PHY_INIT_STATUS: //ang_20190821
			api_data->ret = gpPhyPriv->phy_init_done;
			break;

		//get ngpon2 tx/rx chan, ang_20190605
		case PON_GET_PHY_NGPON2_CHAN:
            api_data->ret = xfp_trans_get_ngpon2_chan(api_data->ngpon2_chan_sel);
            break;

		default:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
			api_data->ret = PHY_NO_API;
			break;
	}

	return PHY_SUCCESS;	//ang_20180208
}

int en7580_xgpon_phy_set_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	
	UINT32 i=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;
	phy_reg_all=xgpon_phy_reg_all;

	//////by ang_20180129
	UINT32 xgpon_preamble_Up=0 ;
	UINT32 xgpon_preamble_Low=0;

	UINT32 xgpon_delimiter_Up=0;
	UINT32 xgpon_delimiter_Low=0;

	REG_PHY_FEC_INDICATION phyFec_indication;
	REG_PHY_PSBU_INFO psbuInfo;
	//////
	
    api_data->ret = PHY_SUCCESS;
    
    switch(api_data->cmd_id)
    {
        case PON_SET_PHY_MODE_CONFIG:
            api_data->ret = phy_mode_config(api_data->phy_mode_cfg->mode, api_data->phy_mode_cfg->txEnable);
            break;

		case PHY_SET_ERR_CNT_EN:
		case PHY_SET_BIP_CNT_EN:
		case PHY_SET_FM_CNT_EN:
			//PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, cmd_id=0x%x, data=%d\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id,(*api_data->data));
			break;

		case PHY_SET_PCS_FPGAIF_RESET:
			#if A60972_SERDES
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);			
			mdelay(200);			
			
			IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x0);
			
			//reset PCS logic, hold PCS reset
			IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N,XGPON_PHY_XG_PHY_RST_N_PHYD) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_PHY_RST_N);
			printk("Hold PCS reset, Hold XGPON_PHY_XG_PHY_RST_N=0x%x \n",read_data);			
			
			//FPGA interface Reset
			write_data = EN7580_IF_RG_A60972_CTRL_IF_SW_RESET_ONLY;
			IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, write_data);
			mdelay(1);
			write_data = EN7580_IF_RG_A60972_CTRL_NORMAL_OPERATION;
			IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, write_data);
			printk("EN7580_IF_RG_A60972_CTRL_IF_RX_SW_RESET_ONLY\r\n");			
			
			//reset PCS logic, release PCS reset
			IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N,XGPON_PHY_XG_PHY_RST_N_DISABLE) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_PHY_RST_N);
			printk("Release PCS reset, Release XGPON_PHY_XG_PHY_RST_N=0x%x \n",read_data);
			
			IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);			
			mdelay(200);			
			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
			handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
			#endif
			break;
		case PON_SET_PHY_LOGIC_RESET:
			#if ASIC_SERDES
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;

			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);// ang_20180921
		
			//disable rx
			read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data & (~XGPON_PHY_RX_ENABLE); //disable rx
			IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
		
			gpPhyPriv->first_plugin_flag = TRUE; 
			xpon_init(gpPhyPriv->wan_sel); 
			phy_pma_reset();
			PON_PHY_PRINT(PHY_MSG_ERR,"PHY_LOGIC_RESET.\n");
			#endif
			break;

		case PON_SET_PHY_SCU_RST:
			#if ASIC_SERDES
			phy_trans_power_switch(PHY_TX_DIS_ON_HW_ONLY); //tx_dis_reset_protection
			gpPhyPriv->debugLevel_backup=gpPhyPriv->debugLevel;
			gpPhyPriv->debugLevel=PHY_MSG_OFF;
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			//PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
			xPON_Phy_Reset();
			gpPhyPriv->phy_init_done=FALSE;
			gpPhyPriv->is_phy_start = FALSE;
			gpPhyPriv->phy_init_done = FALSE; 
			gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
			gpPhyPriv->trans_tx_enable = PHY_DISABLE;
			gpPhyPriv->phy_unexpected_isr_flag=FALSE;
			gpPhyPriv->first_plugin_flag = TRUE; //ang_20180926
			gpPhyPriv->pma_init_done = FALSE;	//ang_20190108
			gpPhyPriv->trans_msg_print_cnt = 95;
			pon_phy_mod_init();
			if(SCU_WAN_CONF_REG_WAN_SEL_XGPON == gpPhyPriv->wan_sel)
			{
	            if(gpPhyPriv->rx_fec_setting == PHY_ENABLE)
				{
					gpPhyPriv->rx_fec_setting=PHY_ENABLE;
					read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
					IO_SPHYREG(XGPON_PHY_DBG_CTRL, read_data&~XGPON_PHY_DBG_CTRL_DBG_RX_FEC_DISABLE);
					read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
					PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Enable (0x%x)=0x%.8x\n", XGPON_PHY_DBG_CTRL,read_data);

				}
				else if(gpPhyPriv->rx_fec_setting == PHY_DISABLE)
				{
					gpPhyPriv->rx_fec_setting=PHY_DISABLE;
					read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
					IO_SPHYREG(XGPON_PHY_DBG_CTRL, read_data|XGPON_PHY_DBG_CTRL_DBG_RX_FEC_DISABLE);
					read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
					PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Disable (0x%x)=0x%.8x\n", XGPON_PHY_DBG_CTRL,read_data);
				}
				else
				{
					gpPhyPriv->rx_fec_setting=PHY_DEFAULT;
				}
			}
			xpon_phy_start();
			gpPhyPriv->debugLevel=gpPhyPriv->debugLevel_backup;
			phy_trans_power_switch(PHY_TX_DIS_RESTORE_BY_SW);  //tx_dis_reset_protection
			PON_PHY_PRINT(PHY_MSG_ERR,"PHY_SCU_RESET\n");
			#endif
			break;

			
		case PHY_SET_PCS_RESET:
			#if A60972_SERDES
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);			
			mdelay(200);			
			
			IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x0);
			
			//reset PCS logic, hold PCS reset
			IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N,XGPON_PHY_XG_PHY_RST_N_PHYD) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_PHY_RST_N);
			printk("Hold PCS reset, Hold XGPON_PHY_XG_PHY_RST_N=0x%x \n",read_data);
			mdelay(1);
						
			//reset PCS logic, release PCS reset
			IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N,XGPON_PHY_XG_PHY_RST_N_DISABLE) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_PHY_RST_N);
			printk("Release PCS reset, Release XGPON_PHY_XG_PHY_RST_N=0x%x \n",read_data);
			
			IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);			
			mdelay(200);			
			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
			handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
			#endif

			#if EN7580_SERDES
			xpon_pma_mode_init();
			phy_pma_reset();	
			printk("PHY PCS RESET!\n");
			#endif
		
			break;
			
        case PON_SET_PHY_RESET_COUNTER:
            read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
			IO_SPHYREG(XGPON_PHY_DBG_CTRL,read_data|XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
			read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
			IO_SPHYREG(XGPON_PHY_DBG_CTRL,read_data&~XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
            break;

        case PON_SET_PHY_START:
            api_data->ret = xpon_phy_start();
            break;

        case PON_SET_PHY_STOP:
            api_data->ret = xpon_phy_stop();
            break;

        case PON_SET_PHY_FW_READY:
            break;

        case PON_SET_PHY_COUNTER_CLEAR:
            read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
			IO_SPHYREG(XGPON_PHY_DBG_CTRL,read_data|XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
			read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
			IO_SPHYREG(XGPON_PHY_DBG_CTRL,read_data&~XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
            break;

        case PON_SET_EPONMODE_PHY_RESET:
            break;

        case PON_SET_PHY_BIT_DELAY:
			read_data=IO_GPHYREG(XGPON_PHY_DBG_PHYD_DLY_STA);
			//IO_SPHYREG(XGPON_PHY_DBG_PHYD_DLY_STA,(*api_data->data));
            break;

        case PON_SET_PHY_TX_POWER_CONFIG:
            phy_tx_power_config(*api_data->data);
            break;
			
		case PON_SET_PHY_TX_D_PADDING_CONFIG:
			api_data->ret = PHY_SUCCESS;
            if(api_data->data == PHY_ENABLE)
			{
				IO_SPHYREG(XGPON_PHY_XG_TX_IDLE_CTRL, XGPON_PHY_XG_TX_IDLE_CTRL_PATTERN_SEL(0));
				write_data=IO_GPHYREG(XGPON_PHY_XG_TX_IDLE_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"TX_D_PADDING ON (0x%x)=0x%.8x\n", XGPON_PHY_XG_TX_IDLE_CTRL,write_data);

			}
			else if(api_data->data == PHY_DISABLE)
			{
				IO_SPHYREG(XGPON_PHY_XG_TX_IDLE_CTRL, XGPON_PHY_XG_TX_IDLE_CTRL_OFF);
				write_data=IO_GPHYREG(XGPON_PHY_XG_TX_IDLE_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"TX_D_PADDING OFF (0x%x)=0x%.8x\n", XGPON_PHY_XG_TX_IDLE_CTRL,write_data);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}		
            break;

        case PON_SET_PHY_ROGUE_PRBS_CONFIG:
			
			#if ASIC_SERDES
			if(gpPhyPriv->first_plugin_flag == TRUE)
			{
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
				gpPhyPriv->first_plugin_flag = FALSE;
				gpPhyPriv->pma_init_done= TRUE;
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
				mdelay(350);	//add by david 20190508  
				phy_trans_power_switch(PHY_ENABLE);	
			}	// by david 20190522	incase onu startup without OLT DS Signal, RX & TX could L2R lock.
			#endif
			
			PON_PHY_PRINT(PHY_MSG_ERR,"rogue_mode=%d, rogue_pattern=%d, tx_d_in_timeslot=%d\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);

			if((api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_ROGUE_MODE)&&(api_data->phy_rogue_cfg->rogue_pattern<=3)&&(api_data->phy_rogue_cfg->tx_d_in_timeslot<=1))
			{
				gpPhyPriv->phyCfg.flags.rogue = 1;
				
				IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CTRL,XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_EN\
													|XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_PATTERN_SEL(api_data->phy_rogue_cfg->rogue_pattern)\
													|XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_MODE_SEL(api_data->phy_rogue_cfg->tx_d_in_timeslot));
				read_data=IO_GPHYREG(XGPON_PHY_XG_CONTINUE_CTRL);
		
				PON_PHY_PRINT(PHY_MSG_ERR,"Set XGPON_PHY_XG_CONTINUE_CTRL(0x%x)=0x%x \n",XGPON_PHY_XG_CONTINUE_CTRL,read_data);		
				PON_PHY_PRINT(PHY_MSG_ERR,"Rogue %s\n",(XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_EN&read_data)?"ON":"OFF");
				PON_PHY_PRINT(PHY_MSG_ERR,"Rogue pattern %d (0=PRBS7,1=PRBS23,2=PRBS31,3=USER_CONFIG)\n",api_data->phy_rogue_cfg->rogue_pattern);
				PON_PHY_PRINT(PHY_MSG_ERR,"In_timeslot %s\n",(XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_MODE_SEL_ALL_PRBS&read_data)?"Rogue":"Normal");
				if(3==api_data->phy_rogue_cfg->rogue_pattern)
				{
					PON_PHY_PRINT(PHY_MSG_ERR,"Note : Rogue pattern is USER_CONFIG\n\tPlease set data by follow cmds:\n\tsys memwl bfaf0a7c <low_32bit_data>\n\tsys memwl bfaf0a80 <up_32bit_data>\n");
				}			
			} 
			else if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_NORMAL_MODE)
			{

				IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CTRL,XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_DISABLE\
													|XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_PATTERN_SEL(0)\
													|XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_MODE_SEL(0));
				read_data=IO_GPHYREG(XGPON_PHY_XG_CONTINUE_CTRL);
				PON_PHY_PRINT(PHY_MSG_ERR, "Set XGPON_PHY_XG_CONTINUE_CTRL=0x%x \n",read_data);	

				gpPhyPriv->phyCfg.flags.rogue = 0;

				PON_PHY_PRINT(PHY_MSG_ERR, "Phy_NO_Rogue_PRBS mode\n");
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1), pattern %d(must <=3), in_timeslot %d(must <=1)\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);
				api_data->ret = PHY_FAILURE;
			}

            break;

        case PON_SET_PHY_DEV_INIT:
            //api_data->ret = phy_dev_init();
            break;

        case PON_SET_PHY_TRANS_POWER_SWITCH:
            api_data->ret = phy_trans_power_switch(*api_data->data);
            break;

        case PON_SET_PHY_RESTORE_PREAMBLE_AND_GUARD_BIT:
            break;

        case PON_SET_PHY_TRANS_MODEL_SETTING:
            if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{
				// SFP RX valid level

#if ASIC_SERDES || A60972_SERDES

				read_data=IO_GPHYREG(XGPON_PHY_SFP_VLD_LEVEL);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Read XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",XGPON_PHY_SFP_VLD_LEVEL,read_data);
				IO_SPHYREG(XGPON_PHY_SFP_VLD_LEVEL,phy_trans_iot_list[gpPhyPriv->trans_index].xgpon_phy_sfp_vld_level) ;
				read_data=IO_GPHYREG(XGPON_PHY_SFP_VLD_LEVEL);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Set XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",XGPON_PHY_SFP_VLD_LEVEL,read_data);
				
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Read XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_0,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Set XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);

				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Read XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_1,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_1) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Set XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);				
#endif
            }

            break;

        case PON_SET_PHY_RX_FEC_SETTING:

			if(SCU_WAN_CONF_REG_WAN_SEL_XGPON == gpPhyPriv->wan_sel)
			{
            if((*api_data->data) == PHY_ENABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_ENABLE;
				read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
				IO_SPHYREG(XGPON_PHY_DBG_CTRL, read_data&~XGPON_PHY_DBG_CTRL_DBG_RX_FEC_DISABLE);
				read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Enable (0x%x)=0x%.8x\n", XGPON_PHY_DBG_CTRL,read_data);

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_DISABLE;
				read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
				IO_SPHYREG(XGPON_PHY_DBG_CTRL, read_data|XGPON_PHY_DBG_CTRL_DBG_RX_FEC_DISABLE);
				read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Disable (0x%x)=0x%.8x\n", XGPON_PHY_DBG_CTRL,read_data);
			}
			else
			{
				gpPhyPriv->rx_fec_setting=PHY_DEFAULT;
			}
			}
            break;

        case PON_SET_PHY_TX_BURST_CONFIG:
			PON_PHY_PRINT(PHY_MSG_ERR, "TX_BURST_CONFIG=%d\n",(*api_data->data));

            if(((*api_data->data) != PHY_TX_BURST_MODE) && ((*api_data->data) != PHY_TX_CONT_MODE))
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : burst_mode = %d\n",(*api_data->data));
				api_data->ret = PHY_FAILURE;
			}
			else
			{
				//set idle pattern. 0x00:PRBS7; 0x01:PRBS31; 0x11:user pattern
				IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CTRL,(*api_data->data)\
													|XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_PATTERN_SEL(0x0)\
													|XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_MODE_SEL(0));
				//IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER,0);
				//IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER,0);
				read_data=IO_GPHYREG(XGPON_PHY_XG_CONTINUE_CTRL);
				PON_PHY_PRINT(PHY_MSG_ERR,"XG_CONTINUE_CTRL (0x%x)=0x%.8x\n", XGPON_PHY_XG_CONTINUE_CTRL,read_data);
				gpPhyPriv->phyCfg.flags.txLongFlag = (((*api_data->data)==PHY_TX_CONT_MODE) ? PHY_TRUE : PHY_FALSE);
				api_data->ret = PHY_SUCCESS;
			}            
            break;

        case PON_SET_PHY_GPON_DELIMITER_GUARD:

            break;

        case PON_SET_PHY_TRANS_TX_SETTINGS:
            break;

        case PON_SET_PHY_TRANS_RX_SETTING:
            break;

        case PON_SET_PHY_GPON_PREAMBLE:
            break;

        case PON_SET_PHY_GPON_EXTEND_PREAMBLE:
             break;
			
        case PHY_SET_XGPON_PROFILE_DELIMITER:
			for(i=13;i<=20;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,(*api_data->data));
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s 0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}

			break;     

		case PHY_SET_XGPON_PROFILE_PREAMBLE:
			for(i=5;i<=12;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,(*api_data->data));
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s 0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}

			break;
			
		case PHY_SET_XGPON_PROFILE_LEN:
			for(i=22;i<=25;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,(*api_data->data));
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s 0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			break;		
			
		case PHY_SET_XGPON_LASER_ON_LEN:
			for(i=26;i<=26;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,(*api_data->data));
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s 0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}	
			break;

		case PHY_SET_XGPON_LASER_ON_PATTERN:
			for(i=27;i<=27;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,(*api_data->data));
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s 0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}	
			break;
			
		case PHY_SET_TX_FEC_EN:
            if((*api_data->data) == PHY_ENABLE)
			{
				read_data=IO_GPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL);
				IO_SPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL,XGPON_PHY_XG_TX_FEC_EN_CTRL_ENABLE);
				read_data=IO_GPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Tx FEC Enable (0x%x)=0x%.8x\n", XGPON_PHY_XG_TX_FEC_EN_CTRL,read_data);

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				read_data=IO_GPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL);
				IO_SPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL, XGPON_PHY_XG_TX_FEC_EN_CTRL_DISABLE);
				read_data=IO_GPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Tx FEC Disable (0x%x)=0x%.8x\n", XGPON_PHY_XG_TX_FEC_EN_CTRL,read_data);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			break;
			
#if (defined(TCSUPPORT_WAN_GPON) || defined(TCSUPPORT_WAN_EPON))&& defined(TCSUPPORT_XPON_HAL_API_EXT)
        case PON_SET_PHY_NOTIFY_EVENT:
            if(gpPhyPriv->phyXponStateNotifyHandler){
                gpPhyPriv->phyXponStateNotifyHandler(*api_data->data);
            }
            break;
#endif
		case PON_SET_PHY_TRAFFIC_STATUS:
			api_data->ret = phy_set_traffic_status(*api_data->data);
				break;

		case PON_SET_PHY_XGPON_PROFILE:
			//get preamble and deilmiter
			xgpon_preamble_Up=((api_data->xgpon_profile->preamble[0])<<24)|((api_data->xgpon_profile->preamble[1])<<16)\
					   			|(api_data->xgpon_profile->preamble[2]<<8)|(api_data->xgpon_profile->preamble[3]);
			
			xgpon_preamble_Low=(api_data->xgpon_profile->preamble[4]<<24)|(api_data->xgpon_profile->preamble[5]<<16)\
					   			|(api_data->xgpon_profile->preamble[6]<<8)|(api_data->xgpon_profile->preamble[7]);
			
			xgpon_delimiter_Up=((api_data->xgpon_profile->delimiter[0])<<24)|((api_data->xgpon_profile->delimiter[1])<<16)\
					   			|((api_data->xgpon_profile->delimiter[2])<<8)|(api_data->xgpon_profile->delimiter[3]);
			
			xgpon_delimiter_Low=((api_data->xgpon_profile->delimiter[4])<<24)|((api_data->xgpon_profile->delimiter[5])<<16)\
					   			|((api_data->xgpon_profile->delimiter[6])<<8)|(api_data->xgpon_profile->delimiter[7]);

			phyFec_indication.Raw =IO_GPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL);		//get current fec status
			
			//get preamble repeat count & preableb length & delimiter length
			psbuInfo.Bits.prmb_rpt_num = api_data->xgpon_profile->preamble_repeat_count;	
			psbuInfo.Bits.ptmb_len = api_data->xgpon_profile->preamble_length;
			psbuInfo.Bits.dlmt_len = api_data->xgpon_profile->delimiter_length;	

			if(api_data->xgpon_profile->profile_index==0)	// profile index is 0
			{	
				//set preamble
				IO_SPHYREG(XGPON_PHY_PREAMBLE1_UPPER, xgpon_preamble_Up);
				IO_SPHYREG(XGPON_PHY_PREAMBLE1_LOWER, xgpon_preamble_Low);	
				//set delimiter
				IO_SPHYREG(XGPON_PHY_DELIMITER1_UPPER, xgpon_delimiter_Up);
				IO_SPHYREG(XGPON_PHY_DELIMITER1_LOWER, xgpon_delimiter_Low);	
				//set fec enabler or disable
				phyFec_indication.Bits.xg_tx_fec_en1=api_data->xgpon_profile->fec_indication;
				IO_SPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL, phyFec_indication.Raw);		
				//set preamble repeat count & preableb length & delimiter length
				IO_SPHYREG(XGPON_PHY_PSBU_INFO1, psbuInfo.Raw);
			}
			else if(api_data->xgpon_profile->profile_index==1)	// profile index is 1
			{
				IO_SPHYREG(XGPON_PHY_PREAMBLE2_UPPER, xgpon_preamble_Up);
				IO_SPHYREG(XGPON_PHY_PREAMBLE2_LOWER, xgpon_preamble_Low);

				IO_SPHYREG(XGPON_PHY_DELIMITER2_UPPER, xgpon_delimiter_Up);
				IO_SPHYREG(XGPON_PHY_DELIMITER2_LOWER, xgpon_delimiter_Low);

				phyFec_indication.Bits.xg_tx_fec_en2=api_data->xgpon_profile->fec_indication;
				IO_SPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL, phyFec_indication.Raw);
				
				IO_SPHYREG(XGPON_PHY_PSBU_INFO2, psbuInfo.Raw);
			}
			else if(api_data->xgpon_profile->profile_index==2)	// profile index is 2
			{
				IO_SPHYREG(XGPON_PHY_PREAMBLE3_UPPER, xgpon_preamble_Up);
				IO_SPHYREG(XGPON_PHY_PREAMBLE3_LOWER, xgpon_preamble_Low); 
				
				IO_SPHYREG(XGPON_PHY_DELIMITER3_UPPER, xgpon_delimiter_Up);
				IO_SPHYREG(XGPON_PHY_DELIMITER3_LOWER, xgpon_delimiter_Low);

				phyFec_indication.Bits.xg_tx_fec_en3=api_data->xgpon_profile->fec_indication;
				IO_SPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL, phyFec_indication.Raw);
				
				IO_SPHYREG(XGPON_PHY_PSBU_INFO3, psbuInfo.Raw);
			}
			else if(api_data->xgpon_profile->profile_index==3)	// profile index is 3
			{
				IO_SPHYREG(XGPON_PHY_PREAMBLE4_UPPER, xgpon_preamble_Up);
				IO_SPHYREG(XGPON_PHY_PREAMBLE4_LOWER, xgpon_preamble_Low); 

				IO_SPHYREG(XGPON_PHY_DELIMITER4_UPPER, xgpon_delimiter_Up);
				IO_SPHYREG(XGPON_PHY_DELIMITER4_LOWER, xgpon_delimiter_Low);

				phyFec_indication.Bits.xg_tx_fec_en4=api_data->xgpon_profile->fec_indication;
				IO_SPHYREG(XGPON_PHY_XG_TX_FEC_EN_CTRL, phyFec_indication.Raw);
				
				IO_SPHYREG(XGPON_PHY_PSBU_INFO4, psbuInfo.Raw);
			}
			else
			{	
				PON_PHY_PRINT(PHY_MSG_ERR, "PROFILE SETTING ERROR! profile_index is not between 0 and 3 ! \n");
			}
		
				break;
			
		case PON_SET_PHY_XGPON_RX_ENABLE:
			
			read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data | XGPON_PHY_RX_ENABLE;	//enable rx
			IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			
			break;

		case PON_SET_PHY_XGPON_RX_DISABLE:
			
			read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data & (~XGPON_PHY_RX_ENABLE);	//disable rx
			IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			
			break;

		//set ngpon2 tx/rx chan, ang_20190605
		case PON_SET_PHY_NGPON2_TX_CHAN_CONFIG:
			api_data->ret = xfp_trans_set_ngpon2_tx_chan(api_data->ngpon2_chan_sel);
			break;
		case PON_SET_PHY_NGPON2_RX_CHAN_CONFIG:
			api_data->ret = xfp_trans_set_ngpon2_rx_chan(api_data->ngpon2_chan_sel);
			break;
			
#ifdef TCSUPPORT_MT7570
		case PON_SET_PHY_EN7571_INIT_DONE:

			gpPhyPriv->en7571_init_done = FALSE;
			PON_PHY_PRINT(PHY_MSG_ERR, "ERROR: Should not init 7571 in XGPON mode !\r\n");

			break;
				
		case PON_SET_PHY_RX_CDR:
#if ASIC_SERDES
			if((*api_data->data) == PHY_ENABLE)
			{
				xpon_phy_stop();
				gpPhyPriv->phy_init_done=FALSE;
				gpPhyPriv->first_plugin_flag = TRUE; 
				pon_phy_mod_init();		// xpon_phy_10g module reset (the most complete reset cmd)
				xpon_phy_start();
			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				xpon_phy_stop();
				xpon_init(gpPhyPriv->wan_sel);
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel); 							// reset pma and first_plug_in
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	// force RX L2R lock
				phy_int_config(PHY_INT_EN_MODE_MANUAL, PHY_INT_EN_DISABLE);		//disable PON_PHY isr mask
				PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);	// stop event_poll_timer	
				mdelay(350);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
#endif
			break;
#endif
		
#ifdef LDDLA_SUPPORT_SET_TX_MODE
		case PON_SET_PHY_LDDLA_AUTO_MODE:
			if((*api_data->data) == PHY_ENABLE)
			{
				gpPhyPriv->lddla_auto_mode= TRUE; 

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				gpPhyPriv->lddla_auto_mode= FALSE;
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			PON_PHY_PRINT(PHY_MSG_ERR, "MAC set lddla_auto_mode = %s\n",(gpPhyPriv->lddla_auto_mode==TRUE)?"TRUE":"FALSE");
			break;
#endif		

        default:
            PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
            api_data->ret = PHY_NO_API;
            break;
    }

	return PHY_SUCCESS;	//ang_20180208
}

int en7580_xgpon_phy_isr(char* buf)
{
	ulong flags;
	uint phyIntStatus, phyIntEn,read_data,write_data,read_data_1,read_data_2, phypmaIntEn, phypmaIntStatus ;
    PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_HW_IRQ };
	static uint isr_cnt=0,phy_rdy_cnt=0,phy_los_cnt=0,phy_rx_rdy_cnt=0,phy_lof_cnt=0,phy_other_cnt=0,phy_fake_sync_cnt=0;
	static uint trans_laser_detected = FALSE;		
	isr_cnt++;
	spin_lock_irqsave(&gpPhyPriv->event_handle_lock, flags);
	
	/*get interrupt status and clear*/
	phyIntStatus=IO_GPHYREG(XGPON_PHY_XG_PON_INT_STA);
	IO_SPHYREG(XGPON_PHY_XG_PON_INT_STA, phyIntStatus);
	phy_print_time();

#if ASIC_SERDES

	if(gpPhyPriv->rogue_onu_det_en)
	{
		read_data = IO_GPHYREG(XPON_PMA_XPON_INT_EN_0);

		if(read_data & XPON_PMA_TRANS_ROGUE_ONU_INT_EN)
		{
			phypmaIntStatus=IO_GPHYREG(XPON_PMA_XPON_INT_STA_0);
			IO_SPHYREG(XPON_PMA_XPON_INT_STA_0, phypmaIntStatus);

			//clear rogue onu cnt
			IO_SPHYA_REG_BITS(XPON_PMA_XPON_SD_BEHAVIOR_SETTING_1 , 8 , 8 , 1 );
			IO_SPHYA_REG_BITS(XPON_PMA_XPON_SD_BEHAVIOR_SETTING_1 , 8 , 8 , 0 );

			if(phypmaIntStatus & XPON_PMA_TRANS_ROGUE_ONU_INT)
			{
				
				gpPhyPriv->is_rogue_onu =1;
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_PMA_ROGUE_ONU detected. XGPON. REG IntStatus=0x%08x\r\n",phypmaIntStatus);
				handle_hw_irq_event(PHY_EVENT_TF_INT);
				phy_trans_power_switch(PHY_DISABLE);  //for debug
			}
		}

	}
#endif

	if((gpPhyPriv->phy_status != PHY_LINK_STATUS_LOS)&&(phyIntStatus & XGPON_PHY_RX_LOS_INT))
	{
		gpPhyPriv->phy_unexpected_isr_flag=FALSE;
		
		#if ASIC_SERDES
		
		fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);// ang_20180921
		
		//disable rx
		read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
		write_data= read_data & (~XGPON_PHY_RX_ENABLE); //disable rx
		IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

		read_data=IO_GPHYREG(XPON_PMA_RO_RX_FREQDET);	//for debug
		PON_PHY_PRINT(PHY_MSG_DBG,"after tdc off and disabel rx,RX_CK 0xbfaf3820 = 0x%x\n", read_data);

		#endif

		#if A60972_SERDES
		//disable PCS RX
		read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
		IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL,read_data&~XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE) ;
		read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
		printk("Set XGPON_RX_SYNC_CTRL=0x%x \n",read_data);

		#endif
		
		phy_los_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
		trans_laser_detected=FALSE;
		#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
		#endif

	}
	else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & XGPON_PHY_RX_RDY_INT))
	{
		phy_rx_rdy_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_RDY Reset__Serdes XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		phy_delay1ms(2);
		#if (A60972_SERDES)||(ASIC_SERDES)
		phy_pma_reset();		
		#else
		
		read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
		write_data= read_data | XGPON_PHY_RX_ENABLE;	//enable rx
		IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
		
		IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_SERDES);
		IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_DISABLE);
		#endif
		trans_laser_detected = TRUE;
		phy_delay1ms(8); //after pma reset, have to wait the whole system stable. ang_20180621
	}
	else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & XGPON_PHY_RX_SYNC_OK_INT)) 
	{
		read_data_1 = IO_GPHYREG(XGPON_PHY_SFP_STA);
		read_data_2 = IO_GPHYREG(XGPON_PHY_DBG_RX_SYNC_ST);//read bit[18:20]
		
		if(XGPON_PHY_SFP_RX_LOS_ST!=(read_data_1 & XGPON_PHY_SFP_RX_LOS_ST))
		{
			if(XGPON_PHY_DBG_RX_SYNC_ST_SYNC==(read_data_2& XGPON_PHY_DBG_RX_SYNC_ST_SYNC))
			{
				phy_rdy_cnt++;
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
				gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
				#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
				handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
				#endif
			}
			else
			{
				phy_fake_sync_cnt++;
				gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;//ang_20191009
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY LINK is not stable! XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);	
			}
		}
		else //NO laser
		{
			#if ASIC_SERDES
		
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);// ang_20180921
			
			//disable rx
			read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data & (~XGPON_PHY_RX_ENABLE); //disable rx
			IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

			read_data=IO_GPHYREG(XPON_PMA_RO_RX_FREQDET);	//for debug
			PON_PHY_PRINT(PHY_MSG_DBG,"after tdc off and disabel rx,RX_CK 0xbfaf3820 = 0x%x\n", read_data);

			#endif

			#if A60972_SERDES
			//disable PCS RX
			read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL,read_data&~XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			printk("Set XGPON_RX_SYNC_CTRL=0x%x \n",read_data);

			#endif
			
			phy_los_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS XGPON, REG IntStatus=0x%08x\r\n",phyIntStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			trans_laser_detected=FALSE;
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
			#endif
		}
			
	}
	else if(phyIntStatus & XGPON_PHY_RX_LOF_INT)  // LOF may happen after LOS
	{
		#if ASIC_SERDES
		read_data=IO_GPHYREG(XPON_PMA_RO_RX_FREQDET); //for debug
		PON_PHY_PRINT(PHY_MSG_DBG,"When LOF int,RX_CK 0xbfaf3820 = 0x%x\n", read_data);
		#endif
		
		gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
		phy_lof_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_LOF XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		handle_hw_irq_event(PHY_EVENT_PHY_LOF_INT);  //report lof event to mac


		if(phyIntStatus & XGPON_PHY_RX_SYNC_OK_INT) // in case LOF and SYNC_OK assert at same time. ang_20200820
		{
			read_data_2 = IO_GPHYREG(XGPON_PHY_DBG_RX_SYNC_ST);//read bit[18:20]
			
			if(XGPON_PHY_DBG_RX_SYNC_ST_SYNC==(read_data_2& XGPON_PHY_DBG_RX_SYNC_ST_SYNC))
			{
				phy_rdy_cnt++;
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
				gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
				#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
				handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
				#endif
			}
			else
			{
				phy_fake_sync_cnt++;
				gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;//ang_20191009
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY LINK is not stable! XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);	
			}	
			
		}

	}
	else
	{
		phy_other_cnt++;
		gpPhyPriv->phy_unexpected_isr_flag=TRUE;
		phyIntEn=IO_GPHYREG(XGPON_PHY_XG_PON_INT_EN);
		gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected! fw=0x%x, EN=0x%08x XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",gpPhyPriv->phy_status,phyIntEn,isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
	}

	#ifndef PON_PHY_PERIODIC_POLLING
	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
	#endif		
	spin_unlock(&gpPhyPriv->event_handle_lock); 
	return PHY_SUCCESS;

}

int en7580_xgpon_phy_event_poll(char* buf)
{
	ulong flags;
	uint read_data,write_data ;
	PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_SW_POLL};
	int phyReadyStatus,phyLosStatus;
	static UINT32 poll_set_los_cnt=0,poll_set_ready_cnt=0,poll_no_ready_no_los_cnt=0,poll_los_cnt=0,poll_ready_cnt=0;
	static PHY_Los_Status_t  pre_phy_status=PHY_LINK_STATUS_UNKNOWN;


	if(FALSE == gpPhyPriv->is_phy_start){
		return	PHY_FAILURE;	//ang_20180208
	}
	gpPhyPriv->event_poll_timer_value = 1500;

	spin_lock_irqsave(&gpPhyPriv->event_handle_lock, flags);
	
	//phy_print_time();

	PON_PHY_PRINT(PHY_MSG_INT," - POLLING - __START__ \r\n");
	
	phyReadyStatus=phy_ready_status();
	phyLosStatus=phy_los_status();

	if(PHY_LINK_STATUS_READY == gpPhyPriv->phy_status) 
	{
		//if(!is_phy_sync())  /* LOS */
		if(!(phyReadyStatus&&(!phyLosStatus)))
		{
			poll_set_los_cnt++;
			#if XILINX_SERDES
			
			read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data | XGPON_PHY_RX_ENABLE;	//enable rx
			IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

			IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_SERDES);
			IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_DISABLE);
			PON_PHY_PRINT(PHY_MSG_ERR," | POLLING => LOS(%d) PHY_RST_N_SERDES,ReadyStatus=%d,LosStatus=%d\r\n",poll_set_los_cnt,phyReadyStatus,phyLosStatus);
			//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
			PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
			#else
			//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
			PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
			#endif

			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			gpPhyPriv->phy_unexpected_isr_flag=FALSE;
			
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			phy_event.id = PHY_EVENT_TRANS_LOS_INT;
			phy_event_handler(&phy_event);
			#endif
		}
		poll_no_ready_no_los_cnt=0;
	}
	else
	{	 
		//if(phy_no_ready_no_los())
		if((!phyReadyStatus)&&(!phyLosStatus))
		{
			poll_no_ready_no_los_cnt++;

			#if XILINX_SERDES

			read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data | XGPON_PHY_RX_ENABLE;	//enable rx
			IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

			
			IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_SERDES);
			IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N, XGPON_PHY_XG_PHY_RST_N_DISABLE);
			PON_PHY_PRINT(PHY_MSG_ERR," | POLLING => no_ready_no_los(%d)PHY_RST_N_SERDES,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			#endif

			#if ASIC_SERDES
			read_data=IO_GPHYREG(XPON_PMA_RO_RX_FREQDET); //for debug
			PON_PHY_PRINT(PHY_MSG_DBG,"When polling no_ready_no_loss,RX_CK 0xbfaf3820 = 0x%x\n", read_data);
			
			if(poll_no_ready_no_los_cnt%10 ==0)
			{
				PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)SCU Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
				PHY_INTERNAL_SET_API(PON_SET_PHY_SCU_RST,NULL);
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)Reset pma,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
				//TDC_OFF			
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);// ang_20190307			//disable rx			
				read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);			
				write_data= read_data & (~XGPON_PHY_RX_ENABLE); //disable rx			
				IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);			//need to totally reset during no_ready_no_los status.
				phy_pma_reset();	
			}

			gpPhyPriv->event_poll_timer_value = 3500;
			PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4

			#endif

			#if A60972_SERDES
			PON_PHY_PRINT(PHY_MSG_ERR," | POLLING => no_ready_no_los(%d)Reset A60972,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			phy_pma_reset();
			#endif

			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			#ifdef LDDLA_SUPPORT_SET_TX_MODE
			phy_event.id = PHY_EVENT_NO_LOS_NO_READY;
			phy_event_handler(&phy_event);
			#endif
			#endif
		}																

		//if(is_phy_sync())	/* PHY SYNC */
		else if(phyReadyStatus&&(!phyLosStatus))
		{
			
			if(gpPhyPriv->phy_unexpected_isr_flag==TRUE)
			{

				if(gpPhyPriv->wan_sel==SCU_WAN_CONF_REG_WAN_SEL_XGSPON)		//ALU XGSPON OLT is not stable when laser on.
				{
					gpPhyPriv->phy_unexpected_isr_flag=FALSE;
					PON_PHY_PRINT(PHY_MSG_INT,"Unexpected ISR happened, make it LOS\r\n");
					gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
					#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
					handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
					#endif
				}
			}
			
			poll_set_ready_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT," | POLLING => READY(%d) ReadyStatus=%d,LosStatus=%d\r\n",poll_set_ready_cnt,phyReadyStatus,phyLosStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
			
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			phy_event.id = PHY_EVENT_PHYRDY_INT;
			phy_event_handler(&phy_event);
			#endif
			poll_no_ready_no_los_cnt=0;
		}

		else if(!(phyReadyStatus&&(!phyLosStatus)))
		{
			poll_set_los_cnt++;
			
			if (gpPhyPriv->phy_status == PHY_LINK_STATUS_UNKNOWN)	//,only execute when power on without laser. ang_20180502
			{
				#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
				phy_event.id = PHY_EVENT_TRANS_LOS_INT;
				phy_event_handler(&phy_event);
				#endif

				#if ASIC_SERDES
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);// ang_20180921
				#endif
				
				//disable rx
				read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
				write_data= read_data & (~XGPON_PHY_RX_ENABLE); //disable rx
				IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

			}

			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			gpPhyPriv->phy_unexpected_isr_flag=FALSE;
			PON_PHY_PRINT(PHY_MSG_INT," | POLLING => NO laser! (%d) ReadyStatus=%d,LosStatus=%d\r\n",poll_set_ready_cnt,phyReadyStatus,phyLosStatus);
		}
	}
	
	phy_print_time();

#ifdef PON_PHY_PERIODIC_POLLING
	if(pre_phy_status!=gpPhyPriv->phy_status)
	{
		PON_PHY_PRINT(PHY_MSG_INT," - POLLING - __END__ (%d)(%d)(%d) phy_status = %d\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status);
	}
	pre_phy_status=gpPhyPriv->phy_status;

	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
#else
	if(PHY_LINK_STATUS_LOS==gpPhyPriv->phy_status)
	{
		phy_event.id = PHY_EVENT_TRANS_LOS_INT;
		poll_los_cnt++;
	}
	else if(PHY_LINK_STATUS_READY==gpPhyPriv->phy_status)
	{
		phy_event.id = PHY_EVENT_PHYRDY_INT;
		poll_ready_cnt++;
	}
	#ifdef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
	if(pre_phy_status!=gpPhyPriv->phy_status)
	{
		phy_event_handler(&phy_event);
	}
	pre_phy_status=gpPhyPriv->phy_status;
	#endif

	PON_PHY_PRINT(PHY_MSG_INT," - POLLING - __END__ (%d)(%d)(%d) Get phy_status = %d (%d)(%d)\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status,poll_los_cnt,poll_ready_cnt);
#endif

	spin_unlock_irqrestore(&gpPhyPriv->event_handle_lock, flags);

	return PHY_SUCCESS;

}



int en7580_xgpon_phy_dbg(char* buf)
{
	char dbg_id = *buf;
	char dbg_option1 = *(buf+1);
	char dbg_option2 = *(buf+2);

	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;
	
	//temp=buf;
	printk("[%s:%d] id = %d\n",__FUNCTION__,__LINE__,dbg_id);

#ifdef	EN7580_HEC_TEST
	phy_reg_total_num=90;
	phy_reg_all=xgpon_phy_reg_all;

	switch(dbg_id)
	{
		case 0:
			printk("\r\n\r\n___________Read XGPON debug counter only(no reset)___________\n");
			printk("%-44s	 address=value\r\n","_____ XGPON CNT_____");
			for(i=33;i<=41;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}			
			for(i=43;i<=45;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=47;i<=47;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=74;i<=77;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			printk("%-44s	 address=value\r\n","_____ XGPON STATUS_____");
			for(i=42;i<=42;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=46;i<=46;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=50;i<=50;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=58;i<=58;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=78;i<=78;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}			
			printk("%-44s	 address=value\r\n","_____ XGPON FEC HEC PON-ID_____");
			for(i=48;i<=49;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=80;i<=82;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=84;i<=85;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			phy_uptime();
			break;
			
		case 1:
			
			read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
			printk("DBG_CNT_CLEAR on 0x%08x\n",XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
			IO_SPHYREG(XGPON_PHY_DBG_CTRL,read_data|XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
			read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
			
			printk("%-44s	 address=value\r\n","_____ XGPON CNT_____");
			for(i=33;i<=41;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}			
			for(i=43;i<=45;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=47;i<=47;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=74;i<=77;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			printk("%-44s	 address=value\r\n","_____ XGPON STATUS_____");
			for(i=42;i<=42;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=46;i<=46;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=50;i<=50;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=58;i<=58;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=78;i<=78;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}			
			printk("%-44s	 address=value\r\n","_____ XGPON FEC HEC PON-ID_____");
			for(i=48;i<=49;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=80;i<=82;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=84;i<=85;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}

			read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
			printk("DBG_CNT_CLEAR off 0x%08x\n",XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
			CLRPOSITION(read_data,XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);	//ang_20180208
			IO_SPHYREG(XGPON_PHY_DBG_CTRL,read_data);
			read_data=IO_GPHYREG(XGPON_PHY_DBG_CTRL);
			printk("Read  XGPON_PHY_DBG_CTRL(0x%x)=0x%08x\n",XGPON_PHY_DBG_CTRL,read_data);

			break;

		case 2:
			read_data=IO_GPHYREG(XGPON_PHY_PHY_PM_CNT_CLR);
			printk("FEC_DECODE_CNT_CLR on 0x%08x\n",XGPON_PHY_PHY_PM_CNT_CLR_FEC_DECODE_CNT_CLR);
			IO_SPHYREG(XGPON_PHY_PHY_PM_CNT_CLR,read_data|XGPON_PHY_PHY_PM_CNT_CLR_FEC_DECODE_CNT_CLR);
			read_data=IO_GPHYREG(XGPON_PHY_PHY_PM_CNT_CLR);

			printk("%-44s	 address=value\r\n","_____ XGPON CNT_____");
			for(i=33;i<=41;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}			
			for(i=43;i<=45;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=47;i<=47;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=74;i<=77;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			printk("%-44s	 address=value\r\n","_____ XGPON STATUS_____");
			for(i=42;i<=42;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=46;i<=46;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=50;i<=50;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=58;i<=58;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=78;i<=78;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}			
			printk("%-44s	 address=value\r\n","_____ XGPON FEC HEC PON-ID_____");
			for(i=48;i<=49;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=80;i<=82;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=84;i<=85;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			read_data=IO_GPHYREG(XGPON_PHY_PHY_PM_CNT_CLR);
			printk("FEC_DECODE_CNT_CLR off 0x%08x\n",XGPON_PHY_PHY_PM_CNT_CLR_FEC_DECODE_CNT_CLR);
			CLRPOSITION(read_data,XGPON_PHY_PHY_PM_CNT_CLR_FEC_DECODE_CNT_CLR);	//ang_20180208
			IO_SPHYREG(XGPON_PHY_PHY_PM_CNT_CLR,read_data);
			read_data=IO_GPHYREG(XGPON_PHY_PHY_PM_CNT_CLR);
			printk("Read  XGPON_PHY_PHY_PM_CNT_CLR(0x%x)=0x%08x\n",XGPON_PHY_PHY_PM_CNT_CLR,read_data);

			break;
			
		case 3:
			read_data=IO_GPHYREG(XGPON_PHY_PHY_PM_CNT_CLR);
			printk("PSBD_HEC_ERR_CNT_CLR on 0x%08x\n",XGPON_PHY_PHY_PM_CNT_CLR_PSBD_HEC_ERR_CNT_CLR);
			IO_SPHYREG(XGPON_PHY_PHY_PM_CNT_CLR,read_data|XGPON_PHY_PHY_PM_CNT_CLR_PSBD_HEC_ERR_CNT_CLR);
			read_data=IO_GPHYREG(XGPON_PHY_PHY_PM_CNT_CLR);
		
		
			printk("%-44s	 address=value\r\n","_____ XGPON CNT_____");
			for(i=33;i<=41;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}			
			for(i=43;i<=45;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=47;i<=47;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=74;i<=77;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			printk("%-44s	 address=value\r\n","_____ XGPON STATUS_____");
			for(i=42;i<=42;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=46;i<=46;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=50;i<=50;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=58;i<=58;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=78;i<=78;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}			
			printk("%-44s	 address=value\r\n","_____ XGPON FEC HEC PON-ID_____");
			for(i=48;i<=49;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=80;i<=82;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=84;i<=85;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			read_data=IO_GPHYREG(XGPON_PHY_PHY_PM_CNT_CLR);
			printk("PSBD_HEC_ERR_CNT_CLR off 0x%08x\n",XGPON_PHY_PHY_PM_CNT_CLR_PSBD_HEC_ERR_CNT_CLR);
			CLRPOSITION(read_data,XGPON_PHY_PHY_PM_CNT_CLR_PSBD_HEC_ERR_CNT_CLR);	//ang_20180208
			IO_SPHYREG(XGPON_PHY_PHY_PM_CNT_CLR,read_data);
			read_data=IO_GPHYREG(XGPON_PHY_PHY_PM_CNT_CLR);
			printk("Read  XGPON_PHY_PHY_PM_CNT_CLR(0x%x)=0x%08x\n",XGPON_PHY_PHY_PM_CNT_CLR,read_data);
		
			break;	
			
		case 10:
		
			read_data=IO_GPHYREG(XGPON_PHY_SFP_STA);
			printk("Read XGPON_PHY_SFP_STA=0x%8x\n",read_data);
			printk("	 _MOD_ABS_ST      =0x%8x\n",read_data&XGPON_PHY_SFP_MOD_ABS_ST);
			printk("	 _TX_FAULT_ST     =0x%8x\n",read_data&XGPON_PHY_SFP_TX_FAULT_ST);
			printk("	 _TX_SD_ST  	  =0x%8x\n",read_data&XGPON_PHY_SFP_TX_SD_ST);
			printk("	 _RX_LOS_ST	 	  =0x%8x\n",read_data&XGPON_PHY_SFP_RX_LOS_ST);
		
			break;	

		case 20:
		case 21:
		case 22:
			REPORT_EVENT_TO_MAC(PHY_EVENT_START_ROGUE_MODE);
			gpPhyPriv->phyCfg.flags.rogue = 1;
		
			IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CTRL,XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_PATTERN_SEL(dbg_id-20)|XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_EN) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_CONTINUE_CTRL);
			printk("Set XGPON_PHY_XG_CONTINUE_CTRL=0x%x \n",read_data);	

			break;			

		case 23:
			IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER,0x0ff00ff0) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER);
			printk("Set XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER=0x%x \n",read_data);
			IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER,0xa00ff00a) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER);
			printk("Set XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER=0x%x \n",read_data);
		
			REPORT_EVENT_TO_MAC(PHY_EVENT_START_ROGUE_MODE);
			gpPhyPriv->phyCfg.flags.rogue = 1;
		
			IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CTRL,XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_PATTERN_SEL(dbg_id-20)|XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_EN) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_CONTINUE_CTRL);
			printk("Set XGPON_PHY_XG_CONTINUE_CTRL=0x%x \n",read_data);		
		
			break;
			
		case 24:
		case 25:
		case 26:
		
			break;
		
		case 27:
			IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER,0x0ff00ff0) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER);
			printk("Set XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER=0x%x \n",read_data);
			IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER,0xa00ff00a) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER);
			printk("Set XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER=0x%x \n",read_data);

			break;
		
		case 28:
			IO_SPHYREG(XGPON_PHY_XG_CONTINUE_CTRL,XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_DISABLE) ;
			read_data=IO_GPHYREG(XGPON_PHY_XG_CONTINUE_CTRL);
			printk("Set XGPON_PHY_XG_CONTINUE_CTRL=0x%x \n",read_data);		
			
			REPORT_EVENT_TO_MAC(PHY_EVENT_STOP_ROGUE_MODE);
			gpPhyPriv->phyCfg.flags.rogue = 0;
		
			break;
#if A60972_SERDES //EN7580_ASIC			
		case 100:

			read_data=IO_GPHYREG(EN7580_IF_RO_INTERFACE_D);
			printk("Read EN7580_IF_RO_INTERFACE_D = 0x%08x \n",read_data); 	
			printk("	                  _TX_MAX = 0x%x \n",(read_data&EN7580_IF_RO_INTERFACE_D_TX_MAX)>>16);
			printk("	                  _RX_MAX = 0x%x \n",(read_data&EN7580_IF_RO_INTERFACE_D_RX_MAX));
		
			break;
			
		case 101:
			
			read_data=IO_GPHYREG(EN7580_IF_RG_INTERFACE_0);
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_0,read_data&~EN7580_IF_RG_INTERFACE_0_BIST_PAT_RX_CHECK_EN) ;		
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_0,read_data|EN7580_IF_RG_INTERFACE_0_BIST_PAT_RX_CHECK_EN) ;	
			printk("Reset EN7580_IF_RG_INTERFACE_0_BIST_PAT_RX_CHECK_EN \n");	

			break;
			
		case 102:
		
			read_data=IO_GPHYREG(EN7580_IF_RO_INTERFACE_G_TX_MAX_LSB);
			printk("EN7580_IF_RO_INTERFACE_G_TX_MAX_LSB = 0x%08x \n",read_data);	
			read_data=IO_GPHYREG(EN7580_IF_RO_INTERFACE_H_RX_MAX_LSB);
			printk("EN7580_IF_RO_INTERFACE_H_RX_MAX_LSB = 0x%08x \n",read_data);
				
			break;
#endif
		default:
			printk("[%s:%d] UNKNOWN ID\n",__FUNCTION__,__LINE__);

			return PHY_FAILURE;
	}
#endif
	return PHY_SUCCESS;

}



int en7580_xgpon_phy_pma_reset(char* buf)
{

	uint read_data = 0;
	uint write_data = 0;

	//PON_PHY_PRINT(PHY_MSG_INT,"[%s:%d]\r\n",__FUNCTION__,__LINE__);	//ang_20181010

#if A60972_SERDES	// for FPGA	

	//disable PCS RX
	read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
	IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL,read_data&~XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE) ;
	read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
	printk("Set XGPON_RX_SYNC_CTRL=0x%x \n",read_data);

	//reset PCS logic
	IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N,XGPON_PHY_XG_PHY_RST_N_PHYD) ;
	read_data=IO_GPHYREG(XGPON_PHY_XG_PHY_RST_N);
	printk("Hold XGPON_PHY_XG_PHY_RST_N=0x%x \n",read_data);

	//reset A60972
	printk("reset A60972\r\n");
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_ON);
	mdelay(1);
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_OFF);
	//init A60972
	a60972_init();

	write_data = EN7580_IF_RG_A60972_CTRL_IF_SW_RESET_ONLY;
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, write_data);
	mdelay(1);
	write_data = EN7580_IF_RG_A60972_CTRL_NORMAL_OPERATION;
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, write_data);
	printk("EN7580_IF_RG_A60972_CTRL_IF_RX_SW_RESET_ONLY\r\n");

	//reset PCS logic
	IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N,XGPON_PHY_XG_PHY_RST_N_DISABLE) ;
	read_data=IO_GPHYREG(XGPON_PHY_XG_PHY_RST_N);
	printk("Release XGPON_PHY_XG_PHY_RST_N=0x%x \n",read_data);

	//enable PCS RX
	read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
	IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL,read_data|XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE) ;
	read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
	printk("Set XGPON_RX_SYNC_CTRL=0x%x \n",read_data);


#endif
	
#if ASIC_SERDES	// for ASIC

	//reset PCS logic, hold PCS reset
	IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N,XGPON_PHY_XG_PHY_RST_N_ALL) ;
	read_data=IO_GPHYREG(XGPON_PHY_XG_PHY_RST_N);
	PON_PHY_PRINT(PHY_MSG_DBG,"Hold XGPON_PHY_XG_PHY_RST_N=0x%x \n",read_data);
	//mdelay(1);
	phy_delay1ms(1);

	if(gpPhyPriv->first_plugin_flag == TRUE)
	{
		fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
		gpPhyPriv->first_plugin_flag = FALSE;
		gpPhyPriv->pma_init_done= TRUE;
	}
	else
	{
		fiber_plug_reset(PLUG_IN,gpPhyPriv->wan_sel);
	}
	
	PON_PHY_PRINT(PHY_MSG_DBG,"XGPON PMA Reset! \n");	//ang_20181010
	//mdelay(1);
	phy_delay1ms(1);
	
//reset PCS logic, release PCS reset
	IO_SPHYREG(XGPON_PHY_XG_PHY_RST_N,XGPON_PHY_XG_PHY_RST_N_DISABLE) ;
	read_data=IO_GPHYREG(XGPON_PHY_XG_PHY_RST_N);
	PON_PHY_PRINT(PHY_MSG_DBG,"Release XGPON_PHY_XG_PHY_RST_N=0x%x \n",read_data);
	//mdelay(1);
	phy_delay1ms(1);

//enable rx	
	read_data=IO_GPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL);
	write_data= read_data | XGPON_PHY_RX_ENABLE;	//enable rx
	IO_SPHYREG(XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

#endif

	return PHY_SUCCESS;
}

#define _I_AM_XEPON_SPLIT_LINE_

/////////////////////////////////////////////////////////////////////////////////////
// en7580_xepon_func
/////////////////////////////////////////////////////////////////////////////////////

int en7580_xepon_pon_phy_reset(char* buf)
{
	uint val = 0;

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d]\r\n",__FUNCTION__,__LINE__);
	
#ifdef TCSUPPORT_CPU_ARMV8 //julia_arm //7523
	val = GET_SCU_RST_RG(); 
	val |=0x01;
	SET_SCU_RST_RG(val);
	udelay(1);
	//release xpon phy top reset 
	val &=~(0x01);
	SET_SCU_RST_RG(val);		
#else
	val = IO_GPHYREG(TOP_RST_CTRL_SW2);
	val |=0x01;
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
	udelay(1);
	//release xpon phy top reset 
	val &=~(0x01);
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
#endif


	return PHY_SUCCESS;
}


int en7580_xepon_phy_dump(char* buf)
{
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;

	phy_reg_total_num=172;
	phy_reg_all=xepon_phy_reg_all;

	printk("\r\n%-44s	 address=value\r\n","_____ XEPON REG DUMP _____");
	for(i=0;i<=phy_reg_total_num;i++)
	{
		(phy_reg_all+i)->def=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
	}
	
#if	PHY_REG_READ_WRITE_TEST
	//Register Read/Write test
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ZEROS ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_0");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ONES ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_1");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ END ___\r\n");
#endif
	

	return PHY_SUCCESS;
}


int en7580_xepon_phy_dev_init(char* buf)
{
	uint read_data = 0;
	uint write_data = 0;

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d]\r\n",__FUNCTION__,__LINE__);

	/* switch GPIO to XPON mode*/
#ifdef TCSUPPORT_CPU_ARMV8 
	/* switch GPIO to XPON mode*/		
	read_data = GET_IOMUX_CTRL_3(); //GPIO_PON_MODE_7523	
	write_data =(read_data | 0x01); //bit [0]: 1	
	SET_IOMUX_CTRL_3(write_data); //0-->1 //GPIO_PON_MODE_7523
#else 
		
	/* switch GPIO to XPON mode*/
	read_data = IO_GREG(IOMUX_Control_1_register);                                               
	read_data = read_data | (RG_GPIO_PON_MODE) | (RG_PON_I2C_MODE) ;
	IO_SREG(IOMUX_Control_1_register, read_data);                          
#endif 	

	

	/*sigdet*/
	//read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	//read_data = read_data & ~(PHY_SQUELCH_DEGLITCH);
	//IO_SPHYREG(PHY_CSR_PHYSET3, read_data);

#ifdef TCSUPPORT_CPU_ARMV8 //julia_asic
	/*after sw reset, register can be modify by PBUS*/
	read_data = GET_SCU_RST_ACC_CHK_BMA(); //TOP_CSR_PBUS_CTRL
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	SET_SCU_RST_ACC_CHK_BMA(read_data); //TOP_CSR_PBUS_CTRL
	/* transceiver power initial setting*/
	read_data = GET_GPIO_SSR(); //TOP_CSR_GPIO_SHARE
	read_data = read_data & ~(TOP_LED1_MODE);
	SET_GPIO_SSR(read_data); //TOP_CSR_GPIO_SHARE
#else	
	/*after sw reset, register can be modify by PBUS*/
	read_data = IO_GREG(TOP_CSR_PBUS_CTRL);
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	IO_SREG(TOP_CSR_PBUS_CTRL, read_data);
	/* transceiver power initial setting*/
	read_data = IO_GREG(TOP_CSR_GPIO_SHARE);
	read_data = read_data & ~(TOP_LED1_MODE);
	IO_SREG(TOP_CSR_GPIO_SHARE, read_data);
#endif
	//enable PCS RX control 	david 20170124
	read_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
	printk("Read XEPON PCS RX CTRL=0x%x \n",read_data);

	#if ASIC_SERDES
	IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG,read_data|XEPON_PCS_RX_CTRL_CFG_SYNC_TIMES(0)) ;
//	IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG,read_data|XEPON_PCS_RX_CTRL_CFG_PCS_RX_EN|XEPON_PCS_RX_CTRL_CFG_SYNC_TIMES(0)) ;
	#endif 
	#if A60972_SERDES
	IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG,read_data|XEPON_PCS_RX_CTRL_CFG_PCS_RX_EN) ;
	#endif

	read_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
	printk("Set XEPON PCS RX CTRL=0x%x \n",read_data);

	//configure ISR_EN
	//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);   //XEPON_PCS_INT_SYNC_OK_EN); //
	//phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE); 

	/* Enable BIP error counter */
	
	/* change the guard time pattern */
	//gpPhyPriv->phyGuardBitDelm = 0xAAAB5983;
	//IO_SPHYREG(PHY_CSR_GPON_DELIMITER_GUARD, 0xAAAB5983);

	/* clear phy register*/
	
	return PHY_SUCCESS ;

}

int en7580_xepon_phy_int_config(char* buf)
{
	UINT32 *mode = (UINT32 *)buf;
	UINT32 *int_type = (UINT32 *)buf+1;

	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 phyIntStatus;

	if(PHY_INT_EN_MODE_DEFAULT==(*mode))
	{
		write_data=(XEPON_PCS_INT_SYNC_OK_EN\
					|XEPON_PCS_INT_SYNC_LOSS_EN\
					|XEPON_PCS_INT_LASER_RX_LOSS_EN\
					|XEPON_PCS_INT_NOT_LASER_RX_LOSS_EN);									

		/*
			
					|XEPON_PCS_INT_TX_DET_OVERRUN\
					|XEPON_PCS_INT_TX_DET_UNDERRUN\
					|XEPON_PCS_INT_TX_GB_OVERRUN_EN\
					|XEPON_PCS_INT_TX_GB_UNDERRUN_EN\
					|XEPON_PCS_INT_RX_GB_OVERRUN\
					|XEPON_PCS_INT_BER_MONITOR_DONE\
					|XEPON_PCS_INT_BER_HIGH_FLAG

					|XEPON_PCS_INT_CDR_STABLE\
					|XEPON_PCS_INT_CDR_STABLE_LOSS
		
					|XEPON_PCS_INT_RX_GB_EMPTY_APPEAR\
					
					|XEPON_PCS_INT_PHYA_TX_RDY\
					|XEPON_PCS_INT_PHYA_RX_RDY\	

					|XEPON_PCS_INT_RX_SOF_NO_EOF\
					|XEPON_PCS_INT_RX_EOF_NO_SOF\

					|XEPON_PCS_INT_TX_SD_NORMAL\
					|XEPON_PCS_INT_TXSD_NEQUAL_TXBST\

					|XEPON_PCS_INT_PHYA_TX_RDY_LOSS\
					|XEPON_PCS_INT_PHYA_RX_RDY_LOSS);

		*/
		
		PON_PHY_PRINT(PHY_MSG_TRACE,"XEPON Phy use default int_type:0x%.8x\n", write_data);
	}
	else
	{
		write_data=(*int_type);
		PON_PHY_PRINT(PHY_MSG_TRACE,"XEPON Phy_Int_Config type     :0x%.8x\n", write_data);
	}

	//NOT Clear PHY ISR Status, Let PHY_ISR() function deal with the ISR Status state machine
	//Otherwise, we may miss the ISR status after PHY init and before register ISR to system

	read_data = IO_GPHYREG(XEPON_PCS_INT_EN);	
	PON_PHY_PRINT(PHY_MSG_TRACE,"XEPON Phy_Int_Config read     :0x%.8x\n", read_data);

	IO_SPHYREG(XEPON_PCS_INT_EN, write_data);
	read_data = IO_GPHYREG(XEPON_PCS_INT_EN);
	PON_PHY_PRINT(PHY_MSG_TRACE,"XEPON Phy_Int_Config write    :0x%.8x\n", read_data);
	

	return PHY_SUCCESS;
}

int en7580_xepon_phy_get_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;


	api_data->ret = PHY_SUCCESS;

	switch(api_data->cmd_id)
	{
		case PON_GET_PHY_LOS_STATUS:
			read_data = IO_GPHYREG(XEPON_PCS_SFP_STATUS);
			if( (read_data & XEPON_PCS_SFP_STATUS_RX_LOSS) == XEPON_PCS_SFP_STATUS_RX_LOSS) //it is transceiver sfp rx LOSS no PHY digital
			{
				api_data->ret = PHY_LOS_HAPPEN;
			}
			else
			{
				api_data->ret = PHY_NO_LOS_HAPPEN;
			}
			break;
			
		case PON_GET_PHY_READY_STATUS:
			read_data = IO_GPHYREG(XEPON_PCS_RX_SYNC_STATUS);//read bit[18:20]
			if((read_data& XEPON_PCS_RX_SYNC_STATUS_OK) == XEPON_PCS_RX_SYNC_STATUS_OK)
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Ready. \n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy not Ready. \n");
				api_data->ret = PHY_FALSE;
			}
			break; 
			
		case PON_GET_PHY_IS_SYNC:
			read_data = IO_GPHYREG(XEPON_PCS_SFP_STATUS);
			if( (read_data & XEPON_PCS_SFP_STATUS_RX_LOSS) != XEPON_PCS_SFP_STATUS_RX_LOSS) //it is transceiver sfp rx LOSS no PHY digital
			{
				read_data = IO_GPHYREG(XEPON_PCS_RX_SYNC_STATUS);
				if((read_data& XEPON_PCS_RX_SYNC_STATUS_OK) == XEPON_PCS_RX_SYNC_STATUS_OK)
				{
					api_data->ret = PHY_TRUE; // PHY Sync OK 
					break;
				}
				else{
					api_data->ret = PHY_FALSE; // PHY Not Sync 
			break;
				}
			}
			else{
				api_data->ret = PHY_FALSE; // PHY Not Sync 
				break;
			}

		case PON_GET_PHY_MODE:
			api_data->ret = gpPhyPriv->phyCfg.flags.mode;
			break;
			
		case PON_GET_PHY_TX_LONG_FLAG:
			api_data->ret = gpPhyPriv->phyCfg.flags.txLongFlag;
			break;

		case PON_GET_PHY_TRANS_STATUS:
			phy_trans_param_status_real(&api_data->trans_status->params);
			api_data->trans_status->alarms = phy_tx_alarm() | phy_rx_power_alarm();
			break;

		case PON_GET_PHY_GET_TX_POWER_EN_FLAG:
			api_data->ret = gpPhyPriv->phyCfg.flags.txPowerEnFlag;
			break;

		case PON_GET_PHY_BIP_COUNTER:
			
			break;

		case PON_GET_PHY_RX_FEC_COUNTER:
			api_data->rx_fec_cnt->correct_bytes=IO_GPHYREG(XEPON_PCS_ALL_ERR_CORR_CDWD);	
			api_data->rx_fec_cnt->correct_codewords=IO_GPHYREG(XEPON_PCS_ALL_ERR_CORR_CDWD);
			api_data->rx_fec_cnt->uncorrect_codewords=IO_GPHYREG(XEPON_PCS_ALL_ERR_NO_CORR_CDWD);	// no correctable bytes counter for XEPON
			api_data->rx_fec_cnt->total_rx_codewords=IO_GPHYREG(XEPON_PCS_ALL_CDWD);
			api_data->rx_fec_cnt->fec_seconds=IO_GPHYREG(XEPON_PCS_FEC_ERR_SECOND); 
			break;

		case PON_GET_PHY_RX_FRAME_COUNTER:
			api_data->rx_frame_cnt->frame_count_high = 0x0;
			api_data->rx_frame_cnt->frame_count_low = IO_GPHYREG(XEPON_PCS_RX_SOF_NUM);;
			break;

		case PON_GET_PHY_RX_FEC_GETTING:
			//api_data->ret = (IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG)>>31);//cheng_20220328
			if(((IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG)>>31) & 0x01) == 1)//cheng_20220328 call trace
			{
				api_data->ret = PHY_TRUE;
			}
			else
			{
				api_data->ret = PHY_FALSE;
			}
			break;

		case PON_GET_PHY_RX_FEC_STATUS:
			read_data = IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
			if((read_data & XEPON_PHY_RX_FEC) == XEPON_PHY_RX_FEC)
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "RX FEC on.\n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "RX FEC off.\n");
				api_data->ret = PHY_FALSE;
			}
			break;

		case PON_GET_PHY_TX_FEC_STATUS:
			if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G)
			{
				read_data = IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
				if((read_data & XEPON_SYMM_PHY_TX_FEC) == XEPON_SYMM_PHY_TX_FEC)
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC on. \n");
					api_data->ret = PHY_TRUE;
				}
				else
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC off. \n");
					api_data->ret = PHY_FALSE;
				}
			}
			else if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G)
			{
				  read_data = IO_GPHYREG(XEPON_1G_PHYTX_STATUS);
				if((read_data & XEPON_ASYM_PHY_TX_FEC) == XEPON_ASYM_PHY_TX_FEC)
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC on. \n");
					api_data->ret = PHY_TRUE;
				}
				else
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC off. \n");
					api_data->ret = PHY_FALSE;
				}
			}
			else 
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "Mode Error:wan_sel =%d \n",gpPhyPriv->wan_sel);
				api_data->ret = PHY_NO_API;
			}
				
			break;

		case PON_GET_PHY_TX_BURST_GETTING:

#if ASIC_SERDES
           	read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1);
			if( (read_data & XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK)== XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK )
			{
				api_data->ret = PHY_TX_BURST_MODE;
			}
			else
			{
	           	read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2);

				if( (read_data & XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)== XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)
				{
				api_data->ret = PHY_TX_CONT_MODE;
			}
				else
				{
					api_data->ret = PHY_TX_BURST_MODE;
				}
			}
#endif
			break;

		case PON_GET_PHY_TRANS_TX:
			#if ASIC_SERDES //EN7580_ASIC
			read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
			if((read_data & ~(XPON_PMA_XPON_SETTING_1_TX_SD_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_DISABLE;

			read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
			if((read_data & ~(XPON_PMA_XPON_SETTING_0_BURST_EN_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_DISABLE;
			if((read_data & ~(XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_DISABLE;
			#endif
			break;

		case PON_GET_PHY_TRANS_RX_GETTING:
			
#if ASIC_SERDES
            read_data = IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
			if((read_data & XPON_PMA_XPON_SETTING_0_RX_SD_INV) == 0)
				api_data->ret = PHY_DISABLE;
			else
				api_data->ret = PHY_ENABLE;
#endif
			break;

		case PON_GET_PHY_ROUND_TRIP_DELAY:

			break;

        case PON_GET_PHY_LOF_STATUS:
			//read_data = IO_GPHYREG(LOF);	  
	  		api_data->ret=0;

			break;
				
		case PON_GET_PHY_INIT_STATUS: //ang_20190821
			api_data->ret = gpPhyPriv->phy_init_done;
			break;
				
		default:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
			api_data->ret = PHY_NO_API;
			break;
	}

	return PHY_SUCCESS; //ang_20180208
}

int en7580_xepon_phy_set_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	int rogue_pattern = 0;

    api_data->ret = PHY_SUCCESS;
    
    switch(api_data->cmd_id)
    {
        case PON_SET_PHY_MODE_CONFIG:
            api_data->ret = phy_mode_config(api_data->phy_mode_cfg->mode, api_data->phy_mode_cfg->txEnable);
            break;
			
		case PHY_SET_ERR_CNT_EN:
		case PHY_SET_BIP_CNT_EN:
		case PHY_SET_FM_CNT_EN:
			break; 

        case PON_SET_PHY_RESET_COUNTER:
            IO_SPHYREG(XEPON_PCS_CLR_ALL_NUM, XEPON_PCS_CLR_ALL_NUM_CLR);
            break;
			
        case PON_SET_PHY_START:
            api_data->ret = xpon_phy_start();
            break;

        case PON_SET_PHY_STOP:
            api_data->ret = xpon_phy_stop();
            break;

        case PON_SET_PHY_FW_READY:
            break;

        case PON_SET_PHY_COUNTER_CLEAR:
			IO_SPHYREG(XEPON_PCS_CLR_ALL_NUM, XEPON_PCS_CLR_ALL_NUM_CLR);
            break;

        case PON_SET_EPONMODE_PHY_RESET:
            break;

        case PON_SET_PHY_BIT_DELAY:
            break;

        case PON_SET_PHY_TX_POWER_CONFIG:
            phy_tx_power_config(*api_data->data);
            break;

        case PON_SET_PHY_ROGUE_PRBS_CONFIG:

			#if ASIC_SERDES
			if(gpPhyPriv->first_plugin_flag == TRUE)
			{
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
				gpPhyPriv->first_plugin_flag = FALSE;
				gpPhyPriv->pma_init_done= TRUE;
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
				mdelay(350);	//add by david 20190508  
				phy_trans_power_switch(PHY_ENABLE);	
			}	// by david 20190522	incase onu startup without OLT DS Signal, RX & TX could L2R lock.
			#endif
			
		//	printk("api_data->phy_rogue_cfg->rogue_onoff = 0x%x \n",api_data->phy_rogue_cfg->rogue_onoff);
		//	printk("api_data->phy_rogue_cfg->rogue_pattern = 0x%x \n",api_data->phy_rogue_cfg->rogue_pattern);
		//	printk("api_data->phy_rogue_cfg->tx_d_in_timeslot = 0x%x \n",api_data->phy_rogue_cfg->tx_d_in_timeslot);
			if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G)
			{
				PON_PHY_PRINT(PHY_MSG_ERR,"rogue_mode=%d, rogue_pattern_sel=%d, tx_d_in_timeslot=%d\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);
				if((api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_ROGUE_MODE)&&(api_data->phy_rogue_cfg->rogue_pattern<=3)&&(api_data->phy_rogue_cfg->tx_d_in_timeslot<=1))
				{
					gpPhyPriv->phyCfg.flags.rogue = 1;
					if (api_data->phy_rogue_cfg->tx_d_in_timeslot == 1)
					{
						api_data->phy_rogue_cfg->tx_d_in_timeslot = 0;
					}else if (api_data->phy_rogue_cfg->tx_d_in_timeslot == 0)
					{
						api_data->phy_rogue_cfg->tx_d_in_timeslot = 1;
					}

					rogue_pattern = api_data->phy_rogue_cfg->rogue_pattern;
						
					if (api_data->phy_rogue_cfg->rogue_pattern == 0)
					{
						api_data->phy_rogue_cfg->rogue_pattern = 1;
					}else if (api_data->phy_rogue_cfg->rogue_pattern == 1)
					{
						api_data->phy_rogue_cfg->rogue_pattern = 0;
					}
				
					IO_SPHYREG(XEPON_PCS_CONTINUE_CTRL,XEPON_PCS_CONTINUE_CTRL_EN\
														|XEPON_PCS_CONTINUE_CTRL_DATA_SEL(api_data->phy_rogue_cfg->rogue_pattern)\
														|XEPON_PCS_CONTINUE_CTRL_MODE_SEL(api_data->phy_rogue_cfg->tx_d_in_timeslot)) ;
					read_data=IO_GPHYREG(XEPON_PCS_CONTINUE_CTRL);
					PON_PHY_PRINT(PHY_MSG_ERR,"Set XEPON_PCS_CONTINUE_CTRL(0x%x)=0x%x \n",XEPON_PCS_CONTINUE_CTRL,read_data);		
					PON_PHY_PRINT(PHY_MSG_ERR,"Rogue %s\n",(XEPON_PCS_CONTINUE_CTRL_EN&read_data)?"ON":"OFF");
					PON_PHY_PRINT(PHY_MSG_ERR,"Rogue pattern %d (0=PRBS7,1=PRBS23,2=PRBS31,3=USER_CONFIG)\n",rogue_pattern);
					PON_PHY_PRINT(PHY_MSG_ERR,"In_timeslot %s\n",(XEPON_PCS_CONTINUE_CTRL_MODE_NORMAL&read_data)?"Normal":"Rogue");
					if(3==api_data->phy_rogue_cfg->rogue_pattern)
					{
						PON_PHY_PRINT(PHY_MSG_DBG,"Note : Rogue pattern is USER_CONFIG\n\tPlease set data by follow cmds:\n\tsys memwl bfaf102c <high_32bit_data>\n\tsys memwl bfaf1030 <low_32bit_data>\n");
					}
				} 
				else if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_NORMAL_MODE) 
				{
					IO_SPHYREG(XEPON_PCS_CONTINUE_CTRL,XEPON_PCS_CONTINUE_CTRL_DISABLE\
														|XEPON_PCS_CONTINUE_CTRL_DATA_SEL(0)\
														|XEPON_PCS_CONTINUE_CTRL_MODE_SEL(0)) ;
					read_data=IO_GPHYREG(XEPON_PCS_CONTINUE_CTRL);
					PON_PHY_PRINT(PHY_MSG_ERR,"Set XEPON_PCS_CONTINUE_CTRL(0x%x)=0x%x \n",XEPON_PCS_CONTINUE_CTRL,read_data);	
					PON_PHY_PRINT(PHY_MSG_ERR, "Rogue %s\n",(XEPON_PCS_CONTINUE_CTRL_EN&read_data)?"ON":"OFF");
					gpPhyPriv->phyCfg.flags.rogue = 0;
				}
				else
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1), pattern %d(must <=3), in_timeslot %d(must <=1)\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);
					api_data->ret = PHY_FAILURE;
				}
			}
			else if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G)
			{
				PON_PHY_PRINT(PHY_MSG_ERR,"rogue_mode=%d, rogue_pattern_sel=%d, tx_d_in_timeslot=%d\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);
				#if A60972_SERDES
				if((api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_ROGUE_MODE)&&(api_data->phy_rogue_cfg->rogue_pattern<=3)&&(api_data->phy_rogue_cfg->tx_d_in_timeslot<=1)) 
				{
        			gpPhyPriv->phyCfg.flags.rogue = 1;
					
					/* EPON mode set continue mode */
					//IO_SBITS(0xBFAF0108,0x80);
					read_data = IO_GPHYREG(XEPON_1G_PHYSET3) ;
					write_data = read_data | 0x80;
					IO_SPHYREG(XEPON_1G_PHYSET3,write_data);
					
					PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Rogue_PRBS mode\n");
			     	} 
				else if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_NORMAL_MODE) 
				{
					//IO_CBITS(0xBFAF0108,0x80);
					read_data = IO_GPHYREG(XEPON_1G_PHYSET3) ;
					write_data = read_data & ~0x80;
					IO_SPHYREG(XEPON_1G_PHYSET3,write_data);	
			        gpPhyPriv->phyCfg.flags.rogue = 0;
			        PON_PHY_PRINT(PHY_MSG_DBG, "Phy_NO_Rogue_PRBS mode\n");
		    	}
				else
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1), pattern %d(must <=3), in_timeslot %d(must <=1)\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);
					api_data->ret = PHY_FAILURE;
				}
				#endif
				#if ASIC_SERDES
				if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_ROGUE_MODE)
				{
        			gpPhyPriv->phyCfg.flags.rogue = 1;
					if (api_data->phy_rogue_cfg->tx_d_in_timeslot == 1)
					{
						if(api_data->phy_rogue_cfg->rogue_pattern<=3)
						{
					read_data = IO_GPHYREG(XPON_PMA_BISTCTL_CONTROL) ;
							write_data = (read_data & XPON_PMA_BISTCTL_CONTROL_PAT_SEL_MASK)|(api_data->phy_rogue_cfg->rogue_pattern+1);
					write_data = (write_data & XPON_PMA_BISTCTL_CONTROL_PAT_TX_EN_MASK)|0x100;
					IO_SPHYREG(XPON_PMA_BISTCTL_CONTROL,write_data);
        
					read_data = IO_GPHYREG(XPON_PMA_ADD_XPON_MODE_1);
					write_data = (read_data & XPON_PMA_ADD_XPON_MODE_1_R2T_MODE_MASK);
					write_data = (write_data & XPON_PMA_ADD_XPON_MODE_1_TX_BIST_MASK)|0x10000;				
					IO_SPHYREG(XPON_PMA_ADD_XPON_MODE_1,write_data);				
				
							PON_PHY_PRINT(PHY_MSG_ERR,"Rogue ON\n");
							PON_PHY_PRINT(PHY_MSG_ERR,"Rogue pattern %d (0=PRBS7,1=PRBS9,2=PRBS15,3=PRBS23)\n",api_data->phy_rogue_cfg->rogue_pattern);
							PON_PHY_PRINT(PHY_MSG_ERR,"In_timeslot Rogue\n");
						}
						else
						{
							PON_PHY_PRINT(PHY_MSG_ERR, "error input: pattern %d(must <=3) while Rogue mode 1 and in_timeslot 1\n",api_data->phy_rogue_cfg->rogue_pattern);
							api_data->ret = PHY_FAILURE;
						}
					}
					else if (api_data->phy_rogue_cfg->tx_d_in_timeslot == 0)
					{
						if(api_data->phy_rogue_cfg->rogue_pattern<=1)
						{
							read_data = IO_GPHYREG(XPON_PMA_MD32_MEM_CLK_CTRL) ;
							write_data = (read_data & XPON_PMA_MD32_MEM_CLK_CTRL_CLK_SEL_MASK)|0x3;
							IO_SPHYREG(XPON_PMA_MD32_MEM_CLK_CTRL,write_data);
				
							read_data = IO_GPHYREG(XPON_PMA_TX_DLY_CTRL);
							write_data = (read_data & XPON_PMA_TX_DLY_CTRL_OUTBEN_DATA_MASK)|((api_data->phy_rogue_cfg->rogue_pattern+2)<<28);
							IO_SPHYREG(XPON_PMA_TX_DLY_CTRL,write_data);	
				
				
							read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1);
							write_data = read_data &( ~XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK);
							IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1, write_data);
				
							read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2);
							write_data = read_data |(XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK);
							IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2, write_data);
				
							PON_PHY_PRINT(PHY_MSG_ERR,"Rogue ON\n");
							PON_PHY_PRINT(PHY_MSG_ERR,"Rogue pattern %d (0=PRBS9,1=PRBS31)\n",api_data->phy_rogue_cfg->rogue_pattern);
							PON_PHY_PRINT(PHY_MSG_ERR,"In_timeslot Normal\n");
						}
						else
						{
							PON_PHY_PRINT(PHY_MSG_ERR, "error input: pattern %d(must <=1) while Rogue mode 1 and in_timeslot 0\n",api_data->phy_rogue_cfg->rogue_pattern);
							api_data->ret = PHY_FAILURE;
						}
					}
					else
					{
						PON_PHY_PRINT(PHY_MSG_ERR, "error input: in_timeslot %d(must <=1)\n",api_data->phy_rogue_cfg->tx_d_in_timeslot);
						api_data->ret = PHY_FAILURE;
					}
				} 
				else if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_NORMAL_MODE)
				{
					gpPhyPriv->phyCfg.flags.rogue = 0;
					PON_PHY_PRINT(PHY_MSG_ERR, "Phy_NO_Rogue_PRBS mode\n");
					read_data = IO_GPHYREG(XPON_PMA_BISTCTL_CONTROL) ;
					write_data = (read_data & XPON_PMA_BISTCTL_CONTROL_PAT_SEL_MASK);
					write_data = (write_data & XPON_PMA_BISTCTL_CONTROL_PAT_TX_EN_MASK);
					IO_SPHYREG(XPON_PMA_BISTCTL_CONTROL,write_data);
        
					read_data = IO_GPHYREG(XPON_PMA_ADD_XPON_MODE_1);
					write_data = (read_data & XPON_PMA_ADD_XPON_MODE_1_R2T_MODE_MASK);
					write_data = (write_data & XPON_PMA_ADD_XPON_MODE_1_TX_BIST_MASK);				
					IO_SPHYREG(XPON_PMA_ADD_XPON_MODE_1,write_data);

					read_data = IO_GPHYREG(XPON_PMA_MD32_MEM_CLK_CTRL) ;
					write_data = (read_data & XPON_PMA_MD32_MEM_CLK_CTRL_CLK_SEL_MASK)|0x3;
					IO_SPHYREG(XPON_PMA_MD32_MEM_CLK_CTRL,write_data);
				
					read_data = IO_GPHYREG(XPON_PMA_TX_DLY_CTRL);
					write_data = (read_data & XPON_PMA_TX_DLY_CTRL_OUTBEN_DATA_MASK)|(1<<28);
					IO_SPHYREG(XPON_PMA_TX_DLY_CTRL,write_data);	
				
				
					read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1);
					write_data = read_data |(XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK);
					IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_1, write_data);
				
					read_data = IO_GPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2);
					write_data = read_data &(~XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK);
					IO_SPHYREG(XPON_PMA_DA_XPON_TX_FORCE_2, write_data);
				}
				else
				{
					//PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1)\n",api_data->phy_rogue_cfg->rogue_onoff);
					api_data->ret = PHY_FAILURE;
				}
				#endif
			}
            break;
			
		case PON_SET_PHY_LOGIC_RESET:
			
			break;

		case PON_SET_PHY_SCU_RST:
			#if ASIC_SERDES
			phy_trans_power_switch(PHY_TX_DIS_ON_HW_ONLY); //tx_dis_reset_protection
			gpPhyPriv->debugLevel_backup=gpPhyPriv->debugLevel;
			gpPhyPriv->debugLevel=PHY_MSG_OFF;
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			//PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
			xPON_Phy_Reset();
			gpPhyPriv->phy_init_done=FALSE;
			gpPhyPriv->is_phy_start = FALSE;
			gpPhyPriv->phy_init_done = FALSE; 
			gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
			gpPhyPriv->trans_tx_enable = PHY_DISABLE;
			gpPhyPriv->phy_unexpected_isr_flag=FALSE;
			gpPhyPriv->first_plugin_flag = TRUE; //ang_20180926
			gpPhyPriv->pma_init_done = FALSE;	//ang_20190108
			gpPhyPriv->trans_msg_print_cnt = 95;
			pon_phy_mod_init();
			if(gpPhyPriv->rx_fec_setting == PHY_ENABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_ENABLE;
				write_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
				IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, write_data|XEPON_PCS_RX_CTRL_CFG_FEC_DEC_EN);
				write_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Enable (%x)=0x%.8x\n", XEPON_PCS_RX_CTRL_CFG,write_data);

			}
			else if(gpPhyPriv->rx_fec_setting == PHY_DISABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_DISABLE;
				write_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
				IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, write_data&~XEPON_PCS_RX_CTRL_CFG_FEC_DEC_EN);
				write_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Disable (%x)=0x%.8x\n", XEPON_PCS_RX_CTRL_CFG,write_data);
			}
			xpon_phy_start();
			gpPhyPriv->debugLevel=gpPhyPriv->debugLevel_backup;
			phy_trans_power_switch(PHY_TX_DIS_RESTORE_BY_SW);  //tx_dis_reset_protection
			PON_PHY_PRINT(PHY_MSG_ERR,"PHY_SCU_RESET\n");
			#endif
			break;

        case PON_SET_PHY_DEV_INIT:
            api_data->ret = phy_dev_init();
            break;

        case PON_SET_PHY_TRANS_POWER_SWITCH:
            api_data->ret = phy_trans_power_switch(*api_data->data);
            break;

        case PON_SET_PHY_RESTORE_PREAMBLE_AND_GUARD_BIT:
            break;

        case PON_SET_PHY_TRANS_MODEL_SETTING:
            if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{

#if ASIC_SERDES || A60972_SERDES

				read_data=IO_GPHYREG(XEPON_PCS_SFP_STATUS);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Read XEPON_PCS_SFP_STATUS(0x%x)=0x%x \n",XEPON_PCS_SFP_STATUS,read_data);
				IO_SPHYREG(XEPON_PCS_SFP_STATUS,phy_trans_iot_list[gpPhyPriv->trans_index].xepon_pcs_sfp_status) ;
				read_data=IO_GPHYREG(XEPON_PCS_SFP_STATUS);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Set XEPON_PCS_SFP_STATUS(0x%x)=0x%x \n",XEPON_PCS_SFP_STATUS,read_data);
				
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Read XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_0,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_0);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Set XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_0,read_data);

				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Read XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);
				IO_SPHYREG(XPON_PMA_XPON_SETTING_1,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_1) ;
				read_data=IO_GPHYREG(XPON_PMA_XPON_SETTING_1);
				PON_PHY_PRINT(PHY_MSG_TRACE, "Set XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",XPON_PMA_XPON_SETTING_1,read_data);	
#endif
            }
            break;


        case PON_SET_PHY_RX_FEC_SETTING:
			if((*api_data->data) == PHY_ENABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_ENABLE;
				write_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
				IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, write_data|XEPON_PCS_RX_CTRL_CFG_FEC_DEC_EN);
				write_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Enable (%x)=0x%.8x\n", XEPON_PCS_RX_CTRL_CFG,write_data);

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_DISABLE;
				write_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
				IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, write_data&~XEPON_PCS_RX_CTRL_CFG_FEC_DEC_EN);
				write_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Disable (%x)=0x%.8x\n", XEPON_PCS_RX_CTRL_CFG,write_data);
			}
			else
			{
				gpPhyPriv->rx_fec_setting=PHY_DEFAULT;
			}
            break;

        case PON_SET_PHY_TX_BURST_CONFIG:
            if(((*api_data->data) != PHY_TX_BURST_MODE) && ((*api_data->data) != PHY_TX_CONT_MODE))
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : burst_mode = %d\n",(*api_data->data));
				api_data->ret = PHY_FAILURE;
			}
			else
			{
				IO_SPHYREG(XEPON_PCS_CONTINUE_CTRL, (XEPON_PCS_CONTINUE_CTRL_MODE_NORMAL\
													|XEPON_PCS_CONTINUE_CTRL_DATA_SEL(0x1)\
													|(*api_data->data)));
				//	XEPON_PCS_CONTINUE_CTRL_DATA_SEL: 
				//										0x0  PRBS23
				//										0x1  PRBS 7
				//										0x2  PRBS31
				//										0x3  User Config 
				//	IO_SPHYREG(XEPON_PCS_CONTINUE_USER_CFG_DATA0,0);
				//	IO_SPHYREG(XEPON_PCS_CONTINUE_USER_CFG_DATA1,0);
				gpPhyPriv->phyCfg.flags.txLongFlag = (((*api_data->data)==PHY_TX_CONT_MODE)? PHY_TRUE : PHY_FALSE);
				api_data->ret = PHY_SUCCESS;
			}
			
            break;

        case PON_SET_PHY_GPON_DELIMITER_GUARD:
             break;

        case PON_SET_PHY_TRANS_TX_SETTINGS:
            break;

        case PON_SET_PHY_TRANS_RX_SETTING:
            break;

        case PON_SET_PHY_GPON_PREAMBLE:
            break;

        case PON_SET_PHY_GPON_EXTEND_PREAMBLE:
            break;
			
		case PHY_SET_TX_FEC_EN:
            if((*api_data->data) == PHY_ENABLE)
			{
				read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
				IO_SPHYREG(XEPON_PCS_TX_CTRL_CFG,read_data|XEPON_PCS_TX_CTRL_CFG_FEC_EN);
				read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Tx FEC Enable (0x%x)=0x%.8x\n", XEPON_PCS_TX_CTRL_CFG,read_data);

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
				IO_SPHYREG(XEPON_PCS_TX_CTRL_CFG, read_data&~XEPON_PCS_TX_CTRL_CFG_FEC_EN);
				read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Tx FEC Disable (0x%x)=0x%.8x\n", XEPON_PCS_TX_CTRL_CFG,read_data);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			break;			
#if (defined(TCSUPPORT_WAN_GPON) || defined(TCSUPPORT_WAN_EPON))&& defined(TCSUPPORT_XPON_HAL_API_EXT)
        case PON_SET_PHY_NOTIFY_EVENT:
            if(gpPhyPriv->phyXponStateNotifyHandler){
                gpPhyPriv->phyXponStateNotifyHandler(*api_data->data);
            }
            break;
#endif
		case PON_SET_PHY_TRAFFIC_STATUS:
			api_data->ret = phy_set_traffic_status(*api_data->data);
				break;
#ifdef TCSUPPORT_MT7570
		case PON_SET_PHY_EN7571_INIT_DONE:

			gpPhyPriv->en7571_init_done = FALSE;
			PON_PHY_PRINT(PHY_MSG_ERR, "ERROR: Should not init 7571 in XEPON mode !\r\n");

			break;

		case PON_SET_PHY_RX_CDR:
#if ASIC_SERDES
			if((*api_data->data) == PHY_ENABLE)
			{
				xpon_phy_stop();
				gpPhyPriv->phy_init_done=FALSE;
				gpPhyPriv->first_plugin_flag = TRUE; 
				pon_phy_mod_init();		// xpon_phy_10g module reset (the most complete reset cmd)
				xpon_phy_start();
			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				xpon_phy_stop();
				xpon_init(gpPhyPriv->wan_sel);
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel); 							// reset pma and first_plug_in
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	// force RX L2R lock
				phy_int_config(PHY_INT_EN_MODE_MANUAL, PHY_INT_EN_DISABLE);		//disable PON_PHY isr mask
				PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);	// stop event_poll_timer	
				mdelay(350);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
#endif
			break;
#endif

#ifdef LDDLA_SUPPORT_SET_TX_MODE
		case PON_SET_PHY_LDDLA_AUTO_MODE:
			if((*api_data->data) == PHY_ENABLE)
			{
				gpPhyPriv->lddla_auto_mode= TRUE; 

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				gpPhyPriv->lddla_auto_mode= FALSE;
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			PON_PHY_PRINT(PHY_MSG_ERR, "MAC set lddla_auto_mode = %s\n",(gpPhyPriv->lddla_auto_mode==TRUE)?"TRUE":"FALSE");
			break;
#endif

        default:
            PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
            api_data->ret = PHY_NO_API;
            break;
    }

	return PHY_SUCCESS ;	//ang_20180208
}



int en7580_xepon_phy_isr(char* buf)
{
	ulong flags;
	uint phyIntStatus, phyIntEn ;
	PHY_TransConfig_T trans_status;
	PPHY_TransConfig_T p_trans_status = & trans_status;
    PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_HW_IRQ };
	static uint isr_cnt=0,phy_rdy_cnt=0,phy_los_cnt=0,laser_no_los_cnt=0,phy_other_cnt=0,laser_los_cnt=0,phy_lof_cnt=0;
	uint read_data = 0;
	static uint trans_laser_detected = FALSE;  //david 20170606
	//solve linux 3.18 compile issue

	
	phy_print_time();

	spin_lock(&gpPhyPriv->event_handle_lock);	
	isr_cnt++;	
	/*get interrupt status and clear*/
	phyIntStatus=IO_GPHYREG(XEPON_PCS_INT_STATUS);
	IO_SPHYREG(XEPON_PCS_INT_STATUS, phyIntStatus);

#if A60972_SERDES
	if (trans_laser_detected == TRUE)	
	{		
		if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_SYNC_OK)) 
		{
			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
		
			handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
			phy_rdy_cnt++;
			trans_laser_detected = FALSE;			
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY ");
		}
		else
		{
			phy_other_cnt++;
			phyIntEn=IO_GPHYREG(XEPON_PCS_INT_EN);
			trans_laser_detected = FALSE;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected when detected! fw status=0x%x,INT EN=0x%08x",gpPhyPriv->phy_status,phyIntEn);
		}
	}
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&((phyIntStatus & XEPON_PCS_INT_SYNC_LOSS)||(phyIntStatus & XEPON_PCS_INT_LASER_RX_LOSS)))
	{
		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
	
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT) ;
		phy_los_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS " );
	}
	else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_NOT_LASER_RX_LOSS))
	{
		
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> No Laser Loss Reset_PMA_start trans_laser_detected=%d\r\n", trans_laser_detected);

		laser_no_los_cnt++;
		trans_laser_detected = TRUE;
		#if 0	
		IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_ON);
		IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_OFF);
		PON_PHY_PRINT(PHY_MSG_TRACE,"\r\n<ISR> No Laser Loss, Reset Serdes" );
		//IO_SPHYREG(XEPON_PCS_INT_EN,(XEPON_PCS_INT_SYNC_OK_EN|XEPON_PCS_INT_SYNC_LOSS_EN));
		#endif
				
		phy_pma_reset();	
		
		phy_print_time();
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> No Laser Loss Reset_PMA_done trans_laser_detected=%d\r\n", trans_laser_detected);
	}
	
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_TX_GB_OVERRUN))
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> TX_GB_OVERRUN " );
	}

	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_TX_GB_UNDERRUN))
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> TX_GB_UNDERRUN " );
	}
	else if(phyIntStatus & XEPON_PCS_INT_BER_MONITOR_DONE)
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> XEPON_PCS_INT_BER_MONITOR_DONE " );
		if(phyIntStatus & XEPON_PCS_INT_BER_HIGH_FLAG)
		{
			PON_PHY_PRINT(PHY_MSG_INT," with XEPON_PCS_INT_BER_HIGH_FLAG " );		
		}	
	}
	
	else
	{
		phy_other_cnt++;
		phyIntEn=IO_GPHYREG(XEPON_PCS_INT_EN);
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected! fw status=0x%x,INT EN=0x%08x",gpPhyPriv->phy_status,phyIntEn);
	}
#endif
#if ASIC_SERDES
	if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_NOT_LASER_RX_LOSS))
	{		
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> No Laser Loss Reset_PMA_start trans_laser_detected=%d\r\n", trans_laser_detected);
		read_data = IO_GPHYREG(XEPON_PCS_SFP_STATUS);
		if((read_data& XEPON_PCS_SFP_STATUS_RX_LOSS) != XEPON_PCS_SFP_STATUS_RX_LOSS)
		{
			laser_no_los_cnt++;
			trans_laser_detected = TRUE;
			phy_pma_reset();
			phy_print_time();
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> No Laser Loss Reset_PMA_done trans_laser_detected=%d\r\n", trans_laser_detected);
		}
		else
		{
			phy_other_cnt++;
			trans_laser_detected = FALSE;
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_DISABLE);
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Fake No Laser Loss Reset_PMA_done trans_laser_detected=%d\r\n", trans_laser_detected);
		}		
	}
		
	else if(trans_laser_detected == TRUE)
	{		
		if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_SYNC_OK)) 
		{
			read_data = IO_GPHYREG(XEPON_PCS_RX_SYNC_STATUS);
			if((read_data& XEPON_PCS_RX_SYNC_STATUS_OK) == XEPON_PCS_RX_SYNC_STATUS_OK)
			{
				gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
				phy_rdy_cnt++;			
				trans_laser_detected = FALSE;			
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY ");
				handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
			}
			else{
				phy_other_cnt++;
				phyIntEn=IO_GPHYREG(XEPON_PCS_INT_EN);
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX receive signal yet sync loss! fw status=0x%x,INT EN=0x%08x",gpPhyPriv->phy_status,phyIntEn);
			#if 0	//for debug need
                read_data=IO_GPHYREG(XPON_PMA_ADD_RO_RX2ANA_3);
				printk("RX_FIFO 0xbfaf376c = 0x%x\n",read_data);
				read_data=IO_GPHYREG(XPON_PMA_XPON_INT_STA_2);
				printk("PMA_INT_STA 0xbfaf37c8 = 0x%x\n",read_data);
            #endif
			//	phy_pma_reset();
			}
		}
		else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_LASER_RX_LOSS))
		{		
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_DISABLE);
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			phy_los_cnt++;
			trans_laser_detected = FALSE;	
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> LASER_RX_LOSS " );
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );				
		}	
	}
	
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_LASER_RX_LOSS)&&(phyIntStatus & XEPON_PCS_INT_NOT_LASER_RX_LOSS))
	{

		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;	
		phy_los_cnt++;
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT) ;

		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> LOS and NO_LOS int assert at same time. Reset_PMA_start \r\n");
		
		
		read_data = IO_GPHYREG(XEPON_PCS_SFP_STATUS);
		if((read_data& XEPON_PCS_SFP_STATUS_RX_LOSS) != XEPON_PCS_SFP_STATUS_RX_LOSS)
		{
			laser_no_los_cnt++;
			trans_laser_detected = TRUE;
			phy_pma_reset();
			phy_print_time();
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Reset_PMA_done \r\n");
		}
		else
		{
			phy_other_cnt++;
			trans_laser_detected = FALSE;
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_DISABLE);
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Fake No Laser Loss Reset_PMA_done trans_laser_detected=%d\r\n", trans_laser_detected);
		}

	}


	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_LASER_RX_LOSS))
	{

		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;	
		phy_los_cnt++;
		IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_DISABLE);
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS " );
		PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT) ;
		fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
	}
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_SYNC_LOSS))
	{	

		gpPhyPriv->xe_syncloss_cnt++;

		phy_lof_cnt++ ;

		if(gpPhyPriv->xe_syncloss_cnt <= 3)
		{
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_LOF, ");
			handle_hw_irq_event(PHY_EVENT_PHY_LOF_INT) ;	
			gpPhyPriv->event_poll_timer_value = 10;
		}
		else //if sync loss int occur 3 times in a row, signal quality issue,  make it loss
		{
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;	
			phy_los_cnt++;
			// should not disable rx before Laser LOS
			//IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_DISABLE);
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS " );
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT) ;
			//fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
		}

/*
// for debug, ang_20200819

			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_LOF. \r\n");
			handle_hw_irq_event(PHY_EVENT_PHY_LOF_INT) ;

			if(phyIntStatus & XEPON_PCS_INT_SYNC_OK) 	// LOF and SYNC_OK assert at same time.
			{
				read_data = IO_GPHYREG(XEPON_PCS_RX_SYNC_STATUS);
				if((read_data& XEPON_PCS_RX_SYNC_STATUS_OK) == XEPON_PCS_RX_SYNC_STATUS_OK)
				{
					gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
					phy_rdy_cnt++;			
					trans_laser_detected = FALSE;			
					PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY from LOF ");
					handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
				}
			}
*/
		
	}
	
	else if(((gpPhyPriv->phy_status == PHY_LINK_STATUS_UNKNOWN)||(gpPhyPriv->phy_status == PHY_LINK_STATUS_READY))&&(phyIntStatus & XEPON_PCS_INT_SYNC_OK))
	{
		read_data = IO_GPHYREG(XEPON_PCS_SFP_STATUS);
		if((read_data& XEPON_PCS_SFP_STATUS_RX_LOSS) != XEPON_PCS_SFP_STATUS_RX_LOSS)
		{
			read_data = IO_GPHYREG(XEPON_PCS_RX_SYNC_STATUS);
			if((read_data& XEPON_PCS_RX_SYNC_STATUS_OK) == XEPON_PCS_RX_SYNC_STATUS_OK)
			{
				gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
				phy_rdy_cnt++;			
				trans_laser_detected = FALSE;			
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY ");
				handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
			}
		}
		else
		{
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_DISABLE);
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			trans_laser_detected = FALSE;
			phy_other_cnt++;
			phyIntEn=IO_GPHYREG(XEPON_PCS_INT_EN);
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Please Plug In Fiber! fw status=0x%x,INT EN=0x%08x",gpPhyPriv->phy_status,phyIntEn);
		}
	}
	
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_TX_GB_OVERRUN))
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> TX_GB_OVERRUN " );
	}
	
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & XEPON_PCS_INT_TX_GB_UNDERRUN))
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> TX_GB_UNDERRUN " );
	}
	else if(phyIntStatus & XEPON_PCS_INT_BER_MONITOR_DONE)
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> XEPON_PCS_INT_BER_MONITOR_DONE " );
		if(phyIntStatus & XEPON_PCS_INT_BER_HIGH_FLAG)
		{
			PON_PHY_PRINT(PHY_MSG_INT," with XEPON_PCS_INT_BER_HIGH_FLAG " );		
		}	
	}

	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_LOS)&&(phyIntStatus & XEPON_PCS_INT_LASER_RX_LOSS))
	{
		laser_los_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> LASER_LOS_INT <DO NOTHING>" );						
	}
		
	else{
		phy_other_cnt++;
		phyIntEn=IO_GPHYREG(XEPON_PCS_INT_EN);
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected! fw status=0x%x,INT EN=0x%08x",gpPhyPriv->phy_status,phyIntEn);
		gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN; //ang_20200416
	}
#endif

	PON_PHY_PRINT(PHY_MSG_INT," XEPON ISR=%d=%d+%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,laser_no_los_cnt,phy_other_cnt,laser_los_cnt,phy_lof_cnt,phyIntStatus);

	#ifndef PON_PHY_PERIODIC_POLLING
	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
	#endif

	spin_unlock(&gpPhyPriv->event_handle_lock); 

	return PHY_SUCCESS;
}


int en7580_xepon_phy_event_poll(char* buf)
{
	ulong flags;
	PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_SW_POLL};
	int phyReadyStatus,phyLosStatus;
	static UINT32 poll_set_los_cnt=0,poll_set_ready_cnt=0,poll_no_ready_no_los_cnt=0,poll_los_cnt=0,poll_ready_cnt=0;
	static PHY_Los_Status_t  pre_phy_status=PHY_LINK_STATUS_UNKNOWN;
	
	if(FALSE == gpPhyPriv->is_phy_start){
		return	PHY_FAILURE;	//ang_20180208
	}
	//phy_print_time();

	gpPhyPriv->event_poll_timer.data = 1500;

	spin_lock_irqsave(&gpPhyPriv->event_handle_lock, flags);

	phyReadyStatus=phy_ready_status();
	phyLosStatus=phy_los_status();

#if ASIC_SERDES

	gpPhyPriv->xe_syncloss_cnt=0;	

	if((PHY_LINK_STATUS_LOS==gpPhyPriv->phy_status) && (phyLosStatus))
		{
		PON_PHY_PRINT(PHY_MSG_ERR," - POLLING => LOS (%d) phyReadyStatus=%d,phyLosStatus=%d\r\n",poll_los_cnt,phyReadyStatus,phyLosStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
		phy_event.id = PHY_EVENT_TRANS_LOS_INT;
		phy_event_handler(&phy_event);
		poll_los_cnt++;
	}
			
	else if((PHY_LINK_STATUS_READY==gpPhyPriv->phy_status) && (phyReadyStatus &&(!phyLosStatus)))
	{
		poll_ready_cnt++;
		poll_no_ready_no_los_cnt=0;
		PON_PHY_PRINT(PHY_MSG_ERR," - POLLING => READY (%d) phyReadyStatus=%d,phyLosStatus=%d\r\n",poll_ready_cnt,phyReadyStatus,phyLosStatus);

		gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
		phy_event.id = PHY_EVENT_PHYRDY_INT;
		phy_event_handler(&phy_event); 
	 }
	else if((PHY_LINK_STATUS_UNKNOWN == gpPhyPriv->phy_status)&&(phyReadyStatus && (!phyLosStatus))) //ang_20200416
	{

		fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
		// PCS_RX_Disable
		IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_DISABLE);
		mdelay(10);
		
		phy_pma_reset();
		mdelay(10);

		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> reset, recover from fake sync. \r\n");
	}
	else if((PHY_LINK_STATUS_UNKNOWN == gpPhyPriv->phy_status)&&((!phyReadyStatus) && phyLosStatus))//ang_20200416
	{
		PON_PHY_PRINT(PHY_MSG_ERR," - POLLING => LOS(%d)No Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);  
		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS; 
		phy_event.id = PHY_EVENT_TRANS_LOS_INT;
		phy_event_handler(&phy_event);
		poll_los_cnt++;
	}
	else	
	{
		if(!phyLosStatus)
		{
			poll_no_ready_no_los_cnt++;
			#if XILINX_SERDES
			IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_ON);
			IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_OFF);
			PON_PHY_PRINT(PHY_MSG_ERR," - POLLING => no_ready_no_los(%d) PHY_RST_N_SERDES,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			#else

			if(poll_no_ready_no_los_cnt%10 ==0)
			{
				PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)SCU Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
				PHY_INTERNAL_SET_API(PON_SET_PHY_SCU_RST,NULL);
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)Reset pma,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);// ang_20190307			//disable rx			
				phy_pma_reset();	
			}	

			gpPhyPriv->event_poll_timer_value = 3500;
			PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4

			#endif
			#ifdef LDDLA_SUPPORT_SET_TX_MODE
			phy_event.id = PHY_EVENT_NO_LOS_NO_READY;
			phy_event_handler(&phy_event);
			#endif
		}
		else
		{
			PON_PHY_PRINT(PHY_MSG_INT," - POLLING => LOS(%d)No Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);  
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS; 
			phy_event.id = PHY_EVENT_TRANS_LOS_INT;
			phy_event_handler(&phy_event);
			poll_los_cnt++;
		}
	}

#endif 
#if A60972_SERDES

	if(PHY_LINK_STATUS_READY == gpPhyPriv->phy_status) 
	{
		if(!(phyReadyStatus&&(!phyLosStatus)))
		{
			poll_set_los_cnt++;
			#if XILINX_SERDES
			IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_ON);
			IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_OFF);
			PON_PHY_PRINT(PHY_MSG_ERR," - POLLING => LOS(%d)PHY_RST_N_SERDES,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			#else
			PON_PHY_PRINT(PHY_MSG_ERR," - POLLING => LOS(%d)No Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			#endif
			//should not handle event here, since LOSS in not detected here, it is just double check on whether ISR result is error, nomarlly code will not enter this route
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			phy_event.id = PHY_EVENT_TRANS_LOS_INT;
			phy_event_handler(&phy_event);
		}
	}
	else
	{	 
		if((!phyReadyStatus)&&(!phyLosStatus))
		{
			poll_no_ready_no_los_cnt++;
			#if XILINX_SERDES
			IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_ON);
			IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_OFF);
			PON_PHY_PRINT(PHY_MSG_ERR," - POLLING => no_ready_no_los(%d) PHY_RST_N_SERDES,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			#else
			PON_PHY_PRINT(PHY_MSG_ERR," - POLLING => no_ready_no_los(%d) No Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			phy_pma_reset();
			//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
			PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
			#endif
		}

		if(phyReadyStatus&&(!phyLosStatus))
		{
			poll_set_ready_cnt++;
			PON_PHY_PRINT(PHY_MSG_ERR," - POLLING => READY(%d) phyReadyStatus=%d,phyLosStatus=%d\r\n",poll_set_ready_cnt,phyReadyStatus,phyLosStatus);
			
			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
			phy_event.id = PHY_EVENT_PHYRDY_INT;
			phy_event_handler(&phy_event);
		}
		if(phyLosStatus)
		{
			PON_PHY_PRINT(PHY_MSG_INT," - POLLING => LOS(%d)No Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);  
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS; 
			phy_event.id = PHY_EVENT_TRANS_LOS_INT;
			phy_event_handler(&phy_event);
			poll_los_cnt++;
		}

	}

#endif

	spin_unlock_irqrestore(&gpPhyPriv->event_handle_lock, flags);

#ifdef PON_PHY_PERIODIC_POLLING
	if(pre_phy_status!=gpPhyPriv->phy_status)
	{
		PON_PHY_PRINT(PHY_MSG_INT," - POLLING - (%d)(%d)(%d) phy_status = %d\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status);
	}
	pre_phy_status=gpPhyPriv->phy_status;
	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
#else
	PON_PHY_PRINT(PHY_MSG_INT," - POLLING - set(%d)(%d)(%d) query phy_status = %d (%d)(%d)\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status,poll_los_cnt,poll_ready_cnt);
#endif

	return PHY_SUCCESS;

}



int en7580_xepon_phy_dbg(char* buf)
{
	char dbg_id = *buf;
	char dbg_option1 = *(buf+1);
	char dbg_option2 = *(buf+2);
	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 i=0,phy_reg_total_num=0;
	UINT32 frame_count_high=0,frame_count_low=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;

	printk("[%s:%d] id = %d\n",__FUNCTION__,__LINE__,dbg_id);
	//printk("gpPhyPriv->phyCfg.flags.mode = %d\n",gpPhyPriv->phyCfg.flags.mode);
	
#ifdef	EN7580_HEC_TEST

	phy_reg_total_num=172;
	phy_reg_all=xepon_phy_reg_all;

	switch(dbg_id)
	{
		case 0:
			printk("\r\n\r\n___________Read XEPON debug counter only(no reset)___________\n");
			switch(gpPhyPriv->wan_sel)
			{
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_1G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					for(i=40;i<=46;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					//Tx framer cnt
					read_data = IO_GPHYREG(XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_TX_FRAME_COUNTER",XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
					
					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_10G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					for(i=40;i<=46;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					for(i=23;i<=26;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					
					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
					/*	XEPON 1G mode  */
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON HEC test 1 CNT 1G mode_____");
					//Rx framer cnt
					IO_SPHYREG(XEPON_1G_XP_ERRCNT_CTL, PHY_RXFRAME_LATCH);//latch 
					frame_count_high = IO_GPHYREG(XEPON_1G_FRAME_CNT_H);
					frame_count_low = IO_GPHYREG(XEPON_1G_FRAME_CNT_L);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_FRAME_CNT_H",XEPON_1G_FRAME_CNT_H,frame_count_high,frame_count_high);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_FRAME_CNT_L",XEPON_1G_FRAME_CNT_L,frame_count_low,frame_count_low);
					
					//Tx framer cnt
					read_data = IO_GPHYREG(XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_TX_FRAME_COUNTER",XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
					

					break;

				default:
					printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

					return PHY_FAILURE;
			}
			read_data = IO_GPHYREG(0xbfb66120); //  msb 8 bit  = MAC error
			printk("#MAC: Error CNT 0x%8x=0x%8x MSB 8 bit=0x%2x\r\n",0xbfb66120,read_data,read_data>>24);
			read_data = IO_GPHYREG(0xbfb66360); //  MAC error
			printk("#MAC: Error CNT 0x%8x=0x%8x\r\n",0xbfb66360,read_data);

			break;
			
		case 2:
			/*	XEPON 10G mode	*/
			IO_SPHYREG(XEPON_PCS_CLR_ALL_NUM, XEPON_PCS_CLR_ALL_NUM_CLR);//latch 
			
		
			switch(gpPhyPriv->wan_sel)
			{
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_1G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					for(i=143;i<=148;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					//Tx framer cnt
					read_data = IO_GPHYREG(XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_TX_FRAME_COUNTER",XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
				
					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_10G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					//for(i=143;i<=148;i++)
					for(i=40;i<=46;i++)//cheng_20220314  wrong register index
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					for(i=23;i<=26;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
		
					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
					/*	XEPON 1G mode  */
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON HEC test 1 CNT 1G mode_____");
					//Rx framer cnt
					IO_SPHYREG(XEPON_1G_XP_ERRCNT_CTL, PHY_RXFRAME_LATCH);//latch 
					frame_count_high = IO_GPHYREG(XEPON_1G_FRAME_CNT_H);
					frame_count_low = IO_GPHYREG(XEPON_1G_FRAME_CNT_L);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_FRAME_CNT_H",XEPON_1G_FRAME_CNT_H,frame_count_high,frame_count_high);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_FRAME_CNT_L",XEPON_1G_FRAME_CNT_L,frame_count_low,frame_count_low);
					
					//Tx framer cnt
					read_data = IO_GPHYREG(XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_TX_FRAME_COUNTER",XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
					
		
					break;
		
				default:
					printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);
		
					return PHY_FAILURE;
			}
			
			break;

		case 3:
			printk("XEPON Tx gear box test : Threshold = %d\r\n",dbg_option1);
			/* Disable ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);

			/* Set Tx gear box threshold */
			read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
			printk("Read XEPON_PCS_TX_CTRL_CFG=0x%8x\n",read_data);
			printk("     TX_GB_THRESHOLD      =0x%8x\n",dbg_option1);
			IO_SPHYREG(XEPON_PCS_TX_CTRL_CFG,((read_data&XEPON_PCS_TX_CTRL_CFG_TX_GB_THR_MASK)|dbg_option1)) ;
			read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
			printk("Set  XEPON_PCS_TX_CTRL_CFG=0x%8x\n",read_data);
				
			/* Reset PCS logic to clear Tx gear box */
			IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_ON);
			IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_OFF);

			/* Clear ISR status*/
			read_data=IO_GPHYREG(XEPON_PCS_INT_STATUS);
			IO_SPHYREG(XEPON_PCS_INT_STATUS, read_data);
			printk("Read & Clear XEPON_PCS_INT_STATUS=0x%08x\n",read_data);
			
			/* Enable ISR */
			phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);

			break;
			
		case 4:
			read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
			printk("Read XEPON_PCS_TX_CTRL_CFG=0x%8x\n",read_data);

			read_data=IO_GPHYREG(XEPON_PCS_INT_STATUS);
			IO_SPHYREG(XEPON_PCS_INT_STATUS, read_data);
			printk("Before Clear XEPON_PCS_INT_STATUS=0x%08x\n",read_data);

			if(read_data&XEPON_PCS_INT_TX_DET_OVERRUN)
			{
				printk("21:TX_DET_OVERRUN\n");
			}
			if(read_data&XEPON_PCS_INT_TX_DET_UNDERRUN)
			{
				printk("20: TX_DET_UNDERRUN\n");
			}
			if(read_data&XEPON_PCS_INT_TX_GB_OVERRUN)
			{
				printk("19:  TX_GB_OVERRUN\n");
			}
			if(read_data&XEPON_PCS_INT_TX_GB_UNDERRUN)
			{
				printk("18:   TX_GB_UNDERRUN\n");
			}
			if(read_data&XEPON_PCS_INT_RX_GB_OVERRUN)
			{
				printk("17:    RX_GB_OVERRUN\n");
			}
			if(read_data&XEPON_PCS_INT_RX_GB_EMPTY_APPEAR)
			{
				printk("16:     RX_GB_EMPTY_APPEAR\n");
			}

			read_data=IO_GPHYREG(XEPON_PCS_INT_STATUS);
			printk("After  Clear XEPON_PCS_INT_STATUS=0x%08x\n",read_data);
			
			break;

		case 5:

			read_data=IO_GPHYREG(XEPON_PCS_SFP_STATUS);
			printk("Read XEPON_PCS_SFP_STATUS=0x%8x\n",read_data);
			printk("     _TX_FAULT           =0x%8x\n",read_data&XEPON_PCS_SFP_STATUS_TX_FAULT);
			printk("	 _RX_LOSS            =0x%8x\n",read_data&XEPON_PCS_SFP_STATUS_RX_LOSS);
			printk("	 _TX_SD              =0x%8x\n",read_data&XEPON_PCS_SFP_STATUS_TX_SD);
			printk("	 _MOD_ABS            =0x%8x\n",read_data&XEPON_PCS_SFP_STATUS_MOD_ABS);
			printk("	 _P_DOWN             =0x%8x\n",read_data&XEPON_PCS_SFP_STATUS_P_DOWN);

			break;	
			
#if A60972_SERDES //EN7580_ASIC

		case 8:

			PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] before script\r\n",__FUNCTION__,__LINE__);
			
			IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_ON);
			printk("EN7580 << XEPON >> logic reset hold\r\n");
			
			//reset A60972
			printk("reset A60972\r\n");
			IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_ON);
			mdelay(1);
			IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_OFF);
			break;	


		case 9:
			
			PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] after script\r\n",__FUNCTION__,__LINE__);
#if 1
			read_data = EN7580_IF_RG_A60972_CTRL_IF_SW_RESET_ONLY;
			IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, read_data);
			mdelay(1);
			read_data = EN7580_IF_RG_A60972_CTRL_NORMAL_OPERATION;
			IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, read_data);
			printk("EN7580_IF_RG_A60972_CTRL_IF_SW_RESET_ONLY\r\n");
#endif
			printk("EN7580 << XEPON >> logic reset release\r\n");
			IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_OFF);
			break;	
#endif

		case 10:
			
			printk("\r\n\r\n___________XEPON BER Monitor & Counter___________\n");

			read_data = XEPON_PCS_RX_BER_START_MONITOR;
			IO_SPHYREG(XEPON_PCS_BER_START_MONITOR,read_data);
			mdelay(10);

			(phy_reg_all+15)->cur=IO_GPHYREG((phy_reg_all+15)->addr);
			printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+15)->name,(phy_reg_all+15)->addr,(phy_reg_all+15)->cur,(phy_reg_all+15)->cur);
			
			for(i=17;i<=19;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			break;
			
		case 41:
			switch(gpPhyPriv->wan_sel)
			{
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_1G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					for(i=143;i<=148;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					//Tx framer cnt
					read_data = IO_GPHYREG(XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_TX_FRAME_COUNTER",XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
				
					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_10G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					//for(i=143;i<=148;i++)
					for(i=40;i<=46;i++)//cheng_20220314  wrong register index
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					for(i=23;i<=26;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}

					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
					/*	XEPON 1G mode  */
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON HEC test 1 CNT 1G mode_____");
					//Rx framer cnt
					IO_SPHYREG(XEPON_1G_XP_ERRCNT_CTL, PHY_RXFRAME_LATCH);//latch 
					frame_count_high = IO_GPHYREG(XEPON_1G_FRAME_CNT_H);
					frame_count_low = IO_GPHYREG(XEPON_1G_FRAME_CNT_L);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_FRAME_CNT_H",XEPON_1G_FRAME_CNT_H,frame_count_high,frame_count_high);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_FRAME_CNT_L",XEPON_1G_FRAME_CNT_L,frame_count_low,frame_count_low);
					
					//Tx framer cnt
					read_data = IO_GPHYREG(XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","XEPON_1G_TX_FRAME_COUNTER",XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
					

					break;

				default:
					printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

					return PHY_FAILURE;
			}
			read_data = IO_GPHYREG(0xbfb66120); //  msb 8 bit  = MAC error
			printk("#MAC: Error CNT 0x%8x=0x%8x MSB 8 bit=0x%2x\r\n",0xbfb66120,read_data,read_data>>24);
			read_data = IO_GPHYREG(0xbfb66360); //  MAC error
			printk("#MAC: Error CNT 0x%8x=0x%8x\r\n",0xbfb66360,read_data);

			break;
		


		case 51:
			printk("\r\n___ PCS_LOOP_BACK ___ Increase ___\r\n");

			/*	XEPON PCS loop back write	*/
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			
			write_data=0x1c48;
			IO_SPHYREG(XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
			printk("Write XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_CTRL_CFG,read_data);

			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			read_data=IO_GPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG);
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,read_data);

			write_data=0x10302;
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
			printk("Write XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_RX_CTRL_CFG,read_data);

			
			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	
			



		case 52:
			printk("\r\n___ PCS_LOOP_BACK ___ Increase check ___\r\n");
			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);

			
			if(XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE==(read_data&XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE))
			{
				for(i=85;i<=136;i++)
				{
					(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
					printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
				}				
			}
			else
			{
				printk("XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE Fail !\n");
			}
			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);

			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;

			
		case 53:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ZEROS ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			
			write_data=0x1c48;
			IO_SPHYREG(XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
			printk("Write XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);
			}

			write_data=0x10302;
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
			printk("Write XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_RX_CTRL_CFG,read_data);
			
			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	
	
		case 54:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ZEROS check ___\r\n");
			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);

			
			if(XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE==(read_data&XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE))
			{
				for(i=31;i<=84;i++)
				{
					(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
					printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				}
				for(i=85;i<=136;i++)
				{
					(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
					printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
					if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
					{
						printk("=Fail");
					}
					else
					{
						printk("=Pass");
					}
				}				
			}
			else
			{
				printk("XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE Fail !\n");
			}
			
			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;	

		case 55:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);

			write_data=0x1c48;
			IO_SPHYREG(XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
			printk("Write XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);

			}
			
			write_data=0x10302;
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
			printk("Write XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_RX_CTRL_CFG,read_data);

			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	

		
		case 56:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES check ___\r\n");

			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);

			
			if(XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE==(read_data&XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE))
			{
				for(i=31;i<=84;i++)
				{
					(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
					printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				}
				for(i=85;i<=136;i++)
				{
					(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
					printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
					if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
					{
						printk("=Fail");
					}
					else
					{
						printk("=Pass");
					}
				}				
			}
			else
			{
				printk("XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE Fail !\n");
			}
			
			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;

		case 57:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ZEROS ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);

			write_data=0x1c48;
			IO_SPHYREG(XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
			printk("Write XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);

			}
			
			write_data=0x10302;
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
			printk("Write XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_RX_CTRL_CFG,read_data);

			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	

		
		case 58:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES check ___\r\n");

			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);

			
			if(XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE==(read_data&XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE))
			{
				printk("XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE\n");
			}
			else
			{
				printk("XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE Fail !\n");
			}
			
			for(i=31;i<=84;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=85;i<=136;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
				{
					printk("=Fail");
				}
				else
				{
					printk("=Pass");
				}
			}

			
			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;

		case 59:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);

			write_data=0x1c48;
			IO_SPHYREG(XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
			printk("Write XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);

			}
			
			write_data=0x10302;
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
			printk("Write XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_RX_CTRL_CFG,read_data);

			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	

		
		case 60:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES check ___\r\n");

			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);

			
			if(XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE==(read_data&XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE))
			{
				printk("XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE\n");
			}
			else
			{
				printk("XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE Fail !\n");
			}
			
			for(i=31;i<=84;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=85;i<=136;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
				{
					printk("=Fail");
				}
				else
				{
					printk("=Pass");
				}
			}

			
			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;


		case 61:
			printk("\r\n___ PCS TX (NOT Internal LPBK) RX LPBK mode ___ Increase ___\r\n");
		
			/*	XEPON PCS loop back write	*/
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			
			write_data=0x1c48;
			IO_SPHYREG(XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
			printk("Write XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_CTRL_CFG,read_data);
		
			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			read_data=IO_GPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG);
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
		
			write_data=0x10302;
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
			printk("Write XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_RX_CTRL_CFG,read_data);
		
			
			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
		
			break;


		case 62:
			printk("\r\n___ PCS_LOOP_BACK ___ Increase check ___\r\n");
			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);

			for(i=85;i<=136;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}				

			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);

			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;

			
		case 63:
			printk("\r\n___ PCS TX (NO Internal LPBK) RX LPBK mode ___ ALL ZEROS ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			
			write_data=0x1c48;
			IO_SPHYREG(XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
			printk("Write XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);
			}

			write_data=0x10302;
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
			printk("Write XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_RX_CTRL_CFG,read_data);
			
			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	
		
		case 64:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ZEROS check ___\r\n");
			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);

			for(i=31;i<=84;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=85;i<=136;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
				{
					printk("=Fail");
				}
				else
				{
					printk("=Pass");
				}
			}				

			
			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;

		case 65:
			printk("\r\n___ PCS TX (NOT Internal LPBK) RX LPBK mode ___ ALL ONES ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);

			write_data=0x1c48;
			IO_SPHYREG(XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_TX_CTRL_CFG);
			printk("Write XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);

			}
			
			write_data=0x10302;
			IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(XEPON_PCS_RX_CTRL_CFG);
			printk("Write XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_RX_CTRL_CFG,read_data);

			IO_SPHYREG(XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",XEPON_PCS_TX_TEST_CTRL_CFG,(XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	

		
		case 66:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES check ___\r\n");

			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);

			for(i=31;i<=84;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=85;i<=136;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
				{
					printk("=Fail");
				}
				else
				{
					printk("=Pass");
				}
			}				
			
			read_data=IO_GPHYREG(XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;






		default:
			printk("[%s:%d] UNKNOWN ID\n",__FUNCTION__,__LINE__);

			return PHY_FAILURE;
	}
#endif
	return PHY_SUCCESS;

}


int en7580_xepon_phy_pma_reset(char* buf)
{
#if A60972_SERDES	// for FPGA
	uint read_data = 0;

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d]\r\n",__FUNCTION__,__LINE__);

	IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_ON);
	printk("EN7580 << XEPON >> logic reset hold\r\n");

//	IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_DISABLE);
//	printk("EN7580 << XEPON >> PCS DISABLE \r\n");

	//reset A60972
	printk("reset A60972\r\n");
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_ON);
	mdelay(1);
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_OFF);

	//init A60972
	a60972_init();

	read_data = EN7580_IF_RG_A60972_CTRL_IF_SW_RESET_ONLY;
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, read_data);
	mdelay(1);
	read_data = EN7580_IF_RG_A60972_CTRL_NORMAL_OPERATION;
	IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, read_data);
	printk("EN7580_IF_RG_A60972_CTRL_IF_SW_RESET_ONLY\r\n");

	printk("EN7580 << XEPON >> logic reset release\r\n");
	IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_OFF);

//	IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_ENABLE);
//	printk("EN7580 << XEPON >> PCS ENABLE \r\n");
#endif	

#if ASIC_SERDES	// for ASIC
	IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_DISABLE);
	IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_ON);
	phy_delay1ms(1);

	if(gpPhyPriv->first_plugin_flag == TRUE)
	{
		fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
		gpPhyPriv->first_plugin_flag = FALSE;
		gpPhyPriv->pma_init_done= TRUE;
	}
	else
	{
		fiber_plug_reset(PLUG_IN,gpPhyPriv->wan_sel);
	}
	PON_PHY_PRINT(PHY_MSG_DBG,"XEPON PMA Reset! \n");
	phy_delay1ms(1);

	IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_OFF);
	phy_delay1ms(1);

	IO_SPHYREG(XEPON_PCS_RX_CTRL_CFG, XEPON_PCS_RX_ENABLE);
	PON_PHY_PRINT(PHY_MSG_DBG,"<RX_ENABLE ON>!\n");
#endif
	
	return PHY_SUCCESS;
}


#define _I_AM_PMA_SPLIT_LINE_
#if ASIC_SERDES
int xpon_pma_param_init(void)
{
	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] wan_sel = %d\r\n",__FUNCTION__,__LINE__,gpPhyPriv->wan_sel);

	xpon_pma_param.TX_Bist_Pattern=PATTERN_DISABLE;
	xpon_pma_param.LoopBack_Path=normal_mode;
	xpon_pma_param.Trans_Ben_Active_Level=gpPhyPriv->trans_tx_ben_level;

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:			
			xpon_pma_param.LCPLL_Mode=lcpll_1p25G_1p25G;
			xpon_pma_param.FM_Mode=fm_1p25G_1p25G;
			xpon_pma_param.RX_Mode=Rx_Rate_1p25G;
			xpon_pma_param.TX_Mode=Tx_Rate_1p25G_RX_1p25G;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			xpon_pma_param.LCPLL_Mode=lcpll_2p48832G_1p24416G;
			xpon_pma_param.FM_Mode=fm_2p48832G_1p24416G;
			xpon_pma_param.RX_Mode=Rx_Rate_2p48832G;
			xpon_pma_param.TX_Mode=Tx_Rate_1p24416G;
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G:
			xpon_pma_param.LCPLL_Mode=lcpll_9p95328G_2p48832G;
			xpon_pma_param.FM_Mode=fm_9p95328G_2p48832G;
			xpon_pma_param.RX_Mode=RX_Rate_9p95328G;
			xpon_pma_param.TX_Mode=Tx_Rate_2p48832G;
			break;			

		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			xpon_pma_param.LCPLL_Mode=lcpll_10p3125G_1p25G;
			xpon_pma_param.FM_Mode=fm_10p3125G_1p25G;
			xpon_pma_param.RX_Mode=RX_Rate_10p3125G;
			xpon_pma_param.TX_Mode=Tx_Rate_1p25G_RX_10p3125G;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			xpon_pma_param.LCPLL_Mode=lcpll_10p3125G_10p3125G;
			xpon_pma_param.FM_Mode=fm_10p3125G_10p3125G;
			xpon_pma_param.RX_Mode=RX_Rate_10p3125G;
			xpon_pma_param.TX_Mode=TX_Rate_10p3125G;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
			xpon_pma_param.LCPLL_Mode=lcpll_9p95328G_9p95328G;
			xpon_pma_param.FM_Mode=fm_9p95328G_9p95328G;
			xpon_pma_param.RX_Mode=RX_Rate_9p95328G;
			xpon_pma_param.TX_Mode=TX_Rate_9p95328G; 
			break;

		default:
			printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

			return PHY_FAILURE;
	}
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x  0x%8x  *=0x%8x  **=0x%8x	pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);
	return 0;
}
#endif

#if ASIC_SERDES
int xpon_pma_param_disp(void)
{
	printk("%s (wan_sel = %d)\r\n",__FUNCTION__,gpPhyPriv->wan_sel);
	printk("%% LCPLL_Mode = %d\r\n",xpon_pma_param.LCPLL_Mode);
	printk("%% FM_Mode = %d\r\n",xpon_pma_param.FM_Mode);
	printk("%% RX_Mod = %d\r\n",xpon_pma_param.RX_Mode);
	printk("%% TX_Mode = %d\r\n",xpon_pma_param.TX_Mode);
	printk("%% TX_Bist_Pattern = %d\r\n",xpon_pma_param.TX_Bist_Pattern);
	printk("%% LoopBack_Path = %d\r\n",xpon_pma_param.LoopBack_Path);
	printk("%% Trans_Ben_Active_Level = %d\r\n",xpon_pma_param.Trans_Ben_Active_Level);
	return 0;
}
#endif

#if ASIC_SERDES
int xpon_pma_init(void)
{
	PON_PHY_PRINT(PHY_MSG_TRACE,"%s\r\n",__FUNCTION__);

	//param init based on wan_sel
	xpon_pma_param_init();
	xpon_pma_param_disp();
	xpon_pma_mode_init();
	xpon_pma_param_opt();

	return PHY_SUCCESS;
}
#endif

#define _I_AM_BIST_CHECK_SPLIT_LINE_   

#if ASIC_SERDES

//****************************************************************************************************
//RX Bist Align Control
//PRBS7:  6081 df53
//PRBS9:  d97b 0d53
//PRBS15: e1fd 7d53
//PRBS23: 5cf1 fd53
//PRBS31: 8ff1 fd53
//PRBS11: a190_3d53
//****************************************************************************************************
int xpon_bist_check_pattern_sel(XPON_PMA_RX_Bist_Check_t align_pattern)
{
	printk("[%s:%d] pattern %d\n",__FUNCTION__,__LINE__,align_pattern);

	switch(align_pattern)
	{
		case PRBS7_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,		 4,	 0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,	 31,	0,	0x6081DF53);//rg_px_lnx_bist_ctl_align_pattern	
			break;

		case PRBS9_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,			4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,		31,	0,	0xD97B0D53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS15_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,			4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,		31,	0,	0xE1FD7D53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS23_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,			4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,		31,	0,	0x5CF1FD53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS31_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,			4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,		31,	0,	0x8FF1FD53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS11_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,			4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT, 	   31, 0,	0xA1903D53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		default:
			break;
				
	}

	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED, 	31, 0,	0x0FF1FD53);//bistctl_prbs_init_seed	
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,  15, 0,	0x01);//rg_bistctl_prbs_fail_threshold

	return 0;
}

//****************************************************************************************************
//RX Bist Align check
//****************************************************************************************************
void xpon_rx_bist_recheck_result(uint check_onoff, uint duration)
{
	int i;
	int prbs_check_duration;
	prbs_check_duration = duration * 1000;
    printk("prbs_check_duration = %d \n", prbs_check_duration);
	if(check_onoff == PHY_ENABLE)
	{
		bist_check_err_cnt = 0;
		bist_err_cnt_clear_times = 0;

		//clear bit_error
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 24, 24, 0x1);//bit_error_rst_sel
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 24, 24, 0x0);

		phy_print_time();
		printk("PRBS Error Counter = %u \n", bist_check_err_cnt);
		bist_check_cnt_add(prbs_check_duration);
		phy_print_time();
		printk("PRBS Error Counter = %u \n", bist_check_err_cnt); //unsigned output
		PON_PHY_PRINT(PHY_MSG_TIME, "bist_err_cnt_clear_times = %d \n", bist_err_cnt_clear_times);			
	}
	else if(check_onoff == PHY_DISABLE)
	{
		bist_check_err_cnt = 0;
		bist_err_cnt_clear_times = 0;
		//reset rx_bist_check error counter in case it overflow
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 24, 24, 0x1);//bit_error_rst_sel
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 24, 24, 0x0);
	}
}

void normal_rx_bist_check(uint BIST_check_switch, XPON_PMA_RX_Bist_Check_t align_pattern)
{
	uint prbs_result = 0;
	uint prbs_err_cnt = 0;
	int i;

	//phy_pma_reset();	 //ang_20180712

	if(BIST_check_switch == PHY_ENABLE)
	{
		xpon_bist_check_pattern_sel(align_pattern);

		 						// Create_Morris_(PON_CSR_PMA_BASE + 0x3684 )//
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 31, 0, 0x00000000); 	// BFAF3684 default = 0x01000000
								// IO_SPHYA_REG_BITS(SS_BIST_1, 0, 0, 0x00));  	        		//3684 , anlt_px_lnx_lt_los
								// IO_SPHYA_REG_BITS(SS_BIST_1, 8, 8, 0x01));  	        		//3684 , all_lane_prbs_tx_en
								// IO_SPHYA_REG_BITS(SS_BIST_1, 24, 24, 0x00));  	    		//3684 , rg_lnx_bistctl_bit_error_rst_sel
		// customer need both tx and rx work, marked by ang_20180713
		//IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 	8,	8,	0x0);	//BIST TX EN		check RX BIST only, no need TX sending PRBS	

		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,  16, 16, 0x0);//rg_bistctl_pat_rx_check_en
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x01);	//3500, rg_bistctl_pat_rx_check_en	 do rx BIST check
													
		for(i=0; i<16; i++){		
		prbs_result = IO_GPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,	16, 0);//ro_bistctl_prbs_compare&fail&done	  david 20170428
		printk("0x%8x = %8x\r\n",XPON_PMA_BISTCTL_PRBS_EVENT, prbs_result);
		if(prbs_result == 0x10001)	   //david 20170428
		{
			printk("Pattern Aligned!\n");		
				break;
		}
		else if(prbs_result == 0x10101)
		{
			printk("Pattern Aligned with BER error!\n");
				break;
		}
		else
		{
				printk("PRBS check Not Align yet!\n");
			}
			mdelay(500);
		}		
		if((prbs_result == 0x0)||(prbs_result == 0x100))
		{
			printk("PRBS check Failed! please check RX rates & selected pattern!\n");
		}
	}
	else if(BIST_check_switch == PHY_DISABLE)
	{
		#if ASIC_SERDES
		fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
		#endif
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,  16, 0, 0x0);//rg_bistctl_pat_rx_check_en
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 31, 0, 0x01000000); 	// BFAF3684 default = 0x01000000
		printk("RX BIST Check End!\n");	
	}
}	

void t2r_rx_bist_check(uint BIST_check_switch, XPON_PMA_RX_Bist_Check_t align_pattern)
{
	uint prbs_result = 0;
	uint prbs_err_cnt = 0;
	int i;

	if(BIST_check_switch == PHY_ENABLE)
	{
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x01);//rg_tx_bist_gen_en
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,  8,	8,	0x00);//rg_bistctl_pat_tx_en	
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,  2,	0,	0x00);//rg_xpon_mode
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,	 1,	1,	0x01);//t2r_bist_mode					   
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,	 0,	0,	0x00);//t2r_FIFO_en 		

		xpon_bist_check_pattern_sel(align_pattern);

		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,		8,	8,	0x01);//all_lane_prbs_tx_en
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,		0,	0,	0x00);//anlt_px_lnx_lt_los
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 24, 24, 0x0);//rg_bistctl_io_data_inv

		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,  8, 8, 0x1); //TX_BIST_EN		

		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,  16, 16, 0x0);//rg_bistctl_pat_rx_check_en
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x01); 	//3500, rg_bistctl_pat_rx_check_en	 do rx BIST check

		for(i=0; i<16; i++){		
		prbs_result = IO_GPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,	16, 0);//ro_bistctl_prbs_compare&fail&done	  david 20170428
		printk("0x%8x = %8x\r\n",XPON_PMA_BISTCTL_PRBS_EVENT, prbs_result);
		if(prbs_result == 0x10001)	   //david 20170428
		{
			printk("Pattern Aligned!\n");		
				break;
		}
		else if(prbs_result == 0x10101)
		{
			printk("Pattern Aligned with BER error!\n");
				break;
		}
		else
		{
				printk("PRBS check Not Align yet!\n");
			}
			mdelay(500);
		}		
		if((prbs_result == 0x0)||(prbs_result == 0x100))
		{
			printk("PRBS check Failed! please check RX rates & selected pattern!\n");
		}
	}
	else if(BIST_check_switch == PHY_DISABLE)
	{
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,  16, 0, 0x0);//rg_bistctl_pat_rx_check_en
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 31, 0, 0x01000000); 	// BFAF3684 default = 0x01000000
		printk("RX BIST Check End!\n");	
	}
}	

uint rx_bist_check_cnt_calculator (void)
{
	uint prbs_err_cnt = 0;
	prbs_err_cnt = IO_GPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,	15, 0);//ro_bistctl_prbs_err_cnt 
	//reset rx_bist_check error counter in case it overflow
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 24, 24, 0x1);//bit_error_rst_sel
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 24, 24, 0x0);
	bist_check_err_cnt += prbs_err_cnt;
	bist_err_cnt_clear_times++;

	return bist_check_err_cnt;
}	
void bist_check_cnt_add(int ms)
{
	volatile uint32 timer_now, timer_last;
	volatile uint32 tick_acc;

	uint32 one_tick_unit = SYS_HCLK * 500;// 1 * SYS_HCLK * 1000 / 2

	volatile uint32 tick_wait = ms * one_tick_unit; 
	volatile uint32 timer1_ldv = regRead32(CR_TIMER1_LDV);

	tick_acc = 0;
	timer_last = regRead32(CR_TIMER1_VLR);
	do {
		timer_now = regRead32(CR_TIMER1_VLR);
		rx_bist_check_cnt_calculator();
	  	if (timer_last >= timer_now) 
	  		tick_acc += timer_last - timer_now;
		else
			tick_acc += timer1_ldv - timer_now + timer_last;
		timer_last = timer_now;
	} while (tick_acc < tick_wait);
}

#endif

#define _I_AM_PMA_SUPPLEMENT_FUNC_SPLIT_LINE_   

#if ASIC_SERDES
//*****************************Following Functions are defined for EN7580 ASIC application*************************************//
void freq_check (void)
{
	uint read_data;
	read_data = IO_GPHYREG(XPON_PMA_RX_FORCE_MODE_0);
	PON_PHY_PRINT(PHY_MSG_DBG,"0xbfaf3630:%x \n", read_data);
	if ((read_data >>24)==1){
		printk("RX clock: L2D ");
	}
	else{
		printk("RX clock: L2R ");
	}
	read_data = IO_GPHYREG(XPON_PMA_RO_RX_FREQDET);
	if ((read_data & XPON_PMA_FBCK_LOCK) == XPON_PMA_FBCK_LOCK){
		printk(" LOCK \n");
	}
	else{
		printk(" UNLOCK \n");
	}
	PON_PHY_PRINT(PHY_MSG_DBG,"0xbfaf3820:%x \n", read_data);
	read_data = IO_GPHYREG(XPON_PMA_ADD_LCPLL_RO_1);
	PON_PHY_PRINT(PHY_MSG_DBG,"0xbfaf3760:%x \n", read_data);
	if ((read_data & XPON_PMA_ADD_LCPLL_RO_1_RO_PLL_FBCK_LOCK2_MASK ) == XPON_PMA_ADD_LCPLL_RO_1_RO_PLL_FBCK_LOCK2_MASK){
		printk("TX clock: LOCK \n");
	}
	else{
		printk("TX clock: UNLOCK \n");
	}
	read_data = IO_GPHYREG(XPON_PMA_SS_LCPLL_TDC_PW_0);
	PON_PHY_PRINT(PHY_MSG_DBG,"0xbfaf3210:%x \n", read_data);
	if ((read_data & XPON_PMA_SS_LCPLL_TDC_PW_0_TDC_DIG_PWDB ) == XPON_PMA_SS_LCPLL_TDC_PW_0_TDC_DIG_PWDB){
		printk("TDC state: ENABLE \n");
	}
	else{
		printk("TDC state: DISABLE \n");
	}                
}


void set_pma_fir(void) 
{
	uint mode_sel, read_data;
	read_data = IO_GPHYREG(SCU_WAN_CONF_REG);
	mode_sel = read_data & SCU_WAN_CONF_REG_WAN_SEL_BITS;
	switch(mode_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 31, 0, pma_low_rate_opt_val);
			read_data = IO_GPHYREG(XPON_PMA_RG_XPON_TX_EN_0);
			PON_PHY_PRINT(PHY_MSG_ERR,"set low rate pma_optimize_val = 0x%x\n", read_data);
			break;	
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 31, 0, pma_hi_rate_opt_val);
			read_data = IO_GPHYREG(XPON_PMA_RG_XPON_TX_EN_0);
			PON_PHY_PRINT(PHY_MSG_ERR,"set high rate pma_optimize_val = 0x%x\n", read_data);
			break;
			
		default:
			return PHY_FAILURE;
	}
	return PHY_SUCCESS;
}


void get_pma_fir(void)	// by david 20180914
{
	uint read_data;

	PON_PHY_PRINT(PHY_MSG_ERR,"get SW pma_hi_rate_opt_val = 0x%x\n", pma_hi_rate_opt_val);
	PON_PHY_PRINT(PHY_MSG_ERR,"get SW pma_low_rate_opt_val = 0x%x\n", pma_low_rate_opt_val);
	
	read_data = IO_GPHYREG(XPON_PMA_RG_XPON_TX_EN_0);
	PON_PHY_PRINT(PHY_MSG_ERR,"get HW pma_optimize_val = 0x%x\n", read_data);
}


//******************************************************************************************************************//

//*****************************************************************************//
//function :
//		tdc_bw
//description : 
//		this function is used to set TDC bandwidth level.
//		TDC with higher BW level will response faster to RX frequency jitter
// TDC Bandwidth Table: 
								// BW: 160k	0x08
								// BW: 140k	0x09
								// BW: 110k	0x01
								// BW: 84k	0x10
								// BW: 69k	0x11
								// BW: 53k	0x02
								// BW: 39k	0x12
								// BW: 33k	0x13
								// BW: 25k	0x03
								// BW: 19k	0x14
								// BW: 12k	0x04
								// BW: 9.5k	0x15
								// BW: 6.1k	0x05
								// BW: 3k	0x06
								// BW: 1.5k	0x07
// by david 20190306
//******************************************************************************//
void tdc_bw(uint bandwidth_level)
{
//	uint lcpll_a_tdc;
	switch(bandwidth_level)
	{
		case 1:
			lcpll_a_tdc = 0x01;
			printk("========== SET TDC BW: 110kHz ==========\n");
			break;

		case 2:
			lcpll_a_tdc = 0x02;
			printk("========== SET TDC BW: 53kHz ==========\n");
			break;

		case 3:
			lcpll_a_tdc = 0x03;
			printk("========== SET TDC BW: 25kHz ==========\n");
			break;

		case 4:
			lcpll_a_tdc = 0x04;
			printk("========== SET TDC BW: 12kHz ==========\n");
			break;		

		case 5:
			lcpll_a_tdc = 0x05;
			printk("========== SET TDC BW: 6.1kHz ==========\n");
			break;

		default:
			//lcpll_a_tdc = bandwidth_level;
			printk("=== extra BW level: please choose integer between 6 and 15 ===\n");
			printk("========== SET lcpll_a_tdc: 0x%x==========\n", lcpll_a_tdc);
			break;
	}		
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1, 11, 8, lcpll_a_tdc);	

}

void xpon_pma_param_opt (void)
{
	unchar read_data;
	uint write_data;
	uint mode_sel, opt_val;

//*********************************************** TX FIR PARAM OPTIMIZE *************************************************************//
	set_pma_fir();
	get_pma_fir();

//*******************************************************************************************************************************//

//********************************************* TX DATA toggle outside timeslot *********************************************************//
	//free run counter setting:
	IO_SPHYA_REG_BITS(XPON_PMA_MD32_MEM_CLK_CTRL, 1, 0, 0x03);	// BFAF38A0 free run counter clk sel
	IO_SPHYA_REG_BITS(XPON_PMA_TX_DLY_CTRL, 30, 28, 0x01);	// BFAF37B8 sel free run counter
//*******************************************************************************************************************************//

}

void pma_fifo_check (uint ncpo_check_cnt)	//add by ang_20190130
{
	uint read_data=0;
	uint write_data=0;
	uint i=0;
	uint *ncpo_arr;
	uint ncpo_max=0;
	uint ncpo_min=0;

	if((ncpo_check_cnt>0)&&(ncpo_check_cnt<=50000))
	{
	ncpo_arr = (uint*)vmalloc(sizeof(uint) * ncpo_check_cnt);
	}
	else
	{
		printk("Alloc data memory size %d error, recommand(1~50000)\n",ncpo_check_cnt) ;
		return 0;
	}

	if(!ncpo_arr) 
	{
		printk("Alloc data memory failed\n") ;
		return 0;
	}

	memset(ncpo_arr, 0, (sizeof(*ncpo_arr)*ncpo_check_cnt)) ;

	read_data = IO_GPHYREG(XPON_PMA_RX_DEBUG_0);

	IO_SPHYREG(XPON_PMA_RX_DEBUG_0,reversebit(read_data,24));
	IO_SPHYREG(XPON_PMA_RX_DEBUG_0,reversebit(read_data,24));

	printk("	0x%08x=0x%08x\r\n",XPON_PMA_ADD_RO_RX2ANA_3,IO_GPHYREG(XPON_PMA_ADD_RO_RX2ANA_3));
	printk("	0x%08x=0x%08x\r\n",XPON_PMA_ADD_RO_TX2ANA_1,IO_GPHYREG(XPON_PMA_ADD_RO_TX2ANA_1));
	printk("	0x%08x=0x%08x\r\n",XPON_PMA_FIFO_CK_STATUS,IO_GPHYREG(XPON_PMA_FIFO_CK_STATUS));

	for(i=0;i<ncpo_check_cnt;i++)
	{
		ncpo_arr[i]=IO_GPHYREG(XPON_PMA_SS_LCPLL_TDC_RO_4);
	}

	ncpo_max=ncpo_arr[0];
	ncpo_min=ncpo_arr[0];
	
	for(i=1;i<ncpo_check_cnt;i++)
	{
		if(ncpo_max<ncpo_arr[i])
		{
			ncpo_max=ncpo_arr[i];
		}

		if(ncpo_min>ncpo_arr[i])
		{
			ncpo_min=ncpo_arr[i];
		}
	}
	
	printk("	ncpo_max=0x%08x\r\n",ncpo_max);
	printk("	ncpo_min=0x%08x\r\n",ncpo_min);
	printk("	ncpo_diff=0x%08x (ncpo_max-ncpo_min)\r\n",(ncpo_max-ncpo_min));

	vfree(ncpo_arr);
	return 0;

}
#endif	/*	EN7580_SERDES */


