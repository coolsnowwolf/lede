#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "api/mgr_api.h"
#include "dspch/dspch.h"

#define MACRO_PONMGR_CREATE_API(destTask, name) \
MGR_Ret PonApi_##name##_Create	(MACRO_CMD_STRUCT(name) *pData)  	\
{ \
	return api_do_set_command(destTask, MACRO_CMDID_CREATE(name), 0, (void *)pData, sizeof(MACRO_CMD_STRUCT(name))) ; \
}

#define MACRO_PONMGR_DELETE_API(destTask, name) \
MGR_Ret PonApi_##name##_Delete	(MACRO_CMD_STRUCT(name) *pData)  	\
{ \
	return api_do_set_command(destTask, MACRO_CMDID_DELETE(name), 0, (void *)pData, sizeof(MACRO_CMD_STRUCT(name))) ; \
} 

#define MACRO_PONMGR_GET_API(destTask, name) \
MGR_Ret PonApi_##name##_Get	(MACRO_CMD_STRUCT(name) *pData)  	\
{ \
	return api_do_get_command(destTask, MACRO_CMDID_GET(name), 0, (void *)pData, sizeof(MACRO_CMD_STRUCT(name))) ; \
}

#define MACRO_PONMGR_SET_API(destTask, name) \
MGR_Ret PonApi_##name##_Set	(MGR_Mask cmdMask, MACRO_CMD_STRUCT(name) *pData)  	\
{ \
	return api_do_set_command(destTask, MACRO_CMDID_SET(name), cmdMask, (void *)pData, sizeof(MACRO_CMD_STRUCT(name))) ; \
} 



 
/**************************************************************************************************
**************************************************************************************************/
static inline MGR_Ret api_do_set_command(MGR_U16 dTask, MGR_U16 cmdId, MGR_U32 cmdMask, void *pData, MGR_U32 lens) 
{
	MGR_CmdMsg_T msg ;
	
	msg.Dtid  = dTask ;
	msg.Stid  = appInfo.srcTask ;
	msg.CmdId = cmdId ;
	msg.Mask  = cmdMask ;
	
	memcpy((char *)&msg.u, (char *)pData, lens) ;
	dspch_shunt_cmd_request(&msg) ;

	return msg.Result ;
}

/**************************************************************************************************
**************************************************************************************************/
static inline MGR_Ret api_do_get_command(MGR_U16 dTask, MGR_U16 cmdId, MGR_U32 cmdMask, void *pData, MGR_U32 lens) 
{
	MGR_CmdMsg_T msg ;
	
	msg.Dtid  = dTask ;
	msg.Stid  = appInfo.srcTask ;
	msg.CmdId = cmdId ;
	msg.Mask  = cmdMask ;

	memcpy((char *)&msg.u, (char *)pData, lens) ;
	dspch_shunt_cmd_request(&msg) ;
	memcpy((char *)pData, (char *)&msg.u, lens) ;
	return msg.Result ;
}

/*****************************************************************************
*****************************************************************************/
/************************************
 performance management
*************************************/
/************************************
 performance management
*************************************/
MACRO_PONMGR_SET_API(TASK_TYPE_PMGR, PhyFecCounter)
MACRO_PONMGR_GET_API(TASK_TYPE_PMGR, PhyFecCounter)

MACRO_PONMGR_SET_API(TASK_TYPE_PMGR, PhyFrameCounter)
MACRO_PONMGR_GET_API(TASK_TYPE_PMGR, PhyFrameCounter)

/************************************
 configure management
*************************************/
//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, SysLinkConfig)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, SysLinkConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, SysLinkConfig)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, SysLinkConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, SysOnuType)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, SysOnuType)
//MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, SysOnuType)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, SysOnuType)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, SysOnuType)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, SysOnuType)
//MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, SysOnuType)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, SysClearConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, PhyFecConfig)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, PhyFecConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, PhyFecConfig)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, PhyFecConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, PhyTransConfig)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, PhyTransConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, PhyTransConfig)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, PhyTransConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, PhyTransParameters)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, PhyTransParameters)
//MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, PhyTransParameters)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, PhyTransParameters)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, PhyTxBurstConfig)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, PhyTxBurstConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, PhyTxBurstConfig)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, PhyTxBurstConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, PWanChannelQoS)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, PWanChannelQoS)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, PWanChannelQoS)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, PWanChannelQoS)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, PWanCongestConfig
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, PWanCongestConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, PWanCongestConfig)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, PWanCongestConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, PWanTrtcmConfig)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, PWanTrtcmConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, PWanTrtcmConfig)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, PWanTrtcmConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, PWanPcpConfig)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, PWanPcpConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, PWanPcpConfig)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, PWanPcpConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, PWanCntStats)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, PWanCntStats)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, PWanCntStats)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, PWanCntStats)

MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, StormCtrlConfig)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, StormCtrlConfig)


#ifdef TCSUPPORT_WAN_GPON
//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, GponSnPasswd)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, GponSnPasswd)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, GponSnPasswd)
//MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, GponSnPasswd)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, GponActTimer)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, GponActTimer)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, GponActTimer)
//MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, GponActTimer)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, GponSystemInfo)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, GponSystemInfo)
//MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, GponSystemInfo)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, GponSystemInfo)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, GponDbaConfig)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, GponDbaConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, GponDbaConfig)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, GponDbaConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, GponTodConfig)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, GponTodConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, GponTodConfig)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, GponTodConfig)

MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, GponGemPortConfig)
MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, GponGemPortConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, GponGemPortConfig)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, GponGemPortConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, GponTcontStatus)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, GponTcontStatus)
//MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, GponTcontStatus)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, GponTcontStatus)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, GponGemCounter)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, GponGemCounter)
//MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, GponGemCounter)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, GponGemCounter)
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, EponRxConfig)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, EponRxConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, EponRxConfig)
//MACRO_PONMGR_DELETE_DELETE(TASK_TYPE_CMGR, EponRxConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, EponTxConfig)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, EponTxConfig)
MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, EponTxConfig)
//MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, EponTxConfig)

//MACRO_PONMGR_CREATE_API(TASK_TYPE_CMGR, EponLlidStatus)
//MACRO_PONMGR_DELETE_API(TASK_TYPE_CMGR, EponLlidStatus)
//MACRO_PONMGR_SET_API(TASK_TYPE_CMGR, EponLlidStatus)
MACRO_PONMGR_GET_API(TASK_TYPE_CMGR, EponLlidStatus)
#endif /* TCSUPPORT_WAN_EPON */

/************************************
 fault management
*************************************/
//MACRO_PONMGR_API_FUNCTION(TASK_TYPE_FMGR, AlarmClear)
