#ifndef _CMGR_CONST_H_
#define _CMGR_CONST_H_

#include "core/cmgr/cmgr_system_const.h"
#include "core/cmgr/cmgr_pwan_const.h"
#include "core/cmgr/cmgr_phy_const.h"
	
#ifdef TCSUPPORT_WAN_GPON
	#include "core/cmgr/cmgr_gpon_const.h"
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
	#include "core/cmgr/cmgr_epon_const.h"
#endif /* TCSUPPORT_WAN_EPON */

#endif /* _CMGR_CONST_H_ */
