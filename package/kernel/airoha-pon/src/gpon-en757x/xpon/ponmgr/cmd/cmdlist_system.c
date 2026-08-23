#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd_cmds.h"

CmdStruct gSystemCmd[] = {
	/**********************************************
	**********************************************/
	{ "startup" ,
		(CmdProc) cmdSystemSetStartMode ,
		"<connection start(start/stop)>",
		"Start/stop the xpon connection.",
		kSysCmdSet ,
		1, 1
	},
	{ "detect" ,
		(CmdProc) cmdSystemSetWanDetection ,
		"<wan detection(auto/gpon/epon)>",
		"Set the WAN detection mode. The WAN must be stop when using this command.",
		kSysCmdSet ,
		1, 1
	},
	{ "clear" ,
		(CmdProc) cmdSystemClearConfig ,
		"<xpon>",
		"set the xpon modules to 0.should rmmod xpon_imgp/ponmacfilter/ponvlan/xpon_map first",
		kSysCmdSet ,
		1, 1
	},
	/**********************************************
	**********************************************/
	{ "link_mode",
		(CmdProc) cmdSystemGetLinkMode ,
		"",
		"Get the ONU link mode.",
		kSysCmdGet ,
		0, 0
	},
	{ "onu_type",
		(CmdProc) cmdSystemGetOnuType ,
		"",
		"Get the ONU type.",
		kSysCmdGet ,
		0, 0
	},

	
} ;
size_t gSystemCmdNum = ((size_t)(sizeof(gSystemCmd)/sizeof(CmdStruct))) ;

