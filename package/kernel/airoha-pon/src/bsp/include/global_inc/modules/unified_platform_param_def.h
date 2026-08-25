#ifndef _UNIFIED_PLATFORM_PARAM_DEF_H
#define _UNIFIED_PLATFORM_PARAM_DEF_H

/*******************************************************************************************************/

/*******************************************************************************************************/
#define FH_UNIFIED_PLATFORM_BOOT2KRENEL_ENV_ADDR 					(0x80001000) /*80002000-80001000=4k*/
/*******************************************************************************************************/
#define FH_UNIFIED_PLATFORM_PARAM_SIZE		(0x20)
/*******************************************************************************************************/
#define FH_UNIFIED_PLATFORM_PARAM_FLASH_ADDR			(0x80000)  /*the 4th block*/
/*******************************************************************************************************/

typedef enum _FH_UNIFIED_PLATFORM_START_UP_MARK_
{
	E_FH_UNIFIED_PLATFORM_START_UP_A		 = 0,
	E_FH_UNIFIED_PLATFORM_START_UP_B		 = 1,
}E_FH_UNIFIED_PLATFORM_START_UP_MARK_T;

/*******************************************************************************************************/


typedef struct _unified_platform_param_
{
	unsigned char   kernel_boot;
	unsigned char   kernel_commit;
	unsigned char   rootfs_boot;
	unsigned char   rootfs_commit;
	unsigned char   padding[FH_UNIFIED_PLATFORM_PARAM_SIZE-0x4];
	unsigned char   boot_version[FH_UNIFIED_PLATFORM_PARAM_SIZE];
	unsigned char   hw_type[FH_UNIFIED_PLATFORM_PARAM_SIZE];		
	unsigned char   reserve0[FH_UNIFIED_PLATFORM_PARAM_SIZE];
	unsigned char   reserve1[FH_UNIFIED_PLATFORM_PARAM_SIZE];
	unsigned char   reserve2[FH_UNIFIED_PLATFORM_PARAM_SIZE];
	unsigned char   reserve3[FH_UNIFIED_PLATFORM_PARAM_SIZE];
	unsigned char   reserve4[FH_UNIFIED_PLATFORM_PARAM_SIZE];
	unsigned char   reserve5[FH_UNIFIED_PLATFORM_PARAM_SIZE];
	unsigned char   reserve6[FH_UNIFIED_PLATFORM_PARAM_SIZE];
	unsigned char   reserve7[FH_UNIFIED_PLATFORM_PARAM_SIZE];		
	unsigned char   reserve8[FH_UNIFIED_PLATFORM_PARAM_SIZE];
	unsigned char   reserve9[FH_UNIFIED_PLATFORM_PARAM_SIZE];	
}unified_platform_param_t, *pt_unified_platform_param;



/********************************************************************************************/
#endif
