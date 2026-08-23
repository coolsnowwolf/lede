#include <stdio.h>
#include <string.h>

#include "core/fmgr/fmgr.h"


/***********************************************************************************************
************************************************************************************************
*** CMD Process Function for FMGR                                                            ***
************************************************************************************************
***********************************************************************************************/
/************************************************************************************

************************************************************************************/
MACRO_CMD_PROC_FUNCTION(AlarmClear)
{
	MGR_Ret ret = EXEC_ERR_CMD_NOT_SUPPORT ;

	switch(pMsg->CmdId)
	{
		case MACRO_CMDID_CREATE(LogMsgConfig):
			break ;

		case MACRO_CMDID_DELETE(LogMsgConfig):
			break ;

		case MACRO_CMDID_SET(LogMsgConfig):
			break ;

		case MACRO_CMDID_GET(LogMsgConfig):
			break ;

		default:
			break ;
	}
	
	MACRO_PROC_RETURN(AlarmClear, ret) ;
}

