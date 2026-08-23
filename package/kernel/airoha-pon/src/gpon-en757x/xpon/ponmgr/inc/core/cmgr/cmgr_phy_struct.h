#ifndef _CMGR_PHY_STRUCT_H_
#define _CMGR_PHY_STRUCT_H_

#include "core/cmgr/cmgr_enum.h"

MACRO_CMD_STRUCT(PhyFecConfig)
{
	MGR_Enable_t				RxFecMode ;
	MGR_Enable_t				RxFecStatus ;
} ;

MACRO_CMD_STRUCT(PhyTransConfig)
{
	MGR_Enable_t				TxSdInverse ;
	MGR_Enable_t				TxFaultInverse ;
	MGR_Enable_t				TxBurstEnInverse ;
	MGR_Enable_t				RxSdInverse ;
} ;

MACRO_CMD_STRUCT(PhyTransParameters)
{
	ushort					Temperature ;
	ushort					Voltage ;
	ushort					TxCurrent ;
	ushort					TxPower ;
	ushort					RxPower ;
} ;	

MACRO_CMD_STRUCT(PhyTxBurstConfig)
{
	MGR_PhyTxBurstMode_t	BurstMode ;
} ;	

#endif /* _CMGR_PHY_STRUCT_H_ */
