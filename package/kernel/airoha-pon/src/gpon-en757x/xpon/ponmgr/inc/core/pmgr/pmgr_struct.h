#ifndef _PMGR_STRUCT_H_
#define _PMGR_STRUCT_H_

#include "common/mgr_cmd.h"

MACRO_CMD_STRUCT(PhyFecCounter)
{
	MGR_U32			CorrBytes ;
	MGR_U32 		CorrCodeWords ;
	MGR_U32			UncorrCodeWords ;
	MGR_U32			TotalRxCodeWords ;
	MGR_U32			FecSeconds ;
} ;

MACRO_CMD_STRUCT(PhyFrameCounter)
{
	MGR_U32			RxFrameLow ;
	MGR_U32			RxFrameHigh ;
	MGR_U32			RxLosFrame ;
} ;

#endif /* _PMGR_STRUCT_H_ */

