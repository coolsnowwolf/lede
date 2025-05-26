/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */
#ifndef __RTK_PHYLIB_DEF_H
#define __RTK_PHYLIB_DEF_H

#include "type.h"

//#define PHY_C22_MMD_PAGE            0
#define PHY_C22_MMD_PAGE            0x0A41
#define PHY_C22_MMD_DEV_REG         13
#define PHY_C22_MMD_ADD_REG         14

/* MDIO Manageable Device(MDD) address*/
#define PHY_MMD_PMAPMD              1
#define PHY_MMD_PCS                 3
#define PHY_MMD_AN                  7
#define PHY_MMD_VEND1               30   /* Vendor specific 1 */
#define PHY_MMD_VEND2               31   /* Vendor specific 2 */

#define BIT_0        0x00000001U
#define BIT_1        0x00000002U
#define BIT_2        0x00000004U
#define BIT_3        0x00000008U
#define BIT_4        0x00000010U
#define BIT_5        0x00000020U
#define BIT_6        0x00000040U
#define BIT_7        0x00000080U
#define BIT_8        0x00000100U
#define BIT_9        0x00000200U
#define BIT_10       0x00000400U
#define BIT_11       0x00000800U
#define BIT_12       0x00001000U
#define BIT_13       0x00002000U
#define BIT_14       0x00004000U
#define BIT_15       0x00008000U
#define BIT_16       0x00010000U
#define BIT_17       0x00020000U
#define BIT_18       0x00040000U
#define BIT_19       0x00080000U
#define BIT_20       0x00100000U
#define BIT_21       0x00200000U
#define BIT_22       0x00400000U
#define BIT_23       0x00800000U
#define BIT_24       0x01000000U
#define BIT_25       0x02000000U
#define BIT_26       0x04000000U
#define BIT_27       0x08000000U
#define BIT_28       0x10000000U
#define BIT_29       0x20000000U
#define BIT_30       0x40000000U
#define BIT_31       0x80000000U

#define MASK_1_BITS     (BIT_1 - 1)
#define MASK_2_BITS     (BIT_2 - 1)
#define MASK_3_BITS     (BIT_3 - 1)
#define MASK_4_BITS     (BIT_4 - 1)
#define MASK_5_BITS     (BIT_5 - 1)
#define MASK_6_BITS     (BIT_6 - 1)
#define MASK_7_BITS     (BIT_7 - 1)
#define MASK_8_BITS     (BIT_8 - 1)
#define MASK_9_BITS     (BIT_9 - 1)
#define MASK_10_BITS    (BIT_10 - 1)
#define MASK_11_BITS    (BIT_11 - 1)
#define MASK_12_BITS    (BIT_12 - 1)
#define MASK_13_BITS    (BIT_13 - 1)
#define MASK_14_BITS    (BIT_14 - 1)
#define MASK_15_BITS    (BIT_15 - 1)
#define MASK_16_BITS    (BIT_16 - 1)
#define MASK_17_BITS    (BIT_17 - 1)
#define MASK_18_BITS    (BIT_18 - 1)
#define MASK_19_BITS    (BIT_19 - 1)
#define MASK_20_BITS    (BIT_20 - 1)
#define MASK_21_BITS    (BIT_21 - 1)
#define MASK_22_BITS    (BIT_22 - 1)
#define MASK_23_BITS    (BIT_23 - 1)
#define MASK_24_BITS    (BIT_24 - 1)
#define MASK_25_BITS    (BIT_25 - 1)
#define MASK_26_BITS    (BIT_26 - 1)
#define MASK_27_BITS    (BIT_27 - 1)
#define MASK_28_BITS    (BIT_28 - 1)
#define MASK_29_BITS    (BIT_29 - 1)
#define MASK_30_BITS    (BIT_30 - 1)
#define MASK_31_BITS    (BIT_31 - 1)

#define REG32_FIELD_SET(_data, _val, _fOffset, _fMask)      ((_data & ~(_fMask)) | ((_val << (_fOffset)) & (_fMask)))
#define REG32_FIELD_GET(_data, _fOffset, _fMask)            ((_data & (_fMask)) >> (_fOffset))
#define UINT32_BITS_MASK(_mBit, _lBit)                      ((0xFFFFFFFF >> (31 - _mBit)) ^ ((1 << _lBit) - 1))

typedef struct phy_device *  rtk_port_t;

#if 1 /* ss\sdk\include\hal\phy\phydef.h */
/* unified patch format */
typedef enum rtk_phypatch_type_e
{
    PHY_PATCH_TYPE_NONE = 0,
    PHY_PATCH_TYPE_TOP = 1,
    PHY_PATCH_TYPE_SDS,
    PHY_PATCH_TYPE_AFE,
    PHY_PATCH_TYPE_UC,
    PHY_PATCH_TYPE_UC2,
    PHY_PATCH_TYPE_NCTL0,
    PHY_PATCH_TYPE_NCTL1,
    PHY_PATCH_TYPE_NCTL2,
    PHY_PATCH_TYPE_ALGXG,
    PHY_PATCH_TYPE_ALG1G,
    PHY_PATCH_TYPE_NORMAL,
    PHY_PATCH_TYPE_DATARAM,
    PHY_PATCH_TYPE_RTCT,
    PHY_PATCH_TYPE_END
} rtk_phypatch_type_t;

#define RTK_PATCH_TYPE_FLOW(_id)    (PHY_PATCH_TYPE_END + _id)
#define RTK_PATCH_TYPE_FLOWID_MAX   PHY_PATCH_TYPE_END
#define RTK_PATCH_SEQ_MAX     ( PHY_PATCH_TYPE_END + RTK_PATCH_TYPE_FLOWID_MAX -1)

typedef struct rtk_hwpatch_s
{
    uint8    patch_op;
    uint8    portmask;
    uint16   pagemmd;
    uint16   addr;
    uint8    msb;
    uint8    lsb;
    uint16   data;
    uint8    compare_op;
    uint16   sram_p;
    uint16   sram_rr;
    uint16   sram_rw;
    uint16   sram_a;
} rtk_hwpatch_t;

typedef struct rtk_hwpatch_data_s
{
    rtk_hwpatch_t *conf;
    uint32        size;
} rtk_hwpatch_data_t;

typedef struct rtk_hwpatch_seq_s
{
    uint8 patch_type;
    union
    {
        rtk_hwpatch_data_t data;
        uint8 flow_id;
    } patch;
} rtk_hwpatch_seq_t;

typedef struct rt_phy_patch_db_s
{
    /* patch operation */
    int32   (*fPatch_op)(uint32 unit, rtk_port_t port, uint8 portOffset, rtk_hwpatch_t *pPatch_data, uint8 patch_mode);
    int32   (*fPatch_flow)(uint32 unit, rtk_port_t port, uint8 portOffset, uint8 patch_flow, uint8 patch_mode);

    /* patch data */
    rtk_hwpatch_seq_t seq_table[RTK_PATCH_SEQ_MAX];
    rtk_hwpatch_seq_t cmp_table[RTK_PATCH_SEQ_MAX];

} rt_phy_patch_db_t;
#endif



#endif /* __RTK_PHYLIB_DEF_H */
