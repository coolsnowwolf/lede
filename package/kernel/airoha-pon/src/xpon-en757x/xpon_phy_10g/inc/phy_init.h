#ifndef _PHY_INIT_H_
#define _PHY_INIT_H_

#include <linux/workqueue.h>
#include "phy.h"

int phy_register_cb_handler(PHY_CbType_t type, void *pCbFun) ;
int phy_unregister_cb_handler(PHY_CbType_t type) ;
int phy_mode_config(Xpon_Phy_Mode_t xpon_mode, int tx_enable);
#ifndef TCSUPPORT_CPU_EN7581
void phy_eyescan_test(PHY_EYESCAN_MODE_LIST mode, int start_p, int sweep_r, int Ovr);
#endif
void XPON_eyescan_setting(PHY_EYESCAN_MODE_LIST mode);
void XFI_eyescan_setting(PHY_EYESCAN_MODE_LIST mode);
void XPON_eyescan_Cal(void);
void XFI_eyescan_Cal(void);
int eyescan_countPoint(uint EYE_X_FW, uint EYE_Y_FW, PHY_EYESCAN_MODE_LIST mode);
uint eyescan_moveX(uint EYE_X_HW, uint EYE_Y_HW, int Ovr_sel, PHY_EYESCAN_MODE_LIST mode);
int phy_pma_reset(void);
int phy_pma_reset_with_lock(void);
int phy_pcs_fpgaif_reset(void);	//by ang_20170901
int phy_pcs_reset(void);		//by ang_20170911
int xpon_phy_start(void) ;
int xpon_phy_stop(void);
int pon_phy_mod_init(void);
int pon_phy_init(void) ;
void pon_phy_deinit(void) ;
void phy_los_handler(void) ;
void phy_ready_handler(void) ;
void handle_hw_irq_event(PHY_Event_Type_t id);
int pon_phy_get_wan_sel(void);
int pon_phy_func_init(void);
void NCPO_Force(int enable); //julia_20221003
#if defined(TCSUPPORT_COMBO_PON)
int phy_mode_detection(void);  //1=low priority, 2=high priority
int phy_mode_task_wait(void);
int combo_pon_phy_init(void);
extern char get_onutype(void);
#endif
static inline int is_phy_sync(void)
{
    return ((phy_ready_status() == PHY_TRUE) && (phy_los_status() == PHY_NO_LOS_HAPPEN) ); 
}

static inline int phy_no_ready_no_los(void)
{
    return ((phy_ready_status() == PHY_FALSE) && (phy_los_status() == PHY_NO_LOS_HAPPEN) ); 
}


extern struct workqueue_struct * i2c_access_queue;


#endif /* _PHY_INIT_H_ */



