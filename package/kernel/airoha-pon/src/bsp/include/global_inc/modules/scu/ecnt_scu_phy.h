#ifndef __ECNT_SCU_PHY_H__
#define __ECNT_SCU_PHY_H__

#include <linux/version.h>
#include <asm/types.h>



#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
extern u32 get_thermal_ADC(void);
#endif
extern void set_BG_CKEN(void);
 
 
 
 
#endif