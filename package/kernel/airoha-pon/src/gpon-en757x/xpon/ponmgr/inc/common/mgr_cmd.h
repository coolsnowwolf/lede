#ifndef _MGR_CMD_H_
#define _MGR_CMD_H_

#define CONST_GROUP_CMD_NUM		8

/************************************************************
 Command ID Definition
************************************************************/
#define MACRO_CMDID_CREATE(name)		CmdId_##name##_Create
#define MACRO_CMDID_DELETE(name)		CmdId_##name##_Delete
#define MACRO_CMDID_GET(name)			CmdId_##name##_Get
#define MACRO_CMDID_SET(name)			CmdId_##name##_Set
	
#define MACRO_CMDID_COLLECTIONS(group, name) \
               CmdId_##name##_Create = group*CONST_GROUP_CMD_NUM+0, \
               CmdId_##name##_CreateAck, 	 \
               CmdId_##name##_Delete, 		 \
               CmdId_##name##_DeleteAck, 	 \
               CmdId_##name##_Get, 			 \
               CmdId_##name##_GetAck, 		 \
               CmdId_##name##_Set, 			 \
               CmdId_##name##_SetAck, 		 
                                             
typedef enum {
	/***************************************************
	* Performance management (0x000~0x0FF)
	***************************************************/
	MACRO_CMDID_COLLECTIONS(0x000, PhyFecCounter)
	MACRO_CMDID_COLLECTIONS(0x001, PhyFrameCounter)

	/***************************************************
	* Configuration management (0x100~0x4FF)
	***************************************************/
	MACRO_CMDID_COLLECTIONS(0x100, SysLinkConfig)
	MACRO_CMDID_COLLECTIONS(0x101, SysOnuType)
	MACRO_CMDID_COLLECTIONS(0x102, SysClearConfig)

	MACRO_CMDID_COLLECTIONS(0x110, PhyFecConfig)
	MACRO_CMDID_COLLECTIONS(0x111, PhyTransConfig)
	MACRO_CMDID_COLLECTIONS(0x112, PhyTransParameters)
	MACRO_CMDID_COLLECTIONS(0x113, PhyTxBurstConfig)

	MACRO_CMDID_COLLECTIONS(0x120, PWanChannelQoS)
	MACRO_CMDID_COLLECTIONS(0x121, PWanCongestConfig)
	MACRO_CMDID_COLLECTIONS(0x122, PWanTrtcmConfig)
	MACRO_CMDID_COLLECTIONS(0x123, PWanPcpConfig)
	MACRO_CMDID_COLLECTIONS(0x124, PWanCntStats)

	MACRO_CMDID_COLLECTIONS(0x125, StormCtrlConfig)

#ifdef TCSUPPORT_WAN_GPON
	MACRO_CMDID_COLLECTIONS(0x200, GponSnPasswd)
	MACRO_CMDID_COLLECTIONS(0x201, GponActTimer)
	MACRO_CMDID_COLLECTIONS(0x202, GponSystemInfo)
	MACRO_CMDID_COLLECTIONS(0x203, GponDbaConfig)
	MACRO_CMDID_COLLECTIONS(0x204, GponTodConfig)
	MACRO_CMDID_COLLECTIONS(0x206, GponGemPortConfig)
	MACRO_CMDID_COLLECTIONS(0x207, GponTcontStatus)
	MACRO_CMDID_COLLECTIONS(0x208, GponGemCounter)
#endif /* TCSUPPORT_WAN_GPON */
	
#ifdef TCSUPPORT_WAN_EPON
	MACRO_CMDID_COLLECTIONS(0x303, EponRxConfig)
	MACRO_CMDID_COLLECTIONS(0x304, EponTxConfig)
	MACRO_CMDID_COLLECTIONS(0x305, EponLlidStatus)
#endif /* TCSUPPORT_WAN_EPON */
	/***************************************************
	* Log management (0x500~0x53F)
	***************************************************/
	MACRO_CMDID_COLLECTIONS(0x502, LogMsgConfig)

	/***************************************************
	* Fault management (0x540~0x57F)
	***************************************************/
	MACRO_CMDID_COLLECTIONS(0x540, AlarmClear)

	/***************************************************
	* Maintain management (0x580~0x5AF)
	***************************************************/
	MACRO_CMDID_COLLECTIONS(0x585, TrapAppRegister)
	MACRO_CMDID_COLLECTIONS(0x586, TrapAppReport)
} PONMGR_CmdId_t ;



/************************************************************
 Command Struct Definition
************************************************************/
#define MACRO_CMD_STRUCT(name)			struct PONMGR_##name##_S


/************************************************************
 Command Mask Definition
************************************************************/
#define MACRO_CMD_MASK(name)			enum PONMGR_##name##_MASK

#define MASK_ALL 						0xFFFF
#define MASK_NONE						0

/************************************************************
 Command Process definition
************************************************************/
#define MACRO_CMD_PROC_FUNCTION(name)			MGR_Ret Cmd_##name##_Proc(MGR_CmdMsg_T *pMsg)

#define MACRO_CMD_PROC_SWITCH(name) \
						case (MACRO_CMDID_CREATE(name)/CONST_GROUP_CMD_NUM): \
							ret = Cmd_##name##_Proc(pMsg) ; \
							break ;

#endif /* _MGR_CMD_H_ */

