/******************************************************************************
**
** FILE NAME    : ifxmips_atm_ppe_danube.h
** PROJECT      : UEIP
** MODULES     	: ATM (ADSL)
**
** DATE         : 1 AUG 2005
** AUTHOR       : Xu Liang
** DESCRIPTION  : ATM Driver (PPE Registers)
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



#ifndef IFXMIPS_ATM_PPE_DANUBE_H
#define IFXMIPS_ATM_PPE_DANUBE_H



/*
 *  FPI Configuration Bus Register and Memory Address Mapping
 */
#define IFX_PPE                      	(KSEG1 | 0x1E180000)
#define PP32_DEBUG_REG_ADDR(i, x)       ((volatile unsigned int*)(IFX_PPE + (((x) + 0x0000) << 2)))
#define PPM_INT_REG_ADDR(i, x)          ((volatile unsigned int*)(IFX_PPE + (((x) + 0x0030) << 2)))
#define PP32_INTERNAL_RES_ADDR(i, x)    ((volatile unsigned int*)(IFX_PPE + (((x) + 0x0040) << 2)))
#define CDM_CODE_MEMORY(i, x)           ((volatile unsigned int*)(IFX_PPE + (((x) + 0x1000) << 2)))
#define PPE_REG_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x4000) << 2)))
#define CDM_DATA_MEMORY(i, x)           ((volatile unsigned int*)(IFX_PPE + (((x) + 0x5000) << 2)))
#define PPM_INT_UNIT_ADDR(x)            ((volatile unsigned int*)(IFX_PPE + (((x) + 0x6000) << 2)))
#define PPM_TIMER0_ADDR(x)              ((volatile unsigned int*)(IFX_PPE + (((x) + 0x6100) << 2)))
#define PPM_TASK_IND_REG_ADDR(x)        ((volatile unsigned int*)(IFX_PPE + (((x) + 0x6200) << 2)))
#define PPS_BRK_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x6300) << 2)))
#define PPM_TIMER1_ADDR(x)              ((volatile unsigned int*)(IFX_PPE + (((x) + 0x6400) << 2)))
#define SB_RAM0_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x8000) << 2)))
#define SB_RAM1_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x8400) << 2)))
#define SB_RAM2_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x8C00) << 2)))
#define SB_RAM3_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x9600) << 2)))
#define QSB_CONF_REG_ADDR(x)            ((volatile unsigned int*)(IFX_PPE + (((x) + 0xC000) << 2)))

/*
 *  DWORD-Length of Memory Blocks
 */
#define PP32_DEBUG_REG_DWLEN            0x0030
#define PPM_INT_REG_DWLEN               0x0010
#define PP32_INTERNAL_RES_DWLEN         0x00C0
#define CDM_CODE_MEMORYn_DWLEN(n)       ((n) == 0 ? 0x1000 : 0x0800)
#define PPE_REG_DWLEN                   0x1000
#define CDM_DATA_MEMORY_DWLEN           CDM_CODE_MEMORYn_DWLEN(1)
#define PPM_INT_UNIT_DWLEN              0x0100
#define PPM_TIMER0_DWLEN                0x0100
#define PPM_TASK_IND_REG_DWLEN          0x0100
#define PPS_BRK_DWLEN                   0x0100
#define PPM_TIMER1_DWLEN                0x0100
#define SB_RAM0_DWLEN                   0x0400
#define SB_RAM1_DWLEN                   0x0800
#define SB_RAM2_DWLEN                   0x0A00
#define SB_RAM3_DWLEN                   0x0400
#define QSB_CONF_REG_DWLEN              0x0100

/*
 *  PP32 to FPI Address Mapping
 */
#define SB_BUFFER(__sb_addr)            ((volatile unsigned int *)((((__sb_addr) >= 0x2000) && ((__sb_addr) <= 0x23FF)) ? SB_RAM0_ADDR((__sb_addr) - 0x2000) :   \
                                                                   (((__sb_addr) >= 0x2400) && ((__sb_addr) <= 0x2BFF)) ? SB_RAM1_ADDR((__sb_addr) - 0x2400) :   \
                                                                   (((__sb_addr) >= 0x2C00) && ((__sb_addr) <= 0x35FF)) ? SB_RAM2_ADDR((__sb_addr) - 0x2C00) :   \
                                                                   (((__sb_addr) >= 0x3600) && ((__sb_addr) <= 0x39FF)) ? SB_RAM3_ADDR((__sb_addr) - 0x3600) :   \
                                                                0))

/*
 *  PP32 Debug Control Register
 */
#define PP32_DBG_CTRL                   PP32_DEBUG_REG_ADDR(0, 0x0000)

#define DBG_CTRL_START_SET(value)       ((value) ? (1 << 0) : 0)
#define DBG_CTRL_STOP_SET(value)        ((value) ? (1 << 1) : 0)
#define DBG_CTRL_STEP_SET(value)        ((value) ? (1 << 2) : 0)

#define PP32_HALT_STAT                  PP32_DEBUG_REG_ADDR(0, 0x0001)

#define PP32_BRK_SRC                    PP32_DEBUG_REG_ADDR(0, 0x0002)

#define PP32_DBG_PC_MIN(i)              PP32_DEBUG_REG_ADDR(0, 0x0010 + (i))
#define PP32_DBG_PC_MAX(i)              PP32_DEBUG_REG_ADDR(0, 0x0014 + (i))
#define PP32_DBG_DATA_MIN(i)            PP32_DEBUG_REG_ADDR(0, 0x0018 + (i))
#define PP32_DBG_DATA_MAX(i)            PP32_DEBUG_REG_ADDR(0, 0x001A + (i))
#define PP32_DBG_DATA_VAL(i)            PP32_DEBUG_REG_ADDR(0, 0x001C + (i))

#define PP32_DBG_CUR_PC                 PP32_DEBUG_REG_ADDR(0, 0x0080)

#define PP32_DBG_TASK_NO                PP32_DEBUG_REG_ADDR(0, 0x0081)

#define PP32_DBG_REG_BASE(tsk, i)       PP32_DEBUG_REG_ADDR(0, 0x0100 + (tsk) * 16 + (i))

/*
 *  EMA Registers
 */
#define EMA_CMDCFG                      PPE_REG_ADDR(0x0A00)
#define EMA_DATACFG                     PPE_REG_ADDR(0x0A01)
#define EMA_CMDCNT                      PPE_REG_ADDR(0x0A02)
#define EMA_DATACNT                     PPE_REG_ADDR(0x0A03)
#define EMA_ISR                         PPE_REG_ADDR(0x0A04)
#define EMA_IER                         PPE_REG_ADDR(0x0A05)
#define EMA_CFG                         PPE_REG_ADDR(0x0A06)
#define EMA_SUBID                       PPE_REG_ADDR(0x0A07)

#define EMA_ALIGNMENT                   4

/*
 *  Mailbox IGU1 Interrupt
 */
#define PPE_MAILBOX_IGU1_INT            INT_NUM_IM2_IRL24



#endif  //  IFXMIPS_ATM_PPE_DANUBE_H
