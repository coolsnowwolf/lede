#ifndef _CMGR_SYSTEM_STRUCT_H_
#define _CMGR_SYSTEM_STRUCT_H_

#include "core/cmgr/cmgr_enum.h"

MACRO_CMD_STRUCT(SysLinkConfig)
{
	MGR_Enable_t				LinkStart ;
	MGR_SysWanDetectMode_t		WanDetectMode ;
	MGR_SysLinkStatus_t			LinkStatus ;
} ;
MACRO_CMD_STRUCT(SysOnuType)
{
	MGR_SysOnuType_t			OnuType;
};

MACRO_CMD_STRUCT(SysClearConfig)
{
	MGR_SysClearType_t			ClearType;
};
#endif /* _CMGR_SYSTEM_STRUCT_H_ */
