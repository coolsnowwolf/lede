#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "common/mgr_pthread.h"
#include "common/mgr_cmd.h"
#include "common/mgr_util.h"
#include "common/mgr_struct.h"
#include "common/mgr_sdi.h"
#include "core/fmgr/fmgr.h"

extern int gXponMode; /* 0, Auto, 1, GPON, 2, EPON */

/*****************************************************************************************
struct XMCS_PonEventStatus_S {
	uint		phyEventSet ;
	uint		gponEventSet ;
	uint		eponEventSet ;
} ;
typedef enum {
	XMCS_EVENT_GPON_LOS,
	XMCS_EVENT_GPON_PHY_READY,
	XMCS_EVNET_GPON_DEACTIVATE,
	XMCS_EVENT_GPON_ACTIVATE,
} XMCS_GponEventId_t ;

typedef enum {
	XMCS_EVENT_EPON_LOS,
	XMCS_EVENT_EPON_PHY_READY,
	XMCS_EVENT_EPON_REGISTER,
	XMCS_EVENT_EPON_DEREGISTER
} XMCS_EponEventId_t ;
*****************************************************************************************/
void *fmgr_periph_irq_handler(void *arg)
{
	MACRO_CMD_STRUCT(SystemTrapInfo) event ;
	struct XMCS_PonEventStatus_S ponEventStatus ;
	int i ;

	while(1) {
		if(XMCS_IOCTL_SDI(FDET_IOG_WAITING_EVENT, &ponEventStatus) != 0) {
			continue ;
		}

		for(i=0 ; i<ponEventStatus.items ; i++) {
#ifdef TCSUPPORT_WAN_GPON
			if(ponEventStatus.event[i].type == XMCS_EVENT_TYPE_GPON) {
                                if (gXponMode == 0 || gXponMode == 1) {/* only for auto and gpon mode */
				  event.trapIndex = ((TRAP_GROUP_GPON<<8) + ponEventStatus.event[i].id) ;
				  event.trapValue = ponEventStatus.event[i].value ;
				  fmgr_send_trap_to_task(TASK_TYPE_FMGR, TASK_TYPE_OMCI, &event) ;
				}
				continue ;
			} 
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
			if(ponEventStatus.event[i].type == XMCS_EVENT_TYPE_EPON) {
                                if (gXponMode == 0 || gXponMode == 2) {/* only for auto and epon mode */
				  event.trapIndex = ((TRAP_GROUP_EPON<<8) + ponEventStatus.event[i].id) ;
				  event.trapValue = ponEventStatus.event[i].value ;
				  fmgr_send_trap_to_task(TASK_TYPE_FMGR, TASK_TYPE_OAM, &event) ;
				}
                                continue ;
			} 
#endif /* TCSUPPORT_WAN_EPON */
		}
	}
	
	PONMGR_PTHREAD_EXIT(NULL) ;
}
