/*
** $Id: tc3162.h,v 1.7 2011/01/07 06:05:58 pork Exp $
*/
/************************************************************************
 *
 *	Copyright (C) 2006 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
/*
** $Log: tc3162.h,v $
** Revision 1.7  2011/01/07 06:05:58  pork
** add the definition of INT!16,INT32,SINT15,SINT7
**
** Revision 1.6  2010/09/20 07:08:02  shnwind
** decrease nf_conntrack buffer size
**
** Revision 1.5  2010/09/03 16:43:07  here
** [Ehance] TC3182 GMAC Driver is support TC-Console & WAN2LAN function & update the tc3182 dmt version (3.12.8.83)
**
** Revision 1.4  2010/09/02 07:04:50  here
** [Ehance] Support TC3162U/TC3182 Auto-Bench
**
** Revision 1.3  2010/08/30 07:53:02  lino
** add power saving mode kernel module support
**
** Revision 1.2  2010/06/05 05:40:29  lino
** add tc3182 asic board support
**
** Revision 1.1.1.1  2010/04/09 09:39:21  feiyan
** New TC Linux Make Flow Trunk
**
** Revision 1.4  2010/01/14 10:56:42  shnwind
** recommit
**
** Revision 1.3  2010/01/14 08:00:10  shnwind
** add TC3182 support
**
** Revision 1.2  2010/01/10 15:27:26  here
** [Ehancement]TC3162U MAC EEE is operated at 100M-FD, SAR interface is accroding the SAR_CLK to calculate atm rate.
**
** Revision 1.1.1.1  2009/12/17 01:42:47  josephxu
** 20091217, from Hinchu ,with VoIP
**
** Revision 1.2  2006/07/06 07:24:57  lino
** update copyright year
**
** Revision 1.1.1.1  2005/11/02 05:45:38  lino
** no message
**
** Revision 1.5  2005/09/27 08:01:38  bread.hsu
** adding IMEM support for Tc3162L2
**
** Revision 1.4  2005/09/14 11:06:20  bread.hsu
** new definition for TC3162L2
**
** Revision 1.3  2005/06/17 16:26:16  jasonlin
** Remove redundant code to gain extra 100K bytes free memory.
** Add "CODE_REDUCTION" definition to switch
**
** Revision 1.2  2005/06/14 10:02:01  jasonlin
** Merge TC3162L2 source code into new main trunk
**
** Revision 1.1.1.1  2005/03/30 14:04:22  jasonlin
** Import Linos source code
**
** Revision 1.4  2004/11/15 03:43:17  lino
** rename ATM SAR max packet length register
**
** Revision 1.3  2004/09/01 13:15:47  lino
** fixed when pc shutdown, system will reboot
**
** Revision 1.2  2004/08/27 12:16:37  lino
** change SYS_HCLK to 96Mhz
**
** Revision 1.1  2004/07/02 08:03:04  lino
** tc3160 and tc3162 code merge
**
*/

#ifndef _TC3162_H_
#define _TC3162_H_

#include <asm/io.h>
#include <cpu/pkgid.h>
#include <linux/types.h>
#include <linux/version.h>

#if defined(TCSUPPORT_CPU_ARMV8_64)
/* all original cache op functions are defined in 
 * arch/arm64/mm/cache.S and arch/arm64/include/asm/cacheflush.h */
#include <asm/cacheflush.h>
#define ecnt_dcache_wback_inv(start, len) __flush_dcache_area(start, len)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0)
#define ecnt_dcache_inv(start, len) __inval_dcache_area(start, len)
#else /* kernel4.4.115 */
extern void __inval_cache_range(unsigned long start, unsigned long end);
#define ecnt_dcache_inv(start, len) __inval_cache_range(start, (start+len))
#endif
/* for ARM64 Performance Monitor Uint APIs */
#include <linux/ecnt_profiling.h>

#else /* ARM32 */
/* ecnt_dma_xxx_range funcs are in linux-ecnt/arch/arm/mm/cache-v7.S */
#define ecnt_dcache_wback_inv(start,len) ecnt_dma_flush_range((unsigned long)start,((unsigned long)start+(unsigned long)len))
#define ecnt_dcache_inv(start,len) ecnt_dma_inv_range((unsigned long)start,((unsigned long)start+(unsigned long)len))
extern void ecnt_dma_flush_range(unsigned long start, unsigned long end);
extern void ecnt_dma_inv_range(unsigned long start, unsigned long end);
/* for ARM32 Performance Monitor Uint APIs */
#include "arm_v7_pmu.h"
#endif

/* For device tree */
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include <spi/spi_controller.h>

#ifndef INT32
#define INT32
typedef int32_t int32;    		/* 32-bit signed integer        */
#endif

#ifndef UINT32
#define UINT32
typedef uint32_t uint32; 		/* 32-bit unsigned integer      */
#endif

#ifndef UINT16
#define UINT16
typedef uint16_t uint16;          /* 16-bit unsigned integer      */
#endif

#ifndef UINT8
#define UINT8
typedef uint8_t uint8;            /* 8-bit unsigned integer       */
#endif

#ifndef INT16
#define INT16
typedef signed short int int16;         /* 16-bit signed integer        */
#endif

/* Old IC */
#define isEN7580GT		0
#define isEN7580ST		0
#define isEN7580GAT		0
#define isTC3162L2P2	0
#define isTC3162L3P3	0
#define isTC3162L4P4	0
#define isTC3162L5P5E2	0
#define isTC3162L5P5E3	0
#define isTC3162L5P5	0
#define isTC3162U		0
#define isRT63260		0
#define isTC3169		0
#define isTC3182		0
#define isRT65168		0
#define isRT63165		0
#define isRT63365		0
#define isRT63368		0
#define isRT62806		0
#define isMT751020		0
#define isMT7505		0
#define isEN7512        0
#define isEN7526c		0
#define isEN7526FC_U2	0
#define isEN7526G		0
#define isEN751221		0
#define isEN7513        0
#define isMT7525G		0
#define isMT7520G		0

#define isEN7513G       0
#define isEN751627		0
#define isEN7516G		0
#define isMT7520        0
#define isMT7510        0
#define isMT7511        0
#define isMT7520S       0
#define isEN7521F       0
#define isEN7521G       0
#define isEN7521S       0
#define isMT7525        0
#define isEN7526D       0
#define isEN7526F       0
#define isEN7526FT      0
#define isEN7526FT_C    0
#define isEN7527G       0
#define isEN7527H       0
#define isEN7528		0
#define isEN7586        0
#define isMT7520E2E3     0

extern uint32_t GET_HIR(void);

#define isEN7523		(GET_HIR() == EN7523_HIR)
#define isEN7581		(GET_HIR() == EN7581_HIR)
#define isAN7552		(GET_HIR() == AN7552_HIR)
#define isAN7583		(GET_HIR() == AN7583_HIR)
#define isEN7580		((GET_HIR() == EN7580_HIR))

#define isEN7561DU	    0
#define isEN7561DN	    0
#define isEN7561G       0
#define isEN7561HU       0

#define isEN7528HU 0
#define isEN7526FH_EN7528DU 0
#define isEN7521G_EN7528DU 0
#define isEN7565 0
#define isEN7521FCUD 0
#define isEN7526FP 0
#define isEN7528DU 0

extern u32 GET_PDIDR(void);
extern u32 GET_IS_FPGA(void);
extern u32 GET_SYS_CLK(void);

#define PDIDR			(GET_PDIDR()) /* ECO ID */
#define isFPGA			GET_IS_FPGA()
#define	SYS_HCLK		(GET_SYS_CLK())

#define GET_IS_DDR4     0
#define ENABLE          1
#define DISABLE         0
#define GET_POWER_SAVING 0


#define WAN2LAN_CH_ID	(1<<31)

#define isMT7530	(((get_frame_engine_data(0xbfb58000+0x7ffc)&0xffff0000))==0x75300000)
#if defined(TCSUPPORT_OPENWRT) || defined(RDKB_BUILD)
#define RESERVEAREA "art"
#else
#define RESERVEAREA "reservearea"
#endif

#endif /* _TC3162_H_ */

