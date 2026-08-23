#ifndef _GPON_REG_H_
#define _GPON_REG_H_

#include "gpon_mac_reg_c_header.h"

#define G_ACTIVATION							(0xBFB640BC)

#define CONFIG_GPON_BASE_ADDR					(0x1FB60000)
#define CONFIG_GPON_REG_RANGE					(sizeof(gpon_mac_reg_REGS))


#define GPON_INT_TOD							(0x00000600)
#define GPON_INT_PLOAM							(0x00000003)
#ifdef TCSUPPORT_CPU_EN7521
#define GPON_INT_ERROR							(0x40FF00C0)
#define GPON_INT_INDICATION						(0x3F00093C)
#else
#define GPON_INT_ERROR							(0x001F00C0)
#define GPON_INT_INDICATION						(0x0000093C)
#endif

#endif /* _GPON_REG_H_ */




