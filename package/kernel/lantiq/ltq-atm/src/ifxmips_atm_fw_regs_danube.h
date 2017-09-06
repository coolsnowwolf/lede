/******************************************************************************
**
** FILE NAME    : ifxmips_atm_fw_regs_danube.h
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



#ifndef IFXMIPS_ATM_FW_REGS_DANUBE_H
#define IFXMIPS_ATM_FW_REGS_DANUBE_H

#define FW_VER_ID                       ((volatile struct fw_ver_id *)      SB_BUFFER(0x2001))
#define CFG_WRX_HTUTS                   SB_BUFFER(0x2400)   /*  WAN RX HTU Table Size, must be configured before enable PPE firmware.   */
#define CFG_WRX_QNUM                    SB_BUFFER(0x2401)   /*  WAN RX Queue Number */
#define CFG_WRX_DCHNUM                  SB_BUFFER(0x2402)   /*  WAN RX DMA Channel Number, no more than 8, must be configured before enable PPE firmware.   */
#define CFG_WTX_DCHNUM                  SB_BUFFER(0x2403)   /*  WAN TX DMA Channel Number, no more than 16, must be configured before enable PPE firmware.  */
#define CFG_WRDES_DELAY                 SB_BUFFER(0x2404)   /*  WAN Descriptor Write Delay, must be configured before enable PPE firmware.  */
#define WRX_DMACH_ON                    SB_BUFFER(0x2405)   /*  WAN RX DMA Channel Enable, must be configured before enable PPE firmware.   */
#define WTX_DMACH_ON                    SB_BUFFER(0x2406)   /*  WAN TX DMA Channel Enable, must be configured before enable PPE firmware.   */
#define WRX_HUNT_BITTH                  SB_BUFFER(0x2407)   /*  WAN RX HUNT Threshold, must be between 2 to 8.  */

#define WRX_QUEUE_CONFIG(i)             ((struct wrx_queue_config*)         SB_BUFFER(0x2500 + (i) * 20))
#define WRX_DMA_CHANNEL_CONFIG(i)       ((struct wrx_dma_channel_config*)   SB_BUFFER(0x2640 + (i) * 7))
#define WTX_PORT_CONFIG(i)              ((struct wtx_port_config*)          SB_BUFFER(0x2440 + (i)))
#define WTX_QUEUE_CONFIG(i)             ((struct wtx_queue_config*)         SB_BUFFER(0x2710 + (i) * 27))
#define WTX_DMA_CHANNEL_CONFIG(i)       ((struct wtx_dma_channel_config*)   SB_BUFFER(0x2711 + (i) * 27))
#define WAN_MIB_TABLE                   ((struct wan_mib_table*)            SB_BUFFER(0x2410))
#define HTU_ENTRY(i)                  ((struct htu_entry*)                SB_BUFFER(0x2000 + (i)))
#define HTU_MASK(i)                   ((struct htu_mask*)                 SB_BUFFER(0x2020 + (i)))
#define HTU_RESULT(i)                 ((struct htu_result*)               SB_BUFFER(0x2040 + (i)))

#endif
