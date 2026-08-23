#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "common/mgr_ipc.h"
#include "common/mgr_cmd.h"
#include "common/mgr_util.h"
#include "common/mgr_ipc.h"

#include "dspch/dspch.h"
#include "core/fmgr/fmgr.h"

/*****************************************************************************************
*****************************************************************************************/
MGR_Ret fmgr_cmd_shunter(MGR_CmdMsg_T *pMsg)
{
	MGR_Ret ret ;
	MGR_U16 cmdId = pMsg->CmdId/CONST_GROUP_CMD_NUM ;
	
	switch(cmdId) {
		MACRO_CMD_PROC_SWITCH(AlarmClear) ;
		

		default:
			ret = pMsg->Result = EXEC_ERR_NO_SUCH_CMD ; 
			DEBUG_PERROR("Paraer the fmgr command %d error, errno:%d.\n", cmdId, ret) ;
			break ;
	}

	return ret ;	
}

/*****************************************************************************************
 *****************************************************************************************/
MGR_Ret fmgr_send_trap_to_task(MGR_U16 srcTask, MGR_U16 destTask, MACRO_CMD_STRUCT(SystemTrapInfo) *pTrapInfo)
{
	MGR_Ret ret = EXEC_OK ;
	IPC_Msg_T ipcMsg ; 
	
	ipcMsg.mtype = ENUM_MSG_TYPE_TRAP ;
	ipcMsg.data.CmdId = (pTrapInfo->trapIndex & CONST_TRAP_GROUP_ID_MASK) >> 8 ;
	ipcMsg.data.Stid = srcTask ;
	ipcMsg.data.Dtid = destTask ;

	ipcMsg.data.Length = sizeof(IPC_Msg_T) - sizeof(ipcMsg.data.u) + sizeof(MACRO_CMD_STRUCT(SystemTrapInfo)) ;
	memcpy(&ipcMsg.data.u.oSystemTrapInfo, pTrapInfo, sizeof(MACRO_CMD_STRUCT(SystemTrapInfo))) ;

	ret = dspch_report_trap_msg(&ipcMsg) ;

	return ret ;
}  

