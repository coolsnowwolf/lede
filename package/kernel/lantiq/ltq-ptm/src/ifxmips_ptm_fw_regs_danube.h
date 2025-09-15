/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_fw_regs_danube.h
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver header file (firmware register for Danube)
** COPYRIGHT    :       Copyright (c) 2006
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
** 07 JUL 2009  Xu Liang        Init Version
*******************************************************************************/



#ifndef IFXMIPS_PTM_FW_REGS_DANUBE_H
#define IFXMIPS_PTM_FW_REGS_DANUBE_H



/*
 *  Host-PPE Communication Data Address Mapping
 */
#define FW_VER_ID                       ((volatile struct fw_ver_id *)              SB_BUFFER(0x2001))
#define CFG_WAN_WRDES_DELAY             SB_BUFFER(0x2404)
#define CFG_WRX_DMACH_ON                SB_BUFFER(0x2405)
#define CFG_WTX_DMACH_ON                SB_BUFFER(0x2406)
#define CFG_WRX_LOOK_BITTH              SB_BUFFER(0x2407)
#define CFG_ETH_EFMTC_CRC               ((volatile struct eth_efmtc_crc_cfg *)      SB_BUFFER(0x2408))
#define WAN_MIB_TABLE                   ((volatile struct wan_mib_table*)           SB_BUFFER(0x2440))
#define WRX_PORT_CONFIG(i)              ((volatile struct wrx_port_cfg_status*)     SB_BUFFER(0x2500 + (i) * 20))
#define WRX_DMA_CHANNEL_CONFIG(i)       ((volatile struct wrx_dma_channel_config*)  SB_BUFFER(0x2640 + (i) * 7))
#define WTX_PORT_CONFIG(i)              ((volatile struct wtx_port_cfg*)            SB_BUFFER(0x2710 + (i) * 31))
#define WTX_DMA_CHANNEL_CONFIG(i)       ((volatile struct wtx_dma_channel_config*)  SB_BUFFER(0x2711 + (i) * 31))



#endif  //  IFXMIPS_PTM_FW_REGS_DANUBE_H
