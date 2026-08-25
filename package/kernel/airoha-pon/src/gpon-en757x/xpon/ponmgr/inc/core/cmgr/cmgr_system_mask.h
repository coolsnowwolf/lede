#ifndef _CMGR_SYSTEM_MASK_H_
#define _CMGR_SYSTEM_MASK_H_

#include "common/mgr_cmd.h"

MACRO_CMD_MASK(SysLinkConfig)
{	
	MASK_SysLinkStart					= (1<<0) ,
	MASK_SysWanDetectMode				= (1<<1)
} ;

MACRO_CMD_MASK(SysClearConfig)
{	
	MASK_SysClearXpon					= (1<<0) 
} ;
#endif /* _CMGR_SYSTEM_MASK_H_ */
