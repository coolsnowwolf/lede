#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <utime.h>
#include <dirent.h>

#include "common/mgr_pthread.h"
#include "common/mgr_cmd.h"
#include "common/mgr_util.h"
#include "common/mgr_sdi.h"

#include "core/fmgr/fmgr.h"
#include "core/cmgr/cmgr.h"
#include "core/dbmgr/dbmgr.h"

#if 0
/***********************************************************************************
MACRO_CMD_STRUCT(GponSnPasswd)
{
	MGR_U8 		sn[CONST_GPON_SN_LENS] ;
	MGR_U8 		passwd[CONST_GPON_PASSWD_LENS] ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(GponSnPasswd)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(GponSnPasswd) *pObj = &pMsg->u.oGponSnPasswd ;
	
	switch(pMsg->CmdId) {
		case MACRO_CMDID_CREATE(GponSnPasswd) :		
			break ;
		case MACRO_CMDID_DELETE(GponSnPasswd) :
			break ;
		case MACRO_CMDID_SET(GponSnPasswd) :
			break ;
		case MACRO_CMDID_GET(GponSnPasswd) :
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(GponSnPasswd, ret) ;
}
#endif /* 0 */

/***********************************************************************************
MACRO_CMD_STRUCT(EponRxConfig)
{
	MGR_U8				LlidIdx ;
	MGR_EponRxMode_t	RxMode ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(EponRxConfig)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(EponRxConfig) *pObj = &pMsg->u.oEponRxConfig ;
	struct XMCS_EponRxConfig_S eponRxConfig ;
	
	switch(pMsg->CmdId) {
		//case MACRO_CMDID_CREATE(EponRxConfig) :		
		//	break ;
		//case MACRO_CMDID_DELETE(EponRxConfig) :		
		//	break ;
		case MACRO_CMDID_SET(EponRxConfig) :
			if(pObj->LlidIdx >= CONFIG_EPON_MAX_LLID) {
				ret = EXEC_ERR_PARAM_INVAL ;
			} else {
				eponRxConfig.idx = pObj->LlidIdx ;
				if(pObj->RxMode == ENUM_EPON_RX_FORWARDING) {
					eponRxConfig.rxMode = EPON_RX_FORWARDING ;
				} else if(pObj->RxMode == ENUM_EPON_RX_LOOPBACK) {
					eponRxConfig.rxMode = EPON_RX_LOOPBACK ;
				} else {
					eponRxConfig.rxMode = EPON_RX_DISCARD ;
				}
				ret = XMCS_IOCTL_SDI(IF_IOS_LLID_RX_CFG, &eponRxConfig) ;
			}
			
			break ;
		//case MACRO_CMDID_GET(EponRxConfig) :		
		//	break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(EponRxConfig, ret) ;
}

/***********************************************************************************
MACRO_CMD_STRUCT(EponTxConfig)
{
	MGR_U8				LlidIdx ;
	MGR_EponTxMode_t	TxMode ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(EponTxConfig)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(EponTxConfig) *pObj = &pMsg->u.oEponTxConfig ;
	struct XMCS_EponTxConfig_S eponTxConfig ;
	
	switch(pMsg->CmdId) {
		//case MACRO_CMDID_CREATE(EponTxConfig) :		
		//	break ;
		//case MACRO_CMDID_DELETE(EponTxConfig) :		
		//	break ;
		case MACRO_CMDID_SET(EponTxConfig) :
			if(pObj->LlidIdx >= CONFIG_EPON_MAX_LLID) {
				ret = EXEC_ERR_PARAM_INVAL ;
			} else {
				eponTxConfig.idx = pObj->LlidIdx ;
				if(pObj->TxMode == ENUM_EPON_TX_FORWARDING) {
					eponTxConfig.txMode = EPON_TX_FORWARDING ;
				} else {
					eponTxConfig.txMode = EPON_TX_DISCARD ;
				}
				ret = XMCS_IOCTL_SDI(IF_IOS_LLID_TX_CFG, &eponTxConfig) ;
			}
			break ;
		//case MACRO_CMDID_GET(EponTxConfig) :		
		//	break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(EponTxConfig, ret) ;
}

/***********************************************************************************
MACRO_CMD_STRUCT(EponLlidStatus)
{
	struct {
		MGR_U8				LlidIdx ;
		MGR_U16				Llid ;
		MGR_U8				Channel ;		
		MGR_EponRxMode_t	RxMode ;		
		MGR_EponTxMode_t	TxMode ;	
	} LlidInfo[CONFIG_EPON_MAX_LLID] ;
	MGR_U8		EntryNum ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(EponLlidStatus)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(EponLlidStatus) *pObj = &pMsg->u.oEponLlidStatus ;
	struct XMCS_EponLlidInfo_S eponLlidInfo ;
	int i ;
	
	switch(pMsg->CmdId) {
		//case MACRO_CMDID_CREATE(EponLlidStatus) :		
		//	break ;
		//case MACRO_CMDID_DELETE(EponLlidStatus) :		
		//	break ;
		//case MACRO_CMDID_SET(EponLlidStatus) :
		//	break ;
		case MACRO_CMDID_GET(EponLlidStatus) :		
			ret = XMCS_IOCTL_SDI(IF_IOG_LLID_INFO, &eponLlidInfo) ;
			if(ret == 0) {
				if(eponLlidInfo.entryNum > CONFIG_EPON_MAX_LLID) {
					ret = EXEC_ERR ;
				} else {
					for(i=0 ; i<eponLlidInfo.entryNum ; i++) {
						pObj->LlidInfo[i].LlidIdx = eponLlidInfo.info[i].idx ;	
						pObj->LlidInfo[i].Llid = eponLlidInfo.info[i].llid ;	
						pObj->LlidInfo[i].Channel = eponLlidInfo.info[i].channel ;	
						if(eponLlidInfo.info[i].rxMode == EPON_RX_FORWARDING) {
							pObj->LlidInfo[i].RxMode = ENUM_EPON_RX_FORWARDING ;
						} else if(eponLlidInfo.info[i].rxMode == EPON_RX_LOOPBACK) {
							pObj->LlidInfo[i].RxMode = ENUM_EPON_RX_LOOPBACK ;
						} else {
							pObj->LlidInfo[i].RxMode = ENUM_EPON_RX_DISCARD ;							
						}

						if(eponLlidInfo.info[i].txMode == EPON_TX_FORWARDING) {
							pObj->LlidInfo[i].TxMode = ENUM_EPON_TX_FORWARDING ;
						} else {
							pObj->LlidInfo[i].TxMode = ENUM_EPON_TX_DISCARD ;							
						}
					}
					pObj->EntryNum = eponLlidInfo.entryNum ;
				}
			}
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(EponLlidStatus, ret) ;
}
