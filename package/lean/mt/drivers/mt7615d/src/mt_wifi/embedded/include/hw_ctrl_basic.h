
/*==============================================*/
/*		Define HW Ctrl Handler and Mapping Table						  */
/*==============================================*/

typedef NTSTATUS(*HwCmdHdlr)(RTMP_ADAPTER * pAd, HwCmdQElmt * CMDQelmt);
typedef NTSTATUS(*HwFlagHdlr)(RTMP_ADAPTER * pAd);

typedef struct {
	UINT32 CmdID;
	HwCmdHdlr CmdHdlr;
	UINT32 RfCnt;
} HW_CMD_TABLE_T;


typedef struct {
	UINT32 FlagId;
	HwFlagHdlr FlagHdlr;
	UINT32 RfCnt;
} HW_FLAG_TABLE_T;


