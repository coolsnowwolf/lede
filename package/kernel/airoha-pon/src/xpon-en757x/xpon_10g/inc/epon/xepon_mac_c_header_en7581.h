/***************************************************************
Copyright Statement:

This software/firmware and related documentation (“EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (“EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (“ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN “AS IS
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER’S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER’S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef __EPON_MAC_REGS_H__
#define __EPON_MAC_REGS_H__

#include <linux/mtd/rt_flash.h>
#include "common/drv_types.h"

#ifdef UINT32
#undef UINT32
#endif
#ifdef UINT16
#undef UINT16
#endif
#ifdef UINT8
#undef UINT8
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef REG_BASE_C_MODULE
// ----------------- XEPON_MAC Bit Field Definitions -------------------

#define PACKING
typedef unsigned int FIELD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 1;
        FIELD rpt_txpri_ctrl            : 1;
        FIELD rsv_2                     : 1;
        FIELD epon_oam_cal_in_eth       : 1;
        FIELD mpcp_done_set             : 1;
        FIELD rsv_5                     : 2;
        FIELD chk_all_gnt_mode          : 1;
        FIELD txmbi_stop                : 1;
        FIELD rxmbi_stop                : 1;
        FIELD rx_nml_gate_fwd           : 1;
        FIELD phy_pwr_down              : 1;
        FIELD txmpi_stop                : 1;
        FIELD rxmpi_stop                : 1;
        FIELD rsv_14                    : 2;
        FIELD llid_crc8_err_fwd         : 1;
        FIELD fcs_err_fwd               : 1;
        FIELD allucst_llid_fwd          : 1;
        FIELD mcst_llid_drop            : 1;
        FIELD bcst_llid_m0_drop         : 1;
        FIELD bcst_llid_m1_drop         : 1;
        FIELD mpcp_fwd                  : 1;
        FIELD discv_burst_en            : 1;
        FIELD burst_en_dly              : 1;
        FIELD txoam_favor               : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_e_glb_cfg, *PREG_e_glb_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dscvgate_infochk_dis      : 1;
        FIELD rsv_1                     : 4;
        FIELD tdrift_loctupd_dis        : 1;
        FIELD gntlen_stat_widscv        : 1;
        FIELD rsv_7                     : 5;
        FIELD dscvgate_filt             : 1;
        FIELD oam_cal_in_bytecnt        : 1;
        FIELD snf_cal_in_bytecnt        : 1;
        FIELD eth_cal_in_bytecnt        : 1;
        FIELD u10g_txmode               : 1;
        FIELD crc_cal_in_bytecnt        : 1;
        FIELD rsv_18                    : 2;
        FIELD rgreq_lsrtime_mask        : 1;
        FIELD loctime_mtd               : 1;
        FIELD txmbi_nack_enden          : 1;
        FIELD rxdv_tmout_en             : 1;
        FIELD rsv_24                    : 7;
        FIELD llidnum_sel               : 1;
    } Bits;
    UINT32 Raw;
} REG_e_glb_cfg2, *PREG_e_glb_cfg2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xepon_glb_cfg_sts         : 26;
        FIELD txmbi_stop_sts            : 1;
        FIELD rxmbi_stop_sts            : 1;
        FIELD txmpi_stop_sts            : 1;
        FIELD rxmpi_stop_sts            : 1;
        FIELD xepon_type                : 2;
    } Bits;
    UINT32 Raw;
} REG_e_glb_sts1, *PREG_e_glb_sts1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xepon_glb_cfg2_sts        : 32;
    } Bits;
    UINT32 Raw;
} REG_e_glb_sts2, *PREG_e_glb_sts2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rcv_dscvry_gate_int       : 1;
        FIELD llid0_rcv_rgst_int        : 1;
        FIELD llid1_rcv_rgst_int        : 1;
        FIELD llid2_rcv_rgst_int        : 1;
        FIELD llid3_rcv_rgst_int        : 1;
        FIELD llid4_rcv_rgst_int        : 1;
        FIELD llid5_rcv_rgst_int        : 1;
        FIELD llid6_rcv_rgst_int        : 1;
        FIELD llid7_rcv_rgst_int        : 1;
        FIELD gnt_buf_ovrrun_int        : 1;
        FIELD ptp_msg_tx_int            : 1;
        FIELD tod_updt_int              : 1;
        FIELD tod_1pps_int              : 1;
        FIELD timedrft_int              : 1;
        FIELD mpcp_timeout_int          : 1;
        FIELD rpt_overintvl_int         : 1;
        FIELD txfifo_udrun_int          : 1;
        FIELD ps_sleep_int              : 1;
        FIELD ps_wakeup_int             : 1;
        FIELD rx_sleep_allow_int        : 1;
        FIELD ps_early_wakeup_int       : 1;
        FIELD hidn_gnt_int              : 1;
        FIELD b2b_gnt_int               : 1;
        FIELD reorder1_gnt_int          : 1;
        FIELD reg_req_done_int          : 1;
        FIELD reg_ack_done_int          : 1;
        FIELD sniff_fifo_ovrun_int      : 1;
        FIELD ds_keychg_int             : 1;
        FIELD ds_keymis_int             : 1;
        FIELD us_keychg_int             : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_e_int_status, *PREG_e_int_status;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rcv_dscvry_gate_en        : 1;
        FIELD llid0_rcv_rgst_en         : 1;
        FIELD llid1_rcv_rgst_en         : 1;
        FIELD llid2_rcv_rgst_en         : 1;
        FIELD llid3_rcv_rgst_en         : 1;
        FIELD llid4_rcv_rgst_en         : 1;
        FIELD llid5_rcv_rgst_en         : 1;
        FIELD llid6_rcv_rgst_en         : 1;
        FIELD llid7_rcv_rgst_en         : 1;
        FIELD gnt_buf_ovrrun_en         : 1;
        FIELD ptp_msg_tx_en             : 1;
        FIELD tod_updt_en               : 1;
        FIELD tod_1pps_en               : 1;
        FIELD timedrft_en               : 1;
        FIELD mpcp_timeout_en           : 1;
        FIELD rpt_overintvl_en          : 1;
        FIELD txfifo_udrun_en           : 1;
        FIELD ps_sleep_en               : 1;
        FIELD ps_wakeup_en              : 1;
        FIELD rx_sleep_allow_en         : 1;
        FIELD ps_early_wakeup_en        : 1;
        FIELD hidn_gnt_en               : 1;
        FIELD b2b_gnt_en                : 1;
        FIELD reorder1_gnt_en           : 1;
        FIELD reg_req_done_en           : 1;
        FIELD reg_ack_done_en           : 1;
        FIELD sniff_fifo_ovrun_en       : 1;
        FIELD ds_keychg_en              : 1;
        FIELD ds_keymis_en              : 1;
        FIELD us_keychg_en              : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_e_int_en, *PREG_e_int_en;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rcv_excgate_int           : 1;
        FIELD rcv_crcerr_int            : 1;
        FIELD rcv_eofdrop_int           : 1;
        FIELD rcvgnt_infofail_int       : 1;
        FIELD schgnt_inv_int            : 1;
        FIELD schfch_timeo_int          : 1;
        FIELD txmpi_pldenneq_int        : 1;
        FIELD rxfifo_ovrun_int          : 1;
        FIELD schfch_nopkt_int          : 1;
        FIELD rcv_len_err_int           : 1;
        FIELD rx_rgst_exc_int           : 1;
        FIELD rcv_ifchk_err_int         : 1;
        FIELD tx_latestart_int          : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_e_int_sts2, *PREG_e_int_sts2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rcv_excgate_en            : 1;
        FIELD rcv_crcerr_en             : 1;
        FIELD rcv_eofdrop_en            : 1;
        FIELD rcvgnt_infofail_en        : 1;
        FIELD schgnt_inv_en             : 1;
        FIELD schfch_timeo_en           : 1;
        FIELD txmpi_pldenneq_en         : 1;
        FIELD rxfifo_ovrun_en           : 1;
        FIELD schfch_nopkt_en           : 1;
        FIELD rcv_len_err_en            : 1;
        FIELD rx_rgst_exc_en            : 1;
        FIELD rcv_ifchk_err_en          : 1;
        FIELD tx_latestart_en           : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_e_int_en2, *PREG_e_int_en2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid8_rcv_rgst_int        : 1;
        FIELD llid9_rcv_rgst_int        : 1;
        FIELD llid10_rcv_rgst_int       : 1;
        FIELD llid11_rcv_rgst_int       : 1;
        FIELD llid12_rcv_rgst_int       : 1;
        FIELD llid13_rcv_rgst_int       : 1;
        FIELD llid14_rcv_rgst_int       : 1;
        FIELD llid15_rcv_rgst_int       : 1;
        FIELD llid16_rcv_rgst_int       : 1;
        FIELD llid17_rcv_rgst_int       : 1;
        FIELD llid18_rcv_rgst_int       : 1;
        FIELD llid19_rcv_rgst_int       : 1;
        FIELD llid20_rcv_rgst_int       : 1;
        FIELD llid21_rcv_rgst_int       : 1;
        FIELD llid22_rcv_rgst_int       : 1;
        FIELD llid23_rcv_rgst_int       : 1;
        FIELD llid24_rcv_rgst_int       : 1;
        FIELD llid25_rcv_rgst_int       : 1;
        FIELD llid26_rcv_rgst_int       : 1;
        FIELD llid27_rcv_rgst_int       : 1;
        FIELD llid28_rcv_rgst_int       : 1;
        FIELD llid29_rcv_rgst_int       : 1;
        FIELD llid30_rcv_rgst_int       : 1;
        FIELD llid31_rcv_rgst_int       : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_e_int_sts3, *PREG_e_int_sts3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid8_rcv_rgst_en         : 1;
        FIELD llid9_rcv_rgst_en         : 1;
        FIELD llid10_rcv_rgst_en        : 1;
        FIELD llid11_rcv_rgst_en        : 1;
        FIELD llid12_rcv_rgst_en        : 1;
        FIELD llid13_rcv_rgst_en        : 1;
        FIELD llid14_rcv_rgst_en        : 1;
        FIELD llid15_rcv_rgst_en        : 1;
        FIELD llid16_rcv_rgst_en        : 1;
        FIELD llid17_rcv_rgst_en        : 1;
        FIELD llid18_rcv_rgst_en        : 1;
        FIELD llid19_rcv_rgst_en        : 1;
        FIELD llid20_rcv_rgst_en        : 1;
        FIELD llid21_rcv_rgst_en        : 1;
        FIELD llid22_rcv_rgst_en        : 1;
        FIELD llid23_rcv_rgst_en        : 1;
        FIELD llid24_rcv_rgst_en        : 1;
        FIELD llid25_rcv_rgst_en        : 1;
        FIELD llid26_rcv_rgst_en        : 1;
        FIELD llid27_rcv_rgst_en        : 1;
        FIELD llid28_rcv_rgst_en        : 1;
        FIELD llid29_rcv_rgst_en        : 1;
        FIELD llid30_rcv_rgst_en        : 1;
        FIELD llid31_rcv_rgst_en        : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_e_int_en3, *PREG_e_int_en3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rcv_dscvgate_infomis_hit  : 1;
        FIELD rcv_dscvgate_filt_hit     : 1;
        FIELD rcv_nrlgate_llidbc_hit    : 1;
        FIELD rcv_demacs_flagexc_hit    : 1;
        FIELD rcv_crc8err_hit           : 1;
        FIELD rcv_crc32err_hit          : 1;
        FIELD rcv_mpcp_crcerr_hit       : 1;
        FIELD rsv_7                     : 1;
        FIELD rcv_len_short_hit         : 1;
        FIELD rcv_len_long_hit          : 1;
        FIELD decrpt_sofeof_col_hit     : 1;
        FIELD rsv_11                    : 1;
        FIELD schgnt_idxinv_hit         : 1;
        FIELD schgnt_stinv_hit          : 1;
        FIELD rsv_14                    : 2;
        FIELD tx_gntph_exceed_hit       : 1;
        FIELD tx_gntph_fifound_hit      : 1;
        FIELD tx_gntst_fifound_hit      : 1;
        FIELD tx_gntst_nosog_hit        : 1;
        FIELD snf_pktcnt_err_hit        : 1;
        FIELD rx_noeof_hit              : 1;
        FIELD rx_nodv_tmo_hit           : 1;
        FIELD rx_slderr_hit             : 1;
        FIELD rx_rgst_nack_hit          : 1;
        FIELD rx_rgst_dergst_hit        : 1;
        FIELD schgnt_tmout_hit          : 1;
        FIELD encfrm_alierr_hit         : 1;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_e_exc_sts, *PREG_e_exc_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid0_rpt_tmo             : 1;
        FIELD llid1_rpt_tmo             : 1;
        FIELD llid2_rpt_tmo             : 1;
        FIELD llid3_rpt_tmo             : 1;
        FIELD llid4_rpt_tmo             : 1;
        FIELD llid5_rpt_tmo             : 1;
        FIELD llid6_rpt_tmo             : 1;
        FIELD llid7_rpt_tmo             : 1;
        FIELD llid8_rpt_tmo             : 1;
        FIELD llid9_rpt_tmo             : 1;
        FIELD llid10_rpt_tmo            : 1;
        FIELD llid11_rpt_tmo            : 1;
        FIELD llid12_rpt_tmo            : 1;
        FIELD llid13_rpt_tmo            : 1;
        FIELD llid14_rpt_tmo            : 1;
        FIELD llid15_rpt_tmo            : 1;
        FIELD llid16_rpt_tmo            : 1;
        FIELD llid17_rpt_tmo            : 1;
        FIELD llid18_rpt_tmo            : 1;
        FIELD llid19_rpt_tmo            : 1;
        FIELD llid20_rpt_tmo            : 1;
        FIELD llid21_rpt_tmo            : 1;
        FIELD llid22_rpt_tmo            : 1;
        FIELD llid23_rpt_tmo            : 1;
        FIELD llid24_rpt_tmo            : 1;
        FIELD llid25_rpt_tmo            : 1;
        FIELD llid26_rpt_tmo            : 1;
        FIELD llid27_rpt_tmo            : 1;
        FIELD llid28_rpt_tmo            : 1;
        FIELD llid29_rpt_tmo            : 1;
        FIELD llid30_rpt_tmo            : 1;
        FIELD llid31_rpt_tmo            : 1;
    } Bits;
    UINT32 Raw;
} REG_e_rpt_timout, *PREG_e_rpt_timout;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid0_mpcp_tmo            : 1;
        FIELD llid1_mpcp_tmo            : 1;
        FIELD llid2_mpcp_tmo            : 1;
        FIELD llid3_mpcp_tmo            : 1;
        FIELD llid4_mpcp_tmo            : 1;
        FIELD llid5_mpcp_tmo            : 1;
        FIELD llid6_mpcp_tmo            : 1;
        FIELD llid7_mpcp_tmo            : 1;
        FIELD llid8_mpcp_tmo            : 1;
        FIELD llid9_mpcp_tmo            : 1;
        FIELD llid10_mpcp_tmo           : 1;
        FIELD llid11_mpcp_tmo           : 1;
        FIELD llid12_mpcp_tmo           : 1;
        FIELD llid13_mpcp_tmo           : 1;
        FIELD llid14_mpcp_tmo           : 1;
        FIELD llid15_mpcp_tmo           : 1;
        FIELD llid16_mpcp_tmo           : 1;
        FIELD llid17_mpcp_tmo           : 1;
        FIELD llid18_mpcp_tmo           : 1;
        FIELD llid19_mpcp_tmo           : 1;
        FIELD llid20_mpcp_tmo           : 1;
        FIELD llid21_mpcp_tmo           : 1;
        FIELD llid22_mpcp_tmo           : 1;
        FIELD llid23_mpcp_tmo           : 1;
        FIELD llid24_mpcp_tmo           : 1;
        FIELD llid25_mpcp_tmo           : 1;
        FIELD llid26_mpcp_tmo           : 1;
        FIELD llid27_mpcp_tmo           : 1;
        FIELD llid28_mpcp_tmo           : 1;
        FIELD llid29_mpcp_tmo           : 1;
        FIELD llid30_mpcp_tmo           : 1;
        FIELD llid31_mpcp_tmo           : 1;
    } Bits;
    UINT32 Raw;
} REG_e_mpcp_timout, *PREG_e_mpcp_timout;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid0_ds_keychg           : 1;
        FIELD llid1_ds_keychg           : 1;
        FIELD llid2_ds_keychg           : 1;
        FIELD llid3_ds_keychg           : 1;
        FIELD llid4_ds_keychg           : 1;
        FIELD llid5_ds_keychg           : 1;
        FIELD llid6_ds_keychg           : 1;
        FIELD llid7_ds_keychg           : 1;
        FIELD llid8_ds_keychg           : 1;
        FIELD llid9_ds_keychg           : 1;
        FIELD llid10_ds_keychg          : 1;
        FIELD llid11_ds_keychg          : 1;
        FIELD llid12_ds_keychg          : 1;
        FIELD llid13_ds_keychg          : 1;
        FIELD llid14_ds_keychg          : 1;
        FIELD llid15_ds_keychg          : 1;
        FIELD llid16_ds_keychg          : 1;
        FIELD llid17_ds_keychg          : 1;
        FIELD llid18_ds_keychg          : 1;
        FIELD llid19_ds_keychg          : 1;
        FIELD llid20_ds_keychg          : 1;
        FIELD llid21_ds_keychg          : 1;
        FIELD llid22_ds_keychg          : 1;
        FIELD llid23_ds_keychg          : 1;
        FIELD llid24_ds_keychg          : 1;
        FIELD llid25_ds_keychg          : 1;
        FIELD llid26_ds_keychg          : 1;
        FIELD llid27_ds_keychg          : 1;
        FIELD llid28_ds_keychg          : 1;
        FIELD llid29_ds_keychg          : 1;
        FIELD llid30_ds_keychg          : 1;
        FIELD llid31_ds_keychg          : 1;
    } Bits;
    UINT32 Raw;
} REG_e_ds_keychg, *PREG_e_ds_keychg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid32_ds_keychg          : 1;
        FIELD llid33_ds_keychg          : 1;
        FIELD llid34_ds_keychg          : 1;
        FIELD llid35_ds_keychg          : 1;
        FIELD llid36_ds_keychg          : 1;
        FIELD llid37_ds_keychg          : 1;
        FIELD llid38_ds_keychg          : 1;
        FIELD llid39_ds_keychg          : 1;
        FIELD llid40_ds_keychg          : 1;
        FIELD llid41_ds_keychg          : 1;
        FIELD llid42_ds_keychg          : 1;
        FIELD llid43_ds_keychg          : 1;
        FIELD llid44_ds_keychg          : 1;
        FIELD llid45_ds_keychg          : 1;
        FIELD llid46_ds_keychg          : 1;
        FIELD llid47_ds_keychg          : 1;
        FIELD llid48_ds_keychg          : 1;
        FIELD llid49_ds_keychg          : 1;
        FIELD llid50_ds_keychg          : 1;
        FIELD llid51_ds_keychg          : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_e_ds_keychg2, *PREG_e_ds_keychg2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid0_ds_keymis           : 1;
        FIELD llid1_ds_keymis           : 1;
        FIELD llid2_ds_keymis           : 1;
        FIELD llid3_ds_keymis           : 1;
        FIELD llid4_ds_keymis           : 1;
        FIELD llid5_ds_keymis           : 1;
        FIELD llid6_ds_keymis           : 1;
        FIELD llid7_ds_keymis           : 1;
        FIELD llid8_ds_keymis           : 1;
        FIELD llid9_ds_keymis           : 1;
        FIELD llid10_ds_keymis          : 1;
        FIELD llid11_ds_keymis          : 1;
        FIELD llid12_ds_keymis          : 1;
        FIELD llid13_ds_keymis          : 1;
        FIELD llid14_ds_keymis          : 1;
        FIELD llid15_ds_keymis          : 1;
        FIELD llid16_ds_keymis          : 1;
        FIELD llid17_ds_keymis          : 1;
        FIELD llid18_ds_keymis          : 1;
        FIELD llid19_ds_keymis          : 1;
        FIELD llid20_ds_keymis          : 1;
        FIELD llid21_ds_keymis          : 1;
        FIELD llid22_ds_keymis          : 1;
        FIELD llid23_ds_keymis          : 1;
        FIELD llid24_ds_keymis          : 1;
        FIELD llid25_ds_keymis          : 1;
        FIELD llid26_ds_keymis          : 1;
        FIELD llid27_ds_keymis          : 1;
        FIELD llid28_ds_keymis          : 1;
        FIELD llid29_ds_keymis          : 1;
        FIELD llid30_ds_keymis          : 1;
        FIELD llid31_ds_keymis          : 1;
    } Bits;
    UINT32 Raw;
} REG_e_ds_keymis, *PREG_e_ds_keymis;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid32_ds_keymis          : 1;
        FIELD llid33_ds_keymis          : 1;
        FIELD llid34_ds_keymis          : 1;
        FIELD llid35_ds_keymis          : 1;
        FIELD llid36_ds_keymis          : 1;
        FIELD llid37_ds_keymis          : 1;
        FIELD llid38_ds_keymis          : 1;
        FIELD llid39_ds_keymis          : 1;
        FIELD llid40_ds_keymis          : 1;
        FIELD llid41_ds_keymis          : 1;
        FIELD llid42_ds_keymis          : 1;
        FIELD llid43_ds_keymis          : 1;
        FIELD llid44_ds_keymis          : 1;
        FIELD llid45_ds_keymis          : 1;
        FIELD llid46_ds_keymis          : 1;
        FIELD llid47_ds_keymis          : 1;
        FIELD llid48_ds_keymis          : 1;
        FIELD llid49_ds_keymis          : 1;
        FIELD llid50_ds_keymis          : 1;
        FIELD llid51_ds_keymis          : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_e_ds_keymis2, *PREG_e_ds_keymis2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid0_us_keychg           : 1;
        FIELD llid1_us_keychg           : 1;
        FIELD llid2_us_keychg           : 1;
        FIELD llid3_us_keychg           : 1;
        FIELD llid4_us_keychg           : 1;
        FIELD llid5_us_keychg           : 1;
        FIELD llid6_us_keychg           : 1;
        FIELD llid7_us_keychg           : 1;
        FIELD llid8_us_keychg           : 1;
        FIELD llid9_us_keychg           : 1;
        FIELD llid10_us_keychg          : 1;
        FIELD llid11_us_keychg          : 1;
        FIELD llid12_us_keychg          : 1;
        FIELD llid13_us_keychg          : 1;
        FIELD llid14_us_keychg          : 1;
        FIELD llid15_us_keychg          : 1;
        FIELD llid16_us_keychg          : 1;
        FIELD llid17_us_keychg          : 1;
        FIELD llid18_us_keychg          : 1;
        FIELD llid19_us_keychg          : 1;
        FIELD llid20_us_keychg          : 1;
        FIELD llid21_us_keychg          : 1;
        FIELD llid22_us_keychg          : 1;
        FIELD llid23_us_keychg          : 1;
        FIELD llid24_us_keychg          : 1;
        FIELD llid25_us_keychg          : 1;
        FIELD llid26_us_keychg          : 1;
        FIELD llid27_us_keychg          : 1;
        FIELD llid28_us_keychg          : 1;
        FIELD llid29_us_keychg          : 1;
        FIELD llid30_us_keychg          : 1;
        FIELD llid31_us_keychg          : 1;
    } Bits;
    UINT32 Raw;
} REG_e_us_keychg, *PREG_e_us_keychg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy0               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy0, *PREG_e_cfg_dmy0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy1               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy1, *PREG_e_cfg_dmy1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid0_oam_lpbk_en         : 1;
        FIELD llid0_dcrypt_mode         : 1;
        FIELD llid0_dcrypt_en           : 1;
        FIELD llid0_txfec_en            : 1;
        FIELD llid0_encrypt_en          : 1;
        FIELD llid0_encrypt_key         : 1;
        FIELD llid0_dmy                 : 2;
        FIELD llid1_oam_lpbk_en         : 1;
        FIELD llid1_dcrypt_mode         : 1;
        FIELD llid1_dcrypt_en           : 1;
        FIELD llid1_txfec_en            : 1;
        FIELD llid1_encrypt_en          : 1;
        FIELD llid1_encrypt_key         : 1;
        FIELD llid1_dmy                 : 2;
        FIELD llid2_oam_lpbk_en         : 1;
        FIELD llid2_dcrypt_mode         : 1;
        FIELD llid2_dcrypt_en           : 1;
        FIELD llid2_txfec_en            : 1;
        FIELD llid2_encrypt_en          : 1;
        FIELD llid2_encrypt_key         : 1;
        FIELD llid2_dmy                 : 2;
        FIELD llid3_oam_lpbk_en         : 1;
        FIELD llid3_dcrypt_mode         : 1;
        FIELD llid3_dcrypt_en           : 1;
        FIELD llid3_txfec_en            : 1;
        FIELD llid3_encrypt_en          : 1;
        FIELD llid3_encrypt_key         : 1;
        FIELD llid3_dmy                 : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid0_3_cfg, *PREG_e_llid0_3_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid4_oam_lpbk_en         : 1;
        FIELD llid4_dcrypt_mode         : 1;
        FIELD llid4_dcrypt_en           : 1;
        FIELD llid4_txfec_en            : 1;
        FIELD llid4_encrypt_en          : 1;
        FIELD llid4_encrypt_key         : 1;
        FIELD llid4_dmy                 : 2;
        FIELD llid5_oam_lpbk_en         : 1;
        FIELD llid5_dcrypt_mode         : 1;
        FIELD llid5_dcrypt_en           : 1;
        FIELD llid5_txfec_en            : 1;
        FIELD llid5_encrypt_en          : 1;
        FIELD llid5_encrypt_key         : 1;
        FIELD llid5_dmy                 : 2;
        FIELD llid6_oam_lpbk_en         : 1;
        FIELD llid6_dcrypt_mode         : 1;
        FIELD llid6_dcrypt_en           : 1;
        FIELD llid6_txfec_en            : 1;
        FIELD llid6_encrypt_en          : 1;
        FIELD llid6_encrypt_key         : 1;
        FIELD llid6_dmy                 : 2;
        FIELD llid7_oam_lpbk_en         : 1;
        FIELD llid7_dcrypt_mode         : 1;
        FIELD llid7_dcrypt_en           : 1;
        FIELD llid7_txfec_en            : 1;
        FIELD llid7_encrypt_en          : 1;
        FIELD llid7_encrypt_key         : 1;
        FIELD llid7_dmy                 : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid4_7_cfg, *PREG_e_llid4_7_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid8_oamlpbk_en          : 1;
        FIELD llid8_dcrypt_mode         : 1;
        FIELD llid8_dcrypt_en           : 1;
        FIELD llid8_txfec_en            : 1;
        FIELD llid8_encrypt_en          : 1;
        FIELD llid8_encrypt_key         : 1;
        FIELD llid8_dmy                 : 2;
        FIELD llid9_oamlpbk_en          : 1;
        FIELD llid9_dcrypt_mode         : 1;
        FIELD llid9_dcrypt_en           : 1;
        FIELD llid9_txfec_en            : 1;
        FIELD llid9_encrypt_en          : 1;
        FIELD llid9_encrypt_key         : 1;
        FIELD llid9_dmy                 : 2;
        FIELD llid10_oamlpbk_en         : 1;
        FIELD llid10_dcrypt_mode        : 1;
        FIELD llid10_dcrypt_en          : 1;
        FIELD llid10_txfec_en           : 1;
        FIELD llid10_encrypt_en         : 1;
        FIELD llid10_encrypt_key        : 1;
        FIELD llid10_dmy                : 2;
        FIELD llid11_oamlpbk_en         : 1;
        FIELD llid11_dcrypt_mode        : 1;
        FIELD llid11_dcrypt_en          : 1;
        FIELD llid11_txfec_en           : 1;
        FIELD llid11_encrypt_en         : 1;
        FIELD llid11_encrypt_key        : 1;
        FIELD llid11_dmy                : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid8_11_cfg, *PREG_e_llid8_11_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid12_oam_lpbk_en        : 1;
        FIELD llid12_dcrypt_mode        : 1;
        FIELD llid12_dcrypt_en          : 1;
        FIELD llid12_txfec_en           : 1;
        FIELD llid12_encrypt_en         : 1;
        FIELD llid12_encrypt_key        : 1;
        FIELD llid12_dmy                : 2;
        FIELD llid13_oamlpbk_en         : 1;
        FIELD llid13_dcrypt_mode        : 1;
        FIELD llid13_dcrypt_en          : 1;
        FIELD llid13_txfec_en           : 1;
        FIELD llid13_encrypt_en         : 1;
        FIELD llid13_encrypt_key        : 1;
        FIELD llid13_dmy                : 2;
        FIELD llid14_oamlpbk_en         : 1;
        FIELD llid14_dcrypt_mode        : 1;
        FIELD llid14_dcrypt_en          : 1;
        FIELD llid14_txfec_en           : 1;
        FIELD llid14_encrypt_en         : 1;
        FIELD llid14_encrypt_key        : 1;
        FIELD llid14_dmy                : 2;
        FIELD llid15_oamlpbk_en         : 1;
        FIELD llid15_dcrypt_mode        : 1;
        FIELD llid15_dcrypt_en          : 1;
        FIELD llid15_txfec_en           : 1;
        FIELD llid15_encrypt_en         : 1;
        FIELD llid15_encrypt_key        : 1;
        FIELD llid15_dmy                : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid12_15_cfg, *PREG_e_llid12_15_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid16_oam_lpbk_en        : 1;
        FIELD llid16_dcrypt_mode        : 1;
        FIELD llid16_dcrypt_en          : 1;
        FIELD llid16_txfec_en           : 1;
        FIELD llid16_encrypt_en         : 1;
        FIELD llid16_encrypt_key        : 1;
        FIELD llid16_dmy                : 2;
        FIELD llid17_oamlpbk_en         : 1;
        FIELD llid17_dcrypt_mode        : 1;
        FIELD llid17_dcrypt_en          : 1;
        FIELD llid17_txfec_en           : 1;
        FIELD llid17_encrypt_en         : 1;
        FIELD llid17_encrypt_key        : 1;
        FIELD llid17_dmy                : 2;
        FIELD llid18_oamlpbk_en         : 1;
        FIELD llid18_dcrypt_mode        : 1;
        FIELD llid18_dcrypt_en          : 1;
        FIELD llid18_txfec_en           : 1;
        FIELD llid18_encrypt_en         : 1;
        FIELD llid18_encrypt_key        : 1;
        FIELD llid18_dmy                : 2;
        FIELD llid19_oamlpbk_en         : 1;
        FIELD llid19_dcrypt_mode        : 1;
        FIELD llid19_dcrypt_en          : 1;
        FIELD llid19_txfec_en           : 1;
        FIELD llid19_encrypt_en         : 1;
        FIELD llid19_encrypt_key        : 1;
        FIELD llid19_dmy                : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid16_19_cfg, *PREG_e_llid16_19_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid20_oam_lpbk_en        : 1;
        FIELD llid20_dcrypt_mode        : 1;
        FIELD llid20_dcrypt_en          : 1;
        FIELD llid20_txfec_en           : 1;
        FIELD llid20_encrypt_en         : 1;
        FIELD llid20_encrypt_key        : 1;
        FIELD llid20_dmy                : 2;
        FIELD llid21_oamlpbk_en         : 1;
        FIELD llid21_dcrypt_mode        : 1;
        FIELD llid21_dcrypt_en          : 1;
        FIELD llid21_txfec_en           : 1;
        FIELD llid21_encrypt_en         : 1;
        FIELD llid21_encrypt_key        : 1;
        FIELD llid21_dmy                : 2;
        FIELD llid22_oamlpbk_en         : 1;
        FIELD llid22_dcrypt_mode        : 1;
        FIELD llid22_dcrypt_en          : 1;
        FIELD llid22_txfec_en           : 1;
        FIELD llid22_encrypt_en         : 1;
        FIELD llid22_encrypt_key        : 1;
        FIELD llid22_dmy                : 2;
        FIELD llid23_oamlpbk_en         : 1;
        FIELD llid23_dcrypt_mode        : 1;
        FIELD llid23_dcrypt_en          : 1;
        FIELD llid23_txfec_en           : 1;
        FIELD llid23_encrypt_en         : 1;
        FIELD llid23_encrypt_key        : 1;
        FIELD llid23_dmy                : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid20_23_cfg, *PREG_e_llid20_23_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid24_oam_lpbk_en        : 1;
        FIELD llid24_dcrypt_mode        : 1;
        FIELD llid24_dcrypt_en          : 1;
        FIELD llid24_txfec_en           : 1;
        FIELD llid24_encrypt_en         : 1;
        FIELD llid24_encrypt_key        : 1;
        FIELD llid24_dmy                : 2;
        FIELD llid25_oamlpbk_en         : 1;
        FIELD llid25_dcrypt_mode        : 1;
        FIELD llid25_dcrypt_en          : 1;
        FIELD llid25_txfec_en           : 1;
        FIELD llid25_encrypt_en         : 1;
        FIELD llid25_encrypt_key        : 1;
        FIELD llid25_dmy                : 2;
        FIELD llid26_oamlpbk_en         : 1;
        FIELD llid26_dcrypt_mode        : 1;
        FIELD llid26_dcrypt_en          : 1;
        FIELD llid26_txfec_en           : 1;
        FIELD llid26_encrypt_en         : 1;
        FIELD llid26_encrypt_key        : 1;
        FIELD llid26_dmy                : 2;
        FIELD llid27_oamlpbk_en         : 1;
        FIELD llid27_dcrypt_mode        : 1;
        FIELD llid27_dcrypt_en          : 1;
        FIELD llid27_txfec_en           : 1;
        FIELD llid27_encrypt_en         : 1;
        FIELD llid27_encrypt_key        : 1;
        FIELD llid27_dmy                : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid24_27_cfg, *PREG_e_llid24_27_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid28_oam_lpbk_en        : 1;
        FIELD llid28_dcrypt_mode        : 1;
        FIELD llid28_dcrypt_en          : 1;
        FIELD llid28_txfec_en           : 1;
        FIELD llid28_encrypt_en         : 1;
        FIELD llid28_encrypt_key        : 1;
        FIELD llid28_dmy                : 2;
        FIELD llid29_oamlpbk_en         : 1;
        FIELD llid29_dcrypt_mode        : 1;
        FIELD llid29_dcrypt_en          : 1;
        FIELD llid29_txfec_en           : 1;
        FIELD llid29_encrypt_en         : 1;
        FIELD llid29_encrypt_key        : 1;
        FIELD llid29_dmy                : 2;
        FIELD llid30_oamlpbk_en         : 1;
        FIELD llid30_dcrypt_mode        : 1;
        FIELD llid30_dcrypt_en          : 1;
        FIELD llid30_txfec_en           : 1;
        FIELD llid30_encrypt_en         : 1;
        FIELD llid30_encrypt_key        : 1;
        FIELD llid30_dmy                : 2;
        FIELD llid31_oamlpbk_en         : 1;
        FIELD llid31_dcrypt_mode        : 1;
        FIELD llid31_dcrypt_en          : 1;
        FIELD llid31_txfec_en           : 1;
        FIELD llid31_encrypt_en         : 1;
        FIELD llid31_encrypt_key        : 1;
        FIELD llid31_dmy                : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid28_31_cfg, *PREG_e_llid28_31_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy2               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy2, *PREG_e_cfg_dmy2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy3               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy3, *PREG_e_cfg_dmy3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy4               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy4, *PREG_e_cfg_dmy4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_mpcp_llid_idx          : 5;
        FIELD rsv_5                     : 3;
        FIELD rgstr_req_flg             : 1;
        FIELD rsv_9                     : 3;
        FIELD rgstr_ack_flg             : 1;
        FIELD rsv_13                    : 3;
        FIELD mpcp_cmd_done             : 1;
        FIELD rsv_17                    : 13;
        FIELD mpcp_cmd                  : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid_dscvry_ctrl, *PREG_e_llid_dscvry_ctrl;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid0_value               : 16;
        FIELD llid0_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid0_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid0_dscvry_sts          : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid0_dscvry_sts, *PREG_e_llid0_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid1_value               : 16;
        FIELD llid1_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid1_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid1_dscvry_sts          : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid1_dscvry_sts, *PREG_e_llid1_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid2_value               : 16;
        FIELD llid2_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid2_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid2_dscvry_sts          : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid2_dscvry_sts, *PREG_e_llid2_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid3_value               : 16;
        FIELD llid3_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid3_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid3_dscvry_sts          : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid3_dscvry_sts, *PREG_e_llid3_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid4_value               : 16;
        FIELD llid4_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid4_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid4_dscvry_sts          : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid4_dscvry_sts, *PREG_e_llid4_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid5_value               : 16;
        FIELD llid5_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid5_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid5_dscvry_sts          : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid5_dscvry_sts, *PREG_e_llid5_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid6_value               : 16;
        FIELD llid6_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid6_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid6_dscvry_sts          : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid6_dscvry_sts, *PREG_e_llid6_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid7_value               : 16;
        FIELD llid7_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid7_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid7_dscvry_sts          : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid7_dscvry_sts, *PREG_e_llid7_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid8_value               : 16;
        FIELD llid8_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid8_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid8_dscvry_sts          : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid8_dscvry_sts, *PREG_e_llid8_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid9_value               : 16;
        FIELD llid9_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid9_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid9_dscvry_sts          : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid9_dscvry_sts, *PREG_e_llid9_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid10_value              : 16;
        FIELD llid10_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid10_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid10_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid10_dscvry_sts, *PREG_e_llid10_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid11_value              : 16;
        FIELD llid11_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid11_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid11_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid11_dscvry_sts, *PREG_e_llid11_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid12_value              : 16;
        FIELD llid12_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid12_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid12_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid12_dscvry_sts, *PREG_e_llid12_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid13_value              : 16;
        FIELD llid13_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid13_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid13_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid13_dscvry_sts, *PREG_e_llid13_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid14_value              : 16;
        FIELD llid14_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid14_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid14_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid14_dscvry_sts, *PREG_e_llid14_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid15_value              : 16;
        FIELD llid15_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid15_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid15_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid15_dscvry_sts, *PREG_e_llid15_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid16_value              : 16;
        FIELD llid16_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid16_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid16_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid16_dscvry_sts, *PREG_e_llid16_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid17_value              : 16;
        FIELD llid17_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid17_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid17_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid17_dscvry_sts, *PREG_e_llid17_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid18_value              : 16;
        FIELD llid18_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid18_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid18_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid18_dscvry_sts, *PREG_e_llid18_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid19_value              : 16;
        FIELD llid19_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid19_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid19_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid19_dscvry_sts, *PREG_e_llid19_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid20_value              : 16;
        FIELD llid20_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid20_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid20_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid20_dscvry_sts, *PREG_e_llid20_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid21_value              : 16;
        FIELD llid21_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid21_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid21_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid21_dscvry_sts, *PREG_e_llid21_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid22_value              : 16;
        FIELD llid22_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid22_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid22_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid22_dscvry_sts, *PREG_e_llid22_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid23_value              : 16;
        FIELD llid23_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid23_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid23_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid23_dscvry_sts, *PREG_e_llid23_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid24_value              : 16;
        FIELD llid24_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid24_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid24_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid24_dscvry_sts, *PREG_e_llid24_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid25_value              : 16;
        FIELD llid25_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid25_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid25_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid25_dscvry_sts, *PREG_e_llid25_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid26_value              : 16;
        FIELD llid26_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid26_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid26_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid26_dscvry_sts, *PREG_e_llid26_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid27_value              : 16;
        FIELD llid27_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid27_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid27_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid27_dscvry_sts, *PREG_e_llid27_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid28_value              : 16;
        FIELD llid28_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid28_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid28_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid28_dscvry_sts, *PREG_e_llid28_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid29_value              : 16;
        FIELD llid29_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid29_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid29_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid29_dscvry_sts, *PREG_e_llid29_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid30_value              : 16;
        FIELD llid30_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid30_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid30_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid30_dscvry_sts, *PREG_e_llid30_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid31_value              : 16;
        FIELD llid31_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid31_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid31_dscvry_sts         : 2;
    } Bits;
    UINT32 Raw;
} REG_e_llid31_dscvry_sts, *PREG_e_llid31_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pending_gnt_num           : 8;
        FIELD echoed_pending_gnt        : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_pending_gnt_num, *PREG_e_pending_gnt_num;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mac_addr_dw_idx           : 1;
        FIELD mac_addr_llid_indx        : 5;
        FIELD rsv_6                     : 10;
        FIELD mac_addr_rwcmd_done       : 1;
        FIELD rsv_17                    : 14;
        FIELD mac_addr_rwcmd            : 1;
    } Bits;
    UINT32 Raw;
} REG_e_mac_addr_cfg, *PREG_e_mac_addr_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mac_addr_value            : 32;
    } Bits;
    UINT32 Raw;
} REG_e_mac_addr_value, *PREG_e_mac_addr_value;

typedef PACKING union
{
    PACKING struct
    {
        FIELD key_dw_indx               : 2;
        FIELD rsv_2                     : 2;
        FIELD key_idx                   : 1;
        FIELD rsv_5                     : 3;
        FIELD key_llid_index            : 6;
        FIELD rsv_14                    : 2;
        FIELD key_rwcmd_done            : 1;
        FIELD rsv_17                    : 3;
        FIELD key_macsec_an             : 2;
        FIELD rsv_22                    : 9;
        FIELD key_rwcmd                 : 1;
    } Bits;
    UINT32 Raw;
} REG_e_security_key_cfg, *PREG_e_security_key_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD key_value                 : 32;
    } Bits;
    UINT32 Raw;
} REG_e_key_value, *PREG_e_key_value;

typedef PACKING union
{
    PACKING struct
    {
        FIELD enckey_dwidx              : 2;
        FIELD rsv_2                     : 2;
        FIELD enckey_keyidx             : 1;
        FIELD rsv_5                     : 3;
        FIELD enckey_llididx            : 6;
        FIELD rsv_14                    : 2;
        FIELD enckey_rwcmd_done         : 1;
        FIELD rsv_17                    : 3;
        FIELD enckey_macsec_an          : 2;
        FIELD rsv_22                    : 9;
        FIELD enckey_rwcmd              : 1;
    } Bits;
    UINT32 Raw;
} REG_e_enckey_cfg, *PREG_e_enckey_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD enckey_value              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_enckey_val, *PREG_e_enckey_val;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy5               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy5, *PREG_e_cfg_dmy5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid0_rpt_cfg             : 2;
        FIELD llid1_rpt_cfg             : 2;
        FIELD llid2_rpt_cfg             : 2;
        FIELD llid3_rpt_cfg             : 2;
        FIELD llid4_rpt_cfg             : 2;
        FIELD llid5_rpt_cfg             : 2;
        FIELD llid6_rpt_cfg             : 2;
        FIELD llid7_rpt_cfg             : 2;
        FIELD llid8_rpt_cfg             : 2;
        FIELD llid9_rpt_cfg             : 2;
        FIELD llid10_rpt_cfg            : 2;
        FIELD llid11_rpt_cfg            : 2;
        FIELD llid12_rpt_cfg            : 2;
        FIELD llid13_rpt_cfg            : 2;
        FIELD llid14_rpt_cfg            : 2;
        FIELD llid15_rpt_cfg            : 2;
    } Bits;
    UINT32 Raw;
} REG_e_rpt_cfg, *PREG_e_rpt_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD llid16_rpt_cfg            : 2;
        FIELD llid17_rpt_cfg            : 2;
        FIELD llid18_rpt_cfg            : 2;
        FIELD llid19_rpt_cfg            : 2;
        FIELD llid20_rpt_cfg            : 2;
        FIELD llid21_rpt_cfg            : 2;
        FIELD llid22_rpt_cfg            : 2;
        FIELD llid23_rpt_cfg            : 2;
        FIELD llid24_rpt_cfg            : 2;
        FIELD llid25_rpt_cfg            : 2;
        FIELD llid26_rpt_cfg            : 2;
        FIELD llid27_rpt_cfg            : 2;
        FIELD llid28_rpt_cfg            : 2;
        FIELD llid29_rpt_cfg            : 2;
        FIELD llid30_rpt_cfg            : 2;
        FIELD llid31_rpt_cfg            : 2;
    } Bits;
    UINT32 Raw;
} REG_e_rpt_cfg2, *PREG_e_rpt_cfg2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD queue_idx                 : 3;
        FIELD rsv_3                     : 3;
        FIELD qthld_idx                 : 2;
        FIELD qthld_value               : 16;
        FIELD rpt_llid_idx              : 5;
        FIELD rsv_29                    : 1;
        FIELD qthld_rwcmd_done          : 1;
        FIELD qthld_rwcmd               : 1;
    } Bits;
    UINT32 Raw;
} REG_e_rpt_qthld_cfg, *PREG_e_rpt_qthld_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_default_rpt            : 1;
        FIELD rpt_qsize_mgnt_sel        : 1;
        FIELD rpt_qsize_dec             : 2;
        FIELD rpt_dpoe_en               : 1;
        FIELD rpt_dpoe_queidx           : 3;
        FIELD rpt_dpoe_thrstop          : 1;
        FIELD rpt_fullqsize_head        : 1;
        FIELD rpt_qsize_sp              : 1;
        FIELD gntfrpt_ignore            : 1;
        FIELD ctrlpkt_frpt              : 1;
        FIELD txgnt_cnthit_frpt         : 1;
        FIELD dba_gtthr_frpt            : 1;
        FIELD rpt_dmy1                  : 17;
    } Bits;
    UINT32 Raw;
} REG_e_rpt_glb_cfg, *PREG_e_rpt_glb_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rpt_bitmap_set            : 8;
        FIELD rpt_bitmap_ctrl           : 8;
        FIELD rpt_dmy2                  : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rpt_bmap, *PREG_e_rpt_bmap;

typedef PACKING union
{
    PACKING struct
    {
        FIELD frpt_dba_totsize_thr      : 16;
        FIELD frpt_txgnt_cnt            : 16;
    } Bits;
    UINT32 Raw;
} REG_e_frpt_thr, *PREG_e_frpt_thr;

typedef PACKING union
{
    PACKING struct
    {
        FIELD u1g_fecoff_rpt_qsizeadj   : 16;
        FIELD u1g_fecon_rpt_qsizeadj    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_u1g_rpt_qsizeadj, *PREG_e_u1g_rpt_qsizeadj;

typedef PACKING union
{
    PACKING struct
    {
        FIELD u10g_rpt_qsizeadj         : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_u10g_rpt_qsizeadj, *PREG_e_u10g_rpt_qsizeadj;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy7               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy7, *PREG_e_cfg_dmy7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy8               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy8, *PREG_e_cfg_dmy8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy9               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy9, *PREG_e_cfg_dmy9;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy10              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy10, *PREG_e_cfg_dmy10;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy11              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy11, *PREG_e_cfg_dmy11;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy12              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy12, *PREG_e_cfg_dmy12;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy13              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy13, *PREG_e_cfg_dmy13;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy14              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy14, *PREG_e_cfg_dmy14;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_cfg_dmy15              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_cfg_dmy15, *PREG_e_cfg_dmy15;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dfrpt_data5               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dfrpt_data5, *PREG_e_dfrpt_data5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dfrpt_data6               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dfrpt_data6, *PREG_e_dfrpt_data6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dfrpt_data7               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dfrpt_data7, *PREG_e_dfrpt_data7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dfrpt_data8               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dfrpt_data8, *PREG_e_dfrpt_data8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dfrpt_data9               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dfrpt_data9, *PREG_e_dfrpt_data9;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dfrpt_data10              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dfrpt_data10, *PREG_e_dfrpt_data10;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dfrpt_data11              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dfrpt_data11, *PREG_e_dfrpt_data11;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dfrpt_data12              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dfrpt_data12, *PREG_e_dfrpt_data12;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dfrpt_data13              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dfrpt_data13, *PREG_e_dfrpt_data13;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dfrpt_data14              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dfrpt_data14, *PREG_e_dfrpt_data14;

typedef PACKING union
{
    PACKING struct
    {
        FIELD laser_on_time             : 8;
        FIELD laser_off_time            : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_laser_onoff_time, *PREG_e_laser_onoff_time;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sync_time                 : 16;
        FIELD sync_time_updte           : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} REG_e_sync_time, *PREG_e_sync_time;

typedef PACKING union
{
    PACKING struct
    {
        FIELD lsron_time_maxval         : 8;
        FIELD lsroff_time_maxval        : 8;
        FIELD sync_time_maxval          : 8;
        FIELD rsv_24                    : 5;
        FIELD lsron_time_maxen          : 1;
        FIELD lsroff_time_maxen         : 1;
        FIELD sync_time_maxen           : 1;
    } Bits;
    UINT32 Raw;
} REG_e_overhead_time_thr, *PREG_e_overhead_time_thr;

typedef PACKING union
{
    PACKING struct
    {
        FIELD olt_sync_time             : 16;
        FIELD olt_lsron_time            : 8;
        FIELD olt_lsroff_time           : 8;
    } Bits;
    UINT32 Raw;
} REG_e_laser_onoff_time2, *PREG_e_laser_onoff_time2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD olt_dscvinfo_mis          : 4;
        FIELD olt_dscvinfo_match        : 4;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_e_olt_dscvinfo, *PREG_e_olt_dscvinfo;

typedef PACKING union
{
    PACKING struct
    {
        FIELD guard_thrshld             : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_e_grd_thrshld, *PREG_e_grd_thrshld;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mpcp_timeout_intvl        : 32;
    } Bits;
    UINT32 Raw;
} REG_e_mpcp_timeout_intvl, *PREG_e_mpcp_timeout_intvl;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rpt_timeout_intvl         : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_e_rpt_timeout_intvl, *PREG_e_rpt_timeout_intvl;

typedef PACKING union
{
    PACKING struct
    {
        FIELD max_future_gnt_time       : 32;
    } Bits;
    UINT32 Raw;
} REG_e_max_future_gnt_time, *PREG_e_max_future_gnt_time;

typedef PACKING union
{
    PACKING struct
    {
        FIELD min_proc_time             : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_min_proc_time, *PREG_e_min_proc_time;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_stm_adj                : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_trx_adjust_time1, *PREG_e_trx_adjust_time1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_tmstp_adj              : 16;
        FIELD tx_len_adj                : 16;
    } Bits;
    UINT32 Raw;
} REG_e_trx_adjust_time2, *PREG_e_trx_adjust_time2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD u10g_tx_stmadj            : 16;
        FIELD u10g_tx_tsadj             : 16;
    } Bits;
    UINT32 Raw;
} REG_e_trx_adjtime3, *PREG_e_trx_adjtime3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 8;
        FIELD d10g_rx_tsadj_ofst        : 5;
        FIELD rsv_13                    : 3;
        FIELD d10g_rx_tsadj             : 16;
    } Bits;
    UINT32 Raw;
} REG_e_trx_adjtime4, *PREG_e_trx_adjtime4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 16;
        FIELD u1g_tx_tsadj              : 16;
    } Bits;
    UINT32 Raw;
} REG_e_trx_adjtime5, *PREG_e_trx_adjtime5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD d10g_rx_phydly            : 11;
        FIELD d10g_rx_phydly_ofst       : 5;
        FIELD d1g_rx_phydly             : 11;
        FIELD d1g_rx_phydly_ofst        : 1;
        FIELD rsv_28                    : 3;
        FIELD rx_phydly_dft             : 1;
    } Bits;
    UINT32 Raw;
} REG_e_rxphydly_adjtime, *PREG_e_rxphydly_adjtime;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_dma_leadtime           : 16;
        FIELD tx_fetch_leadtime         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_e_txfetch_cfg, *PREG_e_txfetch_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD default_ovrhd             : 6;
        FIELD rsv_6                     : 2;
        FIELD tail_grd                  : 8;
        FIELD fec_tail_grd              : 8;
        FIELD dscvr_gnt_len             : 8;
    } Bits;
    UINT32 Raw;
} REG_e_tx_cal_cnst, *PREG_e_tx_cal_cnst;

typedef PACKING union
{
    PACKING struct
    {
        FIELD u10g_tail_grd             : 8;
        FIELD u10g_dscv_gntlen          : 8;
        FIELD rsv_16                    : 8;
        FIELD rpt_feccal_cnt_multival   : 6;
        FIELD rsv_30                    : 1;
        FIELD ipgalign_mtd              : 1;
    } Bits;
    UINT32 Raw;
} REG_e_txcal_cnst2, *PREG_e_txcal_cnst2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD u1g_fecoff_min_gntlen     : 8;
        FIELD u10g_min_gntlen           : 8;
        FIELD u10g_eoblen               : 8;
        FIELD u1g_fecon_min_gntlen      : 8;
    } Bits;
    UINT32 Raw;
} REG_e_txcal_cnst3, *PREG_e_txcal_cnst3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txfifo_pad_lthr           : 10;
        FIELD txsch_dmy1                : 2;
        FIELD txfifo_pad_hthr           : 10;
        FIELD txsch_dmy0                : 2;
        FIELD txfifo_wr_thr             : 7;
        FIELD txfifo_wr_thren           : 1;
    } Bits;
    UINT32 Raw;
} REG_e_txsch_cfg, *PREG_e_txsch_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxfifo_mbithr             : 5;
        FIELD rx_dmy0                   : 11;
        FIELD rx_dmy1                   : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxfifo_thr, *PREG_e_rxfifo_thr;

typedef PACKING union
{
    PACKING struct
    {
        FIELD d1g_bcllid                : 16;
        FIELD d10g_bcllid               : 16;
    } Bits;
    UINT32 Raw;
} REG_e_bcllid_cfg, *PREG_e_bcllid_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_mpcp_addrl             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_txfrm_cfg1, *PREG_e_txfrm_cfg1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_mpcp_etype             : 16;
        FIELD tx_mpcp_addrh             : 16;
    } Bits;
    UINT32 Raw;
} REG_e_txfrm_cfg2, *PREG_e_txfrm_cfg2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_rgack_op               : 16;
        FIELD tx_rgreq_op               : 16;
    } Bits;
    UINT32 Raw;
} REG_e_txfrm_cfg3, *PREG_e_txfrm_cfg3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tod_sync_x                : 32;
    } Bits;
    UINT32 Raw;
} REG_e_tod_sync_x, *PREG_e_tod_sync_x;

typedef PACKING union
{
    PACKING struct
    {
        FIELD egrs_latency              : 8;
        FIELD ingrs_latency             : 11;
        FIELD rl_rx_phydly_ofst         : 5;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_e_tod_ltncy, *PREG_e_tod_ltncy;

typedef PACKING union
{
    PACKING struct
    {
        FIELD new_tod_p2p_offset_sec_l32 : 32;
    } Bits;
    UINT32 Raw;
} REG_e_new_tod_p2p_offset_sec_l32, *PREG_e_new_tod_p2p_offset_sec_l32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD new_tod_nsec              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_new_tod_p2p_tod_offset_nsec, *PREG_e_new_tod_p2p_tod_offset_nsec;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tod_p2p_sec_l32           : 32;
    } Bits;
    UINT32 Raw;
} REG_e_tod_p2p_tod_sec_l32, *PREG_e_tod_p2p_tod_sec_l32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tod_p2p_nsec              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_tod_p2p_tod_nsec, *PREG_e_tod_p2p_tod_nsec;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tod_period                : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_e_tod_period, *PREG_e_tod_period;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tod_1pps_width_ctrl       : 32;
    } Bits;
    UINT32 Raw;
} REG_e_tod_1pps_ctrl, *PREG_e_tod_1pps_ctrl;

typedef PACKING union
{
    PACKING struct
    {
        FIELD slp_duration_max_h        : 16;
        FIELD rsv_16                    : 4;
        FIELD pwd_mode_i                : 3;
        FIELD rsv_23                    : 1;
        FIELD pwr_sv_start              : 1;
        FIELD onu_wakeup                : 1;
        FIELD timedrift_ignore          : 1;
        FIELD rsv_27                    : 1;
        FIELD pwd_mode                  : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_e_pwr_sv_cfg, *PREG_e_pwr_sv_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD slp_duration_max_l        : 32;
    } Bits;
    UINT32 Raw;
} REG_e_slp_durt_max, *PREG_e_slp_durt_max;

typedef PACKING union
{
    PACKING struct
    {
        FIELD slp_duration              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_slp_duration, *PREG_e_slp_duration;

typedef PACKING union
{
    PACKING struct
    {
        FIELD act_duration              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_act_duration, *PREG_e_act_duration;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pwron_dly                 : 32;
    } Bits;
    UINT32 Raw;
} REG_e_pwron_dly, *PREG_e_pwron_dly;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_slpalw_duration        : 16;
        FIELD rx_slpalw_flag            : 8;
        FIELD rx_slpalw_pwd_mode        : 8;
    } Bits;
    UINT32 Raw;
} REG_e_slp_duration_i, *PREG_e_slp_duration_i;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oui_a_val                 : 24;
        FIELD oui_a_dmy                 : 7;
        FIELD oui_a_en                  : 1;
    } Bits;
    UINT32 Raw;
} REG_e_oui_a_cfg, *PREG_e_oui_a_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oui_b_val                 : 24;
        FIELD oui_b_dmy                 : 7;
        FIELD oui_b_en                  : 1;
    } Bits;
    UINT32 Raw;
} REG_e_oui_b_cfg, *PREG_e_oui_b_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oui_c_val                 : 24;
        FIELD oui_c_dmy                 : 7;
        FIELD oui_c_en                  : 1;
    } Bits;
    UINT32 Raw;
} REG_e_oui_c_cfg, *PREG_e_oui_c_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 8;
        FIELD dygasp_num_of_times       : 8;
        FIELD sw_init_dygasp            : 1;
        FIELD rsv_17                    : 14;
        FIELD hw_dygasp_en              : 1;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_cfg, *PREG_e_dyinggsp_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w1_len_type      : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w1, *PREG_e_dyinggsp_w1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w2               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w2, *PREG_e_dyinggsp_w2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w3               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w3, *PREG_e_dyinggsp_w3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w4               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w4, *PREG_e_dyinggsp_w4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w5               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w5, *PREG_e_dyinggsp_w5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w6               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w6, *PREG_e_dyinggsp_w6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w7               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w7, *PREG_e_dyinggsp_w7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w8               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w8, *PREG_e_dyinggsp_w8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w9               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w9, *PREG_e_dyinggsp_w9;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w10              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w10, *PREG_e_dyinggsp_w10;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w11              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w11, *PREG_e_dyinggsp_w11;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dyinggsp_w12              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_w12, *PREG_e_dyinggsp_w12;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_en              : 1;
        FIELD oam_kpalv_sw_cfg          : 1;
        FIELD oam_kpalv_sw_trig         : 1;
        FIELD rsv_3                     : 5;
        FIELD oam_kpalv_interval        : 12;
        FIELD oam_kpalv_llid_idx        : 6;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_ctrl, *PREG_e_oam_kpalv_ctrl;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w1              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w1, *PREG_e_oam_kpalv_w1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w2              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w2, *PREG_e_oam_kpalv_w2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w3              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w3, *PREG_e_oam_kpalv_w3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w4              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w4, *PREG_e_oam_kpalv_w4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w5              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w5, *PREG_e_oam_kpalv_w5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w6              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w6, *PREG_e_oam_kpalv_w6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w7              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w7, *PREG_e_oam_kpalv_w7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w8              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w8, *PREG_e_oam_kpalv_w8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w9              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w9, *PREG_e_oam_kpalv_w9;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w10             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w10, *PREG_e_oam_kpalv_w10;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w11             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w11, *PREG_e_oam_kpalv_w11;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oam_kpalv_w12             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_w12, *PREG_e_oam_kpalv_w12;

typedef PACKING union
{
    PACKING struct
    {
        FIELD kpalv_hwen_sts            : 32;
    } Bits;
    UINT32 Raw;
} REG_e_kpalv_hwen_sts, *PREG_e_kpalv_hwen_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mcgrp_llid_en             : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_mcgrp_en, *PREG_e_mcgrp_en;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mcgrp_llid0_val           : 16;
        FIELD mcgrp_llid1_val           : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg0, *PREG_e_rxuni_mcllid_cfg0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mcgrp_llid2_val           : 16;
        FIELD mcgrp_llid3_val           : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg1, *PREG_e_rxuni_mcllid_cfg1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mcgrp_llid4_val           : 16;
        FIELD mcgrp_llid5_val           : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg2, *PREG_e_rxuni_mcllid_cfg2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mcgrp_llid6_val           : 16;
        FIELD mcgrp_llid7_val           : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg3, *PREG_e_rxuni_mcllid_cfg3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mcgrp_llid8_val           : 16;
        FIELD mcgrp_llid9_val           : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg4, *PREG_e_rxuni_mcllid_cfg4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mcgrp_llid10_val          : 16;
        FIELD mcgrp_llid11_val          : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg5, *PREG_e_rxuni_mcllid_cfg5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mcgrp_llid12_val          : 16;
        FIELD mcgrp_llid13_val          : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg6, *PREG_e_rxuni_mcllid_cfg6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mcgrp_llid14_val          : 16;
        FIELD mcgrp_llid15_val          : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg7, *PREG_e_rxuni_mcllid_cfg7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD decrpt_mode               : 3;
        FIELD crcchk_mode               : 1;
        FIELD encrpt_mode               : 2;
        FIELD crpt_dmy1                 : 2;
        FIELD mcgrp_decrpt_en           : 16;
        FIELD llidbc_decrpt_en          : 1;
        FIELD llidmc_decrpt_en          : 1;
        FIELD llidalluc_decrpt_en       : 1;
        FIELD dygasp_encrpt_en          : 1;
        FIELD mpcp_encrpt_en            : 1;
        FIELD kpalv_encrpt_en           : 1;
        FIELD crpt_dmy0                 : 2;
    } Bits;
    UINT32 Raw;
} REG_e_crpt_cfg, *PREG_e_crpt_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cfbdec_padd_mode          : 1;
        FIELD cfbdec_iv_srcsel          : 1;
        FIELD cfbsec_dmy                : 30;
    } Bits;
    UINT32 Raw;
} REG_e_cfbsec_cfg, *PREG_e_cfbsec_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ctrdec_padd_mode          : 1;
        FIELD ctrdec_srcaddr_sel        : 1;
        FIELD ctrsec_dmy1               : 14;
        FIELD ctrenc_padd_mode          : 1;
        FIELD ctrsec_dmy0               : 15;
    } Bits;
    UINT32 Raw;
} REG_e_ctrsec_cfg, *PREG_e_ctrsec_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD desec_srcaddr_value_h     : 16;
        FIELD desec_srcaddr_llid_idx    : 6;
        FIELD rsv_22                    : 8;
        FIELD desec_srcaddr_rwcmd_done  : 1;
        FIELD desec_srcaddr_rwcmd       : 1;
    } Bits;
    UINT32 Raw;
} REG_e_desec_srcaddr_cfg, *PREG_e_desec_srcaddr_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD desec_srcaddr_value_l     : 32;
    } Bits;
    UINT32 Raw;
} REG_e_desec_srcaddr_val, *PREG_e_desec_srcaddr_val;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sectag_e1c1_drop          : 1;
        FIELD sectag_e1c1_icvchk        : 1;
        FIELD sectag_e1c1_desec         : 1;
        FIELD sectag_e1c0_drop          : 1;
        FIELD sectag_e1c0_icvchk        : 1;
        FIELD sectag_e1c0_desec         : 1;
        FIELD sectag_e0c1_drop          : 1;
        FIELD sectag_e0c1_icvchk        : 1;
        FIELD sectag_e0c1_desec         : 1;
        FIELD sectag_e0c0_drop          : 1;
        FIELD sectag_e0c0_icvchk        : 1;
        FIELD sectag_e0c0_desec         : 1;
        FIELD macsdec_conofst           : 8;
        FIELD macsdec_timout_en         : 1;
        FIELD macsdec_srcaddr_sel       : 1;
        FIELD macsdec_ct_bit_swap       : 1;
        FIELD macsdec_pt_bit_swap       : 1;
        FIELD macsdec_aad_bit_swap      : 1;
        FIELD macsdec_iv_bit_swap       : 1;
        FIELD macsdec_tag_bit_swap      : 1;
        FIELD macsdec_dmy0              : 5;
    } Bits;
    UINT32 Raw;
} REG_e_macsdec_cfg, *PREG_e_macsdec_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD macsenc_dmy2              : 2;
        FIELD macsenc_tag_c             : 1;
        FIELD macsenc_tag_e             : 1;
        FIELD macsenc_tag_scb           : 1;
        FIELD macsenc_tag_sc            : 1;
        FIELD macsenc_tag_es            : 1;
        FIELD macsenc_tag_v             : 1;
        FIELD macsenc_dmy1              : 12;
        FIELD macsenc_timout_en         : 1;
        FIELD macsenc_srcaddr_sel       : 1;
        FIELD macsenc_ct_bit_swap       : 1;
        FIELD macsenc_pt_bit_swap       : 1;
        FIELD macsenc_aad_bit_swap      : 1;
        FIELD macsenc_iv_bit_swap       : 1;
        FIELD macsenc_tag_bit_swap      : 1;
        FIELD macsenc_dmy0              : 3;
        FIELD macsenc_mode              : 2;
    } Bits;
    UINT32 Raw;
} REG_e_macsenc_cfg, *PREG_e_macsenc_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD macsenc_pn_inival         : 32;
    } Bits;
    UINT32 Raw;
} REG_e_macsenc_pnini_cfg, *PREG_e_macsenc_pnini_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 16;
        FIELD macsenc_pn_llid_idx       : 5;
        FIELD rsv_21                    : 9;
        FIELD macsenc_pn_rwcmd_done     : 1;
        FIELD macsenc_pn_rwcmd          : 1;
    } Bits;
    UINT32 Raw;
} REG_e_macsenc_pn_cfg, *PREG_e_macsenc_pn_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD macsenc_pn_val            : 32;
    } Bits;
    UINT32 Raw;
} REG_e_macsenc_pn_val, *PREG_e_macsenc_pn_val;

typedef PACKING union
{
    PACKING struct
    {
        FIELD olt_macaddr_h             : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_olt_macaddr_h, *PREG_e_olt_macaddr_h;

typedef PACKING union
{
    PACKING struct
    {
        FIELD olt_macaddr_l             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_olt_macaddr_l, *PREG_e_olt_macaddr_l;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ds_mpcp_snf               : 1;
        FIELD ds_nrlgate_snf            : 1;
        FIELD snf_rsv3                  : 2;
        FIELD alluc_llid_snf            : 1;
        FIELD alluc_oam_snf             : 1;
        FIELD alluc_nrlgate_snf         : 1;
        FIELD alluc_eth_snf             : 1;
        FIELD mcgrp_llid_snf            : 1;
        FIELD mcgrp_oam_snf             : 1;
        FIELD mcgrp_nrlgate_snf         : 1;
        FIELD mcgrp_eth_snf             : 1;
        FIELD snf_rsv2                  : 4;
        FIELD us_oam_snf                : 1;
        FIELD us_mpcp_snf               : 1;
        FIELD us_rpt_snf                : 1;
        FIELD us_dygasp_snf             : 1;
        FIELD us_kpalv_snf              : 1;
        FIELD us_snf_mode               : 1;
        FIELD snf_rsv1                  : 2;
        FIELD snf_fcserr_fwd            : 1;
        FIELD llidinfo_snf              : 1;
        FIELD snf_rsv0                  : 5;
        FIELD sniffer_mode              : 1;
    } Bits;
    UINT32 Raw;
} REG_e_snf_cfg, *PREG_e_snf_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD snf_sp_tag                : 32;
    } Bits;
    UINT32 Raw;
} REG_e_snf_sp_tag, *PREG_e_snf_sp_tag;

typedef PACKING union
{
    PACKING struct
    {
        FIELD snf_pkt_dah               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_snf_dah, *PREG_e_snf_dah;

typedef PACKING union
{
    PACKING struct
    {
        FIELD snf_pkt_sah               : 16;
        FIELD snf_pkt_dal               : 16;
    } Bits;
    UINT32 Raw;
} REG_e_snf_dal, *PREG_e_snf_dal;

typedef PACKING union
{
    PACKING struct
    {
        FIELD snf_pkt_sal               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_snf_sal, *PREG_e_snf_sal;

typedef PACKING union
{
    PACKING struct
    {
        FIELD snf_pkt_etype             : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_snf_etype, *PREG_e_snf_etype;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rdmdly_hthr               : 16;
        FIELD rdmdly_lthr               : 14;
        FIELD rdmdly_mode               : 2;
    } Bits;
    UINT32 Raw;
} REG_e_rdmdly_cfg, *PREG_e_rdmdly_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rdmdly                    : 16;
        FIELD dscvgate_gntlen           : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rdmdly_stat, *PREG_e_rdmdly_stat;

typedef PACKING union
{
    PACKING struct
    {
        FIELD probe_sel                 : 8;
        FIELD probe_bit0_sel            : 8;
        FIELD probe_dtgrp_sel           : 8;
        FIELD probe_dmysel              : 8;
    } Bits;
    UINT32 Raw;
} REG_e_dbg_prb_sel, *PREG_e_dbg_prb_sel;

typedef PACKING union
{
    PACKING struct
    {
        FIELD probe_h32                 : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dbg_prb_h32, *PREG_e_dbg_prb_h32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD probe_l32                 : 32;
    } Bits;
    UINT32 Raw;
} REG_e_dbg_prb_l32, *PREG_e_dbg_prb_l32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cf_utili_mode             : 1;
        FIELD cf_utili_pkttype          : 1;
        FIELD cf_utili_rsv              : 6;
        FIELD cf_utili_pktovhd          : 8;
        FIELD cf_utili_wdm              : 16;
    } Bits;
    UINT32 Raw;
} REG_e_utili_cfg, *PREG_e_utili_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cf_utilization_chken      : 1;
        FIELD cf_utilization_cntclr     : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_e_utili_check, *PREG_e_utili_check;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_gnt_size_h32        : 32;
    } Bits;
    UINT32 Raw;
} REG_total_gnt_sizeh, *PREG_total_gnt_sizeh;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_gnt_size_l32        : 32;
    } Bits;
    UINT32 Raw;
} REG_total_gnt_sizel, *PREG_total_gnt_sizel;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_pkt_cnt             : 32;
    } Bits;
    UINT32 Raw;
} REG_total_pkt_cnt, *PREG_total_pkt_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_pkt_size_l32        : 32;
    } Bits;
    UINT32 Raw;
} REG_total_pkt_size_l, *PREG_total_pkt_size_l;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_pkt_size_h32        : 32;
    } Bits;
    UINT32 Raw;
} REG_total_pkt_size_h, *PREG_total_pkt_size_h;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_u10g_nouse_gntlen   : 32;
    } Bits;
    UINT32 Raw;
} REG_total_u10g_nouse_gntlen, *PREG_total_u10g_nouse_gntlen;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_gnt_cnt             : 32;
    } Bits;
    UINT32 Raw;
} REG_total_gnt_cnt, *PREG_total_gnt_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_tx_cyc_h32          : 32;
    } Bits;
    UINT32 Raw;
} REG_total_tx_cyc_h, *PREG_total_tx_cyc_h;

typedef PACKING union
{
    PACKING struct
    {
        FIELD total_tx_cyc_l32          : 32;
    } Bits;
    UINT32 Raw;
} REG_total_tx_cyc_l, *PREG_total_tx_cyc_l;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy6               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy6, *PREG_e_sts_dmy6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy7               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy7, *PREG_e_sts_dmy7;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy8               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy8, *PREG_e_sts_dmy8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy9               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy9, *PREG_e_sts_dmy9;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy10              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy10, *PREG_e_sts_dmy10;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy26              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy26, *PREG_e_sts_dmy26;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy27              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy27, *PREG_e_sts_dmy27;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy28              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy28, *PREG_e_sts_dmy28;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy29              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy29, *PREG_e_sts_dmy29;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xemac_sram_pd             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sram_pd, *PREG_e_sram_pd;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xemac_mbist_delsel_cfg0   : 32;
    } Bits;
    UINT32 Raw;
} REG_e_mbist_delsel_cfg0, *PREG_e_mbist_delsel_cfg0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xemac_mbist_delsel_cfg1   : 32;
    } Bits;
    UINT32 Raw;
} REG_e_mbist_delsel_cfg1, *PREG_e_mbist_delsel_cfg1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xemac_mbist_delsel_cfg2   : 32;
    } Bits;
    UINT32 Raw;
} REG_e_mbist_delsel_cfg2, *PREG_e_mbist_delsel_cfg2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xemac_mbist_delsel_cfg3   : 32;
    } Bits;
    UINT32 Raw;
} REG_e_mbist_delsel_cfg3, *PREG_e_mbist_delsel_cfg3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xemac_mbist_done_sts0     : 32;
    } Bits;
    UINT32 Raw;
} REG_e_mbist_done_sts0, *PREG_e_mbist_done_sts0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xemac_mbist_fail_sts0     : 32;
    } Bits;
    UINT32 Raw;
} REG_e_mbist_fail_sts0, *PREG_e_mbist_fail_sts0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xemac_mbist_fail_sts1     : 32;
    } Bits;
    UINT32 Raw;
} REG_e_mbist_fail_sts1, *PREG_e_mbist_fail_sts1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sld_b0                    : 1;
        FIELD sld_b1                    : 1;
        FIELD sld_b2                    : 1;
        FIELD sld_b3                    : 1;
        FIELD sld_b4                    : 1;
        FIELD sld_b5                    : 1;
        FIELD sld_b6                    : 1;
        FIELD sld_b7                    : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_rx_sld_sts, *PREG_rx_sld_sts;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmpi_fifound_thr         : 11;
        FIELD rsv_11                    : 4;
        FIELD txmpi_fifound_pktgate_en  : 1;
        FIELD txmpi_fifound_pktgate_gntcnt : 16;
    } Bits;
    UINT32 Raw;
} REG_e_glue_cfg, *PREG_e_glue_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD gntreq_grdcyc             : 31;
        FIELD gntreq_grden              : 1;
    } Bits;
    UINT32 Raw;
} REG_e_gntreq_tmout, *PREG_e_gntreq_tmout;

typedef PACKING union
{
    PACKING struct
    {
        FIELD fpga_gendef               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_fpga_gendef, *PREG_e_fpga_gendef;

typedef PACKING union
{
    PACKING struct
    {
        FIELD fpga_genver               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_fpga_genver, *PREG_e_fpga_genver;

typedef PACKING union
{
    PACKING struct
    {
        FIELD glb_cntclr                : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_e_cnt_clr, *PREG_e_cnt_clr;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmbi_eth_cnt             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmbi_eth_cnt, *PREG_e_rxmbi_eth_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_eth_cnt             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_eth_cnt, *PREG_e_rxmpi_eth_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmbi_eth_cnt             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_txmbi_eth_cnt, *PREG_e_txmbi_eth_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmpi_eth_cnt             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_txmpi_eth_cnt, *PREG_e_txmpi_eth_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_oam_cnt                : 16;
        FIELD rx_oam_cnt                : 16;
    } Bits;
    UINT32 Raw;
} REG_e_oam_stat, *PREG_e_oam_stat;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mpcp_dscv_gate_cnt        : 16;
        FIELD mpcp_rgst_cnt             : 8;
        FIELD mpcp_err_cnt              : 8;
    } Bits;
    UINT32 Raw;
} REG_e_mpcp_stat, *PREG_e_mpcp_stat;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mpcp_rgst_ack_cnt         : 8;
        FIELD mpcp_rgst_req_cnt         : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_mpcp_rgst_stat, *PREG_e_mpcp_rgst_stat;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmbi_bytecnt_h           : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmbi_bytecnt_h, *PREG_e_rxmbi_bytecnt_h;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmbi_bytecnt_l           : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmbi_bytecnt_l, *PREG_e_rxmbi_bytecnt_l;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmbi_snf_cnt             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmbi_snf_cnt, *PREG_e_rxmbi_snf_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_uceth_cnt           : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_uc_cnt, *PREG_e_rxmpi_uc_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_bceth_cnt           : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_bc_cnt, *PREG_e_rxmpi_bc_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_mceth_cnt           : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_mc_cnt, *PREG_e_rxmpi_mc_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_oam_cnt             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_oam_cnt, *PREG_e_rxmpi_oam_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_mpcp_cnt            : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_mpcp_cnt, *PREG_e_rxmpi_mpcp_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_gate_cnt            : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_gate_cnt, *PREG_e_rxmpi_gate_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_nrlgate_cnt         : 32;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_nrlgate_cnt, *PREG_e_rxmpi_nrlgate_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmbi_uceth_cnt           : 32;
    } Bits;
    UINT32 Raw;
} REG_e_txmbi_uceth_cnt, *PREG_e_txmbi_uceth_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmbi_mceth_cnt           : 16;
        FIELD txmbi_bceth_cnt           : 16;
    } Bits;
    UINT32 Raw;
} REG_e_txmbi_mceth_cnt, *PREG_e_txmbi_mceth_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmpi_dygasp_cnt          : 16;
        FIELD txmbi_dygasp_cnt          : 16;
    } Bits;
    UINT32 Raw;
} REG_e_tx_dygasp_cnt, *PREG_e_tx_dygasp_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmpi_rpt_cnt             : 16;
        FIELD txmbi_rpt_cnt             : 16;
    } Bits;
    UINT32 Raw;
} REG_e_tx_rpt_cnt, *PREG_e_tx_rpt_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmpi_kpalv_cnt           : 16;
        FIELD txmbi_kpalv_cnt           : 16;
    } Bits;
    UINT32 Raw;
} REG_e_kpoam_stat, *PREG_e_kpoam_stat;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmpi_rgack_cnt           : 8;
        FIELD txmpi_rgreq_cnt           : 8;
        FIELD txmpi_oam_cnt             : 16;
    } Bits;
    UINT32 Raw;
} REG_e_txmpi_mpcp_cnt, *PREG_e_txmpi_mpcp_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmpi_uceth_cnt           : 32;
    } Bits;
    UINT32 Raw;
} REG_e_txmpi_uceth_cnt, *PREG_e_txmpi_uceth_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmpi_mceth_cnt           : 16;
        FIELD txmpi_bceth_cnt           : 16;
    } Bits;
    UINT32 Raw;
} REG_e_txmpi_mceth_cnt, *PREG_e_txmpi_mceth_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 8;
        FIELD rx_filt_dscvgate_cnt      : 8;
        FIELD rx_llidbc_nrlgate_cnt     : 8;
        FIELD rx_infomis_dscvgate_cnt   : 8;
    } Bits;
    UINT32 Raw;
} REG_e_rxadv_cnt, *PREG_e_rxadv_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_churn_crcerr_cnt    : 16;
        FIELD rxmpi_churn_crcok_cnt     : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_churn_cnt, *PREG_e_rxmpi_churn_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_abchurn_cnt         : 16;
        FIELD rxmpi_churncrc_llididx    : 6;
        FIELD rsv_22                    : 1;
        FIELD rxmpi_churncrc_keyidx     : 1;
        FIELD rxmpi_churn_llididx       : 6;
        FIELD rsv_30                    : 1;
        FIELD rxmpi_churn_keyidx        : 1;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_churn_info, *PREG_e_rxmpi_churn_info;

typedef PACKING union
{
    PACKING struct
    {
        FIELD desec_plain_ok_cnt        : 16;
        FIELD desec_rx_ok_cnt           : 16;
    } Bits;
    UINT32 Raw;
} REG_e_desec_ok_cnt, *PREG_e_desec_ok_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_p_err_cnt           : 16;
        FIELD rxmpi_p_ok_cnt            : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_p_cnt, *PREG_e_rxmpi_p_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_crc8err_cnt         : 16;
        FIELD rxmpi_crc32err_cnt        : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_crc8err_cnt, *PREG_e_rxmpi_crc8err_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD desec_rxsof_ovrundrop_cnt : 16;
        FIELD desec_rxfifo_ovrundrop_cnt : 16;
    } Bits;
    UINT32 Raw;
} REG_e_desec_rxdrop_cnt, *PREG_e_desec_rxdrop_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD desec_rx_preerr_cnt       : 16;
        FIELD desec_rx_crcerr_cnt       : 16;
    } Bits;
    UINT32 Raw;
} REG_e_desec_rxerr_cnt, *PREG_e_desec_rxerr_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD desec_ciph_csdrop_cnt     : 16;
        FIELD desec_rxsof_hdrdrop_cnt   : 16;
    } Bits;
    UINT32 Raw;
} REG_e_desec_ciphdrop_cnt, *PREG_e_desec_ciphdrop_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD desec_aesw_csdrop_cnt     : 16;
        FIELD desec_ciph_actdrop_cnt    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_desec_aeswdrop_cnt, *PREG_e_desec_aeswdrop_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD desec_aesw_icverrdrop_cnt : 16;
        FIELD desec_aesw_ovrundrop_cnt  : 16;
    } Bits;
    UINT32 Raw;
} REG_e_desec_aeswdrop_cnt2, *PREG_e_desec_aeswdrop_cnt2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmpi_fifoovrun_cnt       : 16;
        FIELD rxmpi_eofdrop_cnt         : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_drop_cnt, *PREG_e_rxmpi_drop_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmbi_snfdrop_cnt         : 16;
        FIELD rxmbi_sofdrop_cnt         : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxmbi_drop_cnt, *PREG_e_rxmbi_drop_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rxmbi_enddrop_cnt         : 16;
        FIELD rxmbi_crcerr_cnt          : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxmbi_drop_cnt2, *PREG_e_rxmbi_drop_cnt2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmbi_err_cnt             : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_e_txmbi_err_cnt, *PREG_e_txmbi_err_cnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy11              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy11, *PREG_e_sts_dmy11;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy12              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy12, *PREG_e_sts_dmy12;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy13              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy13, *PREG_e_sts_dmy13;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy14              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy14, *PREG_e_sts_dmy14;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy15              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy15, *PREG_e_sts_dmy15;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy16              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy16, *PREG_e_sts_dmy16;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy17              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy17, *PREG_e_sts_dmy17;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy18              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy18, *PREG_e_sts_dmy18;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy19              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy19, *PREG_e_sts_dmy19;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy20              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy20, *PREG_e_sts_dmy20;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy21              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy21, *PREG_e_sts_dmy21;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy22              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy22, *PREG_e_sts_dmy22;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy23              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy23, *PREG_e_sts_dmy23;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy24              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy24, *PREG_e_sts_dmy24;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy25              : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy25, *PREG_e_sts_dmy25;

typedef PACKING union
{
    PACKING struct
    {
        FIELD hdn_gnt_cnt               : 16;
        FIELD b2b_gnt_cnt               : 16;
    } Bits;
    UINT32 Raw;
} REG_e_gnt_type_stat, *PREG_e_gnt_type_stat;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cur_gnt_pending_cnt       : 8;
        FIELD rsv_8                     : 8;
        FIELD max_gnt_pending_cnt       : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_e_gnt_pending_stat, *PREG_e_gnt_pending_stat;

typedef PACKING union
{
    PACKING struct
    {
        FIELD min_gnt_length            : 16;
        FIELD max_gnt_length            : 16;
    } Bits;
    UINT32 Raw;
} REG_e_gnt_length_stat, *PREG_e_gnt_length_stat;

typedef PACKING union
{
    PACKING struct
    {
        FIELD local_time                : 32;
    } Bits;
    UINT32 Raw;
} REG_e_local_time, *PREG_e_local_time;

typedef PACKING union
{
    PACKING struct
    {
        FIELD max_time_drift            : 8;
        FIELD cur_time_drift            : 8;
        FIELD max_time_drift_ofst       : 5;
        FIELD rsv_21                    : 3;
        FIELD cur_time_drift_ofst       : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_e_time_drft_stat, *PREG_e_time_drft_stat;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_max_time_drift         : 8;
        FIELD tx_cur_time_drift         : 8;
        FIELD tx_max_time_drift_ofst    : 5;
        FIELD rsv_21                    : 3;
        FIELD tx_cur_time_drift_ofst    : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_e_tx_timedrift_stat, *PREG_e_tx_timedrift_stat;

typedef PACKING union
{
    PACKING struct
    {
        FIELD max_rxfifo_depth          : 16;
        FIELD cur_rxfifo_depth          : 16;
    } Bits;
    UINT32 Raw;
} REG_e_rxfifo_depth_stat, *PREG_e_rxfifo_depth_stat;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy0               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy0, *PREG_e_sts_dmy0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy1               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy1, *PREG_e_sts_dmy1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy2               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy2, *PREG_e_sts_dmy2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy3               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy3, *PREG_e_sts_dmy3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy4               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy4, *PREG_e_sts_dmy4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD xe_sts_dmy5               : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sts_dmy5, *PREG_e_sts_dmy5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmbi_bufwrap_wr_cnt      : 16;
        FIELD txmbi_bufwrap_wrerr_cnt   : 16;
    } Bits;
    UINT32 Raw;
} REG_e_txmbi_bufwrap_wrcnt, *PREG_e_txmbi_bufwrap_wrcnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD txmbi_bufwrap_rd_cnt      : 16;
        FIELD txmbi_bufwrap_rderr_cnt   : 16;
    } Bits;
    UINT32 Raw;
} REG_e_txmbi_bufwrap_rdcnt, *PREG_e_txmbi_bufwrap_rdcnt;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 16;
        FIELD txmbi_bufwrap_rddmy_cnt   : 16;
    } Bits;
    UINT32 Raw;
} REG_e_txmbi_bufwrap_rdcnt2, *PREG_e_txmbi_bufwrap_rdcnt2;

// ----------------- XEPON_MAC  Grouping Definitions -------------------
// ----------------- XEPON_MAC Register Definition -------------------
typedef volatile PACKING struct
{
    UINT32                          rsv_0000[6144];   // 0000..5FFC
    REG_e_glb_cfg                   e_glb_cfg;        // 6000
    REG_e_glb_cfg2                  e_glb_cfg2;       // 6004
    REG_e_glb_sts1                  e_glb_sts1;       // 6008
    REG_e_glb_sts2                  e_glb_sts2;       // 600C
    REG_e_int_status                e_int_status;     // 6010
    REG_e_int_en                    e_int_en;         // 6014
    REG_e_int_sts2                  e_int_sts2;       // 6018
    REG_e_int_en2                   e_int_en2;        // 601C
    REG_e_int_sts3                  e_int_sts3;       // 6020
    REG_e_int_en3                   e_int_en3;        // 6024
    REG_e_exc_sts                   e_exc_sts;        // 6028
    REG_e_rpt_timout                e_rpt_timout;     // 602C
    REG_e_mpcp_timout               e_mpcp_timout;    // 6030
    REG_e_ds_keychg                 e_ds_keychg;      // 6034
    REG_e_ds_keychg2                e_ds_keychg2;     // 6038
    REG_e_ds_keymis                 e_ds_keymis;      // 603C
    REG_e_ds_keymis2                e_ds_keymis2;     // 6040
    REG_e_us_keychg                 e_us_keychg;      // 6044
    REG_e_cfg_dmy0                  e_cfg_dmy0;       // 6048
    REG_e_cfg_dmy1                  e_cfg_dmy1;       // 604C
	union
    {
		struct{
			REG_e_llid0_3_cfg				e_llid0_3_cfg;	  // 6050
			REG_e_llid4_7_cfg				e_llid4_7_cfg;	  // 6054
			REG_e_llid8_11_cfg				e_llid8_11_cfg;   // 6058
			REG_e_llid12_15_cfg 			e_llid12_15_cfg;  // 605C
			REG_e_llid16_19_cfg 			e_llid16_19_cfg;  // 6060
			REG_e_llid20_23_cfg 			e_llid20_23_cfg;  // 6064
			REG_e_llid24_27_cfg 			e_llid24_27_cfg;  // 6068
			REG_e_llid28_31_cfg 			e_llid28_31_cfg;  // 606C		
		};
		REG_e_llid0_3_cfg			e_llid0_31_cfg[8];// 6050~606C
    };
    REG_e_cfg_dmy2                  e_cfg_dmy2;       // 6070
    REG_e_cfg_dmy3                  e_cfg_dmy3;       // 6074
    REG_e_cfg_dmy4                  e_cfg_dmy4;       // 6078
    REG_e_llid_dscvry_ctrl          e_llid_dscvry_ctrl; // 607C
    union
	{
		struct{
			REG_e_llid0_dscvry_sts			e_llid0_dscvry_sts; // 6080
			REG_e_llid1_dscvry_sts			e_llid1_dscvry_sts; // 6084
			REG_e_llid2_dscvry_sts			e_llid2_dscvry_sts; // 6088
			REG_e_llid3_dscvry_sts			e_llid3_dscvry_sts; // 608C
			REG_e_llid4_dscvry_sts			e_llid4_dscvry_sts; // 6090
			REG_e_llid5_dscvry_sts			e_llid5_dscvry_sts; // 6094
			REG_e_llid6_dscvry_sts			e_llid6_dscvry_sts; // 6098
			REG_e_llid7_dscvry_sts			e_llid7_dscvry_sts; // 609C
			REG_e_llid8_dscvry_sts			e_llid8_dscvry_sts; // 60A0
			REG_e_llid9_dscvry_sts			e_llid9_dscvry_sts; // 60A4
			REG_e_llid10_dscvry_sts 		e_llid10_dscvry_sts; // 60A8
			REG_e_llid11_dscvry_sts 		e_llid11_dscvry_sts; // 60AC
			REG_e_llid12_dscvry_sts 		e_llid12_dscvry_sts; // 60B0
			REG_e_llid13_dscvry_sts 		e_llid13_dscvry_sts; // 60B4
			REG_e_llid14_dscvry_sts 		e_llid14_dscvry_sts; // 60B8
			REG_e_llid15_dscvry_sts 		e_llid15_dscvry_sts; // 60BC
			REG_e_llid16_dscvry_sts 		e_llid16_dscvry_sts; // 60C0
			REG_e_llid17_dscvry_sts 		e_llid17_dscvry_sts; // 60C4
			REG_e_llid18_dscvry_sts 		e_llid18_dscvry_sts; // 60C8
			REG_e_llid19_dscvry_sts 		e_llid19_dscvry_sts; // 60CC
			REG_e_llid20_dscvry_sts 		e_llid20_dscvry_sts; // 60D0
			REG_e_llid21_dscvry_sts 		e_llid21_dscvry_sts; // 60D4
			REG_e_llid22_dscvry_sts 		e_llid22_dscvry_sts; // 60D8
			REG_e_llid23_dscvry_sts 		e_llid23_dscvry_sts; // 60DC
			REG_e_llid24_dscvry_sts 		e_llid24_dscvry_sts; // 60E0
			REG_e_llid25_dscvry_sts 		e_llid25_dscvry_sts; // 60E4
			REG_e_llid26_dscvry_sts 		e_llid26_dscvry_sts; // 60E8
			REG_e_llid27_dscvry_sts 		e_llid27_dscvry_sts; // 60EC
			REG_e_llid28_dscvry_sts 		e_llid28_dscvry_sts; // 60F0
			REG_e_llid29_dscvry_sts 		e_llid29_dscvry_sts; // 60F4
			REG_e_llid30_dscvry_sts 		e_llid30_dscvry_sts; // 60F8
			REG_e_llid31_dscvry_sts 		e_llid31_dscvry_sts; // 60FC
		};
		REG_e_llid0_dscvry_sts			e_llid0_31_dscvry_sts[32];// 6080~60FC
    };
    REG_e_pending_gnt_num           e_pending_gnt_num; // 6100
    REG_e_mac_addr_cfg              e_mac_addr_cfg;   // 6104
    REG_e_mac_addr_value            e_mac_addr_value; // 6108
    REG_e_security_key_cfg          e_security_key_cfg; // 610C
    REG_e_key_value                 e_key_value;      // 6110
    REG_e_enckey_cfg                e_enckey_cfg;     // 6114
    REG_e_enckey_val                e_enckey_val;     // 6118
    UINT32                          rsv_611C;         // 611C
    REG_e_cfg_dmy5                  e_cfg_dmy5;       // 6120
    REG_e_rpt_cfg                   e_rpt_cfg;        // 6124
    REG_e_rpt_cfg2                  e_rpt_cfg2;       // 6128
    REG_e_rpt_qthld_cfg             e_rpt_qthld_cfg;  // 612C
    REG_e_rpt_glb_cfg               e_rpt_glb_cfg;    // 6130
    REG_e_rpt_bmap                  e_rpt_bmap;       // 6134
    REG_e_frpt_thr                  e_frpt_thr;       // 6138
    REG_e_u1g_rpt_qsizeadj          e_u1g_rpt_qsizeadj; // 613C
    REG_e_u10g_rpt_qsizeadj         e_u10g_rpt_qsizeadj; // 6140
    REG_e_cfg_dmy7                  e_cfg_dmy7;       // 6144
    REG_e_cfg_dmy8                  e_cfg_dmy8;       // 6148
    REG_e_cfg_dmy9                  e_cfg_dmy9;       // 614C
    REG_e_cfg_dmy10                 e_cfg_dmy10;      // 6150
    REG_e_cfg_dmy11                 e_cfg_dmy11;      // 6154
    REG_e_cfg_dmy12                 e_cfg_dmy12;      // 6158
    REG_e_cfg_dmy13                 e_cfg_dmy13;      // 615C
    REG_e_cfg_dmy14                 e_cfg_dmy14;      // 6160
    REG_e_cfg_dmy15                 e_cfg_dmy15;      // 6164
    UINT32                          rsv_6168[6];      // 6168..617C
    REG_e_dfrpt_data5               e_dfrpt_data5;    // 6180
    REG_e_dfrpt_data6               e_dfrpt_data6;    // 6184
    REG_e_dfrpt_data7               e_dfrpt_data7;    // 6188
    REG_e_dfrpt_data8               e_dfrpt_data8;    // 618C
    REG_e_dfrpt_data9               e_dfrpt_data9;    // 6190
    REG_e_dfrpt_data10              e_dfrpt_data10;   // 6194
    REG_e_dfrpt_data11              e_dfrpt_data11;   // 6198
    REG_e_dfrpt_data12              e_dfrpt_data12;   // 619C
    REG_e_dfrpt_data13              e_dfrpt_data13;   // 61A0
    REG_e_dfrpt_data14              e_dfrpt_data14;   // 61A4
    UINT32                          rsv_61A8[6];      // 61A8..61BC
    REG_e_laser_onoff_time          e_laser_onoff_time; // 61C0
    REG_e_sync_time                 e_sync_time;      // 61C4
    REG_e_overhead_time_thr         e_overhead_time_thr; // 61C8
    REG_e_laser_onoff_time2         e_laser_onoff_time2; // 61CC
    REG_e_olt_dscvinfo              e_olt_dscvinfo;   // 61D0
    REG_e_grd_thrshld               e_grd_thrshld;    // 61D4
    REG_e_mpcp_timeout_intvl        e_mpcp_timeout_intvl; // 61D8
    REG_e_rpt_timeout_intvl         e_rpt_timeout_intvl; // 61DC
    REG_e_max_future_gnt_time       e_max_future_gnt_time; // 61E0
    REG_e_min_proc_time             e_min_proc_time;  // 61E4
    REG_e_trx_adjust_time1          e_trx_adjust_time1; // 61E8
    REG_e_trx_adjust_time2          e_trx_adjust_time2; // 61EC
    REG_e_trx_adjtime3              e_trx_adjtime3;   // 61F0
    REG_e_trx_adjtime4              e_trx_adjtime4;   // 61F4
    REG_e_trx_adjtime5              e_trx_adjtime5;   // 61F8
    REG_e_rxphydly_adjtime          e_rxphydly_adjtime; // 61FC
    REG_e_txfetch_cfg               e_txfetch_cfg;    // 6200
    REG_e_tx_cal_cnst               e_tx_cal_cnst;    // 6204
    REG_e_txcal_cnst2               e_txcal_cnst2;    // 6208
    REG_e_txcal_cnst3               e_txcal_cnst3;    // 620C
    REG_e_txsch_cfg                 e_txsch_cfg;      // 6210
    UINT32                          rsv_6214[11];     // 6214..623C
    REG_e_rxfifo_thr                e_rxfifo_thr;     // 6240
    REG_e_bcllid_cfg                e_bcllid_cfg;     // 6244
    REG_e_txfrm_cfg1                e_txfrm_cfg1;     // 6248
    REG_e_txfrm_cfg2                e_txfrm_cfg2;     // 624C
    REG_e_txfrm_cfg3                e_txfrm_cfg3;     // 6250
    UINT32                          rsv_6254[3];      // 6254..625C
    REG_e_tod_sync_x                e_tod_sync_x;     // 6260
    REG_e_tod_ltncy                 e_tod_ltncy;      // 6264
    REG_e_new_tod_p2p_offset_sec_l32 e_new_tod_p2p_offset_sec_l32; // 6268
    REG_e_new_tod_p2p_tod_offset_nsec e_new_tod_p2p_tod_offset_nsec; // 626C
    REG_e_tod_p2p_tod_sec_l32       e_tod_p2p_tod_sec_l32; // 6270
    REG_e_tod_p2p_tod_nsec          e_tod_p2p_tod_nsec; // 6274
    REG_e_tod_period                e_tod_period;     // 6278
    REG_e_tod_1pps_ctrl             e_tod_1pps_ctrl;  // 627C
    REG_e_pwr_sv_cfg                e_pwr_sv_cfg;     // 6280
    REG_e_slp_durt_max              e_slp_durt_max;   // 6284
    REG_e_slp_duration              e_slp_duration;   // 6288
    REG_e_act_duration              e_act_duration;   // 628C
    REG_e_pwron_dly                 e_pwron_dly;      // 6290
    REG_e_slp_duration_i            e_slp_duration_i; // 6294
    REG_e_oui_a_cfg                 e_oui_a_cfg;      // 6298
    REG_e_oui_b_cfg                 e_oui_b_cfg;      // 629C
    REG_e_oui_c_cfg                 e_oui_c_cfg;      // 62A0
    UINT32                          rsv_62A4[2];      // 62A4..62A8
    REG_e_dyinggsp_cfg              e_dyinggsp_cfg;   // 62AC
    REG_e_dyinggsp_w1               e_dyinggsp_w1;    // 62B0
    REG_e_dyinggsp_w2               e_dyinggsp_w2;    // 62B4
    REG_e_dyinggsp_w3               e_dyinggsp_w3;    // 62B8
    REG_e_dyinggsp_w4               e_dyinggsp_w4;    // 62BC
    REG_e_dyinggsp_w5               e_dyinggsp_w5;    // 62C0
    REG_e_dyinggsp_w6               e_dyinggsp_w6;    // 62C4
    REG_e_dyinggsp_w7               e_dyinggsp_w7;    // 62C8
    REG_e_dyinggsp_w8               e_dyinggsp_w8;    // 62CC
    REG_e_dyinggsp_w9               e_dyinggsp_w9;    // 62D0
    REG_e_dyinggsp_w10              e_dyinggsp_w10;   // 62D4
    REG_e_dyinggsp_w11              e_dyinggsp_w11;   // 62D8
    REG_e_dyinggsp_w12              e_dyinggsp_w12;   // 62DC
    REG_e_oam_kpalv_ctrl            e_oam_kpalv_ctrl; // 62E0
    REG_e_oam_kpalv_w1              e_oam_kpalv_w1;   // 62E4
    REG_e_oam_kpalv_w2              e_oam_kpalv_w2;   // 62E8
    REG_e_oam_kpalv_w3              e_oam_kpalv_w3;   // 62EC
    REG_e_oam_kpalv_w4              e_oam_kpalv_w4;   // 62F0
    REG_e_oam_kpalv_w5              e_oam_kpalv_w5;   // 62F4
    REG_e_oam_kpalv_w6              e_oam_kpalv_w6;   // 62F8
    REG_e_oam_kpalv_w7              e_oam_kpalv_w7;   // 62FC
    REG_e_oam_kpalv_w8              e_oam_kpalv_w8;   // 6300
    REG_e_oam_kpalv_w9              e_oam_kpalv_w9;   // 6304
    REG_e_oam_kpalv_w10             e_oam_kpalv_w10;  // 6308
    REG_e_oam_kpalv_w11             e_oam_kpalv_w11;  // 630C
    REG_e_oam_kpalv_w12             e_oam_kpalv_w12;  // 6310
    REG_e_kpalv_hwen_sts            e_kpalv_hwen_sts; // 6314
    UINT32                          rsv_6318[2];      // 6318..631C
    REG_e_mcgrp_en                  e_mcgrp_en;       // 6320
    REG_e_rxuni_mcllid_cfg0         e_rxuni_mcllid_cfg0; // 6324
    REG_e_rxuni_mcllid_cfg1         e_rxuni_mcllid_cfg1; // 6328
    REG_e_rxuni_mcllid_cfg2         e_rxuni_mcllid_cfg2; // 632C
    REG_e_rxuni_mcllid_cfg3         e_rxuni_mcllid_cfg3; // 6330
    REG_e_rxuni_mcllid_cfg4         e_rxuni_mcllid_cfg4; // 6334
    REG_e_rxuni_mcllid_cfg5         e_rxuni_mcllid_cfg5; // 6338
    REG_e_rxuni_mcllid_cfg6         e_rxuni_mcllid_cfg6; // 633C
    REG_e_rxuni_mcllid_cfg7         e_rxuni_mcllid_cfg7; // 6340
    UINT32                          rsv_6344[3];      // 6344..634C
    REG_e_crpt_cfg                  e_crpt_cfg;       // 6350
    REG_e_cfbsec_cfg                e_cfbsec_cfg;     // 6354
    REG_e_ctrsec_cfg                e_ctrsec_cfg;     // 6358
    UINT32                          rsv_635C;         // 635C
    REG_e_desec_srcaddr_cfg         e_desec_srcaddr_cfg; // 6360
    REG_e_desec_srcaddr_val         e_desec_srcaddr_val; // 6364
    UINT32                          rsv_6368[2];      // 6368..636C
    REG_e_macsdec_cfg               e_macsdec_cfg;    // 6370
    REG_e_macsenc_cfg               e_macsenc_cfg;    // 6374
    REG_e_macsenc_pnini_cfg         e_macsenc_pnini_cfg; // 6378
    UINT32                          rsv_637C;         // 637C
    REG_e_macsenc_pn_cfg            e_macsenc_pn_cfg; // 6380
    REG_e_macsenc_pn_val            e_macsenc_pn_val; // 6384
    UINT32                          rsv_6388[2];      // 6388..638C
    REG_e_olt_macaddr_h             e_olt_macaddr_h;  // 6390
    REG_e_olt_macaddr_l             e_olt_macaddr_l;  // 6394
    UINT32                          rsv_6398[26];     // 6398..63FC
    REG_e_snf_cfg                   e_snf_cfg;        // 6400
    REG_e_snf_sp_tag                e_snf_sp_tag;     // 6404
    REG_e_snf_dah                   e_snf_dah;        // 6408
    REG_e_snf_dal                   e_snf_dal;        // 640C
    REG_e_snf_sal                   e_snf_sal;        // 6410
    REG_e_snf_etype                 e_snf_etype;      // 6414
    UINT32                          rsv_6418[6];      // 6418..642C
    REG_e_rdmdly_cfg                e_rdmdly_cfg;     // 6430
    REG_e_rdmdly_stat               e_rdmdly_stat;    // 6434
    UINT32                          rsv_6438[2];      // 6438..643C
    REG_e_dbg_prb_sel               e_dbg_prb_sel;    // 6440
    REG_e_dbg_prb_h32               e_dbg_prb_h32;    // 6444
    REG_e_dbg_prb_l32               e_dbg_prb_l32;    // 6448
    REG_e_utili_cfg                 e_utili_cfg;      // 644C
    REG_e_utili_check               e_utili_check;    // 6450
    REG_total_gnt_sizeh             total_gnt_sizeh;  // 6454
    REG_total_gnt_sizel             total_gnt_sizel;  // 6458
    REG_total_pkt_cnt               total_pkt_cnt;    // 645C
    REG_total_pkt_size_l            total_pkt_size_l; // 6460
    REG_total_pkt_size_h            total_pkt_size_h; // 6464
    REG_total_u10g_nouse_gntlen     total_u10g_nouse_gntlen; // 6468
    REG_total_gnt_cnt               total_gnt_cnt;    // 646C
    REG_total_tx_cyc_h              total_tx_cyc_h;   // 6470
    REG_total_tx_cyc_l              total_tx_cyc_l;   // 6474
    REG_e_sts_dmy6                  e_sts_dmy6;       // 6478
    REG_e_sts_dmy7                  e_sts_dmy7;       // 647C
    REG_e_sts_dmy8                  e_sts_dmy8;       // 6480
    REG_e_sts_dmy9                  e_sts_dmy9;       // 6484
    REG_e_sts_dmy10                 e_sts_dmy10;      // 6488
    REG_e_sts_dmy26                 e_sts_dmy26;      // 648C
    REG_e_sts_dmy27                 e_sts_dmy27;      // 6490
    REG_e_sts_dmy28                 e_sts_dmy28;      // 6494
    REG_e_sts_dmy29                 e_sts_dmy29;      // 6498
    REG_e_sram_pd                   e_sram_pd;        // 649C
    REG_e_mbist_delsel_cfg0         e_mbist_delsel_cfg0; // 64A0
    REG_e_mbist_delsel_cfg1         e_mbist_delsel_cfg1; // 64A4
    REG_e_mbist_delsel_cfg2         e_mbist_delsel_cfg2; // 64A8
    REG_e_mbist_delsel_cfg3         e_mbist_delsel_cfg3; // 64AC
    REG_e_mbist_done_sts0           e_mbist_done_sts0; // 64B0
    REG_e_mbist_fail_sts0           e_mbist_fail_sts0; // 64B4
    REG_e_mbist_fail_sts1           e_mbist_fail_sts1; // 64B8
    UINT32                          rsv_64BC[5];      // 64BC..64CC
    REG_rx_sld_sts                  rx_sld_sts;       // 64D0
    UINT32                          rsv_64D4[3];      // 64D4..64DC
    REG_e_glue_cfg                  e_glue_cfg;       // 64E0
    REG_e_gntreq_tmout              e_gntreq_tmout;   // 64E4
    UINT32                          rsv_64E8[2];      // 64E8..64EC
    REG_e_fpga_gendef               e_fpga_gendef;    // 64F0
    REG_e_fpga_genver               e_fpga_genver;    // 64F4
    UINT32                          rsv_64F8[2];      // 64F8..64FC
    REG_e_cnt_clr                   e_cnt_clr;        // 6500
    UINT32                          rsv_6504[3];      // 6504..650C
    REG_e_rxmbi_eth_cnt             e_rxmbi_eth_cnt;  // 6510
    REG_e_rxmpi_eth_cnt             e_rxmpi_eth_cnt;  // 6514
    REG_e_txmbi_eth_cnt             e_txmbi_eth_cnt;  // 6518
    REG_e_txmpi_eth_cnt             e_txmpi_eth_cnt;  // 651C
    REG_e_oam_stat                  e_oam_stat;       // 6520
    REG_e_mpcp_stat                 e_mpcp_stat;      // 6524
    REG_e_mpcp_rgst_stat            e_mpcp_rgst_stat; // 6528
    UINT32                          rsv_652C[21];     // 652C..657C
    REG_e_rxmbi_bytecnt_h           e_rxmbi_bytecnt_h; // 6580
    REG_e_rxmbi_bytecnt_l           e_rxmbi_bytecnt_l; // 6584
    REG_e_rxmbi_snf_cnt             e_rxmbi_snf_cnt;  // 6588
    UINT32                          rsv_658C[5];      // 658C..659C
    REG_e_rxmpi_uc_cnt              e_rxmpi_uc_cnt;   // 65A0
    REG_e_rxmpi_bc_cnt              e_rxmpi_bc_cnt;   // 65A4
    REG_e_rxmpi_mc_cnt              e_rxmpi_mc_cnt;   // 65A8
    REG_e_rxmpi_oam_cnt             e_rxmpi_oam_cnt;  // 65AC
    REG_e_rxmpi_mpcp_cnt            e_rxmpi_mpcp_cnt; // 65B0
    REG_e_rxmpi_gate_cnt            e_rxmpi_gate_cnt; // 65B4
    REG_e_rxmpi_nrlgate_cnt         e_rxmpi_nrlgate_cnt; // 65B8
    UINT32                          rsv_65BC[5];      // 65BC..65CC
    REG_e_txmbi_uceth_cnt           e_txmbi_uceth_cnt; // 65D0
    REG_e_txmbi_mceth_cnt           e_txmbi_mceth_cnt; // 65D4
    REG_e_tx_dygasp_cnt             e_tx_dygasp_cnt;  // 65D8
    REG_e_tx_rpt_cnt                e_tx_rpt_cnt;     // 65DC
    REG_e_kpoam_stat                e_kpoam_stat;     // 65E0
    REG_e_txmpi_mpcp_cnt            e_txmpi_mpcp_cnt; // 65E4
    REG_e_txmpi_uceth_cnt           e_txmpi_uceth_cnt; // 65E8
    REG_e_txmpi_mceth_cnt           e_txmpi_mceth_cnt; // 65EC
    UINT32                          rsv_65F0[4];      // 65F0..65FC
    REG_e_rxadv_cnt                 e_rxadv_cnt;      // 6600
    REG_e_rxmpi_churn_cnt           e_rxmpi_churn_cnt; // 6604
    REG_e_rxmpi_churn_info          e_rxmpi_churn_info; // 6608
    UINT32                          rsv_660C;         // 660C
    REG_e_desec_ok_cnt              e_desec_ok_cnt;   // 6610
    REG_e_rxmpi_p_cnt               e_rxmpi_p_cnt;    // 6614
    REG_e_rxmpi_crc8err_cnt         e_rxmpi_crc8err_cnt; // 6618
    REG_e_desec_rxdrop_cnt          e_desec_rxdrop_cnt; // 661C
    REG_e_desec_rxerr_cnt           e_desec_rxerr_cnt; // 6620
    REG_e_desec_ciphdrop_cnt        e_desec_ciphdrop_cnt; // 6624
    REG_e_desec_aeswdrop_cnt        e_desec_aeswdrop_cnt; // 6628
    REG_e_desec_aeswdrop_cnt2       e_desec_aeswdrop_cnt2; // 662C
    REG_e_rxmpi_drop_cnt            e_rxmpi_drop_cnt; // 6630
    UINT32                          rsv_6634[3];      // 6634..663C
    REG_e_rxmbi_drop_cnt            e_rxmbi_drop_cnt; // 6640
    REG_e_rxmbi_drop_cnt2           e_rxmbi_drop_cnt2; // 6644
    UINT32                          rsv_6648[14];     // 6648..667C
    REG_e_txmbi_err_cnt             e_txmbi_err_cnt;  // 6680
    REG_e_sts_dmy11                 e_sts_dmy11;      // 6684
    REG_e_sts_dmy12                 e_sts_dmy12;      // 6688
    REG_e_sts_dmy13                 e_sts_dmy13;      // 668C
    REG_e_sts_dmy14                 e_sts_dmy14;      // 6690
    REG_e_sts_dmy15                 e_sts_dmy15;      // 6694
    REG_e_sts_dmy16                 e_sts_dmy16;      // 6698
    REG_e_sts_dmy17                 e_sts_dmy17;      // 669C
    REG_e_sts_dmy18                 e_sts_dmy18;      // 66A0
    REG_e_sts_dmy19                 e_sts_dmy19;      // 66A4
    REG_e_sts_dmy20                 e_sts_dmy20;      // 66A8
    REG_e_sts_dmy21                 e_sts_dmy21;      // 66AC
    REG_e_sts_dmy22                 e_sts_dmy22;      // 66B0
    REG_e_sts_dmy23                 e_sts_dmy23;      // 66B4
    REG_e_sts_dmy24                 e_sts_dmy24;      // 66B8
    REG_e_sts_dmy25                 e_sts_dmy25;      // 66BC
    REG_e_gnt_type_stat             e_gnt_type_stat;  // 66C0
    REG_e_gnt_pending_stat          e_gnt_pending_stat; // 66C4
    REG_e_gnt_length_stat           e_gnt_length_stat; // 66C8
    REG_e_local_time                e_local_time;     // 66CC
    REG_e_time_drft_stat            e_time_drft_stat; // 66D0
    REG_e_tx_timedrift_stat         e_tx_timedrift_stat; // 66D4
    REG_e_rxfifo_depth_stat         e_rxfifo_depth_stat; // 66D8
    REG_e_sts_dmy0                  e_sts_dmy0;       // 66DC
    REG_e_sts_dmy1                  e_sts_dmy1;       // 66E0
    REG_e_sts_dmy2                  e_sts_dmy2;       // 66E4
    REG_e_sts_dmy3                  e_sts_dmy3;       // 66E8
    REG_e_sts_dmy4                  e_sts_dmy4;       // 66EC
    REG_e_sts_dmy5                  e_sts_dmy5;       // 66F0
    REG_e_txmbi_bufwrap_wrcnt       e_txmbi_bufwrap_wrcnt; // 66F4
    REG_e_txmbi_bufwrap_rdcnt       e_txmbi_bufwrap_rdcnt; // 66F8
    REG_e_txmbi_bufwrap_rdcnt2      e_txmbi_bufwrap_rdcnt2; // 66FC
    UINT32                          rsv_6700[7743];   // 6700..DFF8
    UINT8                           rsv_DFFC;         // DFFC
    UINT16                          rsv_DFFD;         // DFFD
    UINT8                           rsv_E0FF;         // E0FF
    UINT32                          rsv_E100[959];    // E100..EFF8
    UINT8                           rsv_EFFC;         // EFFC
    UINT16                          rsv_EFFD;         // EFFD
}EPON_MAC_REGS, *PEPON_MAC_REGS;

// ---------- XEPON_MAC Enum Definitions      ----------
// ---------- XEPON_MAC C Macro Definitions   ----------
extern PEPON_MAC_REGS g_EPON_MAC_BASE;

#define XEPON_MAC_BASE                                         (g_EPON_MAC_BASE)

#define e_glb_cfg                                              INREG32(&XEPON_MAC_BASE->e_glb_cfg) // 6000
#define e_glb_cfg2                                             INREG32(&XEPON_MAC_BASE->e_glb_cfg2) // 6004
#define e_glb_sts1                                             INREG32(&XEPON_MAC_BASE->e_glb_sts1) // 6008
#define e_glb_sts2                                             INREG32(&XEPON_MAC_BASE->e_glb_sts2) // 600C
#define e_int_status                                           INREG32(&XEPON_MAC_BASE->e_int_status) // 6010
#define e_int_en                                               INREG32(&XEPON_MAC_BASE->e_int_en) // 6014
#define e_int_sts2                                             INREG32(&XEPON_MAC_BASE->e_int_sts2) // 6018
#define e_int_en2                                              INREG32(&XEPON_MAC_BASE->e_int_en2) // 601C
#define e_int_sts3                                             INREG32(&XEPON_MAC_BASE->e_int_sts3) // 6020
#define e_int_en3                                              INREG32(&XEPON_MAC_BASE->e_int_en3) // 6024
#define e_exc_sts                                              INREG32(&XEPON_MAC_BASE->e_exc_sts) // 6028
#define e_rpt_timout                                           INREG32(&XEPON_MAC_BASE->e_rpt_timout) // 602C
#define e_mpcp_timout                                          INREG32(&XEPON_MAC_BASE->e_mpcp_timout) // 6030
#define e_ds_keychg                                            INREG32(&XEPON_MAC_BASE->e_ds_keychg) // 6034
#define e_ds_keychg2                                           INREG32(&XEPON_MAC_BASE->e_ds_keychg2) // 6038
#define e_ds_keymis                                            INREG32(&XEPON_MAC_BASE->e_ds_keymis) // 603C
#define e_ds_keymis2                                           INREG32(&XEPON_MAC_BASE->e_ds_keymis2) // 6040
#define e_us_keychg                                            INREG32(&XEPON_MAC_BASE->e_us_keychg) // 6044
#define e_cfg_dmy0                                             INREG32(&XEPON_MAC_BASE->e_cfg_dmy0) // 6048
#define e_cfg_dmy1                                             INREG32(&XEPON_MAC_BASE->e_cfg_dmy1) // 604C
#define e_llid0_3_cfg                                          INREG32(&XEPON_MAC_BASE->e_llid0_3_cfg) // 6050
#define e_llid4_7_cfg                                          INREG32(&XEPON_MAC_BASE->e_llid4_7_cfg) // 6054
#define e_llid8_11_cfg                                         INREG32(&XEPON_MAC_BASE->e_llid8_11_cfg) // 6058
#define e_llid12_15_cfg                                        INREG32(&XEPON_MAC_BASE->e_llid12_15_cfg) // 605C
#define e_llid16_19_cfg                                        INREG32(&XEPON_MAC_BASE->e_llid16_19_cfg) // 6060
#define e_llid20_23_cfg                                        INREG32(&XEPON_MAC_BASE->e_llid20_23_cfg) // 6064
#define e_llid24_27_cfg                                        INREG32(&XEPON_MAC_BASE->e_llid24_27_cfg) // 6068
#define e_llid28_31_cfg                                        INREG32(&XEPON_MAC_BASE->e_llid28_31_cfg) // 606C
#define e_cfg_dmy2                                             INREG32(&XEPON_MAC_BASE->e_cfg_dmy2) // 6070
#define e_cfg_dmy3                                             INREG32(&XEPON_MAC_BASE->e_cfg_dmy3) // 6074
#define e_cfg_dmy4                                             INREG32(&XEPON_MAC_BASE->e_cfg_dmy4) // 6078
#define e_llid_dscvry_ctrl                                     INREG32(&XEPON_MAC_BASE->e_llid_dscvry_ctrl) // 607C
#define e_llid0_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid0_dscvry_sts) // 6080
#define e_llid1_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid1_dscvry_sts) // 6084
#define e_llid2_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid2_dscvry_sts) // 6088
#define e_llid3_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid3_dscvry_sts) // 608C
#define e_llid4_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid4_dscvry_sts) // 6090
#define e_llid5_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid5_dscvry_sts) // 6094
#define e_llid6_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid6_dscvry_sts) // 6098
#define e_llid7_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid7_dscvry_sts) // 609C
#define e_llid8_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid8_dscvry_sts) // 60A0
#define e_llid9_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid9_dscvry_sts) // 60A4
#define e_llid10_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid10_dscvry_sts) // 60A8
#define e_llid11_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid11_dscvry_sts) // 60AC
#define e_llid12_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid12_dscvry_sts) // 60B0
#define e_llid13_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid13_dscvry_sts) // 60B4
#define e_llid14_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid14_dscvry_sts) // 60B8
#define e_llid15_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid15_dscvry_sts) // 60BC
#define e_llid16_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid16_dscvry_sts) // 60C0
#define e_llid17_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid17_dscvry_sts) // 60C4
#define e_llid18_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid18_dscvry_sts) // 60C8
#define e_llid19_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid19_dscvry_sts) // 60CC
#define e_llid20_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid20_dscvry_sts) // 60D0
#define e_llid21_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid21_dscvry_sts) // 60D4
#define e_llid22_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid22_dscvry_sts) // 60D8
#define e_llid23_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid23_dscvry_sts) // 60DC
#define e_llid24_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid24_dscvry_sts) // 60E0
#define e_llid25_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid25_dscvry_sts) // 60E4
#define e_llid26_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid26_dscvry_sts) // 60E8
#define e_llid27_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid27_dscvry_sts) // 60EC
#define e_llid28_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid28_dscvry_sts) // 60F0
#define e_llid29_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid29_dscvry_sts) // 60F4
#define e_llid30_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid30_dscvry_sts) // 60F8
#define e_llid31_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid31_dscvry_sts) // 60FC
#define e_llid0_31_dscvry_sts(i) \
({ \
	int idx = (i);\
	INREG32(&(XEPON_MAC_BASE->e_llid0_31_dscvry_sts[idx])); \
})
#define e_pending_gnt_num                                      INREG32(&XEPON_MAC_BASE->e_pending_gnt_num) // 6100
#define e_mac_addr_cfg                                         INREG32(&XEPON_MAC_BASE->e_mac_addr_cfg) // 6104
#define e_mac_addr_value                                       INREG32(&XEPON_MAC_BASE->e_mac_addr_value) // 6108
#define e_security_key_cfg                                     INREG32(&XEPON_MAC_BASE->e_security_key_cfg) // 610C
#define e_key_value                                            INREG32(&XEPON_MAC_BASE->e_key_value) // 6110
#define e_enckey_cfg                                           INREG32(&XEPON_MAC_BASE->e_enckey_cfg) // 6114
#define e_enckey_val                                           INREG32(&XEPON_MAC_BASE->e_enckey_val) // 6118
#define e_cfg_dmy5                                             INREG32(&XEPON_MAC_BASE->e_cfg_dmy5) // 6120
#define e_rpt_cfg                                              INREG32(&XEPON_MAC_BASE->e_rpt_cfg) // 6124
#define e_rpt_cfg2                                             INREG32(&XEPON_MAC_BASE->e_rpt_cfg2) // 6128
#define e_rpt_qthld_cfg                                        INREG32(&XEPON_MAC_BASE->e_rpt_qthld_cfg) // 612C
#define e_rpt_glb_cfg                                          INREG32(&XEPON_MAC_BASE->e_rpt_glb_cfg) // 6130
#define e_rpt_bmap                                             INREG32(&XEPON_MAC_BASE->e_rpt_bmap) // 6134
#define e_frpt_thr                                             INREG32(&XEPON_MAC_BASE->e_frpt_thr) // 6138
#define e_u1g_rpt_qsizeadj                                     INREG32(&XEPON_MAC_BASE->e_u1g_rpt_qsizeadj) // 613C
#define e_u10g_rpt_qsizeadj                                    INREG32(&XEPON_MAC_BASE->e_u10g_rpt_qsizeadj) // 6140
#define e_cfg_dmy7                                             INREG32(&XEPON_MAC_BASE->e_cfg_dmy7) // 6144
#define e_cfg_dmy8                                             INREG32(&XEPON_MAC_BASE->e_cfg_dmy8) // 6148
#define e_cfg_dmy9                                             INREG32(&XEPON_MAC_BASE->e_cfg_dmy9) // 614C
#define e_cfg_dmy10                                            INREG32(&XEPON_MAC_BASE->e_cfg_dmy10) // 6150
#define e_cfg_dmy11                                            INREG32(&XEPON_MAC_BASE->e_cfg_dmy11) // 6154
#define e_cfg_dmy12                                            INREG32(&XEPON_MAC_BASE->e_cfg_dmy12) // 6158
#define e_cfg_dmy13                                            INREG32(&XEPON_MAC_BASE->e_cfg_dmy13) // 615C
#define e_cfg_dmy14                                            INREG32(&XEPON_MAC_BASE->e_cfg_dmy14) // 6160
#define e_cfg_dmy15                                            INREG32(&XEPON_MAC_BASE->e_cfg_dmy15) // 6164
#define e_dfrpt_data5                                          INREG32(&XEPON_MAC_BASE->e_dfrpt_data5) // 6180
#define e_dfrpt_data6                                          INREG32(&XEPON_MAC_BASE->e_dfrpt_data6) // 6184
#define e_dfrpt_data7                                          INREG32(&XEPON_MAC_BASE->e_dfrpt_data7) // 6188
#define e_dfrpt_data8                                          INREG32(&XEPON_MAC_BASE->e_dfrpt_data8) // 618C
#define e_dfrpt_data9                                          INREG32(&XEPON_MAC_BASE->e_dfrpt_data9) // 6190
#define e_dfrpt_data10                                         INREG32(&XEPON_MAC_BASE->e_dfrpt_data10) // 6194
#define e_dfrpt_data11                                         INREG32(&XEPON_MAC_BASE->e_dfrpt_data11) // 6198
#define e_dfrpt_data12                                         INREG32(&XEPON_MAC_BASE->e_dfrpt_data12) // 619C
#define e_dfrpt_data13                                         INREG32(&XEPON_MAC_BASE->e_dfrpt_data13) // 61A0
#define e_dfrpt_data14                                         INREG32(&XEPON_MAC_BASE->e_dfrpt_data14) // 61A4
#define e_laser_onoff_time                                     INREG32(&XEPON_MAC_BASE->e_laser_onoff_time) // 61C0
#define e_sync_time                                            INREG32(&XEPON_MAC_BASE->e_sync_time) // 61C4
#define e_overhead_time_thr                                    INREG32(&XEPON_MAC_BASE->e_overhead_time_thr) // 61C8
#define e_laser_onoff_time2                                    INREG32(&XEPON_MAC_BASE->e_laser_onoff_time2) // 61CC
#define e_olt_dscvinfo                                         INREG32(&XEPON_MAC_BASE->e_olt_dscvinfo) // 61D0
#define e_grd_thrshld                                          INREG32(&XEPON_MAC_BASE->e_grd_thrshld) // 61D4
#define e_mpcp_timeout_intvl                                   INREG32(&XEPON_MAC_BASE->e_mpcp_timeout_intvl) // 61D8
#define e_rpt_timeout_intvl                                    INREG32(&XEPON_MAC_BASE->e_rpt_timeout_intvl) // 61DC
#define e_max_future_gnt_time                                  INREG32(&XEPON_MAC_BASE->e_max_future_gnt_time) // 61E0
#define e_min_proc_time                                        INREG32(&XEPON_MAC_BASE->e_min_proc_time) // 61E4
#define e_trx_adjust_time1                                     INREG32(&XEPON_MAC_BASE->e_trx_adjust_time1) // 61E8
#define e_trx_adjust_time2                                     INREG32(&XEPON_MAC_BASE->e_trx_adjust_time2) // 61EC
#define e_trx_adjtime3                                         INREG32(&XEPON_MAC_BASE->e_trx_adjtime3) // 61F0
#define e_trx_adjtime4                                         INREG32(&XEPON_MAC_BASE->e_trx_adjtime4) // 61F4
#define e_trx_adjtime5                                         INREG32(&XEPON_MAC_BASE->e_trx_adjtime5) // 61F8
#define e_rxphydly_adjtime                                     INREG32(&XEPON_MAC_BASE->e_rxphydly_adjtime) // 61FC
#define e_txfetch_cfg                                          INREG32(&XEPON_MAC_BASE->e_txfetch_cfg) // 6200
#define e_tx_cal_cnst                                          INREG32(&XEPON_MAC_BASE->e_tx_cal_cnst) // 6204
#define e_txcal_cnst2                                          INREG32(&XEPON_MAC_BASE->e_txcal_cnst2) // 6208
#define e_txcal_cnst3                                          INREG32(&XEPON_MAC_BASE->e_txcal_cnst3) // 620C
#define e_txsch_cfg                                            INREG32(&XEPON_MAC_BASE->e_txsch_cfg) // 6210
#define e_rxfifo_thr                                           INREG32(&XEPON_MAC_BASE->e_rxfifo_thr) // 6240
#define e_bcllid_cfg                                           INREG32(&XEPON_MAC_BASE->e_bcllid_cfg) // 6244
#define e_txfrm_cfg1                                           INREG32(&XEPON_MAC_BASE->e_txfrm_cfg1) // 6248
#define e_txfrm_cfg2                                           INREG32(&XEPON_MAC_BASE->e_txfrm_cfg2) // 624C
#define e_txfrm_cfg3                                           INREG32(&XEPON_MAC_BASE->e_txfrm_cfg3) // 6250
#define e_tod_sync_x                                           INREG32(&XEPON_MAC_BASE->e_tod_sync_x) // 6260
#define e_tod_ltncy                                            INREG32(&XEPON_MAC_BASE->e_tod_ltncy) // 6264
#define e_new_tod_p2p_offset_sec_l32                           INREG32(&XEPON_MAC_BASE->e_new_tod_p2p_offset_sec_l32) // 6268
#define e_new_tod_p2p_tod_offset_nsec                          INREG32(&XEPON_MAC_BASE->e_new_tod_p2p_tod_offset_nsec) // 626C
#define e_tod_p2p_tod_sec_l32                                  INREG32(&XEPON_MAC_BASE->e_tod_p2p_tod_sec_l32) // 6270
#define e_tod_p2p_tod_nsec                                     INREG32(&XEPON_MAC_BASE->e_tod_p2p_tod_nsec) // 6274
#define e_tod_period                                           INREG32(&XEPON_MAC_BASE->e_tod_period) // 6278
#define e_tod_1pps_ctrl                                        INREG32(&XEPON_MAC_BASE->e_tod_1pps_ctrl) // 627C
#define e_pwr_sv_cfg                                           INREG32(&XEPON_MAC_BASE->e_pwr_sv_cfg) // 6280
#define e_slp_durt_max                                         INREG32(&XEPON_MAC_BASE->e_slp_durt_max) // 6284
#define e_slp_duration                                         INREG32(&XEPON_MAC_BASE->e_slp_duration) // 6288
#define e_act_duration                                         INREG32(&XEPON_MAC_BASE->e_act_duration) // 628C
#define e_pwron_dly                                            INREG32(&XEPON_MAC_BASE->e_pwron_dly) // 6290
#define e_slp_duration_i                                       INREG32(&XEPON_MAC_BASE->e_slp_duration_i) // 6294
#define e_oui_a_cfg                                            INREG32(&XEPON_MAC_BASE->e_oui_a_cfg) // 6298
#define e_oui_b_cfg                                            INREG32(&XEPON_MAC_BASE->e_oui_b_cfg) // 629C
#define e_oui_c_cfg                                            INREG32(&XEPON_MAC_BASE->e_oui_c_cfg) // 62A0
#define e_dyinggsp_cfg                                         INREG32(&XEPON_MAC_BASE->e_dyinggsp_cfg) // 62AC
#define e_dyinggsp_w1                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w1) // 62B0
#define e_dyinggsp_w2                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w2) // 62B4
#define e_dyinggsp_w3                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w3) // 62B8
#define e_dyinggsp_w4                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w4) // 62BC
#define e_dyinggsp_w5                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w5) // 62C0
#define e_dyinggsp_w6                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w6) // 62C4
#define e_dyinggsp_w7                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w7) // 62C8
#define e_dyinggsp_w8                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w8) // 62CC
#define e_dyinggsp_w9                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w9) // 62D0
#define e_dyinggsp_w10                                         INREG32(&XEPON_MAC_BASE->e_dyinggsp_w10) // 62D4
#define e_dyinggsp_w11                                         INREG32(&XEPON_MAC_BASE->e_dyinggsp_w11) // 62D8
#define e_dyinggsp_w12                                         INREG32(&XEPON_MAC_BASE->e_dyinggsp_w12) // 62DC
#define e_oam_kpalv_ctrl                                       INREG32(&XEPON_MAC_BASE->e_oam_kpalv_ctrl) // 62E0
#define e_oam_kpalv_w1                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w1) // 62E4
#define e_oam_kpalv_w2                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w2) // 62E8
#define e_oam_kpalv_w3                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w3) // 62EC
#define e_oam_kpalv_w4                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w4) // 62F0
#define e_oam_kpalv_w5                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w5) // 62F4
#define e_oam_kpalv_w6                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w6) // 62F8
#define e_oam_kpalv_w7                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w7) // 62FC
#define e_oam_kpalv_w8                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w8) // 6300
#define e_oam_kpalv_w9                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w9) // 6304
#define e_oam_kpalv_w10                                        INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w10) // 6308
#define e_oam_kpalv_w11                                        INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w11) // 630C
#define e_oam_kpalv_w12                                        INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w12) // 6310
#define e_kpalv_hwen_sts                                       INREG32(&XEPON_MAC_BASE->e_kpalv_hwen_sts) // 6314
#define e_mcgrp_en                                             INREG32(&XEPON_MAC_BASE->e_mcgrp_en) // 6320
#define e_rxuni_mcllid_cfg0                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg0) // 6324
#define e_rxuni_mcllid_cfg1                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg1) // 6328
#define e_rxuni_mcllid_cfg2                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg2) // 632C
#define e_rxuni_mcllid_cfg3                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg3) // 6330
#define e_rxuni_mcllid_cfg4                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg4) // 6334
#define e_rxuni_mcllid_cfg5                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg5) // 6338
#define e_rxuni_mcllid_cfg6                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg6) // 633C
#define e_rxuni_mcllid_cfg7                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg7) // 6340
#define e_crpt_cfg                                             INREG32(&XEPON_MAC_BASE->e_crpt_cfg) // 6350
#define e_cfbsec_cfg                                           INREG32(&XEPON_MAC_BASE->e_cfbsec_cfg) // 6354
#define e_ctrsec_cfg                                           INREG32(&XEPON_MAC_BASE->e_ctrsec_cfg) // 6358
#define e_desec_srcaddr_cfg                                    INREG32(&XEPON_MAC_BASE->e_desec_srcaddr_cfg) // 6360
#define e_desec_srcaddr_val                                    INREG32(&XEPON_MAC_BASE->e_desec_srcaddr_val) // 6364
#define e_macsdec_cfg                                          INREG32(&XEPON_MAC_BASE->e_macsdec_cfg) // 6370
#define e_macsenc_cfg                                          INREG32(&XEPON_MAC_BASE->e_macsenc_cfg) // 6374
#define e_macsenc_pnini_cfg                                    INREG32(&XEPON_MAC_BASE->e_macsenc_pnini_cfg) // 6378
#define e_macsenc_pn_cfg                                       INREG32(&XEPON_MAC_BASE->e_macsenc_pn_cfg) // 6380
#define e_macsenc_pn_val                                       INREG32(&XEPON_MAC_BASE->e_macsenc_pn_val) // 6384
#define e_olt_macaddr_h                                        INREG32(&XEPON_MAC_BASE->e_olt_macaddr_h) // 6390
#define e_olt_macaddr_l                                        INREG32(&XEPON_MAC_BASE->e_olt_macaddr_l) // 6394
#define e_snf_cfg                                              INREG32(&XEPON_MAC_BASE->e_snf_cfg) // 6400
#define e_snf_sp_tag                                           INREG32(&XEPON_MAC_BASE->e_snf_sp_tag) // 6404
#define e_snf_dah                                              INREG32(&XEPON_MAC_BASE->e_snf_dah) // 6408
#define e_snf_dal                                              INREG32(&XEPON_MAC_BASE->e_snf_dal) // 640C
#define e_snf_sal                                              INREG32(&XEPON_MAC_BASE->e_snf_sal) // 6410
#define e_snf_etype                                            INREG32(&XEPON_MAC_BASE->e_snf_etype) // 6414
#define e_rdmdly_cfg                                           INREG32(&XEPON_MAC_BASE->e_rdmdly_cfg) // 6430
#define e_rdmdly_stat                                          INREG32(&XEPON_MAC_BASE->e_rdmdly_stat) // 6434
#define e_dbg_prb_sel                                          INREG32(&XEPON_MAC_BASE->e_dbg_prb_sel) // 6440
#define e_dbg_prb_h32                                          INREG32(&XEPON_MAC_BASE->e_dbg_prb_h32) // 6444
#define e_dbg_prb_l32                                          INREG32(&XEPON_MAC_BASE->e_dbg_prb_l32) // 6448
#define e_utili_cfg                                            INREG32(&XEPON_MAC_BASE->e_utili_cfg) // 644C
#define e_utili_check                                          INREG32(&XEPON_MAC_BASE->e_utili_check) // 6450
#define total_gnt_sizeh                                        INREG32(&XEPON_MAC_BASE->total_gnt_sizeh) // 6454
#define total_gnt_sizel                                        INREG32(&XEPON_MAC_BASE->total_gnt_sizel) // 6458
#define total_pkt_cnt                                          INREG32(&XEPON_MAC_BASE->total_pkt_cnt) // 645C
#define total_pkt_size_l                                       INREG32(&XEPON_MAC_BASE->total_pkt_size_l) // 6460
#define total_pkt_size_h                                       INREG32(&XEPON_MAC_BASE->total_pkt_size_h) // 6464
#define total_u10g_nouse_gntlen                                INREG32(&XEPON_MAC_BASE->total_u10g_nouse_gntlen) // 6468
#define total_gnt_cnt                                          INREG32(&XEPON_MAC_BASE->total_gnt_cnt) // 646C
#define total_tx_cyc_h                                         INREG32(&XEPON_MAC_BASE->total_tx_cyc_h) // 6470
#define total_tx_cyc_l                                         INREG32(&XEPON_MAC_BASE->total_tx_cyc_l) // 6474
#define e_sts_dmy6                                             INREG32(&XEPON_MAC_BASE->e_sts_dmy6) // 6478
#define e_sts_dmy7                                             INREG32(&XEPON_MAC_BASE->e_sts_dmy7) // 647C
#define e_sts_dmy8                                             INREG32(&XEPON_MAC_BASE->e_sts_dmy8) // 6480
#define e_sts_dmy9                                             INREG32(&XEPON_MAC_BASE->e_sts_dmy9) // 6484
#define e_sts_dmy10                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy10) // 6488
#define e_sts_dmy26                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy26) // 648C
#define e_sts_dmy27                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy27) // 6490
#define e_sts_dmy28                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy28) // 6494
#define e_sts_dmy29                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy29) // 6498
#define e_sram_pd                                              INREG32(&XEPON_MAC_BASE->e_sram_pd) // 649C
#define e_mbist_delsel_cfg0                                    INREG32(&XEPON_MAC_BASE->e_mbist_delsel_cfg0) // 64A0
#define e_mbist_delsel_cfg1                                    INREG32(&XEPON_MAC_BASE->e_mbist_delsel_cfg1) // 64A4
#define e_mbist_delsel_cfg2                                    INREG32(&XEPON_MAC_BASE->e_mbist_delsel_cfg2) // 64A8
#define e_mbist_delsel_cfg3                                    INREG32(&XEPON_MAC_BASE->e_mbist_delsel_cfg3) // 64AC
#define e_mbist_done_sts0                                      INREG32(&XEPON_MAC_BASE->e_mbist_done_sts0) // 64B0
#define e_mbist_fail_sts0                                      INREG32(&XEPON_MAC_BASE->e_mbist_fail_sts0) // 64B4
#define e_mbist_fail_sts1                                      INREG32(&XEPON_MAC_BASE->e_mbist_fail_sts1) // 64B8
#define rx_sld_sts                                             INREG32(&XEPON_MAC_BASE->rx_sld_sts) // 64D0
#define e_glue_cfg                                             INREG32(&XEPON_MAC_BASE->e_glue_cfg) // 64E0
#define e_gntreq_tmout                                         INREG32(&XEPON_MAC_BASE->e_gntreq_tmout) // 64E4
#define e_fpga_gendef                                          INREG32(&XEPON_MAC_BASE->e_fpga_gendef) // 64F0
#define e_fpga_genver                                          INREG32(&XEPON_MAC_BASE->e_fpga_genver) // 64F4
#define e_cnt_clr                                              INREG32(&XEPON_MAC_BASE->e_cnt_clr) // 6500
#define e_rxmbi_eth_cnt                                        INREG32(&XEPON_MAC_BASE->e_rxmbi_eth_cnt) // 6510
#define e_rxmpi_eth_cnt                                        INREG32(&XEPON_MAC_BASE->e_rxmpi_eth_cnt) // 6514
#define e_txmbi_eth_cnt                                        INREG32(&XEPON_MAC_BASE->e_txmbi_eth_cnt) // 6518
#define e_txmpi_eth_cnt                                        INREG32(&XEPON_MAC_BASE->e_txmpi_eth_cnt) // 651C
#define e_oam_stat                                             INREG32(&XEPON_MAC_BASE->e_oam_stat) // 6520
#define e_mpcp_stat                                            INREG32(&XEPON_MAC_BASE->e_mpcp_stat) // 6524
#define e_mpcp_rgst_stat                                       INREG32(&XEPON_MAC_BASE->e_mpcp_rgst_stat) // 6528
#define e_rxmbi_bytecnt_h                                      INREG32(&XEPON_MAC_BASE->e_rxmbi_bytecnt_h) // 6580
#define e_rxmbi_bytecnt_l                                      INREG32(&XEPON_MAC_BASE->e_rxmbi_bytecnt_l) // 6584
#define e_rxmbi_snf_cnt                                        INREG32(&XEPON_MAC_BASE->e_rxmbi_snf_cnt) // 6588
#define e_rxmpi_uc_cnt                                         INREG32(&XEPON_MAC_BASE->e_rxmpi_uc_cnt) // 65A0
#define e_rxmpi_bc_cnt                                         INREG32(&XEPON_MAC_BASE->e_rxmpi_bc_cnt) // 65A4
#define e_rxmpi_mc_cnt                                         INREG32(&XEPON_MAC_BASE->e_rxmpi_mc_cnt) // 65A8
#define e_rxmpi_oam_cnt                                        INREG32(&XEPON_MAC_BASE->e_rxmpi_oam_cnt) // 65AC
#define e_rxmpi_mpcp_cnt                                       INREG32(&XEPON_MAC_BASE->e_rxmpi_mpcp_cnt) // 65B0
#define e_rxmpi_gate_cnt                                       INREG32(&XEPON_MAC_BASE->e_rxmpi_gate_cnt) // 65B4
#define e_rxmpi_nrlgate_cnt                                    INREG32(&XEPON_MAC_BASE->e_rxmpi_nrlgate_cnt) // 65B8
#define e_txmbi_uceth_cnt                                      INREG32(&XEPON_MAC_BASE->e_txmbi_uceth_cnt) // 65D0
#define e_txmbi_mceth_cnt                                      INREG32(&XEPON_MAC_BASE->e_txmbi_mceth_cnt) // 65D4
#define e_tx_dygasp_cnt                                        INREG32(&XEPON_MAC_BASE->e_tx_dygasp_cnt) // 65D8
#define e_tx_rpt_cnt                                           INREG32(&XEPON_MAC_BASE->e_tx_rpt_cnt) // 65DC
#define e_kpoam_stat                                           INREG32(&XEPON_MAC_BASE->e_kpoam_stat) // 65E0
#define e_txmpi_mpcp_cnt                                       INREG32(&XEPON_MAC_BASE->e_txmpi_mpcp_cnt) // 65E4
#define e_txmpi_uceth_cnt                                      INREG32(&XEPON_MAC_BASE->e_txmpi_uceth_cnt) // 65E8
#define e_txmpi_mceth_cnt                                      INREG32(&XEPON_MAC_BASE->e_txmpi_mceth_cnt) // 65EC
#define e_rxadv_cnt                                            INREG32(&XEPON_MAC_BASE->e_rxadv_cnt) // 6600
#define e_rxmpi_churn_cnt                                      INREG32(&XEPON_MAC_BASE->e_rxmpi_churn_cnt) // 6604
#define e_rxmpi_churn_info                                     INREG32(&XEPON_MAC_BASE->e_rxmpi_churn_info) // 6608
#define e_desec_ok_cnt                                         INREG32(&XEPON_MAC_BASE->e_desec_ok_cnt) // 6610
#define e_rxmpi_p_cnt                                          INREG32(&XEPON_MAC_BASE->e_rxmpi_p_cnt) // 6614
#define e_rxmpi_crc8err_cnt                                    INREG32(&XEPON_MAC_BASE->e_rxmpi_crc8err_cnt) // 6618
#define e_desec_rxdrop_cnt                                     INREG32(&XEPON_MAC_BASE->e_desec_rxdrop_cnt) // 661C
#define e_desec_rxerr_cnt                                      INREG32(&XEPON_MAC_BASE->e_desec_rxerr_cnt) // 6620
#define e_desec_ciphdrop_cnt                                   INREG32(&XEPON_MAC_BASE->e_desec_ciphdrop_cnt) // 6624
#define e_desec_aeswdrop_cnt                                   INREG32(&XEPON_MAC_BASE->e_desec_aeswdrop_cnt) // 6628
#define e_desec_aeswdrop_cnt2                                  INREG32(&XEPON_MAC_BASE->e_desec_aeswdrop_cnt2) // 662C
#define e_rxmpi_drop_cnt                                       INREG32(&XEPON_MAC_BASE->e_rxmpi_drop_cnt) // 6630
#define e_rxmbi_drop_cnt                                       INREG32(&XEPON_MAC_BASE->e_rxmbi_drop_cnt) // 6640
#define e_rxmbi_drop_cnt2                                      INREG32(&XEPON_MAC_BASE->e_rxmbi_drop_cnt2) // 6644
#define e_txmbi_err_cnt                                        INREG32(&XEPON_MAC_BASE->e_txmbi_err_cnt) // 6680
#define e_sts_dmy11                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy11) // 6684
#define e_sts_dmy12                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy12) // 6688
#define e_sts_dmy13                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy13) // 668C
#define e_sts_dmy14                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy14) // 6690
#define e_sts_dmy15                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy15) // 6694
#define e_sts_dmy16                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy16) // 6698
#define e_sts_dmy17                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy17) // 669C
#define e_sts_dmy18                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy18) // 66A0
#define e_sts_dmy19                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy19) // 66A4
#define e_sts_dmy20                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy20) // 66A8
#define e_sts_dmy21                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy21) // 66AC
#define e_sts_dmy22                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy22) // 66B0
#define e_sts_dmy23                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy23) // 66B4
#define e_sts_dmy24                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy24) // 66B8
#define e_sts_dmy25                                            INREG32(&XEPON_MAC_BASE->e_sts_dmy25) // 66BC
#define e_gnt_type_stat                                        INREG32(&XEPON_MAC_BASE->e_gnt_type_stat) // 66C0
#define e_gnt_pending_stat                                     INREG32(&XEPON_MAC_BASE->e_gnt_pending_stat) // 66C4
#define e_gnt_length_stat                                      INREG32(&XEPON_MAC_BASE->e_gnt_length_stat) // 66C8
#define e_local_time                                           INREG32(&XEPON_MAC_BASE->e_local_time) // 66CC
#define e_time_drft_stat                                       INREG32(&XEPON_MAC_BASE->e_time_drft_stat) // 66D0
#define e_tx_timedrift_stat                                    INREG32(&XEPON_MAC_BASE->e_tx_timedrift_stat) // 66D4
#define e_rxfifo_depth_stat                                    INREG32(&XEPON_MAC_BASE->e_rxfifo_depth_stat) // 66D8
#define e_sts_dmy0                                             INREG32(&XEPON_MAC_BASE->e_sts_dmy0) // 66DC
#define e_sts_dmy1                                             INREG32(&XEPON_MAC_BASE->e_sts_dmy1) // 66E0
#define e_sts_dmy2                                             INREG32(&XEPON_MAC_BASE->e_sts_dmy2) // 66E4
#define e_sts_dmy3                                             INREG32(&XEPON_MAC_BASE->e_sts_dmy3) // 66E8
#define e_sts_dmy4                                             INREG32(&XEPON_MAC_BASE->e_sts_dmy4) // 66EC
#define e_sts_dmy5                                             INREG32(&XEPON_MAC_BASE->e_sts_dmy5) // 66F0
#define e_txmbi_bufwrap_wrcnt                                  INREG32(&XEPON_MAC_BASE->e_txmbi_bufwrap_wrcnt) // 66F4
#define e_txmbi_bufwrap_rdcnt                                  INREG32(&XEPON_MAC_BASE->e_txmbi_bufwrap_rdcnt) // 66F8
#define e_txmbi_bufwrap_rdcnt2                                 INREG32(&XEPON_MAC_BASE->e_txmbi_bufwrap_rdcnt2) // 66FC

#endif


#define e_glb_cfg_FLD_txoam_favor                              REG_FLD(1, 25)
#define e_glb_cfg_FLD_burst_en_dly                             REG_FLD(1, 24)
#define e_glb_cfg_FLD_discv_burst_en                           REG_FLD(1, 23)
#define e_glb_cfg_FLD_mpcp_fwd                                 REG_FLD(1, 22)
#define e_glb_cfg_FLD_bcst_llid_m1_drop                        REG_FLD(1, 21)
#define e_glb_cfg_FLD_bcst_llid_m0_drop                        REG_FLD(1, 20)
#define e_glb_cfg_FLD_mcst_llid_drop                           REG_FLD(1, 19)
#define e_glb_cfg_FLD_allucst_llid_fwd                         REG_FLD(1, 18)
#define e_glb_cfg_FLD_fcs_err_fwd                              REG_FLD(1, 17)
#define e_glb_cfg_FLD_llid_crc8_err_fwd                        REG_FLD(1, 16)
#define e_glb_cfg_FLD_rxmpi_stop                               REG_FLD(1, 13)
#define e_glb_cfg_FLD_txmpi_stop                               REG_FLD(1, 12)
#define e_glb_cfg_FLD_phy_pwr_down                             REG_FLD(1, 11)
#define e_glb_cfg_FLD_rx_nml_gate_fwd                          REG_FLD(1, 10)
#define e_glb_cfg_FLD_rxmbi_stop                               REG_FLD(1, 9)
#define e_glb_cfg_FLD_txmbi_stop                               REG_FLD(1, 8)
#define e_glb_cfg_FLD_chk_all_gnt_mode                         REG_FLD(1, 7)
#define e_glb_cfg_FLD_mpcp_done_set                            REG_FLD(1, 4)
#define e_glb_cfg_FLD_epon_oam_cal_in_eth                      REG_FLD(1, 3)
#define e_glb_cfg_FLD_rpt_txpri_ctrl                           REG_FLD(1, 1)

#define e_glb_cfg2_FLD_llidnum_sel                             REG_FLD(1, 31)
#define e_glb_cfg2_FLD_rxdv_tmout_en                           REG_FLD(1, 23)
#define e_glb_cfg2_FLD_txmbi_nack_enden                        REG_FLD(1, 22)
#define e_glb_cfg2_FLD_loctime_mtd                             REG_FLD(1, 21)
#define e_glb_cfg2_FLD_rgreq_lsrtime_mask                      REG_FLD(1, 20)
#define e_glb_cfg2_FLD_crc_cal_in_bytecnt                      REG_FLD(1, 17)
#define e_glb_cfg2_FLD_u10g_txmode                             REG_FLD(1, 16)
#define e_glb_cfg2_FLD_eth_cal_in_bytecnt                      REG_FLD(1, 15)
#define e_glb_cfg2_FLD_snf_cal_in_bytecnt                      REG_FLD(1, 14)
#define e_glb_cfg2_FLD_oam_cal_in_bytecnt                      REG_FLD(1, 13)
#define e_glb_cfg2_FLD_dscvgate_filt                           REG_FLD(1, 12)
#define e_glb_cfg2_FLD_gntlen_stat_widscv                      REG_FLD(1, 6)
#define e_glb_cfg2_FLD_tdrift_loctupd_dis                      REG_FLD(1, 5)
#define e_glb_cfg2_FLD_dscvgate_infochk_dis                    REG_FLD(1, 0)

#define e_glb_sts1_FLD_xepon_type                              REG_FLD(2, 30)
#define e_glb_sts1_FLD_rxmpi_stop_sts                          REG_FLD(1, 29)
#define e_glb_sts1_FLD_txmpi_stop_sts                          REG_FLD(1, 28)
#define e_glb_sts1_FLD_rxmbi_stop_sts                          REG_FLD(1, 27)
#define e_glb_sts1_FLD_txmbi_stop_sts                          REG_FLD(1, 26)
#define e_glb_sts1_FLD_xepon_glb_cfg_sts                       REG_FLD(26, 0)

#define e_glb_sts2_FLD_xepon_glb_cfg2_sts                      REG_FLD(32, 0)

#define e_int_status_FLD_us_keychg_int                         REG_FLD(1, 29)
#define e_int_status_FLD_ds_keymis_int                         REG_FLD(1, 28)
#define e_int_status_FLD_ds_keychg_int                         REG_FLD(1, 27)
#define e_int_status_FLD_sniff_fifo_ovrun_int                  REG_FLD(1, 26)
#define e_int_status_FLD_reg_ack_done_int                      REG_FLD(1, 25)
#define e_int_status_FLD_reg_req_done_int                      REG_FLD(1, 24)
#define e_int_status_FLD_reorder1_gnt_int                      REG_FLD(1, 23)
#define e_int_status_FLD_b2b_gnt_int                           REG_FLD(1, 22)
#define e_int_status_FLD_hidn_gnt_int                          REG_FLD(1, 21)
#define e_int_status_FLD_ps_early_wakeup_int                   REG_FLD(1, 20)
#define e_int_status_FLD_rx_sleep_allow_int                    REG_FLD(1, 19)
#define e_int_status_FLD_ps_wakeup_int                         REG_FLD(1, 18)
#define e_int_status_FLD_ps_sleep_int                          REG_FLD(1, 17)
#define e_int_status_FLD_txfifo_udrun_int                      REG_FLD(1, 16)
#define e_int_status_FLD_rpt_overintvl_int                     REG_FLD(1, 15)
#define e_int_status_FLD_mpcp_timeout_int                      REG_FLD(1, 14)
#define e_int_status_FLD_timedrft_int                          REG_FLD(1, 13)
#define e_int_status_FLD_tod_1pps_int                          REG_FLD(1, 12)
#define e_int_status_FLD_tod_updt_int                          REG_FLD(1, 11)
#define e_int_status_FLD_ptp_msg_tx_int                        REG_FLD(1, 10)
#define e_int_status_FLD_gnt_buf_ovrrun_int                    REG_FLD(1, 9)
#define e_int_status_FLD_llid7_rcv_rgst_int                    REG_FLD(1, 8)
#define e_int_status_FLD_llid6_rcv_rgst_int                    REG_FLD(1, 7)
#define e_int_status_FLD_llid5_rcv_rgst_int                    REG_FLD(1, 6)
#define e_int_status_FLD_llid4_rcv_rgst_int                    REG_FLD(1, 5)
#define e_int_status_FLD_llid3_rcv_rgst_int                    REG_FLD(1, 4)
#define e_int_status_FLD_llid2_rcv_rgst_int                    REG_FLD(1, 3)
#define e_int_status_FLD_llid1_rcv_rgst_int                    REG_FLD(1, 2)
#define e_int_status_FLD_llid0_rcv_rgst_int                    REG_FLD(1, 1)
#define e_int_status_FLD_rcv_dscvry_gate_int                   REG_FLD(1, 0)

#define e_int_en_FLD_us_keychg_en                              REG_FLD(1, 29)
#define e_int_en_FLD_ds_keymis_en                              REG_FLD(1, 28)
#define e_int_en_FLD_ds_keychg_en                              REG_FLD(1, 27)
#define e_int_en_FLD_sniff_fifo_ovrun_en                       REG_FLD(1, 26)
#define e_int_en_FLD_reg_ack_done_en                           REG_FLD(1, 25)
#define e_int_en_FLD_reg_req_done_en                           REG_FLD(1, 24)
#define e_int_en_FLD_reorder1_gnt_en                           REG_FLD(1, 23)
#define e_int_en_FLD_b2b_gnt_en                                REG_FLD(1, 22)
#define e_int_en_FLD_hidn_gnt_en                               REG_FLD(1, 21)
#define e_int_en_FLD_ps_early_wakeup_en                        REG_FLD(1, 20)
#define e_int_en_FLD_rx_sleep_allow_en                         REG_FLD(1, 19)
#define e_int_en_FLD_ps_wakeup_en                              REG_FLD(1, 18)
#define e_int_en_FLD_ps_sleep_en                               REG_FLD(1, 17)
#define e_int_en_FLD_txfifo_udrun_en                           REG_FLD(1, 16)
#define e_int_en_FLD_rpt_overintvl_en                          REG_FLD(1, 15)
#define e_int_en_FLD_mpcp_timeout_en                           REG_FLD(1, 14)
#define e_int_en_FLD_timedrft_en                               REG_FLD(1, 13)
#define e_int_en_FLD_tod_1pps_en                               REG_FLD(1, 12)
#define e_int_en_FLD_tod_updt_en                               REG_FLD(1, 11)
#define e_int_en_FLD_ptp_msg_tx_en                             REG_FLD(1, 10)
#define e_int_en_FLD_gnt_buf_ovrrun_en                         REG_FLD(1, 9)
#define e_int_en_FLD_llid7_rcv_rgst_en                         REG_FLD(1, 8)
#define e_int_en_FLD_llid6_rcv_rgst_en                         REG_FLD(1, 7)
#define e_int_en_FLD_llid5_rcv_rgst_en                         REG_FLD(1, 6)
#define e_int_en_FLD_llid4_rcv_rgst_en                         REG_FLD(1, 5)
#define e_int_en_FLD_llid3_rcv_rgst_en                         REG_FLD(1, 4)
#define e_int_en_FLD_llid2_rcv_rgst_en                         REG_FLD(1, 3)
#define e_int_en_FLD_llid1_rcv_rgst_en                         REG_FLD(1, 2)
#define e_int_en_FLD_llid0_rcv_rgst_en                         REG_FLD(1, 1)
#define e_int_en_FLD_rcv_dscvry_gate_en                        REG_FLD(1, 0)

#define e_int_sts2_FLD_tx_latestart_int                        REG_FLD(1, 12)
#define e_int_sts2_FLD_rcv_ifchk_err_int                       REG_FLD(1, 11)
#define e_int_sts2_FLD_rx_rgst_exc_int                         REG_FLD(1, 10)
#define e_int_sts2_FLD_rcv_len_err_int                         REG_FLD(1, 9)
#define e_int_sts2_FLD_schfch_nopkt_int                        REG_FLD(1, 8)
#define e_int_sts2_FLD_rxfifo_ovrun_int                        REG_FLD(1, 7)
#define e_int_sts2_FLD_txmpi_pldenneq_int                      REG_FLD(1, 6)
#define e_int_sts2_FLD_schfch_timeo_int                        REG_FLD(1, 5)
#define e_int_sts2_FLD_schgnt_inv_int                          REG_FLD(1, 4)
#define e_int_sts2_FLD_rcvgnt_infofail_int                     REG_FLD(1, 3)
#define e_int_sts2_FLD_rcv_eofdrop_int                         REG_FLD(1, 2)
#define e_int_sts2_FLD_rcv_crcerr_int                          REG_FLD(1, 1)
#define e_int_sts2_FLD_rcv_excgate_int                         REG_FLD(1, 0)

#define e_int_en2_FLD_tx_latestart_en                          REG_FLD(1, 12)
#define e_int_en2_FLD_rcv_ifchk_err_en                         REG_FLD(1, 11)
#define e_int_en2_FLD_rx_rgst_exc_en                           REG_FLD(1, 10)
#define e_int_en2_FLD_rcv_len_err_en                           REG_FLD(1, 9)
#define e_int_en2_FLD_schfch_nopkt_en                          REG_FLD(1, 8)
#define e_int_en2_FLD_rxfifo_ovrun_en                          REG_FLD(1, 7)
#define e_int_en2_FLD_txmpi_pldenneq_en                        REG_FLD(1, 6)
#define e_int_en2_FLD_schfch_timeo_en                          REG_FLD(1, 5)
#define e_int_en2_FLD_schgnt_inv_en                            REG_FLD(1, 4)
#define e_int_en2_FLD_rcvgnt_infofail_en                       REG_FLD(1, 3)
#define e_int_en2_FLD_rcv_eofdrop_en                           REG_FLD(1, 2)
#define e_int_en2_FLD_rcv_crcerr_en                            REG_FLD(1, 1)
#define e_int_en2_FLD_rcv_excgate_en                           REG_FLD(1, 0)

#define e_int_sts3_FLD_llid31_rcv_rgst_int                     REG_FLD(1, 23)
#define e_int_sts3_FLD_llid30_rcv_rgst_int                     REG_FLD(1, 22)
#define e_int_sts3_FLD_llid29_rcv_rgst_int                     REG_FLD(1, 21)
#define e_int_sts3_FLD_llid28_rcv_rgst_int                     REG_FLD(1, 20)
#define e_int_sts3_FLD_llid27_rcv_rgst_int                     REG_FLD(1, 19)
#define e_int_sts3_FLD_llid26_rcv_rgst_int                     REG_FLD(1, 18)
#define e_int_sts3_FLD_llid25_rcv_rgst_int                     REG_FLD(1, 17)
#define e_int_sts3_FLD_llid24_rcv_rgst_int                     REG_FLD(1, 16)
#define e_int_sts3_FLD_llid23_rcv_rgst_int                     REG_FLD(1, 15)
#define e_int_sts3_FLD_llid22_rcv_rgst_int                     REG_FLD(1, 14)
#define e_int_sts3_FLD_llid21_rcv_rgst_int                     REG_FLD(1, 13)
#define e_int_sts3_FLD_llid20_rcv_rgst_int                     REG_FLD(1, 12)
#define e_int_sts3_FLD_llid19_rcv_rgst_int                     REG_FLD(1, 11)
#define e_int_sts3_FLD_llid18_rcv_rgst_int                     REG_FLD(1, 10)
#define e_int_sts3_FLD_llid17_rcv_rgst_int                     REG_FLD(1, 9)
#define e_int_sts3_FLD_llid16_rcv_rgst_int                     REG_FLD(1, 8)
#define e_int_sts3_FLD_llid15_rcv_rgst_int                     REG_FLD(1, 7)
#define e_int_sts3_FLD_llid14_rcv_rgst_int                     REG_FLD(1, 6)
#define e_int_sts3_FLD_llid13_rcv_rgst_int                     REG_FLD(1, 5)
#define e_int_sts3_FLD_llid12_rcv_rgst_int                     REG_FLD(1, 4)
#define e_int_sts3_FLD_llid11_rcv_rgst_int                     REG_FLD(1, 3)
#define e_int_sts3_FLD_llid10_rcv_rgst_int                     REG_FLD(1, 2)
#define e_int_sts3_FLD_llid9_rcv_rgst_int                      REG_FLD(1, 1)
#define e_int_sts3_FLD_llid8_rcv_rgst_int                      REG_FLD(1, 0)

#define e_int_en3_FLD_llid31_rcv_rgst_en                       REG_FLD(1, 23)
#define e_int_en3_FLD_llid30_rcv_rgst_en                       REG_FLD(1, 22)
#define e_int_en3_FLD_llid29_rcv_rgst_en                       REG_FLD(1, 21)
#define e_int_en3_FLD_llid28_rcv_rgst_en                       REG_FLD(1, 20)
#define e_int_en3_FLD_llid27_rcv_rgst_en                       REG_FLD(1, 19)
#define e_int_en3_FLD_llid26_rcv_rgst_en                       REG_FLD(1, 18)
#define e_int_en3_FLD_llid25_rcv_rgst_en                       REG_FLD(1, 17)
#define e_int_en3_FLD_llid24_rcv_rgst_en                       REG_FLD(1, 16)
#define e_int_en3_FLD_llid23_rcv_rgst_en                       REG_FLD(1, 15)
#define e_int_en3_FLD_llid22_rcv_rgst_en                       REG_FLD(1, 14)
#define e_int_en3_FLD_llid21_rcv_rgst_en                       REG_FLD(1, 13)
#define e_int_en3_FLD_llid20_rcv_rgst_en                       REG_FLD(1, 12)
#define e_int_en3_FLD_llid19_rcv_rgst_en                       REG_FLD(1, 11)
#define e_int_en3_FLD_llid18_rcv_rgst_en                       REG_FLD(1, 10)
#define e_int_en3_FLD_llid17_rcv_rgst_en                       REG_FLD(1, 9)
#define e_int_en3_FLD_llid16_rcv_rgst_en                       REG_FLD(1, 8)
#define e_int_en3_FLD_llid15_rcv_rgst_en                       REG_FLD(1, 7)
#define e_int_en3_FLD_llid14_rcv_rgst_en                       REG_FLD(1, 6)
#define e_int_en3_FLD_llid13_rcv_rgst_en                       REG_FLD(1, 5)
#define e_int_en3_FLD_llid12_rcv_rgst_en                       REG_FLD(1, 4)
#define e_int_en3_FLD_llid11_rcv_rgst_en                       REG_FLD(1, 3)
#define e_int_en3_FLD_llid10_rcv_rgst_en                       REG_FLD(1, 2)
#define e_int_en3_FLD_llid9_rcv_rgst_en                        REG_FLD(1, 1)
#define e_int_en3_FLD_llid8_rcv_rgst_en                        REG_FLD(1, 0)

#define e_exc_sts_FLD_encfrm_alierr_hit                        REG_FLD(1, 27)
#define e_exc_sts_FLD_schgnt_tmout_hit                         REG_FLD(1, 26)
#define e_exc_sts_FLD_rx_rgst_dergst_hit                       REG_FLD(1, 25)
#define e_exc_sts_FLD_rx_rgst_nack_hit                         REG_FLD(1, 24)
#define e_exc_sts_FLD_rx_slderr_hit                            REG_FLD(1, 23)
#define e_exc_sts_FLD_rx_nodv_tmo_hit                          REG_FLD(1, 22)
#define e_exc_sts_FLD_rx_noeof_hit                             REG_FLD(1, 21)
#define e_exc_sts_FLD_snf_pktcnt_err_hit                       REG_FLD(1, 20)
#define e_exc_sts_FLD_tx_gntst_nosog_hit                       REG_FLD(1, 19)
#define e_exc_sts_FLD_tx_gntst_fifound_hit                     REG_FLD(1, 18)
#define e_exc_sts_FLD_tx_gntph_fifound_hit                     REG_FLD(1, 17)
#define e_exc_sts_FLD_tx_gntph_exceed_hit                      REG_FLD(1, 16)
#define e_exc_sts_FLD_schgnt_stinv_hit                         REG_FLD(1, 13)
#define e_exc_sts_FLD_schgnt_idxinv_hit                        REG_FLD(1, 12)
#define e_exc_sts_FLD_decrpt_sofeof_col_hit                    REG_FLD(1, 10)
#define e_exc_sts_FLD_rcv_len_long_hit                         REG_FLD(1, 9)
#define e_exc_sts_FLD_rcv_len_short_hit                        REG_FLD(1, 8)
#define e_exc_sts_FLD_rcv_mpcp_crcerr_hit                      REG_FLD(1, 6)
#define e_exc_sts_FLD_rcv_crc32err_hit                         REG_FLD(1, 5)
#define e_exc_sts_FLD_rcv_crc8err_hit                          REG_FLD(1, 4)
#define e_exc_sts_FLD_rcv_demacs_flagexc_hit                   REG_FLD(1, 3)
#define e_exc_sts_FLD_rcv_nrlgate_llidbc_hit                   REG_FLD(1, 2)
#define e_exc_sts_FLD_rcv_dscvgate_filt_hit                    REG_FLD(1, 1)
#define e_exc_sts_FLD_rcv_dscvgate_infomis_hit                 REG_FLD(1, 0)

#define e_rpt_timout_FLD_llid31_rpt_tmo                        REG_FLD(1, 31)
#define e_rpt_timout_FLD_llid30_rpt_tmo                        REG_FLD(1, 30)
#define e_rpt_timout_FLD_llid29_rpt_tmo                        REG_FLD(1, 29)
#define e_rpt_timout_FLD_llid28_rpt_tmo                        REG_FLD(1, 28)
#define e_rpt_timout_FLD_llid27_rpt_tmo                        REG_FLD(1, 27)
#define e_rpt_timout_FLD_llid26_rpt_tmo                        REG_FLD(1, 26)
#define e_rpt_timout_FLD_llid25_rpt_tmo                        REG_FLD(1, 25)
#define e_rpt_timout_FLD_llid24_rpt_tmo                        REG_FLD(1, 24)
#define e_rpt_timout_FLD_llid23_rpt_tmo                        REG_FLD(1, 23)
#define e_rpt_timout_FLD_llid22_rpt_tmo                        REG_FLD(1, 22)
#define e_rpt_timout_FLD_llid21_rpt_tmo                        REG_FLD(1, 21)
#define e_rpt_timout_FLD_llid20_rpt_tmo                        REG_FLD(1, 20)
#define e_rpt_timout_FLD_llid19_rpt_tmo                        REG_FLD(1, 19)
#define e_rpt_timout_FLD_llid18_rpt_tmo                        REG_FLD(1, 18)
#define e_rpt_timout_FLD_llid17_rpt_tmo                        REG_FLD(1, 17)
#define e_rpt_timout_FLD_llid16_rpt_tmo                        REG_FLD(1, 16)
#define e_rpt_timout_FLD_llid15_rpt_tmo                        REG_FLD(1, 15)
#define e_rpt_timout_FLD_llid14_rpt_tmo                        REG_FLD(1, 14)
#define e_rpt_timout_FLD_llid13_rpt_tmo                        REG_FLD(1, 13)
#define e_rpt_timout_FLD_llid12_rpt_tmo                        REG_FLD(1, 12)
#define e_rpt_timout_FLD_llid11_rpt_tmo                        REG_FLD(1, 11)
#define e_rpt_timout_FLD_llid10_rpt_tmo                        REG_FLD(1, 10)
#define e_rpt_timout_FLD_llid9_rpt_tmo                         REG_FLD(1, 9)
#define e_rpt_timout_FLD_llid8_rpt_tmo                         REG_FLD(1, 8)
#define e_rpt_timout_FLD_llid7_rpt_tmo                         REG_FLD(1, 7)
#define e_rpt_timout_FLD_llid6_rpt_tmo                         REG_FLD(1, 6)
#define e_rpt_timout_FLD_llid5_rpt_tmo                         REG_FLD(1, 5)
#define e_rpt_timout_FLD_llid4_rpt_tmo                         REG_FLD(1, 4)
#define e_rpt_timout_FLD_llid3_rpt_tmo                         REG_FLD(1, 3)
#define e_rpt_timout_FLD_llid2_rpt_tmo                         REG_FLD(1, 2)
#define e_rpt_timout_FLD_llid1_rpt_tmo                         REG_FLD(1, 1)
#define e_rpt_timout_FLD_llid0_rpt_tmo                         REG_FLD(1, 0)

#define e_mpcp_timout_FLD_llid31_mpcp_tmo                      REG_FLD(1, 31)
#define e_mpcp_timout_FLD_llid30_mpcp_tmo                      REG_FLD(1, 30)
#define e_mpcp_timout_FLD_llid29_mpcp_tmo                      REG_FLD(1, 29)
#define e_mpcp_timout_FLD_llid28_mpcp_tmo                      REG_FLD(1, 28)
#define e_mpcp_timout_FLD_llid27_mpcp_tmo                      REG_FLD(1, 27)
#define e_mpcp_timout_FLD_llid26_mpcp_tmo                      REG_FLD(1, 26)
#define e_mpcp_timout_FLD_llid25_mpcp_tmo                      REG_FLD(1, 25)
#define e_mpcp_timout_FLD_llid24_mpcp_tmo                      REG_FLD(1, 24)
#define e_mpcp_timout_FLD_llid23_mpcp_tmo                      REG_FLD(1, 23)
#define e_mpcp_timout_FLD_llid22_mpcp_tmo                      REG_FLD(1, 22)
#define e_mpcp_timout_FLD_llid21_mpcp_tmo                      REG_FLD(1, 21)
#define e_mpcp_timout_FLD_llid20_mpcp_tmo                      REG_FLD(1, 20)
#define e_mpcp_timout_FLD_llid19_mpcp_tmo                      REG_FLD(1, 19)
#define e_mpcp_timout_FLD_llid18_mpcp_tmo                      REG_FLD(1, 18)
#define e_mpcp_timout_FLD_llid17_mpcp_tmo                      REG_FLD(1, 17)
#define e_mpcp_timout_FLD_llid16_mpcp_tmo                      REG_FLD(1, 16)
#define e_mpcp_timout_FLD_llid15_mpcp_tmo                      REG_FLD(1, 15)
#define e_mpcp_timout_FLD_llid14_mpcp_tmo                      REG_FLD(1, 14)
#define e_mpcp_timout_FLD_llid13_mpcp_tmo                      REG_FLD(1, 13)
#define e_mpcp_timout_FLD_llid12_mpcp_tmo                      REG_FLD(1, 12)
#define e_mpcp_timout_FLD_llid11_mpcp_tmo                      REG_FLD(1, 11)
#define e_mpcp_timout_FLD_llid10_mpcp_tmo                      REG_FLD(1, 10)
#define e_mpcp_timout_FLD_llid9_mpcp_tmo                       REG_FLD(1, 9)
#define e_mpcp_timout_FLD_llid8_mpcp_tmo                       REG_FLD(1, 8)
#define e_mpcp_timout_FLD_llid7_mpcp_tmo                       REG_FLD(1, 7)
#define e_mpcp_timout_FLD_llid6_mpcp_tmo                       REG_FLD(1, 6)
#define e_mpcp_timout_FLD_llid5_mpcp_tmo                       REG_FLD(1, 5)
#define e_mpcp_timout_FLD_llid4_mpcp_tmo                       REG_FLD(1, 4)
#define e_mpcp_timout_FLD_llid3_mpcp_tmo                       REG_FLD(1, 3)
#define e_mpcp_timout_FLD_llid2_mpcp_tmo                       REG_FLD(1, 2)
#define e_mpcp_timout_FLD_llid1_mpcp_tmo                       REG_FLD(1, 1)
#define e_mpcp_timout_FLD_llid0_mpcp_tmo                       REG_FLD(1, 0)

#define e_ds_keychg_FLD_llid31_ds_keychg                       REG_FLD(1, 31)
#define e_ds_keychg_FLD_llid30_ds_keychg                       REG_FLD(1, 30)
#define e_ds_keychg_FLD_llid29_ds_keychg                       REG_FLD(1, 29)
#define e_ds_keychg_FLD_llid28_ds_keychg                       REG_FLD(1, 28)
#define e_ds_keychg_FLD_llid27_ds_keychg                       REG_FLD(1, 27)
#define e_ds_keychg_FLD_llid26_ds_keychg                       REG_FLD(1, 26)
#define e_ds_keychg_FLD_llid25_ds_keychg                       REG_FLD(1, 25)
#define e_ds_keychg_FLD_llid24_ds_keychg                       REG_FLD(1, 24)
#define e_ds_keychg_FLD_llid23_ds_keychg                       REG_FLD(1, 23)
#define e_ds_keychg_FLD_llid22_ds_keychg                       REG_FLD(1, 22)
#define e_ds_keychg_FLD_llid21_ds_keychg                       REG_FLD(1, 21)
#define e_ds_keychg_FLD_llid20_ds_keychg                       REG_FLD(1, 20)
#define e_ds_keychg_FLD_llid19_ds_keychg                       REG_FLD(1, 19)
#define e_ds_keychg_FLD_llid18_ds_keychg                       REG_FLD(1, 18)
#define e_ds_keychg_FLD_llid17_ds_keychg                       REG_FLD(1, 17)
#define e_ds_keychg_FLD_llid16_ds_keychg                       REG_FLD(1, 16)
#define e_ds_keychg_FLD_llid15_ds_keychg                       REG_FLD(1, 15)
#define e_ds_keychg_FLD_llid14_ds_keychg                       REG_FLD(1, 14)
#define e_ds_keychg_FLD_llid13_ds_keychg                       REG_FLD(1, 13)
#define e_ds_keychg_FLD_llid12_ds_keychg                       REG_FLD(1, 12)
#define e_ds_keychg_FLD_llid11_ds_keychg                       REG_FLD(1, 11)
#define e_ds_keychg_FLD_llid10_ds_keychg                       REG_FLD(1, 10)
#define e_ds_keychg_FLD_llid9_ds_keychg                        REG_FLD(1, 9)
#define e_ds_keychg_FLD_llid8_ds_keychg                        REG_FLD(1, 8)
#define e_ds_keychg_FLD_llid7_ds_keychg                        REG_FLD(1, 7)
#define e_ds_keychg_FLD_llid6_ds_keychg                        REG_FLD(1, 6)
#define e_ds_keychg_FLD_llid5_ds_keychg                        REG_FLD(1, 5)
#define e_ds_keychg_FLD_llid4_ds_keychg                        REG_FLD(1, 4)
#define e_ds_keychg_FLD_llid3_ds_keychg                        REG_FLD(1, 3)
#define e_ds_keychg_FLD_llid2_ds_keychg                        REG_FLD(1, 2)
#define e_ds_keychg_FLD_llid1_ds_keychg                        REG_FLD(1, 1)
#define e_ds_keychg_FLD_llid0_ds_keychg                        REG_FLD(1, 0)

#define e_ds_keychg2_FLD_llid51_ds_keychg                      REG_FLD(1, 19)
#define e_ds_keychg2_FLD_llid50_ds_keychg                      REG_FLD(1, 18)
#define e_ds_keychg2_FLD_llid49_ds_keychg                      REG_FLD(1, 17)
#define e_ds_keychg2_FLD_llid48_ds_keychg                      REG_FLD(1, 16)
#define e_ds_keychg2_FLD_llid47_ds_keychg                      REG_FLD(1, 15)
#define e_ds_keychg2_FLD_llid46_ds_keychg                      REG_FLD(1, 14)
#define e_ds_keychg2_FLD_llid45_ds_keychg                      REG_FLD(1, 13)
#define e_ds_keychg2_FLD_llid44_ds_keychg                      REG_FLD(1, 12)
#define e_ds_keychg2_FLD_llid43_ds_keychg                      REG_FLD(1, 11)
#define e_ds_keychg2_FLD_llid42_ds_keychg                      REG_FLD(1, 10)
#define e_ds_keychg2_FLD_llid41_ds_keychg                      REG_FLD(1, 9)
#define e_ds_keychg2_FLD_llid40_ds_keychg                      REG_FLD(1, 8)
#define e_ds_keychg2_FLD_llid39_ds_keychg                      REG_FLD(1, 7)
#define e_ds_keychg2_FLD_llid38_ds_keychg                      REG_FLD(1, 6)
#define e_ds_keychg2_FLD_llid37_ds_keychg                      REG_FLD(1, 5)
#define e_ds_keychg2_FLD_llid36_ds_keychg                      REG_FLD(1, 4)
#define e_ds_keychg2_FLD_llid35_ds_keychg                      REG_FLD(1, 3)
#define e_ds_keychg2_FLD_llid34_ds_keychg                      REG_FLD(1, 2)
#define e_ds_keychg2_FLD_llid33_ds_keychg                      REG_FLD(1, 1)
#define e_ds_keychg2_FLD_llid32_ds_keychg                      REG_FLD(1, 0)

#define e_ds_keymis_FLD_llid31_ds_keymis                       REG_FLD(1, 31)
#define e_ds_keymis_FLD_llid30_ds_keymis                       REG_FLD(1, 30)
#define e_ds_keymis_FLD_llid29_ds_keymis                       REG_FLD(1, 29)
#define e_ds_keymis_FLD_llid28_ds_keymis                       REG_FLD(1, 28)
#define e_ds_keymis_FLD_llid27_ds_keymis                       REG_FLD(1, 27)
#define e_ds_keymis_FLD_llid26_ds_keymis                       REG_FLD(1, 26)
#define e_ds_keymis_FLD_llid25_ds_keymis                       REG_FLD(1, 25)
#define e_ds_keymis_FLD_llid24_ds_keymis                       REG_FLD(1, 24)
#define e_ds_keymis_FLD_llid23_ds_keymis                       REG_FLD(1, 23)
#define e_ds_keymis_FLD_llid22_ds_keymis                       REG_FLD(1, 22)
#define e_ds_keymis_FLD_llid21_ds_keymis                       REG_FLD(1, 21)
#define e_ds_keymis_FLD_llid20_ds_keymis                       REG_FLD(1, 20)
#define e_ds_keymis_FLD_llid19_ds_keymis                       REG_FLD(1, 19)
#define e_ds_keymis_FLD_llid18_ds_keymis                       REG_FLD(1, 18)
#define e_ds_keymis_FLD_llid17_ds_keymis                       REG_FLD(1, 17)
#define e_ds_keymis_FLD_llid16_ds_keymis                       REG_FLD(1, 16)
#define e_ds_keymis_FLD_llid15_ds_keymis                       REG_FLD(1, 15)
#define e_ds_keymis_FLD_llid14_ds_keymis                       REG_FLD(1, 14)
#define e_ds_keymis_FLD_llid13_ds_keymis                       REG_FLD(1, 13)
#define e_ds_keymis_FLD_llid12_ds_keymis                       REG_FLD(1, 12)
#define e_ds_keymis_FLD_llid11_ds_keymis                       REG_FLD(1, 11)
#define e_ds_keymis_FLD_llid10_ds_keymis                       REG_FLD(1, 10)
#define e_ds_keymis_FLD_llid9_ds_keymis                        REG_FLD(1, 9)
#define e_ds_keymis_FLD_llid8_ds_keymis                        REG_FLD(1, 8)
#define e_ds_keymis_FLD_llid7_ds_keymis                        REG_FLD(1, 7)
#define e_ds_keymis_FLD_llid6_ds_keymis                        REG_FLD(1, 6)
#define e_ds_keymis_FLD_llid5_ds_keymis                        REG_FLD(1, 5)
#define e_ds_keymis_FLD_llid4_ds_keymis                        REG_FLD(1, 4)
#define e_ds_keymis_FLD_llid3_ds_keymis                        REG_FLD(1, 3)
#define e_ds_keymis_FLD_llid2_ds_keymis                        REG_FLD(1, 2)
#define e_ds_keymis_FLD_llid1_ds_keymis                        REG_FLD(1, 1)
#define e_ds_keymis_FLD_llid0_ds_keymis                        REG_FLD(1, 0)

#define e_ds_keymis2_FLD_llid51_ds_keymis                      REG_FLD(1, 19)
#define e_ds_keymis2_FLD_llid50_ds_keymis                      REG_FLD(1, 18)
#define e_ds_keymis2_FLD_llid49_ds_keymis                      REG_FLD(1, 17)
#define e_ds_keymis2_FLD_llid48_ds_keymis                      REG_FLD(1, 16)
#define e_ds_keymis2_FLD_llid47_ds_keymis                      REG_FLD(1, 15)
#define e_ds_keymis2_FLD_llid46_ds_keymis                      REG_FLD(1, 14)
#define e_ds_keymis2_FLD_llid45_ds_keymis                      REG_FLD(1, 13)
#define e_ds_keymis2_FLD_llid44_ds_keymis                      REG_FLD(1, 12)
#define e_ds_keymis2_FLD_llid43_ds_keymis                      REG_FLD(1, 11)
#define e_ds_keymis2_FLD_llid42_ds_keymis                      REG_FLD(1, 10)
#define e_ds_keymis2_FLD_llid41_ds_keymis                      REG_FLD(1, 9)
#define e_ds_keymis2_FLD_llid40_ds_keymis                      REG_FLD(1, 8)
#define e_ds_keymis2_FLD_llid39_ds_keymis                      REG_FLD(1, 7)
#define e_ds_keymis2_FLD_llid38_ds_keymis                      REG_FLD(1, 6)
#define e_ds_keymis2_FLD_llid37_ds_keymis                      REG_FLD(1, 5)
#define e_ds_keymis2_FLD_llid36_ds_keymis                      REG_FLD(1, 4)
#define e_ds_keymis2_FLD_llid35_ds_keymis                      REG_FLD(1, 3)
#define e_ds_keymis2_FLD_llid34_ds_keymis                      REG_FLD(1, 2)
#define e_ds_keymis2_FLD_llid33_ds_keymis                      REG_FLD(1, 1)
#define e_ds_keymis2_FLD_llid32_ds_keymis                      REG_FLD(1, 0)

#define e_us_keychg_FLD_llid31_us_keychg                       REG_FLD(1, 31)
#define e_us_keychg_FLD_llid30_us_keychg                       REG_FLD(1, 30)
#define e_us_keychg_FLD_llid29_us_keychg                       REG_FLD(1, 29)
#define e_us_keychg_FLD_llid28_us_keychg                       REG_FLD(1, 28)
#define e_us_keychg_FLD_llid27_us_keychg                       REG_FLD(1, 27)
#define e_us_keychg_FLD_llid26_us_keychg                       REG_FLD(1, 26)
#define e_us_keychg_FLD_llid25_us_keychg                       REG_FLD(1, 25)
#define e_us_keychg_FLD_llid24_us_keychg                       REG_FLD(1, 24)
#define e_us_keychg_FLD_llid23_us_keychg                       REG_FLD(1, 23)
#define e_us_keychg_FLD_llid22_us_keychg                       REG_FLD(1, 22)
#define e_us_keychg_FLD_llid21_us_keychg                       REG_FLD(1, 21)
#define e_us_keychg_FLD_llid20_us_keychg                       REG_FLD(1, 20)
#define e_us_keychg_FLD_llid19_us_keychg                       REG_FLD(1, 19)
#define e_us_keychg_FLD_llid18_us_keychg                       REG_FLD(1, 18)
#define e_us_keychg_FLD_llid17_us_keychg                       REG_FLD(1, 17)
#define e_us_keychg_FLD_llid16_us_keychg                       REG_FLD(1, 16)
#define e_us_keychg_FLD_llid15_us_keychg                       REG_FLD(1, 15)
#define e_us_keychg_FLD_llid14_us_keychg                       REG_FLD(1, 14)
#define e_us_keychg_FLD_llid13_us_keychg                       REG_FLD(1, 13)
#define e_us_keychg_FLD_llid12_us_keychg                       REG_FLD(1, 12)
#define e_us_keychg_FLD_llid11_us_keychg                       REG_FLD(1, 11)
#define e_us_keychg_FLD_llid10_us_keychg                       REG_FLD(1, 10)
#define e_us_keychg_FLD_llid9_us_keychg                        REG_FLD(1, 9)
#define e_us_keychg_FLD_llid8_us_keychg                        REG_FLD(1, 8)
#define e_us_keychg_FLD_llid7_us_keychg                        REG_FLD(1, 7)
#define e_us_keychg_FLD_llid6_us_keychg                        REG_FLD(1, 6)
#define e_us_keychg_FLD_llid5_us_keychg                        REG_FLD(1, 5)
#define e_us_keychg_FLD_llid4_us_keychg                        REG_FLD(1, 4)
#define e_us_keychg_FLD_llid3_us_keychg                        REG_FLD(1, 3)
#define e_us_keychg_FLD_llid2_us_keychg                        REG_FLD(1, 2)
#define e_us_keychg_FLD_llid1_us_keychg                        REG_FLD(1, 1)
#define e_us_keychg_FLD_llid0_us_keychg                        REG_FLD(1, 0)

#define e_cfg_dmy0_FLD_xe_cfg_dmy0                             REG_FLD(32, 0)

#define e_cfg_dmy1_FLD_xe_cfg_dmy1                             REG_FLD(32, 0)

#define e_llid0_3_cfg_FLD_llid3_dmy                            REG_FLD(2, 30)
#define e_llid0_3_cfg_FLD_llid3_encrypt_key                    REG_FLD(1, 29)
#define e_llid0_3_cfg_FLD_llid3_encrypt_en                     REG_FLD(1, 28)
#define e_llid0_3_cfg_FLD_llid3_txfec_en                       REG_FLD(1, 27)
#define e_llid0_3_cfg_FLD_llid3_dcrypt_en                      REG_FLD(1, 26)
#define e_llid0_3_cfg_FLD_llid3_dcrypt_mode                    REG_FLD(1, 25)
#define e_llid0_3_cfg_FLD_llid3_oam_lpbk_en                    REG_FLD(1, 24)
#define e_llid0_3_cfg_FLD_llid2_dmy                            REG_FLD(2, 22)
#define e_llid0_3_cfg_FLD_llid2_encrypt_key                    REG_FLD(1, 21)
#define e_llid0_3_cfg_FLD_llid2_encrypt_en                     REG_FLD(1, 20)
#define e_llid0_3_cfg_FLD_llid2_txfec_en                       REG_FLD(1, 19)
#define e_llid0_3_cfg_FLD_llid2_dcrypt_en                      REG_FLD(1, 18)
#define e_llid0_3_cfg_FLD_llid2_dcrypt_mode                    REG_FLD(1, 17)
#define e_llid0_3_cfg_FLD_llid2_oam_lpbk_en                    REG_FLD(1, 16)
#define e_llid0_3_cfg_FLD_llid1_dmy                            REG_FLD(2, 14)
#define e_llid0_3_cfg_FLD_llid1_encrypt_key                    REG_FLD(1, 13)
#define e_llid0_3_cfg_FLD_llid1_encrypt_en                     REG_FLD(1, 12)
#define e_llid0_3_cfg_FLD_llid1_txfec_en                       REG_FLD(1, 11)
#define e_llid0_3_cfg_FLD_llid1_dcrypt_en                      REG_FLD(1, 10)
#define e_llid0_3_cfg_FLD_llid1_dcrypt_mode                    REG_FLD(1, 9)
#define e_llid0_3_cfg_FLD_llid1_oam_lpbk_en                    REG_FLD(1, 8)
#define e_llid0_3_cfg_FLD_llid0_dmy                            REG_FLD(2, 6)
#define e_llid0_3_cfg_FLD_llid0_encrypt_key                    REG_FLD(1, 5)
#define e_llid0_3_cfg_FLD_llid0_encrypt_en                     REG_FLD(1, 4)
#define e_llid0_3_cfg_FLD_llid0_txfec_en                       REG_FLD(1, 3)
#define e_llid0_3_cfg_FLD_llid0_dcrypt_en                      REG_FLD(1, 2)
#define e_llid0_3_cfg_FLD_llid0_dcrypt_mode                    REG_FLD(1, 1)
#define e_llid0_3_cfg_FLD_llid0_oam_lpbk_en                    REG_FLD(1, 0)

#define e_llid4_7_cfg_FLD_llid7_dmy                            REG_FLD(2, 30)
#define e_llid4_7_cfg_FLD_llid7_encrypt_key                    REG_FLD(1, 29)
#define e_llid4_7_cfg_FLD_llid7_encrypt_en                     REG_FLD(1, 28)
#define e_llid4_7_cfg_FLD_llid7_txfec_en                       REG_FLD(1, 27)
#define e_llid4_7_cfg_FLD_llid7_dcrypt_en                      REG_FLD(1, 26)
#define e_llid4_7_cfg_FLD_llid7_dcrypt_mode                    REG_FLD(1, 25)
#define e_llid4_7_cfg_FLD_llid7_oam_lpbk_en                    REG_FLD(1, 24)
#define e_llid4_7_cfg_FLD_llid6_dmy                            REG_FLD(2, 22)
#define e_llid4_7_cfg_FLD_llid6_encrypt_key                    REG_FLD(1, 21)
#define e_llid4_7_cfg_FLD_llid6_encrypt_en                     REG_FLD(1, 20)
#define e_llid4_7_cfg_FLD_llid6_txfec_en                       REG_FLD(1, 19)
#define e_llid4_7_cfg_FLD_llid6_dcrypt_en                      REG_FLD(1, 18)
#define e_llid4_7_cfg_FLD_llid6_dcrypt_mode                    REG_FLD(1, 17)
#define e_llid4_7_cfg_FLD_llid6_oam_lpbk_en                    REG_FLD(1, 16)
#define e_llid4_7_cfg_FLD_llid5_dmy                            REG_FLD(2, 14)
#define e_llid4_7_cfg_FLD_llid5_encrypt_key                    REG_FLD(1, 13)
#define e_llid4_7_cfg_FLD_llid5_encrypt_en                     REG_FLD(1, 12)
#define e_llid4_7_cfg_FLD_llid5_txfec_en                       REG_FLD(1, 11)
#define e_llid4_7_cfg_FLD_llid5_dcrypt_en                      REG_FLD(1, 10)
#define e_llid4_7_cfg_FLD_llid5_dcrypt_mode                    REG_FLD(1, 9)
#define e_llid4_7_cfg_FLD_llid5_oam_lpbk_en                    REG_FLD(1, 8)
#define e_llid4_7_cfg_FLD_llid4_dmy                            REG_FLD(2, 6)
#define e_llid4_7_cfg_FLD_llid4_encrypt_key                    REG_FLD(1, 5)
#define e_llid4_7_cfg_FLD_llid4_encrypt_en                     REG_FLD(1, 4)
#define e_llid4_7_cfg_FLD_llid4_txfec_en                       REG_FLD(1, 3)
#define e_llid4_7_cfg_FLD_llid4_dcrypt_en                      REG_FLD(1, 2)
#define e_llid4_7_cfg_FLD_llid4_dcrypt_mode                    REG_FLD(1, 1)
#define e_llid4_7_cfg_FLD_llid4_oam_lpbk_en                    REG_FLD(1, 0)

#define e_llid8_11_cfg_FLD_llid11_dmy                          REG_FLD(2, 30)
#define e_llid8_11_cfg_FLD_llid11_encrypt_key                  REG_FLD(1, 29)
#define e_llid8_11_cfg_FLD_llid11_encrypt_en                   REG_FLD(1, 28)
#define e_llid8_11_cfg_FLD_llid11_txfec_en                     REG_FLD(1, 27)
#define e_llid8_11_cfg_FLD_llid11_dcrypt_en                    REG_FLD(1, 26)
#define e_llid8_11_cfg_FLD_llid11_dcrypt_mode                  REG_FLD(1, 25)
#define e_llid8_11_cfg_FLD_llid11_oamlpbk_en                   REG_FLD(1, 24)
#define e_llid8_11_cfg_FLD_llid10_dmy                          REG_FLD(2, 22)
#define e_llid8_11_cfg_FLD_llid10_encrypt_key                  REG_FLD(1, 21)
#define e_llid8_11_cfg_FLD_llid10_encrypt_en                   REG_FLD(1, 20)
#define e_llid8_11_cfg_FLD_llid10_txfec_en                     REG_FLD(1, 19)
#define e_llid8_11_cfg_FLD_llid10_dcrypt_en                    REG_FLD(1, 18)
#define e_llid8_11_cfg_FLD_llid10_dcrypt_mode                  REG_FLD(1, 17)
#define e_llid8_11_cfg_FLD_llid10_oamlpbk_en                   REG_FLD(1, 16)
#define e_llid8_11_cfg_FLD_llid9_dmy                           REG_FLD(2, 14)
#define e_llid8_11_cfg_FLD_llid9_encrypt_key                   REG_FLD(1, 13)
#define e_llid8_11_cfg_FLD_llid9_encrypt_en                    REG_FLD(1, 12)
#define e_llid8_11_cfg_FLD_llid9_txfec_en                      REG_FLD(1, 11)
#define e_llid8_11_cfg_FLD_llid9_dcrypt_en                     REG_FLD(1, 10)
#define e_llid8_11_cfg_FLD_llid9_dcrypt_mode                   REG_FLD(1, 9)
#define e_llid8_11_cfg_FLD_llid9_oamlpbk_en                    REG_FLD(1, 8)
#define e_llid8_11_cfg_FLD_llid8_dmy                           REG_FLD(2, 6)
#define e_llid8_11_cfg_FLD_llid8_encrypt_key                   REG_FLD(1, 5)
#define e_llid8_11_cfg_FLD_llid8_encrypt_en                    REG_FLD(1, 4)
#define e_llid8_11_cfg_FLD_llid8_txfec_en                      REG_FLD(1, 3)
#define e_llid8_11_cfg_FLD_llid8_dcrypt_en                     REG_FLD(1, 2)
#define e_llid8_11_cfg_FLD_llid8_dcrypt_mode                   REG_FLD(1, 1)
#define e_llid8_11_cfg_FLD_llid8_oamlpbk_en                    REG_FLD(1, 0)

#define e_llid12_15_cfg_FLD_llid15_dmy                         REG_FLD(2, 30)
#define e_llid12_15_cfg_FLD_llid15_encrypt_key                 REG_FLD(1, 29)
#define e_llid12_15_cfg_FLD_llid15_encrypt_en                  REG_FLD(1, 28)
#define e_llid12_15_cfg_FLD_llid15_txfec_en                    REG_FLD(1, 27)
#define e_llid12_15_cfg_FLD_llid15_dcrypt_en                   REG_FLD(1, 26)
#define e_llid12_15_cfg_FLD_llid15_dcrypt_mode                 REG_FLD(1, 25)
#define e_llid12_15_cfg_FLD_llid15_oamlpbk_en                  REG_FLD(1, 24)
#define e_llid12_15_cfg_FLD_llid14_dmy                         REG_FLD(2, 22)
#define e_llid12_15_cfg_FLD_llid14_encrypt_key                 REG_FLD(1, 21)
#define e_llid12_15_cfg_FLD_llid14_encrypt_en                  REG_FLD(1, 20)
#define e_llid12_15_cfg_FLD_llid14_txfec_en                    REG_FLD(1, 19)
#define e_llid12_15_cfg_FLD_llid14_dcrypt_en                   REG_FLD(1, 18)
#define e_llid12_15_cfg_FLD_llid14_dcrypt_mode                 REG_FLD(1, 17)
#define e_llid12_15_cfg_FLD_llid14_oamlpbk_en                  REG_FLD(1, 16)
#define e_llid12_15_cfg_FLD_llid13_dmy                         REG_FLD(2, 14)
#define e_llid12_15_cfg_FLD_llid13_encrypt_key                 REG_FLD(1, 13)
#define e_llid12_15_cfg_FLD_llid13_encrypt_en                  REG_FLD(1, 12)
#define e_llid12_15_cfg_FLD_llid13_txfec_en                    REG_FLD(1, 11)
#define e_llid12_15_cfg_FLD_llid13_dcrypt_en                   REG_FLD(1, 10)
#define e_llid12_15_cfg_FLD_llid13_dcrypt_mode                 REG_FLD(1, 9)
#define e_llid12_15_cfg_FLD_llid13_oamlpbk_en                  REG_FLD(1, 8)
#define e_llid12_15_cfg_FLD_llid12_dmy                         REG_FLD(2, 6)
#define e_llid12_15_cfg_FLD_llid12_encrypt_key                 REG_FLD(1, 5)
#define e_llid12_15_cfg_FLD_llid12_encrypt_en                  REG_FLD(1, 4)
#define e_llid12_15_cfg_FLD_llid12_txfec_en                    REG_FLD(1, 3)
#define e_llid12_15_cfg_FLD_llid12_dcrypt_en                   REG_FLD(1, 2)
#define e_llid12_15_cfg_FLD_llid12_dcrypt_mode                 REG_FLD(1, 1)
#define e_llid12_15_cfg_FLD_llid12_oam_lpbk_en                 REG_FLD(1, 0)

#define e_llid16_19_cfg_FLD_llid19_dmy                         REG_FLD(2, 30)
#define e_llid16_19_cfg_FLD_llid19_encrypt_key                 REG_FLD(1, 29)
#define e_llid16_19_cfg_FLD_llid19_encrypt_en                  REG_FLD(1, 28)
#define e_llid16_19_cfg_FLD_llid19_txfec_en                    REG_FLD(1, 27)
#define e_llid16_19_cfg_FLD_llid19_dcrypt_en                   REG_FLD(1, 26)
#define e_llid16_19_cfg_FLD_llid19_dcrypt_mode                 REG_FLD(1, 25)
#define e_llid16_19_cfg_FLD_llid19_oamlpbk_en                  REG_FLD(1, 24)
#define e_llid16_19_cfg_FLD_llid18_dmy                         REG_FLD(2, 22)
#define e_llid16_19_cfg_FLD_llid18_encrypt_key                 REG_FLD(1, 21)
#define e_llid16_19_cfg_FLD_llid18_encrypt_en                  REG_FLD(1, 20)
#define e_llid16_19_cfg_FLD_llid18_txfec_en                    REG_FLD(1, 19)
#define e_llid16_19_cfg_FLD_llid18_dcrypt_en                   REG_FLD(1, 18)
#define e_llid16_19_cfg_FLD_llid18_dcrypt_mode                 REG_FLD(1, 17)
#define e_llid16_19_cfg_FLD_llid18_oamlpbk_en                  REG_FLD(1, 16)
#define e_llid16_19_cfg_FLD_llid17_dmy                         REG_FLD(2, 14)
#define e_llid16_19_cfg_FLD_llid17_encrypt_key                 REG_FLD(1, 13)
#define e_llid16_19_cfg_FLD_llid17_encrypt_en                  REG_FLD(1, 12)
#define e_llid16_19_cfg_FLD_llid17_txfec_en                    REG_FLD(1, 11)
#define e_llid16_19_cfg_FLD_llid17_dcrypt_en                   REG_FLD(1, 10)
#define e_llid16_19_cfg_FLD_llid17_dcrypt_mode                 REG_FLD(1, 9)
#define e_llid16_19_cfg_FLD_llid17_oamlpbk_en                  REG_FLD(1, 8)
#define e_llid16_19_cfg_FLD_llid16_dmy                         REG_FLD(2, 6)
#define e_llid16_19_cfg_FLD_llid16_encrypt_key                 REG_FLD(1, 5)
#define e_llid16_19_cfg_FLD_llid16_encrypt_en                  REG_FLD(1, 4)
#define e_llid16_19_cfg_FLD_llid16_txfec_en                    REG_FLD(1, 3)
#define e_llid16_19_cfg_FLD_llid16_dcrypt_en                   REG_FLD(1, 2)
#define e_llid16_19_cfg_FLD_llid16_dcrypt_mode                 REG_FLD(1, 1)
#define e_llid16_19_cfg_FLD_llid16_oam_lpbk_en                 REG_FLD(1, 0)

#define e_llid20_23_cfg_FLD_llid23_dmy                         REG_FLD(2, 30)
#define e_llid20_23_cfg_FLD_llid23_encrypt_key                 REG_FLD(1, 29)
#define e_llid20_23_cfg_FLD_llid23_encrypt_en                  REG_FLD(1, 28)
#define e_llid20_23_cfg_FLD_llid23_txfec_en                    REG_FLD(1, 27)
#define e_llid20_23_cfg_FLD_llid23_dcrypt_en                   REG_FLD(1, 26)
#define e_llid20_23_cfg_FLD_llid23_dcrypt_mode                 REG_FLD(1, 25)
#define e_llid20_23_cfg_FLD_llid23_oamlpbk_en                  REG_FLD(1, 24)
#define e_llid20_23_cfg_FLD_llid22_dmy                         REG_FLD(2, 22)
#define e_llid20_23_cfg_FLD_llid22_encrypt_key                 REG_FLD(1, 21)
#define e_llid20_23_cfg_FLD_llid22_encrypt_en                  REG_FLD(1, 20)
#define e_llid20_23_cfg_FLD_llid22_txfec_en                    REG_FLD(1, 19)
#define e_llid20_23_cfg_FLD_llid22_dcrypt_en                   REG_FLD(1, 18)
#define e_llid20_23_cfg_FLD_llid22_dcrypt_mode                 REG_FLD(1, 17)
#define e_llid20_23_cfg_FLD_llid22_oamlpbk_en                  REG_FLD(1, 16)
#define e_llid20_23_cfg_FLD_llid21_dmy                         REG_FLD(2, 14)
#define e_llid20_23_cfg_FLD_llid21_encrypt_key                 REG_FLD(1, 13)
#define e_llid20_23_cfg_FLD_llid21_encrypt_en                  REG_FLD(1, 12)
#define e_llid20_23_cfg_FLD_llid21_txfec_en                    REG_FLD(1, 11)
#define e_llid20_23_cfg_FLD_llid21_dcrypt_en                   REG_FLD(1, 10)
#define e_llid20_23_cfg_FLD_llid21_dcrypt_mode                 REG_FLD(1, 9)
#define e_llid20_23_cfg_FLD_llid21_oamlpbk_en                  REG_FLD(1, 8)
#define e_llid20_23_cfg_FLD_llid20_dmy                         REG_FLD(2, 6)
#define e_llid20_23_cfg_FLD_llid20_encrypt_key                 REG_FLD(1, 5)
#define e_llid20_23_cfg_FLD_llid20_encrypt_en                  REG_FLD(1, 4)
#define e_llid20_23_cfg_FLD_llid20_txfec_en                    REG_FLD(1, 3)
#define e_llid20_23_cfg_FLD_llid20_dcrypt_en                   REG_FLD(1, 2)
#define e_llid20_23_cfg_FLD_llid20_dcrypt_mode                 REG_FLD(1, 1)
#define e_llid20_23_cfg_FLD_llid20_oam_lpbk_en                 REG_FLD(1, 0)

#define e_llid24_27_cfg_FLD_llid27_dmy                         REG_FLD(2, 30)
#define e_llid24_27_cfg_FLD_llid27_encrypt_key                 REG_FLD(1, 29)
#define e_llid24_27_cfg_FLD_llid27_encrypt_en                  REG_FLD(1, 28)
#define e_llid24_27_cfg_FLD_llid27_txfec_en                    REG_FLD(1, 27)
#define e_llid24_27_cfg_FLD_llid27_dcrypt_en                   REG_FLD(1, 26)
#define e_llid24_27_cfg_FLD_llid27_dcrypt_mode                 REG_FLD(1, 25)
#define e_llid24_27_cfg_FLD_llid27_oamlpbk_en                  REG_FLD(1, 24)
#define e_llid24_27_cfg_FLD_llid26_dmy                         REG_FLD(2, 22)
#define e_llid24_27_cfg_FLD_llid26_encrypt_key                 REG_FLD(1, 21)
#define e_llid24_27_cfg_FLD_llid26_encrypt_en                  REG_FLD(1, 20)
#define e_llid24_27_cfg_FLD_llid26_txfec_en                    REG_FLD(1, 19)
#define e_llid24_27_cfg_FLD_llid26_dcrypt_en                   REG_FLD(1, 18)
#define e_llid24_27_cfg_FLD_llid26_dcrypt_mode                 REG_FLD(1, 17)
#define e_llid24_27_cfg_FLD_llid26_oamlpbk_en                  REG_FLD(1, 16)
#define e_llid24_27_cfg_FLD_llid25_dmy                         REG_FLD(2, 14)
#define e_llid24_27_cfg_FLD_llid25_encrypt_key                 REG_FLD(1, 13)
#define e_llid24_27_cfg_FLD_llid25_encrypt_en                  REG_FLD(1, 12)
#define e_llid24_27_cfg_FLD_llid25_txfec_en                    REG_FLD(1, 11)
#define e_llid24_27_cfg_FLD_llid25_dcrypt_en                   REG_FLD(1, 10)
#define e_llid24_27_cfg_FLD_llid25_dcrypt_mode                 REG_FLD(1, 9)
#define e_llid24_27_cfg_FLD_llid25_oamlpbk_en                  REG_FLD(1, 8)
#define e_llid24_27_cfg_FLD_llid24_dmy                         REG_FLD(2, 6)
#define e_llid24_27_cfg_FLD_llid24_encrypt_key                 REG_FLD(1, 5)
#define e_llid24_27_cfg_FLD_llid24_encrypt_en                  REG_FLD(1, 4)
#define e_llid24_27_cfg_FLD_llid24_txfec_en                    REG_FLD(1, 3)
#define e_llid24_27_cfg_FLD_llid24_dcrypt_en                   REG_FLD(1, 2)
#define e_llid24_27_cfg_FLD_llid24_dcrypt_mode                 REG_FLD(1, 1)
#define e_llid24_27_cfg_FLD_llid24_oam_lpbk_en                 REG_FLD(1, 0)

#define e_llid28_31_cfg_FLD_llid31_dmy                         REG_FLD(2, 30)
#define e_llid28_31_cfg_FLD_llid31_encrypt_key                 REG_FLD(1, 29)
#define e_llid28_31_cfg_FLD_llid31_encrypt_en                  REG_FLD(1, 28)
#define e_llid28_31_cfg_FLD_llid31_txfec_en                    REG_FLD(1, 27)
#define e_llid28_31_cfg_FLD_llid31_dcrypt_en                   REG_FLD(1, 26)
#define e_llid28_31_cfg_FLD_llid31_dcrypt_mode                 REG_FLD(1, 25)
#define e_llid28_31_cfg_FLD_llid31_oamlpbk_en                  REG_FLD(1, 24)
#define e_llid28_31_cfg_FLD_llid30_dmy                         REG_FLD(2, 22)
#define e_llid28_31_cfg_FLD_llid30_encrypt_key                 REG_FLD(1, 21)
#define e_llid28_31_cfg_FLD_llid30_encrypt_en                  REG_FLD(1, 20)
#define e_llid28_31_cfg_FLD_llid30_txfec_en                    REG_FLD(1, 19)
#define e_llid28_31_cfg_FLD_llid30_dcrypt_en                   REG_FLD(1, 18)
#define e_llid28_31_cfg_FLD_llid30_dcrypt_mode                 REG_FLD(1, 17)
#define e_llid28_31_cfg_FLD_llid30_oamlpbk_en                  REG_FLD(1, 16)
#define e_llid28_31_cfg_FLD_llid29_dmy                         REG_FLD(2, 14)
#define e_llid28_31_cfg_FLD_llid29_encrypt_key                 REG_FLD(1, 13)
#define e_llid28_31_cfg_FLD_llid29_encrypt_en                  REG_FLD(1, 12)
#define e_llid28_31_cfg_FLD_llid29_txfec_en                    REG_FLD(1, 11)
#define e_llid28_31_cfg_FLD_llid29_dcrypt_en                   REG_FLD(1, 10)
#define e_llid28_31_cfg_FLD_llid29_dcrypt_mode                 REG_FLD(1, 9)
#define e_llid28_31_cfg_FLD_llid29_oamlpbk_en                  REG_FLD(1, 8)
#define e_llid28_31_cfg_FLD_llid28_dmy                         REG_FLD(2, 6)
#define e_llid28_31_cfg_FLD_llid28_encrypt_key                 REG_FLD(1, 5)
#define e_llid28_31_cfg_FLD_llid28_encrypt_en                  REG_FLD(1, 4)
#define e_llid28_31_cfg_FLD_llid28_txfec_en                    REG_FLD(1, 3)
#define e_llid28_31_cfg_FLD_llid28_dcrypt_en                   REG_FLD(1, 2)
#define e_llid28_31_cfg_FLD_llid28_dcrypt_mode                 REG_FLD(1, 1)
#define e_llid28_31_cfg_FLD_llid28_oam_lpbk_en                 REG_FLD(1, 0)

#define e_cfg_dmy2_FLD_xe_cfg_dmy2                             REG_FLD(32, 0)

#define e_cfg_dmy3_FLD_xe_cfg_dmy3                             REG_FLD(32, 0)

#define e_cfg_dmy4_FLD_xe_cfg_dmy4                             REG_FLD(32, 0)

#define e_llid_dscvry_ctrl_FLD_mpcp_cmd                        REG_FLD(2, 30)
#define e_llid_dscvry_ctrl_FLD_mpcp_cmd_done                   REG_FLD(1, 16)
#define e_llid_dscvry_ctrl_FLD_rgstr_ack_flg                   REG_FLD(1, 12)
#define e_llid_dscvry_ctrl_FLD_rgstr_req_flg                   REG_FLD(1, 8)
#define e_llid_dscvry_ctrl_FLD_tx_mpcp_llid_idx                REG_FLD(5, 0)

#define e_llid0_dscvry_sts_FLD_llid0_dscvry_sts                REG_FLD(2, 30)
#define e_llid0_dscvry_sts_FLD_llid0_rgstr_flg_sts             REG_FLD(2, 24)
#define e_llid0_dscvry_sts_FLD_llid0_valid                     REG_FLD(1, 16)
#define e_llid0_dscvry_sts_FLD_llid0_value                     REG_FLD(16, 0)

#define e_llid1_dscvry_sts_FLD_llid1_dscvry_sts                REG_FLD(2, 30)
#define e_llid1_dscvry_sts_FLD_llid1_rgstr_flg_sts             REG_FLD(2, 24)
#define e_llid1_dscvry_sts_FLD_llid1_valid                     REG_FLD(1, 16)
#define e_llid1_dscvry_sts_FLD_llid1_value                     REG_FLD(16, 0)

#define e_llid2_dscvry_sts_FLD_llid2_dscvry_sts                REG_FLD(2, 30)
#define e_llid2_dscvry_sts_FLD_llid2_rgstr_flg_sts             REG_FLD(2, 24)
#define e_llid2_dscvry_sts_FLD_llid2_valid                     REG_FLD(1, 16)
#define e_llid2_dscvry_sts_FLD_llid2_value                     REG_FLD(16, 0)

#define e_llid3_dscvry_sts_FLD_llid3_dscvry_sts                REG_FLD(2, 30)
#define e_llid3_dscvry_sts_FLD_llid3_rgstr_flg_sts             REG_FLD(2, 24)
#define e_llid3_dscvry_sts_FLD_llid3_valid                     REG_FLD(1, 16)
#define e_llid3_dscvry_sts_FLD_llid3_value                     REG_FLD(16, 0)

#define e_llid4_dscvry_sts_FLD_llid4_dscvry_sts                REG_FLD(2, 30)
#define e_llid4_dscvry_sts_FLD_llid4_rgstr_flg_sts             REG_FLD(2, 24)
#define e_llid4_dscvry_sts_FLD_llid4_valid                     REG_FLD(1, 16)
#define e_llid4_dscvry_sts_FLD_llid4_value                     REG_FLD(16, 0)

#define e_llid5_dscvry_sts_FLD_llid5_dscvry_sts                REG_FLD(2, 30)
#define e_llid5_dscvry_sts_FLD_llid5_rgstr_flg_sts             REG_FLD(2, 24)
#define e_llid5_dscvry_sts_FLD_llid5_valid                     REG_FLD(1, 16)
#define e_llid5_dscvry_sts_FLD_llid5_value                     REG_FLD(16, 0)

#define e_llid6_dscvry_sts_FLD_llid6_dscvry_sts                REG_FLD(2, 30)
#define e_llid6_dscvry_sts_FLD_llid6_rgstr_flg_sts             REG_FLD(2, 24)
#define e_llid6_dscvry_sts_FLD_llid6_valid                     REG_FLD(1, 16)
#define e_llid6_dscvry_sts_FLD_llid6_value                     REG_FLD(16, 0)

#define e_llid7_dscvry_sts_FLD_llid7_dscvry_sts                REG_FLD(2, 30)
#define e_llid7_dscvry_sts_FLD_llid7_rgstr_flg_sts             REG_FLD(2, 24)
#define e_llid7_dscvry_sts_FLD_llid7_valid                     REG_FLD(1, 16)
#define e_llid7_dscvry_sts_FLD_llid7_value                     REG_FLD(16, 0)

#define e_llid8_dscvry_sts_FLD_llid8_dscvry_sts                REG_FLD(2, 30)
#define e_llid8_dscvry_sts_FLD_llid8_rgstr_flg_sts             REG_FLD(2, 24)
#define e_llid8_dscvry_sts_FLD_llid8_valid                     REG_FLD(1, 16)
#define e_llid8_dscvry_sts_FLD_llid8_value                     REG_FLD(16, 0)

#define e_llid9_dscvry_sts_FLD_llid9_dscvry_sts                REG_FLD(2, 30)
#define e_llid9_dscvry_sts_FLD_llid9_rgstr_flg_sts             REG_FLD(2, 24)
#define e_llid9_dscvry_sts_FLD_llid9_valid                     REG_FLD(1, 16)
#define e_llid9_dscvry_sts_FLD_llid9_value                     REG_FLD(16, 0)

#define e_llid10_dscvry_sts_FLD_llid10_dscvry_sts              REG_FLD(2, 30)
#define e_llid10_dscvry_sts_FLD_llid10_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid10_dscvry_sts_FLD_llid10_valid                   REG_FLD(1, 16)
#define e_llid10_dscvry_sts_FLD_llid10_value                   REG_FLD(16, 0)

#define e_llid11_dscvry_sts_FLD_llid11_dscvry_sts              REG_FLD(2, 30)
#define e_llid11_dscvry_sts_FLD_llid11_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid11_dscvry_sts_FLD_llid11_valid                   REG_FLD(1, 16)
#define e_llid11_dscvry_sts_FLD_llid11_value                   REG_FLD(16, 0)

#define e_llid12_dscvry_sts_FLD_llid12_dscvry_sts              REG_FLD(2, 30)
#define e_llid12_dscvry_sts_FLD_llid12_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid12_dscvry_sts_FLD_llid12_valid                   REG_FLD(1, 16)
#define e_llid12_dscvry_sts_FLD_llid12_value                   REG_FLD(16, 0)

#define e_llid13_dscvry_sts_FLD_llid13_dscvry_sts              REG_FLD(2, 30)
#define e_llid13_dscvry_sts_FLD_llid13_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid13_dscvry_sts_FLD_llid13_valid                   REG_FLD(1, 16)
#define e_llid13_dscvry_sts_FLD_llid13_value                   REG_FLD(16, 0)

#define e_llid14_dscvry_sts_FLD_llid14_dscvry_sts              REG_FLD(2, 30)
#define e_llid14_dscvry_sts_FLD_llid14_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid14_dscvry_sts_FLD_llid14_valid                   REG_FLD(1, 16)
#define e_llid14_dscvry_sts_FLD_llid14_value                   REG_FLD(16, 0)

#define e_llid15_dscvry_sts_FLD_llid15_dscvry_sts              REG_FLD(2, 30)
#define e_llid15_dscvry_sts_FLD_llid15_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid15_dscvry_sts_FLD_llid15_valid                   REG_FLD(1, 16)
#define e_llid15_dscvry_sts_FLD_llid15_value                   REG_FLD(16, 0)

#define e_llid16_dscvry_sts_FLD_llid16_dscvry_sts              REG_FLD(2, 30)
#define e_llid16_dscvry_sts_FLD_llid16_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid16_dscvry_sts_FLD_llid16_valid                   REG_FLD(1, 16)
#define e_llid16_dscvry_sts_FLD_llid16_value                   REG_FLD(16, 0)

#define e_llid17_dscvry_sts_FLD_llid17_dscvry_sts              REG_FLD(2, 30)
#define e_llid17_dscvry_sts_FLD_llid17_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid17_dscvry_sts_FLD_llid17_valid                   REG_FLD(1, 16)
#define e_llid17_dscvry_sts_FLD_llid17_value                   REG_FLD(16, 0)

#define e_llid18_dscvry_sts_FLD_llid18_dscvry_sts              REG_FLD(2, 30)
#define e_llid18_dscvry_sts_FLD_llid18_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid18_dscvry_sts_FLD_llid18_valid                   REG_FLD(1, 16)
#define e_llid18_dscvry_sts_FLD_llid18_value                   REG_FLD(16, 0)

#define e_llid19_dscvry_sts_FLD_llid19_dscvry_sts              REG_FLD(2, 30)
#define e_llid19_dscvry_sts_FLD_llid19_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid19_dscvry_sts_FLD_llid19_valid                   REG_FLD(1, 16)
#define e_llid19_dscvry_sts_FLD_llid19_value                   REG_FLD(16, 0)

#define e_llid20_dscvry_sts_FLD_llid20_dscvry_sts              REG_FLD(2, 30)
#define e_llid20_dscvry_sts_FLD_llid20_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid20_dscvry_sts_FLD_llid20_valid                   REG_FLD(1, 16)
#define e_llid20_dscvry_sts_FLD_llid20_value                   REG_FLD(16, 0)

#define e_llid21_dscvry_sts_FLD_llid21_dscvry_sts              REG_FLD(2, 30)
#define e_llid21_dscvry_sts_FLD_llid21_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid21_dscvry_sts_FLD_llid21_valid                   REG_FLD(1, 16)
#define e_llid21_dscvry_sts_FLD_llid21_value                   REG_FLD(16, 0)

#define e_llid22_dscvry_sts_FLD_llid22_dscvry_sts              REG_FLD(2, 30)
#define e_llid22_dscvry_sts_FLD_llid22_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid22_dscvry_sts_FLD_llid22_valid                   REG_FLD(1, 16)
#define e_llid22_dscvry_sts_FLD_llid22_value                   REG_FLD(16, 0)

#define e_llid23_dscvry_sts_FLD_llid23_dscvry_sts              REG_FLD(2, 30)
#define e_llid23_dscvry_sts_FLD_llid23_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid23_dscvry_sts_FLD_llid23_valid                   REG_FLD(1, 16)
#define e_llid23_dscvry_sts_FLD_llid23_value                   REG_FLD(16, 0)

#define e_llid24_dscvry_sts_FLD_llid24_dscvry_sts              REG_FLD(2, 30)
#define e_llid24_dscvry_sts_FLD_llid24_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid24_dscvry_sts_FLD_llid24_valid                   REG_FLD(1, 16)
#define e_llid24_dscvry_sts_FLD_llid24_value                   REG_FLD(16, 0)

#define e_llid25_dscvry_sts_FLD_llid25_dscvry_sts              REG_FLD(2, 30)
#define e_llid25_dscvry_sts_FLD_llid25_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid25_dscvry_sts_FLD_llid25_valid                   REG_FLD(1, 16)
#define e_llid25_dscvry_sts_FLD_llid25_value                   REG_FLD(16, 0)

#define e_llid26_dscvry_sts_FLD_llid26_dscvry_sts              REG_FLD(2, 30)
#define e_llid26_dscvry_sts_FLD_llid26_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid26_dscvry_sts_FLD_llid26_valid                   REG_FLD(1, 16)
#define e_llid26_dscvry_sts_FLD_llid26_value                   REG_FLD(16, 0)

#define e_llid27_dscvry_sts_FLD_llid27_dscvry_sts              REG_FLD(2, 30)
#define e_llid27_dscvry_sts_FLD_llid27_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid27_dscvry_sts_FLD_llid27_valid                   REG_FLD(1, 16)
#define e_llid27_dscvry_sts_FLD_llid27_value                   REG_FLD(16, 0)

#define e_llid28_dscvry_sts_FLD_llid28_dscvry_sts              REG_FLD(2, 30)
#define e_llid28_dscvry_sts_FLD_llid28_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid28_dscvry_sts_FLD_llid28_valid                   REG_FLD(1, 16)
#define e_llid28_dscvry_sts_FLD_llid28_value                   REG_FLD(16, 0)

#define e_llid29_dscvry_sts_FLD_llid29_dscvry_sts              REG_FLD(2, 30)
#define e_llid29_dscvry_sts_FLD_llid29_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid29_dscvry_sts_FLD_llid29_valid                   REG_FLD(1, 16)
#define e_llid29_dscvry_sts_FLD_llid29_value                   REG_FLD(16, 0)

#define e_llid30_dscvry_sts_FLD_llid30_dscvry_sts              REG_FLD(2, 30)
#define e_llid30_dscvry_sts_FLD_llid30_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid30_dscvry_sts_FLD_llid30_valid                   REG_FLD(1, 16)
#define e_llid30_dscvry_sts_FLD_llid30_value                   REG_FLD(16, 0)

#define e_llid31_dscvry_sts_FLD_llid31_dscvry_sts              REG_FLD(2, 30)
#define e_llid31_dscvry_sts_FLD_llid31_rgstr_flg_sts           REG_FLD(2, 24)
#define e_llid31_dscvry_sts_FLD_llid31_valid                   REG_FLD(1, 16)
#define e_llid31_dscvry_sts_FLD_llid31_value                   REG_FLD(16, 0)

#define e_pending_gnt_num_FLD_echoed_pending_gnt               REG_FLD(8, 8)
#define e_pending_gnt_num_FLD_pending_gnt_num                  REG_FLD(8, 0)

#define e_mac_addr_cfg_FLD_mac_addr_rwcmd                      REG_FLD(1, 31)
#define e_mac_addr_cfg_FLD_mac_addr_rwcmd_done                 REG_FLD(1, 16)
#define e_mac_addr_cfg_FLD_mac_addr_llid_indx                  REG_FLD(5, 1)
#define e_mac_addr_cfg_FLD_mac_addr_dw_idx                     REG_FLD(1, 0)

#define e_mac_addr_value_FLD_mac_addr_value                    REG_FLD(32, 0)

#define e_security_key_cfg_FLD_key_rwcmd                       REG_FLD(1, 31)
#define e_security_key_cfg_FLD_key_macsec_an                   REG_FLD(2, 20)
#define e_security_key_cfg_FLD_key_rwcmd_done                  REG_FLD(1, 16)
#define e_security_key_cfg_FLD_key_llid_index                  REG_FLD(6, 8)
#define e_security_key_cfg_FLD_key_idx                         REG_FLD(1, 4)
#define e_security_key_cfg_FLD_key_dw_indx                     REG_FLD(2, 0)

#define e_key_value_FLD_key_value                              REG_FLD(32, 0)

#define e_enckey_cfg_FLD_enckey_rwcmd                          REG_FLD(1, 31)
#define e_enckey_cfg_FLD_enckey_macsec_an                      REG_FLD(2, 20)
#define e_enckey_cfg_FLD_enckey_rwcmd_done                     REG_FLD(1, 16)
#define e_enckey_cfg_FLD_enckey_llididx                        REG_FLD(6, 8)
#define e_enckey_cfg_FLD_enckey_keyidx                         REG_FLD(1, 4)
#define e_enckey_cfg_FLD_enckey_dwidx                          REG_FLD(2, 0)

#define e_enckey_val_FLD_enckey_value                          REG_FLD(32, 0)

#define e_cfg_dmy5_FLD_xe_cfg_dmy5                             REG_FLD(32, 0)

#define e_rpt_cfg_FLD_llid15_rpt_cfg                           REG_FLD(2, 30)
#define e_rpt_cfg_FLD_llid14_rpt_cfg                           REG_FLD(2, 28)
#define e_rpt_cfg_FLD_llid13_rpt_cfg                           REG_FLD(2, 26)
#define e_rpt_cfg_FLD_llid12_rpt_cfg                           REG_FLD(2, 24)
#define e_rpt_cfg_FLD_llid11_rpt_cfg                           REG_FLD(2, 22)
#define e_rpt_cfg_FLD_llid10_rpt_cfg                           REG_FLD(2, 20)
#define e_rpt_cfg_FLD_llid9_rpt_cfg                            REG_FLD(2, 18)
#define e_rpt_cfg_FLD_llid8_rpt_cfg                            REG_FLD(2, 16)
#define e_rpt_cfg_FLD_llid7_rpt_cfg                            REG_FLD(2, 14)
#define e_rpt_cfg_FLD_llid6_rpt_cfg                            REG_FLD(2, 12)
#define e_rpt_cfg_FLD_llid5_rpt_cfg                            REG_FLD(2, 10)
#define e_rpt_cfg_FLD_llid4_rpt_cfg                            REG_FLD(2, 8)
#define e_rpt_cfg_FLD_llid3_rpt_cfg                            REG_FLD(2, 6)
#define e_rpt_cfg_FLD_llid2_rpt_cfg                            REG_FLD(2, 4)
#define e_rpt_cfg_FLD_llid1_rpt_cfg                            REG_FLD(2, 2)
#define e_rpt_cfg_FLD_llid0_rpt_cfg                            REG_FLD(2, 0)

#define e_rpt_cfg2_FLD_llid31_rpt_cfg                          REG_FLD(2, 30)
#define e_rpt_cfg2_FLD_llid30_rpt_cfg                          REG_FLD(2, 28)
#define e_rpt_cfg2_FLD_llid29_rpt_cfg                          REG_FLD(2, 26)
#define e_rpt_cfg2_FLD_llid28_rpt_cfg                          REG_FLD(2, 24)
#define e_rpt_cfg2_FLD_llid27_rpt_cfg                          REG_FLD(2, 22)
#define e_rpt_cfg2_FLD_llid26_rpt_cfg                          REG_FLD(2, 20)
#define e_rpt_cfg2_FLD_llid25_rpt_cfg                          REG_FLD(2, 18)
#define e_rpt_cfg2_FLD_llid24_rpt_cfg                          REG_FLD(2, 16)
#define e_rpt_cfg2_FLD_llid23_rpt_cfg                          REG_FLD(2, 14)
#define e_rpt_cfg2_FLD_llid22_rpt_cfg                          REG_FLD(2, 12)
#define e_rpt_cfg2_FLD_llid21_rpt_cfg                          REG_FLD(2, 10)
#define e_rpt_cfg2_FLD_llid20_rpt_cfg                          REG_FLD(2, 8)
#define e_rpt_cfg2_FLD_llid19_rpt_cfg                          REG_FLD(2, 6)
#define e_rpt_cfg2_FLD_llid18_rpt_cfg                          REG_FLD(2, 4)
#define e_rpt_cfg2_FLD_llid17_rpt_cfg                          REG_FLD(2, 2)
#define e_rpt_cfg2_FLD_llid16_rpt_cfg                          REG_FLD(2, 0)

#define e_rpt_qthld_cfg_FLD_qthld_rwcmd                        REG_FLD(1, 31)
#define e_rpt_qthld_cfg_FLD_qthld_rwcmd_done                   REG_FLD(1, 30)
#define e_rpt_qthld_cfg_FLD_rpt_llid_idx                       REG_FLD(5, 24)
#define e_rpt_qthld_cfg_FLD_qthld_value                        REG_FLD(16, 8)
#define e_rpt_qthld_cfg_FLD_qthld_idx                          REG_FLD(2, 6)
#define e_rpt_qthld_cfg_FLD_queue_idx                          REG_FLD(3, 0)

#define e_rpt_glb_cfg_FLD_rpt_dmy1                             REG_FLD(17, 15)
#define e_rpt_glb_cfg_FLD_dba_gtthr_frpt                       REG_FLD(1, 14)
#define e_rpt_glb_cfg_FLD_txgnt_cnthit_frpt                    REG_FLD(1, 13)
#define e_rpt_glb_cfg_FLD_ctrlpkt_frpt                         REG_FLD(1, 12)
#define e_rpt_glb_cfg_FLD_gntfrpt_ignore                       REG_FLD(1, 11)
#define e_rpt_glb_cfg_FLD_rpt_qsize_sp                         REG_FLD(1, 10)
#define e_rpt_glb_cfg_FLD_rpt_fullqsize_head                   REG_FLD(1, 9)
#define e_rpt_glb_cfg_FLD_rpt_dpoe_thrstop                     REG_FLD(1, 8)
#define e_rpt_glb_cfg_FLD_rpt_dpoe_queidx                      REG_FLD(3, 5)
#define e_rpt_glb_cfg_FLD_rpt_dpoe_en                          REG_FLD(1, 4)
#define e_rpt_glb_cfg_FLD_rpt_qsize_dec                        REG_FLD(2, 2)
#define e_rpt_glb_cfg_FLD_rpt_qsize_mgnt_sel                   REG_FLD(1, 1)
#define e_rpt_glb_cfg_FLD_tx_default_rpt                       REG_FLD(1, 0)

#define e_rpt_bmap_FLD_rpt_dmy2                                REG_FLD(16, 16)
#define e_rpt_bmap_FLD_rpt_bitmap_ctrl                         REG_FLD(8, 8)
#define e_rpt_bmap_FLD_rpt_bitmap_set                          REG_FLD(8, 0)

#define e_frpt_thr_FLD_frpt_txgnt_cnt                          REG_FLD(16, 16)
#define e_frpt_thr_FLD_frpt_dba_totsize_thr                    REG_FLD(16, 0)

#define e_u1g_rpt_qsizeadj_FLD_u1g_fecon_rpt_qsizeadj          REG_FLD(16, 16)
#define e_u1g_rpt_qsizeadj_FLD_u1g_fecoff_rpt_qsizeadj         REG_FLD(16, 0)

#define e_u10g_rpt_qsizeadj_FLD_u10g_rpt_qsizeadj              REG_FLD(16, 0)

#define e_cfg_dmy7_FLD_xe_cfg_dmy7                             REG_FLD(32, 0)

#define e_cfg_dmy8_FLD_xe_cfg_dmy8                             REG_FLD(32, 0)

#define e_cfg_dmy9_FLD_xe_cfg_dmy9                             REG_FLD(32, 0)

#define e_cfg_dmy10_FLD_xe_cfg_dmy10                           REG_FLD(32, 0)

#define e_cfg_dmy11_FLD_xe_cfg_dmy11                           REG_FLD(32, 0)

#define e_cfg_dmy12_FLD_xe_cfg_dmy12                           REG_FLD(32, 0)

#define e_cfg_dmy13_FLD_xe_cfg_dmy13                           REG_FLD(32, 0)

#define e_cfg_dmy14_FLD_xe_cfg_dmy14                           REG_FLD(32, 0)

#define e_cfg_dmy15_FLD_xe_cfg_dmy15                           REG_FLD(32, 0)

#define e_dfrpt_data5_FLD_dfrpt_data5                          REG_FLD(32, 0)

#define e_dfrpt_data6_FLD_dfrpt_data6                          REG_FLD(32, 0)

#define e_dfrpt_data7_FLD_dfrpt_data7                          REG_FLD(32, 0)

#define e_dfrpt_data8_FLD_dfrpt_data8                          REG_FLD(32, 0)

#define e_dfrpt_data9_FLD_dfrpt_data9                          REG_FLD(32, 0)

#define e_dfrpt_data10_FLD_dfrpt_data10                        REG_FLD(32, 0)

#define e_dfrpt_data11_FLD_dfrpt_data11                        REG_FLD(32, 0)

#define e_dfrpt_data12_FLD_dfrpt_data12                        REG_FLD(32, 0)

#define e_dfrpt_data13_FLD_dfrpt_data13                        REG_FLD(32, 0)

#define e_dfrpt_data14_FLD_dfrpt_data14                        REG_FLD(32, 0)

#define e_laser_onoff_time_FLD_laser_off_time                  REG_FLD(8, 8)
#define e_laser_onoff_time_FLD_laser_on_time                   REG_FLD(8, 0)

#define e_sync_time_FLD_sync_time_updte                        REG_FLD(1, 16)
#define e_sync_time_FLD_sync_time                              REG_FLD(16, 0)

#define e_overhead_time_thr_FLD_sync_time_maxen                REG_FLD(1, 31)
#define e_overhead_time_thr_FLD_lsroff_time_maxen              REG_FLD(1, 30)
#define e_overhead_time_thr_FLD_lsron_time_maxen               REG_FLD(1, 29)
#define e_overhead_time_thr_FLD_sync_time_maxval               REG_FLD(8, 16)
#define e_overhead_time_thr_FLD_lsroff_time_maxval             REG_FLD(8, 8)
#define e_overhead_time_thr_FLD_lsron_time_maxval              REG_FLD(8, 0)

#define e_laser_onoff_time2_FLD_olt_lsroff_time                REG_FLD(8, 24)
#define e_laser_onoff_time2_FLD_olt_lsron_time                 REG_FLD(8, 16)
#define e_laser_onoff_time2_FLD_olt_sync_time                  REG_FLD(16, 0)

#define e_olt_dscvinfo_FLD_olt_dscvinfo_match                  REG_FLD(4, 4)
#define e_olt_dscvinfo_FLD_olt_dscvinfo_mis                    REG_FLD(4, 0)

#define e_grd_thrshld_FLD_guard_thrshld                        REG_FLD(8, 0)

#define e_mpcp_timeout_intvl_FLD_mpcp_timeout_intvl            REG_FLD(32, 0)

#define e_rpt_timeout_intvl_FLD_rpt_timeout_intvl              REG_FLD(24, 0)

#define e_max_future_gnt_time_FLD_max_future_gnt_time          REG_FLD(32, 0)

#define e_min_proc_time_FLD_min_proc_time                      REG_FLD(16, 0)

#define e_trx_adjust_time1_FLD_tx_stm_adj                      REG_FLD(16, 0)

#define e_trx_adjust_time2_FLD_tx_len_adj                      REG_FLD(16, 16)
#define e_trx_adjust_time2_FLD_rx_tmstp_adj                    REG_FLD(16, 0)

#define e_trx_adjtime3_FLD_u10g_tx_tsadj                       REG_FLD(16, 16)
#define e_trx_adjtime3_FLD_u10g_tx_stmadj                      REG_FLD(16, 0)

#define e_trx_adjtime4_FLD_d10g_rx_tsadj                       REG_FLD(16, 16)
#define e_trx_adjtime4_FLD_d10g_rx_tsadj_ofst                  REG_FLD(5, 8)

#define e_trx_adjtime5_FLD_u1g_tx_tsadj                        REG_FLD(16, 16)

#define e_rxphydly_adjtime_FLD_rx_phydly_dft                   REG_FLD(1, 31)
#define e_rxphydly_adjtime_FLD_d1g_rx_phydly_ofst              REG_FLD(1, 27)
#define e_rxphydly_adjtime_FLD_d1g_rx_phydly                   REG_FLD(11, 16)
#define e_rxphydly_adjtime_FLD_d10g_rx_phydly_ofst             REG_FLD(5, 11)
#define e_rxphydly_adjtime_FLD_d10g_rx_phydly                  REG_FLD(11, 0)

#define e_txfetch_cfg_FLD_tx_fetch_leadtime                    REG_FLD(8, 16)
#define e_txfetch_cfg_FLD_tx_dma_leadtime                      REG_FLD(16, 0)

#define e_tx_cal_cnst_FLD_dscvr_gnt_len                        REG_FLD(8, 24)
#define e_tx_cal_cnst_FLD_fec_tail_grd                         REG_FLD(8, 16)
#define e_tx_cal_cnst_FLD_tail_grd                             REG_FLD(8, 8)
#define e_tx_cal_cnst_FLD_default_ovrhd                        REG_FLD(6, 0)

#define e_txcal_cnst2_FLD_ipgalign_mtd                         REG_FLD(1, 31)
#define e_txcal_cnst2_FLD_rpt_feccal_cnt_multival              REG_FLD(6, 24)
#define e_txcal_cnst2_FLD_u10g_dscv_gntlen                     REG_FLD(8, 8)
#define e_txcal_cnst2_FLD_u10g_tail_grd                        REG_FLD(8, 0)

#define e_txcal_cnst3_FLD_u1g_fecon_min_gntlen                 REG_FLD(8, 24)
#define e_txcal_cnst3_FLD_u10g_eoblen                          REG_FLD(8, 16)
#define e_txcal_cnst3_FLD_u10g_min_gntlen                      REG_FLD(8, 8)
#define e_txcal_cnst3_FLD_u1g_fecoff_min_gntlen                REG_FLD(8, 0)

#define e_txsch_cfg_FLD_txfifo_wr_thren                        REG_FLD(1, 31)
#define e_txsch_cfg_FLD_txfifo_wr_thr                          REG_FLD(7, 24)
#define e_txsch_cfg_FLD_txsch_dmy0                             REG_FLD(2, 22)
#define e_txsch_cfg_FLD_txfifo_pad_hthr                        REG_FLD(10, 12)
#define e_txsch_cfg_FLD_txsch_dmy1                             REG_FLD(2, 10)
#define e_txsch_cfg_FLD_txfifo_pad_lthr                        REG_FLD(10, 0)

#define e_rxfifo_thr_FLD_rx_dmy1                               REG_FLD(16, 16)
#define e_rxfifo_thr_FLD_rx_dmy0                               REG_FLD(11, 5)
#define e_rxfifo_thr_FLD_rxfifo_mbithr                         REG_FLD(5, 0)

#define e_bcllid_cfg_FLD_d10g_bcllid                           REG_FLD(16, 16)
#define e_bcllid_cfg_FLD_d1g_bcllid                            REG_FLD(16, 0)

#define e_txfrm_cfg1_FLD_tx_mpcp_addrl                         REG_FLD(32, 0)

#define e_txfrm_cfg2_FLD_tx_mpcp_addrh                         REG_FLD(16, 16)
#define e_txfrm_cfg2_FLD_tx_mpcp_etype                         REG_FLD(16, 0)

#define e_txfrm_cfg3_FLD_tx_rgreq_op                           REG_FLD(16, 16)
#define e_txfrm_cfg3_FLD_tx_rgack_op                           REG_FLD(16, 0)

#define e_tod_sync_x_FLD_tod_sync_x                            REG_FLD(32, 0)

#define e_tod_ltncy_FLD_rl_rx_phydly_ofst                      REG_FLD(5, 19)
#define e_tod_ltncy_FLD_ingrs_latency                          REG_FLD(11, 8)
#define e_tod_ltncy_FLD_egrs_latency                           REG_FLD(8, 0)

#define e_new_tod_p2p_offset_sec_l32_FLD_new_tod_p2p_offset_sec_l32 REG_FLD(32, 0)

#define e_new_tod_p2p_tod_offset_nsec_FLD_new_tod_nsec         REG_FLD(32, 0)

#define e_tod_p2p_tod_sec_l32_FLD_tod_p2p_sec_l32              REG_FLD(32, 0)

#define e_tod_p2p_tod_nsec_FLD_tod_p2p_nsec                    REG_FLD(32, 0)

#define e_tod_period_FLD_tod_period                            REG_FLD(8, 0)

#define e_tod_1pps_ctrl_FLD_tod_1pps_width_ctrl                REG_FLD(32, 0)

#define e_pwr_sv_cfg_FLD_pwd_mode                              REG_FLD(2, 28)
#define e_pwr_sv_cfg_FLD_timedrift_ignore                      REG_FLD(1, 26)
#define e_pwr_sv_cfg_FLD_onu_wakeup                            REG_FLD(1, 25)
#define e_pwr_sv_cfg_FLD_pwr_sv_start                          REG_FLD(1, 24)
#define e_pwr_sv_cfg_FLD_pwd_mode_i                            REG_FLD(3, 20)
#define e_pwr_sv_cfg_FLD_slp_duration_max_h                    REG_FLD(16, 0)

#define e_slp_durt_max_FLD_slp_duration_max_l                  REG_FLD(32, 0)

#define e_slp_duration_FLD_slp_duration                        REG_FLD(32, 0)

#define e_act_duration_FLD_act_duration                        REG_FLD(32, 0)

#define e_pwron_dly_FLD_pwron_dly                              REG_FLD(32, 0)

#define e_slp_duration_i_FLD_rx_slpalw_pwd_mode                REG_FLD(8, 24)
#define e_slp_duration_i_FLD_rx_slpalw_flag                    REG_FLD(8, 16)
#define e_slp_duration_i_FLD_rx_slpalw_duration                REG_FLD(16, 0)

#define e_oui_a_cfg_FLD_oui_a_en                               REG_FLD(1, 31)
#define e_oui_a_cfg_FLD_oui_a_dmy                              REG_FLD(7, 24)
#define e_oui_a_cfg_FLD_oui_a_val                              REG_FLD(24, 0)

#define e_oui_b_cfg_FLD_oui_b_en                               REG_FLD(1, 31)
#define e_oui_b_cfg_FLD_oui_b_dmy                              REG_FLD(7, 24)
#define e_oui_b_cfg_FLD_oui_b_val                              REG_FLD(24, 0)

#define e_oui_c_cfg_FLD_oui_c_en                               REG_FLD(1, 31)
#define e_oui_c_cfg_FLD_oui_c_dmy                              REG_FLD(7, 24)
#define e_oui_c_cfg_FLD_oui_c_val                              REG_FLD(24, 0)

#define e_dyinggsp_cfg_FLD_hw_dygasp_en                        REG_FLD(1, 31)
#define e_dyinggsp_cfg_FLD_sw_init_dygasp                      REG_FLD(1, 16)
#define e_dyinggsp_cfg_FLD_dygasp_num_of_times                 REG_FLD(8, 8)

#define e_dyinggsp_w1_FLD_dyinggsp_w1_len_type                 REG_FLD(32, 0)

#define e_dyinggsp_w2_FLD_dyinggsp_w2                          REG_FLD(32, 0)

#define e_dyinggsp_w3_FLD_dyinggsp_w3                          REG_FLD(32, 0)

#define e_dyinggsp_w4_FLD_dyinggsp_w4                          REG_FLD(32, 0)

#define e_dyinggsp_w5_FLD_dyinggsp_w5                          REG_FLD(32, 0)

#define e_dyinggsp_w6_FLD_dyinggsp_w6                          REG_FLD(32, 0)

#define e_dyinggsp_w7_FLD_dyinggsp_w7                          REG_FLD(32, 0)

#define e_dyinggsp_w8_FLD_dyinggsp_w8                          REG_FLD(32, 0)

#define e_dyinggsp_w9_FLD_dyinggsp_w9                          REG_FLD(32, 0)

#define e_dyinggsp_w10_FLD_dyinggsp_w10                        REG_FLD(32, 0)

#define e_dyinggsp_w11_FLD_dyinggsp_w11                        REG_FLD(32, 0)

#define e_dyinggsp_w12_FLD_dyinggsp_w12                        REG_FLD(32, 0)

#define e_oam_kpalv_ctrl_FLD_oam_kpalv_llid_idx                REG_FLD(6, 20)
#define e_oam_kpalv_ctrl_FLD_oam_kpalv_interval                REG_FLD(12, 8)
#define e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_trig                 REG_FLD(1, 2)
#define e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_cfg                  REG_FLD(1, 1)
#define e_oam_kpalv_ctrl_FLD_oam_kpalv_en                      REG_FLD(1, 0)

#define e_oam_kpalv_w1_FLD_oam_kpalv_w1                        REG_FLD(32, 0)

#define e_oam_kpalv_w2_FLD_oam_kpalv_w2                        REG_FLD(32, 0)

#define e_oam_kpalv_w3_FLD_oam_kpalv_w3                        REG_FLD(32, 0)

#define e_oam_kpalv_w4_FLD_oam_kpalv_w4                        REG_FLD(32, 0)

#define e_oam_kpalv_w5_FLD_oam_kpalv_w5                        REG_FLD(32, 0)

#define e_oam_kpalv_w6_FLD_oam_kpalv_w6                        REG_FLD(32, 0)

#define e_oam_kpalv_w7_FLD_oam_kpalv_w7                        REG_FLD(32, 0)

#define e_oam_kpalv_w8_FLD_oam_kpalv_w8                        REG_FLD(32, 0)

#define e_oam_kpalv_w9_FLD_oam_kpalv_w9                        REG_FLD(32, 0)

#define e_oam_kpalv_w10_FLD_oam_kpalv_w10                      REG_FLD(32, 0)

#define e_oam_kpalv_w11_FLD_oam_kpalv_w11                      REG_FLD(32, 0)

#define e_oam_kpalv_w12_FLD_oam_kpalv_w12                      REG_FLD(32, 0)

#define e_kpalv_hwen_sts_FLD_kpalv_hwen_sts                    REG_FLD(32, 0)

#define e_mcgrp_en_FLD_mcgrp_llid_en                           REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg0_FLD_mcgrp_llid1_val                REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg0_FLD_mcgrp_llid0_val                REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg1_FLD_mcgrp_llid3_val                REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg1_FLD_mcgrp_llid2_val                REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg2_FLD_mcgrp_llid5_val                REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg2_FLD_mcgrp_llid4_val                REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg3_FLD_mcgrp_llid7_val                REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg3_FLD_mcgrp_llid6_val                REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg4_FLD_mcgrp_llid9_val                REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg4_FLD_mcgrp_llid8_val                REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg5_FLD_mcgrp_llid11_val               REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg5_FLD_mcgrp_llid10_val               REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg6_FLD_mcgrp_llid13_val               REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg6_FLD_mcgrp_llid12_val               REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg7_FLD_mcgrp_llid15_val               REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg7_FLD_mcgrp_llid14_val               REG_FLD(16, 0)

#define e_crpt_cfg_FLD_crpt_dmy0                               REG_FLD(2, 30)
#define e_crpt_cfg_FLD_kpalv_encrpt_en                         REG_FLD(1, 29)
#define e_crpt_cfg_FLD_mpcp_encrpt_en                          REG_FLD(1, 28)
#define e_crpt_cfg_FLD_dygasp_encrpt_en                        REG_FLD(1, 27)
#define e_crpt_cfg_FLD_llidalluc_decrpt_en                     REG_FLD(1, 26)
#define e_crpt_cfg_FLD_llidmc_decrpt_en                        REG_FLD(1, 25)
#define e_crpt_cfg_FLD_llidbc_decrpt_en                        REG_FLD(1, 24)
#define e_crpt_cfg_FLD_mcgrp_decrpt_en                         REG_FLD(16, 8)
#define e_crpt_cfg_FLD_crpt_dmy1                               REG_FLD(2, 6)
#define e_crpt_cfg_FLD_encrpt_mode                             REG_FLD(2, 4)
#define e_crpt_cfg_FLD_crcchk_mode                             REG_FLD(1, 3)
#define e_crpt_cfg_FLD_decrpt_mode                             REG_FLD(3, 0)

#define e_cfbsec_cfg_FLD_cfbsec_dmy                            REG_FLD(30, 2)
#define e_cfbsec_cfg_FLD_cfbdec_iv_srcsel                      REG_FLD(1, 1)
#define e_cfbsec_cfg_FLD_cfbdec_padd_mode                      REG_FLD(1, 0)

#define e_ctrsec_cfg_FLD_ctrsec_dmy0                           REG_FLD(15, 17)
#define e_ctrsec_cfg_FLD_ctrenc_padd_mode                      REG_FLD(1, 16)
#define e_ctrsec_cfg_FLD_ctrsec_dmy1                           REG_FLD(14, 2)
#define e_ctrsec_cfg_FLD_ctrdec_srcaddr_sel                    REG_FLD(1, 1)
#define e_ctrsec_cfg_FLD_ctrdec_padd_mode                      REG_FLD(1, 0)

#define e_desec_srcaddr_cfg_FLD_desec_srcaddr_rwcmd            REG_FLD(1, 31)
#define e_desec_srcaddr_cfg_FLD_desec_srcaddr_rwcmd_done       REG_FLD(1, 30)
#define e_desec_srcaddr_cfg_FLD_desec_srcaddr_llid_idx         REG_FLD(6, 16)
#define e_desec_srcaddr_cfg_FLD_desec_srcaddr_value_h          REG_FLD(16, 0)

#define e_desec_srcaddr_val_FLD_desec_srcaddr_value_l          REG_FLD(32, 0)

#define e_macsdec_cfg_FLD_macsdec_dmy0                         REG_FLD(5, 27)
#define e_macsdec_cfg_FLD_macsdec_tag_bit_swap                 REG_FLD(1, 26)
#define e_macsdec_cfg_FLD_macsdec_iv_bit_swap                  REG_FLD(1, 25)
#define e_macsdec_cfg_FLD_macsdec_aad_bit_swap                 REG_FLD(1, 24)
#define e_macsdec_cfg_FLD_macsdec_pt_bit_swap                  REG_FLD(1, 23)
#define e_macsdec_cfg_FLD_macsdec_ct_bit_swap                  REG_FLD(1, 22)
#define e_macsdec_cfg_FLD_macsdec_srcaddr_sel                  REG_FLD(1, 21)
#define e_macsdec_cfg_FLD_macsdec_timout_en                    REG_FLD(1, 20)
#define e_macsdec_cfg_FLD_macsdec_conofst                      REG_FLD(8, 12)
#define e_macsdec_cfg_FLD_sectag_e0c0_desec                    REG_FLD(1, 11)
#define e_macsdec_cfg_FLD_sectag_e0c0_icvchk                   REG_FLD(1, 10)
#define e_macsdec_cfg_FLD_sectag_e0c0_drop                     REG_FLD(1, 9)
#define e_macsdec_cfg_FLD_sectag_e0c1_desec                    REG_FLD(1, 8)
#define e_macsdec_cfg_FLD_sectag_e0c1_icvchk                   REG_FLD(1, 7)
#define e_macsdec_cfg_FLD_sectag_e0c1_drop                     REG_FLD(1, 6)
#define e_macsdec_cfg_FLD_sectag_e1c0_desec                    REG_FLD(1, 5)
#define e_macsdec_cfg_FLD_sectag_e1c0_icvchk                   REG_FLD(1, 4)
#define e_macsdec_cfg_FLD_sectag_e1c0_drop                     REG_FLD(1, 3)
#define e_macsdec_cfg_FLD_sectag_e1c1_desec                    REG_FLD(1, 2)
#define e_macsdec_cfg_FLD_sectag_e1c1_icvchk                   REG_FLD(1, 1)
#define e_macsdec_cfg_FLD_sectag_e1c1_drop                     REG_FLD(1, 0)

#define e_macsenc_cfg_FLD_macsenc_mode                         REG_FLD(2, 30)
#define e_macsenc_cfg_FLD_macsenc_dmy0                         REG_FLD(3, 27)
#define e_macsenc_cfg_FLD_macsenc_tag_bit_swap                 REG_FLD(1, 26)
#define e_macsenc_cfg_FLD_macsenc_iv_bit_swap                  REG_FLD(1, 25)
#define e_macsenc_cfg_FLD_macsenc_aad_bit_swap                 REG_FLD(1, 24)
#define e_macsenc_cfg_FLD_macsenc_pt_bit_swap                  REG_FLD(1, 23)
#define e_macsenc_cfg_FLD_macsenc_ct_bit_swap                  REG_FLD(1, 22)
#define e_macsenc_cfg_FLD_macsenc_srcaddr_sel                  REG_FLD(1, 21)
#define e_macsenc_cfg_FLD_macsenc_timout_en                    REG_FLD(1, 20)
#define e_macsenc_cfg_FLD_macsenc_dmy1                         REG_FLD(12, 8)
#define e_macsenc_cfg_FLD_macsenc_tag_v                        REG_FLD(1, 7)
#define e_macsenc_cfg_FLD_macsenc_tag_es                       REG_FLD(1, 6)
#define e_macsenc_cfg_FLD_macsenc_tag_sc                       REG_FLD(1, 5)
#define e_macsenc_cfg_FLD_macsenc_tag_scb                      REG_FLD(1, 4)
#define e_macsenc_cfg_FLD_macsenc_tag_e                        REG_FLD(1, 3)
#define e_macsenc_cfg_FLD_macsenc_tag_c                        REG_FLD(1, 2)
#define e_macsenc_cfg_FLD_macsenc_dmy2                         REG_FLD(2, 0)

#define e_macsenc_pnini_cfg_FLD_macsenc_pn_inival              REG_FLD(32, 0)

#define e_macsenc_pn_cfg_FLD_macsenc_pn_rwcmd                  REG_FLD(1, 31)
#define e_macsenc_pn_cfg_FLD_macsenc_pn_rwcmd_done             REG_FLD(1, 30)
#define e_macsenc_pn_cfg_FLD_macsenc_pn_llid_idx               REG_FLD(5, 16)

#define e_macsenc_pn_val_FLD_macsenc_pn_val                    REG_FLD(32, 0)

#define e_olt_macaddr_h_FLD_olt_macaddr_h                      REG_FLD(16, 0)

#define e_olt_macaddr_l_FLD_olt_macaddr_l                      REG_FLD(32, 0)

#define e_snf_cfg_FLD_sniffer_mode                             REG_FLD(1, 31)
#define e_snf_cfg_FLD_snf_rsv0                                 REG_FLD(5, 26)
#define e_snf_cfg_FLD_llidinfo_snf                             REG_FLD(1, 25)
#define e_snf_cfg_FLD_snf_fcserr_fwd                           REG_FLD(1, 24)
#define e_snf_cfg_FLD_snf_rsv1                                 REG_FLD(2, 22)
#define e_snf_cfg_FLD_us_snf_mode                              REG_FLD(1, 21)
#define e_snf_cfg_FLD_us_kpalv_snf                             REG_FLD(1, 20)
#define e_snf_cfg_FLD_us_dygasp_snf                            REG_FLD(1, 19)
#define e_snf_cfg_FLD_us_rpt_snf                               REG_FLD(1, 18)
#define e_snf_cfg_FLD_us_mpcp_snf                              REG_FLD(1, 17)
#define e_snf_cfg_FLD_us_oam_snf                               REG_FLD(1, 16)
#define e_snf_cfg_FLD_snf_rsv2                                 REG_FLD(4, 12)
#define e_snf_cfg_FLD_mcgrp_eth_snf                            REG_FLD(1, 11)
#define e_snf_cfg_FLD_mcgrp_nrlgate_snf                        REG_FLD(1, 10)
#define e_snf_cfg_FLD_mcgrp_oam_snf                            REG_FLD(1, 9)
#define e_snf_cfg_FLD_mcgrp_llid_snf                           REG_FLD(1, 8)
#define e_snf_cfg_FLD_alluc_eth_snf                            REG_FLD(1, 7)
#define e_snf_cfg_FLD_alluc_nrlgate_snf                        REG_FLD(1, 6)
#define e_snf_cfg_FLD_alluc_oam_snf                            REG_FLD(1, 5)
#define e_snf_cfg_FLD_alluc_llid_snf                           REG_FLD(1, 4)
#define e_snf_cfg_FLD_snf_rsv3                                 REG_FLD(2, 2)
#define e_snf_cfg_FLD_ds_nrlgate_snf                           REG_FLD(1, 1)
#define e_snf_cfg_FLD_ds_mpcp_snf                              REG_FLD(1, 0)

#define e_snf_sp_tag_FLD_snf_sp_tag                            REG_FLD(32, 0)

#define e_snf_dah_FLD_snf_pkt_dah                              REG_FLD(32, 0)

#define e_snf_dal_FLD_snf_pkt_dal                              REG_FLD(16, 16)
#define e_snf_dal_FLD_snf_pkt_sah                              REG_FLD(16, 0)

#define e_snf_sal_FLD_snf_pkt_sal                              REG_FLD(32, 0)

#define e_snf_etype_FLD_snf_pkt_etype                          REG_FLD(16, 0)

#define e_rdmdly_cfg_FLD_rdmdly_mode                           REG_FLD(2, 30)
#define e_rdmdly_cfg_FLD_rdmdly_lthr                           REG_FLD(14, 16)
#define e_rdmdly_cfg_FLD_rdmdly_hthr                           REG_FLD(16, 0)

#define e_rdmdly_stat_FLD_dscvgate_gntlen                      REG_FLD(16, 16)
#define e_rdmdly_stat_FLD_rdmdly                               REG_FLD(16, 0)

#define e_dbg_prb_sel_FLD_probe_dmysel                         REG_FLD(8, 24)
#define e_dbg_prb_sel_FLD_probe_dtgrp_sel                      REG_FLD(8, 16)
#define e_dbg_prb_sel_FLD_probe_bit0_sel                       REG_FLD(8, 8)
#define e_dbg_prb_sel_FLD_probe_sel                            REG_FLD(8, 0)

#define e_dbg_prb_h32_FLD_probe_h32                            REG_FLD(32, 0)

#define e_dbg_prb_l32_FLD_probe_l32                            REG_FLD(32, 0)

#define e_utili_cfg_FLD_cf_utili_wdm                           REG_FLD(16, 16)
#define e_utili_cfg_FLD_cf_utili_pktovhd                       REG_FLD(8, 8)
#define e_utili_cfg_FLD_cf_utili_rsv                           REG_FLD(6, 2)
#define e_utili_cfg_FLD_cf_utili_pkttype                       REG_FLD(1, 1)
#define e_utili_cfg_FLD_cf_utili_mode                          REG_FLD(1, 0)

#define e_utili_check_FLD_cf_utilization_cntclr                REG_FLD(1, 1)
#define e_utili_check_FLD_cf_utilization_chken                 REG_FLD(1, 0)

#define total_gnt_sizeh_FLD_total_gnt_size_h32                 REG_FLD(32, 0)

#define total_gnt_sizel_FLD_total_gnt_size_l32                 REG_FLD(32, 0)

#define total_pkt_cnt_FLD_total_pkt_cnt                        REG_FLD(32, 0)

#define total_pkt_size_l_FLD_total_pkt_size_l32                REG_FLD(32, 0)

#define total_pkt_size_h_FLD_total_pkt_size_h32                REG_FLD(32, 0)

#define total_u10g_nouse_gntlen_FLD_total_u10g_nouse_gntlen    REG_FLD(32, 0)

#define total_gnt_cnt_FLD_total_gnt_cnt                        REG_FLD(32, 0)

#define total_tx_cyc_h_FLD_total_tx_cyc_h32                    REG_FLD(32, 0)

#define total_tx_cyc_l_FLD_total_tx_cyc_l32                    REG_FLD(32, 0)

#define e_sts_dmy6_FLD_xe_sts_dmy6                             REG_FLD(32, 0)

#define e_sts_dmy7_FLD_xe_sts_dmy7                             REG_FLD(32, 0)

#define e_sts_dmy8_FLD_xe_sts_dmy8                             REG_FLD(32, 0)

#define e_sts_dmy9_FLD_xe_sts_dmy9                             REG_FLD(32, 0)

#define e_sts_dmy10_FLD_xe_sts_dmy10                           REG_FLD(32, 0)

#define e_sts_dmy26_FLD_xe_sts_dmy26                           REG_FLD(32, 0)

#define e_sts_dmy27_FLD_xe_sts_dmy27                           REG_FLD(32, 0)

#define e_sts_dmy28_FLD_xe_sts_dmy28                           REG_FLD(32, 0)

#define e_sts_dmy29_FLD_xe_sts_dmy29                           REG_FLD(32, 0)

#define e_sram_pd_FLD_xemac_sram_pd                            REG_FLD(32, 0)

#define e_mbist_delsel_cfg0_FLD_xemac_mbist_delsel_cfg0        REG_FLD(32, 0)

#define e_mbist_delsel_cfg1_FLD_xemac_mbist_delsel_cfg1        REG_FLD(32, 0)

#define e_mbist_delsel_cfg2_FLD_xemac_mbist_delsel_cfg2        REG_FLD(32, 0)

#define e_mbist_delsel_cfg3_FLD_xemac_mbist_delsel_cfg3        REG_FLD(32, 0)

#define e_mbist_done_sts0_FLD_xemac_mbist_done_sts0            REG_FLD(32, 0)

#define e_mbist_fail_sts0_FLD_xemac_mbist_fail_sts0            REG_FLD(32, 0)

#define e_mbist_fail_sts1_FLD_xemac_mbist_fail_sts1            REG_FLD(32, 0)

#define rx_sld_sts_FLD_sld_b7                                  REG_FLD(1, 7)
#define rx_sld_sts_FLD_sld_b6                                  REG_FLD(1, 6)
#define rx_sld_sts_FLD_sld_b5                                  REG_FLD(1, 5)
#define rx_sld_sts_FLD_sld_b4                                  REG_FLD(1, 4)
#define rx_sld_sts_FLD_sld_b3                                  REG_FLD(1, 3)
#define rx_sld_sts_FLD_sld_b2                                  REG_FLD(1, 2)
#define rx_sld_sts_FLD_sld_b1                                  REG_FLD(1, 1)
#define rx_sld_sts_FLD_sld_b0                                  REG_FLD(1, 0)

#define e_glue_cfg_FLD_txmpi_fifound_pktgate_gntcnt            REG_FLD(16, 16)
#define e_glue_cfg_FLD_txmpi_fifound_pktgate_en                REG_FLD(1, 15)
#define e_glue_cfg_FLD_txmpi_fifound_thr                       REG_FLD(11, 0)

#define e_gntreq_tmout_FLD_gntreq_grden                        REG_FLD(1, 31)
#define e_gntreq_tmout_FLD_gntreq_grdcyc                       REG_FLD(31, 0)

#define e_fpga_gendef_FLD_fpga_gendef                          REG_FLD(32, 0)

#define e_fpga_genver_FLD_fpga_genver                          REG_FLD(32, 0)

#define e_cnt_clr_FLD_glb_cntclr                               REG_FLD(1, 0)

#define e_rxmbi_eth_cnt_FLD_rxmbi_eth_cnt                      REG_FLD(32, 0)

#define e_rxmpi_eth_cnt_FLD_rxmpi_eth_cnt                      REG_FLD(32, 0)

#define e_txmbi_eth_cnt_FLD_txmbi_eth_cnt                      REG_FLD(32, 0)

#define e_txmpi_eth_cnt_FLD_txmpi_eth_cnt                      REG_FLD(32, 0)

#define e_oam_stat_FLD_rx_oam_cnt                              REG_FLD(16, 16)
#define e_oam_stat_FLD_tx_oam_cnt                              REG_FLD(16, 0)

#define e_mpcp_stat_FLD_mpcp_err_cnt                           REG_FLD(8, 24)
#define e_mpcp_stat_FLD_mpcp_rgst_cnt                          REG_FLD(8, 16)
#define e_mpcp_stat_FLD_mpcp_dscv_gate_cnt                     REG_FLD(16, 0)

#define e_mpcp_rgst_stat_FLD_mpcp_rgst_req_cnt                 REG_FLD(8, 8)
#define e_mpcp_rgst_stat_FLD_mpcp_rgst_ack_cnt                 REG_FLD(8, 0)

#define e_rxmbi_bytecnt_h_FLD_rxmbi_bytecnt_h                  REG_FLD(32, 0)

#define e_rxmbi_bytecnt_l_FLD_rxmbi_bytecnt_l                  REG_FLD(32, 0)

#define e_rxmbi_snf_cnt_FLD_rxmbi_snf_cnt                      REG_FLD(32, 0)

#define e_rxmpi_uc_cnt_FLD_rxmpi_uceth_cnt                     REG_FLD(32, 0)

#define e_rxmpi_bc_cnt_FLD_rxmpi_bceth_cnt                     REG_FLD(32, 0)

#define e_rxmpi_mc_cnt_FLD_rxmpi_mceth_cnt                     REG_FLD(32, 0)

#define e_rxmpi_oam_cnt_FLD_rxmpi_oam_cnt                      REG_FLD(32, 0)

#define e_rxmpi_mpcp_cnt_FLD_rxmpi_mpcp_cnt                    REG_FLD(32, 0)

#define e_rxmpi_gate_cnt_FLD_rxmpi_gate_cnt                    REG_FLD(32, 0)

#define e_rxmpi_nrlgate_cnt_FLD_rxmpi_nrlgate_cnt              REG_FLD(32, 0)

#define e_txmbi_uceth_cnt_FLD_txmbi_uceth_cnt                  REG_FLD(32, 0)

#define e_txmbi_mceth_cnt_FLD_txmbi_bceth_cnt                  REG_FLD(16, 16)
#define e_txmbi_mceth_cnt_FLD_txmbi_mceth_cnt                  REG_FLD(16, 0)

#define e_tx_dygasp_cnt_FLD_txmbi_dygasp_cnt                   REG_FLD(16, 16)
#define e_tx_dygasp_cnt_FLD_txmpi_dygasp_cnt                   REG_FLD(16, 0)

#define e_tx_rpt_cnt_FLD_txmbi_rpt_cnt                         REG_FLD(16, 16)
#define e_tx_rpt_cnt_FLD_txmpi_rpt_cnt                         REG_FLD(16, 0)

#define e_kpoam_stat_FLD_txmbi_kpalv_cnt                       REG_FLD(16, 16)
#define e_kpoam_stat_FLD_txmpi_kpalv_cnt                       REG_FLD(16, 0)

#define e_txmpi_mpcp_cnt_FLD_txmpi_oam_cnt                     REG_FLD(16, 16)
#define e_txmpi_mpcp_cnt_FLD_txmpi_rgreq_cnt                   REG_FLD(8, 8)
#define e_txmpi_mpcp_cnt_FLD_txmpi_rgack_cnt                   REG_FLD(8, 0)

#define e_txmpi_uceth_cnt_FLD_txmpi_uceth_cnt                  REG_FLD(32, 0)

#define e_txmpi_mceth_cnt_FLD_txmpi_bceth_cnt                  REG_FLD(16, 16)
#define e_txmpi_mceth_cnt_FLD_txmpi_mceth_cnt                  REG_FLD(16, 0)

#define e_rxadv_cnt_FLD_rx_infomis_dscvgate_cnt                REG_FLD(8, 24)
#define e_rxadv_cnt_FLD_rx_llidbc_nrlgate_cnt                  REG_FLD(8, 16)
#define e_rxadv_cnt_FLD_rx_filt_dscvgate_cnt                   REG_FLD(8, 8)

#define e_rxmpi_churn_cnt_FLD_rxmpi_churn_crcok_cnt            REG_FLD(16, 16)
#define e_rxmpi_churn_cnt_FLD_rxmpi_churn_crcerr_cnt           REG_FLD(16, 0)

#define e_rxmpi_churn_info_FLD_rxmpi_churn_keyidx              REG_FLD(1, 31)
#define e_rxmpi_churn_info_FLD_rxmpi_churn_llididx             REG_FLD(6, 24)
#define e_rxmpi_churn_info_FLD_rxmpi_churncrc_keyidx           REG_FLD(1, 23)
#define e_rxmpi_churn_info_FLD_rxmpi_churncrc_llididx          REG_FLD(6, 16)
#define e_rxmpi_churn_info_FLD_rxmpi_abchurn_cnt               REG_FLD(16, 0)

#define e_desec_ok_cnt_FLD_desec_rx_ok_cnt                     REG_FLD(16, 16)
#define e_desec_ok_cnt_FLD_desec_plain_ok_cnt                  REG_FLD(16, 0)

#define e_rxmpi_p_cnt_FLD_rxmpi_p_ok_cnt                       REG_FLD(16, 16)
#define e_rxmpi_p_cnt_FLD_rxmpi_p_err_cnt                      REG_FLD(16, 0)

#define e_rxmpi_crc8err_cnt_FLD_rxmpi_crc32err_cnt             REG_FLD(16, 16)
#define e_rxmpi_crc8err_cnt_FLD_rxmpi_crc8err_cnt              REG_FLD(16, 0)

#define e_desec_rxdrop_cnt_FLD_desec_rxfifo_ovrundrop_cnt      REG_FLD(16, 16)
#define e_desec_rxdrop_cnt_FLD_desec_rxsof_ovrundrop_cnt       REG_FLD(16, 0)

#define e_desec_rxerr_cnt_FLD_desec_rx_crcerr_cnt              REG_FLD(16, 16)
#define e_desec_rxerr_cnt_FLD_desec_rx_preerr_cnt              REG_FLD(16, 0)

#define e_desec_ciphdrop_cnt_FLD_desec_rxsof_hdrdrop_cnt       REG_FLD(16, 16)
#define e_desec_ciphdrop_cnt_FLD_desec_ciph_csdrop_cnt         REG_FLD(16, 0)

#define e_desec_aeswdrop_cnt_FLD_desec_ciph_actdrop_cnt        REG_FLD(16, 16)
#define e_desec_aeswdrop_cnt_FLD_desec_aesw_csdrop_cnt         REG_FLD(16, 0)

#define e_desec_aeswdrop_cnt2_FLD_desec_aesw_ovrundrop_cnt     REG_FLD(16, 16)
#define e_desec_aeswdrop_cnt2_FLD_desec_aesw_icverrdrop_cnt    REG_FLD(16, 0)

#define e_rxmpi_drop_cnt_FLD_rxmpi_eofdrop_cnt                 REG_FLD(16, 16)
#define e_rxmpi_drop_cnt_FLD_rxmpi_fifoovrun_cnt               REG_FLD(16, 0)

#define e_rxmbi_drop_cnt_FLD_rxmbi_sofdrop_cnt                 REG_FLD(16, 16)
#define e_rxmbi_drop_cnt_FLD_rxmbi_snfdrop_cnt                 REG_FLD(16, 0)

#define e_rxmbi_drop_cnt2_FLD_rxmbi_crcerr_cnt                 REG_FLD(16, 16)
#define e_rxmbi_drop_cnt2_FLD_rxmbi_enddrop_cnt                REG_FLD(16, 0)

#define e_txmbi_err_cnt_FLD_txmbi_err_cnt                      REG_FLD(16, 0)

#define e_sts_dmy11_FLD_xe_sts_dmy11                           REG_FLD(32, 0)

#define e_sts_dmy12_FLD_xe_sts_dmy12                           REG_FLD(32, 0)

#define e_sts_dmy13_FLD_xe_sts_dmy13                           REG_FLD(32, 0)

#define e_sts_dmy14_FLD_xe_sts_dmy14                           REG_FLD(32, 0)

#define e_sts_dmy15_FLD_xe_sts_dmy15                           REG_FLD(32, 0)

#define e_sts_dmy16_FLD_xe_sts_dmy16                           REG_FLD(32, 0)

#define e_sts_dmy17_FLD_xe_sts_dmy17                           REG_FLD(32, 0)

#define e_sts_dmy18_FLD_xe_sts_dmy18                           REG_FLD(32, 0)

#define e_sts_dmy19_FLD_xe_sts_dmy19                           REG_FLD(32, 0)

#define e_sts_dmy20_FLD_xe_sts_dmy20                           REG_FLD(32, 0)

#define e_sts_dmy21_FLD_xe_sts_dmy21                           REG_FLD(32, 0)

#define e_sts_dmy22_FLD_xe_sts_dmy22                           REG_FLD(32, 0)

#define e_sts_dmy23_FLD_xe_sts_dmy23                           REG_FLD(32, 0)

#define e_sts_dmy24_FLD_xe_sts_dmy24                           REG_FLD(32, 0)

#define e_sts_dmy25_FLD_xe_sts_dmy25                           REG_FLD(32, 0)

#define e_gnt_type_stat_FLD_b2b_gnt_cnt                        REG_FLD(16, 16)
#define e_gnt_type_stat_FLD_hdn_gnt_cnt                        REG_FLD(16, 0)

#define e_gnt_pending_stat_FLD_max_gnt_pending_cnt             REG_FLD(8, 16)
#define e_gnt_pending_stat_FLD_cur_gnt_pending_cnt             REG_FLD(8, 0)

#define e_gnt_length_stat_FLD_max_gnt_length                   REG_FLD(16, 16)
#define e_gnt_length_stat_FLD_min_gnt_length                   REG_FLD(16, 0)

#define e_local_time_FLD_local_time                            REG_FLD(32, 0)

#define e_time_drft_stat_FLD_cur_time_drift_ofst               REG_FLD(5, 24)
#define e_time_drft_stat_FLD_max_time_drift_ofst               REG_FLD(5, 16)
#define e_time_drft_stat_FLD_cur_time_drift                    REG_FLD(8, 8)
#define e_time_drft_stat_FLD_max_time_drift                    REG_FLD(8, 0)

#define e_tx_timedrift_stat_FLD_tx_cur_time_drift_ofst         REG_FLD(5, 24)
#define e_tx_timedrift_stat_FLD_tx_max_time_drift_ofst         REG_FLD(5, 16)
#define e_tx_timedrift_stat_FLD_tx_cur_time_drift              REG_FLD(8, 8)
#define e_tx_timedrift_stat_FLD_tx_max_time_drift              REG_FLD(8, 0)

#define e_rxfifo_depth_stat_FLD_cur_rxfifo_depth               REG_FLD(16, 16)
#define e_rxfifo_depth_stat_FLD_max_rxfifo_depth               REG_FLD(16, 0)

#define e_sts_dmy0_FLD_xe_sts_dmy0                             REG_FLD(32, 0)

#define e_sts_dmy1_FLD_xe_sts_dmy1                             REG_FLD(32, 0)

#define e_sts_dmy2_FLD_xe_sts_dmy2                             REG_FLD(32, 0)

#define e_sts_dmy3_FLD_xe_sts_dmy3                             REG_FLD(32, 0)

#define e_sts_dmy4_FLD_xe_sts_dmy4                             REG_FLD(32, 0)

#define e_sts_dmy5_FLD_xe_sts_dmy5                             REG_FLD(32, 0)

#define e_txmbi_bufwrap_wrcnt_FLD_txmbi_bufwrap_wrerr_cnt      REG_FLD(16, 16)
#define e_txmbi_bufwrap_wrcnt_FLD_txmbi_bufwrap_wr_cnt         REG_FLD(16, 0)

#define e_txmbi_bufwrap_rdcnt_FLD_txmbi_bufwrap_rderr_cnt      REG_FLD(16, 16)
#define e_txmbi_bufwrap_rdcnt_FLD_txmbi_bufwrap_rd_cnt         REG_FLD(16, 0)

#define e_txmbi_bufwrap_rdcnt2_FLD_txmbi_bufwrap_rddmy_cnt     REG_FLD(16, 16)

#define e_glb_cfg_GET_txoam_favor(reg32)                       REG_FLD_GET(e_glb_cfg_FLD_txoam_favor, (reg32))
#define e_glb_cfg_GET_burst_en_dly(reg32)                      REG_FLD_GET(e_glb_cfg_FLD_burst_en_dly, (reg32))
#define e_glb_cfg_GET_discv_burst_en(reg32)                    REG_FLD_GET(e_glb_cfg_FLD_discv_burst_en, (reg32))
#define e_glb_cfg_GET_mpcp_fwd(reg32)                          REG_FLD_GET(e_glb_cfg_FLD_mpcp_fwd, (reg32))
#define e_glb_cfg_GET_bcst_llid_m1_drop(reg32)                 REG_FLD_GET(e_glb_cfg_FLD_bcst_llid_m1_drop, (reg32))
#define e_glb_cfg_GET_bcst_llid_m0_drop(reg32)                 REG_FLD_GET(e_glb_cfg_FLD_bcst_llid_m0_drop, (reg32))
#define e_glb_cfg_GET_mcst_llid_drop(reg32)                    REG_FLD_GET(e_glb_cfg_FLD_mcst_llid_drop, (reg32))
#define e_glb_cfg_GET_allucst_llid_fwd(reg32)                  REG_FLD_GET(e_glb_cfg_FLD_allucst_llid_fwd, (reg32))
#define e_glb_cfg_GET_fcs_err_fwd(reg32)                       REG_FLD_GET(e_glb_cfg_FLD_fcs_err_fwd, (reg32))
#define e_glb_cfg_GET_llid_crc8_err_fwd(reg32)                 REG_FLD_GET(e_glb_cfg_FLD_llid_crc8_err_fwd, (reg32))
#define e_glb_cfg_GET_rxmpi_stop(reg32)                        REG_FLD_GET(e_glb_cfg_FLD_rxmpi_stop, (reg32))
#define e_glb_cfg_GET_txmpi_stop(reg32)                        REG_FLD_GET(e_glb_cfg_FLD_txmpi_stop, (reg32))
#define e_glb_cfg_GET_phy_pwr_down(reg32)                      REG_FLD_GET(e_glb_cfg_FLD_phy_pwr_down, (reg32))
#define e_glb_cfg_GET_rx_nml_gate_fwd(reg32)                   REG_FLD_GET(e_glb_cfg_FLD_rx_nml_gate_fwd, (reg32))
#define e_glb_cfg_GET_rxmbi_stop(reg32)                        REG_FLD_GET(e_glb_cfg_FLD_rxmbi_stop, (reg32))
#define e_glb_cfg_GET_txmbi_stop(reg32)                        REG_FLD_GET(e_glb_cfg_FLD_txmbi_stop, (reg32))
#define e_glb_cfg_GET_chk_all_gnt_mode(reg32)                  REG_FLD_GET(e_glb_cfg_FLD_chk_all_gnt_mode, (reg32))
#define e_glb_cfg_GET_mpcp_done_set(reg32)                     REG_FLD_GET(e_glb_cfg_FLD_mpcp_done_set, (reg32))
#define e_glb_cfg_GET_epon_oam_cal_in_eth(reg32)               REG_FLD_GET(e_glb_cfg_FLD_epon_oam_cal_in_eth, (reg32))
#define e_glb_cfg_GET_rpt_txpri_ctrl(reg32)                    REG_FLD_GET(e_glb_cfg_FLD_rpt_txpri_ctrl, (reg32))

#define e_glb_cfg2_GET_llidnum_sel(reg32)                      REG_FLD_GET(e_glb_cfg2_FLD_llidnum_sel, (reg32))
#define e_glb_cfg2_GET_rxdv_tmout_en(reg32)                    REG_FLD_GET(e_glb_cfg2_FLD_rxdv_tmout_en, (reg32))
#define e_glb_cfg2_GET_txmbi_nack_enden(reg32)                 REG_FLD_GET(e_glb_cfg2_FLD_txmbi_nack_enden, (reg32))
#define e_glb_cfg2_GET_loctime_mtd(reg32)                      REG_FLD_GET(e_glb_cfg2_FLD_loctime_mtd, (reg32))
#define e_glb_cfg2_GET_rgreq_lsrtime_mask(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_rgreq_lsrtime_mask, (reg32))
#define e_glb_cfg2_GET_crc_cal_in_bytecnt(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_crc_cal_in_bytecnt, (reg32))
#define e_glb_cfg2_GET_u10g_txmode(reg32)                      REG_FLD_GET(e_glb_cfg2_FLD_u10g_txmode, (reg32))
#define e_glb_cfg2_GET_eth_cal_in_bytecnt(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_eth_cal_in_bytecnt, (reg32))
#define e_glb_cfg2_GET_snf_cal_in_bytecnt(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_snf_cal_in_bytecnt, (reg32))
#define e_glb_cfg2_GET_oam_cal_in_bytecnt(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_oam_cal_in_bytecnt, (reg32))
#define e_glb_cfg2_GET_dscvgate_filt(reg32)                    REG_FLD_GET(e_glb_cfg2_FLD_dscvgate_filt, (reg32))
#define e_glb_cfg2_GET_gntlen_stat_widscv(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_gntlen_stat_widscv, (reg32))
#define e_glb_cfg2_GET_tdrift_loctupd_dis(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_tdrift_loctupd_dis, (reg32))
#define e_glb_cfg2_GET_dscvgate_infochk_dis(reg32)             REG_FLD_GET(e_glb_cfg2_FLD_dscvgate_infochk_dis, (reg32))

#define e_glb_sts1_GET_xepon_type(reg32)                       REG_FLD_GET(e_glb_sts1_FLD_xepon_type, (reg32))
#define e_glb_sts1_GET_rxmpi_stop_sts(reg32)                   REG_FLD_GET(e_glb_sts1_FLD_rxmpi_stop_sts, (reg32))
#define e_glb_sts1_GET_txmpi_stop_sts(reg32)                   REG_FLD_GET(e_glb_sts1_FLD_txmpi_stop_sts, (reg32))
#define e_glb_sts1_GET_rxmbi_stop_sts(reg32)                   REG_FLD_GET(e_glb_sts1_FLD_rxmbi_stop_sts, (reg32))
#define e_glb_sts1_GET_txmbi_stop_sts(reg32)                   REG_FLD_GET(e_glb_sts1_FLD_txmbi_stop_sts, (reg32))
#define e_glb_sts1_GET_xepon_glb_cfg_sts(reg32)                REG_FLD_GET(e_glb_sts1_FLD_xepon_glb_cfg_sts, (reg32))

#define e_glb_sts2_GET_xepon_glb_cfg2_sts(reg32)               REG_FLD_GET(e_glb_sts2_FLD_xepon_glb_cfg2_sts, (reg32))

#define e_int_status_GET_us_keychg_int(reg32)                  REG_FLD_GET(e_int_status_FLD_us_keychg_int, (reg32))
#define e_int_status_GET_ds_keymis_int(reg32)                  REG_FLD_GET(e_int_status_FLD_ds_keymis_int, (reg32))
#define e_int_status_GET_ds_keychg_int(reg32)                  REG_FLD_GET(e_int_status_FLD_ds_keychg_int, (reg32))
#define e_int_status_GET_sniff_fifo_ovrun_int(reg32)           REG_FLD_GET(e_int_status_FLD_sniff_fifo_ovrun_int, (reg32))
#define e_int_status_GET_reg_ack_done_int(reg32)               REG_FLD_GET(e_int_status_FLD_reg_ack_done_int, (reg32))
#define e_int_status_GET_reg_req_done_int(reg32)               REG_FLD_GET(e_int_status_FLD_reg_req_done_int, (reg32))
#define e_int_status_GET_reorder1_gnt_int(reg32)               REG_FLD_GET(e_int_status_FLD_reorder1_gnt_int, (reg32))
#define e_int_status_GET_b2b_gnt_int(reg32)                    REG_FLD_GET(e_int_status_FLD_b2b_gnt_int, (reg32))
#define e_int_status_GET_hidn_gnt_int(reg32)                   REG_FLD_GET(e_int_status_FLD_hidn_gnt_int, (reg32))
#define e_int_status_GET_ps_early_wakeup_int(reg32)            REG_FLD_GET(e_int_status_FLD_ps_early_wakeup_int, (reg32))
#define e_int_status_GET_rx_sleep_allow_int(reg32)             REG_FLD_GET(e_int_status_FLD_rx_sleep_allow_int, (reg32))
#define e_int_status_GET_ps_wakeup_int(reg32)                  REG_FLD_GET(e_int_status_FLD_ps_wakeup_int, (reg32))
#define e_int_status_GET_ps_sleep_int(reg32)                   REG_FLD_GET(e_int_status_FLD_ps_sleep_int, (reg32))
#define e_int_status_GET_txfifo_udrun_int(reg32)               REG_FLD_GET(e_int_status_FLD_txfifo_udrun_int, (reg32))
#define e_int_status_GET_rpt_overintvl_int(reg32)              REG_FLD_GET(e_int_status_FLD_rpt_overintvl_int, (reg32))
#define e_int_status_GET_mpcp_timeout_int(reg32)               REG_FLD_GET(e_int_status_FLD_mpcp_timeout_int, (reg32))
#define e_int_status_GET_timedrft_int(reg32)                   REG_FLD_GET(e_int_status_FLD_timedrft_int, (reg32))
#define e_int_status_GET_tod_1pps_int(reg32)                   REG_FLD_GET(e_int_status_FLD_tod_1pps_int, (reg32))
#define e_int_status_GET_tod_updt_int(reg32)                   REG_FLD_GET(e_int_status_FLD_tod_updt_int, (reg32))
#define e_int_status_GET_ptp_msg_tx_int(reg32)                 REG_FLD_GET(e_int_status_FLD_ptp_msg_tx_int, (reg32))
#define e_int_status_GET_gnt_buf_ovrrun_int(reg32)             REG_FLD_GET(e_int_status_FLD_gnt_buf_ovrrun_int, (reg32))
#define e_int_status_GET_llid7_rcv_rgst_int(reg32)             REG_FLD_GET(e_int_status_FLD_llid7_rcv_rgst_int, (reg32))
#define e_int_status_GET_llid6_rcv_rgst_int(reg32)             REG_FLD_GET(e_int_status_FLD_llid6_rcv_rgst_int, (reg32))
#define e_int_status_GET_llid5_rcv_rgst_int(reg32)             REG_FLD_GET(e_int_status_FLD_llid5_rcv_rgst_int, (reg32))
#define e_int_status_GET_llid4_rcv_rgst_int(reg32)             REG_FLD_GET(e_int_status_FLD_llid4_rcv_rgst_int, (reg32))
#define e_int_status_GET_llid3_rcv_rgst_int(reg32)             REG_FLD_GET(e_int_status_FLD_llid3_rcv_rgst_int, (reg32))
#define e_int_status_GET_llid2_rcv_rgst_int(reg32)             REG_FLD_GET(e_int_status_FLD_llid2_rcv_rgst_int, (reg32))
#define e_int_status_GET_llid1_rcv_rgst_int(reg32)             REG_FLD_GET(e_int_status_FLD_llid1_rcv_rgst_int, (reg32))
#define e_int_status_GET_llid0_rcv_rgst_int(reg32)             REG_FLD_GET(e_int_status_FLD_llid0_rcv_rgst_int, (reg32))
#define e_int_status_GET_rcv_dscvry_gate_int(reg32)            REG_FLD_GET(e_int_status_FLD_rcv_dscvry_gate_int, (reg32))

#define e_int_en_GET_us_keychg_en(reg32)                       REG_FLD_GET(e_int_en_FLD_us_keychg_en, (reg32))
#define e_int_en_GET_ds_keymis_en(reg32)                       REG_FLD_GET(e_int_en_FLD_ds_keymis_en, (reg32))
#define e_int_en_GET_ds_keychg_en(reg32)                       REG_FLD_GET(e_int_en_FLD_ds_keychg_en, (reg32))
#define e_int_en_GET_sniff_fifo_ovrun_en(reg32)                REG_FLD_GET(e_int_en_FLD_sniff_fifo_ovrun_en, (reg32))
#define e_int_en_GET_reg_ack_done_en(reg32)                    REG_FLD_GET(e_int_en_FLD_reg_ack_done_en, (reg32))
#define e_int_en_GET_reg_req_done_en(reg32)                    REG_FLD_GET(e_int_en_FLD_reg_req_done_en, (reg32))
#define e_int_en_GET_reorder1_gnt_en(reg32)                    REG_FLD_GET(e_int_en_FLD_reorder1_gnt_en, (reg32))
#define e_int_en_GET_b2b_gnt_en(reg32)                         REG_FLD_GET(e_int_en_FLD_b2b_gnt_en, (reg32))
#define e_int_en_GET_hidn_gnt_en(reg32)                        REG_FLD_GET(e_int_en_FLD_hidn_gnt_en, (reg32))
#define e_int_en_GET_ps_early_wakeup_en(reg32)                 REG_FLD_GET(e_int_en_FLD_ps_early_wakeup_en, (reg32))
#define e_int_en_GET_rx_sleep_allow_en(reg32)                  REG_FLD_GET(e_int_en_FLD_rx_sleep_allow_en, (reg32))
#define e_int_en_GET_ps_wakeup_en(reg32)                       REG_FLD_GET(e_int_en_FLD_ps_wakeup_en, (reg32))
#define e_int_en_GET_ps_sleep_en(reg32)                        REG_FLD_GET(e_int_en_FLD_ps_sleep_en, (reg32))
#define e_int_en_GET_txfifo_udrun_en(reg32)                    REG_FLD_GET(e_int_en_FLD_txfifo_udrun_en, (reg32))
#define e_int_en_GET_rpt_overintvl_en(reg32)                   REG_FLD_GET(e_int_en_FLD_rpt_overintvl_en, (reg32))
#define e_int_en_GET_mpcp_timeout_en(reg32)                    REG_FLD_GET(e_int_en_FLD_mpcp_timeout_en, (reg32))
#define e_int_en_GET_timedrft_en(reg32)                        REG_FLD_GET(e_int_en_FLD_timedrft_en, (reg32))
#define e_int_en_GET_tod_1pps_en(reg32)                        REG_FLD_GET(e_int_en_FLD_tod_1pps_en, (reg32))
#define e_int_en_GET_tod_updt_en(reg32)                        REG_FLD_GET(e_int_en_FLD_tod_updt_en, (reg32))
#define e_int_en_GET_ptp_msg_tx_en(reg32)                      REG_FLD_GET(e_int_en_FLD_ptp_msg_tx_en, (reg32))
#define e_int_en_GET_gnt_buf_ovrrun_en(reg32)                  REG_FLD_GET(e_int_en_FLD_gnt_buf_ovrrun_en, (reg32))
#define e_int_en_GET_llid7_rcv_rgst_en(reg32)                  REG_FLD_GET(e_int_en_FLD_llid7_rcv_rgst_en, (reg32))
#define e_int_en_GET_llid6_rcv_rgst_en(reg32)                  REG_FLD_GET(e_int_en_FLD_llid6_rcv_rgst_en, (reg32))
#define e_int_en_GET_llid5_rcv_rgst_en(reg32)                  REG_FLD_GET(e_int_en_FLD_llid5_rcv_rgst_en, (reg32))
#define e_int_en_GET_llid4_rcv_rgst_en(reg32)                  REG_FLD_GET(e_int_en_FLD_llid4_rcv_rgst_en, (reg32))
#define e_int_en_GET_llid3_rcv_rgst_en(reg32)                  REG_FLD_GET(e_int_en_FLD_llid3_rcv_rgst_en, (reg32))
#define e_int_en_GET_llid2_rcv_rgst_en(reg32)                  REG_FLD_GET(e_int_en_FLD_llid2_rcv_rgst_en, (reg32))
#define e_int_en_GET_llid1_rcv_rgst_en(reg32)                  REG_FLD_GET(e_int_en_FLD_llid1_rcv_rgst_en, (reg32))
#define e_int_en_GET_llid0_rcv_rgst_en(reg32)                  REG_FLD_GET(e_int_en_FLD_llid0_rcv_rgst_en, (reg32))
#define e_int_en_GET_rcv_dscvry_gate_en(reg32)                 REG_FLD_GET(e_int_en_FLD_rcv_dscvry_gate_en, (reg32))

#define e_int_sts2_GET_tx_latestart_int(reg32)                 REG_FLD_GET(e_int_sts2_FLD_tx_latestart_int, (reg32))
#define e_int_sts2_GET_rcv_ifchk_err_int(reg32)                REG_FLD_GET(e_int_sts2_FLD_rcv_ifchk_err_int, (reg32))
#define e_int_sts2_GET_rx_rgst_exc_int(reg32)                  REG_FLD_GET(e_int_sts2_FLD_rx_rgst_exc_int, (reg32))
#define e_int_sts2_GET_rcv_len_err_int(reg32)                  REG_FLD_GET(e_int_sts2_FLD_rcv_len_err_int, (reg32))
#define e_int_sts2_GET_schfch_nopkt_int(reg32)                 REG_FLD_GET(e_int_sts2_FLD_schfch_nopkt_int, (reg32))
#define e_int_sts2_GET_rxfifo_ovrun_int(reg32)                 REG_FLD_GET(e_int_sts2_FLD_rxfifo_ovrun_int, (reg32))
#define e_int_sts2_GET_txmpi_pldenneq_int(reg32)               REG_FLD_GET(e_int_sts2_FLD_txmpi_pldenneq_int, (reg32))
#define e_int_sts2_GET_schfch_timeo_int(reg32)                 REG_FLD_GET(e_int_sts2_FLD_schfch_timeo_int, (reg32))
#define e_int_sts2_GET_schgnt_inv_int(reg32)                   REG_FLD_GET(e_int_sts2_FLD_schgnt_inv_int, (reg32))
#define e_int_sts2_GET_rcvgnt_infofail_int(reg32)              REG_FLD_GET(e_int_sts2_FLD_rcvgnt_infofail_int, (reg32))
#define e_int_sts2_GET_rcv_eofdrop_int(reg32)                  REG_FLD_GET(e_int_sts2_FLD_rcv_eofdrop_int, (reg32))
#define e_int_sts2_GET_rcv_crcerr_int(reg32)                   REG_FLD_GET(e_int_sts2_FLD_rcv_crcerr_int, (reg32))
#define e_int_sts2_GET_rcv_excgate_int(reg32)                  REG_FLD_GET(e_int_sts2_FLD_rcv_excgate_int, (reg32))

#define e_int_en2_GET_tx_latestart_en(reg32)                   REG_FLD_GET(e_int_en2_FLD_tx_latestart_en, (reg32))
#define e_int_en2_GET_rcv_ifchk_err_en(reg32)                  REG_FLD_GET(e_int_en2_FLD_rcv_ifchk_err_en, (reg32))
#define e_int_en2_GET_rx_rgst_exc_en(reg32)                    REG_FLD_GET(e_int_en2_FLD_rx_rgst_exc_en, (reg32))
#define e_int_en2_GET_rcv_len_err_en(reg32)                    REG_FLD_GET(e_int_en2_FLD_rcv_len_err_en, (reg32))
#define e_int_en2_GET_schfch_nopkt_en(reg32)                   REG_FLD_GET(e_int_en2_FLD_schfch_nopkt_en, (reg32))
#define e_int_en2_GET_rxfifo_ovrun_en(reg32)                   REG_FLD_GET(e_int_en2_FLD_rxfifo_ovrun_en, (reg32))
#define e_int_en2_GET_txmpi_pldenneq_en(reg32)                 REG_FLD_GET(e_int_en2_FLD_txmpi_pldenneq_en, (reg32))
#define e_int_en2_GET_schfch_timeo_en(reg32)                   REG_FLD_GET(e_int_en2_FLD_schfch_timeo_en, (reg32))
#define e_int_en2_GET_schgnt_inv_en(reg32)                     REG_FLD_GET(e_int_en2_FLD_schgnt_inv_en, (reg32))
#define e_int_en2_GET_rcvgnt_infofail_en(reg32)                REG_FLD_GET(e_int_en2_FLD_rcvgnt_infofail_en, (reg32))
#define e_int_en2_GET_rcv_eofdrop_en(reg32)                    REG_FLD_GET(e_int_en2_FLD_rcv_eofdrop_en, (reg32))
#define e_int_en2_GET_rcv_crcerr_en(reg32)                     REG_FLD_GET(e_int_en2_FLD_rcv_crcerr_en, (reg32))
#define e_int_en2_GET_rcv_excgate_en(reg32)                    REG_FLD_GET(e_int_en2_FLD_rcv_excgate_en, (reg32))

#define e_int_sts3_GET_llid31_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid31_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid30_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid30_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid29_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid29_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid28_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid28_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid27_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid27_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid26_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid26_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid25_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid25_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid24_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid24_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid23_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid23_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid22_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid22_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid21_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid21_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid20_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid20_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid19_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid19_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid18_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid18_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid17_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid17_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid16_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid16_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid15_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid15_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid14_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid14_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid13_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid13_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid12_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid12_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid11_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid11_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid10_rcv_rgst_int(reg32)              REG_FLD_GET(e_int_sts3_FLD_llid10_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid9_rcv_rgst_int(reg32)               REG_FLD_GET(e_int_sts3_FLD_llid9_rcv_rgst_int, (reg32))
#define e_int_sts3_GET_llid8_rcv_rgst_int(reg32)               REG_FLD_GET(e_int_sts3_FLD_llid8_rcv_rgst_int, (reg32))

#define e_int_en3_GET_llid31_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid31_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid30_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid30_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid29_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid29_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid28_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid28_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid27_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid27_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid26_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid26_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid25_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid25_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid24_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid24_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid23_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid23_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid22_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid22_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid21_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid21_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid20_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid20_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid19_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid19_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid18_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid18_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid17_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid17_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid16_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid16_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid15_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid15_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid14_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid14_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid13_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid13_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid12_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid12_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid11_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid11_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid10_rcv_rgst_en(reg32)                REG_FLD_GET(e_int_en3_FLD_llid10_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid9_rcv_rgst_en(reg32)                 REG_FLD_GET(e_int_en3_FLD_llid9_rcv_rgst_en, (reg32))
#define e_int_en3_GET_llid8_rcv_rgst_en(reg32)                 REG_FLD_GET(e_int_en3_FLD_llid8_rcv_rgst_en, (reg32))

#define e_exc_sts_GET_encfrm_alierr_hit(reg32)                 REG_FLD_GET(e_exc_sts_FLD_encfrm_alierr_hit, (reg32))
#define e_exc_sts_GET_schgnt_tmout_hit(reg32)                  REG_FLD_GET(e_exc_sts_FLD_schgnt_tmout_hit, (reg32))
#define e_exc_sts_GET_rx_rgst_dergst_hit(reg32)                REG_FLD_GET(e_exc_sts_FLD_rx_rgst_dergst_hit, (reg32))
#define e_exc_sts_GET_rx_rgst_nack_hit(reg32)                  REG_FLD_GET(e_exc_sts_FLD_rx_rgst_nack_hit, (reg32))
#define e_exc_sts_GET_rx_slderr_hit(reg32)                     REG_FLD_GET(e_exc_sts_FLD_rx_slderr_hit, (reg32))
#define e_exc_sts_GET_rx_nodv_tmo_hit(reg32)                   REG_FLD_GET(e_exc_sts_FLD_rx_nodv_tmo_hit, (reg32))
#define e_exc_sts_GET_rx_noeof_hit(reg32)                      REG_FLD_GET(e_exc_sts_FLD_rx_noeof_hit, (reg32))
#define e_exc_sts_GET_snf_pktcnt_err_hit(reg32)                REG_FLD_GET(e_exc_sts_FLD_snf_pktcnt_err_hit, (reg32))
#define e_exc_sts_GET_tx_gntst_nosog_hit(reg32)                REG_FLD_GET(e_exc_sts_FLD_tx_gntst_nosog_hit, (reg32))
#define e_exc_sts_GET_tx_gntst_fifound_hit(reg32)              REG_FLD_GET(e_exc_sts_FLD_tx_gntst_fifound_hit, (reg32))
#define e_exc_sts_GET_tx_gntph_fifound_hit(reg32)              REG_FLD_GET(e_exc_sts_FLD_tx_gntph_fifound_hit, (reg32))
#define e_exc_sts_GET_tx_gntph_exceed_hit(reg32)               REG_FLD_GET(e_exc_sts_FLD_tx_gntph_exceed_hit, (reg32))
#define e_exc_sts_GET_schgnt_stinv_hit(reg32)                  REG_FLD_GET(e_exc_sts_FLD_schgnt_stinv_hit, (reg32))
#define e_exc_sts_GET_schgnt_idxinv_hit(reg32)                 REG_FLD_GET(e_exc_sts_FLD_schgnt_idxinv_hit, (reg32))
#define e_exc_sts_GET_decrpt_sofeof_col_hit(reg32)             REG_FLD_GET(e_exc_sts_FLD_decrpt_sofeof_col_hit, (reg32))
#define e_exc_sts_GET_rcv_len_long_hit(reg32)                  REG_FLD_GET(e_exc_sts_FLD_rcv_len_long_hit, (reg32))
#define e_exc_sts_GET_rcv_len_short_hit(reg32)                 REG_FLD_GET(e_exc_sts_FLD_rcv_len_short_hit, (reg32))
#define e_exc_sts_GET_rcv_mpcp_crcerr_hit(reg32)               REG_FLD_GET(e_exc_sts_FLD_rcv_mpcp_crcerr_hit, (reg32))
#define e_exc_sts_GET_rcv_crc32err_hit(reg32)                  REG_FLD_GET(e_exc_sts_FLD_rcv_crc32err_hit, (reg32))
#define e_exc_sts_GET_rcv_crc8err_hit(reg32)                   REG_FLD_GET(e_exc_sts_FLD_rcv_crc8err_hit, (reg32))
#define e_exc_sts_GET_rcv_demacs_flagexc_hit(reg32)            REG_FLD_GET(e_exc_sts_FLD_rcv_demacs_flagexc_hit, (reg32))
#define e_exc_sts_GET_rcv_nrlgate_llidbc_hit(reg32)            REG_FLD_GET(e_exc_sts_FLD_rcv_nrlgate_llidbc_hit, (reg32))
#define e_exc_sts_GET_rcv_dscvgate_filt_hit(reg32)             REG_FLD_GET(e_exc_sts_FLD_rcv_dscvgate_filt_hit, (reg32))
#define e_exc_sts_GET_rcv_dscvgate_infomis_hit(reg32)          REG_FLD_GET(e_exc_sts_FLD_rcv_dscvgate_infomis_hit, (reg32))

#define e_rpt_timout_GET_llid31_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid31_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid30_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid30_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid29_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid29_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid28_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid28_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid27_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid27_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid26_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid26_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid25_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid25_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid24_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid24_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid23_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid23_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid22_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid22_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid21_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid21_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid20_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid20_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid19_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid19_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid18_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid18_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid17_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid17_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid16_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid16_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid15_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid15_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid14_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid14_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid13_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid13_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid12_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid12_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid11_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid11_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid10_rpt_tmo(reg32)                 REG_FLD_GET(e_rpt_timout_FLD_llid10_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid9_rpt_tmo(reg32)                  REG_FLD_GET(e_rpt_timout_FLD_llid9_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid8_rpt_tmo(reg32)                  REG_FLD_GET(e_rpt_timout_FLD_llid8_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid7_rpt_tmo(reg32)                  REG_FLD_GET(e_rpt_timout_FLD_llid7_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid6_rpt_tmo(reg32)                  REG_FLD_GET(e_rpt_timout_FLD_llid6_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid5_rpt_tmo(reg32)                  REG_FLD_GET(e_rpt_timout_FLD_llid5_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid4_rpt_tmo(reg32)                  REG_FLD_GET(e_rpt_timout_FLD_llid4_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid3_rpt_tmo(reg32)                  REG_FLD_GET(e_rpt_timout_FLD_llid3_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid2_rpt_tmo(reg32)                  REG_FLD_GET(e_rpt_timout_FLD_llid2_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid1_rpt_tmo(reg32)                  REG_FLD_GET(e_rpt_timout_FLD_llid1_rpt_tmo, (reg32))
#define e_rpt_timout_GET_llid0_rpt_tmo(reg32)                  REG_FLD_GET(e_rpt_timout_FLD_llid0_rpt_tmo, (reg32))

#define e_mpcp_timout_GET_llid31_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid31_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid30_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid30_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid29_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid29_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid28_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid28_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid27_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid27_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid26_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid26_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid25_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid25_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid24_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid24_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid23_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid23_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid22_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid22_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid21_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid21_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid20_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid20_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid19_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid19_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid18_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid18_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid17_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid17_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid16_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid16_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid15_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid15_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid14_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid14_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid13_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid13_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid12_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid12_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid11_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid11_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid10_mpcp_tmo(reg32)               REG_FLD_GET(e_mpcp_timout_FLD_llid10_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid9_mpcp_tmo(reg32)                REG_FLD_GET(e_mpcp_timout_FLD_llid9_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid8_mpcp_tmo(reg32)                REG_FLD_GET(e_mpcp_timout_FLD_llid8_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid7_mpcp_tmo(reg32)                REG_FLD_GET(e_mpcp_timout_FLD_llid7_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid6_mpcp_tmo(reg32)                REG_FLD_GET(e_mpcp_timout_FLD_llid6_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid5_mpcp_tmo(reg32)                REG_FLD_GET(e_mpcp_timout_FLD_llid5_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid4_mpcp_tmo(reg32)                REG_FLD_GET(e_mpcp_timout_FLD_llid4_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid3_mpcp_tmo(reg32)                REG_FLD_GET(e_mpcp_timout_FLD_llid3_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid2_mpcp_tmo(reg32)                REG_FLD_GET(e_mpcp_timout_FLD_llid2_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid1_mpcp_tmo(reg32)                REG_FLD_GET(e_mpcp_timout_FLD_llid1_mpcp_tmo, (reg32))
#define e_mpcp_timout_GET_llid0_mpcp_tmo(reg32)                REG_FLD_GET(e_mpcp_timout_FLD_llid0_mpcp_tmo, (reg32))

#define e_ds_keychg_GET_llid31_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid31_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid30_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid30_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid29_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid29_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid28_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid28_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid27_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid27_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid26_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid26_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid25_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid25_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid24_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid24_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid23_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid23_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid22_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid22_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid21_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid21_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid20_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid20_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid19_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid19_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid18_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid18_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid17_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid17_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid16_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid16_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid15_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid15_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid14_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid14_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid13_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid13_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid12_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid12_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid11_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid11_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid10_ds_keychg(reg32)                REG_FLD_GET(e_ds_keychg_FLD_llid10_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid9_ds_keychg(reg32)                 REG_FLD_GET(e_ds_keychg_FLD_llid9_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid8_ds_keychg(reg32)                 REG_FLD_GET(e_ds_keychg_FLD_llid8_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid7_ds_keychg(reg32)                 REG_FLD_GET(e_ds_keychg_FLD_llid7_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid6_ds_keychg(reg32)                 REG_FLD_GET(e_ds_keychg_FLD_llid6_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid5_ds_keychg(reg32)                 REG_FLD_GET(e_ds_keychg_FLD_llid5_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid4_ds_keychg(reg32)                 REG_FLD_GET(e_ds_keychg_FLD_llid4_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid3_ds_keychg(reg32)                 REG_FLD_GET(e_ds_keychg_FLD_llid3_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid2_ds_keychg(reg32)                 REG_FLD_GET(e_ds_keychg_FLD_llid2_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid1_ds_keychg(reg32)                 REG_FLD_GET(e_ds_keychg_FLD_llid1_ds_keychg, (reg32))
#define e_ds_keychg_GET_llid0_ds_keychg(reg32)                 REG_FLD_GET(e_ds_keychg_FLD_llid0_ds_keychg, (reg32))

#define e_ds_keychg2_GET_llid51_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid51_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid50_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid50_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid49_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid49_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid48_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid48_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid47_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid47_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid46_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid46_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid45_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid45_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid44_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid44_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid43_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid43_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid42_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid42_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid41_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid41_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid40_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid40_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid39_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid39_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid38_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid38_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid37_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid37_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid36_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid36_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid35_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid35_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid34_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid34_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid33_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid33_ds_keychg, (reg32))
#define e_ds_keychg2_GET_llid32_ds_keychg(reg32)               REG_FLD_GET(e_ds_keychg2_FLD_llid32_ds_keychg, (reg32))

#define e_ds_keymis_GET_llid31_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid31_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid30_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid30_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid29_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid29_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid28_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid28_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid27_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid27_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid26_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid26_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid25_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid25_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid24_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid24_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid23_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid23_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid22_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid22_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid21_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid21_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid20_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid20_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid19_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid19_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid18_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid18_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid17_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid17_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid16_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid16_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid15_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid15_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid14_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid14_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid13_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid13_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid12_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid12_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid11_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid11_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid10_ds_keymis(reg32)                REG_FLD_GET(e_ds_keymis_FLD_llid10_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid9_ds_keymis(reg32)                 REG_FLD_GET(e_ds_keymis_FLD_llid9_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid8_ds_keymis(reg32)                 REG_FLD_GET(e_ds_keymis_FLD_llid8_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid7_ds_keymis(reg32)                 REG_FLD_GET(e_ds_keymis_FLD_llid7_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid6_ds_keymis(reg32)                 REG_FLD_GET(e_ds_keymis_FLD_llid6_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid5_ds_keymis(reg32)                 REG_FLD_GET(e_ds_keymis_FLD_llid5_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid4_ds_keymis(reg32)                 REG_FLD_GET(e_ds_keymis_FLD_llid4_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid3_ds_keymis(reg32)                 REG_FLD_GET(e_ds_keymis_FLD_llid3_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid2_ds_keymis(reg32)                 REG_FLD_GET(e_ds_keymis_FLD_llid2_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid1_ds_keymis(reg32)                 REG_FLD_GET(e_ds_keymis_FLD_llid1_ds_keymis, (reg32))
#define e_ds_keymis_GET_llid0_ds_keymis(reg32)                 REG_FLD_GET(e_ds_keymis_FLD_llid0_ds_keymis, (reg32))

#define e_ds_keymis2_GET_llid51_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid51_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid50_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid50_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid49_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid49_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid48_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid48_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid47_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid47_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid46_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid46_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid45_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid45_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid44_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid44_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid43_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid43_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid42_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid42_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid41_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid41_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid40_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid40_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid39_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid39_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid38_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid38_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid37_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid37_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid36_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid36_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid35_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid35_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid34_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid34_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid33_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid33_ds_keymis, (reg32))
#define e_ds_keymis2_GET_llid32_ds_keymis(reg32)               REG_FLD_GET(e_ds_keymis2_FLD_llid32_ds_keymis, (reg32))

#define e_us_keychg_GET_llid31_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid31_us_keychg, (reg32))
#define e_us_keychg_GET_llid30_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid30_us_keychg, (reg32))
#define e_us_keychg_GET_llid29_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid29_us_keychg, (reg32))
#define e_us_keychg_GET_llid28_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid28_us_keychg, (reg32))
#define e_us_keychg_GET_llid27_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid27_us_keychg, (reg32))
#define e_us_keychg_GET_llid26_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid26_us_keychg, (reg32))
#define e_us_keychg_GET_llid25_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid25_us_keychg, (reg32))
#define e_us_keychg_GET_llid24_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid24_us_keychg, (reg32))
#define e_us_keychg_GET_llid23_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid23_us_keychg, (reg32))
#define e_us_keychg_GET_llid22_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid22_us_keychg, (reg32))
#define e_us_keychg_GET_llid21_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid21_us_keychg, (reg32))
#define e_us_keychg_GET_llid20_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid20_us_keychg, (reg32))
#define e_us_keychg_GET_llid19_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid19_us_keychg, (reg32))
#define e_us_keychg_GET_llid18_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid18_us_keychg, (reg32))
#define e_us_keychg_GET_llid17_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid17_us_keychg, (reg32))
#define e_us_keychg_GET_llid16_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid16_us_keychg, (reg32))
#define e_us_keychg_GET_llid15_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid15_us_keychg, (reg32))
#define e_us_keychg_GET_llid14_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid14_us_keychg, (reg32))
#define e_us_keychg_GET_llid13_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid13_us_keychg, (reg32))
#define e_us_keychg_GET_llid12_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid12_us_keychg, (reg32))
#define e_us_keychg_GET_llid11_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid11_us_keychg, (reg32))
#define e_us_keychg_GET_llid10_us_keychg(reg32)                REG_FLD_GET(e_us_keychg_FLD_llid10_us_keychg, (reg32))
#define e_us_keychg_GET_llid9_us_keychg(reg32)                 REG_FLD_GET(e_us_keychg_FLD_llid9_us_keychg, (reg32))
#define e_us_keychg_GET_llid8_us_keychg(reg32)                 REG_FLD_GET(e_us_keychg_FLD_llid8_us_keychg, (reg32))
#define e_us_keychg_GET_llid7_us_keychg(reg32)                 REG_FLD_GET(e_us_keychg_FLD_llid7_us_keychg, (reg32))
#define e_us_keychg_GET_llid6_us_keychg(reg32)                 REG_FLD_GET(e_us_keychg_FLD_llid6_us_keychg, (reg32))
#define e_us_keychg_GET_llid5_us_keychg(reg32)                 REG_FLD_GET(e_us_keychg_FLD_llid5_us_keychg, (reg32))
#define e_us_keychg_GET_llid4_us_keychg(reg32)                 REG_FLD_GET(e_us_keychg_FLD_llid4_us_keychg, (reg32))
#define e_us_keychg_GET_llid3_us_keychg(reg32)                 REG_FLD_GET(e_us_keychg_FLD_llid3_us_keychg, (reg32))
#define e_us_keychg_GET_llid2_us_keychg(reg32)                 REG_FLD_GET(e_us_keychg_FLD_llid2_us_keychg, (reg32))
#define e_us_keychg_GET_llid1_us_keychg(reg32)                 REG_FLD_GET(e_us_keychg_FLD_llid1_us_keychg, (reg32))
#define e_us_keychg_GET_llid0_us_keychg(reg32)                 REG_FLD_GET(e_us_keychg_FLD_llid0_us_keychg, (reg32))

#define e_cfg_dmy0_GET_xe_cfg_dmy0(reg32)                      REG_FLD_GET(e_cfg_dmy0_FLD_xe_cfg_dmy0, (reg32))

#define e_cfg_dmy1_GET_xe_cfg_dmy1(reg32)                      REG_FLD_GET(e_cfg_dmy1_FLD_xe_cfg_dmy1, (reg32))

#define e_llid0_3_cfg_GET_llid3_dmy(reg32)                     REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_dmy, (reg32))
#define e_llid0_3_cfg_GET_llid3_encrypt_key(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_encrypt_key, (reg32))
#define e_llid0_3_cfg_GET_llid3_encrypt_en(reg32)              REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_encrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid3_txfec_en(reg32)                REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_txfec_en, (reg32))
#define e_llid0_3_cfg_GET_llid3_dcrypt_en(reg32)               REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_dcrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid3_dcrypt_mode(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_dcrypt_mode, (reg32))
#define e_llid0_3_cfg_GET_llid3_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_oam_lpbk_en, (reg32))
#define e_llid0_3_cfg_GET_llid2_dmy(reg32)                     REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_dmy, (reg32))
#define e_llid0_3_cfg_GET_llid2_encrypt_key(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_encrypt_key, (reg32))
#define e_llid0_3_cfg_GET_llid2_encrypt_en(reg32)              REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_encrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid2_txfec_en(reg32)                REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_txfec_en, (reg32))
#define e_llid0_3_cfg_GET_llid2_dcrypt_en(reg32)               REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_dcrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid2_dcrypt_mode(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_dcrypt_mode, (reg32))
#define e_llid0_3_cfg_GET_llid2_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_oam_lpbk_en, (reg32))
#define e_llid0_3_cfg_GET_llid1_dmy(reg32)                     REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_dmy, (reg32))
#define e_llid0_3_cfg_GET_llid1_encrypt_key(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_encrypt_key, (reg32))
#define e_llid0_3_cfg_GET_llid1_encrypt_en(reg32)              REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_encrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid1_txfec_en(reg32)                REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_txfec_en, (reg32))
#define e_llid0_3_cfg_GET_llid1_dcrypt_en(reg32)               REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_dcrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid1_dcrypt_mode(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_dcrypt_mode, (reg32))
#define e_llid0_3_cfg_GET_llid1_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_oam_lpbk_en, (reg32))
#define e_llid0_3_cfg_GET_llid0_dmy(reg32)                     REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_dmy, (reg32))
#define e_llid0_3_cfg_GET_llid0_encrypt_key(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_encrypt_key, (reg32))
#define e_llid0_3_cfg_GET_llid0_encrypt_en(reg32)              REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_encrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid0_txfec_en(reg32)                REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_txfec_en, (reg32))
#define e_llid0_3_cfg_GET_llid0_dcrypt_en(reg32)               REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_dcrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid0_dcrypt_mode(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_dcrypt_mode, (reg32))
#define e_llid0_3_cfg_GET_llid0_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_oam_lpbk_en, (reg32))

#define e_llid4_7_cfg_GET_llid7_dmy(reg32)                     REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_dmy, (reg32))
#define e_llid4_7_cfg_GET_llid7_encrypt_key(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_encrypt_key, (reg32))
#define e_llid4_7_cfg_GET_llid7_encrypt_en(reg32)              REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_encrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid7_txfec_en(reg32)                REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_txfec_en, (reg32))
#define e_llid4_7_cfg_GET_llid7_dcrypt_en(reg32)               REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_dcrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid7_dcrypt_mode(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_dcrypt_mode, (reg32))
#define e_llid4_7_cfg_GET_llid7_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_oam_lpbk_en, (reg32))
#define e_llid4_7_cfg_GET_llid6_dmy(reg32)                     REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_dmy, (reg32))
#define e_llid4_7_cfg_GET_llid6_encrypt_key(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_encrypt_key, (reg32))
#define e_llid4_7_cfg_GET_llid6_encrypt_en(reg32)              REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_encrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid6_txfec_en(reg32)                REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_txfec_en, (reg32))
#define e_llid4_7_cfg_GET_llid6_dcrypt_en(reg32)               REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_dcrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid6_dcrypt_mode(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_dcrypt_mode, (reg32))
#define e_llid4_7_cfg_GET_llid6_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_oam_lpbk_en, (reg32))
#define e_llid4_7_cfg_GET_llid5_dmy(reg32)                     REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_dmy, (reg32))
#define e_llid4_7_cfg_GET_llid5_encrypt_key(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_encrypt_key, (reg32))
#define e_llid4_7_cfg_GET_llid5_encrypt_en(reg32)              REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_encrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid5_txfec_en(reg32)                REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_txfec_en, (reg32))
#define e_llid4_7_cfg_GET_llid5_dcrypt_en(reg32)               REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_dcrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid5_dcrypt_mode(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_dcrypt_mode, (reg32))
#define e_llid4_7_cfg_GET_llid5_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_oam_lpbk_en, (reg32))
#define e_llid4_7_cfg_GET_llid4_dmy(reg32)                     REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_dmy, (reg32))
#define e_llid4_7_cfg_GET_llid4_encrypt_key(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_encrypt_key, (reg32))
#define e_llid4_7_cfg_GET_llid4_encrypt_en(reg32)              REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_encrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid4_txfec_en(reg32)                REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_txfec_en, (reg32))
#define e_llid4_7_cfg_GET_llid4_dcrypt_en(reg32)               REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_dcrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid4_dcrypt_mode(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_dcrypt_mode, (reg32))
#define e_llid4_7_cfg_GET_llid4_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_oam_lpbk_en, (reg32))

#define e_llid8_11_cfg_GET_llid11_dmy(reg32)                   REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_dmy, (reg32))
#define e_llid8_11_cfg_GET_llid11_encrypt_key(reg32)           REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_encrypt_key, (reg32))
#define e_llid8_11_cfg_GET_llid11_encrypt_en(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_encrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid11_txfec_en(reg32)              REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_txfec_en, (reg32))
#define e_llid8_11_cfg_GET_llid11_dcrypt_en(reg32)             REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_dcrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid11_dcrypt_mode(reg32)           REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_dcrypt_mode, (reg32))
#define e_llid8_11_cfg_GET_llid11_oamlpbk_en(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_oamlpbk_en, (reg32))
#define e_llid8_11_cfg_GET_llid10_dmy(reg32)                   REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_dmy, (reg32))
#define e_llid8_11_cfg_GET_llid10_encrypt_key(reg32)           REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_encrypt_key, (reg32))
#define e_llid8_11_cfg_GET_llid10_encrypt_en(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_encrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid10_txfec_en(reg32)              REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_txfec_en, (reg32))
#define e_llid8_11_cfg_GET_llid10_dcrypt_en(reg32)             REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_dcrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid10_dcrypt_mode(reg32)           REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_dcrypt_mode, (reg32))
#define e_llid8_11_cfg_GET_llid10_oamlpbk_en(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_oamlpbk_en, (reg32))
#define e_llid8_11_cfg_GET_llid9_dmy(reg32)                    REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_dmy, (reg32))
#define e_llid8_11_cfg_GET_llid9_encrypt_key(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_encrypt_key, (reg32))
#define e_llid8_11_cfg_GET_llid9_encrypt_en(reg32)             REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_encrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid9_txfec_en(reg32)               REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_txfec_en, (reg32))
#define e_llid8_11_cfg_GET_llid9_dcrypt_en(reg32)              REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_dcrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid9_dcrypt_mode(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_dcrypt_mode, (reg32))
#define e_llid8_11_cfg_GET_llid9_oamlpbk_en(reg32)             REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_oamlpbk_en, (reg32))
#define e_llid8_11_cfg_GET_llid8_dmy(reg32)                    REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_dmy, (reg32))
#define e_llid8_11_cfg_GET_llid8_encrypt_key(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_encrypt_key, (reg32))
#define e_llid8_11_cfg_GET_llid8_encrypt_en(reg32)             REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_encrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid8_txfec_en(reg32)               REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_txfec_en, (reg32))
#define e_llid8_11_cfg_GET_llid8_dcrypt_en(reg32)              REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_dcrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid8_dcrypt_mode(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_dcrypt_mode, (reg32))
#define e_llid8_11_cfg_GET_llid8_oamlpbk_en(reg32)             REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_oamlpbk_en, (reg32))

#define e_llid12_15_cfg_GET_llid15_dmy(reg32)                  REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_dmy, (reg32))
#define e_llid12_15_cfg_GET_llid15_encrypt_key(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_encrypt_key, (reg32))
#define e_llid12_15_cfg_GET_llid15_encrypt_en(reg32)           REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_encrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid15_txfec_en(reg32)             REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_txfec_en, (reg32))
#define e_llid12_15_cfg_GET_llid15_dcrypt_en(reg32)            REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_dcrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid15_dcrypt_mode(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_dcrypt_mode, (reg32))
#define e_llid12_15_cfg_GET_llid15_oamlpbk_en(reg32)           REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_oamlpbk_en, (reg32))
#define e_llid12_15_cfg_GET_llid14_dmy(reg32)                  REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_dmy, (reg32))
#define e_llid12_15_cfg_GET_llid14_encrypt_key(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_encrypt_key, (reg32))
#define e_llid12_15_cfg_GET_llid14_encrypt_en(reg32)           REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_encrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid14_txfec_en(reg32)             REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_txfec_en, (reg32))
#define e_llid12_15_cfg_GET_llid14_dcrypt_en(reg32)            REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_dcrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid14_dcrypt_mode(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_dcrypt_mode, (reg32))
#define e_llid12_15_cfg_GET_llid14_oamlpbk_en(reg32)           REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_oamlpbk_en, (reg32))
#define e_llid12_15_cfg_GET_llid13_dmy(reg32)                  REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_dmy, (reg32))
#define e_llid12_15_cfg_GET_llid13_encrypt_key(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_encrypt_key, (reg32))
#define e_llid12_15_cfg_GET_llid13_encrypt_en(reg32)           REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_encrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid13_txfec_en(reg32)             REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_txfec_en, (reg32))
#define e_llid12_15_cfg_GET_llid13_dcrypt_en(reg32)            REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_dcrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid13_dcrypt_mode(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_dcrypt_mode, (reg32))
#define e_llid12_15_cfg_GET_llid13_oamlpbk_en(reg32)           REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_oamlpbk_en, (reg32))
#define e_llid12_15_cfg_GET_llid12_dmy(reg32)                  REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_dmy, (reg32))
#define e_llid12_15_cfg_GET_llid12_encrypt_key(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_encrypt_key, (reg32))
#define e_llid12_15_cfg_GET_llid12_encrypt_en(reg32)           REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_encrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid12_txfec_en(reg32)             REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_txfec_en, (reg32))
#define e_llid12_15_cfg_GET_llid12_dcrypt_en(reg32)            REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_dcrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid12_dcrypt_mode(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_dcrypt_mode, (reg32))
#define e_llid12_15_cfg_GET_llid12_oam_lpbk_en(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_oam_lpbk_en, (reg32))

#define e_llid16_19_cfg_GET_llid19_dmy(reg32)                  REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_dmy, (reg32))
#define e_llid16_19_cfg_GET_llid19_encrypt_key(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_encrypt_key, (reg32))
#define e_llid16_19_cfg_GET_llid19_encrypt_en(reg32)           REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_encrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid19_txfec_en(reg32)             REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_txfec_en, (reg32))
#define e_llid16_19_cfg_GET_llid19_dcrypt_en(reg32)            REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_dcrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid19_dcrypt_mode(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_dcrypt_mode, (reg32))
#define e_llid16_19_cfg_GET_llid19_oamlpbk_en(reg32)           REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_oamlpbk_en, (reg32))
#define e_llid16_19_cfg_GET_llid18_dmy(reg32)                  REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_dmy, (reg32))
#define e_llid16_19_cfg_GET_llid18_encrypt_key(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_encrypt_key, (reg32))
#define e_llid16_19_cfg_GET_llid18_encrypt_en(reg32)           REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_encrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid18_txfec_en(reg32)             REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_txfec_en, (reg32))
#define e_llid16_19_cfg_GET_llid18_dcrypt_en(reg32)            REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_dcrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid18_dcrypt_mode(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_dcrypt_mode, (reg32))
#define e_llid16_19_cfg_GET_llid18_oamlpbk_en(reg32)           REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_oamlpbk_en, (reg32))
#define e_llid16_19_cfg_GET_llid17_dmy(reg32)                  REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_dmy, (reg32))
#define e_llid16_19_cfg_GET_llid17_encrypt_key(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_encrypt_key, (reg32))
#define e_llid16_19_cfg_GET_llid17_encrypt_en(reg32)           REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_encrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid17_txfec_en(reg32)             REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_txfec_en, (reg32))
#define e_llid16_19_cfg_GET_llid17_dcrypt_en(reg32)            REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_dcrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid17_dcrypt_mode(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_dcrypt_mode, (reg32))
#define e_llid16_19_cfg_GET_llid17_oamlpbk_en(reg32)           REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_oamlpbk_en, (reg32))
#define e_llid16_19_cfg_GET_llid16_dmy(reg32)                  REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_dmy, (reg32))
#define e_llid16_19_cfg_GET_llid16_encrypt_key(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_encrypt_key, (reg32))
#define e_llid16_19_cfg_GET_llid16_encrypt_en(reg32)           REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_encrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid16_txfec_en(reg32)             REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_txfec_en, (reg32))
#define e_llid16_19_cfg_GET_llid16_dcrypt_en(reg32)            REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_dcrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid16_dcrypt_mode(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_dcrypt_mode, (reg32))
#define e_llid16_19_cfg_GET_llid16_oam_lpbk_en(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_oam_lpbk_en, (reg32))

#define e_llid20_23_cfg_GET_llid23_dmy(reg32)                  REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_dmy, (reg32))
#define e_llid20_23_cfg_GET_llid23_encrypt_key(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_encrypt_key, (reg32))
#define e_llid20_23_cfg_GET_llid23_encrypt_en(reg32)           REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_encrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid23_txfec_en(reg32)             REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_txfec_en, (reg32))
#define e_llid20_23_cfg_GET_llid23_dcrypt_en(reg32)            REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_dcrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid23_dcrypt_mode(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_dcrypt_mode, (reg32))
#define e_llid20_23_cfg_GET_llid23_oamlpbk_en(reg32)           REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_oamlpbk_en, (reg32))
#define e_llid20_23_cfg_GET_llid22_dmy(reg32)                  REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_dmy, (reg32))
#define e_llid20_23_cfg_GET_llid22_encrypt_key(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_encrypt_key, (reg32))
#define e_llid20_23_cfg_GET_llid22_encrypt_en(reg32)           REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_encrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid22_txfec_en(reg32)             REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_txfec_en, (reg32))
#define e_llid20_23_cfg_GET_llid22_dcrypt_en(reg32)            REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_dcrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid22_dcrypt_mode(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_dcrypt_mode, (reg32))
#define e_llid20_23_cfg_GET_llid22_oamlpbk_en(reg32)           REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_oamlpbk_en, (reg32))
#define e_llid20_23_cfg_GET_llid21_dmy(reg32)                  REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_dmy, (reg32))
#define e_llid20_23_cfg_GET_llid21_encrypt_key(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_encrypt_key, (reg32))
#define e_llid20_23_cfg_GET_llid21_encrypt_en(reg32)           REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_encrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid21_txfec_en(reg32)             REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_txfec_en, (reg32))
#define e_llid20_23_cfg_GET_llid21_dcrypt_en(reg32)            REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_dcrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid21_dcrypt_mode(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_dcrypt_mode, (reg32))
#define e_llid20_23_cfg_GET_llid21_oamlpbk_en(reg32)           REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_oamlpbk_en, (reg32))
#define e_llid20_23_cfg_GET_llid20_dmy(reg32)                  REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_dmy, (reg32))
#define e_llid20_23_cfg_GET_llid20_encrypt_key(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_encrypt_key, (reg32))
#define e_llid20_23_cfg_GET_llid20_encrypt_en(reg32)           REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_encrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid20_txfec_en(reg32)             REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_txfec_en, (reg32))
#define e_llid20_23_cfg_GET_llid20_dcrypt_en(reg32)            REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_dcrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid20_dcrypt_mode(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_dcrypt_mode, (reg32))
#define e_llid20_23_cfg_GET_llid20_oam_lpbk_en(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_oam_lpbk_en, (reg32))

#define e_llid24_27_cfg_GET_llid27_dmy(reg32)                  REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_dmy, (reg32))
#define e_llid24_27_cfg_GET_llid27_encrypt_key(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_encrypt_key, (reg32))
#define e_llid24_27_cfg_GET_llid27_encrypt_en(reg32)           REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_encrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid27_txfec_en(reg32)             REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_txfec_en, (reg32))
#define e_llid24_27_cfg_GET_llid27_dcrypt_en(reg32)            REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_dcrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid27_dcrypt_mode(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_dcrypt_mode, (reg32))
#define e_llid24_27_cfg_GET_llid27_oamlpbk_en(reg32)           REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_oamlpbk_en, (reg32))
#define e_llid24_27_cfg_GET_llid26_dmy(reg32)                  REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_dmy, (reg32))
#define e_llid24_27_cfg_GET_llid26_encrypt_key(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_encrypt_key, (reg32))
#define e_llid24_27_cfg_GET_llid26_encrypt_en(reg32)           REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_encrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid26_txfec_en(reg32)             REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_txfec_en, (reg32))
#define e_llid24_27_cfg_GET_llid26_dcrypt_en(reg32)            REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_dcrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid26_dcrypt_mode(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_dcrypt_mode, (reg32))
#define e_llid24_27_cfg_GET_llid26_oamlpbk_en(reg32)           REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_oamlpbk_en, (reg32))
#define e_llid24_27_cfg_GET_llid25_dmy(reg32)                  REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_dmy, (reg32))
#define e_llid24_27_cfg_GET_llid25_encrypt_key(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_encrypt_key, (reg32))
#define e_llid24_27_cfg_GET_llid25_encrypt_en(reg32)           REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_encrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid25_txfec_en(reg32)             REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_txfec_en, (reg32))
#define e_llid24_27_cfg_GET_llid25_dcrypt_en(reg32)            REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_dcrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid25_dcrypt_mode(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_dcrypt_mode, (reg32))
#define e_llid24_27_cfg_GET_llid25_oamlpbk_en(reg32)           REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_oamlpbk_en, (reg32))
#define e_llid24_27_cfg_GET_llid24_dmy(reg32)                  REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_dmy, (reg32))
#define e_llid24_27_cfg_GET_llid24_encrypt_key(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_encrypt_key, (reg32))
#define e_llid24_27_cfg_GET_llid24_encrypt_en(reg32)           REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_encrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid24_txfec_en(reg32)             REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_txfec_en, (reg32))
#define e_llid24_27_cfg_GET_llid24_dcrypt_en(reg32)            REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_dcrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid24_dcrypt_mode(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_dcrypt_mode, (reg32))
#define e_llid24_27_cfg_GET_llid24_oam_lpbk_en(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_oam_lpbk_en, (reg32))

#define e_llid28_31_cfg_GET_llid31_dmy(reg32)                  REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_dmy, (reg32))
#define e_llid28_31_cfg_GET_llid31_encrypt_key(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_encrypt_key, (reg32))
#define e_llid28_31_cfg_GET_llid31_encrypt_en(reg32)           REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_encrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid31_txfec_en(reg32)             REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_txfec_en, (reg32))
#define e_llid28_31_cfg_GET_llid31_dcrypt_en(reg32)            REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_dcrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid31_dcrypt_mode(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_dcrypt_mode, (reg32))
#define e_llid28_31_cfg_GET_llid31_oamlpbk_en(reg32)           REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_oamlpbk_en, (reg32))
#define e_llid28_31_cfg_GET_llid30_dmy(reg32)                  REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_dmy, (reg32))
#define e_llid28_31_cfg_GET_llid30_encrypt_key(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_encrypt_key, (reg32))
#define e_llid28_31_cfg_GET_llid30_encrypt_en(reg32)           REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_encrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid30_txfec_en(reg32)             REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_txfec_en, (reg32))
#define e_llid28_31_cfg_GET_llid30_dcrypt_en(reg32)            REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_dcrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid30_dcrypt_mode(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_dcrypt_mode, (reg32))
#define e_llid28_31_cfg_GET_llid30_oamlpbk_en(reg32)           REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_oamlpbk_en, (reg32))
#define e_llid28_31_cfg_GET_llid29_dmy(reg32)                  REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_dmy, (reg32))
#define e_llid28_31_cfg_GET_llid29_encrypt_key(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_encrypt_key, (reg32))
#define e_llid28_31_cfg_GET_llid29_encrypt_en(reg32)           REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_encrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid29_txfec_en(reg32)             REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_txfec_en, (reg32))
#define e_llid28_31_cfg_GET_llid29_dcrypt_en(reg32)            REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_dcrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid29_dcrypt_mode(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_dcrypt_mode, (reg32))
#define e_llid28_31_cfg_GET_llid29_oamlpbk_en(reg32)           REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_oamlpbk_en, (reg32))
#define e_llid28_31_cfg_GET_llid28_dmy(reg32)                  REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_dmy, (reg32))
#define e_llid28_31_cfg_GET_llid28_encrypt_key(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_encrypt_key, (reg32))
#define e_llid28_31_cfg_GET_llid28_encrypt_en(reg32)           REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_encrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid28_txfec_en(reg32)             REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_txfec_en, (reg32))
#define e_llid28_31_cfg_GET_llid28_dcrypt_en(reg32)            REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_dcrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid28_dcrypt_mode(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_dcrypt_mode, (reg32))
#define e_llid28_31_cfg_GET_llid28_oam_lpbk_en(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_oam_lpbk_en, (reg32))

#define e_cfg_dmy2_GET_xe_cfg_dmy2(reg32)                      REG_FLD_GET(e_cfg_dmy2_FLD_xe_cfg_dmy2, (reg32))

#define e_cfg_dmy3_GET_xe_cfg_dmy3(reg32)                      REG_FLD_GET(e_cfg_dmy3_FLD_xe_cfg_dmy3, (reg32))

#define e_cfg_dmy4_GET_xe_cfg_dmy4(reg32)                      REG_FLD_GET(e_cfg_dmy4_FLD_xe_cfg_dmy4, (reg32))

#define e_llid_dscvry_ctrl_GET_mpcp_cmd(reg32)                 REG_FLD_GET(e_llid_dscvry_ctrl_FLD_mpcp_cmd, (reg32))
#define e_llid_dscvry_ctrl_GET_mpcp_cmd_done(reg32)            REG_FLD_GET(e_llid_dscvry_ctrl_FLD_mpcp_cmd_done, (reg32))
#define e_llid_dscvry_ctrl_GET_rgstr_ack_flg(reg32)            REG_FLD_GET(e_llid_dscvry_ctrl_FLD_rgstr_ack_flg, (reg32))
#define e_llid_dscvry_ctrl_GET_rgstr_req_flg(reg32)            REG_FLD_GET(e_llid_dscvry_ctrl_FLD_rgstr_req_flg, (reg32))
#define e_llid_dscvry_ctrl_GET_tx_mpcp_llid_idx(reg32)         REG_FLD_GET(e_llid_dscvry_ctrl_FLD_tx_mpcp_llid_idx, (reg32))

#define e_llid0_dscvry_sts_GET_llid0_dscvry_sts(reg32)         REG_FLD_GET(e_llid0_dscvry_sts_FLD_llid0_dscvry_sts, (reg32))
#define e_llid0_dscvry_sts_GET_llid0_rgstr_flg_sts(reg32)      REG_FLD_GET(e_llid0_dscvry_sts_FLD_llid0_rgstr_flg_sts, (reg32))
#define e_llid0_dscvry_sts_GET_llid0_valid(reg32)              REG_FLD_GET(e_llid0_dscvry_sts_FLD_llid0_valid, (reg32))
#define e_llid0_dscvry_sts_GET_llid0_value(reg32)              REG_FLD_GET(e_llid0_dscvry_sts_FLD_llid0_value, (reg32))

#define e_llid1_dscvry_sts_GET_llid1_dscvry_sts(reg32)         REG_FLD_GET(e_llid1_dscvry_sts_FLD_llid1_dscvry_sts, (reg32))
#define e_llid1_dscvry_sts_GET_llid1_rgstr_flg_sts(reg32)      REG_FLD_GET(e_llid1_dscvry_sts_FLD_llid1_rgstr_flg_sts, (reg32))
#define e_llid1_dscvry_sts_GET_llid1_valid(reg32)              REG_FLD_GET(e_llid1_dscvry_sts_FLD_llid1_valid, (reg32))
#define e_llid1_dscvry_sts_GET_llid1_value(reg32)              REG_FLD_GET(e_llid1_dscvry_sts_FLD_llid1_value, (reg32))

#define e_llid2_dscvry_sts_GET_llid2_dscvry_sts(reg32)         REG_FLD_GET(e_llid2_dscvry_sts_FLD_llid2_dscvry_sts, (reg32))
#define e_llid2_dscvry_sts_GET_llid2_rgstr_flg_sts(reg32)      REG_FLD_GET(e_llid2_dscvry_sts_FLD_llid2_rgstr_flg_sts, (reg32))
#define e_llid2_dscvry_sts_GET_llid2_valid(reg32)              REG_FLD_GET(e_llid2_dscvry_sts_FLD_llid2_valid, (reg32))
#define e_llid2_dscvry_sts_GET_llid2_value(reg32)              REG_FLD_GET(e_llid2_dscvry_sts_FLD_llid2_value, (reg32))

#define e_llid3_dscvry_sts_GET_llid3_dscvry_sts(reg32)         REG_FLD_GET(e_llid3_dscvry_sts_FLD_llid3_dscvry_sts, (reg32))
#define e_llid3_dscvry_sts_GET_llid3_rgstr_flg_sts(reg32)      REG_FLD_GET(e_llid3_dscvry_sts_FLD_llid3_rgstr_flg_sts, (reg32))
#define e_llid3_dscvry_sts_GET_llid3_valid(reg32)              REG_FLD_GET(e_llid3_dscvry_sts_FLD_llid3_valid, (reg32))
#define e_llid3_dscvry_sts_GET_llid3_value(reg32)              REG_FLD_GET(e_llid3_dscvry_sts_FLD_llid3_value, (reg32))

#define e_llid4_dscvry_sts_GET_llid4_dscvry_sts(reg32)         REG_FLD_GET(e_llid4_dscvry_sts_FLD_llid4_dscvry_sts, (reg32))
#define e_llid4_dscvry_sts_GET_llid4_rgstr_flg_sts(reg32)      REG_FLD_GET(e_llid4_dscvry_sts_FLD_llid4_rgstr_flg_sts, (reg32))
#define e_llid4_dscvry_sts_GET_llid4_valid(reg32)              REG_FLD_GET(e_llid4_dscvry_sts_FLD_llid4_valid, (reg32))
#define e_llid4_dscvry_sts_GET_llid4_value(reg32)              REG_FLD_GET(e_llid4_dscvry_sts_FLD_llid4_value, (reg32))

#define e_llid5_dscvry_sts_GET_llid5_dscvry_sts(reg32)         REG_FLD_GET(e_llid5_dscvry_sts_FLD_llid5_dscvry_sts, (reg32))
#define e_llid5_dscvry_sts_GET_llid5_rgstr_flg_sts(reg32)      REG_FLD_GET(e_llid5_dscvry_sts_FLD_llid5_rgstr_flg_sts, (reg32))
#define e_llid5_dscvry_sts_GET_llid5_valid(reg32)              REG_FLD_GET(e_llid5_dscvry_sts_FLD_llid5_valid, (reg32))
#define e_llid5_dscvry_sts_GET_llid5_value(reg32)              REG_FLD_GET(e_llid5_dscvry_sts_FLD_llid5_value, (reg32))

#define e_llid6_dscvry_sts_GET_llid6_dscvry_sts(reg32)         REG_FLD_GET(e_llid6_dscvry_sts_FLD_llid6_dscvry_sts, (reg32))
#define e_llid6_dscvry_sts_GET_llid6_rgstr_flg_sts(reg32)      REG_FLD_GET(e_llid6_dscvry_sts_FLD_llid6_rgstr_flg_sts, (reg32))
#define e_llid6_dscvry_sts_GET_llid6_valid(reg32)              REG_FLD_GET(e_llid6_dscvry_sts_FLD_llid6_valid, (reg32))
#define e_llid6_dscvry_sts_GET_llid6_value(reg32)              REG_FLD_GET(e_llid6_dscvry_sts_FLD_llid6_value, (reg32))

#define e_llid7_dscvry_sts_GET_llid7_dscvry_sts(reg32)         REG_FLD_GET(e_llid7_dscvry_sts_FLD_llid7_dscvry_sts, (reg32))
#define e_llid7_dscvry_sts_GET_llid7_rgstr_flg_sts(reg32)      REG_FLD_GET(e_llid7_dscvry_sts_FLD_llid7_rgstr_flg_sts, (reg32))
#define e_llid7_dscvry_sts_GET_llid7_valid(reg32)              REG_FLD_GET(e_llid7_dscvry_sts_FLD_llid7_valid, (reg32))
#define e_llid7_dscvry_sts_GET_llid7_value(reg32)              REG_FLD_GET(e_llid7_dscvry_sts_FLD_llid7_value, (reg32))

#define e_llid8_dscvry_sts_GET_llid8_dscvry_sts(reg32)         REG_FLD_GET(e_llid8_dscvry_sts_FLD_llid8_dscvry_sts, (reg32))
#define e_llid8_dscvry_sts_GET_llid8_rgstr_flg_sts(reg32)      REG_FLD_GET(e_llid8_dscvry_sts_FLD_llid8_rgstr_flg_sts, (reg32))
#define e_llid8_dscvry_sts_GET_llid8_valid(reg32)              REG_FLD_GET(e_llid8_dscvry_sts_FLD_llid8_valid, (reg32))
#define e_llid8_dscvry_sts_GET_llid8_value(reg32)              REG_FLD_GET(e_llid8_dscvry_sts_FLD_llid8_value, (reg32))

#define e_llid9_dscvry_sts_GET_llid9_dscvry_sts(reg32)         REG_FLD_GET(e_llid9_dscvry_sts_FLD_llid9_dscvry_sts, (reg32))
#define e_llid9_dscvry_sts_GET_llid9_rgstr_flg_sts(reg32)      REG_FLD_GET(e_llid9_dscvry_sts_FLD_llid9_rgstr_flg_sts, (reg32))
#define e_llid9_dscvry_sts_GET_llid9_valid(reg32)              REG_FLD_GET(e_llid9_dscvry_sts_FLD_llid9_valid, (reg32))
#define e_llid9_dscvry_sts_GET_llid9_value(reg32)              REG_FLD_GET(e_llid9_dscvry_sts_FLD_llid9_value, (reg32))

#define e_llid10_dscvry_sts_GET_llid10_dscvry_sts(reg32)       REG_FLD_GET(e_llid10_dscvry_sts_FLD_llid10_dscvry_sts, (reg32))
#define e_llid10_dscvry_sts_GET_llid10_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid10_dscvry_sts_FLD_llid10_rgstr_flg_sts, (reg32))
#define e_llid10_dscvry_sts_GET_llid10_valid(reg32)            REG_FLD_GET(e_llid10_dscvry_sts_FLD_llid10_valid, (reg32))
#define e_llid10_dscvry_sts_GET_llid10_value(reg32)            REG_FLD_GET(e_llid10_dscvry_sts_FLD_llid10_value, (reg32))

#define e_llid11_dscvry_sts_GET_llid11_dscvry_sts(reg32)       REG_FLD_GET(e_llid11_dscvry_sts_FLD_llid11_dscvry_sts, (reg32))
#define e_llid11_dscvry_sts_GET_llid11_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid11_dscvry_sts_FLD_llid11_rgstr_flg_sts, (reg32))
#define e_llid11_dscvry_sts_GET_llid11_valid(reg32)            REG_FLD_GET(e_llid11_dscvry_sts_FLD_llid11_valid, (reg32))
#define e_llid11_dscvry_sts_GET_llid11_value(reg32)            REG_FLD_GET(e_llid11_dscvry_sts_FLD_llid11_value, (reg32))

#define e_llid12_dscvry_sts_GET_llid12_dscvry_sts(reg32)       REG_FLD_GET(e_llid12_dscvry_sts_FLD_llid12_dscvry_sts, (reg32))
#define e_llid12_dscvry_sts_GET_llid12_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid12_dscvry_sts_FLD_llid12_rgstr_flg_sts, (reg32))
#define e_llid12_dscvry_sts_GET_llid12_valid(reg32)            REG_FLD_GET(e_llid12_dscvry_sts_FLD_llid12_valid, (reg32))
#define e_llid12_dscvry_sts_GET_llid12_value(reg32)            REG_FLD_GET(e_llid12_dscvry_sts_FLD_llid12_value, (reg32))

#define e_llid13_dscvry_sts_GET_llid13_dscvry_sts(reg32)       REG_FLD_GET(e_llid13_dscvry_sts_FLD_llid13_dscvry_sts, (reg32))
#define e_llid13_dscvry_sts_GET_llid13_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid13_dscvry_sts_FLD_llid13_rgstr_flg_sts, (reg32))
#define e_llid13_dscvry_sts_GET_llid13_valid(reg32)            REG_FLD_GET(e_llid13_dscvry_sts_FLD_llid13_valid, (reg32))
#define e_llid13_dscvry_sts_GET_llid13_value(reg32)            REG_FLD_GET(e_llid13_dscvry_sts_FLD_llid13_value, (reg32))

#define e_llid14_dscvry_sts_GET_llid14_dscvry_sts(reg32)       REG_FLD_GET(e_llid14_dscvry_sts_FLD_llid14_dscvry_sts, (reg32))
#define e_llid14_dscvry_sts_GET_llid14_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid14_dscvry_sts_FLD_llid14_rgstr_flg_sts, (reg32))
#define e_llid14_dscvry_sts_GET_llid14_valid(reg32)            REG_FLD_GET(e_llid14_dscvry_sts_FLD_llid14_valid, (reg32))
#define e_llid14_dscvry_sts_GET_llid14_value(reg32)            REG_FLD_GET(e_llid14_dscvry_sts_FLD_llid14_value, (reg32))

#define e_llid15_dscvry_sts_GET_llid15_dscvry_sts(reg32)       REG_FLD_GET(e_llid15_dscvry_sts_FLD_llid15_dscvry_sts, (reg32))
#define e_llid15_dscvry_sts_GET_llid15_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid15_dscvry_sts_FLD_llid15_rgstr_flg_sts, (reg32))
#define e_llid15_dscvry_sts_GET_llid15_valid(reg32)            REG_FLD_GET(e_llid15_dscvry_sts_FLD_llid15_valid, (reg32))
#define e_llid15_dscvry_sts_GET_llid15_value(reg32)            REG_FLD_GET(e_llid15_dscvry_sts_FLD_llid15_value, (reg32))

#define e_llid16_dscvry_sts_GET_llid16_dscvry_sts(reg32)       REG_FLD_GET(e_llid16_dscvry_sts_FLD_llid16_dscvry_sts, (reg32))
#define e_llid16_dscvry_sts_GET_llid16_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid16_dscvry_sts_FLD_llid16_rgstr_flg_sts, (reg32))
#define e_llid16_dscvry_sts_GET_llid16_valid(reg32)            REG_FLD_GET(e_llid16_dscvry_sts_FLD_llid16_valid, (reg32))
#define e_llid16_dscvry_sts_GET_llid16_value(reg32)            REG_FLD_GET(e_llid16_dscvry_sts_FLD_llid16_value, (reg32))

#define e_llid17_dscvry_sts_GET_llid17_dscvry_sts(reg32)       REG_FLD_GET(e_llid17_dscvry_sts_FLD_llid17_dscvry_sts, (reg32))
#define e_llid17_dscvry_sts_GET_llid17_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid17_dscvry_sts_FLD_llid17_rgstr_flg_sts, (reg32))
#define e_llid17_dscvry_sts_GET_llid17_valid(reg32)            REG_FLD_GET(e_llid17_dscvry_sts_FLD_llid17_valid, (reg32))
#define e_llid17_dscvry_sts_GET_llid17_value(reg32)            REG_FLD_GET(e_llid17_dscvry_sts_FLD_llid17_value, (reg32))

#define e_llid18_dscvry_sts_GET_llid18_dscvry_sts(reg32)       REG_FLD_GET(e_llid18_dscvry_sts_FLD_llid18_dscvry_sts, (reg32))
#define e_llid18_dscvry_sts_GET_llid18_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid18_dscvry_sts_FLD_llid18_rgstr_flg_sts, (reg32))
#define e_llid18_dscvry_sts_GET_llid18_valid(reg32)            REG_FLD_GET(e_llid18_dscvry_sts_FLD_llid18_valid, (reg32))
#define e_llid18_dscvry_sts_GET_llid18_value(reg32)            REG_FLD_GET(e_llid18_dscvry_sts_FLD_llid18_value, (reg32))

#define e_llid19_dscvry_sts_GET_llid19_dscvry_sts(reg32)       REG_FLD_GET(e_llid19_dscvry_sts_FLD_llid19_dscvry_sts, (reg32))
#define e_llid19_dscvry_sts_GET_llid19_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid19_dscvry_sts_FLD_llid19_rgstr_flg_sts, (reg32))
#define e_llid19_dscvry_sts_GET_llid19_valid(reg32)            REG_FLD_GET(e_llid19_dscvry_sts_FLD_llid19_valid, (reg32))
#define e_llid19_dscvry_sts_GET_llid19_value(reg32)            REG_FLD_GET(e_llid19_dscvry_sts_FLD_llid19_value, (reg32))

#define e_llid20_dscvry_sts_GET_llid20_dscvry_sts(reg32)       REG_FLD_GET(e_llid20_dscvry_sts_FLD_llid20_dscvry_sts, (reg32))
#define e_llid20_dscvry_sts_GET_llid20_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid20_dscvry_sts_FLD_llid20_rgstr_flg_sts, (reg32))
#define e_llid20_dscvry_sts_GET_llid20_valid(reg32)            REG_FLD_GET(e_llid20_dscvry_sts_FLD_llid20_valid, (reg32))
#define e_llid20_dscvry_sts_GET_llid20_value(reg32)            REG_FLD_GET(e_llid20_dscvry_sts_FLD_llid20_value, (reg32))

#define e_llid21_dscvry_sts_GET_llid21_dscvry_sts(reg32)       REG_FLD_GET(e_llid21_dscvry_sts_FLD_llid21_dscvry_sts, (reg32))
#define e_llid21_dscvry_sts_GET_llid21_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid21_dscvry_sts_FLD_llid21_rgstr_flg_sts, (reg32))
#define e_llid21_dscvry_sts_GET_llid21_valid(reg32)            REG_FLD_GET(e_llid21_dscvry_sts_FLD_llid21_valid, (reg32))
#define e_llid21_dscvry_sts_GET_llid21_value(reg32)            REG_FLD_GET(e_llid21_dscvry_sts_FLD_llid21_value, (reg32))

#define e_llid22_dscvry_sts_GET_llid22_dscvry_sts(reg32)       REG_FLD_GET(e_llid22_dscvry_sts_FLD_llid22_dscvry_sts, (reg32))
#define e_llid22_dscvry_sts_GET_llid22_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid22_dscvry_sts_FLD_llid22_rgstr_flg_sts, (reg32))
#define e_llid22_dscvry_sts_GET_llid22_valid(reg32)            REG_FLD_GET(e_llid22_dscvry_sts_FLD_llid22_valid, (reg32))
#define e_llid22_dscvry_sts_GET_llid22_value(reg32)            REG_FLD_GET(e_llid22_dscvry_sts_FLD_llid22_value, (reg32))

#define e_llid23_dscvry_sts_GET_llid23_dscvry_sts(reg32)       REG_FLD_GET(e_llid23_dscvry_sts_FLD_llid23_dscvry_sts, (reg32))
#define e_llid23_dscvry_sts_GET_llid23_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid23_dscvry_sts_FLD_llid23_rgstr_flg_sts, (reg32))
#define e_llid23_dscvry_sts_GET_llid23_valid(reg32)            REG_FLD_GET(e_llid23_dscvry_sts_FLD_llid23_valid, (reg32))
#define e_llid23_dscvry_sts_GET_llid23_value(reg32)            REG_FLD_GET(e_llid23_dscvry_sts_FLD_llid23_value, (reg32))

#define e_llid24_dscvry_sts_GET_llid24_dscvry_sts(reg32)       REG_FLD_GET(e_llid24_dscvry_sts_FLD_llid24_dscvry_sts, (reg32))
#define e_llid24_dscvry_sts_GET_llid24_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid24_dscvry_sts_FLD_llid24_rgstr_flg_sts, (reg32))
#define e_llid24_dscvry_sts_GET_llid24_valid(reg32)            REG_FLD_GET(e_llid24_dscvry_sts_FLD_llid24_valid, (reg32))
#define e_llid24_dscvry_sts_GET_llid24_value(reg32)            REG_FLD_GET(e_llid24_dscvry_sts_FLD_llid24_value, (reg32))

#define e_llid25_dscvry_sts_GET_llid25_dscvry_sts(reg32)       REG_FLD_GET(e_llid25_dscvry_sts_FLD_llid25_dscvry_sts, (reg32))
#define e_llid25_dscvry_sts_GET_llid25_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid25_dscvry_sts_FLD_llid25_rgstr_flg_sts, (reg32))
#define e_llid25_dscvry_sts_GET_llid25_valid(reg32)            REG_FLD_GET(e_llid25_dscvry_sts_FLD_llid25_valid, (reg32))
#define e_llid25_dscvry_sts_GET_llid25_value(reg32)            REG_FLD_GET(e_llid25_dscvry_sts_FLD_llid25_value, (reg32))

#define e_llid26_dscvry_sts_GET_llid26_dscvry_sts(reg32)       REG_FLD_GET(e_llid26_dscvry_sts_FLD_llid26_dscvry_sts, (reg32))
#define e_llid26_dscvry_sts_GET_llid26_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid26_dscvry_sts_FLD_llid26_rgstr_flg_sts, (reg32))
#define e_llid26_dscvry_sts_GET_llid26_valid(reg32)            REG_FLD_GET(e_llid26_dscvry_sts_FLD_llid26_valid, (reg32))
#define e_llid26_dscvry_sts_GET_llid26_value(reg32)            REG_FLD_GET(e_llid26_dscvry_sts_FLD_llid26_value, (reg32))

#define e_llid27_dscvry_sts_GET_llid27_dscvry_sts(reg32)       REG_FLD_GET(e_llid27_dscvry_sts_FLD_llid27_dscvry_sts, (reg32))
#define e_llid27_dscvry_sts_GET_llid27_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid27_dscvry_sts_FLD_llid27_rgstr_flg_sts, (reg32))
#define e_llid27_dscvry_sts_GET_llid27_valid(reg32)            REG_FLD_GET(e_llid27_dscvry_sts_FLD_llid27_valid, (reg32))
#define e_llid27_dscvry_sts_GET_llid27_value(reg32)            REG_FLD_GET(e_llid27_dscvry_sts_FLD_llid27_value, (reg32))

#define e_llid28_dscvry_sts_GET_llid28_dscvry_sts(reg32)       REG_FLD_GET(e_llid28_dscvry_sts_FLD_llid28_dscvry_sts, (reg32))
#define e_llid28_dscvry_sts_GET_llid28_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid28_dscvry_sts_FLD_llid28_rgstr_flg_sts, (reg32))
#define e_llid28_dscvry_sts_GET_llid28_valid(reg32)            REG_FLD_GET(e_llid28_dscvry_sts_FLD_llid28_valid, (reg32))
#define e_llid28_dscvry_sts_GET_llid28_value(reg32)            REG_FLD_GET(e_llid28_dscvry_sts_FLD_llid28_value, (reg32))

#define e_llid29_dscvry_sts_GET_llid29_dscvry_sts(reg32)       REG_FLD_GET(e_llid29_dscvry_sts_FLD_llid29_dscvry_sts, (reg32))
#define e_llid29_dscvry_sts_GET_llid29_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid29_dscvry_sts_FLD_llid29_rgstr_flg_sts, (reg32))
#define e_llid29_dscvry_sts_GET_llid29_valid(reg32)            REG_FLD_GET(e_llid29_dscvry_sts_FLD_llid29_valid, (reg32))
#define e_llid29_dscvry_sts_GET_llid29_value(reg32)            REG_FLD_GET(e_llid29_dscvry_sts_FLD_llid29_value, (reg32))

#define e_llid30_dscvry_sts_GET_llid30_dscvry_sts(reg32)       REG_FLD_GET(e_llid30_dscvry_sts_FLD_llid30_dscvry_sts, (reg32))
#define e_llid30_dscvry_sts_GET_llid30_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid30_dscvry_sts_FLD_llid30_rgstr_flg_sts, (reg32))
#define e_llid30_dscvry_sts_GET_llid30_valid(reg32)            REG_FLD_GET(e_llid30_dscvry_sts_FLD_llid30_valid, (reg32))
#define e_llid30_dscvry_sts_GET_llid30_value(reg32)            REG_FLD_GET(e_llid30_dscvry_sts_FLD_llid30_value, (reg32))

#define e_llid31_dscvry_sts_GET_llid31_dscvry_sts(reg32)       REG_FLD_GET(e_llid31_dscvry_sts_FLD_llid31_dscvry_sts, (reg32))
#define e_llid31_dscvry_sts_GET_llid31_rgstr_flg_sts(reg32)    REG_FLD_GET(e_llid31_dscvry_sts_FLD_llid31_rgstr_flg_sts, (reg32))
#define e_llid31_dscvry_sts_GET_llid31_valid(reg32)            REG_FLD_GET(e_llid31_dscvry_sts_FLD_llid31_valid, (reg32))
#define e_llid31_dscvry_sts_GET_llid31_value(reg32)            REG_FLD_GET(e_llid31_dscvry_sts_FLD_llid31_value, (reg32))

#define e_pending_gnt_num_GET_echoed_pending_gnt(reg32)        REG_FLD_GET(e_pending_gnt_num_FLD_echoed_pending_gnt, (reg32))
#define e_pending_gnt_num_GET_pending_gnt_num(reg32)           REG_FLD_GET(e_pending_gnt_num_FLD_pending_gnt_num, (reg32))

#define e_mac_addr_cfg_GET_mac_addr_rwcmd(reg32)               REG_FLD_GET(e_mac_addr_cfg_FLD_mac_addr_rwcmd, (reg32))
#define e_mac_addr_cfg_GET_mac_addr_rwcmd_done(reg32)          REG_FLD_GET(e_mac_addr_cfg_FLD_mac_addr_rwcmd_done, (reg32))
#define e_mac_addr_cfg_GET_mac_addr_llid_indx(reg32)           REG_FLD_GET(e_mac_addr_cfg_FLD_mac_addr_llid_indx, (reg32))
#define e_mac_addr_cfg_GET_mac_addr_dw_idx(reg32)              REG_FLD_GET(e_mac_addr_cfg_FLD_mac_addr_dw_idx, (reg32))

#define e_mac_addr_value_GET_mac_addr_value(reg32)             REG_FLD_GET(e_mac_addr_value_FLD_mac_addr_value, (reg32))

#define e_security_key_cfg_GET_key_rwcmd(reg32)                REG_FLD_GET(e_security_key_cfg_FLD_key_rwcmd, (reg32))
#define e_security_key_cfg_GET_key_macsec_an(reg32)            REG_FLD_GET(e_security_key_cfg_FLD_key_macsec_an, (reg32))
#define e_security_key_cfg_GET_key_rwcmd_done(reg32)           REG_FLD_GET(e_security_key_cfg_FLD_key_rwcmd_done, (reg32))
#define e_security_key_cfg_GET_key_llid_index(reg32)           REG_FLD_GET(e_security_key_cfg_FLD_key_llid_index, (reg32))
#define e_security_key_cfg_GET_key_idx(reg32)                  REG_FLD_GET(e_security_key_cfg_FLD_key_idx, (reg32))
#define e_security_key_cfg_GET_key_dw_indx(reg32)              REG_FLD_GET(e_security_key_cfg_FLD_key_dw_indx, (reg32))

#define e_key_value_GET_key_value(reg32)                       REG_FLD_GET(e_key_value_FLD_key_value, (reg32))

#define e_enckey_cfg_GET_enckey_rwcmd(reg32)                   REG_FLD_GET(e_enckey_cfg_FLD_enckey_rwcmd, (reg32))
#define e_enckey_cfg_GET_enckey_macsec_an(reg32)               REG_FLD_GET(e_enckey_cfg_FLD_enckey_macsec_an, (reg32))
#define e_enckey_cfg_GET_enckey_rwcmd_done(reg32)              REG_FLD_GET(e_enckey_cfg_FLD_enckey_rwcmd_done, (reg32))
#define e_enckey_cfg_GET_enckey_llididx(reg32)                 REG_FLD_GET(e_enckey_cfg_FLD_enckey_llididx, (reg32))
#define e_enckey_cfg_GET_enckey_keyidx(reg32)                  REG_FLD_GET(e_enckey_cfg_FLD_enckey_keyidx, (reg32))
#define e_enckey_cfg_GET_enckey_dwidx(reg32)                   REG_FLD_GET(e_enckey_cfg_FLD_enckey_dwidx, (reg32))

#define e_enckey_val_GET_enckey_value(reg32)                   REG_FLD_GET(e_enckey_val_FLD_enckey_value, (reg32))

#define e_cfg_dmy5_GET_xe_cfg_dmy5(reg32)                      REG_FLD_GET(e_cfg_dmy5_FLD_xe_cfg_dmy5, (reg32))

#define e_rpt_cfg_GET_llid15_rpt_cfg(reg32)                    REG_FLD_GET(e_rpt_cfg_FLD_llid15_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid14_rpt_cfg(reg32)                    REG_FLD_GET(e_rpt_cfg_FLD_llid14_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid13_rpt_cfg(reg32)                    REG_FLD_GET(e_rpt_cfg_FLD_llid13_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid12_rpt_cfg(reg32)                    REG_FLD_GET(e_rpt_cfg_FLD_llid12_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid11_rpt_cfg(reg32)                    REG_FLD_GET(e_rpt_cfg_FLD_llid11_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid10_rpt_cfg(reg32)                    REG_FLD_GET(e_rpt_cfg_FLD_llid10_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid9_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid9_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid8_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid8_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid7_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid7_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid6_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid6_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid5_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid5_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid4_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid4_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid3_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid3_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid2_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid2_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid1_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid1_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid0_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid0_rpt_cfg, (reg32))

#define e_rpt_cfg2_GET_llid31_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid31_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid30_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid30_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid29_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid29_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid28_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid28_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid27_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid27_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid26_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid26_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid25_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid25_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid24_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid24_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid23_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid23_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid22_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid22_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid21_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid21_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid20_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid20_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid19_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid19_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid18_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid18_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid17_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid17_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid16_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid16_rpt_cfg, (reg32))

#define e_rpt_qthld_cfg_GET_qthld_rwcmd(reg32)                 REG_FLD_GET(e_rpt_qthld_cfg_FLD_qthld_rwcmd, (reg32))
#define e_rpt_qthld_cfg_GET_qthld_rwcmd_done(reg32)            REG_FLD_GET(e_rpt_qthld_cfg_FLD_qthld_rwcmd_done, (reg32))
#define e_rpt_qthld_cfg_GET_rpt_llid_idx(reg32)                REG_FLD_GET(e_rpt_qthld_cfg_FLD_rpt_llid_idx, (reg32))
#define e_rpt_qthld_cfg_GET_qthld_value(reg32)                 REG_FLD_GET(e_rpt_qthld_cfg_FLD_qthld_value, (reg32))
#define e_rpt_qthld_cfg_GET_qthld_idx(reg32)                   REG_FLD_GET(e_rpt_qthld_cfg_FLD_qthld_idx, (reg32))
#define e_rpt_qthld_cfg_GET_queue_idx(reg32)                   REG_FLD_GET(e_rpt_qthld_cfg_FLD_queue_idx, (reg32))

#define e_rpt_glb_cfg_GET_rpt_dmy1(reg32)                      REG_FLD_GET(e_rpt_glb_cfg_FLD_rpt_dmy1, (reg32))
#define e_rpt_glb_cfg_GET_dba_gtthr_frpt(reg32)                REG_FLD_GET(e_rpt_glb_cfg_FLD_dba_gtthr_frpt, (reg32))
#define e_rpt_glb_cfg_GET_txgnt_cnthit_frpt(reg32)             REG_FLD_GET(e_rpt_glb_cfg_FLD_txgnt_cnthit_frpt, (reg32))
#define e_rpt_glb_cfg_GET_ctrlpkt_frpt(reg32)                  REG_FLD_GET(e_rpt_glb_cfg_FLD_ctrlpkt_frpt, (reg32))
#define e_rpt_glb_cfg_GET_gntfrpt_ignore(reg32)                REG_FLD_GET(e_rpt_glb_cfg_FLD_gntfrpt_ignore, (reg32))
#define e_rpt_glb_cfg_GET_rpt_qsize_sp(reg32)                  REG_FLD_GET(e_rpt_glb_cfg_FLD_rpt_qsize_sp, (reg32))
#define e_rpt_glb_cfg_GET_rpt_fullqsize_head(reg32)            REG_FLD_GET(e_rpt_glb_cfg_FLD_rpt_fullqsize_head, (reg32))
#define e_rpt_glb_cfg_GET_rpt_dpoe_thrstop(reg32)              REG_FLD_GET(e_rpt_glb_cfg_FLD_rpt_dpoe_thrstop, (reg32))
#define e_rpt_glb_cfg_GET_rpt_dpoe_queidx(reg32)               REG_FLD_GET(e_rpt_glb_cfg_FLD_rpt_dpoe_queidx, (reg32))
#define e_rpt_glb_cfg_GET_rpt_dpoe_en(reg32)                   REG_FLD_GET(e_rpt_glb_cfg_FLD_rpt_dpoe_en, (reg32))
#define e_rpt_glb_cfg_GET_rpt_qsize_dec(reg32)                 REG_FLD_GET(e_rpt_glb_cfg_FLD_rpt_qsize_dec, (reg32))
#define e_rpt_glb_cfg_GET_rpt_qsize_mgnt_sel(reg32)            REG_FLD_GET(e_rpt_glb_cfg_FLD_rpt_qsize_mgnt_sel, (reg32))
#define e_rpt_glb_cfg_GET_tx_default_rpt(reg32)                REG_FLD_GET(e_rpt_glb_cfg_FLD_tx_default_rpt, (reg32))

#define e_rpt_bmap_GET_rpt_dmy2(reg32)                         REG_FLD_GET(e_rpt_bmap_FLD_rpt_dmy2, (reg32))
#define e_rpt_bmap_GET_rpt_bitmap_ctrl(reg32)                  REG_FLD_GET(e_rpt_bmap_FLD_rpt_bitmap_ctrl, (reg32))
#define e_rpt_bmap_GET_rpt_bitmap_set(reg32)                   REG_FLD_GET(e_rpt_bmap_FLD_rpt_bitmap_set, (reg32))

#define e_frpt_thr_GET_frpt_txgnt_cnt(reg32)                   REG_FLD_GET(e_frpt_thr_FLD_frpt_txgnt_cnt, (reg32))
#define e_frpt_thr_GET_frpt_dba_totsize_thr(reg32)             REG_FLD_GET(e_frpt_thr_FLD_frpt_dba_totsize_thr, (reg32))

#define e_u1g_rpt_qsizeadj_GET_u1g_fecon_rpt_qsizeadj(reg32)   REG_FLD_GET(e_u1g_rpt_qsizeadj_FLD_u1g_fecon_rpt_qsizeadj, (reg32))
#define e_u1g_rpt_qsizeadj_GET_u1g_fecoff_rpt_qsizeadj(reg32)  REG_FLD_GET(e_u1g_rpt_qsizeadj_FLD_u1g_fecoff_rpt_qsizeadj, (reg32))

#define e_u10g_rpt_qsizeadj_GET_u10g_rpt_qsizeadj(reg32)       REG_FLD_GET(e_u10g_rpt_qsizeadj_FLD_u10g_rpt_qsizeadj, (reg32))

#define e_cfg_dmy7_GET_xe_cfg_dmy7(reg32)                      REG_FLD_GET(e_cfg_dmy7_FLD_xe_cfg_dmy7, (reg32))

#define e_cfg_dmy8_GET_xe_cfg_dmy8(reg32)                      REG_FLD_GET(e_cfg_dmy8_FLD_xe_cfg_dmy8, (reg32))

#define e_cfg_dmy9_GET_xe_cfg_dmy9(reg32)                      REG_FLD_GET(e_cfg_dmy9_FLD_xe_cfg_dmy9, (reg32))

#define e_cfg_dmy10_GET_xe_cfg_dmy10(reg32)                    REG_FLD_GET(e_cfg_dmy10_FLD_xe_cfg_dmy10, (reg32))

#define e_cfg_dmy11_GET_xe_cfg_dmy11(reg32)                    REG_FLD_GET(e_cfg_dmy11_FLD_xe_cfg_dmy11, (reg32))

#define e_cfg_dmy12_GET_xe_cfg_dmy12(reg32)                    REG_FLD_GET(e_cfg_dmy12_FLD_xe_cfg_dmy12, (reg32))

#define e_cfg_dmy13_GET_xe_cfg_dmy13(reg32)                    REG_FLD_GET(e_cfg_dmy13_FLD_xe_cfg_dmy13, (reg32))

#define e_cfg_dmy14_GET_xe_cfg_dmy14(reg32)                    REG_FLD_GET(e_cfg_dmy14_FLD_xe_cfg_dmy14, (reg32))

#define e_cfg_dmy15_GET_xe_cfg_dmy15(reg32)                    REG_FLD_GET(e_cfg_dmy15_FLD_xe_cfg_dmy15, (reg32))

#define e_dfrpt_data5_GET_dfrpt_data5(reg32)                   REG_FLD_GET(e_dfrpt_data5_FLD_dfrpt_data5, (reg32))

#define e_dfrpt_data6_GET_dfrpt_data6(reg32)                   REG_FLD_GET(e_dfrpt_data6_FLD_dfrpt_data6, (reg32))

#define e_dfrpt_data7_GET_dfrpt_data7(reg32)                   REG_FLD_GET(e_dfrpt_data7_FLD_dfrpt_data7, (reg32))

#define e_dfrpt_data8_GET_dfrpt_data8(reg32)                   REG_FLD_GET(e_dfrpt_data8_FLD_dfrpt_data8, (reg32))

#define e_dfrpt_data9_GET_dfrpt_data9(reg32)                   REG_FLD_GET(e_dfrpt_data9_FLD_dfrpt_data9, (reg32))

#define e_dfrpt_data10_GET_dfrpt_data10(reg32)                 REG_FLD_GET(e_dfrpt_data10_FLD_dfrpt_data10, (reg32))

#define e_dfrpt_data11_GET_dfrpt_data11(reg32)                 REG_FLD_GET(e_dfrpt_data11_FLD_dfrpt_data11, (reg32))

#define e_dfrpt_data12_GET_dfrpt_data12(reg32)                 REG_FLD_GET(e_dfrpt_data12_FLD_dfrpt_data12, (reg32))

#define e_dfrpt_data13_GET_dfrpt_data13(reg32)                 REG_FLD_GET(e_dfrpt_data13_FLD_dfrpt_data13, (reg32))

#define e_dfrpt_data14_GET_dfrpt_data14(reg32)                 REG_FLD_GET(e_dfrpt_data14_FLD_dfrpt_data14, (reg32))

#define e_laser_onoff_time_GET_laser_off_time(reg32)           REG_FLD_GET(e_laser_onoff_time_FLD_laser_off_time, (reg32))
#define e_laser_onoff_time_GET_laser_on_time(reg32)            REG_FLD_GET(e_laser_onoff_time_FLD_laser_on_time, (reg32))

#define e_sync_time_GET_sync_time_updte(reg32)                 REG_FLD_GET(e_sync_time_FLD_sync_time_updte, (reg32))
#define e_sync_time_GET_sync_time(reg32)                       REG_FLD_GET(e_sync_time_FLD_sync_time, (reg32))

#define e_overhead_time_thr_GET_sync_time_maxen(reg32)         REG_FLD_GET(e_overhead_time_thr_FLD_sync_time_maxen, (reg32))
#define e_overhead_time_thr_GET_lsroff_time_maxen(reg32)       REG_FLD_GET(e_overhead_time_thr_FLD_lsroff_time_maxen, (reg32))
#define e_overhead_time_thr_GET_lsron_time_maxen(reg32)        REG_FLD_GET(e_overhead_time_thr_FLD_lsron_time_maxen, (reg32))
#define e_overhead_time_thr_GET_sync_time_maxval(reg32)        REG_FLD_GET(e_overhead_time_thr_FLD_sync_time_maxval, (reg32))
#define e_overhead_time_thr_GET_lsroff_time_maxval(reg32)      REG_FLD_GET(e_overhead_time_thr_FLD_lsroff_time_maxval, (reg32))
#define e_overhead_time_thr_GET_lsron_time_maxval(reg32)       REG_FLD_GET(e_overhead_time_thr_FLD_lsron_time_maxval, (reg32))

#define e_laser_onoff_time2_GET_olt_lsroff_time(reg32)         REG_FLD_GET(e_laser_onoff_time2_FLD_olt_lsroff_time, (reg32))
#define e_laser_onoff_time2_GET_olt_lsron_time(reg32)          REG_FLD_GET(e_laser_onoff_time2_FLD_olt_lsron_time, (reg32))
#define e_laser_onoff_time2_GET_olt_sync_time(reg32)           REG_FLD_GET(e_laser_onoff_time2_FLD_olt_sync_time, (reg32))

#define e_olt_dscvinfo_GET_olt_dscvinfo_match(reg32)           REG_FLD_GET(e_olt_dscvinfo_FLD_olt_dscvinfo_match, (reg32))
#define e_olt_dscvinfo_GET_olt_dscvinfo_mis(reg32)             REG_FLD_GET(e_olt_dscvinfo_FLD_olt_dscvinfo_mis, (reg32))

#define e_grd_thrshld_GET_guard_thrshld(reg32)                 REG_FLD_GET(e_grd_thrshld_FLD_guard_thrshld, (reg32))

#define e_mpcp_timeout_intvl_GET_mpcp_timeout_intvl(reg32)     REG_FLD_GET(e_mpcp_timeout_intvl_FLD_mpcp_timeout_intvl, (reg32))

#define e_rpt_timeout_intvl_GET_rpt_timeout_intvl(reg32)       REG_FLD_GET(e_rpt_timeout_intvl_FLD_rpt_timeout_intvl, (reg32))

#define e_max_future_gnt_time_GET_max_future_gnt_time(reg32)   REG_FLD_GET(e_max_future_gnt_time_FLD_max_future_gnt_time, (reg32))

#define e_min_proc_time_GET_min_proc_time(reg32)               REG_FLD_GET(e_min_proc_time_FLD_min_proc_time, (reg32))

#define e_trx_adjust_time1_GET_tx_stm_adj(reg32)               REG_FLD_GET(e_trx_adjust_time1_FLD_tx_stm_adj, (reg32))

#define e_trx_adjust_time2_GET_tx_len_adj(reg32)               REG_FLD_GET(e_trx_adjust_time2_FLD_tx_len_adj, (reg32))
#define e_trx_adjust_time2_GET_rx_tmstp_adj(reg32)             REG_FLD_GET(e_trx_adjust_time2_FLD_rx_tmstp_adj, (reg32))

#define e_trx_adjtime3_GET_u10g_tx_tsadj(reg32)                REG_FLD_GET(e_trx_adjtime3_FLD_u10g_tx_tsadj, (reg32))
#define e_trx_adjtime3_GET_u10g_tx_stmadj(reg32)               REG_FLD_GET(e_trx_adjtime3_FLD_u10g_tx_stmadj, (reg32))

#define e_trx_adjtime4_GET_d10g_rx_tsadj(reg32)                REG_FLD_GET(e_trx_adjtime4_FLD_d10g_rx_tsadj, (reg32))
#define e_trx_adjtime4_GET_d10g_rx_tsadj_ofst(reg32)           REG_FLD_GET(e_trx_adjtime4_FLD_d10g_rx_tsadj_ofst, (reg32))

#define e_trx_adjtime5_GET_u1g_tx_tsadj(reg32)                 REG_FLD_GET(e_trx_adjtime5_FLD_u1g_tx_tsadj, (reg32))

#define e_rxphydly_adjtime_GET_rx_phydly_dft(reg32)            REG_FLD_GET(e_rxphydly_adjtime_FLD_rx_phydly_dft, (reg32))
#define e_rxphydly_adjtime_GET_d1g_rx_phydly_ofst(reg32)       REG_FLD_GET(e_rxphydly_adjtime_FLD_d1g_rx_phydly_ofst, (reg32))
#define e_rxphydly_adjtime_GET_d1g_rx_phydly(reg32)            REG_FLD_GET(e_rxphydly_adjtime_FLD_d1g_rx_phydly, (reg32))
#define e_rxphydly_adjtime_GET_d10g_rx_phydly_ofst(reg32)      REG_FLD_GET(e_rxphydly_adjtime_FLD_d10g_rx_phydly_ofst, (reg32))
#define e_rxphydly_adjtime_GET_d10g_rx_phydly(reg32)           REG_FLD_GET(e_rxphydly_adjtime_FLD_d10g_rx_phydly, (reg32))

#define e_txfetch_cfg_GET_tx_fetch_leadtime(reg32)             REG_FLD_GET(e_txfetch_cfg_FLD_tx_fetch_leadtime, (reg32))
#define e_txfetch_cfg_GET_tx_dma_leadtime(reg32)               REG_FLD_GET(e_txfetch_cfg_FLD_tx_dma_leadtime, (reg32))

#define e_tx_cal_cnst_GET_dscvr_gnt_len(reg32)                 REG_FLD_GET(e_tx_cal_cnst_FLD_dscvr_gnt_len, (reg32))
#define e_tx_cal_cnst_GET_fec_tail_grd(reg32)                  REG_FLD_GET(e_tx_cal_cnst_FLD_fec_tail_grd, (reg32))
#define e_tx_cal_cnst_GET_tail_grd(reg32)                      REG_FLD_GET(e_tx_cal_cnst_FLD_tail_grd, (reg32))
#define e_tx_cal_cnst_GET_default_ovrhd(reg32)                 REG_FLD_GET(e_tx_cal_cnst_FLD_default_ovrhd, (reg32))

#define e_txcal_cnst2_GET_ipgalign_mtd(reg32)                  REG_FLD_GET(e_txcal_cnst2_FLD_ipgalign_mtd, (reg32))
#define e_txcal_cnst2_GET_rpt_feccal_cnt_multival(reg32)       REG_FLD_GET(e_txcal_cnst2_FLD_rpt_feccal_cnt_multival, (reg32))
#define e_txcal_cnst2_GET_u10g_dscv_gntlen(reg32)              REG_FLD_GET(e_txcal_cnst2_FLD_u10g_dscv_gntlen, (reg32))
#define e_txcal_cnst2_GET_u10g_tail_grd(reg32)                 REG_FLD_GET(e_txcal_cnst2_FLD_u10g_tail_grd, (reg32))

#define e_txcal_cnst3_GET_u1g_fecon_min_gntlen(reg32)          REG_FLD_GET(e_txcal_cnst3_FLD_u1g_fecon_min_gntlen, (reg32))
#define e_txcal_cnst3_GET_u10g_eoblen(reg32)                   REG_FLD_GET(e_txcal_cnst3_FLD_u10g_eoblen, (reg32))
#define e_txcal_cnst3_GET_u10g_min_gntlen(reg32)               REG_FLD_GET(e_txcal_cnst3_FLD_u10g_min_gntlen, (reg32))
#define e_txcal_cnst3_GET_u1g_fecoff_min_gntlen(reg32)         REG_FLD_GET(e_txcal_cnst3_FLD_u1g_fecoff_min_gntlen, (reg32))

#define e_txsch_cfg_GET_txfifo_wr_thren(reg32)                 REG_FLD_GET(e_txsch_cfg_FLD_txfifo_wr_thren, (reg32))
#define e_txsch_cfg_GET_txfifo_wr_thr(reg32)                   REG_FLD_GET(e_txsch_cfg_FLD_txfifo_wr_thr, (reg32))
#define e_txsch_cfg_GET_txsch_dmy0(reg32)                      REG_FLD_GET(e_txsch_cfg_FLD_txsch_dmy0, (reg32))
#define e_txsch_cfg_GET_txfifo_pad_hthr(reg32)                 REG_FLD_GET(e_txsch_cfg_FLD_txfifo_pad_hthr, (reg32))
#define e_txsch_cfg_GET_txsch_dmy1(reg32)                      REG_FLD_GET(e_txsch_cfg_FLD_txsch_dmy1, (reg32))
#define e_txsch_cfg_GET_txfifo_pad_lthr(reg32)                 REG_FLD_GET(e_txsch_cfg_FLD_txfifo_pad_lthr, (reg32))

#define e_rxfifo_thr_GET_rx_dmy1(reg32)                        REG_FLD_GET(e_rxfifo_thr_FLD_rx_dmy1, (reg32))
#define e_rxfifo_thr_GET_rx_dmy0(reg32)                        REG_FLD_GET(e_rxfifo_thr_FLD_rx_dmy0, (reg32))
#define e_rxfifo_thr_GET_rxfifo_mbithr(reg32)                  REG_FLD_GET(e_rxfifo_thr_FLD_rxfifo_mbithr, (reg32))

#define e_bcllid_cfg_GET_d10g_bcllid(reg32)                    REG_FLD_GET(e_bcllid_cfg_FLD_d10g_bcllid, (reg32))
#define e_bcllid_cfg_GET_d1g_bcllid(reg32)                     REG_FLD_GET(e_bcllid_cfg_FLD_d1g_bcllid, (reg32))

#define e_txfrm_cfg1_GET_tx_mpcp_addrl(reg32)                  REG_FLD_GET(e_txfrm_cfg1_FLD_tx_mpcp_addrl, (reg32))

#define e_txfrm_cfg2_GET_tx_mpcp_addrh(reg32)                  REG_FLD_GET(e_txfrm_cfg2_FLD_tx_mpcp_addrh, (reg32))
#define e_txfrm_cfg2_GET_tx_mpcp_etype(reg32)                  REG_FLD_GET(e_txfrm_cfg2_FLD_tx_mpcp_etype, (reg32))

#define e_txfrm_cfg3_GET_tx_rgreq_op(reg32)                    REG_FLD_GET(e_txfrm_cfg3_FLD_tx_rgreq_op, (reg32))
#define e_txfrm_cfg3_GET_tx_rgack_op(reg32)                    REG_FLD_GET(e_txfrm_cfg3_FLD_tx_rgack_op, (reg32))

#define e_tod_sync_x_GET_tod_sync_x(reg32)                     REG_FLD_GET(e_tod_sync_x_FLD_tod_sync_x, (reg32))

#define e_tod_ltncy_GET_rl_rx_phydly_ofst(reg32)               REG_FLD_GET(e_tod_ltncy_FLD_rl_rx_phydly_ofst, (reg32))
#define e_tod_ltncy_GET_ingrs_latency(reg32)                   REG_FLD_GET(e_tod_ltncy_FLD_ingrs_latency, (reg32))
#define e_tod_ltncy_GET_egrs_latency(reg32)                    REG_FLD_GET(e_tod_ltncy_FLD_egrs_latency, (reg32))

#define e_new_tod_p2p_offset_sec_l32_GET_new_tod_p2p_offset_sec_l32(reg32) REG_FLD_GET(e_new_tod_p2p_offset_sec_l32_FLD_new_tod_p2p_offset_sec_l32, (reg32))

#define e_new_tod_p2p_tod_offset_nsec_GET_new_tod_nsec(reg32)  REG_FLD_GET(e_new_tod_p2p_tod_offset_nsec_FLD_new_tod_nsec, (reg32))

#define e_tod_p2p_tod_sec_l32_GET_tod_p2p_sec_l32(reg32)       REG_FLD_GET(e_tod_p2p_tod_sec_l32_FLD_tod_p2p_sec_l32, (reg32))

#define e_tod_p2p_tod_nsec_GET_tod_p2p_nsec(reg32)             REG_FLD_GET(e_tod_p2p_tod_nsec_FLD_tod_p2p_nsec, (reg32))

#define e_tod_period_GET_tod_period(reg32)                     REG_FLD_GET(e_tod_period_FLD_tod_period, (reg32))

#define e_tod_1pps_ctrl_GET_tod_1pps_width_ctrl(reg32)         REG_FLD_GET(e_tod_1pps_ctrl_FLD_tod_1pps_width_ctrl, (reg32))

#define e_pwr_sv_cfg_GET_pwd_mode(reg32)                       REG_FLD_GET(e_pwr_sv_cfg_FLD_pwd_mode, (reg32))
#define e_pwr_sv_cfg_GET_timedrift_ignore(reg32)               REG_FLD_GET(e_pwr_sv_cfg_FLD_timedrift_ignore, (reg32))
#define e_pwr_sv_cfg_GET_onu_wakeup(reg32)                     REG_FLD_GET(e_pwr_sv_cfg_FLD_onu_wakeup, (reg32))
#define e_pwr_sv_cfg_GET_pwr_sv_start(reg32)                   REG_FLD_GET(e_pwr_sv_cfg_FLD_pwr_sv_start, (reg32))
#define e_pwr_sv_cfg_GET_pwd_mode_i(reg32)                     REG_FLD_GET(e_pwr_sv_cfg_FLD_pwd_mode_i, (reg32))
#define e_pwr_sv_cfg_GET_slp_duration_max_h(reg32)             REG_FLD_GET(e_pwr_sv_cfg_FLD_slp_duration_max_h, (reg32))

#define e_slp_durt_max_GET_slp_duration_max_l(reg32)           REG_FLD_GET(e_slp_durt_max_FLD_slp_duration_max_l, (reg32))

#define e_slp_duration_GET_slp_duration(reg32)                 REG_FLD_GET(e_slp_duration_FLD_slp_duration, (reg32))

#define e_act_duration_GET_act_duration(reg32)                 REG_FLD_GET(e_act_duration_FLD_act_duration, (reg32))

#define e_pwron_dly_GET_pwron_dly(reg32)                       REG_FLD_GET(e_pwron_dly_FLD_pwron_dly, (reg32))

#define e_slp_duration_i_GET_rx_slpalw_pwd_mode(reg32)         REG_FLD_GET(e_slp_duration_i_FLD_rx_slpalw_pwd_mode, (reg32))
#define e_slp_duration_i_GET_rx_slpalw_flag(reg32)             REG_FLD_GET(e_slp_duration_i_FLD_rx_slpalw_flag, (reg32))
#define e_slp_duration_i_GET_rx_slpalw_duration(reg32)         REG_FLD_GET(e_slp_duration_i_FLD_rx_slpalw_duration, (reg32))

#define e_oui_a_cfg_GET_oui_a_en(reg32)                        REG_FLD_GET(e_oui_a_cfg_FLD_oui_a_en, (reg32))
#define e_oui_a_cfg_GET_oui_a_dmy(reg32)                       REG_FLD_GET(e_oui_a_cfg_FLD_oui_a_dmy, (reg32))
#define e_oui_a_cfg_GET_oui_a_val(reg32)                       REG_FLD_GET(e_oui_a_cfg_FLD_oui_a_val, (reg32))

#define e_oui_b_cfg_GET_oui_b_en(reg32)                        REG_FLD_GET(e_oui_b_cfg_FLD_oui_b_en, (reg32))
#define e_oui_b_cfg_GET_oui_b_dmy(reg32)                       REG_FLD_GET(e_oui_b_cfg_FLD_oui_b_dmy, (reg32))
#define e_oui_b_cfg_GET_oui_b_val(reg32)                       REG_FLD_GET(e_oui_b_cfg_FLD_oui_b_val, (reg32))

#define e_oui_c_cfg_GET_oui_c_en(reg32)                        REG_FLD_GET(e_oui_c_cfg_FLD_oui_c_en, (reg32))
#define e_oui_c_cfg_GET_oui_c_dmy(reg32)                       REG_FLD_GET(e_oui_c_cfg_FLD_oui_c_dmy, (reg32))
#define e_oui_c_cfg_GET_oui_c_val(reg32)                       REG_FLD_GET(e_oui_c_cfg_FLD_oui_c_val, (reg32))

#define e_dyinggsp_cfg_GET_hw_dygasp_en(reg32)                 REG_FLD_GET(e_dyinggsp_cfg_FLD_hw_dygasp_en, (reg32))
#define e_dyinggsp_cfg_GET_sw_init_dygasp(reg32)               REG_FLD_GET(e_dyinggsp_cfg_FLD_sw_init_dygasp, (reg32))
#define e_dyinggsp_cfg_GET_dygasp_num_of_times(reg32)          REG_FLD_GET(e_dyinggsp_cfg_FLD_dygasp_num_of_times, (reg32))

#define e_dyinggsp_w1_GET_dyinggsp_w1_len_type(reg32)          REG_FLD_GET(e_dyinggsp_w1_FLD_dyinggsp_w1_len_type, (reg32))

#define e_dyinggsp_w2_GET_dyinggsp_w2(reg32)                   REG_FLD_GET(e_dyinggsp_w2_FLD_dyinggsp_w2, (reg32))

#define e_dyinggsp_w3_GET_dyinggsp_w3(reg32)                   REG_FLD_GET(e_dyinggsp_w3_FLD_dyinggsp_w3, (reg32))

#define e_dyinggsp_w4_GET_dyinggsp_w4(reg32)                   REG_FLD_GET(e_dyinggsp_w4_FLD_dyinggsp_w4, (reg32))

#define e_dyinggsp_w5_GET_dyinggsp_w5(reg32)                   REG_FLD_GET(e_dyinggsp_w5_FLD_dyinggsp_w5, (reg32))

#define e_dyinggsp_w6_GET_dyinggsp_w6(reg32)                   REG_FLD_GET(e_dyinggsp_w6_FLD_dyinggsp_w6, (reg32))

#define e_dyinggsp_w7_GET_dyinggsp_w7(reg32)                   REG_FLD_GET(e_dyinggsp_w7_FLD_dyinggsp_w7, (reg32))

#define e_dyinggsp_w8_GET_dyinggsp_w8(reg32)                   REG_FLD_GET(e_dyinggsp_w8_FLD_dyinggsp_w8, (reg32))

#define e_dyinggsp_w9_GET_dyinggsp_w9(reg32)                   REG_FLD_GET(e_dyinggsp_w9_FLD_dyinggsp_w9, (reg32))

#define e_dyinggsp_w10_GET_dyinggsp_w10(reg32)                 REG_FLD_GET(e_dyinggsp_w10_FLD_dyinggsp_w10, (reg32))

#define e_dyinggsp_w11_GET_dyinggsp_w11(reg32)                 REG_FLD_GET(e_dyinggsp_w11_FLD_dyinggsp_w11, (reg32))

#define e_dyinggsp_w12_GET_dyinggsp_w12(reg32)                 REG_FLD_GET(e_dyinggsp_w12_FLD_dyinggsp_w12, (reg32))

#define e_oam_kpalv_ctrl_GET_oam_kpalv_llid_idx(reg32)         REG_FLD_GET(e_oam_kpalv_ctrl_FLD_oam_kpalv_llid_idx, (reg32))
#define e_oam_kpalv_ctrl_GET_oam_kpalv_interval(reg32)         REG_FLD_GET(e_oam_kpalv_ctrl_FLD_oam_kpalv_interval, (reg32))
#define e_oam_kpalv_ctrl_GET_oam_kpalv_sw_trig(reg32)          REG_FLD_GET(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_trig, (reg32))
#define e_oam_kpalv_ctrl_GET_oam_kpalv_sw_cfg(reg32)           REG_FLD_GET(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_cfg, (reg32))
#define e_oam_kpalv_ctrl_GET_oam_kpalv_en(reg32)               REG_FLD_GET(e_oam_kpalv_ctrl_FLD_oam_kpalv_en, (reg32))

#define e_oam_kpalv_w1_GET_oam_kpalv_w1(reg32)                 REG_FLD_GET(e_oam_kpalv_w1_FLD_oam_kpalv_w1, (reg32))

#define e_oam_kpalv_w2_GET_oam_kpalv_w2(reg32)                 REG_FLD_GET(e_oam_kpalv_w2_FLD_oam_kpalv_w2, (reg32))

#define e_oam_kpalv_w3_GET_oam_kpalv_w3(reg32)                 REG_FLD_GET(e_oam_kpalv_w3_FLD_oam_kpalv_w3, (reg32))

#define e_oam_kpalv_w4_GET_oam_kpalv_w4(reg32)                 REG_FLD_GET(e_oam_kpalv_w4_FLD_oam_kpalv_w4, (reg32))

#define e_oam_kpalv_w5_GET_oam_kpalv_w5(reg32)                 REG_FLD_GET(e_oam_kpalv_w5_FLD_oam_kpalv_w5, (reg32))

#define e_oam_kpalv_w6_GET_oam_kpalv_w6(reg32)                 REG_FLD_GET(e_oam_kpalv_w6_FLD_oam_kpalv_w6, (reg32))

#define e_oam_kpalv_w7_GET_oam_kpalv_w7(reg32)                 REG_FLD_GET(e_oam_kpalv_w7_FLD_oam_kpalv_w7, (reg32))

#define e_oam_kpalv_w8_GET_oam_kpalv_w8(reg32)                 REG_FLD_GET(e_oam_kpalv_w8_FLD_oam_kpalv_w8, (reg32))

#define e_oam_kpalv_w9_GET_oam_kpalv_w9(reg32)                 REG_FLD_GET(e_oam_kpalv_w9_FLD_oam_kpalv_w9, (reg32))

#define e_oam_kpalv_w10_GET_oam_kpalv_w10(reg32)               REG_FLD_GET(e_oam_kpalv_w10_FLD_oam_kpalv_w10, (reg32))

#define e_oam_kpalv_w11_GET_oam_kpalv_w11(reg32)               REG_FLD_GET(e_oam_kpalv_w11_FLD_oam_kpalv_w11, (reg32))

#define e_oam_kpalv_w12_GET_oam_kpalv_w12(reg32)               REG_FLD_GET(e_oam_kpalv_w12_FLD_oam_kpalv_w12, (reg32))

#define e_kpalv_hwen_sts_GET_kpalv_hwen_sts(reg32)             REG_FLD_GET(e_kpalv_hwen_sts_FLD_kpalv_hwen_sts, (reg32))

#define e_mcgrp_en_GET_mcgrp_llid_en(reg32)                    REG_FLD_GET(e_mcgrp_en_FLD_mcgrp_llid_en, (reg32))

#define e_rxuni_mcllid_cfg0_GET_mcgrp_llid1_val(reg32)         REG_FLD_GET(e_rxuni_mcllid_cfg0_FLD_mcgrp_llid1_val, (reg32))
#define e_rxuni_mcllid_cfg0_GET_mcgrp_llid0_val(reg32)         REG_FLD_GET(e_rxuni_mcllid_cfg0_FLD_mcgrp_llid0_val, (reg32))

#define e_rxuni_mcllid_cfg1_GET_mcgrp_llid3_val(reg32)         REG_FLD_GET(e_rxuni_mcllid_cfg1_FLD_mcgrp_llid3_val, (reg32))
#define e_rxuni_mcllid_cfg1_GET_mcgrp_llid2_val(reg32)         REG_FLD_GET(e_rxuni_mcllid_cfg1_FLD_mcgrp_llid2_val, (reg32))

#define e_rxuni_mcllid_cfg2_GET_mcgrp_llid5_val(reg32)         REG_FLD_GET(e_rxuni_mcllid_cfg2_FLD_mcgrp_llid5_val, (reg32))
#define e_rxuni_mcllid_cfg2_GET_mcgrp_llid4_val(reg32)         REG_FLD_GET(e_rxuni_mcllid_cfg2_FLD_mcgrp_llid4_val, (reg32))

#define e_rxuni_mcllid_cfg3_GET_mcgrp_llid7_val(reg32)         REG_FLD_GET(e_rxuni_mcllid_cfg3_FLD_mcgrp_llid7_val, (reg32))
#define e_rxuni_mcllid_cfg3_GET_mcgrp_llid6_val(reg32)         REG_FLD_GET(e_rxuni_mcllid_cfg3_FLD_mcgrp_llid6_val, (reg32))

#define e_rxuni_mcllid_cfg4_GET_mcgrp_llid9_val(reg32)         REG_FLD_GET(e_rxuni_mcllid_cfg4_FLD_mcgrp_llid9_val, (reg32))
#define e_rxuni_mcllid_cfg4_GET_mcgrp_llid8_val(reg32)         REG_FLD_GET(e_rxuni_mcllid_cfg4_FLD_mcgrp_llid8_val, (reg32))

#define e_rxuni_mcllid_cfg5_GET_mcgrp_llid11_val(reg32)        REG_FLD_GET(e_rxuni_mcllid_cfg5_FLD_mcgrp_llid11_val, (reg32))
#define e_rxuni_mcllid_cfg5_GET_mcgrp_llid10_val(reg32)        REG_FLD_GET(e_rxuni_mcllid_cfg5_FLD_mcgrp_llid10_val, (reg32))

#define e_rxuni_mcllid_cfg6_GET_mcgrp_llid13_val(reg32)        REG_FLD_GET(e_rxuni_mcllid_cfg6_FLD_mcgrp_llid13_val, (reg32))
#define e_rxuni_mcllid_cfg6_GET_mcgrp_llid12_val(reg32)        REG_FLD_GET(e_rxuni_mcllid_cfg6_FLD_mcgrp_llid12_val, (reg32))

#define e_rxuni_mcllid_cfg7_GET_mcgrp_llid15_val(reg32)        REG_FLD_GET(e_rxuni_mcllid_cfg7_FLD_mcgrp_llid15_val, (reg32))
#define e_rxuni_mcllid_cfg7_GET_mcgrp_llid14_val(reg32)        REG_FLD_GET(e_rxuni_mcllid_cfg7_FLD_mcgrp_llid14_val, (reg32))

#define e_crpt_cfg_GET_crpt_dmy0(reg32)                        REG_FLD_GET(e_crpt_cfg_FLD_crpt_dmy0, (reg32))
#define e_crpt_cfg_GET_kpalv_encrpt_en(reg32)                  REG_FLD_GET(e_crpt_cfg_FLD_kpalv_encrpt_en, (reg32))
#define e_crpt_cfg_GET_mpcp_encrpt_en(reg32)                   REG_FLD_GET(e_crpt_cfg_FLD_mpcp_encrpt_en, (reg32))
#define e_crpt_cfg_GET_dygasp_encrpt_en(reg32)                 REG_FLD_GET(e_crpt_cfg_FLD_dygasp_encrpt_en, (reg32))
#define e_crpt_cfg_GET_llidalluc_decrpt_en(reg32)              REG_FLD_GET(e_crpt_cfg_FLD_llidalluc_decrpt_en, (reg32))
#define e_crpt_cfg_GET_llidmc_decrpt_en(reg32)                 REG_FLD_GET(e_crpt_cfg_FLD_llidmc_decrpt_en, (reg32))
#define e_crpt_cfg_GET_llidbc_decrpt_en(reg32)                 REG_FLD_GET(e_crpt_cfg_FLD_llidbc_decrpt_en, (reg32))
#define e_crpt_cfg_GET_mcgrp_decrpt_en(reg32)                  REG_FLD_GET(e_crpt_cfg_FLD_mcgrp_decrpt_en, (reg32))
#define e_crpt_cfg_GET_crpt_dmy1(reg32)                        REG_FLD_GET(e_crpt_cfg_FLD_crpt_dmy1, (reg32))
#define e_crpt_cfg_GET_encrpt_mode(reg32)                      REG_FLD_GET(e_crpt_cfg_FLD_encrpt_mode, (reg32))
#define e_crpt_cfg_GET_crcchk_mode(reg32)                      REG_FLD_GET(e_crpt_cfg_FLD_crcchk_mode, (reg32))
#define e_crpt_cfg_GET_decrpt_mode(reg32)                      REG_FLD_GET(e_crpt_cfg_FLD_decrpt_mode, (reg32))

#define e_cfbsec_cfg_GET_cfbsec_dmy(reg32)                     REG_FLD_GET(e_cfbsec_cfg_FLD_cfbsec_dmy, (reg32))
#define e_cfbsec_cfg_GET_cfbdec_iv_srcsel(reg32)               REG_FLD_GET(e_cfbsec_cfg_FLD_cfbdec_iv_srcsel, (reg32))
#define e_cfbsec_cfg_GET_cfbdec_padd_mode(reg32)               REG_FLD_GET(e_cfbsec_cfg_FLD_cfbdec_padd_mode, (reg32))

#define e_ctrsec_cfg_GET_ctrsec_dmy0(reg32)                    REG_FLD_GET(e_ctrsec_cfg_FLD_ctrsec_dmy0, (reg32))
#define e_ctrsec_cfg_GET_ctrenc_padd_mode(reg32)               REG_FLD_GET(e_ctrsec_cfg_FLD_ctrenc_padd_mode, (reg32))
#define e_ctrsec_cfg_GET_ctrsec_dmy1(reg32)                    REG_FLD_GET(e_ctrsec_cfg_FLD_ctrsec_dmy1, (reg32))
#define e_ctrsec_cfg_GET_ctrdec_srcaddr_sel(reg32)             REG_FLD_GET(e_ctrsec_cfg_FLD_ctrdec_srcaddr_sel, (reg32))
#define e_ctrsec_cfg_GET_ctrdec_padd_mode(reg32)               REG_FLD_GET(e_ctrsec_cfg_FLD_ctrdec_padd_mode, (reg32))

#define e_desec_srcaddr_cfg_GET_desec_srcaddr_rwcmd(reg32)     REG_FLD_GET(e_desec_srcaddr_cfg_FLD_desec_srcaddr_rwcmd, (reg32))
#define e_desec_srcaddr_cfg_GET_desec_srcaddr_rwcmd_done(reg32) REG_FLD_GET(e_desec_srcaddr_cfg_FLD_desec_srcaddr_rwcmd_done, (reg32))
#define e_desec_srcaddr_cfg_GET_desec_srcaddr_llid_idx(reg32)  REG_FLD_GET(e_desec_srcaddr_cfg_FLD_desec_srcaddr_llid_idx, (reg32))
#define e_desec_srcaddr_cfg_GET_desec_srcaddr_value_h(reg32)   REG_FLD_GET(e_desec_srcaddr_cfg_FLD_desec_srcaddr_value_h, (reg32))

#define e_desec_srcaddr_val_GET_desec_srcaddr_value_l(reg32)   REG_FLD_GET(e_desec_srcaddr_val_FLD_desec_srcaddr_value_l, (reg32))

#define e_macsdec_cfg_GET_macsdec_dmy0(reg32)                  REG_FLD_GET(e_macsdec_cfg_FLD_macsdec_dmy0, (reg32))
#define e_macsdec_cfg_GET_macsdec_tag_bit_swap(reg32)          REG_FLD_GET(e_macsdec_cfg_FLD_macsdec_tag_bit_swap, (reg32))
#define e_macsdec_cfg_GET_macsdec_iv_bit_swap(reg32)           REG_FLD_GET(e_macsdec_cfg_FLD_macsdec_iv_bit_swap, (reg32))
#define e_macsdec_cfg_GET_macsdec_aad_bit_swap(reg32)          REG_FLD_GET(e_macsdec_cfg_FLD_macsdec_aad_bit_swap, (reg32))
#define e_macsdec_cfg_GET_macsdec_pt_bit_swap(reg32)           REG_FLD_GET(e_macsdec_cfg_FLD_macsdec_pt_bit_swap, (reg32))
#define e_macsdec_cfg_GET_macsdec_ct_bit_swap(reg32)           REG_FLD_GET(e_macsdec_cfg_FLD_macsdec_ct_bit_swap, (reg32))
#define e_macsdec_cfg_GET_macsdec_srcaddr_sel(reg32)           REG_FLD_GET(e_macsdec_cfg_FLD_macsdec_srcaddr_sel, (reg32))
#define e_macsdec_cfg_GET_macsdec_timout_en(reg32)             REG_FLD_GET(e_macsdec_cfg_FLD_macsdec_timout_en, (reg32))
#define e_macsdec_cfg_GET_macsdec_conofst(reg32)               REG_FLD_GET(e_macsdec_cfg_FLD_macsdec_conofst, (reg32))
#define e_macsdec_cfg_GET_sectag_e0c0_desec(reg32)             REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e0c0_desec, (reg32))
#define e_macsdec_cfg_GET_sectag_e0c0_icvchk(reg32)            REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e0c0_icvchk, (reg32))
#define e_macsdec_cfg_GET_sectag_e0c0_drop(reg32)              REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e0c0_drop, (reg32))
#define e_macsdec_cfg_GET_sectag_e0c1_desec(reg32)             REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e0c1_desec, (reg32))
#define e_macsdec_cfg_GET_sectag_e0c1_icvchk(reg32)            REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e0c1_icvchk, (reg32))
#define e_macsdec_cfg_GET_sectag_e0c1_drop(reg32)              REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e0c1_drop, (reg32))
#define e_macsdec_cfg_GET_sectag_e1c0_desec(reg32)             REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e1c0_desec, (reg32))
#define e_macsdec_cfg_GET_sectag_e1c0_icvchk(reg32)            REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e1c0_icvchk, (reg32))
#define e_macsdec_cfg_GET_sectag_e1c0_drop(reg32)              REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e1c0_drop, (reg32))
#define e_macsdec_cfg_GET_sectag_e1c1_desec(reg32)             REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e1c1_desec, (reg32))
#define e_macsdec_cfg_GET_sectag_e1c1_icvchk(reg32)            REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e1c1_icvchk, (reg32))
#define e_macsdec_cfg_GET_sectag_e1c1_drop(reg32)              REG_FLD_GET(e_macsdec_cfg_FLD_sectag_e1c1_drop, (reg32))

#define e_macsenc_cfg_GET_macsenc_mode(reg32)                  REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_mode, (reg32))
#define e_macsenc_cfg_GET_macsenc_dmy0(reg32)                  REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_dmy0, (reg32))
#define e_macsenc_cfg_GET_macsenc_tag_bit_swap(reg32)          REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_tag_bit_swap, (reg32))
#define e_macsenc_cfg_GET_macsenc_iv_bit_swap(reg32)           REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_iv_bit_swap, (reg32))
#define e_macsenc_cfg_GET_macsenc_aad_bit_swap(reg32)          REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_aad_bit_swap, (reg32))
#define e_macsenc_cfg_GET_macsenc_pt_bit_swap(reg32)           REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_pt_bit_swap, (reg32))
#define e_macsenc_cfg_GET_macsenc_ct_bit_swap(reg32)           REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_ct_bit_swap, (reg32))
#define e_macsenc_cfg_GET_macsenc_srcaddr_sel(reg32)           REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_srcaddr_sel, (reg32))
#define e_macsenc_cfg_GET_macsenc_timout_en(reg32)             REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_timout_en, (reg32))
#define e_macsenc_cfg_GET_macsenc_dmy1(reg32)                  REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_dmy1, (reg32))
#define e_macsenc_cfg_GET_macsenc_tag_v(reg32)                 REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_tag_v, (reg32))
#define e_macsenc_cfg_GET_macsenc_tag_es(reg32)                REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_tag_es, (reg32))
#define e_macsenc_cfg_GET_macsenc_tag_sc(reg32)                REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_tag_sc, (reg32))
#define e_macsenc_cfg_GET_macsenc_tag_scb(reg32)               REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_tag_scb, (reg32))
#define e_macsenc_cfg_GET_macsenc_tag_e(reg32)                 REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_tag_e, (reg32))
#define e_macsenc_cfg_GET_macsenc_tag_c(reg32)                 REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_tag_c, (reg32))
#define e_macsenc_cfg_GET_macsenc_dmy2(reg32)                  REG_FLD_GET(e_macsenc_cfg_FLD_macsenc_dmy2, (reg32))

#define e_macsenc_pnini_cfg_GET_macsenc_pn_inival(reg32)       REG_FLD_GET(e_macsenc_pnini_cfg_FLD_macsenc_pn_inival, (reg32))

#define e_macsenc_pn_cfg_GET_macsenc_pn_rwcmd(reg32)           REG_FLD_GET(e_macsenc_pn_cfg_FLD_macsenc_pn_rwcmd, (reg32))
#define e_macsenc_pn_cfg_GET_macsenc_pn_rwcmd_done(reg32)      REG_FLD_GET(e_macsenc_pn_cfg_FLD_macsenc_pn_rwcmd_done, (reg32))
#define e_macsenc_pn_cfg_GET_macsenc_pn_llid_idx(reg32)        REG_FLD_GET(e_macsenc_pn_cfg_FLD_macsenc_pn_llid_idx, (reg32))

#define e_macsenc_pn_val_GET_macsenc_pn_val(reg32)             REG_FLD_GET(e_macsenc_pn_val_FLD_macsenc_pn_val, (reg32))

#define e_olt_macaddr_h_GET_olt_macaddr_h(reg32)               REG_FLD_GET(e_olt_macaddr_h_FLD_olt_macaddr_h, (reg32))

#define e_olt_macaddr_l_GET_olt_macaddr_l(reg32)               REG_FLD_GET(e_olt_macaddr_l_FLD_olt_macaddr_l, (reg32))

#define e_snf_cfg_GET_sniffer_mode(reg32)                      REG_FLD_GET(e_snf_cfg_FLD_sniffer_mode, (reg32))
#define e_snf_cfg_GET_snf_rsv0(reg32)                          REG_FLD_GET(e_snf_cfg_FLD_snf_rsv0, (reg32))
#define e_snf_cfg_GET_llidinfo_snf(reg32)                      REG_FLD_GET(e_snf_cfg_FLD_llidinfo_snf, (reg32))
#define e_snf_cfg_GET_snf_fcserr_fwd(reg32)                    REG_FLD_GET(e_snf_cfg_FLD_snf_fcserr_fwd, (reg32))
#define e_snf_cfg_GET_snf_rsv1(reg32)                          REG_FLD_GET(e_snf_cfg_FLD_snf_rsv1, (reg32))
#define e_snf_cfg_GET_us_snf_mode(reg32)                       REG_FLD_GET(e_snf_cfg_FLD_us_snf_mode, (reg32))
#define e_snf_cfg_GET_us_kpalv_snf(reg32)                      REG_FLD_GET(e_snf_cfg_FLD_us_kpalv_snf, (reg32))
#define e_snf_cfg_GET_us_dygasp_snf(reg32)                     REG_FLD_GET(e_snf_cfg_FLD_us_dygasp_snf, (reg32))
#define e_snf_cfg_GET_us_rpt_snf(reg32)                        REG_FLD_GET(e_snf_cfg_FLD_us_rpt_snf, (reg32))
#define e_snf_cfg_GET_us_mpcp_snf(reg32)                       REG_FLD_GET(e_snf_cfg_FLD_us_mpcp_snf, (reg32))
#define e_snf_cfg_GET_us_oam_snf(reg32)                        REG_FLD_GET(e_snf_cfg_FLD_us_oam_snf, (reg32))
#define e_snf_cfg_GET_snf_rsv2(reg32)                          REG_FLD_GET(e_snf_cfg_FLD_snf_rsv2, (reg32))
#define e_snf_cfg_GET_mcgrp_eth_snf(reg32)                     REG_FLD_GET(e_snf_cfg_FLD_mcgrp_eth_snf, (reg32))
#define e_snf_cfg_GET_mcgrp_nrlgate_snf(reg32)                 REG_FLD_GET(e_snf_cfg_FLD_mcgrp_nrlgate_snf, (reg32))
#define e_snf_cfg_GET_mcgrp_oam_snf(reg32)                     REG_FLD_GET(e_snf_cfg_FLD_mcgrp_oam_snf, (reg32))
#define e_snf_cfg_GET_mcgrp_llid_snf(reg32)                    REG_FLD_GET(e_snf_cfg_FLD_mcgrp_llid_snf, (reg32))
#define e_snf_cfg_GET_alluc_eth_snf(reg32)                     REG_FLD_GET(e_snf_cfg_FLD_alluc_eth_snf, (reg32))
#define e_snf_cfg_GET_alluc_nrlgate_snf(reg32)                 REG_FLD_GET(e_snf_cfg_FLD_alluc_nrlgate_snf, (reg32))
#define e_snf_cfg_GET_alluc_oam_snf(reg32)                     REG_FLD_GET(e_snf_cfg_FLD_alluc_oam_snf, (reg32))
#define e_snf_cfg_GET_alluc_llid_snf(reg32)                    REG_FLD_GET(e_snf_cfg_FLD_alluc_llid_snf, (reg32))
#define e_snf_cfg_GET_snf_rsv3(reg32)                          REG_FLD_GET(e_snf_cfg_FLD_snf_rsv3, (reg32))
#define e_snf_cfg_GET_ds_nrlgate_snf(reg32)                    REG_FLD_GET(e_snf_cfg_FLD_ds_nrlgate_snf, (reg32))
#define e_snf_cfg_GET_ds_mpcp_snf(reg32)                       REG_FLD_GET(e_snf_cfg_FLD_ds_mpcp_snf, (reg32))

#define e_snf_sp_tag_GET_snf_sp_tag(reg32)                     REG_FLD_GET(e_snf_sp_tag_FLD_snf_sp_tag, (reg32))

#define e_snf_dah_GET_snf_pkt_dah(reg32)                       REG_FLD_GET(e_snf_dah_FLD_snf_pkt_dah, (reg32))

#define e_snf_dal_GET_snf_pkt_dal(reg32)                       REG_FLD_GET(e_snf_dal_FLD_snf_pkt_dal, (reg32))
#define e_snf_dal_GET_snf_pkt_sah(reg32)                       REG_FLD_GET(e_snf_dal_FLD_snf_pkt_sah, (reg32))

#define e_snf_sal_GET_snf_pkt_sal(reg32)                       REG_FLD_GET(e_snf_sal_FLD_snf_pkt_sal, (reg32))

#define e_snf_etype_GET_snf_pkt_etype(reg32)                   REG_FLD_GET(e_snf_etype_FLD_snf_pkt_etype, (reg32))

#define e_rdmdly_cfg_GET_rdmdly_mode(reg32)                    REG_FLD_GET(e_rdmdly_cfg_FLD_rdmdly_mode, (reg32))
#define e_rdmdly_cfg_GET_rdmdly_lthr(reg32)                    REG_FLD_GET(e_rdmdly_cfg_FLD_rdmdly_lthr, (reg32))
#define e_rdmdly_cfg_GET_rdmdly_hthr(reg32)                    REG_FLD_GET(e_rdmdly_cfg_FLD_rdmdly_hthr, (reg32))

#define e_rdmdly_stat_GET_dscvgate_gntlen(reg32)               REG_FLD_GET(e_rdmdly_stat_FLD_dscvgate_gntlen, (reg32))
#define e_rdmdly_stat_GET_rdmdly(reg32)                        REG_FLD_GET(e_rdmdly_stat_FLD_rdmdly, (reg32))

#define e_dbg_prb_sel_GET_probe_dmysel(reg32)                  REG_FLD_GET(e_dbg_prb_sel_FLD_probe_dmysel, (reg32))
#define e_dbg_prb_sel_GET_probe_dtgrp_sel(reg32)               REG_FLD_GET(e_dbg_prb_sel_FLD_probe_dtgrp_sel, (reg32))
#define e_dbg_prb_sel_GET_probe_bit0_sel(reg32)                REG_FLD_GET(e_dbg_prb_sel_FLD_probe_bit0_sel, (reg32))
#define e_dbg_prb_sel_GET_probe_sel(reg32)                     REG_FLD_GET(e_dbg_prb_sel_FLD_probe_sel, (reg32))

#define e_dbg_prb_h32_GET_probe_h32(reg32)                     REG_FLD_GET(e_dbg_prb_h32_FLD_probe_h32, (reg32))

#define e_dbg_prb_l32_GET_probe_l32(reg32)                     REG_FLD_GET(e_dbg_prb_l32_FLD_probe_l32, (reg32))

#define e_utili_cfg_GET_cf_utili_wdm(reg32)                    REG_FLD_GET(e_utili_cfg_FLD_cf_utili_wdm, (reg32))
#define e_utili_cfg_GET_cf_utili_pktovhd(reg32)                REG_FLD_GET(e_utili_cfg_FLD_cf_utili_pktovhd, (reg32))
#define e_utili_cfg_GET_cf_utili_rsv(reg32)                    REG_FLD_GET(e_utili_cfg_FLD_cf_utili_rsv, (reg32))
#define e_utili_cfg_GET_cf_utili_pkttype(reg32)                REG_FLD_GET(e_utili_cfg_FLD_cf_utili_pkttype, (reg32))
#define e_utili_cfg_GET_cf_utili_mode(reg32)                   REG_FLD_GET(e_utili_cfg_FLD_cf_utili_mode, (reg32))

#define e_utili_check_GET_cf_utilization_cntclr(reg32)         REG_FLD_GET(e_utili_check_FLD_cf_utilization_cntclr, (reg32))
#define e_utili_check_GET_cf_utilization_chken(reg32)          REG_FLD_GET(e_utili_check_FLD_cf_utilization_chken, (reg32))

#define total_gnt_sizeh_GET_total_gnt_size_h32(reg32)          REG_FLD_GET(total_gnt_sizeh_FLD_total_gnt_size_h32, (reg32))

#define total_gnt_sizel_GET_total_gnt_size_l32(reg32)          REG_FLD_GET(total_gnt_sizel_FLD_total_gnt_size_l32, (reg32))

#define total_pkt_cnt_GET_total_pkt_cnt(reg32)                 REG_FLD_GET(total_pkt_cnt_FLD_total_pkt_cnt, (reg32))

#define total_pkt_size_l_GET_total_pkt_size_l32(reg32)         REG_FLD_GET(total_pkt_size_l_FLD_total_pkt_size_l32, (reg32))

#define total_pkt_size_h_GET_total_pkt_size_h32(reg32)         REG_FLD_GET(total_pkt_size_h_FLD_total_pkt_size_h32, (reg32))

#define total_u10g_nouse_gntlen_GET_total_u10g_nouse_gntlen(reg32) REG_FLD_GET(total_u10g_nouse_gntlen_FLD_total_u10g_nouse_gntlen, (reg32))

#define total_gnt_cnt_GET_total_gnt_cnt(reg32)                 REG_FLD_GET(total_gnt_cnt_FLD_total_gnt_cnt, (reg32))

#define total_tx_cyc_h_GET_total_tx_cyc_h32(reg32)             REG_FLD_GET(total_tx_cyc_h_FLD_total_tx_cyc_h32, (reg32))

#define total_tx_cyc_l_GET_total_tx_cyc_l32(reg32)             REG_FLD_GET(total_tx_cyc_l_FLD_total_tx_cyc_l32, (reg32))

#define e_sts_dmy6_GET_xe_sts_dmy6(reg32)                      REG_FLD_GET(e_sts_dmy6_FLD_xe_sts_dmy6, (reg32))

#define e_sts_dmy7_GET_xe_sts_dmy7(reg32)                      REG_FLD_GET(e_sts_dmy7_FLD_xe_sts_dmy7, (reg32))

#define e_sts_dmy8_GET_xe_sts_dmy8(reg32)                      REG_FLD_GET(e_sts_dmy8_FLD_xe_sts_dmy8, (reg32))

#define e_sts_dmy9_GET_xe_sts_dmy9(reg32)                      REG_FLD_GET(e_sts_dmy9_FLD_xe_sts_dmy9, (reg32))

#define e_sts_dmy10_GET_xe_sts_dmy10(reg32)                    REG_FLD_GET(e_sts_dmy10_FLD_xe_sts_dmy10, (reg32))

#define e_sts_dmy26_GET_xe_sts_dmy26(reg32)                    REG_FLD_GET(e_sts_dmy26_FLD_xe_sts_dmy26, (reg32))

#define e_sts_dmy27_GET_xe_sts_dmy27(reg32)                    REG_FLD_GET(e_sts_dmy27_FLD_xe_sts_dmy27, (reg32))

#define e_sts_dmy28_GET_xe_sts_dmy28(reg32)                    REG_FLD_GET(e_sts_dmy28_FLD_xe_sts_dmy28, (reg32))

#define e_sts_dmy29_GET_xe_sts_dmy29(reg32)                    REG_FLD_GET(e_sts_dmy29_FLD_xe_sts_dmy29, (reg32))

#define e_sram_pd_GET_xemac_sram_pd(reg32)                     REG_FLD_GET(e_sram_pd_FLD_xemac_sram_pd, (reg32))

#define e_mbist_delsel_cfg0_GET_xemac_mbist_delsel_cfg0(reg32) REG_FLD_GET(e_mbist_delsel_cfg0_FLD_xemac_mbist_delsel_cfg0, (reg32))

#define e_mbist_delsel_cfg1_GET_xemac_mbist_delsel_cfg1(reg32) REG_FLD_GET(e_mbist_delsel_cfg1_FLD_xemac_mbist_delsel_cfg1, (reg32))

#define e_mbist_delsel_cfg2_GET_xemac_mbist_delsel_cfg2(reg32) REG_FLD_GET(e_mbist_delsel_cfg2_FLD_xemac_mbist_delsel_cfg2, (reg32))

#define e_mbist_delsel_cfg3_GET_xemac_mbist_delsel_cfg3(reg32) REG_FLD_GET(e_mbist_delsel_cfg3_FLD_xemac_mbist_delsel_cfg3, (reg32))

#define e_mbist_done_sts0_GET_xemac_mbist_done_sts0(reg32)     REG_FLD_GET(e_mbist_done_sts0_FLD_xemac_mbist_done_sts0, (reg32))

#define e_mbist_fail_sts0_GET_xemac_mbist_fail_sts0(reg32)     REG_FLD_GET(e_mbist_fail_sts0_FLD_xemac_mbist_fail_sts0, (reg32))

#define e_mbist_fail_sts1_GET_xemac_mbist_fail_sts1(reg32)     REG_FLD_GET(e_mbist_fail_sts1_FLD_xemac_mbist_fail_sts1, (reg32))

#define rx_sld_sts_GET_sld_b7(reg32)                           REG_FLD_GET(rx_sld_sts_FLD_sld_b7, (reg32))
#define rx_sld_sts_GET_sld_b6(reg32)                           REG_FLD_GET(rx_sld_sts_FLD_sld_b6, (reg32))
#define rx_sld_sts_GET_sld_b5(reg32)                           REG_FLD_GET(rx_sld_sts_FLD_sld_b5, (reg32))
#define rx_sld_sts_GET_sld_b4(reg32)                           REG_FLD_GET(rx_sld_sts_FLD_sld_b4, (reg32))
#define rx_sld_sts_GET_sld_b3(reg32)                           REG_FLD_GET(rx_sld_sts_FLD_sld_b3, (reg32))
#define rx_sld_sts_GET_sld_b2(reg32)                           REG_FLD_GET(rx_sld_sts_FLD_sld_b2, (reg32))
#define rx_sld_sts_GET_sld_b1(reg32)                           REG_FLD_GET(rx_sld_sts_FLD_sld_b1, (reg32))
#define rx_sld_sts_GET_sld_b0(reg32)                           REG_FLD_GET(rx_sld_sts_FLD_sld_b0, (reg32))

#define e_glue_cfg_GET_txmpi_fifound_pktgate_gntcnt(reg32)     REG_FLD_GET(e_glue_cfg_FLD_txmpi_fifound_pktgate_gntcnt, (reg32))
#define e_glue_cfg_GET_txmpi_fifound_pktgate_en(reg32)         REG_FLD_GET(e_glue_cfg_FLD_txmpi_fifound_pktgate_en, (reg32))
#define e_glue_cfg_GET_txmpi_fifound_thr(reg32)                REG_FLD_GET(e_glue_cfg_FLD_txmpi_fifound_thr, (reg32))

#define e_gntreq_tmout_GET_gntreq_grden(reg32)                 REG_FLD_GET(e_gntreq_tmout_FLD_gntreq_grden, (reg32))
#define e_gntreq_tmout_GET_gntreq_grdcyc(reg32)                REG_FLD_GET(e_gntreq_tmout_FLD_gntreq_grdcyc, (reg32))

#define e_fpga_gendef_GET_fpga_gendef(reg32)                   REG_FLD_GET(e_fpga_gendef_FLD_fpga_gendef, (reg32))

#define e_fpga_genver_GET_fpga_genver(reg32)                   REG_FLD_GET(e_fpga_genver_FLD_fpga_genver, (reg32))

#define e_cnt_clr_GET_glb_cntclr(reg32)                        REG_FLD_GET(e_cnt_clr_FLD_glb_cntclr, (reg32))

#define e_rxmbi_eth_cnt_GET_rxmbi_eth_cnt(reg32)               REG_FLD_GET(e_rxmbi_eth_cnt_FLD_rxmbi_eth_cnt, (reg32))

#define e_rxmpi_eth_cnt_GET_rxmpi_eth_cnt(reg32)               REG_FLD_GET(e_rxmpi_eth_cnt_FLD_rxmpi_eth_cnt, (reg32))

#define e_txmbi_eth_cnt_GET_txmbi_eth_cnt(reg32)               REG_FLD_GET(e_txmbi_eth_cnt_FLD_txmbi_eth_cnt, (reg32))

#define e_txmpi_eth_cnt_GET_txmpi_eth_cnt(reg32)               REG_FLD_GET(e_txmpi_eth_cnt_FLD_txmpi_eth_cnt, (reg32))

#define e_oam_stat_GET_rx_oam_cnt(reg32)                       REG_FLD_GET(e_oam_stat_FLD_rx_oam_cnt, (reg32))
#define e_oam_stat_GET_tx_oam_cnt(reg32)                       REG_FLD_GET(e_oam_stat_FLD_tx_oam_cnt, (reg32))

#define e_mpcp_stat_GET_mpcp_err_cnt(reg32)                    REG_FLD_GET(e_mpcp_stat_FLD_mpcp_err_cnt, (reg32))
#define e_mpcp_stat_GET_mpcp_rgst_cnt(reg32)                   REG_FLD_GET(e_mpcp_stat_FLD_mpcp_rgst_cnt, (reg32))
#define e_mpcp_stat_GET_mpcp_dscv_gate_cnt(reg32)              REG_FLD_GET(e_mpcp_stat_FLD_mpcp_dscv_gate_cnt, (reg32))

#define e_mpcp_rgst_stat_GET_mpcp_rgst_req_cnt(reg32)          REG_FLD_GET(e_mpcp_rgst_stat_FLD_mpcp_rgst_req_cnt, (reg32))
#define e_mpcp_rgst_stat_GET_mpcp_rgst_ack_cnt(reg32)          REG_FLD_GET(e_mpcp_rgst_stat_FLD_mpcp_rgst_ack_cnt, (reg32))

#define e_rxmbi_bytecnt_h_GET_rxmbi_bytecnt_h(reg32)           REG_FLD_GET(e_rxmbi_bytecnt_h_FLD_rxmbi_bytecnt_h, (reg32))

#define e_rxmbi_bytecnt_l_GET_rxmbi_bytecnt_l(reg32)           REG_FLD_GET(e_rxmbi_bytecnt_l_FLD_rxmbi_bytecnt_l, (reg32))

#define e_rxmbi_snf_cnt_GET_rxmbi_snf_cnt(reg32)               REG_FLD_GET(e_rxmbi_snf_cnt_FLD_rxmbi_snf_cnt, (reg32))

#define e_rxmpi_uc_cnt_GET_rxmpi_uceth_cnt(reg32)              REG_FLD_GET(e_rxmpi_uc_cnt_FLD_rxmpi_uceth_cnt, (reg32))

#define e_rxmpi_bc_cnt_GET_rxmpi_bceth_cnt(reg32)              REG_FLD_GET(e_rxmpi_bc_cnt_FLD_rxmpi_bceth_cnt, (reg32))

#define e_rxmpi_mc_cnt_GET_rxmpi_mceth_cnt(reg32)              REG_FLD_GET(e_rxmpi_mc_cnt_FLD_rxmpi_mceth_cnt, (reg32))

#define e_rxmpi_oam_cnt_GET_rxmpi_oam_cnt(reg32)               REG_FLD_GET(e_rxmpi_oam_cnt_FLD_rxmpi_oam_cnt, (reg32))

#define e_rxmpi_mpcp_cnt_GET_rxmpi_mpcp_cnt(reg32)             REG_FLD_GET(e_rxmpi_mpcp_cnt_FLD_rxmpi_mpcp_cnt, (reg32))

#define e_rxmpi_gate_cnt_GET_rxmpi_gate_cnt(reg32)             REG_FLD_GET(e_rxmpi_gate_cnt_FLD_rxmpi_gate_cnt, (reg32))

#define e_rxmpi_nrlgate_cnt_GET_rxmpi_nrlgate_cnt(reg32)       REG_FLD_GET(e_rxmpi_nrlgate_cnt_FLD_rxmpi_nrlgate_cnt, (reg32))

#define e_txmbi_uceth_cnt_GET_txmbi_uceth_cnt(reg32)           REG_FLD_GET(e_txmbi_uceth_cnt_FLD_txmbi_uceth_cnt, (reg32))

#define e_txmbi_mceth_cnt_GET_txmbi_bceth_cnt(reg32)           REG_FLD_GET(e_txmbi_mceth_cnt_FLD_txmbi_bceth_cnt, (reg32))
#define e_txmbi_mceth_cnt_GET_txmbi_mceth_cnt(reg32)           REG_FLD_GET(e_txmbi_mceth_cnt_FLD_txmbi_mceth_cnt, (reg32))

#define e_tx_dygasp_cnt_GET_txmbi_dygasp_cnt(reg32)            REG_FLD_GET(e_tx_dygasp_cnt_FLD_txmbi_dygasp_cnt, (reg32))
#define e_tx_dygasp_cnt_GET_txmpi_dygasp_cnt(reg32)            REG_FLD_GET(e_tx_dygasp_cnt_FLD_txmpi_dygasp_cnt, (reg32))

#define e_tx_rpt_cnt_GET_txmbi_rpt_cnt(reg32)                  REG_FLD_GET(e_tx_rpt_cnt_FLD_txmbi_rpt_cnt, (reg32))
#define e_tx_rpt_cnt_GET_txmpi_rpt_cnt(reg32)                  REG_FLD_GET(e_tx_rpt_cnt_FLD_txmpi_rpt_cnt, (reg32))

#define e_kpoam_stat_GET_txmbi_kpalv_cnt(reg32)                REG_FLD_GET(e_kpoam_stat_FLD_txmbi_kpalv_cnt, (reg32))
#define e_kpoam_stat_GET_txmpi_kpalv_cnt(reg32)                REG_FLD_GET(e_kpoam_stat_FLD_txmpi_kpalv_cnt, (reg32))

#define e_txmpi_mpcp_cnt_GET_txmpi_oam_cnt(reg32)              REG_FLD_GET(e_txmpi_mpcp_cnt_FLD_txmpi_oam_cnt, (reg32))
#define e_txmpi_mpcp_cnt_GET_txmpi_rgreq_cnt(reg32)            REG_FLD_GET(e_txmpi_mpcp_cnt_FLD_txmpi_rgreq_cnt, (reg32))
#define e_txmpi_mpcp_cnt_GET_txmpi_rgack_cnt(reg32)            REG_FLD_GET(e_txmpi_mpcp_cnt_FLD_txmpi_rgack_cnt, (reg32))

#define e_txmpi_uceth_cnt_GET_txmpi_uceth_cnt(reg32)           REG_FLD_GET(e_txmpi_uceth_cnt_FLD_txmpi_uceth_cnt, (reg32))

#define e_txmpi_mceth_cnt_GET_txmpi_bceth_cnt(reg32)           REG_FLD_GET(e_txmpi_mceth_cnt_FLD_txmpi_bceth_cnt, (reg32))
#define e_txmpi_mceth_cnt_GET_txmpi_mceth_cnt(reg32)           REG_FLD_GET(e_txmpi_mceth_cnt_FLD_txmpi_mceth_cnt, (reg32))

#define e_rxadv_cnt_GET_rx_infomis_dscvgate_cnt(reg32)         REG_FLD_GET(e_rxadv_cnt_FLD_rx_infomis_dscvgate_cnt, (reg32))
#define e_rxadv_cnt_GET_rx_llidbc_nrlgate_cnt(reg32)           REG_FLD_GET(e_rxadv_cnt_FLD_rx_llidbc_nrlgate_cnt, (reg32))
#define e_rxadv_cnt_GET_rx_filt_dscvgate_cnt(reg32)            REG_FLD_GET(e_rxadv_cnt_FLD_rx_filt_dscvgate_cnt, (reg32))

#define e_rxmpi_churn_cnt_GET_rxmpi_churn_crcok_cnt(reg32)     REG_FLD_GET(e_rxmpi_churn_cnt_FLD_rxmpi_churn_crcok_cnt, (reg32))
#define e_rxmpi_churn_cnt_GET_rxmpi_churn_crcerr_cnt(reg32)    REG_FLD_GET(e_rxmpi_churn_cnt_FLD_rxmpi_churn_crcerr_cnt, (reg32))

#define e_rxmpi_churn_info_GET_rxmpi_churn_keyidx(reg32)       REG_FLD_GET(e_rxmpi_churn_info_FLD_rxmpi_churn_keyidx, (reg32))
#define e_rxmpi_churn_info_GET_rxmpi_churn_llididx(reg32)      REG_FLD_GET(e_rxmpi_churn_info_FLD_rxmpi_churn_llididx, (reg32))
#define e_rxmpi_churn_info_GET_rxmpi_churncrc_keyidx(reg32)    REG_FLD_GET(e_rxmpi_churn_info_FLD_rxmpi_churncrc_keyidx, (reg32))
#define e_rxmpi_churn_info_GET_rxmpi_churncrc_llididx(reg32)   REG_FLD_GET(e_rxmpi_churn_info_FLD_rxmpi_churncrc_llididx, (reg32))
#define e_rxmpi_churn_info_GET_rxmpi_abchurn_cnt(reg32)        REG_FLD_GET(e_rxmpi_churn_info_FLD_rxmpi_abchurn_cnt, (reg32))

#define e_desec_ok_cnt_GET_desec_rx_ok_cnt(reg32)              REG_FLD_GET(e_desec_ok_cnt_FLD_desec_rx_ok_cnt, (reg32))
#define e_desec_ok_cnt_GET_desec_plain_ok_cnt(reg32)           REG_FLD_GET(e_desec_ok_cnt_FLD_desec_plain_ok_cnt, (reg32))

#define e_rxmpi_p_cnt_GET_rxmpi_p_ok_cnt(reg32)                REG_FLD_GET(e_rxmpi_p_cnt_FLD_rxmpi_p_ok_cnt, (reg32))
#define e_rxmpi_p_cnt_GET_rxmpi_p_err_cnt(reg32)               REG_FLD_GET(e_rxmpi_p_cnt_FLD_rxmpi_p_err_cnt, (reg32))

#define e_rxmpi_crc8err_cnt_GET_rxmpi_crc32err_cnt(reg32)      REG_FLD_GET(e_rxmpi_crc8err_cnt_FLD_rxmpi_crc32err_cnt, (reg32))
#define e_rxmpi_crc8err_cnt_GET_rxmpi_crc8err_cnt(reg32)       REG_FLD_GET(e_rxmpi_crc8err_cnt_FLD_rxmpi_crc8err_cnt, (reg32))

#define e_desec_rxdrop_cnt_GET_desec_rxfifo_ovrundrop_cnt(reg32) REG_FLD_GET(e_desec_rxdrop_cnt_FLD_desec_rxfifo_ovrundrop_cnt, (reg32))
#define e_desec_rxdrop_cnt_GET_desec_rxsof_ovrundrop_cnt(reg32) REG_FLD_GET(e_desec_rxdrop_cnt_FLD_desec_rxsof_ovrundrop_cnt, (reg32))

#define e_desec_rxerr_cnt_GET_desec_rx_crcerr_cnt(reg32)       REG_FLD_GET(e_desec_rxerr_cnt_FLD_desec_rx_crcerr_cnt, (reg32))
#define e_desec_rxerr_cnt_GET_desec_rx_preerr_cnt(reg32)       REG_FLD_GET(e_desec_rxerr_cnt_FLD_desec_rx_preerr_cnt, (reg32))

#define e_desec_ciphdrop_cnt_GET_desec_rxsof_hdrdrop_cnt(reg32) REG_FLD_GET(e_desec_ciphdrop_cnt_FLD_desec_rxsof_hdrdrop_cnt, (reg32))
#define e_desec_ciphdrop_cnt_GET_desec_ciph_csdrop_cnt(reg32)  REG_FLD_GET(e_desec_ciphdrop_cnt_FLD_desec_ciph_csdrop_cnt, (reg32))

#define e_desec_aeswdrop_cnt_GET_desec_ciph_actdrop_cnt(reg32) REG_FLD_GET(e_desec_aeswdrop_cnt_FLD_desec_ciph_actdrop_cnt, (reg32))
#define e_desec_aeswdrop_cnt_GET_desec_aesw_csdrop_cnt(reg32)  REG_FLD_GET(e_desec_aeswdrop_cnt_FLD_desec_aesw_csdrop_cnt, (reg32))

#define e_desec_aeswdrop_cnt2_GET_desec_aesw_ovrundrop_cnt(reg32) REG_FLD_GET(e_desec_aeswdrop_cnt2_FLD_desec_aesw_ovrundrop_cnt, (reg32))
#define e_desec_aeswdrop_cnt2_GET_desec_aesw_icverrdrop_cnt(reg32) REG_FLD_GET(e_desec_aeswdrop_cnt2_FLD_desec_aesw_icverrdrop_cnt, (reg32))

#define e_rxmpi_drop_cnt_GET_rxmpi_eofdrop_cnt(reg32)          REG_FLD_GET(e_rxmpi_drop_cnt_FLD_rxmpi_eofdrop_cnt, (reg32))
#define e_rxmpi_drop_cnt_GET_rxmpi_fifoovrun_cnt(reg32)        REG_FLD_GET(e_rxmpi_drop_cnt_FLD_rxmpi_fifoovrun_cnt, (reg32))

#define e_rxmbi_drop_cnt_GET_rxmbi_sofdrop_cnt(reg32)          REG_FLD_GET(e_rxmbi_drop_cnt_FLD_rxmbi_sofdrop_cnt, (reg32))
#define e_rxmbi_drop_cnt_GET_rxmbi_snfdrop_cnt(reg32)          REG_FLD_GET(e_rxmbi_drop_cnt_FLD_rxmbi_snfdrop_cnt, (reg32))

#define e_rxmbi_drop_cnt2_GET_rxmbi_crcerr_cnt(reg32)          REG_FLD_GET(e_rxmbi_drop_cnt2_FLD_rxmbi_crcerr_cnt, (reg32))
#define e_rxmbi_drop_cnt2_GET_rxmbi_enddrop_cnt(reg32)         REG_FLD_GET(e_rxmbi_drop_cnt2_FLD_rxmbi_enddrop_cnt, (reg32))

#define e_txmbi_err_cnt_GET_txmbi_err_cnt(reg32)               REG_FLD_GET(e_txmbi_err_cnt_FLD_txmbi_err_cnt, (reg32))

#define e_sts_dmy11_GET_xe_sts_dmy11(reg32)                    REG_FLD_GET(e_sts_dmy11_FLD_xe_sts_dmy11, (reg32))

#define e_sts_dmy12_GET_xe_sts_dmy12(reg32)                    REG_FLD_GET(e_sts_dmy12_FLD_xe_sts_dmy12, (reg32))

#define e_sts_dmy13_GET_xe_sts_dmy13(reg32)                    REG_FLD_GET(e_sts_dmy13_FLD_xe_sts_dmy13, (reg32))

#define e_sts_dmy14_GET_xe_sts_dmy14(reg32)                    REG_FLD_GET(e_sts_dmy14_FLD_xe_sts_dmy14, (reg32))

#define e_sts_dmy15_GET_xe_sts_dmy15(reg32)                    REG_FLD_GET(e_sts_dmy15_FLD_xe_sts_dmy15, (reg32))

#define e_sts_dmy16_GET_xe_sts_dmy16(reg32)                    REG_FLD_GET(e_sts_dmy16_FLD_xe_sts_dmy16, (reg32))

#define e_sts_dmy17_GET_xe_sts_dmy17(reg32)                    REG_FLD_GET(e_sts_dmy17_FLD_xe_sts_dmy17, (reg32))

#define e_sts_dmy18_GET_xe_sts_dmy18(reg32)                    REG_FLD_GET(e_sts_dmy18_FLD_xe_sts_dmy18, (reg32))

#define e_sts_dmy19_GET_xe_sts_dmy19(reg32)                    REG_FLD_GET(e_sts_dmy19_FLD_xe_sts_dmy19, (reg32))

#define e_sts_dmy20_GET_xe_sts_dmy20(reg32)                    REG_FLD_GET(e_sts_dmy20_FLD_xe_sts_dmy20, (reg32))

#define e_sts_dmy21_GET_xe_sts_dmy21(reg32)                    REG_FLD_GET(e_sts_dmy21_FLD_xe_sts_dmy21, (reg32))

#define e_sts_dmy22_GET_xe_sts_dmy22(reg32)                    REG_FLD_GET(e_sts_dmy22_FLD_xe_sts_dmy22, (reg32))

#define e_sts_dmy23_GET_xe_sts_dmy23(reg32)                    REG_FLD_GET(e_sts_dmy23_FLD_xe_sts_dmy23, (reg32))

#define e_sts_dmy24_GET_xe_sts_dmy24(reg32)                    REG_FLD_GET(e_sts_dmy24_FLD_xe_sts_dmy24, (reg32))

#define e_sts_dmy25_GET_xe_sts_dmy25(reg32)                    REG_FLD_GET(e_sts_dmy25_FLD_xe_sts_dmy25, (reg32))

#define e_gnt_type_stat_GET_b2b_gnt_cnt(reg32)                 REG_FLD_GET(e_gnt_type_stat_FLD_b2b_gnt_cnt, (reg32))
#define e_gnt_type_stat_GET_hdn_gnt_cnt(reg32)                 REG_FLD_GET(e_gnt_type_stat_FLD_hdn_gnt_cnt, (reg32))

#define e_gnt_pending_stat_GET_max_gnt_pending_cnt(reg32)      REG_FLD_GET(e_gnt_pending_stat_FLD_max_gnt_pending_cnt, (reg32))
#define e_gnt_pending_stat_GET_cur_gnt_pending_cnt(reg32)      REG_FLD_GET(e_gnt_pending_stat_FLD_cur_gnt_pending_cnt, (reg32))

#define e_gnt_length_stat_GET_max_gnt_length(reg32)            REG_FLD_GET(e_gnt_length_stat_FLD_max_gnt_length, (reg32))
#define e_gnt_length_stat_GET_min_gnt_length(reg32)            REG_FLD_GET(e_gnt_length_stat_FLD_min_gnt_length, (reg32))

#define e_local_time_GET_local_time(reg32)                     REG_FLD_GET(e_local_time_FLD_local_time, (reg32))

#define e_time_drft_stat_GET_cur_time_drift_ofst(reg32)        REG_FLD_GET(e_time_drft_stat_FLD_cur_time_drift_ofst, (reg32))
#define e_time_drft_stat_GET_max_time_drift_ofst(reg32)        REG_FLD_GET(e_time_drft_stat_FLD_max_time_drift_ofst, (reg32))
#define e_time_drft_stat_GET_cur_time_drift(reg32)             REG_FLD_GET(e_time_drft_stat_FLD_cur_time_drift, (reg32))
#define e_time_drft_stat_GET_max_time_drift(reg32)             REG_FLD_GET(e_time_drft_stat_FLD_max_time_drift, (reg32))

#define e_tx_timedrift_stat_GET_tx_cur_time_drift_ofst(reg32)  REG_FLD_GET(e_tx_timedrift_stat_FLD_tx_cur_time_drift_ofst, (reg32))
#define e_tx_timedrift_stat_GET_tx_max_time_drift_ofst(reg32)  REG_FLD_GET(e_tx_timedrift_stat_FLD_tx_max_time_drift_ofst, (reg32))
#define e_tx_timedrift_stat_GET_tx_cur_time_drift(reg32)       REG_FLD_GET(e_tx_timedrift_stat_FLD_tx_cur_time_drift, (reg32))
#define e_tx_timedrift_stat_GET_tx_max_time_drift(reg32)       REG_FLD_GET(e_tx_timedrift_stat_FLD_tx_max_time_drift, (reg32))

#define e_rxfifo_depth_stat_GET_cur_rxfifo_depth(reg32)        REG_FLD_GET(e_rxfifo_depth_stat_FLD_cur_rxfifo_depth, (reg32))
#define e_rxfifo_depth_stat_GET_max_rxfifo_depth(reg32)        REG_FLD_GET(e_rxfifo_depth_stat_FLD_max_rxfifo_depth, (reg32))

#define e_sts_dmy0_GET_xe_sts_dmy0(reg32)                      REG_FLD_GET(e_sts_dmy0_FLD_xe_sts_dmy0, (reg32))

#define e_sts_dmy1_GET_xe_sts_dmy1(reg32)                      REG_FLD_GET(e_sts_dmy1_FLD_xe_sts_dmy1, (reg32))

#define e_sts_dmy2_GET_xe_sts_dmy2(reg32)                      REG_FLD_GET(e_sts_dmy2_FLD_xe_sts_dmy2, (reg32))

#define e_sts_dmy3_GET_xe_sts_dmy3(reg32)                      REG_FLD_GET(e_sts_dmy3_FLD_xe_sts_dmy3, (reg32))

#define e_sts_dmy4_GET_xe_sts_dmy4(reg32)                      REG_FLD_GET(e_sts_dmy4_FLD_xe_sts_dmy4, (reg32))

#define e_sts_dmy5_GET_xe_sts_dmy5(reg32)                      REG_FLD_GET(e_sts_dmy5_FLD_xe_sts_dmy5, (reg32))

#define e_txmbi_bufwrap_wrcnt_GET_txmbi_bufwrap_wrerr_cnt(reg32) REG_FLD_GET(e_txmbi_bufwrap_wrcnt_FLD_txmbi_bufwrap_wrerr_cnt, (reg32))
#define e_txmbi_bufwrap_wrcnt_GET_txmbi_bufwrap_wr_cnt(reg32)  REG_FLD_GET(e_txmbi_bufwrap_wrcnt_FLD_txmbi_bufwrap_wr_cnt, (reg32))

#define e_txmbi_bufwrap_rdcnt_GET_txmbi_bufwrap_rderr_cnt(reg32) REG_FLD_GET(e_txmbi_bufwrap_rdcnt_FLD_txmbi_bufwrap_rderr_cnt, (reg32))
#define e_txmbi_bufwrap_rdcnt_GET_txmbi_bufwrap_rd_cnt(reg32)  REG_FLD_GET(e_txmbi_bufwrap_rdcnt_FLD_txmbi_bufwrap_rd_cnt, (reg32))

#define e_txmbi_bufwrap_rdcnt2_GET_txmbi_bufwrap_rddmy_cnt(reg32) REG_FLD_GET(e_txmbi_bufwrap_rdcnt2_FLD_txmbi_bufwrap_rddmy_cnt, (reg32))

#define e_glb_cfg_SET_txoam_favor(reg32, val)                  REG_FLD_SET(e_glb_cfg_FLD_txoam_favor, (reg32), (val))
#define e_glb_cfg_SET_burst_en_dly(reg32, val)                 REG_FLD_SET(e_glb_cfg_FLD_burst_en_dly, (reg32), (val))
#define e_glb_cfg_SET_discv_burst_en(reg32, val)               REG_FLD_SET(e_glb_cfg_FLD_discv_burst_en, (reg32), (val))
#define e_glb_cfg_SET_mpcp_fwd(reg32, val)                     REG_FLD_SET(e_glb_cfg_FLD_mpcp_fwd, (reg32), (val))
#define e_glb_cfg_SET_bcst_llid_m1_drop(reg32, val)            REG_FLD_SET(e_glb_cfg_FLD_bcst_llid_m1_drop, (reg32), (val))
#define e_glb_cfg_SET_bcst_llid_m0_drop(reg32, val)            REG_FLD_SET(e_glb_cfg_FLD_bcst_llid_m0_drop, (reg32), (val))
#define e_glb_cfg_SET_mcst_llid_drop(reg32, val)               REG_FLD_SET(e_glb_cfg_FLD_mcst_llid_drop, (reg32), (val))
#define e_glb_cfg_SET_allucst_llid_fwd(reg32, val)             REG_FLD_SET(e_glb_cfg_FLD_allucst_llid_fwd, (reg32), (val))
#define e_glb_cfg_SET_fcs_err_fwd(reg32, val)                  REG_FLD_SET(e_glb_cfg_FLD_fcs_err_fwd, (reg32), (val))
#define e_glb_cfg_SET_llid_crc8_err_fwd(reg32, val)            REG_FLD_SET(e_glb_cfg_FLD_llid_crc8_err_fwd, (reg32), (val))
#define e_glb_cfg_SET_rxmpi_stop(reg32, val)                   REG_FLD_SET(e_glb_cfg_FLD_rxmpi_stop, (reg32), (val))
#define e_glb_cfg_SET_txmpi_stop(reg32, val)                   REG_FLD_SET(e_glb_cfg_FLD_txmpi_stop, (reg32), (val))
#define e_glb_cfg_SET_phy_pwr_down(reg32, val)                 REG_FLD_SET(e_glb_cfg_FLD_phy_pwr_down, (reg32), (val))
#define e_glb_cfg_SET_rx_nml_gate_fwd(reg32, val)              REG_FLD_SET(e_glb_cfg_FLD_rx_nml_gate_fwd, (reg32), (val))
#define e_glb_cfg_SET_rxmbi_stop(reg32, val)                   REG_FLD_SET(e_glb_cfg_FLD_rxmbi_stop, (reg32), (val))
#define e_glb_cfg_SET_txmbi_stop(reg32, val)                   REG_FLD_SET(e_glb_cfg_FLD_txmbi_stop, (reg32), (val))
#define e_glb_cfg_SET_chk_all_gnt_mode(reg32, val)             REG_FLD_SET(e_glb_cfg_FLD_chk_all_gnt_mode, (reg32), (val))
#define e_glb_cfg_SET_mpcp_done_set(reg32, val)                REG_FLD_SET(e_glb_cfg_FLD_mpcp_done_set, (reg32), (val))
#define e_glb_cfg_SET_epon_oam_cal_in_eth(reg32, val)          REG_FLD_SET(e_glb_cfg_FLD_epon_oam_cal_in_eth, (reg32), (val))
#define e_glb_cfg_SET_rpt_txpri_ctrl(reg32, val)               REG_FLD_SET(e_glb_cfg_FLD_rpt_txpri_ctrl, (reg32), (val))

#define e_glb_cfg2_SET_llidnum_sel(reg32, val)                 REG_FLD_SET(e_glb_cfg2_FLD_llidnum_sel, (reg32), (val))
#define e_glb_cfg2_SET_rxdv_tmout_en(reg32, val)               REG_FLD_SET(e_glb_cfg2_FLD_rxdv_tmout_en, (reg32), (val))
#define e_glb_cfg2_SET_txmbi_nack_enden(reg32, val)            REG_FLD_SET(e_glb_cfg2_FLD_txmbi_nack_enden, (reg32), (val))
#define e_glb_cfg2_SET_loctime_mtd(reg32, val)                 REG_FLD_SET(e_glb_cfg2_FLD_loctime_mtd, (reg32), (val))
#define e_glb_cfg2_SET_rgreq_lsrtime_mask(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_rgreq_lsrtime_mask, (reg32), (val))
#define e_glb_cfg2_SET_crc_cal_in_bytecnt(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_crc_cal_in_bytecnt, (reg32), (val))
#define e_glb_cfg2_SET_u10g_txmode(reg32, val)                 REG_FLD_SET(e_glb_cfg2_FLD_u10g_txmode, (reg32), (val))
#define e_glb_cfg2_SET_eth_cal_in_bytecnt(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_eth_cal_in_bytecnt, (reg32), (val))
#define e_glb_cfg2_SET_snf_cal_in_bytecnt(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_snf_cal_in_bytecnt, (reg32), (val))
#define e_glb_cfg2_SET_oam_cal_in_bytecnt(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_oam_cal_in_bytecnt, (reg32), (val))
#define e_glb_cfg2_SET_dscvgate_filt(reg32, val)               REG_FLD_SET(e_glb_cfg2_FLD_dscvgate_filt, (reg32), (val))
#define e_glb_cfg2_SET_gntlen_stat_widscv(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_gntlen_stat_widscv, (reg32), (val))
#define e_glb_cfg2_SET_tdrift_loctupd_dis(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_tdrift_loctupd_dis, (reg32), (val))
#define e_glb_cfg2_SET_dscvgate_infochk_dis(reg32, val)        REG_FLD_SET(e_glb_cfg2_FLD_dscvgate_infochk_dis, (reg32), (val))

#define e_glb_sts1_SET_xepon_type(reg32, val)                  REG_FLD_SET(e_glb_sts1_FLD_xepon_type, (reg32), (val))
#define e_glb_sts1_SET_rxmpi_stop_sts(reg32, val)              REG_FLD_SET(e_glb_sts1_FLD_rxmpi_stop_sts, (reg32), (val))
#define e_glb_sts1_SET_txmpi_stop_sts(reg32, val)              REG_FLD_SET(e_glb_sts1_FLD_txmpi_stop_sts, (reg32), (val))
#define e_glb_sts1_SET_rxmbi_stop_sts(reg32, val)              REG_FLD_SET(e_glb_sts1_FLD_rxmbi_stop_sts, (reg32), (val))
#define e_glb_sts1_SET_txmbi_stop_sts(reg32, val)              REG_FLD_SET(e_glb_sts1_FLD_txmbi_stop_sts, (reg32), (val))
#define e_glb_sts1_SET_xepon_glb_cfg_sts(reg32, val)           REG_FLD_SET(e_glb_sts1_FLD_xepon_glb_cfg_sts, (reg32), (val))

#define e_glb_sts2_SET_xepon_glb_cfg2_sts(reg32, val)          REG_FLD_SET(e_glb_sts2_FLD_xepon_glb_cfg2_sts, (reg32), (val))

#define e_int_status_SET_us_keychg_int(reg32, val)             REG_FLD_SET(e_int_status_FLD_us_keychg_int, (reg32), (val))
#define e_int_status_SET_ds_keymis_int(reg32, val)             REG_FLD_SET(e_int_status_FLD_ds_keymis_int, (reg32), (val))
#define e_int_status_SET_ds_keychg_int(reg32, val)             REG_FLD_SET(e_int_status_FLD_ds_keychg_int, (reg32), (val))
#define e_int_status_SET_sniff_fifo_ovrun_int(reg32, val)      REG_FLD_SET(e_int_status_FLD_sniff_fifo_ovrun_int, (reg32), (val))
#define e_int_status_SET_reg_ack_done_int(reg32, val)          REG_FLD_SET(e_int_status_FLD_reg_ack_done_int, (reg32), (val))
#define e_int_status_SET_reg_req_done_int(reg32, val)          REG_FLD_SET(e_int_status_FLD_reg_req_done_int, (reg32), (val))
#define e_int_status_SET_reorder1_gnt_int(reg32, val)          REG_FLD_SET(e_int_status_FLD_reorder1_gnt_int, (reg32), (val))
#define e_int_status_SET_b2b_gnt_int(reg32, val)               REG_FLD_SET(e_int_status_FLD_b2b_gnt_int, (reg32), (val))
#define e_int_status_SET_hidn_gnt_int(reg32, val)              REG_FLD_SET(e_int_status_FLD_hidn_gnt_int, (reg32), (val))
#define e_int_status_SET_ps_early_wakeup_int(reg32, val)       REG_FLD_SET(e_int_status_FLD_ps_early_wakeup_int, (reg32), (val))
#define e_int_status_SET_rx_sleep_allow_int(reg32, val)        REG_FLD_SET(e_int_status_FLD_rx_sleep_allow_int, (reg32), (val))
#define e_int_status_SET_ps_wakeup_int(reg32, val)             REG_FLD_SET(e_int_status_FLD_ps_wakeup_int, (reg32), (val))
#define e_int_status_SET_ps_sleep_int(reg32, val)              REG_FLD_SET(e_int_status_FLD_ps_sleep_int, (reg32), (val))
#define e_int_status_SET_txfifo_udrun_int(reg32, val)          REG_FLD_SET(e_int_status_FLD_txfifo_udrun_int, (reg32), (val))
#define e_int_status_SET_rpt_overintvl_int(reg32, val)         REG_FLD_SET(e_int_status_FLD_rpt_overintvl_int, (reg32), (val))
#define e_int_status_SET_mpcp_timeout_int(reg32, val)          REG_FLD_SET(e_int_status_FLD_mpcp_timeout_int, (reg32), (val))
#define e_int_status_SET_timedrft_int(reg32, val)              REG_FLD_SET(e_int_status_FLD_timedrft_int, (reg32), (val))
#define e_int_status_SET_tod_1pps_int(reg32, val)              REG_FLD_SET(e_int_status_FLD_tod_1pps_int, (reg32), (val))
#define e_int_status_SET_tod_updt_int(reg32, val)              REG_FLD_SET(e_int_status_FLD_tod_updt_int, (reg32), (val))
#define e_int_status_SET_ptp_msg_tx_int(reg32, val)            REG_FLD_SET(e_int_status_FLD_ptp_msg_tx_int, (reg32), (val))
#define e_int_status_SET_gnt_buf_ovrrun_int(reg32, val)        REG_FLD_SET(e_int_status_FLD_gnt_buf_ovrrun_int, (reg32), (val))
#define e_int_status_SET_llid7_rcv_rgst_int(reg32, val)        REG_FLD_SET(e_int_status_FLD_llid7_rcv_rgst_int, (reg32), (val))
#define e_int_status_SET_llid6_rcv_rgst_int(reg32, val)        REG_FLD_SET(e_int_status_FLD_llid6_rcv_rgst_int, (reg32), (val))
#define e_int_status_SET_llid5_rcv_rgst_int(reg32, val)        REG_FLD_SET(e_int_status_FLD_llid5_rcv_rgst_int, (reg32), (val))
#define e_int_status_SET_llid4_rcv_rgst_int(reg32, val)        REG_FLD_SET(e_int_status_FLD_llid4_rcv_rgst_int, (reg32), (val))
#define e_int_status_SET_llid3_rcv_rgst_int(reg32, val)        REG_FLD_SET(e_int_status_FLD_llid3_rcv_rgst_int, (reg32), (val))
#define e_int_status_SET_llid2_rcv_rgst_int(reg32, val)        REG_FLD_SET(e_int_status_FLD_llid2_rcv_rgst_int, (reg32), (val))
#define e_int_status_SET_llid1_rcv_rgst_int(reg32, val)        REG_FLD_SET(e_int_status_FLD_llid1_rcv_rgst_int, (reg32), (val))
#define e_int_status_SET_llid0_rcv_rgst_int(reg32, val)        REG_FLD_SET(e_int_status_FLD_llid0_rcv_rgst_int, (reg32), (val))
#define e_int_status_SET_rcv_dscvry_gate_int(reg32, val)       REG_FLD_SET(e_int_status_FLD_rcv_dscvry_gate_int, (reg32), (val))

#define e_int_en_SET_us_keychg_en(reg32, val)                  REG_FLD_SET(e_int_en_FLD_us_keychg_en, (reg32), (val))
#define e_int_en_SET_ds_keymis_en(reg32, val)                  REG_FLD_SET(e_int_en_FLD_ds_keymis_en, (reg32), (val))
#define e_int_en_SET_ds_keychg_en(reg32, val)                  REG_FLD_SET(e_int_en_FLD_ds_keychg_en, (reg32), (val))
#define e_int_en_SET_sniff_fifo_ovrun_en(reg32, val)           REG_FLD_SET(e_int_en_FLD_sniff_fifo_ovrun_en, (reg32), (val))
#define e_int_en_SET_reg_ack_done_en(reg32, val)               REG_FLD_SET(e_int_en_FLD_reg_ack_done_en, (reg32), (val))
#define e_int_en_SET_reg_req_done_en(reg32, val)               REG_FLD_SET(e_int_en_FLD_reg_req_done_en, (reg32), (val))
#define e_int_en_SET_reorder1_gnt_en(reg32, val)               REG_FLD_SET(e_int_en_FLD_reorder1_gnt_en, (reg32), (val))
#define e_int_en_SET_b2b_gnt_en(reg32, val)                    REG_FLD_SET(e_int_en_FLD_b2b_gnt_en, (reg32), (val))
#define e_int_en_SET_hidn_gnt_en(reg32, val)                   REG_FLD_SET(e_int_en_FLD_hidn_gnt_en, (reg32), (val))
#define e_int_en_SET_ps_early_wakeup_en(reg32, val)            REG_FLD_SET(e_int_en_FLD_ps_early_wakeup_en, (reg32), (val))
#define e_int_en_SET_rx_sleep_allow_en(reg32, val)             REG_FLD_SET(e_int_en_FLD_rx_sleep_allow_en, (reg32), (val))
#define e_int_en_SET_ps_wakeup_en(reg32, val)                  REG_FLD_SET(e_int_en_FLD_ps_wakeup_en, (reg32), (val))
#define e_int_en_SET_ps_sleep_en(reg32, val)                   REG_FLD_SET(e_int_en_FLD_ps_sleep_en, (reg32), (val))
#define e_int_en_SET_txfifo_udrun_en(reg32, val)               REG_FLD_SET(e_int_en_FLD_txfifo_udrun_en, (reg32), (val))
#define e_int_en_SET_rpt_overintvl_en(reg32, val)              REG_FLD_SET(e_int_en_FLD_rpt_overintvl_en, (reg32), (val))
#define e_int_en_SET_mpcp_timeout_en(reg32, val)               REG_FLD_SET(e_int_en_FLD_mpcp_timeout_en, (reg32), (val))
#define e_int_en_SET_timedrft_en(reg32, val)                   REG_FLD_SET(e_int_en_FLD_timedrft_en, (reg32), (val))
#define e_int_en_SET_tod_1pps_en(reg32, val)                   REG_FLD_SET(e_int_en_FLD_tod_1pps_en, (reg32), (val))
#define e_int_en_SET_tod_updt_en(reg32, val)                   REG_FLD_SET(e_int_en_FLD_tod_updt_en, (reg32), (val))
#define e_int_en_SET_ptp_msg_tx_en(reg32, val)                 REG_FLD_SET(e_int_en_FLD_ptp_msg_tx_en, (reg32), (val))
#define e_int_en_SET_gnt_buf_ovrrun_en(reg32, val)             REG_FLD_SET(e_int_en_FLD_gnt_buf_ovrrun_en, (reg32), (val))
#define e_int_en_SET_llid7_rcv_rgst_en(reg32, val)             REG_FLD_SET(e_int_en_FLD_llid7_rcv_rgst_en, (reg32), (val))
#define e_int_en_SET_llid6_rcv_rgst_en(reg32, val)             REG_FLD_SET(e_int_en_FLD_llid6_rcv_rgst_en, (reg32), (val))
#define e_int_en_SET_llid5_rcv_rgst_en(reg32, val)             REG_FLD_SET(e_int_en_FLD_llid5_rcv_rgst_en, (reg32), (val))
#define e_int_en_SET_llid4_rcv_rgst_en(reg32, val)             REG_FLD_SET(e_int_en_FLD_llid4_rcv_rgst_en, (reg32), (val))
#define e_int_en_SET_llid3_rcv_rgst_en(reg32, val)             REG_FLD_SET(e_int_en_FLD_llid3_rcv_rgst_en, (reg32), (val))
#define e_int_en_SET_llid2_rcv_rgst_en(reg32, val)             REG_FLD_SET(e_int_en_FLD_llid2_rcv_rgst_en, (reg32), (val))
#define e_int_en_SET_llid1_rcv_rgst_en(reg32, val)             REG_FLD_SET(e_int_en_FLD_llid1_rcv_rgst_en, (reg32), (val))
#define e_int_en_SET_llid0_rcv_rgst_en(reg32, val)             REG_FLD_SET(e_int_en_FLD_llid0_rcv_rgst_en, (reg32), (val))
#define e_int_en_SET_rcv_dscvry_gate_en(reg32, val)            REG_FLD_SET(e_int_en_FLD_rcv_dscvry_gate_en, (reg32), (val))

#define e_int_sts2_SET_tx_latestart_int(reg32, val)            REG_FLD_SET(e_int_sts2_FLD_tx_latestart_int, (reg32), (val))
#define e_int_sts2_SET_rcv_ifchk_err_int(reg32, val)           REG_FLD_SET(e_int_sts2_FLD_rcv_ifchk_err_int, (reg32), (val))
#define e_int_sts2_SET_rx_rgst_exc_int(reg32, val)             REG_FLD_SET(e_int_sts2_FLD_rx_rgst_exc_int, (reg32), (val))
#define e_int_sts2_SET_rcv_len_err_int(reg32, val)             REG_FLD_SET(e_int_sts2_FLD_rcv_len_err_int, (reg32), (val))
#define e_int_sts2_SET_schfch_nopkt_int(reg32, val)            REG_FLD_SET(e_int_sts2_FLD_schfch_nopkt_int, (reg32), (val))
#define e_int_sts2_SET_rxfifo_ovrun_int(reg32, val)            REG_FLD_SET(e_int_sts2_FLD_rxfifo_ovrun_int, (reg32), (val))
#define e_int_sts2_SET_txmpi_pldenneq_int(reg32, val)          REG_FLD_SET(e_int_sts2_FLD_txmpi_pldenneq_int, (reg32), (val))
#define e_int_sts2_SET_schfch_timeo_int(reg32, val)            REG_FLD_SET(e_int_sts2_FLD_schfch_timeo_int, (reg32), (val))
#define e_int_sts2_SET_schgnt_inv_int(reg32, val)              REG_FLD_SET(e_int_sts2_FLD_schgnt_inv_int, (reg32), (val))
#define e_int_sts2_SET_rcvgnt_infofail_int(reg32, val)         REG_FLD_SET(e_int_sts2_FLD_rcvgnt_infofail_int, (reg32), (val))
#define e_int_sts2_SET_rcv_eofdrop_int(reg32, val)             REG_FLD_SET(e_int_sts2_FLD_rcv_eofdrop_int, (reg32), (val))
#define e_int_sts2_SET_rcv_crcerr_int(reg32, val)              REG_FLD_SET(e_int_sts2_FLD_rcv_crcerr_int, (reg32), (val))
#define e_int_sts2_SET_rcv_excgate_int(reg32, val)             REG_FLD_SET(e_int_sts2_FLD_rcv_excgate_int, (reg32), (val))

#define e_int_en2_SET_tx_latestart_en(reg32, val)              REG_FLD_SET(e_int_en2_FLD_tx_latestart_en, (reg32), (val))
#define e_int_en2_SET_rcv_ifchk_err_en(reg32, val)             REG_FLD_SET(e_int_en2_FLD_rcv_ifchk_err_en, (reg32), (val))
#define e_int_en2_SET_rx_rgst_exc_en(reg32, val)               REG_FLD_SET(e_int_en2_FLD_rx_rgst_exc_en, (reg32), (val))
#define e_int_en2_SET_rcv_len_err_en(reg32, val)               REG_FLD_SET(e_int_en2_FLD_rcv_len_err_en, (reg32), (val))
#define e_int_en2_SET_schfch_nopkt_en(reg32, val)              REG_FLD_SET(e_int_en2_FLD_schfch_nopkt_en, (reg32), (val))
#define e_int_en2_SET_rxfifo_ovrun_en(reg32, val)              REG_FLD_SET(e_int_en2_FLD_rxfifo_ovrun_en, (reg32), (val))
#define e_int_en2_SET_txmpi_pldenneq_en(reg32, val)            REG_FLD_SET(e_int_en2_FLD_txmpi_pldenneq_en, (reg32), (val))
#define e_int_en2_SET_schfch_timeo_en(reg32, val)              REG_FLD_SET(e_int_en2_FLD_schfch_timeo_en, (reg32), (val))
#define e_int_en2_SET_schgnt_inv_en(reg32, val)                REG_FLD_SET(e_int_en2_FLD_schgnt_inv_en, (reg32), (val))
#define e_int_en2_SET_rcvgnt_infofail_en(reg32, val)           REG_FLD_SET(e_int_en2_FLD_rcvgnt_infofail_en, (reg32), (val))
#define e_int_en2_SET_rcv_eofdrop_en(reg32, val)               REG_FLD_SET(e_int_en2_FLD_rcv_eofdrop_en, (reg32), (val))
#define e_int_en2_SET_rcv_crcerr_en(reg32, val)                REG_FLD_SET(e_int_en2_FLD_rcv_crcerr_en, (reg32), (val))
#define e_int_en2_SET_rcv_excgate_en(reg32, val)               REG_FLD_SET(e_int_en2_FLD_rcv_excgate_en, (reg32), (val))

#define e_int_sts3_SET_llid31_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid31_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid30_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid30_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid29_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid29_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid28_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid28_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid27_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid27_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid26_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid26_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid25_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid25_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid24_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid24_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid23_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid23_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid22_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid22_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid21_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid21_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid20_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid20_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid19_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid19_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid18_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid18_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid17_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid17_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid16_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid16_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid15_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid15_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid14_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid14_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid13_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid13_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid12_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid12_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid11_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid11_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid10_rcv_rgst_int(reg32, val)         REG_FLD_SET(e_int_sts3_FLD_llid10_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid9_rcv_rgst_int(reg32, val)          REG_FLD_SET(e_int_sts3_FLD_llid9_rcv_rgst_int, (reg32), (val))
#define e_int_sts3_SET_llid8_rcv_rgst_int(reg32, val)          REG_FLD_SET(e_int_sts3_FLD_llid8_rcv_rgst_int, (reg32), (val))

#define e_int_en3_SET_llid31_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid31_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid30_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid30_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid29_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid29_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid28_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid28_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid27_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid27_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid26_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid26_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid25_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid25_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid24_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid24_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid23_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid23_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid22_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid22_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid21_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid21_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid20_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid20_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid19_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid19_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid18_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid18_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid17_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid17_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid16_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid16_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid15_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid15_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid14_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid14_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid13_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid13_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid12_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid12_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid11_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid11_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid10_rcv_rgst_en(reg32, val)           REG_FLD_SET(e_int_en3_FLD_llid10_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid9_rcv_rgst_en(reg32, val)            REG_FLD_SET(e_int_en3_FLD_llid9_rcv_rgst_en, (reg32), (val))
#define e_int_en3_SET_llid8_rcv_rgst_en(reg32, val)            REG_FLD_SET(e_int_en3_FLD_llid8_rcv_rgst_en, (reg32), (val))

#define e_exc_sts_SET_encfrm_alierr_hit(reg32, val)            REG_FLD_SET(e_exc_sts_FLD_encfrm_alierr_hit, (reg32), (val))
#define e_exc_sts_SET_schgnt_tmout_hit(reg32, val)             REG_FLD_SET(e_exc_sts_FLD_schgnt_tmout_hit, (reg32), (val))
#define e_exc_sts_SET_rx_rgst_dergst_hit(reg32, val)           REG_FLD_SET(e_exc_sts_FLD_rx_rgst_dergst_hit, (reg32), (val))
#define e_exc_sts_SET_rx_rgst_nack_hit(reg32, val)             REG_FLD_SET(e_exc_sts_FLD_rx_rgst_nack_hit, (reg32), (val))
#define e_exc_sts_SET_rx_slderr_hit(reg32, val)                REG_FLD_SET(e_exc_sts_FLD_rx_slderr_hit, (reg32), (val))
#define e_exc_sts_SET_rx_nodv_tmo_hit(reg32, val)              REG_FLD_SET(e_exc_sts_FLD_rx_nodv_tmo_hit, (reg32), (val))
#define e_exc_sts_SET_rx_noeof_hit(reg32, val)                 REG_FLD_SET(e_exc_sts_FLD_rx_noeof_hit, (reg32), (val))
#define e_exc_sts_SET_snf_pktcnt_err_hit(reg32, val)           REG_FLD_SET(e_exc_sts_FLD_snf_pktcnt_err_hit, (reg32), (val))
#define e_exc_sts_SET_tx_gntst_nosog_hit(reg32, val)           REG_FLD_SET(e_exc_sts_FLD_tx_gntst_nosog_hit, (reg32), (val))
#define e_exc_sts_SET_tx_gntst_fifound_hit(reg32, val)         REG_FLD_SET(e_exc_sts_FLD_tx_gntst_fifound_hit, (reg32), (val))
#define e_exc_sts_SET_tx_gntph_fifound_hit(reg32, val)         REG_FLD_SET(e_exc_sts_FLD_tx_gntph_fifound_hit, (reg32), (val))
#define e_exc_sts_SET_tx_gntph_exceed_hit(reg32, val)          REG_FLD_SET(e_exc_sts_FLD_tx_gntph_exceed_hit, (reg32), (val))
#define e_exc_sts_SET_schgnt_stinv_hit(reg32, val)             REG_FLD_SET(e_exc_sts_FLD_schgnt_stinv_hit, (reg32), (val))
#define e_exc_sts_SET_schgnt_idxinv_hit(reg32, val)            REG_FLD_SET(e_exc_sts_FLD_schgnt_idxinv_hit, (reg32), (val))
#define e_exc_sts_SET_decrpt_sofeof_col_hit(reg32, val)        REG_FLD_SET(e_exc_sts_FLD_decrpt_sofeof_col_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_len_long_hit(reg32, val)             REG_FLD_SET(e_exc_sts_FLD_rcv_len_long_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_len_short_hit(reg32, val)            REG_FLD_SET(e_exc_sts_FLD_rcv_len_short_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_mpcp_crcerr_hit(reg32, val)          REG_FLD_SET(e_exc_sts_FLD_rcv_mpcp_crcerr_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_crc32err_hit(reg32, val)             REG_FLD_SET(e_exc_sts_FLD_rcv_crc32err_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_crc8err_hit(reg32, val)              REG_FLD_SET(e_exc_sts_FLD_rcv_crc8err_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_demacs_flagexc_hit(reg32, val)       REG_FLD_SET(e_exc_sts_FLD_rcv_demacs_flagexc_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_nrlgate_llidbc_hit(reg32, val)       REG_FLD_SET(e_exc_sts_FLD_rcv_nrlgate_llidbc_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_dscvgate_filt_hit(reg32, val)        REG_FLD_SET(e_exc_sts_FLD_rcv_dscvgate_filt_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_dscvgate_infomis_hit(reg32, val)     REG_FLD_SET(e_exc_sts_FLD_rcv_dscvgate_infomis_hit, (reg32), (val))

#define e_rpt_timout_SET_llid31_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid31_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid30_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid30_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid29_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid29_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid28_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid28_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid27_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid27_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid26_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid26_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid25_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid25_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid24_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid24_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid23_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid23_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid22_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid22_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid21_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid21_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid20_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid20_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid19_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid19_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid18_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid18_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid17_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid17_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid16_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid16_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid15_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid15_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid14_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid14_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid13_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid13_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid12_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid12_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid11_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid11_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid10_rpt_tmo(reg32, val)            REG_FLD_SET(e_rpt_timout_FLD_llid10_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid9_rpt_tmo(reg32, val)             REG_FLD_SET(e_rpt_timout_FLD_llid9_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid8_rpt_tmo(reg32, val)             REG_FLD_SET(e_rpt_timout_FLD_llid8_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid7_rpt_tmo(reg32, val)             REG_FLD_SET(e_rpt_timout_FLD_llid7_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid6_rpt_tmo(reg32, val)             REG_FLD_SET(e_rpt_timout_FLD_llid6_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid5_rpt_tmo(reg32, val)             REG_FLD_SET(e_rpt_timout_FLD_llid5_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid4_rpt_tmo(reg32, val)             REG_FLD_SET(e_rpt_timout_FLD_llid4_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid3_rpt_tmo(reg32, val)             REG_FLD_SET(e_rpt_timout_FLD_llid3_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid2_rpt_tmo(reg32, val)             REG_FLD_SET(e_rpt_timout_FLD_llid2_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid1_rpt_tmo(reg32, val)             REG_FLD_SET(e_rpt_timout_FLD_llid1_rpt_tmo, (reg32), (val))
#define e_rpt_timout_SET_llid0_rpt_tmo(reg32, val)             REG_FLD_SET(e_rpt_timout_FLD_llid0_rpt_tmo, (reg32), (val))

#define e_mpcp_timout_SET_llid31_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid31_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid30_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid30_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid29_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid29_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid28_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid28_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid27_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid27_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid26_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid26_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid25_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid25_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid24_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid24_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid23_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid23_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid22_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid22_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid21_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid21_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid20_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid20_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid19_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid19_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid18_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid18_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid17_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid17_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid16_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid16_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid15_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid15_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid14_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid14_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid13_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid13_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid12_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid12_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid11_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid11_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid10_mpcp_tmo(reg32, val)          REG_FLD_SET(e_mpcp_timout_FLD_llid10_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid9_mpcp_tmo(reg32, val)           REG_FLD_SET(e_mpcp_timout_FLD_llid9_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid8_mpcp_tmo(reg32, val)           REG_FLD_SET(e_mpcp_timout_FLD_llid8_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid7_mpcp_tmo(reg32, val)           REG_FLD_SET(e_mpcp_timout_FLD_llid7_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid6_mpcp_tmo(reg32, val)           REG_FLD_SET(e_mpcp_timout_FLD_llid6_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid5_mpcp_tmo(reg32, val)           REG_FLD_SET(e_mpcp_timout_FLD_llid5_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid4_mpcp_tmo(reg32, val)           REG_FLD_SET(e_mpcp_timout_FLD_llid4_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid3_mpcp_tmo(reg32, val)           REG_FLD_SET(e_mpcp_timout_FLD_llid3_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid2_mpcp_tmo(reg32, val)           REG_FLD_SET(e_mpcp_timout_FLD_llid2_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid1_mpcp_tmo(reg32, val)           REG_FLD_SET(e_mpcp_timout_FLD_llid1_mpcp_tmo, (reg32), (val))
#define e_mpcp_timout_SET_llid0_mpcp_tmo(reg32, val)           REG_FLD_SET(e_mpcp_timout_FLD_llid0_mpcp_tmo, (reg32), (val))

#define e_ds_keychg_SET_llid31_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid31_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid30_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid30_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid29_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid29_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid28_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid28_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid27_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid27_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid26_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid26_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid25_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid25_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid24_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid24_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid23_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid23_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid22_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid22_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid21_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid21_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid20_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid20_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid19_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid19_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid18_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid18_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid17_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid17_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid16_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid16_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid15_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid15_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid14_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid14_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid13_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid13_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid12_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid12_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid11_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid11_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid10_ds_keychg(reg32, val)           REG_FLD_SET(e_ds_keychg_FLD_llid10_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid9_ds_keychg(reg32, val)            REG_FLD_SET(e_ds_keychg_FLD_llid9_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid8_ds_keychg(reg32, val)            REG_FLD_SET(e_ds_keychg_FLD_llid8_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid7_ds_keychg(reg32, val)            REG_FLD_SET(e_ds_keychg_FLD_llid7_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid6_ds_keychg(reg32, val)            REG_FLD_SET(e_ds_keychg_FLD_llid6_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid5_ds_keychg(reg32, val)            REG_FLD_SET(e_ds_keychg_FLD_llid5_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid4_ds_keychg(reg32, val)            REG_FLD_SET(e_ds_keychg_FLD_llid4_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid3_ds_keychg(reg32, val)            REG_FLD_SET(e_ds_keychg_FLD_llid3_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid2_ds_keychg(reg32, val)            REG_FLD_SET(e_ds_keychg_FLD_llid2_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid1_ds_keychg(reg32, val)            REG_FLD_SET(e_ds_keychg_FLD_llid1_ds_keychg, (reg32), (val))
#define e_ds_keychg_SET_llid0_ds_keychg(reg32, val)            REG_FLD_SET(e_ds_keychg_FLD_llid0_ds_keychg, (reg32), (val))

#define e_ds_keychg2_SET_llid51_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid51_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid50_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid50_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid49_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid49_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid48_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid48_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid47_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid47_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid46_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid46_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid45_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid45_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid44_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid44_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid43_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid43_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid42_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid42_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid41_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid41_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid40_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid40_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid39_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid39_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid38_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid38_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid37_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid37_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid36_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid36_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid35_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid35_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid34_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid34_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid33_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid33_ds_keychg, (reg32), (val))
#define e_ds_keychg2_SET_llid32_ds_keychg(reg32, val)          REG_FLD_SET(e_ds_keychg2_FLD_llid32_ds_keychg, (reg32), (val))

#define e_ds_keymis_SET_llid31_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid31_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid30_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid30_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid29_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid29_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid28_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid28_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid27_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid27_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid26_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid26_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid25_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid25_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid24_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid24_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid23_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid23_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid22_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid22_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid21_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid21_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid20_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid20_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid19_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid19_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid18_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid18_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid17_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid17_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid16_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid16_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid15_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid15_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid14_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid14_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid13_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid13_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid12_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid12_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid11_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid11_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid10_ds_keymis(reg32, val)           REG_FLD_SET(e_ds_keymis_FLD_llid10_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid9_ds_keymis(reg32, val)            REG_FLD_SET(e_ds_keymis_FLD_llid9_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid8_ds_keymis(reg32, val)            REG_FLD_SET(e_ds_keymis_FLD_llid8_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid7_ds_keymis(reg32, val)            REG_FLD_SET(e_ds_keymis_FLD_llid7_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid6_ds_keymis(reg32, val)            REG_FLD_SET(e_ds_keymis_FLD_llid6_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid5_ds_keymis(reg32, val)            REG_FLD_SET(e_ds_keymis_FLD_llid5_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid4_ds_keymis(reg32, val)            REG_FLD_SET(e_ds_keymis_FLD_llid4_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid3_ds_keymis(reg32, val)            REG_FLD_SET(e_ds_keymis_FLD_llid3_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid2_ds_keymis(reg32, val)            REG_FLD_SET(e_ds_keymis_FLD_llid2_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid1_ds_keymis(reg32, val)            REG_FLD_SET(e_ds_keymis_FLD_llid1_ds_keymis, (reg32), (val))
#define e_ds_keymis_SET_llid0_ds_keymis(reg32, val)            REG_FLD_SET(e_ds_keymis_FLD_llid0_ds_keymis, (reg32), (val))

#define e_ds_keymis2_SET_llid51_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid51_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid50_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid50_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid49_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid49_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid48_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid48_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid47_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid47_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid46_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid46_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid45_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid45_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid44_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid44_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid43_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid43_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid42_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid42_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid41_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid41_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid40_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid40_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid39_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid39_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid38_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid38_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid37_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid37_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid36_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid36_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid35_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid35_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid34_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid34_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid33_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid33_ds_keymis, (reg32), (val))
#define e_ds_keymis2_SET_llid32_ds_keymis(reg32, val)          REG_FLD_SET(e_ds_keymis2_FLD_llid32_ds_keymis, (reg32), (val))

#define e_us_keychg_SET_llid31_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid31_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid30_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid30_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid29_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid29_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid28_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid28_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid27_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid27_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid26_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid26_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid25_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid25_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid24_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid24_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid23_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid23_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid22_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid22_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid21_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid21_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid20_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid20_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid19_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid19_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid18_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid18_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid17_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid17_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid16_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid16_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid15_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid15_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid14_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid14_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid13_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid13_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid12_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid12_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid11_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid11_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid10_us_keychg(reg32, val)           REG_FLD_SET(e_us_keychg_FLD_llid10_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid9_us_keychg(reg32, val)            REG_FLD_SET(e_us_keychg_FLD_llid9_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid8_us_keychg(reg32, val)            REG_FLD_SET(e_us_keychg_FLD_llid8_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid7_us_keychg(reg32, val)            REG_FLD_SET(e_us_keychg_FLD_llid7_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid6_us_keychg(reg32, val)            REG_FLD_SET(e_us_keychg_FLD_llid6_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid5_us_keychg(reg32, val)            REG_FLD_SET(e_us_keychg_FLD_llid5_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid4_us_keychg(reg32, val)            REG_FLD_SET(e_us_keychg_FLD_llid4_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid3_us_keychg(reg32, val)            REG_FLD_SET(e_us_keychg_FLD_llid3_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid2_us_keychg(reg32, val)            REG_FLD_SET(e_us_keychg_FLD_llid2_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid1_us_keychg(reg32, val)            REG_FLD_SET(e_us_keychg_FLD_llid1_us_keychg, (reg32), (val))
#define e_us_keychg_SET_llid0_us_keychg(reg32, val)            REG_FLD_SET(e_us_keychg_FLD_llid0_us_keychg, (reg32), (val))

#define e_cfg_dmy0_SET_xe_cfg_dmy0(reg32, val)                 REG_FLD_SET(e_cfg_dmy0_FLD_xe_cfg_dmy0, (reg32), (val))

#define e_cfg_dmy1_SET_xe_cfg_dmy1(reg32, val)                 REG_FLD_SET(e_cfg_dmy1_FLD_xe_cfg_dmy1, (reg32), (val))

#define e_llid0_3_cfg_SET_llid3_dmy(reg32, val)                REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_dmy, (reg32), (val))
#define e_llid0_3_cfg_SET_llid3_encrypt_key(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_encrypt_key, (reg32), (val))
#define e_llid0_3_cfg_SET_llid3_encrypt_en(reg32, val)         REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_encrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid3_txfec_en(reg32, val)           REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_txfec_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid3_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_dcrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid3_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_dcrypt_mode, (reg32), (val))
#define e_llid0_3_cfg_SET_llid3_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_oam_lpbk_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_dmy(reg32, val)                REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_dmy, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_encrypt_key(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_encrypt_key, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_encrypt_en(reg32, val)         REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_encrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_txfec_en(reg32, val)           REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_txfec_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_dcrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_dcrypt_mode, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_oam_lpbk_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_dmy(reg32, val)                REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_dmy, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_encrypt_key(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_encrypt_key, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_encrypt_en(reg32, val)         REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_encrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_txfec_en(reg32, val)           REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_txfec_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_dcrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_dcrypt_mode, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_oam_lpbk_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_dmy(reg32, val)                REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_dmy, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_encrypt_key(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_encrypt_key, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_encrypt_en(reg32, val)         REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_encrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_txfec_en(reg32, val)           REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_txfec_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_dcrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_dcrypt_mode, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_oam_lpbk_en, (reg32), (val))

#define e_llid4_7_cfg_SET_llid7_dmy(reg32, val)                REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_dmy, (reg32), (val))
#define e_llid4_7_cfg_SET_llid7_encrypt_key(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_encrypt_key, (reg32), (val))
#define e_llid4_7_cfg_SET_llid7_encrypt_en(reg32, val)         REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_encrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid7_txfec_en(reg32, val)           REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_txfec_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid7_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_dcrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid7_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_dcrypt_mode, (reg32), (val))
#define e_llid4_7_cfg_SET_llid7_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_oam_lpbk_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_dmy(reg32, val)                REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_dmy, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_encrypt_key(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_encrypt_key, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_encrypt_en(reg32, val)         REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_encrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_txfec_en(reg32, val)           REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_txfec_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_dcrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_dcrypt_mode, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_oam_lpbk_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_dmy(reg32, val)                REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_dmy, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_encrypt_key(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_encrypt_key, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_encrypt_en(reg32, val)         REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_encrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_txfec_en(reg32, val)           REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_txfec_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_dcrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_dcrypt_mode, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_oam_lpbk_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_dmy(reg32, val)                REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_dmy, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_encrypt_key(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_encrypt_key, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_encrypt_en(reg32, val)         REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_encrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_txfec_en(reg32, val)           REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_txfec_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_dcrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_dcrypt_mode, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_oam_lpbk_en, (reg32), (val))

#define e_llid8_11_cfg_SET_llid11_dmy(reg32, val)              REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_dmy, (reg32), (val))
#define e_llid8_11_cfg_SET_llid11_encrypt_key(reg32, val)      REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_encrypt_key, (reg32), (val))
#define e_llid8_11_cfg_SET_llid11_encrypt_en(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_encrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid11_txfec_en(reg32, val)         REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_txfec_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid11_dcrypt_en(reg32, val)        REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_dcrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid11_dcrypt_mode(reg32, val)      REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_dcrypt_mode, (reg32), (val))
#define e_llid8_11_cfg_SET_llid11_oamlpbk_en(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_oamlpbk_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_dmy(reg32, val)              REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_dmy, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_encrypt_key(reg32, val)      REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_encrypt_key, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_encrypt_en(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_encrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_txfec_en(reg32, val)         REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_txfec_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_dcrypt_en(reg32, val)        REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_dcrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_dcrypt_mode(reg32, val)      REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_dcrypt_mode, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_oamlpbk_en(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_oamlpbk_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_dmy(reg32, val)               REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_dmy, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_encrypt_key(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_encrypt_key, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_encrypt_en(reg32, val)        REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_encrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_txfec_en(reg32, val)          REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_txfec_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_dcrypt_en(reg32, val)         REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_dcrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_dcrypt_mode(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_dcrypt_mode, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_oamlpbk_en(reg32, val)        REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_oamlpbk_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_dmy(reg32, val)               REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_dmy, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_encrypt_key(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_encrypt_key, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_encrypt_en(reg32, val)        REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_encrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_txfec_en(reg32, val)          REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_txfec_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_dcrypt_en(reg32, val)         REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_dcrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_dcrypt_mode(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_dcrypt_mode, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_oamlpbk_en(reg32, val)        REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_oamlpbk_en, (reg32), (val))

#define e_llid12_15_cfg_SET_llid15_dmy(reg32, val)             REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_dmy, (reg32), (val))
#define e_llid12_15_cfg_SET_llid15_encrypt_key(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_encrypt_key, (reg32), (val))
#define e_llid12_15_cfg_SET_llid15_encrypt_en(reg32, val)      REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_encrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid15_txfec_en(reg32, val)        REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_txfec_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid15_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_dcrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid15_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_dcrypt_mode, (reg32), (val))
#define e_llid12_15_cfg_SET_llid15_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_oamlpbk_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_dmy(reg32, val)             REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_dmy, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_encrypt_key(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_encrypt_key, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_encrypt_en(reg32, val)      REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_encrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_txfec_en(reg32, val)        REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_txfec_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_dcrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_dcrypt_mode, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_oamlpbk_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_dmy(reg32, val)             REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_dmy, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_encrypt_key(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_encrypt_key, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_encrypt_en(reg32, val)      REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_encrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_txfec_en(reg32, val)        REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_txfec_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_dcrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_dcrypt_mode, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_oamlpbk_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_dmy(reg32, val)             REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_dmy, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_encrypt_key(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_encrypt_key, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_encrypt_en(reg32, val)      REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_encrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_txfec_en(reg32, val)        REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_txfec_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_dcrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_dcrypt_mode, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_oam_lpbk_en(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_oam_lpbk_en, (reg32), (val))

#define e_llid16_19_cfg_SET_llid19_dmy(reg32, val)             REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_dmy, (reg32), (val))
#define e_llid16_19_cfg_SET_llid19_encrypt_key(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_encrypt_key, (reg32), (val))
#define e_llid16_19_cfg_SET_llid19_encrypt_en(reg32, val)      REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_encrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid19_txfec_en(reg32, val)        REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_txfec_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid19_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_dcrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid19_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_dcrypt_mode, (reg32), (val))
#define e_llid16_19_cfg_SET_llid19_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_oamlpbk_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_dmy(reg32, val)             REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_dmy, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_encrypt_key(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_encrypt_key, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_encrypt_en(reg32, val)      REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_encrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_txfec_en(reg32, val)        REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_txfec_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_dcrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_dcrypt_mode, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_oamlpbk_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_dmy(reg32, val)             REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_dmy, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_encrypt_key(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_encrypt_key, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_encrypt_en(reg32, val)      REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_encrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_txfec_en(reg32, val)        REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_txfec_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_dcrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_dcrypt_mode, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_oamlpbk_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_dmy(reg32, val)             REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_dmy, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_encrypt_key(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_encrypt_key, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_encrypt_en(reg32, val)      REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_encrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_txfec_en(reg32, val)        REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_txfec_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_dcrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_dcrypt_mode, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_oam_lpbk_en(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_oam_lpbk_en, (reg32), (val))

#define e_llid20_23_cfg_SET_llid23_dmy(reg32, val)             REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_dmy, (reg32), (val))
#define e_llid20_23_cfg_SET_llid23_encrypt_key(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_encrypt_key, (reg32), (val))
#define e_llid20_23_cfg_SET_llid23_encrypt_en(reg32, val)      REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_encrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid23_txfec_en(reg32, val)        REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_txfec_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid23_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_dcrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid23_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_dcrypt_mode, (reg32), (val))
#define e_llid20_23_cfg_SET_llid23_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_oamlpbk_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_dmy(reg32, val)             REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_dmy, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_encrypt_key(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_encrypt_key, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_encrypt_en(reg32, val)      REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_encrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_txfec_en(reg32, val)        REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_txfec_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_dcrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_dcrypt_mode, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_oamlpbk_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_dmy(reg32, val)             REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_dmy, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_encrypt_key(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_encrypt_key, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_encrypt_en(reg32, val)      REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_encrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_txfec_en(reg32, val)        REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_txfec_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_dcrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_dcrypt_mode, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_oamlpbk_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_dmy(reg32, val)             REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_dmy, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_encrypt_key(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_encrypt_key, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_encrypt_en(reg32, val)      REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_encrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_txfec_en(reg32, val)        REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_txfec_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_dcrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_dcrypt_mode, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_oam_lpbk_en(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_oam_lpbk_en, (reg32), (val))

#define e_llid24_27_cfg_SET_llid27_dmy(reg32, val)             REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_dmy, (reg32), (val))
#define e_llid24_27_cfg_SET_llid27_encrypt_key(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_encrypt_key, (reg32), (val))
#define e_llid24_27_cfg_SET_llid27_encrypt_en(reg32, val)      REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_encrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid27_txfec_en(reg32, val)        REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_txfec_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid27_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_dcrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid27_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_dcrypt_mode, (reg32), (val))
#define e_llid24_27_cfg_SET_llid27_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_oamlpbk_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_dmy(reg32, val)             REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_dmy, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_encrypt_key(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_encrypt_key, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_encrypt_en(reg32, val)      REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_encrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_txfec_en(reg32, val)        REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_txfec_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_dcrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_dcrypt_mode, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_oamlpbk_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_dmy(reg32, val)             REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_dmy, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_encrypt_key(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_encrypt_key, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_encrypt_en(reg32, val)      REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_encrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_txfec_en(reg32, val)        REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_txfec_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_dcrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_dcrypt_mode, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_oamlpbk_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_dmy(reg32, val)             REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_dmy, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_encrypt_key(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_encrypt_key, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_encrypt_en(reg32, val)      REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_encrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_txfec_en(reg32, val)        REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_txfec_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_dcrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_dcrypt_mode, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_oam_lpbk_en(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_oam_lpbk_en, (reg32), (val))

#define e_llid28_31_cfg_SET_llid31_dmy(reg32, val)             REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_dmy, (reg32), (val))
#define e_llid28_31_cfg_SET_llid31_encrypt_key(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_encrypt_key, (reg32), (val))
#define e_llid28_31_cfg_SET_llid31_encrypt_en(reg32, val)      REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_encrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid31_txfec_en(reg32, val)        REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_txfec_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid31_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_dcrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid31_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_dcrypt_mode, (reg32), (val))
#define e_llid28_31_cfg_SET_llid31_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_oamlpbk_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_dmy(reg32, val)             REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_dmy, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_encrypt_key(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_encrypt_key, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_encrypt_en(reg32, val)      REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_encrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_txfec_en(reg32, val)        REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_txfec_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_dcrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_dcrypt_mode, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_oamlpbk_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_dmy(reg32, val)             REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_dmy, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_encrypt_key(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_encrypt_key, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_encrypt_en(reg32, val)      REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_encrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_txfec_en(reg32, val)        REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_txfec_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_dcrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_dcrypt_mode, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_oamlpbk_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_dmy(reg32, val)             REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_dmy, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_encrypt_key(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_encrypt_key, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_encrypt_en(reg32, val)      REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_encrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_txfec_en(reg32, val)        REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_txfec_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_dcrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_dcrypt_mode, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_oam_lpbk_en(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_oam_lpbk_en, (reg32), (val))

#define e_cfg_dmy2_SET_xe_cfg_dmy2(reg32, val)                 REG_FLD_SET(e_cfg_dmy2_FLD_xe_cfg_dmy2, (reg32), (val))

#define e_cfg_dmy3_SET_xe_cfg_dmy3(reg32, val)                 REG_FLD_SET(e_cfg_dmy3_FLD_xe_cfg_dmy3, (reg32), (val))

#define e_cfg_dmy4_SET_xe_cfg_dmy4(reg32, val)                 REG_FLD_SET(e_cfg_dmy4_FLD_xe_cfg_dmy4, (reg32), (val))

#define e_llid_dscvry_ctrl_SET_mpcp_cmd(reg32, val)            REG_FLD_SET(e_llid_dscvry_ctrl_FLD_mpcp_cmd, (reg32), (val))
#define e_llid_dscvry_ctrl_SET_mpcp_cmd_done(reg32, val)       REG_FLD_SET(e_llid_dscvry_ctrl_FLD_mpcp_cmd_done, (reg32), (val))
#define e_llid_dscvry_ctrl_SET_rgstr_ack_flg(reg32, val)       REG_FLD_SET(e_llid_dscvry_ctrl_FLD_rgstr_ack_flg, (reg32), (val))
#define e_llid_dscvry_ctrl_SET_rgstr_req_flg(reg32, val)       REG_FLD_SET(e_llid_dscvry_ctrl_FLD_rgstr_req_flg, (reg32), (val))
#define e_llid_dscvry_ctrl_SET_tx_mpcp_llid_idx(reg32, val)    REG_FLD_SET(e_llid_dscvry_ctrl_FLD_tx_mpcp_llid_idx, (reg32), (val))

#define e_llid0_dscvry_sts_SET_llid0_dscvry_sts(reg32, val)    REG_FLD_SET(e_llid0_dscvry_sts_FLD_llid0_dscvry_sts, (reg32), (val))
#define e_llid0_dscvry_sts_SET_llid0_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid0_dscvry_sts_FLD_llid0_rgstr_flg_sts, (reg32), (val))
#define e_llid0_dscvry_sts_SET_llid0_valid(reg32, val)         REG_FLD_SET(e_llid0_dscvry_sts_FLD_llid0_valid, (reg32), (val))
#define e_llid0_dscvry_sts_SET_llid0_value(reg32, val)         REG_FLD_SET(e_llid0_dscvry_sts_FLD_llid0_value, (reg32), (val))

#define e_llid1_dscvry_sts_SET_llid1_dscvry_sts(reg32, val)    REG_FLD_SET(e_llid1_dscvry_sts_FLD_llid1_dscvry_sts, (reg32), (val))
#define e_llid1_dscvry_sts_SET_llid1_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid1_dscvry_sts_FLD_llid1_rgstr_flg_sts, (reg32), (val))
#define e_llid1_dscvry_sts_SET_llid1_valid(reg32, val)         REG_FLD_SET(e_llid1_dscvry_sts_FLD_llid1_valid, (reg32), (val))
#define e_llid1_dscvry_sts_SET_llid1_value(reg32, val)         REG_FLD_SET(e_llid1_dscvry_sts_FLD_llid1_value, (reg32), (val))

#define e_llid2_dscvry_sts_SET_llid2_dscvry_sts(reg32, val)    REG_FLD_SET(e_llid2_dscvry_sts_FLD_llid2_dscvry_sts, (reg32), (val))
#define e_llid2_dscvry_sts_SET_llid2_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid2_dscvry_sts_FLD_llid2_rgstr_flg_sts, (reg32), (val))
#define e_llid2_dscvry_sts_SET_llid2_valid(reg32, val)         REG_FLD_SET(e_llid2_dscvry_sts_FLD_llid2_valid, (reg32), (val))
#define e_llid2_dscvry_sts_SET_llid2_value(reg32, val)         REG_FLD_SET(e_llid2_dscvry_sts_FLD_llid2_value, (reg32), (val))

#define e_llid3_dscvry_sts_SET_llid3_dscvry_sts(reg32, val)    REG_FLD_SET(e_llid3_dscvry_sts_FLD_llid3_dscvry_sts, (reg32), (val))
#define e_llid3_dscvry_sts_SET_llid3_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid3_dscvry_sts_FLD_llid3_rgstr_flg_sts, (reg32), (val))
#define e_llid3_dscvry_sts_SET_llid3_valid(reg32, val)         REG_FLD_SET(e_llid3_dscvry_sts_FLD_llid3_valid, (reg32), (val))
#define e_llid3_dscvry_sts_SET_llid3_value(reg32, val)         REG_FLD_SET(e_llid3_dscvry_sts_FLD_llid3_value, (reg32), (val))

#define e_llid4_dscvry_sts_SET_llid4_dscvry_sts(reg32, val)    REG_FLD_SET(e_llid4_dscvry_sts_FLD_llid4_dscvry_sts, (reg32), (val))
#define e_llid4_dscvry_sts_SET_llid4_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid4_dscvry_sts_FLD_llid4_rgstr_flg_sts, (reg32), (val))
#define e_llid4_dscvry_sts_SET_llid4_valid(reg32, val)         REG_FLD_SET(e_llid4_dscvry_sts_FLD_llid4_valid, (reg32), (val))
#define e_llid4_dscvry_sts_SET_llid4_value(reg32, val)         REG_FLD_SET(e_llid4_dscvry_sts_FLD_llid4_value, (reg32), (val))

#define e_llid5_dscvry_sts_SET_llid5_dscvry_sts(reg32, val)    REG_FLD_SET(e_llid5_dscvry_sts_FLD_llid5_dscvry_sts, (reg32), (val))
#define e_llid5_dscvry_sts_SET_llid5_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid5_dscvry_sts_FLD_llid5_rgstr_flg_sts, (reg32), (val))
#define e_llid5_dscvry_sts_SET_llid5_valid(reg32, val)         REG_FLD_SET(e_llid5_dscvry_sts_FLD_llid5_valid, (reg32), (val))
#define e_llid5_dscvry_sts_SET_llid5_value(reg32, val)         REG_FLD_SET(e_llid5_dscvry_sts_FLD_llid5_value, (reg32), (val))

#define e_llid6_dscvry_sts_SET_llid6_dscvry_sts(reg32, val)    REG_FLD_SET(e_llid6_dscvry_sts_FLD_llid6_dscvry_sts, (reg32), (val))
#define e_llid6_dscvry_sts_SET_llid6_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid6_dscvry_sts_FLD_llid6_rgstr_flg_sts, (reg32), (val))
#define e_llid6_dscvry_sts_SET_llid6_valid(reg32, val)         REG_FLD_SET(e_llid6_dscvry_sts_FLD_llid6_valid, (reg32), (val))
#define e_llid6_dscvry_sts_SET_llid6_value(reg32, val)         REG_FLD_SET(e_llid6_dscvry_sts_FLD_llid6_value, (reg32), (val))

#define e_llid7_dscvry_sts_SET_llid7_dscvry_sts(reg32, val)    REG_FLD_SET(e_llid7_dscvry_sts_FLD_llid7_dscvry_sts, (reg32), (val))
#define e_llid7_dscvry_sts_SET_llid7_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid7_dscvry_sts_FLD_llid7_rgstr_flg_sts, (reg32), (val))
#define e_llid7_dscvry_sts_SET_llid7_valid(reg32, val)         REG_FLD_SET(e_llid7_dscvry_sts_FLD_llid7_valid, (reg32), (val))
#define e_llid7_dscvry_sts_SET_llid7_value(reg32, val)         REG_FLD_SET(e_llid7_dscvry_sts_FLD_llid7_value, (reg32), (val))

#define e_llid8_dscvry_sts_SET_llid8_dscvry_sts(reg32, val)    REG_FLD_SET(e_llid8_dscvry_sts_FLD_llid8_dscvry_sts, (reg32), (val))
#define e_llid8_dscvry_sts_SET_llid8_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid8_dscvry_sts_FLD_llid8_rgstr_flg_sts, (reg32), (val))
#define e_llid8_dscvry_sts_SET_llid8_valid(reg32, val)         REG_FLD_SET(e_llid8_dscvry_sts_FLD_llid8_valid, (reg32), (val))
#define e_llid8_dscvry_sts_SET_llid8_value(reg32, val)         REG_FLD_SET(e_llid8_dscvry_sts_FLD_llid8_value, (reg32), (val))

#define e_llid9_dscvry_sts_SET_llid9_dscvry_sts(reg32, val)    REG_FLD_SET(e_llid9_dscvry_sts_FLD_llid9_dscvry_sts, (reg32), (val))
#define e_llid9_dscvry_sts_SET_llid9_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid9_dscvry_sts_FLD_llid9_rgstr_flg_sts, (reg32), (val))
#define e_llid9_dscvry_sts_SET_llid9_valid(reg32, val)         REG_FLD_SET(e_llid9_dscvry_sts_FLD_llid9_valid, (reg32), (val))
#define e_llid9_dscvry_sts_SET_llid9_value(reg32, val)         REG_FLD_SET(e_llid9_dscvry_sts_FLD_llid9_value, (reg32), (val))

#define e_llid10_dscvry_sts_SET_llid10_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid10_dscvry_sts_FLD_llid10_dscvry_sts, (reg32), (val))
#define e_llid10_dscvry_sts_SET_llid10_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid10_dscvry_sts_FLD_llid10_rgstr_flg_sts, (reg32), (val))
#define e_llid10_dscvry_sts_SET_llid10_valid(reg32, val)       REG_FLD_SET(e_llid10_dscvry_sts_FLD_llid10_valid, (reg32), (val))
#define e_llid10_dscvry_sts_SET_llid10_value(reg32, val)       REG_FLD_SET(e_llid10_dscvry_sts_FLD_llid10_value, (reg32), (val))

#define e_llid11_dscvry_sts_SET_llid11_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid11_dscvry_sts_FLD_llid11_dscvry_sts, (reg32), (val))
#define e_llid11_dscvry_sts_SET_llid11_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid11_dscvry_sts_FLD_llid11_rgstr_flg_sts, (reg32), (val))
#define e_llid11_dscvry_sts_SET_llid11_valid(reg32, val)       REG_FLD_SET(e_llid11_dscvry_sts_FLD_llid11_valid, (reg32), (val))
#define e_llid11_dscvry_sts_SET_llid11_value(reg32, val)       REG_FLD_SET(e_llid11_dscvry_sts_FLD_llid11_value, (reg32), (val))

#define e_llid12_dscvry_sts_SET_llid12_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid12_dscvry_sts_FLD_llid12_dscvry_sts, (reg32), (val))
#define e_llid12_dscvry_sts_SET_llid12_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid12_dscvry_sts_FLD_llid12_rgstr_flg_sts, (reg32), (val))
#define e_llid12_dscvry_sts_SET_llid12_valid(reg32, val)       REG_FLD_SET(e_llid12_dscvry_sts_FLD_llid12_valid, (reg32), (val))
#define e_llid12_dscvry_sts_SET_llid12_value(reg32, val)       REG_FLD_SET(e_llid12_dscvry_sts_FLD_llid12_value, (reg32), (val))

#define e_llid13_dscvry_sts_SET_llid13_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid13_dscvry_sts_FLD_llid13_dscvry_sts, (reg32), (val))
#define e_llid13_dscvry_sts_SET_llid13_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid13_dscvry_sts_FLD_llid13_rgstr_flg_sts, (reg32), (val))
#define e_llid13_dscvry_sts_SET_llid13_valid(reg32, val)       REG_FLD_SET(e_llid13_dscvry_sts_FLD_llid13_valid, (reg32), (val))
#define e_llid13_dscvry_sts_SET_llid13_value(reg32, val)       REG_FLD_SET(e_llid13_dscvry_sts_FLD_llid13_value, (reg32), (val))

#define e_llid14_dscvry_sts_SET_llid14_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid14_dscvry_sts_FLD_llid14_dscvry_sts, (reg32), (val))
#define e_llid14_dscvry_sts_SET_llid14_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid14_dscvry_sts_FLD_llid14_rgstr_flg_sts, (reg32), (val))
#define e_llid14_dscvry_sts_SET_llid14_valid(reg32, val)       REG_FLD_SET(e_llid14_dscvry_sts_FLD_llid14_valid, (reg32), (val))
#define e_llid14_dscvry_sts_SET_llid14_value(reg32, val)       REG_FLD_SET(e_llid14_dscvry_sts_FLD_llid14_value, (reg32), (val))

#define e_llid15_dscvry_sts_SET_llid15_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid15_dscvry_sts_FLD_llid15_dscvry_sts, (reg32), (val))
#define e_llid15_dscvry_sts_SET_llid15_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid15_dscvry_sts_FLD_llid15_rgstr_flg_sts, (reg32), (val))
#define e_llid15_dscvry_sts_SET_llid15_valid(reg32, val)       REG_FLD_SET(e_llid15_dscvry_sts_FLD_llid15_valid, (reg32), (val))
#define e_llid15_dscvry_sts_SET_llid15_value(reg32, val)       REG_FLD_SET(e_llid15_dscvry_sts_FLD_llid15_value, (reg32), (val))

#define e_llid16_dscvry_sts_SET_llid16_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid16_dscvry_sts_FLD_llid16_dscvry_sts, (reg32), (val))
#define e_llid16_dscvry_sts_SET_llid16_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid16_dscvry_sts_FLD_llid16_rgstr_flg_sts, (reg32), (val))
#define e_llid16_dscvry_sts_SET_llid16_valid(reg32, val)       REG_FLD_SET(e_llid16_dscvry_sts_FLD_llid16_valid, (reg32), (val))
#define e_llid16_dscvry_sts_SET_llid16_value(reg32, val)       REG_FLD_SET(e_llid16_dscvry_sts_FLD_llid16_value, (reg32), (val))

#define e_llid17_dscvry_sts_SET_llid17_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid17_dscvry_sts_FLD_llid17_dscvry_sts, (reg32), (val))
#define e_llid17_dscvry_sts_SET_llid17_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid17_dscvry_sts_FLD_llid17_rgstr_flg_sts, (reg32), (val))
#define e_llid17_dscvry_sts_SET_llid17_valid(reg32, val)       REG_FLD_SET(e_llid17_dscvry_sts_FLD_llid17_valid, (reg32), (val))
#define e_llid17_dscvry_sts_SET_llid17_value(reg32, val)       REG_FLD_SET(e_llid17_dscvry_sts_FLD_llid17_value, (reg32), (val))

#define e_llid18_dscvry_sts_SET_llid18_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid18_dscvry_sts_FLD_llid18_dscvry_sts, (reg32), (val))
#define e_llid18_dscvry_sts_SET_llid18_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid18_dscvry_sts_FLD_llid18_rgstr_flg_sts, (reg32), (val))
#define e_llid18_dscvry_sts_SET_llid18_valid(reg32, val)       REG_FLD_SET(e_llid18_dscvry_sts_FLD_llid18_valid, (reg32), (val))
#define e_llid18_dscvry_sts_SET_llid18_value(reg32, val)       REG_FLD_SET(e_llid18_dscvry_sts_FLD_llid18_value, (reg32), (val))

#define e_llid19_dscvry_sts_SET_llid19_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid19_dscvry_sts_FLD_llid19_dscvry_sts, (reg32), (val))
#define e_llid19_dscvry_sts_SET_llid19_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid19_dscvry_sts_FLD_llid19_rgstr_flg_sts, (reg32), (val))
#define e_llid19_dscvry_sts_SET_llid19_valid(reg32, val)       REG_FLD_SET(e_llid19_dscvry_sts_FLD_llid19_valid, (reg32), (val))
#define e_llid19_dscvry_sts_SET_llid19_value(reg32, val)       REG_FLD_SET(e_llid19_dscvry_sts_FLD_llid19_value, (reg32), (val))

#define e_llid20_dscvry_sts_SET_llid20_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid20_dscvry_sts_FLD_llid20_dscvry_sts, (reg32), (val))
#define e_llid20_dscvry_sts_SET_llid20_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid20_dscvry_sts_FLD_llid20_rgstr_flg_sts, (reg32), (val))
#define e_llid20_dscvry_sts_SET_llid20_valid(reg32, val)       REG_FLD_SET(e_llid20_dscvry_sts_FLD_llid20_valid, (reg32), (val))
#define e_llid20_dscvry_sts_SET_llid20_value(reg32, val)       REG_FLD_SET(e_llid20_dscvry_sts_FLD_llid20_value, (reg32), (val))

#define e_llid21_dscvry_sts_SET_llid21_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid21_dscvry_sts_FLD_llid21_dscvry_sts, (reg32), (val))
#define e_llid21_dscvry_sts_SET_llid21_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid21_dscvry_sts_FLD_llid21_rgstr_flg_sts, (reg32), (val))
#define e_llid21_dscvry_sts_SET_llid21_valid(reg32, val)       REG_FLD_SET(e_llid21_dscvry_sts_FLD_llid21_valid, (reg32), (val))
#define e_llid21_dscvry_sts_SET_llid21_value(reg32, val)       REG_FLD_SET(e_llid21_dscvry_sts_FLD_llid21_value, (reg32), (val))

#define e_llid22_dscvry_sts_SET_llid22_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid22_dscvry_sts_FLD_llid22_dscvry_sts, (reg32), (val))
#define e_llid22_dscvry_sts_SET_llid22_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid22_dscvry_sts_FLD_llid22_rgstr_flg_sts, (reg32), (val))
#define e_llid22_dscvry_sts_SET_llid22_valid(reg32, val)       REG_FLD_SET(e_llid22_dscvry_sts_FLD_llid22_valid, (reg32), (val))
#define e_llid22_dscvry_sts_SET_llid22_value(reg32, val)       REG_FLD_SET(e_llid22_dscvry_sts_FLD_llid22_value, (reg32), (val))

#define e_llid23_dscvry_sts_SET_llid23_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid23_dscvry_sts_FLD_llid23_dscvry_sts, (reg32), (val))
#define e_llid23_dscvry_sts_SET_llid23_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid23_dscvry_sts_FLD_llid23_rgstr_flg_sts, (reg32), (val))
#define e_llid23_dscvry_sts_SET_llid23_valid(reg32, val)       REG_FLD_SET(e_llid23_dscvry_sts_FLD_llid23_valid, (reg32), (val))
#define e_llid23_dscvry_sts_SET_llid23_value(reg32, val)       REG_FLD_SET(e_llid23_dscvry_sts_FLD_llid23_value, (reg32), (val))

#define e_llid24_dscvry_sts_SET_llid24_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid24_dscvry_sts_FLD_llid24_dscvry_sts, (reg32), (val))
#define e_llid24_dscvry_sts_SET_llid24_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid24_dscvry_sts_FLD_llid24_rgstr_flg_sts, (reg32), (val))
#define e_llid24_dscvry_sts_SET_llid24_valid(reg32, val)       REG_FLD_SET(e_llid24_dscvry_sts_FLD_llid24_valid, (reg32), (val))
#define e_llid24_dscvry_sts_SET_llid24_value(reg32, val)       REG_FLD_SET(e_llid24_dscvry_sts_FLD_llid24_value, (reg32), (val))

#define e_llid25_dscvry_sts_SET_llid25_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid25_dscvry_sts_FLD_llid25_dscvry_sts, (reg32), (val))
#define e_llid25_dscvry_sts_SET_llid25_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid25_dscvry_sts_FLD_llid25_rgstr_flg_sts, (reg32), (val))
#define e_llid25_dscvry_sts_SET_llid25_valid(reg32, val)       REG_FLD_SET(e_llid25_dscvry_sts_FLD_llid25_valid, (reg32), (val))
#define e_llid25_dscvry_sts_SET_llid25_value(reg32, val)       REG_FLD_SET(e_llid25_dscvry_sts_FLD_llid25_value, (reg32), (val))

#define e_llid26_dscvry_sts_SET_llid26_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid26_dscvry_sts_FLD_llid26_dscvry_sts, (reg32), (val))
#define e_llid26_dscvry_sts_SET_llid26_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid26_dscvry_sts_FLD_llid26_rgstr_flg_sts, (reg32), (val))
#define e_llid26_dscvry_sts_SET_llid26_valid(reg32, val)       REG_FLD_SET(e_llid26_dscvry_sts_FLD_llid26_valid, (reg32), (val))
#define e_llid26_dscvry_sts_SET_llid26_value(reg32, val)       REG_FLD_SET(e_llid26_dscvry_sts_FLD_llid26_value, (reg32), (val))

#define e_llid27_dscvry_sts_SET_llid27_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid27_dscvry_sts_FLD_llid27_dscvry_sts, (reg32), (val))
#define e_llid27_dscvry_sts_SET_llid27_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid27_dscvry_sts_FLD_llid27_rgstr_flg_sts, (reg32), (val))
#define e_llid27_dscvry_sts_SET_llid27_valid(reg32, val)       REG_FLD_SET(e_llid27_dscvry_sts_FLD_llid27_valid, (reg32), (val))
#define e_llid27_dscvry_sts_SET_llid27_value(reg32, val)       REG_FLD_SET(e_llid27_dscvry_sts_FLD_llid27_value, (reg32), (val))

#define e_llid28_dscvry_sts_SET_llid28_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid28_dscvry_sts_FLD_llid28_dscvry_sts, (reg32), (val))
#define e_llid28_dscvry_sts_SET_llid28_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid28_dscvry_sts_FLD_llid28_rgstr_flg_sts, (reg32), (val))
#define e_llid28_dscvry_sts_SET_llid28_valid(reg32, val)       REG_FLD_SET(e_llid28_dscvry_sts_FLD_llid28_valid, (reg32), (val))
#define e_llid28_dscvry_sts_SET_llid28_value(reg32, val)       REG_FLD_SET(e_llid28_dscvry_sts_FLD_llid28_value, (reg32), (val))

#define e_llid29_dscvry_sts_SET_llid29_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid29_dscvry_sts_FLD_llid29_dscvry_sts, (reg32), (val))
#define e_llid29_dscvry_sts_SET_llid29_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid29_dscvry_sts_FLD_llid29_rgstr_flg_sts, (reg32), (val))
#define e_llid29_dscvry_sts_SET_llid29_valid(reg32, val)       REG_FLD_SET(e_llid29_dscvry_sts_FLD_llid29_valid, (reg32), (val))
#define e_llid29_dscvry_sts_SET_llid29_value(reg32, val)       REG_FLD_SET(e_llid29_dscvry_sts_FLD_llid29_value, (reg32), (val))

#define e_llid30_dscvry_sts_SET_llid30_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid30_dscvry_sts_FLD_llid30_dscvry_sts, (reg32), (val))
#define e_llid30_dscvry_sts_SET_llid30_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid30_dscvry_sts_FLD_llid30_rgstr_flg_sts, (reg32), (val))
#define e_llid30_dscvry_sts_SET_llid30_valid(reg32, val)       REG_FLD_SET(e_llid30_dscvry_sts_FLD_llid30_valid, (reg32), (val))
#define e_llid30_dscvry_sts_SET_llid30_value(reg32, val)       REG_FLD_SET(e_llid30_dscvry_sts_FLD_llid30_value, (reg32), (val))

#define e_llid31_dscvry_sts_SET_llid31_dscvry_sts(reg32, val)  REG_FLD_SET(e_llid31_dscvry_sts_FLD_llid31_dscvry_sts, (reg32), (val))
#define e_llid31_dscvry_sts_SET_llid31_rgstr_flg_sts(reg32, val) REG_FLD_SET(e_llid31_dscvry_sts_FLD_llid31_rgstr_flg_sts, (reg32), (val))
#define e_llid31_dscvry_sts_SET_llid31_valid(reg32, val)       REG_FLD_SET(e_llid31_dscvry_sts_FLD_llid31_valid, (reg32), (val))
#define e_llid31_dscvry_sts_SET_llid31_value(reg32, val)       REG_FLD_SET(e_llid31_dscvry_sts_FLD_llid31_value, (reg32), (val))

#define e_pending_gnt_num_SET_echoed_pending_gnt(reg32, val)   REG_FLD_SET(e_pending_gnt_num_FLD_echoed_pending_gnt, (reg32), (val))
#define e_pending_gnt_num_SET_pending_gnt_num(reg32, val)      REG_FLD_SET(e_pending_gnt_num_FLD_pending_gnt_num, (reg32), (val))

#define e_mac_addr_cfg_SET_mac_addr_rwcmd(reg32, val)          REG_FLD_SET(e_mac_addr_cfg_FLD_mac_addr_rwcmd, (reg32), (val))
#define e_mac_addr_cfg_SET_mac_addr_rwcmd_done(reg32, val)     REG_FLD_SET(e_mac_addr_cfg_FLD_mac_addr_rwcmd_done, (reg32), (val))
#define e_mac_addr_cfg_SET_mac_addr_llid_indx(reg32, val)      REG_FLD_SET(e_mac_addr_cfg_FLD_mac_addr_llid_indx, (reg32), (val))
#define e_mac_addr_cfg_SET_mac_addr_dw_idx(reg32, val)         REG_FLD_SET(e_mac_addr_cfg_FLD_mac_addr_dw_idx, (reg32), (val))

#define e_mac_addr_value_SET_mac_addr_value(reg32, val)        REG_FLD_SET(e_mac_addr_value_FLD_mac_addr_value, (reg32), (val))

#define e_security_key_cfg_SET_key_rwcmd(reg32, val)           REG_FLD_SET(e_security_key_cfg_FLD_key_rwcmd, (reg32), (val))
#define e_security_key_cfg_SET_key_macsec_an(reg32, val)       REG_FLD_SET(e_security_key_cfg_FLD_key_macsec_an, (reg32), (val))
#define e_security_key_cfg_SET_key_rwcmd_done(reg32, val)      REG_FLD_SET(e_security_key_cfg_FLD_key_rwcmd_done, (reg32), (val))
#define e_security_key_cfg_SET_key_llid_index(reg32, val)      REG_FLD_SET(e_security_key_cfg_FLD_key_llid_index, (reg32), (val))
#define e_security_key_cfg_SET_key_idx(reg32, val)             REG_FLD_SET(e_security_key_cfg_FLD_key_idx, (reg32), (val))
#define e_security_key_cfg_SET_key_dw_indx(reg32, val)         REG_FLD_SET(e_security_key_cfg_FLD_key_dw_indx, (reg32), (val))

#define e_key_value_SET_key_value(reg32, val)                  REG_FLD_SET(e_key_value_FLD_key_value, (reg32), (val))

#define e_enckey_cfg_SET_enckey_rwcmd(reg32, val)              REG_FLD_SET(e_enckey_cfg_FLD_enckey_rwcmd, (reg32), (val))
#define e_enckey_cfg_SET_enckey_macsec_an(reg32, val)          REG_FLD_SET(e_enckey_cfg_FLD_enckey_macsec_an, (reg32), (val))
#define e_enckey_cfg_SET_enckey_rwcmd_done(reg32, val)         REG_FLD_SET(e_enckey_cfg_FLD_enckey_rwcmd_done, (reg32), (val))
#define e_enckey_cfg_SET_enckey_llididx(reg32, val)            REG_FLD_SET(e_enckey_cfg_FLD_enckey_llididx, (reg32), (val))
#define e_enckey_cfg_SET_enckey_keyidx(reg32, val)             REG_FLD_SET(e_enckey_cfg_FLD_enckey_keyidx, (reg32), (val))
#define e_enckey_cfg_SET_enckey_dwidx(reg32, val)              REG_FLD_SET(e_enckey_cfg_FLD_enckey_dwidx, (reg32), (val))

#define e_enckey_val_SET_enckey_value(reg32, val)              REG_FLD_SET(e_enckey_val_FLD_enckey_value, (reg32), (val))

#define e_cfg_dmy5_SET_xe_cfg_dmy5(reg32, val)                 REG_FLD_SET(e_cfg_dmy5_FLD_xe_cfg_dmy5, (reg32), (val))

#define e_rpt_cfg_SET_llid15_rpt_cfg(reg32, val)               REG_FLD_SET(e_rpt_cfg_FLD_llid15_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid14_rpt_cfg(reg32, val)               REG_FLD_SET(e_rpt_cfg_FLD_llid14_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid13_rpt_cfg(reg32, val)               REG_FLD_SET(e_rpt_cfg_FLD_llid13_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid12_rpt_cfg(reg32, val)               REG_FLD_SET(e_rpt_cfg_FLD_llid12_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid11_rpt_cfg(reg32, val)               REG_FLD_SET(e_rpt_cfg_FLD_llid11_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid10_rpt_cfg(reg32, val)               REG_FLD_SET(e_rpt_cfg_FLD_llid10_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid9_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid9_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid8_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid8_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid7_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid7_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid6_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid6_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid5_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid5_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid4_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid4_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid3_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid3_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid2_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid2_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid1_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid1_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid0_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid0_rpt_cfg, (reg32), (val))

#define e_rpt_cfg2_SET_llid31_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid31_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid30_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid30_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid29_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid29_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid28_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid28_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid27_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid27_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid26_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid26_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid25_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid25_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid24_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid24_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid23_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid23_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid22_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid22_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid21_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid21_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid20_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid20_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid19_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid19_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid18_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid18_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid17_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid17_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid16_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid16_rpt_cfg, (reg32), (val))

#define e_rpt_qthld_cfg_SET_qthld_rwcmd(reg32, val)            REG_FLD_SET(e_rpt_qthld_cfg_FLD_qthld_rwcmd, (reg32), (val))
#define e_rpt_qthld_cfg_SET_qthld_rwcmd_done(reg32, val)       REG_FLD_SET(e_rpt_qthld_cfg_FLD_qthld_rwcmd_done, (reg32), (val))
#define e_rpt_qthld_cfg_SET_rpt_llid_idx(reg32, val)           REG_FLD_SET(e_rpt_qthld_cfg_FLD_rpt_llid_idx, (reg32), (val))
#define e_rpt_qthld_cfg_SET_qthld_value(reg32, val)            REG_FLD_SET(e_rpt_qthld_cfg_FLD_qthld_value, (reg32), (val))
#define e_rpt_qthld_cfg_SET_qthld_idx(reg32, val)              REG_FLD_SET(e_rpt_qthld_cfg_FLD_qthld_idx, (reg32), (val))
#define e_rpt_qthld_cfg_SET_queue_idx(reg32, val)              REG_FLD_SET(e_rpt_qthld_cfg_FLD_queue_idx, (reg32), (val))

#define e_rpt_glb_cfg_SET_rpt_dmy1(reg32, val)                 REG_FLD_SET(e_rpt_glb_cfg_FLD_rpt_dmy1, (reg32), (val))
#define e_rpt_glb_cfg_SET_dba_gtthr_frpt(reg32, val)           REG_FLD_SET(e_rpt_glb_cfg_FLD_dba_gtthr_frpt, (reg32), (val))
#define e_rpt_glb_cfg_SET_txgnt_cnthit_frpt(reg32, val)        REG_FLD_SET(e_rpt_glb_cfg_FLD_txgnt_cnthit_frpt, (reg32), (val))
#define e_rpt_glb_cfg_SET_ctrlpkt_frpt(reg32, val)             REG_FLD_SET(e_rpt_glb_cfg_FLD_ctrlpkt_frpt, (reg32), (val))
#define e_rpt_glb_cfg_SET_gntfrpt_ignore(reg32, val)           REG_FLD_SET(e_rpt_glb_cfg_FLD_gntfrpt_ignore, (reg32), (val))
#define e_rpt_glb_cfg_SET_rpt_qsize_sp(reg32, val)             REG_FLD_SET(e_rpt_glb_cfg_FLD_rpt_qsize_sp, (reg32), (val))
#define e_rpt_glb_cfg_SET_rpt_fullqsize_head(reg32, val)       REG_FLD_SET(e_rpt_glb_cfg_FLD_rpt_fullqsize_head, (reg32), (val))
#define e_rpt_glb_cfg_SET_rpt_dpoe_thrstop(reg32, val)         REG_FLD_SET(e_rpt_glb_cfg_FLD_rpt_dpoe_thrstop, (reg32), (val))
#define e_rpt_glb_cfg_SET_rpt_dpoe_queidx(reg32, val)          REG_FLD_SET(e_rpt_glb_cfg_FLD_rpt_dpoe_queidx, (reg32), (val))
#define e_rpt_glb_cfg_SET_rpt_dpoe_en(reg32, val)              REG_FLD_SET(e_rpt_glb_cfg_FLD_rpt_dpoe_en, (reg32), (val))
#define e_rpt_glb_cfg_SET_rpt_qsize_dec(reg32, val)            REG_FLD_SET(e_rpt_glb_cfg_FLD_rpt_qsize_dec, (reg32), (val))
#define e_rpt_glb_cfg_SET_rpt_qsize_mgnt_sel(reg32, val)       REG_FLD_SET(e_rpt_glb_cfg_FLD_rpt_qsize_mgnt_sel, (reg32), (val))
#define e_rpt_glb_cfg_SET_tx_default_rpt(reg32, val)           REG_FLD_SET(e_rpt_glb_cfg_FLD_tx_default_rpt, (reg32), (val))

#define e_rpt_bmap_SET_rpt_dmy2(reg32, val)                    REG_FLD_SET(e_rpt_bmap_FLD_rpt_dmy2, (reg32), (val))
#define e_rpt_bmap_SET_rpt_bitmap_ctrl(reg32, val)             REG_FLD_SET(e_rpt_bmap_FLD_rpt_bitmap_ctrl, (reg32), (val))
#define e_rpt_bmap_SET_rpt_bitmap_set(reg32, val)              REG_FLD_SET(e_rpt_bmap_FLD_rpt_bitmap_set, (reg32), (val))

#define e_frpt_thr_SET_frpt_txgnt_cnt(reg32, val)              REG_FLD_SET(e_frpt_thr_FLD_frpt_txgnt_cnt, (reg32), (val))
#define e_frpt_thr_SET_frpt_dba_totsize_thr(reg32, val)        REG_FLD_SET(e_frpt_thr_FLD_frpt_dba_totsize_thr, (reg32), (val))

#define e_u1g_rpt_qsizeadj_SET_u1g_fecon_rpt_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj_FLD_u1g_fecon_rpt_qsizeadj, (reg32), (val))
#define e_u1g_rpt_qsizeadj_SET_u1g_fecoff_rpt_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj_FLD_u1g_fecoff_rpt_qsizeadj, (reg32), (val))

#define e_u10g_rpt_qsizeadj_SET_u10g_rpt_qsizeadj(reg32, val)  REG_FLD_SET(e_u10g_rpt_qsizeadj_FLD_u10g_rpt_qsizeadj, (reg32), (val))

#define e_cfg_dmy7_SET_xe_cfg_dmy7(reg32, val)                 REG_FLD_SET(e_cfg_dmy7_FLD_xe_cfg_dmy7, (reg32), (val))

#define e_cfg_dmy8_SET_xe_cfg_dmy8(reg32, val)                 REG_FLD_SET(e_cfg_dmy8_FLD_xe_cfg_dmy8, (reg32), (val))

#define e_cfg_dmy9_SET_xe_cfg_dmy9(reg32, val)                 REG_FLD_SET(e_cfg_dmy9_FLD_xe_cfg_dmy9, (reg32), (val))

#define e_cfg_dmy10_SET_xe_cfg_dmy10(reg32, val)               REG_FLD_SET(e_cfg_dmy10_FLD_xe_cfg_dmy10, (reg32), (val))

#define e_cfg_dmy11_SET_xe_cfg_dmy11(reg32, val)               REG_FLD_SET(e_cfg_dmy11_FLD_xe_cfg_dmy11, (reg32), (val))

#define e_cfg_dmy12_SET_xe_cfg_dmy12(reg32, val)               REG_FLD_SET(e_cfg_dmy12_FLD_xe_cfg_dmy12, (reg32), (val))

#define e_cfg_dmy13_SET_xe_cfg_dmy13(reg32, val)               REG_FLD_SET(e_cfg_dmy13_FLD_xe_cfg_dmy13, (reg32), (val))

#define e_cfg_dmy14_SET_xe_cfg_dmy14(reg32, val)               REG_FLD_SET(e_cfg_dmy14_FLD_xe_cfg_dmy14, (reg32), (val))

#define e_cfg_dmy15_SET_xe_cfg_dmy15(reg32, val)               REG_FLD_SET(e_cfg_dmy15_FLD_xe_cfg_dmy15, (reg32), (val))

#define e_dfrpt_data5_SET_dfrpt_data5(reg32, val)              REG_FLD_SET(e_dfrpt_data5_FLD_dfrpt_data5, (reg32), (val))

#define e_dfrpt_data6_SET_dfrpt_data6(reg32, val)              REG_FLD_SET(e_dfrpt_data6_FLD_dfrpt_data6, (reg32), (val))

#define e_dfrpt_data7_SET_dfrpt_data7(reg32, val)              REG_FLD_SET(e_dfrpt_data7_FLD_dfrpt_data7, (reg32), (val))

#define e_dfrpt_data8_SET_dfrpt_data8(reg32, val)              REG_FLD_SET(e_dfrpt_data8_FLD_dfrpt_data8, (reg32), (val))

#define e_dfrpt_data9_SET_dfrpt_data9(reg32, val)              REG_FLD_SET(e_dfrpt_data9_FLD_dfrpt_data9, (reg32), (val))

#define e_dfrpt_data10_SET_dfrpt_data10(reg32, val)            REG_FLD_SET(e_dfrpt_data10_FLD_dfrpt_data10, (reg32), (val))

#define e_dfrpt_data11_SET_dfrpt_data11(reg32, val)            REG_FLD_SET(e_dfrpt_data11_FLD_dfrpt_data11, (reg32), (val))

#define e_dfrpt_data12_SET_dfrpt_data12(reg32, val)            REG_FLD_SET(e_dfrpt_data12_FLD_dfrpt_data12, (reg32), (val))

#define e_dfrpt_data13_SET_dfrpt_data13(reg32, val)            REG_FLD_SET(e_dfrpt_data13_FLD_dfrpt_data13, (reg32), (val))

#define e_dfrpt_data14_SET_dfrpt_data14(reg32, val)            REG_FLD_SET(e_dfrpt_data14_FLD_dfrpt_data14, (reg32), (val))

#define e_laser_onoff_time_SET_laser_off_time(reg32, val)      REG_FLD_SET(e_laser_onoff_time_FLD_laser_off_time, (reg32), (val))
#define e_laser_onoff_time_SET_laser_on_time(reg32, val)       REG_FLD_SET(e_laser_onoff_time_FLD_laser_on_time, (reg32), (val))

#define e_sync_time_SET_sync_time_updte(reg32, val)            REG_FLD_SET(e_sync_time_FLD_sync_time_updte, (reg32), (val))
#define e_sync_time_SET_sync_time(reg32, val)                  REG_FLD_SET(e_sync_time_FLD_sync_time, (reg32), (val))

#define e_overhead_time_thr_SET_sync_time_maxen(reg32, val)    REG_FLD_SET(e_overhead_time_thr_FLD_sync_time_maxen, (reg32), (val))
#define e_overhead_time_thr_SET_lsroff_time_maxen(reg32, val)  REG_FLD_SET(e_overhead_time_thr_FLD_lsroff_time_maxen, (reg32), (val))
#define e_overhead_time_thr_SET_lsron_time_maxen(reg32, val)   REG_FLD_SET(e_overhead_time_thr_FLD_lsron_time_maxen, (reg32), (val))
#define e_overhead_time_thr_SET_sync_time_maxval(reg32, val)   REG_FLD_SET(e_overhead_time_thr_FLD_sync_time_maxval, (reg32), (val))
#define e_overhead_time_thr_SET_lsroff_time_maxval(reg32, val) REG_FLD_SET(e_overhead_time_thr_FLD_lsroff_time_maxval, (reg32), (val))
#define e_overhead_time_thr_SET_lsron_time_maxval(reg32, val)  REG_FLD_SET(e_overhead_time_thr_FLD_lsron_time_maxval, (reg32), (val))

#define e_laser_onoff_time2_SET_olt_lsroff_time(reg32, val)    REG_FLD_SET(e_laser_onoff_time2_FLD_olt_lsroff_time, (reg32), (val))
#define e_laser_onoff_time2_SET_olt_lsron_time(reg32, val)     REG_FLD_SET(e_laser_onoff_time2_FLD_olt_lsron_time, (reg32), (val))
#define e_laser_onoff_time2_SET_olt_sync_time(reg32, val)      REG_FLD_SET(e_laser_onoff_time2_FLD_olt_sync_time, (reg32), (val))

#define e_olt_dscvinfo_SET_olt_dscvinfo_match(reg32, val)      REG_FLD_SET(e_olt_dscvinfo_FLD_olt_dscvinfo_match, (reg32), (val))
#define e_olt_dscvinfo_SET_olt_dscvinfo_mis(reg32, val)        REG_FLD_SET(e_olt_dscvinfo_FLD_olt_dscvinfo_mis, (reg32), (val))

#define e_grd_thrshld_SET_guard_thrshld(reg32, val)            REG_FLD_SET(e_grd_thrshld_FLD_guard_thrshld, (reg32), (val))

#define e_mpcp_timeout_intvl_SET_mpcp_timeout_intvl(reg32, val) REG_FLD_SET(e_mpcp_timeout_intvl_FLD_mpcp_timeout_intvl, (reg32), (val))

#define e_rpt_timeout_intvl_SET_rpt_timeout_intvl(reg32, val)  REG_FLD_SET(e_rpt_timeout_intvl_FLD_rpt_timeout_intvl, (reg32), (val))

#define e_max_future_gnt_time_SET_max_future_gnt_time(reg32, val) REG_FLD_SET(e_max_future_gnt_time_FLD_max_future_gnt_time, (reg32), (val))

#define e_min_proc_time_SET_min_proc_time(reg32, val)          REG_FLD_SET(e_min_proc_time_FLD_min_proc_time, (reg32), (val))

#define e_trx_adjust_time1_SET_tx_stm_adj(reg32, val)          REG_FLD_SET(e_trx_adjust_time1_FLD_tx_stm_adj, (reg32), (val))

#define e_trx_adjust_time2_SET_tx_len_adj(reg32, val)          REG_FLD_SET(e_trx_adjust_time2_FLD_tx_len_adj, (reg32), (val))
#define e_trx_adjust_time2_SET_rx_tmstp_adj(reg32, val)        REG_FLD_SET(e_trx_adjust_time2_FLD_rx_tmstp_adj, (reg32), (val))

#define e_trx_adjtime3_SET_u10g_tx_tsadj(reg32, val)           REG_FLD_SET(e_trx_adjtime3_FLD_u10g_tx_tsadj, (reg32), (val))
#define e_trx_adjtime3_SET_u10g_tx_stmadj(reg32, val)          REG_FLD_SET(e_trx_adjtime3_FLD_u10g_tx_stmadj, (reg32), (val))

#define e_trx_adjtime4_SET_d10g_rx_tsadj(reg32, val)           REG_FLD_SET(e_trx_adjtime4_FLD_d10g_rx_tsadj, (reg32), (val))
#define e_trx_adjtime4_SET_d10g_rx_tsadj_ofst(reg32, val)      REG_FLD_SET(e_trx_adjtime4_FLD_d10g_rx_tsadj_ofst, (reg32), (val))

#define e_trx_adjtime5_SET_u1g_tx_tsadj(reg32, val)            REG_FLD_SET(e_trx_adjtime5_FLD_u1g_tx_tsadj, (reg32), (val))

#define e_rxphydly_adjtime_SET_rx_phydly_dft(reg32, val)       REG_FLD_SET(e_rxphydly_adjtime_FLD_rx_phydly_dft, (reg32), (val))
#define e_rxphydly_adjtime_SET_d1g_rx_phydly_ofst(reg32, val)  REG_FLD_SET(e_rxphydly_adjtime_FLD_d1g_rx_phydly_ofst, (reg32), (val))
#define e_rxphydly_adjtime_SET_d1g_rx_phydly(reg32, val)       REG_FLD_SET(e_rxphydly_adjtime_FLD_d1g_rx_phydly, (reg32), (val))
#define e_rxphydly_adjtime_SET_d10g_rx_phydly_ofst(reg32, val) REG_FLD_SET(e_rxphydly_adjtime_FLD_d10g_rx_phydly_ofst, (reg32), (val))
#define e_rxphydly_adjtime_SET_d10g_rx_phydly(reg32, val)      REG_FLD_SET(e_rxphydly_adjtime_FLD_d10g_rx_phydly, (reg32), (val))

#define e_txfetch_cfg_SET_tx_fetch_leadtime(reg32, val)        REG_FLD_SET(e_txfetch_cfg_FLD_tx_fetch_leadtime, (reg32), (val))
#define e_txfetch_cfg_SET_tx_dma_leadtime(reg32, val)          REG_FLD_SET(e_txfetch_cfg_FLD_tx_dma_leadtime, (reg32), (val))

#define e_tx_cal_cnst_SET_dscvr_gnt_len(reg32, val)            REG_FLD_SET(e_tx_cal_cnst_FLD_dscvr_gnt_len, (reg32), (val))
#define e_tx_cal_cnst_SET_fec_tail_grd(reg32, val)             REG_FLD_SET(e_tx_cal_cnst_FLD_fec_tail_grd, (reg32), (val))
#define e_tx_cal_cnst_SET_tail_grd(reg32, val)                 REG_FLD_SET(e_tx_cal_cnst_FLD_tail_grd, (reg32), (val))
#define e_tx_cal_cnst_SET_default_ovrhd(reg32, val)            REG_FLD_SET(e_tx_cal_cnst_FLD_default_ovrhd, (reg32), (val))

#define e_txcal_cnst2_SET_ipgalign_mtd(reg32, val)             REG_FLD_SET(e_txcal_cnst2_FLD_ipgalign_mtd, (reg32), (val))
#define e_txcal_cnst2_SET_rpt_feccal_cnt_multival(reg32, val)  REG_FLD_SET(e_txcal_cnst2_FLD_rpt_feccal_cnt_multival, (reg32), (val))
#define e_txcal_cnst2_SET_u10g_dscv_gntlen(reg32, val)         REG_FLD_SET(e_txcal_cnst2_FLD_u10g_dscv_gntlen, (reg32), (val))
#define e_txcal_cnst2_SET_u10g_tail_grd(reg32, val)            REG_FLD_SET(e_txcal_cnst2_FLD_u10g_tail_grd, (reg32), (val))

#define e_txcal_cnst3_SET_u1g_fecon_min_gntlen(reg32, val)     REG_FLD_SET(e_txcal_cnst3_FLD_u1g_fecon_min_gntlen, (reg32), (val))
#define e_txcal_cnst3_SET_u10g_eoblen(reg32, val)              REG_FLD_SET(e_txcal_cnst3_FLD_u10g_eoblen, (reg32), (val))
#define e_txcal_cnst3_SET_u10g_min_gntlen(reg32, val)          REG_FLD_SET(e_txcal_cnst3_FLD_u10g_min_gntlen, (reg32), (val))
#define e_txcal_cnst3_SET_u1g_fecoff_min_gntlen(reg32, val)    REG_FLD_SET(e_txcal_cnst3_FLD_u1g_fecoff_min_gntlen, (reg32), (val))

#define e_txsch_cfg_SET_txfifo_wr_thren(reg32, val)            REG_FLD_SET(e_txsch_cfg_FLD_txfifo_wr_thren, (reg32), (val))
#define e_txsch_cfg_SET_txfifo_wr_thr(reg32, val)              REG_FLD_SET(e_txsch_cfg_FLD_txfifo_wr_thr, (reg32), (val))
#define e_txsch_cfg_SET_txsch_dmy0(reg32, val)                 REG_FLD_SET(e_txsch_cfg_FLD_txsch_dmy0, (reg32), (val))
#define e_txsch_cfg_SET_txfifo_pad_hthr(reg32, val)            REG_FLD_SET(e_txsch_cfg_FLD_txfifo_pad_hthr, (reg32), (val))
#define e_txsch_cfg_SET_txsch_dmy1(reg32, val)                 REG_FLD_SET(e_txsch_cfg_FLD_txsch_dmy1, (reg32), (val))
#define e_txsch_cfg_SET_txfifo_pad_lthr(reg32, val)            REG_FLD_SET(e_txsch_cfg_FLD_txfifo_pad_lthr, (reg32), (val))

#define e_rxfifo_thr_SET_rx_dmy1(reg32, val)                   REG_FLD_SET(e_rxfifo_thr_FLD_rx_dmy1, (reg32), (val))
#define e_rxfifo_thr_SET_rx_dmy0(reg32, val)                   REG_FLD_SET(e_rxfifo_thr_FLD_rx_dmy0, (reg32), (val))
#define e_rxfifo_thr_SET_rxfifo_mbithr(reg32, val)             REG_FLD_SET(e_rxfifo_thr_FLD_rxfifo_mbithr, (reg32), (val))

#define e_bcllid_cfg_SET_d10g_bcllid(reg32, val)               REG_FLD_SET(e_bcllid_cfg_FLD_d10g_bcllid, (reg32), (val))
#define e_bcllid_cfg_SET_d1g_bcllid(reg32, val)                REG_FLD_SET(e_bcllid_cfg_FLD_d1g_bcllid, (reg32), (val))

#define e_txfrm_cfg1_SET_tx_mpcp_addrl(reg32, val)             REG_FLD_SET(e_txfrm_cfg1_FLD_tx_mpcp_addrl, (reg32), (val))

#define e_txfrm_cfg2_SET_tx_mpcp_addrh(reg32, val)             REG_FLD_SET(e_txfrm_cfg2_FLD_tx_mpcp_addrh, (reg32), (val))
#define e_txfrm_cfg2_SET_tx_mpcp_etype(reg32, val)             REG_FLD_SET(e_txfrm_cfg2_FLD_tx_mpcp_etype, (reg32), (val))

#define e_txfrm_cfg3_SET_tx_rgreq_op(reg32, val)               REG_FLD_SET(e_txfrm_cfg3_FLD_tx_rgreq_op, (reg32), (val))
#define e_txfrm_cfg3_SET_tx_rgack_op(reg32, val)               REG_FLD_SET(e_txfrm_cfg3_FLD_tx_rgack_op, (reg32), (val))

#define e_tod_sync_x_SET_tod_sync_x(reg32, val)                REG_FLD_SET(e_tod_sync_x_FLD_tod_sync_x, (reg32), (val))

#define e_tod_ltncy_SET_rl_rx_phydly_ofst(reg32, val)          REG_FLD_SET(e_tod_ltncy_FLD_rl_rx_phydly_ofst, (reg32), (val))
#define e_tod_ltncy_SET_ingrs_latency(reg32, val)              REG_FLD_SET(e_tod_ltncy_FLD_ingrs_latency, (reg32), (val))
#define e_tod_ltncy_SET_egrs_latency(reg32, val)               REG_FLD_SET(e_tod_ltncy_FLD_egrs_latency, (reg32), (val))

#define e_new_tod_p2p_offset_sec_l32_SET_new_tod_p2p_offset_sec_l32(reg32, val) REG_FLD_SET(e_new_tod_p2p_offset_sec_l32_FLD_new_tod_p2p_offset_sec_l32, (reg32), (val))

#define e_new_tod_p2p_tod_offset_nsec_SET_new_tod_nsec(reg32, val) REG_FLD_SET(e_new_tod_p2p_tod_offset_nsec_FLD_new_tod_nsec, (reg32), (val))

#define e_tod_p2p_tod_sec_l32_SET_tod_p2p_sec_l32(reg32, val)  REG_FLD_SET(e_tod_p2p_tod_sec_l32_FLD_tod_p2p_sec_l32, (reg32), (val))

#define e_tod_p2p_tod_nsec_SET_tod_p2p_nsec(reg32, val)        REG_FLD_SET(e_tod_p2p_tod_nsec_FLD_tod_p2p_nsec, (reg32), (val))

#define e_tod_period_SET_tod_period(reg32, val)                REG_FLD_SET(e_tod_period_FLD_tod_period, (reg32), (val))

#define e_tod_1pps_ctrl_SET_tod_1pps_width_ctrl(reg32, val)    REG_FLD_SET(e_tod_1pps_ctrl_FLD_tod_1pps_width_ctrl, (reg32), (val))

#define e_pwr_sv_cfg_SET_pwd_mode(reg32, val)                  REG_FLD_SET(e_pwr_sv_cfg_FLD_pwd_mode, (reg32), (val))
#define e_pwr_sv_cfg_SET_timedrift_ignore(reg32, val)          REG_FLD_SET(e_pwr_sv_cfg_FLD_timedrift_ignore, (reg32), (val))
#define e_pwr_sv_cfg_SET_onu_wakeup(reg32, val)                REG_FLD_SET(e_pwr_sv_cfg_FLD_onu_wakeup, (reg32), (val))
#define e_pwr_sv_cfg_SET_pwr_sv_start(reg32, val)              REG_FLD_SET(e_pwr_sv_cfg_FLD_pwr_sv_start, (reg32), (val))
#define e_pwr_sv_cfg_SET_pwd_mode_i(reg32, val)                REG_FLD_SET(e_pwr_sv_cfg_FLD_pwd_mode_i, (reg32), (val))
#define e_pwr_sv_cfg_SET_slp_duration_max_h(reg32, val)        REG_FLD_SET(e_pwr_sv_cfg_FLD_slp_duration_max_h, (reg32), (val))

#define e_slp_durt_max_SET_slp_duration_max_l(reg32, val)      REG_FLD_SET(e_slp_durt_max_FLD_slp_duration_max_l, (reg32), (val))

#define e_slp_duration_SET_slp_duration(reg32, val)            REG_FLD_SET(e_slp_duration_FLD_slp_duration, (reg32), (val))

#define e_act_duration_SET_act_duration(reg32, val)            REG_FLD_SET(e_act_duration_FLD_act_duration, (reg32), (val))

#define e_pwron_dly_SET_pwron_dly(reg32, val)                  REG_FLD_SET(e_pwron_dly_FLD_pwron_dly, (reg32), (val))

#define e_slp_duration_i_SET_rx_slpalw_pwd_mode(reg32, val)    REG_FLD_SET(e_slp_duration_i_FLD_rx_slpalw_pwd_mode, (reg32), (val))
#define e_slp_duration_i_SET_rx_slpalw_flag(reg32, val)        REG_FLD_SET(e_slp_duration_i_FLD_rx_slpalw_flag, (reg32), (val))
#define e_slp_duration_i_SET_rx_slpalw_duration(reg32, val)    REG_FLD_SET(e_slp_duration_i_FLD_rx_slpalw_duration, (reg32), (val))

#define e_oui_a_cfg_SET_oui_a_en(reg32, val)                   REG_FLD_SET(e_oui_a_cfg_FLD_oui_a_en, (reg32), (val))
#define e_oui_a_cfg_SET_oui_a_dmy(reg32, val)                  REG_FLD_SET(e_oui_a_cfg_FLD_oui_a_dmy, (reg32), (val))
#define e_oui_a_cfg_SET_oui_a_val(reg32, val)                  REG_FLD_SET(e_oui_a_cfg_FLD_oui_a_val, (reg32), (val))

#define e_oui_b_cfg_SET_oui_b_en(reg32, val)                   REG_FLD_SET(e_oui_b_cfg_FLD_oui_b_en, (reg32), (val))
#define e_oui_b_cfg_SET_oui_b_dmy(reg32, val)                  REG_FLD_SET(e_oui_b_cfg_FLD_oui_b_dmy, (reg32), (val))
#define e_oui_b_cfg_SET_oui_b_val(reg32, val)                  REG_FLD_SET(e_oui_b_cfg_FLD_oui_b_val, (reg32), (val))

#define e_oui_c_cfg_SET_oui_c_en(reg32, val)                   REG_FLD_SET(e_oui_c_cfg_FLD_oui_c_en, (reg32), (val))
#define e_oui_c_cfg_SET_oui_c_dmy(reg32, val)                  REG_FLD_SET(e_oui_c_cfg_FLD_oui_c_dmy, (reg32), (val))
#define e_oui_c_cfg_SET_oui_c_val(reg32, val)                  REG_FLD_SET(e_oui_c_cfg_FLD_oui_c_val, (reg32), (val))

#define e_dyinggsp_cfg_SET_hw_dygasp_en(reg32, val)            REG_FLD_SET(e_dyinggsp_cfg_FLD_hw_dygasp_en, (reg32), (val))
#define e_dyinggsp_cfg_SET_sw_init_dygasp(reg32, val)          REG_FLD_SET(e_dyinggsp_cfg_FLD_sw_init_dygasp, (reg32), (val))
#define e_dyinggsp_cfg_SET_dygasp_num_of_times(reg32, val)     REG_FLD_SET(e_dyinggsp_cfg_FLD_dygasp_num_of_times, (reg32), (val))

#define e_dyinggsp_w1_SET_dyinggsp_w1_len_type(reg32, val)     REG_FLD_SET(e_dyinggsp_w1_FLD_dyinggsp_w1_len_type, (reg32), (val))

#define e_dyinggsp_w2_SET_dyinggsp_w2(reg32, val)              REG_FLD_SET(e_dyinggsp_w2_FLD_dyinggsp_w2, (reg32), (val))

#define e_dyinggsp_w3_SET_dyinggsp_w3(reg32, val)              REG_FLD_SET(e_dyinggsp_w3_FLD_dyinggsp_w3, (reg32), (val))

#define e_dyinggsp_w4_SET_dyinggsp_w4(reg32, val)              REG_FLD_SET(e_dyinggsp_w4_FLD_dyinggsp_w4, (reg32), (val))

#define e_dyinggsp_w5_SET_dyinggsp_w5(reg32, val)              REG_FLD_SET(e_dyinggsp_w5_FLD_dyinggsp_w5, (reg32), (val))

#define e_dyinggsp_w6_SET_dyinggsp_w6(reg32, val)              REG_FLD_SET(e_dyinggsp_w6_FLD_dyinggsp_w6, (reg32), (val))

#define e_dyinggsp_w7_SET_dyinggsp_w7(reg32, val)              REG_FLD_SET(e_dyinggsp_w7_FLD_dyinggsp_w7, (reg32), (val))

#define e_dyinggsp_w8_SET_dyinggsp_w8(reg32, val)              REG_FLD_SET(e_dyinggsp_w8_FLD_dyinggsp_w8, (reg32), (val))

#define e_dyinggsp_w9_SET_dyinggsp_w9(reg32, val)              REG_FLD_SET(e_dyinggsp_w9_FLD_dyinggsp_w9, (reg32), (val))

#define e_dyinggsp_w10_SET_dyinggsp_w10(reg32, val)            REG_FLD_SET(e_dyinggsp_w10_FLD_dyinggsp_w10, (reg32), (val))

#define e_dyinggsp_w11_SET_dyinggsp_w11(reg32, val)            REG_FLD_SET(e_dyinggsp_w11_FLD_dyinggsp_w11, (reg32), (val))

#define e_dyinggsp_w12_SET_dyinggsp_w12(reg32, val)            REG_FLD_SET(e_dyinggsp_w12_FLD_dyinggsp_w12, (reg32), (val))

#define e_oam_kpalv_ctrl_SET_oam_kpalv_llid_idx(reg32, val)    REG_FLD_SET(e_oam_kpalv_ctrl_FLD_oam_kpalv_llid_idx, (reg32), (val))
#define e_oam_kpalv_ctrl_SET_oam_kpalv_interval(reg32, val)    REG_FLD_SET(e_oam_kpalv_ctrl_FLD_oam_kpalv_interval, (reg32), (val))
#define e_oam_kpalv_ctrl_SET_oam_kpalv_sw_trig(reg32, val)     REG_FLD_SET(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_trig, (reg32), (val))
#define e_oam_kpalv_ctrl_SET_oam_kpalv_sw_cfg(reg32, val)      REG_FLD_SET(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_cfg, (reg32), (val))
#define e_oam_kpalv_ctrl_SET_oam_kpalv_en(reg32, val)          REG_FLD_SET(e_oam_kpalv_ctrl_FLD_oam_kpalv_en, (reg32), (val))

#define e_oam_kpalv_w1_SET_oam_kpalv_w1(reg32, val)            REG_FLD_SET(e_oam_kpalv_w1_FLD_oam_kpalv_w1, (reg32), (val))

#define e_oam_kpalv_w2_SET_oam_kpalv_w2(reg32, val)            REG_FLD_SET(e_oam_kpalv_w2_FLD_oam_kpalv_w2, (reg32), (val))

#define e_oam_kpalv_w3_SET_oam_kpalv_w3(reg32, val)            REG_FLD_SET(e_oam_kpalv_w3_FLD_oam_kpalv_w3, (reg32), (val))

#define e_oam_kpalv_w4_SET_oam_kpalv_w4(reg32, val)            REG_FLD_SET(e_oam_kpalv_w4_FLD_oam_kpalv_w4, (reg32), (val))

#define e_oam_kpalv_w5_SET_oam_kpalv_w5(reg32, val)            REG_FLD_SET(e_oam_kpalv_w5_FLD_oam_kpalv_w5, (reg32), (val))

#define e_oam_kpalv_w6_SET_oam_kpalv_w6(reg32, val)            REG_FLD_SET(e_oam_kpalv_w6_FLD_oam_kpalv_w6, (reg32), (val))

#define e_oam_kpalv_w7_SET_oam_kpalv_w7(reg32, val)            REG_FLD_SET(e_oam_kpalv_w7_FLD_oam_kpalv_w7, (reg32), (val))

#define e_oam_kpalv_w8_SET_oam_kpalv_w8(reg32, val)            REG_FLD_SET(e_oam_kpalv_w8_FLD_oam_kpalv_w8, (reg32), (val))

#define e_oam_kpalv_w9_SET_oam_kpalv_w9(reg32, val)            REG_FLD_SET(e_oam_kpalv_w9_FLD_oam_kpalv_w9, (reg32), (val))

#define e_oam_kpalv_w10_SET_oam_kpalv_w10(reg32, val)          REG_FLD_SET(e_oam_kpalv_w10_FLD_oam_kpalv_w10, (reg32), (val))

#define e_oam_kpalv_w11_SET_oam_kpalv_w11(reg32, val)          REG_FLD_SET(e_oam_kpalv_w11_FLD_oam_kpalv_w11, (reg32), (val))

#define e_oam_kpalv_w12_SET_oam_kpalv_w12(reg32, val)          REG_FLD_SET(e_oam_kpalv_w12_FLD_oam_kpalv_w12, (reg32), (val))

#define e_kpalv_hwen_sts_SET_kpalv_hwen_sts(reg32, val)        REG_FLD_SET(e_kpalv_hwen_sts_FLD_kpalv_hwen_sts, (reg32), (val))

#define e_mcgrp_en_SET_mcgrp_llid_en(reg32, val)               REG_FLD_SET(e_mcgrp_en_FLD_mcgrp_llid_en, (reg32), (val))

#define e_rxuni_mcllid_cfg0_SET_mcgrp_llid1_val(reg32, val)    REG_FLD_SET(e_rxuni_mcllid_cfg0_FLD_mcgrp_llid1_val, (reg32), (val))
#define e_rxuni_mcllid_cfg0_SET_mcgrp_llid0_val(reg32, val)    REG_FLD_SET(e_rxuni_mcllid_cfg0_FLD_mcgrp_llid0_val, (reg32), (val))

#define e_rxuni_mcllid_cfg1_SET_mcgrp_llid3_val(reg32, val)    REG_FLD_SET(e_rxuni_mcllid_cfg1_FLD_mcgrp_llid3_val, (reg32), (val))
#define e_rxuni_mcllid_cfg1_SET_mcgrp_llid2_val(reg32, val)    REG_FLD_SET(e_rxuni_mcllid_cfg1_FLD_mcgrp_llid2_val, (reg32), (val))

#define e_rxuni_mcllid_cfg2_SET_mcgrp_llid5_val(reg32, val)    REG_FLD_SET(e_rxuni_mcllid_cfg2_FLD_mcgrp_llid5_val, (reg32), (val))
#define e_rxuni_mcllid_cfg2_SET_mcgrp_llid4_val(reg32, val)    REG_FLD_SET(e_rxuni_mcllid_cfg2_FLD_mcgrp_llid4_val, (reg32), (val))

#define e_rxuni_mcllid_cfg3_SET_mcgrp_llid7_val(reg32, val)    REG_FLD_SET(e_rxuni_mcllid_cfg3_FLD_mcgrp_llid7_val, (reg32), (val))
#define e_rxuni_mcllid_cfg3_SET_mcgrp_llid6_val(reg32, val)    REG_FLD_SET(e_rxuni_mcllid_cfg3_FLD_mcgrp_llid6_val, (reg32), (val))

#define e_rxuni_mcllid_cfg4_SET_mcgrp_llid9_val(reg32, val)    REG_FLD_SET(e_rxuni_mcllid_cfg4_FLD_mcgrp_llid9_val, (reg32), (val))
#define e_rxuni_mcllid_cfg4_SET_mcgrp_llid8_val(reg32, val)    REG_FLD_SET(e_rxuni_mcllid_cfg4_FLD_mcgrp_llid8_val, (reg32), (val))

#define e_rxuni_mcllid_cfg5_SET_mcgrp_llid11_val(reg32, val)   REG_FLD_SET(e_rxuni_mcllid_cfg5_FLD_mcgrp_llid11_val, (reg32), (val))
#define e_rxuni_mcllid_cfg5_SET_mcgrp_llid10_val(reg32, val)   REG_FLD_SET(e_rxuni_mcllid_cfg5_FLD_mcgrp_llid10_val, (reg32), (val))

#define e_rxuni_mcllid_cfg6_SET_mcgrp_llid13_val(reg32, val)   REG_FLD_SET(e_rxuni_mcllid_cfg6_FLD_mcgrp_llid13_val, (reg32), (val))
#define e_rxuni_mcllid_cfg6_SET_mcgrp_llid12_val(reg32, val)   REG_FLD_SET(e_rxuni_mcllid_cfg6_FLD_mcgrp_llid12_val, (reg32), (val))

#define e_rxuni_mcllid_cfg7_SET_mcgrp_llid15_val(reg32, val)   REG_FLD_SET(e_rxuni_mcllid_cfg7_FLD_mcgrp_llid15_val, (reg32), (val))
#define e_rxuni_mcllid_cfg7_SET_mcgrp_llid14_val(reg32, val)   REG_FLD_SET(e_rxuni_mcllid_cfg7_FLD_mcgrp_llid14_val, (reg32), (val))

#define e_crpt_cfg_SET_crpt_dmy0(reg32, val)                   REG_FLD_SET(e_crpt_cfg_FLD_crpt_dmy0, (reg32), (val))
#define e_crpt_cfg_SET_kpalv_encrpt_en(reg32, val)             REG_FLD_SET(e_crpt_cfg_FLD_kpalv_encrpt_en, (reg32), (val))
#define e_crpt_cfg_SET_mpcp_encrpt_en(reg32, val)              REG_FLD_SET(e_crpt_cfg_FLD_mpcp_encrpt_en, (reg32), (val))
#define e_crpt_cfg_SET_dygasp_encrpt_en(reg32, val)            REG_FLD_SET(e_crpt_cfg_FLD_dygasp_encrpt_en, (reg32), (val))
#define e_crpt_cfg_SET_llidalluc_decrpt_en(reg32, val)         REG_FLD_SET(e_crpt_cfg_FLD_llidalluc_decrpt_en, (reg32), (val))
#define e_crpt_cfg_SET_llidmc_decrpt_en(reg32, val)            REG_FLD_SET(e_crpt_cfg_FLD_llidmc_decrpt_en, (reg32), (val))
#define e_crpt_cfg_SET_llidbc_decrpt_en(reg32, val)            REG_FLD_SET(e_crpt_cfg_FLD_llidbc_decrpt_en, (reg32), (val))
#define e_crpt_cfg_SET_mcgrp_decrpt_en(reg32, val)             REG_FLD_SET(e_crpt_cfg_FLD_mcgrp_decrpt_en, (reg32), (val))
#define e_crpt_cfg_SET_crpt_dmy1(reg32, val)                   REG_FLD_SET(e_crpt_cfg_FLD_crpt_dmy1, (reg32), (val))
#define e_crpt_cfg_SET_encrpt_mode(reg32, val)                 REG_FLD_SET(e_crpt_cfg_FLD_encrpt_mode, (reg32), (val))
#define e_crpt_cfg_SET_crcchk_mode(reg32, val)                 REG_FLD_SET(e_crpt_cfg_FLD_crcchk_mode, (reg32), (val))
#define e_crpt_cfg_SET_decrpt_mode(reg32, val)                 REG_FLD_SET(e_crpt_cfg_FLD_decrpt_mode, (reg32), (val))

#define e_cfbsec_cfg_SET_cfbsec_dmy(reg32, val)                REG_FLD_SET(e_cfbsec_cfg_FLD_cfbsec_dmy, (reg32), (val))
#define e_cfbsec_cfg_SET_cfbdec_iv_srcsel(reg32, val)          REG_FLD_SET(e_cfbsec_cfg_FLD_cfbdec_iv_srcsel, (reg32), (val))
#define e_cfbsec_cfg_SET_cfbdec_padd_mode(reg32, val)          REG_FLD_SET(e_cfbsec_cfg_FLD_cfbdec_padd_mode, (reg32), (val))

#define e_ctrsec_cfg_SET_ctrsec_dmy0(reg32, val)               REG_FLD_SET(e_ctrsec_cfg_FLD_ctrsec_dmy0, (reg32), (val))
#define e_ctrsec_cfg_SET_ctrenc_padd_mode(reg32, val)          REG_FLD_SET(e_ctrsec_cfg_FLD_ctrenc_padd_mode, (reg32), (val))
#define e_ctrsec_cfg_SET_ctrsec_dmy1(reg32, val)               REG_FLD_SET(e_ctrsec_cfg_FLD_ctrsec_dmy1, (reg32), (val))
#define e_ctrsec_cfg_SET_ctrdec_srcaddr_sel(reg32, val)        REG_FLD_SET(e_ctrsec_cfg_FLD_ctrdec_srcaddr_sel, (reg32), (val))
#define e_ctrsec_cfg_SET_ctrdec_padd_mode(reg32, val)          REG_FLD_SET(e_ctrsec_cfg_FLD_ctrdec_padd_mode, (reg32), (val))

#define e_desec_srcaddr_cfg_SET_desec_srcaddr_rwcmd(reg32, val) REG_FLD_SET(e_desec_srcaddr_cfg_FLD_desec_srcaddr_rwcmd, (reg32), (val))
#define e_desec_srcaddr_cfg_SET_desec_srcaddr_rwcmd_done(reg32, val) REG_FLD_SET(e_desec_srcaddr_cfg_FLD_desec_srcaddr_rwcmd_done, (reg32), (val))
#define e_desec_srcaddr_cfg_SET_desec_srcaddr_llid_idx(reg32, val) REG_FLD_SET(e_desec_srcaddr_cfg_FLD_desec_srcaddr_llid_idx, (reg32), (val))
#define e_desec_srcaddr_cfg_SET_desec_srcaddr_value_h(reg32, val) REG_FLD_SET(e_desec_srcaddr_cfg_FLD_desec_srcaddr_value_h, (reg32), (val))

#define e_desec_srcaddr_val_SET_desec_srcaddr_value_l(reg32, val) REG_FLD_SET(e_desec_srcaddr_val_FLD_desec_srcaddr_value_l, (reg32), (val))

#define e_macsdec_cfg_SET_macsdec_dmy0(reg32, val)             REG_FLD_SET(e_macsdec_cfg_FLD_macsdec_dmy0, (reg32), (val))
#define e_macsdec_cfg_SET_macsdec_tag_bit_swap(reg32, val)     REG_FLD_SET(e_macsdec_cfg_FLD_macsdec_tag_bit_swap, (reg32), (val))
#define e_macsdec_cfg_SET_macsdec_iv_bit_swap(reg32, val)      REG_FLD_SET(e_macsdec_cfg_FLD_macsdec_iv_bit_swap, (reg32), (val))
#define e_macsdec_cfg_SET_macsdec_aad_bit_swap(reg32, val)     REG_FLD_SET(e_macsdec_cfg_FLD_macsdec_aad_bit_swap, (reg32), (val))
#define e_macsdec_cfg_SET_macsdec_pt_bit_swap(reg32, val)      REG_FLD_SET(e_macsdec_cfg_FLD_macsdec_pt_bit_swap, (reg32), (val))
#define e_macsdec_cfg_SET_macsdec_ct_bit_swap(reg32, val)      REG_FLD_SET(e_macsdec_cfg_FLD_macsdec_ct_bit_swap, (reg32), (val))
#define e_macsdec_cfg_SET_macsdec_srcaddr_sel(reg32, val)      REG_FLD_SET(e_macsdec_cfg_FLD_macsdec_srcaddr_sel, (reg32), (val))
#define e_macsdec_cfg_SET_macsdec_timout_en(reg32, val)        REG_FLD_SET(e_macsdec_cfg_FLD_macsdec_timout_en, (reg32), (val))
#define e_macsdec_cfg_SET_macsdec_conofst(reg32, val)          REG_FLD_SET(e_macsdec_cfg_FLD_macsdec_conofst, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e0c0_desec(reg32, val)        REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e0c0_desec, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e0c0_icvchk(reg32, val)       REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e0c0_icvchk, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e0c0_drop(reg32, val)         REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e0c0_drop, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e0c1_desec(reg32, val)        REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e0c1_desec, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e0c1_icvchk(reg32, val)       REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e0c1_icvchk, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e0c1_drop(reg32, val)         REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e0c1_drop, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e1c0_desec(reg32, val)        REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e1c0_desec, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e1c0_icvchk(reg32, val)       REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e1c0_icvchk, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e1c0_drop(reg32, val)         REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e1c0_drop, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e1c1_desec(reg32, val)        REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e1c1_desec, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e1c1_icvchk(reg32, val)       REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e1c1_icvchk, (reg32), (val))
#define e_macsdec_cfg_SET_sectag_e1c1_drop(reg32, val)         REG_FLD_SET(e_macsdec_cfg_FLD_sectag_e1c1_drop, (reg32), (val))

#define e_macsenc_cfg_SET_macsenc_mode(reg32, val)             REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_mode, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_dmy0(reg32, val)             REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_dmy0, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_tag_bit_swap(reg32, val)     REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_tag_bit_swap, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_iv_bit_swap(reg32, val)      REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_iv_bit_swap, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_aad_bit_swap(reg32, val)     REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_aad_bit_swap, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_pt_bit_swap(reg32, val)      REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_pt_bit_swap, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_ct_bit_swap(reg32, val)      REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_ct_bit_swap, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_srcaddr_sel(reg32, val)      REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_srcaddr_sel, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_timout_en(reg32, val)        REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_timout_en, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_dmy1(reg32, val)             REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_dmy1, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_tag_v(reg32, val)            REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_tag_v, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_tag_es(reg32, val)           REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_tag_es, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_tag_sc(reg32, val)           REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_tag_sc, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_tag_scb(reg32, val)          REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_tag_scb, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_tag_e(reg32, val)            REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_tag_e, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_tag_c(reg32, val)            REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_tag_c, (reg32), (val))
#define e_macsenc_cfg_SET_macsenc_dmy2(reg32, val)             REG_FLD_SET(e_macsenc_cfg_FLD_macsenc_dmy2, (reg32), (val))

#define e_macsenc_pnini_cfg_SET_macsenc_pn_inival(reg32, val)  REG_FLD_SET(e_macsenc_pnini_cfg_FLD_macsenc_pn_inival, (reg32), (val))

#define e_macsenc_pn_cfg_SET_macsenc_pn_rwcmd(reg32, val)      REG_FLD_SET(e_macsenc_pn_cfg_FLD_macsenc_pn_rwcmd, (reg32), (val))
#define e_macsenc_pn_cfg_SET_macsenc_pn_rwcmd_done(reg32, val) REG_FLD_SET(e_macsenc_pn_cfg_FLD_macsenc_pn_rwcmd_done, (reg32), (val))
#define e_macsenc_pn_cfg_SET_macsenc_pn_llid_idx(reg32, val)   REG_FLD_SET(e_macsenc_pn_cfg_FLD_macsenc_pn_llid_idx, (reg32), (val))

#define e_macsenc_pn_val_SET_macsenc_pn_val(reg32, val)        REG_FLD_SET(e_macsenc_pn_val_FLD_macsenc_pn_val, (reg32), (val))

#define e_olt_macaddr_h_SET_olt_macaddr_h(reg32, val)          REG_FLD_SET(e_olt_macaddr_h_FLD_olt_macaddr_h, (reg32), (val))

#define e_olt_macaddr_l_SET_olt_macaddr_l(reg32, val)          REG_FLD_SET(e_olt_macaddr_l_FLD_olt_macaddr_l, (reg32), (val))

#define e_snf_cfg_SET_sniffer_mode(reg32, val)                 REG_FLD_SET(e_snf_cfg_FLD_sniffer_mode, (reg32), (val))
#define e_snf_cfg_SET_snf_rsv0(reg32, val)                     REG_FLD_SET(e_snf_cfg_FLD_snf_rsv0, (reg32), (val))
#define e_snf_cfg_SET_llidinfo_snf(reg32, val)                 REG_FLD_SET(e_snf_cfg_FLD_llidinfo_snf, (reg32), (val))
#define e_snf_cfg_SET_snf_fcserr_fwd(reg32, val)               REG_FLD_SET(e_snf_cfg_FLD_snf_fcserr_fwd, (reg32), (val))
#define e_snf_cfg_SET_snf_rsv1(reg32, val)                     REG_FLD_SET(e_snf_cfg_FLD_snf_rsv1, (reg32), (val))
#define e_snf_cfg_SET_us_snf_mode(reg32, val)                  REG_FLD_SET(e_snf_cfg_FLD_us_snf_mode, (reg32), (val))
#define e_snf_cfg_SET_us_kpalv_snf(reg32, val)                 REG_FLD_SET(e_snf_cfg_FLD_us_kpalv_snf, (reg32), (val))
#define e_snf_cfg_SET_us_dygasp_snf(reg32, val)                REG_FLD_SET(e_snf_cfg_FLD_us_dygasp_snf, (reg32), (val))
#define e_snf_cfg_SET_us_rpt_snf(reg32, val)                   REG_FLD_SET(e_snf_cfg_FLD_us_rpt_snf, (reg32), (val))
#define e_snf_cfg_SET_us_mpcp_snf(reg32, val)                  REG_FLD_SET(e_snf_cfg_FLD_us_mpcp_snf, (reg32), (val))
#define e_snf_cfg_SET_us_oam_snf(reg32, val)                   REG_FLD_SET(e_snf_cfg_FLD_us_oam_snf, (reg32), (val))
#define e_snf_cfg_SET_snf_rsv2(reg32, val)                     REG_FLD_SET(e_snf_cfg_FLD_snf_rsv2, (reg32), (val))
#define e_snf_cfg_SET_mcgrp_eth_snf(reg32, val)                REG_FLD_SET(e_snf_cfg_FLD_mcgrp_eth_snf, (reg32), (val))
#define e_snf_cfg_SET_mcgrp_nrlgate_snf(reg32, val)            REG_FLD_SET(e_snf_cfg_FLD_mcgrp_nrlgate_snf, (reg32), (val))
#define e_snf_cfg_SET_mcgrp_oam_snf(reg32, val)                REG_FLD_SET(e_snf_cfg_FLD_mcgrp_oam_snf, (reg32), (val))
#define e_snf_cfg_SET_mcgrp_llid_snf(reg32, val)               REG_FLD_SET(e_snf_cfg_FLD_mcgrp_llid_snf, (reg32), (val))
#define e_snf_cfg_SET_alluc_eth_snf(reg32, val)                REG_FLD_SET(e_snf_cfg_FLD_alluc_eth_snf, (reg32), (val))
#define e_snf_cfg_SET_alluc_nrlgate_snf(reg32, val)            REG_FLD_SET(e_snf_cfg_FLD_alluc_nrlgate_snf, (reg32), (val))
#define e_snf_cfg_SET_alluc_oam_snf(reg32, val)                REG_FLD_SET(e_snf_cfg_FLD_alluc_oam_snf, (reg32), (val))
#define e_snf_cfg_SET_alluc_llid_snf(reg32, val)               REG_FLD_SET(e_snf_cfg_FLD_alluc_llid_snf, (reg32), (val))
#define e_snf_cfg_SET_snf_rsv3(reg32, val)                     REG_FLD_SET(e_snf_cfg_FLD_snf_rsv3, (reg32), (val))
#define e_snf_cfg_SET_ds_nrlgate_snf(reg32, val)               REG_FLD_SET(e_snf_cfg_FLD_ds_nrlgate_snf, (reg32), (val))
#define e_snf_cfg_SET_ds_mpcp_snf(reg32, val)                  REG_FLD_SET(e_snf_cfg_FLD_ds_mpcp_snf, (reg32), (val))

#define e_snf_sp_tag_SET_snf_sp_tag(reg32, val)                REG_FLD_SET(e_snf_sp_tag_FLD_snf_sp_tag, (reg32), (val))

#define e_snf_dah_SET_snf_pkt_dah(reg32, val)                  REG_FLD_SET(e_snf_dah_FLD_snf_pkt_dah, (reg32), (val))

#define e_snf_dal_SET_snf_pkt_dal(reg32, val)                  REG_FLD_SET(e_snf_dal_FLD_snf_pkt_dal, (reg32), (val))
#define e_snf_dal_SET_snf_pkt_sah(reg32, val)                  REG_FLD_SET(e_snf_dal_FLD_snf_pkt_sah, (reg32), (val))

#define e_snf_sal_SET_snf_pkt_sal(reg32, val)                  REG_FLD_SET(e_snf_sal_FLD_snf_pkt_sal, (reg32), (val))

#define e_snf_etype_SET_snf_pkt_etype(reg32, val)              REG_FLD_SET(e_snf_etype_FLD_snf_pkt_etype, (reg32), (val))

#define e_rdmdly_cfg_SET_rdmdly_mode(reg32, val)               REG_FLD_SET(e_rdmdly_cfg_FLD_rdmdly_mode, (reg32), (val))
#define e_rdmdly_cfg_SET_rdmdly_lthr(reg32, val)               REG_FLD_SET(e_rdmdly_cfg_FLD_rdmdly_lthr, (reg32), (val))
#define e_rdmdly_cfg_SET_rdmdly_hthr(reg32, val)               REG_FLD_SET(e_rdmdly_cfg_FLD_rdmdly_hthr, (reg32), (val))

#define e_rdmdly_stat_SET_dscvgate_gntlen(reg32, val)          REG_FLD_SET(e_rdmdly_stat_FLD_dscvgate_gntlen, (reg32), (val))
#define e_rdmdly_stat_SET_rdmdly(reg32, val)                   REG_FLD_SET(e_rdmdly_stat_FLD_rdmdly, (reg32), (val))

#define e_dbg_prb_sel_SET_probe_dmysel(reg32, val)             REG_FLD_SET(e_dbg_prb_sel_FLD_probe_dmysel, (reg32), (val))
#define e_dbg_prb_sel_SET_probe_dtgrp_sel(reg32, val)          REG_FLD_SET(e_dbg_prb_sel_FLD_probe_dtgrp_sel, (reg32), (val))
#define e_dbg_prb_sel_SET_probe_bit0_sel(reg32, val)           REG_FLD_SET(e_dbg_prb_sel_FLD_probe_bit0_sel, (reg32), (val))
#define e_dbg_prb_sel_SET_probe_sel(reg32, val)                REG_FLD_SET(e_dbg_prb_sel_FLD_probe_sel, (reg32), (val))

#define e_dbg_prb_h32_SET_probe_h32(reg32, val)                REG_FLD_SET(e_dbg_prb_h32_FLD_probe_h32, (reg32), (val))

#define e_dbg_prb_l32_SET_probe_l32(reg32, val)                REG_FLD_SET(e_dbg_prb_l32_FLD_probe_l32, (reg32), (val))

#define e_utili_cfg_SET_cf_utili_wdm(reg32, val)               REG_FLD_SET(e_utili_cfg_FLD_cf_utili_wdm, (reg32), (val))
#define e_utili_cfg_SET_cf_utili_pktovhd(reg32, val)           REG_FLD_SET(e_utili_cfg_FLD_cf_utili_pktovhd, (reg32), (val))
#define e_utili_cfg_SET_cf_utili_rsv(reg32, val)               REG_FLD_SET(e_utili_cfg_FLD_cf_utili_rsv, (reg32), (val))
#define e_utili_cfg_SET_cf_utili_pkttype(reg32, val)           REG_FLD_SET(e_utili_cfg_FLD_cf_utili_pkttype, (reg32), (val))
#define e_utili_cfg_SET_cf_utili_mode(reg32, val)              REG_FLD_SET(e_utili_cfg_FLD_cf_utili_mode, (reg32), (val))

#define e_utili_check_SET_cf_utilization_cntclr(reg32, val)    REG_FLD_SET(e_utili_check_FLD_cf_utilization_cntclr, (reg32), (val))
#define e_utili_check_SET_cf_utilization_chken(reg32, val)     REG_FLD_SET(e_utili_check_FLD_cf_utilization_chken, (reg32), (val))

#define total_gnt_sizeh_SET_total_gnt_size_h32(reg32, val)     REG_FLD_SET(total_gnt_sizeh_FLD_total_gnt_size_h32, (reg32), (val))

#define total_gnt_sizel_SET_total_gnt_size_l32(reg32, val)     REG_FLD_SET(total_gnt_sizel_FLD_total_gnt_size_l32, (reg32), (val))

#define total_pkt_cnt_SET_total_pkt_cnt(reg32, val)            REG_FLD_SET(total_pkt_cnt_FLD_total_pkt_cnt, (reg32), (val))

#define total_pkt_size_l_SET_total_pkt_size_l32(reg32, val)    REG_FLD_SET(total_pkt_size_l_FLD_total_pkt_size_l32, (reg32), (val))

#define total_pkt_size_h_SET_total_pkt_size_h32(reg32, val)    REG_FLD_SET(total_pkt_size_h_FLD_total_pkt_size_h32, (reg32), (val))

#define total_u10g_nouse_gntlen_SET_total_u10g_nouse_gntlen(reg32, val) REG_FLD_SET(total_u10g_nouse_gntlen_FLD_total_u10g_nouse_gntlen, (reg32), (val))

#define total_gnt_cnt_SET_total_gnt_cnt(reg32, val)            REG_FLD_SET(total_gnt_cnt_FLD_total_gnt_cnt, (reg32), (val))

#define total_tx_cyc_h_SET_total_tx_cyc_h32(reg32, val)        REG_FLD_SET(total_tx_cyc_h_FLD_total_tx_cyc_h32, (reg32), (val))

#define total_tx_cyc_l_SET_total_tx_cyc_l32(reg32, val)        REG_FLD_SET(total_tx_cyc_l_FLD_total_tx_cyc_l32, (reg32), (val))

#define e_sts_dmy6_SET_xe_sts_dmy6(reg32, val)                 REG_FLD_SET(e_sts_dmy6_FLD_xe_sts_dmy6, (reg32), (val))

#define e_sts_dmy7_SET_xe_sts_dmy7(reg32, val)                 REG_FLD_SET(e_sts_dmy7_FLD_xe_sts_dmy7, (reg32), (val))

#define e_sts_dmy8_SET_xe_sts_dmy8(reg32, val)                 REG_FLD_SET(e_sts_dmy8_FLD_xe_sts_dmy8, (reg32), (val))

#define e_sts_dmy9_SET_xe_sts_dmy9(reg32, val)                 REG_FLD_SET(e_sts_dmy9_FLD_xe_sts_dmy9, (reg32), (val))

#define e_sts_dmy10_SET_xe_sts_dmy10(reg32, val)               REG_FLD_SET(e_sts_dmy10_FLD_xe_sts_dmy10, (reg32), (val))

#define e_sts_dmy26_SET_xe_sts_dmy26(reg32, val)               REG_FLD_SET(e_sts_dmy26_FLD_xe_sts_dmy26, (reg32), (val))

#define e_sts_dmy27_SET_xe_sts_dmy27(reg32, val)               REG_FLD_SET(e_sts_dmy27_FLD_xe_sts_dmy27, (reg32), (val))

#define e_sts_dmy28_SET_xe_sts_dmy28(reg32, val)               REG_FLD_SET(e_sts_dmy28_FLD_xe_sts_dmy28, (reg32), (val))

#define e_sts_dmy29_SET_xe_sts_dmy29(reg32, val)               REG_FLD_SET(e_sts_dmy29_FLD_xe_sts_dmy29, (reg32), (val))

#define e_sram_pd_SET_xemac_sram_pd(reg32, val)                REG_FLD_SET(e_sram_pd_FLD_xemac_sram_pd, (reg32), (val))

#define e_mbist_delsel_cfg0_SET_xemac_mbist_delsel_cfg0(reg32, val) REG_FLD_SET(e_mbist_delsel_cfg0_FLD_xemac_mbist_delsel_cfg0, (reg32), (val))

#define e_mbist_delsel_cfg1_SET_xemac_mbist_delsel_cfg1(reg32, val) REG_FLD_SET(e_mbist_delsel_cfg1_FLD_xemac_mbist_delsel_cfg1, (reg32), (val))

#define e_mbist_delsel_cfg2_SET_xemac_mbist_delsel_cfg2(reg32, val) REG_FLD_SET(e_mbist_delsel_cfg2_FLD_xemac_mbist_delsel_cfg2, (reg32), (val))

#define e_mbist_delsel_cfg3_SET_xemac_mbist_delsel_cfg3(reg32, val) REG_FLD_SET(e_mbist_delsel_cfg3_FLD_xemac_mbist_delsel_cfg3, (reg32), (val))

#define e_mbist_done_sts0_SET_xemac_mbist_done_sts0(reg32, val) REG_FLD_SET(e_mbist_done_sts0_FLD_xemac_mbist_done_sts0, (reg32), (val))

#define e_mbist_fail_sts0_SET_xemac_mbist_fail_sts0(reg32, val) REG_FLD_SET(e_mbist_fail_sts0_FLD_xemac_mbist_fail_sts0, (reg32), (val))

#define e_mbist_fail_sts1_SET_xemac_mbist_fail_sts1(reg32, val) REG_FLD_SET(e_mbist_fail_sts1_FLD_xemac_mbist_fail_sts1, (reg32), (val))

#define rx_sld_sts_SET_sld_b7(reg32, val)                      REG_FLD_SET(rx_sld_sts_FLD_sld_b7, (reg32), (val))
#define rx_sld_sts_SET_sld_b6(reg32, val)                      REG_FLD_SET(rx_sld_sts_FLD_sld_b6, (reg32), (val))
#define rx_sld_sts_SET_sld_b5(reg32, val)                      REG_FLD_SET(rx_sld_sts_FLD_sld_b5, (reg32), (val))
#define rx_sld_sts_SET_sld_b4(reg32, val)                      REG_FLD_SET(rx_sld_sts_FLD_sld_b4, (reg32), (val))
#define rx_sld_sts_SET_sld_b3(reg32, val)                      REG_FLD_SET(rx_sld_sts_FLD_sld_b3, (reg32), (val))
#define rx_sld_sts_SET_sld_b2(reg32, val)                      REG_FLD_SET(rx_sld_sts_FLD_sld_b2, (reg32), (val))
#define rx_sld_sts_SET_sld_b1(reg32, val)                      REG_FLD_SET(rx_sld_sts_FLD_sld_b1, (reg32), (val))
#define rx_sld_sts_SET_sld_b0(reg32, val)                      REG_FLD_SET(rx_sld_sts_FLD_sld_b0, (reg32), (val))

#define e_glue_cfg_SET_txmpi_fifound_pktgate_gntcnt(reg32, val) REG_FLD_SET(e_glue_cfg_FLD_txmpi_fifound_pktgate_gntcnt, (reg32), (val))
#define e_glue_cfg_SET_txmpi_fifound_pktgate_en(reg32, val)    REG_FLD_SET(e_glue_cfg_FLD_txmpi_fifound_pktgate_en, (reg32), (val))
#define e_glue_cfg_SET_txmpi_fifound_thr(reg32, val)           REG_FLD_SET(e_glue_cfg_FLD_txmpi_fifound_thr, (reg32), (val))

#define e_gntreq_tmout_SET_gntreq_grden(reg32, val)            REG_FLD_SET(e_gntreq_tmout_FLD_gntreq_grden, (reg32), (val))
#define e_gntreq_tmout_SET_gntreq_grdcyc(reg32, val)           REG_FLD_SET(e_gntreq_tmout_FLD_gntreq_grdcyc, (reg32), (val))

#define e_fpga_gendef_SET_fpga_gendef(reg32, val)              REG_FLD_SET(e_fpga_gendef_FLD_fpga_gendef, (reg32), (val))

#define e_fpga_genver_SET_fpga_genver(reg32, val)              REG_FLD_SET(e_fpga_genver_FLD_fpga_genver, (reg32), (val))

#define e_cnt_clr_SET_glb_cntclr(reg32, val)                   REG_FLD_SET(e_cnt_clr_FLD_glb_cntclr, (reg32), (val))

#define e_rxmbi_eth_cnt_SET_rxmbi_eth_cnt(reg32, val)          REG_FLD_SET(e_rxmbi_eth_cnt_FLD_rxmbi_eth_cnt, (reg32), (val))

#define e_rxmpi_eth_cnt_SET_rxmpi_eth_cnt(reg32, val)          REG_FLD_SET(e_rxmpi_eth_cnt_FLD_rxmpi_eth_cnt, (reg32), (val))

#define e_txmbi_eth_cnt_SET_txmbi_eth_cnt(reg32, val)          REG_FLD_SET(e_txmbi_eth_cnt_FLD_txmbi_eth_cnt, (reg32), (val))

#define e_txmpi_eth_cnt_SET_txmpi_eth_cnt(reg32, val)          REG_FLD_SET(e_txmpi_eth_cnt_FLD_txmpi_eth_cnt, (reg32), (val))

#define e_oam_stat_SET_rx_oam_cnt(reg32, val)                  REG_FLD_SET(e_oam_stat_FLD_rx_oam_cnt, (reg32), (val))
#define e_oam_stat_SET_tx_oam_cnt(reg32, val)                  REG_FLD_SET(e_oam_stat_FLD_tx_oam_cnt, (reg32), (val))

#define e_mpcp_stat_SET_mpcp_err_cnt(reg32, val)               REG_FLD_SET(e_mpcp_stat_FLD_mpcp_err_cnt, (reg32), (val))
#define e_mpcp_stat_SET_mpcp_rgst_cnt(reg32, val)              REG_FLD_SET(e_mpcp_stat_FLD_mpcp_rgst_cnt, (reg32), (val))
#define e_mpcp_stat_SET_mpcp_dscv_gate_cnt(reg32, val)         REG_FLD_SET(e_mpcp_stat_FLD_mpcp_dscv_gate_cnt, (reg32), (val))

#define e_mpcp_rgst_stat_SET_mpcp_rgst_req_cnt(reg32, val)     REG_FLD_SET(e_mpcp_rgst_stat_FLD_mpcp_rgst_req_cnt, (reg32), (val))
#define e_mpcp_rgst_stat_SET_mpcp_rgst_ack_cnt(reg32, val)     REG_FLD_SET(e_mpcp_rgst_stat_FLD_mpcp_rgst_ack_cnt, (reg32), (val))

#define e_rxmbi_bytecnt_h_SET_rxmbi_bytecnt_h(reg32, val)      REG_FLD_SET(e_rxmbi_bytecnt_h_FLD_rxmbi_bytecnt_h, (reg32), (val))

#define e_rxmbi_bytecnt_l_SET_rxmbi_bytecnt_l(reg32, val)      REG_FLD_SET(e_rxmbi_bytecnt_l_FLD_rxmbi_bytecnt_l, (reg32), (val))

#define e_rxmbi_snf_cnt_SET_rxmbi_snf_cnt(reg32, val)          REG_FLD_SET(e_rxmbi_snf_cnt_FLD_rxmbi_snf_cnt, (reg32), (val))

#define e_rxmpi_uc_cnt_SET_rxmpi_uceth_cnt(reg32, val)         REG_FLD_SET(e_rxmpi_uc_cnt_FLD_rxmpi_uceth_cnt, (reg32), (val))

#define e_rxmpi_bc_cnt_SET_rxmpi_bceth_cnt(reg32, val)         REG_FLD_SET(e_rxmpi_bc_cnt_FLD_rxmpi_bceth_cnt, (reg32), (val))

#define e_rxmpi_mc_cnt_SET_rxmpi_mceth_cnt(reg32, val)         REG_FLD_SET(e_rxmpi_mc_cnt_FLD_rxmpi_mceth_cnt, (reg32), (val))

#define e_rxmpi_oam_cnt_SET_rxmpi_oam_cnt(reg32, val)          REG_FLD_SET(e_rxmpi_oam_cnt_FLD_rxmpi_oam_cnt, (reg32), (val))

#define e_rxmpi_mpcp_cnt_SET_rxmpi_mpcp_cnt(reg32, val)        REG_FLD_SET(e_rxmpi_mpcp_cnt_FLD_rxmpi_mpcp_cnt, (reg32), (val))

#define e_rxmpi_gate_cnt_SET_rxmpi_gate_cnt(reg32, val)        REG_FLD_SET(e_rxmpi_gate_cnt_FLD_rxmpi_gate_cnt, (reg32), (val))

#define e_rxmpi_nrlgate_cnt_SET_rxmpi_nrlgate_cnt(reg32, val)  REG_FLD_SET(e_rxmpi_nrlgate_cnt_FLD_rxmpi_nrlgate_cnt, (reg32), (val))

#define e_txmbi_uceth_cnt_SET_txmbi_uceth_cnt(reg32, val)      REG_FLD_SET(e_txmbi_uceth_cnt_FLD_txmbi_uceth_cnt, (reg32), (val))

#define e_txmbi_mceth_cnt_SET_txmbi_bceth_cnt(reg32, val)      REG_FLD_SET(e_txmbi_mceth_cnt_FLD_txmbi_bceth_cnt, (reg32), (val))
#define e_txmbi_mceth_cnt_SET_txmbi_mceth_cnt(reg32, val)      REG_FLD_SET(e_txmbi_mceth_cnt_FLD_txmbi_mceth_cnt, (reg32), (val))

#define e_tx_dygasp_cnt_SET_txmbi_dygasp_cnt(reg32, val)       REG_FLD_SET(e_tx_dygasp_cnt_FLD_txmbi_dygasp_cnt, (reg32), (val))
#define e_tx_dygasp_cnt_SET_txmpi_dygasp_cnt(reg32, val)       REG_FLD_SET(e_tx_dygasp_cnt_FLD_txmpi_dygasp_cnt, (reg32), (val))

#define e_tx_rpt_cnt_SET_txmbi_rpt_cnt(reg32, val)             REG_FLD_SET(e_tx_rpt_cnt_FLD_txmbi_rpt_cnt, (reg32), (val))
#define e_tx_rpt_cnt_SET_txmpi_rpt_cnt(reg32, val)             REG_FLD_SET(e_tx_rpt_cnt_FLD_txmpi_rpt_cnt, (reg32), (val))

#define e_kpoam_stat_SET_txmbi_kpalv_cnt(reg32, val)           REG_FLD_SET(e_kpoam_stat_FLD_txmbi_kpalv_cnt, (reg32), (val))
#define e_kpoam_stat_SET_txmpi_kpalv_cnt(reg32, val)           REG_FLD_SET(e_kpoam_stat_FLD_txmpi_kpalv_cnt, (reg32), (val))

#define e_txmpi_mpcp_cnt_SET_txmpi_oam_cnt(reg32, val)         REG_FLD_SET(e_txmpi_mpcp_cnt_FLD_txmpi_oam_cnt, (reg32), (val))
#define e_txmpi_mpcp_cnt_SET_txmpi_rgreq_cnt(reg32, val)       REG_FLD_SET(e_txmpi_mpcp_cnt_FLD_txmpi_rgreq_cnt, (reg32), (val))
#define e_txmpi_mpcp_cnt_SET_txmpi_rgack_cnt(reg32, val)       REG_FLD_SET(e_txmpi_mpcp_cnt_FLD_txmpi_rgack_cnt, (reg32), (val))

#define e_txmpi_uceth_cnt_SET_txmpi_uceth_cnt(reg32, val)      REG_FLD_SET(e_txmpi_uceth_cnt_FLD_txmpi_uceth_cnt, (reg32), (val))

#define e_txmpi_mceth_cnt_SET_txmpi_bceth_cnt(reg32, val)      REG_FLD_SET(e_txmpi_mceth_cnt_FLD_txmpi_bceth_cnt, (reg32), (val))
#define e_txmpi_mceth_cnt_SET_txmpi_mceth_cnt(reg32, val)      REG_FLD_SET(e_txmpi_mceth_cnt_FLD_txmpi_mceth_cnt, (reg32), (val))

#define e_rxadv_cnt_SET_rx_infomis_dscvgate_cnt(reg32, val)    REG_FLD_SET(e_rxadv_cnt_FLD_rx_infomis_dscvgate_cnt, (reg32), (val))
#define e_rxadv_cnt_SET_rx_llidbc_nrlgate_cnt(reg32, val)      REG_FLD_SET(e_rxadv_cnt_FLD_rx_llidbc_nrlgate_cnt, (reg32), (val))
#define e_rxadv_cnt_SET_rx_filt_dscvgate_cnt(reg32, val)       REG_FLD_SET(e_rxadv_cnt_FLD_rx_filt_dscvgate_cnt, (reg32), (val))

#define e_rxmpi_churn_cnt_SET_rxmpi_churn_crcok_cnt(reg32, val) REG_FLD_SET(e_rxmpi_churn_cnt_FLD_rxmpi_churn_crcok_cnt, (reg32), (val))
#define e_rxmpi_churn_cnt_SET_rxmpi_churn_crcerr_cnt(reg32, val) REG_FLD_SET(e_rxmpi_churn_cnt_FLD_rxmpi_churn_crcerr_cnt, (reg32), (val))

#define e_rxmpi_churn_info_SET_rxmpi_churn_keyidx(reg32, val)  REG_FLD_SET(e_rxmpi_churn_info_FLD_rxmpi_churn_keyidx, (reg32), (val))
#define e_rxmpi_churn_info_SET_rxmpi_churn_llididx(reg32, val) REG_FLD_SET(e_rxmpi_churn_info_FLD_rxmpi_churn_llididx, (reg32), (val))
#define e_rxmpi_churn_info_SET_rxmpi_churncrc_keyidx(reg32, val) REG_FLD_SET(e_rxmpi_churn_info_FLD_rxmpi_churncrc_keyidx, (reg32), (val))
#define e_rxmpi_churn_info_SET_rxmpi_churncrc_llididx(reg32, val) REG_FLD_SET(e_rxmpi_churn_info_FLD_rxmpi_churncrc_llididx, (reg32), (val))
#define e_rxmpi_churn_info_SET_rxmpi_abchurn_cnt(reg32, val)   REG_FLD_SET(e_rxmpi_churn_info_FLD_rxmpi_abchurn_cnt, (reg32), (val))

#define e_desec_ok_cnt_SET_desec_rx_ok_cnt(reg32, val)         REG_FLD_SET(e_desec_ok_cnt_FLD_desec_rx_ok_cnt, (reg32), (val))
#define e_desec_ok_cnt_SET_desec_plain_ok_cnt(reg32, val)      REG_FLD_SET(e_desec_ok_cnt_FLD_desec_plain_ok_cnt, (reg32), (val))

#define e_rxmpi_p_cnt_SET_rxmpi_p_ok_cnt(reg32, val)           REG_FLD_SET(e_rxmpi_p_cnt_FLD_rxmpi_p_ok_cnt, (reg32), (val))
#define e_rxmpi_p_cnt_SET_rxmpi_p_err_cnt(reg32, val)          REG_FLD_SET(e_rxmpi_p_cnt_FLD_rxmpi_p_err_cnt, (reg32), (val))

#define e_rxmpi_crc8err_cnt_SET_rxmpi_crc32err_cnt(reg32, val) REG_FLD_SET(e_rxmpi_crc8err_cnt_FLD_rxmpi_crc32err_cnt, (reg32), (val))
#define e_rxmpi_crc8err_cnt_SET_rxmpi_crc8err_cnt(reg32, val)  REG_FLD_SET(e_rxmpi_crc8err_cnt_FLD_rxmpi_crc8err_cnt, (reg32), (val))

#define e_desec_rxdrop_cnt_SET_desec_rxfifo_ovrundrop_cnt(reg32, val) REG_FLD_SET(e_desec_rxdrop_cnt_FLD_desec_rxfifo_ovrundrop_cnt, (reg32), (val))
#define e_desec_rxdrop_cnt_SET_desec_rxsof_ovrundrop_cnt(reg32, val) REG_FLD_SET(e_desec_rxdrop_cnt_FLD_desec_rxsof_ovrundrop_cnt, (reg32), (val))

#define e_desec_rxerr_cnt_SET_desec_rx_crcerr_cnt(reg32, val)  REG_FLD_SET(e_desec_rxerr_cnt_FLD_desec_rx_crcerr_cnt, (reg32), (val))
#define e_desec_rxerr_cnt_SET_desec_rx_preerr_cnt(reg32, val)  REG_FLD_SET(e_desec_rxerr_cnt_FLD_desec_rx_preerr_cnt, (reg32), (val))

#define e_desec_ciphdrop_cnt_SET_desec_rxsof_hdrdrop_cnt(reg32, val) REG_FLD_SET(e_desec_ciphdrop_cnt_FLD_desec_rxsof_hdrdrop_cnt, (reg32), (val))
#define e_desec_ciphdrop_cnt_SET_desec_ciph_csdrop_cnt(reg32, val) REG_FLD_SET(e_desec_ciphdrop_cnt_FLD_desec_ciph_csdrop_cnt, (reg32), (val))

#define e_desec_aeswdrop_cnt_SET_desec_ciph_actdrop_cnt(reg32, val) REG_FLD_SET(e_desec_aeswdrop_cnt_FLD_desec_ciph_actdrop_cnt, (reg32), (val))
#define e_desec_aeswdrop_cnt_SET_desec_aesw_csdrop_cnt(reg32, val) REG_FLD_SET(e_desec_aeswdrop_cnt_FLD_desec_aesw_csdrop_cnt, (reg32), (val))

#define e_desec_aeswdrop_cnt2_SET_desec_aesw_ovrundrop_cnt(reg32, val) REG_FLD_SET(e_desec_aeswdrop_cnt2_FLD_desec_aesw_ovrundrop_cnt, (reg32), (val))
#define e_desec_aeswdrop_cnt2_SET_desec_aesw_icverrdrop_cnt(reg32, val) REG_FLD_SET(e_desec_aeswdrop_cnt2_FLD_desec_aesw_icverrdrop_cnt, (reg32), (val))

#define e_rxmpi_drop_cnt_SET_rxmpi_eofdrop_cnt(reg32, val)     REG_FLD_SET(e_rxmpi_drop_cnt_FLD_rxmpi_eofdrop_cnt, (reg32), (val))
#define e_rxmpi_drop_cnt_SET_rxmpi_fifoovrun_cnt(reg32, val)   REG_FLD_SET(e_rxmpi_drop_cnt_FLD_rxmpi_fifoovrun_cnt, (reg32), (val))

#define e_rxmbi_drop_cnt_SET_rxmbi_sofdrop_cnt(reg32, val)     REG_FLD_SET(e_rxmbi_drop_cnt_FLD_rxmbi_sofdrop_cnt, (reg32), (val))
#define e_rxmbi_drop_cnt_SET_rxmbi_snfdrop_cnt(reg32, val)     REG_FLD_SET(e_rxmbi_drop_cnt_FLD_rxmbi_snfdrop_cnt, (reg32), (val))

#define e_rxmbi_drop_cnt2_SET_rxmbi_crcerr_cnt(reg32, val)     REG_FLD_SET(e_rxmbi_drop_cnt2_FLD_rxmbi_crcerr_cnt, (reg32), (val))
#define e_rxmbi_drop_cnt2_SET_rxmbi_enddrop_cnt(reg32, val)    REG_FLD_SET(e_rxmbi_drop_cnt2_FLD_rxmbi_enddrop_cnt, (reg32), (val))

#define e_txmbi_err_cnt_SET_txmbi_err_cnt(reg32, val)          REG_FLD_SET(e_txmbi_err_cnt_FLD_txmbi_err_cnt, (reg32), (val))

#define e_sts_dmy11_SET_xe_sts_dmy11(reg32, val)               REG_FLD_SET(e_sts_dmy11_FLD_xe_sts_dmy11, (reg32), (val))

#define e_sts_dmy12_SET_xe_sts_dmy12(reg32, val)               REG_FLD_SET(e_sts_dmy12_FLD_xe_sts_dmy12, (reg32), (val))

#define e_sts_dmy13_SET_xe_sts_dmy13(reg32, val)               REG_FLD_SET(e_sts_dmy13_FLD_xe_sts_dmy13, (reg32), (val))

#define e_sts_dmy14_SET_xe_sts_dmy14(reg32, val)               REG_FLD_SET(e_sts_dmy14_FLD_xe_sts_dmy14, (reg32), (val))

#define e_sts_dmy15_SET_xe_sts_dmy15(reg32, val)               REG_FLD_SET(e_sts_dmy15_FLD_xe_sts_dmy15, (reg32), (val))

#define e_sts_dmy16_SET_xe_sts_dmy16(reg32, val)               REG_FLD_SET(e_sts_dmy16_FLD_xe_sts_dmy16, (reg32), (val))

#define e_sts_dmy17_SET_xe_sts_dmy17(reg32, val)               REG_FLD_SET(e_sts_dmy17_FLD_xe_sts_dmy17, (reg32), (val))

#define e_sts_dmy18_SET_xe_sts_dmy18(reg32, val)               REG_FLD_SET(e_sts_dmy18_FLD_xe_sts_dmy18, (reg32), (val))

#define e_sts_dmy19_SET_xe_sts_dmy19(reg32, val)               REG_FLD_SET(e_sts_dmy19_FLD_xe_sts_dmy19, (reg32), (val))

#define e_sts_dmy20_SET_xe_sts_dmy20(reg32, val)               REG_FLD_SET(e_sts_dmy20_FLD_xe_sts_dmy20, (reg32), (val))

#define e_sts_dmy21_SET_xe_sts_dmy21(reg32, val)               REG_FLD_SET(e_sts_dmy21_FLD_xe_sts_dmy21, (reg32), (val))

#define e_sts_dmy22_SET_xe_sts_dmy22(reg32, val)               REG_FLD_SET(e_sts_dmy22_FLD_xe_sts_dmy22, (reg32), (val))

#define e_sts_dmy23_SET_xe_sts_dmy23(reg32, val)               REG_FLD_SET(e_sts_dmy23_FLD_xe_sts_dmy23, (reg32), (val))

#define e_sts_dmy24_SET_xe_sts_dmy24(reg32, val)               REG_FLD_SET(e_sts_dmy24_FLD_xe_sts_dmy24, (reg32), (val))

#define e_sts_dmy25_SET_xe_sts_dmy25(reg32, val)               REG_FLD_SET(e_sts_dmy25_FLD_xe_sts_dmy25, (reg32), (val))

#define e_gnt_type_stat_SET_b2b_gnt_cnt(reg32, val)            REG_FLD_SET(e_gnt_type_stat_FLD_b2b_gnt_cnt, (reg32), (val))
#define e_gnt_type_stat_SET_hdn_gnt_cnt(reg32, val)            REG_FLD_SET(e_gnt_type_stat_FLD_hdn_gnt_cnt, (reg32), (val))

#define e_gnt_pending_stat_SET_max_gnt_pending_cnt(reg32, val) REG_FLD_SET(e_gnt_pending_stat_FLD_max_gnt_pending_cnt, (reg32), (val))
#define e_gnt_pending_stat_SET_cur_gnt_pending_cnt(reg32, val) REG_FLD_SET(e_gnt_pending_stat_FLD_cur_gnt_pending_cnt, (reg32), (val))

#define e_gnt_length_stat_SET_max_gnt_length(reg32, val)       REG_FLD_SET(e_gnt_length_stat_FLD_max_gnt_length, (reg32), (val))
#define e_gnt_length_stat_SET_min_gnt_length(reg32, val)       REG_FLD_SET(e_gnt_length_stat_FLD_min_gnt_length, (reg32), (val))

#define e_local_time_SET_local_time(reg32, val)                REG_FLD_SET(e_local_time_FLD_local_time, (reg32), (val))

#define e_time_drft_stat_SET_cur_time_drift_ofst(reg32, val)   REG_FLD_SET(e_time_drft_stat_FLD_cur_time_drift_ofst, (reg32), (val))
#define e_time_drft_stat_SET_max_time_drift_ofst(reg32, val)   REG_FLD_SET(e_time_drft_stat_FLD_max_time_drift_ofst, (reg32), (val))
#define e_time_drft_stat_SET_cur_time_drift(reg32, val)        REG_FLD_SET(e_time_drft_stat_FLD_cur_time_drift, (reg32), (val))
#define e_time_drft_stat_SET_max_time_drift(reg32, val)        REG_FLD_SET(e_time_drft_stat_FLD_max_time_drift, (reg32), (val))

#define e_tx_timedrift_stat_SET_tx_cur_time_drift_ofst(reg32, val) REG_FLD_SET(e_tx_timedrift_stat_FLD_tx_cur_time_drift_ofst, (reg32), (val))
#define e_tx_timedrift_stat_SET_tx_max_time_drift_ofst(reg32, val) REG_FLD_SET(e_tx_timedrift_stat_FLD_tx_max_time_drift_ofst, (reg32), (val))
#define e_tx_timedrift_stat_SET_tx_cur_time_drift(reg32, val)  REG_FLD_SET(e_tx_timedrift_stat_FLD_tx_cur_time_drift, (reg32), (val))
#define e_tx_timedrift_stat_SET_tx_max_time_drift(reg32, val)  REG_FLD_SET(e_tx_timedrift_stat_FLD_tx_max_time_drift, (reg32), (val))

#define e_rxfifo_depth_stat_SET_cur_rxfifo_depth(reg32, val)   REG_FLD_SET(e_rxfifo_depth_stat_FLD_cur_rxfifo_depth, (reg32), (val))
#define e_rxfifo_depth_stat_SET_max_rxfifo_depth(reg32, val)   REG_FLD_SET(e_rxfifo_depth_stat_FLD_max_rxfifo_depth, (reg32), (val))

#define e_sts_dmy0_SET_xe_sts_dmy0(reg32, val)                 REG_FLD_SET(e_sts_dmy0_FLD_xe_sts_dmy0, (reg32), (val))

#define e_sts_dmy1_SET_xe_sts_dmy1(reg32, val)                 REG_FLD_SET(e_sts_dmy1_FLD_xe_sts_dmy1, (reg32), (val))

#define e_sts_dmy2_SET_xe_sts_dmy2(reg32, val)                 REG_FLD_SET(e_sts_dmy2_FLD_xe_sts_dmy2, (reg32), (val))

#define e_sts_dmy3_SET_xe_sts_dmy3(reg32, val)                 REG_FLD_SET(e_sts_dmy3_FLD_xe_sts_dmy3, (reg32), (val))

#define e_sts_dmy4_SET_xe_sts_dmy4(reg32, val)                 REG_FLD_SET(e_sts_dmy4_FLD_xe_sts_dmy4, (reg32), (val))

#define e_sts_dmy5_SET_xe_sts_dmy5(reg32, val)                 REG_FLD_SET(e_sts_dmy5_FLD_xe_sts_dmy5, (reg32), (val))

#define e_txmbi_bufwrap_wrcnt_SET_txmbi_bufwrap_wrerr_cnt(reg32, val) REG_FLD_SET(e_txmbi_bufwrap_wrcnt_FLD_txmbi_bufwrap_wrerr_cnt, (reg32), (val))
#define e_txmbi_bufwrap_wrcnt_SET_txmbi_bufwrap_wr_cnt(reg32, val) REG_FLD_SET(e_txmbi_bufwrap_wrcnt_FLD_txmbi_bufwrap_wr_cnt, (reg32), (val))

#define e_txmbi_bufwrap_rdcnt_SET_txmbi_bufwrap_rderr_cnt(reg32, val) REG_FLD_SET(e_txmbi_bufwrap_rdcnt_FLD_txmbi_bufwrap_rderr_cnt, (reg32), (val))
#define e_txmbi_bufwrap_rdcnt_SET_txmbi_bufwrap_rd_cnt(reg32, val) REG_FLD_SET(e_txmbi_bufwrap_rdcnt_FLD_txmbi_bufwrap_rd_cnt, (reg32), (val))

#define e_txmbi_bufwrap_rdcnt2_SET_txmbi_bufwrap_rddmy_cnt(reg32, val) REG_FLD_SET(e_txmbi_bufwrap_rdcnt2_FLD_txmbi_bufwrap_rddmy_cnt, (reg32), (val))

#define e_glb_cfg_VAL_txoam_favor(val)                         REG_FLD_VAL(e_glb_cfg_FLD_txoam_favor, (val))
#define e_glb_cfg_VAL_burst_en_dly(val)                        REG_FLD_VAL(e_glb_cfg_FLD_burst_en_dly, (val))
#define e_glb_cfg_VAL_discv_burst_en(val)                      REG_FLD_VAL(e_glb_cfg_FLD_discv_burst_en, (val))
#define e_glb_cfg_VAL_mpcp_fwd(val)                            REG_FLD_VAL(e_glb_cfg_FLD_mpcp_fwd, (val))
#define e_glb_cfg_VAL_bcst_llid_m1_drop(val)                   REG_FLD_VAL(e_glb_cfg_FLD_bcst_llid_m1_drop, (val))
#define e_glb_cfg_VAL_bcst_llid_m0_drop(val)                   REG_FLD_VAL(e_glb_cfg_FLD_bcst_llid_m0_drop, (val))
#define e_glb_cfg_VAL_mcst_llid_drop(val)                      REG_FLD_VAL(e_glb_cfg_FLD_mcst_llid_drop, (val))
#define e_glb_cfg_VAL_allucst_llid_fwd(val)                    REG_FLD_VAL(e_glb_cfg_FLD_allucst_llid_fwd, (val))
#define e_glb_cfg_VAL_fcs_err_fwd(val)                         REG_FLD_VAL(e_glb_cfg_FLD_fcs_err_fwd, (val))
#define e_glb_cfg_VAL_llid_crc8_err_fwd(val)                   REG_FLD_VAL(e_glb_cfg_FLD_llid_crc8_err_fwd, (val))
#define e_glb_cfg_VAL_rxmpi_stop(val)                          REG_FLD_VAL(e_glb_cfg_FLD_rxmpi_stop, (val))
#define e_glb_cfg_VAL_txmpi_stop(val)                          REG_FLD_VAL(e_glb_cfg_FLD_txmpi_stop, (val))
#define e_glb_cfg_VAL_phy_pwr_down(val)                        REG_FLD_VAL(e_glb_cfg_FLD_phy_pwr_down, (val))
#define e_glb_cfg_VAL_rx_nml_gate_fwd(val)                     REG_FLD_VAL(e_glb_cfg_FLD_rx_nml_gate_fwd, (val))
#define e_glb_cfg_VAL_rxmbi_stop(val)                          REG_FLD_VAL(e_glb_cfg_FLD_rxmbi_stop, (val))
#define e_glb_cfg_VAL_txmbi_stop(val)                          REG_FLD_VAL(e_glb_cfg_FLD_txmbi_stop, (val))
#define e_glb_cfg_VAL_chk_all_gnt_mode(val)                    REG_FLD_VAL(e_glb_cfg_FLD_chk_all_gnt_mode, (val))
#define e_glb_cfg_VAL_mpcp_done_set(val)                       REG_FLD_VAL(e_glb_cfg_FLD_mpcp_done_set, (val))
#define e_glb_cfg_VAL_epon_oam_cal_in_eth(val)                 REG_FLD_VAL(e_glb_cfg_FLD_epon_oam_cal_in_eth, (val))
#define e_glb_cfg_VAL_rpt_txpri_ctrl(val)                      REG_FLD_VAL(e_glb_cfg_FLD_rpt_txpri_ctrl, (val))

#define e_glb_cfg2_VAL_llidnum_sel(val)                        REG_FLD_VAL(e_glb_cfg2_FLD_llidnum_sel, (val))
#define e_glb_cfg2_VAL_rxdv_tmout_en(val)                      REG_FLD_VAL(e_glb_cfg2_FLD_rxdv_tmout_en, (val))
#define e_glb_cfg2_VAL_txmbi_nack_enden(val)                   REG_FLD_VAL(e_glb_cfg2_FLD_txmbi_nack_enden, (val))
#define e_glb_cfg2_VAL_loctime_mtd(val)                        REG_FLD_VAL(e_glb_cfg2_FLD_loctime_mtd, (val))
#define e_glb_cfg2_VAL_rgreq_lsrtime_mask(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_rgreq_lsrtime_mask, (val))
#define e_glb_cfg2_VAL_crc_cal_in_bytecnt(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_crc_cal_in_bytecnt, (val))
#define e_glb_cfg2_VAL_u10g_txmode(val)                        REG_FLD_VAL(e_glb_cfg2_FLD_u10g_txmode, (val))
#define e_glb_cfg2_VAL_eth_cal_in_bytecnt(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_eth_cal_in_bytecnt, (val))
#define e_glb_cfg2_VAL_snf_cal_in_bytecnt(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_snf_cal_in_bytecnt, (val))
#define e_glb_cfg2_VAL_oam_cal_in_bytecnt(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_oam_cal_in_bytecnt, (val))
#define e_glb_cfg2_VAL_dscvgate_filt(val)                      REG_FLD_VAL(e_glb_cfg2_FLD_dscvgate_filt, (val))
#define e_glb_cfg2_VAL_gntlen_stat_widscv(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_gntlen_stat_widscv, (val))
#define e_glb_cfg2_VAL_tdrift_loctupd_dis(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_tdrift_loctupd_dis, (val))
#define e_glb_cfg2_VAL_dscvgate_infochk_dis(val)               REG_FLD_VAL(e_glb_cfg2_FLD_dscvgate_infochk_dis, (val))

#define e_glb_sts1_VAL_xepon_type(val)                         REG_FLD_VAL(e_glb_sts1_FLD_xepon_type, (val))
#define e_glb_sts1_VAL_rxmpi_stop_sts(val)                     REG_FLD_VAL(e_glb_sts1_FLD_rxmpi_stop_sts, (val))
#define e_glb_sts1_VAL_txmpi_stop_sts(val)                     REG_FLD_VAL(e_glb_sts1_FLD_txmpi_stop_sts, (val))
#define e_glb_sts1_VAL_rxmbi_stop_sts(val)                     REG_FLD_VAL(e_glb_sts1_FLD_rxmbi_stop_sts, (val))
#define e_glb_sts1_VAL_txmbi_stop_sts(val)                     REG_FLD_VAL(e_glb_sts1_FLD_txmbi_stop_sts, (val))
#define e_glb_sts1_VAL_xepon_glb_cfg_sts(val)                  REG_FLD_VAL(e_glb_sts1_FLD_xepon_glb_cfg_sts, (val))

#define e_glb_sts2_VAL_xepon_glb_cfg2_sts(val)                 REG_FLD_VAL(e_glb_sts2_FLD_xepon_glb_cfg2_sts, (val))

#define e_int_status_VAL_us_keychg_int(val)                    REG_FLD_VAL(e_int_status_FLD_us_keychg_int, (val))
#define e_int_status_VAL_ds_keymis_int(val)                    REG_FLD_VAL(e_int_status_FLD_ds_keymis_int, (val))
#define e_int_status_VAL_ds_keychg_int(val)                    REG_FLD_VAL(e_int_status_FLD_ds_keychg_int, (val))
#define e_int_status_VAL_sniff_fifo_ovrun_int(val)             REG_FLD_VAL(e_int_status_FLD_sniff_fifo_ovrun_int, (val))
#define e_int_status_VAL_reg_ack_done_int(val)                 REG_FLD_VAL(e_int_status_FLD_reg_ack_done_int, (val))
#define e_int_status_VAL_reg_req_done_int(val)                 REG_FLD_VAL(e_int_status_FLD_reg_req_done_int, (val))
#define e_int_status_VAL_reorder1_gnt_int(val)                 REG_FLD_VAL(e_int_status_FLD_reorder1_gnt_int, (val))
#define e_int_status_VAL_b2b_gnt_int(val)                      REG_FLD_VAL(e_int_status_FLD_b2b_gnt_int, (val))
#define e_int_status_VAL_hidn_gnt_int(val)                     REG_FLD_VAL(e_int_status_FLD_hidn_gnt_int, (val))
#define e_int_status_VAL_ps_early_wakeup_int(val)              REG_FLD_VAL(e_int_status_FLD_ps_early_wakeup_int, (val))
#define e_int_status_VAL_rx_sleep_allow_int(val)               REG_FLD_VAL(e_int_status_FLD_rx_sleep_allow_int, (val))
#define e_int_status_VAL_ps_wakeup_int(val)                    REG_FLD_VAL(e_int_status_FLD_ps_wakeup_int, (val))
#define e_int_status_VAL_ps_sleep_int(val)                     REG_FLD_VAL(e_int_status_FLD_ps_sleep_int, (val))
#define e_int_status_VAL_txfifo_udrun_int(val)                 REG_FLD_VAL(e_int_status_FLD_txfifo_udrun_int, (val))
#define e_int_status_VAL_rpt_overintvl_int(val)                REG_FLD_VAL(e_int_status_FLD_rpt_overintvl_int, (val))
#define e_int_status_VAL_mpcp_timeout_int(val)                 REG_FLD_VAL(e_int_status_FLD_mpcp_timeout_int, (val))
#define e_int_status_VAL_timedrft_int(val)                     REG_FLD_VAL(e_int_status_FLD_timedrft_int, (val))
#define e_int_status_VAL_tod_1pps_int(val)                     REG_FLD_VAL(e_int_status_FLD_tod_1pps_int, (val))
#define e_int_status_VAL_tod_updt_int(val)                     REG_FLD_VAL(e_int_status_FLD_tod_updt_int, (val))
#define e_int_status_VAL_ptp_msg_tx_int(val)                   REG_FLD_VAL(e_int_status_FLD_ptp_msg_tx_int, (val))
#define e_int_status_VAL_gnt_buf_ovrrun_int(val)               REG_FLD_VAL(e_int_status_FLD_gnt_buf_ovrrun_int, (val))
#define e_int_status_VAL_llid7_rcv_rgst_int(val)               REG_FLD_VAL(e_int_status_FLD_llid7_rcv_rgst_int, (val))
#define e_int_status_VAL_llid6_rcv_rgst_int(val)               REG_FLD_VAL(e_int_status_FLD_llid6_rcv_rgst_int, (val))
#define e_int_status_VAL_llid5_rcv_rgst_int(val)               REG_FLD_VAL(e_int_status_FLD_llid5_rcv_rgst_int, (val))
#define e_int_status_VAL_llid4_rcv_rgst_int(val)               REG_FLD_VAL(e_int_status_FLD_llid4_rcv_rgst_int, (val))
#define e_int_status_VAL_llid3_rcv_rgst_int(val)               REG_FLD_VAL(e_int_status_FLD_llid3_rcv_rgst_int, (val))
#define e_int_status_VAL_llid2_rcv_rgst_int(val)               REG_FLD_VAL(e_int_status_FLD_llid2_rcv_rgst_int, (val))
#define e_int_status_VAL_llid1_rcv_rgst_int(val)               REG_FLD_VAL(e_int_status_FLD_llid1_rcv_rgst_int, (val))
#define e_int_status_VAL_llid0_rcv_rgst_int(val)               REG_FLD_VAL(e_int_status_FLD_llid0_rcv_rgst_int, (val))
#define e_int_status_VAL_rcv_dscvry_gate_int(val)              REG_FLD_VAL(e_int_status_FLD_rcv_dscvry_gate_int, (val))

#define e_int_en_VAL_us_keychg_en(val)                         REG_FLD_VAL(e_int_en_FLD_us_keychg_en, (val))
#define e_int_en_VAL_ds_keymis_en(val)                         REG_FLD_VAL(e_int_en_FLD_ds_keymis_en, (val))
#define e_int_en_VAL_ds_keychg_en(val)                         REG_FLD_VAL(e_int_en_FLD_ds_keychg_en, (val))
#define e_int_en_VAL_sniff_fifo_ovrun_en(val)                  REG_FLD_VAL(e_int_en_FLD_sniff_fifo_ovrun_en, (val))
#define e_int_en_VAL_reg_ack_done_en(val)                      REG_FLD_VAL(e_int_en_FLD_reg_ack_done_en, (val))
#define e_int_en_VAL_reg_req_done_en(val)                      REG_FLD_VAL(e_int_en_FLD_reg_req_done_en, (val))
#define e_int_en_VAL_reorder1_gnt_en(val)                      REG_FLD_VAL(e_int_en_FLD_reorder1_gnt_en, (val))
#define e_int_en_VAL_b2b_gnt_en(val)                           REG_FLD_VAL(e_int_en_FLD_b2b_gnt_en, (val))
#define e_int_en_VAL_hidn_gnt_en(val)                          REG_FLD_VAL(e_int_en_FLD_hidn_gnt_en, (val))
#define e_int_en_VAL_ps_early_wakeup_en(val)                   REG_FLD_VAL(e_int_en_FLD_ps_early_wakeup_en, (val))
#define e_int_en_VAL_rx_sleep_allow_en(val)                    REG_FLD_VAL(e_int_en_FLD_rx_sleep_allow_en, (val))
#define e_int_en_VAL_ps_wakeup_en(val)                         REG_FLD_VAL(e_int_en_FLD_ps_wakeup_en, (val))
#define e_int_en_VAL_ps_sleep_en(val)                          REG_FLD_VAL(e_int_en_FLD_ps_sleep_en, (val))
#define e_int_en_VAL_txfifo_udrun_en(val)                      REG_FLD_VAL(e_int_en_FLD_txfifo_udrun_en, (val))
#define e_int_en_VAL_rpt_overintvl_en(val)                     REG_FLD_VAL(e_int_en_FLD_rpt_overintvl_en, (val))
#define e_int_en_VAL_mpcp_timeout_en(val)                      REG_FLD_VAL(e_int_en_FLD_mpcp_timeout_en, (val))
#define e_int_en_VAL_timedrft_en(val)                          REG_FLD_VAL(e_int_en_FLD_timedrft_en, (val))
#define e_int_en_VAL_tod_1pps_en(val)                          REG_FLD_VAL(e_int_en_FLD_tod_1pps_en, (val))
#define e_int_en_VAL_tod_updt_en(val)                          REG_FLD_VAL(e_int_en_FLD_tod_updt_en, (val))
#define e_int_en_VAL_ptp_msg_tx_en(val)                        REG_FLD_VAL(e_int_en_FLD_ptp_msg_tx_en, (val))
#define e_int_en_VAL_gnt_buf_ovrrun_en(val)                    REG_FLD_VAL(e_int_en_FLD_gnt_buf_ovrrun_en, (val))
#define e_int_en_VAL_llid7_rcv_rgst_en(val)                    REG_FLD_VAL(e_int_en_FLD_llid7_rcv_rgst_en, (val))
#define e_int_en_VAL_llid6_rcv_rgst_en(val)                    REG_FLD_VAL(e_int_en_FLD_llid6_rcv_rgst_en, (val))
#define e_int_en_VAL_llid5_rcv_rgst_en(val)                    REG_FLD_VAL(e_int_en_FLD_llid5_rcv_rgst_en, (val))
#define e_int_en_VAL_llid4_rcv_rgst_en(val)                    REG_FLD_VAL(e_int_en_FLD_llid4_rcv_rgst_en, (val))
#define e_int_en_VAL_llid3_rcv_rgst_en(val)                    REG_FLD_VAL(e_int_en_FLD_llid3_rcv_rgst_en, (val))
#define e_int_en_VAL_llid2_rcv_rgst_en(val)                    REG_FLD_VAL(e_int_en_FLD_llid2_rcv_rgst_en, (val))
#define e_int_en_VAL_llid1_rcv_rgst_en(val)                    REG_FLD_VAL(e_int_en_FLD_llid1_rcv_rgst_en, (val))
#define e_int_en_VAL_llid0_rcv_rgst_en(val)                    REG_FLD_VAL(e_int_en_FLD_llid0_rcv_rgst_en, (val))
#define e_int_en_VAL_rcv_dscvry_gate_en(val)                   REG_FLD_VAL(e_int_en_FLD_rcv_dscvry_gate_en, (val))

#define e_int_sts2_VAL_tx_latestart_int(val)                   REG_FLD_VAL(e_int_sts2_FLD_tx_latestart_int, (val))
#define e_int_sts2_VAL_rcv_ifchk_err_int(val)                  REG_FLD_VAL(e_int_sts2_FLD_rcv_ifchk_err_int, (val))
#define e_int_sts2_VAL_rx_rgst_exc_int(val)                    REG_FLD_VAL(e_int_sts2_FLD_rx_rgst_exc_int, (val))
#define e_int_sts2_VAL_rcv_len_err_int(val)                    REG_FLD_VAL(e_int_sts2_FLD_rcv_len_err_int, (val))
#define e_int_sts2_VAL_schfch_nopkt_int(val)                   REG_FLD_VAL(e_int_sts2_FLD_schfch_nopkt_int, (val))
#define e_int_sts2_VAL_rxfifo_ovrun_int(val)                   REG_FLD_VAL(e_int_sts2_FLD_rxfifo_ovrun_int, (val))
#define e_int_sts2_VAL_txmpi_pldenneq_int(val)                 REG_FLD_VAL(e_int_sts2_FLD_txmpi_pldenneq_int, (val))
#define e_int_sts2_VAL_schfch_timeo_int(val)                   REG_FLD_VAL(e_int_sts2_FLD_schfch_timeo_int, (val))
#define e_int_sts2_VAL_schgnt_inv_int(val)                     REG_FLD_VAL(e_int_sts2_FLD_schgnt_inv_int, (val))
#define e_int_sts2_VAL_rcvgnt_infofail_int(val)                REG_FLD_VAL(e_int_sts2_FLD_rcvgnt_infofail_int, (val))
#define e_int_sts2_VAL_rcv_eofdrop_int(val)                    REG_FLD_VAL(e_int_sts2_FLD_rcv_eofdrop_int, (val))
#define e_int_sts2_VAL_rcv_crcerr_int(val)                     REG_FLD_VAL(e_int_sts2_FLD_rcv_crcerr_int, (val))
#define e_int_sts2_VAL_rcv_excgate_int(val)                    REG_FLD_VAL(e_int_sts2_FLD_rcv_excgate_int, (val))

#define e_int_en2_VAL_tx_latestart_en(val)                     REG_FLD_VAL(e_int_en2_FLD_tx_latestart_en, (val))
#define e_int_en2_VAL_rcv_ifchk_err_en(val)                    REG_FLD_VAL(e_int_en2_FLD_rcv_ifchk_err_en, (val))
#define e_int_en2_VAL_rx_rgst_exc_en(val)                      REG_FLD_VAL(e_int_en2_FLD_rx_rgst_exc_en, (val))
#define e_int_en2_VAL_rcv_len_err_en(val)                      REG_FLD_VAL(e_int_en2_FLD_rcv_len_err_en, (val))
#define e_int_en2_VAL_schfch_nopkt_en(val)                     REG_FLD_VAL(e_int_en2_FLD_schfch_nopkt_en, (val))
#define e_int_en2_VAL_rxfifo_ovrun_en(val)                     REG_FLD_VAL(e_int_en2_FLD_rxfifo_ovrun_en, (val))
#define e_int_en2_VAL_txmpi_pldenneq_en(val)                   REG_FLD_VAL(e_int_en2_FLD_txmpi_pldenneq_en, (val))
#define e_int_en2_VAL_schfch_timeo_en(val)                     REG_FLD_VAL(e_int_en2_FLD_schfch_timeo_en, (val))
#define e_int_en2_VAL_schgnt_inv_en(val)                       REG_FLD_VAL(e_int_en2_FLD_schgnt_inv_en, (val))
#define e_int_en2_VAL_rcvgnt_infofail_en(val)                  REG_FLD_VAL(e_int_en2_FLD_rcvgnt_infofail_en, (val))
#define e_int_en2_VAL_rcv_eofdrop_en(val)                      REG_FLD_VAL(e_int_en2_FLD_rcv_eofdrop_en, (val))
#define e_int_en2_VAL_rcv_crcerr_en(val)                       REG_FLD_VAL(e_int_en2_FLD_rcv_crcerr_en, (val))
#define e_int_en2_VAL_rcv_excgate_en(val)                      REG_FLD_VAL(e_int_en2_FLD_rcv_excgate_en, (val))

#define e_int_sts3_VAL_llid31_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid31_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid30_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid30_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid29_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid29_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid28_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid28_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid27_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid27_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid26_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid26_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid25_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid25_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid24_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid24_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid23_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid23_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid22_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid22_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid21_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid21_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid20_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid20_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid19_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid19_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid18_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid18_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid17_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid17_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid16_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid16_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid15_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid15_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid14_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid14_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid13_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid13_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid12_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid12_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid11_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid11_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid10_rcv_rgst_int(val)                REG_FLD_VAL(e_int_sts3_FLD_llid10_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid9_rcv_rgst_int(val)                 REG_FLD_VAL(e_int_sts3_FLD_llid9_rcv_rgst_int, (val))
#define e_int_sts3_VAL_llid8_rcv_rgst_int(val)                 REG_FLD_VAL(e_int_sts3_FLD_llid8_rcv_rgst_int, (val))

#define e_int_en3_VAL_llid31_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid31_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid30_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid30_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid29_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid29_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid28_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid28_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid27_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid27_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid26_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid26_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid25_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid25_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid24_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid24_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid23_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid23_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid22_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid22_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid21_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid21_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid20_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid20_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid19_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid19_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid18_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid18_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid17_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid17_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid16_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid16_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid15_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid15_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid14_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid14_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid13_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid13_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid12_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid12_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid11_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid11_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid10_rcv_rgst_en(val)                  REG_FLD_VAL(e_int_en3_FLD_llid10_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid9_rcv_rgst_en(val)                   REG_FLD_VAL(e_int_en3_FLD_llid9_rcv_rgst_en, (val))
#define e_int_en3_VAL_llid8_rcv_rgst_en(val)                   REG_FLD_VAL(e_int_en3_FLD_llid8_rcv_rgst_en, (val))

#define e_exc_sts_VAL_encfrm_alierr_hit(val)                   REG_FLD_VAL(e_exc_sts_FLD_encfrm_alierr_hit, (val))
#define e_exc_sts_VAL_schgnt_tmout_hit(val)                    REG_FLD_VAL(e_exc_sts_FLD_schgnt_tmout_hit, (val))
#define e_exc_sts_VAL_rx_rgst_dergst_hit(val)                  REG_FLD_VAL(e_exc_sts_FLD_rx_rgst_dergst_hit, (val))
#define e_exc_sts_VAL_rx_rgst_nack_hit(val)                    REG_FLD_VAL(e_exc_sts_FLD_rx_rgst_nack_hit, (val))
#define e_exc_sts_VAL_rx_slderr_hit(val)                       REG_FLD_VAL(e_exc_sts_FLD_rx_slderr_hit, (val))
#define e_exc_sts_VAL_rx_nodv_tmo_hit(val)                     REG_FLD_VAL(e_exc_sts_FLD_rx_nodv_tmo_hit, (val))
#define e_exc_sts_VAL_rx_noeof_hit(val)                        REG_FLD_VAL(e_exc_sts_FLD_rx_noeof_hit, (val))
#define e_exc_sts_VAL_snf_pktcnt_err_hit(val)                  REG_FLD_VAL(e_exc_sts_FLD_snf_pktcnt_err_hit, (val))
#define e_exc_sts_VAL_tx_gntst_nosog_hit(val)                  REG_FLD_VAL(e_exc_sts_FLD_tx_gntst_nosog_hit, (val))
#define e_exc_sts_VAL_tx_gntst_fifound_hit(val)                REG_FLD_VAL(e_exc_sts_FLD_tx_gntst_fifound_hit, (val))
#define e_exc_sts_VAL_tx_gntph_fifound_hit(val)                REG_FLD_VAL(e_exc_sts_FLD_tx_gntph_fifound_hit, (val))
#define e_exc_sts_VAL_tx_gntph_exceed_hit(val)                 REG_FLD_VAL(e_exc_sts_FLD_tx_gntph_exceed_hit, (val))
#define e_exc_sts_VAL_schgnt_stinv_hit(val)                    REG_FLD_VAL(e_exc_sts_FLD_schgnt_stinv_hit, (val))
#define e_exc_sts_VAL_schgnt_idxinv_hit(val)                   REG_FLD_VAL(e_exc_sts_FLD_schgnt_idxinv_hit, (val))
#define e_exc_sts_VAL_decrpt_sofeof_col_hit(val)               REG_FLD_VAL(e_exc_sts_FLD_decrpt_sofeof_col_hit, (val))
#define e_exc_sts_VAL_rcv_len_long_hit(val)                    REG_FLD_VAL(e_exc_sts_FLD_rcv_len_long_hit, (val))
#define e_exc_sts_VAL_rcv_len_short_hit(val)                   REG_FLD_VAL(e_exc_sts_FLD_rcv_len_short_hit, (val))
#define e_exc_sts_VAL_rcv_mpcp_crcerr_hit(val)                 REG_FLD_VAL(e_exc_sts_FLD_rcv_mpcp_crcerr_hit, (val))
#define e_exc_sts_VAL_rcv_crc32err_hit(val)                    REG_FLD_VAL(e_exc_sts_FLD_rcv_crc32err_hit, (val))
#define e_exc_sts_VAL_rcv_crc8err_hit(val)                     REG_FLD_VAL(e_exc_sts_FLD_rcv_crc8err_hit, (val))
#define e_exc_sts_VAL_rcv_demacs_flagexc_hit(val)              REG_FLD_VAL(e_exc_sts_FLD_rcv_demacs_flagexc_hit, (val))
#define e_exc_sts_VAL_rcv_nrlgate_llidbc_hit(val)              REG_FLD_VAL(e_exc_sts_FLD_rcv_nrlgate_llidbc_hit, (val))
#define e_exc_sts_VAL_rcv_dscvgate_filt_hit(val)               REG_FLD_VAL(e_exc_sts_FLD_rcv_dscvgate_filt_hit, (val))
#define e_exc_sts_VAL_rcv_dscvgate_infomis_hit(val)            REG_FLD_VAL(e_exc_sts_FLD_rcv_dscvgate_infomis_hit, (val))

#define e_rpt_timout_VAL_llid31_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid31_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid30_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid30_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid29_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid29_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid28_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid28_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid27_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid27_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid26_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid26_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid25_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid25_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid24_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid24_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid23_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid23_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid22_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid22_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid21_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid21_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid20_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid20_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid19_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid19_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid18_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid18_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid17_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid17_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid16_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid16_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid15_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid15_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid14_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid14_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid13_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid13_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid12_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid12_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid11_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid11_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid10_rpt_tmo(val)                   REG_FLD_VAL(e_rpt_timout_FLD_llid10_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid9_rpt_tmo(val)                    REG_FLD_VAL(e_rpt_timout_FLD_llid9_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid8_rpt_tmo(val)                    REG_FLD_VAL(e_rpt_timout_FLD_llid8_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid7_rpt_tmo(val)                    REG_FLD_VAL(e_rpt_timout_FLD_llid7_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid6_rpt_tmo(val)                    REG_FLD_VAL(e_rpt_timout_FLD_llid6_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid5_rpt_tmo(val)                    REG_FLD_VAL(e_rpt_timout_FLD_llid5_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid4_rpt_tmo(val)                    REG_FLD_VAL(e_rpt_timout_FLD_llid4_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid3_rpt_tmo(val)                    REG_FLD_VAL(e_rpt_timout_FLD_llid3_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid2_rpt_tmo(val)                    REG_FLD_VAL(e_rpt_timout_FLD_llid2_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid1_rpt_tmo(val)                    REG_FLD_VAL(e_rpt_timout_FLD_llid1_rpt_tmo, (val))
#define e_rpt_timout_VAL_llid0_rpt_tmo(val)                    REG_FLD_VAL(e_rpt_timout_FLD_llid0_rpt_tmo, (val))

#define e_mpcp_timout_VAL_llid31_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid31_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid30_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid30_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid29_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid29_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid28_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid28_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid27_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid27_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid26_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid26_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid25_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid25_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid24_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid24_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid23_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid23_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid22_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid22_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid21_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid21_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid20_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid20_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid19_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid19_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid18_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid18_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid17_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid17_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid16_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid16_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid15_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid15_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid14_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid14_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid13_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid13_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid12_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid12_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid11_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid11_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid10_mpcp_tmo(val)                 REG_FLD_VAL(e_mpcp_timout_FLD_llid10_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid9_mpcp_tmo(val)                  REG_FLD_VAL(e_mpcp_timout_FLD_llid9_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid8_mpcp_tmo(val)                  REG_FLD_VAL(e_mpcp_timout_FLD_llid8_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid7_mpcp_tmo(val)                  REG_FLD_VAL(e_mpcp_timout_FLD_llid7_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid6_mpcp_tmo(val)                  REG_FLD_VAL(e_mpcp_timout_FLD_llid6_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid5_mpcp_tmo(val)                  REG_FLD_VAL(e_mpcp_timout_FLD_llid5_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid4_mpcp_tmo(val)                  REG_FLD_VAL(e_mpcp_timout_FLD_llid4_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid3_mpcp_tmo(val)                  REG_FLD_VAL(e_mpcp_timout_FLD_llid3_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid2_mpcp_tmo(val)                  REG_FLD_VAL(e_mpcp_timout_FLD_llid2_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid1_mpcp_tmo(val)                  REG_FLD_VAL(e_mpcp_timout_FLD_llid1_mpcp_tmo, (val))
#define e_mpcp_timout_VAL_llid0_mpcp_tmo(val)                  REG_FLD_VAL(e_mpcp_timout_FLD_llid0_mpcp_tmo, (val))

#define e_ds_keychg_VAL_llid31_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid31_ds_keychg, (val))
#define e_ds_keychg_VAL_llid30_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid30_ds_keychg, (val))
#define e_ds_keychg_VAL_llid29_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid29_ds_keychg, (val))
#define e_ds_keychg_VAL_llid28_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid28_ds_keychg, (val))
#define e_ds_keychg_VAL_llid27_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid27_ds_keychg, (val))
#define e_ds_keychg_VAL_llid26_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid26_ds_keychg, (val))
#define e_ds_keychg_VAL_llid25_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid25_ds_keychg, (val))
#define e_ds_keychg_VAL_llid24_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid24_ds_keychg, (val))
#define e_ds_keychg_VAL_llid23_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid23_ds_keychg, (val))
#define e_ds_keychg_VAL_llid22_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid22_ds_keychg, (val))
#define e_ds_keychg_VAL_llid21_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid21_ds_keychg, (val))
#define e_ds_keychg_VAL_llid20_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid20_ds_keychg, (val))
#define e_ds_keychg_VAL_llid19_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid19_ds_keychg, (val))
#define e_ds_keychg_VAL_llid18_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid18_ds_keychg, (val))
#define e_ds_keychg_VAL_llid17_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid17_ds_keychg, (val))
#define e_ds_keychg_VAL_llid16_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid16_ds_keychg, (val))
#define e_ds_keychg_VAL_llid15_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid15_ds_keychg, (val))
#define e_ds_keychg_VAL_llid14_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid14_ds_keychg, (val))
#define e_ds_keychg_VAL_llid13_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid13_ds_keychg, (val))
#define e_ds_keychg_VAL_llid12_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid12_ds_keychg, (val))
#define e_ds_keychg_VAL_llid11_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid11_ds_keychg, (val))
#define e_ds_keychg_VAL_llid10_ds_keychg(val)                  REG_FLD_VAL(e_ds_keychg_FLD_llid10_ds_keychg, (val))
#define e_ds_keychg_VAL_llid9_ds_keychg(val)                   REG_FLD_VAL(e_ds_keychg_FLD_llid9_ds_keychg, (val))
#define e_ds_keychg_VAL_llid8_ds_keychg(val)                   REG_FLD_VAL(e_ds_keychg_FLD_llid8_ds_keychg, (val))
#define e_ds_keychg_VAL_llid7_ds_keychg(val)                   REG_FLD_VAL(e_ds_keychg_FLD_llid7_ds_keychg, (val))
#define e_ds_keychg_VAL_llid6_ds_keychg(val)                   REG_FLD_VAL(e_ds_keychg_FLD_llid6_ds_keychg, (val))
#define e_ds_keychg_VAL_llid5_ds_keychg(val)                   REG_FLD_VAL(e_ds_keychg_FLD_llid5_ds_keychg, (val))
#define e_ds_keychg_VAL_llid4_ds_keychg(val)                   REG_FLD_VAL(e_ds_keychg_FLD_llid4_ds_keychg, (val))
#define e_ds_keychg_VAL_llid3_ds_keychg(val)                   REG_FLD_VAL(e_ds_keychg_FLD_llid3_ds_keychg, (val))
#define e_ds_keychg_VAL_llid2_ds_keychg(val)                   REG_FLD_VAL(e_ds_keychg_FLD_llid2_ds_keychg, (val))
#define e_ds_keychg_VAL_llid1_ds_keychg(val)                   REG_FLD_VAL(e_ds_keychg_FLD_llid1_ds_keychg, (val))
#define e_ds_keychg_VAL_llid0_ds_keychg(val)                   REG_FLD_VAL(e_ds_keychg_FLD_llid0_ds_keychg, (val))

#define e_ds_keychg2_VAL_llid51_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid51_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid50_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid50_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid49_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid49_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid48_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid48_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid47_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid47_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid46_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid46_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid45_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid45_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid44_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid44_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid43_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid43_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid42_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid42_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid41_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid41_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid40_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid40_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid39_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid39_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid38_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid38_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid37_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid37_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid36_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid36_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid35_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid35_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid34_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid34_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid33_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid33_ds_keychg, (val))
#define e_ds_keychg2_VAL_llid32_ds_keychg(val)                 REG_FLD_VAL(e_ds_keychg2_FLD_llid32_ds_keychg, (val))

#define e_ds_keymis_VAL_llid31_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid31_ds_keymis, (val))
#define e_ds_keymis_VAL_llid30_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid30_ds_keymis, (val))
#define e_ds_keymis_VAL_llid29_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid29_ds_keymis, (val))
#define e_ds_keymis_VAL_llid28_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid28_ds_keymis, (val))
#define e_ds_keymis_VAL_llid27_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid27_ds_keymis, (val))
#define e_ds_keymis_VAL_llid26_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid26_ds_keymis, (val))
#define e_ds_keymis_VAL_llid25_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid25_ds_keymis, (val))
#define e_ds_keymis_VAL_llid24_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid24_ds_keymis, (val))
#define e_ds_keymis_VAL_llid23_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid23_ds_keymis, (val))
#define e_ds_keymis_VAL_llid22_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid22_ds_keymis, (val))
#define e_ds_keymis_VAL_llid21_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid21_ds_keymis, (val))
#define e_ds_keymis_VAL_llid20_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid20_ds_keymis, (val))
#define e_ds_keymis_VAL_llid19_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid19_ds_keymis, (val))
#define e_ds_keymis_VAL_llid18_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid18_ds_keymis, (val))
#define e_ds_keymis_VAL_llid17_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid17_ds_keymis, (val))
#define e_ds_keymis_VAL_llid16_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid16_ds_keymis, (val))
#define e_ds_keymis_VAL_llid15_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid15_ds_keymis, (val))
#define e_ds_keymis_VAL_llid14_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid14_ds_keymis, (val))
#define e_ds_keymis_VAL_llid13_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid13_ds_keymis, (val))
#define e_ds_keymis_VAL_llid12_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid12_ds_keymis, (val))
#define e_ds_keymis_VAL_llid11_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid11_ds_keymis, (val))
#define e_ds_keymis_VAL_llid10_ds_keymis(val)                  REG_FLD_VAL(e_ds_keymis_FLD_llid10_ds_keymis, (val))
#define e_ds_keymis_VAL_llid9_ds_keymis(val)                   REG_FLD_VAL(e_ds_keymis_FLD_llid9_ds_keymis, (val))
#define e_ds_keymis_VAL_llid8_ds_keymis(val)                   REG_FLD_VAL(e_ds_keymis_FLD_llid8_ds_keymis, (val))
#define e_ds_keymis_VAL_llid7_ds_keymis(val)                   REG_FLD_VAL(e_ds_keymis_FLD_llid7_ds_keymis, (val))
#define e_ds_keymis_VAL_llid6_ds_keymis(val)                   REG_FLD_VAL(e_ds_keymis_FLD_llid6_ds_keymis, (val))
#define e_ds_keymis_VAL_llid5_ds_keymis(val)                   REG_FLD_VAL(e_ds_keymis_FLD_llid5_ds_keymis, (val))
#define e_ds_keymis_VAL_llid4_ds_keymis(val)                   REG_FLD_VAL(e_ds_keymis_FLD_llid4_ds_keymis, (val))
#define e_ds_keymis_VAL_llid3_ds_keymis(val)                   REG_FLD_VAL(e_ds_keymis_FLD_llid3_ds_keymis, (val))
#define e_ds_keymis_VAL_llid2_ds_keymis(val)                   REG_FLD_VAL(e_ds_keymis_FLD_llid2_ds_keymis, (val))
#define e_ds_keymis_VAL_llid1_ds_keymis(val)                   REG_FLD_VAL(e_ds_keymis_FLD_llid1_ds_keymis, (val))
#define e_ds_keymis_VAL_llid0_ds_keymis(val)                   REG_FLD_VAL(e_ds_keymis_FLD_llid0_ds_keymis, (val))

#define e_ds_keymis2_VAL_llid51_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid51_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid50_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid50_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid49_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid49_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid48_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid48_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid47_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid47_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid46_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid46_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid45_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid45_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid44_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid44_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid43_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid43_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid42_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid42_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid41_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid41_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid40_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid40_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid39_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid39_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid38_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid38_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid37_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid37_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid36_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid36_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid35_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid35_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid34_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid34_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid33_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid33_ds_keymis, (val))
#define e_ds_keymis2_VAL_llid32_ds_keymis(val)                 REG_FLD_VAL(e_ds_keymis2_FLD_llid32_ds_keymis, (val))

#define e_us_keychg_VAL_llid31_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid31_us_keychg, (val))
#define e_us_keychg_VAL_llid30_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid30_us_keychg, (val))
#define e_us_keychg_VAL_llid29_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid29_us_keychg, (val))
#define e_us_keychg_VAL_llid28_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid28_us_keychg, (val))
#define e_us_keychg_VAL_llid27_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid27_us_keychg, (val))
#define e_us_keychg_VAL_llid26_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid26_us_keychg, (val))
#define e_us_keychg_VAL_llid25_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid25_us_keychg, (val))
#define e_us_keychg_VAL_llid24_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid24_us_keychg, (val))
#define e_us_keychg_VAL_llid23_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid23_us_keychg, (val))
#define e_us_keychg_VAL_llid22_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid22_us_keychg, (val))
#define e_us_keychg_VAL_llid21_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid21_us_keychg, (val))
#define e_us_keychg_VAL_llid20_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid20_us_keychg, (val))
#define e_us_keychg_VAL_llid19_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid19_us_keychg, (val))
#define e_us_keychg_VAL_llid18_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid18_us_keychg, (val))
#define e_us_keychg_VAL_llid17_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid17_us_keychg, (val))
#define e_us_keychg_VAL_llid16_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid16_us_keychg, (val))
#define e_us_keychg_VAL_llid15_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid15_us_keychg, (val))
#define e_us_keychg_VAL_llid14_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid14_us_keychg, (val))
#define e_us_keychg_VAL_llid13_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid13_us_keychg, (val))
#define e_us_keychg_VAL_llid12_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid12_us_keychg, (val))
#define e_us_keychg_VAL_llid11_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid11_us_keychg, (val))
#define e_us_keychg_VAL_llid10_us_keychg(val)                  REG_FLD_VAL(e_us_keychg_FLD_llid10_us_keychg, (val))
#define e_us_keychg_VAL_llid9_us_keychg(val)                   REG_FLD_VAL(e_us_keychg_FLD_llid9_us_keychg, (val))
#define e_us_keychg_VAL_llid8_us_keychg(val)                   REG_FLD_VAL(e_us_keychg_FLD_llid8_us_keychg, (val))
#define e_us_keychg_VAL_llid7_us_keychg(val)                   REG_FLD_VAL(e_us_keychg_FLD_llid7_us_keychg, (val))
#define e_us_keychg_VAL_llid6_us_keychg(val)                   REG_FLD_VAL(e_us_keychg_FLD_llid6_us_keychg, (val))
#define e_us_keychg_VAL_llid5_us_keychg(val)                   REG_FLD_VAL(e_us_keychg_FLD_llid5_us_keychg, (val))
#define e_us_keychg_VAL_llid4_us_keychg(val)                   REG_FLD_VAL(e_us_keychg_FLD_llid4_us_keychg, (val))
#define e_us_keychg_VAL_llid3_us_keychg(val)                   REG_FLD_VAL(e_us_keychg_FLD_llid3_us_keychg, (val))
#define e_us_keychg_VAL_llid2_us_keychg(val)                   REG_FLD_VAL(e_us_keychg_FLD_llid2_us_keychg, (val))
#define e_us_keychg_VAL_llid1_us_keychg(val)                   REG_FLD_VAL(e_us_keychg_FLD_llid1_us_keychg, (val))
#define e_us_keychg_VAL_llid0_us_keychg(val)                   REG_FLD_VAL(e_us_keychg_FLD_llid0_us_keychg, (val))

#define e_cfg_dmy0_VAL_xe_cfg_dmy0(val)                        REG_FLD_VAL(e_cfg_dmy0_FLD_xe_cfg_dmy0, (val))

#define e_cfg_dmy1_VAL_xe_cfg_dmy1(val)                        REG_FLD_VAL(e_cfg_dmy1_FLD_xe_cfg_dmy1, (val))

#define e_llid0_3_cfg_VAL_llid3_dmy(val)                       REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_dmy, (val))
#define e_llid0_3_cfg_VAL_llid3_encrypt_key(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_encrypt_key, (val))
#define e_llid0_3_cfg_VAL_llid3_encrypt_en(val)                REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_encrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid3_txfec_en(val)                  REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_txfec_en, (val))
#define e_llid0_3_cfg_VAL_llid3_dcrypt_en(val)                 REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_dcrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid3_dcrypt_mode(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_dcrypt_mode, (val))
#define e_llid0_3_cfg_VAL_llid3_oam_lpbk_en(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_oam_lpbk_en, (val))
#define e_llid0_3_cfg_VAL_llid2_dmy(val)                       REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_dmy, (val))
#define e_llid0_3_cfg_VAL_llid2_encrypt_key(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_encrypt_key, (val))
#define e_llid0_3_cfg_VAL_llid2_encrypt_en(val)                REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_encrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid2_txfec_en(val)                  REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_txfec_en, (val))
#define e_llid0_3_cfg_VAL_llid2_dcrypt_en(val)                 REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_dcrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid2_dcrypt_mode(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_dcrypt_mode, (val))
#define e_llid0_3_cfg_VAL_llid2_oam_lpbk_en(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_oam_lpbk_en, (val))
#define e_llid0_3_cfg_VAL_llid1_dmy(val)                       REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_dmy, (val))
#define e_llid0_3_cfg_VAL_llid1_encrypt_key(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_encrypt_key, (val))
#define e_llid0_3_cfg_VAL_llid1_encrypt_en(val)                REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_encrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid1_txfec_en(val)                  REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_txfec_en, (val))
#define e_llid0_3_cfg_VAL_llid1_dcrypt_en(val)                 REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_dcrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid1_dcrypt_mode(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_dcrypt_mode, (val))
#define e_llid0_3_cfg_VAL_llid1_oam_lpbk_en(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_oam_lpbk_en, (val))
#define e_llid0_3_cfg_VAL_llid0_dmy(val)                       REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_dmy, (val))
#define e_llid0_3_cfg_VAL_llid0_encrypt_key(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_encrypt_key, (val))
#define e_llid0_3_cfg_VAL_llid0_encrypt_en(val)                REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_encrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid0_txfec_en(val)                  REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_txfec_en, (val))
#define e_llid0_3_cfg_VAL_llid0_dcrypt_en(val)                 REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_dcrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid0_dcrypt_mode(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_dcrypt_mode, (val))
#define e_llid0_3_cfg_VAL_llid0_oam_lpbk_en(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_oam_lpbk_en, (val))

#define e_llid4_7_cfg_VAL_llid7_dmy(val)                       REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_dmy, (val))
#define e_llid4_7_cfg_VAL_llid7_encrypt_key(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_encrypt_key, (val))
#define e_llid4_7_cfg_VAL_llid7_encrypt_en(val)                REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_encrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid7_txfec_en(val)                  REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_txfec_en, (val))
#define e_llid4_7_cfg_VAL_llid7_dcrypt_en(val)                 REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_dcrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid7_dcrypt_mode(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_dcrypt_mode, (val))
#define e_llid4_7_cfg_VAL_llid7_oam_lpbk_en(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_oam_lpbk_en, (val))
#define e_llid4_7_cfg_VAL_llid6_dmy(val)                       REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_dmy, (val))
#define e_llid4_7_cfg_VAL_llid6_encrypt_key(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_encrypt_key, (val))
#define e_llid4_7_cfg_VAL_llid6_encrypt_en(val)                REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_encrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid6_txfec_en(val)                  REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_txfec_en, (val))
#define e_llid4_7_cfg_VAL_llid6_dcrypt_en(val)                 REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_dcrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid6_dcrypt_mode(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_dcrypt_mode, (val))
#define e_llid4_7_cfg_VAL_llid6_oam_lpbk_en(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_oam_lpbk_en, (val))
#define e_llid4_7_cfg_VAL_llid5_dmy(val)                       REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_dmy, (val))
#define e_llid4_7_cfg_VAL_llid5_encrypt_key(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_encrypt_key, (val))
#define e_llid4_7_cfg_VAL_llid5_encrypt_en(val)                REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_encrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid5_txfec_en(val)                  REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_txfec_en, (val))
#define e_llid4_7_cfg_VAL_llid5_dcrypt_en(val)                 REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_dcrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid5_dcrypt_mode(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_dcrypt_mode, (val))
#define e_llid4_7_cfg_VAL_llid5_oam_lpbk_en(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_oam_lpbk_en, (val))
#define e_llid4_7_cfg_VAL_llid4_dmy(val)                       REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_dmy, (val))
#define e_llid4_7_cfg_VAL_llid4_encrypt_key(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_encrypt_key, (val))
#define e_llid4_7_cfg_VAL_llid4_encrypt_en(val)                REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_encrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid4_txfec_en(val)                  REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_txfec_en, (val))
#define e_llid4_7_cfg_VAL_llid4_dcrypt_en(val)                 REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_dcrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid4_dcrypt_mode(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_dcrypt_mode, (val))
#define e_llid4_7_cfg_VAL_llid4_oam_lpbk_en(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_oam_lpbk_en, (val))

#define e_llid8_11_cfg_VAL_llid11_dmy(val)                     REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_dmy, (val))
#define e_llid8_11_cfg_VAL_llid11_encrypt_key(val)             REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_encrypt_key, (val))
#define e_llid8_11_cfg_VAL_llid11_encrypt_en(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_encrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid11_txfec_en(val)                REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_txfec_en, (val))
#define e_llid8_11_cfg_VAL_llid11_dcrypt_en(val)               REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_dcrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid11_dcrypt_mode(val)             REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_dcrypt_mode, (val))
#define e_llid8_11_cfg_VAL_llid11_oamlpbk_en(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_oamlpbk_en, (val))
#define e_llid8_11_cfg_VAL_llid10_dmy(val)                     REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_dmy, (val))
#define e_llid8_11_cfg_VAL_llid10_encrypt_key(val)             REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_encrypt_key, (val))
#define e_llid8_11_cfg_VAL_llid10_encrypt_en(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_encrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid10_txfec_en(val)                REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_txfec_en, (val))
#define e_llid8_11_cfg_VAL_llid10_dcrypt_en(val)               REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_dcrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid10_dcrypt_mode(val)             REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_dcrypt_mode, (val))
#define e_llid8_11_cfg_VAL_llid10_oamlpbk_en(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_oamlpbk_en, (val))
#define e_llid8_11_cfg_VAL_llid9_dmy(val)                      REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_dmy, (val))
#define e_llid8_11_cfg_VAL_llid9_encrypt_key(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_encrypt_key, (val))
#define e_llid8_11_cfg_VAL_llid9_encrypt_en(val)               REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_encrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid9_txfec_en(val)                 REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_txfec_en, (val))
#define e_llid8_11_cfg_VAL_llid9_dcrypt_en(val)                REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_dcrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid9_dcrypt_mode(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_dcrypt_mode, (val))
#define e_llid8_11_cfg_VAL_llid9_oamlpbk_en(val)               REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_oamlpbk_en, (val))
#define e_llid8_11_cfg_VAL_llid8_dmy(val)                      REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_dmy, (val))
#define e_llid8_11_cfg_VAL_llid8_encrypt_key(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_encrypt_key, (val))
#define e_llid8_11_cfg_VAL_llid8_encrypt_en(val)               REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_encrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid8_txfec_en(val)                 REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_txfec_en, (val))
#define e_llid8_11_cfg_VAL_llid8_dcrypt_en(val)                REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_dcrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid8_dcrypt_mode(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_dcrypt_mode, (val))
#define e_llid8_11_cfg_VAL_llid8_oamlpbk_en(val)               REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_oamlpbk_en, (val))

#define e_llid12_15_cfg_VAL_llid15_dmy(val)                    REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_dmy, (val))
#define e_llid12_15_cfg_VAL_llid15_encrypt_key(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_encrypt_key, (val))
#define e_llid12_15_cfg_VAL_llid15_encrypt_en(val)             REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_encrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid15_txfec_en(val)               REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_txfec_en, (val))
#define e_llid12_15_cfg_VAL_llid15_dcrypt_en(val)              REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_dcrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid15_dcrypt_mode(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_dcrypt_mode, (val))
#define e_llid12_15_cfg_VAL_llid15_oamlpbk_en(val)             REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_oamlpbk_en, (val))
#define e_llid12_15_cfg_VAL_llid14_dmy(val)                    REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_dmy, (val))
#define e_llid12_15_cfg_VAL_llid14_encrypt_key(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_encrypt_key, (val))
#define e_llid12_15_cfg_VAL_llid14_encrypt_en(val)             REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_encrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid14_txfec_en(val)               REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_txfec_en, (val))
#define e_llid12_15_cfg_VAL_llid14_dcrypt_en(val)              REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_dcrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid14_dcrypt_mode(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_dcrypt_mode, (val))
#define e_llid12_15_cfg_VAL_llid14_oamlpbk_en(val)             REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_oamlpbk_en, (val))
#define e_llid12_15_cfg_VAL_llid13_dmy(val)                    REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_dmy, (val))
#define e_llid12_15_cfg_VAL_llid13_encrypt_key(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_encrypt_key, (val))
#define e_llid12_15_cfg_VAL_llid13_encrypt_en(val)             REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_encrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid13_txfec_en(val)               REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_txfec_en, (val))
#define e_llid12_15_cfg_VAL_llid13_dcrypt_en(val)              REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_dcrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid13_dcrypt_mode(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_dcrypt_mode, (val))
#define e_llid12_15_cfg_VAL_llid13_oamlpbk_en(val)             REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_oamlpbk_en, (val))
#define e_llid12_15_cfg_VAL_llid12_dmy(val)                    REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_dmy, (val))
#define e_llid12_15_cfg_VAL_llid12_encrypt_key(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_encrypt_key, (val))
#define e_llid12_15_cfg_VAL_llid12_encrypt_en(val)             REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_encrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid12_txfec_en(val)               REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_txfec_en, (val))
#define e_llid12_15_cfg_VAL_llid12_dcrypt_en(val)              REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_dcrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid12_dcrypt_mode(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_dcrypt_mode, (val))
#define e_llid12_15_cfg_VAL_llid12_oam_lpbk_en(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_oam_lpbk_en, (val))

#define e_llid16_19_cfg_VAL_llid19_dmy(val)                    REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_dmy, (val))
#define e_llid16_19_cfg_VAL_llid19_encrypt_key(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_encrypt_key, (val))
#define e_llid16_19_cfg_VAL_llid19_encrypt_en(val)             REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_encrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid19_txfec_en(val)               REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_txfec_en, (val))
#define e_llid16_19_cfg_VAL_llid19_dcrypt_en(val)              REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_dcrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid19_dcrypt_mode(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_dcrypt_mode, (val))
#define e_llid16_19_cfg_VAL_llid19_oamlpbk_en(val)             REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_oamlpbk_en, (val))
#define e_llid16_19_cfg_VAL_llid18_dmy(val)                    REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_dmy, (val))
#define e_llid16_19_cfg_VAL_llid18_encrypt_key(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_encrypt_key, (val))
#define e_llid16_19_cfg_VAL_llid18_encrypt_en(val)             REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_encrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid18_txfec_en(val)               REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_txfec_en, (val))
#define e_llid16_19_cfg_VAL_llid18_dcrypt_en(val)              REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_dcrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid18_dcrypt_mode(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_dcrypt_mode, (val))
#define e_llid16_19_cfg_VAL_llid18_oamlpbk_en(val)             REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_oamlpbk_en, (val))
#define e_llid16_19_cfg_VAL_llid17_dmy(val)                    REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_dmy, (val))
#define e_llid16_19_cfg_VAL_llid17_encrypt_key(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_encrypt_key, (val))
#define e_llid16_19_cfg_VAL_llid17_encrypt_en(val)             REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_encrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid17_txfec_en(val)               REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_txfec_en, (val))
#define e_llid16_19_cfg_VAL_llid17_dcrypt_en(val)              REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_dcrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid17_dcrypt_mode(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_dcrypt_mode, (val))
#define e_llid16_19_cfg_VAL_llid17_oamlpbk_en(val)             REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_oamlpbk_en, (val))
#define e_llid16_19_cfg_VAL_llid16_dmy(val)                    REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_dmy, (val))
#define e_llid16_19_cfg_VAL_llid16_encrypt_key(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_encrypt_key, (val))
#define e_llid16_19_cfg_VAL_llid16_encrypt_en(val)             REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_encrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid16_txfec_en(val)               REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_txfec_en, (val))
#define e_llid16_19_cfg_VAL_llid16_dcrypt_en(val)              REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_dcrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid16_dcrypt_mode(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_dcrypt_mode, (val))
#define e_llid16_19_cfg_VAL_llid16_oam_lpbk_en(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_oam_lpbk_en, (val))

#define e_llid20_23_cfg_VAL_llid23_dmy(val)                    REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_dmy, (val))
#define e_llid20_23_cfg_VAL_llid23_encrypt_key(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_encrypt_key, (val))
#define e_llid20_23_cfg_VAL_llid23_encrypt_en(val)             REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_encrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid23_txfec_en(val)               REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_txfec_en, (val))
#define e_llid20_23_cfg_VAL_llid23_dcrypt_en(val)              REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_dcrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid23_dcrypt_mode(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_dcrypt_mode, (val))
#define e_llid20_23_cfg_VAL_llid23_oamlpbk_en(val)             REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_oamlpbk_en, (val))
#define e_llid20_23_cfg_VAL_llid22_dmy(val)                    REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_dmy, (val))
#define e_llid20_23_cfg_VAL_llid22_encrypt_key(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_encrypt_key, (val))
#define e_llid20_23_cfg_VAL_llid22_encrypt_en(val)             REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_encrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid22_txfec_en(val)               REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_txfec_en, (val))
#define e_llid20_23_cfg_VAL_llid22_dcrypt_en(val)              REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_dcrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid22_dcrypt_mode(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_dcrypt_mode, (val))
#define e_llid20_23_cfg_VAL_llid22_oamlpbk_en(val)             REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_oamlpbk_en, (val))
#define e_llid20_23_cfg_VAL_llid21_dmy(val)                    REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_dmy, (val))
#define e_llid20_23_cfg_VAL_llid21_encrypt_key(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_encrypt_key, (val))
#define e_llid20_23_cfg_VAL_llid21_encrypt_en(val)             REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_encrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid21_txfec_en(val)               REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_txfec_en, (val))
#define e_llid20_23_cfg_VAL_llid21_dcrypt_en(val)              REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_dcrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid21_dcrypt_mode(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_dcrypt_mode, (val))
#define e_llid20_23_cfg_VAL_llid21_oamlpbk_en(val)             REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_oamlpbk_en, (val))
#define e_llid20_23_cfg_VAL_llid20_dmy(val)                    REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_dmy, (val))
#define e_llid20_23_cfg_VAL_llid20_encrypt_key(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_encrypt_key, (val))
#define e_llid20_23_cfg_VAL_llid20_encrypt_en(val)             REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_encrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid20_txfec_en(val)               REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_txfec_en, (val))
#define e_llid20_23_cfg_VAL_llid20_dcrypt_en(val)              REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_dcrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid20_dcrypt_mode(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_dcrypt_mode, (val))
#define e_llid20_23_cfg_VAL_llid20_oam_lpbk_en(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_oam_lpbk_en, (val))

#define e_llid24_27_cfg_VAL_llid27_dmy(val)                    REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_dmy, (val))
#define e_llid24_27_cfg_VAL_llid27_encrypt_key(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_encrypt_key, (val))
#define e_llid24_27_cfg_VAL_llid27_encrypt_en(val)             REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_encrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid27_txfec_en(val)               REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_txfec_en, (val))
#define e_llid24_27_cfg_VAL_llid27_dcrypt_en(val)              REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_dcrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid27_dcrypt_mode(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_dcrypt_mode, (val))
#define e_llid24_27_cfg_VAL_llid27_oamlpbk_en(val)             REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_oamlpbk_en, (val))
#define e_llid24_27_cfg_VAL_llid26_dmy(val)                    REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_dmy, (val))
#define e_llid24_27_cfg_VAL_llid26_encrypt_key(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_encrypt_key, (val))
#define e_llid24_27_cfg_VAL_llid26_encrypt_en(val)             REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_encrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid26_txfec_en(val)               REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_txfec_en, (val))
#define e_llid24_27_cfg_VAL_llid26_dcrypt_en(val)              REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_dcrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid26_dcrypt_mode(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_dcrypt_mode, (val))
#define e_llid24_27_cfg_VAL_llid26_oamlpbk_en(val)             REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_oamlpbk_en, (val))
#define e_llid24_27_cfg_VAL_llid25_dmy(val)                    REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_dmy, (val))
#define e_llid24_27_cfg_VAL_llid25_encrypt_key(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_encrypt_key, (val))
#define e_llid24_27_cfg_VAL_llid25_encrypt_en(val)             REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_encrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid25_txfec_en(val)               REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_txfec_en, (val))
#define e_llid24_27_cfg_VAL_llid25_dcrypt_en(val)              REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_dcrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid25_dcrypt_mode(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_dcrypt_mode, (val))
#define e_llid24_27_cfg_VAL_llid25_oamlpbk_en(val)             REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_oamlpbk_en, (val))
#define e_llid24_27_cfg_VAL_llid24_dmy(val)                    REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_dmy, (val))
#define e_llid24_27_cfg_VAL_llid24_encrypt_key(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_encrypt_key, (val))
#define e_llid24_27_cfg_VAL_llid24_encrypt_en(val)             REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_encrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid24_txfec_en(val)               REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_txfec_en, (val))
#define e_llid24_27_cfg_VAL_llid24_dcrypt_en(val)              REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_dcrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid24_dcrypt_mode(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_dcrypt_mode, (val))
#define e_llid24_27_cfg_VAL_llid24_oam_lpbk_en(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_oam_lpbk_en, (val))

#define e_llid28_31_cfg_VAL_llid31_dmy(val)                    REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_dmy, (val))
#define e_llid28_31_cfg_VAL_llid31_encrypt_key(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_encrypt_key, (val))
#define e_llid28_31_cfg_VAL_llid31_encrypt_en(val)             REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_encrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid31_txfec_en(val)               REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_txfec_en, (val))
#define e_llid28_31_cfg_VAL_llid31_dcrypt_en(val)              REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_dcrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid31_dcrypt_mode(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_dcrypt_mode, (val))
#define e_llid28_31_cfg_VAL_llid31_oamlpbk_en(val)             REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_oamlpbk_en, (val))
#define e_llid28_31_cfg_VAL_llid30_dmy(val)                    REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_dmy, (val))
#define e_llid28_31_cfg_VAL_llid30_encrypt_key(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_encrypt_key, (val))
#define e_llid28_31_cfg_VAL_llid30_encrypt_en(val)             REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_encrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid30_txfec_en(val)               REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_txfec_en, (val))
#define e_llid28_31_cfg_VAL_llid30_dcrypt_en(val)              REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_dcrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid30_dcrypt_mode(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_dcrypt_mode, (val))
#define e_llid28_31_cfg_VAL_llid30_oamlpbk_en(val)             REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_oamlpbk_en, (val))
#define e_llid28_31_cfg_VAL_llid29_dmy(val)                    REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_dmy, (val))
#define e_llid28_31_cfg_VAL_llid29_encrypt_key(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_encrypt_key, (val))
#define e_llid28_31_cfg_VAL_llid29_encrypt_en(val)             REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_encrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid29_txfec_en(val)               REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_txfec_en, (val))
#define e_llid28_31_cfg_VAL_llid29_dcrypt_en(val)              REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_dcrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid29_dcrypt_mode(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_dcrypt_mode, (val))
#define e_llid28_31_cfg_VAL_llid29_oamlpbk_en(val)             REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_oamlpbk_en, (val))
#define e_llid28_31_cfg_VAL_llid28_dmy(val)                    REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_dmy, (val))
#define e_llid28_31_cfg_VAL_llid28_encrypt_key(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_encrypt_key, (val))
#define e_llid28_31_cfg_VAL_llid28_encrypt_en(val)             REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_encrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid28_txfec_en(val)               REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_txfec_en, (val))
#define e_llid28_31_cfg_VAL_llid28_dcrypt_en(val)              REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_dcrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid28_dcrypt_mode(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_dcrypt_mode, (val))
#define e_llid28_31_cfg_VAL_llid28_oam_lpbk_en(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_oam_lpbk_en, (val))

#define e_cfg_dmy2_VAL_xe_cfg_dmy2(val)                        REG_FLD_VAL(e_cfg_dmy2_FLD_xe_cfg_dmy2, (val))

#define e_cfg_dmy3_VAL_xe_cfg_dmy3(val)                        REG_FLD_VAL(e_cfg_dmy3_FLD_xe_cfg_dmy3, (val))

#define e_cfg_dmy4_VAL_xe_cfg_dmy4(val)                        REG_FLD_VAL(e_cfg_dmy4_FLD_xe_cfg_dmy4, (val))

#define e_llid_dscvry_ctrl_VAL_mpcp_cmd(val)                   REG_FLD_VAL(e_llid_dscvry_ctrl_FLD_mpcp_cmd, (val))
#define e_llid_dscvry_ctrl_VAL_mpcp_cmd_done(val)              REG_FLD_VAL(e_llid_dscvry_ctrl_FLD_mpcp_cmd_done, (val))
#define e_llid_dscvry_ctrl_VAL_rgstr_ack_flg(val)              REG_FLD_VAL(e_llid_dscvry_ctrl_FLD_rgstr_ack_flg, (val))
#define e_llid_dscvry_ctrl_VAL_rgstr_req_flg(val)              REG_FLD_VAL(e_llid_dscvry_ctrl_FLD_rgstr_req_flg, (val))
#define e_llid_dscvry_ctrl_VAL_tx_mpcp_llid_idx(val)           REG_FLD_VAL(e_llid_dscvry_ctrl_FLD_tx_mpcp_llid_idx, (val))

#define e_llid0_dscvry_sts_VAL_llid0_dscvry_sts(val)           REG_FLD_VAL(e_llid0_dscvry_sts_FLD_llid0_dscvry_sts, (val))
#define e_llid0_dscvry_sts_VAL_llid0_rgstr_flg_sts(val)        REG_FLD_VAL(e_llid0_dscvry_sts_FLD_llid0_rgstr_flg_sts, (val))
#define e_llid0_dscvry_sts_VAL_llid0_valid(val)                REG_FLD_VAL(e_llid0_dscvry_sts_FLD_llid0_valid, (val))
#define e_llid0_dscvry_sts_VAL_llid0_value(val)                REG_FLD_VAL(e_llid0_dscvry_sts_FLD_llid0_value, (val))

#define e_llid1_dscvry_sts_VAL_llid1_dscvry_sts(val)           REG_FLD_VAL(e_llid1_dscvry_sts_FLD_llid1_dscvry_sts, (val))
#define e_llid1_dscvry_sts_VAL_llid1_rgstr_flg_sts(val)        REG_FLD_VAL(e_llid1_dscvry_sts_FLD_llid1_rgstr_flg_sts, (val))
#define e_llid1_dscvry_sts_VAL_llid1_valid(val)                REG_FLD_VAL(e_llid1_dscvry_sts_FLD_llid1_valid, (val))
#define e_llid1_dscvry_sts_VAL_llid1_value(val)                REG_FLD_VAL(e_llid1_dscvry_sts_FLD_llid1_value, (val))

#define e_llid2_dscvry_sts_VAL_llid2_dscvry_sts(val)           REG_FLD_VAL(e_llid2_dscvry_sts_FLD_llid2_dscvry_sts, (val))
#define e_llid2_dscvry_sts_VAL_llid2_rgstr_flg_sts(val)        REG_FLD_VAL(e_llid2_dscvry_sts_FLD_llid2_rgstr_flg_sts, (val))
#define e_llid2_dscvry_sts_VAL_llid2_valid(val)                REG_FLD_VAL(e_llid2_dscvry_sts_FLD_llid2_valid, (val))
#define e_llid2_dscvry_sts_VAL_llid2_value(val)                REG_FLD_VAL(e_llid2_dscvry_sts_FLD_llid2_value, (val))

#define e_llid3_dscvry_sts_VAL_llid3_dscvry_sts(val)           REG_FLD_VAL(e_llid3_dscvry_sts_FLD_llid3_dscvry_sts, (val))
#define e_llid3_dscvry_sts_VAL_llid3_rgstr_flg_sts(val)        REG_FLD_VAL(e_llid3_dscvry_sts_FLD_llid3_rgstr_flg_sts, (val))
#define e_llid3_dscvry_sts_VAL_llid3_valid(val)                REG_FLD_VAL(e_llid3_dscvry_sts_FLD_llid3_valid, (val))
#define e_llid3_dscvry_sts_VAL_llid3_value(val)                REG_FLD_VAL(e_llid3_dscvry_sts_FLD_llid3_value, (val))

#define e_llid4_dscvry_sts_VAL_llid4_dscvry_sts(val)           REG_FLD_VAL(e_llid4_dscvry_sts_FLD_llid4_dscvry_sts, (val))
#define e_llid4_dscvry_sts_VAL_llid4_rgstr_flg_sts(val)        REG_FLD_VAL(e_llid4_dscvry_sts_FLD_llid4_rgstr_flg_sts, (val))
#define e_llid4_dscvry_sts_VAL_llid4_valid(val)                REG_FLD_VAL(e_llid4_dscvry_sts_FLD_llid4_valid, (val))
#define e_llid4_dscvry_sts_VAL_llid4_value(val)                REG_FLD_VAL(e_llid4_dscvry_sts_FLD_llid4_value, (val))

#define e_llid5_dscvry_sts_VAL_llid5_dscvry_sts(val)           REG_FLD_VAL(e_llid5_dscvry_sts_FLD_llid5_dscvry_sts, (val))
#define e_llid5_dscvry_sts_VAL_llid5_rgstr_flg_sts(val)        REG_FLD_VAL(e_llid5_dscvry_sts_FLD_llid5_rgstr_flg_sts, (val))
#define e_llid5_dscvry_sts_VAL_llid5_valid(val)                REG_FLD_VAL(e_llid5_dscvry_sts_FLD_llid5_valid, (val))
#define e_llid5_dscvry_sts_VAL_llid5_value(val)                REG_FLD_VAL(e_llid5_dscvry_sts_FLD_llid5_value, (val))

#define e_llid6_dscvry_sts_VAL_llid6_dscvry_sts(val)           REG_FLD_VAL(e_llid6_dscvry_sts_FLD_llid6_dscvry_sts, (val))
#define e_llid6_dscvry_sts_VAL_llid6_rgstr_flg_sts(val)        REG_FLD_VAL(e_llid6_dscvry_sts_FLD_llid6_rgstr_flg_sts, (val))
#define e_llid6_dscvry_sts_VAL_llid6_valid(val)                REG_FLD_VAL(e_llid6_dscvry_sts_FLD_llid6_valid, (val))
#define e_llid6_dscvry_sts_VAL_llid6_value(val)                REG_FLD_VAL(e_llid6_dscvry_sts_FLD_llid6_value, (val))

#define e_llid7_dscvry_sts_VAL_llid7_dscvry_sts(val)           REG_FLD_VAL(e_llid7_dscvry_sts_FLD_llid7_dscvry_sts, (val))
#define e_llid7_dscvry_sts_VAL_llid7_rgstr_flg_sts(val)        REG_FLD_VAL(e_llid7_dscvry_sts_FLD_llid7_rgstr_flg_sts, (val))
#define e_llid7_dscvry_sts_VAL_llid7_valid(val)                REG_FLD_VAL(e_llid7_dscvry_sts_FLD_llid7_valid, (val))
#define e_llid7_dscvry_sts_VAL_llid7_value(val)                REG_FLD_VAL(e_llid7_dscvry_sts_FLD_llid7_value, (val))

#define e_llid8_dscvry_sts_VAL_llid8_dscvry_sts(val)           REG_FLD_VAL(e_llid8_dscvry_sts_FLD_llid8_dscvry_sts, (val))
#define e_llid8_dscvry_sts_VAL_llid8_rgstr_flg_sts(val)        REG_FLD_VAL(e_llid8_dscvry_sts_FLD_llid8_rgstr_flg_sts, (val))
#define e_llid8_dscvry_sts_VAL_llid8_valid(val)                REG_FLD_VAL(e_llid8_dscvry_sts_FLD_llid8_valid, (val))
#define e_llid8_dscvry_sts_VAL_llid8_value(val)                REG_FLD_VAL(e_llid8_dscvry_sts_FLD_llid8_value, (val))

#define e_llid9_dscvry_sts_VAL_llid9_dscvry_sts(val)           REG_FLD_VAL(e_llid9_dscvry_sts_FLD_llid9_dscvry_sts, (val))
#define e_llid9_dscvry_sts_VAL_llid9_rgstr_flg_sts(val)        REG_FLD_VAL(e_llid9_dscvry_sts_FLD_llid9_rgstr_flg_sts, (val))
#define e_llid9_dscvry_sts_VAL_llid9_valid(val)                REG_FLD_VAL(e_llid9_dscvry_sts_FLD_llid9_valid, (val))
#define e_llid9_dscvry_sts_VAL_llid9_value(val)                REG_FLD_VAL(e_llid9_dscvry_sts_FLD_llid9_value, (val))

#define e_llid10_dscvry_sts_VAL_llid10_dscvry_sts(val)         REG_FLD_VAL(e_llid10_dscvry_sts_FLD_llid10_dscvry_sts, (val))
#define e_llid10_dscvry_sts_VAL_llid10_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid10_dscvry_sts_FLD_llid10_rgstr_flg_sts, (val))
#define e_llid10_dscvry_sts_VAL_llid10_valid(val)              REG_FLD_VAL(e_llid10_dscvry_sts_FLD_llid10_valid, (val))
#define e_llid10_dscvry_sts_VAL_llid10_value(val)              REG_FLD_VAL(e_llid10_dscvry_sts_FLD_llid10_value, (val))

#define e_llid11_dscvry_sts_VAL_llid11_dscvry_sts(val)         REG_FLD_VAL(e_llid11_dscvry_sts_FLD_llid11_dscvry_sts, (val))
#define e_llid11_dscvry_sts_VAL_llid11_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid11_dscvry_sts_FLD_llid11_rgstr_flg_sts, (val))
#define e_llid11_dscvry_sts_VAL_llid11_valid(val)              REG_FLD_VAL(e_llid11_dscvry_sts_FLD_llid11_valid, (val))
#define e_llid11_dscvry_sts_VAL_llid11_value(val)              REG_FLD_VAL(e_llid11_dscvry_sts_FLD_llid11_value, (val))

#define e_llid12_dscvry_sts_VAL_llid12_dscvry_sts(val)         REG_FLD_VAL(e_llid12_dscvry_sts_FLD_llid12_dscvry_sts, (val))
#define e_llid12_dscvry_sts_VAL_llid12_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid12_dscvry_sts_FLD_llid12_rgstr_flg_sts, (val))
#define e_llid12_dscvry_sts_VAL_llid12_valid(val)              REG_FLD_VAL(e_llid12_dscvry_sts_FLD_llid12_valid, (val))
#define e_llid12_dscvry_sts_VAL_llid12_value(val)              REG_FLD_VAL(e_llid12_dscvry_sts_FLD_llid12_value, (val))

#define e_llid13_dscvry_sts_VAL_llid13_dscvry_sts(val)         REG_FLD_VAL(e_llid13_dscvry_sts_FLD_llid13_dscvry_sts, (val))
#define e_llid13_dscvry_sts_VAL_llid13_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid13_dscvry_sts_FLD_llid13_rgstr_flg_sts, (val))
#define e_llid13_dscvry_sts_VAL_llid13_valid(val)              REG_FLD_VAL(e_llid13_dscvry_sts_FLD_llid13_valid, (val))
#define e_llid13_dscvry_sts_VAL_llid13_value(val)              REG_FLD_VAL(e_llid13_dscvry_sts_FLD_llid13_value, (val))

#define e_llid14_dscvry_sts_VAL_llid14_dscvry_sts(val)         REG_FLD_VAL(e_llid14_dscvry_sts_FLD_llid14_dscvry_sts, (val))
#define e_llid14_dscvry_sts_VAL_llid14_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid14_dscvry_sts_FLD_llid14_rgstr_flg_sts, (val))
#define e_llid14_dscvry_sts_VAL_llid14_valid(val)              REG_FLD_VAL(e_llid14_dscvry_sts_FLD_llid14_valid, (val))
#define e_llid14_dscvry_sts_VAL_llid14_value(val)              REG_FLD_VAL(e_llid14_dscvry_sts_FLD_llid14_value, (val))

#define e_llid15_dscvry_sts_VAL_llid15_dscvry_sts(val)         REG_FLD_VAL(e_llid15_dscvry_sts_FLD_llid15_dscvry_sts, (val))
#define e_llid15_dscvry_sts_VAL_llid15_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid15_dscvry_sts_FLD_llid15_rgstr_flg_sts, (val))
#define e_llid15_dscvry_sts_VAL_llid15_valid(val)              REG_FLD_VAL(e_llid15_dscvry_sts_FLD_llid15_valid, (val))
#define e_llid15_dscvry_sts_VAL_llid15_value(val)              REG_FLD_VAL(e_llid15_dscvry_sts_FLD_llid15_value, (val))

#define e_llid16_dscvry_sts_VAL_llid16_dscvry_sts(val)         REG_FLD_VAL(e_llid16_dscvry_sts_FLD_llid16_dscvry_sts, (val))
#define e_llid16_dscvry_sts_VAL_llid16_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid16_dscvry_sts_FLD_llid16_rgstr_flg_sts, (val))
#define e_llid16_dscvry_sts_VAL_llid16_valid(val)              REG_FLD_VAL(e_llid16_dscvry_sts_FLD_llid16_valid, (val))
#define e_llid16_dscvry_sts_VAL_llid16_value(val)              REG_FLD_VAL(e_llid16_dscvry_sts_FLD_llid16_value, (val))

#define e_llid17_dscvry_sts_VAL_llid17_dscvry_sts(val)         REG_FLD_VAL(e_llid17_dscvry_sts_FLD_llid17_dscvry_sts, (val))
#define e_llid17_dscvry_sts_VAL_llid17_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid17_dscvry_sts_FLD_llid17_rgstr_flg_sts, (val))
#define e_llid17_dscvry_sts_VAL_llid17_valid(val)              REG_FLD_VAL(e_llid17_dscvry_sts_FLD_llid17_valid, (val))
#define e_llid17_dscvry_sts_VAL_llid17_value(val)              REG_FLD_VAL(e_llid17_dscvry_sts_FLD_llid17_value, (val))

#define e_llid18_dscvry_sts_VAL_llid18_dscvry_sts(val)         REG_FLD_VAL(e_llid18_dscvry_sts_FLD_llid18_dscvry_sts, (val))
#define e_llid18_dscvry_sts_VAL_llid18_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid18_dscvry_sts_FLD_llid18_rgstr_flg_sts, (val))
#define e_llid18_dscvry_sts_VAL_llid18_valid(val)              REG_FLD_VAL(e_llid18_dscvry_sts_FLD_llid18_valid, (val))
#define e_llid18_dscvry_sts_VAL_llid18_value(val)              REG_FLD_VAL(e_llid18_dscvry_sts_FLD_llid18_value, (val))

#define e_llid19_dscvry_sts_VAL_llid19_dscvry_sts(val)         REG_FLD_VAL(e_llid19_dscvry_sts_FLD_llid19_dscvry_sts, (val))
#define e_llid19_dscvry_sts_VAL_llid19_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid19_dscvry_sts_FLD_llid19_rgstr_flg_sts, (val))
#define e_llid19_dscvry_sts_VAL_llid19_valid(val)              REG_FLD_VAL(e_llid19_dscvry_sts_FLD_llid19_valid, (val))
#define e_llid19_dscvry_sts_VAL_llid19_value(val)              REG_FLD_VAL(e_llid19_dscvry_sts_FLD_llid19_value, (val))

#define e_llid20_dscvry_sts_VAL_llid20_dscvry_sts(val)         REG_FLD_VAL(e_llid20_dscvry_sts_FLD_llid20_dscvry_sts, (val))
#define e_llid20_dscvry_sts_VAL_llid20_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid20_dscvry_sts_FLD_llid20_rgstr_flg_sts, (val))
#define e_llid20_dscvry_sts_VAL_llid20_valid(val)              REG_FLD_VAL(e_llid20_dscvry_sts_FLD_llid20_valid, (val))
#define e_llid20_dscvry_sts_VAL_llid20_value(val)              REG_FLD_VAL(e_llid20_dscvry_sts_FLD_llid20_value, (val))

#define e_llid21_dscvry_sts_VAL_llid21_dscvry_sts(val)         REG_FLD_VAL(e_llid21_dscvry_sts_FLD_llid21_dscvry_sts, (val))
#define e_llid21_dscvry_sts_VAL_llid21_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid21_dscvry_sts_FLD_llid21_rgstr_flg_sts, (val))
#define e_llid21_dscvry_sts_VAL_llid21_valid(val)              REG_FLD_VAL(e_llid21_dscvry_sts_FLD_llid21_valid, (val))
#define e_llid21_dscvry_sts_VAL_llid21_value(val)              REG_FLD_VAL(e_llid21_dscvry_sts_FLD_llid21_value, (val))

#define e_llid22_dscvry_sts_VAL_llid22_dscvry_sts(val)         REG_FLD_VAL(e_llid22_dscvry_sts_FLD_llid22_dscvry_sts, (val))
#define e_llid22_dscvry_sts_VAL_llid22_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid22_dscvry_sts_FLD_llid22_rgstr_flg_sts, (val))
#define e_llid22_dscvry_sts_VAL_llid22_valid(val)              REG_FLD_VAL(e_llid22_dscvry_sts_FLD_llid22_valid, (val))
#define e_llid22_dscvry_sts_VAL_llid22_value(val)              REG_FLD_VAL(e_llid22_dscvry_sts_FLD_llid22_value, (val))

#define e_llid23_dscvry_sts_VAL_llid23_dscvry_sts(val)         REG_FLD_VAL(e_llid23_dscvry_sts_FLD_llid23_dscvry_sts, (val))
#define e_llid23_dscvry_sts_VAL_llid23_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid23_dscvry_sts_FLD_llid23_rgstr_flg_sts, (val))
#define e_llid23_dscvry_sts_VAL_llid23_valid(val)              REG_FLD_VAL(e_llid23_dscvry_sts_FLD_llid23_valid, (val))
#define e_llid23_dscvry_sts_VAL_llid23_value(val)              REG_FLD_VAL(e_llid23_dscvry_sts_FLD_llid23_value, (val))

#define e_llid24_dscvry_sts_VAL_llid24_dscvry_sts(val)         REG_FLD_VAL(e_llid24_dscvry_sts_FLD_llid24_dscvry_sts, (val))
#define e_llid24_dscvry_sts_VAL_llid24_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid24_dscvry_sts_FLD_llid24_rgstr_flg_sts, (val))
#define e_llid24_dscvry_sts_VAL_llid24_valid(val)              REG_FLD_VAL(e_llid24_dscvry_sts_FLD_llid24_valid, (val))
#define e_llid24_dscvry_sts_VAL_llid24_value(val)              REG_FLD_VAL(e_llid24_dscvry_sts_FLD_llid24_value, (val))

#define e_llid25_dscvry_sts_VAL_llid25_dscvry_sts(val)         REG_FLD_VAL(e_llid25_dscvry_sts_FLD_llid25_dscvry_sts, (val))
#define e_llid25_dscvry_sts_VAL_llid25_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid25_dscvry_sts_FLD_llid25_rgstr_flg_sts, (val))
#define e_llid25_dscvry_sts_VAL_llid25_valid(val)              REG_FLD_VAL(e_llid25_dscvry_sts_FLD_llid25_valid, (val))
#define e_llid25_dscvry_sts_VAL_llid25_value(val)              REG_FLD_VAL(e_llid25_dscvry_sts_FLD_llid25_value, (val))

#define e_llid26_dscvry_sts_VAL_llid26_dscvry_sts(val)         REG_FLD_VAL(e_llid26_dscvry_sts_FLD_llid26_dscvry_sts, (val))
#define e_llid26_dscvry_sts_VAL_llid26_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid26_dscvry_sts_FLD_llid26_rgstr_flg_sts, (val))
#define e_llid26_dscvry_sts_VAL_llid26_valid(val)              REG_FLD_VAL(e_llid26_dscvry_sts_FLD_llid26_valid, (val))
#define e_llid26_dscvry_sts_VAL_llid26_value(val)              REG_FLD_VAL(e_llid26_dscvry_sts_FLD_llid26_value, (val))

#define e_llid27_dscvry_sts_VAL_llid27_dscvry_sts(val)         REG_FLD_VAL(e_llid27_dscvry_sts_FLD_llid27_dscvry_sts, (val))
#define e_llid27_dscvry_sts_VAL_llid27_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid27_dscvry_sts_FLD_llid27_rgstr_flg_sts, (val))
#define e_llid27_dscvry_sts_VAL_llid27_valid(val)              REG_FLD_VAL(e_llid27_dscvry_sts_FLD_llid27_valid, (val))
#define e_llid27_dscvry_sts_VAL_llid27_value(val)              REG_FLD_VAL(e_llid27_dscvry_sts_FLD_llid27_value, (val))

#define e_llid28_dscvry_sts_VAL_llid28_dscvry_sts(val)         REG_FLD_VAL(e_llid28_dscvry_sts_FLD_llid28_dscvry_sts, (val))
#define e_llid28_dscvry_sts_VAL_llid28_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid28_dscvry_sts_FLD_llid28_rgstr_flg_sts, (val))
#define e_llid28_dscvry_sts_VAL_llid28_valid(val)              REG_FLD_VAL(e_llid28_dscvry_sts_FLD_llid28_valid, (val))
#define e_llid28_dscvry_sts_VAL_llid28_value(val)              REG_FLD_VAL(e_llid28_dscvry_sts_FLD_llid28_value, (val))

#define e_llid29_dscvry_sts_VAL_llid29_dscvry_sts(val)         REG_FLD_VAL(e_llid29_dscvry_sts_FLD_llid29_dscvry_sts, (val))
#define e_llid29_dscvry_sts_VAL_llid29_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid29_dscvry_sts_FLD_llid29_rgstr_flg_sts, (val))
#define e_llid29_dscvry_sts_VAL_llid29_valid(val)              REG_FLD_VAL(e_llid29_dscvry_sts_FLD_llid29_valid, (val))
#define e_llid29_dscvry_sts_VAL_llid29_value(val)              REG_FLD_VAL(e_llid29_dscvry_sts_FLD_llid29_value, (val))

#define e_llid30_dscvry_sts_VAL_llid30_dscvry_sts(val)         REG_FLD_VAL(e_llid30_dscvry_sts_FLD_llid30_dscvry_sts, (val))
#define e_llid30_dscvry_sts_VAL_llid30_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid30_dscvry_sts_FLD_llid30_rgstr_flg_sts, (val))
#define e_llid30_dscvry_sts_VAL_llid30_valid(val)              REG_FLD_VAL(e_llid30_dscvry_sts_FLD_llid30_valid, (val))
#define e_llid30_dscvry_sts_VAL_llid30_value(val)              REG_FLD_VAL(e_llid30_dscvry_sts_FLD_llid30_value, (val))

#define e_llid31_dscvry_sts_VAL_llid31_dscvry_sts(val)         REG_FLD_VAL(e_llid31_dscvry_sts_FLD_llid31_dscvry_sts, (val))
#define e_llid31_dscvry_sts_VAL_llid31_rgstr_flg_sts(val)      REG_FLD_VAL(e_llid31_dscvry_sts_FLD_llid31_rgstr_flg_sts, (val))
#define e_llid31_dscvry_sts_VAL_llid31_valid(val)              REG_FLD_VAL(e_llid31_dscvry_sts_FLD_llid31_valid, (val))
#define e_llid31_dscvry_sts_VAL_llid31_value(val)              REG_FLD_VAL(e_llid31_dscvry_sts_FLD_llid31_value, (val))

#define e_pending_gnt_num_VAL_echoed_pending_gnt(val)          REG_FLD_VAL(e_pending_gnt_num_FLD_echoed_pending_gnt, (val))
#define e_pending_gnt_num_VAL_pending_gnt_num(val)             REG_FLD_VAL(e_pending_gnt_num_FLD_pending_gnt_num, (val))

#define e_mac_addr_cfg_VAL_mac_addr_rwcmd(val)                 REG_FLD_VAL(e_mac_addr_cfg_FLD_mac_addr_rwcmd, (val))
#define e_mac_addr_cfg_VAL_mac_addr_rwcmd_done(val)            REG_FLD_VAL(e_mac_addr_cfg_FLD_mac_addr_rwcmd_done, (val))
#define e_mac_addr_cfg_VAL_mac_addr_llid_indx(val)             REG_FLD_VAL(e_mac_addr_cfg_FLD_mac_addr_llid_indx, (val))
#define e_mac_addr_cfg_VAL_mac_addr_dw_idx(val)                REG_FLD_VAL(e_mac_addr_cfg_FLD_mac_addr_dw_idx, (val))

#define e_mac_addr_value_VAL_mac_addr_value(val)               REG_FLD_VAL(e_mac_addr_value_FLD_mac_addr_value, (val))

#define e_security_key_cfg_VAL_key_rwcmd(val)                  REG_FLD_VAL(e_security_key_cfg_FLD_key_rwcmd, (val))
#define e_security_key_cfg_VAL_key_macsec_an(val)              REG_FLD_VAL(e_security_key_cfg_FLD_key_macsec_an, (val))
#define e_security_key_cfg_VAL_key_rwcmd_done(val)             REG_FLD_VAL(e_security_key_cfg_FLD_key_rwcmd_done, (val))
#define e_security_key_cfg_VAL_key_llid_index(val)             REG_FLD_VAL(e_security_key_cfg_FLD_key_llid_index, (val))
#define e_security_key_cfg_VAL_key_idx(val)                    REG_FLD_VAL(e_security_key_cfg_FLD_key_idx, (val))
#define e_security_key_cfg_VAL_key_dw_indx(val)                REG_FLD_VAL(e_security_key_cfg_FLD_key_dw_indx, (val))

#define e_key_value_VAL_key_value(val)                         REG_FLD_VAL(e_key_value_FLD_key_value, (val))

#define e_enckey_cfg_VAL_enckey_rwcmd(val)                     REG_FLD_VAL(e_enckey_cfg_FLD_enckey_rwcmd, (val))
#define e_enckey_cfg_VAL_enckey_macsec_an(val)                 REG_FLD_VAL(e_enckey_cfg_FLD_enckey_macsec_an, (val))
#define e_enckey_cfg_VAL_enckey_rwcmd_done(val)                REG_FLD_VAL(e_enckey_cfg_FLD_enckey_rwcmd_done, (val))
#define e_enckey_cfg_VAL_enckey_llididx(val)                   REG_FLD_VAL(e_enckey_cfg_FLD_enckey_llididx, (val))
#define e_enckey_cfg_VAL_enckey_keyidx(val)                    REG_FLD_VAL(e_enckey_cfg_FLD_enckey_keyidx, (val))
#define e_enckey_cfg_VAL_enckey_dwidx(val)                     REG_FLD_VAL(e_enckey_cfg_FLD_enckey_dwidx, (val))

#define e_enckey_val_VAL_enckey_value(val)                     REG_FLD_VAL(e_enckey_val_FLD_enckey_value, (val))

#define e_cfg_dmy5_VAL_xe_cfg_dmy5(val)                        REG_FLD_VAL(e_cfg_dmy5_FLD_xe_cfg_dmy5, (val))

#define e_rpt_cfg_VAL_llid15_rpt_cfg(val)                      REG_FLD_VAL(e_rpt_cfg_FLD_llid15_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid14_rpt_cfg(val)                      REG_FLD_VAL(e_rpt_cfg_FLD_llid14_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid13_rpt_cfg(val)                      REG_FLD_VAL(e_rpt_cfg_FLD_llid13_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid12_rpt_cfg(val)                      REG_FLD_VAL(e_rpt_cfg_FLD_llid12_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid11_rpt_cfg(val)                      REG_FLD_VAL(e_rpt_cfg_FLD_llid11_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid10_rpt_cfg(val)                      REG_FLD_VAL(e_rpt_cfg_FLD_llid10_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid9_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid9_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid8_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid8_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid7_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid7_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid6_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid6_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid5_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid5_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid4_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid4_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid3_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid3_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid2_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid2_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid1_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid1_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid0_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid0_rpt_cfg, (val))

#define e_rpt_cfg2_VAL_llid31_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid31_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid30_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid30_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid29_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid29_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid28_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid28_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid27_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid27_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid26_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid26_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid25_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid25_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid24_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid24_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid23_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid23_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid22_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid22_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid21_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid21_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid20_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid20_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid19_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid19_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid18_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid18_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid17_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid17_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid16_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid16_rpt_cfg, (val))

#define e_rpt_qthld_cfg_VAL_qthld_rwcmd(val)                   REG_FLD_VAL(e_rpt_qthld_cfg_FLD_qthld_rwcmd, (val))
#define e_rpt_qthld_cfg_VAL_qthld_rwcmd_done(val)              REG_FLD_VAL(e_rpt_qthld_cfg_FLD_qthld_rwcmd_done, (val))
#define e_rpt_qthld_cfg_VAL_rpt_llid_idx(val)                  REG_FLD_VAL(e_rpt_qthld_cfg_FLD_rpt_llid_idx, (val))
#define e_rpt_qthld_cfg_VAL_qthld_value(val)                   REG_FLD_VAL(e_rpt_qthld_cfg_FLD_qthld_value, (val))
#define e_rpt_qthld_cfg_VAL_qthld_idx(val)                     REG_FLD_VAL(e_rpt_qthld_cfg_FLD_qthld_idx, (val))
#define e_rpt_qthld_cfg_VAL_queue_idx(val)                     REG_FLD_VAL(e_rpt_qthld_cfg_FLD_queue_idx, (val))

#define e_rpt_glb_cfg_VAL_rpt_dmy1(val)                        REG_FLD_VAL(e_rpt_glb_cfg_FLD_rpt_dmy1, (val))
#define e_rpt_glb_cfg_VAL_dba_gtthr_frpt(val)                  REG_FLD_VAL(e_rpt_glb_cfg_FLD_dba_gtthr_frpt, (val))
#define e_rpt_glb_cfg_VAL_txgnt_cnthit_frpt(val)               REG_FLD_VAL(e_rpt_glb_cfg_FLD_txgnt_cnthit_frpt, (val))
#define e_rpt_glb_cfg_VAL_ctrlpkt_frpt(val)                    REG_FLD_VAL(e_rpt_glb_cfg_FLD_ctrlpkt_frpt, (val))
#define e_rpt_glb_cfg_VAL_gntfrpt_ignore(val)                  REG_FLD_VAL(e_rpt_glb_cfg_FLD_gntfrpt_ignore, (val))
#define e_rpt_glb_cfg_VAL_rpt_qsize_sp(val)                    REG_FLD_VAL(e_rpt_glb_cfg_FLD_rpt_qsize_sp, (val))
#define e_rpt_glb_cfg_VAL_rpt_fullqsize_head(val)              REG_FLD_VAL(e_rpt_glb_cfg_FLD_rpt_fullqsize_head, (val))
#define e_rpt_glb_cfg_VAL_rpt_dpoe_thrstop(val)                REG_FLD_VAL(e_rpt_glb_cfg_FLD_rpt_dpoe_thrstop, (val))
#define e_rpt_glb_cfg_VAL_rpt_dpoe_queidx(val)                 REG_FLD_VAL(e_rpt_glb_cfg_FLD_rpt_dpoe_queidx, (val))
#define e_rpt_glb_cfg_VAL_rpt_dpoe_en(val)                     REG_FLD_VAL(e_rpt_glb_cfg_FLD_rpt_dpoe_en, (val))
#define e_rpt_glb_cfg_VAL_rpt_qsize_dec(val)                   REG_FLD_VAL(e_rpt_glb_cfg_FLD_rpt_qsize_dec, (val))
#define e_rpt_glb_cfg_VAL_rpt_qsize_mgnt_sel(val)              REG_FLD_VAL(e_rpt_glb_cfg_FLD_rpt_qsize_mgnt_sel, (val))
#define e_rpt_glb_cfg_VAL_tx_default_rpt(val)                  REG_FLD_VAL(e_rpt_glb_cfg_FLD_tx_default_rpt, (val))

#define e_rpt_bmap_VAL_rpt_dmy2(val)                           REG_FLD_VAL(e_rpt_bmap_FLD_rpt_dmy2, (val))
#define e_rpt_bmap_VAL_rpt_bitmap_ctrl(val)                    REG_FLD_VAL(e_rpt_bmap_FLD_rpt_bitmap_ctrl, (val))
#define e_rpt_bmap_VAL_rpt_bitmap_set(val)                     REG_FLD_VAL(e_rpt_bmap_FLD_rpt_bitmap_set, (val))

#define e_frpt_thr_VAL_frpt_txgnt_cnt(val)                     REG_FLD_VAL(e_frpt_thr_FLD_frpt_txgnt_cnt, (val))
#define e_frpt_thr_VAL_frpt_dba_totsize_thr(val)               REG_FLD_VAL(e_frpt_thr_FLD_frpt_dba_totsize_thr, (val))

#define e_u1g_rpt_qsizeadj_VAL_u1g_fecon_rpt_qsizeadj(val)     REG_FLD_VAL(e_u1g_rpt_qsizeadj_FLD_u1g_fecon_rpt_qsizeadj, (val))
#define e_u1g_rpt_qsizeadj_VAL_u1g_fecoff_rpt_qsizeadj(val)    REG_FLD_VAL(e_u1g_rpt_qsizeadj_FLD_u1g_fecoff_rpt_qsizeadj, (val))

#define e_u10g_rpt_qsizeadj_VAL_u10g_rpt_qsizeadj(val)         REG_FLD_VAL(e_u10g_rpt_qsizeadj_FLD_u10g_rpt_qsizeadj, (val))

#define e_cfg_dmy7_VAL_xe_cfg_dmy7(val)                        REG_FLD_VAL(e_cfg_dmy7_FLD_xe_cfg_dmy7, (val))

#define e_cfg_dmy8_VAL_xe_cfg_dmy8(val)                        REG_FLD_VAL(e_cfg_dmy8_FLD_xe_cfg_dmy8, (val))

#define e_cfg_dmy9_VAL_xe_cfg_dmy9(val)                        REG_FLD_VAL(e_cfg_dmy9_FLD_xe_cfg_dmy9, (val))

#define e_cfg_dmy10_VAL_xe_cfg_dmy10(val)                      REG_FLD_VAL(e_cfg_dmy10_FLD_xe_cfg_dmy10, (val))

#define e_cfg_dmy11_VAL_xe_cfg_dmy11(val)                      REG_FLD_VAL(e_cfg_dmy11_FLD_xe_cfg_dmy11, (val))

#define e_cfg_dmy12_VAL_xe_cfg_dmy12(val)                      REG_FLD_VAL(e_cfg_dmy12_FLD_xe_cfg_dmy12, (val))

#define e_cfg_dmy13_VAL_xe_cfg_dmy13(val)                      REG_FLD_VAL(e_cfg_dmy13_FLD_xe_cfg_dmy13, (val))

#define e_cfg_dmy14_VAL_xe_cfg_dmy14(val)                      REG_FLD_VAL(e_cfg_dmy14_FLD_xe_cfg_dmy14, (val))

#define e_cfg_dmy15_VAL_xe_cfg_dmy15(val)                      REG_FLD_VAL(e_cfg_dmy15_FLD_xe_cfg_dmy15, (val))

#define e_dfrpt_data5_VAL_dfrpt_data5(val)                     REG_FLD_VAL(e_dfrpt_data5_FLD_dfrpt_data5, (val))

#define e_dfrpt_data6_VAL_dfrpt_data6(val)                     REG_FLD_VAL(e_dfrpt_data6_FLD_dfrpt_data6, (val))

#define e_dfrpt_data7_VAL_dfrpt_data7(val)                     REG_FLD_VAL(e_dfrpt_data7_FLD_dfrpt_data7, (val))

#define e_dfrpt_data8_VAL_dfrpt_data8(val)                     REG_FLD_VAL(e_dfrpt_data8_FLD_dfrpt_data8, (val))

#define e_dfrpt_data9_VAL_dfrpt_data9(val)                     REG_FLD_VAL(e_dfrpt_data9_FLD_dfrpt_data9, (val))

#define e_dfrpt_data10_VAL_dfrpt_data10(val)                   REG_FLD_VAL(e_dfrpt_data10_FLD_dfrpt_data10, (val))

#define e_dfrpt_data11_VAL_dfrpt_data11(val)                   REG_FLD_VAL(e_dfrpt_data11_FLD_dfrpt_data11, (val))

#define e_dfrpt_data12_VAL_dfrpt_data12(val)                   REG_FLD_VAL(e_dfrpt_data12_FLD_dfrpt_data12, (val))

#define e_dfrpt_data13_VAL_dfrpt_data13(val)                   REG_FLD_VAL(e_dfrpt_data13_FLD_dfrpt_data13, (val))

#define e_dfrpt_data14_VAL_dfrpt_data14(val)                   REG_FLD_VAL(e_dfrpt_data14_FLD_dfrpt_data14, (val))

#define e_laser_onoff_time_VAL_laser_off_time(val)             REG_FLD_VAL(e_laser_onoff_time_FLD_laser_off_time, (val))
#define e_laser_onoff_time_VAL_laser_on_time(val)              REG_FLD_VAL(e_laser_onoff_time_FLD_laser_on_time, (val))

#define e_sync_time_VAL_sync_time_updte(val)                   REG_FLD_VAL(e_sync_time_FLD_sync_time_updte, (val))
#define e_sync_time_VAL_sync_time(val)                         REG_FLD_VAL(e_sync_time_FLD_sync_time, (val))

#define e_overhead_time_thr_VAL_sync_time_maxen(val)           REG_FLD_VAL(e_overhead_time_thr_FLD_sync_time_maxen, (val))
#define e_overhead_time_thr_VAL_lsroff_time_maxen(val)         REG_FLD_VAL(e_overhead_time_thr_FLD_lsroff_time_maxen, (val))
#define e_overhead_time_thr_VAL_lsron_time_maxen(val)          REG_FLD_VAL(e_overhead_time_thr_FLD_lsron_time_maxen, (val))
#define e_overhead_time_thr_VAL_sync_time_maxval(val)          REG_FLD_VAL(e_overhead_time_thr_FLD_sync_time_maxval, (val))
#define e_overhead_time_thr_VAL_lsroff_time_maxval(val)        REG_FLD_VAL(e_overhead_time_thr_FLD_lsroff_time_maxval, (val))
#define e_overhead_time_thr_VAL_lsron_time_maxval(val)         REG_FLD_VAL(e_overhead_time_thr_FLD_lsron_time_maxval, (val))

#define e_laser_onoff_time2_VAL_olt_lsroff_time(val)           REG_FLD_VAL(e_laser_onoff_time2_FLD_olt_lsroff_time, (val))
#define e_laser_onoff_time2_VAL_olt_lsron_time(val)            REG_FLD_VAL(e_laser_onoff_time2_FLD_olt_lsron_time, (val))
#define e_laser_onoff_time2_VAL_olt_sync_time(val)             REG_FLD_VAL(e_laser_onoff_time2_FLD_olt_sync_time, (val))

#define e_olt_dscvinfo_VAL_olt_dscvinfo_match(val)             REG_FLD_VAL(e_olt_dscvinfo_FLD_olt_dscvinfo_match, (val))
#define e_olt_dscvinfo_VAL_olt_dscvinfo_mis(val)               REG_FLD_VAL(e_olt_dscvinfo_FLD_olt_dscvinfo_mis, (val))

#define e_grd_thrshld_VAL_guard_thrshld(val)                   REG_FLD_VAL(e_grd_thrshld_FLD_guard_thrshld, (val))

#define e_mpcp_timeout_intvl_VAL_mpcp_timeout_intvl(val)       REG_FLD_VAL(e_mpcp_timeout_intvl_FLD_mpcp_timeout_intvl, (val))

#define e_rpt_timeout_intvl_VAL_rpt_timeout_intvl(val)         REG_FLD_VAL(e_rpt_timeout_intvl_FLD_rpt_timeout_intvl, (val))

#define e_max_future_gnt_time_VAL_max_future_gnt_time(val)     REG_FLD_VAL(e_max_future_gnt_time_FLD_max_future_gnt_time, (val))

#define e_min_proc_time_VAL_min_proc_time(val)                 REG_FLD_VAL(e_min_proc_time_FLD_min_proc_time, (val))

#define e_trx_adjust_time1_VAL_tx_stm_adj(val)                 REG_FLD_VAL(e_trx_adjust_time1_FLD_tx_stm_adj, (val))

#define e_trx_adjust_time2_VAL_tx_len_adj(val)                 REG_FLD_VAL(e_trx_adjust_time2_FLD_tx_len_adj, (val))
#define e_trx_adjust_time2_VAL_rx_tmstp_adj(val)               REG_FLD_VAL(e_trx_adjust_time2_FLD_rx_tmstp_adj, (val))

#define e_trx_adjtime3_VAL_u10g_tx_tsadj(val)                  REG_FLD_VAL(e_trx_adjtime3_FLD_u10g_tx_tsadj, (val))
#define e_trx_adjtime3_VAL_u10g_tx_stmadj(val)                 REG_FLD_VAL(e_trx_adjtime3_FLD_u10g_tx_stmadj, (val))

#define e_trx_adjtime4_VAL_d10g_rx_tsadj(val)                  REG_FLD_VAL(e_trx_adjtime4_FLD_d10g_rx_tsadj, (val))
#define e_trx_adjtime4_VAL_d10g_rx_tsadj_ofst(val)             REG_FLD_VAL(e_trx_adjtime4_FLD_d10g_rx_tsadj_ofst, (val))

#define e_trx_adjtime5_VAL_u1g_tx_tsadj(val)                   REG_FLD_VAL(e_trx_adjtime5_FLD_u1g_tx_tsadj, (val))

#define e_rxphydly_adjtime_VAL_rx_phydly_dft(val)              REG_FLD_VAL(e_rxphydly_adjtime_FLD_rx_phydly_dft, (val))
#define e_rxphydly_adjtime_VAL_d1g_rx_phydly_ofst(val)         REG_FLD_VAL(e_rxphydly_adjtime_FLD_d1g_rx_phydly_ofst, (val))
#define e_rxphydly_adjtime_VAL_d1g_rx_phydly(val)              REG_FLD_VAL(e_rxphydly_adjtime_FLD_d1g_rx_phydly, (val))
#define e_rxphydly_adjtime_VAL_d10g_rx_phydly_ofst(val)        REG_FLD_VAL(e_rxphydly_adjtime_FLD_d10g_rx_phydly_ofst, (val))
#define e_rxphydly_adjtime_VAL_d10g_rx_phydly(val)             REG_FLD_VAL(e_rxphydly_adjtime_FLD_d10g_rx_phydly, (val))

#define e_txfetch_cfg_VAL_tx_fetch_leadtime(val)               REG_FLD_VAL(e_txfetch_cfg_FLD_tx_fetch_leadtime, (val))
#define e_txfetch_cfg_VAL_tx_dma_leadtime(val)                 REG_FLD_VAL(e_txfetch_cfg_FLD_tx_dma_leadtime, (val))

#define e_tx_cal_cnst_VAL_dscvr_gnt_len(val)                   REG_FLD_VAL(e_tx_cal_cnst_FLD_dscvr_gnt_len, (val))
#define e_tx_cal_cnst_VAL_fec_tail_grd(val)                    REG_FLD_VAL(e_tx_cal_cnst_FLD_fec_tail_grd, (val))
#define e_tx_cal_cnst_VAL_tail_grd(val)                        REG_FLD_VAL(e_tx_cal_cnst_FLD_tail_grd, (val))
#define e_tx_cal_cnst_VAL_default_ovrhd(val)                   REG_FLD_VAL(e_tx_cal_cnst_FLD_default_ovrhd, (val))

#define e_txcal_cnst2_VAL_ipgalign_mtd(val)                    REG_FLD_VAL(e_txcal_cnst2_FLD_ipgalign_mtd, (val))
#define e_txcal_cnst2_VAL_rpt_feccal_cnt_multival(val)         REG_FLD_VAL(e_txcal_cnst2_FLD_rpt_feccal_cnt_multival, (val))
#define e_txcal_cnst2_VAL_u10g_dscv_gntlen(val)                REG_FLD_VAL(e_txcal_cnst2_FLD_u10g_dscv_gntlen, (val))
#define e_txcal_cnst2_VAL_u10g_tail_grd(val)                   REG_FLD_VAL(e_txcal_cnst2_FLD_u10g_tail_grd, (val))

#define e_txcal_cnst3_VAL_u1g_fecon_min_gntlen(val)            REG_FLD_VAL(e_txcal_cnst3_FLD_u1g_fecon_min_gntlen, (val))
#define e_txcal_cnst3_VAL_u10g_eoblen(val)                     REG_FLD_VAL(e_txcal_cnst3_FLD_u10g_eoblen, (val))
#define e_txcal_cnst3_VAL_u10g_min_gntlen(val)                 REG_FLD_VAL(e_txcal_cnst3_FLD_u10g_min_gntlen, (val))
#define e_txcal_cnst3_VAL_u1g_fecoff_min_gntlen(val)           REG_FLD_VAL(e_txcal_cnst3_FLD_u1g_fecoff_min_gntlen, (val))

#define e_txsch_cfg_VAL_txfifo_wr_thren(val)                   REG_FLD_VAL(e_txsch_cfg_FLD_txfifo_wr_thren, (val))
#define e_txsch_cfg_VAL_txfifo_wr_thr(val)                     REG_FLD_VAL(e_txsch_cfg_FLD_txfifo_wr_thr, (val))
#define e_txsch_cfg_VAL_txsch_dmy0(val)                        REG_FLD_VAL(e_txsch_cfg_FLD_txsch_dmy0, (val))
#define e_txsch_cfg_VAL_txfifo_pad_hthr(val)                   REG_FLD_VAL(e_txsch_cfg_FLD_txfifo_pad_hthr, (val))
#define e_txsch_cfg_VAL_txsch_dmy1(val)                        REG_FLD_VAL(e_txsch_cfg_FLD_txsch_dmy1, (val))
#define e_txsch_cfg_VAL_txfifo_pad_lthr(val)                   REG_FLD_VAL(e_txsch_cfg_FLD_txfifo_pad_lthr, (val))

#define e_rxfifo_thr_VAL_rx_dmy1(val)                          REG_FLD_VAL(e_rxfifo_thr_FLD_rx_dmy1, (val))
#define e_rxfifo_thr_VAL_rx_dmy0(val)                          REG_FLD_VAL(e_rxfifo_thr_FLD_rx_dmy0, (val))
#define e_rxfifo_thr_VAL_rxfifo_mbithr(val)                    REG_FLD_VAL(e_rxfifo_thr_FLD_rxfifo_mbithr, (val))

#define e_bcllid_cfg_VAL_d10g_bcllid(val)                      REG_FLD_VAL(e_bcllid_cfg_FLD_d10g_bcllid, (val))
#define e_bcllid_cfg_VAL_d1g_bcllid(val)                       REG_FLD_VAL(e_bcllid_cfg_FLD_d1g_bcllid, (val))

#define e_txfrm_cfg1_VAL_tx_mpcp_addrl(val)                    REG_FLD_VAL(e_txfrm_cfg1_FLD_tx_mpcp_addrl, (val))

#define e_txfrm_cfg2_VAL_tx_mpcp_addrh(val)                    REG_FLD_VAL(e_txfrm_cfg2_FLD_tx_mpcp_addrh, (val))
#define e_txfrm_cfg2_VAL_tx_mpcp_etype(val)                    REG_FLD_VAL(e_txfrm_cfg2_FLD_tx_mpcp_etype, (val))

#define e_txfrm_cfg3_VAL_tx_rgreq_op(val)                      REG_FLD_VAL(e_txfrm_cfg3_FLD_tx_rgreq_op, (val))
#define e_txfrm_cfg3_VAL_tx_rgack_op(val)                      REG_FLD_VAL(e_txfrm_cfg3_FLD_tx_rgack_op, (val))

#define e_tod_sync_x_VAL_tod_sync_x(val)                       REG_FLD_VAL(e_tod_sync_x_FLD_tod_sync_x, (val))

#define e_tod_ltncy_VAL_rl_rx_phydly_ofst(val)                 REG_FLD_VAL(e_tod_ltncy_FLD_rl_rx_phydly_ofst, (val))
#define e_tod_ltncy_VAL_ingrs_latency(val)                     REG_FLD_VAL(e_tod_ltncy_FLD_ingrs_latency, (val))
#define e_tod_ltncy_VAL_egrs_latency(val)                      REG_FLD_VAL(e_tod_ltncy_FLD_egrs_latency, (val))

#define e_new_tod_p2p_offset_sec_l32_VAL_new_tod_p2p_offset_sec_l32(val) REG_FLD_VAL(e_new_tod_p2p_offset_sec_l32_FLD_new_tod_p2p_offset_sec_l32, (val))

#define e_new_tod_p2p_tod_offset_nsec_VAL_new_tod_nsec(val)    REG_FLD_VAL(e_new_tod_p2p_tod_offset_nsec_FLD_new_tod_nsec, (val))

#define e_tod_p2p_tod_sec_l32_VAL_tod_p2p_sec_l32(val)         REG_FLD_VAL(e_tod_p2p_tod_sec_l32_FLD_tod_p2p_sec_l32, (val))

#define e_tod_p2p_tod_nsec_VAL_tod_p2p_nsec(val)               REG_FLD_VAL(e_tod_p2p_tod_nsec_FLD_tod_p2p_nsec, (val))

#define e_tod_period_VAL_tod_period(val)                       REG_FLD_VAL(e_tod_period_FLD_tod_period, (val))

#define e_tod_1pps_ctrl_VAL_tod_1pps_width_ctrl(val)           REG_FLD_VAL(e_tod_1pps_ctrl_FLD_tod_1pps_width_ctrl, (val))

#define e_pwr_sv_cfg_VAL_pwd_mode(val)                         REG_FLD_VAL(e_pwr_sv_cfg_FLD_pwd_mode, (val))
#define e_pwr_sv_cfg_VAL_timedrift_ignore(val)                 REG_FLD_VAL(e_pwr_sv_cfg_FLD_timedrift_ignore, (val))
#define e_pwr_sv_cfg_VAL_onu_wakeup(val)                       REG_FLD_VAL(e_pwr_sv_cfg_FLD_onu_wakeup, (val))
#define e_pwr_sv_cfg_VAL_pwr_sv_start(val)                     REG_FLD_VAL(e_pwr_sv_cfg_FLD_pwr_sv_start, (val))
#define e_pwr_sv_cfg_VAL_pwd_mode_i(val)                       REG_FLD_VAL(e_pwr_sv_cfg_FLD_pwd_mode_i, (val))
#define e_pwr_sv_cfg_VAL_slp_duration_max_h(val)               REG_FLD_VAL(e_pwr_sv_cfg_FLD_slp_duration_max_h, (val))

#define e_slp_durt_max_VAL_slp_duration_max_l(val)             REG_FLD_VAL(e_slp_durt_max_FLD_slp_duration_max_l, (val))

#define e_slp_duration_VAL_slp_duration(val)                   REG_FLD_VAL(e_slp_duration_FLD_slp_duration, (val))

#define e_act_duration_VAL_act_duration(val)                   REG_FLD_VAL(e_act_duration_FLD_act_duration, (val))

#define e_pwron_dly_VAL_pwron_dly(val)                         REG_FLD_VAL(e_pwron_dly_FLD_pwron_dly, (val))

#define e_slp_duration_i_VAL_rx_slpalw_pwd_mode(val)           REG_FLD_VAL(e_slp_duration_i_FLD_rx_slpalw_pwd_mode, (val))
#define e_slp_duration_i_VAL_rx_slpalw_flag(val)               REG_FLD_VAL(e_slp_duration_i_FLD_rx_slpalw_flag, (val))
#define e_slp_duration_i_VAL_rx_slpalw_duration(val)           REG_FLD_VAL(e_slp_duration_i_FLD_rx_slpalw_duration, (val))

#define e_oui_a_cfg_VAL_oui_a_en(val)                          REG_FLD_VAL(e_oui_a_cfg_FLD_oui_a_en, (val))
#define e_oui_a_cfg_VAL_oui_a_dmy(val)                         REG_FLD_VAL(e_oui_a_cfg_FLD_oui_a_dmy, (val))
#define e_oui_a_cfg_VAL_oui_a_val(val)                         REG_FLD_VAL(e_oui_a_cfg_FLD_oui_a_val, (val))

#define e_oui_b_cfg_VAL_oui_b_en(val)                          REG_FLD_VAL(e_oui_b_cfg_FLD_oui_b_en, (val))
#define e_oui_b_cfg_VAL_oui_b_dmy(val)                         REG_FLD_VAL(e_oui_b_cfg_FLD_oui_b_dmy, (val))
#define e_oui_b_cfg_VAL_oui_b_val(val)                         REG_FLD_VAL(e_oui_b_cfg_FLD_oui_b_val, (val))

#define e_oui_c_cfg_VAL_oui_c_en(val)                          REG_FLD_VAL(e_oui_c_cfg_FLD_oui_c_en, (val))
#define e_oui_c_cfg_VAL_oui_c_dmy(val)                         REG_FLD_VAL(e_oui_c_cfg_FLD_oui_c_dmy, (val))
#define e_oui_c_cfg_VAL_oui_c_val(val)                         REG_FLD_VAL(e_oui_c_cfg_FLD_oui_c_val, (val))

#define e_dyinggsp_cfg_VAL_hw_dygasp_en(val)                   REG_FLD_VAL(e_dyinggsp_cfg_FLD_hw_dygasp_en, (val))
#define e_dyinggsp_cfg_VAL_sw_init_dygasp(val)                 REG_FLD_VAL(e_dyinggsp_cfg_FLD_sw_init_dygasp, (val))
#define e_dyinggsp_cfg_VAL_dygasp_num_of_times(val)            REG_FLD_VAL(e_dyinggsp_cfg_FLD_dygasp_num_of_times, (val))

#define e_dyinggsp_w1_VAL_dyinggsp_w1_len_type(val)            REG_FLD_VAL(e_dyinggsp_w1_FLD_dyinggsp_w1_len_type, (val))

#define e_dyinggsp_w2_VAL_dyinggsp_w2(val)                     REG_FLD_VAL(e_dyinggsp_w2_FLD_dyinggsp_w2, (val))

#define e_dyinggsp_w3_VAL_dyinggsp_w3(val)                     REG_FLD_VAL(e_dyinggsp_w3_FLD_dyinggsp_w3, (val))

#define e_dyinggsp_w4_VAL_dyinggsp_w4(val)                     REG_FLD_VAL(e_dyinggsp_w4_FLD_dyinggsp_w4, (val))

#define e_dyinggsp_w5_VAL_dyinggsp_w5(val)                     REG_FLD_VAL(e_dyinggsp_w5_FLD_dyinggsp_w5, (val))

#define e_dyinggsp_w6_VAL_dyinggsp_w6(val)                     REG_FLD_VAL(e_dyinggsp_w6_FLD_dyinggsp_w6, (val))

#define e_dyinggsp_w7_VAL_dyinggsp_w7(val)                     REG_FLD_VAL(e_dyinggsp_w7_FLD_dyinggsp_w7, (val))

#define e_dyinggsp_w8_VAL_dyinggsp_w8(val)                     REG_FLD_VAL(e_dyinggsp_w8_FLD_dyinggsp_w8, (val))

#define e_dyinggsp_w9_VAL_dyinggsp_w9(val)                     REG_FLD_VAL(e_dyinggsp_w9_FLD_dyinggsp_w9, (val))

#define e_dyinggsp_w10_VAL_dyinggsp_w10(val)                   REG_FLD_VAL(e_dyinggsp_w10_FLD_dyinggsp_w10, (val))

#define e_dyinggsp_w11_VAL_dyinggsp_w11(val)                   REG_FLD_VAL(e_dyinggsp_w11_FLD_dyinggsp_w11, (val))

#define e_dyinggsp_w12_VAL_dyinggsp_w12(val)                   REG_FLD_VAL(e_dyinggsp_w12_FLD_dyinggsp_w12, (val))

#define e_oam_kpalv_ctrl_VAL_oam_kpalv_llid_idx(val)           REG_FLD_VAL(e_oam_kpalv_ctrl_FLD_oam_kpalv_llid_idx, (val))
#define e_oam_kpalv_ctrl_VAL_oam_kpalv_interval(val)           REG_FLD_VAL(e_oam_kpalv_ctrl_FLD_oam_kpalv_interval, (val))
#define e_oam_kpalv_ctrl_VAL_oam_kpalv_sw_trig(val)            REG_FLD_VAL(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_trig, (val))
#define e_oam_kpalv_ctrl_VAL_oam_kpalv_sw_cfg(val)             REG_FLD_VAL(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_cfg, (val))
#define e_oam_kpalv_ctrl_VAL_oam_kpalv_en(val)                 REG_FLD_VAL(e_oam_kpalv_ctrl_FLD_oam_kpalv_en, (val))

#define e_oam_kpalv_w1_VAL_oam_kpalv_w1(val)                   REG_FLD_VAL(e_oam_kpalv_w1_FLD_oam_kpalv_w1, (val))

#define e_oam_kpalv_w2_VAL_oam_kpalv_w2(val)                   REG_FLD_VAL(e_oam_kpalv_w2_FLD_oam_kpalv_w2, (val))

#define e_oam_kpalv_w3_VAL_oam_kpalv_w3(val)                   REG_FLD_VAL(e_oam_kpalv_w3_FLD_oam_kpalv_w3, (val))

#define e_oam_kpalv_w4_VAL_oam_kpalv_w4(val)                   REG_FLD_VAL(e_oam_kpalv_w4_FLD_oam_kpalv_w4, (val))

#define e_oam_kpalv_w5_VAL_oam_kpalv_w5(val)                   REG_FLD_VAL(e_oam_kpalv_w5_FLD_oam_kpalv_w5, (val))

#define e_oam_kpalv_w6_VAL_oam_kpalv_w6(val)                   REG_FLD_VAL(e_oam_kpalv_w6_FLD_oam_kpalv_w6, (val))

#define e_oam_kpalv_w7_VAL_oam_kpalv_w7(val)                   REG_FLD_VAL(e_oam_kpalv_w7_FLD_oam_kpalv_w7, (val))

#define e_oam_kpalv_w8_VAL_oam_kpalv_w8(val)                   REG_FLD_VAL(e_oam_kpalv_w8_FLD_oam_kpalv_w8, (val))

#define e_oam_kpalv_w9_VAL_oam_kpalv_w9(val)                   REG_FLD_VAL(e_oam_kpalv_w9_FLD_oam_kpalv_w9, (val))

#define e_oam_kpalv_w10_VAL_oam_kpalv_w10(val)                 REG_FLD_VAL(e_oam_kpalv_w10_FLD_oam_kpalv_w10, (val))

#define e_oam_kpalv_w11_VAL_oam_kpalv_w11(val)                 REG_FLD_VAL(e_oam_kpalv_w11_FLD_oam_kpalv_w11, (val))

#define e_oam_kpalv_w12_VAL_oam_kpalv_w12(val)                 REG_FLD_VAL(e_oam_kpalv_w12_FLD_oam_kpalv_w12, (val))

#define e_kpalv_hwen_sts_VAL_kpalv_hwen_sts(val)               REG_FLD_VAL(e_kpalv_hwen_sts_FLD_kpalv_hwen_sts, (val))

#define e_mcgrp_en_VAL_mcgrp_llid_en(val)                      REG_FLD_VAL(e_mcgrp_en_FLD_mcgrp_llid_en, (val))

#define e_rxuni_mcllid_cfg0_VAL_mcgrp_llid1_val(val)           REG_FLD_VAL(e_rxuni_mcllid_cfg0_FLD_mcgrp_llid1_val, (val))
#define e_rxuni_mcllid_cfg0_VAL_mcgrp_llid0_val(val)           REG_FLD_VAL(e_rxuni_mcllid_cfg0_FLD_mcgrp_llid0_val, (val))

#define e_rxuni_mcllid_cfg1_VAL_mcgrp_llid3_val(val)           REG_FLD_VAL(e_rxuni_mcllid_cfg1_FLD_mcgrp_llid3_val, (val))
#define e_rxuni_mcllid_cfg1_VAL_mcgrp_llid2_val(val)           REG_FLD_VAL(e_rxuni_mcllid_cfg1_FLD_mcgrp_llid2_val, (val))

#define e_rxuni_mcllid_cfg2_VAL_mcgrp_llid5_val(val)           REG_FLD_VAL(e_rxuni_mcllid_cfg2_FLD_mcgrp_llid5_val, (val))
#define e_rxuni_mcllid_cfg2_VAL_mcgrp_llid4_val(val)           REG_FLD_VAL(e_rxuni_mcllid_cfg2_FLD_mcgrp_llid4_val, (val))

#define e_rxuni_mcllid_cfg3_VAL_mcgrp_llid7_val(val)           REG_FLD_VAL(e_rxuni_mcllid_cfg3_FLD_mcgrp_llid7_val, (val))
#define e_rxuni_mcllid_cfg3_VAL_mcgrp_llid6_val(val)           REG_FLD_VAL(e_rxuni_mcllid_cfg3_FLD_mcgrp_llid6_val, (val))

#define e_rxuni_mcllid_cfg4_VAL_mcgrp_llid9_val(val)           REG_FLD_VAL(e_rxuni_mcllid_cfg4_FLD_mcgrp_llid9_val, (val))
#define e_rxuni_mcllid_cfg4_VAL_mcgrp_llid8_val(val)           REG_FLD_VAL(e_rxuni_mcllid_cfg4_FLD_mcgrp_llid8_val, (val))

#define e_rxuni_mcllid_cfg5_VAL_mcgrp_llid11_val(val)          REG_FLD_VAL(e_rxuni_mcllid_cfg5_FLD_mcgrp_llid11_val, (val))
#define e_rxuni_mcllid_cfg5_VAL_mcgrp_llid10_val(val)          REG_FLD_VAL(e_rxuni_mcllid_cfg5_FLD_mcgrp_llid10_val, (val))

#define e_rxuni_mcllid_cfg6_VAL_mcgrp_llid13_val(val)          REG_FLD_VAL(e_rxuni_mcllid_cfg6_FLD_mcgrp_llid13_val, (val))
#define e_rxuni_mcllid_cfg6_VAL_mcgrp_llid12_val(val)          REG_FLD_VAL(e_rxuni_mcllid_cfg6_FLD_mcgrp_llid12_val, (val))

#define e_rxuni_mcllid_cfg7_VAL_mcgrp_llid15_val(val)          REG_FLD_VAL(e_rxuni_mcllid_cfg7_FLD_mcgrp_llid15_val, (val))
#define e_rxuni_mcllid_cfg7_VAL_mcgrp_llid14_val(val)          REG_FLD_VAL(e_rxuni_mcllid_cfg7_FLD_mcgrp_llid14_val, (val))

#define e_crpt_cfg_VAL_crpt_dmy0(val)                          REG_FLD_VAL(e_crpt_cfg_FLD_crpt_dmy0, (val))
#define e_crpt_cfg_VAL_kpalv_encrpt_en(val)                    REG_FLD_VAL(e_crpt_cfg_FLD_kpalv_encrpt_en, (val))
#define e_crpt_cfg_VAL_mpcp_encrpt_en(val)                     REG_FLD_VAL(e_crpt_cfg_FLD_mpcp_encrpt_en, (val))
#define e_crpt_cfg_VAL_dygasp_encrpt_en(val)                   REG_FLD_VAL(e_crpt_cfg_FLD_dygasp_encrpt_en, (val))
#define e_crpt_cfg_VAL_llidalluc_decrpt_en(val)                REG_FLD_VAL(e_crpt_cfg_FLD_llidalluc_decrpt_en, (val))
#define e_crpt_cfg_VAL_llidmc_decrpt_en(val)                   REG_FLD_VAL(e_crpt_cfg_FLD_llidmc_decrpt_en, (val))
#define e_crpt_cfg_VAL_llidbc_decrpt_en(val)                   REG_FLD_VAL(e_crpt_cfg_FLD_llidbc_decrpt_en, (val))
#define e_crpt_cfg_VAL_mcgrp_decrpt_en(val)                    REG_FLD_VAL(e_crpt_cfg_FLD_mcgrp_decrpt_en, (val))
#define e_crpt_cfg_VAL_crpt_dmy1(val)                          REG_FLD_VAL(e_crpt_cfg_FLD_crpt_dmy1, (val))
#define e_crpt_cfg_VAL_encrpt_mode(val)                        REG_FLD_VAL(e_crpt_cfg_FLD_encrpt_mode, (val))
#define e_crpt_cfg_VAL_crcchk_mode(val)                        REG_FLD_VAL(e_crpt_cfg_FLD_crcchk_mode, (val))
#define e_crpt_cfg_VAL_decrpt_mode(val)                        REG_FLD_VAL(e_crpt_cfg_FLD_decrpt_mode, (val))

#define e_cfbsec_cfg_VAL_cfbsec_dmy(val)                       REG_FLD_VAL(e_cfbsec_cfg_FLD_cfbsec_dmy, (val))
#define e_cfbsec_cfg_VAL_cfbdec_iv_srcsel(val)                 REG_FLD_VAL(e_cfbsec_cfg_FLD_cfbdec_iv_srcsel, (val))
#define e_cfbsec_cfg_VAL_cfbdec_padd_mode(val)                 REG_FLD_VAL(e_cfbsec_cfg_FLD_cfbdec_padd_mode, (val))

#define e_ctrsec_cfg_VAL_ctrsec_dmy0(val)                      REG_FLD_VAL(e_ctrsec_cfg_FLD_ctrsec_dmy0, (val))
#define e_ctrsec_cfg_VAL_ctrenc_padd_mode(val)                 REG_FLD_VAL(e_ctrsec_cfg_FLD_ctrenc_padd_mode, (val))
#define e_ctrsec_cfg_VAL_ctrsec_dmy1(val)                      REG_FLD_VAL(e_ctrsec_cfg_FLD_ctrsec_dmy1, (val))
#define e_ctrsec_cfg_VAL_ctrdec_srcaddr_sel(val)               REG_FLD_VAL(e_ctrsec_cfg_FLD_ctrdec_srcaddr_sel, (val))
#define e_ctrsec_cfg_VAL_ctrdec_padd_mode(val)                 REG_FLD_VAL(e_ctrsec_cfg_FLD_ctrdec_padd_mode, (val))

#define e_desec_srcaddr_cfg_VAL_desec_srcaddr_rwcmd(val)       REG_FLD_VAL(e_desec_srcaddr_cfg_FLD_desec_srcaddr_rwcmd, (val))
#define e_desec_srcaddr_cfg_VAL_desec_srcaddr_rwcmd_done(val)  REG_FLD_VAL(e_desec_srcaddr_cfg_FLD_desec_srcaddr_rwcmd_done, (val))
#define e_desec_srcaddr_cfg_VAL_desec_srcaddr_llid_idx(val)    REG_FLD_VAL(e_desec_srcaddr_cfg_FLD_desec_srcaddr_llid_idx, (val))
#define e_desec_srcaddr_cfg_VAL_desec_srcaddr_value_h(val)     REG_FLD_VAL(e_desec_srcaddr_cfg_FLD_desec_srcaddr_value_h, (val))

#define e_desec_srcaddr_val_VAL_desec_srcaddr_value_l(val)     REG_FLD_VAL(e_desec_srcaddr_val_FLD_desec_srcaddr_value_l, (val))

#define e_macsdec_cfg_VAL_macsdec_dmy0(val)                    REG_FLD_VAL(e_macsdec_cfg_FLD_macsdec_dmy0, (val))
#define e_macsdec_cfg_VAL_macsdec_tag_bit_swap(val)            REG_FLD_VAL(e_macsdec_cfg_FLD_macsdec_tag_bit_swap, (val))
#define e_macsdec_cfg_VAL_macsdec_iv_bit_swap(val)             REG_FLD_VAL(e_macsdec_cfg_FLD_macsdec_iv_bit_swap, (val))
#define e_macsdec_cfg_VAL_macsdec_aad_bit_swap(val)            REG_FLD_VAL(e_macsdec_cfg_FLD_macsdec_aad_bit_swap, (val))
#define e_macsdec_cfg_VAL_macsdec_pt_bit_swap(val)             REG_FLD_VAL(e_macsdec_cfg_FLD_macsdec_pt_bit_swap, (val))
#define e_macsdec_cfg_VAL_macsdec_ct_bit_swap(val)             REG_FLD_VAL(e_macsdec_cfg_FLD_macsdec_ct_bit_swap, (val))
#define e_macsdec_cfg_VAL_macsdec_srcaddr_sel(val)             REG_FLD_VAL(e_macsdec_cfg_FLD_macsdec_srcaddr_sel, (val))
#define e_macsdec_cfg_VAL_macsdec_timout_en(val)               REG_FLD_VAL(e_macsdec_cfg_FLD_macsdec_timout_en, (val))
#define e_macsdec_cfg_VAL_macsdec_conofst(val)                 REG_FLD_VAL(e_macsdec_cfg_FLD_macsdec_conofst, (val))
#define e_macsdec_cfg_VAL_sectag_e0c0_desec(val)               REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e0c0_desec, (val))
#define e_macsdec_cfg_VAL_sectag_e0c0_icvchk(val)              REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e0c0_icvchk, (val))
#define e_macsdec_cfg_VAL_sectag_e0c0_drop(val)                REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e0c0_drop, (val))
#define e_macsdec_cfg_VAL_sectag_e0c1_desec(val)               REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e0c1_desec, (val))
#define e_macsdec_cfg_VAL_sectag_e0c1_icvchk(val)              REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e0c1_icvchk, (val))
#define e_macsdec_cfg_VAL_sectag_e0c1_drop(val)                REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e0c1_drop, (val))
#define e_macsdec_cfg_VAL_sectag_e1c0_desec(val)               REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e1c0_desec, (val))
#define e_macsdec_cfg_VAL_sectag_e1c0_icvchk(val)              REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e1c0_icvchk, (val))
#define e_macsdec_cfg_VAL_sectag_e1c0_drop(val)                REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e1c0_drop, (val))
#define e_macsdec_cfg_VAL_sectag_e1c1_desec(val)               REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e1c1_desec, (val))
#define e_macsdec_cfg_VAL_sectag_e1c1_icvchk(val)              REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e1c1_icvchk, (val))
#define e_macsdec_cfg_VAL_sectag_e1c1_drop(val)                REG_FLD_VAL(e_macsdec_cfg_FLD_sectag_e1c1_drop, (val))

#define e_macsenc_cfg_VAL_macsenc_mode(val)                    REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_mode, (val))
#define e_macsenc_cfg_VAL_macsenc_dmy0(val)                    REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_dmy0, (val))
#define e_macsenc_cfg_VAL_macsenc_tag_bit_swap(val)            REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_tag_bit_swap, (val))
#define e_macsenc_cfg_VAL_macsenc_iv_bit_swap(val)             REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_iv_bit_swap, (val))
#define e_macsenc_cfg_VAL_macsenc_aad_bit_swap(val)            REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_aad_bit_swap, (val))
#define e_macsenc_cfg_VAL_macsenc_pt_bit_swap(val)             REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_pt_bit_swap, (val))
#define e_macsenc_cfg_VAL_macsenc_ct_bit_swap(val)             REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_ct_bit_swap, (val))
#define e_macsenc_cfg_VAL_macsenc_srcaddr_sel(val)             REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_srcaddr_sel, (val))
#define e_macsenc_cfg_VAL_macsenc_timout_en(val)               REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_timout_en, (val))
#define e_macsenc_cfg_VAL_macsenc_dmy1(val)                    REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_dmy1, (val))
#define e_macsenc_cfg_VAL_macsenc_tag_v(val)                   REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_tag_v, (val))
#define e_macsenc_cfg_VAL_macsenc_tag_es(val)                  REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_tag_es, (val))
#define e_macsenc_cfg_VAL_macsenc_tag_sc(val)                  REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_tag_sc, (val))
#define e_macsenc_cfg_VAL_macsenc_tag_scb(val)                 REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_tag_scb, (val))
#define e_macsenc_cfg_VAL_macsenc_tag_e(val)                   REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_tag_e, (val))
#define e_macsenc_cfg_VAL_macsenc_tag_c(val)                   REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_tag_c, (val))
#define e_macsenc_cfg_VAL_macsenc_dmy2(val)                    REG_FLD_VAL(e_macsenc_cfg_FLD_macsenc_dmy2, (val))

#define e_macsenc_pnini_cfg_VAL_macsenc_pn_inival(val)         REG_FLD_VAL(e_macsenc_pnini_cfg_FLD_macsenc_pn_inival, (val))

#define e_macsenc_pn_cfg_VAL_macsenc_pn_rwcmd(val)             REG_FLD_VAL(e_macsenc_pn_cfg_FLD_macsenc_pn_rwcmd, (val))
#define e_macsenc_pn_cfg_VAL_macsenc_pn_rwcmd_done(val)        REG_FLD_VAL(e_macsenc_pn_cfg_FLD_macsenc_pn_rwcmd_done, (val))
#define e_macsenc_pn_cfg_VAL_macsenc_pn_llid_idx(val)          REG_FLD_VAL(e_macsenc_pn_cfg_FLD_macsenc_pn_llid_idx, (val))

#define e_macsenc_pn_val_VAL_macsenc_pn_val(val)               REG_FLD_VAL(e_macsenc_pn_val_FLD_macsenc_pn_val, (val))

#define e_olt_macaddr_h_VAL_olt_macaddr_h(val)                 REG_FLD_VAL(e_olt_macaddr_h_FLD_olt_macaddr_h, (val))

#define e_olt_macaddr_l_VAL_olt_macaddr_l(val)                 REG_FLD_VAL(e_olt_macaddr_l_FLD_olt_macaddr_l, (val))

#define e_snf_cfg_VAL_sniffer_mode(val)                        REG_FLD_VAL(e_snf_cfg_FLD_sniffer_mode, (val))
#define e_snf_cfg_VAL_snf_rsv0(val)                            REG_FLD_VAL(e_snf_cfg_FLD_snf_rsv0, (val))
#define e_snf_cfg_VAL_llidinfo_snf(val)                        REG_FLD_VAL(e_snf_cfg_FLD_llidinfo_snf, (val))
#define e_snf_cfg_VAL_snf_fcserr_fwd(val)                      REG_FLD_VAL(e_snf_cfg_FLD_snf_fcserr_fwd, (val))
#define e_snf_cfg_VAL_snf_rsv1(val)                            REG_FLD_VAL(e_snf_cfg_FLD_snf_rsv1, (val))
#define e_snf_cfg_VAL_us_snf_mode(val)                         REG_FLD_VAL(e_snf_cfg_FLD_us_snf_mode, (val))
#define e_snf_cfg_VAL_us_kpalv_snf(val)                        REG_FLD_VAL(e_snf_cfg_FLD_us_kpalv_snf, (val))
#define e_snf_cfg_VAL_us_dygasp_snf(val)                       REG_FLD_VAL(e_snf_cfg_FLD_us_dygasp_snf, (val))
#define e_snf_cfg_VAL_us_rpt_snf(val)                          REG_FLD_VAL(e_snf_cfg_FLD_us_rpt_snf, (val))
#define e_snf_cfg_VAL_us_mpcp_snf(val)                         REG_FLD_VAL(e_snf_cfg_FLD_us_mpcp_snf, (val))
#define e_snf_cfg_VAL_us_oam_snf(val)                          REG_FLD_VAL(e_snf_cfg_FLD_us_oam_snf, (val))
#define e_snf_cfg_VAL_snf_rsv2(val)                            REG_FLD_VAL(e_snf_cfg_FLD_snf_rsv2, (val))
#define e_snf_cfg_VAL_mcgrp_eth_snf(val)                       REG_FLD_VAL(e_snf_cfg_FLD_mcgrp_eth_snf, (val))
#define e_snf_cfg_VAL_mcgrp_nrlgate_snf(val)                   REG_FLD_VAL(e_snf_cfg_FLD_mcgrp_nrlgate_snf, (val))
#define e_snf_cfg_VAL_mcgrp_oam_snf(val)                       REG_FLD_VAL(e_snf_cfg_FLD_mcgrp_oam_snf, (val))
#define e_snf_cfg_VAL_mcgrp_llid_snf(val)                      REG_FLD_VAL(e_snf_cfg_FLD_mcgrp_llid_snf, (val))
#define e_snf_cfg_VAL_alluc_eth_snf(val)                       REG_FLD_VAL(e_snf_cfg_FLD_alluc_eth_snf, (val))
#define e_snf_cfg_VAL_alluc_nrlgate_snf(val)                   REG_FLD_VAL(e_snf_cfg_FLD_alluc_nrlgate_snf, (val))
#define e_snf_cfg_VAL_alluc_oam_snf(val)                       REG_FLD_VAL(e_snf_cfg_FLD_alluc_oam_snf, (val))
#define e_snf_cfg_VAL_alluc_llid_snf(val)                      REG_FLD_VAL(e_snf_cfg_FLD_alluc_llid_snf, (val))
#define e_snf_cfg_VAL_snf_rsv3(val)                            REG_FLD_VAL(e_snf_cfg_FLD_snf_rsv3, (val))
#define e_snf_cfg_VAL_ds_nrlgate_snf(val)                      REG_FLD_VAL(e_snf_cfg_FLD_ds_nrlgate_snf, (val))
#define e_snf_cfg_VAL_ds_mpcp_snf(val)                         REG_FLD_VAL(e_snf_cfg_FLD_ds_mpcp_snf, (val))

#define e_snf_sp_tag_VAL_snf_sp_tag(val)                       REG_FLD_VAL(e_snf_sp_tag_FLD_snf_sp_tag, (val))

#define e_snf_dah_VAL_snf_pkt_dah(val)                         REG_FLD_VAL(e_snf_dah_FLD_snf_pkt_dah, (val))

#define e_snf_dal_VAL_snf_pkt_dal(val)                         REG_FLD_VAL(e_snf_dal_FLD_snf_pkt_dal, (val))
#define e_snf_dal_VAL_snf_pkt_sah(val)                         REG_FLD_VAL(e_snf_dal_FLD_snf_pkt_sah, (val))

#define e_snf_sal_VAL_snf_pkt_sal(val)                         REG_FLD_VAL(e_snf_sal_FLD_snf_pkt_sal, (val))

#define e_snf_etype_VAL_snf_pkt_etype(val)                     REG_FLD_VAL(e_snf_etype_FLD_snf_pkt_etype, (val))

#define e_rdmdly_cfg_VAL_rdmdly_mode(val)                      REG_FLD_VAL(e_rdmdly_cfg_FLD_rdmdly_mode, (val))
#define e_rdmdly_cfg_VAL_rdmdly_lthr(val)                      REG_FLD_VAL(e_rdmdly_cfg_FLD_rdmdly_lthr, (val))
#define e_rdmdly_cfg_VAL_rdmdly_hthr(val)                      REG_FLD_VAL(e_rdmdly_cfg_FLD_rdmdly_hthr, (val))

#define e_rdmdly_stat_VAL_dscvgate_gntlen(val)                 REG_FLD_VAL(e_rdmdly_stat_FLD_dscvgate_gntlen, (val))
#define e_rdmdly_stat_VAL_rdmdly(val)                          REG_FLD_VAL(e_rdmdly_stat_FLD_rdmdly, (val))

#define e_dbg_prb_sel_VAL_probe_dmysel(val)                    REG_FLD_VAL(e_dbg_prb_sel_FLD_probe_dmysel, (val))
#define e_dbg_prb_sel_VAL_probe_dtgrp_sel(val)                 REG_FLD_VAL(e_dbg_prb_sel_FLD_probe_dtgrp_sel, (val))
#define e_dbg_prb_sel_VAL_probe_bit0_sel(val)                  REG_FLD_VAL(e_dbg_prb_sel_FLD_probe_bit0_sel, (val))
#define e_dbg_prb_sel_VAL_probe_sel(val)                       REG_FLD_VAL(e_dbg_prb_sel_FLD_probe_sel, (val))

#define e_dbg_prb_h32_VAL_probe_h32(val)                       REG_FLD_VAL(e_dbg_prb_h32_FLD_probe_h32, (val))

#define e_dbg_prb_l32_VAL_probe_l32(val)                       REG_FLD_VAL(e_dbg_prb_l32_FLD_probe_l32, (val))

#define e_utili_cfg_VAL_cf_utili_wdm(val)                      REG_FLD_VAL(e_utili_cfg_FLD_cf_utili_wdm, (val))
#define e_utili_cfg_VAL_cf_utili_pktovhd(val)                  REG_FLD_VAL(e_utili_cfg_FLD_cf_utili_pktovhd, (val))
#define e_utili_cfg_VAL_cf_utili_rsv(val)                      REG_FLD_VAL(e_utili_cfg_FLD_cf_utili_rsv, (val))
#define e_utili_cfg_VAL_cf_utili_pkttype(val)                  REG_FLD_VAL(e_utili_cfg_FLD_cf_utili_pkttype, (val))
#define e_utili_cfg_VAL_cf_utili_mode(val)                     REG_FLD_VAL(e_utili_cfg_FLD_cf_utili_mode, (val))

#define e_utili_check_VAL_cf_utilization_cntclr(val)           REG_FLD_VAL(e_utili_check_FLD_cf_utilization_cntclr, (val))
#define e_utili_check_VAL_cf_utilization_chken(val)            REG_FLD_VAL(e_utili_check_FLD_cf_utilization_chken, (val))

#define total_gnt_sizeh_VAL_total_gnt_size_h32(val)            REG_FLD_VAL(total_gnt_sizeh_FLD_total_gnt_size_h32, (val))

#define total_gnt_sizel_VAL_total_gnt_size_l32(val)            REG_FLD_VAL(total_gnt_sizel_FLD_total_gnt_size_l32, (val))

#define total_pkt_cnt_VAL_total_pkt_cnt(val)                   REG_FLD_VAL(total_pkt_cnt_FLD_total_pkt_cnt, (val))

#define total_pkt_size_l_VAL_total_pkt_size_l32(val)           REG_FLD_VAL(total_pkt_size_l_FLD_total_pkt_size_l32, (val))

#define total_pkt_size_h_VAL_total_pkt_size_h32(val)           REG_FLD_VAL(total_pkt_size_h_FLD_total_pkt_size_h32, (val))

#define total_u10g_nouse_gntlen_VAL_total_u10g_nouse_gntlen(val) REG_FLD_VAL(total_u10g_nouse_gntlen_FLD_total_u10g_nouse_gntlen, (val))

#define total_gnt_cnt_VAL_total_gnt_cnt(val)                   REG_FLD_VAL(total_gnt_cnt_FLD_total_gnt_cnt, (val))

#define total_tx_cyc_h_VAL_total_tx_cyc_h32(val)               REG_FLD_VAL(total_tx_cyc_h_FLD_total_tx_cyc_h32, (val))

#define total_tx_cyc_l_VAL_total_tx_cyc_l32(val)               REG_FLD_VAL(total_tx_cyc_l_FLD_total_tx_cyc_l32, (val))

#define e_sts_dmy6_VAL_xe_sts_dmy6(val)                        REG_FLD_VAL(e_sts_dmy6_FLD_xe_sts_dmy6, (val))

#define e_sts_dmy7_VAL_xe_sts_dmy7(val)                        REG_FLD_VAL(e_sts_dmy7_FLD_xe_sts_dmy7, (val))

#define e_sts_dmy8_VAL_xe_sts_dmy8(val)                        REG_FLD_VAL(e_sts_dmy8_FLD_xe_sts_dmy8, (val))

#define e_sts_dmy9_VAL_xe_sts_dmy9(val)                        REG_FLD_VAL(e_sts_dmy9_FLD_xe_sts_dmy9, (val))

#define e_sts_dmy10_VAL_xe_sts_dmy10(val)                      REG_FLD_VAL(e_sts_dmy10_FLD_xe_sts_dmy10, (val))

#define e_sts_dmy26_VAL_xe_sts_dmy26(val)                      REG_FLD_VAL(e_sts_dmy26_FLD_xe_sts_dmy26, (val))

#define e_sts_dmy27_VAL_xe_sts_dmy27(val)                      REG_FLD_VAL(e_sts_dmy27_FLD_xe_sts_dmy27, (val))

#define e_sts_dmy28_VAL_xe_sts_dmy28(val)                      REG_FLD_VAL(e_sts_dmy28_FLD_xe_sts_dmy28, (val))

#define e_sts_dmy29_VAL_xe_sts_dmy29(val)                      REG_FLD_VAL(e_sts_dmy29_FLD_xe_sts_dmy29, (val))

#define e_sram_pd_VAL_xemac_sram_pd(val)                       REG_FLD_VAL(e_sram_pd_FLD_xemac_sram_pd, (val))

#define e_mbist_delsel_cfg0_VAL_xemac_mbist_delsel_cfg0(val)   REG_FLD_VAL(e_mbist_delsel_cfg0_FLD_xemac_mbist_delsel_cfg0, (val))

#define e_mbist_delsel_cfg1_VAL_xemac_mbist_delsel_cfg1(val)   REG_FLD_VAL(e_mbist_delsel_cfg1_FLD_xemac_mbist_delsel_cfg1, (val))

#define e_mbist_delsel_cfg2_VAL_xemac_mbist_delsel_cfg2(val)   REG_FLD_VAL(e_mbist_delsel_cfg2_FLD_xemac_mbist_delsel_cfg2, (val))

#define e_mbist_delsel_cfg3_VAL_xemac_mbist_delsel_cfg3(val)   REG_FLD_VAL(e_mbist_delsel_cfg3_FLD_xemac_mbist_delsel_cfg3, (val))

#define e_mbist_done_sts0_VAL_xemac_mbist_done_sts0(val)       REG_FLD_VAL(e_mbist_done_sts0_FLD_xemac_mbist_done_sts0, (val))

#define e_mbist_fail_sts0_VAL_xemac_mbist_fail_sts0(val)       REG_FLD_VAL(e_mbist_fail_sts0_FLD_xemac_mbist_fail_sts0, (val))

#define e_mbist_fail_sts1_VAL_xemac_mbist_fail_sts1(val)       REG_FLD_VAL(e_mbist_fail_sts1_FLD_xemac_mbist_fail_sts1, (val))

#define rx_sld_sts_VAL_sld_b7(val)                             REG_FLD_VAL(rx_sld_sts_FLD_sld_b7, (val))
#define rx_sld_sts_VAL_sld_b6(val)                             REG_FLD_VAL(rx_sld_sts_FLD_sld_b6, (val))
#define rx_sld_sts_VAL_sld_b5(val)                             REG_FLD_VAL(rx_sld_sts_FLD_sld_b5, (val))
#define rx_sld_sts_VAL_sld_b4(val)                             REG_FLD_VAL(rx_sld_sts_FLD_sld_b4, (val))
#define rx_sld_sts_VAL_sld_b3(val)                             REG_FLD_VAL(rx_sld_sts_FLD_sld_b3, (val))
#define rx_sld_sts_VAL_sld_b2(val)                             REG_FLD_VAL(rx_sld_sts_FLD_sld_b2, (val))
#define rx_sld_sts_VAL_sld_b1(val)                             REG_FLD_VAL(rx_sld_sts_FLD_sld_b1, (val))
#define rx_sld_sts_VAL_sld_b0(val)                             REG_FLD_VAL(rx_sld_sts_FLD_sld_b0, (val))

#define e_glue_cfg_VAL_txmpi_fifound_pktgate_gntcnt(val)       REG_FLD_VAL(e_glue_cfg_FLD_txmpi_fifound_pktgate_gntcnt, (val))
#define e_glue_cfg_VAL_txmpi_fifound_pktgate_en(val)           REG_FLD_VAL(e_glue_cfg_FLD_txmpi_fifound_pktgate_en, (val))
#define e_glue_cfg_VAL_txmpi_fifound_thr(val)                  REG_FLD_VAL(e_glue_cfg_FLD_txmpi_fifound_thr, (val))

#define e_gntreq_tmout_VAL_gntreq_grden(val)                   REG_FLD_VAL(e_gntreq_tmout_FLD_gntreq_grden, (val))
#define e_gntreq_tmout_VAL_gntreq_grdcyc(val)                  REG_FLD_VAL(e_gntreq_tmout_FLD_gntreq_grdcyc, (val))

#define e_fpga_gendef_VAL_fpga_gendef(val)                     REG_FLD_VAL(e_fpga_gendef_FLD_fpga_gendef, (val))

#define e_fpga_genver_VAL_fpga_genver(val)                     REG_FLD_VAL(e_fpga_genver_FLD_fpga_genver, (val))

#define e_cnt_clr_VAL_glb_cntclr(val)                          REG_FLD_VAL(e_cnt_clr_FLD_glb_cntclr, (val))

#define e_rxmbi_eth_cnt_VAL_rxmbi_eth_cnt(val)                 REG_FLD_VAL(e_rxmbi_eth_cnt_FLD_rxmbi_eth_cnt, (val))

#define e_rxmpi_eth_cnt_VAL_rxmpi_eth_cnt(val)                 REG_FLD_VAL(e_rxmpi_eth_cnt_FLD_rxmpi_eth_cnt, (val))

#define e_txmbi_eth_cnt_VAL_txmbi_eth_cnt(val)                 REG_FLD_VAL(e_txmbi_eth_cnt_FLD_txmbi_eth_cnt, (val))

#define e_txmpi_eth_cnt_VAL_txmpi_eth_cnt(val)                 REG_FLD_VAL(e_txmpi_eth_cnt_FLD_txmpi_eth_cnt, (val))

#define e_oam_stat_VAL_rx_oam_cnt(val)                         REG_FLD_VAL(e_oam_stat_FLD_rx_oam_cnt, (val))
#define e_oam_stat_VAL_tx_oam_cnt(val)                         REG_FLD_VAL(e_oam_stat_FLD_tx_oam_cnt, (val))

#define e_mpcp_stat_VAL_mpcp_err_cnt(val)                      REG_FLD_VAL(e_mpcp_stat_FLD_mpcp_err_cnt, (val))
#define e_mpcp_stat_VAL_mpcp_rgst_cnt(val)                     REG_FLD_VAL(e_mpcp_stat_FLD_mpcp_rgst_cnt, (val))
#define e_mpcp_stat_VAL_mpcp_dscv_gate_cnt(val)                REG_FLD_VAL(e_mpcp_stat_FLD_mpcp_dscv_gate_cnt, (val))

#define e_mpcp_rgst_stat_VAL_mpcp_rgst_req_cnt(val)            REG_FLD_VAL(e_mpcp_rgst_stat_FLD_mpcp_rgst_req_cnt, (val))
#define e_mpcp_rgst_stat_VAL_mpcp_rgst_ack_cnt(val)            REG_FLD_VAL(e_mpcp_rgst_stat_FLD_mpcp_rgst_ack_cnt, (val))

#define e_rxmbi_bytecnt_h_VAL_rxmbi_bytecnt_h(val)             REG_FLD_VAL(e_rxmbi_bytecnt_h_FLD_rxmbi_bytecnt_h, (val))

#define e_rxmbi_bytecnt_l_VAL_rxmbi_bytecnt_l(val)             REG_FLD_VAL(e_rxmbi_bytecnt_l_FLD_rxmbi_bytecnt_l, (val))

#define e_rxmbi_snf_cnt_VAL_rxmbi_snf_cnt(val)                 REG_FLD_VAL(e_rxmbi_snf_cnt_FLD_rxmbi_snf_cnt, (val))

#define e_rxmpi_uc_cnt_VAL_rxmpi_uceth_cnt(val)                REG_FLD_VAL(e_rxmpi_uc_cnt_FLD_rxmpi_uceth_cnt, (val))

#define e_rxmpi_bc_cnt_VAL_rxmpi_bceth_cnt(val)                REG_FLD_VAL(e_rxmpi_bc_cnt_FLD_rxmpi_bceth_cnt, (val))

#define e_rxmpi_mc_cnt_VAL_rxmpi_mceth_cnt(val)                REG_FLD_VAL(e_rxmpi_mc_cnt_FLD_rxmpi_mceth_cnt, (val))

#define e_rxmpi_oam_cnt_VAL_rxmpi_oam_cnt(val)                 REG_FLD_VAL(e_rxmpi_oam_cnt_FLD_rxmpi_oam_cnt, (val))

#define e_rxmpi_mpcp_cnt_VAL_rxmpi_mpcp_cnt(val)               REG_FLD_VAL(e_rxmpi_mpcp_cnt_FLD_rxmpi_mpcp_cnt, (val))

#define e_rxmpi_gate_cnt_VAL_rxmpi_gate_cnt(val)               REG_FLD_VAL(e_rxmpi_gate_cnt_FLD_rxmpi_gate_cnt, (val))

#define e_rxmpi_nrlgate_cnt_VAL_rxmpi_nrlgate_cnt(val)         REG_FLD_VAL(e_rxmpi_nrlgate_cnt_FLD_rxmpi_nrlgate_cnt, (val))

#define e_txmbi_uceth_cnt_VAL_txmbi_uceth_cnt(val)             REG_FLD_VAL(e_txmbi_uceth_cnt_FLD_txmbi_uceth_cnt, (val))

#define e_txmbi_mceth_cnt_VAL_txmbi_bceth_cnt(val)             REG_FLD_VAL(e_txmbi_mceth_cnt_FLD_txmbi_bceth_cnt, (val))
#define e_txmbi_mceth_cnt_VAL_txmbi_mceth_cnt(val)             REG_FLD_VAL(e_txmbi_mceth_cnt_FLD_txmbi_mceth_cnt, (val))

#define e_tx_dygasp_cnt_VAL_txmbi_dygasp_cnt(val)              REG_FLD_VAL(e_tx_dygasp_cnt_FLD_txmbi_dygasp_cnt, (val))
#define e_tx_dygasp_cnt_VAL_txmpi_dygasp_cnt(val)              REG_FLD_VAL(e_tx_dygasp_cnt_FLD_txmpi_dygasp_cnt, (val))

#define e_tx_rpt_cnt_VAL_txmbi_rpt_cnt(val)                    REG_FLD_VAL(e_tx_rpt_cnt_FLD_txmbi_rpt_cnt, (val))
#define e_tx_rpt_cnt_VAL_txmpi_rpt_cnt(val)                    REG_FLD_VAL(e_tx_rpt_cnt_FLD_txmpi_rpt_cnt, (val))

#define e_kpoam_stat_VAL_txmbi_kpalv_cnt(val)                  REG_FLD_VAL(e_kpoam_stat_FLD_txmbi_kpalv_cnt, (val))
#define e_kpoam_stat_VAL_txmpi_kpalv_cnt(val)                  REG_FLD_VAL(e_kpoam_stat_FLD_txmpi_kpalv_cnt, (val))

#define e_txmpi_mpcp_cnt_VAL_txmpi_oam_cnt(val)                REG_FLD_VAL(e_txmpi_mpcp_cnt_FLD_txmpi_oam_cnt, (val))
#define e_txmpi_mpcp_cnt_VAL_txmpi_rgreq_cnt(val)              REG_FLD_VAL(e_txmpi_mpcp_cnt_FLD_txmpi_rgreq_cnt, (val))
#define e_txmpi_mpcp_cnt_VAL_txmpi_rgack_cnt(val)              REG_FLD_VAL(e_txmpi_mpcp_cnt_FLD_txmpi_rgack_cnt, (val))

#define e_txmpi_uceth_cnt_VAL_txmpi_uceth_cnt(val)             REG_FLD_VAL(e_txmpi_uceth_cnt_FLD_txmpi_uceth_cnt, (val))

#define e_txmpi_mceth_cnt_VAL_txmpi_bceth_cnt(val)             REG_FLD_VAL(e_txmpi_mceth_cnt_FLD_txmpi_bceth_cnt, (val))
#define e_txmpi_mceth_cnt_VAL_txmpi_mceth_cnt(val)             REG_FLD_VAL(e_txmpi_mceth_cnt_FLD_txmpi_mceth_cnt, (val))

#define e_rxadv_cnt_VAL_rx_infomis_dscvgate_cnt(val)           REG_FLD_VAL(e_rxadv_cnt_FLD_rx_infomis_dscvgate_cnt, (val))
#define e_rxadv_cnt_VAL_rx_llidbc_nrlgate_cnt(val)             REG_FLD_VAL(e_rxadv_cnt_FLD_rx_llidbc_nrlgate_cnt, (val))
#define e_rxadv_cnt_VAL_rx_filt_dscvgate_cnt(val)              REG_FLD_VAL(e_rxadv_cnt_FLD_rx_filt_dscvgate_cnt, (val))

#define e_rxmpi_churn_cnt_VAL_rxmpi_churn_crcok_cnt(val)       REG_FLD_VAL(e_rxmpi_churn_cnt_FLD_rxmpi_churn_crcok_cnt, (val))
#define e_rxmpi_churn_cnt_VAL_rxmpi_churn_crcerr_cnt(val)      REG_FLD_VAL(e_rxmpi_churn_cnt_FLD_rxmpi_churn_crcerr_cnt, (val))

#define e_rxmpi_churn_info_VAL_rxmpi_churn_keyidx(val)         REG_FLD_VAL(e_rxmpi_churn_info_FLD_rxmpi_churn_keyidx, (val))
#define e_rxmpi_churn_info_VAL_rxmpi_churn_llididx(val)        REG_FLD_VAL(e_rxmpi_churn_info_FLD_rxmpi_churn_llididx, (val))
#define e_rxmpi_churn_info_VAL_rxmpi_churncrc_keyidx(val)      REG_FLD_VAL(e_rxmpi_churn_info_FLD_rxmpi_churncrc_keyidx, (val))
#define e_rxmpi_churn_info_VAL_rxmpi_churncrc_llididx(val)     REG_FLD_VAL(e_rxmpi_churn_info_FLD_rxmpi_churncrc_llididx, (val))
#define e_rxmpi_churn_info_VAL_rxmpi_abchurn_cnt(val)          REG_FLD_VAL(e_rxmpi_churn_info_FLD_rxmpi_abchurn_cnt, (val))

#define e_desec_ok_cnt_VAL_desec_rx_ok_cnt(val)                REG_FLD_VAL(e_desec_ok_cnt_FLD_desec_rx_ok_cnt, (val))
#define e_desec_ok_cnt_VAL_desec_plain_ok_cnt(val)             REG_FLD_VAL(e_desec_ok_cnt_FLD_desec_plain_ok_cnt, (val))

#define e_rxmpi_p_cnt_VAL_rxmpi_p_ok_cnt(val)                  REG_FLD_VAL(e_rxmpi_p_cnt_FLD_rxmpi_p_ok_cnt, (val))
#define e_rxmpi_p_cnt_VAL_rxmpi_p_err_cnt(val)                 REG_FLD_VAL(e_rxmpi_p_cnt_FLD_rxmpi_p_err_cnt, (val))

#define e_rxmpi_crc8err_cnt_VAL_rxmpi_crc32err_cnt(val)        REG_FLD_VAL(e_rxmpi_crc8err_cnt_FLD_rxmpi_crc32err_cnt, (val))
#define e_rxmpi_crc8err_cnt_VAL_rxmpi_crc8err_cnt(val)         REG_FLD_VAL(e_rxmpi_crc8err_cnt_FLD_rxmpi_crc8err_cnt, (val))

#define e_desec_rxdrop_cnt_VAL_desec_rxfifo_ovrundrop_cnt(val) REG_FLD_VAL(e_desec_rxdrop_cnt_FLD_desec_rxfifo_ovrundrop_cnt, (val))
#define e_desec_rxdrop_cnt_VAL_desec_rxsof_ovrundrop_cnt(val)  REG_FLD_VAL(e_desec_rxdrop_cnt_FLD_desec_rxsof_ovrundrop_cnt, (val))

#define e_desec_rxerr_cnt_VAL_desec_rx_crcerr_cnt(val)         REG_FLD_VAL(e_desec_rxerr_cnt_FLD_desec_rx_crcerr_cnt, (val))
#define e_desec_rxerr_cnt_VAL_desec_rx_preerr_cnt(val)         REG_FLD_VAL(e_desec_rxerr_cnt_FLD_desec_rx_preerr_cnt, (val))

#define e_desec_ciphdrop_cnt_VAL_desec_rxsof_hdrdrop_cnt(val)  REG_FLD_VAL(e_desec_ciphdrop_cnt_FLD_desec_rxsof_hdrdrop_cnt, (val))
#define e_desec_ciphdrop_cnt_VAL_desec_ciph_csdrop_cnt(val)    REG_FLD_VAL(e_desec_ciphdrop_cnt_FLD_desec_ciph_csdrop_cnt, (val))

#define e_desec_aeswdrop_cnt_VAL_desec_ciph_actdrop_cnt(val)   REG_FLD_VAL(e_desec_aeswdrop_cnt_FLD_desec_ciph_actdrop_cnt, (val))
#define e_desec_aeswdrop_cnt_VAL_desec_aesw_csdrop_cnt(val)    REG_FLD_VAL(e_desec_aeswdrop_cnt_FLD_desec_aesw_csdrop_cnt, (val))

#define e_desec_aeswdrop_cnt2_VAL_desec_aesw_ovrundrop_cnt(val) REG_FLD_VAL(e_desec_aeswdrop_cnt2_FLD_desec_aesw_ovrundrop_cnt, (val))
#define e_desec_aeswdrop_cnt2_VAL_desec_aesw_icverrdrop_cnt(val) REG_FLD_VAL(e_desec_aeswdrop_cnt2_FLD_desec_aesw_icverrdrop_cnt, (val))

#define e_rxmpi_drop_cnt_VAL_rxmpi_eofdrop_cnt(val)            REG_FLD_VAL(e_rxmpi_drop_cnt_FLD_rxmpi_eofdrop_cnt, (val))
#define e_rxmpi_drop_cnt_VAL_rxmpi_fifoovrun_cnt(val)          REG_FLD_VAL(e_rxmpi_drop_cnt_FLD_rxmpi_fifoovrun_cnt, (val))

#define e_rxmbi_drop_cnt_VAL_rxmbi_sofdrop_cnt(val)            REG_FLD_VAL(e_rxmbi_drop_cnt_FLD_rxmbi_sofdrop_cnt, (val))
#define e_rxmbi_drop_cnt_VAL_rxmbi_snfdrop_cnt(val)            REG_FLD_VAL(e_rxmbi_drop_cnt_FLD_rxmbi_snfdrop_cnt, (val))

#define e_rxmbi_drop_cnt2_VAL_rxmbi_crcerr_cnt(val)            REG_FLD_VAL(e_rxmbi_drop_cnt2_FLD_rxmbi_crcerr_cnt, (val))
#define e_rxmbi_drop_cnt2_VAL_rxmbi_enddrop_cnt(val)           REG_FLD_VAL(e_rxmbi_drop_cnt2_FLD_rxmbi_enddrop_cnt, (val))

#define e_txmbi_err_cnt_VAL_txmbi_err_cnt(val)                 REG_FLD_VAL(e_txmbi_err_cnt_FLD_txmbi_err_cnt, (val))

#define e_sts_dmy11_VAL_xe_sts_dmy11(val)                      REG_FLD_VAL(e_sts_dmy11_FLD_xe_sts_dmy11, (val))

#define e_sts_dmy12_VAL_xe_sts_dmy12(val)                      REG_FLD_VAL(e_sts_dmy12_FLD_xe_sts_dmy12, (val))

#define e_sts_dmy13_VAL_xe_sts_dmy13(val)                      REG_FLD_VAL(e_sts_dmy13_FLD_xe_sts_dmy13, (val))

#define e_sts_dmy14_VAL_xe_sts_dmy14(val)                      REG_FLD_VAL(e_sts_dmy14_FLD_xe_sts_dmy14, (val))

#define e_sts_dmy15_VAL_xe_sts_dmy15(val)                      REG_FLD_VAL(e_sts_dmy15_FLD_xe_sts_dmy15, (val))

#define e_sts_dmy16_VAL_xe_sts_dmy16(val)                      REG_FLD_VAL(e_sts_dmy16_FLD_xe_sts_dmy16, (val))

#define e_sts_dmy17_VAL_xe_sts_dmy17(val)                      REG_FLD_VAL(e_sts_dmy17_FLD_xe_sts_dmy17, (val))

#define e_sts_dmy18_VAL_xe_sts_dmy18(val)                      REG_FLD_VAL(e_sts_dmy18_FLD_xe_sts_dmy18, (val))

#define e_sts_dmy19_VAL_xe_sts_dmy19(val)                      REG_FLD_VAL(e_sts_dmy19_FLD_xe_sts_dmy19, (val))

#define e_sts_dmy20_VAL_xe_sts_dmy20(val)                      REG_FLD_VAL(e_sts_dmy20_FLD_xe_sts_dmy20, (val))

#define e_sts_dmy21_VAL_xe_sts_dmy21(val)                      REG_FLD_VAL(e_sts_dmy21_FLD_xe_sts_dmy21, (val))

#define e_sts_dmy22_VAL_xe_sts_dmy22(val)                      REG_FLD_VAL(e_sts_dmy22_FLD_xe_sts_dmy22, (val))

#define e_sts_dmy23_VAL_xe_sts_dmy23(val)                      REG_FLD_VAL(e_sts_dmy23_FLD_xe_sts_dmy23, (val))

#define e_sts_dmy24_VAL_xe_sts_dmy24(val)                      REG_FLD_VAL(e_sts_dmy24_FLD_xe_sts_dmy24, (val))

#define e_sts_dmy25_VAL_xe_sts_dmy25(val)                      REG_FLD_VAL(e_sts_dmy25_FLD_xe_sts_dmy25, (val))

#define e_gnt_type_stat_VAL_b2b_gnt_cnt(val)                   REG_FLD_VAL(e_gnt_type_stat_FLD_b2b_gnt_cnt, (val))
#define e_gnt_type_stat_VAL_hdn_gnt_cnt(val)                   REG_FLD_VAL(e_gnt_type_stat_FLD_hdn_gnt_cnt, (val))

#define e_gnt_pending_stat_VAL_max_gnt_pending_cnt(val)        REG_FLD_VAL(e_gnt_pending_stat_FLD_max_gnt_pending_cnt, (val))
#define e_gnt_pending_stat_VAL_cur_gnt_pending_cnt(val)        REG_FLD_VAL(e_gnt_pending_stat_FLD_cur_gnt_pending_cnt, (val))

#define e_gnt_length_stat_VAL_max_gnt_length(val)              REG_FLD_VAL(e_gnt_length_stat_FLD_max_gnt_length, (val))
#define e_gnt_length_stat_VAL_min_gnt_length(val)              REG_FLD_VAL(e_gnt_length_stat_FLD_min_gnt_length, (val))

#define e_local_time_VAL_local_time(val)                       REG_FLD_VAL(e_local_time_FLD_local_time, (val))

#define e_time_drft_stat_VAL_cur_time_drift_ofst(val)          REG_FLD_VAL(e_time_drft_stat_FLD_cur_time_drift_ofst, (val))
#define e_time_drft_stat_VAL_max_time_drift_ofst(val)          REG_FLD_VAL(e_time_drft_stat_FLD_max_time_drift_ofst, (val))
#define e_time_drft_stat_VAL_cur_time_drift(val)               REG_FLD_VAL(e_time_drft_stat_FLD_cur_time_drift, (val))
#define e_time_drft_stat_VAL_max_time_drift(val)               REG_FLD_VAL(e_time_drft_stat_FLD_max_time_drift, (val))

#define e_tx_timedrift_stat_VAL_tx_cur_time_drift_ofst(val)    REG_FLD_VAL(e_tx_timedrift_stat_FLD_tx_cur_time_drift_ofst, (val))
#define e_tx_timedrift_stat_VAL_tx_max_time_drift_ofst(val)    REG_FLD_VAL(e_tx_timedrift_stat_FLD_tx_max_time_drift_ofst, (val))
#define e_tx_timedrift_stat_VAL_tx_cur_time_drift(val)         REG_FLD_VAL(e_tx_timedrift_stat_FLD_tx_cur_time_drift, (val))
#define e_tx_timedrift_stat_VAL_tx_max_time_drift(val)         REG_FLD_VAL(e_tx_timedrift_stat_FLD_tx_max_time_drift, (val))

#define e_rxfifo_depth_stat_VAL_cur_rxfifo_depth(val)          REG_FLD_VAL(e_rxfifo_depth_stat_FLD_cur_rxfifo_depth, (val))
#define e_rxfifo_depth_stat_VAL_max_rxfifo_depth(val)          REG_FLD_VAL(e_rxfifo_depth_stat_FLD_max_rxfifo_depth, (val))

#define e_sts_dmy0_VAL_xe_sts_dmy0(val)                        REG_FLD_VAL(e_sts_dmy0_FLD_xe_sts_dmy0, (val))

#define e_sts_dmy1_VAL_xe_sts_dmy1(val)                        REG_FLD_VAL(e_sts_dmy1_FLD_xe_sts_dmy1, (val))

#define e_sts_dmy2_VAL_xe_sts_dmy2(val)                        REG_FLD_VAL(e_sts_dmy2_FLD_xe_sts_dmy2, (val))

#define e_sts_dmy3_VAL_xe_sts_dmy3(val)                        REG_FLD_VAL(e_sts_dmy3_FLD_xe_sts_dmy3, (val))

#define e_sts_dmy4_VAL_xe_sts_dmy4(val)                        REG_FLD_VAL(e_sts_dmy4_FLD_xe_sts_dmy4, (val))

#define e_sts_dmy5_VAL_xe_sts_dmy5(val)                        REG_FLD_VAL(e_sts_dmy5_FLD_xe_sts_dmy5, (val))

#define e_txmbi_bufwrap_wrcnt_VAL_txmbi_bufwrap_wrerr_cnt(val) REG_FLD_VAL(e_txmbi_bufwrap_wrcnt_FLD_txmbi_bufwrap_wrerr_cnt, (val))
#define e_txmbi_bufwrap_wrcnt_VAL_txmbi_bufwrap_wr_cnt(val)    REG_FLD_VAL(e_txmbi_bufwrap_wrcnt_FLD_txmbi_bufwrap_wr_cnt, (val))

#define e_txmbi_bufwrap_rdcnt_VAL_txmbi_bufwrap_rderr_cnt(val) REG_FLD_VAL(e_txmbi_bufwrap_rdcnt_FLD_txmbi_bufwrap_rderr_cnt, (val))
#define e_txmbi_bufwrap_rdcnt_VAL_txmbi_bufwrap_rd_cnt(val)    REG_FLD_VAL(e_txmbi_bufwrap_rdcnt_FLD_txmbi_bufwrap_rd_cnt, (val))

#define e_txmbi_bufwrap_rdcnt2_VAL_txmbi_bufwrap_rddmy_cnt(val) REG_FLD_VAL(e_txmbi_bufwrap_rdcnt2_FLD_txmbi_bufwrap_rddmy_cnt, (val))

#ifdef __cplusplus
}
#endif

#endif // __EPON_MAC_REGS_H__
