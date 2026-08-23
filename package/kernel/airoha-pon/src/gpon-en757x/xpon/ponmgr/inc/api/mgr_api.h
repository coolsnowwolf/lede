#ifndef _MGR_API_H_
#define _MGR_API_H_

#include "common/mgr_type.h"
#include "common/mgr_ipc.h"
#include "common/mgr_sem.h"
#include "common/mgr_cmd.h"
#include "common/mgr_util.h"
#include "core/cmgr/cmgr.h"
#include "core/fmgr/fmgr.h"
#include "core/pmgr/pmgr.h"
#include "core/dbmgr/dbmgr.h"

struct PONAPI_AppInfo_S 
{
	MGR_U32		pid ;
	MGR_U32		srcTask ;
	MGR_U32		trapFlag ;
} ;

extern struct PONAPI_AppInfo_S appInfo ;
extern struct DBMGR_Database_S *gpSysDbAddr ;

/******************************************************************************************
 command api
******************************************************************************************/
#define MACRO_PONMGR_CREATE_API_PROTOTYPE(name) 	MGR_Ret PonApi_##name##_Create(MACRO_CMD_STRUCT(name) *pData)
#define MACRO_PONMGR_DELETE_API_PROTOTYPE(name)		MGR_Ret PonApi_##name##_Delete(MACRO_CMD_STRUCT(name) *pData)
#define MACRO_PONMGR_GET_API_PROTOTYPE(name)		MGR_Ret PonApi_##name##_Get(MACRO_CMD_STRUCT(name) *pData)
#define MACRO_PONMGR_SET_API_PROTOTYPE(name)		MGR_Ret PonApi_##name##_Set(MGR_Mask cmdMask, MACRO_CMD_STRUCT(name) *pData)


/************************************
 performance management
*************************************/
MACRO_PONMGR_SET_API_PROTOTYPE(PhyFecCounter) ;
MACRO_PONMGR_GET_API_PROTOTYPE(PhyFecCounter) ;

MACRO_PONMGR_SET_API_PROTOTYPE(PhyFrameCounter) ;
MACRO_PONMGR_GET_API_PROTOTYPE(PhyFrameCounter) ;

/************************************
 configure management
*************************************/
//MACRO_PONMGR_CREATE_API_PROTOTYPE(SysLinkConfig) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(SysLinkConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(SysLinkConfig) ;
MACRO_PONMGR_GET_API_PROTOTYPE(SysLinkConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(SysOnuType) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(SysOnuType) ;
//MACRO_PONMGR_SET_API_PROTOTYPE(SysOnuType) ;
MACRO_PONMGR_GET_API_PROTOTYPE(SysOnuType) ;
MACRO_PONMGR_SET_API_PROTOTYPE(SysClearConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(PhyFecConfig) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(PhyFecConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(PhyFecConfig) ;
MACRO_PONMGR_GET_API_PROTOTYPE(PhyFecConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(PhyTransConfig) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(PhyTransConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(PhyTransConfig) ;
MACRO_PONMGR_GET_API_PROTOTYPE(PhyTransConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(PhyTransParameters) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(PhyTransParameters) ;
//MACRO_PONMGR_SET_API_PROTOTYPE(PhyTransParameters) ;
MACRO_PONMGR_GET_API_PROTOTYPE(PhyTransParameters) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(PhyTxBurstConfig) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(PhyTxBurstConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(PhyTxBurstConfig) ;
MACRO_PONMGR_GET_API_PROTOTYPE(PhyTxBurstConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(PWanChannelQoS) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(PWanChannelQoS) ;
MACRO_PONMGR_SET_API_PROTOTYPE(PWanChannelQoS) ;
MACRO_PONMGR_GET_API_PROTOTYPE(PWanChannelQoS) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(PWanCongestConfig) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(PWanCongestConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(PWanCongestConfig) ;
MACRO_PONMGR_GET_API_PROTOTYPE(PWanCongestConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(PWanTrtcmConfig) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(PWanTrtcmConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(PWanTrtcmConfig) ;
MACRO_PONMGR_GET_API_PROTOTYPE(PWanTrtcmConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(PWanPcpConfig) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(PWanPcpConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(PWanPcpConfig) ;
MACRO_PONMGR_GET_API_PROTOTYPE(PWanPcpConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(PWanCntStats) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(PWanCntStats) ;
MACRO_PONMGR_SET_API_PROTOTYPE(PWanCntStats) ;
MACRO_PONMGR_GET_API_PROTOTYPE(PWanCntStats) ;

MACRO_PONMGR_SET_API_PROTOTYPE(StormCtrlConfig) ;
MACRO_PONMGR_GET_API_PROTOTYPE(StormCtrlConfig) ;


#ifdef TCSUPPORT_WAN_GPON
//MACRO_PONMGR_CREATE_API_PROTOTYPE(GponSnPasswd) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(GponSnPasswd) ;
MACRO_PONMGR_SET_API_PROTOTYPE(GponSnPasswd) ;
//MACRO_PONMGR_DELETE_DELETE_PROTOTYPE(GponSnPasswd) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(GponActTimer) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(GponActTimer) ;
MACRO_PONMGR_SET_API_PROTOTYPE(GponActTimer) ;
//MACRO_PONMGR_GET_API_PROTOTYPE(GponActTimer) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(GponSystemInfo) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(GponSystemInfo) ;
//MACRO_PONMGR_SET_API_PROTOTYPE(GponSystemInfo) ;
MACRO_PONMGR_GET_API_PROTOTYPE(GponSystemInfo) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(GponTodConfig) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(GponTodConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(GponTodConfig) ;
MACRO_PONMGR_GET_API_PROTOTYPE(GponTodConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(GponDbaConfig) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(GponDbaConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(GponDbaConfig) ;
MACRO_PONMGR_GET_API_PROTOTYPE(GponDbaConfig) ;

MACRO_PONMGR_CREATE_API_PROTOTYPE(GponGemPortConfig) ;
MACRO_PONMGR_DELETE_API_PROTOTYPE(GponGemPortConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(GponGemPortConfig) ;
MACRO_PONMGR_GET_API_PROTOTYPE(GponGemPortConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(GponTcontStatus) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(GponTcontStatus) ;
//MACRO_PONMGR_SET_API_PROTOTYPE(GponTcontStatus) ;
MACRO_PONMGR_GET_API_PROTOTYPE(GponTcontStatus) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(GponGemCounter) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(GponGemCounter) ;
//MACRO_PONMGR_SET_API_PROTOTYPE(GponGemCounter) ;
MACRO_PONMGR_GET_API_PROTOTYPE(GponGemCounter) ;
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
//MACRO_PONMGR_CREATE_API_PROTOTYPE(EponRxConfig) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(EponRxConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(EponRxConfig) ;
//MACRO_PONMGR_GET_API_PROTOTYPE(EponRxConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(EponTxConfig) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(EponTxConfig) ;
MACRO_PONMGR_SET_API_PROTOTYPE(EponTxConfig) ;
//MACRO_PONMGR_GET_API_PROTOTYPE(EponTxConfig) ;

//MACRO_PONMGR_CREATE_API_PROTOTYPE(EponLlidStatus) ;
//MACRO_PONMGR_DELETE_API_PROTOTYPE(EponLlidStatus) ;
//MACRO_PONMGR_SET_API_PROTOTYPE(EponLlidStatus) ;
MACRO_PONMGR_GET_API_PROTOTYPE(EponLlidStatus) ;
#endif /* TCSUPPORT_WAN_EPON */
	
/************************************
 fault management
*************************************/
//MACRO_PONMGR_API_PROTOTYPE(AlarmClear)

/******************************************************************************************
 initial api
******************************************************************************************/
MGR_Ret ponapi_init(int xponmode) ;
MGR_Ret ponapi_destory(void) ;
MGR_Ret ponapi_trap_init(MGR_U16 srcTask, MGR_Ret (*trapCallbackFun)(MACRO_CMD_STRUCT(SystemTrapInfo) *)) ;
MGR_Ret ponapi_trap_destory(void) ;

#endif /* _MGR_API_H_ */
