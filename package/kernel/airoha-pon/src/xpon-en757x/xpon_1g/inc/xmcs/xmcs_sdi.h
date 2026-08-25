#ifndef _XMCS_SDI_H_
#define _XMCS_SDI_H_

#include "xmcs/xmcs_phy.h"

#ifdef TCSUPPORT_WAN_EPON
	#include "xmcs/xmcs_epon.h"
#endif /* TCSUPPORT_WAN_EPON */

#ifdef TCSUPPORT_WAN_GPON
	#include "xmcs/xmcs_gpon.h"
#endif /* TCSUPPORT_WAN_GPON */

#include "xmcs/xmcs_fdet.h"
#include "xmcs/xmcs_if.h"

#endif /* _XMCS_SDI_H_ */
