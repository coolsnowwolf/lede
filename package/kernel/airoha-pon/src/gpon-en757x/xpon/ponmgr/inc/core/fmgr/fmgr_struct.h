#ifndef _FMGR_STRUCT_H_
#define _FMGR_STRUCT_H_

#include "common/mgr_cmd.h"
#include "core/fmgr/fmgr_event.h"

//system event
MACRO_CMD_STRUCT(SystemTrapInfo)
{
	FMGR_PonTrapIndex_t		trapIndex ;
	MGR_U32					trapValue ;
} ;


MACRO_CMD_STRUCT(AlarmClear)
{
} ;

#endif /*_FMGR_STRUCT_H_*/
