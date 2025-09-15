/******************************************************************************
**
** FILE NAME    : ifxmips_atm_ppe_vr9.h
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



#ifndef IFXMIPS_ATM_PPE_VR9_H
#define IFXMIPS_ATM_PPE_VR9_H



/*
 *  FPI Configuration Bus Register and Memory Address Mapping
 */
#define IFX_PPE                         (KSEG1 | 0x1E200000)
#define PP32_DEBUG_REG_ADDR(i, x)       ((volatile unsigned int*)(IFX_PPE + (((x) + 0x000000 + (i) * 0x00010000) << 2)))
#define CDM_CODE_MEMORY(i, x)           ((volatile unsigned int*)(IFX_PPE + (((x) + 0x001000 + (i) * 0x00010000) << 2)))
#define CDM_DATA_MEMORY(i, x)           ((volatile unsigned int*)(IFX_PPE + (((x) + 0x004000 + (i) * 0x00010000) << 2)))
#define SB_RAM0_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x008000) << 2)))
#define SB_RAM1_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x009000) << 2)))
#define SB_RAM2_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x00A000) << 2)))
#define SB_RAM3_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x00B000) << 2)))
#define PPE_REG_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x00D000) << 2)))
#define QSB_CONF_REG_ADDR(x)            ((volatile unsigned int*)(IFX_PPE + (((x) + 0x00E000) << 2)))
#define SB_RAM6_ADDR(x)                 ((volatile unsigned int*)(IFX_PPE + (((x) + 0x018000) << 2)))

/*
 *  DWORD-Length of Memory Blocks
 */
#define PP32_DEBUG_REG_DWLEN            0x0030
#define CDM_CODE_MEMORYn_DWLEN(n)       ((n) == 0 ? 0x1000 : 0x0800)
#define CDM_DATA_MEMORY_DWLEN           CDM_CODE_MEMORYn_DWLEN(1)
#define SB_RAM0_DWLEN                   0x1000
#define SB_RAM1_DWLEN                   0x1000
#define SB_RAM2_DWLEN                   0x1000
#define SB_RAM3_DWLEN                   0x1000
#define SB_RAM6_DWLEN                   0x8000
#define QSB_CONF_REG_DWLEN              0x0100

/*
 *  PP32 to FPI Address Mapping
 */
#define SB_BUFFER(__sb_addr)            ((volatile unsigned int *)((((__sb_addr) >= 0x0000) && ((__sb_addr) <= 0x1FFF)) ? PPE_REG_ADDR((__sb_addr)) :           \
                                                                   (((__sb_addr) >= 0x2000) && ((__sb_addr) <= 0x2FFF)) ? SB_RAM0_ADDR((__sb_addr) - 0x2000) :  \
                                                                   (((__sb_addr) >= 0x3000) && ((__sb_addr) <= 0x3FFF)) ? SB_RAM1_ADDR((__sb_addr) - 0x3000) :  \
                                                                   (((__sb_addr) >= 0x4000) && ((__sb_addr) <= 0x4FFF)) ? SB_RAM2_ADDR((__sb_addr) - 0x4000) :  \
                                                                   (((__sb_addr) >= 0x5000) && ((__sb_addr) <= 0x5FFF)) ? SB_RAM3_ADDR((__sb_addr) - 0x5000) :  \
                                                                   (((__sb_addr) >= 0x7000) && ((__sb_addr) <= 0x7FFF)) ? PPE_REG_ADDR((__sb_addr) - 0x7000) :  \
                                                                   (((__sb_addr) >= 0x8000) && ((__sb_addr) <= 0xFFFF)) ? SB_RAM6_ADDR((__sb_addr) - 0x8000) :  \
                                                                0))

/*
 *  PP32 Debug Control Register
 */
#define NUM_OF_PP32                             2

#define PP32_FREEZE                             PPE_REG_ADDR(0x0000)
#define PP32_SRST                               PPE_REG_ADDR(0x0020)

#define PP32_DBG_CTRL(n)                        PP32_DEBUG_REG_ADDR(n, 0x0000)

#define DBG_CTRL_RESTART                        0
#define DBG_CTRL_STOP                           1

#define PP32_CTRL_CMD(n)                        PP32_DEBUG_REG_ADDR(n, 0x0B00)
  #define PP32_CTRL_CMD_RESTART                 (1 << 0)
  #define PP32_CTRL_CMD_STOP                    (1 << 1)
  #define PP32_CTRL_CMD_STEP                    (1 << 2)
  #define PP32_CTRL_CMD_BREAKOUT                (1 << 3)

#define PP32_CTRL_OPT(n)                        PP32_DEBUG_REG_ADDR(n, 0x0C00)
  #define PP32_CTRL_OPT_BREAKOUT_ON_STOP_ON     (3 << 0)
  #define PP32_CTRL_OPT_BREAKOUT_ON_STOP_OFF    (2 << 0)
  #define PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN_ON  (3 << 2)
  #define PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN_OFF (2 << 2)
  #define PP32_CTRL_OPT_STOP_ON_BREAKIN_ON      (3 << 4)
  #define PP32_CTRL_OPT_STOP_ON_BREAKIN_OFF     (2 << 4)
  #define PP32_CTRL_OPT_STOP_ON_BREAKPOINT_ON   (3 << 6)
  #define PP32_CTRL_OPT_STOP_ON_BREAKPOINT_OFF  (2 << 6)
  #define PP32_CTRL_OPT_BREAKOUT_ON_STOP(n)     (*PP32_CTRL_OPT(n) & (1 << 0))
  #define PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN(n)  (*PP32_CTRL_OPT(n) & (1 << 2))
  #define PP32_CTRL_OPT_STOP_ON_BREAKIN(n)      (*PP32_CTRL_OPT(n) & (1 << 4))
  #define PP32_CTRL_OPT_STOP_ON_BREAKPOINT(n)   (*PP32_CTRL_OPT(n) & (1 << 6))

#define PP32_BRK_PC(n, i)                       PP32_DEBUG_REG_ADDR(n, 0x0900 + (i) * 2)
#define PP32_BRK_PC_MASK(n, i)                  PP32_DEBUG_REG_ADDR(n, 0x0901 + (i) * 2)
#define PP32_BRK_DATA_ADDR(n, i)                PP32_DEBUG_REG_ADDR(n, 0x0904 + (i) * 2)
#define PP32_BRK_DATA_ADDR_MASK(n, i)           PP32_DEBUG_REG_ADDR(n, 0x0905 + (i) * 2)
#define PP32_BRK_DATA_VALUE_RD(n, i)            PP32_DEBUG_REG_ADDR(n, 0x0908 + (i) * 2)
#define PP32_BRK_DATA_VALUE_RD_MASK(n, i)       PP32_DEBUG_REG_ADDR(n, 0x0909 + (i) * 2)
#define PP32_BRK_DATA_VALUE_WR(n, i)            PP32_DEBUG_REG_ADDR(n, 0x090C + (i) * 2)
#define PP32_BRK_DATA_VALUE_WR_MASK(n, i)       PP32_DEBUG_REG_ADDR(n, 0x090D + (i) * 2)
  #define PP32_BRK_CONTEXT_MASK(i)              (1 << (i))
  #define PP32_BRK_CONTEXT_MASK_EN              (1 << 4)
  #define PP32_BRK_COMPARE_GREATER_EQUAL        (1 << 5)    //  valid for break data value rd/wr only
  #define PP32_BRK_COMPARE_LOWER_EQUAL          (1 << 6)
  #define PP32_BRK_COMPARE_EN                   (1 << 7)

#define PP32_BRK_TRIG(n)                        PP32_DEBUG_REG_ADDR(n, 0x0F00)
  #define PP32_BRK_GRPi_PCn_ON(i, n)            ((3 << ((n) * 2)) << ((i) * 16))
  #define PP32_BRK_GRPi_PCn_OFF(i, n)           ((2 << ((n) * 2)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_ADDRn_ON(i, n)     ((3 << ((n) * 2 + 4)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_ADDRn_OFF(i, n)    ((2 << ((n) * 2 + 4)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_RDn_ON(i, n) ((3 << ((n) * 2 + 8)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_RDn_OFF(i, n)((2 << ((n) * 2 + 8)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_WRn_ON(i, n) ((3 << ((n) * 2 + 12)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_WRn_OFF(i, n)((2 << ((n) * 2 + 12)) << ((i) * 16))
  #define PP32_BRK_GRPi_PCn(k, i, n)            (*PP32_BRK_TRIG(k) & ((1 << ((n))) << ((i) * 8)))
  #define PP32_BRK_GRPi_DATA_ADDRn(k, i, n)     (*PP32_BRK_TRIG(k) & ((1 << ((n) + 2)) << ((i) * 8)))
  #define PP32_BRK_GRPi_DATA_VALUE_RDn(k, i, n) (*PP32_BRK_TRIG(k) & ((1 << ((n) + 4)) << ((i) * 8)))
  #define PP32_BRK_GRPi_DATA_VALUE_WRn(k, i, n) (*PP32_BRK_TRIG(k) & ((1 << ((n) + 6)) << ((i) * 8)))

#define PP32_CPU_STATUS(n)                      PP32_DEBUG_REG_ADDR(n, 0x0D00)
#define PP32_HALT_STAT(n)                       PP32_CPU_STATUS(n)
#define PP32_DBG_CUR_PC(n)                      PP32_CPU_STATUS(n)
  #define PP32_CPU_USER_STOPPED(n)              (*PP32_CPU_STATUS(n) & (1 << 0))
  #define PP32_CPU_USER_BREAKIN_RCV(n)          (*PP32_CPU_STATUS(n) & (1 << 1))
  #define PP32_CPU_USER_BREAKPOINT_MET(n)       (*PP32_CPU_STATUS(n) & (1 << 2))
  #define PP32_CPU_CUR_PC(n)                    (*PP32_CPU_STATUS(n) >> 16)

#define PP32_BREAKPOINT_REASONS(n)              PP32_DEBUG_REG_ADDR(n, 0x0A00)
  #define PP32_BRK_PC_MET(n, i)                 (*PP32_BREAKPOINT_REASONS(n) & (1 << (i)))
  #define PP32_BRK_DATA_ADDR_MET(n, i)          (*PP32_BREAKPOINT_REASONS(n) & (1 << ((i) + 2)))
  #define PP32_BRK_DATA_VALUE_RD_MET(n, i)      (*PP32_BREAKPOINT_REASONS(n) & (1 << ((i) + 4)))
  #define PP32_BRK_DATA_VALUE_WR_MET(n, i)      (*PP32_BREAKPOINT_REASONS(n) & (1 << ((i) + 6)))
  #define PP32_BRK_DATA_VALUE_RD_LO_EQ(n, i)    (*PP32_BREAKPOINT_REASONS(n) & (1 << ((i) * 2 + 8)))
  #define PP32_BRK_DATA_VALUE_RD_GT_EQ(n, i)    (*PP32_BREAKPOINT_REASONS(n) & (1 << ((i) * 2 + 9)))
  #define PP32_BRK_DATA_VALUE_WR_LO_EQ(n, i)    (*PP32_BREAKPOINT_REASONS(n) & (1 << ((i) * 2 + 12)))
  #define PP32_BRK_DATA_VALUE_WR_GT_EQ(n, i)    (*PP32_BREAKPOINT_REASONS(n) & (1 << ((i) * 2 + 13)))
  #define PP32_BRK_CUR_CONTEXT(n)               ((*PP32_BREAKPOINT_REASONS(n) >> 16) & 0x03)

#define PP32_GP_REG_BASE(n)                     PP32_DEBUG_REG_ADDR(n, 0x0E00)
#define PP32_GP_CONTEXTi_REGn(n, i, j)          PP32_DEBUG_REG_ADDR(n, 0x0E00 + (i) * 16 + (j))

/*
 *  PDMA/EMA Registers
 */
#define PDMA_CFG                        PPE_REG_ADDR(0x0A00)
#define PDMA_RX_CMDCNT                  PPE_REG_ADDR(0x0A01)
#define PDMA_TX_CMDCNT                  PPE_REG_ADDR(0x0A02)
#define PDMA_RX_FWDATACNT               PPE_REG_ADDR(0x0A03)
#define PDMA_TX_FWDATACNT               PPE_REG_ADDR(0x0A04)
#define PDMA_RX_CTX_CFG                 PPE_REG_ADDR(0x0A05)
#define PDMA_TX_CTX_CFG                 PPE_REG_ADDR(0x0A06)
#define PDMA_RX_MAX_LEN_REG             PPE_REG_ADDR(0x0A07)
#define PDMA_RX_DELAY_CFG               PPE_REG_ADDR(0x0A08)
#define PDMA_INT_FIFO_RD                PPE_REG_ADDR(0x0A09)
#define PDMA_ISR                        PPE_REG_ADDR(0x0A0A)
#define PDMA_IER                        PPE_REG_ADDR(0x0A0B)
#define PDMA_SUBID                      PPE_REG_ADDR(0x0A0C)
#define PDMA_BAR0                       PPE_REG_ADDR(0x0A0D)
#define PDMA_BAR1                       PPE_REG_ADDR(0x0A0E)

#define SAR_PDMA_RX_CMDBUF_CFG          PPE_REG_ADDR(0x0F00)
#define SAR_PDMA_TX_CMDBUF_CFG          PPE_REG_ADDR(0x0F01)
#define SAR_PDMA_RX_FW_CMDBUF_CFG       PPE_REG_ADDR(0x0F02)
#define SAR_PDMA_TX_FW_CMDBUF_CFG       PPE_REG_ADDR(0x0F03)
#define SAR_PDMA_RX_CMDBUF_STATUS       PPE_REG_ADDR(0x0F04)
#define SAR_PDMA_TX_CMDBUF_STATUS       PPE_REG_ADDR(0x0F05)

#define PDMA_ALIGNMENT                  4
#define EMA_ALIGNMENT                   PDMA_ALIGNMENT

/*
 *  Mailbox IGU1 Interrupt
 */
#define PPE_MAILBOX_IGU1_INT            INT_NUM_IM2_IRL24



#endif  //  IFXMIPS_ATM_PPE_VR9_H
