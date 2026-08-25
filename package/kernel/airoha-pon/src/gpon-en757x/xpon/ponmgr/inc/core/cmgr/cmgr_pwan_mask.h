#ifndef _CMGR_PWAN_MASK_H_
#define _CMGR_PWAN_MASK_H_

#include "common/mgr_cmd.h"

MACRO_CMD_MASK(PWanChannelQoS)
{
	MASK_ChannelQosWeightParameters		= (1<<0) ,
	MASK_ChannelQosConfig				= (1<<1)
} ;

MACRO_CMD_MASK(PWanCongestConfig)
{
	MASK_CongestTrtcmMode				= (1<<0) ,
	MASK_CongestDeiDropMode				= (1<<1) ,
	MASK_CongestThresholdMode			= (1<<2) ,
	MASK_CongestScale					= (1<<3) ,
	MASK_CongestDropProbability			= (1<<4) ,
	MASK_CongestThreshold				= (1<<5)
} ;

MACRO_CMD_MASK(PWanTrtcmConfig)
{
	MASK_TrtcmScale						= (1<<0) ,
	MASK_TrtcmParameters				= (1<<1) 
} ;

MACRO_CMD_MASK(PWanPcpConfig)
{
	MASK_PcpCdmTxEncoding				= (1<<0) ,
	MASK_PcpCdmRxDecoding				= (1<<1) ,
	MASK_PcpGdmRxDecoding				= (1<<2) 
} ;

MACRO_CMD_MASK(StormCtrlConfig)
{
	MASK_StormThreld						= (1<<0) ,
	MASK_StormTimer				= (1<<1) 
} ;


#endif /* _CMGR_PWAN_MASK_H_ */
