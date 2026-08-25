/***************************************************************
Copyright Statement:

This software/firmware and related documentation (“EcoNet Software”) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (“EcoNet”) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (“ECONET SOFTWARE”) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN “AS IS” 
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
#ifdef __BIG_ENDIAN
        FIELD rsv_27                    : 5;
        FIELD sniffer_mode              : 1;
        FIELD txoam_favor               : 1;
        FIELD burst_en_dly              : 1;
        FIELD discv_burst_en            : 1;
        FIELD mpcp_fwd                  : 1;
        FIELD bcst_llid_m1_drop         : 1;
        FIELD bcst_llid_m0_drop         : 1;
        FIELD mcst_llid_drop            : 1;
        FIELD all_unicast_llid_pkt_fwd  : 1;
        FIELD fcs_err_fwd               : 1;
        FIELD llid_crc8_err_fwd         : 1;
        FIELD rsv_14                    : 2;
        FIELD rxmpi_stop                : 1;
        FIELD txmpi_stop                : 1;
        FIELD phy_pwr_down              : 1;
        FIELD rx_nml_gate_fwd           : 1;
        FIELD rxmbi_stop                : 1;
        FIELD txmbi_stop                : 1;
        FIELD chk_all_gnt_mode          : 1;
        FIELD rsv_6                     : 1;
        FIELD tx_default_rpt            : 1;
        FIELD epon_mac_sw_rst           : 1;
        FIELD epon_oam_cal_in_eth       : 1;
        FIELD epon_mac_lpbk_en          : 1;
        FIELD rpt_txpri_ctrl            : 1;
        FIELD rsv_0                     : 1;
#else
        FIELD rsv_0                     : 1;
        FIELD rpt_txpri_ctrl            : 1;
        FIELD epon_mac_lpbk_en          : 1;
        FIELD epon_oam_cal_in_eth       : 1;
        FIELD epon_mac_sw_rst           : 1;
        FIELD tx_default_rpt            : 1;
        FIELD rsv_6                     : 1;
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
        FIELD all_unicast_llid_pkt_fwd  : 1;
        FIELD mcst_llid_drop            : 1;
        FIELD bcst_llid_m0_drop         : 1;
        FIELD bcst_llid_m1_drop         : 1;
        FIELD mpcp_fwd                  : 1;
        FIELD discv_burst_en            : 1;
        FIELD burst_en_dly              : 1;
        FIELD txoam_favor               : 1;
        FIELD sniffer_mode              : 1;
        FIELD rsv_27                    : 5;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_glb_cfg, *PREG_e_glb_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_27                    : 5;
        FIELD sniff_fifo_ovrun_int      : 1;
        FIELD reg_ack_done_int          : 1;
        FIELD reg_req_done_int          : 1;
        FIELD reorder1_gnt_int          : 1;
        FIELD b2b_gnt_int               : 1;
        FIELD hidn_gnt_int              : 1;
        FIELD ps_early_wakeup_int       : 1;
        FIELD rx_sleep_allow_int        : 1;
        FIELD ps_wakeup_int             : 1;
        FIELD ps_sleep_int              : 1;
        FIELD txfifo_udrun_int          : 1;
        FIELD rpt_overintvl_int         : 1;
        FIELD mpcp_timeout_int          : 1;
        FIELD timedrft_int              : 1;
        FIELD tod_1pps_int              : 1;
        FIELD tod_updt_int              : 1;
        FIELD ptp_msg_tx_int            : 1;
        FIELD gnt_buf_ovrrun_int        : 1;
        FIELD llid7_rcv_rgst_int        : 1;
        FIELD llid6_rcv_rgst_int        : 1;
        FIELD llid5_rcv_rgst_int        : 1;
        FIELD llid4_rcv_rgst_int        : 1;
        FIELD llid3_rcv_rgst_int        : 1;
        FIELD llid2_rcv_rgst_int        : 1;
        FIELD llid1_rcv_rgst_int        : 1;
        FIELD llid0_rcv_rgst_int        : 1;
        FIELD rcv_dscvry_gate_int       : 1;
#else
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
        FIELD rsv_27                    : 5;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_int_status, *PREG_e_int_status;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_27                    : 5;
        FIELD sniff_fifo_ovrun_en       : 1;
        FIELD reg_ack_done_en           : 1;
        FIELD reg_req_done_en           : 1;
        FIELD reorder1_gnt_en           : 1;
        FIELD b2b_gnt_en                : 1;
        FIELD hidn_gnt_en               : 1;
        FIELD ps_early_wakeup_en        : 1;
        FIELD rx_sleep_allow_en         : 1;
        FIELD ps_wakeup_en              : 1;
        FIELD ps_sleep_en               : 1;
        FIELD txfifo_udrun_en           : 1;
        FIELD rpt_overintvl_en          : 1;
        FIELD mpcp_timeout_en           : 1;
        FIELD timedrft_en               : 1;
        FIELD tod_1pps_en               : 1;
        FIELD tod_updt_en               : 1;
        FIELD ptp_msg_tx_en             : 1;
        FIELD gnt_buf_ovrrun_en         : 1;
        FIELD llid7_rcv_rgst_en         : 1;
        FIELD llid6_rcv_rgst_en         : 1;
        FIELD llid5_rcv_rgst_en         : 1;
        FIELD llid4_rcv_rgst_en         : 1;
        FIELD llid3_rcv_rgst_en         : 1;
        FIELD llid2_rcv_rgst_en         : 1;
        FIELD llid1_rcv_rgst_en         : 1;
        FIELD llid0_rcv_rgst_en         : 1;
        FIELD rcv_dscvry_gate_en        : 1;
#else
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
        FIELD rsv_27                    : 5;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_int_en, *PREG_e_int_en;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid15_mpcp_tmo           : 1;
        FIELD llid14_mpcp_tmo           : 1;
        FIELD llid13_mpcp_tmo           : 1;
        FIELD llid12_mpcp_tmo           : 1;
        FIELD llid11_mpcp_tmo           : 1;
        FIELD llid10_mpcp_tmo           : 1;
        FIELD llid9_mpcp_tmo            : 1;
        FIELD llid8_mpcp_tmo            : 1;
        FIELD llid15_rpt_tmo            : 1;
        FIELD llid14_rpt_tmo            : 1;
        FIELD llid13_rpt_tmo            : 1;
        FIELD llid12_rpt_tmo            : 1;
        FIELD llid11_rpt_tmo            : 1;
        FIELD llid10_rpt_tmo            : 1;
        FIELD llid9_rpt_tmo             : 1;
        FIELD llid8_rpt_tmo             : 1;
        FIELD llid7_mpcp_tmo            : 1;
        FIELD llid6_mpcp_tmo            : 1;
        FIELD llid5_mpcp_tmo            : 1;
        FIELD llid4_mpcp_tmo            : 1;
        FIELD llid3_mpcp_tmo            : 1;
        FIELD llid2_mpcp_tmo            : 1;
        FIELD llid1_mpcp_tmo            : 1;
        FIELD llid0_mpcp_tmo            : 1;
        FIELD llid7_rpt_tmo             : 1;
        FIELD llid6_rpt_tmo             : 1;
        FIELD llid5_rpt_tmo             : 1;
        FIELD llid4_rpt_tmo             : 1;
        FIELD llid3_rpt_tmo             : 1;
        FIELD llid2_rpt_tmo             : 1;
        FIELD llid1_rpt_tmo             : 1;
        FIELD llid0_rpt_tmo             : 1;
#else
        FIELD llid0_rpt_tmo             : 1;
        FIELD llid1_rpt_tmo             : 1;
        FIELD llid2_rpt_tmo             : 1;
        FIELD llid3_rpt_tmo             : 1;
        FIELD llid4_rpt_tmo             : 1;
        FIELD llid5_rpt_tmo             : 1;
        FIELD llid6_rpt_tmo             : 1;
        FIELD llid7_rpt_tmo             : 1;
        FIELD llid0_mpcp_tmo            : 1;
        FIELD llid1_mpcp_tmo            : 1;
        FIELD llid2_mpcp_tmo            : 1;
        FIELD llid3_mpcp_tmo            : 1;
        FIELD llid4_mpcp_tmo            : 1;
        FIELD llid5_mpcp_tmo            : 1;
        FIELD llid6_mpcp_tmo            : 1;
        FIELD llid7_mpcp_tmo            : 1;
        FIELD llid8_rpt_tmo             : 1;
        FIELD llid9_rpt_tmo             : 1;
        FIELD llid10_rpt_tmo            : 1;
        FIELD llid11_rpt_tmo            : 1;
        FIELD llid12_rpt_tmo            : 1;
        FIELD llid13_rpt_tmo            : 1;
        FIELD llid14_rpt_tmo            : 1;
        FIELD llid15_rpt_tmo            : 1;
        FIELD llid8_mpcp_tmo            : 1;
        FIELD llid9_mpcp_tmo            : 1;
        FIELD llid10_mpcp_tmo           : 1;
        FIELD llid11_mpcp_tmo           : 1;
        FIELD llid12_mpcp_tmo           : 1;
        FIELD llid13_mpcp_tmo           : 1;
        FIELD llid14_mpcp_tmo           : 1;
        FIELD llid15_mpcp_tmo           : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rpt_mpcp_timeout_llid_idx, *PREG_e_rpt_mpcp_timeout_llid_idx;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD hw_dying_gasp_en          : 1;
        FIELD rsv_17                    : 14;
        FIELD sw_init_dying_gasp        : 1;
        FIELD rsv_10                    : 6;
        FIELD dygsp_num_of_times        : 2;
        FIELD rsv_0                     : 8;
#else
        FIELD rsv_0                     : 8;
        FIELD dygsp_num_of_times        : 2;
        FIELD rsv_10                    : 6;
        FIELD sw_init_dying_gasp        : 1;
        FIELD rsv_17                    : 14;
        FIELD hw_dying_gasp_en          : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_dyinggsp_cfg, *PREG_e_dyinggsp_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD echoed_pending_gnt        : 8;
        FIELD pending_gnt_num           : 8;
#else
        FIELD pending_gnt_num           : 8;
        FIELD echoed_pending_gnt        : 8;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_pending_gnt_num, *PREG_e_pending_gnt_num;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid3_dummy               : 4;
        FIELD llid3_txfec_en            : 1;
        FIELD llid3_dcrypt_en           : 1;
        FIELD llid3_dcrypt_mode         : 1;
        FIELD llid3_oam_lpbk_en         : 1;
        FIELD llid2_dummy               : 4;
        FIELD llid2_txfec_en            : 1;
        FIELD llid2_dcrypt_en           : 1;
        FIELD llid2_dcrypt_mode         : 1;
        FIELD llid2_oam_lpbk_en         : 1;
        FIELD llid1_dummy               : 4;
        FIELD llid1_txfec_en            : 1;
        FIELD llid1_dcrypt_en           : 1;
        FIELD llid1_dcrypt_mode         : 1;
        FIELD llid1_oam_lpbk_en         : 1;
        FIELD llid0_dummy               : 4;
        FIELD llid0_txfec_en            : 1;
        FIELD llid0_dcrypt_en           : 1;
        FIELD llid0_dcrypt_mode         : 1;
        FIELD llid0_oam_lpbk_en         : 1;
#else
        FIELD llid0_oam_lpbk_en         : 1;
        FIELD llid0_dcrypt_mode         : 1;
        FIELD llid0_dcrypt_en           : 1;
        FIELD llid0_txfec_en            : 1;
        FIELD llid0_dummy               : 4;
        FIELD llid1_oam_lpbk_en         : 1;
        FIELD llid1_dcrypt_mode         : 1;
        FIELD llid1_dcrypt_en           : 1;
        FIELD llid1_txfec_en            : 1;
        FIELD llid1_dummy               : 4;
        FIELD llid2_oam_lpbk_en         : 1;
        FIELD llid2_dcrypt_mode         : 1;
        FIELD llid2_dcrypt_en           : 1;
        FIELD llid2_txfec_en            : 1;
        FIELD llid2_dummy               : 4;
        FIELD llid3_oam_lpbk_en         : 1;
        FIELD llid3_dcrypt_mode         : 1;
        FIELD llid3_dcrypt_en           : 1;
        FIELD llid3_txfec_en            : 1;
        FIELD llid3_dummy               : 4;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid0_3_cfg, *PREG_e_llid0_3_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid7_dummy               : 4;
        FIELD llid7_txfec_en            : 1;
        FIELD llid7_dcrypt_en           : 1;
        FIELD llid7_dcrypt_mode         : 1;
        FIELD llid7_oam_lpbk_en         : 1;
        FIELD llid6_dummy               : 4;
        FIELD llid6_txfec_en            : 1;
        FIELD llid6_dcrypt_en           : 1;
        FIELD llid6_dcrypt_mode         : 1;
        FIELD llid6_oam_lpbk_en         : 1;
        FIELD llid5_dummy               : 4;
        FIELD llid5_txfec_en            : 1;
        FIELD llid5_dcrypt_en           : 1;
        FIELD llid5_dcrypt_mode         : 1;
        FIELD llid5_oam_lpbk_en         : 1;
        FIELD llid4_dummy               : 4;
        FIELD llid4_txfec_en            : 1;
        FIELD llid4_dcrypt_en           : 1;
        FIELD llid4_dcrypt_mode         : 1;
        FIELD llid4_oam_lpbk_en         : 1;
#else
        FIELD llid4_oam_lpbk_en         : 1;
        FIELD llid4_dcrypt_mode         : 1;
        FIELD llid4_dcrypt_en           : 1;
        FIELD llid4_txfec_en            : 1;
        FIELD llid4_dummy               : 4;
        FIELD llid5_oam_lpbk_en         : 1;
        FIELD llid5_dcrypt_mode         : 1;
        FIELD llid5_dcrypt_en           : 1;
        FIELD llid5_txfec_en            : 1;
        FIELD llid5_dummy               : 4;
        FIELD llid6_oam_lpbk_en         : 1;
        FIELD llid6_dcrypt_mode         : 1;
        FIELD llid6_dcrypt_en           : 1;
        FIELD llid6_txfec_en            : 1;
        FIELD llid6_dummy               : 4;
        FIELD llid7_oam_lpbk_en         : 1;
        FIELD llid7_dcrypt_mode         : 1;
        FIELD llid7_dcrypt_en           : 1;
        FIELD llid7_txfec_en            : 1;
        FIELD llid7_dummy               : 4;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid4_7_cfg, *PREG_e_llid4_7_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD mpcp_cmd                  : 2;
        FIELD rsv_17                    : 13;
        FIELD mpcp_cmd_done             : 1;
        FIELD rsv_13                    : 3;
        FIELD rgstr_ack_flg             : 1;
        FIELD rsv_9                     : 3;
        FIELD rgstr_req_flg             : 1;
        FIELD rsv_5                     : 3;
        FIELD tx_mpcp_llid_idx          : 5;
#else
        FIELD tx_mpcp_llid_idx          : 5;
        FIELD rsv_5                     : 3;
        FIELD rgstr_req_flg             : 1;
        FIELD rsv_9                     : 3;
        FIELD rgstr_ack_flg             : 1;
        FIELD rsv_13                    : 3;
        FIELD mpcp_cmd_done             : 1;
        FIELD rsv_17                    : 13;
        FIELD mpcp_cmd                  : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid_dscvry_ctrl, *PREG_e_llid_dscvry_ctrl;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid0_dscvry_sts          : 2;
        FIELD rsv_26                    : 4;
        FIELD llid0_rgstr_flg_sts       : 2;
        FIELD rsv_17                    : 7;
        FIELD llid0_valid               : 1;
        FIELD llid0_value               : 16;
#else
        FIELD llid0_value               : 16;
        FIELD llid0_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid0_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid0_dscvry_sts          : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid0_dscvry_sts, *PREG_e_llid0_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid1_dscvry_sts          : 2;
        FIELD rsv_26                    : 4;
        FIELD llid1_rgstr_flg_sts       : 2;
        FIELD rsv_17                    : 7;
        FIELD llid1_valid               : 1;
        FIELD llid1_value               : 16;
#else
        FIELD llid1_value               : 16;
        FIELD llid1_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid1_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid1_dscvry_sts          : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid1_dscvry_sts, *PREG_e_llid1_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid2_dscvry_sts          : 2;
        FIELD rsv_26                    : 4;
        FIELD llid2_rgstr_flg_sts       : 2;
        FIELD rsv_17                    : 7;
        FIELD llid2_valid               : 1;
        FIELD llid2_value               : 16;
#else
        FIELD llid2_value               : 16;
        FIELD llid2_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid2_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid2_dscvry_sts          : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid2_dscvry_sts, *PREG_e_llid2_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid3_dscvry_sts          : 2;
        FIELD rsv_26                    : 4;
        FIELD llid3_rgstr_flg_sts       : 2;
        FIELD rsv_17                    : 7;
        FIELD llid3_valid               : 1;
        FIELD llid3_value               : 16;
#else
        FIELD llid3_value               : 16;
        FIELD llid3_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid3_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid3_dscvry_sts          : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid3_dscvry_sts, *PREG_e_llid3_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid4_dscvry_sts          : 2;
        FIELD rsv_26                    : 4;
        FIELD llid4_rgstr_flg_sts       : 2;
        FIELD rsv_17                    : 7;
        FIELD llid4_valid               : 1;
        FIELD llid4_value               : 16;
#else
        FIELD llid4_value               : 16;
        FIELD llid4_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid4_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid4_dscvry_sts          : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid4_dscvry_sts, *PREG_e_llid4_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid5_dscvry_sts          : 2;
        FIELD rsv_26                    : 4;
        FIELD llid5_rgstr_flg_sts       : 2;
        FIELD rsv_17                    : 7;
        FIELD llid5_valid               : 1;
        FIELD llid5_value               : 16;
#else
        FIELD llid5_value               : 16;
        FIELD llid5_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid5_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid5_dscvry_sts          : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid5_dscvry_sts, *PREG_e_llid5_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid6_dscvry_sts          : 2;
        FIELD rsv_26                    : 4;
        FIELD llid6_rgstr_flg_sts       : 2;
        FIELD rsv_17                    : 7;
        FIELD llid6_valid               : 1;
        FIELD llid6_value               : 16;
#else
        FIELD llid6_value               : 16;
        FIELD llid6_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid6_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid6_dscvry_sts          : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid6_dscvry_sts, *PREG_e_llid6_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid7_dscvry_sts          : 2;
        FIELD rsv_26                    : 4;
        FIELD llid7_rgstr_flg_sts       : 2;
        FIELD rsv_17                    : 7;
        FIELD llid7_valid               : 1;
        FIELD llid7_value               : 16;
#else
        FIELD llid7_value               : 16;
        FIELD llid7_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid7_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid7_dscvry_sts          : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid7_dscvry_sts, *PREG_e_llid7_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD mac_addr_rwcmd            : 1;
        FIELD rsv_17                    : 14;
        FIELD mac_addr_rwcmd_done       : 1;
        FIELD rsv_6                     : 10;
        FIELD mac_addr_llid_indx        : 5;
        FIELD mac_addr_dw_idx           : 1;
#else
        FIELD mac_addr_dw_idx           : 1;
        FIELD mac_addr_llid_indx        : 5;
        FIELD rsv_6                     : 10;
        FIELD mac_addr_rwcmd_done       : 1;
        FIELD rsv_17                    : 14;
        FIELD mac_addr_rwcmd            : 1;
#endif /* __BIG_ENDIAN */
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
#ifdef __BIG_ENDIAN
        FIELD key_rwcmd                 : 1;
        FIELD rsv_17                    : 14;
        FIELD key_rwcmd_done            : 1;
        FIELD rsv_8                     : 8;
        FIELD key_llid_index            : 5;
        FIELD key_idx                   : 1;
        FIELD key_dw_indx               : 2;
#else
        FIELD key_dw_indx               : 2;
        FIELD key_idx                   : 1;
        FIELD key_llid_index            : 5;
        FIELD rsv_8                     : 8;
        FIELD key_rwcmd_done            : 1;
        FIELD rsv_17                    : 14;
        FIELD key_rwcmd                 : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_security_key_cfg, *PREG_e_security_key_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD key_value                 : 24;
#else
        FIELD key_value                 : 24;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_key_value, *PREG_e_key_value;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD llid7_rpt_cfg             : 2;
        FIELD llid6_rpt_cfg             : 2;
        FIELD llid5_rpt_cfg             : 2;
        FIELD llid4_rpt_cfg             : 2;
        FIELD llid3_rpt_cfg             : 2;
        FIELD llid2_rpt_cfg             : 2;
        FIELD llid1_rpt_cfg             : 2;
        FIELD llid0_rpt_cfg             : 2;
#else
        FIELD llid0_rpt_cfg             : 2;
        FIELD llid1_rpt_cfg             : 2;
        FIELD llid2_rpt_cfg             : 2;
        FIELD llid3_rpt_cfg             : 2;
        FIELD llid4_rpt_cfg             : 2;
        FIELD llid5_rpt_cfg             : 2;
        FIELD llid6_rpt_cfg             : 2;
        FIELD llid7_rpt_cfg             : 2;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rpt_cfg, *PREG_e_rpt_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD qthld_rwcmd               : 1;
        FIELD qthld_rwcmd_done          : 1;
        FIELD rsv_24                    : 6;
        FIELD qthld_value               : 16;
        FIELD qthld_idx                 : 2;
        FIELD rpt_llid_idx_2_0          : 3;
        FIELD queue_idx                 : 3;
#else
        FIELD queue_idx                 : 3;
        FIELD rpt_llid_idx_2_0          : 3;
        FIELD qthld_idx                 : 2;
        FIELD qthld_value               : 16;
        FIELD rsv_24                    : 6;
        FIELD qthld_rwcmd_done          : 1;
        FIELD qthld_rwcmd               : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rpt_qthld_cfg, *PREG_e_rpt_qthld_cfg;

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
        FIELD tod_sync_x                : 32;
    } Bits;
    UINT32 Raw;
} REG_e_tod_sync_x, *PREG_e_tod_sync_x;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD rl_rx_phydly_ofst         : 5;
        FIELD ingrs_latency             : 11;
        FIELD egrs_latency              : 8;
#else
        FIELD egrs_latency              : 8;
        FIELD ingrs_latency             : 11;
        FIELD rl_rx_phydly_ofst         : 5;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
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
#ifdef __BIG_ENDIAN
        FIELD rsv_8                     : 24;
        FIELD tod_period                : 8;
#else
        FIELD tod_period                : 8;
        FIELD rsv_8                     : 24;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_tod_period, *PREG_e_tod_period;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_30                    : 2;
        FIELD pwd_mode                  : 2;
        FIELD rsv_27                    : 1;
        FIELD timedrift_ignore          : 1;
        FIELD onu_wakeup                : 1;
        FIELD pwr_sv_start              : 1;
        FIELD rsv_23                    : 1;
        FIELD pwd_mode_i                : 3;
        FIELD rsv_16                    : 4;
        FIELD slp_duration_max_h        : 16;
#else
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
#endif /* __BIG_ENDIAN */
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
#ifdef __BIG_ENDIAN
        FIELD rsv_27                    : 5;
        FIELD slp_pwd_mode_i            : 3;
        FIELD slp_flag_i                : 8;
        FIELD slp_duration_i            : 16;
#else
        FIELD slp_duration_i            : 16;
        FIELD slp_flag_i                : 8;
        FIELD slp_pwd_mode_i            : 3;
        FIELD rsv_27                    : 5;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_slp_duration_i, *PREG_e_slp_duration_i;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD tx_fetch_leadtime         : 8;
        FIELD tx_dma_leadtime           : 16;
#else
        FIELD tx_dma_leadtime           : 16;
        FIELD tx_fetch_leadtime         : 8;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_txfetch_cfg, *PREG_e_txfetch_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_17                    : 15;
        FIELD sync_time_updte           : 1;
        FIELD sync_time                 : 16;
#else
        FIELD sync_time                 : 16;
        FIELD sync_time_updte           : 1;
        FIELD rsv_17                    : 15;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_sync_time, *PREG_e_sync_time;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD dscvr_gnt_len             : 8;
        FIELD fec_tail_grd              : 8;
        FIELD tail_grd                  : 8;
        FIELD rsv_6                     : 2;
        FIELD default_ovrhd             : 6;
#else
        FIELD default_ovrhd             : 6;
        FIELD rsv_6                     : 2;
        FIELD tail_grd                  : 8;
        FIELD fec_tail_grd              : 8;
        FIELD dscvr_gnt_len             : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_tx_cal_cnst, *PREG_e_tx_cal_cnst;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD laser_off_time            : 8;
        FIELD laser_on_time             : 8;
#else
        FIELD laser_on_time             : 8;
        FIELD laser_off_time            : 8;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_laser_onoff_time, *PREG_e_laser_onoff_time;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_8                     : 24;
        FIELD guard_thrshld             : 8;
#else
        FIELD guard_thrshld             : 8;
        FIELD rsv_8                     : 24;
#endif /* __BIG_ENDIAN */
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
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD rpt_timeout_intvl         : 24;
#else
        FIELD rpt_timeout_intvl         : 24;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
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
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD min_proc_time             : 16;
#else
        FIELD min_proc_time             : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_min_proc_time, *PREG_e_min_proc_time;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD tx_stm_adj                : 16;
#else
        FIELD tx_stm_adj                : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_trx_adjust_time1, *PREG_e_trx_adjust_time1;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD tx_len_adj                : 16;
        FIELD rx_tmstp_adj              : 16;
#else
        FIELD rx_tmstp_adj              : 16;
        FIELD tx_len_adj                : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_trx_adjust_time2, *PREG_e_trx_adjust_time2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_18                    : 14;
        FIELD probe_dtgrp_sel           : 2;
        FIELD rsv_13                    : 3;
        FIELD probe_bit0_sel            : 5;
        FIELD rsv_5                     : 3;
        FIELD probe_sel                 : 5;
#else
        FIELD probe_sel                 : 5;
        FIELD rsv_5                     : 3;
        FIELD probe_bit0_sel            : 5;
        FIELD rsv_13                    : 3;
        FIELD probe_dtgrp_sel           : 2;
        FIELD rsv_18                    : 14;
#endif /* __BIG_ENDIAN */
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
#ifdef __BIG_ENDIAN
        FIELD rx_oam_cnt                : 16;
        FIELD tx_oam_cnt                : 16;
#else
        FIELD tx_oam_cnt                : 16;
        FIELD rx_oam_cnt                : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_oam_stat, *PREG_e_oam_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD mpcp_err_cnt              : 8;
        FIELD mpcp_rgst_cnt             : 8;
        FIELD mpcp_dscv_gate_cnt        : 16;
#else
        FIELD mpcp_dscv_gate_cnt        : 16;
        FIELD mpcp_rgst_cnt             : 8;
        FIELD mpcp_err_cnt              : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_mpcp_stat, *PREG_e_mpcp_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD mpcp_rgst_req_cnt         : 8;
        FIELD mpcp_rgst_ack_cnt         : 8;
#else
        FIELD mpcp_rgst_ack_cnt         : 8;
        FIELD mpcp_rgst_req_cnt         : 8;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_mpcp_rgst_stat, *PREG_e_mpcp_rgst_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD max_gnt_pending_cnt       : 8;
        FIELD rsv_8                     : 8;
        FIELD cur_gnt_pending_cnt       : 8;
#else
        FIELD cur_gnt_pending_cnt       : 8;
        FIELD rsv_8                     : 8;
        FIELD max_gnt_pending_cnt       : 8;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_gnt_pending_stat, *PREG_e_gnt_pending_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD max_gnt_length            : 16;
        FIELD min_gnt_length            : 16;
#else
        FIELD min_gnt_length            : 16;
        FIELD max_gnt_length            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_gnt_length_stat, *PREG_e_gnt_length_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD b2b_gnt_cnt               : 16;
        FIELD hdn_gnt_cnt               : 16;
#else
        FIELD hdn_gnt_cnt               : 16;
        FIELD b2b_gnt_cnt               : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_gnt_type_stat, *PREG_e_gnt_type_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_29                    : 3;
        FIELD cur_time_drift_ofst       : 5;
        FIELD rsv_21                    : 3;
        FIELD max_time_drift_ofst       : 5;
        FIELD cur_time_drift            : 8;
        FIELD max_time_drift            : 8;
#else
        FIELD max_time_drift            : 8;
        FIELD cur_time_drift            : 8;
        FIELD max_time_drift_ofst       : 5;
        FIELD rsv_21                    : 3;
        FIELD cur_time_drift_ofst       : 5;
        FIELD rsv_29                    : 3;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_time_drft_stat, *PREG_e_time_drft_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD llid0_gnt_cnt             : 16;
#else
        FIELD llid0_gnt_cnt             : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid0_gnt_stat, *PREG_e_llid0_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD llid1_gnt_cnt             : 16;
#else
        FIELD llid1_gnt_cnt             : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid1_gnt_stat, *PREG_e_llid1_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD llid2_gnt_cnt             : 16;
#else
        FIELD llid2_gnt_cnt             : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid2_gnt_stat, *PREG_e_llid2_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD llid3_gnt_cnt             : 16;
#else
        FIELD llid3_gnt_cnt             : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid3_gnt_stat, *PREG_e_llid3_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD llid4_gnt_cnt             : 16;
#else
        FIELD llid4_gnt_cnt             : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid4_gnt_stat, *PREG_e_llid4_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD llid5_gnt_cnt             : 16;
#else
        FIELD llid5_gnt_cnt             : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid5_gnt_stat, *PREG_e_llid5_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD llid6_gnt_cnt             : 16;
#else
        FIELD llid6_gnt_cnt             : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid6_gnt_stat, *PREG_e_llid6_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD llid7_gnt_cnt             : 16;
#else
        FIELD llid7_gnt_cnt             : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid7_gnt_stat, *PREG_e_llid7_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD snf_mpcp_cap_en           : 1;
        FIELD snf_oam_cap_en            : 1;
        FIELD snf_mpcp_oam_cnt_set      : 30;
#else
        FIELD snf_mpcp_oam_cnt_set      : 30;
        FIELD snf_oam_cap_en            : 1;
        FIELD snf_mpcp_cap_en           : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_snf_mpcp_oam_ctl, *PREG_e_snf_mpcp_oam_ctl;

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
#ifdef __BIG_ENDIAN
        FIELD oam_kpalv_interval        : 24;
        FIELD rsv_3                     : 5;
        FIELD oam_kpalv_sw_trig         : 1;
        FIELD oam_kpalv_sw_cfg          : 1;
        FIELD oam_kpalv_en              : 1;
#else
        FIELD oam_kpalv_en              : 1;
        FIELD oam_kpalv_sw_cfg          : 1;
        FIELD oam_kpalv_sw_trig         : 1;
        FIELD rsv_3                     : 5;
        FIELD oam_kpalv_interval        : 24;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_oam_kpalv_ctrl, *PREG_e_oam_kpalv_ctrl;

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
        FIELD sniffer_sp_tag            : 32;
    } Bits;
    UINT32 Raw;
} REG_e_sniff_sp_tag, *PREG_e_sniff_sp_tag;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llidnum_sel               : 1;
        FIELD rsv_22                    : 9;
        FIELD loctime_mtd               : 1;
        FIELD rgreq_lsrtime_mask        : 1;
        FIELD rsv_18                    : 2;
        FIELD all_ucllid_mpcpsnf        : 1;
        FIELD u10g_txmode               : 1;
        FIELD eth_cal_in_bytecnt        : 1;
        FIELD snf_cal_in_bytecnt        : 1;
        FIELD oam_cal_in_bytecnt        : 1;
        FIELD dscvgate_filt             : 1;
        FIELD us_snf                    : 1;
        FIELD llidinfo_snf              : 1;
        FIELD all_ucllid_ethsnf         : 1;
        FIELD all_ucllid_snf            : 1;
        FIELD rxuni_mcllid_en           : 1;
        FIELD gntlen_stat_widscv        : 1;
        FIELD tdrift_loctupd_dis        : 1;
        FIELD rsv_3                     : 2;
        FIELD snf_fcserr_fwd            : 1;
        FIELD mpcp_fcserr_fwd           : 1;
        FIELD dscvgate_infochk_dis      : 1;
#else
        FIELD dscvgate_infochk_dis      : 1;
        FIELD mpcp_fcserr_fwd           : 1;
        FIELD snf_fcserr_fwd            : 1;
        FIELD rsv_3                     : 2;
        FIELD tdrift_loctupd_dis        : 1;
        FIELD gntlen_stat_widscv        : 1;
        FIELD rxuni_mcllid_en           : 1;
        FIELD all_ucllid_snf            : 1;
        FIELD all_ucllid_ethsnf         : 1;
        FIELD llidinfo_snf              : 1;
        FIELD us_snf                    : 1;
        FIELD dscvgate_filt             : 1;
        FIELD oam_cal_in_bytecnt        : 1;
        FIELD snf_cal_in_bytecnt        : 1;
        FIELD eth_cal_in_bytecnt        : 1;
        FIELD u10g_txmode               : 1;
        FIELD all_ucllid_mpcpsnf        : 1;
        FIELD rsv_18                    : 2;
        FIELD rgreq_lsrtime_mask        : 1;
        FIELD loctime_mtd               : 1;
        FIELD rsv_22                    : 9;
        FIELD llidnum_sel               : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_glb_cfg2, *PREG_e_glb_cfg2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_11                    : 21;
        FIELD rx_rgst_exc_int           : 1;
        FIELD rcv_len_err_int           : 1;
        FIELD schfch_nopkt_int          : 1;
        FIELD rxfifo_ovrun_int          : 1;
        FIELD txmpi_pldenneq_int        : 1;
        FIELD schfch_timeo_int          : 1;
        FIELD schgnt_inv_int            : 1;
        FIELD rcvgnt_infofail_int       : 1;
        FIELD rcv_eofdrop_int           : 1;
        FIELD rcv_crcerr_int            : 1;
        FIELD rcv_dauc_dscvgate_int     : 1;
#else
        FIELD rcv_dauc_dscvgate_int     : 1;
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
        FIELD rsv_11                    : 21;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_int_sts2, *PREG_e_int_sts2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_11                    : 21;
        FIELD rx_rgst_exc_en            : 1;
        FIELD rcv_len_err_en            : 1;
        FIELD schfch_nopkt_en           : 1;
        FIELD rxfifo_ovrun_en           : 1;
        FIELD txmpi_pldenneq_en         : 1;
        FIELD schfch_timeo_en           : 1;
        FIELD schgnt_inv_en             : 1;
        FIELD rcvgnt_infofail_en        : 1;
        FIELD rcv_eofdrop_en            : 1;
        FIELD rcv_crcerr_en             : 1;
        FIELD rcv_dauc_dscvgate_en      : 1;
#else
        FIELD rcv_dauc_dscvgate_en      : 1;
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
        FIELD rsv_11                    : 21;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_int_en2, *PREG_e_int_en2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_26                    : 6;
        FIELD rx_rgst_dergst_hit        : 1;
        FIELD rx_rgst_nack_hit          : 1;
        FIELD rsv_21                    : 3;
        FIELD snf_pktcnt_err_hit        : 1;
        FIELD tx_gntst_nosog_hit        : 1;
        FIELD tx_gntst_fifound_hit      : 1;
        FIELD tx_gntph_fifound_hit      : 1;
        FIELD tx_gntph_exceed_hit       : 1;
        FIELD rsv_14                    : 2;
        FIELD schgnt_stinv_hit          : 1;
        FIELD schgnt_idxinv_hit         : 1;
        FIELD rsv_11                    : 1;
        FIELD decrpt_vecm_acccol_hit    : 1;
        FIELD rcv_len_long_hit          : 1;
        FIELD rcv_len_short_hit         : 1;
        FIELD rsv_7                     : 1;
        FIELD rcv_mpcp_crcerr_hit       : 1;
        FIELD rcv_eth_crcerr_hit        : 1;
        FIELD rcv_crc8err_hit           : 1;
        FIELD rsv_3                     : 1;
        FIELD rcv_nrlgate_llidbc_hit    : 1;
        FIELD rcv_dscvgate_filt_hit     : 1;
        FIELD rcv_dscvgate_infomis_hit  : 1;
#else
        FIELD rcv_dscvgate_infomis_hit  : 1;
        FIELD rcv_dscvgate_filt_hit     : 1;
        FIELD rcv_nrlgate_llidbc_hit    : 1;
        FIELD rsv_3                     : 1;
        FIELD rcv_crc8err_hit           : 1;
        FIELD rcv_eth_crcerr_hit        : 1;
        FIELD rcv_mpcp_crcerr_hit       : 1;
        FIELD rsv_7                     : 1;
        FIELD rcv_len_short_hit         : 1;
        FIELD rcv_len_long_hit          : 1;
        FIELD decrpt_vecm_acccol_hit    : 1;
        FIELD rsv_11                    : 1;
        FIELD schgnt_idxinv_hit         : 1;
        FIELD schgnt_stinv_hit          : 1;
        FIELD rsv_14                    : 2;
        FIELD tx_gntph_exceed_hit       : 1;
        FIELD tx_gntph_fifound_hit      : 1;
        FIELD tx_gntst_fifound_hit      : 1;
        FIELD tx_gntst_nosog_hit        : 1;
        FIELD snf_pktcnt_err_hit        : 1;
        FIELD rsv_21                    : 3;
        FIELD rx_rgst_nack_hit          : 1;
        FIELD rx_rgst_dergst_hit        : 1;
        FIELD rsv_26                    : 6;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_exc_sts, *PREG_e_exc_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_8                     : 24;
        FIELD olt_dscvinfo_match        : 4;
        FIELD olt_dscvinfo_mis          : 4;
#else
        FIELD olt_dscvinfo_mis          : 4;
        FIELD olt_dscvinfo_match        : 4;
        FIELD rsv_8                     : 24;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_olt_dscvinfo, *PREG_e_olt_dscvinfo;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD lsroff_time_olt           : 8;
        FIELD lsron_time_olt            : 8;
        FIELD sync_time_olt             : 16;
#else
        FIELD sync_time_olt             : 16;
        FIELD lsron_time_olt            : 8;
        FIELD lsroff_time_olt           : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_laser_onoff_time2, *PREG_e_laser_onoff_time2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u10g_tx_tsadj             : 16;
        FIELD u10g_tx_stmadj            : 16;
#else
        FIELD u10g_tx_stmadj            : 16;
        FIELD u10g_tx_tsadj             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_trx_adjtime3, *PREG_e_trx_adjtime3;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD d10g_rx_tsadj             : 16;
        FIELD rsv_13                    : 3;
        FIELD d10g_rx_tsadj_ofst        : 5;
        FIELD rsv_0                     : 8;
#else
        FIELD rsv_0                     : 8;
        FIELD d10g_rx_tsadj_ofst        : 5;
        FIELD rsv_13                    : 3;
        FIELD d10g_rx_tsadj             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_trx_adjtime4, *PREG_e_trx_adjtime4;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u1g_tx_tsadj              : 16;
        FIELD rsv_0                     : 16;
#else
        FIELD rsv_0                     : 16;
        FIELD u1g_tx_tsadj              : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_trx_adjtime5, *PREG_e_trx_adjtime5;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rx_phydly_dft             : 1;
        FIELD rsv_28                    : 3;
        FIELD d1g_rx_phydly_ofst        : 1;
        FIELD d1g_rx_phydly             : 11;
        FIELD d10g_rx_phydly_ofst       : 5;
        FIELD d10g_rx_phydly            : 11;
#else
        FIELD d10g_rx_phydly            : 11;
        FIELD d10g_rx_phydly_ofst       : 5;
        FIELD d1g_rx_phydly             : 11;
        FIELD d1g_rx_phydly_ofst        : 1;
        FIELD rsv_28                    : 3;
        FIELD rx_phydly_dft             : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxphydly_adjtime, *PREG_e_rxphydly_adjtime;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD sync_time_maxen           : 1;
        FIELD lsroff_time_maxen         : 1;
        FIELD lsron_time_maxen          : 1;
        FIELD rsv_24                    : 5;
        FIELD sync_time_maxval          : 8;
        FIELD lsroff_time_maxval        : 8;
        FIELD lsron_time_maxval         : 8;
#else
        FIELD lsron_time_maxval         : 8;
        FIELD lsroff_time_maxval        : 8;
        FIELD sync_time_maxval          : 8;
        FIELD rsv_24                    : 5;
        FIELD lsron_time_maxen          : 1;
        FIELD lsroff_time_maxen         : 1;
        FIELD sync_time_maxen           : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_overhead_time_thr, *PREG_e_overhead_time_thr;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD ipgalign_mtd              : 1;
        FIELD rsv_16                    : 15;
        FIELD u10g_dscv_gntlen          : 8;
        FIELD u10g_tail_grd             : 8;
#else
        FIELD u10g_tail_grd             : 8;
        FIELD u10g_dscv_gntlen          : 8;
        FIELD rsv_16                    : 15;
        FIELD ipgalign_mtd              : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_txcal_cnst2, *PREG_e_txcal_cnst2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u1g_fecon_min_gntlen      : 8;
        FIELD u10g_eoblen               : 8;
        FIELD u10g_min_gntlen           : 8;
        FIELD u1g_fecoff_min_gntlen     : 8;
#else
        FIELD u1g_fecoff_min_gntlen     : 8;
        FIELD u10g_min_gntlen           : 8;
        FIELD u10g_eoblen               : 8;
        FIELD u1g_fecon_min_gntlen      : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_txcal_cnst3, *PREG_e_txcal_cnst3;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD txsch_dmy0                : 3;
        FIELD rpt_qsize_mgnt_sel        : 1; 
        FIELD rsv_2                     : 2;
        FIELD txfifo_pad_hthr           : 10;
        FIELD txsch_dmy1                : 6;
        FIELD txfifo_pad_lthr           : 10;
#else
        FIELD txfifo_pad_lthr           : 10;
        FIELD txsch_dmy1                : 6;
        FIELD txfifo_pad_hthr           : 10;
        FIELD rsv_2                     : 2;
        FIELD rpt_qsize_mgnt_sel        : 1;
        FIELD txsch_dmy0                : 3;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_txsch_cfg, *PREG_e_txsch_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_5                     : 27;
        FIELD rxfifo_mbithr             : 5;
#else
        FIELD rxfifo_mbithr             : 5;
        FIELD rsv_5                     : 27;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxfifo_thr, *PREG_e_rxfifo_thr;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD d10g_bcllid               : 16;
        FIELD d1g_bcllid                : 16;
#else
        FIELD d1g_bcllid                : 16;
        FIELD d10g_bcllid               : 16;
#endif /* __BIG_ENDIAN */
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
#ifdef __BIG_ENDIAN
        FIELD tx_mpcp_addrh             : 16;
        FIELD tx_mpcp_etype             : 16;
#else
        FIELD tx_mpcp_etype             : 16;
        FIELD tx_mpcp_addrh             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_txfrm_cfg2, *PREG_e_txfrm_cfg2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD tx_rgreq_op               : 16;
        FIELD tx_rgack_op               : 16;
#else
        FIELD tx_rgack_op               : 16;
        FIELD tx_rgreq_op               : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_txfrm_cfg3, *PREG_e_txfrm_cfg3;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u1g_fecoff_rpt_q1_qsizeadj : 16;
        FIELD u1g_fecoff_rpt_q0_qsizeadj : 16;
#else
        FIELD u1g_fecoff_rpt_q0_qsizeadj : 16;
        FIELD u1g_fecoff_rpt_q1_qsizeadj : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u1g_rpt_qsizeadj1, *PREG_e_u1g_rpt_qsizeadj1;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u1g_fecoff_rpt_q3_qsizeadj : 16;
        FIELD u1g_fecoff_rpt_q2_qsizeadj : 16;
#else
        FIELD u1g_fecoff_rpt_q2_qsizeadj : 16;
        FIELD u1g_fecoff_rpt_q3_qsizeadj : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u1g_rpt_qsizeadj2, *PREG_e_u1g_rpt_qsizeadj2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u1g_fecoff_rpt_q5_qsizeadj : 16;
        FIELD u1g_fecoff_rpt_q4_qsizeadj : 16;
#else
        FIELD u1g_fecoff_rpt_q4_qsizeadj : 16;
        FIELD u1g_fecoff_rpt_q5_qsizeadj : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u1g_rpt_qsizeadj3, *PREG_e_u1g_rpt_qsizeadj3;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u1g_fecoff_rpt_q7_qsizeadj : 16;
        FIELD u1g_fecoff_rpt_q6_qsizeadj : 16;
#else
        FIELD u1g_fecoff_rpt_q6_qsizeadj : 16;
        FIELD u1g_fecoff_rpt_q7_qsizeadj : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u1g_rpt_qsizeadj4, *PREG_e_u1g_rpt_qsizeadj4;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u1g_fecon_rpt_q1_qsizeadj : 16;
        FIELD u1g_fecon_rpt_q0_qsizeadj : 16;
#else
        FIELD u1g_fecon_rpt_q0_qsizeadj : 16;
        FIELD u1g_fecon_rpt_q1_qsizeadj : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u1g_rpt_qsizeadj5, *PREG_e_u1g_rpt_qsizeadj5;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u1g_fecon_rpt_q3_qsizeadj : 16;
        FIELD u1g_fecon_rpt_q2_qsizeadj : 16;
#else
        FIELD u1g_fecon_rpt_q2_qsizeadj : 16;
        FIELD u1g_fecon_rpt_q3_qsizeadj : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u1g_rpt_qsizeadj6, *PREG_e_u1g_rpt_qsizeadj6;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u1g_fecon_rpt_q5_qsizeadj : 16;
        FIELD u1g_fecon_rpt_q4_qsizeadj : 16;
#else
        FIELD u1g_fecon_rpt_q4_qsizeadj : 16;
        FIELD u1g_fecon_rpt_q5_qsizeadj : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u1g_rpt_qsizeadj7, *PREG_e_u1g_rpt_qsizeadj7;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u1g_fecon_rpt_q7_qsizeadj : 16;
        FIELD u1g_fecon_rpt_q6_qsizeadj : 16;
#else
        FIELD u1g_fecon_rpt_q6_qsizeadj : 16;
        FIELD u1g_fecon_rpt_q7_qsizeadj : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u1g_rpt_qsizeadj8, *PREG_e_u1g_rpt_qsizeadj8;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u10g_rpt_q1_qsizeadj      : 16;
        FIELD u10g_rpt_q0_qsizeadj      : 16;
#else
        FIELD u10g_rpt_q0_qsizeadj      : 16;
        FIELD u10g_rpt_q1_qsizeadj      : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u10g_rpt_qsizeadj1, *PREG_e_u10g_rpt_qsizeadj1;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u10g_rpt_q3_qsizeadj      : 16;
        FIELD u10g_rpt_q2_qsizeadj      : 16;
#else
        FIELD u10g_rpt_q2_qsizeadj      : 16;
        FIELD u10g_rpt_q3_qsizeadj      : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u10g_rpt_qsizeadj2, *PREG_e_u10g_rpt_qsizeadj2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u10g_rpt_q5_qsizeadj      : 16;
        FIELD u10g_rpt_q4_qsizeadj      : 16;
#else
        FIELD u10g_rpt_q4_qsizeadj      : 16;
        FIELD u10g_rpt_q5_qsizeadj      : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u10g_rpt_qsizeadj3, *PREG_e_u10g_rpt_qsizeadj3;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD u10g_rpt_q7_qsizeadj      : 16;
        FIELD u10g_rpt_q6_qsizeadj      : 16;
#else
        FIELD u10g_rpt_q6_qsizeadj      : 16;
        FIELD u10g_rpt_q7_qsizeadj      : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_u10g_rpt_qsizeadj4, *PREG_e_u10g_rpt_qsizeadj4;

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
#ifdef __BIG_ENDIAN
        FIELD snf_pkt_dal               : 16;
        FIELD snf_pkt_sah               : 16;
#else
        FIELD snf_pkt_sah               : 16;
        FIELD snf_pkt_dal               : 16;
#endif /* __BIG_ENDIAN */
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
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD snf_pkt_etype             : 16;
#else
        FIELD snf_pkt_etype             : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_snf_etype, *PREG_e_snf_etype;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxuni_mcllid1             : 16;
        FIELD rxuni_mcllid0             : 16;
#else
        FIELD rxuni_mcllid0             : 16;
        FIELD rxuni_mcllid1             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg0, *PREG_e_rxuni_mcllid_cfg0;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxuni_mcllid3             : 16;
        FIELD rxuni_mcllid2             : 16;
#else
        FIELD rxuni_mcllid2             : 16;
        FIELD rxuni_mcllid3             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg1, *PREG_e_rxuni_mcllid_cfg1;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxuni_mcllid5             : 16;
        FIELD rxuni_mcllid4             : 16;
#else
        FIELD rxuni_mcllid4             : 16;
        FIELD rxuni_mcllid5             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg2, *PREG_e_rxuni_mcllid_cfg2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxuni_mcllid7             : 16;
        FIELD rxuni_mcllid6             : 16;
#else
        FIELD rxuni_mcllid6             : 16;
        FIELD rxuni_mcllid7             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg3, *PREG_e_rxuni_mcllid_cfg3;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxuni_mcllid9             : 16;
        FIELD rxuni_mcllid8             : 16;
#else
        FIELD rxuni_mcllid8             : 16;
        FIELD rxuni_mcllid9             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg4, *PREG_e_rxuni_mcllid_cfg4;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxuni_mcllid11            : 16;
        FIELD rxuni_mcllid10            : 16;
#else
        FIELD rxuni_mcllid10            : 16;
        FIELD rxuni_mcllid11            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg5, *PREG_e_rxuni_mcllid_cfg5;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxuni_mcllid13            : 16;
        FIELD rxuni_mcllid12            : 16;
#else
        FIELD rxuni_mcllid12            : 16;
        FIELD rxuni_mcllid13            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg6, *PREG_e_rxuni_mcllid_cfg6;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxuni_mcllid15            : 16;
        FIELD rxuni_mcllid14            : 16;
#else
        FIELD rxuni_mcllid14            : 16;
        FIELD rxuni_mcllid15            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxuni_mcllid_cfg7, *PREG_e_rxuni_mcllid_cfg7;

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
#ifdef __BIG_ENDIAN
        FIELD rsv_30                    : 2;
        FIELD txmpi_fifound_pktgate_en  : 1;
        FIELD rsv_11                    : 18;
        FIELD txmpi_fifound_thr         : 11;
#else
        FIELD txmpi_fifound_thr         : 11;
        FIELD rsv_11                    : 18;
        FIELD txmpi_fifound_pktgate_en  : 1;
        FIELD rsv_30                    : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_glue_cfg, *PREG_e_glue_cfg;

typedef PACKING union
{
    PACKING struct
    {
        FIELD gntreq_grdcyc             : 32;
    } Bits;
    UINT32 Raw;
} REG_e_gntreq_tmout, *PREG_e_gntreq_tmout;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_2                     : 30;
        FIELD xepon_type                : 2;
#else
        FIELD xepon_type                : 2;
        FIELD rsv_2                     : 30;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_glb_sts, *PREG_e_glb_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD glb_cntclr                : 1;
#else
        FIELD glb_cntclr                : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_cnt_clr, *PREG_e_cnt_clr;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rx_infomis_dscvgate_cnt   : 8;
        FIELD rx_dauc_dscvgate_cnt      : 8;
        FIELD rx_filt_dscvgate_cnt      : 8;
        FIELD rsv_0                     : 8;
#else
        FIELD rsv_0                     : 8;
        FIELD rx_filt_dscvgate_cnt      : 8;
        FIELD rx_dauc_dscvgate_cnt      : 8;
        FIELD rx_infomis_dscvgate_cnt   : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxadv_cnt, *PREG_e_rxadv_cnt;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD txmbi_dygasp_cnt          : 8;
        FIELD txmpi_dygasp_cnt          : 8;
        FIELD rsv_0                     : 16;
#else
        FIELD rsv_0                     : 16;
        FIELD txmpi_dygasp_cnt          : 8;
        FIELD txmbi_dygasp_cnt          : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_tx_dygasp_cnt, *PREG_e_tx_dygasp_cnt;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD txmbi_rpt_cnt             : 16;
        FIELD txmpi_rpt_cnt             : 16;
#else
        FIELD txmpi_rpt_cnt             : 16;
        FIELD txmbi_rpt_cnt             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_tx_rpt_cnt, *PREG_e_tx_rpt_cnt;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD txmpi_oam_cnt             : 16;
        FIELD txmpi_rgreq_cnt           : 8;
        FIELD txmpi_rgack_cnt           : 8;
#else
        FIELD txmpi_rgack_cnt           : 8;
        FIELD txmpi_rgreq_cnt           : 8;
        FIELD txmpi_oam_cnt             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_txmpi_mpcp_cnt, *PREG_e_txmpi_mpcp_cnt;

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
#ifdef __BIG_ENDIAN
        FIELD rxmpi_crceth_cnt          : 16;
        FIELD rxmpi_crc8err_cnt         : 16;
#else
        FIELD rxmpi_crc8err_cnt         : 16;
        FIELD rxmpi_crceth_cnt          : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_crc8err_cnt, *PREG_e_rxmpi_crc8err_cnt;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD rxmpi_eofdrop_cnt         : 16;
#else
        FIELD rxmpi_eofdrop_cnt         : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_drop_cnt, *PREG_e_rxmpi_drop_cnt;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxmpi_abchurn_cnt         : 16;
        FIELD rxmpi_churn_cnt           : 16;
#else
        FIELD rxmpi_churn_cnt           : 16;
        FIELD rxmpi_abchurn_cnt         : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_churn_cnt, *PREG_e_rxmpi_churn_cnt;

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
#ifdef __BIG_ENDIAN
        FIELD rxmbi_sofdrop_cnt         : 16;
        FIELD rxmbi_snfdrop_cnt         : 16;
#else
        FIELD rxmbi_snfdrop_cnt         : 16;
        FIELD rxmbi_sofdrop_cnt         : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxmbi_drop_cnt, *PREG_e_rxmbi_drop_cnt;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxmbi_crcerr_cnt          : 16;
        FIELD rxmbi_enddrop_cnt         : 16;
#else
        FIELD rxmbi_enddrop_cnt         : 16;
        FIELD rxmbi_crcerr_cnt          : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxmbi_drop_cnt2, *PREG_e_rxmbi_drop_cnt2;

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
        FIELD txmbi_uceth_cnt           : 32;
    } Bits;
    UINT32 Raw;
} REG_e_txmbi_uceth_cnt, *PREG_e_txmbi_uceth_cnt;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD txmbi_bceth_cnt           : 16;
        FIELD txmbi_mceth_cnt           : 16;
#else
        FIELD txmbi_mceth_cnt           : 16;
        FIELD txmbi_bceth_cnt           : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_txmbi_mceth_cnt, *PREG_e_txmbi_mceth_cnt;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD txmbi_err_cnt             : 16;
#else
        FIELD txmbi_err_cnt             : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_txmbi_err_cnt, *PREG_e_txmbi_err_cnt;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_29                    : 3;
        FIELD tx_cur_time_drift_ofst    : 5;
        FIELD rsv_21                    : 3;
        FIELD tx_max_time_drift_ofst    : 5;
        FIELD tx_cur_time_drift         : 8;
        FIELD tx_max_time_drift         : 8;
#else
        FIELD tx_max_time_drift         : 8;
        FIELD tx_cur_time_drift         : 8;
        FIELD tx_max_time_drift_ofst    : 5;
        FIELD rsv_21                    : 3;
        FIELD tx_cur_time_drift_ofst    : 5;
        FIELD rsv_29                    : 3;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_tx_timedrift_stat, *PREG_e_tx_timedrift_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxmpi_churn_mpcpcnt       : 16;
        FIELD rxmpi_churn_ethcnt        : 16;
#else
        FIELD rxmpi_churn_ethcnt        : 16;
        FIELD rxmpi_churn_mpcpcnt       : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_churn_cnt2, *PREG_e_rxmpi_churn_cnt2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rxmpi_churn_keyidx        : 1;
        FIELD rsv_29                    : 2;
        FIELD rxmpi_churn_llididx       : 5;
        FIELD rxmpi_churncrc_keyidx     : 1;
        FIELD rsv_21                    : 2;
        FIELD rxmpi_churncrc_llididx    : 5;
        FIELD rxmpi_churn_crccnt        : 16;
#else
        FIELD rxmpi_churn_crccnt        : 16;
        FIELD rxmpi_churncrc_llididx    : 5;
        FIELD rsv_21                    : 2;
        FIELD rxmpi_churncrc_keyidx     : 1;
        FIELD rxmpi_churn_llididx       : 5;
        FIELD rsv_29                    : 2;
        FIELD rxmpi_churn_keyidx        : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxmpi_churn_cnt3, *PREG_e_rxmpi_churn_cnt3;

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
#ifdef __BIG_ENDIAN
        FIELD txmpi_bceth_cnt           : 16;
        FIELD txmpi_mceth_cnt           : 16;
#else
        FIELD txmpi_mceth_cnt           : 16;
        FIELD txmpi_bceth_cnt           : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_txmpi_mceth_cnt, *PREG_e_txmpi_mceth_cnt;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid9_gnt_cnt             : 16;
        FIELD llid8_gnt_cnt             : 16;
#else
        FIELD llid8_gnt_cnt             : 16;
        FIELD llid9_gnt_cnt             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid8_9_gnt_stat, *PREG_e_llid8_9_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid11_gnt_cnt            : 16;
        FIELD llid10_gnt_cnt            : 16;
#else
        FIELD llid10_gnt_cnt            : 16;
        FIELD llid11_gnt_cnt            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid10_11_gnt_stat, *PREG_e_llid10_11_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid13_gnt_cnt            : 16;
        FIELD llid12_gnt_cnt            : 16;
#else
        FIELD llid12_gnt_cnt            : 16;
        FIELD llid13_gnt_cnt            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid12_13_gnt_stat, *PREG_e_llid12_13_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid15_gnt_cnt            : 16;
        FIELD llid14_gnt_cnt            : 16;
#else
        FIELD llid14_gnt_cnt            : 16;
        FIELD llid15_gnt_cnt            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid14_15_gnt_stat, *PREG_e_llid14_15_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid17_gnt_cnt            : 16;
        FIELD llid16_gnt_cnt            : 16;
#else
        FIELD llid16_gnt_cnt            : 16;
        FIELD llid17_gnt_cnt            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid16_17_gnt_stat, *PREG_e_llid16_17_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid19_gnt_cnt            : 16;
        FIELD llid18_gnt_cnt            : 16;
#else
        FIELD llid18_gnt_cnt            : 16;
        FIELD llid19_gnt_cnt            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid18_19_gnt_stat, *PREG_e_llid18_19_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid21_gnt_cnt            : 16;
        FIELD llid20_gnt_cnt            : 16;
#else
        FIELD llid20_gnt_cnt            : 16;
        FIELD llid21_gnt_cnt            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid20_21_gnt_stat, *PREG_e_llid20_21_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid23_gnt_cnt            : 16;
        FIELD llid22_gnt_cnt            : 16;
#else
        FIELD llid22_gnt_cnt            : 16;
        FIELD llid23_gnt_cnt            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid22_23_gnt_stat, *PREG_e_llid22_23_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid25_gnt_cnt            : 16;
        FIELD llid24_gnt_cnt            : 16;
#else
        FIELD llid24_gnt_cnt            : 16;
        FIELD llid25_gnt_cnt            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid24_25_gnt_stat, *PREG_e_llid24_25_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid27_gnt_cnt            : 16;
        FIELD llid26_gnt_cnt            : 16;
#else
        FIELD llid26_gnt_cnt            : 16;
        FIELD llid27_gnt_cnt            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid26_27_gnt_stat, *PREG_e_llid26_27_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid29_gnt_cnt            : 16;
        FIELD llid28_gnt_cnt            : 16;
#else
        FIELD llid28_gnt_cnt            : 16;
        FIELD llid29_gnt_cnt            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid28_29_gnt_stat, *PREG_e_llid28_29_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid31_gnt_cnt            : 16;
        FIELD llid30_gnt_cnt            : 16;
#else
        FIELD llid30_gnt_cnt            : 16;
        FIELD llid31_gnt_cnt            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid30_31_gnt_stat, *PREG_e_llid30_31_gnt_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD max_rxfifo_depth          : 16;
#else
        FIELD max_rxfifo_depth          : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rxfifo_depth_stat, *PREG_e_rxfifo_depth_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD rdmdly                    : 16;
#else
        FIELD rdmdly                    : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rdmdly_stat, *PREG_e_rdmdly_stat;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD llid31_rcv_rgst_int       : 1;
        FIELD llid30_rcv_rgst_int       : 1;
        FIELD llid29_rcv_rgst_int       : 1;
        FIELD llid28_rcv_rgst_int       : 1;
        FIELD llid27_rcv_rgst_int       : 1;
        FIELD llid26_rcv_rgst_int       : 1;
        FIELD llid25_rcv_rgst_int       : 1;
        FIELD llid24_rcv_rgst_int       : 1;
        FIELD llid23_rcv_rgst_int       : 1;
        FIELD llid22_rcv_rgst_int       : 1;
        FIELD llid21_rcv_rgst_int       : 1;
        FIELD llid20_rcv_rgst_int       : 1;
        FIELD llid19_rcv_rgst_int       : 1;
        FIELD llid18_rcv_rgst_int       : 1;
        FIELD llid17_rcv_rgst_int       : 1;
        FIELD llid16_rcv_rgst_int       : 1;
        FIELD llid15_rcv_rgst_int       : 1;
        FIELD llid14_rcv_rgst_int       : 1;
        FIELD llid13_rcv_rgst_int       : 1;
        FIELD llid12_rcv_rgst_int       : 1;
        FIELD llid11_rcv_rgst_int       : 1;
        FIELD llid10_rcv_rgst_int       : 1;
        FIELD llid9_rcv_rgst_int        : 1;
        FIELD llid8_rcv_rgst_int        : 1;
#else
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
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_int_sts3, *PREG_e_int_sts3;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD llid31_rcv_rgst_en        : 1;
        FIELD llid30_rcv_rgst_en        : 1;
        FIELD llid29_rcv_rgst_en        : 1;
        FIELD llid28_rcv_rgst_en        : 1;
        FIELD llid27_rcv_rgst_en        : 1;
        FIELD llid26_rcv_rgst_en        : 1;
        FIELD llid25_rcv_rgst_en        : 1;
        FIELD llid24_rcv_rgst_en        : 1;
        FIELD llid23_rcv_rgst_en        : 1;
        FIELD llid22_rcv_rgst_en        : 1;
        FIELD llid21_rcv_rgst_en        : 1;
        FIELD llid20_rcv_rgst_en        : 1;
        FIELD llid19_rcv_rgst_en        : 1;
        FIELD llid18_rcv_rgst_en        : 1;
        FIELD llid17_rcv_rgst_en        : 1;
        FIELD llid16_rcv_rgst_en        : 1;
        FIELD llid15_rcv_rgst_en        : 1;
        FIELD llid14_rcv_rgst_en        : 1;
        FIELD llid13_rcv_rgst_en        : 1;
        FIELD llid12_rcv_rgst_en        : 1;
        FIELD llid11_rcv_rgst_en        : 1;
        FIELD llid10_rcv_rgst_en        : 1;
        FIELD llid9_rcv_rgst_en         : 1;
        FIELD llid8_rcv_rgst_en         : 1;
#else
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
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_int_en3, *PREG_e_int_en3;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid31_mpcp_tmo           : 1;
        FIELD llid30_mpcp_tmo           : 1;
        FIELD llid29_mpcp_tmo           : 1;
        FIELD llid28_mpcp_tmo           : 1;
        FIELD llid27_mpcp_tmo           : 1;
        FIELD llid26_mpcp_tmo           : 1;
        FIELD llid25_mpcp_tmo           : 1;
        FIELD llid24_mpcp_tmo           : 1;
        FIELD llid23_mpcp_tmo           : 1;
        FIELD llid22_mpcp_tmo           : 1;
        FIELD llid21_mpcp_tmo           : 1;
        FIELD llid20_mpcp_tmo           : 1;
        FIELD llid19_mpcp_tmo           : 1;
        FIELD llid18_mpcp_tmo           : 1;
        FIELD llid17_mpcp_tmo           : 1;
        FIELD llid16_mpcp_tmo           : 1;
        FIELD llid31_rpt_tmo            : 1;
        FIELD llid30_rpt_tmo            : 1;
        FIELD llid29_rpt_tmo            : 1;
        FIELD llid28_rpt_tmo            : 1;
        FIELD llid27_rpt_tmo            : 1;
        FIELD llid26_rpt_tmo            : 1;
        FIELD llid25_rpt_tmo            : 1;
        FIELD llid24_rpt_tmo            : 1;
        FIELD llid23_rpt_tmo            : 1;
        FIELD llid22_rpt_tmo            : 1;
        FIELD llid21_rpt_tmo            : 1;
        FIELD llid20_rpt_tmo            : 1;
        FIELD llid19_rpt_tmo            : 1;
        FIELD llid18_rpt_tmo            : 1;
        FIELD llid17_rpt_tmo            : 1;
        FIELD llid16_rpt_tmo            : 1;
#else
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
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rpt_mpcp_timout2, *PREG_e_rpt_mpcp_timout2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid11_dummy              : 4;
        FIELD llid11_txfec_en           : 1;
        FIELD llid11_dcrypt_en          : 1;
        FIELD llid11_dcrypt_mode        : 1;
        FIELD llid11_oamlpbk_en         : 1;
        FIELD llid10_dummy              : 4;
        FIELD llid10_txfec_en           : 1;
        FIELD llid10_dcrypt_en          : 1;
        FIELD llid10_dcrypt_mode        : 1;
        FIELD llid10_oamlpbk_en         : 1;
        FIELD llid9_dummy               : 4;
        FIELD llid9_txfec_en            : 1;
        FIELD llid9_dcrypt_en           : 1;
        FIELD llid9_dcrypt_mode         : 1;
        FIELD llid9_oamlpbk_en          : 1;
        FIELD llid8_dummy               : 4;
        FIELD llid8_txfec_en            : 1;
        FIELD llid8_dcrypt_en           : 1;
        FIELD llid8_dcrypt_mode         : 1;
        FIELD llid8_oamlpbk_en          : 1;
#else
        FIELD llid8_oamlpbk_en          : 1;
        FIELD llid8_dcrypt_mode         : 1;
        FIELD llid8_dcrypt_en           : 1;
        FIELD llid8_txfec_en            : 1;
        FIELD llid8_dummy               : 4;
        FIELD llid9_oamlpbk_en          : 1;
        FIELD llid9_dcrypt_mode         : 1;
        FIELD llid9_dcrypt_en           : 1;
        FIELD llid9_txfec_en            : 1;
        FIELD llid9_dummy               : 4;
        FIELD llid10_oamlpbk_en         : 1;
        FIELD llid10_dcrypt_mode        : 1;
        FIELD llid10_dcrypt_en          : 1;
        FIELD llid10_txfec_en           : 1;
        FIELD llid10_dummy              : 4;
        FIELD llid11_oamlpbk_en         : 1;
        FIELD llid11_dcrypt_mode        : 1;
        FIELD llid11_dcrypt_en          : 1;
        FIELD llid11_txfec_en           : 1;
        FIELD llid11_dummy              : 4;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid8_11_cfg, *PREG_e_llid8_11_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid15_dummy              : 4;
        FIELD llid15_txfec_en           : 1;
        FIELD llid15_dcrypt_en          : 1;
        FIELD llid15_dcrypt_mode        : 1;
        FIELD llid15_oamlpbk_en         : 1;
        FIELD llid14_dummy              : 4;
        FIELD llid14_txfec_en           : 1;
        FIELD llid14_dcrypt_en          : 1;
        FIELD llid14_dcrypt_mode        : 1;
        FIELD llid14_oamlpbk_en         : 1;
        FIELD llid13_dummy              : 4;
        FIELD llid13_txfec_en           : 1;
        FIELD llid13_dcrypt_en          : 1;
        FIELD llid13_dcrypt_mode        : 1;
        FIELD llid13_oamlpbk_en         : 1;
        FIELD llid12_dummy              : 4;
        FIELD llid12_txfec_en           : 1;
        FIELD llid12_dcrypt_en          : 1;
        FIELD llid12_dcrypt_mode        : 1;
        FIELD llid12_oam_lpbk_en        : 1;
#else
        FIELD llid12_oam_lpbk_en        : 1;
        FIELD llid12_dcrypt_mode        : 1;
        FIELD llid12_dcrypt_en          : 1;
        FIELD llid12_txfec_en           : 1;
        FIELD llid12_dummy              : 4;
        FIELD llid13_oamlpbk_en         : 1;
        FIELD llid13_dcrypt_mode        : 1;
        FIELD llid13_dcrypt_en          : 1;
        FIELD llid13_txfec_en           : 1;
        FIELD llid13_dummy              : 4;
        FIELD llid14_oamlpbk_en         : 1;
        FIELD llid14_dcrypt_mode        : 1;
        FIELD llid14_dcrypt_en          : 1;
        FIELD llid14_txfec_en           : 1;
        FIELD llid14_dummy              : 4;
        FIELD llid15_oamlpbk_en         : 1;
        FIELD llid15_dcrypt_mode        : 1;
        FIELD llid15_dcrypt_en          : 1;
        FIELD llid15_txfec_en           : 1;
        FIELD llid15_dummy              : 4;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid12_15_cfg, *PREG_e_llid12_15_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid19_dummy              : 4;
        FIELD llid19_txfec_en           : 1;
        FIELD llid19_dcrypt_en          : 1;
        FIELD llid19_dcrypt_mode        : 1;
        FIELD llid19_oamlpbk_en         : 1;
        FIELD llid18_dummy              : 4;
        FIELD llid18_txfec_en           : 1;
        FIELD llid18_dcrypt_en          : 1;
        FIELD llid18_dcrypt_mode        : 1;
        FIELD llid18_oamlpbk_en         : 1;
        FIELD llid17_dummy              : 4;
        FIELD llid17_txfec_en           : 1;
        FIELD llid17_dcrypt_en          : 1;
        FIELD llid17_dcrypt_mode        : 1;
        FIELD llid17_oamlpbk_en         : 1;
        FIELD llid16_dummy              : 4;
        FIELD llid16_txfec_en           : 1;
        FIELD llid16_dcrypt_en          : 1;
        FIELD llid16_dcrypt_mode        : 1;
        FIELD llid16_oam_lpbk_en        : 1;
#else
        FIELD llid16_oam_lpbk_en        : 1;
        FIELD llid16_dcrypt_mode        : 1;
        FIELD llid16_dcrypt_en          : 1;
        FIELD llid16_txfec_en           : 1;
        FIELD llid16_dummy              : 4;
        FIELD llid17_oamlpbk_en         : 1;
        FIELD llid17_dcrypt_mode        : 1;
        FIELD llid17_dcrypt_en          : 1;
        FIELD llid17_txfec_en           : 1;
        FIELD llid17_dummy              : 4;
        FIELD llid18_oamlpbk_en         : 1;
        FIELD llid18_dcrypt_mode        : 1;
        FIELD llid18_dcrypt_en          : 1;
        FIELD llid18_txfec_en           : 1;
        FIELD llid18_dummy              : 4;
        FIELD llid19_oamlpbk_en         : 1;
        FIELD llid19_dcrypt_mode        : 1;
        FIELD llid19_dcrypt_en          : 1;
        FIELD llid19_txfec_en           : 1;
        FIELD llid19_dummy              : 4;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid16_19_cfg, *PREG_e_llid16_19_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid23_dummy              : 4;
        FIELD llid23_txfec_en           : 1;
        FIELD llid23_dcrypt_en          : 1;
        FIELD llid23_dcrypt_mode        : 1;
        FIELD llid23_oamlpbk_en         : 1;
        FIELD llid22_dummy              : 4;
        FIELD llid22_txfec_en           : 1;
        FIELD llid22_dcrypt_en          : 1;
        FIELD llid22_dcrypt_mode        : 1;
        FIELD llid22_oamlpbk_en         : 1;
        FIELD llid21_dummy              : 4;
        FIELD llid21_txfec_en           : 1;
        FIELD llid21_dcrypt_en          : 1;
        FIELD llid21_dcrypt_mode        : 1;
        FIELD llid21_oamlpbk_en         : 1;
        FIELD llid20_dummy              : 4;
        FIELD llid20_txfec_en           : 1;
        FIELD llid20_dcrypt_en          : 1;
        FIELD llid20_dcrypt_mode        : 1;
        FIELD llid20_oam_lpbk_en        : 1;
#else
        FIELD llid20_oam_lpbk_en        : 1;
        FIELD llid20_dcrypt_mode        : 1;
        FIELD llid20_dcrypt_en          : 1;
        FIELD llid20_txfec_en           : 1;
        FIELD llid20_dummy              : 4;
        FIELD llid21_oamlpbk_en         : 1;
        FIELD llid21_dcrypt_mode        : 1;
        FIELD llid21_dcrypt_en          : 1;
        FIELD llid21_txfec_en           : 1;
        FIELD llid21_dummy              : 4;
        FIELD llid22_oamlpbk_en         : 1;
        FIELD llid22_dcrypt_mode        : 1;
        FIELD llid22_dcrypt_en          : 1;
        FIELD llid22_txfec_en           : 1;
        FIELD llid22_dummy              : 4;
        FIELD llid23_oamlpbk_en         : 1;
        FIELD llid23_dcrypt_mode        : 1;
        FIELD llid23_dcrypt_en          : 1;
        FIELD llid23_txfec_en           : 1;
        FIELD llid23_dummy              : 4;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid20_23_cfg, *PREG_e_llid20_23_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid27_dummy              : 4;
        FIELD llid27_txfec_en           : 1;
        FIELD llid27_dcrypt_en          : 1;
        FIELD llid27_dcrypt_mode        : 1;
        FIELD llid27_oamlpbk_en         : 1;
        FIELD llid26_dummy              : 4;
        FIELD llid26_txfec_en           : 1;
        FIELD llid26_dcrypt_en          : 1;
        FIELD llid26_dcrypt_mode        : 1;
        FIELD llid26_oamlpbk_en         : 1;
        FIELD llid25_dummy              : 4;
        FIELD llid25_txfec_en           : 1;
        FIELD llid25_dcrypt_en          : 1;
        FIELD llid25_dcrypt_mode        : 1;
        FIELD llid25_oamlpbk_en         : 1;
        FIELD llid24_dummy              : 4;
        FIELD llid24_txfec_en           : 1;
        FIELD llid24_dcrypt_en          : 1;
        FIELD llid24_dcrypt_mode        : 1;
        FIELD llid24_oam_lpbk_en        : 1;
#else
        FIELD llid24_oam_lpbk_en        : 1;
        FIELD llid24_dcrypt_mode        : 1;
        FIELD llid24_dcrypt_en          : 1;
        FIELD llid24_txfec_en           : 1;
        FIELD llid24_dummy              : 4;
        FIELD llid25_oamlpbk_en         : 1;
        FIELD llid25_dcrypt_mode        : 1;
        FIELD llid25_dcrypt_en          : 1;
        FIELD llid25_txfec_en           : 1;
        FIELD llid25_dummy              : 4;
        FIELD llid26_oamlpbk_en         : 1;
        FIELD llid26_dcrypt_mode        : 1;
        FIELD llid26_dcrypt_en          : 1;
        FIELD llid26_txfec_en           : 1;
        FIELD llid26_dummy              : 4;
        FIELD llid27_oamlpbk_en         : 1;
        FIELD llid27_dcrypt_mode        : 1;
        FIELD llid27_dcrypt_en          : 1;
        FIELD llid27_txfec_en           : 1;
        FIELD llid27_dummy              : 4;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid24_27_cfg, *PREG_e_llid24_27_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid31_dummy              : 4;
        FIELD llid31_txfec_en           : 1;
        FIELD llid31_dcrypt_en          : 1;
        FIELD llid31_dcrypt_mode        : 1;
        FIELD llid31_oamlpbk_en         : 1;
        FIELD llid30_dummy              : 4;
        FIELD llid30_txfec_en           : 1;
        FIELD llid30_dcrypt_en          : 1;
        FIELD llid30_dcrypt_mode        : 1;
        FIELD llid30_oamlpbk_en         : 1;
        FIELD llid29_dummy              : 4;
        FIELD llid29_txfec_en           : 1;
        FIELD llid29_dcrypt_en          : 1;
        FIELD llid29_dcrypt_mode        : 1;
        FIELD llid29_oamlpbk_en         : 1;
        FIELD llid28_dummy              : 4;
        FIELD llid28_txfec_en           : 1;
        FIELD llid28_dcrypt_en          : 1;
        FIELD llid28_dcrypt_mode        : 1;
        FIELD llid28_oam_lpbk_en        : 1;
#else
        FIELD llid28_oam_lpbk_en        : 1;
        FIELD llid28_dcrypt_mode        : 1;
        FIELD llid28_dcrypt_en          : 1;
        FIELD llid28_txfec_en           : 1;
        FIELD llid28_dummy              : 4;
        FIELD llid29_oamlpbk_en         : 1;
        FIELD llid29_dcrypt_mode        : 1;
        FIELD llid29_dcrypt_en          : 1;
        FIELD llid29_txfec_en           : 1;
        FIELD llid29_dummy              : 4;
        FIELD llid30_oamlpbk_en         : 1;
        FIELD llid30_dcrypt_mode        : 1;
        FIELD llid30_dcrypt_en          : 1;
        FIELD llid30_txfec_en           : 1;
        FIELD llid30_dummy              : 4;
        FIELD llid31_oamlpbk_en         : 1;
        FIELD llid31_dcrypt_mode        : 1;
        FIELD llid31_dcrypt_en          : 1;
        FIELD llid31_txfec_en           : 1;
        FIELD llid31_dummy              : 4;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid28_31_cfg, *PREG_e_llid28_31_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid8_dscvry_sts          : 2;
        FIELD rsv_26                    : 4;
        FIELD llid8_rgstr_flg_sts       : 2;
        FIELD rsv_17                    : 7;
        FIELD llid8_valid               : 1;
        FIELD llid8_value               : 16;
#else
        FIELD llid8_value               : 16;
        FIELD llid8_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid8_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid8_dscvry_sts          : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid8_dscvry_sts, *PREG_e_llid8_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid9_dscvry_sts          : 2;
        FIELD rsv_26                    : 4;
        FIELD llid9_rgstr_flg_sts       : 2;
        FIELD rsv_17                    : 7;
        FIELD llid9_valid               : 1;
        FIELD llid9_value               : 16;
#else
        FIELD llid9_value               : 16;
        FIELD llid9_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid9_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid9_dscvry_sts          : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid9_dscvry_sts, *PREG_e_llid9_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid10_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid10_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid10_valid              : 1;
        FIELD llid10_value              : 16;
#else
        FIELD llid10_value              : 16;
        FIELD llid10_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid10_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid10_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid10_dscvry_sts, *PREG_e_llid10_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid11_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid11_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid11_valid              : 1;
        FIELD llid11_value              : 16;
#else
        FIELD llid11_value              : 16;
        FIELD llid11_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid11_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid11_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid11_dscvry_sts, *PREG_e_llid11_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid12_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid12_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid12_valid              : 1;
        FIELD llid12_value              : 16;
#else
        FIELD llid12_value              : 16;
        FIELD llid12_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid12_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid12_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid12_dscvry_sts, *PREG_e_llid12_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid13_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid13_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid13_valid              : 1;
        FIELD llid13_value              : 16;
#else
        FIELD llid13_value              : 16;
        FIELD llid13_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid13_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid13_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid13_dscvry_sts, *PREG_e_llid13_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid14_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid14_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid14_valid              : 1;
        FIELD llid14_value              : 16;
#else
        FIELD llid14_value              : 16;
        FIELD llid14_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid14_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid14_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid14_dscvry_sts, *PREG_e_llid14_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid15_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid15_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid15_valid              : 1;
        FIELD llid15_value              : 16;
#else
        FIELD llid15_value              : 16;
        FIELD llid15_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid15_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid15_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid15_dscvry_sts, *PREG_e_llid15_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid16_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid16_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid16_valid              : 1;
        FIELD llid16_value              : 16;
#else
        FIELD llid16_value              : 16;
        FIELD llid16_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid16_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid16_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid16_dscvry_sts, *PREG_e_llid16_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid17_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid17_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid17_valid              : 1;
        FIELD llid17_value              : 16;
#else
        FIELD llid17_value              : 16;
        FIELD llid17_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid17_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid17_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid17_dscvry_sts, *PREG_e_llid17_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid18_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid18_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid18_valid              : 1;
        FIELD llid18_value              : 16;
#else
        FIELD llid18_value              : 16;
        FIELD llid18_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid18_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid18_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid18_dscvry_sts, *PREG_e_llid18_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid19_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid19_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid19_valid              : 1;
        FIELD llid19_value              : 16;
#else
        FIELD llid19_value              : 16;
        FIELD llid19_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid19_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid19_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid19_dscvry_sts, *PREG_e_llid19_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid20_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid20_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid20_valid              : 1;
        FIELD llid20_value              : 16;
#else
        FIELD llid20_value              : 16;
        FIELD llid20_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid20_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid20_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid20_dscvry_sts, *PREG_e_llid20_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid21_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid21_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid21_valid              : 1;
        FIELD llid21_value              : 16;
#else
        FIELD llid21_value              : 16;
        FIELD llid21_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid21_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid21_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid21_dscvry_sts, *PREG_e_llid21_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid22_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid22_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid22_valid              : 1;
        FIELD llid22_value              : 16;
#else
        FIELD llid22_value              : 16;
        FIELD llid22_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid22_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid22_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid22_dscvry_sts, *PREG_e_llid22_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid23_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid23_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid23_valid              : 1;
        FIELD llid23_value              : 16;
#else
        FIELD llid23_value              : 16;
        FIELD llid23_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid23_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid23_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid23_dscvry_sts, *PREG_e_llid23_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid24_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid24_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid24_valid              : 1;
        FIELD llid24_value              : 16;
#else
        FIELD llid24_value              : 16;
        FIELD llid24_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid24_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid24_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid24_dscvry_sts, *PREG_e_llid24_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid25_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid25_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid25_valid              : 1;
        FIELD llid25_value              : 16;
#else
        FIELD llid25_value              : 16;
        FIELD llid25_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid25_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid25_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid25_dscvry_sts, *PREG_e_llid25_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid26_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid26_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid26_valid              : 1;
        FIELD llid26_value              : 16;
#else
        FIELD llid26_value              : 16;
        FIELD llid26_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid26_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid26_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid26_dscvry_sts, *PREG_e_llid26_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid27_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid27_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid27_valid              : 1;
        FIELD llid27_value              : 16;
#else
        FIELD llid27_value              : 16;
        FIELD llid27_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid27_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid27_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid27_dscvry_sts, *PREG_e_llid27_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid28_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid28_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid28_valid              : 1;
        FIELD llid28_value              : 16;
#else
        FIELD llid28_value              : 16;
        FIELD llid28_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid28_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid28_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid28_dscvry_sts, *PREG_e_llid28_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid29_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid29_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid29_valid              : 1;
        FIELD llid29_value              : 16;
#else
        FIELD llid29_value              : 16;
        FIELD llid29_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid29_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid29_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid29_dscvry_sts, *PREG_e_llid29_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid30_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid30_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid30_valid              : 1;
        FIELD llid30_value              : 16;
#else
        FIELD llid30_value              : 16;
        FIELD llid30_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid30_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid30_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid30_dscvry_sts, *PREG_e_llid30_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid31_dscvry_sts         : 2;
        FIELD rsv_26                    : 4;
        FIELD llid31_rgstr_flg_sts      : 2;
        FIELD rsv_17                    : 7;
        FIELD llid31_valid              : 1;
        FIELD llid31_value              : 16;
#else
        FIELD llid31_value              : 16;
        FIELD llid31_valid              : 1;
        FIELD rsv_17                    : 7;
        FIELD llid31_rgstr_flg_sts      : 2;
        FIELD rsv_26                    : 4;
        FIELD llid31_dscvry_sts         : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid31_dscvry_sts, *PREG_e_llid31_dscvry_sts;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD llid15_rpt_cfg            : 2;
        FIELD llid14_rpt_cfg            : 2;
        FIELD llid13_rpt_cfg            : 2;
        FIELD llid12_rpt_cfg            : 2;
        FIELD llid11_rpt_cfg            : 2;
        FIELD llid10_rpt_cfg            : 2;
        FIELD llid9_rpt_cfg             : 2;
        FIELD llid8_rpt_cfg             : 2;
#else
        FIELD llid8_rpt_cfg             : 2;
        FIELD llid9_rpt_cfg             : 2;
        FIELD llid10_rpt_cfg            : 2;
        FIELD llid11_rpt_cfg            : 2;
        FIELD llid12_rpt_cfg            : 2;
        FIELD llid13_rpt_cfg            : 2;
        FIELD llid14_rpt_cfg            : 2;
        FIELD llid15_rpt_cfg            : 2;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rpt_cfg2, *PREG_e_rpt_cfg2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid31_rpt_cfg            : 2;
        FIELD llid30_rpt_cfg            : 2;
        FIELD llid29_rpt_cfg            : 2;
        FIELD llid28_rpt_cfg            : 2;
        FIELD llid27_rpt_cfg            : 2;
        FIELD llid26_rpt_cfg            : 2;
        FIELD llid25_rpt_cfg            : 2;
        FIELD llid24_rpt_cfg            : 2;
        FIELD llid23_rpt_cfg            : 2;
        FIELD llid22_rpt_cfg            : 2;
        FIELD llid21_rpt_cfg            : 2;
        FIELD llid20_rpt_cfg            : 2;
        FIELD llid19_rpt_cfg            : 2;
        FIELD llid18_rpt_cfg            : 2;
        FIELD llid17_rpt_cfg            : 2;
        FIELD llid16_rpt_cfg            : 2;
#else
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
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rpt_cfg3, *PREG_e_rpt_cfg3;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_2                     : 30;
        FIELD rpt_llid_idx_4_3          : 2;
#else
        FIELD rpt_llid_idx_4_3          : 2;
        FIELD rsv_2                     : 30;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rpt_qthld_cfg2, *PREG_e_rpt_qthld_cfg2;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rpt_bmap_rwcmd            : 1;
        FIELD rpt_bmap_rwcmd_done       : 1;
        FIELD rsv_5                     : 25;
        FIELD rpt_bmap_llid_idx         : 5;
#else
        FIELD rpt_bmap_llid_idx         : 5;
        FIELD rsv_5                     : 25;
        FIELD rpt_bmap_rwcmd_done       : 1;
        FIELD rpt_bmap_rwcmd            : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rpt_bitmap_cfg, *PREG_e_rpt_bitmap_cfg;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD rpt_qsizeadj_frc          : 8;
        FIELD rpt_bitmap_ctrl           : 8;
        FIELD rpt_bitmap_set            : 8;
#else
        FIELD rpt_bitmap_set            : 8;
        FIELD rpt_bitmap_ctrl           : 8;
        FIELD rpt_qsizeadj_frc          : 8;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_rpt_bitmap_val, *PREG_e_rpt_bitmap_val;

// ----------------- XEPON_MAC  Grouping Definitions -------------------
// ----------------- XEPON_MAC Register Definition -------------------
typedef volatile PACKING struct
{
    UINT32                          rsv_0000[6144];   // 0000..5FFC
    REG_e_glb_cfg                   e_glb_cfg;        // 6000
    REG_e_int_status                e_int_status;     // 6004
    REG_e_int_en                    e_int_en;         // 6008
    REG_e_rpt_mpcp_timeout_llid_idx e_rpt_mpcp_timeout_llid_idx; // 600C
    REG_e_dyinggsp_cfg              e_dyinggsp_cfg;   // 6010
    REG_e_pending_gnt_num           e_pending_gnt_num; // 6014
    UINT32                          rsv_6018[2];      // 6018..601C
    REG_e_llid0_3_cfg               e_llid0_3_cfg;    // 6020
    REG_e_llid4_7_cfg               e_llid4_7_cfg;    // 6024
    REG_e_llid_dscvry_ctrl          e_llid_dscvry_ctrl; // 6028
    union
	{
		struct{
			REG_e_llid0_dscvry_sts			e_llid0_dscvry_sts; // 602C
			REG_e_llid1_dscvry_sts			e_llid1_dscvry_sts; // 6030
			REG_e_llid2_dscvry_sts			e_llid2_dscvry_sts; // 6034
			REG_e_llid3_dscvry_sts			e_llid3_dscvry_sts; // 6038
			REG_e_llid4_dscvry_sts			e_llid4_dscvry_sts; // 603C
			REG_e_llid5_dscvry_sts			e_llid5_dscvry_sts; // 6040
			REG_e_llid6_dscvry_sts			e_llid6_dscvry_sts; // 6044
			REG_e_llid7_dscvry_sts			e_llid7_dscvry_sts; // 6048
		};
		REG_e_llid0_dscvry_sts			e_llid0_7_dscvry_sts[8];// 602C~6048
    };
    UINT32                          rsv_604C;         // 604C
    REG_e_mac_addr_cfg              e_mac_addr_cfg;   // 6050
    REG_e_mac_addr_value            e_mac_addr_value; // 6054
    REG_e_security_key_cfg          e_security_key_cfg; // 6058
    REG_e_key_value                 e_key_value;      // 605C
    UINT32                          rsv_6060[2];      // 6060..6064
    REG_e_rpt_cfg                   e_rpt_cfg;        // 6068
    REG_e_rpt_qthld_cfg             e_rpt_qthld_cfg;  // 606C
    UINT32                          rsv_6070[4];      // 6070..607C
    REG_e_local_time                e_local_time;     // 6080
    REG_e_tod_sync_x                e_tod_sync_x;     // 6084
    REG_e_tod_ltncy                 e_tod_ltncy;      // 6088
    UINT32                          rsv_608C[2];      // 608C..6090
    REG_e_new_tod_p2p_offset_sec_l32 e_new_tod_p2p_offset_sec_l32; // 6094
    REG_e_new_tod_p2p_tod_offset_nsec e_new_tod_p2p_tod_offset_nsec; // 6098
    REG_e_tod_p2p_tod_sec_l32       e_tod_p2p_tod_sec_l32; // 609C
    REG_e_tod_p2p_tod_nsec          e_tod_p2p_tod_nsec; // 60A0
    REG_e_tod_period                e_tod_period;     // 60A4
    UINT32                          rsv_60A8[2];      // 60A8..60AC
    REG_e_pwr_sv_cfg                e_pwr_sv_cfg;     // 60B0
    REG_e_slp_durt_max              e_slp_durt_max;   // 60B4
    REG_e_slp_duration              e_slp_duration;   // 60B8
    REG_e_act_duration              e_act_duration;   // 60BC
    REG_e_pwron_dly                 e_pwron_dly;      // 60C0
    REG_e_slp_duration_i            e_slp_duration_i; // 60C4
    UINT32                          rsv_60C8[2];      // 60C8..60CC
    REG_e_txfetch_cfg               e_txfetch_cfg;    // 60D0
    REG_e_sync_time                 e_sync_time;      // 60D4
    REG_e_tx_cal_cnst               e_tx_cal_cnst;    // 60D8
    REG_e_laser_onoff_time          e_laser_onoff_time; // 60DC
    REG_e_grd_thrshld               e_grd_thrshld;    // 60E0
    REG_e_mpcp_timeout_intvl        e_mpcp_timeout_intvl; // 60E4
    REG_e_rpt_timeout_intvl         e_rpt_timeout_intvl; // 60E8
    REG_e_max_future_gnt_time       e_max_future_gnt_time; // 60EC
    REG_e_min_proc_time             e_min_proc_time;  // 60F0
    REG_e_trx_adjust_time1          e_trx_adjust_time1; // 60F4
    REG_e_trx_adjust_time2          e_trx_adjust_time2; // 60F8
    UINT32                          rsv_60FC;         // 60FC
    REG_e_dbg_prb_sel               e_dbg_prb_sel;    // 6100
    REG_e_dbg_prb_h32               e_dbg_prb_h32;    // 6104
    REG_e_dbg_prb_l32               e_dbg_prb_l32;    // 6108
    REG_e_rxmbi_eth_cnt             e_rxmbi_eth_cnt;  // 610C
    REG_e_rxmpi_eth_cnt             e_rxmpi_eth_cnt;  // 6110
    REG_e_txmbi_eth_cnt             e_txmbi_eth_cnt;  // 6114
    REG_e_txmpi_eth_cnt             e_txmpi_eth_cnt;  // 6118
    REG_e_oam_stat                  e_oam_stat;       // 611C
    REG_e_mpcp_stat                 e_mpcp_stat;      // 6120
    REG_e_mpcp_rgst_stat            e_mpcp_rgst_stat; // 6124
    REG_e_gnt_pending_stat          e_gnt_pending_stat; // 6128
    REG_e_gnt_length_stat           e_gnt_length_stat; // 612C
    REG_e_gnt_type_stat             e_gnt_type_stat;  // 6130
    REG_e_time_drft_stat            e_time_drft_stat; // 6134
    REG_e_llid0_gnt_stat            e_llid0_gnt_stat; // 6138
    REG_e_llid1_gnt_stat            e_llid1_gnt_stat; // 613C
    REG_e_llid2_gnt_stat            e_llid2_gnt_stat; // 6140
    REG_e_llid3_gnt_stat            e_llid3_gnt_stat; // 6144
    REG_e_llid4_gnt_stat            e_llid4_gnt_stat; // 6148
    REG_e_llid5_gnt_stat            e_llid5_gnt_stat; // 614C
    REG_e_llid6_gnt_stat            e_llid6_gnt_stat; // 6150
    REG_e_llid7_gnt_stat            e_llid7_gnt_stat; // 6154
    UINT32                          rsv_6158[2];      // 6158..615C
    REG_e_snf_mpcp_oam_ctl          e_snf_mpcp_oam_ctl; // 6160
    UINT32                          rsv_6164[2];      // 6164..6168
    REG_e_dyinggsp_w1               e_dyinggsp_w1;    // 616C
    REG_e_dyinggsp_w2               e_dyinggsp_w2;    // 6170
    REG_e_dyinggsp_w3               e_dyinggsp_w3;    // 6174
    REG_e_dyinggsp_w4               e_dyinggsp_w4;    // 6178
    REG_e_dyinggsp_w5               e_dyinggsp_w5;    // 617C
    REG_e_dyinggsp_w6               e_dyinggsp_w6;    // 6180
    REG_e_dyinggsp_w7               e_dyinggsp_w7;    // 6184
    REG_e_dyinggsp_w8               e_dyinggsp_w8;    // 6188
    REG_e_dyinggsp_w9               e_dyinggsp_w9;    // 618C
    REG_e_dyinggsp_w10              e_dyinggsp_w10;   // 6190
    REG_e_dyinggsp_w11              e_dyinggsp_w11;   // 6194
    REG_e_dyinggsp_w12              e_dyinggsp_w12;   // 6198
    REG_e_oam_kpalv_w1              e_oam_kpalv_w1;   // 619C
    REG_e_oam_kpalv_w2              e_oam_kpalv_w2;   // 61A0
    REG_e_oam_kpalv_w3              e_oam_kpalv_w3;   // 61A4
    REG_e_oam_kpalv_w4              e_oam_kpalv_w4;   // 61A8
    REG_e_oam_kpalv_w5              e_oam_kpalv_w5;   // 61AC
    REG_e_oam_kpalv_w6              e_oam_kpalv_w6;   // 61B0
    REG_e_oam_kpalv_w7              e_oam_kpalv_w7;   // 61B4
    REG_e_oam_kpalv_w8              e_oam_kpalv_w8;   // 61B8
    REG_e_oam_kpalv_w9              e_oam_kpalv_w9;   // 61BC
    REG_e_oam_kpalv_w10             e_oam_kpalv_w10;  // 61C0
    REG_e_oam_kpalv_w11             e_oam_kpalv_w11;  // 61C4
    REG_e_oam_kpalv_w12             e_oam_kpalv_w12;  // 61C8
    REG_e_oam_kpalv_ctrl            e_oam_kpalv_ctrl; // 61CC
    REG_e_tod_1pps_ctrl             e_tod_1pps_ctrl;  // 61D0
    UINT32                          rsv_61D4;         // 61D4
    REG_e_sniff_sp_tag              e_sniff_sp_tag;   // 61D8
    UINT32                          rsv_61DC[9];      // 61DC..61FC
    REG_e_glb_cfg2                  e_glb_cfg2;       // 6200
    REG_e_int_sts2                  e_int_sts2;       // 6204
    REG_e_int_en2                   e_int_en2;        // 6208
    REG_e_exc_sts                   e_exc_sts;        // 620C
    REG_e_olt_dscvinfo              e_olt_dscvinfo;   // 6210
    REG_e_laser_onoff_time2         e_laser_onoff_time2; // 6214
    REG_e_trx_adjtime3              e_trx_adjtime3;   // 6218
    REG_e_trx_adjtime4              e_trx_adjtime4;   // 621C
    REG_e_trx_adjtime5              e_trx_adjtime5;   // 6220
    UINT32                          rsv_6224;         // 6224
    REG_e_rxphydly_adjtime          e_rxphydly_adjtime; // 6228
    REG_e_overhead_time_thr         e_overhead_time_thr; // 622C
    REG_e_txcal_cnst2               e_txcal_cnst2;    // 6230
    REG_e_txcal_cnst3               e_txcal_cnst3;    // 6234
    REG_e_txsch_cfg                 e_txsch_cfg;      // 6238
    REG_e_rxfifo_thr                e_rxfifo_thr;     // 623C
    REG_e_bcllid_cfg                e_bcllid_cfg;     // 6240
    REG_e_txfrm_cfg1                e_txfrm_cfg1;     // 6244
    REG_e_txfrm_cfg2                e_txfrm_cfg2;     // 6248
    REG_e_txfrm_cfg3                e_txfrm_cfg3;     // 624C
    union
	{
		struct{
			REG_e_u1g_rpt_qsizeadj1 		e_u1g_rpt_qsizeadj1; // 6250
			REG_e_u1g_rpt_qsizeadj2 		e_u1g_rpt_qsizeadj2; // 6254
			REG_e_u1g_rpt_qsizeadj3 		e_u1g_rpt_qsizeadj3; // 6258
			REG_e_u1g_rpt_qsizeadj4 		e_u1g_rpt_qsizeadj4; // 625C
			REG_e_u1g_rpt_qsizeadj5 		e_u1g_rpt_qsizeadj5; // 6260
			REG_e_u1g_rpt_qsizeadj6 		e_u1g_rpt_qsizeadj6; // 6264
			REG_e_u1g_rpt_qsizeadj7 		e_u1g_rpt_qsizeadj7; // 6268
			REG_e_u1g_rpt_qsizeadj8 		e_u1g_rpt_qsizeadj8; // 626C
		};
		REG_e_u1g_rpt_qsizeadj1			e_u1g_rpt_qsizeadj[8];// 6250~626C
    };
    union
	{
		struct{
			REG_e_u10g_rpt_qsizeadj1		e_u10g_rpt_qsizeadj1; // 6270
			REG_e_u10g_rpt_qsizeadj2		e_u10g_rpt_qsizeadj2; // 6274
			REG_e_u10g_rpt_qsizeadj3		e_u10g_rpt_qsizeadj3; // 6278
			REG_e_u10g_rpt_qsizeadj4		e_u10g_rpt_qsizeadj4; // 627C
		};
		REG_e_u10g_rpt_qsizeadj1			e_u10g_rpt_qsizeadj[4];// 6270~627C
    };

    UINT32                          rsv_6280[4];      // 6280..628C
    REG_e_snf_dah                   e_snf_dah;        // 6290
    REG_e_snf_dal                   e_snf_dal;        // 6294
    REG_e_snf_sal                   e_snf_sal;        // 6298
    REG_e_snf_etype                 e_snf_etype;      // 629C
    REG_e_rxuni_mcllid_cfg0         e_rxuni_mcllid_cfg0; // 62A0
    REG_e_rxuni_mcllid_cfg1         e_rxuni_mcllid_cfg1; // 62A4
    REG_e_rxuni_mcllid_cfg2         e_rxuni_mcllid_cfg2; // 62A8
    REG_e_rxuni_mcllid_cfg3         e_rxuni_mcllid_cfg3; // 62AC
    REG_e_rxuni_mcllid_cfg4         e_rxuni_mcllid_cfg4; // 62B0
    REG_e_rxuni_mcllid_cfg5         e_rxuni_mcllid_cfg5; // 62B4
    REG_e_rxuni_mcllid_cfg6         e_rxuni_mcllid_cfg6; // 62B8
    REG_e_rxuni_mcllid_cfg7         e_rxuni_mcllid_cfg7; // 62BC
    UINT32                          rsv_62C0[4];      // 62C0..62CC
    REG_e_dfrpt_data5               e_dfrpt_data5;    // 62D0
    REG_e_dfrpt_data6               e_dfrpt_data6;    // 62D4
    REG_e_dfrpt_data7               e_dfrpt_data7;    // 62D8
    REG_e_dfrpt_data8               e_dfrpt_data8;    // 62DC
    REG_e_dfrpt_data9               e_dfrpt_data9;    // 62E0
    REG_e_dfrpt_data10              e_dfrpt_data10;   // 62E4
    REG_e_dfrpt_data11              e_dfrpt_data11;   // 62E8
    REG_e_dfrpt_data12              e_dfrpt_data12;   // 62EC
    REG_e_dfrpt_data13              e_dfrpt_data13;   // 62F0
    REG_e_dfrpt_data14              e_dfrpt_data14;   // 62F4
    REG_e_glue_cfg                  e_glue_cfg;       // 62F8
    REG_e_gntreq_tmout              e_gntreq_tmout;   // 62FC
    REG_e_glb_sts                   e_glb_sts;        // 6300
    UINT32                          rsv_6304[11];     // 6304..632C
    REG_e_cnt_clr                   e_cnt_clr;        // 6330
    REG_e_rxadv_cnt                 e_rxadv_cnt;      // 6334
    REG_e_tx_dygasp_cnt             e_tx_dygasp_cnt;  // 6338
    REG_e_tx_rpt_cnt                e_tx_rpt_cnt;     // 633C
    REG_e_txmpi_mpcp_cnt            e_txmpi_mpcp_cnt; // 6340
    REG_e_rxmpi_uc_cnt              e_rxmpi_uc_cnt;   // 6344
    REG_e_rxmpi_bc_cnt              e_rxmpi_bc_cnt;   // 6348
    REG_e_rxmpi_mc_cnt              e_rxmpi_mc_cnt;   // 634C
    REG_e_rxmpi_oam_cnt             e_rxmpi_oam_cnt;  // 6350
    REG_e_rxmpi_mpcp_cnt            e_rxmpi_mpcp_cnt; // 6354
    REG_e_rxmpi_gate_cnt            e_rxmpi_gate_cnt; // 6358
    REG_e_rxmpi_nrlgate_cnt         e_rxmpi_nrlgate_cnt; // 635C
    REG_e_rxmpi_crc8err_cnt         e_rxmpi_crc8err_cnt; // 6360
    REG_e_rxmpi_drop_cnt            e_rxmpi_drop_cnt; // 6364
    REG_e_rxmpi_churn_cnt           e_rxmpi_churn_cnt; // 6368
    REG_e_rxmbi_snf_cnt             e_rxmbi_snf_cnt;  // 636C
    REG_e_rxmbi_drop_cnt            e_rxmbi_drop_cnt; // 6370
    REG_e_rxmbi_drop_cnt2           e_rxmbi_drop_cnt2; // 6374
    REG_e_rxmbi_bytecnt_h           e_rxmbi_bytecnt_h; // 6378
    REG_e_rxmbi_bytecnt_l           e_rxmbi_bytecnt_l; // 637C
    REG_e_txmbi_uceth_cnt           e_txmbi_uceth_cnt; // 6380
    REG_e_txmbi_mceth_cnt           e_txmbi_mceth_cnt; // 6384
    REG_e_txmbi_err_cnt             e_txmbi_err_cnt;  // 6388
    REG_e_tx_timedrift_stat         e_tx_timedrift_stat; // 638C
    REG_e_rxmpi_churn_cnt2          e_rxmpi_churn_cnt2; // 6390
    REG_e_rxmpi_churn_cnt3          e_rxmpi_churn_cnt3; // 6394
    REG_e_txmpi_uceth_cnt           e_txmpi_uceth_cnt; // 6398
    REG_e_txmpi_mceth_cnt           e_txmpi_mceth_cnt; // 639C
    REG_e_llid8_9_gnt_stat          e_llid8_9_gnt_stat; // 63A0
    REG_e_llid10_11_gnt_stat        e_llid10_11_gnt_stat; // 63A4
    REG_e_llid12_13_gnt_stat        e_llid12_13_gnt_stat; // 63A8
    REG_e_llid14_15_gnt_stat        e_llid14_15_gnt_stat; // 63AC
    REG_e_llid16_17_gnt_stat        e_llid16_17_gnt_stat; // 63B0
    REG_e_llid18_19_gnt_stat        e_llid18_19_gnt_stat; // 63B4
    REG_e_llid20_21_gnt_stat        e_llid20_21_gnt_stat; // 63B8
    REG_e_llid22_23_gnt_stat        e_llid22_23_gnt_stat; // 63BC
    REG_e_llid24_25_gnt_stat        e_llid24_25_gnt_stat; // 63C0
    REG_e_llid26_27_gnt_stat        e_llid26_27_gnt_stat; // 63C4
    REG_e_llid28_29_gnt_stat        e_llid28_29_gnt_stat; // 63C8
    REG_e_llid30_31_gnt_stat        e_llid30_31_gnt_stat; // 63CC
    REG_e_rxfifo_depth_stat         e_rxfifo_depth_stat; // 63D0
    UINT32                          rsv_63D4[2];      // 63D4..63D8
    REG_e_rdmdly_stat               e_rdmdly_stat;    // 63DC
    UINT32                          rsv_63E0[9];      // 63E0..6400
    REG_e_int_sts3                  e_int_sts3;       // 6404
    REG_e_int_en3                   e_int_en3;        // 6408
    REG_e_rpt_mpcp_timout2          e_rpt_mpcp_timout2; // 640C
    REG_e_llid8_11_cfg              e_llid8_11_cfg;   // 6410
    REG_e_llid12_15_cfg             e_llid12_15_cfg;  // 6414
    REG_e_llid16_19_cfg             e_llid16_19_cfg;  // 6418
    REG_e_llid20_23_cfg             e_llid20_23_cfg;  // 641C
    REG_e_llid24_27_cfg             e_llid24_27_cfg;  // 6420
    REG_e_llid28_31_cfg             e_llid28_31_cfg;  // 6424
    union
	{
		struct{
			REG_e_llid8_dscvry_sts			e_llid8_dscvry_sts; // 6428
			REG_e_llid9_dscvry_sts			e_llid9_dscvry_sts; // 642C
			REG_e_llid10_dscvry_sts 		e_llid10_dscvry_sts; // 6430
			REG_e_llid11_dscvry_sts 		e_llid11_dscvry_sts; // 6434
			REG_e_llid12_dscvry_sts 		e_llid12_dscvry_sts; // 6438
			REG_e_llid13_dscvry_sts 		e_llid13_dscvry_sts; // 643C
			REG_e_llid14_dscvry_sts 		e_llid14_dscvry_sts; // 6440
			REG_e_llid15_dscvry_sts 		e_llid15_dscvry_sts; // 6444
			REG_e_llid16_dscvry_sts 		e_llid16_dscvry_sts; // 6448
			REG_e_llid17_dscvry_sts 		e_llid17_dscvry_sts; // 644C
			REG_e_llid18_dscvry_sts 		e_llid18_dscvry_sts; // 6450
			REG_e_llid19_dscvry_sts 		e_llid19_dscvry_sts; // 6454
			REG_e_llid20_dscvry_sts 		e_llid20_dscvry_sts; // 6458
			REG_e_llid21_dscvry_sts 		e_llid21_dscvry_sts; // 645C
			REG_e_llid22_dscvry_sts 		e_llid22_dscvry_sts; // 6460
			REG_e_llid23_dscvry_sts 		e_llid23_dscvry_sts; // 6464
			REG_e_llid24_dscvry_sts 		e_llid24_dscvry_sts; // 6468
			REG_e_llid25_dscvry_sts 		e_llid25_dscvry_sts; // 646C
			REG_e_llid26_dscvry_sts 		e_llid26_dscvry_sts; // 6470
			REG_e_llid27_dscvry_sts 		e_llid27_dscvry_sts; // 6474
			REG_e_llid28_dscvry_sts 		e_llid28_dscvry_sts; // 6478
			REG_e_llid29_dscvry_sts 		e_llid29_dscvry_sts; // 647C
			REG_e_llid30_dscvry_sts 		e_llid30_dscvry_sts; // 6480
			REG_e_llid31_dscvry_sts 		e_llid31_dscvry_sts; // 6484
		};
		REG_e_llid8_dscvry_sts			e_llid8_31_dscvry_sts[24];// 6428~6484
    };

    REG_e_rpt_cfg2                  e_rpt_cfg2;       // 6488
    REG_e_rpt_cfg3                  e_rpt_cfg3;       // 648C
    REG_e_rpt_qthld_cfg2            e_rpt_qthld_cfg2; // 6490
    REG_e_rpt_bitmap_cfg            e_rpt_bitmap_cfg; // 6494
    REG_e_rpt_bitmap_val            e_rpt_bitmap_val; // 6498
    UINT32                          rsv_649C[7896];   // 649C..DFF8
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
#define e_int_status                                           INREG32(&XEPON_MAC_BASE->e_int_status) // 6004
#define e_int_en                                               INREG32(&XEPON_MAC_BASE->e_int_en) // 6008
#define e_rpt_mpcp_timeout_llid_idx                            INREG32(&XEPON_MAC_BASE->e_rpt_mpcp_timeout_llid_idx) // 600C
#define e_dyinggsp_cfg                                         INREG32(&XEPON_MAC_BASE->e_dyinggsp_cfg) // 6010
#define e_pending_gnt_num                                      INREG32(&XEPON_MAC_BASE->e_pending_gnt_num) // 6014
#define e_llid0_3_cfg                                          INREG32(&XEPON_MAC_BASE->e_llid0_3_cfg) // 6020
#define e_llid4_7_cfg                                          INREG32(&XEPON_MAC_BASE->e_llid4_7_cfg) // 6024
#define e_llid_dscvry_ctrl                                     INREG32(&XEPON_MAC_BASE->e_llid_dscvry_ctrl) // 6028
#define e_llid0_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid0_dscvry_sts) // 602C
#define e_llid1_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid1_dscvry_sts) // 6030
#define e_llid2_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid2_dscvry_sts) // 6034
#define e_llid3_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid3_dscvry_sts) // 6038
#define e_llid4_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid4_dscvry_sts) // 603C
#define e_llid5_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid5_dscvry_sts) // 6040
#define e_llid6_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid6_dscvry_sts) // 6044
#define e_llid7_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid7_dscvry_sts) // 6048
#define e_llid0_7_dscvry_sts(i) \
({ \
	int idx = (i);\
	INREG32(&(XEPON_MAC_BASE->e_llid0_7_dscvry_sts[idx])); \
})

#define e_mac_addr_cfg                                         INREG32(&XEPON_MAC_BASE->e_mac_addr_cfg) // 6050
#define e_mac_addr_value                                       INREG32(&XEPON_MAC_BASE->e_mac_addr_value) // 6054
#define e_security_key_cfg                                     INREG32(&XEPON_MAC_BASE->e_security_key_cfg) // 6058
#define e_key_value                                            INREG32(&XEPON_MAC_BASE->e_key_value) // 605C
#define e_rpt_cfg                                              INREG32(&XEPON_MAC_BASE->e_rpt_cfg) // 6068
#define e_rpt_qthld_cfg                                        INREG32(&XEPON_MAC_BASE->e_rpt_qthld_cfg) // 606C
#define e_local_time                                           INREG32(&XEPON_MAC_BASE->e_local_time) // 6080
#define e_tod_sync_x                                           INREG32(&XEPON_MAC_BASE->e_tod_sync_x) // 6084
#define e_tod_ltncy                                            INREG32(&XEPON_MAC_BASE->e_tod_ltncy) // 6088
#define e_new_tod_p2p_offset_sec_l32                           INREG32(&XEPON_MAC_BASE->e_new_tod_p2p_offset_sec_l32) // 6094
#define e_new_tod_p2p_tod_offset_nsec                          INREG32(&XEPON_MAC_BASE->e_new_tod_p2p_tod_offset_nsec) // 6098
#define e_tod_p2p_tod_sec_l32                                  INREG32(&XEPON_MAC_BASE->e_tod_p2p_tod_sec_l32) // 609C
#define e_tod_p2p_tod_nsec                                     INREG32(&XEPON_MAC_BASE->e_tod_p2p_tod_nsec) // 60A0
#define e_tod_period                                           INREG32(&XEPON_MAC_BASE->e_tod_period) // 60A4
#define e_pwr_sv_cfg                                           INREG32(&XEPON_MAC_BASE->e_pwr_sv_cfg) // 60B0
#define e_slp_durt_max                                         INREG32(&XEPON_MAC_BASE->e_slp_durt_max) // 60B4
#define e_slp_duration                                         INREG32(&XEPON_MAC_BASE->e_slp_duration) // 60B8
#define e_act_duration                                         INREG32(&XEPON_MAC_BASE->e_act_duration) // 60BC
#define e_pwron_dly                                            INREG32(&XEPON_MAC_BASE->e_pwron_dly) // 60C0
#define e_slp_duration_i                                       INREG32(&XEPON_MAC_BASE->e_slp_duration_i) // 60C4
#define e_txfetch_cfg                                          INREG32(&XEPON_MAC_BASE->e_txfetch_cfg) // 60D0
#define e_sync_time                                            INREG32(&XEPON_MAC_BASE->e_sync_time) // 60D4
#define e_tx_cal_cnst                                          INREG32(&XEPON_MAC_BASE->e_tx_cal_cnst) // 60D8
#define e_laser_onoff_time                                     INREG32(&XEPON_MAC_BASE->e_laser_onoff_time) // 60DC
#define e_grd_thrshld                                          INREG32(&XEPON_MAC_BASE->e_grd_thrshld) // 60E0
#define e_mpcp_timeout_intvl                                   INREG32(&XEPON_MAC_BASE->e_mpcp_timeout_intvl) // 60E4
#define e_rpt_timeout_intvl                                    INREG32(&XEPON_MAC_BASE->e_rpt_timeout_intvl) // 60E8
#define e_max_future_gnt_time                                  INREG32(&XEPON_MAC_BASE->e_max_future_gnt_time) // 60EC
#define e_min_proc_time                                        INREG32(&XEPON_MAC_BASE->e_min_proc_time) // 60F0
#define e_trx_adjust_time1                                     INREG32(&XEPON_MAC_BASE->e_trx_adjust_time1) // 60F4
#define e_trx_adjust_time2                                     INREG32(&XEPON_MAC_BASE->e_trx_adjust_time2) // 60F8
#define e_dbg_prb_sel                                          INREG32(&XEPON_MAC_BASE->e_dbg_prb_sel) // 6100
#define e_dbg_prb_h32                                          INREG32(&XEPON_MAC_BASE->e_dbg_prb_h32) // 6104
#define e_dbg_prb_l32                                          INREG32(&XEPON_MAC_BASE->e_dbg_prb_l32) // 6108
#define e_rxmbi_eth_cnt                                        INREG32(&XEPON_MAC_BASE->e_rxmbi_eth_cnt) // 610C
#define e_rxmpi_eth_cnt                                        INREG32(&XEPON_MAC_BASE->e_rxmpi_eth_cnt) // 6110
#define e_txmbi_eth_cnt                                        INREG32(&XEPON_MAC_BASE->e_txmbi_eth_cnt) // 6114
#define e_txmpi_eth_cnt                                        INREG32(&XEPON_MAC_BASE->e_txmpi_eth_cnt) // 6118
#define e_oam_stat                                             INREG32(&XEPON_MAC_BASE->e_oam_stat) // 611C
#define e_mpcp_stat                                            INREG32(&XEPON_MAC_BASE->e_mpcp_stat) // 6120
#define e_mpcp_rgst_stat                                       INREG32(&XEPON_MAC_BASE->e_mpcp_rgst_stat) // 6124
#define e_gnt_pending_stat                                     INREG32(&XEPON_MAC_BASE->e_gnt_pending_stat) // 6128
#define e_gnt_length_stat                                      INREG32(&XEPON_MAC_BASE->e_gnt_length_stat) // 612C
#define e_gnt_type_stat                                        INREG32(&XEPON_MAC_BASE->e_gnt_type_stat) // 6130
#define e_time_drft_stat                                       INREG32(&XEPON_MAC_BASE->e_time_drft_stat) // 6134
#define e_llid0_gnt_stat                                       INREG32(&XEPON_MAC_BASE->e_llid0_gnt_stat) // 6138
#define e_llid1_gnt_stat                                       INREG32(&XEPON_MAC_BASE->e_llid1_gnt_stat) // 613C
#define e_llid2_gnt_stat                                       INREG32(&XEPON_MAC_BASE->e_llid2_gnt_stat) // 6140
#define e_llid3_gnt_stat                                       INREG32(&XEPON_MAC_BASE->e_llid3_gnt_stat) // 6144
#define e_llid4_gnt_stat                                       INREG32(&XEPON_MAC_BASE->e_llid4_gnt_stat) // 6148
#define e_llid5_gnt_stat                                       INREG32(&XEPON_MAC_BASE->e_llid5_gnt_stat) // 614C
#define e_llid6_gnt_stat                                       INREG32(&XEPON_MAC_BASE->e_llid6_gnt_stat) // 6150
#define e_llid7_gnt_stat                                       INREG32(&XEPON_MAC_BASE->e_llid7_gnt_stat) // 6154
#define e_snf_mpcp_oam_ctl                                     INREG32(&XEPON_MAC_BASE->e_snf_mpcp_oam_ctl) // 6160
#define e_dyinggsp_w1                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w1) // 616C
#define e_dyinggsp_w2                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w2) // 6170
#define e_dyinggsp_w3                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w3) // 6174
#define e_dyinggsp_w4                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w4) // 6178
#define e_dyinggsp_w5                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w5) // 617C
#define e_dyinggsp_w6                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w6) // 6180
#define e_dyinggsp_w7                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w7) // 6184
#define e_dyinggsp_w8                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w8) // 6188
#define e_dyinggsp_w9                                          INREG32(&XEPON_MAC_BASE->e_dyinggsp_w9) // 618C
#define e_dyinggsp_w10                                         INREG32(&XEPON_MAC_BASE->e_dyinggsp_w10) // 6190
#define e_dyinggsp_w11                                         INREG32(&XEPON_MAC_BASE->e_dyinggsp_w11) // 6194
#define e_dyinggsp_w12                                         INREG32(&XEPON_MAC_BASE->e_dyinggsp_w12) // 6198
#define e_oam_kpalv_w1                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w1) // 619C
#define e_oam_kpalv_w2                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w2) // 61A0
#define e_oam_kpalv_w3                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w3) // 61A4
#define e_oam_kpalv_w4                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w4) // 61A8
#define e_oam_kpalv_w5                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w5) // 61AC
#define e_oam_kpalv_w6                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w6) // 61B0
#define e_oam_kpalv_w7                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w7) // 61B4
#define e_oam_kpalv_w8                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w8) // 61B8
#define e_oam_kpalv_w9                                         INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w9) // 61BC
#define e_oam_kpalv_w10                                        INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w10) // 61C0
#define e_oam_kpalv_w11                                        INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w11) // 61C4
#define e_oam_kpalv_w12                                        INREG32(&XEPON_MAC_BASE->e_oam_kpalv_w12) // 61C8
#define e_oam_kpalv_ctrl                                       INREG32(&XEPON_MAC_BASE->e_oam_kpalv_ctrl) // 61CC
#define e_tod_1pps_ctrl                                        INREG32(&XEPON_MAC_BASE->e_tod_1pps_ctrl) // 61D0
#define e_sniff_sp_tag                                         INREG32(&XEPON_MAC_BASE->e_sniff_sp_tag) // 61D8
#define e_glb_cfg2                                             INREG32(&XEPON_MAC_BASE->e_glb_cfg2) // 6200
#define e_int_sts2                                             INREG32(&XEPON_MAC_BASE->e_int_sts2) // 6204
#define e_int_en2                                              INREG32(&XEPON_MAC_BASE->e_int_en2) // 6208
#define e_exc_sts                                              INREG32(&XEPON_MAC_BASE->e_exc_sts) // 620C
#define e_olt_dscvinfo                                         INREG32(&XEPON_MAC_BASE->e_olt_dscvinfo) // 6210
#define e_laser_onoff_time2                                    INREG32(&XEPON_MAC_BASE->e_laser_onoff_time2) // 6214
#define e_trx_adjtime3                                         INREG32(&XEPON_MAC_BASE->e_trx_adjtime3) // 6218
#define e_trx_adjtime4                                         INREG32(&XEPON_MAC_BASE->e_trx_adjtime4) // 621C
#define e_trx_adjtime5                                         INREG32(&XEPON_MAC_BASE->e_trx_adjtime5) // 6220
#define e_rxphydly_adjtime                                     INREG32(&XEPON_MAC_BASE->e_rxphydly_adjtime) // 6228
#define e_overhead_time_thr                                    INREG32(&XEPON_MAC_BASE->e_overhead_time_thr) // 622C
#define e_txcal_cnst2                                          INREG32(&XEPON_MAC_BASE->e_txcal_cnst2) // 6230
#define e_txcal_cnst3                                          INREG32(&XEPON_MAC_BASE->e_txcal_cnst3) // 6234
#define e_txsch_cfg                                            INREG32(&XEPON_MAC_BASE->e_txsch_cfg) // 6238
#define e_rxfifo_thr                                           INREG32(&XEPON_MAC_BASE->e_rxfifo_thr) // 623C
#define e_bcllid_cfg                                           INREG32(&XEPON_MAC_BASE->e_bcllid_cfg) // 6240
#define e_txfrm_cfg1                                           INREG32(&XEPON_MAC_BASE->e_txfrm_cfg1) // 6244
#define e_txfrm_cfg2                                           INREG32(&XEPON_MAC_BASE->e_txfrm_cfg2) // 6248
#define e_txfrm_cfg3                                           INREG32(&XEPON_MAC_BASE->e_txfrm_cfg3) // 624C
#define e_u1g_rpt_qsizeadj1                                    INREG32(&XEPON_MAC_BASE->e_u1g_rpt_qsizeadj1) // 6250
#define e_u1g_rpt_qsizeadj2                                    INREG32(&XEPON_MAC_BASE->e_u1g_rpt_qsizeadj2) // 6254
#define e_u1g_rpt_qsizeadj3                                    INREG32(&XEPON_MAC_BASE->e_u1g_rpt_qsizeadj3) // 6258
#define e_u1g_rpt_qsizeadj4                                    INREG32(&XEPON_MAC_BASE->e_u1g_rpt_qsizeadj4) // 625C
#define e_u1g_rpt_qsizeadj5                                    INREG32(&XEPON_MAC_BASE->e_u1g_rpt_qsizeadj5) // 6260
#define e_u1g_rpt_qsizeadj6                                    INREG32(&XEPON_MAC_BASE->e_u1g_rpt_qsizeadj6) // 6264
#define e_u1g_rpt_qsizeadj7                                    INREG32(&XEPON_MAC_BASE->e_u1g_rpt_qsizeadj7) // 6268
#define e_u1g_rpt_qsizeadj8                                    INREG32(&XEPON_MAC_BASE->e_u1g_rpt_qsizeadj8) // 626C
#define e_u1g_rpt_qsizeadj(i) \
({ \
	int idx = (i);\
	INREG32(&(XEPON_MAC_BASE->e_u1g_rpt_qsizeadj[idx])); \
})

#define e_u10g_rpt_qsizeadj1                                   INREG32(&XEPON_MAC_BASE->e_u10g_rpt_qsizeadj1) // 6270
#define e_u10g_rpt_qsizeadj2                                   INREG32(&XEPON_MAC_BASE->e_u10g_rpt_qsizeadj2) // 6274
#define e_u10g_rpt_qsizeadj3                                   INREG32(&XEPON_MAC_BASE->e_u10g_rpt_qsizeadj3) // 6278
#define e_u10g_rpt_qsizeadj4                                   INREG32(&XEPON_MAC_BASE->e_u10g_rpt_qsizeadj4) // 627C
#define e_u10g_rpt_qsizeadj(i) \
({ \
	int idx = (i);\
	INREG32(&(XEPON_MAC_BASE->e_u10g_rpt_qsizeadj[idx])); \
})

#define e_snf_dah                                              INREG32(&XEPON_MAC_BASE->e_snf_dah) // 6290
#define e_snf_dal                                              INREG32(&XEPON_MAC_BASE->e_snf_dal) // 6294
#define e_snf_sal                                              INREG32(&XEPON_MAC_BASE->e_snf_sal) // 6298
#define e_snf_etype                                            INREG32(&XEPON_MAC_BASE->e_snf_etype) // 629C
#define e_rxuni_mcllid_cfg0                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg0) // 62A0
#define e_rxuni_mcllid_cfg1                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg1) // 62A4
#define e_rxuni_mcllid_cfg2                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg2) // 62A8
#define e_rxuni_mcllid_cfg3                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg3) // 62AC
#define e_rxuni_mcllid_cfg4                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg4) // 62B0
#define e_rxuni_mcllid_cfg5                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg5) // 62B4
#define e_rxuni_mcllid_cfg6                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg6) // 62B8
#define e_rxuni_mcllid_cfg7                                    INREG32(&XEPON_MAC_BASE->e_rxuni_mcllid_cfg7) // 62BC
#define e_dfrpt_data5                                          INREG32(&XEPON_MAC_BASE->e_dfrpt_data5) // 62D0
#define e_dfrpt_data6                                          INREG32(&XEPON_MAC_BASE->e_dfrpt_data6) // 62D4
#define e_dfrpt_data7                                          INREG32(&XEPON_MAC_BASE->e_dfrpt_data7) // 62D8
#define e_dfrpt_data8                                          INREG32(&XEPON_MAC_BASE->e_dfrpt_data8) // 62DC
#define e_dfrpt_data9                                          INREG32(&XEPON_MAC_BASE->e_dfrpt_data9) // 62E0
#define e_dfrpt_data10                                         INREG32(&XEPON_MAC_BASE->e_dfrpt_data10) // 62E4
#define e_dfrpt_data11                                         INREG32(&XEPON_MAC_BASE->e_dfrpt_data11) // 62E8
#define e_dfrpt_data12                                         INREG32(&XEPON_MAC_BASE->e_dfrpt_data12) // 62EC
#define e_dfrpt_data13                                         INREG32(&XEPON_MAC_BASE->e_dfrpt_data13) // 62F0
#define e_dfrpt_data14                                         INREG32(&XEPON_MAC_BASE->e_dfrpt_data14) // 62F4
#define e_glue_cfg                                             INREG32(&XEPON_MAC_BASE->e_glue_cfg) // 62F8
#define e_gntreq_tmout                                         INREG32(&XEPON_MAC_BASE->e_gntreq_tmout) // 62FC
#define e_glb_sts                                              INREG32(&XEPON_MAC_BASE->e_glb_sts) // 6300
#define e_cnt_clr                                              INREG32(&XEPON_MAC_BASE->e_cnt_clr) // 6330
#define e_rxadv_cnt                                            INREG32(&XEPON_MAC_BASE->e_rxadv_cnt) // 6334
#define e_tx_dygasp_cnt                                        INREG32(&XEPON_MAC_BASE->e_tx_dygasp_cnt) // 6338
#define e_tx_rpt_cnt                                           INREG32(&XEPON_MAC_BASE->e_tx_rpt_cnt) // 633C
#define e_txmpi_mpcp_cnt                                       INREG32(&XEPON_MAC_BASE->e_txmpi_mpcp_cnt) // 6340
#define e_rxmpi_uc_cnt                                         INREG32(&XEPON_MAC_BASE->e_rxmpi_uc_cnt) // 6344
#define e_rxmpi_bc_cnt                                         INREG32(&XEPON_MAC_BASE->e_rxmpi_bc_cnt) // 6348
#define e_rxmpi_mc_cnt                                         INREG32(&XEPON_MAC_BASE->e_rxmpi_mc_cnt) // 634C
#define e_rxmpi_oam_cnt                                        INREG32(&XEPON_MAC_BASE->e_rxmpi_oam_cnt) // 6350
#define e_rxmpi_mpcp_cnt                                       INREG32(&XEPON_MAC_BASE->e_rxmpi_mpcp_cnt) // 6354
#define e_rxmpi_gate_cnt                                       INREG32(&XEPON_MAC_BASE->e_rxmpi_gate_cnt) // 6358
#define e_rxmpi_nrlgate_cnt                                    INREG32(&XEPON_MAC_BASE->e_rxmpi_nrlgate_cnt) // 635C
#define e_rxmpi_crc8err_cnt                                    INREG32(&XEPON_MAC_BASE->e_rxmpi_crc8err_cnt) // 6360
#define e_rxmpi_drop_cnt                                       INREG32(&XEPON_MAC_BASE->e_rxmpi_drop_cnt) // 6364
#define e_rxmpi_churn_cnt                                      INREG32(&XEPON_MAC_BASE->e_rxmpi_churn_cnt) // 6368
#define e_rxmbi_snf_cnt                                        INREG32(&XEPON_MAC_BASE->e_rxmbi_snf_cnt) // 636C
#define e_rxmbi_drop_cnt                                       INREG32(&XEPON_MAC_BASE->e_rxmbi_drop_cnt) // 6370
#define e_rxmbi_drop_cnt2                                      INREG32(&XEPON_MAC_BASE->e_rxmbi_drop_cnt2) // 6374
#define e_rxmbi_bytecnt_h                                      INREG32(&XEPON_MAC_BASE->e_rxmbi_bytecnt_h) // 6378
#define e_rxmbi_bytecnt_l                                      INREG32(&XEPON_MAC_BASE->e_rxmbi_bytecnt_l) // 637C
#define e_txmbi_uceth_cnt                                      INREG32(&XEPON_MAC_BASE->e_txmbi_uceth_cnt) // 6380
#define e_txmbi_mceth_cnt                                      INREG32(&XEPON_MAC_BASE->e_txmbi_mceth_cnt) // 6384
#define e_txmbi_err_cnt                                        INREG32(&XEPON_MAC_BASE->e_txmbi_err_cnt) // 6388
#define e_tx_timedrift_stat                                    INREG32(&XEPON_MAC_BASE->e_tx_timedrift_stat) // 638C
#define e_rxmpi_churn_cnt2                                     INREG32(&XEPON_MAC_BASE->e_rxmpi_churn_cnt2) // 6390
#define e_rxmpi_churn_cnt3                                     INREG32(&XEPON_MAC_BASE->e_rxmpi_churn_cnt3) // 6394
#define e_txmpi_uceth_cnt                                      INREG32(&XEPON_MAC_BASE->e_txmpi_uceth_cnt) // 6398
#define e_txmpi_mceth_cnt                                      INREG32(&XEPON_MAC_BASE->e_txmpi_mceth_cnt) // 639C
#define e_llid8_9_gnt_stat                                     INREG32(&XEPON_MAC_BASE->e_llid8_9_gnt_stat) // 63A0
#define e_llid10_11_gnt_stat                                   INREG32(&XEPON_MAC_BASE->e_llid10_11_gnt_stat) // 63A4
#define e_llid12_13_gnt_stat                                   INREG32(&XEPON_MAC_BASE->e_llid12_13_gnt_stat) // 63A8
#define e_llid14_15_gnt_stat                                   INREG32(&XEPON_MAC_BASE->e_llid14_15_gnt_stat) // 63AC
#define e_llid16_17_gnt_stat                                   INREG32(&XEPON_MAC_BASE->e_llid16_17_gnt_stat) // 63B0
#define e_llid18_19_gnt_stat                                   INREG32(&XEPON_MAC_BASE->e_llid18_19_gnt_stat) // 63B4
#define e_llid20_21_gnt_stat                                   INREG32(&XEPON_MAC_BASE->e_llid20_21_gnt_stat) // 63B8
#define e_llid22_23_gnt_stat                                   INREG32(&XEPON_MAC_BASE->e_llid22_23_gnt_stat) // 63BC
#define e_llid24_25_gnt_stat                                   INREG32(&XEPON_MAC_BASE->e_llid24_25_gnt_stat) // 63C0
#define e_llid26_27_gnt_stat                                   INREG32(&XEPON_MAC_BASE->e_llid26_27_gnt_stat) // 63C4
#define e_llid28_29_gnt_stat                                   INREG32(&XEPON_MAC_BASE->e_llid28_29_gnt_stat) // 63C8
#define e_llid30_31_gnt_stat                                   INREG32(&XEPON_MAC_BASE->e_llid30_31_gnt_stat) // 63CC
#define e_rxfifo_depth_stat                                    INREG32(&XEPON_MAC_BASE->e_rxfifo_depth_stat) // 63D0
#define e_rdmdly_stat                                          INREG32(&XEPON_MAC_BASE->e_rdmdly_stat) // 63DC
#define e_int_sts3                                             INREG32(&XEPON_MAC_BASE->e_int_sts3) // 6404
#define e_int_en3                                              INREG32(&XEPON_MAC_BASE->e_int_en3) // 6408
#define e_rpt_mpcp_timout2                                     INREG32(&XEPON_MAC_BASE->e_rpt_mpcp_timout2) // 640C
#define e_llid8_11_cfg                                         INREG32(&XEPON_MAC_BASE->e_llid8_11_cfg) // 6410
#define e_llid12_15_cfg                                        INREG32(&XEPON_MAC_BASE->e_llid12_15_cfg) // 6414
#define e_llid16_19_cfg                                        INREG32(&XEPON_MAC_BASE->e_llid16_19_cfg) // 6418
#define e_llid20_23_cfg                                        INREG32(&XEPON_MAC_BASE->e_llid20_23_cfg) // 641C
#define e_llid24_27_cfg                                        INREG32(&XEPON_MAC_BASE->e_llid24_27_cfg) // 6420
#define e_llid28_31_cfg                                        INREG32(&XEPON_MAC_BASE->e_llid28_31_cfg) // 6424
#define e_llid8_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid8_dscvry_sts) // 6428
#define e_llid9_dscvry_sts                                     INREG32(&XEPON_MAC_BASE->e_llid9_dscvry_sts) // 642C
#define e_llid10_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid10_dscvry_sts) // 6430
#define e_llid11_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid11_dscvry_sts) // 6434
#define e_llid12_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid12_dscvry_sts) // 6438
#define e_llid13_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid13_dscvry_sts) // 643C
#define e_llid14_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid14_dscvry_sts) // 6440
#define e_llid15_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid15_dscvry_sts) // 6444
#define e_llid16_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid16_dscvry_sts) // 6448
#define e_llid17_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid17_dscvry_sts) // 644C
#define e_llid18_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid18_dscvry_sts) // 6450
#define e_llid19_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid19_dscvry_sts) // 6454
#define e_llid20_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid20_dscvry_sts) // 6458
#define e_llid21_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid21_dscvry_sts) // 645C
#define e_llid22_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid22_dscvry_sts) // 6460
#define e_llid23_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid23_dscvry_sts) // 6464
#define e_llid24_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid24_dscvry_sts) // 6468
#define e_llid25_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid25_dscvry_sts) // 646C
#define e_llid26_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid26_dscvry_sts) // 6470
#define e_llid27_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid27_dscvry_sts) // 6474
#define e_llid28_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid28_dscvry_sts) // 6478
#define e_llid29_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid29_dscvry_sts) // 647C
#define e_llid30_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid30_dscvry_sts) // 6480
#define e_llid31_dscvry_sts                                    INREG32(&XEPON_MAC_BASE->e_llid31_dscvry_sts) // 6484
#define e_llid8_31_dscvry_sts(i) \
({ \
	int idx = (i);\
	INREG32(&(XEPON_MAC_BASE->e_llid8_31_dscvry_sts[idx])); \
})

#define e_rpt_cfg2                                             INREG32(&XEPON_MAC_BASE->e_rpt_cfg2) // 6488
#define e_rpt_cfg3                                             INREG32(&XEPON_MAC_BASE->e_rpt_cfg3) // 648C
#define e_rpt_qthld_cfg2                                       INREG32(&XEPON_MAC_BASE->e_rpt_qthld_cfg2) // 6490
#define e_rpt_bitmap_cfg                                       INREG32(&XEPON_MAC_BASE->e_rpt_bitmap_cfg) // 6494
#define e_rpt_bitmap_val                                       INREG32(&XEPON_MAC_BASE->e_rpt_bitmap_val) // 6498

#endif


#define e_glb_cfg_FLD_sniffer_mode                             REG_FLD(1, 26)
#define e_glb_cfg_FLD_txoam_favor                              REG_FLD(1, 25)
#define e_glb_cfg_FLD_burst_en_dly                             REG_FLD(1, 24)
#define e_glb_cfg_FLD_discv_burst_en                           REG_FLD(1, 23)
#define e_glb_cfg_FLD_mpcp_fwd                                 REG_FLD(1, 22)
#define e_glb_cfg_FLD_bcst_llid_m1_drop                        REG_FLD(1, 21)
#define e_glb_cfg_FLD_bcst_llid_m0_drop                        REG_FLD(1, 20)
#define e_glb_cfg_FLD_mcst_llid_drop                           REG_FLD(1, 19)
#define e_glb_cfg_FLD_all_unicast_llid_pkt_fwd                 REG_FLD(1, 18)
#define e_glb_cfg_FLD_fcs_err_fwd                              REG_FLD(1, 17)
#define e_glb_cfg_FLD_llid_crc8_err_fwd                        REG_FLD(1, 16)
#define e_glb_cfg_FLD_rxmpi_stop                               REG_FLD(1, 13)
#define e_glb_cfg_FLD_txmpi_stop                               REG_FLD(1, 12)
#define e_glb_cfg_FLD_phy_pwr_down                             REG_FLD(1, 11)
#define e_glb_cfg_FLD_rx_nml_gate_fwd                          REG_FLD(1, 10)
#define e_glb_cfg_FLD_rxmbi_stop                               REG_FLD(1, 9)
#define e_glb_cfg_FLD_txmbi_stop                               REG_FLD(1, 8)
#define e_glb_cfg_FLD_chk_all_gnt_mode                         REG_FLD(1, 7)
#define e_glb_cfg_FLD_tx_default_rpt                           REG_FLD(1, 5)
#define e_glb_cfg_FLD_epon_mac_sw_rst                          REG_FLD(1, 4)
#define e_glb_cfg_FLD_epon_oam_cal_in_eth                      REG_FLD(1, 3)
#define e_glb_cfg_FLD_epon_mac_lpbk_en                         REG_FLD(1, 2)
#define e_glb_cfg_FLD_rpt_txpri_ctrl                           REG_FLD(1, 1)

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

#define e_rpt_mpcp_timeout_llid_idx_FLD_llid15_mpcp_tmo        REG_FLD(1, 31)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid14_mpcp_tmo        REG_FLD(1, 30)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid13_mpcp_tmo        REG_FLD(1, 29)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid12_mpcp_tmo        REG_FLD(1, 28)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid11_mpcp_tmo        REG_FLD(1, 27)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid10_mpcp_tmo        REG_FLD(1, 26)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid9_mpcp_tmo         REG_FLD(1, 25)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid8_mpcp_tmo         REG_FLD(1, 24)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid15_rpt_tmo         REG_FLD(1, 23)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid14_rpt_tmo         REG_FLD(1, 22)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid13_rpt_tmo         REG_FLD(1, 21)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid12_rpt_tmo         REG_FLD(1, 20)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid11_rpt_tmo         REG_FLD(1, 19)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid10_rpt_tmo         REG_FLD(1, 18)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid9_rpt_tmo          REG_FLD(1, 17)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid8_rpt_tmo          REG_FLD(1, 16)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid7_mpcp_tmo         REG_FLD(1, 15)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid6_mpcp_tmo         REG_FLD(1, 14)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid5_mpcp_tmo         REG_FLD(1, 13)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid4_mpcp_tmo         REG_FLD(1, 12)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid3_mpcp_tmo         REG_FLD(1, 11)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid2_mpcp_tmo         REG_FLD(1, 10)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid1_mpcp_tmo         REG_FLD(1, 9)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid0_mpcp_tmo         REG_FLD(1, 8)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid7_rpt_tmo          REG_FLD(1, 7)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid6_rpt_tmo          REG_FLD(1, 6)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid5_rpt_tmo          REG_FLD(1, 5)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid4_rpt_tmo          REG_FLD(1, 4)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid3_rpt_tmo          REG_FLD(1, 3)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid2_rpt_tmo          REG_FLD(1, 2)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid1_rpt_tmo          REG_FLD(1, 1)
#define e_rpt_mpcp_timeout_llid_idx_FLD_llid0_rpt_tmo          REG_FLD(1, 0)

#define e_dyinggsp_cfg_FLD_hw_dying_gasp_en                    REG_FLD(1, 31)
#define e_dyinggsp_cfg_FLD_sw_init_dying_gasp                  REG_FLD(1, 16)
#define e_dyinggsp_cfg_FLD_dygsp_num_of_times                  REG_FLD(2, 8)

#define e_pending_gnt_num_FLD_echoed_pending_gnt               REG_FLD(8, 8)
#define e_pending_gnt_num_FLD_pending_gnt_num                  REG_FLD(8, 0)

#define e_llid0_3_cfg_FLD_llid3_dummy                          REG_FLD(4, 28)
#define e_llid0_3_cfg_FLD_llid3_txfec_en                       REG_FLD(1, 27)
#define e_llid0_3_cfg_FLD_llid3_dcrypt_en                      REG_FLD(1, 26)
#define e_llid0_3_cfg_FLD_llid3_dcrypt_mode                    REG_FLD(1, 25)
#define e_llid0_3_cfg_FLD_llid3_oam_lpbk_en                    REG_FLD(1, 24)
#define e_llid0_3_cfg_FLD_llid2_dummy                          REG_FLD(4, 20)
#define e_llid0_3_cfg_FLD_llid2_txfec_en                       REG_FLD(1, 19)
#define e_llid0_3_cfg_FLD_llid2_dcrypt_en                      REG_FLD(1, 18)
#define e_llid0_3_cfg_FLD_llid2_dcrypt_mode                    REG_FLD(1, 17)
#define e_llid0_3_cfg_FLD_llid2_oam_lpbk_en                    REG_FLD(1, 16)
#define e_llid0_3_cfg_FLD_llid1_dummy                          REG_FLD(4, 12)
#define e_llid0_3_cfg_FLD_llid1_txfec_en                       REG_FLD(1, 11)
#define e_llid0_3_cfg_FLD_llid1_dcrypt_en                      REG_FLD(1, 10)
#define e_llid0_3_cfg_FLD_llid1_dcrypt_mode                    REG_FLD(1, 9)
#define e_llid0_3_cfg_FLD_llid1_oam_lpbk_en                    REG_FLD(1, 8)
#define e_llid0_3_cfg_FLD_llid0_dummy                          REG_FLD(4, 4)
#define e_llid0_3_cfg_FLD_llid0_txfec_en                       REG_FLD(1, 3)
#define e_llid0_3_cfg_FLD_llid0_dcrypt_en                      REG_FLD(1, 2)
#define e_llid0_3_cfg_FLD_llid0_dcrypt_mode                    REG_FLD(1, 1)
#define e_llid0_3_cfg_FLD_llid0_oam_lpbk_en                    REG_FLD(1, 0)

#define e_llid4_7_cfg_FLD_llid7_dummy                          REG_FLD(4, 28)
#define e_llid4_7_cfg_FLD_llid7_txfec_en                       REG_FLD(1, 27)
#define e_llid4_7_cfg_FLD_llid7_dcrypt_en                      REG_FLD(1, 26)
#define e_llid4_7_cfg_FLD_llid7_dcrypt_mode                    REG_FLD(1, 25)
#define e_llid4_7_cfg_FLD_llid7_oam_lpbk_en                    REG_FLD(1, 24)
#define e_llid4_7_cfg_FLD_llid6_dummy                          REG_FLD(4, 20)
#define e_llid4_7_cfg_FLD_llid6_txfec_en                       REG_FLD(1, 19)
#define e_llid4_7_cfg_FLD_llid6_dcrypt_en                      REG_FLD(1, 18)
#define e_llid4_7_cfg_FLD_llid6_dcrypt_mode                    REG_FLD(1, 17)
#define e_llid4_7_cfg_FLD_llid6_oam_lpbk_en                    REG_FLD(1, 16)
#define e_llid4_7_cfg_FLD_llid5_dummy                          REG_FLD(4, 12)
#define e_llid4_7_cfg_FLD_llid5_txfec_en                       REG_FLD(1, 11)
#define e_llid4_7_cfg_FLD_llid5_dcrypt_en                      REG_FLD(1, 10)
#define e_llid4_7_cfg_FLD_llid5_dcrypt_mode                    REG_FLD(1, 9)
#define e_llid4_7_cfg_FLD_llid5_oam_lpbk_en                    REG_FLD(1, 8)
#define e_llid4_7_cfg_FLD_llid4_dummy                          REG_FLD(4, 4)
#define e_llid4_7_cfg_FLD_llid4_txfec_en                       REG_FLD(1, 3)
#define e_llid4_7_cfg_FLD_llid4_dcrypt_en                      REG_FLD(1, 2)
#define e_llid4_7_cfg_FLD_llid4_dcrypt_mode                    REG_FLD(1, 1)
#define e_llid4_7_cfg_FLD_llid4_oam_lpbk_en                    REG_FLD(1, 0)

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

#define e_mac_addr_cfg_FLD_mac_addr_rwcmd                      REG_FLD(1, 31)
#define e_mac_addr_cfg_FLD_mac_addr_rwcmd_done                 REG_FLD(1, 16)
#define e_mac_addr_cfg_FLD_mac_addr_llid_indx                  REG_FLD(5, 1)
#define e_mac_addr_cfg_FLD_mac_addr_dw_idx                     REG_FLD(1, 0)

#define e_mac_addr_value_FLD_mac_addr_value                    REG_FLD(32, 0)

#define e_security_key_cfg_FLD_key_rwcmd                       REG_FLD(1, 31)
#define e_security_key_cfg_FLD_key_rwcmd_done                  REG_FLD(1, 16)
#define e_security_key_cfg_FLD_key_llid_index                  REG_FLD(5, 3)
#define e_security_key_cfg_FLD_key_idx                         REG_FLD(1, 2)
#define e_security_key_cfg_FLD_key_dw_indx                     REG_FLD(2, 0)

#define e_key_value_FLD_key_value                              REG_FLD(24, 0)

#define e_rpt_cfg_FLD_llid7_rpt_cfg                            REG_FLD(2, 14)
#define e_rpt_cfg_FLD_llid6_rpt_cfg                            REG_FLD(2, 12)
#define e_rpt_cfg_FLD_llid5_rpt_cfg                            REG_FLD(2, 10)
#define e_rpt_cfg_FLD_llid4_rpt_cfg                            REG_FLD(2, 8)
#define e_rpt_cfg_FLD_llid3_rpt_cfg                            REG_FLD(2, 6)
#define e_rpt_cfg_FLD_llid2_rpt_cfg                            REG_FLD(2, 4)
#define e_rpt_cfg_FLD_llid1_rpt_cfg                            REG_FLD(2, 2)
#define e_rpt_cfg_FLD_llid0_rpt_cfg                            REG_FLD(2, 0)

#define e_rpt_qthld_cfg_FLD_qthld_rwcmd                        REG_FLD(1, 31)
#define e_rpt_qthld_cfg_FLD_qthld_rwcmd_done                   REG_FLD(1, 30)
#define e_rpt_qthld_cfg_FLD_qthld_value                        REG_FLD(16, 8)
#define e_rpt_qthld_cfg_FLD_qthld_idx                          REG_FLD(2, 6)
#define e_rpt_qthld_cfg_FLD_rpt_llid_idx_2_0                   REG_FLD(3, 3)
#define e_rpt_qthld_cfg_FLD_queue_idx                          REG_FLD(3, 0)

#define e_local_time_FLD_local_time                            REG_FLD(32, 0)

#define e_tod_sync_x_FLD_tod_sync_x                            REG_FLD(32, 0)

#define e_tod_ltncy_FLD_rl_rx_phydly_ofst                      REG_FLD(5, 19)
#define e_tod_ltncy_FLD_ingrs_latency                          REG_FLD(11, 8)
#define e_tod_ltncy_FLD_egrs_latency                           REG_FLD(8, 0)

#define e_new_tod_p2p_offset_sec_l32_FLD_new_tod_p2p_offset_sec_l32 REG_FLD(32, 0)

#define e_new_tod_p2p_tod_offset_nsec_FLD_new_tod_nsec         REG_FLD(32, 0)

#define e_tod_p2p_tod_sec_l32_FLD_tod_p2p_sec_l32              REG_FLD(32, 0)

#define e_tod_p2p_tod_nsec_FLD_tod_p2p_nsec                    REG_FLD(32, 0)

#define e_tod_period_FLD_tod_period                            REG_FLD(8, 0)

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

#define e_slp_duration_i_FLD_slp_pwd_mode_i                    REG_FLD(3, 24)
#define e_slp_duration_i_FLD_slp_flag_i                        REG_FLD(8, 16)
#define e_slp_duration_i_FLD_slp_duration_i                    REG_FLD(16, 0)

#define e_txfetch_cfg_FLD_tx_fetch_leadtime                    REG_FLD(8, 16)
#define e_txfetch_cfg_FLD_tx_dma_leadtime                      REG_FLD(16, 0)

#define e_sync_time_FLD_sync_time_updte                        REG_FLD(1, 16)
#define e_sync_time_FLD_sync_time                              REG_FLD(16, 0)

#define e_tx_cal_cnst_FLD_dscvr_gnt_len                        REG_FLD(8, 24)
#define e_tx_cal_cnst_FLD_fec_tail_grd                         REG_FLD(8, 16)
#define e_tx_cal_cnst_FLD_tail_grd                             REG_FLD(8, 8)
#define e_tx_cal_cnst_FLD_default_ovrhd                        REG_FLD(6, 0)

#define e_laser_onoff_time_FLD_laser_off_time                  REG_FLD(8, 8)
#define e_laser_onoff_time_FLD_laser_on_time                   REG_FLD(8, 0)

#define e_grd_thrshld_FLD_guard_thrshld                        REG_FLD(8, 0)

#define e_mpcp_timeout_intvl_FLD_mpcp_timeout_intvl            REG_FLD(32, 0)

#define e_rpt_timeout_intvl_FLD_rpt_timeout_intvl              REG_FLD(24, 0)

#define e_max_future_gnt_time_FLD_max_future_gnt_time          REG_FLD(32, 0)

#define e_min_proc_time_FLD_min_proc_time                      REG_FLD(16, 0)

#define e_trx_adjust_time1_FLD_tx_stm_adj                      REG_FLD(16, 0)

#define e_trx_adjust_time2_FLD_tx_len_adj                      REG_FLD(16, 16)
#define e_trx_adjust_time2_FLD_rx_tmstp_adj                    REG_FLD(16, 0)

#define e_dbg_prb_sel_FLD_probe_dtgrp_sel                      REG_FLD(2, 16)
#define e_dbg_prb_sel_FLD_probe_bit0_sel                       REG_FLD(5, 8)
#define e_dbg_prb_sel_FLD_probe_sel                            REG_FLD(5, 0)

#define e_dbg_prb_h32_FLD_probe_h32                            REG_FLD(32, 0)

#define e_dbg_prb_l32_FLD_probe_l32                            REG_FLD(32, 0)

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

#define e_gnt_pending_stat_FLD_max_gnt_pending_cnt             REG_FLD(8, 16)
#define e_gnt_pending_stat_FLD_cur_gnt_pending_cnt             REG_FLD(8, 0)

#define e_gnt_length_stat_FLD_max_gnt_length                   REG_FLD(16, 16)
#define e_gnt_length_stat_FLD_min_gnt_length                   REG_FLD(16, 0)

#define e_gnt_type_stat_FLD_b2b_gnt_cnt                        REG_FLD(16, 16)
#define e_gnt_type_stat_FLD_hdn_gnt_cnt                        REG_FLD(16, 0)

#define e_time_drft_stat_FLD_cur_time_drift_ofst               REG_FLD(5, 24)
#define e_time_drft_stat_FLD_max_time_drift_ofst               REG_FLD(5, 16)
#define e_time_drft_stat_FLD_cur_time_drift                    REG_FLD(8, 8)
#define e_time_drft_stat_FLD_max_time_drift                    REG_FLD(8, 0)

#define e_llid0_gnt_stat_FLD_llid0_gnt_cnt                     REG_FLD(16, 0)

#define e_llid1_gnt_stat_FLD_llid1_gnt_cnt                     REG_FLD(16, 0)

#define e_llid2_gnt_stat_FLD_llid2_gnt_cnt                     REG_FLD(16, 0)

#define e_llid3_gnt_stat_FLD_llid3_gnt_cnt                     REG_FLD(16, 0)

#define e_llid4_gnt_stat_FLD_llid4_gnt_cnt                     REG_FLD(16, 0)

#define e_llid5_gnt_stat_FLD_llid5_gnt_cnt                     REG_FLD(16, 0)

#define e_llid6_gnt_stat_FLD_llid6_gnt_cnt                     REG_FLD(16, 0)

#define e_llid7_gnt_stat_FLD_llid7_gnt_cnt                     REG_FLD(16, 0)

#define e_snf_mpcp_oam_ctl_FLD_snf_mpcp_cap_en                 REG_FLD(1, 31)
#define e_snf_mpcp_oam_ctl_FLD_snf_oam_cap_en                  REG_FLD(1, 30)
#define e_snf_mpcp_oam_ctl_FLD_snf_mpcp_oam_cnt_set            REG_FLD(30, 0)

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

#define e_oam_kpalv_ctrl_FLD_oam_kpalv_interval                REG_FLD(24, 8)
#define e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_trig                 REG_FLD(1, 2)
#define e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_cfg                  REG_FLD(1, 1)
#define e_oam_kpalv_ctrl_FLD_oam_kpalv_en                      REG_FLD(1, 0)

#define e_tod_1pps_ctrl_FLD_tod_1pps_width_ctrl                REG_FLD(32, 0)

#define e_sniff_sp_tag_FLD_sniffer_sp_tag                      REG_FLD(32, 0)

#define e_glb_cfg2_FLD_llidnum_sel                             REG_FLD(1, 31)
#define e_glb_cfg2_FLD_loctime_mtd                             REG_FLD(1, 21)
#define e_glb_cfg2_FLD_rgreq_lsrtime_mask                      REG_FLD(1, 20)
#define e_glb_cfg2_FLD_all_ucllid_mpcpsnf                      REG_FLD(1, 17)
#define e_glb_cfg2_FLD_u10g_txmode                             REG_FLD(1, 16)
#define e_glb_cfg2_FLD_eth_cal_in_bytecnt                      REG_FLD(1, 15)
#define e_glb_cfg2_FLD_snf_cal_in_bytecnt                      REG_FLD(1, 14)
#define e_glb_cfg2_FLD_oam_cal_in_bytecnt                      REG_FLD(1, 13)
#define e_glb_cfg2_FLD_dscvgate_filt                           REG_FLD(1, 12)
#define e_glb_cfg2_FLD_us_snf                                  REG_FLD(1, 11)
#define e_glb_cfg2_FLD_llidinfo_snf                            REG_FLD(1, 10)
#define e_glb_cfg2_FLD_all_ucllid_ethsnf                       REG_FLD(1, 9)
#define e_glb_cfg2_FLD_all_ucllid_snf                          REG_FLD(1, 8)
#define e_glb_cfg2_FLD_rxuni_mcllid_en                         REG_FLD(1, 7)
#define e_glb_cfg2_FLD_gntlen_stat_widscv                      REG_FLD(1, 6)
#define e_glb_cfg2_FLD_tdrift_loctupd_dis                      REG_FLD(1, 5)
#define e_glb_cfg2_FLD_snf_fcserr_fwd                          REG_FLD(1, 2)
#define e_glb_cfg2_FLD_mpcp_fcserr_fwd                         REG_FLD(1, 1)
#define e_glb_cfg2_FLD_dscvgate_infochk_dis                    REG_FLD(1, 0)

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
#define e_int_sts2_FLD_rcv_dauc_dscvgate_int                   REG_FLD(1, 0)

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
#define e_int_en2_FLD_rcv_dauc_dscvgate_en                     REG_FLD(1, 0)

#define e_exc_sts_FLD_rx_rgst_dergst_hit                       REG_FLD(1, 25)
#define e_exc_sts_FLD_rx_rgst_nack_hit                         REG_FLD(1, 24)
#define e_exc_sts_FLD_snf_pktcnt_err_hit                       REG_FLD(1, 20)
#define e_exc_sts_FLD_tx_gntst_nosog_hit                       REG_FLD(1, 19)
#define e_exc_sts_FLD_tx_gntst_fifound_hit                     REG_FLD(1, 18)
#define e_exc_sts_FLD_tx_gntph_fifound_hit                     REG_FLD(1, 17)
#define e_exc_sts_FLD_tx_gntph_exceed_hit                      REG_FLD(1, 16)
#define e_exc_sts_FLD_schgnt_stinv_hit                         REG_FLD(1, 13)
#define e_exc_sts_FLD_schgnt_idxinv_hit                        REG_FLD(1, 12)
#define e_exc_sts_FLD_decrpt_vecm_acccol_hit                   REG_FLD(1, 10)
#define e_exc_sts_FLD_rcv_len_long_hit                         REG_FLD(1, 9)
#define e_exc_sts_FLD_rcv_len_short_hit                        REG_FLD(1, 8)
#define e_exc_sts_FLD_rcv_mpcp_crcerr_hit                      REG_FLD(1, 6)
#define e_exc_sts_FLD_rcv_eth_crcerr_hit                       REG_FLD(1, 5)
#define e_exc_sts_FLD_rcv_crc8err_hit                          REG_FLD(1, 4)
#define e_exc_sts_FLD_rcv_nrlgate_llidbc_hit                   REG_FLD(1, 2)
#define e_exc_sts_FLD_rcv_dscvgate_filt_hit                    REG_FLD(1, 1)
#define e_exc_sts_FLD_rcv_dscvgate_infomis_hit                 REG_FLD(1, 0)

#define e_olt_dscvinfo_FLD_olt_dscvinfo_match                  REG_FLD(4, 4)
#define e_olt_dscvinfo_FLD_olt_dscvinfo_mis                    REG_FLD(4, 0)

#define e_laser_onoff_time2_FLD_lsroff_time_olt                REG_FLD(8, 24)
#define e_laser_onoff_time2_FLD_lsron_time_olt                 REG_FLD(8, 16)
#define e_laser_onoff_time2_FLD_sync_time_olt                  REG_FLD(16, 0)

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

#define e_overhead_time_thr_FLD_sync_time_maxen                REG_FLD(1, 31)
#define e_overhead_time_thr_FLD_lsroff_time_maxen              REG_FLD(1, 30)
#define e_overhead_time_thr_FLD_lsron_time_maxen               REG_FLD(1, 29)
#define e_overhead_time_thr_FLD_sync_time_maxval               REG_FLD(8, 16)
#define e_overhead_time_thr_FLD_lsroff_time_maxval             REG_FLD(8, 8)
#define e_overhead_time_thr_FLD_lsron_time_maxval              REG_FLD(8, 0)

#define e_txcal_cnst2_FLD_ipgalign_mtd                         REG_FLD(1, 31)
#define e_txcal_cnst2_FLD_u10g_dscv_gntlen                     REG_FLD(8, 8)
#define e_txcal_cnst2_FLD_u10g_tail_grd                        REG_FLD(8, 0)

#define e_txcal_cnst3_FLD_u1g_fecon_min_gntlen                 REG_FLD(8, 24)
#define e_txcal_cnst3_FLD_u10g_eoblen                          REG_FLD(8, 16)
#define e_txcal_cnst3_FLD_u10g_min_gntlen                      REG_FLD(8, 8)
#define e_txcal_cnst3_FLD_u1g_fecoff_min_gntlen                REG_FLD(8, 0)

#define e_txsch_cfg_FLD_txsch_dmy0                             REG_FLD(6, 26)
#define e_txsch_cfg_FLD_txfifo_pad_hthr                        REG_FLD(10, 16)
#define e_txsch_cfg_FLD_txsch_dmy1                             REG_FLD(6, 10)
#define e_txsch_cfg_FLD_txfifo_pad_lthr                        REG_FLD(10, 0)

#define e_rxfifo_thr_FLD_rxfifo_mbithr                         REG_FLD(5, 0)

#define e_bcllid_cfg_FLD_d10g_bcllid                           REG_FLD(16, 16)
#define e_bcllid_cfg_FLD_d1g_bcllid                            REG_FLD(16, 0)

#define e_txfrm_cfg1_FLD_tx_mpcp_addrl                         REG_FLD(32, 0)

#define e_txfrm_cfg2_FLD_tx_mpcp_addrh                         REG_FLD(16, 16)
#define e_txfrm_cfg2_FLD_tx_mpcp_etype                         REG_FLD(16, 0)

#define e_txfrm_cfg3_FLD_tx_rgreq_op                           REG_FLD(16, 16)
#define e_txfrm_cfg3_FLD_tx_rgack_op                           REG_FLD(16, 0)

#define e_u1g_rpt_qsizeadj1_FLD_u1g_fecoff_rpt_q1_qsizeadj     REG_FLD(16, 16)
#define e_u1g_rpt_qsizeadj1_FLD_u1g_fecoff_rpt_q0_qsizeadj     REG_FLD(16, 0)

#define e_u1g_rpt_qsizeadj2_FLD_u1g_fecoff_rpt_q3_qsizeadj     REG_FLD(16, 16)
#define e_u1g_rpt_qsizeadj2_FLD_u1g_fecoff_rpt_q2_qsizeadj     REG_FLD(16, 0)

#define e_u1g_rpt_qsizeadj3_FLD_u1g_fecoff_rpt_q5_qsizeadj     REG_FLD(16, 16)
#define e_u1g_rpt_qsizeadj3_FLD_u1g_fecoff_rpt_q4_qsizeadj     REG_FLD(16, 0)

#define e_u1g_rpt_qsizeadj4_FLD_u1g_fecoff_rpt_q7_qsizeadj     REG_FLD(16, 16)
#define e_u1g_rpt_qsizeadj4_FLD_u1g_fecoff_rpt_q6_qsizeadj     REG_FLD(16, 0)

#define e_u1g_rpt_qsizeadj5_FLD_u1g_fecon_rpt_q1_qsizeadj      REG_FLD(16, 16)
#define e_u1g_rpt_qsizeadj5_FLD_u1g_fecon_rpt_q0_qsizeadj      REG_FLD(16, 0)

#define e_u1g_rpt_qsizeadj6_FLD_u1g_fecon_rpt_q3_qsizeadj      REG_FLD(16, 16)
#define e_u1g_rpt_qsizeadj6_FLD_u1g_fecon_rpt_q2_qsizeadj      REG_FLD(16, 0)

#define e_u1g_rpt_qsizeadj7_FLD_u1g_fecon_rpt_q5_qsizeadj      REG_FLD(16, 16)
#define e_u1g_rpt_qsizeadj7_FLD_u1g_fecon_rpt_q4_qsizeadj      REG_FLD(16, 0)

#define e_u1g_rpt_qsizeadj8_FLD_u1g_fecon_rpt_q7_qsizeadj      REG_FLD(16, 16)
#define e_u1g_rpt_qsizeadj8_FLD_u1g_fecon_rpt_q6_qsizeadj      REG_FLD(16, 0)

#define e_u10g_rpt_qsizeadj1_FLD_u10g_rpt_q1_qsizeadj          REG_FLD(16, 16)
#define e_u10g_rpt_qsizeadj1_FLD_u10g_rpt_q0_qsizeadj          REG_FLD(16, 0)

#define e_u10g_rpt_qsizeadj2_FLD_u10g_rpt_q3_qsizeadj          REG_FLD(16, 16)
#define e_u10g_rpt_qsizeadj2_FLD_u10g_rpt_q2_qsizeadj          REG_FLD(16, 0)

#define e_u10g_rpt_qsizeadj3_FLD_u10g_rpt_q5_qsizeadj          REG_FLD(16, 16)
#define e_u10g_rpt_qsizeadj3_FLD_u10g_rpt_q4_qsizeadj          REG_FLD(16, 0)

#define e_u10g_rpt_qsizeadj4_FLD_u10g_rpt_q7_qsizeadj          REG_FLD(16, 16)
#define e_u10g_rpt_qsizeadj4_FLD_u10g_rpt_q6_qsizeadj          REG_FLD(16, 0)

#define e_snf_dah_FLD_snf_pkt_dah                              REG_FLD(32, 0)

#define e_snf_dal_FLD_snf_pkt_dal                              REG_FLD(16, 16)
#define e_snf_dal_FLD_snf_pkt_sah                              REG_FLD(16, 0)

#define e_snf_sal_FLD_snf_pkt_sal                              REG_FLD(32, 0)

#define e_snf_etype_FLD_snf_pkt_etype                          REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg0_FLD_rxuni_mcllid1                  REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg0_FLD_rxuni_mcllid0                  REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg1_FLD_rxuni_mcllid3                  REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg1_FLD_rxuni_mcllid2                  REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg2_FLD_rxuni_mcllid5                  REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg2_FLD_rxuni_mcllid4                  REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg3_FLD_rxuni_mcllid7                  REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg3_FLD_rxuni_mcllid6                  REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg4_FLD_rxuni_mcllid9                  REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg4_FLD_rxuni_mcllid8                  REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg5_FLD_rxuni_mcllid11                 REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg5_FLD_rxuni_mcllid10                 REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg6_FLD_rxuni_mcllid13                 REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg6_FLD_rxuni_mcllid12                 REG_FLD(16, 0)

#define e_rxuni_mcllid_cfg7_FLD_rxuni_mcllid15                 REG_FLD(16, 16)
#define e_rxuni_mcllid_cfg7_FLD_rxuni_mcllid14                 REG_FLD(16, 0)

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

#define e_glue_cfg_FLD_txmpi_fifound_pktgate_en                REG_FLD(1, 29)
#define e_glue_cfg_FLD_txmpi_fifound_thr                       REG_FLD(11, 0)

#define e_gntreq_tmout_FLD_gntreq_grdcyc                       REG_FLD(32, 0)

#define e_glb_sts_FLD_xepon_type                               REG_FLD(2, 0)

#define e_cnt_clr_FLD_glb_cntclr                               REG_FLD(1, 0)

#define e_rxadv_cnt_FLD_rx_infomis_dscvgate_cnt                REG_FLD(8, 24)
#define e_rxadv_cnt_FLD_rx_dauc_dscvgate_cnt                   REG_FLD(8, 16)
#define e_rxadv_cnt_FLD_rx_filt_dscvgate_cnt                   REG_FLD(8, 8)

#define e_tx_dygasp_cnt_FLD_txmbi_dygasp_cnt                   REG_FLD(8, 24)
#define e_tx_dygasp_cnt_FLD_txmpi_dygasp_cnt                   REG_FLD(8, 16)

#define e_tx_rpt_cnt_FLD_txmbi_rpt_cnt                         REG_FLD(16, 16)
#define e_tx_rpt_cnt_FLD_txmpi_rpt_cnt                         REG_FLD(16, 0)

#define e_txmpi_mpcp_cnt_FLD_txmpi_oam_cnt                     REG_FLD(16, 16)
#define e_txmpi_mpcp_cnt_FLD_txmpi_rgreq_cnt                   REG_FLD(8, 8)
#define e_txmpi_mpcp_cnt_FLD_txmpi_rgack_cnt                   REG_FLD(8, 0)

#define e_rxmpi_uc_cnt_FLD_rxmpi_uceth_cnt                     REG_FLD(32, 0)

#define e_rxmpi_bc_cnt_FLD_rxmpi_bceth_cnt                     REG_FLD(32, 0)

#define e_rxmpi_mc_cnt_FLD_rxmpi_mceth_cnt                     REG_FLD(32, 0)

#define e_rxmpi_oam_cnt_FLD_rxmpi_oam_cnt                      REG_FLD(32, 0)

#define e_rxmpi_mpcp_cnt_FLD_rxmpi_mpcp_cnt                    REG_FLD(32, 0)

#define e_rxmpi_gate_cnt_FLD_rxmpi_gate_cnt                    REG_FLD(32, 0)

#define e_rxmpi_nrlgate_cnt_FLD_rxmpi_nrlgate_cnt              REG_FLD(32, 0)

#define e_rxmpi_crc8err_cnt_FLD_rxmpi_crceth_cnt               REG_FLD(16, 16)
#define e_rxmpi_crc8err_cnt_FLD_rxmpi_crc8err_cnt              REG_FLD(16, 0)

#define e_rxmpi_drop_cnt_FLD_rxmpi_eofdrop_cnt                 REG_FLD(16, 0)

#define e_rxmpi_churn_cnt_FLD_rxmpi_abchurn_cnt                REG_FLD(16, 16)
#define e_rxmpi_churn_cnt_FLD_rxmpi_churn_cnt                  REG_FLD(16, 0)

#define e_rxmbi_snf_cnt_FLD_rxmbi_snf_cnt                      REG_FLD(32, 0)

#define e_rxmbi_drop_cnt_FLD_rxmbi_sofdrop_cnt                 REG_FLD(16, 16)
#define e_rxmbi_drop_cnt_FLD_rxmbi_snfdrop_cnt                 REG_FLD(16, 0)

#define e_rxmbi_drop_cnt2_FLD_rxmbi_crcerr_cnt                 REG_FLD(16, 16)
#define e_rxmbi_drop_cnt2_FLD_rxmbi_enddrop_cnt                REG_FLD(16, 0)

#define e_rxmbi_bytecnt_h_FLD_rxmbi_bytecnt_h                  REG_FLD(32, 0)

#define e_rxmbi_bytecnt_l_FLD_rxmbi_bytecnt_l                  REG_FLD(32, 0)

#define e_txmbi_uceth_cnt_FLD_txmbi_uceth_cnt                  REG_FLD(32, 0)

#define e_txmbi_mceth_cnt_FLD_txmbi_bceth_cnt                  REG_FLD(16, 16)
#define e_txmbi_mceth_cnt_FLD_txmbi_mceth_cnt                  REG_FLD(16, 0)

#define e_txmbi_err_cnt_FLD_txmbi_err_cnt                      REG_FLD(16, 0)

#define e_tx_timedrift_stat_FLD_tx_cur_time_drift_ofst         REG_FLD(5, 24)
#define e_tx_timedrift_stat_FLD_tx_max_time_drift_ofst         REG_FLD(5, 16)
#define e_tx_timedrift_stat_FLD_tx_cur_time_drift              REG_FLD(8, 8)
#define e_tx_timedrift_stat_FLD_tx_max_time_drift              REG_FLD(8, 0)

#define e_rxmpi_churn_cnt2_FLD_rxmpi_churn_mpcpcnt             REG_FLD(16, 16)
#define e_rxmpi_churn_cnt2_FLD_rxmpi_churn_ethcnt              REG_FLD(16, 0)

#define e_rxmpi_churn_cnt3_FLD_rxmpi_churn_keyidx              REG_FLD(1, 31)
#define e_rxmpi_churn_cnt3_FLD_rxmpi_churn_llididx             REG_FLD(5, 24)
#define e_rxmpi_churn_cnt3_FLD_rxmpi_churncrc_keyidx           REG_FLD(1, 23)
#define e_rxmpi_churn_cnt3_FLD_rxmpi_churncrc_llididx          REG_FLD(5, 16)
#define e_rxmpi_churn_cnt3_FLD_rxmpi_churn_crccnt              REG_FLD(16, 0)

#define e_txmpi_uceth_cnt_FLD_txmpi_uceth_cnt                  REG_FLD(32, 0)

#define e_txmpi_mceth_cnt_FLD_txmpi_bceth_cnt                  REG_FLD(16, 16)
#define e_txmpi_mceth_cnt_FLD_txmpi_mceth_cnt                  REG_FLD(16, 0)

#define e_llid8_9_gnt_stat_FLD_llid9_gnt_cnt                   REG_FLD(16, 16)
#define e_llid8_9_gnt_stat_FLD_llid8_gnt_cnt                   REG_FLD(16, 0)

#define e_llid10_11_gnt_stat_FLD_llid11_gnt_cnt                REG_FLD(16, 16)
#define e_llid10_11_gnt_stat_FLD_llid10_gnt_cnt                REG_FLD(16, 0)

#define e_llid12_13_gnt_stat_FLD_llid13_gnt_cnt                REG_FLD(16, 16)
#define e_llid12_13_gnt_stat_FLD_llid12_gnt_cnt                REG_FLD(16, 0)

#define e_llid14_15_gnt_stat_FLD_llid15_gnt_cnt                REG_FLD(16, 16)
#define e_llid14_15_gnt_stat_FLD_llid14_gnt_cnt                REG_FLD(16, 0)

#define e_llid16_17_gnt_stat_FLD_llid17_gnt_cnt                REG_FLD(16, 16)
#define e_llid16_17_gnt_stat_FLD_llid16_gnt_cnt                REG_FLD(16, 0)

#define e_llid18_19_gnt_stat_FLD_llid19_gnt_cnt                REG_FLD(16, 16)
#define e_llid18_19_gnt_stat_FLD_llid18_gnt_cnt                REG_FLD(16, 0)

#define e_llid20_21_gnt_stat_FLD_llid21_gnt_cnt                REG_FLD(16, 16)
#define e_llid20_21_gnt_stat_FLD_llid20_gnt_cnt                REG_FLD(16, 0)

#define e_llid22_23_gnt_stat_FLD_llid23_gnt_cnt                REG_FLD(16, 16)
#define e_llid22_23_gnt_stat_FLD_llid22_gnt_cnt                REG_FLD(16, 0)

#define e_llid24_25_gnt_stat_FLD_llid25_gnt_cnt                REG_FLD(16, 16)
#define e_llid24_25_gnt_stat_FLD_llid24_gnt_cnt                REG_FLD(16, 0)

#define e_llid26_27_gnt_stat_FLD_llid27_gnt_cnt                REG_FLD(16, 16)
#define e_llid26_27_gnt_stat_FLD_llid26_gnt_cnt                REG_FLD(16, 0)

#define e_llid28_29_gnt_stat_FLD_llid29_gnt_cnt                REG_FLD(16, 16)
#define e_llid28_29_gnt_stat_FLD_llid28_gnt_cnt                REG_FLD(16, 0)

#define e_llid30_31_gnt_stat_FLD_llid31_gnt_cnt                REG_FLD(16, 16)
#define e_llid30_31_gnt_stat_FLD_llid30_gnt_cnt                REG_FLD(16, 0)

#define e_rxfifo_depth_stat_FLD_max_rxfifo_depth               REG_FLD(16, 0)

#define e_rdmdly_stat_FLD_rdmdly                               REG_FLD(16, 0)

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

#define e_rpt_mpcp_timout2_FLD_llid31_mpcp_tmo                 REG_FLD(1, 31)
#define e_rpt_mpcp_timout2_FLD_llid30_mpcp_tmo                 REG_FLD(1, 30)
#define e_rpt_mpcp_timout2_FLD_llid29_mpcp_tmo                 REG_FLD(1, 29)
#define e_rpt_mpcp_timout2_FLD_llid28_mpcp_tmo                 REG_FLD(1, 28)
#define e_rpt_mpcp_timout2_FLD_llid27_mpcp_tmo                 REG_FLD(1, 27)
#define e_rpt_mpcp_timout2_FLD_llid26_mpcp_tmo                 REG_FLD(1, 26)
#define e_rpt_mpcp_timout2_FLD_llid25_mpcp_tmo                 REG_FLD(1, 25)
#define e_rpt_mpcp_timout2_FLD_llid24_mpcp_tmo                 REG_FLD(1, 24)
#define e_rpt_mpcp_timout2_FLD_llid23_mpcp_tmo                 REG_FLD(1, 23)
#define e_rpt_mpcp_timout2_FLD_llid22_mpcp_tmo                 REG_FLD(1, 22)
#define e_rpt_mpcp_timout2_FLD_llid21_mpcp_tmo                 REG_FLD(1, 21)
#define e_rpt_mpcp_timout2_FLD_llid20_mpcp_tmo                 REG_FLD(1, 20)
#define e_rpt_mpcp_timout2_FLD_llid19_mpcp_tmo                 REG_FLD(1, 19)
#define e_rpt_mpcp_timout2_FLD_llid18_mpcp_tmo                 REG_FLD(1, 18)
#define e_rpt_mpcp_timout2_FLD_llid17_mpcp_tmo                 REG_FLD(1, 17)
#define e_rpt_mpcp_timout2_FLD_llid16_mpcp_tmo                 REG_FLD(1, 16)
#define e_rpt_mpcp_timout2_FLD_llid31_rpt_tmo                  REG_FLD(1, 15)
#define e_rpt_mpcp_timout2_FLD_llid30_rpt_tmo                  REG_FLD(1, 14)
#define e_rpt_mpcp_timout2_FLD_llid29_rpt_tmo                  REG_FLD(1, 13)
#define e_rpt_mpcp_timout2_FLD_llid28_rpt_tmo                  REG_FLD(1, 12)
#define e_rpt_mpcp_timout2_FLD_llid27_rpt_tmo                  REG_FLD(1, 11)
#define e_rpt_mpcp_timout2_FLD_llid26_rpt_tmo                  REG_FLD(1, 10)
#define e_rpt_mpcp_timout2_FLD_llid25_rpt_tmo                  REG_FLD(1, 9)
#define e_rpt_mpcp_timout2_FLD_llid24_rpt_tmo                  REG_FLD(1, 8)
#define e_rpt_mpcp_timout2_FLD_llid23_rpt_tmo                  REG_FLD(1, 7)
#define e_rpt_mpcp_timout2_FLD_llid22_rpt_tmo                  REG_FLD(1, 6)
#define e_rpt_mpcp_timout2_FLD_llid21_rpt_tmo                  REG_FLD(1, 5)
#define e_rpt_mpcp_timout2_FLD_llid20_rpt_tmo                  REG_FLD(1, 4)
#define e_rpt_mpcp_timout2_FLD_llid19_rpt_tmo                  REG_FLD(1, 3)
#define e_rpt_mpcp_timout2_FLD_llid18_rpt_tmo                  REG_FLD(1, 2)
#define e_rpt_mpcp_timout2_FLD_llid17_rpt_tmo                  REG_FLD(1, 1)
#define e_rpt_mpcp_timout2_FLD_llid16_rpt_tmo                  REG_FLD(1, 0)

#define e_llid8_11_cfg_FLD_llid11_dummy                        REG_FLD(4, 28)
#define e_llid8_11_cfg_FLD_llid11_txfec_en                     REG_FLD(1, 27)
#define e_llid8_11_cfg_FLD_llid11_dcrypt_en                    REG_FLD(1, 26)
#define e_llid8_11_cfg_FLD_llid11_dcrypt_mode                  REG_FLD(1, 25)
#define e_llid8_11_cfg_FLD_llid11_oamlpbk_en                   REG_FLD(1, 24)
#define e_llid8_11_cfg_FLD_llid10_dummy                        REG_FLD(4, 20)
#define e_llid8_11_cfg_FLD_llid10_txfec_en                     REG_FLD(1, 19)
#define e_llid8_11_cfg_FLD_llid10_dcrypt_en                    REG_FLD(1, 18)
#define e_llid8_11_cfg_FLD_llid10_dcrypt_mode                  REG_FLD(1, 17)
#define e_llid8_11_cfg_FLD_llid10_oamlpbk_en                   REG_FLD(1, 16)
#define e_llid8_11_cfg_FLD_llid9_dummy                         REG_FLD(4, 12)
#define e_llid8_11_cfg_FLD_llid9_txfec_en                      REG_FLD(1, 11)
#define e_llid8_11_cfg_FLD_llid9_dcrypt_en                     REG_FLD(1, 10)
#define e_llid8_11_cfg_FLD_llid9_dcrypt_mode                   REG_FLD(1, 9)
#define e_llid8_11_cfg_FLD_llid9_oamlpbk_en                    REG_FLD(1, 8)
#define e_llid8_11_cfg_FLD_llid8_dummy                         REG_FLD(4, 4)
#define e_llid8_11_cfg_FLD_llid8_txfec_en                      REG_FLD(1, 3)
#define e_llid8_11_cfg_FLD_llid8_dcrypt_en                     REG_FLD(1, 2)
#define e_llid8_11_cfg_FLD_llid8_dcrypt_mode                   REG_FLD(1, 1)
#define e_llid8_11_cfg_FLD_llid8_oamlpbk_en                    REG_FLD(1, 0)

#define e_llid12_15_cfg_FLD_llid15_dummy                       REG_FLD(4, 28)
#define e_llid12_15_cfg_FLD_llid15_txfec_en                    REG_FLD(1, 27)
#define e_llid12_15_cfg_FLD_llid15_dcrypt_en                   REG_FLD(1, 26)
#define e_llid12_15_cfg_FLD_llid15_dcrypt_mode                 REG_FLD(1, 25)
#define e_llid12_15_cfg_FLD_llid15_oamlpbk_en                  REG_FLD(1, 24)
#define e_llid12_15_cfg_FLD_llid14_dummy                       REG_FLD(4, 20)
#define e_llid12_15_cfg_FLD_llid14_txfec_en                    REG_FLD(1, 19)
#define e_llid12_15_cfg_FLD_llid14_dcrypt_en                   REG_FLD(1, 18)
#define e_llid12_15_cfg_FLD_llid14_dcrypt_mode                 REG_FLD(1, 17)
#define e_llid12_15_cfg_FLD_llid14_oamlpbk_en                  REG_FLD(1, 16)
#define e_llid12_15_cfg_FLD_llid13_dummy                       REG_FLD(4, 12)
#define e_llid12_15_cfg_FLD_llid13_txfec_en                    REG_FLD(1, 11)
#define e_llid12_15_cfg_FLD_llid13_dcrypt_en                   REG_FLD(1, 10)
#define e_llid12_15_cfg_FLD_llid13_dcrypt_mode                 REG_FLD(1, 9)
#define e_llid12_15_cfg_FLD_llid13_oamlpbk_en                  REG_FLD(1, 8)
#define e_llid12_15_cfg_FLD_llid12_dummy                       REG_FLD(4, 4)
#define e_llid12_15_cfg_FLD_llid12_txfec_en                    REG_FLD(1, 3)
#define e_llid12_15_cfg_FLD_llid12_dcrypt_en                   REG_FLD(1, 2)
#define e_llid12_15_cfg_FLD_llid12_dcrypt_mode                 REG_FLD(1, 1)
#define e_llid12_15_cfg_FLD_llid12_oam_lpbk_en                 REG_FLD(1, 0)

#define e_llid16_19_cfg_FLD_llid19_dummy                       REG_FLD(4, 28)
#define e_llid16_19_cfg_FLD_llid19_txfec_en                    REG_FLD(1, 27)
#define e_llid16_19_cfg_FLD_llid19_dcrypt_en                   REG_FLD(1, 26)
#define e_llid16_19_cfg_FLD_llid19_dcrypt_mode                 REG_FLD(1, 25)
#define e_llid16_19_cfg_FLD_llid19_oamlpbk_en                  REG_FLD(1, 24)
#define e_llid16_19_cfg_FLD_llid18_dummy                       REG_FLD(4, 20)
#define e_llid16_19_cfg_FLD_llid18_txfec_en                    REG_FLD(1, 19)
#define e_llid16_19_cfg_FLD_llid18_dcrypt_en                   REG_FLD(1, 18)
#define e_llid16_19_cfg_FLD_llid18_dcrypt_mode                 REG_FLD(1, 17)
#define e_llid16_19_cfg_FLD_llid18_oamlpbk_en                  REG_FLD(1, 16)
#define e_llid16_19_cfg_FLD_llid17_dummy                       REG_FLD(4, 12)
#define e_llid16_19_cfg_FLD_llid17_txfec_en                    REG_FLD(1, 11)
#define e_llid16_19_cfg_FLD_llid17_dcrypt_en                   REG_FLD(1, 10)
#define e_llid16_19_cfg_FLD_llid17_dcrypt_mode                 REG_FLD(1, 9)
#define e_llid16_19_cfg_FLD_llid17_oamlpbk_en                  REG_FLD(1, 8)
#define e_llid16_19_cfg_FLD_llid16_dummy                       REG_FLD(4, 4)
#define e_llid16_19_cfg_FLD_llid16_txfec_en                    REG_FLD(1, 3)
#define e_llid16_19_cfg_FLD_llid16_dcrypt_en                   REG_FLD(1, 2)
#define e_llid16_19_cfg_FLD_llid16_dcrypt_mode                 REG_FLD(1, 1)
#define e_llid16_19_cfg_FLD_llid16_oam_lpbk_en                 REG_FLD(1, 0)

#define e_llid20_23_cfg_FLD_llid23_dummy                       REG_FLD(4, 28)
#define e_llid20_23_cfg_FLD_llid23_txfec_en                    REG_FLD(1, 27)
#define e_llid20_23_cfg_FLD_llid23_dcrypt_en                   REG_FLD(1, 26)
#define e_llid20_23_cfg_FLD_llid23_dcrypt_mode                 REG_FLD(1, 25)
#define e_llid20_23_cfg_FLD_llid23_oamlpbk_en                  REG_FLD(1, 24)
#define e_llid20_23_cfg_FLD_llid22_dummy                       REG_FLD(4, 20)
#define e_llid20_23_cfg_FLD_llid22_txfec_en                    REG_FLD(1, 19)
#define e_llid20_23_cfg_FLD_llid22_dcrypt_en                   REG_FLD(1, 18)
#define e_llid20_23_cfg_FLD_llid22_dcrypt_mode                 REG_FLD(1, 17)
#define e_llid20_23_cfg_FLD_llid22_oamlpbk_en                  REG_FLD(1, 16)
#define e_llid20_23_cfg_FLD_llid21_dummy                       REG_FLD(4, 12)
#define e_llid20_23_cfg_FLD_llid21_txfec_en                    REG_FLD(1, 11)
#define e_llid20_23_cfg_FLD_llid21_dcrypt_en                   REG_FLD(1, 10)
#define e_llid20_23_cfg_FLD_llid21_dcrypt_mode                 REG_FLD(1, 9)
#define e_llid20_23_cfg_FLD_llid21_oamlpbk_en                  REG_FLD(1, 8)
#define e_llid20_23_cfg_FLD_llid20_dummy                       REG_FLD(4, 4)
#define e_llid20_23_cfg_FLD_llid20_txfec_en                    REG_FLD(1, 3)
#define e_llid20_23_cfg_FLD_llid20_dcrypt_en                   REG_FLD(1, 2)
#define e_llid20_23_cfg_FLD_llid20_dcrypt_mode                 REG_FLD(1, 1)
#define e_llid20_23_cfg_FLD_llid20_oam_lpbk_en                 REG_FLD(1, 0)

#define e_llid24_27_cfg_FLD_llid27_dummy                       REG_FLD(4, 28)
#define e_llid24_27_cfg_FLD_llid27_txfec_en                    REG_FLD(1, 27)
#define e_llid24_27_cfg_FLD_llid27_dcrypt_en                   REG_FLD(1, 26)
#define e_llid24_27_cfg_FLD_llid27_dcrypt_mode                 REG_FLD(1, 25)
#define e_llid24_27_cfg_FLD_llid27_oamlpbk_en                  REG_FLD(1, 24)
#define e_llid24_27_cfg_FLD_llid26_dummy                       REG_FLD(4, 20)
#define e_llid24_27_cfg_FLD_llid26_txfec_en                    REG_FLD(1, 19)
#define e_llid24_27_cfg_FLD_llid26_dcrypt_en                   REG_FLD(1, 18)
#define e_llid24_27_cfg_FLD_llid26_dcrypt_mode                 REG_FLD(1, 17)
#define e_llid24_27_cfg_FLD_llid26_oamlpbk_en                  REG_FLD(1, 16)
#define e_llid24_27_cfg_FLD_llid25_dummy                       REG_FLD(4, 12)
#define e_llid24_27_cfg_FLD_llid25_txfec_en                    REG_FLD(1, 11)
#define e_llid24_27_cfg_FLD_llid25_dcrypt_en                   REG_FLD(1, 10)
#define e_llid24_27_cfg_FLD_llid25_dcrypt_mode                 REG_FLD(1, 9)
#define e_llid24_27_cfg_FLD_llid25_oamlpbk_en                  REG_FLD(1, 8)
#define e_llid24_27_cfg_FLD_llid24_dummy                       REG_FLD(4, 4)
#define e_llid24_27_cfg_FLD_llid24_txfec_en                    REG_FLD(1, 3)
#define e_llid24_27_cfg_FLD_llid24_dcrypt_en                   REG_FLD(1, 2)
#define e_llid24_27_cfg_FLD_llid24_dcrypt_mode                 REG_FLD(1, 1)
#define e_llid24_27_cfg_FLD_llid24_oam_lpbk_en                 REG_FLD(1, 0)

#define e_llid28_31_cfg_FLD_llid31_dummy                       REG_FLD(4, 28)
#define e_llid28_31_cfg_FLD_llid31_txfec_en                    REG_FLD(1, 27)
#define e_llid28_31_cfg_FLD_llid31_dcrypt_en                   REG_FLD(1, 26)
#define e_llid28_31_cfg_FLD_llid31_dcrypt_mode                 REG_FLD(1, 25)
#define e_llid28_31_cfg_FLD_llid31_oamlpbk_en                  REG_FLD(1, 24)
#define e_llid28_31_cfg_FLD_llid30_dummy                       REG_FLD(4, 20)
#define e_llid28_31_cfg_FLD_llid30_txfec_en                    REG_FLD(1, 19)
#define e_llid28_31_cfg_FLD_llid30_dcrypt_en                   REG_FLD(1, 18)
#define e_llid28_31_cfg_FLD_llid30_dcrypt_mode                 REG_FLD(1, 17)
#define e_llid28_31_cfg_FLD_llid30_oamlpbk_en                  REG_FLD(1, 16)
#define e_llid28_31_cfg_FLD_llid29_dummy                       REG_FLD(4, 12)
#define e_llid28_31_cfg_FLD_llid29_txfec_en                    REG_FLD(1, 11)
#define e_llid28_31_cfg_FLD_llid29_dcrypt_en                   REG_FLD(1, 10)
#define e_llid28_31_cfg_FLD_llid29_dcrypt_mode                 REG_FLD(1, 9)
#define e_llid28_31_cfg_FLD_llid29_oamlpbk_en                  REG_FLD(1, 8)
#define e_llid28_31_cfg_FLD_llid28_dummy                       REG_FLD(4, 4)
#define e_llid28_31_cfg_FLD_llid28_txfec_en                    REG_FLD(1, 3)
#define e_llid28_31_cfg_FLD_llid28_dcrypt_en                   REG_FLD(1, 2)
#define e_llid28_31_cfg_FLD_llid28_dcrypt_mode                 REG_FLD(1, 1)
#define e_llid28_31_cfg_FLD_llid28_oam_lpbk_en                 REG_FLD(1, 0)

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

#define e_rpt_cfg2_FLD_llid15_rpt_cfg                          REG_FLD(2, 14)
#define e_rpt_cfg2_FLD_llid14_rpt_cfg                          REG_FLD(2, 12)
#define e_rpt_cfg2_FLD_llid13_rpt_cfg                          REG_FLD(2, 10)
#define e_rpt_cfg2_FLD_llid12_rpt_cfg                          REG_FLD(2, 8)
#define e_rpt_cfg2_FLD_llid11_rpt_cfg                          REG_FLD(2, 6)
#define e_rpt_cfg2_FLD_llid10_rpt_cfg                          REG_FLD(2, 4)
#define e_rpt_cfg2_FLD_llid9_rpt_cfg                           REG_FLD(2, 2)
#define e_rpt_cfg2_FLD_llid8_rpt_cfg                           REG_FLD(2, 0)

#define e_rpt_cfg3_FLD_llid31_rpt_cfg                          REG_FLD(2, 30)
#define e_rpt_cfg3_FLD_llid30_rpt_cfg                          REG_FLD(2, 28)
#define e_rpt_cfg3_FLD_llid29_rpt_cfg                          REG_FLD(2, 26)
#define e_rpt_cfg3_FLD_llid28_rpt_cfg                          REG_FLD(2, 24)
#define e_rpt_cfg3_FLD_llid27_rpt_cfg                          REG_FLD(2, 22)
#define e_rpt_cfg3_FLD_llid26_rpt_cfg                          REG_FLD(2, 20)
#define e_rpt_cfg3_FLD_llid25_rpt_cfg                          REG_FLD(2, 18)
#define e_rpt_cfg3_FLD_llid24_rpt_cfg                          REG_FLD(2, 16)
#define e_rpt_cfg3_FLD_llid23_rpt_cfg                          REG_FLD(2, 14)
#define e_rpt_cfg3_FLD_llid22_rpt_cfg                          REG_FLD(2, 12)
#define e_rpt_cfg3_FLD_llid21_rpt_cfg                          REG_FLD(2, 10)
#define e_rpt_cfg3_FLD_llid20_rpt_cfg                          REG_FLD(2, 8)
#define e_rpt_cfg3_FLD_llid19_rpt_cfg                          REG_FLD(2, 6)
#define e_rpt_cfg3_FLD_llid18_rpt_cfg                          REG_FLD(2, 4)
#define e_rpt_cfg3_FLD_llid17_rpt_cfg                          REG_FLD(2, 2)
#define e_rpt_cfg3_FLD_llid16_rpt_cfg                          REG_FLD(2, 0)

#define e_rpt_qthld_cfg2_FLD_rpt_llid_idx_4_3                  REG_FLD(2, 0)

#define e_rpt_bitmap_cfg_FLD_rpt_bmap_rwcmd                    REG_FLD(1, 31)
#define e_rpt_bitmap_cfg_FLD_rpt_bmap_rwcmd_done               REG_FLD(1, 30)
#define e_rpt_bitmap_cfg_FLD_rpt_bmap_llid_idx                 REG_FLD(5, 0)

#define e_rpt_bitmap_val_FLD_rpt_qsizeadj_frc                  REG_FLD(8, 16)
#define e_rpt_bitmap_val_FLD_rpt_bitmap_ctrl                   REG_FLD(8, 8)
#define e_rpt_bitmap_val_FLD_rpt_bitmap_set                    REG_FLD(8, 0)

#define e_glb_cfg_GET_sniffer_mode(reg32)                      REG_FLD_GET(e_glb_cfg_FLD_sniffer_mode, (reg32))
#define e_glb_cfg_GET_txoam_favor(reg32)                       REG_FLD_GET(e_glb_cfg_FLD_txoam_favor, (reg32))
#define e_glb_cfg_GET_burst_en_dly(reg32)                      REG_FLD_GET(e_glb_cfg_FLD_burst_en_dly, (reg32))
#define e_glb_cfg_GET_discv_burst_en(reg32)                    REG_FLD_GET(e_glb_cfg_FLD_discv_burst_en, (reg32))
#define e_glb_cfg_GET_mpcp_fwd(reg32)                          REG_FLD_GET(e_glb_cfg_FLD_mpcp_fwd, (reg32))
#define e_glb_cfg_GET_bcst_llid_m1_drop(reg32)                 REG_FLD_GET(e_glb_cfg_FLD_bcst_llid_m1_drop, (reg32))
#define e_glb_cfg_GET_bcst_llid_m0_drop(reg32)                 REG_FLD_GET(e_glb_cfg_FLD_bcst_llid_m0_drop, (reg32))
#define e_glb_cfg_GET_mcst_llid_drop(reg32)                    REG_FLD_GET(e_glb_cfg_FLD_mcst_llid_drop, (reg32))
#define e_glb_cfg_GET_all_unicast_llid_pkt_fwd(reg32)          REG_FLD_GET(e_glb_cfg_FLD_all_unicast_llid_pkt_fwd, (reg32))
#define e_glb_cfg_GET_fcs_err_fwd(reg32)                       REG_FLD_GET(e_glb_cfg_FLD_fcs_err_fwd, (reg32))
#define e_glb_cfg_GET_llid_crc8_err_fwd(reg32)                 REG_FLD_GET(e_glb_cfg_FLD_llid_crc8_err_fwd, (reg32))
#define e_glb_cfg_GET_rxmpi_stop(reg32)                        REG_FLD_GET(e_glb_cfg_FLD_rxmpi_stop, (reg32))
#define e_glb_cfg_GET_txmpi_stop(reg32)                        REG_FLD_GET(e_glb_cfg_FLD_txmpi_stop, (reg32))
#define e_glb_cfg_GET_phy_pwr_down(reg32)                      REG_FLD_GET(e_glb_cfg_FLD_phy_pwr_down, (reg32))
#define e_glb_cfg_GET_rx_nml_gate_fwd(reg32)                   REG_FLD_GET(e_glb_cfg_FLD_rx_nml_gate_fwd, (reg32))
#define e_glb_cfg_GET_rxmbi_stop(reg32)                        REG_FLD_GET(e_glb_cfg_FLD_rxmbi_stop, (reg32))
#define e_glb_cfg_GET_txmbi_stop(reg32)                        REG_FLD_GET(e_glb_cfg_FLD_txmbi_stop, (reg32))
#define e_glb_cfg_GET_chk_all_gnt_mode(reg32)                  REG_FLD_GET(e_glb_cfg_FLD_chk_all_gnt_mode, (reg32))
#define e_glb_cfg_GET_tx_default_rpt(reg32)                    REG_FLD_GET(e_glb_cfg_FLD_tx_default_rpt, (reg32))
#define e_glb_cfg_GET_epon_mac_sw_rst(reg32)                   REG_FLD_GET(e_glb_cfg_FLD_epon_mac_sw_rst, (reg32))
#define e_glb_cfg_GET_epon_oam_cal_in_eth(reg32)               REG_FLD_GET(e_glb_cfg_FLD_epon_oam_cal_in_eth, (reg32))
#define e_glb_cfg_GET_epon_mac_lpbk_en(reg32)                  REG_FLD_GET(e_glb_cfg_FLD_epon_mac_lpbk_en, (reg32))
#define e_glb_cfg_GET_rpt_txpri_ctrl(reg32)                    REG_FLD_GET(e_glb_cfg_FLD_rpt_txpri_ctrl, (reg32))

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

#define e_rpt_mpcp_timeout_llid_idx_GET_llid15_mpcp_tmo(reg32) REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid15_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid14_mpcp_tmo(reg32) REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid14_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid13_mpcp_tmo(reg32) REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid13_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid12_mpcp_tmo(reg32) REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid12_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid11_mpcp_tmo(reg32) REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid11_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid10_mpcp_tmo(reg32) REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid10_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid9_mpcp_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid9_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid8_mpcp_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid8_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid15_rpt_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid15_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid14_rpt_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid14_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid13_rpt_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid13_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid12_rpt_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid12_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid11_rpt_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid11_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid10_rpt_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid10_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid9_rpt_tmo(reg32)   REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid9_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid8_rpt_tmo(reg32)   REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid8_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid7_mpcp_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid7_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid6_mpcp_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid6_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid5_mpcp_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid5_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid4_mpcp_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid4_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid3_mpcp_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid3_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid2_mpcp_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid2_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid1_mpcp_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid1_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid0_mpcp_tmo(reg32)  REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid0_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid7_rpt_tmo(reg32)   REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid7_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid6_rpt_tmo(reg32)   REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid6_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid5_rpt_tmo(reg32)   REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid5_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid4_rpt_tmo(reg32)   REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid4_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid3_rpt_tmo(reg32)   REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid3_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid2_rpt_tmo(reg32)   REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid2_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid1_rpt_tmo(reg32)   REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid1_rpt_tmo, (reg32))
#define e_rpt_mpcp_timeout_llid_idx_GET_llid0_rpt_tmo(reg32)   REG_FLD_GET(e_rpt_mpcp_timeout_llid_idx_FLD_llid0_rpt_tmo, (reg32))

#define e_dyinggsp_cfg_GET_hw_dying_gasp_en(reg32)             REG_FLD_GET(e_dyinggsp_cfg_FLD_hw_dying_gasp_en, (reg32))
#define e_dyinggsp_cfg_GET_sw_init_dying_gasp(reg32)           REG_FLD_GET(e_dyinggsp_cfg_FLD_sw_init_dying_gasp, (reg32))
#define e_dyinggsp_cfg_GET_dygsp_num_of_times(reg32)           REG_FLD_GET(e_dyinggsp_cfg_FLD_dygsp_num_of_times, (reg32))

#define e_pending_gnt_num_GET_echoed_pending_gnt(reg32)        REG_FLD_GET(e_pending_gnt_num_FLD_echoed_pending_gnt, (reg32))
#define e_pending_gnt_num_GET_pending_gnt_num(reg32)           REG_FLD_GET(e_pending_gnt_num_FLD_pending_gnt_num, (reg32))

#define e_llid0_3_cfg_GET_llid3_dummy(reg32)                   REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_dummy, (reg32))
#define e_llid0_3_cfg_GET_llid3_txfec_en(reg32)                REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_txfec_en, (reg32))
#define e_llid0_3_cfg_GET_llid3_dcrypt_en(reg32)               REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_dcrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid3_dcrypt_mode(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_dcrypt_mode, (reg32))
#define e_llid0_3_cfg_GET_llid3_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid3_oam_lpbk_en, (reg32))
#define e_llid0_3_cfg_GET_llid2_dummy(reg32)                   REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_dummy, (reg32))
#define e_llid0_3_cfg_GET_llid2_txfec_en(reg32)                REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_txfec_en, (reg32))
#define e_llid0_3_cfg_GET_llid2_dcrypt_en(reg32)               REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_dcrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid2_dcrypt_mode(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_dcrypt_mode, (reg32))
#define e_llid0_3_cfg_GET_llid2_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid2_oam_lpbk_en, (reg32))
#define e_llid0_3_cfg_GET_llid1_dummy(reg32)                   REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_dummy, (reg32))
#define e_llid0_3_cfg_GET_llid1_txfec_en(reg32)                REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_txfec_en, (reg32))
#define e_llid0_3_cfg_GET_llid1_dcrypt_en(reg32)               REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_dcrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid1_dcrypt_mode(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_dcrypt_mode, (reg32))
#define e_llid0_3_cfg_GET_llid1_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid1_oam_lpbk_en, (reg32))
#define e_llid0_3_cfg_GET_llid0_dummy(reg32)                   REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_dummy, (reg32))
#define e_llid0_3_cfg_GET_llid0_txfec_en(reg32)                REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_txfec_en, (reg32))
#define e_llid0_3_cfg_GET_llid0_dcrypt_en(reg32)               REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_dcrypt_en, (reg32))
#define e_llid0_3_cfg_GET_llid0_dcrypt_mode(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_dcrypt_mode, (reg32))
#define e_llid0_3_cfg_GET_llid0_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid0_3_cfg_FLD_llid0_oam_lpbk_en, (reg32))

#define e_llid4_7_cfg_GET_llid7_dummy(reg32)                   REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_dummy, (reg32))
#define e_llid4_7_cfg_GET_llid7_txfec_en(reg32)                REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_txfec_en, (reg32))
#define e_llid4_7_cfg_GET_llid7_dcrypt_en(reg32)               REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_dcrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid7_dcrypt_mode(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_dcrypt_mode, (reg32))
#define e_llid4_7_cfg_GET_llid7_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid7_oam_lpbk_en, (reg32))
#define e_llid4_7_cfg_GET_llid6_dummy(reg32)                   REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_dummy, (reg32))
#define e_llid4_7_cfg_GET_llid6_txfec_en(reg32)                REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_txfec_en, (reg32))
#define e_llid4_7_cfg_GET_llid6_dcrypt_en(reg32)               REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_dcrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid6_dcrypt_mode(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_dcrypt_mode, (reg32))
#define e_llid4_7_cfg_GET_llid6_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid6_oam_lpbk_en, (reg32))
#define e_llid4_7_cfg_GET_llid5_dummy(reg32)                   REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_dummy, (reg32))
#define e_llid4_7_cfg_GET_llid5_txfec_en(reg32)                REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_txfec_en, (reg32))
#define e_llid4_7_cfg_GET_llid5_dcrypt_en(reg32)               REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_dcrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid5_dcrypt_mode(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_dcrypt_mode, (reg32))
#define e_llid4_7_cfg_GET_llid5_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid5_oam_lpbk_en, (reg32))
#define e_llid4_7_cfg_GET_llid4_dummy(reg32)                   REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_dummy, (reg32))
#define e_llid4_7_cfg_GET_llid4_txfec_en(reg32)                REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_txfec_en, (reg32))
#define e_llid4_7_cfg_GET_llid4_dcrypt_en(reg32)               REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_dcrypt_en, (reg32))
#define e_llid4_7_cfg_GET_llid4_dcrypt_mode(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_dcrypt_mode, (reg32))
#define e_llid4_7_cfg_GET_llid4_oam_lpbk_en(reg32)             REG_FLD_GET(e_llid4_7_cfg_FLD_llid4_oam_lpbk_en, (reg32))

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

#define e_mac_addr_cfg_GET_mac_addr_rwcmd(reg32)               REG_FLD_GET(e_mac_addr_cfg_FLD_mac_addr_rwcmd, (reg32))
#define e_mac_addr_cfg_GET_mac_addr_rwcmd_done(reg32)          REG_FLD_GET(e_mac_addr_cfg_FLD_mac_addr_rwcmd_done, (reg32))
#define e_mac_addr_cfg_GET_mac_addr_llid_indx(reg32)           REG_FLD_GET(e_mac_addr_cfg_FLD_mac_addr_llid_indx, (reg32))
#define e_mac_addr_cfg_GET_mac_addr_dw_idx(reg32)              REG_FLD_GET(e_mac_addr_cfg_FLD_mac_addr_dw_idx, (reg32))

#define e_mac_addr_value_GET_mac_addr_value(reg32)             REG_FLD_GET(e_mac_addr_value_FLD_mac_addr_value, (reg32))

#define e_security_key_cfg_GET_key_rwcmd(reg32)                REG_FLD_GET(e_security_key_cfg_FLD_key_rwcmd, (reg32))
#define e_security_key_cfg_GET_key_rwcmd_done(reg32)           REG_FLD_GET(e_security_key_cfg_FLD_key_rwcmd_done, (reg32))
#define e_security_key_cfg_GET_key_llid_index(reg32)           REG_FLD_GET(e_security_key_cfg_FLD_key_llid_index, (reg32))
#define e_security_key_cfg_GET_key_idx(reg32)                  REG_FLD_GET(e_security_key_cfg_FLD_key_idx, (reg32))
#define e_security_key_cfg_GET_key_dw_indx(reg32)              REG_FLD_GET(e_security_key_cfg_FLD_key_dw_indx, (reg32))

#define e_key_value_GET_key_value(reg32)                       REG_FLD_GET(e_key_value_FLD_key_value, (reg32))

#define e_rpt_cfg_GET_llid7_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid7_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid6_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid6_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid5_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid5_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid4_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid4_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid3_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid3_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid2_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid2_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid1_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid1_rpt_cfg, (reg32))
#define e_rpt_cfg_GET_llid0_rpt_cfg(reg32)                     REG_FLD_GET(e_rpt_cfg_FLD_llid0_rpt_cfg, (reg32))

#define e_rpt_qthld_cfg_GET_qthld_rwcmd(reg32)                 REG_FLD_GET(e_rpt_qthld_cfg_FLD_qthld_rwcmd, (reg32))
#define e_rpt_qthld_cfg_GET_qthld_rwcmd_done(reg32)            REG_FLD_GET(e_rpt_qthld_cfg_FLD_qthld_rwcmd_done, (reg32))
#define e_rpt_qthld_cfg_GET_qthld_value(reg32)                 REG_FLD_GET(e_rpt_qthld_cfg_FLD_qthld_value, (reg32))
#define e_rpt_qthld_cfg_GET_qthld_idx(reg32)                   REG_FLD_GET(e_rpt_qthld_cfg_FLD_qthld_idx, (reg32))
#define e_rpt_qthld_cfg_GET_rpt_llid_idx_2_0(reg32)            REG_FLD_GET(e_rpt_qthld_cfg_FLD_rpt_llid_idx_2_0, (reg32))
#define e_rpt_qthld_cfg_GET_queue_idx(reg32)                   REG_FLD_GET(e_rpt_qthld_cfg_FLD_queue_idx, (reg32))

#define e_local_time_GET_local_time(reg32)                     REG_FLD_GET(e_local_time_FLD_local_time, (reg32))

#define e_tod_sync_x_GET_tod_sync_x(reg32)                     REG_FLD_GET(e_tod_sync_x_FLD_tod_sync_x, (reg32))

#define e_tod_ltncy_GET_rl_rx_phydly_ofst(reg32)               REG_FLD_GET(e_tod_ltncy_FLD_rl_rx_phydly_ofst, (reg32))
#define e_tod_ltncy_GET_ingrs_latency(reg32)                   REG_FLD_GET(e_tod_ltncy_FLD_ingrs_latency, (reg32))
#define e_tod_ltncy_GET_egrs_latency(reg32)                    REG_FLD_GET(e_tod_ltncy_FLD_egrs_latency, (reg32))

#define e_new_tod_p2p_offset_sec_l32_GET_new_tod_p2p_offset_sec_l32(reg32) REG_FLD_GET(e_new_tod_p2p_offset_sec_l32_FLD_new_tod_p2p_offset_sec_l32, (reg32))

#define e_new_tod_p2p_tod_offset_nsec_GET_new_tod_nsec(reg32)  REG_FLD_GET(e_new_tod_p2p_tod_offset_nsec_FLD_new_tod_nsec, (reg32))

#define e_tod_p2p_tod_sec_l32_GET_tod_p2p_sec_l32(reg32)       REG_FLD_GET(e_tod_p2p_tod_sec_l32_FLD_tod_p2p_sec_l32, (reg32))

#define e_tod_p2p_tod_nsec_GET_tod_p2p_nsec(reg32)             REG_FLD_GET(e_tod_p2p_tod_nsec_FLD_tod_p2p_nsec, (reg32))

#define e_tod_period_GET_tod_period(reg32)                     REG_FLD_GET(e_tod_period_FLD_tod_period, (reg32))

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

#define e_slp_duration_i_GET_slp_pwd_mode_i(reg32)             REG_FLD_GET(e_slp_duration_i_FLD_slp_pwd_mode_i, (reg32))
#define e_slp_duration_i_GET_slp_flag_i(reg32)                 REG_FLD_GET(e_slp_duration_i_FLD_slp_flag_i, (reg32))
#define e_slp_duration_i_GET_slp_duration_i(reg32)             REG_FLD_GET(e_slp_duration_i_FLD_slp_duration_i, (reg32))

#define e_txfetch_cfg_GET_tx_fetch_leadtime(reg32)             REG_FLD_GET(e_txfetch_cfg_FLD_tx_fetch_leadtime, (reg32))
#define e_txfetch_cfg_GET_tx_dma_leadtime(reg32)               REG_FLD_GET(e_txfetch_cfg_FLD_tx_dma_leadtime, (reg32))

#define e_sync_time_GET_sync_time_updte(reg32)                 REG_FLD_GET(e_sync_time_FLD_sync_time_updte, (reg32))
#define e_sync_time_GET_sync_time(reg32)                       REG_FLD_GET(e_sync_time_FLD_sync_time, (reg32))

#define e_tx_cal_cnst_GET_dscvr_gnt_len(reg32)                 REG_FLD_GET(e_tx_cal_cnst_FLD_dscvr_gnt_len, (reg32))
#define e_tx_cal_cnst_GET_fec_tail_grd(reg32)                  REG_FLD_GET(e_tx_cal_cnst_FLD_fec_tail_grd, (reg32))
#define e_tx_cal_cnst_GET_tail_grd(reg32)                      REG_FLD_GET(e_tx_cal_cnst_FLD_tail_grd, (reg32))
#define e_tx_cal_cnst_GET_default_ovrhd(reg32)                 REG_FLD_GET(e_tx_cal_cnst_FLD_default_ovrhd, (reg32))

#define e_laser_onoff_time_GET_laser_off_time(reg32)           REG_FLD_GET(e_laser_onoff_time_FLD_laser_off_time, (reg32))
#define e_laser_onoff_time_GET_laser_on_time(reg32)            REG_FLD_GET(e_laser_onoff_time_FLD_laser_on_time, (reg32))

#define e_grd_thrshld_GET_guard_thrshld(reg32)                 REG_FLD_GET(e_grd_thrshld_FLD_guard_thrshld, (reg32))

#define e_mpcp_timeout_intvl_GET_mpcp_timeout_intvl(reg32)     REG_FLD_GET(e_mpcp_timeout_intvl_FLD_mpcp_timeout_intvl, (reg32))

#define e_rpt_timeout_intvl_GET_rpt_timeout_intvl(reg32)       REG_FLD_GET(e_rpt_timeout_intvl_FLD_rpt_timeout_intvl, (reg32))

#define e_max_future_gnt_time_GET_max_future_gnt_time(reg32)   REG_FLD_GET(e_max_future_gnt_time_FLD_max_future_gnt_time, (reg32))

#define e_min_proc_time_GET_min_proc_time(reg32)               REG_FLD_GET(e_min_proc_time_FLD_min_proc_time, (reg32))

#define e_trx_adjust_time1_GET_tx_stm_adj(reg32)               REG_FLD_GET(e_trx_adjust_time1_FLD_tx_stm_adj, (reg32))

#define e_trx_adjust_time2_GET_tx_len_adj(reg32)               REG_FLD_GET(e_trx_adjust_time2_FLD_tx_len_adj, (reg32))
#define e_trx_adjust_time2_GET_rx_tmstp_adj(reg32)             REG_FLD_GET(e_trx_adjust_time2_FLD_rx_tmstp_adj, (reg32))

#define e_dbg_prb_sel_GET_probe_dtgrp_sel(reg32)               REG_FLD_GET(e_dbg_prb_sel_FLD_probe_dtgrp_sel, (reg32))
#define e_dbg_prb_sel_GET_probe_bit0_sel(reg32)                REG_FLD_GET(e_dbg_prb_sel_FLD_probe_bit0_sel, (reg32))
#define e_dbg_prb_sel_GET_probe_sel(reg32)                     REG_FLD_GET(e_dbg_prb_sel_FLD_probe_sel, (reg32))

#define e_dbg_prb_h32_GET_probe_h32(reg32)                     REG_FLD_GET(e_dbg_prb_h32_FLD_probe_h32, (reg32))

#define e_dbg_prb_l32_GET_probe_l32(reg32)                     REG_FLD_GET(e_dbg_prb_l32_FLD_probe_l32, (reg32))

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

#define e_gnt_pending_stat_GET_max_gnt_pending_cnt(reg32)      REG_FLD_GET(e_gnt_pending_stat_FLD_max_gnt_pending_cnt, (reg32))
#define e_gnt_pending_stat_GET_cur_gnt_pending_cnt(reg32)      REG_FLD_GET(e_gnt_pending_stat_FLD_cur_gnt_pending_cnt, (reg32))

#define e_gnt_length_stat_GET_max_gnt_length(reg32)            REG_FLD_GET(e_gnt_length_stat_FLD_max_gnt_length, (reg32))
#define e_gnt_length_stat_GET_min_gnt_length(reg32)            REG_FLD_GET(e_gnt_length_stat_FLD_min_gnt_length, (reg32))

#define e_gnt_type_stat_GET_b2b_gnt_cnt(reg32)                 REG_FLD_GET(e_gnt_type_stat_FLD_b2b_gnt_cnt, (reg32))
#define e_gnt_type_stat_GET_hdn_gnt_cnt(reg32)                 REG_FLD_GET(e_gnt_type_stat_FLD_hdn_gnt_cnt, (reg32))

#define e_time_drft_stat_GET_cur_time_drift_ofst(reg32)        REG_FLD_GET(e_time_drft_stat_FLD_cur_time_drift_ofst, (reg32))
#define e_time_drft_stat_GET_max_time_drift_ofst(reg32)        REG_FLD_GET(e_time_drft_stat_FLD_max_time_drift_ofst, (reg32))
#define e_time_drft_stat_GET_cur_time_drift(reg32)             REG_FLD_GET(e_time_drft_stat_FLD_cur_time_drift, (reg32))
#define e_time_drft_stat_GET_max_time_drift(reg32)             REG_FLD_GET(e_time_drft_stat_FLD_max_time_drift, (reg32))

#define e_llid0_gnt_stat_GET_llid0_gnt_cnt(reg32)              REG_FLD_GET(e_llid0_gnt_stat_FLD_llid0_gnt_cnt, (reg32))

#define e_llid1_gnt_stat_GET_llid1_gnt_cnt(reg32)              REG_FLD_GET(e_llid1_gnt_stat_FLD_llid1_gnt_cnt, (reg32))

#define e_llid2_gnt_stat_GET_llid2_gnt_cnt(reg32)              REG_FLD_GET(e_llid2_gnt_stat_FLD_llid2_gnt_cnt, (reg32))

#define e_llid3_gnt_stat_GET_llid3_gnt_cnt(reg32)              REG_FLD_GET(e_llid3_gnt_stat_FLD_llid3_gnt_cnt, (reg32))

#define e_llid4_gnt_stat_GET_llid4_gnt_cnt(reg32)              REG_FLD_GET(e_llid4_gnt_stat_FLD_llid4_gnt_cnt, (reg32))

#define e_llid5_gnt_stat_GET_llid5_gnt_cnt(reg32)              REG_FLD_GET(e_llid5_gnt_stat_FLD_llid5_gnt_cnt, (reg32))

#define e_llid6_gnt_stat_GET_llid6_gnt_cnt(reg32)              REG_FLD_GET(e_llid6_gnt_stat_FLD_llid6_gnt_cnt, (reg32))

#define e_llid7_gnt_stat_GET_llid7_gnt_cnt(reg32)              REG_FLD_GET(e_llid7_gnt_stat_FLD_llid7_gnt_cnt, (reg32))

#define e_snf_mpcp_oam_ctl_GET_snf_mpcp_cap_en(reg32)          REG_FLD_GET(e_snf_mpcp_oam_ctl_FLD_snf_mpcp_cap_en, (reg32))
#define e_snf_mpcp_oam_ctl_GET_snf_oam_cap_en(reg32)           REG_FLD_GET(e_snf_mpcp_oam_ctl_FLD_snf_oam_cap_en, (reg32))
#define e_snf_mpcp_oam_ctl_GET_snf_mpcp_oam_cnt_set(reg32)     REG_FLD_GET(e_snf_mpcp_oam_ctl_FLD_snf_mpcp_oam_cnt_set, (reg32))

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

#define e_oam_kpalv_ctrl_GET_oam_kpalv_interval(reg32)         REG_FLD_GET(e_oam_kpalv_ctrl_FLD_oam_kpalv_interval, (reg32))
#define e_oam_kpalv_ctrl_GET_oam_kpalv_sw_trig(reg32)          REG_FLD_GET(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_trig, (reg32))
#define e_oam_kpalv_ctrl_GET_oam_kpalv_sw_cfg(reg32)           REG_FLD_GET(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_cfg, (reg32))
#define e_oam_kpalv_ctrl_GET_oam_kpalv_en(reg32)               REG_FLD_GET(e_oam_kpalv_ctrl_FLD_oam_kpalv_en, (reg32))

#define e_tod_1pps_ctrl_GET_tod_1pps_width_ctrl(reg32)         REG_FLD_GET(e_tod_1pps_ctrl_FLD_tod_1pps_width_ctrl, (reg32))

#define e_sniff_sp_tag_GET_sniffer_sp_tag(reg32)               REG_FLD_GET(e_sniff_sp_tag_FLD_sniffer_sp_tag, (reg32))

#define e_glb_cfg2_GET_llidnum_sel(reg32)                      REG_FLD_GET(e_glb_cfg2_FLD_llidnum_sel, (reg32))
#define e_glb_cfg2_GET_loctime_mtd(reg32)                      REG_FLD_GET(e_glb_cfg2_FLD_loctime_mtd, (reg32))
#define e_glb_cfg2_GET_rgreq_lsrtime_mask(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_rgreq_lsrtime_mask, (reg32))
#define e_glb_cfg2_GET_all_ucllid_mpcpsnf(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_all_ucllid_mpcpsnf, (reg32))
#define e_glb_cfg2_GET_u10g_txmode(reg32)                      REG_FLD_GET(e_glb_cfg2_FLD_u10g_txmode, (reg32))
#define e_glb_cfg2_GET_eth_cal_in_bytecnt(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_eth_cal_in_bytecnt, (reg32))
#define e_glb_cfg2_GET_snf_cal_in_bytecnt(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_snf_cal_in_bytecnt, (reg32))
#define e_glb_cfg2_GET_oam_cal_in_bytecnt(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_oam_cal_in_bytecnt, (reg32))
#define e_glb_cfg2_GET_dscvgate_filt(reg32)                    REG_FLD_GET(e_glb_cfg2_FLD_dscvgate_filt, (reg32))
#define e_glb_cfg2_GET_us_snf(reg32)                           REG_FLD_GET(e_glb_cfg2_FLD_us_snf, (reg32))
#define e_glb_cfg2_GET_llidinfo_snf(reg32)                     REG_FLD_GET(e_glb_cfg2_FLD_llidinfo_snf, (reg32))
#define e_glb_cfg2_GET_all_ucllid_ethsnf(reg32)                REG_FLD_GET(e_glb_cfg2_FLD_all_ucllid_ethsnf, (reg32))
#define e_glb_cfg2_GET_all_ucllid_snf(reg32)                   REG_FLD_GET(e_glb_cfg2_FLD_all_ucllid_snf, (reg32))
#define e_glb_cfg2_GET_rxuni_mcllid_en(reg32)                  REG_FLD_GET(e_glb_cfg2_FLD_rxuni_mcllid_en, (reg32))
#define e_glb_cfg2_GET_gntlen_stat_widscv(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_gntlen_stat_widscv, (reg32))
#define e_glb_cfg2_GET_tdrift_loctupd_dis(reg32)               REG_FLD_GET(e_glb_cfg2_FLD_tdrift_loctupd_dis, (reg32))
#define e_glb_cfg2_GET_snf_fcserr_fwd(reg32)                   REG_FLD_GET(e_glb_cfg2_FLD_snf_fcserr_fwd, (reg32))
#define e_glb_cfg2_GET_mpcp_fcserr_fwd(reg32)                  REG_FLD_GET(e_glb_cfg2_FLD_mpcp_fcserr_fwd, (reg32))
#define e_glb_cfg2_GET_dscvgate_infochk_dis(reg32)             REG_FLD_GET(e_glb_cfg2_FLD_dscvgate_infochk_dis, (reg32))

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
#define e_int_sts2_GET_rcv_dauc_dscvgate_int(reg32)            REG_FLD_GET(e_int_sts2_FLD_rcv_dauc_dscvgate_int, (reg32))

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
#define e_int_en2_GET_rcv_dauc_dscvgate_en(reg32)              REG_FLD_GET(e_int_en2_FLD_rcv_dauc_dscvgate_en, (reg32))

#define e_exc_sts_GET_rx_rgst_dergst_hit(reg32)                REG_FLD_GET(e_exc_sts_FLD_rx_rgst_dergst_hit, (reg32))
#define e_exc_sts_GET_rx_rgst_nack_hit(reg32)                  REG_FLD_GET(e_exc_sts_FLD_rx_rgst_nack_hit, (reg32))
#define e_exc_sts_GET_snf_pktcnt_err_hit(reg32)                REG_FLD_GET(e_exc_sts_FLD_snf_pktcnt_err_hit, (reg32))
#define e_exc_sts_GET_tx_gntst_nosog_hit(reg32)                REG_FLD_GET(e_exc_sts_FLD_tx_gntst_nosog_hit, (reg32))
#define e_exc_sts_GET_tx_gntst_fifound_hit(reg32)              REG_FLD_GET(e_exc_sts_FLD_tx_gntst_fifound_hit, (reg32))
#define e_exc_sts_GET_tx_gntph_fifound_hit(reg32)              REG_FLD_GET(e_exc_sts_FLD_tx_gntph_fifound_hit, (reg32))
#define e_exc_sts_GET_tx_gntph_exceed_hit(reg32)               REG_FLD_GET(e_exc_sts_FLD_tx_gntph_exceed_hit, (reg32))
#define e_exc_sts_GET_schgnt_stinv_hit(reg32)                  REG_FLD_GET(e_exc_sts_FLD_schgnt_stinv_hit, (reg32))
#define e_exc_sts_GET_schgnt_idxinv_hit(reg32)                 REG_FLD_GET(e_exc_sts_FLD_schgnt_idxinv_hit, (reg32))
#define e_exc_sts_GET_decrpt_vecm_acccol_hit(reg32)            REG_FLD_GET(e_exc_sts_FLD_decrpt_vecm_acccol_hit, (reg32))
#define e_exc_sts_GET_rcv_len_long_hit(reg32)                  REG_FLD_GET(e_exc_sts_FLD_rcv_len_long_hit, (reg32))
#define e_exc_sts_GET_rcv_len_short_hit(reg32)                 REG_FLD_GET(e_exc_sts_FLD_rcv_len_short_hit, (reg32))
#define e_exc_sts_GET_rcv_mpcp_crcerr_hit(reg32)               REG_FLD_GET(e_exc_sts_FLD_rcv_mpcp_crcerr_hit, (reg32))
#define e_exc_sts_GET_rcv_eth_crcerr_hit(reg32)                REG_FLD_GET(e_exc_sts_FLD_rcv_eth_crcerr_hit, (reg32))
#define e_exc_sts_GET_rcv_crc8err_hit(reg32)                   REG_FLD_GET(e_exc_sts_FLD_rcv_crc8err_hit, (reg32))
#define e_exc_sts_GET_rcv_nrlgate_llidbc_hit(reg32)            REG_FLD_GET(e_exc_sts_FLD_rcv_nrlgate_llidbc_hit, (reg32))
#define e_exc_sts_GET_rcv_dscvgate_filt_hit(reg32)             REG_FLD_GET(e_exc_sts_FLD_rcv_dscvgate_filt_hit, (reg32))
#define e_exc_sts_GET_rcv_dscvgate_infomis_hit(reg32)          REG_FLD_GET(e_exc_sts_FLD_rcv_dscvgate_infomis_hit, (reg32))

#define e_olt_dscvinfo_GET_olt_dscvinfo_match(reg32)           REG_FLD_GET(e_olt_dscvinfo_FLD_olt_dscvinfo_match, (reg32))
#define e_olt_dscvinfo_GET_olt_dscvinfo_mis(reg32)             REG_FLD_GET(e_olt_dscvinfo_FLD_olt_dscvinfo_mis, (reg32))

#define e_laser_onoff_time2_GET_lsroff_time_olt(reg32)         REG_FLD_GET(e_laser_onoff_time2_FLD_lsroff_time_olt, (reg32))
#define e_laser_onoff_time2_GET_lsron_time_olt(reg32)          REG_FLD_GET(e_laser_onoff_time2_FLD_lsron_time_olt, (reg32))
#define e_laser_onoff_time2_GET_sync_time_olt(reg32)           REG_FLD_GET(e_laser_onoff_time2_FLD_sync_time_olt, (reg32))

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

#define e_overhead_time_thr_GET_sync_time_maxen(reg32)         REG_FLD_GET(e_overhead_time_thr_FLD_sync_time_maxen, (reg32))
#define e_overhead_time_thr_GET_lsroff_time_maxen(reg32)       REG_FLD_GET(e_overhead_time_thr_FLD_lsroff_time_maxen, (reg32))
#define e_overhead_time_thr_GET_lsron_time_maxen(reg32)        REG_FLD_GET(e_overhead_time_thr_FLD_lsron_time_maxen, (reg32))
#define e_overhead_time_thr_GET_sync_time_maxval(reg32)        REG_FLD_GET(e_overhead_time_thr_FLD_sync_time_maxval, (reg32))
#define e_overhead_time_thr_GET_lsroff_time_maxval(reg32)      REG_FLD_GET(e_overhead_time_thr_FLD_lsroff_time_maxval, (reg32))
#define e_overhead_time_thr_GET_lsron_time_maxval(reg32)       REG_FLD_GET(e_overhead_time_thr_FLD_lsron_time_maxval, (reg32))

#define e_txcal_cnst2_GET_ipgalign_mtd(reg32)                  REG_FLD_GET(e_txcal_cnst2_FLD_ipgalign_mtd, (reg32))
#define e_txcal_cnst2_GET_u10g_dscv_gntlen(reg32)              REG_FLD_GET(e_txcal_cnst2_FLD_u10g_dscv_gntlen, (reg32))
#define e_txcal_cnst2_GET_u10g_tail_grd(reg32)                 REG_FLD_GET(e_txcal_cnst2_FLD_u10g_tail_grd, (reg32))

#define e_txcal_cnst3_GET_u1g_fecon_min_gntlen(reg32)          REG_FLD_GET(e_txcal_cnst3_FLD_u1g_fecon_min_gntlen, (reg32))
#define e_txcal_cnst3_GET_u10g_eoblen(reg32)                   REG_FLD_GET(e_txcal_cnst3_FLD_u10g_eoblen, (reg32))
#define e_txcal_cnst3_GET_u10g_min_gntlen(reg32)               REG_FLD_GET(e_txcal_cnst3_FLD_u10g_min_gntlen, (reg32))
#define e_txcal_cnst3_GET_u1g_fecoff_min_gntlen(reg32)         REG_FLD_GET(e_txcal_cnst3_FLD_u1g_fecoff_min_gntlen, (reg32))

#define e_txsch_cfg_GET_txsch_dmy0(reg32)                      REG_FLD_GET(e_txsch_cfg_FLD_txsch_dmy0, (reg32))
#define e_txsch_cfg_GET_txfifo_pad_hthr(reg32)                 REG_FLD_GET(e_txsch_cfg_FLD_txfifo_pad_hthr, (reg32))
#define e_txsch_cfg_GET_txsch_dmy1(reg32)                      REG_FLD_GET(e_txsch_cfg_FLD_txsch_dmy1, (reg32))
#define e_txsch_cfg_GET_txfifo_pad_lthr(reg32)                 REG_FLD_GET(e_txsch_cfg_FLD_txfifo_pad_lthr, (reg32))

#define e_rxfifo_thr_GET_rxfifo_mbithr(reg32)                  REG_FLD_GET(e_rxfifo_thr_FLD_rxfifo_mbithr, (reg32))

#define e_bcllid_cfg_GET_d10g_bcllid(reg32)                    REG_FLD_GET(e_bcllid_cfg_FLD_d10g_bcllid, (reg32))
#define e_bcllid_cfg_GET_d1g_bcllid(reg32)                     REG_FLD_GET(e_bcllid_cfg_FLD_d1g_bcllid, (reg32))

#define e_txfrm_cfg1_GET_tx_mpcp_addrl(reg32)                  REG_FLD_GET(e_txfrm_cfg1_FLD_tx_mpcp_addrl, (reg32))

#define e_txfrm_cfg2_GET_tx_mpcp_addrh(reg32)                  REG_FLD_GET(e_txfrm_cfg2_FLD_tx_mpcp_addrh, (reg32))
#define e_txfrm_cfg2_GET_tx_mpcp_etype(reg32)                  REG_FLD_GET(e_txfrm_cfg2_FLD_tx_mpcp_etype, (reg32))

#define e_txfrm_cfg3_GET_tx_rgreq_op(reg32)                    REG_FLD_GET(e_txfrm_cfg3_FLD_tx_rgreq_op, (reg32))
#define e_txfrm_cfg3_GET_tx_rgack_op(reg32)                    REG_FLD_GET(e_txfrm_cfg3_FLD_tx_rgack_op, (reg32))

#define e_u1g_rpt_qsizeadj1_GET_u1g_fecoff_rpt_q1_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj1_FLD_u1g_fecoff_rpt_q1_qsizeadj, (reg32))
#define e_u1g_rpt_qsizeadj1_GET_u1g_fecoff_rpt_q0_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj1_FLD_u1g_fecoff_rpt_q0_qsizeadj, (reg32))

#define e_u1g_rpt_qsizeadj2_GET_u1g_fecoff_rpt_q3_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj2_FLD_u1g_fecoff_rpt_q3_qsizeadj, (reg32))
#define e_u1g_rpt_qsizeadj2_GET_u1g_fecoff_rpt_q2_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj2_FLD_u1g_fecoff_rpt_q2_qsizeadj, (reg32))

#define e_u1g_rpt_qsizeadj3_GET_u1g_fecoff_rpt_q5_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj3_FLD_u1g_fecoff_rpt_q5_qsizeadj, (reg32))
#define e_u1g_rpt_qsizeadj3_GET_u1g_fecoff_rpt_q4_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj3_FLD_u1g_fecoff_rpt_q4_qsizeadj, (reg32))

#define e_u1g_rpt_qsizeadj4_GET_u1g_fecoff_rpt_q7_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj4_FLD_u1g_fecoff_rpt_q7_qsizeadj, (reg32))
#define e_u1g_rpt_qsizeadj4_GET_u1g_fecoff_rpt_q6_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj4_FLD_u1g_fecoff_rpt_q6_qsizeadj, (reg32))

#define e_u1g_rpt_qsizeadj5_GET_u1g_fecon_rpt_q1_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj5_FLD_u1g_fecon_rpt_q1_qsizeadj, (reg32))
#define e_u1g_rpt_qsizeadj5_GET_u1g_fecon_rpt_q0_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj5_FLD_u1g_fecon_rpt_q0_qsizeadj, (reg32))

#define e_u1g_rpt_qsizeadj6_GET_u1g_fecon_rpt_q3_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj6_FLD_u1g_fecon_rpt_q3_qsizeadj, (reg32))
#define e_u1g_rpt_qsizeadj6_GET_u1g_fecon_rpt_q2_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj6_FLD_u1g_fecon_rpt_q2_qsizeadj, (reg32))

#define e_u1g_rpt_qsizeadj7_GET_u1g_fecon_rpt_q5_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj7_FLD_u1g_fecon_rpt_q5_qsizeadj, (reg32))
#define e_u1g_rpt_qsizeadj7_GET_u1g_fecon_rpt_q4_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj7_FLD_u1g_fecon_rpt_q4_qsizeadj, (reg32))

#define e_u1g_rpt_qsizeadj8_GET_u1g_fecon_rpt_q7_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj8_FLD_u1g_fecon_rpt_q7_qsizeadj, (reg32))
#define e_u1g_rpt_qsizeadj8_GET_u1g_fecon_rpt_q6_qsizeadj(reg32) REG_FLD_GET(e_u1g_rpt_qsizeadj8_FLD_u1g_fecon_rpt_q6_qsizeadj, (reg32))

#define e_u10g_rpt_qsizeadj1_GET_u10g_rpt_q1_qsizeadj(reg32)   REG_FLD_GET(e_u10g_rpt_qsizeadj1_FLD_u10g_rpt_q1_qsizeadj, (reg32))
#define e_u10g_rpt_qsizeadj1_GET_u10g_rpt_q0_qsizeadj(reg32)   REG_FLD_GET(e_u10g_rpt_qsizeadj1_FLD_u10g_rpt_q0_qsizeadj, (reg32))

#define e_u10g_rpt_qsizeadj2_GET_u10g_rpt_q3_qsizeadj(reg32)   REG_FLD_GET(e_u10g_rpt_qsizeadj2_FLD_u10g_rpt_q3_qsizeadj, (reg32))
#define e_u10g_rpt_qsizeadj2_GET_u10g_rpt_q2_qsizeadj(reg32)   REG_FLD_GET(e_u10g_rpt_qsizeadj2_FLD_u10g_rpt_q2_qsizeadj, (reg32))

#define e_u10g_rpt_qsizeadj3_GET_u10g_rpt_q5_qsizeadj(reg32)   REG_FLD_GET(e_u10g_rpt_qsizeadj3_FLD_u10g_rpt_q5_qsizeadj, (reg32))
#define e_u10g_rpt_qsizeadj3_GET_u10g_rpt_q4_qsizeadj(reg32)   REG_FLD_GET(e_u10g_rpt_qsizeadj3_FLD_u10g_rpt_q4_qsizeadj, (reg32))

#define e_u10g_rpt_qsizeadj4_GET_u10g_rpt_q7_qsizeadj(reg32)   REG_FLD_GET(e_u10g_rpt_qsizeadj4_FLD_u10g_rpt_q7_qsizeadj, (reg32))
#define e_u10g_rpt_qsizeadj4_GET_u10g_rpt_q6_qsizeadj(reg32)   REG_FLD_GET(e_u10g_rpt_qsizeadj4_FLD_u10g_rpt_q6_qsizeadj, (reg32))

#define e_snf_dah_GET_snf_pkt_dah(reg32)                       REG_FLD_GET(e_snf_dah_FLD_snf_pkt_dah, (reg32))

#define e_snf_dal_GET_snf_pkt_dal(reg32)                       REG_FLD_GET(e_snf_dal_FLD_snf_pkt_dal, (reg32))
#define e_snf_dal_GET_snf_pkt_sah(reg32)                       REG_FLD_GET(e_snf_dal_FLD_snf_pkt_sah, (reg32))

#define e_snf_sal_GET_snf_pkt_sal(reg32)                       REG_FLD_GET(e_snf_sal_FLD_snf_pkt_sal, (reg32))

#define e_snf_etype_GET_snf_pkt_etype(reg32)                   REG_FLD_GET(e_snf_etype_FLD_snf_pkt_etype, (reg32))

#define e_rxuni_mcllid_cfg0_GET_rxuni_mcllid1(reg32)           REG_FLD_GET(e_rxuni_mcllid_cfg0_FLD_rxuni_mcllid1, (reg32))
#define e_rxuni_mcllid_cfg0_GET_rxuni_mcllid0(reg32)           REG_FLD_GET(e_rxuni_mcllid_cfg0_FLD_rxuni_mcllid0, (reg32))

#define e_rxuni_mcllid_cfg1_GET_rxuni_mcllid3(reg32)           REG_FLD_GET(e_rxuni_mcllid_cfg1_FLD_rxuni_mcllid3, (reg32))
#define e_rxuni_mcllid_cfg1_GET_rxuni_mcllid2(reg32)           REG_FLD_GET(e_rxuni_mcllid_cfg1_FLD_rxuni_mcllid2, (reg32))

#define e_rxuni_mcllid_cfg2_GET_rxuni_mcllid5(reg32)           REG_FLD_GET(e_rxuni_mcllid_cfg2_FLD_rxuni_mcllid5, (reg32))
#define e_rxuni_mcllid_cfg2_GET_rxuni_mcllid4(reg32)           REG_FLD_GET(e_rxuni_mcllid_cfg2_FLD_rxuni_mcllid4, (reg32))

#define e_rxuni_mcllid_cfg3_GET_rxuni_mcllid7(reg32)           REG_FLD_GET(e_rxuni_mcllid_cfg3_FLD_rxuni_mcllid7, (reg32))
#define e_rxuni_mcllid_cfg3_GET_rxuni_mcllid6(reg32)           REG_FLD_GET(e_rxuni_mcllid_cfg3_FLD_rxuni_mcllid6, (reg32))

#define e_rxuni_mcllid_cfg4_GET_rxuni_mcllid9(reg32)           REG_FLD_GET(e_rxuni_mcllid_cfg4_FLD_rxuni_mcllid9, (reg32))
#define e_rxuni_mcllid_cfg4_GET_rxuni_mcllid8(reg32)           REG_FLD_GET(e_rxuni_mcllid_cfg4_FLD_rxuni_mcllid8, (reg32))

#define e_rxuni_mcllid_cfg5_GET_rxuni_mcllid11(reg32)          REG_FLD_GET(e_rxuni_mcllid_cfg5_FLD_rxuni_mcllid11, (reg32))
#define e_rxuni_mcllid_cfg5_GET_rxuni_mcllid10(reg32)          REG_FLD_GET(e_rxuni_mcllid_cfg5_FLD_rxuni_mcllid10, (reg32))

#define e_rxuni_mcllid_cfg6_GET_rxuni_mcllid13(reg32)          REG_FLD_GET(e_rxuni_mcllid_cfg6_FLD_rxuni_mcllid13, (reg32))
#define e_rxuni_mcllid_cfg6_GET_rxuni_mcllid12(reg32)          REG_FLD_GET(e_rxuni_mcllid_cfg6_FLD_rxuni_mcllid12, (reg32))

#define e_rxuni_mcllid_cfg7_GET_rxuni_mcllid15(reg32)          REG_FLD_GET(e_rxuni_mcllid_cfg7_FLD_rxuni_mcllid15, (reg32))
#define e_rxuni_mcllid_cfg7_GET_rxuni_mcllid14(reg32)          REG_FLD_GET(e_rxuni_mcllid_cfg7_FLD_rxuni_mcllid14, (reg32))

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

#define e_glue_cfg_GET_txmpi_fifound_pktgate_en(reg32)         REG_FLD_GET(e_glue_cfg_FLD_txmpi_fifound_pktgate_en, (reg32))
#define e_glue_cfg_GET_txmpi_fifound_thr(reg32)                REG_FLD_GET(e_glue_cfg_FLD_txmpi_fifound_thr, (reg32))

#define e_gntreq_tmout_GET_gntreq_grdcyc(reg32)                REG_FLD_GET(e_gntreq_tmout_FLD_gntreq_grdcyc, (reg32))

#define e_glb_sts_GET_xepon_type(reg32)                        REG_FLD_GET(e_glb_sts_FLD_xepon_type, (reg32))

#define e_cnt_clr_GET_glb_cntclr(reg32)                        REG_FLD_GET(e_cnt_clr_FLD_glb_cntclr, (reg32))

#define e_rxadv_cnt_GET_rx_infomis_dscvgate_cnt(reg32)         REG_FLD_GET(e_rxadv_cnt_FLD_rx_infomis_dscvgate_cnt, (reg32))
#define e_rxadv_cnt_GET_rx_dauc_dscvgate_cnt(reg32)            REG_FLD_GET(e_rxadv_cnt_FLD_rx_dauc_dscvgate_cnt, (reg32))
#define e_rxadv_cnt_GET_rx_filt_dscvgate_cnt(reg32)            REG_FLD_GET(e_rxadv_cnt_FLD_rx_filt_dscvgate_cnt, (reg32))

#define e_tx_dygasp_cnt_GET_txmbi_dygasp_cnt(reg32)            REG_FLD_GET(e_tx_dygasp_cnt_FLD_txmbi_dygasp_cnt, (reg32))
#define e_tx_dygasp_cnt_GET_txmpi_dygasp_cnt(reg32)            REG_FLD_GET(e_tx_dygasp_cnt_FLD_txmpi_dygasp_cnt, (reg32))

#define e_tx_rpt_cnt_GET_txmbi_rpt_cnt(reg32)                  REG_FLD_GET(e_tx_rpt_cnt_FLD_txmbi_rpt_cnt, (reg32))
#define e_tx_rpt_cnt_GET_txmpi_rpt_cnt(reg32)                  REG_FLD_GET(e_tx_rpt_cnt_FLD_txmpi_rpt_cnt, (reg32))

#define e_txmpi_mpcp_cnt_GET_txmpi_oam_cnt(reg32)              REG_FLD_GET(e_txmpi_mpcp_cnt_FLD_txmpi_oam_cnt, (reg32))
#define e_txmpi_mpcp_cnt_GET_txmpi_rgreq_cnt(reg32)            REG_FLD_GET(e_txmpi_mpcp_cnt_FLD_txmpi_rgreq_cnt, (reg32))
#define e_txmpi_mpcp_cnt_GET_txmpi_rgack_cnt(reg32)            REG_FLD_GET(e_txmpi_mpcp_cnt_FLD_txmpi_rgack_cnt, (reg32))

#define e_rxmpi_uc_cnt_GET_rxmpi_uceth_cnt(reg32)              REG_FLD_GET(e_rxmpi_uc_cnt_FLD_rxmpi_uceth_cnt, (reg32))

#define e_rxmpi_bc_cnt_GET_rxmpi_bceth_cnt(reg32)              REG_FLD_GET(e_rxmpi_bc_cnt_FLD_rxmpi_bceth_cnt, (reg32))

#define e_rxmpi_mc_cnt_GET_rxmpi_mceth_cnt(reg32)              REG_FLD_GET(e_rxmpi_mc_cnt_FLD_rxmpi_mceth_cnt, (reg32))

#define e_rxmpi_oam_cnt_GET_rxmpi_oam_cnt(reg32)               REG_FLD_GET(e_rxmpi_oam_cnt_FLD_rxmpi_oam_cnt, (reg32))

#define e_rxmpi_mpcp_cnt_GET_rxmpi_mpcp_cnt(reg32)             REG_FLD_GET(e_rxmpi_mpcp_cnt_FLD_rxmpi_mpcp_cnt, (reg32))

#define e_rxmpi_gate_cnt_GET_rxmpi_gate_cnt(reg32)             REG_FLD_GET(e_rxmpi_gate_cnt_FLD_rxmpi_gate_cnt, (reg32))

#define e_rxmpi_nrlgate_cnt_GET_rxmpi_nrlgate_cnt(reg32)       REG_FLD_GET(e_rxmpi_nrlgate_cnt_FLD_rxmpi_nrlgate_cnt, (reg32))

#define e_rxmpi_crc8err_cnt_GET_rxmpi_crceth_cnt(reg32)        REG_FLD_GET(e_rxmpi_crc8err_cnt_FLD_rxmpi_crceth_cnt, (reg32))
#define e_rxmpi_crc8err_cnt_GET_rxmpi_crc8err_cnt(reg32)       REG_FLD_GET(e_rxmpi_crc8err_cnt_FLD_rxmpi_crc8err_cnt, (reg32))

#define e_rxmpi_drop_cnt_GET_rxmpi_eofdrop_cnt(reg32)          REG_FLD_GET(e_rxmpi_drop_cnt_FLD_rxmpi_eofdrop_cnt, (reg32))

#define e_rxmpi_churn_cnt_GET_rxmpi_abchurn_cnt(reg32)         REG_FLD_GET(e_rxmpi_churn_cnt_FLD_rxmpi_abchurn_cnt, (reg32))
#define e_rxmpi_churn_cnt_GET_rxmpi_churn_cnt(reg32)           REG_FLD_GET(e_rxmpi_churn_cnt_FLD_rxmpi_churn_cnt, (reg32))

#define e_rxmbi_snf_cnt_GET_rxmbi_snf_cnt(reg32)               REG_FLD_GET(e_rxmbi_snf_cnt_FLD_rxmbi_snf_cnt, (reg32))

#define e_rxmbi_drop_cnt_GET_rxmbi_sofdrop_cnt(reg32)          REG_FLD_GET(e_rxmbi_drop_cnt_FLD_rxmbi_sofdrop_cnt, (reg32))
#define e_rxmbi_drop_cnt_GET_rxmbi_snfdrop_cnt(reg32)          REG_FLD_GET(e_rxmbi_drop_cnt_FLD_rxmbi_snfdrop_cnt, (reg32))

#define e_rxmbi_drop_cnt2_GET_rxmbi_crcerr_cnt(reg32)          REG_FLD_GET(e_rxmbi_drop_cnt2_FLD_rxmbi_crcerr_cnt, (reg32))
#define e_rxmbi_drop_cnt2_GET_rxmbi_enddrop_cnt(reg32)         REG_FLD_GET(e_rxmbi_drop_cnt2_FLD_rxmbi_enddrop_cnt, (reg32))

#define e_rxmbi_bytecnt_h_GET_rxmbi_bytecnt_h(reg32)           REG_FLD_GET(e_rxmbi_bytecnt_h_FLD_rxmbi_bytecnt_h, (reg32))

#define e_rxmbi_bytecnt_l_GET_rxmbi_bytecnt_l(reg32)           REG_FLD_GET(e_rxmbi_bytecnt_l_FLD_rxmbi_bytecnt_l, (reg32))

#define e_txmbi_uceth_cnt_GET_txmbi_uceth_cnt(reg32)           REG_FLD_GET(e_txmbi_uceth_cnt_FLD_txmbi_uceth_cnt, (reg32))

#define e_txmbi_mceth_cnt_GET_txmbi_bceth_cnt(reg32)           REG_FLD_GET(e_txmbi_mceth_cnt_FLD_txmbi_bceth_cnt, (reg32))
#define e_txmbi_mceth_cnt_GET_txmbi_mceth_cnt(reg32)           REG_FLD_GET(e_txmbi_mceth_cnt_FLD_txmbi_mceth_cnt, (reg32))

#define e_txmbi_err_cnt_GET_txmbi_err_cnt(reg32)               REG_FLD_GET(e_txmbi_err_cnt_FLD_txmbi_err_cnt, (reg32))

#define e_tx_timedrift_stat_GET_tx_cur_time_drift_ofst(reg32)  REG_FLD_GET(e_tx_timedrift_stat_FLD_tx_cur_time_drift_ofst, (reg32))
#define e_tx_timedrift_stat_GET_tx_max_time_drift_ofst(reg32)  REG_FLD_GET(e_tx_timedrift_stat_FLD_tx_max_time_drift_ofst, (reg32))
#define e_tx_timedrift_stat_GET_tx_cur_time_drift(reg32)       REG_FLD_GET(e_tx_timedrift_stat_FLD_tx_cur_time_drift, (reg32))
#define e_tx_timedrift_stat_GET_tx_max_time_drift(reg32)       REG_FLD_GET(e_tx_timedrift_stat_FLD_tx_max_time_drift, (reg32))

#define e_rxmpi_churn_cnt2_GET_rxmpi_churn_mpcpcnt(reg32)      REG_FLD_GET(e_rxmpi_churn_cnt2_FLD_rxmpi_churn_mpcpcnt, (reg32))
#define e_rxmpi_churn_cnt2_GET_rxmpi_churn_ethcnt(reg32)       REG_FLD_GET(e_rxmpi_churn_cnt2_FLD_rxmpi_churn_ethcnt, (reg32))

#define e_rxmpi_churn_cnt3_GET_rxmpi_churn_keyidx(reg32)       REG_FLD_GET(e_rxmpi_churn_cnt3_FLD_rxmpi_churn_keyidx, (reg32))
#define e_rxmpi_churn_cnt3_GET_rxmpi_churn_llididx(reg32)      REG_FLD_GET(e_rxmpi_churn_cnt3_FLD_rxmpi_churn_llididx, (reg32))
#define e_rxmpi_churn_cnt3_GET_rxmpi_churncrc_keyidx(reg32)    REG_FLD_GET(e_rxmpi_churn_cnt3_FLD_rxmpi_churncrc_keyidx, (reg32))
#define e_rxmpi_churn_cnt3_GET_rxmpi_churncrc_llididx(reg32)   REG_FLD_GET(e_rxmpi_churn_cnt3_FLD_rxmpi_churncrc_llididx, (reg32))
#define e_rxmpi_churn_cnt3_GET_rxmpi_churn_crccnt(reg32)       REG_FLD_GET(e_rxmpi_churn_cnt3_FLD_rxmpi_churn_crccnt, (reg32))

#define e_txmpi_uceth_cnt_GET_txmpi_uceth_cnt(reg32)           REG_FLD_GET(e_txmpi_uceth_cnt_FLD_txmpi_uceth_cnt, (reg32))

#define e_txmpi_mceth_cnt_GET_txmpi_bceth_cnt(reg32)           REG_FLD_GET(e_txmpi_mceth_cnt_FLD_txmpi_bceth_cnt, (reg32))
#define e_txmpi_mceth_cnt_GET_txmpi_mceth_cnt(reg32)           REG_FLD_GET(e_txmpi_mceth_cnt_FLD_txmpi_mceth_cnt, (reg32))

#define e_llid8_9_gnt_stat_GET_llid9_gnt_cnt(reg32)            REG_FLD_GET(e_llid8_9_gnt_stat_FLD_llid9_gnt_cnt, (reg32))
#define e_llid8_9_gnt_stat_GET_llid8_gnt_cnt(reg32)            REG_FLD_GET(e_llid8_9_gnt_stat_FLD_llid8_gnt_cnt, (reg32))

#define e_llid10_11_gnt_stat_GET_llid11_gnt_cnt(reg32)         REG_FLD_GET(e_llid10_11_gnt_stat_FLD_llid11_gnt_cnt, (reg32))
#define e_llid10_11_gnt_stat_GET_llid10_gnt_cnt(reg32)         REG_FLD_GET(e_llid10_11_gnt_stat_FLD_llid10_gnt_cnt, (reg32))

#define e_llid12_13_gnt_stat_GET_llid13_gnt_cnt(reg32)         REG_FLD_GET(e_llid12_13_gnt_stat_FLD_llid13_gnt_cnt, (reg32))
#define e_llid12_13_gnt_stat_GET_llid12_gnt_cnt(reg32)         REG_FLD_GET(e_llid12_13_gnt_stat_FLD_llid12_gnt_cnt, (reg32))

#define e_llid14_15_gnt_stat_GET_llid15_gnt_cnt(reg32)         REG_FLD_GET(e_llid14_15_gnt_stat_FLD_llid15_gnt_cnt, (reg32))
#define e_llid14_15_gnt_stat_GET_llid14_gnt_cnt(reg32)         REG_FLD_GET(e_llid14_15_gnt_stat_FLD_llid14_gnt_cnt, (reg32))

#define e_llid16_17_gnt_stat_GET_llid17_gnt_cnt(reg32)         REG_FLD_GET(e_llid16_17_gnt_stat_FLD_llid17_gnt_cnt, (reg32))
#define e_llid16_17_gnt_stat_GET_llid16_gnt_cnt(reg32)         REG_FLD_GET(e_llid16_17_gnt_stat_FLD_llid16_gnt_cnt, (reg32))

#define e_llid18_19_gnt_stat_GET_llid19_gnt_cnt(reg32)         REG_FLD_GET(e_llid18_19_gnt_stat_FLD_llid19_gnt_cnt, (reg32))
#define e_llid18_19_gnt_stat_GET_llid18_gnt_cnt(reg32)         REG_FLD_GET(e_llid18_19_gnt_stat_FLD_llid18_gnt_cnt, (reg32))

#define e_llid20_21_gnt_stat_GET_llid21_gnt_cnt(reg32)         REG_FLD_GET(e_llid20_21_gnt_stat_FLD_llid21_gnt_cnt, (reg32))
#define e_llid20_21_gnt_stat_GET_llid20_gnt_cnt(reg32)         REG_FLD_GET(e_llid20_21_gnt_stat_FLD_llid20_gnt_cnt, (reg32))

#define e_llid22_23_gnt_stat_GET_llid23_gnt_cnt(reg32)         REG_FLD_GET(e_llid22_23_gnt_stat_FLD_llid23_gnt_cnt, (reg32))
#define e_llid22_23_gnt_stat_GET_llid22_gnt_cnt(reg32)         REG_FLD_GET(e_llid22_23_gnt_stat_FLD_llid22_gnt_cnt, (reg32))

#define e_llid24_25_gnt_stat_GET_llid25_gnt_cnt(reg32)         REG_FLD_GET(e_llid24_25_gnt_stat_FLD_llid25_gnt_cnt, (reg32))
#define e_llid24_25_gnt_stat_GET_llid24_gnt_cnt(reg32)         REG_FLD_GET(e_llid24_25_gnt_stat_FLD_llid24_gnt_cnt, (reg32))

#define e_llid26_27_gnt_stat_GET_llid27_gnt_cnt(reg32)         REG_FLD_GET(e_llid26_27_gnt_stat_FLD_llid27_gnt_cnt, (reg32))
#define e_llid26_27_gnt_stat_GET_llid26_gnt_cnt(reg32)         REG_FLD_GET(e_llid26_27_gnt_stat_FLD_llid26_gnt_cnt, (reg32))

#define e_llid28_29_gnt_stat_GET_llid29_gnt_cnt(reg32)         REG_FLD_GET(e_llid28_29_gnt_stat_FLD_llid29_gnt_cnt, (reg32))
#define e_llid28_29_gnt_stat_GET_llid28_gnt_cnt(reg32)         REG_FLD_GET(e_llid28_29_gnt_stat_FLD_llid28_gnt_cnt, (reg32))

#define e_llid30_31_gnt_stat_GET_llid31_gnt_cnt(reg32)         REG_FLD_GET(e_llid30_31_gnt_stat_FLD_llid31_gnt_cnt, (reg32))
#define e_llid30_31_gnt_stat_GET_llid30_gnt_cnt(reg32)         REG_FLD_GET(e_llid30_31_gnt_stat_FLD_llid30_gnt_cnt, (reg32))

#define e_rxfifo_depth_stat_GET_max_rxfifo_depth(reg32)        REG_FLD_GET(e_rxfifo_depth_stat_FLD_max_rxfifo_depth, (reg32))

#define e_rdmdly_stat_GET_rdmdly(reg32)                        REG_FLD_GET(e_rdmdly_stat_FLD_rdmdly, (reg32))

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

#define e_rpt_mpcp_timout2_GET_llid31_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid31_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid30_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid30_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid29_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid29_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid28_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid28_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid27_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid27_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid26_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid26_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid25_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid25_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid24_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid24_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid23_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid23_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid22_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid22_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid21_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid21_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid20_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid20_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid19_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid19_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid18_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid18_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid17_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid17_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid16_mpcp_tmo(reg32)          REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid16_mpcp_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid31_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid31_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid30_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid30_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid29_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid29_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid28_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid28_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid27_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid27_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid26_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid26_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid25_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid25_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid24_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid24_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid23_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid23_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid22_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid22_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid21_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid21_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid20_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid20_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid19_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid19_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid18_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid18_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid17_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid17_rpt_tmo, (reg32))
#define e_rpt_mpcp_timout2_GET_llid16_rpt_tmo(reg32)           REG_FLD_GET(e_rpt_mpcp_timout2_FLD_llid16_rpt_tmo, (reg32))

#define e_llid8_11_cfg_GET_llid11_dummy(reg32)                 REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_dummy, (reg32))
#define e_llid8_11_cfg_GET_llid11_txfec_en(reg32)              REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_txfec_en, (reg32))
#define e_llid8_11_cfg_GET_llid11_dcrypt_en(reg32)             REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_dcrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid11_dcrypt_mode(reg32)           REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_dcrypt_mode, (reg32))
#define e_llid8_11_cfg_GET_llid11_oamlpbk_en(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid11_oamlpbk_en, (reg32))
#define e_llid8_11_cfg_GET_llid10_dummy(reg32)                 REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_dummy, (reg32))
#define e_llid8_11_cfg_GET_llid10_txfec_en(reg32)              REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_txfec_en, (reg32))
#define e_llid8_11_cfg_GET_llid10_dcrypt_en(reg32)             REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_dcrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid10_dcrypt_mode(reg32)           REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_dcrypt_mode, (reg32))
#define e_llid8_11_cfg_GET_llid10_oamlpbk_en(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid10_oamlpbk_en, (reg32))
#define e_llid8_11_cfg_GET_llid9_dummy(reg32)                  REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_dummy, (reg32))
#define e_llid8_11_cfg_GET_llid9_txfec_en(reg32)               REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_txfec_en, (reg32))
#define e_llid8_11_cfg_GET_llid9_dcrypt_en(reg32)              REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_dcrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid9_dcrypt_mode(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_dcrypt_mode, (reg32))
#define e_llid8_11_cfg_GET_llid9_oamlpbk_en(reg32)             REG_FLD_GET(e_llid8_11_cfg_FLD_llid9_oamlpbk_en, (reg32))
#define e_llid8_11_cfg_GET_llid8_dummy(reg32)                  REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_dummy, (reg32))
#define e_llid8_11_cfg_GET_llid8_txfec_en(reg32)               REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_txfec_en, (reg32))
#define e_llid8_11_cfg_GET_llid8_dcrypt_en(reg32)              REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_dcrypt_en, (reg32))
#define e_llid8_11_cfg_GET_llid8_dcrypt_mode(reg32)            REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_dcrypt_mode, (reg32))
#define e_llid8_11_cfg_GET_llid8_oamlpbk_en(reg32)             REG_FLD_GET(e_llid8_11_cfg_FLD_llid8_oamlpbk_en, (reg32))

#define e_llid12_15_cfg_GET_llid15_dummy(reg32)                REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_dummy, (reg32))
#define e_llid12_15_cfg_GET_llid15_txfec_en(reg32)             REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_txfec_en, (reg32))
#define e_llid12_15_cfg_GET_llid15_dcrypt_en(reg32)            REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_dcrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid15_dcrypt_mode(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_dcrypt_mode, (reg32))
#define e_llid12_15_cfg_GET_llid15_oamlpbk_en(reg32)           REG_FLD_GET(e_llid12_15_cfg_FLD_llid15_oamlpbk_en, (reg32))
#define e_llid12_15_cfg_GET_llid14_dummy(reg32)                REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_dummy, (reg32))
#define e_llid12_15_cfg_GET_llid14_txfec_en(reg32)             REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_txfec_en, (reg32))
#define e_llid12_15_cfg_GET_llid14_dcrypt_en(reg32)            REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_dcrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid14_dcrypt_mode(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_dcrypt_mode, (reg32))
#define e_llid12_15_cfg_GET_llid14_oamlpbk_en(reg32)           REG_FLD_GET(e_llid12_15_cfg_FLD_llid14_oamlpbk_en, (reg32))
#define e_llid12_15_cfg_GET_llid13_dummy(reg32)                REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_dummy, (reg32))
#define e_llid12_15_cfg_GET_llid13_txfec_en(reg32)             REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_txfec_en, (reg32))
#define e_llid12_15_cfg_GET_llid13_dcrypt_en(reg32)            REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_dcrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid13_dcrypt_mode(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_dcrypt_mode, (reg32))
#define e_llid12_15_cfg_GET_llid13_oamlpbk_en(reg32)           REG_FLD_GET(e_llid12_15_cfg_FLD_llid13_oamlpbk_en, (reg32))
#define e_llid12_15_cfg_GET_llid12_dummy(reg32)                REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_dummy, (reg32))
#define e_llid12_15_cfg_GET_llid12_txfec_en(reg32)             REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_txfec_en, (reg32))
#define e_llid12_15_cfg_GET_llid12_dcrypt_en(reg32)            REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_dcrypt_en, (reg32))
#define e_llid12_15_cfg_GET_llid12_dcrypt_mode(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_dcrypt_mode, (reg32))
#define e_llid12_15_cfg_GET_llid12_oam_lpbk_en(reg32)          REG_FLD_GET(e_llid12_15_cfg_FLD_llid12_oam_lpbk_en, (reg32))

#define e_llid16_19_cfg_GET_llid19_dummy(reg32)                REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_dummy, (reg32))
#define e_llid16_19_cfg_GET_llid19_txfec_en(reg32)             REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_txfec_en, (reg32))
#define e_llid16_19_cfg_GET_llid19_dcrypt_en(reg32)            REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_dcrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid19_dcrypt_mode(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_dcrypt_mode, (reg32))
#define e_llid16_19_cfg_GET_llid19_oamlpbk_en(reg32)           REG_FLD_GET(e_llid16_19_cfg_FLD_llid19_oamlpbk_en, (reg32))
#define e_llid16_19_cfg_GET_llid18_dummy(reg32)                REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_dummy, (reg32))
#define e_llid16_19_cfg_GET_llid18_txfec_en(reg32)             REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_txfec_en, (reg32))
#define e_llid16_19_cfg_GET_llid18_dcrypt_en(reg32)            REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_dcrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid18_dcrypt_mode(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_dcrypt_mode, (reg32))
#define e_llid16_19_cfg_GET_llid18_oamlpbk_en(reg32)           REG_FLD_GET(e_llid16_19_cfg_FLD_llid18_oamlpbk_en, (reg32))
#define e_llid16_19_cfg_GET_llid17_dummy(reg32)                REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_dummy, (reg32))
#define e_llid16_19_cfg_GET_llid17_txfec_en(reg32)             REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_txfec_en, (reg32))
#define e_llid16_19_cfg_GET_llid17_dcrypt_en(reg32)            REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_dcrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid17_dcrypt_mode(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_dcrypt_mode, (reg32))
#define e_llid16_19_cfg_GET_llid17_oamlpbk_en(reg32)           REG_FLD_GET(e_llid16_19_cfg_FLD_llid17_oamlpbk_en, (reg32))
#define e_llid16_19_cfg_GET_llid16_dummy(reg32)                REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_dummy, (reg32))
#define e_llid16_19_cfg_GET_llid16_txfec_en(reg32)             REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_txfec_en, (reg32))
#define e_llid16_19_cfg_GET_llid16_dcrypt_en(reg32)            REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_dcrypt_en, (reg32))
#define e_llid16_19_cfg_GET_llid16_dcrypt_mode(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_dcrypt_mode, (reg32))
#define e_llid16_19_cfg_GET_llid16_oam_lpbk_en(reg32)          REG_FLD_GET(e_llid16_19_cfg_FLD_llid16_oam_lpbk_en, (reg32))

#define e_llid20_23_cfg_GET_llid23_dummy(reg32)                REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_dummy, (reg32))
#define e_llid20_23_cfg_GET_llid23_txfec_en(reg32)             REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_txfec_en, (reg32))
#define e_llid20_23_cfg_GET_llid23_dcrypt_en(reg32)            REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_dcrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid23_dcrypt_mode(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_dcrypt_mode, (reg32))
#define e_llid20_23_cfg_GET_llid23_oamlpbk_en(reg32)           REG_FLD_GET(e_llid20_23_cfg_FLD_llid23_oamlpbk_en, (reg32))
#define e_llid20_23_cfg_GET_llid22_dummy(reg32)                REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_dummy, (reg32))
#define e_llid20_23_cfg_GET_llid22_txfec_en(reg32)             REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_txfec_en, (reg32))
#define e_llid20_23_cfg_GET_llid22_dcrypt_en(reg32)            REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_dcrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid22_dcrypt_mode(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_dcrypt_mode, (reg32))
#define e_llid20_23_cfg_GET_llid22_oamlpbk_en(reg32)           REG_FLD_GET(e_llid20_23_cfg_FLD_llid22_oamlpbk_en, (reg32))
#define e_llid20_23_cfg_GET_llid21_dummy(reg32)                REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_dummy, (reg32))
#define e_llid20_23_cfg_GET_llid21_txfec_en(reg32)             REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_txfec_en, (reg32))
#define e_llid20_23_cfg_GET_llid21_dcrypt_en(reg32)            REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_dcrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid21_dcrypt_mode(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_dcrypt_mode, (reg32))
#define e_llid20_23_cfg_GET_llid21_oamlpbk_en(reg32)           REG_FLD_GET(e_llid20_23_cfg_FLD_llid21_oamlpbk_en, (reg32))
#define e_llid20_23_cfg_GET_llid20_dummy(reg32)                REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_dummy, (reg32))
#define e_llid20_23_cfg_GET_llid20_txfec_en(reg32)             REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_txfec_en, (reg32))
#define e_llid20_23_cfg_GET_llid20_dcrypt_en(reg32)            REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_dcrypt_en, (reg32))
#define e_llid20_23_cfg_GET_llid20_dcrypt_mode(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_dcrypt_mode, (reg32))
#define e_llid20_23_cfg_GET_llid20_oam_lpbk_en(reg32)          REG_FLD_GET(e_llid20_23_cfg_FLD_llid20_oam_lpbk_en, (reg32))

#define e_llid24_27_cfg_GET_llid27_dummy(reg32)                REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_dummy, (reg32))
#define e_llid24_27_cfg_GET_llid27_txfec_en(reg32)             REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_txfec_en, (reg32))
#define e_llid24_27_cfg_GET_llid27_dcrypt_en(reg32)            REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_dcrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid27_dcrypt_mode(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_dcrypt_mode, (reg32))
#define e_llid24_27_cfg_GET_llid27_oamlpbk_en(reg32)           REG_FLD_GET(e_llid24_27_cfg_FLD_llid27_oamlpbk_en, (reg32))
#define e_llid24_27_cfg_GET_llid26_dummy(reg32)                REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_dummy, (reg32))
#define e_llid24_27_cfg_GET_llid26_txfec_en(reg32)             REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_txfec_en, (reg32))
#define e_llid24_27_cfg_GET_llid26_dcrypt_en(reg32)            REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_dcrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid26_dcrypt_mode(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_dcrypt_mode, (reg32))
#define e_llid24_27_cfg_GET_llid26_oamlpbk_en(reg32)           REG_FLD_GET(e_llid24_27_cfg_FLD_llid26_oamlpbk_en, (reg32))
#define e_llid24_27_cfg_GET_llid25_dummy(reg32)                REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_dummy, (reg32))
#define e_llid24_27_cfg_GET_llid25_txfec_en(reg32)             REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_txfec_en, (reg32))
#define e_llid24_27_cfg_GET_llid25_dcrypt_en(reg32)            REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_dcrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid25_dcrypt_mode(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_dcrypt_mode, (reg32))
#define e_llid24_27_cfg_GET_llid25_oamlpbk_en(reg32)           REG_FLD_GET(e_llid24_27_cfg_FLD_llid25_oamlpbk_en, (reg32))
#define e_llid24_27_cfg_GET_llid24_dummy(reg32)                REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_dummy, (reg32))
#define e_llid24_27_cfg_GET_llid24_txfec_en(reg32)             REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_txfec_en, (reg32))
#define e_llid24_27_cfg_GET_llid24_dcrypt_en(reg32)            REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_dcrypt_en, (reg32))
#define e_llid24_27_cfg_GET_llid24_dcrypt_mode(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_dcrypt_mode, (reg32))
#define e_llid24_27_cfg_GET_llid24_oam_lpbk_en(reg32)          REG_FLD_GET(e_llid24_27_cfg_FLD_llid24_oam_lpbk_en, (reg32))

#define e_llid28_31_cfg_GET_llid31_dummy(reg32)                REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_dummy, (reg32))
#define e_llid28_31_cfg_GET_llid31_txfec_en(reg32)             REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_txfec_en, (reg32))
#define e_llid28_31_cfg_GET_llid31_dcrypt_en(reg32)            REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_dcrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid31_dcrypt_mode(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_dcrypt_mode, (reg32))
#define e_llid28_31_cfg_GET_llid31_oamlpbk_en(reg32)           REG_FLD_GET(e_llid28_31_cfg_FLD_llid31_oamlpbk_en, (reg32))
#define e_llid28_31_cfg_GET_llid30_dummy(reg32)                REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_dummy, (reg32))
#define e_llid28_31_cfg_GET_llid30_txfec_en(reg32)             REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_txfec_en, (reg32))
#define e_llid28_31_cfg_GET_llid30_dcrypt_en(reg32)            REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_dcrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid30_dcrypt_mode(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_dcrypt_mode, (reg32))
#define e_llid28_31_cfg_GET_llid30_oamlpbk_en(reg32)           REG_FLD_GET(e_llid28_31_cfg_FLD_llid30_oamlpbk_en, (reg32))
#define e_llid28_31_cfg_GET_llid29_dummy(reg32)                REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_dummy, (reg32))
#define e_llid28_31_cfg_GET_llid29_txfec_en(reg32)             REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_txfec_en, (reg32))
#define e_llid28_31_cfg_GET_llid29_dcrypt_en(reg32)            REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_dcrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid29_dcrypt_mode(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_dcrypt_mode, (reg32))
#define e_llid28_31_cfg_GET_llid29_oamlpbk_en(reg32)           REG_FLD_GET(e_llid28_31_cfg_FLD_llid29_oamlpbk_en, (reg32))
#define e_llid28_31_cfg_GET_llid28_dummy(reg32)                REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_dummy, (reg32))
#define e_llid28_31_cfg_GET_llid28_txfec_en(reg32)             REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_txfec_en, (reg32))
#define e_llid28_31_cfg_GET_llid28_dcrypt_en(reg32)            REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_dcrypt_en, (reg32))
#define e_llid28_31_cfg_GET_llid28_dcrypt_mode(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_dcrypt_mode, (reg32))
#define e_llid28_31_cfg_GET_llid28_oam_lpbk_en(reg32)          REG_FLD_GET(e_llid28_31_cfg_FLD_llid28_oam_lpbk_en, (reg32))

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

#define e_rpt_cfg2_GET_llid15_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid15_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid14_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid14_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid13_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid13_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid12_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid12_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid11_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid11_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid10_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg2_FLD_llid10_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid9_rpt_cfg(reg32)                    REG_FLD_GET(e_rpt_cfg2_FLD_llid9_rpt_cfg, (reg32))
#define e_rpt_cfg2_GET_llid8_rpt_cfg(reg32)                    REG_FLD_GET(e_rpt_cfg2_FLD_llid8_rpt_cfg, (reg32))

#define e_rpt_cfg3_GET_llid31_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid31_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid30_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid30_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid29_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid29_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid28_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid28_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid27_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid27_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid26_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid26_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid25_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid25_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid24_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid24_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid23_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid23_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid22_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid22_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid21_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid21_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid20_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid20_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid19_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid19_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid18_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid18_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid17_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid17_rpt_cfg, (reg32))
#define e_rpt_cfg3_GET_llid16_rpt_cfg(reg32)                   REG_FLD_GET(e_rpt_cfg3_FLD_llid16_rpt_cfg, (reg32))

#define e_rpt_qthld_cfg2_GET_rpt_llid_idx_4_3(reg32)           REG_FLD_GET(e_rpt_qthld_cfg2_FLD_rpt_llid_idx_4_3, (reg32))

#define e_rpt_bitmap_cfg_GET_rpt_bmap_rwcmd(reg32)             REG_FLD_GET(e_rpt_bitmap_cfg_FLD_rpt_bmap_rwcmd, (reg32))
#define e_rpt_bitmap_cfg_GET_rpt_bmap_rwcmd_done(reg32)        REG_FLD_GET(e_rpt_bitmap_cfg_FLD_rpt_bmap_rwcmd_done, (reg32))
#define e_rpt_bitmap_cfg_GET_rpt_bmap_llid_idx(reg32)          REG_FLD_GET(e_rpt_bitmap_cfg_FLD_rpt_bmap_llid_idx, (reg32))

#define e_rpt_bitmap_val_GET_rpt_qsizeadj_frc(reg32)           REG_FLD_GET(e_rpt_bitmap_val_FLD_rpt_qsizeadj_frc, (reg32))
#define e_rpt_bitmap_val_GET_rpt_bitmap_ctrl(reg32)            REG_FLD_GET(e_rpt_bitmap_val_FLD_rpt_bitmap_ctrl, (reg32))
#define e_rpt_bitmap_val_GET_rpt_bitmap_set(reg32)             REG_FLD_GET(e_rpt_bitmap_val_FLD_rpt_bitmap_set, (reg32))

#define e_glb_cfg_SET_sniffer_mode(reg32, val)                 REG_FLD_SET(e_glb_cfg_FLD_sniffer_mode, (reg32), (val))
#define e_glb_cfg_SET_txoam_favor(reg32, val)                  REG_FLD_SET(e_glb_cfg_FLD_txoam_favor, (reg32), (val))
#define e_glb_cfg_SET_burst_en_dly(reg32, val)                 REG_FLD_SET(e_glb_cfg_FLD_burst_en_dly, (reg32), (val))
#define e_glb_cfg_SET_discv_burst_en(reg32, val)               REG_FLD_SET(e_glb_cfg_FLD_discv_burst_en, (reg32), (val))
#define e_glb_cfg_SET_mpcp_fwd(reg32, val)                     REG_FLD_SET(e_glb_cfg_FLD_mpcp_fwd, (reg32), (val))
#define e_glb_cfg_SET_bcst_llid_m1_drop(reg32, val)            REG_FLD_SET(e_glb_cfg_FLD_bcst_llid_m1_drop, (reg32), (val))
#define e_glb_cfg_SET_bcst_llid_m0_drop(reg32, val)            REG_FLD_SET(e_glb_cfg_FLD_bcst_llid_m0_drop, (reg32), (val))
#define e_glb_cfg_SET_mcst_llid_drop(reg32, val)               REG_FLD_SET(e_glb_cfg_FLD_mcst_llid_drop, (reg32), (val))
#define e_glb_cfg_SET_all_unicast_llid_pkt_fwd(reg32, val)     REG_FLD_SET(e_glb_cfg_FLD_all_unicast_llid_pkt_fwd, (reg32), (val))
#define e_glb_cfg_SET_fcs_err_fwd(reg32, val)                  REG_FLD_SET(e_glb_cfg_FLD_fcs_err_fwd, (reg32), (val))
#define e_glb_cfg_SET_llid_crc8_err_fwd(reg32, val)            REG_FLD_SET(e_glb_cfg_FLD_llid_crc8_err_fwd, (reg32), (val))
#define e_glb_cfg_SET_rxmpi_stop(reg32, val)                   REG_FLD_SET(e_glb_cfg_FLD_rxmpi_stop, (reg32), (val))
#define e_glb_cfg_SET_txmpi_stop(reg32, val)                   REG_FLD_SET(e_glb_cfg_FLD_txmpi_stop, (reg32), (val))
#define e_glb_cfg_SET_phy_pwr_down(reg32, val)                 REG_FLD_SET(e_glb_cfg_FLD_phy_pwr_down, (reg32), (val))
#define e_glb_cfg_SET_rx_nml_gate_fwd(reg32, val)              REG_FLD_SET(e_glb_cfg_FLD_rx_nml_gate_fwd, (reg32), (val))
#define e_glb_cfg_SET_rxmbi_stop(reg32, val)                   REG_FLD_SET(e_glb_cfg_FLD_rxmbi_stop, (reg32), (val))
#define e_glb_cfg_SET_txmbi_stop(reg32, val)                   REG_FLD_SET(e_glb_cfg_FLD_txmbi_stop, (reg32), (val))
#define e_glb_cfg_SET_chk_all_gnt_mode(reg32, val)             REG_FLD_SET(e_glb_cfg_FLD_chk_all_gnt_mode, (reg32), (val))
#define e_glb_cfg_SET_tx_default_rpt(reg32, val)               REG_FLD_SET(e_glb_cfg_FLD_tx_default_rpt, (reg32), (val))
#define e_glb_cfg_SET_epon_mac_sw_rst(reg32, val)              REG_FLD_SET(e_glb_cfg_FLD_epon_mac_sw_rst, (reg32), (val))
#define e_glb_cfg_SET_epon_oam_cal_in_eth(reg32, val)          REG_FLD_SET(e_glb_cfg_FLD_epon_oam_cal_in_eth, (reg32), (val))
#define e_glb_cfg_SET_epon_mac_lpbk_en(reg32, val)             REG_FLD_SET(e_glb_cfg_FLD_epon_mac_lpbk_en, (reg32), (val))
#define e_glb_cfg_SET_rpt_txpri_ctrl(reg32, val)               REG_FLD_SET(e_glb_cfg_FLD_rpt_txpri_ctrl, (reg32), (val))

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

#define e_rpt_mpcp_timeout_llid_idx_SET_llid15_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid15_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid14_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid14_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid13_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid13_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid12_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid12_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid11_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid11_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid10_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid10_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid9_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid9_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid8_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid8_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid15_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid15_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid14_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid14_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid13_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid13_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid12_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid12_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid11_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid11_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid10_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid10_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid9_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid9_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid8_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid8_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid7_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid7_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid6_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid6_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid5_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid5_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid4_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid4_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid3_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid3_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid2_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid2_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid1_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid1_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid0_mpcp_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid0_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid7_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid7_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid6_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid6_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid5_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid5_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid4_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid4_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid3_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid3_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid2_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid2_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid1_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid1_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timeout_llid_idx_SET_llid0_rpt_tmo(reg32, val) REG_FLD_SET(e_rpt_mpcp_timeout_llid_idx_FLD_llid0_rpt_tmo, (reg32), (val))

#define e_dyinggsp_cfg_SET_hw_dying_gasp_en(reg32, val)        REG_FLD_SET(e_dyinggsp_cfg_FLD_hw_dying_gasp_en, (reg32), (val))
#define e_dyinggsp_cfg_SET_sw_init_dying_gasp(reg32, val)      REG_FLD_SET(e_dyinggsp_cfg_FLD_sw_init_dying_gasp, (reg32), (val))
#define e_dyinggsp_cfg_SET_dygsp_num_of_times(reg32, val)      REG_FLD_SET(e_dyinggsp_cfg_FLD_dygsp_num_of_times, (reg32), (val))

#define e_pending_gnt_num_SET_echoed_pending_gnt(reg32, val)   REG_FLD_SET(e_pending_gnt_num_FLD_echoed_pending_gnt, (reg32), (val))
#define e_pending_gnt_num_SET_pending_gnt_num(reg32, val)      REG_FLD_SET(e_pending_gnt_num_FLD_pending_gnt_num, (reg32), (val))

#define e_llid0_3_cfg_SET_llid3_dummy(reg32, val)              REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_dummy, (reg32), (val))
#define e_llid0_3_cfg_SET_llid3_txfec_en(reg32, val)           REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_txfec_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid3_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_dcrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid3_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_dcrypt_mode, (reg32), (val))
#define e_llid0_3_cfg_SET_llid3_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid3_oam_lpbk_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_dummy(reg32, val)              REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_dummy, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_txfec_en(reg32, val)           REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_txfec_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_dcrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_dcrypt_mode, (reg32), (val))
#define e_llid0_3_cfg_SET_llid2_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid2_oam_lpbk_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_dummy(reg32, val)              REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_dummy, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_txfec_en(reg32, val)           REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_txfec_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_dcrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_dcrypt_mode, (reg32), (val))
#define e_llid0_3_cfg_SET_llid1_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid1_oam_lpbk_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_dummy(reg32, val)              REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_dummy, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_txfec_en(reg32, val)           REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_txfec_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_dcrypt_en, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_dcrypt_mode, (reg32), (val))
#define e_llid0_3_cfg_SET_llid0_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid0_3_cfg_FLD_llid0_oam_lpbk_en, (reg32), (val))

#define e_llid4_7_cfg_SET_llid7_dummy(reg32, val)              REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_dummy, (reg32), (val))
#define e_llid4_7_cfg_SET_llid7_txfec_en(reg32, val)           REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_txfec_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid7_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_dcrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid7_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_dcrypt_mode, (reg32), (val))
#define e_llid4_7_cfg_SET_llid7_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid7_oam_lpbk_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_dummy(reg32, val)              REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_dummy, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_txfec_en(reg32, val)           REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_txfec_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_dcrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_dcrypt_mode, (reg32), (val))
#define e_llid4_7_cfg_SET_llid6_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid6_oam_lpbk_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_dummy(reg32, val)              REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_dummy, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_txfec_en(reg32, val)           REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_txfec_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_dcrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_dcrypt_mode, (reg32), (val))
#define e_llid4_7_cfg_SET_llid5_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid5_oam_lpbk_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_dummy(reg32, val)              REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_dummy, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_txfec_en(reg32, val)           REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_txfec_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_dcrypt_en(reg32, val)          REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_dcrypt_en, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_dcrypt_mode(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_dcrypt_mode, (reg32), (val))
#define e_llid4_7_cfg_SET_llid4_oam_lpbk_en(reg32, val)        REG_FLD_SET(e_llid4_7_cfg_FLD_llid4_oam_lpbk_en, (reg32), (val))

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

#define e_mac_addr_cfg_SET_mac_addr_rwcmd(reg32, val)          REG_FLD_SET(e_mac_addr_cfg_FLD_mac_addr_rwcmd, (reg32), (val))
#define e_mac_addr_cfg_SET_mac_addr_rwcmd_done(reg32, val)     REG_FLD_SET(e_mac_addr_cfg_FLD_mac_addr_rwcmd_done, (reg32), (val))
#define e_mac_addr_cfg_SET_mac_addr_llid_indx(reg32, val)      REG_FLD_SET(e_mac_addr_cfg_FLD_mac_addr_llid_indx, (reg32), (val))
#define e_mac_addr_cfg_SET_mac_addr_dw_idx(reg32, val)         REG_FLD_SET(e_mac_addr_cfg_FLD_mac_addr_dw_idx, (reg32), (val))

#define e_mac_addr_value_SET_mac_addr_value(reg32, val)        REG_FLD_SET(e_mac_addr_value_FLD_mac_addr_value, (reg32), (val))

#define e_security_key_cfg_SET_key_rwcmd(reg32, val)           REG_FLD_SET(e_security_key_cfg_FLD_key_rwcmd, (reg32), (val))
#define e_security_key_cfg_SET_key_rwcmd_done(reg32, val)      REG_FLD_SET(e_security_key_cfg_FLD_key_rwcmd_done, (reg32), (val))
#define e_security_key_cfg_SET_key_llid_index(reg32, val)      REG_FLD_SET(e_security_key_cfg_FLD_key_llid_index, (reg32), (val))
#define e_security_key_cfg_SET_key_idx(reg32, val)             REG_FLD_SET(e_security_key_cfg_FLD_key_idx, (reg32), (val))
#define e_security_key_cfg_SET_key_dw_indx(reg32, val)         REG_FLD_SET(e_security_key_cfg_FLD_key_dw_indx, (reg32), (val))

#define e_key_value_SET_key_value(reg32, val)                  REG_FLD_SET(e_key_value_FLD_key_value, (reg32), (val))

#define e_rpt_cfg_SET_llid7_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid7_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid6_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid6_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid5_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid5_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid4_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid4_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid3_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid3_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid2_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid2_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid1_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid1_rpt_cfg, (reg32), (val))
#define e_rpt_cfg_SET_llid0_rpt_cfg(reg32, val)                REG_FLD_SET(e_rpt_cfg_FLD_llid0_rpt_cfg, (reg32), (val))

#define e_rpt_qthld_cfg_SET_qthld_rwcmd(reg32, val)            REG_FLD_SET(e_rpt_qthld_cfg_FLD_qthld_rwcmd, (reg32), (val))
#define e_rpt_qthld_cfg_SET_qthld_rwcmd_done(reg32, val)       REG_FLD_SET(e_rpt_qthld_cfg_FLD_qthld_rwcmd_done, (reg32), (val))
#define e_rpt_qthld_cfg_SET_qthld_value(reg32, val)            REG_FLD_SET(e_rpt_qthld_cfg_FLD_qthld_value, (reg32), (val))
#define e_rpt_qthld_cfg_SET_qthld_idx(reg32, val)              REG_FLD_SET(e_rpt_qthld_cfg_FLD_qthld_idx, (reg32), (val))
#define e_rpt_qthld_cfg_SET_rpt_llid_idx_2_0(reg32, val)       REG_FLD_SET(e_rpt_qthld_cfg_FLD_rpt_llid_idx_2_0, (reg32), (val))
#define e_rpt_qthld_cfg_SET_queue_idx(reg32, val)              REG_FLD_SET(e_rpt_qthld_cfg_FLD_queue_idx, (reg32), (val))

#define e_local_time_SET_local_time(reg32, val)                REG_FLD_SET(e_local_time_FLD_local_time, (reg32), (val))

#define e_tod_sync_x_SET_tod_sync_x(reg32, val)                REG_FLD_SET(e_tod_sync_x_FLD_tod_sync_x, (reg32), (val))

#define e_tod_ltncy_SET_rl_rx_phydly_ofst(reg32, val)          REG_FLD_SET(e_tod_ltncy_FLD_rl_rx_phydly_ofst, (reg32), (val))
#define e_tod_ltncy_SET_ingrs_latency(reg32, val)              REG_FLD_SET(e_tod_ltncy_FLD_ingrs_latency, (reg32), (val))
#define e_tod_ltncy_SET_egrs_latency(reg32, val)               REG_FLD_SET(e_tod_ltncy_FLD_egrs_latency, (reg32), (val))

#define e_new_tod_p2p_offset_sec_l32_SET_new_tod_p2p_offset_sec_l32(reg32, val) REG_FLD_SET(e_new_tod_p2p_offset_sec_l32_FLD_new_tod_p2p_offset_sec_l32, (reg32), (val))

#define e_new_tod_p2p_tod_offset_nsec_SET_new_tod_nsec(reg32, val) REG_FLD_SET(e_new_tod_p2p_tod_offset_nsec_FLD_new_tod_nsec, (reg32), (val))

#define e_tod_p2p_tod_sec_l32_SET_tod_p2p_sec_l32(reg32, val)  REG_FLD_SET(e_tod_p2p_tod_sec_l32_FLD_tod_p2p_sec_l32, (reg32), (val))

#define e_tod_p2p_tod_nsec_SET_tod_p2p_nsec(reg32, val)        REG_FLD_SET(e_tod_p2p_tod_nsec_FLD_tod_p2p_nsec, (reg32), (val))

#define e_tod_period_SET_tod_period(reg32, val)                REG_FLD_SET(e_tod_period_FLD_tod_period, (reg32), (val))

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

#define e_slp_duration_i_SET_slp_pwd_mode_i(reg32, val)        REG_FLD_SET(e_slp_duration_i_FLD_slp_pwd_mode_i, (reg32), (val))
#define e_slp_duration_i_SET_slp_flag_i(reg32, val)            REG_FLD_SET(e_slp_duration_i_FLD_slp_flag_i, (reg32), (val))
#define e_slp_duration_i_SET_slp_duration_i(reg32, val)        REG_FLD_SET(e_slp_duration_i_FLD_slp_duration_i, (reg32), (val))

#define e_txfetch_cfg_SET_tx_fetch_leadtime(reg32, val)        REG_FLD_SET(e_txfetch_cfg_FLD_tx_fetch_leadtime, (reg32), (val))
#define e_txfetch_cfg_SET_tx_dma_leadtime(reg32, val)          REG_FLD_SET(e_txfetch_cfg_FLD_tx_dma_leadtime, (reg32), (val))

#define e_sync_time_SET_sync_time_updte(reg32, val)            REG_FLD_SET(e_sync_time_FLD_sync_time_updte, (reg32), (val))
#define e_sync_time_SET_sync_time(reg32, val)                  REG_FLD_SET(e_sync_time_FLD_sync_time, (reg32), (val))

#define e_tx_cal_cnst_SET_dscvr_gnt_len(reg32, val)            REG_FLD_SET(e_tx_cal_cnst_FLD_dscvr_gnt_len, (reg32), (val))
#define e_tx_cal_cnst_SET_fec_tail_grd(reg32, val)             REG_FLD_SET(e_tx_cal_cnst_FLD_fec_tail_grd, (reg32), (val))
#define e_tx_cal_cnst_SET_tail_grd(reg32, val)                 REG_FLD_SET(e_tx_cal_cnst_FLD_tail_grd, (reg32), (val))
#define e_tx_cal_cnst_SET_default_ovrhd(reg32, val)            REG_FLD_SET(e_tx_cal_cnst_FLD_default_ovrhd, (reg32), (val))

#define e_laser_onoff_time_SET_laser_off_time(reg32, val)      REG_FLD_SET(e_laser_onoff_time_FLD_laser_off_time, (reg32), (val))
#define e_laser_onoff_time_SET_laser_on_time(reg32, val)       REG_FLD_SET(e_laser_onoff_time_FLD_laser_on_time, (reg32), (val))

#define e_grd_thrshld_SET_guard_thrshld(reg32, val)            REG_FLD_SET(e_grd_thrshld_FLD_guard_thrshld, (reg32), (val))

#define e_mpcp_timeout_intvl_SET_mpcp_timeout_intvl(reg32, val) REG_FLD_SET(e_mpcp_timeout_intvl_FLD_mpcp_timeout_intvl, (reg32), (val))

#define e_rpt_timeout_intvl_SET_rpt_timeout_intvl(reg32, val)  REG_FLD_SET(e_rpt_timeout_intvl_FLD_rpt_timeout_intvl, (reg32), (val))

#define e_max_future_gnt_time_SET_max_future_gnt_time(reg32, val) REG_FLD_SET(e_max_future_gnt_time_FLD_max_future_gnt_time, (reg32), (val))

#define e_min_proc_time_SET_min_proc_time(reg32, val)          REG_FLD_SET(e_min_proc_time_FLD_min_proc_time, (reg32), (val))

#define e_trx_adjust_time1_SET_tx_stm_adj(reg32, val)          REG_FLD_SET(e_trx_adjust_time1_FLD_tx_stm_adj, (reg32), (val))

#define e_trx_adjust_time2_SET_tx_len_adj(reg32, val)          REG_FLD_SET(e_trx_adjust_time2_FLD_tx_len_adj, (reg32), (val))
#define e_trx_adjust_time2_SET_rx_tmstp_adj(reg32, val)        REG_FLD_SET(e_trx_adjust_time2_FLD_rx_tmstp_adj, (reg32), (val))

#define e_dbg_prb_sel_SET_probe_dtgrp_sel(reg32, val)          REG_FLD_SET(e_dbg_prb_sel_FLD_probe_dtgrp_sel, (reg32), (val))
#define e_dbg_prb_sel_SET_probe_bit0_sel(reg32, val)           REG_FLD_SET(e_dbg_prb_sel_FLD_probe_bit0_sel, (reg32), (val))
#define e_dbg_prb_sel_SET_probe_sel(reg32, val)                REG_FLD_SET(e_dbg_prb_sel_FLD_probe_sel, (reg32), (val))

#define e_dbg_prb_h32_SET_probe_h32(reg32, val)                REG_FLD_SET(e_dbg_prb_h32_FLD_probe_h32, (reg32), (val))

#define e_dbg_prb_l32_SET_probe_l32(reg32, val)                REG_FLD_SET(e_dbg_prb_l32_FLD_probe_l32, (reg32), (val))

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

#define e_gnt_pending_stat_SET_max_gnt_pending_cnt(reg32, val) REG_FLD_SET(e_gnt_pending_stat_FLD_max_gnt_pending_cnt, (reg32), (val))
#define e_gnt_pending_stat_SET_cur_gnt_pending_cnt(reg32, val) REG_FLD_SET(e_gnt_pending_stat_FLD_cur_gnt_pending_cnt, (reg32), (val))

#define e_gnt_length_stat_SET_max_gnt_length(reg32, val)       REG_FLD_SET(e_gnt_length_stat_FLD_max_gnt_length, (reg32), (val))
#define e_gnt_length_stat_SET_min_gnt_length(reg32, val)       REG_FLD_SET(e_gnt_length_stat_FLD_min_gnt_length, (reg32), (val))

#define e_gnt_type_stat_SET_b2b_gnt_cnt(reg32, val)            REG_FLD_SET(e_gnt_type_stat_FLD_b2b_gnt_cnt, (reg32), (val))
#define e_gnt_type_stat_SET_hdn_gnt_cnt(reg32, val)            REG_FLD_SET(e_gnt_type_stat_FLD_hdn_gnt_cnt, (reg32), (val))

#define e_time_drft_stat_SET_cur_time_drift_ofst(reg32, val)   REG_FLD_SET(e_time_drft_stat_FLD_cur_time_drift_ofst, (reg32), (val))
#define e_time_drft_stat_SET_max_time_drift_ofst(reg32, val)   REG_FLD_SET(e_time_drft_stat_FLD_max_time_drift_ofst, (reg32), (val))
#define e_time_drft_stat_SET_cur_time_drift(reg32, val)        REG_FLD_SET(e_time_drft_stat_FLD_cur_time_drift, (reg32), (val))
#define e_time_drft_stat_SET_max_time_drift(reg32, val)        REG_FLD_SET(e_time_drft_stat_FLD_max_time_drift, (reg32), (val))

#define e_llid0_gnt_stat_SET_llid0_gnt_cnt(reg32, val)         REG_FLD_SET(e_llid0_gnt_stat_FLD_llid0_gnt_cnt, (reg32), (val))

#define e_llid1_gnt_stat_SET_llid1_gnt_cnt(reg32, val)         REG_FLD_SET(e_llid1_gnt_stat_FLD_llid1_gnt_cnt, (reg32), (val))

#define e_llid2_gnt_stat_SET_llid2_gnt_cnt(reg32, val)         REG_FLD_SET(e_llid2_gnt_stat_FLD_llid2_gnt_cnt, (reg32), (val))

#define e_llid3_gnt_stat_SET_llid3_gnt_cnt(reg32, val)         REG_FLD_SET(e_llid3_gnt_stat_FLD_llid3_gnt_cnt, (reg32), (val))

#define e_llid4_gnt_stat_SET_llid4_gnt_cnt(reg32, val)         REG_FLD_SET(e_llid4_gnt_stat_FLD_llid4_gnt_cnt, (reg32), (val))

#define e_llid5_gnt_stat_SET_llid5_gnt_cnt(reg32, val)         REG_FLD_SET(e_llid5_gnt_stat_FLD_llid5_gnt_cnt, (reg32), (val))

#define e_llid6_gnt_stat_SET_llid6_gnt_cnt(reg32, val)         REG_FLD_SET(e_llid6_gnt_stat_FLD_llid6_gnt_cnt, (reg32), (val))

#define e_llid7_gnt_stat_SET_llid7_gnt_cnt(reg32, val)         REG_FLD_SET(e_llid7_gnt_stat_FLD_llid7_gnt_cnt, (reg32), (val))

#define e_snf_mpcp_oam_ctl_SET_snf_mpcp_cap_en(reg32, val)     REG_FLD_SET(e_snf_mpcp_oam_ctl_FLD_snf_mpcp_cap_en, (reg32), (val))
#define e_snf_mpcp_oam_ctl_SET_snf_oam_cap_en(reg32, val)      REG_FLD_SET(e_snf_mpcp_oam_ctl_FLD_snf_oam_cap_en, (reg32), (val))
#define e_snf_mpcp_oam_ctl_SET_snf_mpcp_oam_cnt_set(reg32, val) REG_FLD_SET(e_snf_mpcp_oam_ctl_FLD_snf_mpcp_oam_cnt_set, (reg32), (val))

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

#define e_oam_kpalv_ctrl_SET_oam_kpalv_interval(reg32, val)    REG_FLD_SET(e_oam_kpalv_ctrl_FLD_oam_kpalv_interval, (reg32), (val))
#define e_oam_kpalv_ctrl_SET_oam_kpalv_sw_trig(reg32, val)     REG_FLD_SET(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_trig, (reg32), (val))
#define e_oam_kpalv_ctrl_SET_oam_kpalv_sw_cfg(reg32, val)      REG_FLD_SET(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_cfg, (reg32), (val))
#define e_oam_kpalv_ctrl_SET_oam_kpalv_en(reg32, val)          REG_FLD_SET(e_oam_kpalv_ctrl_FLD_oam_kpalv_en, (reg32), (val))

#define e_tod_1pps_ctrl_SET_tod_1pps_width_ctrl(reg32, val)    REG_FLD_SET(e_tod_1pps_ctrl_FLD_tod_1pps_width_ctrl, (reg32), (val))

#define e_sniff_sp_tag_SET_sniffer_sp_tag(reg32, val)          REG_FLD_SET(e_sniff_sp_tag_FLD_sniffer_sp_tag, (reg32), (val))

#define e_glb_cfg2_SET_llidnum_sel(reg32, val)                 REG_FLD_SET(e_glb_cfg2_FLD_llidnum_sel, (reg32), (val))
#define e_glb_cfg2_SET_loctime_mtd(reg32, val)                 REG_FLD_SET(e_glb_cfg2_FLD_loctime_mtd, (reg32), (val))
#define e_glb_cfg2_SET_rgreq_lsrtime_mask(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_rgreq_lsrtime_mask, (reg32), (val))
#define e_glb_cfg2_SET_all_ucllid_mpcpsnf(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_all_ucllid_mpcpsnf, (reg32), (val))
#define e_glb_cfg2_SET_u10g_txmode(reg32, val)                 REG_FLD_SET(e_glb_cfg2_FLD_u10g_txmode, (reg32), (val))
#define e_glb_cfg2_SET_eth_cal_in_bytecnt(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_eth_cal_in_bytecnt, (reg32), (val))
#define e_glb_cfg2_SET_snf_cal_in_bytecnt(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_snf_cal_in_bytecnt, (reg32), (val))
#define e_glb_cfg2_SET_oam_cal_in_bytecnt(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_oam_cal_in_bytecnt, (reg32), (val))
#define e_glb_cfg2_SET_dscvgate_filt(reg32, val)               REG_FLD_SET(e_glb_cfg2_FLD_dscvgate_filt, (reg32), (val))
#define e_glb_cfg2_SET_us_snf(reg32, val)                      REG_FLD_SET(e_glb_cfg2_FLD_us_snf, (reg32), (val))
#define e_glb_cfg2_SET_llidinfo_snf(reg32, val)                REG_FLD_SET(e_glb_cfg2_FLD_llidinfo_snf, (reg32), (val))
#define e_glb_cfg2_SET_all_ucllid_ethsnf(reg32, val)           REG_FLD_SET(e_glb_cfg2_FLD_all_ucllid_ethsnf, (reg32), (val))
#define e_glb_cfg2_SET_all_ucllid_snf(reg32, val)              REG_FLD_SET(e_glb_cfg2_FLD_all_ucllid_snf, (reg32), (val))
#define e_glb_cfg2_SET_rxuni_mcllid_en(reg32, val)             REG_FLD_SET(e_glb_cfg2_FLD_rxuni_mcllid_en, (reg32), (val))
#define e_glb_cfg2_SET_gntlen_stat_widscv(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_gntlen_stat_widscv, (reg32), (val))
#define e_glb_cfg2_SET_tdrift_loctupd_dis(reg32, val)          REG_FLD_SET(e_glb_cfg2_FLD_tdrift_loctupd_dis, (reg32), (val))
#define e_glb_cfg2_SET_snf_fcserr_fwd(reg32, val)              REG_FLD_SET(e_glb_cfg2_FLD_snf_fcserr_fwd, (reg32), (val))
#define e_glb_cfg2_SET_mpcp_fcserr_fwd(reg32, val)             REG_FLD_SET(e_glb_cfg2_FLD_mpcp_fcserr_fwd, (reg32), (val))
#define e_glb_cfg2_SET_dscvgate_infochk_dis(reg32, val)        REG_FLD_SET(e_glb_cfg2_FLD_dscvgate_infochk_dis, (reg32), (val))

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
#define e_int_sts2_SET_rcv_dauc_dscvgate_int(reg32, val)       REG_FLD_SET(e_int_sts2_FLD_rcv_dauc_dscvgate_int, (reg32), (val))

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
#define e_int_en2_SET_rcv_dauc_dscvgate_en(reg32, val)         REG_FLD_SET(e_int_en2_FLD_rcv_dauc_dscvgate_en, (reg32), (val))

#define e_exc_sts_SET_rx_rgst_dergst_hit(reg32, val)           REG_FLD_SET(e_exc_sts_FLD_rx_rgst_dergst_hit, (reg32), (val))
#define e_exc_sts_SET_rx_rgst_nack_hit(reg32, val)             REG_FLD_SET(e_exc_sts_FLD_rx_rgst_nack_hit, (reg32), (val))
#define e_exc_sts_SET_snf_pktcnt_err_hit(reg32, val)           REG_FLD_SET(e_exc_sts_FLD_snf_pktcnt_err_hit, (reg32), (val))
#define e_exc_sts_SET_tx_gntst_nosog_hit(reg32, val)           REG_FLD_SET(e_exc_sts_FLD_tx_gntst_nosog_hit, (reg32), (val))
#define e_exc_sts_SET_tx_gntst_fifound_hit(reg32, val)         REG_FLD_SET(e_exc_sts_FLD_tx_gntst_fifound_hit, (reg32), (val))
#define e_exc_sts_SET_tx_gntph_fifound_hit(reg32, val)         REG_FLD_SET(e_exc_sts_FLD_tx_gntph_fifound_hit, (reg32), (val))
#define e_exc_sts_SET_tx_gntph_exceed_hit(reg32, val)          REG_FLD_SET(e_exc_sts_FLD_tx_gntph_exceed_hit, (reg32), (val))
#define e_exc_sts_SET_schgnt_stinv_hit(reg32, val)             REG_FLD_SET(e_exc_sts_FLD_schgnt_stinv_hit, (reg32), (val))
#define e_exc_sts_SET_schgnt_idxinv_hit(reg32, val)            REG_FLD_SET(e_exc_sts_FLD_schgnt_idxinv_hit, (reg32), (val))
#define e_exc_sts_SET_decrpt_vecm_acccol_hit(reg32, val)       REG_FLD_SET(e_exc_sts_FLD_decrpt_vecm_acccol_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_len_long_hit(reg32, val)             REG_FLD_SET(e_exc_sts_FLD_rcv_len_long_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_len_short_hit(reg32, val)            REG_FLD_SET(e_exc_sts_FLD_rcv_len_short_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_mpcp_crcerr_hit(reg32, val)          REG_FLD_SET(e_exc_sts_FLD_rcv_mpcp_crcerr_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_eth_crcerr_hit(reg32, val)           REG_FLD_SET(e_exc_sts_FLD_rcv_eth_crcerr_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_crc8err_hit(reg32, val)              REG_FLD_SET(e_exc_sts_FLD_rcv_crc8err_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_nrlgate_llidbc_hit(reg32, val)       REG_FLD_SET(e_exc_sts_FLD_rcv_nrlgate_llidbc_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_dscvgate_filt_hit(reg32, val)        REG_FLD_SET(e_exc_sts_FLD_rcv_dscvgate_filt_hit, (reg32), (val))
#define e_exc_sts_SET_rcv_dscvgate_infomis_hit(reg32, val)     REG_FLD_SET(e_exc_sts_FLD_rcv_dscvgate_infomis_hit, (reg32), (val))

#define e_olt_dscvinfo_SET_olt_dscvinfo_match(reg32, val)      REG_FLD_SET(e_olt_dscvinfo_FLD_olt_dscvinfo_match, (reg32), (val))
#define e_olt_dscvinfo_SET_olt_dscvinfo_mis(reg32, val)        REG_FLD_SET(e_olt_dscvinfo_FLD_olt_dscvinfo_mis, (reg32), (val))

#define e_laser_onoff_time2_SET_lsroff_time_olt(reg32, val)    REG_FLD_SET(e_laser_onoff_time2_FLD_lsroff_time_olt, (reg32), (val))
#define e_laser_onoff_time2_SET_lsron_time_olt(reg32, val)     REG_FLD_SET(e_laser_onoff_time2_FLD_lsron_time_olt, (reg32), (val))
#define e_laser_onoff_time2_SET_sync_time_olt(reg32, val)      REG_FLD_SET(e_laser_onoff_time2_FLD_sync_time_olt, (reg32), (val))

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

#define e_overhead_time_thr_SET_sync_time_maxen(reg32, val)    REG_FLD_SET(e_overhead_time_thr_FLD_sync_time_maxen, (reg32), (val))
#define e_overhead_time_thr_SET_lsroff_time_maxen(reg32, val)  REG_FLD_SET(e_overhead_time_thr_FLD_lsroff_time_maxen, (reg32), (val))
#define e_overhead_time_thr_SET_lsron_time_maxen(reg32, val)   REG_FLD_SET(e_overhead_time_thr_FLD_lsron_time_maxen, (reg32), (val))
#define e_overhead_time_thr_SET_sync_time_maxval(reg32, val)   REG_FLD_SET(e_overhead_time_thr_FLD_sync_time_maxval, (reg32), (val))
#define e_overhead_time_thr_SET_lsroff_time_maxval(reg32, val) REG_FLD_SET(e_overhead_time_thr_FLD_lsroff_time_maxval, (reg32), (val))
#define e_overhead_time_thr_SET_lsron_time_maxval(reg32, val)  REG_FLD_SET(e_overhead_time_thr_FLD_lsron_time_maxval, (reg32), (val))

#define e_txcal_cnst2_SET_ipgalign_mtd(reg32, val)             REG_FLD_SET(e_txcal_cnst2_FLD_ipgalign_mtd, (reg32), (val))
#define e_txcal_cnst2_SET_u10g_dscv_gntlen(reg32, val)         REG_FLD_SET(e_txcal_cnst2_FLD_u10g_dscv_gntlen, (reg32), (val))
#define e_txcal_cnst2_SET_u10g_tail_grd(reg32, val)            REG_FLD_SET(e_txcal_cnst2_FLD_u10g_tail_grd, (reg32), (val))

#define e_txcal_cnst3_SET_u1g_fecon_min_gntlen(reg32, val)     REG_FLD_SET(e_txcal_cnst3_FLD_u1g_fecon_min_gntlen, (reg32), (val))
#define e_txcal_cnst3_SET_u10g_eoblen(reg32, val)              REG_FLD_SET(e_txcal_cnst3_FLD_u10g_eoblen, (reg32), (val))
#define e_txcal_cnst3_SET_u10g_min_gntlen(reg32, val)          REG_FLD_SET(e_txcal_cnst3_FLD_u10g_min_gntlen, (reg32), (val))
#define e_txcal_cnst3_SET_u1g_fecoff_min_gntlen(reg32, val)    REG_FLD_SET(e_txcal_cnst3_FLD_u1g_fecoff_min_gntlen, (reg32), (val))

#define e_txsch_cfg_SET_txsch_dmy0(reg32, val)                 REG_FLD_SET(e_txsch_cfg_FLD_txsch_dmy0, (reg32), (val))
#define e_txsch_cfg_SET_txfifo_pad_hthr(reg32, val)            REG_FLD_SET(e_txsch_cfg_FLD_txfifo_pad_hthr, (reg32), (val))
#define e_txsch_cfg_SET_txsch_dmy1(reg32, val)                 REG_FLD_SET(e_txsch_cfg_FLD_txsch_dmy1, (reg32), (val))
#define e_txsch_cfg_SET_txfifo_pad_lthr(reg32, val)            REG_FLD_SET(e_txsch_cfg_FLD_txfifo_pad_lthr, (reg32), (val))

#define e_rxfifo_thr_SET_rxfifo_mbithr(reg32, val)             REG_FLD_SET(e_rxfifo_thr_FLD_rxfifo_mbithr, (reg32), (val))

#define e_bcllid_cfg_SET_d10g_bcllid(reg32, val)               REG_FLD_SET(e_bcllid_cfg_FLD_d10g_bcllid, (reg32), (val))
#define e_bcllid_cfg_SET_d1g_bcllid(reg32, val)                REG_FLD_SET(e_bcllid_cfg_FLD_d1g_bcllid, (reg32), (val))

#define e_txfrm_cfg1_SET_tx_mpcp_addrl(reg32, val)             REG_FLD_SET(e_txfrm_cfg1_FLD_tx_mpcp_addrl, (reg32), (val))

#define e_txfrm_cfg2_SET_tx_mpcp_addrh(reg32, val)             REG_FLD_SET(e_txfrm_cfg2_FLD_tx_mpcp_addrh, (reg32), (val))
#define e_txfrm_cfg2_SET_tx_mpcp_etype(reg32, val)             REG_FLD_SET(e_txfrm_cfg2_FLD_tx_mpcp_etype, (reg32), (val))

#define e_txfrm_cfg3_SET_tx_rgreq_op(reg32, val)               REG_FLD_SET(e_txfrm_cfg3_FLD_tx_rgreq_op, (reg32), (val))
#define e_txfrm_cfg3_SET_tx_rgack_op(reg32, val)               REG_FLD_SET(e_txfrm_cfg3_FLD_tx_rgack_op, (reg32), (val))

#define e_u1g_rpt_qsizeadj1_SET_u1g_fecoff_rpt_q1_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj1_FLD_u1g_fecoff_rpt_q1_qsizeadj, (reg32), (val))
#define e_u1g_rpt_qsizeadj1_SET_u1g_fecoff_rpt_q0_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj1_FLD_u1g_fecoff_rpt_q0_qsizeadj, (reg32), (val))

#define e_u1g_rpt_qsizeadj2_SET_u1g_fecoff_rpt_q3_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj2_FLD_u1g_fecoff_rpt_q3_qsizeadj, (reg32), (val))
#define e_u1g_rpt_qsizeadj2_SET_u1g_fecoff_rpt_q2_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj2_FLD_u1g_fecoff_rpt_q2_qsizeadj, (reg32), (val))

#define e_u1g_rpt_qsizeadj3_SET_u1g_fecoff_rpt_q5_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj3_FLD_u1g_fecoff_rpt_q5_qsizeadj, (reg32), (val))
#define e_u1g_rpt_qsizeadj3_SET_u1g_fecoff_rpt_q4_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj3_FLD_u1g_fecoff_rpt_q4_qsizeadj, (reg32), (val))

#define e_u1g_rpt_qsizeadj4_SET_u1g_fecoff_rpt_q7_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj4_FLD_u1g_fecoff_rpt_q7_qsizeadj, (reg32), (val))
#define e_u1g_rpt_qsizeadj4_SET_u1g_fecoff_rpt_q6_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj4_FLD_u1g_fecoff_rpt_q6_qsizeadj, (reg32), (val))

#define e_u1g_rpt_qsizeadj5_SET_u1g_fecon_rpt_q1_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj5_FLD_u1g_fecon_rpt_q1_qsizeadj, (reg32), (val))
#define e_u1g_rpt_qsizeadj5_SET_u1g_fecon_rpt_q0_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj5_FLD_u1g_fecon_rpt_q0_qsizeadj, (reg32), (val))

#define e_u1g_rpt_qsizeadj6_SET_u1g_fecon_rpt_q3_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj6_FLD_u1g_fecon_rpt_q3_qsizeadj, (reg32), (val))
#define e_u1g_rpt_qsizeadj6_SET_u1g_fecon_rpt_q2_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj6_FLD_u1g_fecon_rpt_q2_qsizeadj, (reg32), (val))

#define e_u1g_rpt_qsizeadj7_SET_u1g_fecon_rpt_q5_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj7_FLD_u1g_fecon_rpt_q5_qsizeadj, (reg32), (val))
#define e_u1g_rpt_qsizeadj7_SET_u1g_fecon_rpt_q4_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj7_FLD_u1g_fecon_rpt_q4_qsizeadj, (reg32), (val))

#define e_u1g_rpt_qsizeadj8_SET_u1g_fecon_rpt_q7_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj8_FLD_u1g_fecon_rpt_q7_qsizeadj, (reg32), (val))
#define e_u1g_rpt_qsizeadj8_SET_u1g_fecon_rpt_q6_qsizeadj(reg32, val) REG_FLD_SET(e_u1g_rpt_qsizeadj8_FLD_u1g_fecon_rpt_q6_qsizeadj, (reg32), (val))

#define e_u10g_rpt_qsizeadj1_SET_u10g_rpt_q1_qsizeadj(reg32, val) REG_FLD_SET(e_u10g_rpt_qsizeadj1_FLD_u10g_rpt_q1_qsizeadj, (reg32), (val))
#define e_u10g_rpt_qsizeadj1_SET_u10g_rpt_q0_qsizeadj(reg32, val) REG_FLD_SET(e_u10g_rpt_qsizeadj1_FLD_u10g_rpt_q0_qsizeadj, (reg32), (val))

#define e_u10g_rpt_qsizeadj2_SET_u10g_rpt_q3_qsizeadj(reg32, val) REG_FLD_SET(e_u10g_rpt_qsizeadj2_FLD_u10g_rpt_q3_qsizeadj, (reg32), (val))
#define e_u10g_rpt_qsizeadj2_SET_u10g_rpt_q2_qsizeadj(reg32, val) REG_FLD_SET(e_u10g_rpt_qsizeadj2_FLD_u10g_rpt_q2_qsizeadj, (reg32), (val))

#define e_u10g_rpt_qsizeadj3_SET_u10g_rpt_q5_qsizeadj(reg32, val) REG_FLD_SET(e_u10g_rpt_qsizeadj3_FLD_u10g_rpt_q5_qsizeadj, (reg32), (val))
#define e_u10g_rpt_qsizeadj3_SET_u10g_rpt_q4_qsizeadj(reg32, val) REG_FLD_SET(e_u10g_rpt_qsizeadj3_FLD_u10g_rpt_q4_qsizeadj, (reg32), (val))

#define e_u10g_rpt_qsizeadj4_SET_u10g_rpt_q7_qsizeadj(reg32, val) REG_FLD_SET(e_u10g_rpt_qsizeadj4_FLD_u10g_rpt_q7_qsizeadj, (reg32), (val))
#define e_u10g_rpt_qsizeadj4_SET_u10g_rpt_q6_qsizeadj(reg32, val) REG_FLD_SET(e_u10g_rpt_qsizeadj4_FLD_u10g_rpt_q6_qsizeadj, (reg32), (val))

#define e_snf_dah_SET_snf_pkt_dah(reg32, val)                  REG_FLD_SET(e_snf_dah_FLD_snf_pkt_dah, (reg32), (val))

#define e_snf_dal_SET_snf_pkt_dal(reg32, val)                  REG_FLD_SET(e_snf_dal_FLD_snf_pkt_dal, (reg32), (val))
#define e_snf_dal_SET_snf_pkt_sah(reg32, val)                  REG_FLD_SET(e_snf_dal_FLD_snf_pkt_sah, (reg32), (val))

#define e_snf_sal_SET_snf_pkt_sal(reg32, val)                  REG_FLD_SET(e_snf_sal_FLD_snf_pkt_sal, (reg32), (val))

#define e_snf_etype_SET_snf_pkt_etype(reg32, val)              REG_FLD_SET(e_snf_etype_FLD_snf_pkt_etype, (reg32), (val))

#define e_rxuni_mcllid_cfg0_SET_rxuni_mcllid1(reg32, val)      REG_FLD_SET(e_rxuni_mcllid_cfg0_FLD_rxuni_mcllid1, (reg32), (val))
#define e_rxuni_mcllid_cfg0_SET_rxuni_mcllid0(reg32, val)      REG_FLD_SET(e_rxuni_mcllid_cfg0_FLD_rxuni_mcllid0, (reg32), (val))

#define e_rxuni_mcllid_cfg1_SET_rxuni_mcllid3(reg32, val)      REG_FLD_SET(e_rxuni_mcllid_cfg1_FLD_rxuni_mcllid3, (reg32), (val))
#define e_rxuni_mcllid_cfg1_SET_rxuni_mcllid2(reg32, val)      REG_FLD_SET(e_rxuni_mcllid_cfg1_FLD_rxuni_mcllid2, (reg32), (val))

#define e_rxuni_mcllid_cfg2_SET_rxuni_mcllid5(reg32, val)      REG_FLD_SET(e_rxuni_mcllid_cfg2_FLD_rxuni_mcllid5, (reg32), (val))
#define e_rxuni_mcllid_cfg2_SET_rxuni_mcllid4(reg32, val)      REG_FLD_SET(e_rxuni_mcllid_cfg2_FLD_rxuni_mcllid4, (reg32), (val))

#define e_rxuni_mcllid_cfg3_SET_rxuni_mcllid7(reg32, val)      REG_FLD_SET(e_rxuni_mcllid_cfg3_FLD_rxuni_mcllid7, (reg32), (val))
#define e_rxuni_mcllid_cfg3_SET_rxuni_mcllid6(reg32, val)      REG_FLD_SET(e_rxuni_mcllid_cfg3_FLD_rxuni_mcllid6, (reg32), (val))

#define e_rxuni_mcllid_cfg4_SET_rxuni_mcllid9(reg32, val)      REG_FLD_SET(e_rxuni_mcllid_cfg4_FLD_rxuni_mcllid9, (reg32), (val))
#define e_rxuni_mcllid_cfg4_SET_rxuni_mcllid8(reg32, val)      REG_FLD_SET(e_rxuni_mcllid_cfg4_FLD_rxuni_mcllid8, (reg32), (val))

#define e_rxuni_mcllid_cfg5_SET_rxuni_mcllid11(reg32, val)     REG_FLD_SET(e_rxuni_mcllid_cfg5_FLD_rxuni_mcllid11, (reg32), (val))
#define e_rxuni_mcllid_cfg5_SET_rxuni_mcllid10(reg32, val)     REG_FLD_SET(e_rxuni_mcllid_cfg5_FLD_rxuni_mcllid10, (reg32), (val))

#define e_rxuni_mcllid_cfg6_SET_rxuni_mcllid13(reg32, val)     REG_FLD_SET(e_rxuni_mcllid_cfg6_FLD_rxuni_mcllid13, (reg32), (val))
#define e_rxuni_mcllid_cfg6_SET_rxuni_mcllid12(reg32, val)     REG_FLD_SET(e_rxuni_mcllid_cfg6_FLD_rxuni_mcllid12, (reg32), (val))

#define e_rxuni_mcllid_cfg7_SET_rxuni_mcllid15(reg32, val)     REG_FLD_SET(e_rxuni_mcllid_cfg7_FLD_rxuni_mcllid15, (reg32), (val))
#define e_rxuni_mcllid_cfg7_SET_rxuni_mcllid14(reg32, val)     REG_FLD_SET(e_rxuni_mcllid_cfg7_FLD_rxuni_mcllid14, (reg32), (val))

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

#define e_glue_cfg_SET_txmpi_fifound_pktgate_en(reg32, val)    REG_FLD_SET(e_glue_cfg_FLD_txmpi_fifound_pktgate_en, (reg32), (val))
#define e_glue_cfg_SET_txmpi_fifound_thr(reg32, val)           REG_FLD_SET(e_glue_cfg_FLD_txmpi_fifound_thr, (reg32), (val))

#define e_gntreq_tmout_SET_gntreq_grdcyc(reg32, val)           REG_FLD_SET(e_gntreq_tmout_FLD_gntreq_grdcyc, (reg32), (val))

#define e_glb_sts_SET_xepon_type(reg32, val)                   REG_FLD_SET(e_glb_sts_FLD_xepon_type, (reg32), (val))

#define e_cnt_clr_SET_glb_cntclr(reg32, val)                   REG_FLD_SET(e_cnt_clr_FLD_glb_cntclr, (reg32), (val))

#define e_rxadv_cnt_SET_rx_infomis_dscvgate_cnt(reg32, val)    REG_FLD_SET(e_rxadv_cnt_FLD_rx_infomis_dscvgate_cnt, (reg32), (val))
#define e_rxadv_cnt_SET_rx_dauc_dscvgate_cnt(reg32, val)       REG_FLD_SET(e_rxadv_cnt_FLD_rx_dauc_dscvgate_cnt, (reg32), (val))
#define e_rxadv_cnt_SET_rx_filt_dscvgate_cnt(reg32, val)       REG_FLD_SET(e_rxadv_cnt_FLD_rx_filt_dscvgate_cnt, (reg32), (val))

#define e_tx_dygasp_cnt_SET_txmbi_dygasp_cnt(reg32, val)       REG_FLD_SET(e_tx_dygasp_cnt_FLD_txmbi_dygasp_cnt, (reg32), (val))
#define e_tx_dygasp_cnt_SET_txmpi_dygasp_cnt(reg32, val)       REG_FLD_SET(e_tx_dygasp_cnt_FLD_txmpi_dygasp_cnt, (reg32), (val))

#define e_tx_rpt_cnt_SET_txmbi_rpt_cnt(reg32, val)             REG_FLD_SET(e_tx_rpt_cnt_FLD_txmbi_rpt_cnt, (reg32), (val))
#define e_tx_rpt_cnt_SET_txmpi_rpt_cnt(reg32, val)             REG_FLD_SET(e_tx_rpt_cnt_FLD_txmpi_rpt_cnt, (reg32), (val))

#define e_txmpi_mpcp_cnt_SET_txmpi_oam_cnt(reg32, val)         REG_FLD_SET(e_txmpi_mpcp_cnt_FLD_txmpi_oam_cnt, (reg32), (val))
#define e_txmpi_mpcp_cnt_SET_txmpi_rgreq_cnt(reg32, val)       REG_FLD_SET(e_txmpi_mpcp_cnt_FLD_txmpi_rgreq_cnt, (reg32), (val))
#define e_txmpi_mpcp_cnt_SET_txmpi_rgack_cnt(reg32, val)       REG_FLD_SET(e_txmpi_mpcp_cnt_FLD_txmpi_rgack_cnt, (reg32), (val))

#define e_rxmpi_uc_cnt_SET_rxmpi_uceth_cnt(reg32, val)         REG_FLD_SET(e_rxmpi_uc_cnt_FLD_rxmpi_uceth_cnt, (reg32), (val))

#define e_rxmpi_bc_cnt_SET_rxmpi_bceth_cnt(reg32, val)         REG_FLD_SET(e_rxmpi_bc_cnt_FLD_rxmpi_bceth_cnt, (reg32), (val))

#define e_rxmpi_mc_cnt_SET_rxmpi_mceth_cnt(reg32, val)         REG_FLD_SET(e_rxmpi_mc_cnt_FLD_rxmpi_mceth_cnt, (reg32), (val))

#define e_rxmpi_oam_cnt_SET_rxmpi_oam_cnt(reg32, val)          REG_FLD_SET(e_rxmpi_oam_cnt_FLD_rxmpi_oam_cnt, (reg32), (val))

#define e_rxmpi_mpcp_cnt_SET_rxmpi_mpcp_cnt(reg32, val)        REG_FLD_SET(e_rxmpi_mpcp_cnt_FLD_rxmpi_mpcp_cnt, (reg32), (val))

#define e_rxmpi_gate_cnt_SET_rxmpi_gate_cnt(reg32, val)        REG_FLD_SET(e_rxmpi_gate_cnt_FLD_rxmpi_gate_cnt, (reg32), (val))

#define e_rxmpi_nrlgate_cnt_SET_rxmpi_nrlgate_cnt(reg32, val)  REG_FLD_SET(e_rxmpi_nrlgate_cnt_FLD_rxmpi_nrlgate_cnt, (reg32), (val))

#define e_rxmpi_crc8err_cnt_SET_rxmpi_crceth_cnt(reg32, val)   REG_FLD_SET(e_rxmpi_crc8err_cnt_FLD_rxmpi_crceth_cnt, (reg32), (val))
#define e_rxmpi_crc8err_cnt_SET_rxmpi_crc8err_cnt(reg32, val)  REG_FLD_SET(e_rxmpi_crc8err_cnt_FLD_rxmpi_crc8err_cnt, (reg32), (val))

#define e_rxmpi_drop_cnt_SET_rxmpi_eofdrop_cnt(reg32, val)     REG_FLD_SET(e_rxmpi_drop_cnt_FLD_rxmpi_eofdrop_cnt, (reg32), (val))

#define e_rxmpi_churn_cnt_SET_rxmpi_abchurn_cnt(reg32, val)    REG_FLD_SET(e_rxmpi_churn_cnt_FLD_rxmpi_abchurn_cnt, (reg32), (val))
#define e_rxmpi_churn_cnt_SET_rxmpi_churn_cnt(reg32, val)      REG_FLD_SET(e_rxmpi_churn_cnt_FLD_rxmpi_churn_cnt, (reg32), (val))

#define e_rxmbi_snf_cnt_SET_rxmbi_snf_cnt(reg32, val)          REG_FLD_SET(e_rxmbi_snf_cnt_FLD_rxmbi_snf_cnt, (reg32), (val))

#define e_rxmbi_drop_cnt_SET_rxmbi_sofdrop_cnt(reg32, val)     REG_FLD_SET(e_rxmbi_drop_cnt_FLD_rxmbi_sofdrop_cnt, (reg32), (val))
#define e_rxmbi_drop_cnt_SET_rxmbi_snfdrop_cnt(reg32, val)     REG_FLD_SET(e_rxmbi_drop_cnt_FLD_rxmbi_snfdrop_cnt, (reg32), (val))

#define e_rxmbi_drop_cnt2_SET_rxmbi_crcerr_cnt(reg32, val)     REG_FLD_SET(e_rxmbi_drop_cnt2_FLD_rxmbi_crcerr_cnt, (reg32), (val))
#define e_rxmbi_drop_cnt2_SET_rxmbi_enddrop_cnt(reg32, val)    REG_FLD_SET(e_rxmbi_drop_cnt2_FLD_rxmbi_enddrop_cnt, (reg32), (val))

#define e_rxmbi_bytecnt_h_SET_rxmbi_bytecnt_h(reg32, val)      REG_FLD_SET(e_rxmbi_bytecnt_h_FLD_rxmbi_bytecnt_h, (reg32), (val))

#define e_rxmbi_bytecnt_l_SET_rxmbi_bytecnt_l(reg32, val)      REG_FLD_SET(e_rxmbi_bytecnt_l_FLD_rxmbi_bytecnt_l, (reg32), (val))

#define e_txmbi_uceth_cnt_SET_txmbi_uceth_cnt(reg32, val)      REG_FLD_SET(e_txmbi_uceth_cnt_FLD_txmbi_uceth_cnt, (reg32), (val))

#define e_txmbi_mceth_cnt_SET_txmbi_bceth_cnt(reg32, val)      REG_FLD_SET(e_txmbi_mceth_cnt_FLD_txmbi_bceth_cnt, (reg32), (val))
#define e_txmbi_mceth_cnt_SET_txmbi_mceth_cnt(reg32, val)      REG_FLD_SET(e_txmbi_mceth_cnt_FLD_txmbi_mceth_cnt, (reg32), (val))

#define e_txmbi_err_cnt_SET_txmbi_err_cnt(reg32, val)          REG_FLD_SET(e_txmbi_err_cnt_FLD_txmbi_err_cnt, (reg32), (val))

#define e_tx_timedrift_stat_SET_tx_cur_time_drift_ofst(reg32, val) REG_FLD_SET(e_tx_timedrift_stat_FLD_tx_cur_time_drift_ofst, (reg32), (val))
#define e_tx_timedrift_stat_SET_tx_max_time_drift_ofst(reg32, val) REG_FLD_SET(e_tx_timedrift_stat_FLD_tx_max_time_drift_ofst, (reg32), (val))
#define e_tx_timedrift_stat_SET_tx_cur_time_drift(reg32, val)  REG_FLD_SET(e_tx_timedrift_stat_FLD_tx_cur_time_drift, (reg32), (val))
#define e_tx_timedrift_stat_SET_tx_max_time_drift(reg32, val)  REG_FLD_SET(e_tx_timedrift_stat_FLD_tx_max_time_drift, (reg32), (val))

#define e_rxmpi_churn_cnt2_SET_rxmpi_churn_mpcpcnt(reg32, val) REG_FLD_SET(e_rxmpi_churn_cnt2_FLD_rxmpi_churn_mpcpcnt, (reg32), (val))
#define e_rxmpi_churn_cnt2_SET_rxmpi_churn_ethcnt(reg32, val)  REG_FLD_SET(e_rxmpi_churn_cnt2_FLD_rxmpi_churn_ethcnt, (reg32), (val))

#define e_rxmpi_churn_cnt3_SET_rxmpi_churn_keyidx(reg32, val)  REG_FLD_SET(e_rxmpi_churn_cnt3_FLD_rxmpi_churn_keyidx, (reg32), (val))
#define e_rxmpi_churn_cnt3_SET_rxmpi_churn_llididx(reg32, val) REG_FLD_SET(e_rxmpi_churn_cnt3_FLD_rxmpi_churn_llididx, (reg32), (val))
#define e_rxmpi_churn_cnt3_SET_rxmpi_churncrc_keyidx(reg32, val) REG_FLD_SET(e_rxmpi_churn_cnt3_FLD_rxmpi_churncrc_keyidx, (reg32), (val))
#define e_rxmpi_churn_cnt3_SET_rxmpi_churncrc_llididx(reg32, val) REG_FLD_SET(e_rxmpi_churn_cnt3_FLD_rxmpi_churncrc_llididx, (reg32), (val))
#define e_rxmpi_churn_cnt3_SET_rxmpi_churn_crccnt(reg32, val)  REG_FLD_SET(e_rxmpi_churn_cnt3_FLD_rxmpi_churn_crccnt, (reg32), (val))

#define e_txmpi_uceth_cnt_SET_txmpi_uceth_cnt(reg32, val)      REG_FLD_SET(e_txmpi_uceth_cnt_FLD_txmpi_uceth_cnt, (reg32), (val))

#define e_txmpi_mceth_cnt_SET_txmpi_bceth_cnt(reg32, val)      REG_FLD_SET(e_txmpi_mceth_cnt_FLD_txmpi_bceth_cnt, (reg32), (val))
#define e_txmpi_mceth_cnt_SET_txmpi_mceth_cnt(reg32, val)      REG_FLD_SET(e_txmpi_mceth_cnt_FLD_txmpi_mceth_cnt, (reg32), (val))

#define e_llid8_9_gnt_stat_SET_llid9_gnt_cnt(reg32, val)       REG_FLD_SET(e_llid8_9_gnt_stat_FLD_llid9_gnt_cnt, (reg32), (val))
#define e_llid8_9_gnt_stat_SET_llid8_gnt_cnt(reg32, val)       REG_FLD_SET(e_llid8_9_gnt_stat_FLD_llid8_gnt_cnt, (reg32), (val))

#define e_llid10_11_gnt_stat_SET_llid11_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid10_11_gnt_stat_FLD_llid11_gnt_cnt, (reg32), (val))
#define e_llid10_11_gnt_stat_SET_llid10_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid10_11_gnt_stat_FLD_llid10_gnt_cnt, (reg32), (val))

#define e_llid12_13_gnt_stat_SET_llid13_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid12_13_gnt_stat_FLD_llid13_gnt_cnt, (reg32), (val))
#define e_llid12_13_gnt_stat_SET_llid12_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid12_13_gnt_stat_FLD_llid12_gnt_cnt, (reg32), (val))

#define e_llid14_15_gnt_stat_SET_llid15_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid14_15_gnt_stat_FLD_llid15_gnt_cnt, (reg32), (val))
#define e_llid14_15_gnt_stat_SET_llid14_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid14_15_gnt_stat_FLD_llid14_gnt_cnt, (reg32), (val))

#define e_llid16_17_gnt_stat_SET_llid17_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid16_17_gnt_stat_FLD_llid17_gnt_cnt, (reg32), (val))
#define e_llid16_17_gnt_stat_SET_llid16_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid16_17_gnt_stat_FLD_llid16_gnt_cnt, (reg32), (val))

#define e_llid18_19_gnt_stat_SET_llid19_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid18_19_gnt_stat_FLD_llid19_gnt_cnt, (reg32), (val))
#define e_llid18_19_gnt_stat_SET_llid18_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid18_19_gnt_stat_FLD_llid18_gnt_cnt, (reg32), (val))

#define e_llid20_21_gnt_stat_SET_llid21_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid20_21_gnt_stat_FLD_llid21_gnt_cnt, (reg32), (val))
#define e_llid20_21_gnt_stat_SET_llid20_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid20_21_gnt_stat_FLD_llid20_gnt_cnt, (reg32), (val))

#define e_llid22_23_gnt_stat_SET_llid23_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid22_23_gnt_stat_FLD_llid23_gnt_cnt, (reg32), (val))
#define e_llid22_23_gnt_stat_SET_llid22_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid22_23_gnt_stat_FLD_llid22_gnt_cnt, (reg32), (val))

#define e_llid24_25_gnt_stat_SET_llid25_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid24_25_gnt_stat_FLD_llid25_gnt_cnt, (reg32), (val))
#define e_llid24_25_gnt_stat_SET_llid24_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid24_25_gnt_stat_FLD_llid24_gnt_cnt, (reg32), (val))

#define e_llid26_27_gnt_stat_SET_llid27_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid26_27_gnt_stat_FLD_llid27_gnt_cnt, (reg32), (val))
#define e_llid26_27_gnt_stat_SET_llid26_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid26_27_gnt_stat_FLD_llid26_gnt_cnt, (reg32), (val))

#define e_llid28_29_gnt_stat_SET_llid29_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid28_29_gnt_stat_FLD_llid29_gnt_cnt, (reg32), (val))
#define e_llid28_29_gnt_stat_SET_llid28_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid28_29_gnt_stat_FLD_llid28_gnt_cnt, (reg32), (val))

#define e_llid30_31_gnt_stat_SET_llid31_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid30_31_gnt_stat_FLD_llid31_gnt_cnt, (reg32), (val))
#define e_llid30_31_gnt_stat_SET_llid30_gnt_cnt(reg32, val)    REG_FLD_SET(e_llid30_31_gnt_stat_FLD_llid30_gnt_cnt, (reg32), (val))

#define e_rxfifo_depth_stat_SET_max_rxfifo_depth(reg32, val)   REG_FLD_SET(e_rxfifo_depth_stat_FLD_max_rxfifo_depth, (reg32), (val))

#define e_rdmdly_stat_SET_rdmdly(reg32, val)                   REG_FLD_SET(e_rdmdly_stat_FLD_rdmdly, (reg32), (val))

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

#define e_rpt_mpcp_timout2_SET_llid31_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid31_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid30_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid30_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid29_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid29_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid28_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid28_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid27_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid27_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid26_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid26_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid25_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid25_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid24_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid24_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid23_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid23_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid22_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid22_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid21_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid21_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid20_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid20_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid19_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid19_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid18_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid18_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid17_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid17_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid16_mpcp_tmo(reg32, val)     REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid16_mpcp_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid31_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid31_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid30_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid30_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid29_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid29_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid28_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid28_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid27_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid27_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid26_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid26_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid25_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid25_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid24_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid24_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid23_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid23_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid22_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid22_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid21_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid21_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid20_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid20_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid19_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid19_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid18_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid18_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid17_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid17_rpt_tmo, (reg32), (val))
#define e_rpt_mpcp_timout2_SET_llid16_rpt_tmo(reg32, val)      REG_FLD_SET(e_rpt_mpcp_timout2_FLD_llid16_rpt_tmo, (reg32), (val))

#define e_llid8_11_cfg_SET_llid11_dummy(reg32, val)            REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_dummy, (reg32), (val))
#define e_llid8_11_cfg_SET_llid11_txfec_en(reg32, val)         REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_txfec_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid11_dcrypt_en(reg32, val)        REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_dcrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid11_dcrypt_mode(reg32, val)      REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_dcrypt_mode, (reg32), (val))
#define e_llid8_11_cfg_SET_llid11_oamlpbk_en(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid11_oamlpbk_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_dummy(reg32, val)            REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_dummy, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_txfec_en(reg32, val)         REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_txfec_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_dcrypt_en(reg32, val)        REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_dcrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_dcrypt_mode(reg32, val)      REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_dcrypt_mode, (reg32), (val))
#define e_llid8_11_cfg_SET_llid10_oamlpbk_en(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid10_oamlpbk_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_dummy(reg32, val)             REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_dummy, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_txfec_en(reg32, val)          REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_txfec_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_dcrypt_en(reg32, val)         REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_dcrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_dcrypt_mode(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_dcrypt_mode, (reg32), (val))
#define e_llid8_11_cfg_SET_llid9_oamlpbk_en(reg32, val)        REG_FLD_SET(e_llid8_11_cfg_FLD_llid9_oamlpbk_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_dummy(reg32, val)             REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_dummy, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_txfec_en(reg32, val)          REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_txfec_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_dcrypt_en(reg32, val)         REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_dcrypt_en, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_dcrypt_mode(reg32, val)       REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_dcrypt_mode, (reg32), (val))
#define e_llid8_11_cfg_SET_llid8_oamlpbk_en(reg32, val)        REG_FLD_SET(e_llid8_11_cfg_FLD_llid8_oamlpbk_en, (reg32), (val))

#define e_llid12_15_cfg_SET_llid15_dummy(reg32, val)           REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_dummy, (reg32), (val))
#define e_llid12_15_cfg_SET_llid15_txfec_en(reg32, val)        REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_txfec_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid15_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_dcrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid15_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_dcrypt_mode, (reg32), (val))
#define e_llid12_15_cfg_SET_llid15_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid12_15_cfg_FLD_llid15_oamlpbk_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_dummy(reg32, val)           REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_dummy, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_txfec_en(reg32, val)        REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_txfec_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_dcrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_dcrypt_mode, (reg32), (val))
#define e_llid12_15_cfg_SET_llid14_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid12_15_cfg_FLD_llid14_oamlpbk_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_dummy(reg32, val)           REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_dummy, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_txfec_en(reg32, val)        REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_txfec_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_dcrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_dcrypt_mode, (reg32), (val))
#define e_llid12_15_cfg_SET_llid13_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid12_15_cfg_FLD_llid13_oamlpbk_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_dummy(reg32, val)           REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_dummy, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_txfec_en(reg32, val)        REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_txfec_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_dcrypt_en, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_dcrypt_mode, (reg32), (val))
#define e_llid12_15_cfg_SET_llid12_oam_lpbk_en(reg32, val)     REG_FLD_SET(e_llid12_15_cfg_FLD_llid12_oam_lpbk_en, (reg32), (val))

#define e_llid16_19_cfg_SET_llid19_dummy(reg32, val)           REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_dummy, (reg32), (val))
#define e_llid16_19_cfg_SET_llid19_txfec_en(reg32, val)        REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_txfec_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid19_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_dcrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid19_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_dcrypt_mode, (reg32), (val))
#define e_llid16_19_cfg_SET_llid19_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid16_19_cfg_FLD_llid19_oamlpbk_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_dummy(reg32, val)           REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_dummy, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_txfec_en(reg32, val)        REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_txfec_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_dcrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_dcrypt_mode, (reg32), (val))
#define e_llid16_19_cfg_SET_llid18_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid16_19_cfg_FLD_llid18_oamlpbk_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_dummy(reg32, val)           REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_dummy, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_txfec_en(reg32, val)        REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_txfec_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_dcrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_dcrypt_mode, (reg32), (val))
#define e_llid16_19_cfg_SET_llid17_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid16_19_cfg_FLD_llid17_oamlpbk_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_dummy(reg32, val)           REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_dummy, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_txfec_en(reg32, val)        REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_txfec_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_dcrypt_en, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_dcrypt_mode, (reg32), (val))
#define e_llid16_19_cfg_SET_llid16_oam_lpbk_en(reg32, val)     REG_FLD_SET(e_llid16_19_cfg_FLD_llid16_oam_lpbk_en, (reg32), (val))

#define e_llid20_23_cfg_SET_llid23_dummy(reg32, val)           REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_dummy, (reg32), (val))
#define e_llid20_23_cfg_SET_llid23_txfec_en(reg32, val)        REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_txfec_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid23_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_dcrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid23_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_dcrypt_mode, (reg32), (val))
#define e_llid20_23_cfg_SET_llid23_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid20_23_cfg_FLD_llid23_oamlpbk_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_dummy(reg32, val)           REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_dummy, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_txfec_en(reg32, val)        REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_txfec_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_dcrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_dcrypt_mode, (reg32), (val))
#define e_llid20_23_cfg_SET_llid22_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid20_23_cfg_FLD_llid22_oamlpbk_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_dummy(reg32, val)           REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_dummy, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_txfec_en(reg32, val)        REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_txfec_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_dcrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_dcrypt_mode, (reg32), (val))
#define e_llid20_23_cfg_SET_llid21_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid20_23_cfg_FLD_llid21_oamlpbk_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_dummy(reg32, val)           REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_dummy, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_txfec_en(reg32, val)        REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_txfec_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_dcrypt_en, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_dcrypt_mode, (reg32), (val))
#define e_llid20_23_cfg_SET_llid20_oam_lpbk_en(reg32, val)     REG_FLD_SET(e_llid20_23_cfg_FLD_llid20_oam_lpbk_en, (reg32), (val))

#define e_llid24_27_cfg_SET_llid27_dummy(reg32, val)           REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_dummy, (reg32), (val))
#define e_llid24_27_cfg_SET_llid27_txfec_en(reg32, val)        REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_txfec_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid27_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_dcrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid27_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_dcrypt_mode, (reg32), (val))
#define e_llid24_27_cfg_SET_llid27_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid24_27_cfg_FLD_llid27_oamlpbk_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_dummy(reg32, val)           REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_dummy, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_txfec_en(reg32, val)        REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_txfec_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_dcrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_dcrypt_mode, (reg32), (val))
#define e_llid24_27_cfg_SET_llid26_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid24_27_cfg_FLD_llid26_oamlpbk_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_dummy(reg32, val)           REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_dummy, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_txfec_en(reg32, val)        REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_txfec_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_dcrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_dcrypt_mode, (reg32), (val))
#define e_llid24_27_cfg_SET_llid25_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid24_27_cfg_FLD_llid25_oamlpbk_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_dummy(reg32, val)           REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_dummy, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_txfec_en(reg32, val)        REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_txfec_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_dcrypt_en, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_dcrypt_mode, (reg32), (val))
#define e_llid24_27_cfg_SET_llid24_oam_lpbk_en(reg32, val)     REG_FLD_SET(e_llid24_27_cfg_FLD_llid24_oam_lpbk_en, (reg32), (val))

#define e_llid28_31_cfg_SET_llid31_dummy(reg32, val)           REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_dummy, (reg32), (val))
#define e_llid28_31_cfg_SET_llid31_txfec_en(reg32, val)        REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_txfec_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid31_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_dcrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid31_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_dcrypt_mode, (reg32), (val))
#define e_llid28_31_cfg_SET_llid31_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid28_31_cfg_FLD_llid31_oamlpbk_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_dummy(reg32, val)           REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_dummy, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_txfec_en(reg32, val)        REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_txfec_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_dcrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_dcrypt_mode, (reg32), (val))
#define e_llid28_31_cfg_SET_llid30_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid28_31_cfg_FLD_llid30_oamlpbk_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_dummy(reg32, val)           REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_dummy, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_txfec_en(reg32, val)        REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_txfec_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_dcrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_dcrypt_mode, (reg32), (val))
#define e_llid28_31_cfg_SET_llid29_oamlpbk_en(reg32, val)      REG_FLD_SET(e_llid28_31_cfg_FLD_llid29_oamlpbk_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_dummy(reg32, val)           REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_dummy, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_txfec_en(reg32, val)        REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_txfec_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_dcrypt_en(reg32, val)       REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_dcrypt_en, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_dcrypt_mode(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_dcrypt_mode, (reg32), (val))
#define e_llid28_31_cfg_SET_llid28_oam_lpbk_en(reg32, val)     REG_FLD_SET(e_llid28_31_cfg_FLD_llid28_oam_lpbk_en, (reg32), (val))

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

#define e_rpt_cfg2_SET_llid15_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid15_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid14_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid14_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid13_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid13_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid12_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid12_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid11_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid11_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid10_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg2_FLD_llid10_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid9_rpt_cfg(reg32, val)               REG_FLD_SET(e_rpt_cfg2_FLD_llid9_rpt_cfg, (reg32), (val))
#define e_rpt_cfg2_SET_llid8_rpt_cfg(reg32, val)               REG_FLD_SET(e_rpt_cfg2_FLD_llid8_rpt_cfg, (reg32), (val))

#define e_rpt_cfg3_SET_llid31_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid31_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid30_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid30_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid29_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid29_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid28_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid28_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid27_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid27_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid26_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid26_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid25_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid25_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid24_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid24_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid23_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid23_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid22_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid22_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid21_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid21_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid20_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid20_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid19_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid19_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid18_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid18_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid17_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid17_rpt_cfg, (reg32), (val))
#define e_rpt_cfg3_SET_llid16_rpt_cfg(reg32, val)              REG_FLD_SET(e_rpt_cfg3_FLD_llid16_rpt_cfg, (reg32), (val))

#define e_rpt_qthld_cfg2_SET_rpt_llid_idx_4_3(reg32, val)      REG_FLD_SET(e_rpt_qthld_cfg2_FLD_rpt_llid_idx_4_3, (reg32), (val))

#define e_rpt_bitmap_cfg_SET_rpt_bmap_rwcmd(reg32, val)        REG_FLD_SET(e_rpt_bitmap_cfg_FLD_rpt_bmap_rwcmd, (reg32), (val))
#define e_rpt_bitmap_cfg_SET_rpt_bmap_rwcmd_done(reg32, val)   REG_FLD_SET(e_rpt_bitmap_cfg_FLD_rpt_bmap_rwcmd_done, (reg32), (val))
#define e_rpt_bitmap_cfg_SET_rpt_bmap_llid_idx(reg32, val)     REG_FLD_SET(e_rpt_bitmap_cfg_FLD_rpt_bmap_llid_idx, (reg32), (val))

#define e_rpt_bitmap_val_SET_rpt_qsizeadj_frc(reg32, val)      REG_FLD_SET(e_rpt_bitmap_val_FLD_rpt_qsizeadj_frc, (reg32), (val))
#define e_rpt_bitmap_val_SET_rpt_bitmap_ctrl(reg32, val)       REG_FLD_SET(e_rpt_bitmap_val_FLD_rpt_bitmap_ctrl, (reg32), (val))
#define e_rpt_bitmap_val_SET_rpt_bitmap_set(reg32, val)        REG_FLD_SET(e_rpt_bitmap_val_FLD_rpt_bitmap_set, (reg32), (val))

#define e_glb_cfg_VAL_sniffer_mode(val)                        REG_FLD_VAL(e_glb_cfg_FLD_sniffer_mode, (val))
#define e_glb_cfg_VAL_txoam_favor(val)                         REG_FLD_VAL(e_glb_cfg_FLD_txoam_favor, (val))
#define e_glb_cfg_VAL_burst_en_dly(val)                        REG_FLD_VAL(e_glb_cfg_FLD_burst_en_dly, (val))
#define e_glb_cfg_VAL_discv_burst_en(val)                      REG_FLD_VAL(e_glb_cfg_FLD_discv_burst_en, (val))
#define e_glb_cfg_VAL_mpcp_fwd(val)                            REG_FLD_VAL(e_glb_cfg_FLD_mpcp_fwd, (val))
#define e_glb_cfg_VAL_bcst_llid_m1_drop(val)                   REG_FLD_VAL(e_glb_cfg_FLD_bcst_llid_m1_drop, (val))
#define e_glb_cfg_VAL_bcst_llid_m0_drop(val)                   REG_FLD_VAL(e_glb_cfg_FLD_bcst_llid_m0_drop, (val))
#define e_glb_cfg_VAL_mcst_llid_drop(val)                      REG_FLD_VAL(e_glb_cfg_FLD_mcst_llid_drop, (val))
#define e_glb_cfg_VAL_all_unicast_llid_pkt_fwd(val)            REG_FLD_VAL(e_glb_cfg_FLD_all_unicast_llid_pkt_fwd, (val))
#define e_glb_cfg_VAL_fcs_err_fwd(val)                         REG_FLD_VAL(e_glb_cfg_FLD_fcs_err_fwd, (val))
#define e_glb_cfg_VAL_llid_crc8_err_fwd(val)                   REG_FLD_VAL(e_glb_cfg_FLD_llid_crc8_err_fwd, (val))
#define e_glb_cfg_VAL_rxmpi_stop(val)                          REG_FLD_VAL(e_glb_cfg_FLD_rxmpi_stop, (val))
#define e_glb_cfg_VAL_txmpi_stop(val)                          REG_FLD_VAL(e_glb_cfg_FLD_txmpi_stop, (val))
#define e_glb_cfg_VAL_phy_pwr_down(val)                        REG_FLD_VAL(e_glb_cfg_FLD_phy_pwr_down, (val))
#define e_glb_cfg_VAL_rx_nml_gate_fwd(val)                     REG_FLD_VAL(e_glb_cfg_FLD_rx_nml_gate_fwd, (val))
#define e_glb_cfg_VAL_rxmbi_stop(val)                          REG_FLD_VAL(e_glb_cfg_FLD_rxmbi_stop, (val))
#define e_glb_cfg_VAL_txmbi_stop(val)                          REG_FLD_VAL(e_glb_cfg_FLD_txmbi_stop, (val))
#define e_glb_cfg_VAL_chk_all_gnt_mode(val)                    REG_FLD_VAL(e_glb_cfg_FLD_chk_all_gnt_mode, (val))
#define e_glb_cfg_VAL_tx_default_rpt(val)                      REG_FLD_VAL(e_glb_cfg_FLD_tx_default_rpt, (val))
#define e_glb_cfg_VAL_epon_mac_sw_rst(val)                     REG_FLD_VAL(e_glb_cfg_FLD_epon_mac_sw_rst, (val))
#define e_glb_cfg_VAL_epon_oam_cal_in_eth(val)                 REG_FLD_VAL(e_glb_cfg_FLD_epon_oam_cal_in_eth, (val))
#define e_glb_cfg_VAL_epon_mac_lpbk_en(val)                    REG_FLD_VAL(e_glb_cfg_FLD_epon_mac_lpbk_en, (val))
#define e_glb_cfg_VAL_rpt_txpri_ctrl(val)                      REG_FLD_VAL(e_glb_cfg_FLD_rpt_txpri_ctrl, (val))

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

#define e_rpt_mpcp_timeout_llid_idx_VAL_llid15_mpcp_tmo(val)   REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid15_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid14_mpcp_tmo(val)   REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid14_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid13_mpcp_tmo(val)   REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid13_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid12_mpcp_tmo(val)   REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid12_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid11_mpcp_tmo(val)   REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid11_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid10_mpcp_tmo(val)   REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid10_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid9_mpcp_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid9_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid8_mpcp_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid8_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid15_rpt_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid15_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid14_rpt_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid14_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid13_rpt_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid13_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid12_rpt_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid12_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid11_rpt_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid11_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid10_rpt_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid10_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid9_rpt_tmo(val)     REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid9_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid8_rpt_tmo(val)     REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid8_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid7_mpcp_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid7_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid6_mpcp_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid6_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid5_mpcp_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid5_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid4_mpcp_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid4_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid3_mpcp_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid3_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid2_mpcp_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid2_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid1_mpcp_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid1_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid0_mpcp_tmo(val)    REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid0_mpcp_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid7_rpt_tmo(val)     REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid7_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid6_rpt_tmo(val)     REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid6_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid5_rpt_tmo(val)     REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid5_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid4_rpt_tmo(val)     REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid4_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid3_rpt_tmo(val)     REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid3_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid2_rpt_tmo(val)     REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid2_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid1_rpt_tmo(val)     REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid1_rpt_tmo, (val))
#define e_rpt_mpcp_timeout_llid_idx_VAL_llid0_rpt_tmo(val)     REG_FLD_VAL(e_rpt_mpcp_timeout_llid_idx_FLD_llid0_rpt_tmo, (val))

#define e_dyinggsp_cfg_VAL_hw_dying_gasp_en(val)               REG_FLD_VAL(e_dyinggsp_cfg_FLD_hw_dying_gasp_en, (val))
#define e_dyinggsp_cfg_VAL_sw_init_dying_gasp(val)             REG_FLD_VAL(e_dyinggsp_cfg_FLD_sw_init_dying_gasp, (val))
#define e_dyinggsp_cfg_VAL_dygsp_num_of_times(val)             REG_FLD_VAL(e_dyinggsp_cfg_FLD_dygsp_num_of_times, (val))

#define e_pending_gnt_num_VAL_echoed_pending_gnt(val)          REG_FLD_VAL(e_pending_gnt_num_FLD_echoed_pending_gnt, (val))
#define e_pending_gnt_num_VAL_pending_gnt_num(val)             REG_FLD_VAL(e_pending_gnt_num_FLD_pending_gnt_num, (val))

#define e_llid0_3_cfg_VAL_llid3_dummy(val)                     REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_dummy, (val))
#define e_llid0_3_cfg_VAL_llid3_txfec_en(val)                  REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_txfec_en, (val))
#define e_llid0_3_cfg_VAL_llid3_dcrypt_en(val)                 REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_dcrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid3_dcrypt_mode(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_dcrypt_mode, (val))
#define e_llid0_3_cfg_VAL_llid3_oam_lpbk_en(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid3_oam_lpbk_en, (val))
#define e_llid0_3_cfg_VAL_llid2_dummy(val)                     REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_dummy, (val))
#define e_llid0_3_cfg_VAL_llid2_txfec_en(val)                  REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_txfec_en, (val))
#define e_llid0_3_cfg_VAL_llid2_dcrypt_en(val)                 REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_dcrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid2_dcrypt_mode(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_dcrypt_mode, (val))
#define e_llid0_3_cfg_VAL_llid2_oam_lpbk_en(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid2_oam_lpbk_en, (val))
#define e_llid0_3_cfg_VAL_llid1_dummy(val)                     REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_dummy, (val))
#define e_llid0_3_cfg_VAL_llid1_txfec_en(val)                  REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_txfec_en, (val))
#define e_llid0_3_cfg_VAL_llid1_dcrypt_en(val)                 REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_dcrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid1_dcrypt_mode(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_dcrypt_mode, (val))
#define e_llid0_3_cfg_VAL_llid1_oam_lpbk_en(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid1_oam_lpbk_en, (val))
#define e_llid0_3_cfg_VAL_llid0_dummy(val)                     REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_dummy, (val))
#define e_llid0_3_cfg_VAL_llid0_txfec_en(val)                  REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_txfec_en, (val))
#define e_llid0_3_cfg_VAL_llid0_dcrypt_en(val)                 REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_dcrypt_en, (val))
#define e_llid0_3_cfg_VAL_llid0_dcrypt_mode(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_dcrypt_mode, (val))
#define e_llid0_3_cfg_VAL_llid0_oam_lpbk_en(val)               REG_FLD_VAL(e_llid0_3_cfg_FLD_llid0_oam_lpbk_en, (val))

#define e_llid4_7_cfg_VAL_llid7_dummy(val)                     REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_dummy, (val))
#define e_llid4_7_cfg_VAL_llid7_txfec_en(val)                  REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_txfec_en, (val))
#define e_llid4_7_cfg_VAL_llid7_dcrypt_en(val)                 REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_dcrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid7_dcrypt_mode(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_dcrypt_mode, (val))
#define e_llid4_7_cfg_VAL_llid7_oam_lpbk_en(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid7_oam_lpbk_en, (val))
#define e_llid4_7_cfg_VAL_llid6_dummy(val)                     REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_dummy, (val))
#define e_llid4_7_cfg_VAL_llid6_txfec_en(val)                  REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_txfec_en, (val))
#define e_llid4_7_cfg_VAL_llid6_dcrypt_en(val)                 REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_dcrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid6_dcrypt_mode(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_dcrypt_mode, (val))
#define e_llid4_7_cfg_VAL_llid6_oam_lpbk_en(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid6_oam_lpbk_en, (val))
#define e_llid4_7_cfg_VAL_llid5_dummy(val)                     REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_dummy, (val))
#define e_llid4_7_cfg_VAL_llid5_txfec_en(val)                  REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_txfec_en, (val))
#define e_llid4_7_cfg_VAL_llid5_dcrypt_en(val)                 REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_dcrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid5_dcrypt_mode(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_dcrypt_mode, (val))
#define e_llid4_7_cfg_VAL_llid5_oam_lpbk_en(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid5_oam_lpbk_en, (val))
#define e_llid4_7_cfg_VAL_llid4_dummy(val)                     REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_dummy, (val))
#define e_llid4_7_cfg_VAL_llid4_txfec_en(val)                  REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_txfec_en, (val))
#define e_llid4_7_cfg_VAL_llid4_dcrypt_en(val)                 REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_dcrypt_en, (val))
#define e_llid4_7_cfg_VAL_llid4_dcrypt_mode(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_dcrypt_mode, (val))
#define e_llid4_7_cfg_VAL_llid4_oam_lpbk_en(val)               REG_FLD_VAL(e_llid4_7_cfg_FLD_llid4_oam_lpbk_en, (val))

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

#define e_mac_addr_cfg_VAL_mac_addr_rwcmd(val)                 REG_FLD_VAL(e_mac_addr_cfg_FLD_mac_addr_rwcmd, (val))
#define e_mac_addr_cfg_VAL_mac_addr_rwcmd_done(val)            REG_FLD_VAL(e_mac_addr_cfg_FLD_mac_addr_rwcmd_done, (val))
#define e_mac_addr_cfg_VAL_mac_addr_llid_indx(val)             REG_FLD_VAL(e_mac_addr_cfg_FLD_mac_addr_llid_indx, (val))
#define e_mac_addr_cfg_VAL_mac_addr_dw_idx(val)                REG_FLD_VAL(e_mac_addr_cfg_FLD_mac_addr_dw_idx, (val))

#define e_mac_addr_value_VAL_mac_addr_value(val)               REG_FLD_VAL(e_mac_addr_value_FLD_mac_addr_value, (val))

#define e_security_key_cfg_VAL_key_rwcmd(val)                  REG_FLD_VAL(e_security_key_cfg_FLD_key_rwcmd, (val))
#define e_security_key_cfg_VAL_key_rwcmd_done(val)             REG_FLD_VAL(e_security_key_cfg_FLD_key_rwcmd_done, (val))
#define e_security_key_cfg_VAL_key_llid_index(val)             REG_FLD_VAL(e_security_key_cfg_FLD_key_llid_index, (val))
#define e_security_key_cfg_VAL_key_idx(val)                    REG_FLD_VAL(e_security_key_cfg_FLD_key_idx, (val))
#define e_security_key_cfg_VAL_key_dw_indx(val)                REG_FLD_VAL(e_security_key_cfg_FLD_key_dw_indx, (val))

#define e_key_value_VAL_key_value(val)                         REG_FLD_VAL(e_key_value_FLD_key_value, (val))

#define e_rpt_cfg_VAL_llid7_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid7_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid6_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid6_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid5_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid5_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid4_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid4_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid3_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid3_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid2_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid2_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid1_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid1_rpt_cfg, (val))
#define e_rpt_cfg_VAL_llid0_rpt_cfg(val)                       REG_FLD_VAL(e_rpt_cfg_FLD_llid0_rpt_cfg, (val))

#define e_rpt_qthld_cfg_VAL_qthld_rwcmd(val)                   REG_FLD_VAL(e_rpt_qthld_cfg_FLD_qthld_rwcmd, (val))
#define e_rpt_qthld_cfg_VAL_qthld_rwcmd_done(val)              REG_FLD_VAL(e_rpt_qthld_cfg_FLD_qthld_rwcmd_done, (val))
#define e_rpt_qthld_cfg_VAL_qthld_value(val)                   REG_FLD_VAL(e_rpt_qthld_cfg_FLD_qthld_value, (val))
#define e_rpt_qthld_cfg_VAL_qthld_idx(val)                     REG_FLD_VAL(e_rpt_qthld_cfg_FLD_qthld_idx, (val))
#define e_rpt_qthld_cfg_VAL_rpt_llid_idx_2_0(val)              REG_FLD_VAL(e_rpt_qthld_cfg_FLD_rpt_llid_idx_2_0, (val))
#define e_rpt_qthld_cfg_VAL_queue_idx(val)                     REG_FLD_VAL(e_rpt_qthld_cfg_FLD_queue_idx, (val))

#define e_local_time_VAL_local_time(val)                       REG_FLD_VAL(e_local_time_FLD_local_time, (val))

#define e_tod_sync_x_VAL_tod_sync_x(val)                       REG_FLD_VAL(e_tod_sync_x_FLD_tod_sync_x, (val))

#define e_tod_ltncy_VAL_rl_rx_phydly_ofst(val)                 REG_FLD_VAL(e_tod_ltncy_FLD_rl_rx_phydly_ofst, (val))
#define e_tod_ltncy_VAL_ingrs_latency(val)                     REG_FLD_VAL(e_tod_ltncy_FLD_ingrs_latency, (val))
#define e_tod_ltncy_VAL_egrs_latency(val)                      REG_FLD_VAL(e_tod_ltncy_FLD_egrs_latency, (val))

#define e_new_tod_p2p_offset_sec_l32_VAL_new_tod_p2p_offset_sec_l32(val) REG_FLD_VAL(e_new_tod_p2p_offset_sec_l32_FLD_new_tod_p2p_offset_sec_l32, (val))

#define e_new_tod_p2p_tod_offset_nsec_VAL_new_tod_nsec(val)    REG_FLD_VAL(e_new_tod_p2p_tod_offset_nsec_FLD_new_tod_nsec, (val))

#define e_tod_p2p_tod_sec_l32_VAL_tod_p2p_sec_l32(val)         REG_FLD_VAL(e_tod_p2p_tod_sec_l32_FLD_tod_p2p_sec_l32, (val))

#define e_tod_p2p_tod_nsec_VAL_tod_p2p_nsec(val)               REG_FLD_VAL(e_tod_p2p_tod_nsec_FLD_tod_p2p_nsec, (val))

#define e_tod_period_VAL_tod_period(val)                       REG_FLD_VAL(e_tod_period_FLD_tod_period, (val))

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

#define e_slp_duration_i_VAL_slp_pwd_mode_i(val)               REG_FLD_VAL(e_slp_duration_i_FLD_slp_pwd_mode_i, (val))
#define e_slp_duration_i_VAL_slp_flag_i(val)                   REG_FLD_VAL(e_slp_duration_i_FLD_slp_flag_i, (val))
#define e_slp_duration_i_VAL_slp_duration_i(val)               REG_FLD_VAL(e_slp_duration_i_FLD_slp_duration_i, (val))

#define e_txfetch_cfg_VAL_tx_fetch_leadtime(val)               REG_FLD_VAL(e_txfetch_cfg_FLD_tx_fetch_leadtime, (val))
#define e_txfetch_cfg_VAL_tx_dma_leadtime(val)                 REG_FLD_VAL(e_txfetch_cfg_FLD_tx_dma_leadtime, (val))

#define e_sync_time_VAL_sync_time_updte(val)                   REG_FLD_VAL(e_sync_time_FLD_sync_time_updte, (val))
#define e_sync_time_VAL_sync_time(val)                         REG_FLD_VAL(e_sync_time_FLD_sync_time, (val))

#define e_tx_cal_cnst_VAL_dscvr_gnt_len(val)                   REG_FLD_VAL(e_tx_cal_cnst_FLD_dscvr_gnt_len, (val))
#define e_tx_cal_cnst_VAL_fec_tail_grd(val)                    REG_FLD_VAL(e_tx_cal_cnst_FLD_fec_tail_grd, (val))
#define e_tx_cal_cnst_VAL_tail_grd(val)                        REG_FLD_VAL(e_tx_cal_cnst_FLD_tail_grd, (val))
#define e_tx_cal_cnst_VAL_default_ovrhd(val)                   REG_FLD_VAL(e_tx_cal_cnst_FLD_default_ovrhd, (val))

#define e_laser_onoff_time_VAL_laser_off_time(val)             REG_FLD_VAL(e_laser_onoff_time_FLD_laser_off_time, (val))
#define e_laser_onoff_time_VAL_laser_on_time(val)              REG_FLD_VAL(e_laser_onoff_time_FLD_laser_on_time, (val))

#define e_grd_thrshld_VAL_guard_thrshld(val)                   REG_FLD_VAL(e_grd_thrshld_FLD_guard_thrshld, (val))

#define e_mpcp_timeout_intvl_VAL_mpcp_timeout_intvl(val)       REG_FLD_VAL(e_mpcp_timeout_intvl_FLD_mpcp_timeout_intvl, (val))

#define e_rpt_timeout_intvl_VAL_rpt_timeout_intvl(val)         REG_FLD_VAL(e_rpt_timeout_intvl_FLD_rpt_timeout_intvl, (val))

#define e_max_future_gnt_time_VAL_max_future_gnt_time(val)     REG_FLD_VAL(e_max_future_gnt_time_FLD_max_future_gnt_time, (val))

#define e_min_proc_time_VAL_min_proc_time(val)                 REG_FLD_VAL(e_min_proc_time_FLD_min_proc_time, (val))

#define e_trx_adjust_time1_VAL_tx_stm_adj(val)                 REG_FLD_VAL(e_trx_adjust_time1_FLD_tx_stm_adj, (val))

#define e_trx_adjust_time2_VAL_tx_len_adj(val)                 REG_FLD_VAL(e_trx_adjust_time2_FLD_tx_len_adj, (val))
#define e_trx_adjust_time2_VAL_rx_tmstp_adj(val)               REG_FLD_VAL(e_trx_adjust_time2_FLD_rx_tmstp_adj, (val))

#define e_dbg_prb_sel_VAL_probe_dtgrp_sel(val)                 REG_FLD_VAL(e_dbg_prb_sel_FLD_probe_dtgrp_sel, (val))
#define e_dbg_prb_sel_VAL_probe_bit0_sel(val)                  REG_FLD_VAL(e_dbg_prb_sel_FLD_probe_bit0_sel, (val))
#define e_dbg_prb_sel_VAL_probe_sel(val)                       REG_FLD_VAL(e_dbg_prb_sel_FLD_probe_sel, (val))

#define e_dbg_prb_h32_VAL_probe_h32(val)                       REG_FLD_VAL(e_dbg_prb_h32_FLD_probe_h32, (val))

#define e_dbg_prb_l32_VAL_probe_l32(val)                       REG_FLD_VAL(e_dbg_prb_l32_FLD_probe_l32, (val))

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

#define e_gnt_pending_stat_VAL_max_gnt_pending_cnt(val)        REG_FLD_VAL(e_gnt_pending_stat_FLD_max_gnt_pending_cnt, (val))
#define e_gnt_pending_stat_VAL_cur_gnt_pending_cnt(val)        REG_FLD_VAL(e_gnt_pending_stat_FLD_cur_gnt_pending_cnt, (val))

#define e_gnt_length_stat_VAL_max_gnt_length(val)              REG_FLD_VAL(e_gnt_length_stat_FLD_max_gnt_length, (val))
#define e_gnt_length_stat_VAL_min_gnt_length(val)              REG_FLD_VAL(e_gnt_length_stat_FLD_min_gnt_length, (val))

#define e_gnt_type_stat_VAL_b2b_gnt_cnt(val)                   REG_FLD_VAL(e_gnt_type_stat_FLD_b2b_gnt_cnt, (val))
#define e_gnt_type_stat_VAL_hdn_gnt_cnt(val)                   REG_FLD_VAL(e_gnt_type_stat_FLD_hdn_gnt_cnt, (val))

#define e_time_drft_stat_VAL_cur_time_drift_ofst(val)          REG_FLD_VAL(e_time_drft_stat_FLD_cur_time_drift_ofst, (val))
#define e_time_drft_stat_VAL_max_time_drift_ofst(val)          REG_FLD_VAL(e_time_drft_stat_FLD_max_time_drift_ofst, (val))
#define e_time_drft_stat_VAL_cur_time_drift(val)               REG_FLD_VAL(e_time_drft_stat_FLD_cur_time_drift, (val))
#define e_time_drft_stat_VAL_max_time_drift(val)               REG_FLD_VAL(e_time_drft_stat_FLD_max_time_drift, (val))

#define e_llid0_gnt_stat_VAL_llid0_gnt_cnt(val)                REG_FLD_VAL(e_llid0_gnt_stat_FLD_llid0_gnt_cnt, (val))

#define e_llid1_gnt_stat_VAL_llid1_gnt_cnt(val)                REG_FLD_VAL(e_llid1_gnt_stat_FLD_llid1_gnt_cnt, (val))

#define e_llid2_gnt_stat_VAL_llid2_gnt_cnt(val)                REG_FLD_VAL(e_llid2_gnt_stat_FLD_llid2_gnt_cnt, (val))

#define e_llid3_gnt_stat_VAL_llid3_gnt_cnt(val)                REG_FLD_VAL(e_llid3_gnt_stat_FLD_llid3_gnt_cnt, (val))

#define e_llid4_gnt_stat_VAL_llid4_gnt_cnt(val)                REG_FLD_VAL(e_llid4_gnt_stat_FLD_llid4_gnt_cnt, (val))

#define e_llid5_gnt_stat_VAL_llid5_gnt_cnt(val)                REG_FLD_VAL(e_llid5_gnt_stat_FLD_llid5_gnt_cnt, (val))

#define e_llid6_gnt_stat_VAL_llid6_gnt_cnt(val)                REG_FLD_VAL(e_llid6_gnt_stat_FLD_llid6_gnt_cnt, (val))

#define e_llid7_gnt_stat_VAL_llid7_gnt_cnt(val)                REG_FLD_VAL(e_llid7_gnt_stat_FLD_llid7_gnt_cnt, (val))

#define e_snf_mpcp_oam_ctl_VAL_snf_mpcp_cap_en(val)            REG_FLD_VAL(e_snf_mpcp_oam_ctl_FLD_snf_mpcp_cap_en, (val))
#define e_snf_mpcp_oam_ctl_VAL_snf_oam_cap_en(val)             REG_FLD_VAL(e_snf_mpcp_oam_ctl_FLD_snf_oam_cap_en, (val))
#define e_snf_mpcp_oam_ctl_VAL_snf_mpcp_oam_cnt_set(val)       REG_FLD_VAL(e_snf_mpcp_oam_ctl_FLD_snf_mpcp_oam_cnt_set, (val))

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

#define e_oam_kpalv_ctrl_VAL_oam_kpalv_interval(val)           REG_FLD_VAL(e_oam_kpalv_ctrl_FLD_oam_kpalv_interval, (val))
#define e_oam_kpalv_ctrl_VAL_oam_kpalv_sw_trig(val)            REG_FLD_VAL(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_trig, (val))
#define e_oam_kpalv_ctrl_VAL_oam_kpalv_sw_cfg(val)             REG_FLD_VAL(e_oam_kpalv_ctrl_FLD_oam_kpalv_sw_cfg, (val))
#define e_oam_kpalv_ctrl_VAL_oam_kpalv_en(val)                 REG_FLD_VAL(e_oam_kpalv_ctrl_FLD_oam_kpalv_en, (val))

#define e_tod_1pps_ctrl_VAL_tod_1pps_width_ctrl(val)           REG_FLD_VAL(e_tod_1pps_ctrl_FLD_tod_1pps_width_ctrl, (val))

#define e_sniff_sp_tag_VAL_sniffer_sp_tag(val)                 REG_FLD_VAL(e_sniff_sp_tag_FLD_sniffer_sp_tag, (val))

#define e_glb_cfg2_VAL_llidnum_sel(val)                        REG_FLD_VAL(e_glb_cfg2_FLD_llidnum_sel, (val))
#define e_glb_cfg2_VAL_loctime_mtd(val)                        REG_FLD_VAL(e_glb_cfg2_FLD_loctime_mtd, (val))
#define e_glb_cfg2_VAL_rgreq_lsrtime_mask(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_rgreq_lsrtime_mask, (val))
#define e_glb_cfg2_VAL_all_ucllid_mpcpsnf(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_all_ucllid_mpcpsnf, (val))
#define e_glb_cfg2_VAL_u10g_txmode(val)                        REG_FLD_VAL(e_glb_cfg2_FLD_u10g_txmode, (val))
#define e_glb_cfg2_VAL_eth_cal_in_bytecnt(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_eth_cal_in_bytecnt, (val))
#define e_glb_cfg2_VAL_snf_cal_in_bytecnt(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_snf_cal_in_bytecnt, (val))
#define e_glb_cfg2_VAL_oam_cal_in_bytecnt(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_oam_cal_in_bytecnt, (val))
#define e_glb_cfg2_VAL_dscvgate_filt(val)                      REG_FLD_VAL(e_glb_cfg2_FLD_dscvgate_filt, (val))
#define e_glb_cfg2_VAL_us_snf(val)                             REG_FLD_VAL(e_glb_cfg2_FLD_us_snf, (val))
#define e_glb_cfg2_VAL_llidinfo_snf(val)                       REG_FLD_VAL(e_glb_cfg2_FLD_llidinfo_snf, (val))
#define e_glb_cfg2_VAL_all_ucllid_ethsnf(val)                  REG_FLD_VAL(e_glb_cfg2_FLD_all_ucllid_ethsnf, (val))
#define e_glb_cfg2_VAL_all_ucllid_snf(val)                     REG_FLD_VAL(e_glb_cfg2_FLD_all_ucllid_snf, (val))
#define e_glb_cfg2_VAL_rxuni_mcllid_en(val)                    REG_FLD_VAL(e_glb_cfg2_FLD_rxuni_mcllid_en, (val))
#define e_glb_cfg2_VAL_gntlen_stat_widscv(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_gntlen_stat_widscv, (val))
#define e_glb_cfg2_VAL_tdrift_loctupd_dis(val)                 REG_FLD_VAL(e_glb_cfg2_FLD_tdrift_loctupd_dis, (val))
#define e_glb_cfg2_VAL_snf_fcserr_fwd(val)                     REG_FLD_VAL(e_glb_cfg2_FLD_snf_fcserr_fwd, (val))
#define e_glb_cfg2_VAL_mpcp_fcserr_fwd(val)                    REG_FLD_VAL(e_glb_cfg2_FLD_mpcp_fcserr_fwd, (val))
#define e_glb_cfg2_VAL_dscvgate_infochk_dis(val)               REG_FLD_VAL(e_glb_cfg2_FLD_dscvgate_infochk_dis, (val))

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
#define e_int_sts2_VAL_rcv_dauc_dscvgate_int(val)              REG_FLD_VAL(e_int_sts2_FLD_rcv_dauc_dscvgate_int, (val))

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
#define e_int_en2_VAL_rcv_dauc_dscvgate_en(val)                REG_FLD_VAL(e_int_en2_FLD_rcv_dauc_dscvgate_en, (val))

#define e_exc_sts_VAL_rx_rgst_dergst_hit(val)                  REG_FLD_VAL(e_exc_sts_FLD_rx_rgst_dergst_hit, (val))
#define e_exc_sts_VAL_rx_rgst_nack_hit(val)                    REG_FLD_VAL(e_exc_sts_FLD_rx_rgst_nack_hit, (val))
#define e_exc_sts_VAL_snf_pktcnt_err_hit(val)                  REG_FLD_VAL(e_exc_sts_FLD_snf_pktcnt_err_hit, (val))
#define e_exc_sts_VAL_tx_gntst_nosog_hit(val)                  REG_FLD_VAL(e_exc_sts_FLD_tx_gntst_nosog_hit, (val))
#define e_exc_sts_VAL_tx_gntst_fifound_hit(val)                REG_FLD_VAL(e_exc_sts_FLD_tx_gntst_fifound_hit, (val))
#define e_exc_sts_VAL_tx_gntph_fifound_hit(val)                REG_FLD_VAL(e_exc_sts_FLD_tx_gntph_fifound_hit, (val))
#define e_exc_sts_VAL_tx_gntph_exceed_hit(val)                 REG_FLD_VAL(e_exc_sts_FLD_tx_gntph_exceed_hit, (val))
#define e_exc_sts_VAL_schgnt_stinv_hit(val)                    REG_FLD_VAL(e_exc_sts_FLD_schgnt_stinv_hit, (val))
#define e_exc_sts_VAL_schgnt_idxinv_hit(val)                   REG_FLD_VAL(e_exc_sts_FLD_schgnt_idxinv_hit, (val))
#define e_exc_sts_VAL_decrpt_vecm_acccol_hit(val)              REG_FLD_VAL(e_exc_sts_FLD_decrpt_vecm_acccol_hit, (val))
#define e_exc_sts_VAL_rcv_len_long_hit(val)                    REG_FLD_VAL(e_exc_sts_FLD_rcv_len_long_hit, (val))
#define e_exc_sts_VAL_rcv_len_short_hit(val)                   REG_FLD_VAL(e_exc_sts_FLD_rcv_len_short_hit, (val))
#define e_exc_sts_VAL_rcv_mpcp_crcerr_hit(val)                 REG_FLD_VAL(e_exc_sts_FLD_rcv_mpcp_crcerr_hit, (val))
#define e_exc_sts_VAL_rcv_eth_crcerr_hit(val)                  REG_FLD_VAL(e_exc_sts_FLD_rcv_eth_crcerr_hit, (val))
#define e_exc_sts_VAL_rcv_crc8err_hit(val)                     REG_FLD_VAL(e_exc_sts_FLD_rcv_crc8err_hit, (val))
#define e_exc_sts_VAL_rcv_nrlgate_llidbc_hit(val)              REG_FLD_VAL(e_exc_sts_FLD_rcv_nrlgate_llidbc_hit, (val))
#define e_exc_sts_VAL_rcv_dscvgate_filt_hit(val)               REG_FLD_VAL(e_exc_sts_FLD_rcv_dscvgate_filt_hit, (val))
#define e_exc_sts_VAL_rcv_dscvgate_infomis_hit(val)            REG_FLD_VAL(e_exc_sts_FLD_rcv_dscvgate_infomis_hit, (val))

#define e_olt_dscvinfo_VAL_olt_dscvinfo_match(val)             REG_FLD_VAL(e_olt_dscvinfo_FLD_olt_dscvinfo_match, (val))
#define e_olt_dscvinfo_VAL_olt_dscvinfo_mis(val)               REG_FLD_VAL(e_olt_dscvinfo_FLD_olt_dscvinfo_mis, (val))

#define e_laser_onoff_time2_VAL_lsroff_time_olt(val)           REG_FLD_VAL(e_laser_onoff_time2_FLD_lsroff_time_olt, (val))
#define e_laser_onoff_time2_VAL_lsron_time_olt(val)            REG_FLD_VAL(e_laser_onoff_time2_FLD_lsron_time_olt, (val))
#define e_laser_onoff_time2_VAL_sync_time_olt(val)             REG_FLD_VAL(e_laser_onoff_time2_FLD_sync_time_olt, (val))

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

#define e_overhead_time_thr_VAL_sync_time_maxen(val)           REG_FLD_VAL(e_overhead_time_thr_FLD_sync_time_maxen, (val))
#define e_overhead_time_thr_VAL_lsroff_time_maxen(val)         REG_FLD_VAL(e_overhead_time_thr_FLD_lsroff_time_maxen, (val))
#define e_overhead_time_thr_VAL_lsron_time_maxen(val)          REG_FLD_VAL(e_overhead_time_thr_FLD_lsron_time_maxen, (val))
#define e_overhead_time_thr_VAL_sync_time_maxval(val)          REG_FLD_VAL(e_overhead_time_thr_FLD_sync_time_maxval, (val))
#define e_overhead_time_thr_VAL_lsroff_time_maxval(val)        REG_FLD_VAL(e_overhead_time_thr_FLD_lsroff_time_maxval, (val))
#define e_overhead_time_thr_VAL_lsron_time_maxval(val)         REG_FLD_VAL(e_overhead_time_thr_FLD_lsron_time_maxval, (val))

#define e_txcal_cnst2_VAL_ipgalign_mtd(val)                    REG_FLD_VAL(e_txcal_cnst2_FLD_ipgalign_mtd, (val))
#define e_txcal_cnst2_VAL_u10g_dscv_gntlen(val)                REG_FLD_VAL(e_txcal_cnst2_FLD_u10g_dscv_gntlen, (val))
#define e_txcal_cnst2_VAL_u10g_tail_grd(val)                   REG_FLD_VAL(e_txcal_cnst2_FLD_u10g_tail_grd, (val))

#define e_txcal_cnst3_VAL_u1g_fecon_min_gntlen(val)            REG_FLD_VAL(e_txcal_cnst3_FLD_u1g_fecon_min_gntlen, (val))
#define e_txcal_cnst3_VAL_u10g_eoblen(val)                     REG_FLD_VAL(e_txcal_cnst3_FLD_u10g_eoblen, (val))
#define e_txcal_cnst3_VAL_u10g_min_gntlen(val)                 REG_FLD_VAL(e_txcal_cnst3_FLD_u10g_min_gntlen, (val))
#define e_txcal_cnst3_VAL_u1g_fecoff_min_gntlen(val)           REG_FLD_VAL(e_txcal_cnst3_FLD_u1g_fecoff_min_gntlen, (val))

#define e_txsch_cfg_VAL_txsch_dmy0(val)                        REG_FLD_VAL(e_txsch_cfg_FLD_txsch_dmy0, (val))
#define e_txsch_cfg_VAL_txfifo_pad_hthr(val)                   REG_FLD_VAL(e_txsch_cfg_FLD_txfifo_pad_hthr, (val))
#define e_txsch_cfg_VAL_txsch_dmy1(val)                        REG_FLD_VAL(e_txsch_cfg_FLD_txsch_dmy1, (val))
#define e_txsch_cfg_VAL_txfifo_pad_lthr(val)                   REG_FLD_VAL(e_txsch_cfg_FLD_txfifo_pad_lthr, (val))

#define e_rxfifo_thr_VAL_rxfifo_mbithr(val)                    REG_FLD_VAL(e_rxfifo_thr_FLD_rxfifo_mbithr, (val))

#define e_bcllid_cfg_VAL_d10g_bcllid(val)                      REG_FLD_VAL(e_bcllid_cfg_FLD_d10g_bcllid, (val))
#define e_bcllid_cfg_VAL_d1g_bcllid(val)                       REG_FLD_VAL(e_bcllid_cfg_FLD_d1g_bcllid, (val))

#define e_txfrm_cfg1_VAL_tx_mpcp_addrl(val)                    REG_FLD_VAL(e_txfrm_cfg1_FLD_tx_mpcp_addrl, (val))

#define e_txfrm_cfg2_VAL_tx_mpcp_addrh(val)                    REG_FLD_VAL(e_txfrm_cfg2_FLD_tx_mpcp_addrh, (val))
#define e_txfrm_cfg2_VAL_tx_mpcp_etype(val)                    REG_FLD_VAL(e_txfrm_cfg2_FLD_tx_mpcp_etype, (val))

#define e_txfrm_cfg3_VAL_tx_rgreq_op(val)                      REG_FLD_VAL(e_txfrm_cfg3_FLD_tx_rgreq_op, (val))
#define e_txfrm_cfg3_VAL_tx_rgack_op(val)                      REG_FLD_VAL(e_txfrm_cfg3_FLD_tx_rgack_op, (val))

#define e_u1g_rpt_qsizeadj1_VAL_u1g_fecoff_rpt_q1_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj1_FLD_u1g_fecoff_rpt_q1_qsizeadj, (val))
#define e_u1g_rpt_qsizeadj1_VAL_u1g_fecoff_rpt_q0_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj1_FLD_u1g_fecoff_rpt_q0_qsizeadj, (val))

#define e_u1g_rpt_qsizeadj2_VAL_u1g_fecoff_rpt_q3_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj2_FLD_u1g_fecoff_rpt_q3_qsizeadj, (val))
#define e_u1g_rpt_qsizeadj2_VAL_u1g_fecoff_rpt_q2_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj2_FLD_u1g_fecoff_rpt_q2_qsizeadj, (val))

#define e_u1g_rpt_qsizeadj3_VAL_u1g_fecoff_rpt_q5_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj3_FLD_u1g_fecoff_rpt_q5_qsizeadj, (val))
#define e_u1g_rpt_qsizeadj3_VAL_u1g_fecoff_rpt_q4_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj3_FLD_u1g_fecoff_rpt_q4_qsizeadj, (val))

#define e_u1g_rpt_qsizeadj4_VAL_u1g_fecoff_rpt_q7_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj4_FLD_u1g_fecoff_rpt_q7_qsizeadj, (val))
#define e_u1g_rpt_qsizeadj4_VAL_u1g_fecoff_rpt_q6_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj4_FLD_u1g_fecoff_rpt_q6_qsizeadj, (val))

#define e_u1g_rpt_qsizeadj5_VAL_u1g_fecon_rpt_q1_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj5_FLD_u1g_fecon_rpt_q1_qsizeadj, (val))
#define e_u1g_rpt_qsizeadj5_VAL_u1g_fecon_rpt_q0_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj5_FLD_u1g_fecon_rpt_q0_qsizeadj, (val))

#define e_u1g_rpt_qsizeadj6_VAL_u1g_fecon_rpt_q3_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj6_FLD_u1g_fecon_rpt_q3_qsizeadj, (val))
#define e_u1g_rpt_qsizeadj6_VAL_u1g_fecon_rpt_q2_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj6_FLD_u1g_fecon_rpt_q2_qsizeadj, (val))

#define e_u1g_rpt_qsizeadj7_VAL_u1g_fecon_rpt_q5_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj7_FLD_u1g_fecon_rpt_q5_qsizeadj, (val))
#define e_u1g_rpt_qsizeadj7_VAL_u1g_fecon_rpt_q4_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj7_FLD_u1g_fecon_rpt_q4_qsizeadj, (val))

#define e_u1g_rpt_qsizeadj8_VAL_u1g_fecon_rpt_q7_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj8_FLD_u1g_fecon_rpt_q7_qsizeadj, (val))
#define e_u1g_rpt_qsizeadj8_VAL_u1g_fecon_rpt_q6_qsizeadj(val) REG_FLD_VAL(e_u1g_rpt_qsizeadj8_FLD_u1g_fecon_rpt_q6_qsizeadj, (val))

#define e_u10g_rpt_qsizeadj1_VAL_u10g_rpt_q1_qsizeadj(val)     REG_FLD_VAL(e_u10g_rpt_qsizeadj1_FLD_u10g_rpt_q1_qsizeadj, (val))
#define e_u10g_rpt_qsizeadj1_VAL_u10g_rpt_q0_qsizeadj(val)     REG_FLD_VAL(e_u10g_rpt_qsizeadj1_FLD_u10g_rpt_q0_qsizeadj, (val))

#define e_u10g_rpt_qsizeadj2_VAL_u10g_rpt_q3_qsizeadj(val)     REG_FLD_VAL(e_u10g_rpt_qsizeadj2_FLD_u10g_rpt_q3_qsizeadj, (val))
#define e_u10g_rpt_qsizeadj2_VAL_u10g_rpt_q2_qsizeadj(val)     REG_FLD_VAL(e_u10g_rpt_qsizeadj2_FLD_u10g_rpt_q2_qsizeadj, (val))

#define e_u10g_rpt_qsizeadj3_VAL_u10g_rpt_q5_qsizeadj(val)     REG_FLD_VAL(e_u10g_rpt_qsizeadj3_FLD_u10g_rpt_q5_qsizeadj, (val))
#define e_u10g_rpt_qsizeadj3_VAL_u10g_rpt_q4_qsizeadj(val)     REG_FLD_VAL(e_u10g_rpt_qsizeadj3_FLD_u10g_rpt_q4_qsizeadj, (val))

#define e_u10g_rpt_qsizeadj4_VAL_u10g_rpt_q7_qsizeadj(val)     REG_FLD_VAL(e_u10g_rpt_qsizeadj4_FLD_u10g_rpt_q7_qsizeadj, (val))
#define e_u10g_rpt_qsizeadj4_VAL_u10g_rpt_q6_qsizeadj(val)     REG_FLD_VAL(e_u10g_rpt_qsizeadj4_FLD_u10g_rpt_q6_qsizeadj, (val))

#define e_snf_dah_VAL_snf_pkt_dah(val)                         REG_FLD_VAL(e_snf_dah_FLD_snf_pkt_dah, (val))

#define e_snf_dal_VAL_snf_pkt_dal(val)                         REG_FLD_VAL(e_snf_dal_FLD_snf_pkt_dal, (val))
#define e_snf_dal_VAL_snf_pkt_sah(val)                         REG_FLD_VAL(e_snf_dal_FLD_snf_pkt_sah, (val))

#define e_snf_sal_VAL_snf_pkt_sal(val)                         REG_FLD_VAL(e_snf_sal_FLD_snf_pkt_sal, (val))

#define e_snf_etype_VAL_snf_pkt_etype(val)                     REG_FLD_VAL(e_snf_etype_FLD_snf_pkt_etype, (val))

#define e_rxuni_mcllid_cfg0_VAL_rxuni_mcllid1(val)             REG_FLD_VAL(e_rxuni_mcllid_cfg0_FLD_rxuni_mcllid1, (val))
#define e_rxuni_mcllid_cfg0_VAL_rxuni_mcllid0(val)             REG_FLD_VAL(e_rxuni_mcllid_cfg0_FLD_rxuni_mcllid0, (val))

#define e_rxuni_mcllid_cfg1_VAL_rxuni_mcllid3(val)             REG_FLD_VAL(e_rxuni_mcllid_cfg1_FLD_rxuni_mcllid3, (val))
#define e_rxuni_mcllid_cfg1_VAL_rxuni_mcllid2(val)             REG_FLD_VAL(e_rxuni_mcllid_cfg1_FLD_rxuni_mcllid2, (val))

#define e_rxuni_mcllid_cfg2_VAL_rxuni_mcllid5(val)             REG_FLD_VAL(e_rxuni_mcllid_cfg2_FLD_rxuni_mcllid5, (val))
#define e_rxuni_mcllid_cfg2_VAL_rxuni_mcllid4(val)             REG_FLD_VAL(e_rxuni_mcllid_cfg2_FLD_rxuni_mcllid4, (val))

#define e_rxuni_mcllid_cfg3_VAL_rxuni_mcllid7(val)             REG_FLD_VAL(e_rxuni_mcllid_cfg3_FLD_rxuni_mcllid7, (val))
#define e_rxuni_mcllid_cfg3_VAL_rxuni_mcllid6(val)             REG_FLD_VAL(e_rxuni_mcllid_cfg3_FLD_rxuni_mcllid6, (val))

#define e_rxuni_mcllid_cfg4_VAL_rxuni_mcllid9(val)             REG_FLD_VAL(e_rxuni_mcllid_cfg4_FLD_rxuni_mcllid9, (val))
#define e_rxuni_mcllid_cfg4_VAL_rxuni_mcllid8(val)             REG_FLD_VAL(e_rxuni_mcllid_cfg4_FLD_rxuni_mcllid8, (val))

#define e_rxuni_mcllid_cfg5_VAL_rxuni_mcllid11(val)            REG_FLD_VAL(e_rxuni_mcllid_cfg5_FLD_rxuni_mcllid11, (val))
#define e_rxuni_mcllid_cfg5_VAL_rxuni_mcllid10(val)            REG_FLD_VAL(e_rxuni_mcllid_cfg5_FLD_rxuni_mcllid10, (val))

#define e_rxuni_mcllid_cfg6_VAL_rxuni_mcllid13(val)            REG_FLD_VAL(e_rxuni_mcllid_cfg6_FLD_rxuni_mcllid13, (val))
#define e_rxuni_mcllid_cfg6_VAL_rxuni_mcllid12(val)            REG_FLD_VAL(e_rxuni_mcllid_cfg6_FLD_rxuni_mcllid12, (val))

#define e_rxuni_mcllid_cfg7_VAL_rxuni_mcllid15(val)            REG_FLD_VAL(e_rxuni_mcllid_cfg7_FLD_rxuni_mcllid15, (val))
#define e_rxuni_mcllid_cfg7_VAL_rxuni_mcllid14(val)            REG_FLD_VAL(e_rxuni_mcllid_cfg7_FLD_rxuni_mcllid14, (val))

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

#define e_glue_cfg_VAL_txmpi_fifound_pktgate_en(val)           REG_FLD_VAL(e_glue_cfg_FLD_txmpi_fifound_pktgate_en, (val))
#define e_glue_cfg_VAL_txmpi_fifound_thr(val)                  REG_FLD_VAL(e_glue_cfg_FLD_txmpi_fifound_thr, (val))

#define e_gntreq_tmout_VAL_gntreq_grdcyc(val)                  REG_FLD_VAL(e_gntreq_tmout_FLD_gntreq_grdcyc, (val))

#define e_glb_sts_VAL_xepon_type(val)                          REG_FLD_VAL(e_glb_sts_FLD_xepon_type, (val))

#define e_cnt_clr_VAL_glb_cntclr(val)                          REG_FLD_VAL(e_cnt_clr_FLD_glb_cntclr, (val))

#define e_rxadv_cnt_VAL_rx_infomis_dscvgate_cnt(val)           REG_FLD_VAL(e_rxadv_cnt_FLD_rx_infomis_dscvgate_cnt, (val))
#define e_rxadv_cnt_VAL_rx_dauc_dscvgate_cnt(val)              REG_FLD_VAL(e_rxadv_cnt_FLD_rx_dauc_dscvgate_cnt, (val))
#define e_rxadv_cnt_VAL_rx_filt_dscvgate_cnt(val)              REG_FLD_VAL(e_rxadv_cnt_FLD_rx_filt_dscvgate_cnt, (val))

#define e_tx_dygasp_cnt_VAL_txmbi_dygasp_cnt(val)              REG_FLD_VAL(e_tx_dygasp_cnt_FLD_txmbi_dygasp_cnt, (val))
#define e_tx_dygasp_cnt_VAL_txmpi_dygasp_cnt(val)              REG_FLD_VAL(e_tx_dygasp_cnt_FLD_txmpi_dygasp_cnt, (val))

#define e_tx_rpt_cnt_VAL_txmbi_rpt_cnt(val)                    REG_FLD_VAL(e_tx_rpt_cnt_FLD_txmbi_rpt_cnt, (val))
#define e_tx_rpt_cnt_VAL_txmpi_rpt_cnt(val)                    REG_FLD_VAL(e_tx_rpt_cnt_FLD_txmpi_rpt_cnt, (val))

#define e_txmpi_mpcp_cnt_VAL_txmpi_oam_cnt(val)                REG_FLD_VAL(e_txmpi_mpcp_cnt_FLD_txmpi_oam_cnt, (val))
#define e_txmpi_mpcp_cnt_VAL_txmpi_rgreq_cnt(val)              REG_FLD_VAL(e_txmpi_mpcp_cnt_FLD_txmpi_rgreq_cnt, (val))
#define e_txmpi_mpcp_cnt_VAL_txmpi_rgack_cnt(val)              REG_FLD_VAL(e_txmpi_mpcp_cnt_FLD_txmpi_rgack_cnt, (val))

#define e_rxmpi_uc_cnt_VAL_rxmpi_uceth_cnt(val)                REG_FLD_VAL(e_rxmpi_uc_cnt_FLD_rxmpi_uceth_cnt, (val))

#define e_rxmpi_bc_cnt_VAL_rxmpi_bceth_cnt(val)                REG_FLD_VAL(e_rxmpi_bc_cnt_FLD_rxmpi_bceth_cnt, (val))

#define e_rxmpi_mc_cnt_VAL_rxmpi_mceth_cnt(val)                REG_FLD_VAL(e_rxmpi_mc_cnt_FLD_rxmpi_mceth_cnt, (val))

#define e_rxmpi_oam_cnt_VAL_rxmpi_oam_cnt(val)                 REG_FLD_VAL(e_rxmpi_oam_cnt_FLD_rxmpi_oam_cnt, (val))

#define e_rxmpi_mpcp_cnt_VAL_rxmpi_mpcp_cnt(val)               REG_FLD_VAL(e_rxmpi_mpcp_cnt_FLD_rxmpi_mpcp_cnt, (val))

#define e_rxmpi_gate_cnt_VAL_rxmpi_gate_cnt(val)               REG_FLD_VAL(e_rxmpi_gate_cnt_FLD_rxmpi_gate_cnt, (val))

#define e_rxmpi_nrlgate_cnt_VAL_rxmpi_nrlgate_cnt(val)         REG_FLD_VAL(e_rxmpi_nrlgate_cnt_FLD_rxmpi_nrlgate_cnt, (val))

#define e_rxmpi_crc8err_cnt_VAL_rxmpi_crceth_cnt(val)          REG_FLD_VAL(e_rxmpi_crc8err_cnt_FLD_rxmpi_crceth_cnt, (val))
#define e_rxmpi_crc8err_cnt_VAL_rxmpi_crc8err_cnt(val)         REG_FLD_VAL(e_rxmpi_crc8err_cnt_FLD_rxmpi_crc8err_cnt, (val))

#define e_rxmpi_drop_cnt_VAL_rxmpi_eofdrop_cnt(val)            REG_FLD_VAL(e_rxmpi_drop_cnt_FLD_rxmpi_eofdrop_cnt, (val))

#define e_rxmpi_churn_cnt_VAL_rxmpi_abchurn_cnt(val)           REG_FLD_VAL(e_rxmpi_churn_cnt_FLD_rxmpi_abchurn_cnt, (val))
#define e_rxmpi_churn_cnt_VAL_rxmpi_churn_cnt(val)             REG_FLD_VAL(e_rxmpi_churn_cnt_FLD_rxmpi_churn_cnt, (val))

#define e_rxmbi_snf_cnt_VAL_rxmbi_snf_cnt(val)                 REG_FLD_VAL(e_rxmbi_snf_cnt_FLD_rxmbi_snf_cnt, (val))

#define e_rxmbi_drop_cnt_VAL_rxmbi_sofdrop_cnt(val)            REG_FLD_VAL(e_rxmbi_drop_cnt_FLD_rxmbi_sofdrop_cnt, (val))
#define e_rxmbi_drop_cnt_VAL_rxmbi_snfdrop_cnt(val)            REG_FLD_VAL(e_rxmbi_drop_cnt_FLD_rxmbi_snfdrop_cnt, (val))

#define e_rxmbi_drop_cnt2_VAL_rxmbi_crcerr_cnt(val)            REG_FLD_VAL(e_rxmbi_drop_cnt2_FLD_rxmbi_crcerr_cnt, (val))
#define e_rxmbi_drop_cnt2_VAL_rxmbi_enddrop_cnt(val)           REG_FLD_VAL(e_rxmbi_drop_cnt2_FLD_rxmbi_enddrop_cnt, (val))

#define e_rxmbi_bytecnt_h_VAL_rxmbi_bytecnt_h(val)             REG_FLD_VAL(e_rxmbi_bytecnt_h_FLD_rxmbi_bytecnt_h, (val))

#define e_rxmbi_bytecnt_l_VAL_rxmbi_bytecnt_l(val)             REG_FLD_VAL(e_rxmbi_bytecnt_l_FLD_rxmbi_bytecnt_l, (val))

#define e_txmbi_uceth_cnt_VAL_txmbi_uceth_cnt(val)             REG_FLD_VAL(e_txmbi_uceth_cnt_FLD_txmbi_uceth_cnt, (val))

#define e_txmbi_mceth_cnt_VAL_txmbi_bceth_cnt(val)             REG_FLD_VAL(e_txmbi_mceth_cnt_FLD_txmbi_bceth_cnt, (val))
#define e_txmbi_mceth_cnt_VAL_txmbi_mceth_cnt(val)             REG_FLD_VAL(e_txmbi_mceth_cnt_FLD_txmbi_mceth_cnt, (val))

#define e_txmbi_err_cnt_VAL_txmbi_err_cnt(val)                 REG_FLD_VAL(e_txmbi_err_cnt_FLD_txmbi_err_cnt, (val))

#define e_tx_timedrift_stat_VAL_tx_cur_time_drift_ofst(val)    REG_FLD_VAL(e_tx_timedrift_stat_FLD_tx_cur_time_drift_ofst, (val))
#define e_tx_timedrift_stat_VAL_tx_max_time_drift_ofst(val)    REG_FLD_VAL(e_tx_timedrift_stat_FLD_tx_max_time_drift_ofst, (val))
#define e_tx_timedrift_stat_VAL_tx_cur_time_drift(val)         REG_FLD_VAL(e_tx_timedrift_stat_FLD_tx_cur_time_drift, (val))
#define e_tx_timedrift_stat_VAL_tx_max_time_drift(val)         REG_FLD_VAL(e_tx_timedrift_stat_FLD_tx_max_time_drift, (val))

#define e_rxmpi_churn_cnt2_VAL_rxmpi_churn_mpcpcnt(val)        REG_FLD_VAL(e_rxmpi_churn_cnt2_FLD_rxmpi_churn_mpcpcnt, (val))
#define e_rxmpi_churn_cnt2_VAL_rxmpi_churn_ethcnt(val)         REG_FLD_VAL(e_rxmpi_churn_cnt2_FLD_rxmpi_churn_ethcnt, (val))

#define e_rxmpi_churn_cnt3_VAL_rxmpi_churn_keyidx(val)         REG_FLD_VAL(e_rxmpi_churn_cnt3_FLD_rxmpi_churn_keyidx, (val))
#define e_rxmpi_churn_cnt3_VAL_rxmpi_churn_llididx(val)        REG_FLD_VAL(e_rxmpi_churn_cnt3_FLD_rxmpi_churn_llididx, (val))
#define e_rxmpi_churn_cnt3_VAL_rxmpi_churncrc_keyidx(val)      REG_FLD_VAL(e_rxmpi_churn_cnt3_FLD_rxmpi_churncrc_keyidx, (val))
#define e_rxmpi_churn_cnt3_VAL_rxmpi_churncrc_llididx(val)     REG_FLD_VAL(e_rxmpi_churn_cnt3_FLD_rxmpi_churncrc_llididx, (val))
#define e_rxmpi_churn_cnt3_VAL_rxmpi_churn_crccnt(val)         REG_FLD_VAL(e_rxmpi_churn_cnt3_FLD_rxmpi_churn_crccnt, (val))

#define e_txmpi_uceth_cnt_VAL_txmpi_uceth_cnt(val)             REG_FLD_VAL(e_txmpi_uceth_cnt_FLD_txmpi_uceth_cnt, (val))

#define e_txmpi_mceth_cnt_VAL_txmpi_bceth_cnt(val)             REG_FLD_VAL(e_txmpi_mceth_cnt_FLD_txmpi_bceth_cnt, (val))
#define e_txmpi_mceth_cnt_VAL_txmpi_mceth_cnt(val)             REG_FLD_VAL(e_txmpi_mceth_cnt_FLD_txmpi_mceth_cnt, (val))

#define e_llid8_9_gnt_stat_VAL_llid9_gnt_cnt(val)              REG_FLD_VAL(e_llid8_9_gnt_stat_FLD_llid9_gnt_cnt, (val))
#define e_llid8_9_gnt_stat_VAL_llid8_gnt_cnt(val)              REG_FLD_VAL(e_llid8_9_gnt_stat_FLD_llid8_gnt_cnt, (val))

#define e_llid10_11_gnt_stat_VAL_llid11_gnt_cnt(val)           REG_FLD_VAL(e_llid10_11_gnt_stat_FLD_llid11_gnt_cnt, (val))
#define e_llid10_11_gnt_stat_VAL_llid10_gnt_cnt(val)           REG_FLD_VAL(e_llid10_11_gnt_stat_FLD_llid10_gnt_cnt, (val))

#define e_llid12_13_gnt_stat_VAL_llid13_gnt_cnt(val)           REG_FLD_VAL(e_llid12_13_gnt_stat_FLD_llid13_gnt_cnt, (val))
#define e_llid12_13_gnt_stat_VAL_llid12_gnt_cnt(val)           REG_FLD_VAL(e_llid12_13_gnt_stat_FLD_llid12_gnt_cnt, (val))

#define e_llid14_15_gnt_stat_VAL_llid15_gnt_cnt(val)           REG_FLD_VAL(e_llid14_15_gnt_stat_FLD_llid15_gnt_cnt, (val))
#define e_llid14_15_gnt_stat_VAL_llid14_gnt_cnt(val)           REG_FLD_VAL(e_llid14_15_gnt_stat_FLD_llid14_gnt_cnt, (val))

#define e_llid16_17_gnt_stat_VAL_llid17_gnt_cnt(val)           REG_FLD_VAL(e_llid16_17_gnt_stat_FLD_llid17_gnt_cnt, (val))
#define e_llid16_17_gnt_stat_VAL_llid16_gnt_cnt(val)           REG_FLD_VAL(e_llid16_17_gnt_stat_FLD_llid16_gnt_cnt, (val))

#define e_llid18_19_gnt_stat_VAL_llid19_gnt_cnt(val)           REG_FLD_VAL(e_llid18_19_gnt_stat_FLD_llid19_gnt_cnt, (val))
#define e_llid18_19_gnt_stat_VAL_llid18_gnt_cnt(val)           REG_FLD_VAL(e_llid18_19_gnt_stat_FLD_llid18_gnt_cnt, (val))

#define e_llid20_21_gnt_stat_VAL_llid21_gnt_cnt(val)           REG_FLD_VAL(e_llid20_21_gnt_stat_FLD_llid21_gnt_cnt, (val))
#define e_llid20_21_gnt_stat_VAL_llid20_gnt_cnt(val)           REG_FLD_VAL(e_llid20_21_gnt_stat_FLD_llid20_gnt_cnt, (val))

#define e_llid22_23_gnt_stat_VAL_llid23_gnt_cnt(val)           REG_FLD_VAL(e_llid22_23_gnt_stat_FLD_llid23_gnt_cnt, (val))
#define e_llid22_23_gnt_stat_VAL_llid22_gnt_cnt(val)           REG_FLD_VAL(e_llid22_23_gnt_stat_FLD_llid22_gnt_cnt, (val))

#define e_llid24_25_gnt_stat_VAL_llid25_gnt_cnt(val)           REG_FLD_VAL(e_llid24_25_gnt_stat_FLD_llid25_gnt_cnt, (val))
#define e_llid24_25_gnt_stat_VAL_llid24_gnt_cnt(val)           REG_FLD_VAL(e_llid24_25_gnt_stat_FLD_llid24_gnt_cnt, (val))

#define e_llid26_27_gnt_stat_VAL_llid27_gnt_cnt(val)           REG_FLD_VAL(e_llid26_27_gnt_stat_FLD_llid27_gnt_cnt, (val))
#define e_llid26_27_gnt_stat_VAL_llid26_gnt_cnt(val)           REG_FLD_VAL(e_llid26_27_gnt_stat_FLD_llid26_gnt_cnt, (val))

#define e_llid28_29_gnt_stat_VAL_llid29_gnt_cnt(val)           REG_FLD_VAL(e_llid28_29_gnt_stat_FLD_llid29_gnt_cnt, (val))
#define e_llid28_29_gnt_stat_VAL_llid28_gnt_cnt(val)           REG_FLD_VAL(e_llid28_29_gnt_stat_FLD_llid28_gnt_cnt, (val))

#define e_llid30_31_gnt_stat_VAL_llid31_gnt_cnt(val)           REG_FLD_VAL(e_llid30_31_gnt_stat_FLD_llid31_gnt_cnt, (val))
#define e_llid30_31_gnt_stat_VAL_llid30_gnt_cnt(val)           REG_FLD_VAL(e_llid30_31_gnt_stat_FLD_llid30_gnt_cnt, (val))

#define e_rxfifo_depth_stat_VAL_max_rxfifo_depth(val)          REG_FLD_VAL(e_rxfifo_depth_stat_FLD_max_rxfifo_depth, (val))

#define e_rdmdly_stat_VAL_rdmdly(val)                          REG_FLD_VAL(e_rdmdly_stat_FLD_rdmdly, (val))

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

#define e_rpt_mpcp_timout2_VAL_llid31_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid31_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid30_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid30_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid29_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid29_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid28_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid28_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid27_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid27_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid26_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid26_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid25_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid25_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid24_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid24_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid23_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid23_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid22_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid22_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid21_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid21_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid20_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid20_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid19_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid19_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid18_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid18_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid17_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid17_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid16_mpcp_tmo(val)            REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid16_mpcp_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid31_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid31_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid30_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid30_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid29_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid29_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid28_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid28_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid27_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid27_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid26_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid26_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid25_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid25_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid24_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid24_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid23_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid23_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid22_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid22_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid21_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid21_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid20_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid20_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid19_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid19_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid18_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid18_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid17_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid17_rpt_tmo, (val))
#define e_rpt_mpcp_timout2_VAL_llid16_rpt_tmo(val)             REG_FLD_VAL(e_rpt_mpcp_timout2_FLD_llid16_rpt_tmo, (val))

#define e_llid8_11_cfg_VAL_llid11_dummy(val)                   REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_dummy, (val))
#define e_llid8_11_cfg_VAL_llid11_txfec_en(val)                REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_txfec_en, (val))
#define e_llid8_11_cfg_VAL_llid11_dcrypt_en(val)               REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_dcrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid11_dcrypt_mode(val)             REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_dcrypt_mode, (val))
#define e_llid8_11_cfg_VAL_llid11_oamlpbk_en(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid11_oamlpbk_en, (val))
#define e_llid8_11_cfg_VAL_llid10_dummy(val)                   REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_dummy, (val))
#define e_llid8_11_cfg_VAL_llid10_txfec_en(val)                REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_txfec_en, (val))
#define e_llid8_11_cfg_VAL_llid10_dcrypt_en(val)               REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_dcrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid10_dcrypt_mode(val)             REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_dcrypt_mode, (val))
#define e_llid8_11_cfg_VAL_llid10_oamlpbk_en(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid10_oamlpbk_en, (val))
#define e_llid8_11_cfg_VAL_llid9_dummy(val)                    REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_dummy, (val))
#define e_llid8_11_cfg_VAL_llid9_txfec_en(val)                 REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_txfec_en, (val))
#define e_llid8_11_cfg_VAL_llid9_dcrypt_en(val)                REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_dcrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid9_dcrypt_mode(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_dcrypt_mode, (val))
#define e_llid8_11_cfg_VAL_llid9_oamlpbk_en(val)               REG_FLD_VAL(e_llid8_11_cfg_FLD_llid9_oamlpbk_en, (val))
#define e_llid8_11_cfg_VAL_llid8_dummy(val)                    REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_dummy, (val))
#define e_llid8_11_cfg_VAL_llid8_txfec_en(val)                 REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_txfec_en, (val))
#define e_llid8_11_cfg_VAL_llid8_dcrypt_en(val)                REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_dcrypt_en, (val))
#define e_llid8_11_cfg_VAL_llid8_dcrypt_mode(val)              REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_dcrypt_mode, (val))
#define e_llid8_11_cfg_VAL_llid8_oamlpbk_en(val)               REG_FLD_VAL(e_llid8_11_cfg_FLD_llid8_oamlpbk_en, (val))

#define e_llid12_15_cfg_VAL_llid15_dummy(val)                  REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_dummy, (val))
#define e_llid12_15_cfg_VAL_llid15_txfec_en(val)               REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_txfec_en, (val))
#define e_llid12_15_cfg_VAL_llid15_dcrypt_en(val)              REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_dcrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid15_dcrypt_mode(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_dcrypt_mode, (val))
#define e_llid12_15_cfg_VAL_llid15_oamlpbk_en(val)             REG_FLD_VAL(e_llid12_15_cfg_FLD_llid15_oamlpbk_en, (val))
#define e_llid12_15_cfg_VAL_llid14_dummy(val)                  REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_dummy, (val))
#define e_llid12_15_cfg_VAL_llid14_txfec_en(val)               REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_txfec_en, (val))
#define e_llid12_15_cfg_VAL_llid14_dcrypt_en(val)              REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_dcrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid14_dcrypt_mode(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_dcrypt_mode, (val))
#define e_llid12_15_cfg_VAL_llid14_oamlpbk_en(val)             REG_FLD_VAL(e_llid12_15_cfg_FLD_llid14_oamlpbk_en, (val))
#define e_llid12_15_cfg_VAL_llid13_dummy(val)                  REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_dummy, (val))
#define e_llid12_15_cfg_VAL_llid13_txfec_en(val)               REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_txfec_en, (val))
#define e_llid12_15_cfg_VAL_llid13_dcrypt_en(val)              REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_dcrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid13_dcrypt_mode(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_dcrypt_mode, (val))
#define e_llid12_15_cfg_VAL_llid13_oamlpbk_en(val)             REG_FLD_VAL(e_llid12_15_cfg_FLD_llid13_oamlpbk_en, (val))
#define e_llid12_15_cfg_VAL_llid12_dummy(val)                  REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_dummy, (val))
#define e_llid12_15_cfg_VAL_llid12_txfec_en(val)               REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_txfec_en, (val))
#define e_llid12_15_cfg_VAL_llid12_dcrypt_en(val)              REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_dcrypt_en, (val))
#define e_llid12_15_cfg_VAL_llid12_dcrypt_mode(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_dcrypt_mode, (val))
#define e_llid12_15_cfg_VAL_llid12_oam_lpbk_en(val)            REG_FLD_VAL(e_llid12_15_cfg_FLD_llid12_oam_lpbk_en, (val))

#define e_llid16_19_cfg_VAL_llid19_dummy(val)                  REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_dummy, (val))
#define e_llid16_19_cfg_VAL_llid19_txfec_en(val)               REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_txfec_en, (val))
#define e_llid16_19_cfg_VAL_llid19_dcrypt_en(val)              REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_dcrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid19_dcrypt_mode(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_dcrypt_mode, (val))
#define e_llid16_19_cfg_VAL_llid19_oamlpbk_en(val)             REG_FLD_VAL(e_llid16_19_cfg_FLD_llid19_oamlpbk_en, (val))
#define e_llid16_19_cfg_VAL_llid18_dummy(val)                  REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_dummy, (val))
#define e_llid16_19_cfg_VAL_llid18_txfec_en(val)               REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_txfec_en, (val))
#define e_llid16_19_cfg_VAL_llid18_dcrypt_en(val)              REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_dcrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid18_dcrypt_mode(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_dcrypt_mode, (val))
#define e_llid16_19_cfg_VAL_llid18_oamlpbk_en(val)             REG_FLD_VAL(e_llid16_19_cfg_FLD_llid18_oamlpbk_en, (val))
#define e_llid16_19_cfg_VAL_llid17_dummy(val)                  REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_dummy, (val))
#define e_llid16_19_cfg_VAL_llid17_txfec_en(val)               REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_txfec_en, (val))
#define e_llid16_19_cfg_VAL_llid17_dcrypt_en(val)              REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_dcrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid17_dcrypt_mode(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_dcrypt_mode, (val))
#define e_llid16_19_cfg_VAL_llid17_oamlpbk_en(val)             REG_FLD_VAL(e_llid16_19_cfg_FLD_llid17_oamlpbk_en, (val))
#define e_llid16_19_cfg_VAL_llid16_dummy(val)                  REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_dummy, (val))
#define e_llid16_19_cfg_VAL_llid16_txfec_en(val)               REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_txfec_en, (val))
#define e_llid16_19_cfg_VAL_llid16_dcrypt_en(val)              REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_dcrypt_en, (val))
#define e_llid16_19_cfg_VAL_llid16_dcrypt_mode(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_dcrypt_mode, (val))
#define e_llid16_19_cfg_VAL_llid16_oam_lpbk_en(val)            REG_FLD_VAL(e_llid16_19_cfg_FLD_llid16_oam_lpbk_en, (val))

#define e_llid20_23_cfg_VAL_llid23_dummy(val)                  REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_dummy, (val))
#define e_llid20_23_cfg_VAL_llid23_txfec_en(val)               REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_txfec_en, (val))
#define e_llid20_23_cfg_VAL_llid23_dcrypt_en(val)              REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_dcrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid23_dcrypt_mode(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_dcrypt_mode, (val))
#define e_llid20_23_cfg_VAL_llid23_oamlpbk_en(val)             REG_FLD_VAL(e_llid20_23_cfg_FLD_llid23_oamlpbk_en, (val))
#define e_llid20_23_cfg_VAL_llid22_dummy(val)                  REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_dummy, (val))
#define e_llid20_23_cfg_VAL_llid22_txfec_en(val)               REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_txfec_en, (val))
#define e_llid20_23_cfg_VAL_llid22_dcrypt_en(val)              REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_dcrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid22_dcrypt_mode(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_dcrypt_mode, (val))
#define e_llid20_23_cfg_VAL_llid22_oamlpbk_en(val)             REG_FLD_VAL(e_llid20_23_cfg_FLD_llid22_oamlpbk_en, (val))
#define e_llid20_23_cfg_VAL_llid21_dummy(val)                  REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_dummy, (val))
#define e_llid20_23_cfg_VAL_llid21_txfec_en(val)               REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_txfec_en, (val))
#define e_llid20_23_cfg_VAL_llid21_dcrypt_en(val)              REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_dcrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid21_dcrypt_mode(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_dcrypt_mode, (val))
#define e_llid20_23_cfg_VAL_llid21_oamlpbk_en(val)             REG_FLD_VAL(e_llid20_23_cfg_FLD_llid21_oamlpbk_en, (val))
#define e_llid20_23_cfg_VAL_llid20_dummy(val)                  REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_dummy, (val))
#define e_llid20_23_cfg_VAL_llid20_txfec_en(val)               REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_txfec_en, (val))
#define e_llid20_23_cfg_VAL_llid20_dcrypt_en(val)              REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_dcrypt_en, (val))
#define e_llid20_23_cfg_VAL_llid20_dcrypt_mode(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_dcrypt_mode, (val))
#define e_llid20_23_cfg_VAL_llid20_oam_lpbk_en(val)            REG_FLD_VAL(e_llid20_23_cfg_FLD_llid20_oam_lpbk_en, (val))

#define e_llid24_27_cfg_VAL_llid27_dummy(val)                  REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_dummy, (val))
#define e_llid24_27_cfg_VAL_llid27_txfec_en(val)               REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_txfec_en, (val))
#define e_llid24_27_cfg_VAL_llid27_dcrypt_en(val)              REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_dcrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid27_dcrypt_mode(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_dcrypt_mode, (val))
#define e_llid24_27_cfg_VAL_llid27_oamlpbk_en(val)             REG_FLD_VAL(e_llid24_27_cfg_FLD_llid27_oamlpbk_en, (val))
#define e_llid24_27_cfg_VAL_llid26_dummy(val)                  REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_dummy, (val))
#define e_llid24_27_cfg_VAL_llid26_txfec_en(val)               REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_txfec_en, (val))
#define e_llid24_27_cfg_VAL_llid26_dcrypt_en(val)              REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_dcrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid26_dcrypt_mode(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_dcrypt_mode, (val))
#define e_llid24_27_cfg_VAL_llid26_oamlpbk_en(val)             REG_FLD_VAL(e_llid24_27_cfg_FLD_llid26_oamlpbk_en, (val))
#define e_llid24_27_cfg_VAL_llid25_dummy(val)                  REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_dummy, (val))
#define e_llid24_27_cfg_VAL_llid25_txfec_en(val)               REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_txfec_en, (val))
#define e_llid24_27_cfg_VAL_llid25_dcrypt_en(val)              REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_dcrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid25_dcrypt_mode(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_dcrypt_mode, (val))
#define e_llid24_27_cfg_VAL_llid25_oamlpbk_en(val)             REG_FLD_VAL(e_llid24_27_cfg_FLD_llid25_oamlpbk_en, (val))
#define e_llid24_27_cfg_VAL_llid24_dummy(val)                  REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_dummy, (val))
#define e_llid24_27_cfg_VAL_llid24_txfec_en(val)               REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_txfec_en, (val))
#define e_llid24_27_cfg_VAL_llid24_dcrypt_en(val)              REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_dcrypt_en, (val))
#define e_llid24_27_cfg_VAL_llid24_dcrypt_mode(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_dcrypt_mode, (val))
#define e_llid24_27_cfg_VAL_llid24_oam_lpbk_en(val)            REG_FLD_VAL(e_llid24_27_cfg_FLD_llid24_oam_lpbk_en, (val))

#define e_llid28_31_cfg_VAL_llid31_dummy(val)                  REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_dummy, (val))
#define e_llid28_31_cfg_VAL_llid31_txfec_en(val)               REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_txfec_en, (val))
#define e_llid28_31_cfg_VAL_llid31_dcrypt_en(val)              REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_dcrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid31_dcrypt_mode(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_dcrypt_mode, (val))
#define e_llid28_31_cfg_VAL_llid31_oamlpbk_en(val)             REG_FLD_VAL(e_llid28_31_cfg_FLD_llid31_oamlpbk_en, (val))
#define e_llid28_31_cfg_VAL_llid30_dummy(val)                  REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_dummy, (val))
#define e_llid28_31_cfg_VAL_llid30_txfec_en(val)               REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_txfec_en, (val))
#define e_llid28_31_cfg_VAL_llid30_dcrypt_en(val)              REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_dcrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid30_dcrypt_mode(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_dcrypt_mode, (val))
#define e_llid28_31_cfg_VAL_llid30_oamlpbk_en(val)             REG_FLD_VAL(e_llid28_31_cfg_FLD_llid30_oamlpbk_en, (val))
#define e_llid28_31_cfg_VAL_llid29_dummy(val)                  REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_dummy, (val))
#define e_llid28_31_cfg_VAL_llid29_txfec_en(val)               REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_txfec_en, (val))
#define e_llid28_31_cfg_VAL_llid29_dcrypt_en(val)              REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_dcrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid29_dcrypt_mode(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_dcrypt_mode, (val))
#define e_llid28_31_cfg_VAL_llid29_oamlpbk_en(val)             REG_FLD_VAL(e_llid28_31_cfg_FLD_llid29_oamlpbk_en, (val))
#define e_llid28_31_cfg_VAL_llid28_dummy(val)                  REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_dummy, (val))
#define e_llid28_31_cfg_VAL_llid28_txfec_en(val)               REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_txfec_en, (val))
#define e_llid28_31_cfg_VAL_llid28_dcrypt_en(val)              REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_dcrypt_en, (val))
#define e_llid28_31_cfg_VAL_llid28_dcrypt_mode(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_dcrypt_mode, (val))
#define e_llid28_31_cfg_VAL_llid28_oam_lpbk_en(val)            REG_FLD_VAL(e_llid28_31_cfg_FLD_llid28_oam_lpbk_en, (val))

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

#define e_rpt_cfg2_VAL_llid15_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid15_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid14_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid14_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid13_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid13_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid12_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid12_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid11_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid11_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid10_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg2_FLD_llid10_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid9_rpt_cfg(val)                      REG_FLD_VAL(e_rpt_cfg2_FLD_llid9_rpt_cfg, (val))
#define e_rpt_cfg2_VAL_llid8_rpt_cfg(val)                      REG_FLD_VAL(e_rpt_cfg2_FLD_llid8_rpt_cfg, (val))

#define e_rpt_cfg3_VAL_llid31_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid31_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid30_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid30_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid29_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid29_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid28_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid28_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid27_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid27_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid26_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid26_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid25_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid25_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid24_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid24_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid23_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid23_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid22_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid22_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid21_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid21_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid20_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid20_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid19_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid19_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid18_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid18_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid17_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid17_rpt_cfg, (val))
#define e_rpt_cfg3_VAL_llid16_rpt_cfg(val)                     REG_FLD_VAL(e_rpt_cfg3_FLD_llid16_rpt_cfg, (val))

#define e_rpt_qthld_cfg2_VAL_rpt_llid_idx_4_3(val)             REG_FLD_VAL(e_rpt_qthld_cfg2_FLD_rpt_llid_idx_4_3, (val))

#define e_rpt_bitmap_cfg_VAL_rpt_bmap_rwcmd(val)               REG_FLD_VAL(e_rpt_bitmap_cfg_FLD_rpt_bmap_rwcmd, (val))
#define e_rpt_bitmap_cfg_VAL_rpt_bmap_rwcmd_done(val)          REG_FLD_VAL(e_rpt_bitmap_cfg_FLD_rpt_bmap_rwcmd_done, (val))
#define e_rpt_bitmap_cfg_VAL_rpt_bmap_llid_idx(val)            REG_FLD_VAL(e_rpt_bitmap_cfg_FLD_rpt_bmap_llid_idx, (val))

#define e_rpt_bitmap_val_VAL_rpt_qsizeadj_frc(val)             REG_FLD_VAL(e_rpt_bitmap_val_FLD_rpt_qsizeadj_frc, (val))
#define e_rpt_bitmap_val_VAL_rpt_bitmap_ctrl(val)              REG_FLD_VAL(e_rpt_bitmap_val_FLD_rpt_bitmap_ctrl, (val))
#define e_rpt_bitmap_val_VAL_rpt_bitmap_set(val)               REG_FLD_VAL(e_rpt_bitmap_val_FLD_rpt_bitmap_set, (val))

#ifdef __cplusplus
}
#endif

#endif // __EPON_MAC_REGS_H__
