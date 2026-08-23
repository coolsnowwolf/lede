#ifndef _PHY_H_
#define _PHY_H_

#include <ecnt_hook/ecnt_hook_pon_mac.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>

#include "phy_def.h"

typedef struct {
	unchar					t1 ; /* ms */
	unchar					t2 ; /* times */
	ushort					intCounter ;
	uint					intTime ;
	struct {
		unchar				txPowerEnFlag		: 1 ;
		unchar				txLongFlag			: 1 ;
		unchar				trafficStatus		: 1 ;
		unchar				resv 				: 2 ;
        /* after phy driver initialization, it should reflect phy's current mode */
        unsigned int     mode                : 2 ;
        unchar              rogue               : 1 ; /* flag for rogue mode	cmd: echo Phy_Rogue_PRBS 1/0 > /proc/pon_phy/debug */
	} flags ;

} PHY_Config_T ;

typedef struct {
	struct timer_list		phy_timer    ;
	struct timer_list		int_timer    ;
	struct timer_list		mt7570_timer ;
	struct timer_list       phy_ready_timer;

	struct timer_list		APD_timer	 ;
	struct task_struct  *LDDLA_task_wait ;

	ushort					i2c_u2_clk_div;

	void (*phyXponStateNotifyHandler)(int stateNotifyType) ;
	PHY_Config_T			phyCfg ;
	ushort					debugLevel ;

    unchar                  is_phy_start  ;

	struct timer_list		event_poll_timer ;
    spinlock_t              event_handle_lock;
    PHY_Los_Status_t  phy_status ; /* LOS or Ready */
	uint	phyGuardBitDelm;
	uint	phyPreamble    ;
	uint	phyExtPreamble ;

    struct work_struct      EN7570_reset ;
    struct work_struct      EN7570_rouge_clear ;

} PHY_GlbPriv_T ;

ushort phy_I2C_read(unchar u1CHannelID, ushort u2ClkDiv, unchar u1DevAddr,
				 unchar u1WordAddrNum, uint u4WordAddr, unchar *pu1Buf,
				 ushort u2ByteCnt);
ushort phy_I2C_write(unchar u1CHannelID, ushort u2ClkDiv, unchar u1DevAddr,
						  unchar u1WordAddrNum, uint u4WordAddr, unchar *pu1Buf,
						  ushort u2ByteCnt);

int phy_debug_init(void) ;
int phy_debug_deinit(void) ;
unchar phy_gpon_mode(void) ;
#if !defined(CONFIG_USE_MT7520_ASIC) && !defined(CONFIG_USE_A60928)
void io_trans_reg(uint addr, unchar reg) ;
#endif
void phy_tx_fault_reset(void);
void phy_tx_power_config(unchar enable);
int phy_trans_power_switch(unchar trans_switch);


int phy_ready_status(void);
int phy_los_status(void);

extern PHY_GlbPriv_T *gpPhyPriv;
void phy_event_handler(PON_PHY_Event_data_t * pEvent_data);
void phy_event_poll(ulong data);

/*GPON/EPON config*/
int phy_mode_config(Xpon_Phy_Mode_t xpon_mode, int tx_enable);
/*interrupt setting*/
int phy_int_config(uint int_type);

static inline void XPON_MAC_EVENT_HANDLER(PON_PHY_Event_data_t * pEvent)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PHY_MODULE  ;
    data.pEvent      = pEvent ;

    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
        panic("ECNT_HOOK_ERROR occur with event id:%d. %s:%d\n", pEvent->id, __FUNCTION__, __LINE__);
    }
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


#ifdef TCSUPPORT_AUTOBENCH
//UINT16 SIF_X_Read(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum, UINT32 u4WordAddr,
//                  UINT8 *pu1Buf, UINT16 u2ByteCnt);

#endif
#endif /* _PHY_H_ */


