/* Driver for Realtek RTS51xx USB card reader
 * Header file
 *
 * Copyright(c) 2009 Realtek Semiconductor Corp. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 *   wwang (wei_wang@realsil.com.cn)
 *   No. 450, Shenhu Road, Suzhou Industry Park, Suzhou, China
 * Maintainer:
 *   Edwin Rong (edwin_rong@realsil.com.cn)
 *   No. 450, Shenhu Road, Suzhou Industry Park, Suzhou, China
 */

#ifndef __RTS51X_MS_H
#define __RTS51X_MS_H

#include "rts51x_chip.h"

#define	MS_MAX_RETRY_COUNT		3

#define	MS_EXTRA_SIZE			0x9

#define	WRT_PRTCT			0x01

/* Error Code */
#define	MS_NO_ERROR				0x00
#define	MS_CRC16_ERROR				0x80
#define	MS_TO_ERROR				0x40
#define	MS_NO_CARD				0x20
#define	MS_NO_MEMORY				0x10
#define	MS_CMD_NK				0x08
#define	MS_FLASH_READ_ERROR			0x04
#define	MS_FLASH_WRITE_ERROR			0x02
#define	MS_BREQ_ERROR				0x01
#define	MS_NOT_FOUND				0x03

/* Transfer Protocol Command */
#define READ_PAGE_DATA				0x02
#define READ_REG				0x04
#define	GET_INT					0x07
#define WRITE_PAGE_DATA				0x0D
#define WRITE_REG				0x0B
#define SET_RW_REG_ADRS				0x08
#define SET_CMD					0x0E

#define	PRO_READ_LONG_DATA			0x02
#define	PRO_READ_SHORT_DATA			0x03
#define PRO_READ_REG				0x04
#define	PRO_READ_QUAD_DATA			0x05
#define PRO_GET_INT				0x07
#define	PRO_WRITE_LONG_DATA			0x0D
#define	PRO_WRITE_SHORT_DATA			0x0C
#define	PRO_WRITE_QUAD_DATA			0x0A
#define PRO_WRITE_REG				0x0B
#define PRO_SET_RW_REG_ADRS			0x08
#define PRO_SET_CMD				0x0E
#define PRO_EX_SET_CMD				0x09

#ifdef SUPPORT_MAGIC_GATE
#define MG_GET_ID		0x40
#define MG_SET_LID		0x41
#define MG_GET_LEKB		0x42
#define MG_SET_RD		0x43
#define MG_MAKE_RMS		0x44
#define MG_MAKE_KSE		0x45
#define MG_SET_IBD		0x46
#define MG_GET_IBD		0x47
#endif

#ifdef XC_POWERCLASS
#define XC_CHG_POWER		0x16
#endif

/* ++ CMD over Memory Stick */
/*  Flash CMD */
#define BLOCK_READ		0xAA
#define	BLOCK_WRITE		0x55
#define BLOCK_END		0x33
#define BLOCK_ERASE		0x99
#define FLASH_STOP		0xCC

/*  Function CMD */
#define SLEEP			0x5A
#define CLEAR_BUF		0xC3
#define MS_RESET		0x3C
/* -- CMD over Memory Stick */

/* ++ CMD over Memory Stick Pro */
/*  Flash CMD */
#define PRO_READ_DATA		0x20
#define	PRO_WRITE_DATA		0x21
#define PRO_READ_ATRB		0x24
#define PRO_STOP		0x25
#define PRO_ERASE		0x26
#define	PRO_READ_2K_DATA	0x27
#define	PRO_WRITE_2K_DATA	0x28

/*  Function CMD */
#define PRO_FORMAT		0x10
#define PRO_SLEEP		0x11
/* -- CMD over Memory Stick Pro */

/*  register inside memory stick */
#define	IntReg			0x01
#define StatusReg0		0x02
#define StatusReg1		0x03

#define SystemParm		0x10
#define BlockAdrs		0x11
#define CMDParm			0x14
#define PageAdrs		0x15

#define OverwriteFlag		0x16
#define ManagemenFlag		0x17
#define LogicalAdrs		0x18
#define ReserveArea		0x1A

/*  register inside memory pro */
#define	Pro_IntReg		0x01
#define Pro_StatusReg		0x02
#define Pro_TypeReg		0x04
#define	Pro_IFModeReg		0x05
#define Pro_CatagoryReg		0x06
#define Pro_ClassReg		0x07

#define Pro_SystemParm		0x10
#define Pro_DataCount1		0x11
#define Pro_DataCount0		0x12
#define Pro_DataAddr3		0x13
#define Pro_DataAddr2		0x14
#define Pro_DataAddr1		0x15
#define Pro_DataAddr0		0x16

#define Pro_TPCParm		0x17
#define Pro_CMDParm		0x18

/*  define for INT Register */
#define	INT_REG_CED		0x80
#define	INT_REG_ERR		0x40
#define	INT_REG_BREQ		0x20
#define	INT_REG_CMDNK		0x01

/*  INT signal */
#define	INT_CED			0x01
#define	INT_ERR			0x02
#define	INT_BREQ		0x04
#define	INT_CMDNK		0x08

/*  define for OverwriteFlag Register */
#define	BLOCK_BOOT		0xC0
#define	BLOCK_OK		0x80
#define	PAGE_OK			0x60
#define	DATA_COMPL		0x10

/*  define for ManagemenFlag Register */
#define	NOT_BOOT_BLOCK		0x4
#define	NOT_TRANSLATION_TABLE	0x8

/*  Header */
#define	HEADER_ID0		(PPBUF_BASE2)			/* 0 */
#define	HEADER_ID1		(PPBUF_BASE2 + 1)		/* 1 */
/*  System Entry */
#define	DISABLED_BLOCK0		(PPBUF_BASE2 + 0x170 + 4)	/* 2 */
#define	DISABLED_BLOCK1		(PPBUF_BASE2 + 0x170 + 5)	/* 3 */
#define	DISABLED_BLOCK2		(PPBUF_BASE2 + 0x170 + 6)	/* 4 */
#define	DISABLED_BLOCK3		(PPBUF_BASE2 + 0x170 + 7)	/* 5 */
/*  Boot & Attribute Information */
#define	BLOCK_SIZE_0		(PPBUF_BASE2 + 0x1a0 + 2)	/* 6 */
#define	BLOCK_SIZE_1		(PPBUF_BASE2 + 0x1a0 + 3)	/* 7 */
#define	BLOCK_COUNT_0		(PPBUF_BASE2 + 0x1a0 + 4)	/* 8 */
#define	BLOCK_COUNT_1		(PPBUF_BASE2 + 0x1a0 + 5)	/* 9 */
#define	EBLOCK_COUNT_0		(PPBUF_BASE2 + 0x1a0 + 6)	/* 10 */
#define	EBLOCK_COUNT_1		(PPBUF_BASE2 + 0x1a0 + 7)	/* 11 */
#define	PAGE_SIZE_0		(PPBUF_BASE2 + 0x1a0 + 8)	/* 12 */
#define	PAGE_SIZE_1		(PPBUF_BASE2 + 0x1a0 + 9)	/* 13 */

/* joey 2004-08-07 for MS check Procedure */
#define MS_Device_Type	(PPBUF_BASE2 + 0x1D8)	/* 14 */
/* end */

/* joey 2004-05-03 */
#define	MS_4bit_Support	(PPBUF_BASE2 + 0x1D3)	/* 15 */
/* end */

#define setPS_NG	1
#define setPS_Error	0

/*  define for Pro_SystemParm Register */
#define	PARALLEL_8BIT_IF	0x40
#define	PARALLEL_4BIT_IF	0x00
#define	SERIAL_IF		0x80

/*  define for StatusReg0 Register */
#define BUF_FULL	0x10
#define BUF_EMPTY	0x20

/*  define for StatusReg1 Register */
#define	MEDIA_BUSY	0x80
#define	FLASH_BUSY	0x40
#define	DATA_ERROR	0x20
#define	STS_UCDT	0x10
#define	EXTRA_ERROR	0x08
#define	STS_UCEX	0x04
#define	FLAG_ERROR	0x02
#define	STS_UCFG	0x01

#define MS_SHORT_DATA_LEN	32

#define FORMAT_SUCCESS		0
#define FORMAT_FAIL		1
#define FORMAT_IN_PROGRESS	2

#define	MS_SET_BAD_BLOCK_FLG(ms_card)	((ms_card)->multi_flag |= 0x80)
#define MS_CLR_BAD_BLOCK_FLG(ms_card)	((ms_card)->multi_flag &= 0x7F)
#define MS_TST_BAD_BLOCK_FLG(ms_card)	((ms_card)->multi_flag & 0x80)

#define CHECK_MS_TRANS_FAIL(chip, retval)	\
	(((retval) != STATUS_SUCCESS) || \
	(chip->rsp_buf[0] & MS_TRANSFER_ERR))

void rts51x_mspro_polling_format_status(struct rts51x_chip *chip);
void rts51x_mspro_format_sense(struct rts51x_chip *chip, unsigned int lun);

int rts51x_reset_ms_card(struct rts51x_chip *chip);
int rts51x_ms_rw(struct scsi_cmnd *srb, struct rts51x_chip *chip, u32 start_sector,
	  u16 sector_cnt);
int rts51x_mspro_format(struct scsi_cmnd *srb, struct rts51x_chip *chip,
		 int short_data_len, int quick_format);
void rts51x_ms_free_l2p_tbl(struct rts51x_chip *chip);
void rts51x_ms_cleanup_work(struct rts51x_chip *chip);
int rts51x_release_ms_card(struct rts51x_chip *chip);
int rts51x_ms_delay_write(struct rts51x_chip *chip);

#ifdef SUPPORT_MAGIC_GATE

int ms_switch_clock(struct rts51x_chip *chip);
int ms_write_bytes(struct rts51x_chip *chip, u8 tpc, u8 cnt, u8 cfg, u8 *data,
		   int data_len);
int ms_read_bytes(struct rts51x_chip *chip, u8 tpc, u8 cnt, u8 cfg, u8 *data,
		  int data_len);
int ms_set_rw_reg_addr(struct rts51x_chip *chip, u8 read_start, u8 read_cnt,
		       u8 write_start, u8 write_cnt);
int ms_transfer_data(struct rts51x_chip *chip, u8 trans_mode, u8 tpc,
		     u16 sec_cnt, u8 cfg, int mode_2k, int use_sg, void *buf,
		     int buf_len);
#endif

#endif /* __RTS51X_MS_H */
