#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common/mgr_ipc.h"
#include "common/mgr_sem.h"

MGR_S32 gOmciTrapQueueId = -1 ;
MGR_S32 gOamTrapQueueId = -1 ;

/*****************************************************************************
*****************************************************************************/
MGR_Ret dspch_init(void)
{
	MGR_Ret ret = EXEC_OK ;
	
	//create the IPC message queue for OMCI 
	gOmciTrapQueueId = ipc_msg_queue_get_id(IPC_MSG_QUEUE_OMCI) ;
	if(gOmciTrapQueueId >= 0) {
		ipc_msg_queue_delete(0, gOmciTrapQueueId) ;
	}
	
	DEBUG_MESSAGE("Create IPC trap message queue\n") ;
	gOmciTrapQueueId = ipc_msg_queue_create(IPC_MSG_QUEUE_OMCI) ;
	if(gOmciTrapQueueId < 0) {
		ret = EXEC_ERR_RESOURCE_CREATE_FAILED ;
		DEBUG_PERROR("Create IPC trap message queue failed, rtn: %d\n", gOmciTrapQueueId) ;
	}
	
	//create the IPC message queue for OAM
	gOamTrapQueueId = ipc_msg_queue_get_id(IPC_MSG_QUEUE_OAM) ;
	if(gOamTrapQueueId >= 0) {
		ipc_msg_queue_delete(0, gOamTrapQueueId) ;
	}
	
	DEBUG_MESSAGE("Create IPC trap message queue\n") ;
	gOamTrapQueueId = ipc_msg_queue_create(IPC_MSG_QUEUE_OAM) ;
	if(gOamTrapQueueId < 0) {
		ret = EXEC_ERR_RESOURCE_CREATE_FAILED ;
		DEBUG_PERROR("Create IPC trap message queue failed, rtn: %d\n", gOamTrapQueueId) ;
	}
	
	return ret ;
}

/*****************************************************************************
*****************************************************************************/
MGR_Ret dspch_destroy(void)
{
	MGR_Ret ret = EXEC_OK ;
	
	//create the IPC message queue for OMCI 
	gOmciTrapQueueId = ipc_msg_queue_get_id(IPC_MSG_QUEUE_OMCI) ;
	if(gOmciTrapQueueId >= 0) {
		ipc_msg_queue_delete(0, gOmciTrapQueueId) ;
	}
	
	//create the IPC message queue for OAM
	gOamTrapQueueId = ipc_msg_queue_get_id(IPC_MSG_QUEUE_OAM) ;
	if(gOamTrapQueueId >= 0) {
		ipc_msg_queue_delete(0, gOamTrapQueueId) ;
	}
	
	return ret ;	
}

