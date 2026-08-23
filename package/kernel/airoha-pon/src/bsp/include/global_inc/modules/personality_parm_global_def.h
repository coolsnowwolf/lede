#ifndef _PERSONALITY_PARM_GLOBAL_DEF_H
#define _PERSONALITY_PARM_GLOBAL_DEF_H

/*******************************************************************************************************/
#define PERSONALITY_PARM_MAJOR          			(237)
#define PERSONALITY_PARM_MODULE_NAME    			"PERSONALITY_PARM"
#define PERSONALITY_PARM_IOC_MAGIC      			'e'
#define IF_IOG_REALTIME_DUAL_IMAGE_FLAG_INFO		_IOR(PERSONALITY_PARM_IOC_MAGIC, 0x1,  unsigned long)
#define IF_IOG_FH_NW_PRODUCT_BOOT_VERSION			_IOR(PERSONALITY_PARM_IOC_MAGIC, 0x2, unsigned long)
#define IF_IOG_FH_NW_PRODUCT_MEMORY_ENV_RESERVE0	_IOR(PERSONALITY_PARM_IOC_MAGIC, 0x3, unsigned long)
#define IF_IOG_FH_NW_PRODUCT_MEMORY_ENV_RESERVE1	_IOR(PERSONALITY_PARM_IOC_MAGIC, 0x4, unsigned long)
#define IF_IOG_FH_NW_PRODUCT_MEMORY_ENV_RESERVE2	_IOR(PERSONALITY_PARM_IOC_MAGIC, 0x5, unsigned long)
#define IF_IOG_FH_NW_PRODUCT_MEMORY_ENV_RESERVE3	_IOR(PERSONALITY_PARM_IOC_MAGIC, 0x6, unsigned long)
#define IF_IOG_FH_NW_PRODUCT_MEMORY_ENV_RESERVE4	_IOR(PERSONALITY_PARM_IOC_MAGIC, 0x7, unsigned long)
#define IF_IOG_FH_NW_PRODUCT_MEMORY_ENV_RESERVE5	_IOR(PERSONALITY_PARM_IOC_MAGIC, 0x8, unsigned long)
#define IF_IOG_FH_NW_PRODUCT_MEMORY_ENV_RESERVE6	_IOR(PERSONALITY_PARM_IOC_MAGIC, 0x9, unsigned long)

/*******************************************************************************************************/
#define FH_NW_PRODUCT_BLOCK_SIZE                    (0x20000)
#define FH_NW_PRODUCT_BOOT_PARTITION                "boot"
#define FH_NW_PRODUCT_KERNELA_PARTITION             "KernelA"
#define FH_NW_PRODUCT_APP1A_PARTITION               "App1A"
#define FH_NW_PRODUCT_APP2A_PARTITION               "App2A"
#define FH_NW_PRODUCT_ROOTFSA_PARTITION             "RootfsA"
#define FH_NW_PRODUCT_KERNELB_PARTITION             "KernelB"
#define FH_NW_PRODUCT_APP1B_PARTITION               "App1B"
#define FH_NW_PRODUCT_APP2B_PARTITION               "App2B"
#define FH_NW_PRODUCT_ROOTFSB_PARTITION             "RootfsB"
#define FH_NW_PRODUCT_CONFIGURATION_PARTITION       "Configuration"
#define FH_NW_PRODUCT_PERSONALITY_PARM1_PARTITION   "PersonalityParmOne"
#define FH_NW_PRODUCT_PERSONALITY_PARM2_PARTITION   "PersonalityParmTwo"
#define FH_NW_PRODUCT_PERSONALITY_PARM3_PARTITION   "PersonalityParmThree"
#define FH_NW_PRODUCT_PERSONALITY_PARM4_PARTITION   "PersonalityParmFour"
#define FH_NW_PRODUCT_PERSONALITY_PARM5_PARTITION   "PersonalityParmFive"
#define FH_NW_PRODUCT_PERSONALITY_PARM6_PARTITION   "PersonalityParmSix"
#define FH_NW_PRODUCT_PERSONALITY_PARM7_PARTITION   "PersonalityParmSeven"
#define FH_NW_PRODUCT_PERSONALITY_PARM8_PARTITION   "PersonalityParmEight"

#define FH_NW_PRODUCT_BOOT_SIZE            			(0x100000)
#define FH_NW_PRODUCT_KERNEL_SIZE          			(0x400000)
#define FH_NW_PRODUCT_APP1_SIZE            			(0x800000)
#define FH_NW_PRODUCT_APP2_SIZE            			(0x1800000)
#define FH_NW_PRODUCT_ROOTFS_SIZE          			(0xE00000)
#define FH_NW_PRODUCT_CONFIGURATION_SIZE   			(0xA00000)
#define FH_NW_PRODUCT_PERSONALITY_PARM1_SIZE        (FH_NW_PRODUCT_BLOCK_SIZE)
#define FH_NW_PRODUCT_PERSONALITY_PARM2_SIZE        (FH_NW_PRODUCT_BLOCK_SIZE)
#define FH_NW_PRODUCT_PERSONALITY_PARM3_SIZE        (FH_NW_PRODUCT_BLOCK_SIZE)
#define FH_NW_PRODUCT_PERSONALITY_PARM4_SIZE        (FH_NW_PRODUCT_BLOCK_SIZE)
#define FH_NW_PRODUCT_PERSONALITY_PARM5_SIZE        (FH_NW_PRODUCT_BLOCK_SIZE)
#define FH_NW_PRODUCT_PERSONALITY_PARM6_SIZE        (FH_NW_PRODUCT_BLOCK_SIZE)
#define FH_NW_PRODUCT_PERSONALITY_PARM7_SIZE        (FH_NW_PRODUCT_BLOCK_SIZE)
#define FH_NW_PRODUCT_PERSONALITY_PARM8_SIZE        (FH_NW_PRODUCT_BLOCK_SIZE)

#define FH_NW_PRODUCT_KERNELA_ADDR  	   			(0x100000)
#define FH_NW_PRODUCT_APP1A_ADDR  	   				(0x500000)
#define FH_NW_PRODUCT_APP2A_ADDR  	   				(0xD00000)
#define FH_NW_PRODUCT_ROOTFSA_ADDR					(0x2500000)
#define FH_NW_PRODUCT_KERNELB_ADDR  	   			(0x3300000)
#define FH_NW_PRODUCT_APP1B_ADDR  	   				(0x3700000)
#define FH_NW_PRODUCT_APP2B_ADDR  	   				(0x3F00000)
#define FH_NW_PRODUCT_ROOTFSB_ADDR					(0x5700000)
#define FH_NW_PRODUCT_CONFIGURATION_ADDR			(0x6500000)
#define FH_NW_PRODUCT_PERSONALITY_PARM1_ADDR        (0x6F00000)
#define FH_NW_PRODUCT_PERSONALITY_PARM2_ADDR        (0x6F20000)
#define FH_NW_PRODUCT_PERSONALITY_PARM3_ADDR        (0x6F40000)
#define FH_NW_PRODUCT_PERSONALITY_PARM4_ADDR        (0x6F60000)
#define FH_NW_PRODUCT_PERSONALITY_PARM5_ADDR        (0x6F80000)
#define FH_NW_PRODUCT_PERSONALITY_PARM6_ADDR        (0x6FA0000)
#define FH_NW_PRODUCT_PERSONALITY_PARM7_ADDR        (0x6FC0000)
#define FH_NW_PRODUCT_PERSONALITY_PARM8_ADDR        (0x6FE0000)

/*******************************************************************************************************/
#define FH_NW_PRODUCT_FLASH_ENV_READ_BASE_ADDR			(FH_NW_PRODUCT_PERSONALITY_PARM3_ADDR)
#define FH_NW_PRODUCT_FLASH_ENV_WRITE_BASE_ADDR			(FH_NW_PRODUCT_PERSONALITY_PARM4_ADDR)
#define PERSONALITY_PARM_FLASH_ENV_READ_PARTITION		 FH_NW_PRODUCT_PERSONALITY_PARM3_PARTITION
#define PERSONALITY_PARM_FLASH_ENV_WRITE_PARTITION		 FH_NW_PRODUCT_PERSONALITY_PARM4_PARTITION

#define FH_NW_PRODUCT_DUAL_IMAGE_READ_BASE_ADDR 		(FH_NW_PRODUCT_PERSONALITY_PARM1_ADDR)
#define FH_NW_PRODUCT_DUAL_IMAGE_WRITE_BASE_ADDR 		(FH_NW_PRODUCT_PERSONALITY_PARM2_ADDR)
#define PERSONALITY_PARM_DUAL_IMAGE_READ_PARTITION       FH_NW_PRODUCT_PERSONALITY_PARM1_PARTITION
#define PERSONALITY_PARM_DUAL_IMAGE_WRITE_PARTITION		 FH_NW_PRODUCT_PERSONALITY_PARM2_PARTITION
/*******************************************************************************************************/
#define DUAL_IMAGE_OFFSET_SIZE								(0x1)
#define KERNEL_COMMIT_OFFSET								((DUAL_IMAGE_OFFSET_SIZE) * (0x0))
#define KERNEL_ACTIVE_OFFSET								((DUAL_IMAGE_OFFSET_SIZE) * (0x1))
#define ROOTFS_COMMIT_OFFSET								((DUAL_IMAGE_OFFSET_SIZE) * (0x2))
#define ROOTFS_ACTIVE_OFFSET								((DUAL_IMAGE_OFFSET_SIZE) * (0x3))
#define APP1_COMMIT_OFFSET									((DUAL_IMAGE_OFFSET_SIZE) * (0x4))
#define APP1_ACTIVE_OFFSET									((DUAL_IMAGE_OFFSET_SIZE) * (0x5))
#define APP2_COMMIT_OFFSET									((DUAL_IMAGE_OFFSET_SIZE) * (0x6))
#define APP2_ACTIVE_OFFSET									((DUAL_IMAGE_OFFSET_SIZE) * (0x7))
/*******************************************************************************************************/
#define FH_FLASH_ENV_OFFSET_SIZE							(0x20)
#define FH_FLASH_SNOUI_OFFSET								((FH_FLASH_ENV_OFFSET_SIZE) * (0x0))
#define FH_FLASH_HWCFG_OFFSET								((FH_FLASH_ENV_OFFSET_SIZE) * (0x1))
#define FH_FLASH_ETHADDR_OFFSET								((FH_FLASH_ENV_OFFSET_SIZE) * (0x2))
#define FH_FLASH_RESERVE0_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x3))
#define FH_FLASH_RESERVE1_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x4))
#define FH_FLASH_RESERVE2_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x5))
#define FH_FLASH_RESERVE3_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x6))
#define FH_FLASH_RESERVE4_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x7))
#define FH_FLASH_RESERVE5_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x8))
#define FH_FLASH_RESERVE6_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x9))
#define FH_FLASH_RESERVE7_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0xA))
#define FH_FLASH_RESERVE8_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0xB))
#define FH_FLASH_RESERVE9_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0xC))
#define FH_FLASH_RESERVE10_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0xD))
#define FH_FLASH_RESERVE11_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0xE))
#define FH_FLASH_RESERVE12_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0xF))
#define FH_FLASH_RESERVE13_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x10))
#define FH_FLASH_RESERVE14_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x11))
#define FH_FLASH_RESERVE15_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x12))
#define FH_FLASH_RESERVE16_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x13))
#define FH_FLASH_RESERVE17_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x14))
#define FH_FLASH_RESERVE18_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x15))
#define FH_FLASH_RESERVE19_OFFSET							((FH_FLASH_ENV_OFFSET_SIZE) * (0x16))
/*******************************************************************************************************/
#define VALID  												(0x0)
#define INVLID 												(0x1)
#define FH_NW_PRODUCT_POSSIBLE_VALID						(0x2)
#define FH_NW_PRODUCT_CONTINUE                      		(0x3)
#define FH_NW_PRODUCT_BREAK                      			(0x4)
/*******************************************************************************************************/
#define FH_NW_PRODUCT_DUAL_IMAGE_PARTITION					(0x1)
#define FH_NW_PRODUCT_FLASH_ENV_PARTITION					(0x2)
/*******************************************************************************************************/
#define FH_NW_PRODUCT_DUAL_IMAGE_FLAG_SIZE					(0x1)
#define FH_NW_PRODUCT_ENV_SIZE    							(0x20)
/*******************************************************************************************************/
#define FH_NW_PRODUCT_PADDING_SIZE    						(0x14)
#define FH_NW_PRODUCT_DUAL_IMAGE_FLAG_RESERVE_SIZE			(0x18)
/*******************************************************************************************************/
#define FH_NW_PRODUCT_DUAL_IMAGE_MASK 						(0x3)
/*******************************************************************************************************/
#define MAX_MAGIC_VALUE 									(0xFFFF)
#define FH_NW_PRODUCT_CHECK_SIZE							(0x20)
#define FH_NW_PRODUCT_VENDOR_DEFINE   			 	 		 "FH"
#define FH_NW_PRODUCT_CHECK_VENDOR_SIZE						(0x2)
#define FH_NW_PRODUCT_CHECK_RESERVE_SIZE	        		(0x18)
#define FH_NW_PRODUCT_DUAL_IMAGE_FLAG_CHECK_INFO_OFFSET		(0x20)
/*******************************************************************************************************/
#define FH_NW_PRODUCT_BOOT2KRENEL_ENV_ADDR					(0x80001BE0)
/*******************************************************************************************************/
#define FH_NW_PRODUCT_FLASH_ENV_INFO_OFFSET ((FH_FLASH_ENV_OFFSET_SIZE) * (0x17))
#define FH_NW_PRODUCT_FLASH_ENV_GOUNT	(0x16)
/*******************************************************************************************************/
typedef enum _FH_NW_PRODUCT_PERSONALITY_PARM_CMD_
{
	E_FH_NW_PRODUCT_SET_COMMIT_KERNEL   	= (0x0),
	E_FH_NW_PRODUCT_SET_COMMIT_ROOTFS   	= (0x1),
	E_FH_NW_PRODUCT_SET_COMMIT_APP1 		= (0x2),
	E_FH_NW_PRODUCT_SET_COMMIT_APP2 		= (0x3),
	E_FH_NW_PRODUCT_SET_ACTIVE_KERNEL   	= (0x4),
	E_FH_NW_PRODUCT_SET_ACTIVE_ROOTFS   	= (0x5),
	E_FH_NW_PRODUCT_SET_ACTIVE_APP1 		= (0x6),
	E_FH_NW_PRODUCT_SET_ACTIVE_APP2 		= (0x7),

	E_FH_NW_PRODUCT_GET_COMMIT_KERNEL   	= (0x8),
	E_FH_NW_PRODUCT_GET_COMMIT_ROOTFS   	= (0x9),
	E_FH_NW_PRODUCT_GET_COMMIT_APP1 		= (0xA),
	E_FH_NW_PRODUCT_GET_COMMIT_APP2 		= (0xB),
	E_FH_NW_PRODUCT_GET_ACTIVE_KERNEL   	= (0xC),
	E_FH_NW_PRODUCT_GET_ACTIVE_ROOTFS  		= (0xD),
	E_FH_NW_PRODUCT_GET_ACTIVE_APP1 		= (0xE),
	E_FH_NW_PRODUCT_GET_ACTIVE_APP2 		= (0xF),

	E_FH_NW_PRODUCT_SET_SNOUI				= (0x10),
	E_FH_NW_PRODUCT_SET_HWCFG        		= (0x11),
	E_FH_NW_PRODUCT_SET_ETHERADDR			= (0x12),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE0  = (0x13),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE1  = (0x14),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE2	= (0x15),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE3  = (0x16),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE4  = (0x17),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE5	= (0x18),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE6  = (0x19),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE7  = (0x1A),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE8  = (0x1B),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE9  = (0x1C),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE10 = (0x1D),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE11 = (0x1E),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE12 = (0x1F),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE13 = (0x20),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE14 = (0x21),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE15 = (0x22),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE16 = (0x23),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE17 = (0x24),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE18 = (0x25),
	E_FH_NW_PRODUCT_SET_FLASH_ENV_RESERVE19 = (0x26),
	
	E_FH_NW_PRODUCT_GET_SNOUI				= (0x27),
	E_FH_NW_PRODUCT_GET_HWCFG        		= (0x28),
	E_FH_NW_PRODUCT_GET_ETHERADDR			= (0x29),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE0	= (0x2A),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE1  = (0x2B),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE2	= (0x2C),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE3	= (0x2D),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE4	= (0x2E),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE5	= (0x2F),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE6	= (0x30),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE7  = (0x31),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE8  = (0x32),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE9  = (0x33),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE10 = (0x34),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE11 = (0x35),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE12 = (0x36),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE13 = (0x37),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE14 = (0x38),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE15 = (0x39),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE16 = (0x3A),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE17 = (0x3B),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE18 = (0x3C),
	E_FH_NW_PRODUCT_GET_FLASH_ENV_RESERVE19 = (0x3D),
	E_FH_NW_PRODUCT_GET_BOOT_VERSION		= (0x3E),
	E_FH_NW_PRODUCT_GET_MEMORY_ENV_RESERVE0 = (0x3F),
	E_FH_NW_PRODUCT_GET_MEMORY_ENV_RESERVE1 = (0x40),
	E_FH_NW_PRODUCT_GET_MEMORY_ENV_RESERVE2 = (0x41),
	E_FH_NW_PRODUCT_GET_MEMORY_ENV_RESERVE3 = (0x42),
	E_FH_NW_PRODUCT_GET_MEMORY_ENV_RESERVE4 = (0x43),
	E_FH_NW_PRODUCT_GET_MEMORY_ENV_RESERVE5 = (0x44),
	E_FH_NW_PRODUCT_GET_MEMORY_ENV_RESERVE6 = (0x45),
	E_FH_NW_PRODUCT_GET_REALTIME_DUAL_IMAGE_INFO = (0x46),
	
	E_FH_NW_PRODUCT_MAX 				= (0X47),
}E_FH_NW_PRODUCT_PERSONALITY_PARM_CMD_T;

typedef enum _PERSONALITY_PARM_RET_
{
	E_PERSONALITY_PARM_RIGHT_EXECUTE = 0,
	E_PERSONALITY_PARM_INVALID_ARGUMENT = 1,	
}E_PERSONALITY_PARM_RET_T;

typedef enum _FH_NW_PRODUCT_START_UP_MARK_
{
	E_FH_NW_PRODUCT_START_UP_A		 = 0,
	E_FH_NW_PRODUCT_START_UP_B		 = 1,
}E_FH_NW_PRODUCT_START_UP_MARK_T;

typedef enum _FH_NW_PRODUCT_COMMIT_MARK_
{	E_FH_NW_PRODUCT_COMMIT_A = 0,
	E_FH_NW_PRODUCT_COMMIT_B = 1,
}E_FH_NW_PRODUCT_COMMIT_MARK_T;

typedef enum _FH_NW_PRODUCT_ACTIVE_MARK_
{
	E_FH_NW_PRODUCT_ACTIVE_DEFAULT   = 0,
	E_FH_NW_PRODUCT_ACTIVE_A		 = 1,
	E_FH_NW_PRODUCT_ACTIVE_B		 = 2,
}E_FH_NW_PRODUCT_ACTIVE_MARK_T;

typedef enum _FH_NW_PRODUCT_ROOLBACK_MARK_
{
	E_FH_NW_PRODUCT_ROOLBACK_FALSE   = 0,
	E_FH_NW_PRODUCT_ROOLBACK_TRUE    = 1,
}E_FH_NW_PRODUCT_ROOLBACK_MARK_T;

typedef struct _personality_parm_element_
{
	unsigned char             dual_image_flag;
	unsigned char env[FH_NW_PRODUCT_ENV_SIZE];
}personality_parm_element_t, *pt_personality_parm_element;

typedef struct _personality_parm_info_
{
	unsigned int  			   cmd;
    personality_parm_element_t element;
}personality_parm_info_t, *pt_personality_parm_info_t;

typedef struct _personality_parm_
{
	unsigned long		 read_base_addr;
	unsigned long		write_base_addr;	
	unsigned long 			   v_offset;
	unsigned long				 v_size;
	personality_parm_element_t  element;
}personality_parm_t, *pt_personality_parm;

typedef struct _personality_parm_table_
{
	const char						*action_cmd;
	const char						 *node_name;
	const char					 *sub_node_name;
	unsigned int              				cmd;
	unsigned long  				 read_base_addr;
	unsigned long 				write_base_addr;
	unsigned long 		   				 offset;
	unsigned long 			 			   size;
	int (*get_func)(personality_parm_t *pt_info);
	int (*set_func)(personality_parm_t *pt_info);
}personality_parm_table_t, *pt_personality_parm_table;
/*******************************************************************************************************/
typedef struct _dual_image_flag_
{
    /*offset fix,can't insert variable parameter*/
	unsigned char kernel_commit;
	unsigned char kernel_active;
	unsigned char rootfs_commit;
	unsigned char rootfs_active;
	unsigned char app1_commit;
	unsigned char app1_active;
	unsigned char app2_commit;
	unsigned char app2_active;
	unsigned char reserve[FH_NW_PRODUCT_DUAL_IMAGE_FLAG_RESERVE_SIZE];
}dual_image_flag_t, *pt_dual_image_flag;

typedef struct _check_personality_parm_partition_info_
{
	unsigned char  reserve[FH_NW_PRODUCT_CHECK_RESERVE_SIZE];
	unsigned char   vendor[FH_NW_PRODUCT_CHECK_VENDOR_SIZE];
	unsigned short   magic;
	unsigned int crc_check;
}check_personality_parm_partition_info_t, *pt_check_personality_parm_partition_info;

typedef struct _dual_image_personality_parm_partition_info_
{
    dual_image_flag_t 						dual_image_element;
    check_personality_parm_partition_info_t check_info;
}dual_image_personality_parm_partition_info_t,*pt_dual_image_personality_parm_partition_info;

typedef struct _personality_parm_cmd_base_addr_
{
	unsigned long	     read_base_addr;
	unsigned long       write_base_addr;
	unsigned long       valid_read_addr;
	unsigned long      valid_write_addr;
	unsigned char       both_valid_flag;
	unsigned char          A_valid_flag;
	unsigned char          B_valid_flag;
}personality_parm_cmd_base_addr_t, *pt_personality_parm_cmd_base_addr;
/*******************************************************************************************************/

typedef struct _personality_parm_flash_env_
{
	unsigned char       sn_oui[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char       hw_cfg[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char     eth_addr[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   f_reserve0[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   f_reserve1[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   f_reserve2[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   f_reserve3[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   f_reserve4[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   f_reserve5[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   f_reserve6[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   f_reserve7[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   f_reserve8[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   f_reserve9[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char  f_reserve10[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char  f_reserve11[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char  f_reserve12[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char  f_reserve13[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char  f_reserve14[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char  f_reserve15[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char  f_reserve16[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char  f_reserve17[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char  f_reserve18[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char  f_reserve19[FH_NW_PRODUCT_ENV_SIZE];
}personality_parm_flash_env_t, *pt_personality_parm_flash_env;

typedef struct _flash_env_personality_parm_partition_info_
{
    personality_parm_flash_env_t 				   env;
    check_personality_parm_partition_info_t check_info;
}flash_env_personality_parm_partition_info_t,*pt_flash_env_personality_parm_partition_info;

/****************************************************************************/
typedef struct _personality_parm_memory_env_
{
	unsigned char boot_version[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   m_reserve0[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   m_reserve1[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   m_reserve2[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   m_reserve3[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   m_reserve4[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   m_reserve5[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char   m_reserve6[FH_NW_PRODUCT_ENV_SIZE];
}personality_parm_memory_env_t, *pt_personality_parm_memory_env;

typedef struct _realtime_dual_image_env_
{
	unsigned char    kernel_commit;
	unsigned char    kernel_active;
	unsigned char    rootfs_commit;
	unsigned char	 rootfs_active;
	unsigned char    app1_commit;
	unsigned char    app1_active;
	unsigned char    app2_commit;
	unsigned char    app2_active;
	unsigned char    kernel_run;
	unsigned char    kernel_roolback;
	unsigned char    rootfs_run;
	unsigned char    rootfs_roolback;
	unsigned char    padding[FH_NW_PRODUCT_PADDING_SIZE];
}realtime_dual_image_env_t, *pt_realtime_dual_image_env;

typedef struct _personality_parm_boot_to_kernel_info_
{
    realtime_dual_image_env_t     realtime_env;
	/***************Flash  Env **************************/
    personality_parm_flash_env_t  flash_env;
	/***************Memory Env************************/
    personality_parm_memory_env_t memory_env;
}personality_parm_boot_to_kernel_info_t, *pt_personality_parm_boot_to_kernel_info;

typedef union _realtime_env_info_
{
	unsigned char 			   flash_env[FH_NW_PRODUCT_ENV_SIZE];
	unsigned char 			  memory_env[FH_NW_PRODUCT_ENV_SIZE];
	realtime_dual_image_env_t dual_image_env;
}realtime_env_info_t, *pt_realtime_env_info;
/********************************************************************************************/
#endif
