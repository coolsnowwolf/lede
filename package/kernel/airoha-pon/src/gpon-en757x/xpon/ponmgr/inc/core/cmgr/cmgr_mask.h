#ifndef _CMGR_MASK_H_
#define _CMGR_MASK_H_

#include "core/cmgr/cmgr_system_mask.h"
#include "core/cmgr/cmgr_pwan_mask.h"
#include "core/cmgr/cmgr_phy_mask.h"

#ifdef TCSUPPORT_WAN_GPON
	#include "core/cmgr/cmgr_gpon_mask.h"
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
	#include "core/cmgr/cmgr_epon_mask.h"
#endif /* TCSUPPORT_WAN_EPON */


#endif /* _CMGR_MASK_H_ */
