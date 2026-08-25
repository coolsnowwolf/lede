#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "api/mgr_api.h"
#include "common/mgr_pthread.h"
#include "common/mgr_util.h"

#include "core/fmgr/fmgr.h"
#include "core/dbmgr/dbmgr.h"

MGR_Ret (*ponapi_trap_callback_function)(MACRO_CMD_STRUCT(SystemTrapInfo) *) = NULL ;
MGR_S32 trapMsgQueue = 0 ;
MGR_S8 trapFlag = 0 ;
/**************************************************************************************************
**************************************************************************************************/
static void *ponapi_trap_handler(void *arg)
{
	IPC_Msg_T ipcMsg ;
	int count = 0 ;
	
	while(1) {
		if((ipc_msg_queue_recv(trapMsgQueue, 0, &ipcMsg)) <= 0) {
			DEBUG_PERROR("API: receive trap message error, msgid: %d\n", trapMsgQueue);
			
			if(count >=3)
				break ;
			
			count++ ;
			continue ;
		}
		if(ponapi_trap_callback_function != NULL) {
			ponapi_trap_callback_function((MACRO_CMD_STRUCT(SystemTrapInfo) *)&ipcMsg.data.u.oSystemTrapInfo) ;
		}
		
		count = 0 ;
	}
	
	trapFlag = 0 ;
	
	pthread_exit(NULL) ;
}

/**************************************************************************************************
**************************************************************************************************/
MGR_Ret ponapi_trap_init(MGR_U16 srcTask, MGR_Ret (*trapCallbackFun)(MACRO_CMD_STRUCT(SystemTrapInfo) *))
{
	pthread_t myThread ;
	pthread_attr_t myThreadAttr ;

	if(trapCallbackFun==NULL && trapFlag) {
		return EXEC_ERR ;
	}
	ponapi_trap_callback_function = trapCallbackFun ;
	
	if(srcTask == TASK_TYPE_OMCI) {
		trapMsgQueue = ipc_msg_queue_get_id(IPC_MSG_QUEUE_OMCI) ;
		if(trapMsgQueue < 0) {
			return EXEC_ERR ;
		}
	} else if(srcTask == TASK_TYPE_OAM) {
		trapMsgQueue = ipc_msg_queue_get_id(IPC_MSG_QUEUE_OAM) ;
		if(trapMsgQueue < 0) {
			return EXEC_ERR ;
		}
	} else {
		return EXEC_ERR ;
	}
	
	if(pthread_attr_init(&myThreadAttr)  != 0) {
		DEBUG_PERROR("Initial pthread attribution error, errno: %d.\n", EXEC_ERR_RESOURCE_INIT_FAILED) ;
		return EXEC_ERR_RESOURCE_INIT_FAILED ;
	}
	
	if(pthread_attr_setdetachstate(&myThreadAttr, PTHREAD_CREATE_DETACHED) != 0) {
		DEBUG_PERROR("Setting pthread attribution error, errno: %d.\n", EXEC_ERR_RESOURCE_CONFIG_FAILED) ;
		return EXEC_ERR_RESOURCE_CONFIG_FAILED ;
	}

	if(pthread_create(&myThread, &myThreadAttr, ponapi_trap_handler, NULL) != 0) {
		DEBUG_PERROR("API: Create trap handler thread failed.\n") ;
		return EXEC_ERR_RESOURCE_CREATE_FAILED ;
	}
	
	trapFlag = 1 ;
	
	return EXEC_OK ;
}


/**************************************************************************************************
**************************************************************************************************/
MGR_Ret ponapi_trap_destory(void)
{
	if(ponapi_trap_callback_function != NULL) {
		ponapi_trap_callback_function = NULL ;
	}
	
	return EXEC_OK ;
}
