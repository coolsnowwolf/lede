 /***************************************************************************************
 *      Copyright(c) 2016 ECONET Incorporation All rights reserved.
 *
 *      This is unpublished proprietary source code of ECONET Incorporation
 *
 *      The copyright notice above does not evidence any actual or intended
 *      publication of such source code.
 ***************************************************************************************
 */
 
 /*======================================================================================
 * MODULE NAME: spi
 * FILE NAME: spi_nfi.h
 * DATE: 2016/03/18
 * VERSION: 1.00
 * PURPOSE: To Provide SPI NFI(DMA) Access Internace. 
 * NOTES:
 *
 * AUTHOR : Chuck Kuo         REVIEWED by
 *
 * FUNCTIONS
 *
 * DEPENDENCIES
 *
 * * $History: $
 * MODIFICTION HISTORY:
 * Version 1.00 - Date 2016/03/18 By Chuck Kuo
 * ** This is the first versoin for creating to support the functions of
 *    current module.
 *
 *======================================================================================
 */

#ifndef __SPI_NFI_H__
    #define __SPI_NFI_H__

 
/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */
#include <asm/types.h>


/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */

#if defined(CONFIG_ECNT_UBOOT) /* U-boot likes MIPS bootram, don't follow ARMV8 path */
#undef TCSUPPORT_CPU_ARMV8
#endif

#if defined(TCSUPPORT_PARALLEL_NAND)
#ifndef TCSUPPORT_CPU_ARMV8
#if defined(CONFIG_ECNT_UBOOT) /* Only for EN7523 */
#define _SPI_NFI_REGS_BASE			(0x1FA11000)
#else
#define _SPI_NFI_REGS_BASE			(0xBFA11000)
#endif
#define _NFI_REGS_SNF_NFI_CNFG		(_SPI_NFI_REGS_BASE + 0x55C)
#endif

#define NFI_INTR_READ_DONE			(0x0001)
#define NFI_INTR_WRITE_DONE			(0x0002)
#define NFI_INTR_RESET_DONE			(0x0004)
#define NFI_INTR_ERASE_DONE			(0x0008)
#define NFI_INTR_BUSY_RETURN		(0x0010)
#define NFI_INTR_ACCESS_LOCK		(0x0020)
#define NFI_INTR_AHB_DONE			(0x0040)
#define NFI_INTR_CB2R_TIMEOUT		(0x1000)
#define NFI_INTR_ALL				(NFI_INTR_CB2R_TIMEOUT | NFI_INTR_AHB_DONE | NFI_INTR_ACCESS_LOCK | \
											NFI_INTR_BUSY_RETURN | NFI_INTR_ERASE_DONE | NFI_INTR_RESET_DONE | NFI_INTR_WRITE_DONE)

#define NAND_CMD_READ				(0x00)
#define NAND_CMD_READSTART			(0x30)
#define NAND_CMD_SEQIN				(0x80)
#define NAND_CMD_PAGEPROG			(0x10)
#define NAND_CMD_ERASE1				(0x60)
#define NAND_CMD_ERASE2				(0xD0)
#define NAND_CMD_STATUS				(0x70)
#define NAND_CMD_READID				(0x90)
#define NAND_CMD_RESET				(0xFF)

#define NFI_STA_CMD_MODE			(0x0001)
#define NFI_STA_ADDR_MODE			(0x0002)
#define NFI_STA_DATAR_MODE			(0x0004)
#define NFI_STA_DATAW_MODE			(0x0008)
#define NFI_STA_ACCESS_LOCK			(0x0010)
#define NFI_STA_BUSY_RETURN			(0x0040)

#define NFI_PIO_DI_RDY				(0x0001)

#define SPI_NFI_WRITE_DATA			(0)
#define SPI_NFI_READ_DATA			(1)
#endif

/* TYPE DECLARATIONS ----------------------------------------------------------------- */
typedef enum 
{
   SPI_NFI_CON_AUTO_FDM_Disable=0,
   SPI_NFI_CON_AUTO_FDM_Enable,
} SPI_NFI_CONF_AUTO_FDM_T;

typedef enum 
{
   SPI_NFI_CON_HW_ECC_Disable=0,
   SPI_NFI_CON_HW_ECC_Enable,
} SPI_NFI_CONF_HW_ECC_T;

typedef enum 
{
   SPI_NFI_CON_DMA_TRIGGER_READ=0,
   SPI_NFI_CON_DMA_TRIGGER_WRITE,
} SPI_NFI_CONF_DMA_TRIGGER_T;

typedef enum 
{
   SPI_NFI_CON_DMA_BURST_Disable=0,
   SPI_NFI_CON_DMA_BURST_Enable,
} SPI_NFI_CONF_DMA_BURST_T;

typedef enum
{
	SPI_NFI_CONF_SPARE_SIZE_16BYTE = 16,
	SPI_NFI_CONF_SPARE_SIZE_26BYTE = 26,
	SPI_NFI_CONF_SPARE_SIZE_27BYTE = 27,
	SPI_NFI_CONF_SPARE_SIZE_28BYTE = 28,
} SPI_NFI_CONF_SPARE_SIZE_T;

typedef enum
{
	SPI_NFI_CONF_PAGE_SIZE_512BYTE = 512,
	SPI_NFI_CONF_PAGE_SIZE_2KBYTE  = 2048,
	SPI_NFI_CONF_PAGE_SIZE_4KBYTE  = 4096,
} SPI_NFI_CONF_PAGE_SIZE_T;

typedef enum
{
	SPI_NFI_CONF_CUS_SEC_SIZE_Disable=0,
	SPI_NFI_CONF_CUS_SEC_SIZE_Enable,
} SPI_NFI_CONF_CUS_SEC_SIZE_T;


typedef enum
{
	SPI_NFI_MISC_CONTROL_X1		 = 0x000,
	SPI_NFI_MISC_CONTROL_X2		 = 0x001,
	SPI_NFI_MISC_CONTROL_X4		 = 0x002,
	SPI_NFI_MISC_CONTROL_DUAL_IO = 0x005,
	SPI_NFI_MISC_CONTROL_QUAD_IO = 0x006,
	
} SPI_NFI_MISC_SPEDD_CONTROL_T;

typedef struct SPI_NFI_CONFIGURE
{
	SPI_NFI_CONF_AUTO_FDM_T			auto_fdm_t;			/* auto padding oob behind data, or not */
	SPI_NFI_CONF_HW_ECC_T			hw_ecc_t;			/* enable hw ecc or not */
	SPI_NFI_CONF_DMA_BURST_T		dma_burst_t;		/* dma burst */	
	u8								fdm_num;			/* value range : 0 ~ 8 */
	u8								fdm_ecc_num;		/* value range : 0 ~ 8 */
														/* fdm byte under ecc protection */
	SPI_NFI_CONF_SPARE_SIZE_T   	spare_size_t;		/* spare size of eache sector */
	SPI_NFI_CONF_PAGE_SIZE_T		page_size_t;		/* page size (not incluing oob size) */
	u8								sec_num;	   		/* number of sector */
														/* value range : 1 ~ 8 */
	SPI_NFI_CONF_CUS_SEC_SIZE_T	 	cus_sec_size_en_t;	/* To apply user define sector size or not */
														/* Disable : sector size = 512 bytes, 
															and ECC function will work */
														/* Enable  : user define sector size, 
												       		and ECC function will not work */
	u32								sec_size;			/* Only work if cus_sec_size_en is enable */
														/* value range : 1 ~ 8187 */
	SPI_NFI_MISC_SPEDD_CONTROL_T	speed_t;
	
} SPI_NFI_CONF_T;



typedef enum{
	SPI_NFI_RTN_NO_ERROR =0,
	SPI_NFI_RTN_CHECK_AHB_DONE_TIMEOUT,
	SPI_NFI_RTN_LOAD_TO_CACHE_DONE_TIMEOUT,
	SPI_NFI_RTN_READ_FROM_CACHE_DONE_TIMEOUT,
#if defined(TCSUPPORT_PARALLEL_NAND)
	SPI_NFI_RTN_WAIT_TIMEOUT,
	SPI_NFI_RTN_ACCESS_LOCK,
	SPI_NFI_RTN_INVAILD_PARAM,
#endif

	SPI_NFI_RTN_DEF_NO
} SPI_NFI_RTN_T;

typedef enum{
	SPI_NFI_SPI=0,
	SPI_NFI_PARALLEL,
	SPI_NFI_EMMC,
} SPI_NFI_TYPE_T;

typedef enum{
	SPI_NFI_CONF_DMA_WR_BYTE_SWAP_DISABLE =0,
	SPI_NFI_CONF_DMA_WR_BYTE_SWAP_ENABLE
} SPI_NFI_CONF_DMA_WR_BYTE_SWAP_T;	

typedef enum{
	SPI_NFI_CONF_ECC_DATA_SOURCE_INV_DISABLE =0,
	SPI_NFI_CONF_ECC_DATA_SOURCE_INV_ENABLE
} SPI_NFI_CONF_ECC_DATA_SOURCE_INV_T;

/* EXPORTED SUBPROGRAM SPECIFICATION ------------------------------------------------- */ 										
					
SPI_NFI_RTN_T SPI_NFI_Regs_Dump( void );
SPI_NFI_RTN_T SPI_NFI_Read_SPI_NAND_FDM(u8 *ptr_rtn_oob, u32 oob_len);
SPI_NFI_RTN_T SPI_NFI_Write_SPI_NAND_FDM(u8 *ptr_oob, u32 oob_len);
SPI_NFI_RTN_T SPI_NFI_Read_SPI_NAND_Page(SPI_NFI_MISC_SPEDD_CONTROL_T speed_mode, u32 read_cmd, u16 read_addr, unsigned long *prt_rtn_data);
SPI_NFI_RTN_T SPI_NFI_Write_SPI_NAND_page(SPI_NFI_MISC_SPEDD_CONTROL_T speed_mode, u32 write_cmd, u16 write_addr, unsigned long *prt_data);
SPI_NFI_RTN_T SPI_NFI_Read_SPI_NOR(u8 opcode, u16 read_addr, u32 *prt_rtn_data);
SPI_NFI_RTN_T SPI_NFI_Write_SPI_NOR(u8 opcode, u16 write_addr, u32 *prt_data);
SPI_NFI_RTN_T SPI_NFI_Get_Configure( SPI_NFI_CONF_T *ptr_rtn_nfi_conf_t );
SPI_NFI_RTN_T SPI_NFI_Set_Configure( SPI_NFI_CONF_T *ptr_nfi_conf_t );
void SPI_NFI_Reset( void );
SPI_NFI_RTN_T SPI_NFI_Init( void );
void SPI_NFI_DEBUG_ENABLE( void );
void SPI_NFI_DEBUG_DISABLE( void );
/* Set DMA(flash -> SRAM) byte swap*/
void SPI_NFI_DMA_WR_BYTE_SWAP(SPI_NFI_CONF_DMA_WR_BYTE_SWAP_T enable);
/* Set ECC decode invert*/
void SPI_NFI_ECC_DATA_SOURCE_INV(SPI_NFI_CONF_ECC_DATA_SOURCE_INV_T enable);
SPI_NFI_TYPE_T nfi_type(void);

#if defined(TCSUPPORT_PARALLEL_NAND)
extern SPI_NFI_RTN_T PARALLEL_NFI_CHECK_INT_STATUS(u16 intr_check);
extern SPI_NFI_RTN_T PARALLEL_NFI_START_DMA(unsigned long *p_data, u8 dirt);
extern SPI_NFI_RTN_T PARALLEL_NFI_START_BYTE_READ(u8 len, u8 *p_data);
extern SPI_NFI_RTN_T PARALLEL_NFI_ISSUE_CMD_2(u8 cmd2);
extern SPI_NFI_RTN_T PARALLEL_NFI_ISSUE_ADDR(u32 col_addr, u32 row_addr, u8 col_num, u8 row_num);
extern void PARALLEL_NFI_ISSUE_CMD_1(u8 cmd1);
extern void PARALLEL_NFI_RESET(void);
extern void PARALLEL_NFI_SET_TIMING(u32 timing);
extern void PARALLEL_NFI_SET_CHIP_SELECT(u8 chip);
extern void PARALLEL_NFI_INIT(void);
#endif



#endif /* ifndef __SPI_NFI_H__ */
/* End of [spi_nfi.h] package */						   
