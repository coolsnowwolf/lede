#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "common/mgr_ipc.h"
#include "common/mgr_sem.h"
#include "common/mgr_pthread.h"
#include "common/mgr_util.h"

#include "core/cmgr/cmgr.h"
#include "core/pmgr/pmgr.h"
#include "core/fmgr/fmgr.h"

/*****************************************************************************
*****************************************************************************/
void dspch_shunt_cmd_request(MGR_CmdMsg_T *pMsg)
{
	switch(pMsg->Dtid) {
		case TASK_TYPE_CMGR:
			cmgr_cmd_shunter(pMsg) ;
			break ;	
			
		case TASK_TYPE_PMGR:
			pmgr_cmd_shunter(pMsg) ;
			break ;	
		
		case TASK_TYPE_FMGR :
			fmgr_cmd_shunter(pMsg) ;
			break ;	
				
		default:
			pMsg->Result = EXEC_ERR_NO_SUCH_TID ;
			DEBUG_PERROR("Parser dest. task ID %d error, errno:%d\n", pMsg->Dtid, EXEC_ERR_NO_SUCH_TID) ;
			break ;
	}
}


/*****************************************************************************
*****************************************************************************/
MGR_Ret dspch_report_trap_msg(IPC_Msg_T *pIpcMsg)
{
	MGR_Ret ret = EXEC_OK ;

	if(gOmciTrapQueueId>=0 && (pIpcMsg->data.Dtid&TASK_TYPE_OMCI)) {
		if(ipc_msg_queue_send(gOmciTrapQueueId, pIpcMsg) < 0) {
			DEBUG_PERROR("Send a trap to OMCI failed. failed, QID: %d\n", gOmciTrapQueueId) ;
		}
	}
	
	if(gOamTrapQueueId>=0 && (pIpcMsg->data.Dtid&TASK_TYPE_OAM)) {
		if(ipc_msg_queue_send(gOamTrapQueueId, pIpcMsg) < 0) {
			DEBUG_PERROR("Send a trap to app. failed, QID: %d\n", gOamTrapQueueId) ;
		}
	}
	
	return ret ;
}

