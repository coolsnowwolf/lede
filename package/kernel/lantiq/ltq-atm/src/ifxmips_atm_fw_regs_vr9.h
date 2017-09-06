/******************************************************************************
**
** FILE NAME    : ifxmips_atm_fw_regs_vr9.h
** PROJECT      : UEIP
** MODULES     	: ATM (ADSL)
**
** DATE         : 1 AUG 2005
** AUTHOR       : Xu Liang
** DESCRIPTION  : ATM Driver (Firmware Registers)
** COPYRIGHT    : 	Copyright (c) 2006
**			Infineon Technologies AG
**			Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
**  4 AUG 2005  Xu Liang        Initiate Version
** 23 OCT 2006  Xu Liang        Add GPL header.
**  9 JAN 2007  Xu Liang        First version got from Anand (IC designer)
*******************************************************************************/



#ifndef IFXMIPS_ATM_FW_REGS_VR9_H
#define IFXMIPS_ATM_FW_REGS_VR9_H

#define FW_VER_ID		((volatile struct fw_ver_id *) SB_BUFFER(0x2001))

/* WAN RX HTU Table Size, must be configured before enable PPE firmware. */
#define CFG_WRX_HTUTS			SB_BUFFER(0x2010)
/* WAN RX Queue Number */
#define CFG_WRX_QNUM			SB_BUFFER(0x2011)
/* WAN RX DMA Channel Number, no more than 8, must be configured before enable PPE firmware. */
#define CFG_WRX_DCHNUM			SB_BUFFER(0x2012)
/* WAN TX DMA Channel Number, no more than 16, must be configured before enable PPE firmware. */
#define CFG_WTX_DCHNUM			SB_BUFFER(0x2013)
/* WAN Descriptor Write Delay, must be configured before enable PPE firmware. */
#define CFG_WRDES_DELAY			SB_BUFFER(0x2014)
/* WAN RX DMA Channel Enable, must be configured before enable PPE firmware. */
#define WRX_DMACH_ON			SB_BUFFER(0x2015)
/* WAN TX DMA Channel Enable, must be configured before enable PPE firmware. */
#define WTX_DMACH_ON			SB_BUFFER(0x2016)
/* WAN RX HUNT Threshold, must be between 2 to 8. */
#define WRX_HUNT_BITTH			SB_BUFFER(0x2017)
/*  i < 16  */
#define WRX_QUEUE_CONFIG(i)		((struct wrx_queue_config *) SB_BUFFER(0x4C00 + (i) * 20))
/*  i < 8   */
#define WRX_DMA_CHANNEL_CONFIG(i)	((struct wrx_dma_channel_config *) SB_BUFFER(0x4F80 + (i) * 7))
/*  i < 2   */
#define WTX_PORT_CONFIG(i)		((struct wtx_port_config *) SB_BUFFER(0x4FB8 + (i)))
/*  i < 16  */
#define WTX_QUEUE_CONFIG(i)		((struct wtx_queue_config *) SB_BUFFER(0x3A00 + (i) * 27))
/*  i < 16  */
#define WTX_DMA_CHANNEL_CONFIG(i)	((struct wtx_dma_channel_config *) SB_BUFFER(0x3A01 + (i) * 27))

#define WAN_MIB_TABLE			((struct wan_mib_table *) SB_BUFFER(0x4EF0))
/*  i < 32  */
#define HTU_ENTRY(i)			((struct htu_entry *) SB_BUFFER(0x26A0 + (i)))
/*  i < 32  */
#define HTU_MASK(i)			((struct htu_mask *) SB_BUFFER(0x26C0 + (i)))
/*  i < 32  */
#define HTU_RESULT(i)			((struct htu_result *) SB_BUFFER(0x26E0 + (i)))
/* bit 0~3 - 0x0F: in showtime, 0x00: not in showtime */
#define UTP_CFG				SB_BUFFER(0x2018)



#endif  //  IFXMIPS_ATM_FW_REGS_VR9_H
