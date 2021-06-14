/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_ppe_common.h
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver header file (PPE register for all platform)
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



#ifndef IFXMIPS_PTM_PPE_COMMON_H
#define IFXMIPS_PTM_PPE_COMMON_H



#if defined(CONFIG_DANUBE)
  #include "ifxmips_ptm_ppe_danube.h"
#elif defined(CONFIG_AMAZON_SE)
  #include "ifxmips_ptm_ppe_amazon_se.h"
#elif defined(CONFIG_AR9)
  #include "ifxmips_ptm_ppe_ar9.h"
#elif defined(CONFIG_VR9)
  #include "ifxmips_ptm_ppe_vr9.h"
#else
  #error Platform is not specified!
#endif



/*
 *  Code/Data Memory (CDM) Interface Configuration Register
 */
#define CDM_CFG                         PPE_REG_ADDR(0x0100)

#define CDM_CFG_RAM1                    GET_BITS(*CDM_CFG, 3, 2)
#define CDM_CFG_RAM0                    (*CDM_CFG & (1 << 1))

#define CDM_CFG_RAM1_SET(value)         SET_BITS(0, 3, 2, value)
#define CDM_CFG_RAM0_SET(value)         ((value) ? (1 << 1) : 0)

/*
 *  QSB Internal Cell Delay Variation Register
 */
#define QSB_ICDV                        QSB_CONF_REG_ADDR(0x0007)

#define QSB_ICDV_TAU                    GET_BITS(*QSB_ICDV, 5, 0)

#define QSB_ICDV_TAU_SET(value)         SET_BITS(0, 5, 0, value)

/*
 *  QSB Scheduler Burst Limit Register
 */
#define QSB_SBL                         QSB_CONF_REG_ADDR(0x0009)

#define QSB_SBL_SBL                     GET_BITS(*QSB_SBL, 3, 0)

#define QSB_SBL_SBL_SET(value)          SET_BITS(0, 3, 0, value)

/*
 *  QSB Configuration Register
 */
#define QSB_CFG                         QSB_CONF_REG_ADDR(0x000A)

#define QSB_CFG_TSTEPC                  GET_BITS(*QSB_CFG, 1, 0)

#define QSB_CFG_TSTEPC_SET(value)       SET_BITS(0, 1, 0, value)

/*
 *  QSB RAM Transfer Table Register
 */
#define QSB_RTM                         QSB_CONF_REG_ADDR(0x000B)

#define QSB_RTM_DM                      (*QSB_RTM)

#define QSB_RTM_DM_SET(value)           ((value) & 0xFFFFFFFF)

/*
 *  QSB RAM Transfer Data Register
 */
#define QSB_RTD                         QSB_CONF_REG_ADDR(0x000C)

#define QSB_RTD_TTV                     (*QSB_RTD)

#define QSB_RTD_TTV_SET(value)          ((value) & 0xFFFFFFFF)

/*
 *  QSB RAM Access Register
 */
#define QSB_RAMAC                       QSB_CONF_REG_ADDR(0x000D)

#define QSB_RAMAC_RW                    (*QSB_RAMAC & (1 << 31))
#define QSB_RAMAC_TSEL                  GET_BITS(*QSB_RAMAC, 27, 24)
#define QSB_RAMAC_LH                    (*QSB_RAMAC & (1 << 16))
#define QSB_RAMAC_TESEL                 GET_BITS(*QSB_RAMAC, 9, 0)

#define QSB_RAMAC_RW_SET(value)         ((value) ? (1 << 31) : 0)
#define QSB_RAMAC_TSEL_SET(value)       SET_BITS(0, 27, 24, value)
#define QSB_RAMAC_LH_SET(value)         ((value) ? (1 << 16) : 0)
#define QSB_RAMAC_TESEL_SET(value)      SET_BITS(0, 9, 0, value)

/*
 *  QSB Queue Scheduling and Shaping Definitions
 */
#define QSB_WFQ_NONUBR_MAX              0x3f00
#define QSB_WFQ_UBR_BYPASS              0x3fff
#define QSB_TP_TS_MAX                   65472
#define QSB_TAUS_MAX                    64512
#define QSB_GCR_MIN                     18

/*
 *  QSB Constant
 */
#define QSB_RAMAC_RW_READ               0
#define QSB_RAMAC_RW_WRITE              1

#define QSB_RAMAC_TSEL_QPT              0x01
#define QSB_RAMAC_TSEL_SCT              0x02
#define QSB_RAMAC_TSEL_SPT              0x03
#define QSB_RAMAC_TSEL_VBR              0x08

#define QSB_RAMAC_LH_LOW                0
#define QSB_RAMAC_LH_HIGH               1

#define QSB_QPT_SET_MASK                0x0
#define QSB_QVPT_SET_MASK               0x0
#define QSB_SET_SCT_MASK                0x0
#define QSB_SET_SPT_MASK                0x0
#define QSB_SET_SPT_SBVALID_MASK        0x7FFFFFFF

#define QSB_SPT_SBV_VALID               (1 << 31)
#define QSB_SPT_PN_SET(value)           (((value) & 0x01) ? (1 << 16) : 0)
#define QSB_SPT_INTRATE_SET(value)      SET_BITS(0, 13, 0, value)

/*
 *  QSB Queue Parameter Table Entry and Queue VBR Parameter Table Entry
 */
#if defined(__BIG_ENDIAN)
    union qsb_queue_parameter_table {
        struct {
            unsigned int    res1    :1;
            unsigned int    vbr     :1;
            unsigned int    wfqf    :14;
            unsigned int    tp      :16;
        }               bit;
        u32             dword;
    };

    union qsb_queue_vbr_parameter_table {
        struct {
            unsigned int    taus    :16;
            unsigned int    ts      :16;
        }               bit;
        u32             dword;
    };
#else
    union qsb_queue_parameter_table {
        struct {
            unsigned int    tp      :16;
            unsigned int    wfqf    :14;
            unsigned int    vbr     :1;
            unsigned int    res1    :1;
        }               bit;
        u32             dword;
    };

    union qsb_queue_vbr_parameter_table {
        struct {
            unsigned int    ts      :16;
            unsigned int    taus    :16;
        }               bit;
        u32             dword;
    };
#endif  //  defined(__BIG_ENDIAN)

/*
 *  Mailbox IGU0 Registers
 */
#define MBOX_IGU0_ISRS                  PPE_REG_ADDR(0x0200)
#define MBOX_IGU0_ISRC                  PPE_REG_ADDR(0x0201)
#define MBOX_IGU0_ISR                   PPE_REG_ADDR(0x0202)
#define MBOX_IGU0_IER                   PPE_REG_ADDR(0x0203)

#define MBOX_IGU0_ISRS_SET(n)           (1 << (n))
#define MBOX_IGU0_ISRC_CLEAR(n)         (1 << (n))
#define MBOX_IGU0_ISR_ISR(n)            (*MBOX_IGU0_ISR & (1 << (n)))
#define MBOX_IGU0_IER_EN(n)             (*MBOX_IGU0_IER & (1 << (n)))
#define MBOX_IGU0_IER_EN_SET(n)         (1 << (n))

/*
 *  Mailbox IGU1 Registers
 */
#define MBOX_IGU1_ISRS                  PPE_REG_ADDR(0x0204)
#define MBOX_IGU1_ISRC                  PPE_REG_ADDR(0x0205)
#define MBOX_IGU1_ISR                   PPE_REG_ADDR(0x0206)
#define MBOX_IGU1_IER                   PPE_REG_ADDR(0x0207)

#define MBOX_IGU1_ISRS_SET(n)           (1 << (n))
#define MBOX_IGU1_ISRC_CLEAR(n)         (1 << (n))
#define MBOX_IGU1_ISR_ISR(n)            (*MBOX_IGU1_ISR & (1 << (n)))
#define MBOX_IGU1_IER_EN(n)             (*MBOX_IGU1_IER & (1 << (n)))
#define MBOX_IGU1_IER_EN_SET(n)         (1 << (n))

/*
 *  Mailbox IGU3 Registers
 */
#define MBOX_IGU3_ISRS                  PPE_REG_ADDR(0x0214)
#define MBOX_IGU3_ISRC                  PPE_REG_ADDR(0x0215)
#define MBOX_IGU3_ISR                   PPE_REG_ADDR(0x0216)
#define MBOX_IGU3_IER                   PPE_REG_ADDR(0x0217)

#define MBOX_IGU3_ISRS_SET(n)           (1 << (n))
#define MBOX_IGU3_ISRC_CLEAR(n)         (1 << (n))
#define MBOX_IGU3_ISR_ISR(n)            (*MBOX_IGU3_ISR & (1 << (n)))
#define MBOX_IGU3_IER_EN(n)             (*MBOX_IGU3_IER & (1 << (n)))
#define MBOX_IGU3_IER_EN_SET(n)         (1 << (n))

/*
 *  RTHA/TTHA Registers
 */
#define RFBI_CFG                        PPE_REG_ADDR(0x0400)
#define RBA_CFG0                        PPE_REG_ADDR(0x0404)
#define RBA_CFG1                        PPE_REG_ADDR(0x0405)
#define RCA_CFG0                        PPE_REG_ADDR(0x0408)
#define RCA_CFG1                        PPE_REG_ADDR(0x0409)
#define RDES_CFG0                       PPE_REG_ADDR(0x040C)
#define RDES_CFG1                       PPE_REG_ADDR(0x040D)
#define SFSM_STATE0                     PPE_REG_ADDR(0x0410)
#define SFSM_STATE1                     PPE_REG_ADDR(0x0411)
#define SFSM_DBA0                       PPE_REG_ADDR(0x0412)
#define SFSM_DBA1                       PPE_REG_ADDR(0x0413)
#define SFSM_CBA0                       PPE_REG_ADDR(0x0414)
#define SFSM_CBA1                       PPE_REG_ADDR(0x0415)
#define SFSM_CFG0                       PPE_REG_ADDR(0x0416)
#define SFSM_CFG1                       PPE_REG_ADDR(0x0417)
#define SFSM_PGCNT0                     PPE_REG_ADDR(0x041C)
#define SFSM_PGCNT1                     PPE_REG_ADDR(0x041D)
#define FFSM_DBA0                       PPE_REG_ADDR(0x0508)
#define FFSM_DBA1                       PPE_REG_ADDR(0x0509)
#define FFSM_CFG0                       PPE_REG_ADDR(0x050A)
#define FFSM_CFG1                       PPE_REG_ADDR(0x050B)
#define FFSM_IDLE_HEAD_BC0              PPE_REG_ADDR(0x050E)
#define FFSM_IDLE_HEAD_BC1              PPE_REG_ADDR(0x050F)
#define FFSM_PGCNT0                     PPE_REG_ADDR(0x0514)
#define FFSM_PGCNT1                     PPE_REG_ADDR(0x0515)

/*
 *  PPE TC Logic Registers (partial)
 */
#define DREG_A_VERSION                  PPE_REG_ADDR(0x0D00)
#define DREG_A_CFG                      PPE_REG_ADDR(0x0D01)
#define DREG_AT_CTRL                    PPE_REG_ADDR(0x0D02)
#define DREG_AT_CB_CFG0                 PPE_REG_ADDR(0x0D03)
#define DREG_AT_CB_CFG1                 PPE_REG_ADDR(0x0D04)
#define DREG_AR_CTRL                    PPE_REG_ADDR(0x0D08)
#define DREG_AR_CB_CFG0                 PPE_REG_ADDR(0x0D09)
#define DREG_AR_CB_CFG1                 PPE_REG_ADDR(0x0D0A)
#define DREG_A_UTPCFG                   PPE_REG_ADDR(0x0D0E)
#define DREG_A_STATUS                   PPE_REG_ADDR(0x0D0F)
#define DREG_AT_CFG0                    PPE_REG_ADDR(0x0D20)
#define DREG_AT_CFG1                    PPE_REG_ADDR(0x0D21)
#define DREG_AT_FB_SIZE0                PPE_REG_ADDR(0x0D22)
#define DREG_AT_FB_SIZE1                PPE_REG_ADDR(0x0D23)
#define DREG_AT_CELL0                   PPE_REG_ADDR(0x0D24)
#define DREG_AT_CELL1                   PPE_REG_ADDR(0x0D25)
#define DREG_AT_IDLE_CNT0               PPE_REG_ADDR(0x0D26)
#define DREG_AT_IDLE_CNT1               PPE_REG_ADDR(0x0D27)
#define DREG_AT_IDLE0                   PPE_REG_ADDR(0x0D28)
#define DREG_AT_IDLE1                   PPE_REG_ADDR(0x0D29)
#define DREG_AR_CFG0                    PPE_REG_ADDR(0x0D60)
#define DREG_AR_CFG1                    PPE_REG_ADDR(0x0D61)
#define DREG_AR_CELL0                   PPE_REG_ADDR(0x0D68)
#define DREG_AR_CELL1                   PPE_REG_ADDR(0x0D69)
#define DREG_AR_IDLE_CNT0               PPE_REG_ADDR(0x0D6A)
#define DREG_AR_IDLE_CNT1               PPE_REG_ADDR(0x0D6B)
#define DREG_AR_AIIDLE_CNT0             PPE_REG_ADDR(0x0D6C)
#define DREG_AR_AIIDLE_CNT1             PPE_REG_ADDR(0x0D6D)
#define DREG_AR_BE_CNT0                 PPE_REG_ADDR(0x0D6E)
#define DREG_AR_BE_CNT1                 PPE_REG_ADDR(0x0D6F)
#define DREG_AR_HEC_CNT0                PPE_REG_ADDR(0x0D70)
#define DREG_AR_HEC_CNT1                PPE_REG_ADDR(0x0D71)
#define DREG_AR_IDLE0                   PPE_REG_ADDR(0x0D74)
#define DREG_AR_IDLE1                   PPE_REG_ADDR(0x0D75)
#define DREG_AR_CERRN_CNT0              PPE_REG_ADDR(0x0DA0)
#define DREG_AR_CERRN_CNT1              PPE_REG_ADDR(0x0DA1)
#define DREG_AR_CERRNP_CNT0             PPE_REG_ADDR(0x0DA2)
#define DREG_AR_CERRNP_CNT1             PPE_REG_ADDR(0x0DA3)
#define DREG_AR_CVN_CNT0                PPE_REG_ADDR(0x0DA4)
#define DREG_AR_CVN_CNT1                PPE_REG_ADDR(0x0DA5)
#define DREG_AR_CVNP_CNT0               PPE_REG_ADDR(0x0DA6)
#define DREG_AR_CVNP_CNT1               PPE_REG_ADDR(0x0DA7)
#define DREG_B0_LADR                    PPE_REG_ADDR(0x0DA8)
#define DREG_B1_LADR                    PPE_REG_ADDR(0x0DA9)

static inline int
ifx_drv_ver(char *buf, char *module, int major, int mid, int minor)
{
    return sprintf(buf, "Lantiq %s driver, version %d.%d.%d, (c) 2001-2013 Lantiq Deutschland GmbH\n",
                    module, major, mid, minor);
}


#endif  //  IFXMIPS_PTM_PPE_COMMON_H
