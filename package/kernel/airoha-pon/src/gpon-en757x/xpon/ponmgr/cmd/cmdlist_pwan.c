#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd_cmds.h"

CmdStruct gPWanCmd[] = {
	/**********************************************
	**********************************************/
	{ "qos_weight" ,
		(CmdProc) cmdPWanSetQosWeight ,
		"<weight type(packet/byte)> <weight scale(64/16)>",
		"Set the channel QoS weight type and scale.",
		kSysCmdSet ,
		1, 2
	},
	{ "channel_qos" ,
		(CmdProc) cmdPWanSetChannelQos ,
		"<channel(0~15)> <QoS type(sp/wrr/spwrr7...)> <queue 0 weight(0~100, 255 mean don't care)>..<queue 7 weight>",
		"Set the channel QoS parameters.",
		kSysCmdSet ,
		10, 10
	},
	{ "congest_mode" ,
		(CmdProc) cmdPWanSetCongestMode ,
		"<trtcm congestion mode (0/1)> <dei drop congestion mode (0/1)> <threshold congestion mode (0/1)>",
		"Set the trTCM, DEI dropped and threshold congestion mode.",
		kSysCmdSet ,
		3, 3
	},
	{ "congest_scale" ,
		(CmdProc) cmdPWanSetCongestScale ,
		"<max scale(2/4/8/16)> <min scale(2/4/8/16)>",
		"Set the max and min congestion scale.",
		kSysCmdSet ,
		2, 2
	},
	{ "congest_drop" ,
		(CmdProc) cmdPWanSetCongestDropProbability ,
		"<gerrn drop probability (0~100)> <yellow drop probability (0~100)>",
		"Set the green and yellow packet dropped probability.",
		kSysCmdSet ,
		2, 2
	},
	{ "congest_threshold" ,
		(CmdProc) cmdPWanSetCongestThreshold ,
		"<queue id (0~7)> <green max threshold(0~15)> <green min(0~15)> <yellow max(0~15)> <yellow max(0~15)>",
		"Set the greem and yellow threshold for specific queue.",
		kSysCmdSet ,
		5, 5
	},
	{ "trtcm_scale" ,
		(CmdProc) cmdPWanSetTrtcmScale ,
		"<trTCM scale (1/2/4/8/16/32/64.../32768)>",
		"Set the trTCM scale value, the unit of trtcm scale is Byte.",
		kSysCmdSet ,
		1, 1
	},
	{ "trtcm_param" ,
		(CmdProc) cmdPWanSetTrtcmParameters ,
		"<traffic shaping ID (0~31)> <cir(0~16000)> <cps(0~65535)> <pir(cir~16000)> <pbs(0~65535)>",
		"Set the trTCM parameters, the unit of CIR, PIR is 64Kbps .",
		kSysCmdSet ,
		5, 5
	},
	{ "stormctrl",
		(CmdProc) cmdPWanSetStormCtrlConfig ,
		"<threshold (0~128)> <timer (100)> ",
		"Set the storm control threshold.",
		kSysCmdSet ,
		2, 2
	},	
	
	/**********************************************
	**********************************************/
	{ "channel_qos",
		(CmdProc) cmdPWanGetChannelQos ,
		"",
		"Show the channel QoS detail information.",
		kSysCmdGet ,
		0, 0
	},
	{ "congestion",
		(CmdProc) cmdPWanGetCongestion ,
		"",
		"Show the ONU congestion detail information.",
		kSysCmdGet ,
		0, 0
	},
	{ "trtcm",
		(CmdProc) cmdPWanGetTrtcm ,
		"",
		"Show the tx queue trTCM detail information.",
		kSysCmdGet ,
		0, 0
	},
	/**********************************************
	**********************************************/
	{ "stormctrl",
		(CmdProc) cmdPWanGetStormCtrlConfig ,
		"",
		"Get the storm control timer.",
		kSysCmdGet ,
		0, 0
	},	
} ;
size_t gPWanCmdNum = ((size_t)(sizeof(gPWanCmd)/sizeof(CmdStruct))) ;

