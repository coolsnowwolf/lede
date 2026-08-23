#ifndef _PHY_H_
#define _PHY_H_

#include <ecnt_hook/ecnt_hook_pon_mac.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>

#include "phy_def.h"
#include "phy_debug.h"
#include "phy_tx.h"

#if defined(TCSUPPORT_MT7570)||defined(LDDLA_SUPPORT_SET_TX_MODE)
#include <ecnt_hook/ecnt_hook_lddla.h>
#endif


// Define

typedef struct {
	unchar					t1 ; /* ms */
	unchar					t2 ; /* times */
	ushort					intCounter ;
	uint					intTime ;
	#if 1  //EN7580_TEST_CODE for /* BIG_ENDIAN LITTLE_ENDIAN */  
	struct {
		unchar				txPowerEnFlag;
		unchar				txLongFlag;
		unchar				trafficStatus;
        /* after phy driver initialization, it should reflect phy's current mode */
        Xpon_Phy_Mode_t     mode;
        unchar              rogue; /* flag for rogue mode	cmd: echo Phy_Rogue_PRBS 1/0 > /proc/pon_phy/debug */
	} flags ;
	#else
	struct {
		unchar				txPowerEnFlag		: 1 ;
		unchar				txLongFlag			: 1 ;
		unchar				resv 				: 3 ;
        /* after phy driver initialization, it should reflect phy's current mode */
        Xpon_Phy_Mode_t     mode                : 2 ;  ==> 5
        unchar              rogue               : 1 ; /* flag for rogue mode	cmd: echo Phy_Rogue_PRBS 1/0 > /proc/pon_phy/debug */
	} flags ;
    #endif
} PHY_Config_T ;

typedef struct {
    uint					phy_timer_value; //julia_20201017
    struct timer_list		phy_timer    ;
    uint					int_timer_value; //julia_20201017
    struct timer_list		int_timer    ;
	unchar                  phy_deinit_flag;
	unchar                  pon_stop_flag;
	struct task_struct      *phy_trans_task_wait ;
#if defined(TCSUPPORT_COMBO_PON)	
	struct task_struct      *phy_mode_task_wait;
#endif
    ushort					i2c_u2_clk_div;
	ushort					i2c_addr_num;
    void					(*phyXponStateNotifyHandler)(int stateNotifyType) ;
    PHY_Config_T			phyCfg ;
    ushort					debugLevel ;	
    ushort					debugLevel_backup;
    unchar					is_phy_start  ;
	unchar					is_irq_requested;
    uint					event_poll_timer_value; //julia_20201017
    struct timer_list		event_poll_timer ;
    spinlock_t				event_handle_lock;
    spinlock_t				pma_reset_lock;
    PHY_Los_Status_t  		phy_status ; /* LOS or Ready */
    uint					phyGuardBitDelm;
    uint					phyPreamble    ;
    uint					phyExtPreamble ;
    uint 					scu_hir_np_sys_hw_id;
    uint                    wan_sel;
    unchar					phy_init_done;
#ifdef TCSUPPORT_MT7570
	unchar					en7571_init_done;
#endif
    uint					phy_ready_time;
    uint					trans_index;
	uint 					trans_tx_ben_level;
	uint 					trans_tx_enable; //set value from MAC
	unchar					trans_tx_status;     //record value by PHY API //tx_dis_reset_protection
	uint 					mac_set_phy_init_step;
	uint					phy_unexpected_isr_flag;
	uint					first_plugin_flag; //ang_20180926
	uint					pma_init_done;
	uint					xe_syncloss_cnt;
	uint					rogue_onu_det_en;
	uint					rogue_onu_ben_det_en;//julia_20240510
	uint					rogueonu_happen; //julia_20230614
	uint                    rogue_state;
	uint					is_rogue_onu; //julia_20230629
	ushort			 		temprature; //julia_20230614
	uint					trans_mod_id;	//SFP or XFP or other
	char                    trans_msg_print_cnt;
#ifdef LDDLA_SUPPORT_SET_TX_MODE
	char                    lddla_auto_mode;
#endif
	struct task_struct      *phy_ngpon2_task_wait ; //julia_20221021 ngpon2
	PHY_Ngpon2_Chan_Sel_T Ngpon2_chan; //julia_20221021 ngpon2
	unchar                  rx_fec_setting;	
#if defined(TCSUPPORT_COMBO_PON)	
    unchar					is_phy_mode_detected;
    unchar                  combo_pon_enable;
#ifdef TCSUPPORT_CPU_AN7583 //7583_COMBO-PON
     unchar                  i2c1_used;
#endif
#endif
} PHY_GlbPriv_T ;



// Parameter
extern PHY_GlbPriv_T *gpPhyPriv;
extern ponPhyFuncTbl* ponPhyFunc;
extern p_xpon_phy_api_data_t p_cmd_api_data;

#if ASIC_SERDES || A60972_SERDES
extern uint pma_hi_rate_opt_val;
extern uint pma_low_rate_opt_val;
#endif

#if ASIC_SERDES
extern char rxcalib_force_sel;
extern char txcalib_force_sel;
extern char lcpll_a_tdc;
#endif
extern uint lddla_set_tx_mode_flag;// whliu_20230602

// Fucntion


int phy_debug_init(void) ;

int phy_debug_deinit(void) ;



/*transceiver alarm */

int phy_rx_power_alarm(void);
/*transceiver pin define*/

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON)
int los_status_init(void);
#endif/*TCSUPPORT_COMPILE*/

int phy_ready_status(void);
int phy_los_status(void);
int phy_lof_status(void);

int phy_cnt_enable(int errcnt_en, int bipcnt_en, int fmcnt_en);
int phy_reset_counter(void);
int phy_set_trans_setting(void);
int pon_phy_get_trans_status(PHY_Trans_Status_t * trans_status);

extern void (*set_pon_phy_mode_config)(Xpon_Phy_Mode_t xpon_mode, int tx_enable);
extern void (*set_pon_phy_start)(void);
extern void (*set_pon_phy_stop)(void);
extern void (*get_pon_phy_trans_status)(PHY_Trans_Status_t * trans_status);

int phy_bit_delay(int delay_value);

/*Tx test pattern*/									//by HC 20150304
int phy_fw_ready(int fwrdy_en);

void phy_event_handler(PON_PHY_Event_data_t * pEvent_data);
int phy_isr(void);
void phy_event_poll(ulong data);
/*GPON/EPON config*/

/*interrupt setting*/
int phy_int_config(UINT32 mode,UINT32 int_type);
int pon_phy_api_dispatch(struct ecnt_data *in_data);


static inline void XPON_MAC_EVENT_HANDLER(PON_PHY_Event_data_t * pEvent)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PHY_MODULE  ;
    data.pEvent      = pEvent ;
	
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
        panic("ECNT_HOOK_ERROR occur with event id:%d. %s:%d\n", pEvent->id, __FUNCTION__, __LINE__);
    }
	PON_PHY_PRINT(PHY_MSG_DBG,"ECNT_HOOK_DONE!! \n");
}

static inline void REPORT_EVENT_TO_MAC(PHY_Event_Type_t id)
{
    PON_PHY_Event_data_t event = {0};
    struct xpon_mac_hook_data_s data = {0} ;
    
    event.id         = id ;
    data.src_module  = XPON_PHY_MODULE ;
    data.pEvent      = &event ;
        
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
        panic("ECNT_HOOK_ERROR occur with event id:%d. %s:%d\n", id, __FUNCTION__, __LINE__);
    }

}
#if defined(TCSUPPORT_COMBO_PON)
static inline void REPORT_EVENT_TO_COMBO(PHY_Event_Type_t id)
{
    PON_PHY_Event_data_t event = {0};
    struct xpon_mac_hook_data_s data = {0} ;
    
    event.id         = id ;
    data.src_module  = XPON_PHY_MODULE ;
    data.pEvent      = &event ;
        
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_COMBO_PON, ECNT_COMBO_PON_HOOK, (struct ecnt_data * )&data) ){
        panic("ECNT_HOOK_ERROR occur with event id:%d. %s:%d\n", id, __FUNCTION__, __LINE__);
    }

}

#endif


#if defined(TCSUPPORT_MT7570)||defined(LDDLA_SUPPORT_SET_TX_MODE)
static inline int CALL_LDDLA_ENCT_HOOK(struct lddla_api_data_s * data)
{

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_LDDLA, ECNT_LDDLA_API, (struct ecnt_data * )data) ){
        panic("LDDLA_HOOK_ERROR occur with cmd_id:0x%x\n", data->cmd_id);
    }

    if(PHY_NO_API == data->ret){
        panic("LDDLA No such API with type:%d, cmd_id:0x%x\n", data->api_type, data->cmd_id);
    }

    return data->ret;
}

static inline int LDDLA_GET_API(uint cmd_id, void * in_data) 
{
    struct lddla_api_data_s lddla_data = {0};
    
    lddla_data.api_type = LDDLA_API_TYPE_GET;
    lddla_data.cmd_id   = cmd_id  ;
    lddla_data.raw     = in_data ;

    return CALL_LDDLA_ENCT_HOOK(&lddla_data);
}

static inline int LDDLA_SET_API(uint cmd_id, void * in_data) 
{
    struct lddla_api_data_s lddla_data = {0};
    
    lddla_data.api_type = LDDLA_API_TYPE_SET;
    lddla_data.cmd_id   = cmd_id  ;
    lddla_data.raw     = in_data ;

    return CALL_LDDLA_ENCT_HOOK(&lddla_data);
}
#endif

#ifdef TCSUPPORT_MT7570
static inline void LDDLA_GET_TRANS_STATUS(LDDLA_Trans_Status_t * lddla_status)
{
    LDDLA_GET_API(LDDLA_GET_PHY_TRANS_STATUS, lddla_status );
}

static inline void LDDLA_SET_TRANS_RESET(void)
{
    LDDLA_SET_API(LDDLA_SET_TRANS_SWITCH_RESET, TRUE );
}

#endif

#ifdef LDDLA_SUPPORT_SET_TX_MODE
static inline void LDDLA_SET_TX_MODE(int mode)
{
    if(lddla_set_tx_mode_flag != mode) // whliu_20230602 set only once
    {
        lddla_set_tx_mode_flag = mode;
	LDDLA_SET_API(LDDLA_SET_7572_TX_MODE, &mode);
}
}
#endif

void phy_dbg(char id,char option1,char option2); 
void phy_reg_set(uint addr,uint val); //ang_20180208

void phy_dump(void);

/*flash matrix read&write functions*/
int save_flash_matrix(void);
int get_flash_matrix(void);
void flash_dump(void);							
void set_flash_register(uint reg, uint offset);
void set_flash_register_default(void);
uint get_flash_register(int address_offset);
void fir_param_set(uint fir_cn1, uint fir_inv_c0, uint fir_c1, uint fir_c2);

int pon_phy_get_los_status(void);

int phy_ngpon2_set_chan_task_wait(void);////julia_20221021 ngpon2


#if ASIC_SERDES
// -- functions  ----------------------------------------------------------------
uint IO_GPHYA_REG_BITS(UINT32 reg_name,UINT32 end_index,UINT32 start_index);		// for EN7580 ASIC
void IO_SPHYA_REG_BITS(UINT32 reg_name,UINT32 end_index,UINT32 start_index,UINT32 value); 	// for EN7580 ASIC

#endif




#endif /* _PHY_H_ */


