#ifndef _CMGR_GPON_MASK_H_
#define _CMGR_GPON_MASK_H_

#include "common/mgr_cmd.h"


MACRO_CMD_MASK(GponGemPortConfig)
{
	MASK_AssignMacBridge				= (1<<0) ,
	MASK_UnassignMacBridge				= (1<<1) ,
	MASK_Loopback						= (1<<2)
} ;

MACRO_CMD_MASK(GponDbaConfig)
{
	MASK_GponBlockSize					= (1<<0) ,
	MASK_GponTrtcmMode					= (1<<1) ,
	MASK_GponTrtcmScale					= (1<<2) ,
	MASK_GponTrtcmParams				= (1<<3)
} ;

#endif /* _CMGR_GPON_MASK_H_ */
