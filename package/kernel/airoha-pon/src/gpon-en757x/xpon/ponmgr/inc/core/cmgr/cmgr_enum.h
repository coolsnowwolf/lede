#ifndef _CMGR_ENUM_H_
#define _CMGR_ENUM_H_

#include "core/cmgr/cmgr_system_enum.h"
#include "core/cmgr/cmgr_pwan_enum.h"
#include "core/cmgr/cmgr_phy_enum.h"

#ifdef TCSUPPORT_WAN_GPON
	#include "core/cmgr/cmgr_gpon_enum.h"
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
	#include "core/cmgr/cmgr_epon_enum.h"
#endif /* TCSUPPORT_WAN_EPON */

/* keep same as enum XPON_Mode_t in file modules\private\xpon\inc\common\drv_types.h !!!!*/
typedef enum {
	ENUM_DISABLE = 0,
	ENUM_ENABLE, 
	ENUM_POWRDOWN, 
	ENUM_OTHER
} MGR_Enable_t ;



#endif /* _CMGR_ENUM_H_ */
