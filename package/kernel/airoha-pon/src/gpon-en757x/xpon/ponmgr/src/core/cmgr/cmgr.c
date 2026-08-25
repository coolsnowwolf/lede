#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "common/mgr_ipc.h"
#include "common/mgr_cmd.h"

#include "core/cmgr/cmgr.h"

MGR_Ret cmgr_cmd_shunter(MGR_CmdMsg_T *pMsg)
{
	MGR_Ret ret ;
	MGR_U16 cmdId = pMsg->CmdId/CONST_GROUP_CMD_NUM ;
	
	switch(cmdId) {
		MACRO_CMD_PROC_SWITCH(SysLinkConfig)
		MACRO_CMD_PROC_SWITCH(SysOnuType)
		MACRO_CMD_PROC_SWITCH(SysClearConfig)

		MACRO_CMD_PROC_SWITCH(PhyFecConfig)
		MACRO_CMD_PROC_SWITCH(PhyTransConfig)
		MACRO_CMD_PROC_SWITCH(PhyTransParameters)
		MACRO_CMD_PROC_SWITCH(PhyTxBurstConfig)

		MACRO_CMD_PROC_SWITCH(PWanChannelQoS)
		MACRO_CMD_PROC_SWITCH(PWanCongestConfig)
		MACRO_CMD_PROC_SWITCH(PWanTrtcmConfig)
		MACRO_CMD_PROC_SWITCH(PWanPcpConfig)
		MACRO_CMD_PROC_SWITCH(PWanCntStats)

		MACRO_CMD_PROC_SWITCH(StormCtrlConfig)

#ifdef TCSUPPORT_WAN_GPON
		MACRO_CMD_PROC_SWITCH(GponSnPasswd)
		MACRO_CMD_PROC_SWITCH(GponActTimer)
		MACRO_CMD_PROC_SWITCH(GponSystemInfo)
		MACRO_CMD_PROC_SWITCH(GponDbaConfig)
		MACRO_CMD_PROC_SWITCH(GponTodConfig)
		MACRO_CMD_PROC_SWITCH(GponGemPortConfig)
		MACRO_CMD_PROC_SWITCH(GponTcontStatus)
		MACRO_CMD_PROC_SWITCH(GponGemCounter)		
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
		MACRO_CMD_PROC_SWITCH(EponRxConfig)
		MACRO_CMD_PROC_SWITCH(EponTxConfig)
		MACRO_CMD_PROC_SWITCH(EponLlidStatus)
#endif /* TCSUPPORT_WAN_EPON */

		default:
			ret = pMsg->Result = EXEC_ERR_NO_SUCH_CMD ; 
			DEBUG_PERROR("Paraer the cmgr command %d error, errno:%d.\n", cmdId, ret) ;
			break ;
	}

	return ret ;
}
