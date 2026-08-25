#ifndef _CMGR_EPON_STRUCT_H_
#define _CMGR_EPON_STRUCT_H_

#include "core/cmgr/cmgr_epon_enum.h"
#include "core/cmgr/cmgr_epon_const.h"

MACRO_CMD_STRUCT(EponRxConfig)
{
	MGR_U8				LlidIdx ;
	MGR_EponRxMode_t	RxMode ;
} ;


MACRO_CMD_STRUCT(EponTxConfig)
{
	MGR_U8				LlidIdx ;
	MGR_EponTxMode_t	TxMode ;
} ;

MACRO_CMD_STRUCT(EponLlidStatus)
{
	struct {
		MGR_U8				LlidIdx ;
		MGR_U16				Llid ;
		MGR_U8				Channel ;		
		MGR_EponRxMode_t	RxMode ;		
		MGR_EponTxMode_t	TxMode ;	
	} LlidInfo[CONFIG_EPON_MAX_LLID] ;
	MGR_U8		EntryNum ;
} ;

#endif /* _CMGR_EPON_STRUCT_H_ */
