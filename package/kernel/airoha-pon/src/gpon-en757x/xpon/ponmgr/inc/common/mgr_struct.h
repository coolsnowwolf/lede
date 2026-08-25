#ifndef _MGR_STRUCT_H_
#define _MGR_STRUCT_H_

#include "common/mgr_type.h"

#include "core/cmgr/cmgr_struct.h"
#include "core/fmgr/fmgr_struct.h"
#include "core/pmgr/pmgr_struct.h"

#define TASK_TYPE_UNKNOWN					(0)
#define TASK_TYPE_CFGMGR					(1<<0)
#define TASK_TYPE_OAM						(1<<1)
#define TASK_TYPE_OMCI						(1<<2)


#define TASK_TYPE_CMGR						(1<<0)
#define TASK_TYPE_PMGR						(1<<1)
#define TASK_TYPE_FMGR						(1<<2)
#define TASK_TYPE_IMGR						(1<<3)
#define TASK_TYPE_DBMGR						(1<<4)
#define TASK_TYPE_DISPATCHER				(1<<5)


#define MACRO_CMDMSG_STRUCT_UNION(name) 		\
			 MACRO_CMD_STRUCT(name)		o##name ;


typedef struct {
	MGR_Task	Stid ;       //source task
	MGR_Task	Dtid ;       //destination task
	MGR_Cmd		CmdId ;      //command/event identification
	MGR_Mask	Mask ;       //used by set or event mask
	MGR_U16		Length ;
	MGR_Ret		Result ;     //used by ack

	union {
//performance management
		MACRO_CMDMSG_STRUCT_UNION(PhyFecCounter) ;
		MACRO_CMDMSG_STRUCT_UNION(PhyFrameCounter) ;

//configure management
		MACRO_CMDMSG_STRUCT_UNION(SysLinkConfig) ;
		MACRO_CMDMSG_STRUCT_UNION(SysOnuType) ;	
		MACRO_CMDMSG_STRUCT_UNION(SysClearConfig) ;	

		MACRO_CMDMSG_STRUCT_UNION(PhyFecConfig) ;	
		MACRO_CMDMSG_STRUCT_UNION(PhyTransConfig) ;	
		MACRO_CMDMSG_STRUCT_UNION(PhyTransParameters) ;	
		MACRO_CMDMSG_STRUCT_UNION(PhyTxBurstConfig) ;	

		MACRO_CMDMSG_STRUCT_UNION(PWanChannelQoS) ;
		MACRO_CMDMSG_STRUCT_UNION(PWanCongestConfig) ;
		MACRO_CMDMSG_STRUCT_UNION(PWanTrtcmConfig) ;
		MACRO_CMDMSG_STRUCT_UNION(PWanPcpConfig) ;
		MACRO_CMDMSG_STRUCT_UNION(PWanCntStats) ;

		MACRO_CMDMSG_STRUCT_UNION(StormCtrlConfig) ;		

#ifdef TCSUPPORT_WAN_GPON
		MACRO_CMDMSG_STRUCT_UNION(GponSnPasswd) ;
		MACRO_CMDMSG_STRUCT_UNION(GponActTimer) ;
		MACRO_CMDMSG_STRUCT_UNION(GponSystemInfo) ;
		MACRO_CMDMSG_STRUCT_UNION(GponDbaConfig) ;
		MACRO_CMDMSG_STRUCT_UNION(GponTodConfig) ;
		MACRO_CMDMSG_STRUCT_UNION(GponGemPortConfig) ;
		MACRO_CMDMSG_STRUCT_UNION(GponTcontStatus) ;
		MACRO_CMDMSG_STRUCT_UNION(GponGemCounter) ;
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
		MACRO_CMDMSG_STRUCT_UNION(EponRxConfig) ;
		MACRO_CMDMSG_STRUCT_UNION(EponTxConfig) ;
		MACRO_CMDMSG_STRUCT_UNION(EponLlidStatus) ;
#endif /* TCSUPPORT_WAN_EPON */

//fault management
		MACRO_CMDMSG_STRUCT_UNION(SystemTrapInfo) ;
		MACRO_CMDMSG_STRUCT_UNION(AlarmClear);
	} u ;
} MGR_CmdMsg_T ;


#endif /* _MGR_STRUCT_H_ */

