#include <stdio.h>
#include <strings.h>
#include "common/mgr_pthread.h"

#include "core/fmgr/fmgr.h"
#include "core/dbmgr/dbmgr.h"

/*****************************************************************************
*****************************************************************************/
MGR_Ret fmgr_init(void)
{
	//create a thread to monitor system
	if(PONMGR_PTHREAD_CREATE(fmgr_monitor_status_handler, NULL, PTHREAD_CREATE_DETACH) != 0) {
		DEBUG_PERROR("Create system monitor handler thread failed, errno: %d.", EXEC_ERR_RESOURCE_CREATE_FAILED) ;
		return EXEC_ERR_RESOURCE_CREATE_FAILED ;
	}

	//create a thread to maintain system peripheral status(include IRQ)
	if(PONMGR_PTHREAD_CREATE(fmgr_periph_irq_handler, NULL, PTHREAD_CREATE_DETACH) != 0) {
		DEBUG_PERROR("Create system interrupt handler thread failed, errno: %d.", EXEC_ERR_RESOURCE_CREATE_FAILED) ;
		return EXEC_ERR ;
	}

	return EXEC_OK ;
}

/*****************************************************************************
*****************************************************************************/
MGR_Ret fmgr_destroy(void)
{
	MGR_Ret ret = EXEC_OK ;
	
	return ret ;	
}

