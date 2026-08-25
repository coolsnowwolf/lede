#ifndef _CMGR_PROC_H_
#define _CMGR_PROC_H_

#include "core/cmgr/cmgr_system_proc.h"
#include "core/cmgr/cmgr_pwan_proc.h"
#include "core/cmgr/cmgr_phy_proc.h"

#ifdef TCSUPPORT_WAN_GPON
	#include "core/cmgr/cmgr_gpon_proc.h"
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
	#include "core/cmgr/cmgr_epon_proc.h"
#endif /* TCSUPPORT_WAN_EPON */


#endif /* _CMGR_PROC_H_ */
