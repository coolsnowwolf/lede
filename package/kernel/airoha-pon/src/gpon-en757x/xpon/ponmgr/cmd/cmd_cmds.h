#ifndef _CMD_CMDS_H_
#define _CMD_CMDS_H_

#include "api/mgr_api.h"
#include "cmd_errno.h"

#define kSysCmdCreate					0x000001
#define kSysCmdDelete					0x000002
#define kSysCmdSet						0x000004
#define kSysCmdGet					0x000008

typedef void (*CmdProc)(const int argc, char **const argv);

typedef struct {
	const char *name ;
	CmdProc proc ;
	const char *args ;
	const char *usage ;
	int flags ;
	char minArgs ;
	char maxArgs ;
} CmdStruct ;

extern CmdStruct gSystemCmd[] ;
extern size_t gSystemCmdNum ;

extern CmdStruct gPWanCmd[] ;
extern size_t gPWanCmdNum ;

extern CmdStruct gGponCmd[] ;
extern size_t gGponCmdNum ;



/********************************************************************************************
*********************************************************************************************
********************************************************************************************/
//system information function
void cmdSystemSetStartMode(const int argc, char **const argv) ;
void cmdSystemSetWanDetection(const int argc, char **const argv) ;
void cmdSystemGetLinkMode(const int argc, char **const argv) ;
void cmdSystemGetOnuType(const int argc, char **const argv) ;
void cmdSystemClearConfig(const int argc, char **const argv) ;



/********************************************************************************************
*********************************************************************************************
********************************************************************************************/
//GPON configuration function
void cmdGponGetInfo(const int argc, char **const argv) ;
void cmdGponGetCurrentTime(const int argc, char **const argv) ;
void cmdGponGetDbaConfig(const int argc, char **const argv) ;
void cmdGponGetGemPortInfo(const int argc, char **const argv) ;
void cmdGponGetTcontInfo(const int argc, char **const argv) ;

void cmdGponSetSnPasswd(const int argc, char **const argv) ;
void cmdGponSetActTimer(const int argc, char **const argv) ;
void cmdGponSetTodCfg(const int argc, char **const argv) ;
void cmdGponSetDbaBlockSize(const int argc, char **const argv) ;
void cmdGponSetTrtcmMode(const int argc, char **const argv) ;
void cmdGponSetTrtcmScale(const int argc, char **const argv) ;
void cmdGponSetTrtcmParameters(const int argc, char **const argv) ;
void cmdGponSetGemPortAssign(const int argc, char **const argv) ;
void cmdGponSetGemPortUnassign(const int argc, char **const argv) ;
void cmdGponSetGemPortLoopback(const int argc, char **const argv) ;

void cmdGponCreateInterface(const int argc, char **const argv) ;
void cmdGponCreateGemPort(const int argc, char **const argv) ;

void cmdGponDeleteInterface(const int argc, char **const argv) ;
void cmdGponDeleteGemPort(const int argc, char **const argv) ;
void cmdGponGetGemCounter(const int argc, char **const argv) ;

//PWAN configuration function
void cmdPWanGetChannelQos(const int argc, char **const argv) ;
void cmdPWanGetCongestion(const int argc, char **const argv) ;
void cmdPWanGetTrtcm(const int argc, char **const argv) ;
void cmdPWanGetStormCtrlConfig(const int argc, char **const argv) ;

void cmdPWanSetQosWeight(const int argc, char **const argv) ;
void cmdPWanSetChannelQos(const int argc, char **const argv) ;
void cmdPWanSetCongestMode(const int argc, char **const argv) ;
void cmdPWanSetCongestScale(const int argc, char **const argv) ;
void cmdPWanSetCongestDropProbability(const int argc, char **const argv) ;
void cmdPWanSetCongestThreshold(const int argc, char **const argv) ;
void cmdPWanSetTrtcmScale(const int argc, char **const argv) ;
void cmdPWanSetTrtcmParameters(const int argc, char **const argv) ;

void cmdPWanSetStormCtrlConfig(const int argc, char **const argv) ;



#endif /* _CMD_CMDS_H_ */
