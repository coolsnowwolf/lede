#ifndef _PHY_INIT_H_
#define _PHY_INIT_H_

#include <linux/workqueue.h>
#include "phy_global.h"
#include "phy.h"

int phy_register_cb_handler(PHY_CbType_t type, void *pCbFun) ;
int phy_unregister_cb_handler(PHY_CbType_t type) ;
int xpon_phy_start(void) ;
int pon_phy_init(void) ;
void pon_phy_deinit(void) ;
void phy_los_handler(void) ;
void phy_ready_handler(void) ;

static inline int is_phy_sync(void)
{
    return ((phy_ready_status() == PHY_TRUE) && (phy_los_status() == PHY_NO_LOS_HAPPEN) ); 
}

static inline int get_phy_mode(void)
{
    return gpPhyPriv->phyCfg.flags.mode ;
}

extern struct workqueue_struct * i2c_access_queue;
#endif /* _PHY_INIT_H_ */




