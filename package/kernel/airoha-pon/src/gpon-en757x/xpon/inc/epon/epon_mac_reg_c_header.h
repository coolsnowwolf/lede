#ifndef __EPON_MAC_REG_C_HEADER_H__
#define __EPON_MAC_REG_C_HEADER_H__

#ifdef TCSUPPORT_CPU_EN7521
#include "epon_mac_reg_c_header_en7521.h"
#else
#include "epon_mac_reg_c_header_mt7520.h"
#endif

/*epon globle mac control*/
#define	MAC_CONTROL_MPCP_FWD					(1<<22)

#endif // __EPON_MAC_REG_C_HEADER_H__
