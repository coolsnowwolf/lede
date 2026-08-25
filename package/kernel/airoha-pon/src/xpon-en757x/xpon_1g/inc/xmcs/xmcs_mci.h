#ifndef _XMCS_MCI_H_
#define _XMCS_MCI_H_

#include "common/drv_types.h"
#include "xmcs/xmcs_fdet.h"

#define COSNT_XMCI_MAJOR_NUN						190
#ifndef TCSUPPORT_XPON_HAL_API_EXT
#define CONST_XMCI_DEV_NAME							"pon"
#endif

typedef struct {
    struct cdev                     *pPonMciDev ;
    wait_queue_head_t               xmcsWaitQueue ;
    struct XMCS_PonEventStatus_S    xmcsEventStatus ;
    Event_ctrlFlag_t                ctrlFlag;
    spinlock_t                      fdetLock ;
} MCS_GlbPriv_T ;


int pon_mci_init(void) ;
void pon_mci_destroy(void) ; 

#endif /* _XMCS_MCI_H_ */
