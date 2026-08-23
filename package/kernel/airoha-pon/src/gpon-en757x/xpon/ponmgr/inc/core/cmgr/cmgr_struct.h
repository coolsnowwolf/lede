#ifndef _CMGR_STRUCT_H_
#define _CMGR_STRUCT_H_

#include "core/cmgr/cmgr_system_struct.h"
#include "core/cmgr/cmgr_pwan_struct.h"
#include "core/cmgr/cmgr_phy_struct.h"

#ifdef TCSUPPORT_WAN_GPON
	#include "core/cmgr/cmgr_gpon_struct.h"
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
	#include "core/cmgr/cmgr_epon_struct.h"
#endif /* TCSUPPORT_WAN_EPON */

#endif /* _CMGR_STRUCT_H_ */

