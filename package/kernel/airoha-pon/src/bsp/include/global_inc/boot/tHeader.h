/***************************************************************************************
 *      Copyright(c) 2014 ECONET Incorporation All rights reserved.
 *
 *      This is unpublished proprietary source code of ECONET Networks Incorporation
 *
 *      The copyright notice above does not evidence any actual or intended
 *      publication of such source code.
 ***************************************************************************************
 */

#ifndef __THEADER_H__
#define __THEADER_H__

typedef union {
	struct {
#if defined(__LITTLE_ENDIAN)
#if defined(TCSUPPORT_LITTLE_ENDIAN)
	unsigned int magic	: 24 ;
	unsigned int val	: 8 ;	//in normal(flash) boot, this value is fix
	                            //to 3 in bootrom/spram_ext/system.c
#else
	unsigned int val	: 8 ;	//in normal(flash) boot, this value is fix
	                            //to 3 in bootrom/spram_ext/system.c
	unsigned int magic	: 24 ;
#endif
#else
#if defined(TCSUPPORT_LITTLE_ENDIAN)
	unsigned int val	: 8 ;	//in normal(flash) boot, this value is fix
	                            //to 3 in bootrom/spram_ext/system.c
	unsigned int magic	: 24 ;
#else
	unsigned int magic	: 24 ;
	unsigned int val	: 8 ;	//in normal(flash) boot, this value is fix
	                            //to 3 in bootrom/spram_ext/system.c
#endif
#endif
	} field;
	unsigned int raw;
} EFUSE_CLK_WIDTH_T;

typedef struct tcboot_header{
	unsigned int resv1[2];					//0x00 ~ 0x04, this 8bytes must not use becaust of jump instruction in start.s
	unsigned int tcboot_len;				//0x08
	unsigned int tcboot_magic_num;			//0x0c
	unsigned int lzma_flash_start_addr;	//0x10
	unsigned int lzma_flash_end_addr;		//0x14
	unsigned int bootram_flash_start_addr;	//0x18
	unsigned int bootram_flash_end_addr;	//0x1c
	unsigned int i2c_param;					//0x20 for TCSUPPORT_SECURE_BOOT_7526
	unsigned int chip_flash_info;			//0x24
	union {
		unsigned int ecc_info;				//0x28
		unsigned int en7522_page_size;		//0x28
	};
	unsigned int bypass;					//0x2c
	unsigned int spram_exe_addr;			//0x30
	unsigned int lzma_exe_addr;			//0x34
	unsigned int verify_start_addr;		//0x38
	unsigned int verify_end_addr;			//0x3C
	EFUSE_CLK_WIDTH_T efuse_clk_width;		//0x40
	unsigned int resv3;					//0x44
	unsigned int move_data_int_flash_start_addr;	//0x48
	unsigned int move_data_int_flash_end_addr;		//0x4C
	unsigned int boot2_flash_start_addr;	//0x50
	unsigned int boot2_flash_end_addr;		//0x54
	unsigned int spram_flash_start_addr;	//0x58
	unsigned int spram_flash_end_addr;		//0x5c
} TCBOOT_HEADER;


#endif /* __THEADER_H__ */
