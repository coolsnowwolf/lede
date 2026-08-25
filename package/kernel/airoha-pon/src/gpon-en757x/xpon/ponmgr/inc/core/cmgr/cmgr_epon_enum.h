#ifndef _CMGR_EPON_ENUM_H_
#define _CMGR_EPON_ENUM_H_

#include "common/mgr_cmd.h"

typedef enum {
	ENUM_EPON_RX_FORWARDING = 0, 
	ENUM_EPON_RX_DISCARD, 
	ENUM_EPON_RX_LOOPBACK
} MGR_EponRxMode_t ;

typedef enum {
	ENUM_EPON_TX_FORWARDING = 0, 
	ENUM_EPON_TX_DISCARD
} MGR_EponTxMode_t ;

#endif /* _CMGR_EPON_ENUM_H_ */

