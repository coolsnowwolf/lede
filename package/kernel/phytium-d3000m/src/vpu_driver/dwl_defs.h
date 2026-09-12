/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 driver hardware register definition.
 *
 *    Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License, version 2, as
 *    published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License version 2 for more details.
 *
 *    You may obtain a copy of the GNU General Public License
 *    Version 2 at the following locations:
 *    https://opensource.org/licenses/gpl-2.0.php
 */

#ifndef SOFTWARE_LINUX_DWL_DWL_DEFS_H_
#define SOFTWARE_LINUX_DWL_DWL_DEFS_H_

#define DWL_MPEG2_E 31 /* 1 bit  */
#define DWL_VC1_E 29 /* 2 bits */
#define DWL_JPEG_E 28 /* 1 bit  */
#define DWL_JPEG_E_1_1 28 /* 1 bit  */
#define DWL_JPEG_E_1_3 21 /* 1 bit  */
#define DWL_HJPEG_E 17 /* 1 bit  */
#define DWL_HJPEG_E_1_1 17 /* 1 bit  */
#define DWL_HJPEG_E_1_3 22 /* 1 bit  */
#define DWL_MPEG4_E 26 /* 2 bits */
#define DWL_H264_E 24 /* 2 bits */
#define DWL_H264HIGH10_E 20 /* 1 bits */
#define DWL_AVS2_E 18 /* 2 bits */
#define DWL_VP6_E 23 /* 1 bit  */
#define DWL_RV_E 26 /* 2 bits */
#define DWL_VP8_E 23 /* 1 bit  */
#define DWL_VP7_E 24 /* 1 bit  */
#define DWL_WEBP_E 19 /* 1 bit  */
#define DWL_AVS_E 22 /* 1 bit  */
#define DWL_G1_PP_E 16 /* 1 bit  */
#define DWL_G2_PP_E 31 /* 1 bit  */
#define DWL_PP_E 31 /* 1 bit  */
#define DWL_HEVC_E 26 /* 3 bits */
#define DWL_VP9_E 29 /* 3 bits */

#define DWL_H264_PIPELINE_E 31 /* 1 bit */
#define DWL_JPEG_PIPELINE_E 30 /* 1 bit */

#define DWL_G2_HEVC_E 0 /* 1 bits */
#define DWL_G2_VP9_E 1 /* 1 bits */
#define DWL_G2_RFC_E 2 /* 1 bits */
#define DWL_RFC_E 17 /* 2 bits */
#define DWL_G2_DS_E 3 /* 1 bits */
#define DWL_DS_E 28 /* 3 bits */
#define DWL_HEVC_VER 8 /* 4 bits */
#define DWL_VP9_PROFILE 12 /* 3 bits */
#define DWL_RING_E 16 /* 1 bits */

#define FTV310_VPUDEC_IRQ_STAT_DEC 1
#define FTV310_VPUDEC_IRQ_STAT_DEC_OFF (FTV310_VPUDEC_IRQ_STAT_DEC * 4)

#define FTV310_VPUDECPP_SYNTH_CFG 60
#define FTV310_VPUDECPP_SYNTH_CFG_OFF (FTV310_VPUDECPP_SYNTH_CFG * 4)
#define FTV310_VPUDEC_SYNTH_CFG 50
#define FTV310_VPUDEC_SYNTH_CFG_OFF (FTV310_VPUDEC_SYNTH_CFG * 4)
#define FTV310_VPUDEC_SYNTH_CFG_2 54
#define FTV310_VPUDEC_SYNTH_CFG_2_OFF (FTV310_VPUDEC_SYNTH_CFG_2 * 4)
#define FTV310_VPUDEC_SYNTH_CFG_3 56
#define FTV310_VPUDEC_SYNTH_CFG_3_OFF (FTV310_VPUDEC_SYNTH_CFG_3 * 4)
#define FTV310_VPUDEC_CFG_STAT 23
#define FTV310_VPUDEC_CFG_STAT_OFF (FTV310_VPUDEC_CFG_STAT * 4)
#define FTV310_VPUDECPP_CFG_STAT 260
#define FTV310_VPUDECPP_CFG_STAT_OFF (FTV310_VPUDECPP_CFG_STAT * 4)
/* VCD HW build id */
#define FTV310_VPUDEC_HW_BUILD_ID 309
#define FTV310_VPUDEC_HW_BUILD_ID_OFF (FTV310_VPUDEC_HW_BUILD_ID * 4)
#define FTV310_VPUDEC_HW_ID 0
#define FTV310_VPUDEC_HW_ID_OFF (FTV310_VPUDEC_HW_ID * 4)

#define FTV310_VPUDEC_DEC_E 0x01
#define FTV310_VPUDEC_PP_E 0x01
#define FTV310_VPUDEC_DEC_ABORT 0x20
#define FTV310_VPUDEC_DEC_IRQ_DISABLE 0x10
#define FTV310_VPUDEC_DEC_IRQ 0x100

/* Legacy from G1 */
#define FTV310_VPU_IRQ_STAT_DEC 1
#define FTV310_VPU_IRQ_STAT_DEC_OFF (FTV310_VPU_IRQ_STAT_DEC * 4)
#define FTV310_VPU_IRQ_STAT_PP 60
#define FTV310_VPU_IRQ_STAT_PP_OFF (FTV310_VPU_IRQ_STAT_PP * 4)

#define FTV310_VPUPP_SYNTH_CFG 100
#define FTV310_VPUPP_SYNTH_CFG_OFF (FTV310_VPUPP_SYNTH_CFG * 4)
#define FTV310_VPUDEC_SYNTH_CFG 50
#define FTV310_VPUDEC_SYNTH_CFG_OFF (FTV310_VPUDEC_SYNTH_CFG * 4)
#define FTV310_VPUDEC_SYNTH_CFG_2 54
#define FTV310_VPUDEC_SYNTH_CFG_2_OFF (FTV310_VPUDEC_SYNTH_CFG_2 * 4)

#define FTV310_VPU_DEC_E 0x01
#define FTV310_VPU_PP_E 0x01
#define FTV310_VPU_DEC_ABORT 0x20
#define FTV310_VPU_DEC_IRQ_DISABLE 0x10
#define FTV310_VPU_PP_IRQ_DISABLE 0x10
#define FTV310_VPU_DEC_IRQ 0x100
#define FTV310_VPU_PP_IRQ 0x100

#endif /* SOFTWARE_LINUX_DWL_DWL_DEFS_H_ */
