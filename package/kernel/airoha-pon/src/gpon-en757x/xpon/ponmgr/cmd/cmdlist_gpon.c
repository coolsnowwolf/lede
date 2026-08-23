#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd_cmds.h"

CmdStruct gGponCmd[] = {
	/**********************************************
	**********************************************/
	{ "gemport" ,
		(CmdProc) cmdGponCreateGemPort ,
		"<GEM type(multicast/unicast)><GEM port id> <TCONT id>",
		"Create a GEM port.",
		kSysCmdCreate ,
		2, 3
	},
	/**********************************************
	**********************************************/
	{ "gemport" ,
		(CmdProc) cmdGponDeleteGemPort ,
		"<GEM port id>",
		"Delete a GEM port.",
		kSysCmdDelete ,
		1, 1
	},
	/**********************************************
	**********************************************/
	{ "snpasswd" ,
		(CmdProc) cmdGponSetSnPasswd ,
		"<Vendor ID(4 char.)> <Serial number(4 bytes, hex format)> <Password(<12 char.)>",
		"Set the serial number and password.",
		kSysCmdSet ,
		3, 3
	},
	{ "act_timer" ,
		(CmdProc) cmdGponSetActTimer ,
		"<TO1 timer(ms)> <TO2 timer(ms)>",
		"Set the ONU activation timer.",
		kSysCmdSet ,
		2, 2
	},
	{ "tod" ,
		(CmdProc) cmdGponSetTodCfg ,
		"<Superframe counter(30 bits integer)> <Second(32 bits integer)> <Nanosecond(32 bits integer)>",
		"Set the GPON ToD parameters.",
		kSysCmdSet ,
		3, 3
	},
	{ "block_size" ,
		(CmdProc) cmdGponSetDbaBlockSize ,
		"<block size>",
		"Set the GPON DBA block size.",
		kSysCmdSet ,
		1, 1
	},
	{ "trtcm_mode" ,
		(CmdProc) cmdGponSetTrtcmMode ,
		"<trtcm mode (0/1)>",
		"Set the GPON trTCM mode.",
		kSysCmdSet ,
		1, 1
	},
	{ "trtcm_scale" ,
		(CmdProc) cmdGponSetTrtcmScale ,
		"<trTCM scale (1/2/4/8/16/32/64.../32768)>",
		"Set the GPON trTCM scale value, the unit of trtcm scale is Byte.",
		kSysCmdSet ,
		1, 1
	},
	{ "trtcm_param" ,
		(CmdProc) cmdGponSetTrtcmParameters ,
		"<Channel ID(0~15)> <cir(0~16000)> <cps(0~65535)> <pir(cir~16000)> <pbs(0~65535)>",
		"Set the GPON trTCM parameters, the unit of CIR, PIR is 64Kbps .",
		kSysCmdSet ,
		5, 5
	},
	{ "gem_assign" ,
		(CmdProc) cmdGponSetGemPortAssign ,
		"<GEM port id> <MAC bridge interface(0~31)>",
		"Assign a GEM port to specific MAC bridge interface.",
		kSysCmdSet ,
		2, 2
	},
	{ "gem_unassign" ,
		(CmdProc) cmdGponSetGemPortUnassign ,
		"<GEM port id>",
		"Unassign a GEM port.",
		kSysCmdSet ,
		1, 1
	},
	{ "loopback" ,
		(CmdProc) cmdGponSetGemPortLoopback ,
		"<GEM port id> <Loopback mode(0/1)>",
		"Set a GEM port loopback mode.",
		kSysCmdSet ,
		2, 2
	},
	/**********************************************
	**********************************************/
	{ "info",
		(CmdProc) cmdGponGetInfo ,
		"",
		"Get the ONU detail information.",
		kSysCmdGet ,
		0, 0
	},
	{ "tod",
		(CmdProc) cmdGponGetCurrentTime ,
		"",
		"Get the current time of ONU MAC.",
		kSysCmdGet ,
		0, 0
	},
	{ "trtcm",
		(CmdProc) cmdGponGetDbaConfig ,
		"",
		"Show the gpon dba detail information.",
		kSysCmdGet ,
		0, 0
	},
	{ "gemport",
		(CmdProc) cmdGponGetGemPortInfo ,
		"",
		"Get the GPON GEM port information.",
		kSysCmdGet ,
		0, 0
	},
	{ "tcont",
		(CmdProc) cmdGponGetTcontInfo ,
		"",
		"Get the GPON TCONT information.",
		kSysCmdGet ,
		0, 0
	},
	{ "gemcounter",
		(CmdProc) cmdGponGetGemCounter ,
		"",
		"Get the GPON GEM counter information.",
		kSysCmdGet ,
		1, 1
	},
} ;
size_t gGponCmdNum = ((size_t)(sizeof(gGponCmd)/sizeof(CmdStruct))) ;

