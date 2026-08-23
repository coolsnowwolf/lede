/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

#ifndef __xgpon_mac_reg_REGS_H__
#define __xgpon_mac_reg_REGS_H__

/*#include "common.h"*/
#include <linux/mtd/rt_flash.h>
#include "../common/drv_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef REG_BASE_C_MODULE
// ----------------- xgpon_mac_reg Bit Field Definitions -------------------

//#define PACKING
//typedef unsigned int FIELD;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD xgpon_mac_sw_rst_n        : 1;
#else
        FIELD xgpon_mac_sw_rst_n        : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SW_RST, *PREG_SW_RST;

typedef PACKING union
{
    PACKING struct
    {
#if defined(TCSUPPORT_CPU_EN7581)
#ifdef __BIG_ENDIAN
        FIELD mpi_tx_stop_done          : 1;
        FIELD mpi_rx_stop_done          : 1;
        FIELD rsv_25                    : 5;
        FIELD mpi_tx_stop               : 1;
        FIELD rsv_17                    : 7;
        FIELD mpi_rx_stop               : 1;
        FIELD mbi_tx_stop_done          : 1;
        FIELD mbi_rx_stop_done          : 1;
        FIELD dbru_stop_done            : 1;
        FIELD dbru_stop                 : 1;
        FIELD rsv_9                     : 3;
        FIELD mbi_tx_stop               : 1;
        FIELD rsv_5                     : 3;
        FIELD del_rx_stop               : 1;
        FIELD rsv_1                     : 3;
        FIELD mbi_rx_stop               : 1;
#else
        FIELD mbi_rx_stop               : 1;
        FIELD rsv_1                     : 3;
        FIELD del_rx_stop               : 1;
        FIELD rsv_5                     : 3;
        FIELD mbi_tx_stop               : 1;
        FIELD rsv_9                     : 3;
        FIELD dbru_stop                 : 1;
        FIELD dbru_stop_done            : 1;
        FIELD mbi_rx_stop_done          : 1;
        FIELD mbi_tx_stop_done          : 1;
        FIELD mpi_rx_stop               : 1;
        FIELD rsv_17                    : 7;
        FIELD mpi_tx_stop               : 1;
        FIELD rsv_25                    : 5;
        FIELD mpi_rx_stop_done          : 1;
        FIELD mpi_tx_stop_done          : 1;
#endif /* __BIG_ENDIAN */
#else
#ifdef __BIG_ENDIAN
        FIELD mpi_tx_stop_done          : 1;
        FIELD mpi_rx_stop_done          : 1;
        FIELD rsv_25                    : 5;
        FIELD mpi_tx_stop               : 1;
        FIELD rsv_17                    : 7;
        FIELD mpi_rx_stop               : 1;
        FIELD mbi_tx_stop_done          : 1;
        FIELD mbi_rx_stop_done          : 1;
        FIELD rsv_9                     : 5;
        FIELD mbi_tx_stop               : 1;
        FIELD rsv_1                     : 7;
        FIELD mbi_rx_stop               : 1;
#else
        FIELD mbi_rx_stop               : 1;
        FIELD rsv_1                     : 7;
        FIELD mbi_tx_stop               : 1;
        FIELD rsv_9                     : 5;
        FIELD mbi_rx_stop_done          : 1;
        FIELD mbi_tx_stop_done          : 1;
        FIELD mpi_rx_stop               : 1;
        FIELD rsv_17                    : 7;
        FIELD mpi_tx_stop               : 1;
        FIELD rsv_25                    : 5;
        FIELD mpi_rx_stop_done          : 1;
        FIELD mpi_tx_stop_done          : 1;
#endif /* __BIG_ENDIAN */
#endif
    } Bits;
    UINT32 Raw;
} REG_MBI_MPI_STOP, *PREG_MBI_MPI_STOP;

typedef PACKING union
{
    PACKING struct
    {
        FIELD vendor_id                 : 32;
    } Bits;
    UINT32 Raw;
} REG_VENDOR_ID, *PREG_VENDOR_ID;

typedef PACKING union
{
    PACKING struct
    {
        FIELD vs_sn                     : 32;
    } Bits;
    UINT32 Raw;
} REG_VS_SN, *PREG_VS_SN;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD onu_id_vld                : 1;
        FIELD rsv_10                    : 5;
        FIELD onu_id                    : 10;
#else
        FIELD onu_id                    : 10;
        FIELD rsv_10                    : 5;
        FIELD onu_id_vld                : 1;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_ONU_ID, *PREG_ONU_ID;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rgs_id3_0                 : 32;
    } Bits;
    UINT32 Raw;
} REG_RGS_ID3_0, *PREG_RGS_ID3_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rgs_id7_4                 : 32;
    } Bits;
    UINT32 Raw;
} REG_RGS_ID7_4, *PREG_RGS_ID7_4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rgs_id11_8                : 32;
    } Bits;
    UINT32 Raw;
} REG_RGS_ID11_8, *PREG_RGS_ID11_8;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rgs_id15_12               : 32;
    } Bits;
    UINT32 Raw;
} REG_RGS_ID15_12, *PREG_RGS_ID15_12;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rgs_id19_16               : 32;
    } Bits;
    UINT32 Raw;
} REG_RGS_ID19_16, *PREG_RGS_ID19_16;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rgs_id23_20               : 32;
    } Bits;
    UINT32 Raw;
} REG_RGS_ID23_20, *PREG_RGS_ID23_20;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rgs_id27_24               : 32;
    } Bits;
    UINT32 Raw;
} REG_RGS_ID27_24, *PREG_RGS_ID27_24;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rgs_id31_28               : 32;
    } Bits;
    UINT32 Raw;
} REG_RGS_ID31_28, *PREG_RGS_ID31_28;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rgs_id35_32               : 32;
    } Bits;
    UINT32 Raw;
} REG_RGS_ID35_32, *PREG_RGS_ID35_32;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
#if defined(TCSUPPORT_CPU_EN7581)
#if defined(TCSUPPORT_CPU_AN7583)
        FIELD rsv_28                    : 4;
        FIELD mon_not_gnt_int_en        : 1;
#else
        FIELD rsv_27                    : 5;
#endif
        FIELD cal_gnt_zero_int_en       : 1;
#else
        FIELD rsv_26                    : 6;
#endif
        FIELD o5_sn_onu_req_recv_int_en : 1;
        FIELD turning_sfc_match_int_en  : 1;
        FIELD o9_gnt_recv_int_en        : 1;
        FIELD sw1_mic_done_int_en       : 1;
        FIELD sw0_mic_done_int_en       : 1;
        FIELD key_cal_done_int_en       : 1;
        FIELD lwi_int_en                : 1;
        FIELD fwi_int_en                : 1;
        FIELD rx_err_int_en             : 1;
        FIELD tx_err_int_en             : 1;
        FIELD fifo_err_int_en           : 1;
        FIELD o5_eqd_adj_done_int_en    : 1;
        FIELD bwm_chk_err_int_en        : 1;
        FIELD dying_gasp_send_int_en    : 1;
        FIELD tod_1pps_int_en           : 1;
        FIELD tod_update_done_int_en    : 1;
        FIELD olt_ds_fec_chg_int_en     : 1;
        FIELD us_prof_idx_chg_int_en    : 1;
        FIELD us_key_switch_done_int_en : 1;
        FIELD us_no_msg_send_int_en     : 1;
        FIELD o4_registration_send_int_en : 1;
        FIELD o4_ranging_req_recv_int_en : 1;
        FIELD o23_sn_onu_send_int_en    : 1;
        FIELD o23_sn_onu_req_recv_int_en : 1;
        FIELD ploamu_send_int_en        : 1;
        FIELD ploamd_recv_int_en        : 1;
#else
        FIELD ploamd_recv_int_en        : 1;
        FIELD ploamu_send_int_en        : 1;
        FIELD o23_sn_onu_req_recv_int_en : 1;
        FIELD o23_sn_onu_send_int_en    : 1;
        FIELD o4_ranging_req_recv_int_en : 1;
        FIELD o4_registration_send_int_en : 1;
        FIELD us_no_msg_send_int_en     : 1;
        FIELD us_key_switch_done_int_en : 1;
        FIELD us_prof_idx_chg_int_en    : 1;
        FIELD olt_ds_fec_chg_int_en     : 1;
        FIELD tod_update_done_int_en    : 1;
        FIELD tod_1pps_int_en           : 1;
        FIELD dying_gasp_send_int_en    : 1;
        FIELD bwm_chk_err_int_en        : 1;
        FIELD o5_eqd_adj_done_int_en    : 1;
        FIELD fifo_err_int_en           : 1;
        FIELD tx_err_int_en             : 1;
        FIELD rx_err_int_en             : 1;
        FIELD fwi_int_en                : 1;
        FIELD lwi_int_en                : 1;
        FIELD key_cal_done_int_en       : 1;
        FIELD sw0_mic_done_int_en       : 1;
        FIELD sw1_mic_done_int_en       : 1;
        FIELD o9_gnt_recv_int_en        : 1;
        FIELD turning_sfc_match_int_en  : 1;
        FIELD o5_sn_onu_req_recv_int_en : 1;
#if defined(TCSUPPORT_CPU_EN7581)
        FIELD cal_gnt_zero_int_en       : 1;
#if defined(TCSUPPORT_CPU_AN7583)
        FIELD mon_not_gnt_int_en        : 1;
        FIELD rsv_28                    : 4;        
#else
        FIELD rsv_27                    : 5;
#endif
#else
        FIELD rsv_26                    : 6;
#endif
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_INT_ENABLE, *PREG_INT_ENABLE;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
#if defined(TCSUPPORT_CPU_EN7581)
        FIELD rsv_27                    : 4;
        FIELD mon_not_gnt_int           : 1;
        FIELD cal_gnt_zero_int          : 1;
#else
        FIELD rsv_26                    : 6;
#endif
        FIELD o5_sn_onu_req_recv_int    : 1;
        FIELD turning_sfc_match_int     : 1;
        FIELD o9_gnt_recv_int           : 1;
        FIELD sw1_mic_done_int          : 1;
        FIELD sw0_mic_done_int          : 1;
        FIELD key_cal_done_int          : 1;
        FIELD lwi_int                   : 1;
        FIELD fwi_int                   : 1;
        FIELD rx_err_int                : 1;
        FIELD tx_err_int                : 1;
        FIELD fifo_err_int              : 1;
        FIELD o5_eqd_adj_done_int       : 1;
        FIELD bwm_chk_err_int           : 1;
        FIELD dying_gasp_send_int       : 1;
        FIELD tod_1pps_int              : 1;
        FIELD tod_update_done_int       : 1;
        FIELD olt_ds_fec_chg_int        : 1;
        FIELD us_prof_idx_chg_int       : 1;
        FIELD us_key_switch_done_int    : 1;
        FIELD us_no_msg_send_int        : 1;
        FIELD o4_registration_send_int  : 1;
        FIELD o4_ranging_req_recv_int   : 1;
        FIELD o23_sn_onu_send_int       : 1;
        FIELD o23_sn_onu_req_recv_int   : 1;
        FIELD ploamu_send_int           : 1;
        FIELD ploamd_recv_int           : 1;
#else
        FIELD ploamd_recv_int           : 1;
        FIELD ploamu_send_int           : 1;
        FIELD o23_sn_onu_req_recv_int   : 1;
        FIELD o23_sn_onu_send_int       : 1;
        FIELD o4_ranging_req_recv_int   : 1;
        FIELD o4_registration_send_int  : 1;
        FIELD us_no_msg_send_int        : 1;
        FIELD us_key_switch_done_int    : 1;
        FIELD us_prof_idx_chg_int       : 1;
        FIELD olt_ds_fec_chg_int        : 1;
        FIELD tod_update_done_int       : 1;
        FIELD tod_1pps_int              : 1;
        FIELD dying_gasp_send_int       : 1;
        FIELD bwm_chk_err_int           : 1;
        FIELD o5_eqd_adj_done_int       : 1;
        FIELD fifo_err_int              : 1;
        FIELD tx_err_int                : 1;
        FIELD rx_err_int                : 1;
        FIELD fwi_int                   : 1;
        FIELD lwi_int                   : 1;
        FIELD key_cal_done_int          : 1;
        FIELD sw0_mic_done_int          : 1;
        FIELD sw1_mic_done_int          : 1;
        FIELD o9_gnt_recv_int           : 1;
        FIELD turning_sfc_match_int     : 1;
        FIELD o5_sn_onu_req_recv_int    : 1;
#if defined(TCSUPPORT_CPU_EN7581)
        FIELD cal_gnt_zero_int          : 1;
		FIELD mon_not_gnt_int			: 1;
        FIELD rsv_27                    : 4;		
#else
        FIELD rsv_26                    : 6;
#endif
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_INT_STATUS, *PREG_INT_STATUS;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_18                    : 14;
        FIELD tx_aes_rdm_ciph_fifo_ovrn : 1;
        FIELD byte_mib_cmd_fifo_ovrn    : 1;
        FIELD frm_mib_cmd_fifo_ovrn     : 1;
        FIELD mib_cmd_fifo_ovrn         : 1;
        FIELD mib_tx_cmd_fifo_ovrn      : 1;
        FIELD snf_ds_fifo_ovrn          : 1;
        FIELD snf_us_fifo_ovrn          : 1;
        FIELD rx_ploamd_fifo_udrn       : 1;
        FIELD rx_ploamd_fifo_ovrn       : 1;
        FIELD rx_mbi_pl_fifo_ovrn       : 1;
        FIELD rx_mbi_hdr_fifo_ovrn      : 1;
        FIELD rx_aes_rdm_ciph_fifo_ovrn : 1;
        FIELD rx_aes_ciph_txt_fifo_ovrn : 1;
        FIELD rx_aes_cryp_cnt_fifo_ovrn : 1;
        FIELD tx_align_fifo_udrn        : 1;
        FIELD tx_ploamu_fifo_ovrn       : 1;
        FIELD bst_fifo_ovrn             : 1;
        FIELD sgl_fifo_ovrn             : 1;
#else
        FIELD sgl_fifo_ovrn             : 1;
        FIELD bst_fifo_ovrn             : 1;
        FIELD tx_ploamu_fifo_ovrn       : 1;
        FIELD tx_align_fifo_udrn        : 1;
        FIELD rx_aes_cryp_cnt_fifo_ovrn : 1;
        FIELD rx_aes_ciph_txt_fifo_ovrn : 1;
        FIELD rx_aes_rdm_ciph_fifo_ovrn : 1;
        FIELD rx_mbi_hdr_fifo_ovrn      : 1;
        FIELD rx_mbi_pl_fifo_ovrn       : 1;
        FIELD rx_ploamd_fifo_ovrn       : 1;
        FIELD rx_ploamd_fifo_udrn       : 1;
        FIELD snf_us_fifo_ovrn          : 1;
        FIELD snf_ds_fifo_ovrn          : 1;
        FIELD mib_tx_cmd_fifo_ovrn      : 1;
        FIELD mib_cmd_fifo_ovrn         : 1;
        FIELD frm_mib_cmd_fifo_ovrn     : 1;
        FIELD byte_mib_cmd_fifo_ovrn    : 1;
        FIELD tx_aes_rdm_ciph_fifo_ovrn : 1;
        FIELD rsv_18                    : 14;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_FIFO_ERR_STS, *PREG_FIFO_ERR_STS;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_3                     : 29;
        FIELD tx_prof_invld_err         : 1;
        FIELD tx_late_start_err         : 1;
        FIELD tx_bst_sgl_diff_err       : 1;
#else
        FIELD tx_bst_sgl_diff_err       : 1;
        FIELD tx_late_start_err         : 1;
        FIELD tx_prof_invld_err         : 1;
        FIELD rsv_3                     : 29;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_TX_ERR_STS, *PREG_TX_ERR_STS;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_14                    : 18;
        FIELD rx_xgem_hdr_hec_err       : 1;
        FIELD rx_alloc_hec_err          : 1;
        FIELD rx_mbi_xgem_drop_err      : 1;
        FIELD rx_omci_mic_err           : 1;
        FIELD rx_ploam_mic_err          : 1;
        FIELD rx_eth_crc_err            : 1;
        FIELD rx_aes_key_err            : 1;
        FIELD rx_gem_intlv_err          : 1;
        FIELD rx_los_gem_del_err        : 1;
        FIELD rx_bip_err                : 1;
        FIELD rx_hlend_hec_err          : 1;
        FIELD rx_pon_id_hec_err         : 1;
        FIELD rx_sfc_hec_uc_err         : 1;
        FIELD rx_eof_err                : 1;
#else
        FIELD rx_eof_err                : 1;
        FIELD rx_sfc_hec_uc_err         : 1;
        FIELD rx_pon_id_hec_err         : 1;
        FIELD rx_hlend_hec_err          : 1;
        FIELD rx_bip_err                : 1;
        FIELD rx_los_gem_del_err        : 1;
        FIELD rx_gem_intlv_err          : 1;
        FIELD rx_aes_key_err            : 1;
        FIELD rx_eth_crc_err            : 1;
        FIELD rx_ploam_mic_err          : 1;
        FIELD rx_omci_mic_err           : 1;
        FIELD rx_mbi_xgem_drop_err      : 1;
        FIELD rx_alloc_hec_err          : 1;
        FIELD rx_xgem_hdr_hec_err       : 1;
        FIELD rsv_14                    : 18;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_ERR_STS, *PREG_RX_ERR_STS;

#if defined(TCSUPPORT_CPU_AN7583)
typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_18                    : 14;
        FIELD tx_aes_rdm_ciph_fifo_ovrn_en : 1;
        FIELD byte_mib_cmd_fifo_ovrn_en : 1;
        FIELD frm_mib_cmd_fifo_ovrn_en  : 1;
        FIELD mib_cmd_fifo_ovrn_en      : 1;
        FIELD mib_tx_cmd_fifo_ovrn_en   : 1;
        FIELD snf_ds_fifo_ovrn_en       : 1;
        FIELD snf_us_fifo_ovrn_en       : 1;
        FIELD rx_ploamd_fifo_udrn_en    : 1;
        FIELD rx_ploamd_fifo_ovrn_en    : 1;
        FIELD rx_mbi_pl_fifo_ovrn_en    : 1;
        FIELD rx_mbi_hdr_fifo_ovrn_en   : 1;
        FIELD rx_aes_rdm_ciph_fifo_ovrn_en : 1;
        FIELD rx_aes_ciph_txt_fifo_ovrn_en : 1;
        FIELD rx_aes_cryp_cnt_fifo_ovrn_en : 1;
        FIELD tx_align_fifo_udrn_en     : 1;
        FIELD tx_ploamu_fifo_ovrn_en    : 1;
        FIELD bst_fifo_ovrn_en          : 1;
        FIELD sgl_fifo_ovrn_en          : 1;
#else
        FIELD sgl_fifo_ovrn_en          : 1;
        FIELD bst_fifo_ovrn_en          : 1;
        FIELD tx_ploamu_fifo_ovrn_en    : 1;
        FIELD tx_align_fifo_udrn_en     : 1;
        FIELD rx_aes_cryp_cnt_fifo_ovrn_en : 1;
        FIELD rx_aes_ciph_txt_fifo_ovrn_en : 1;
        FIELD rx_aes_rdm_ciph_fifo_ovrn_en : 1;
        FIELD rx_mbi_hdr_fifo_ovrn_en   : 1;
        FIELD rx_mbi_pl_fifo_ovrn_en    : 1;
        FIELD rx_ploamd_fifo_ovrn_en    : 1;
        FIELD rx_ploamd_fifo_udrn_en    : 1;
        FIELD snf_us_fifo_ovrn_en       : 1;
        FIELD snf_ds_fifo_ovrn_en       : 1;
        FIELD mib_tx_cmd_fifo_ovrn_en   : 1;
        FIELD mib_cmd_fifo_ovrn_en      : 1;
        FIELD frm_mib_cmd_fifo_ovrn_en  : 1;
        FIELD byte_mib_cmd_fifo_ovrn_en : 1;
        FIELD tx_aes_rdm_ciph_fifo_ovrn_en : 1;
        FIELD rsv_18                    : 14;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_FIFO_ERR_ENABLE, *PREG_FIFO_ERR_ENABLE;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_3                     : 29;
        FIELD tx_prof_invld_err_en      : 1;
        FIELD tx_late_start_err_en      : 1;
        FIELD tx_bst_sgl_diff_err_en    : 1;
#else
        FIELD tx_bst_sgl_diff_err_en    : 1;
        FIELD tx_late_start_err_en      : 1;
        FIELD tx_prof_invld_err_en      : 1;
        FIELD rsv_3                     : 29;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_TX_ERR_ENABLE, *PREG_TX_ERR_ENABLE;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_14                    : 18;
        FIELD rx_xgem_hdr_hec_err_en    : 1;
        FIELD rx_alloc_hec_err_en       : 1;
        FIELD rx_mbi_xgem_drop_err_en   : 1;
        FIELD rx_omci_mic_err_en        : 1;
        FIELD rx_ploam_mic_err_en       : 1;
        FIELD rx_eth_crc_err_en         : 1;
        FIELD rx_aes_key_err_en         : 1;
        FIELD rx_gem_intlv_err_en       : 1;
        FIELD rx_los_gem_del_err_en     : 1;
        FIELD rx_bip_err_en             : 1;
        FIELD rx_hlend_hec_err_en       : 1;
        FIELD rx_pon_id_hec_err_en      : 1;
        FIELD rx_sfc_hec_uc_err_en      : 1;
        FIELD rx_eof_err_en             : 1;
#else
        FIELD rx_eof_err_en             : 1;
        FIELD rx_sfc_hec_uc_err_en      : 1;
        FIELD rx_pon_id_hec_err_en      : 1;
        FIELD rx_hlend_hec_err_en       : 1;
        FIELD rx_bip_err_en             : 1;
        FIELD rx_los_gem_del_err_en     : 1;
        FIELD rx_gem_intlv_err_en       : 1;
        FIELD rx_aes_key_err_en         : 1;
        FIELD rx_eth_crc_err_en         : 1;
        FIELD rx_ploam_mic_err_en       : 1;
        FIELD rx_omci_mic_err_en        : 1;
        FIELD rx_mbi_xgem_drop_err_en   : 1;
        FIELD rx_alloc_hec_err_en       : 1;
        FIELD rx_xgem_hdr_hec_err_en    : 1;
        FIELD rsv_14                    : 18;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_ERR_ENABLE, *PREG_RX_ERR_ENABLE;
#endif

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD cor_tag                   : 16;
#else
        FIELD cor_tag                   : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_COR_TAG, *PREG_COR_TAG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cur_ds_pon_id             : 32;
    } Bits;
    UINT32 Raw;
} REG_CUR_DS_PON_ID, *PREG_CUR_DS_PON_ID;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cur_us_pon_id             : 32;
    } Bits;
    UINT32 Raw;
} REG_CUR_US_PON_ID, *PREG_CUR_US_PON_ID;

typedef PACKING union
{
    PACKING struct
    {
        FIELD calib_sts_l32             : 32;
    } Bits;
    UINT32 Raw;
} REG_CALIB_STS_L32, *PREG_CALIB_STS_L32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD calib_sts_h32             : 32;
    } Bits;
    UINT32 Raw;
} REG_CALIB_STS_H32, *PREG_CALIB_STS_H32;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_8                     : 24;
        FIELD turning_gran              : 8;
#else
        FIELD turning_gran              : 8;
        FIELD rsv_8                     : 24;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_TUNING_GRAN, *PREG_TUNING_GRAN;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_8                     : 24;
        FIELD step_turning_time         : 8;
#else
        FIELD step_turning_time         : 8;
        FIELD rsv_8                     : 24;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_STEP_TUNING_TIME, *PREG_STEP_TUNING_TIME;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_2                     : 30;
        FIELD us_10g_cap                : 1;
        FIELD us_2p5g_cap               : 1;
#else
        FIELD us_2p5g_cap               : 1;
        FIELD us_10g_cap                : 1;
        FIELD rsv_2                     : 30;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_US_RATE_CAP, *PREG_US_RATE_CAP;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_8                     : 24;
        FIELD attenuation               : 8;
#else
        FIELD attenuation               : 8;
        FIELD rsv_8                     : 24;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_ATTENUATION, *PREG_ATTENUATION;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_8                     : 24;
        FIELD power_level_cap           : 8;
#else
        FIELD power_level_cap           : 8;
        FIELD rsv_8                     : 24;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_POWER_LEVEL_CAP, *PREG_POWER_LEVEL_CAP;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 24;
        FIELD act_debug_info            : 8;
#else
        FIELD act_debug_info            : 8;
        FIELD rsv_24                    : 24;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_ACT_DEBUG_INFO, *PREG_ACT_DEBUG_INFO;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD o23_o4_ploamu_ctrl        : 1;
#else
        FIELD o23_o4_ploamu_ctrl        : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_O23_O4_PLOAMU_CTRL, *PREG_O23_O4_PLOAMU_CTRL;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_4                     : 28;
        FIELD act_st                    : 4;
#else
        FIELD act_st                    : 4;
        FIELD rsv_4                     : 28;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_ACTIVATION_ST, *PREG_ACTIVATION_ST;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_14                    : 18;
        FIELD tresp                     : 14;
#else
        FIELD tresp                     : 14;
        FIELD rsv_14                    : 18;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RSP_TIME, *PREG_RSP_TIME;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_28                    : 4;
        FIELD max_rdm_dly               : 12;
        FIELD rsv_12                    : 4;
        FIELD rdm_dly                   : 12;
#else
        FIELD rdm_dly                   : 12;
        FIELD rsv_12                    : 4;
        FIELD max_rdm_dly               : 12;
        FIELD rsv_28                    : 4;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RDM_DLY, *PREG_RDM_DLY;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD phy_rx_dly                : 16;
        FIELD phy_tx_dly                : 16;
#else
        FIELD phy_tx_dly                : 16;
        FIELD phy_rx_dly                : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_INT_DLY, *PREG_INT_DLY;

typedef PACKING union
{
    PACKING struct
    {
        FIELD eqd                       : 32;
    } Bits;
    UINT32 Raw;
} REG_EQD, *PREG_EQD;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_10                    : 22;
        FIELD syn_mrg                   : 10;
#else
        FIELD syn_mrg                   : 10;
        FIELD rsv_10                    : 22;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SYN_MRG, *PREG_SYN_MRG;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD us_prof3_vsn              : 4;
        FIELD rsv_25                    : 3;
        FIELD us_prof3_vld              : 1;
        FIELD us_prof2_vsn              : 4;
        FIELD rsv_17                    : 3;
        FIELD us_prof2_vld              : 1;
        FIELD us_prof1_vsn              : 4;
        FIELD rsv_9                     : 3;
        FIELD us_prof1_vld              : 1;
        FIELD us_prof0_vsn              : 4;
        FIELD rsv_1                     : 3;
        FIELD us_prof0_vld              : 1;
#else
        FIELD us_prof0_vld              : 1;
        FIELD rsv_1                     : 3;
        FIELD us_prof0_vsn              : 4;
        FIELD us_prof1_vld              : 1;
        FIELD rsv_9                     : 3;
        FIELD us_prof1_vsn              : 4;
        FIELD us_prof2_vld              : 1;
        FIELD rsv_17                    : 3;
        FIELD us_prof2_vsn              : 4;
        FIELD us_prof3_vld              : 1;
        FIELD rsv_25                    : 3;
        FIELD us_prof3_vsn              : 4;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_US_PROF_VLD, *PREG_US_PROF_VLD;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD us_psbu_len_prof1         : 16;
        FIELD us_psbu_len_prof0         : 16;
#else
        FIELD us_psbu_len_prof0         : 16;
        FIELD us_psbu_len_prof1         : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_US_PROF_PSBu_LEN_0_1, *PREG_US_PROF_PSBu_LEN_0_1;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD us_psbu_len_prof3         : 16;
        FIELD us_psbu_len_prof2         : 16;
#else
        FIELD us_psbu_len_prof2         : 16;
        FIELD us_psbu_len_prof3         : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_US_PROF_PSBu_LEN_2_3, *PREG_US_PROF_PSBu_LEN_2_3;

#if defined(TCSUPPORT_CPU_EN7581)
typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD use_ext_rdm_dly           : 1;
        FIELD ext_max_rdm_dly           : 11;
        FIELD rsv_17                    : 3;
        FIELD ext_rdm_dly               : 17;
#else
        FIELD ext_rdm_dly               : 17;
        FIELD rsv_17                    : 3;
        FIELD ext_max_rdm_dly           : 11;
        FIELD use_ext_rdm_dly           : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_EXT_RDM_DLY, *PREG_EXT_RDM_DLY;
#endif

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD us_aes_key_vld            : 1;
        FIELD rsv_1                     : 30;
        FIELD us_aes_key_idx            : 1;
#else
        FIELD us_aes_key_idx            : 1;
        FIELD rsv_1                     : 30;
        FIELD us_aes_key_vld            : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_US_AES_KEY_CTRL, *PREG_US_AES_KEY_CTRL;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_4                     : 28;
        FIELD ds_aes_bc_idx1_key_vld    : 1;
        FIELD ds_aes_bc_idx0_key_vld    : 1;
        FIELD ds_aes_uc_idx1_key_vld    : 1;
        FIELD ds_aes_uc_idx0_key_vld    : 1;
#else
        FIELD ds_aes_uc_idx0_key_vld    : 1;
        FIELD ds_aes_uc_idx1_key_vld    : 1;
        FIELD ds_aes_bc_idx0_key_vld    : 1;
        FIELD ds_aes_bc_idx1_key_vld    : 1;
        FIELD rsv_4                     : 28;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DS_AES_KEY_VLD, *PREG_DS_AES_KEY_VLD;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD deft_gpid_enc             : 1;
#else
        FIELD deft_gpid_enc             : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DEFT_GPID_ENC, *PREG_DEFT_GPID_ENC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_uc_idx0_key0          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_UC_IDX0_KEY0, *PREG_AES_UC_IDX0_KEY0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_uc_idx0_key1          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_UC_IDX0_KEY1, *PREG_AES_UC_IDX0_KEY1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_uc_idx0_key2          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_UC_IDX0_KEY2, *PREG_AES_UC_IDX0_KEY2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_uc_idx0_key3          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_UC_IDX0_KEY3, *PREG_AES_UC_IDX0_KEY3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_uc_idx1_key0          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_UC_IDX1_KEY0, *PREG_AES_UC_IDX1_KEY0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_uc_idx1_key1          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_UC_IDX1_KEY1, *PREG_AES_UC_IDX1_KEY1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_uc_idx1_key2          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_UC_IDX1_KEY2, *PREG_AES_UC_IDX1_KEY2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_uc_idx1_key3          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_UC_IDX1_KEY3, *PREG_AES_UC_IDX1_KEY3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_bc_idx0_key0          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_BC_IDX0_KEY0, *PREG_AES_BC_IDX0_KEY0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_bc_idx0_key1          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_BC_IDX0_KEY1, *PREG_AES_BC_IDX0_KEY1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_bc_idx0_key2          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_BC_IDX0_KEY2, *PREG_AES_BC_IDX0_KEY2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_bc_idx0_key3          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_BC_IDX0_KEY3, *PREG_AES_BC_IDX0_KEY3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_bc_idx1_key0          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_BC_IDX1_KEY0, *PREG_AES_BC_IDX1_KEY0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_bc_idx1_key1          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_BC_IDX1_KEY1, *PREG_AES_BC_IDX1_KEY1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_bc_idx1_key2          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_BC_IDX1_KEY2, *PREG_AES_BC_IDX1_KEY2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD aes_bc_idx1_key3          : 32;
    } Bits;
    UINT32 Raw;
} REG_AES_BC_IDX1_KEY3, *PREG_AES_BC_IDX1_KEY3;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD tcont_cmd                 : 1;
        FIELD rsv_25                    : 6;
        FIELD tcont_id_index            : 5;
        FIELD rsv_17                    : 3;
        FIELD wr_tcont_id_vld           : 1;
        FIELD rsv_14                    : 2;
        FIELD wr_tcont_id               : 14;
#else
        FIELD wr_tcont_id               : 14;
        FIELD rsv_14                    : 2;
        FIELD wr_tcont_id_vld           : 1;
        FIELD rsv_17                    : 3;
        FIELD tcont_id_index            : 5;
        FIELD rsv_25                    : 6;
        FIELD tcont_cmd                 : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_TCONT_ID_CFG, *PREG_TCONT_ID_CFG;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD tcont_cmd_done            : 1;
        FIELD rsv_17                    : 14;
        FIELD rd_tcont_id_vld           : 1;
        FIELD rsv_14                    : 2;
        FIELD rd_tcont_id               : 14;
#else
        FIELD rd_tcont_id               : 14;
        FIELD rsv_14                    : 2;
        FIELD rd_tcont_id_vld           : 1;
        FIELD rsv_17                    : 14;
        FIELD tcont_cmd_done            : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_TCONT_ID_STS, *PREG_TCONT_ID_STS;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_9                     : 23;
        FIELD gpidx_tbl_init_done       : 1;
        FIELD rsv_1                     : 7;
        FIELD gpidx_tbl_init_start      : 1;
#else
        FIELD gpidx_tbl_init_start      : 1;
        FIELD rsv_1                     : 7;
        FIELD gpidx_tbl_init_done       : 1;
        FIELD rsv_9                     : 23;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_GPIDX_TBL_INIT, *PREG_GPIDX_TBL_INIT;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD gpidx_tbl_cmd             : 1;
        FIELD rsv_27                    : 4;
        FIELD gpidx_tbl_addr            : 11;
        FIELD rsv_9                     : 7;
        FIELD gpidx_tbl_wdata           : 9;
#else
        FIELD gpidx_tbl_wdata           : 9;
        FIELD rsv_9                     : 7;
        FIELD gpidx_tbl_addr            : 11;
        FIELD rsv_27                    : 4;
        FIELD gpidx_tbl_cmd             : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_GPIDX_TBL_CTRL, *PREG_GPIDX_TBL_CTRL;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD gpidx_cmd_done            : 1;
        FIELD rsv_9                     : 22;
        FIELD gpidx_tbl_rdata           : 9;
#else
        FIELD gpidx_tbl_rdata           : 9;
        FIELD rsv_9                     : 22;
        FIELD gpidx_cmd_done            : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_GPIDX_TBL_STS, *PREG_GPIDX_TBL_STS;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_9                     : 23;
        FIELD gem_tbl_init_done         : 1;
        FIELD rsv_1                     : 7;
        FIELD gem_tbl_init_start        : 1;
#else
        FIELD gem_tbl_init_start        : 1;
        FIELD rsv_1                     : 7;
        FIELD gem_tbl_init_done         : 1;
        FIELD rsv_9                     : 23;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_GEM_TBL_INIT, *PREG_GEM_TBL_INIT;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD gpid_cmd                  : 1;
        FIELD rsv_19                    : 12;
        FIELD gpid_vld                  : 1;
        FIELD gpid_type                 : 1;
        FIELD gpid_us_encrypt           : 1;
        FIELD gem_port_id               : 16;
#else
        FIELD gem_port_id               : 16;
        FIELD gpid_us_encrypt           : 1;
        FIELD gpid_type                 : 1;
        FIELD gpid_vld                  : 1;
        FIELD rsv_19                    : 12;
        FIELD gpid_cmd                  : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_GEM_PORT_CFG, *PREG_GEM_PORT_CFG;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD gpid_cmd_done             : 1;
        FIELD rsv_3                     : 28;
        FIELD gpid_rd_sts               : 3;
#else
        FIELD gpid_rd_sts               : 3;
        FIELD rsv_3                     : 28;
        FIELD gpid_cmd_done             : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_GEM_PORT_STS, *PREG_GEM_PORT_STS;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_9                     : 23;
        FIELD tx_fcs_tbl_init_done      : 1;
        FIELD rsv_1                     : 7;
        FIELD tx_fcs_tbl_init_start     : 1;
#else
        FIELD tx_fcs_tbl_init_start     : 1;
        FIELD rsv_1                     : 7;
        FIELD tx_fcs_tbl_init_done      : 1;
        FIELD rsv_9                     : 23;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_G_TX_FCS_TBL_INIT, *PREG_G_TX_FCS_TBL_INIT;

typedef PACKING union
{
    PACKING struct
    {
#if defined(TCSUPPORT_CPU_EN7581)
#ifdef __BIG_ENDIAN
        FIELD force_idle_hdr_data_same  : 1;
        FIELD rsv_24                    : 7;
        FIELD idle_gem_max_size         : 8;
        FIELD idle_gem_thld             : 16;
#else
        FIELD idle_gem_thld             : 16;
        FIELD idle_gem_max_size         : 8;
        FIELD rsv_24                    : 7;
        FIELD force_idle_hdr_data_same  : 1;
#endif /* __BIG_ENDIAN */
#else
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD idle_gem_max_size         : 8;
        FIELD idle_gem_thld             : 16;
#else
        FIELD idle_gem_thld             : 16;
        FIELD idle_gem_max_size         : 8;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
#endif
    } Bits;
    UINT32 Raw;
} REG_IDLE_GEM_CTRL, *PREG_IDLE_GEM_CTRL;

typedef PACKING union
{
    PACKING struct
    {
#if defined(TCSUPPORT_CPU_EN7581)
#ifdef __BIG_ENDIAN
        FIELD dying_gasp_send_num       : 20;
        FIELD rsv_9                     : 3;
        FIELD hw_dying_gasp_sts         : 1;
        FIELD rsv_5                     : 3;
        FIELD sw_dying_gasp_trig        : 1;
        FIELD rsv_1                     : 3;
        FIELD hw_dying_gasp_en          : 1;
#else
        FIELD hw_dying_gasp_en          : 1;
        FIELD rsv_1                     : 3;
        FIELD sw_dying_gasp_trig        : 1;
        FIELD rsv_5                     : 3;
        FIELD hw_dying_gasp_sts         : 1;
        FIELD rsv_9                     : 3;
        FIELD dying_gasp_send_num       : 20;
#endif /* __BIG_ENDIAN */
#else
#ifdef __BIG_ENDIAN
        FIELD dying_gasp_send_num       : 8;
        FIELD rsv_17                    : 7;
        FIELD sw_dying_gasp_trig        : 1;
        FIELD rsv_9                     : 7;
        FIELD hw_dying_gasp_sts         : 1;
        FIELD rsv_1                     : 7;
        FIELD hw_dying_gasp_en          : 1;
#else
        FIELD hw_dying_gasp_en          : 1;
        FIELD rsv_1                     : 7;
        FIELD hw_dying_gasp_sts         : 1;
        FIELD rsv_9                     : 7;
        FIELD sw_dying_gasp_trig        : 1;
        FIELD rsv_17                    : 7;
        FIELD dying_gasp_send_num       : 8;
#endif /* __BIG_ENDIAN */
#endif
    } Bits;
    UINT32 Raw;
} REG_US_DYING_GASP_CTRL, *PREG_US_DYING_GASP_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_xgem_idle_data         : 32;
    } Bits;
    UINT32 Raw;
} REG_TX_XGEM_IDLE_DATA, *PREG_TX_XGEM_IDLE_DATA;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD tx_limit_get_omci_size    : 16;
        FIELD rsv_9                     : 7;
        FIELD tx_limit_get_omci_en      : 1;
        FIELD rsv_1                     : 7;
        FIELD tx_pre_get_omci_en        : 1;
#else
        FIELD tx_pre_get_omci_en        : 1;
        FIELD rsv_1                     : 7;
        FIELD tx_limit_get_omci_en      : 1;
        FIELD rsv_9                     : 7;
        FIELD tx_limit_get_omci_size    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_TX_OMCI_PRE_GET, *PREG_TX_OMCI_PRE_GET;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD rx_omci_intr_eth_en       : 1;
#else
        FIELD rx_omci_intr_eth_en       : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_OMCI_PRE_GET, *PREG_RX_OMCI_PRE_GET;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD errploamd_drop_en         : 1;
#else
        FIELD errploamd_drop_en         : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_EPDROP_EN, *PREG_EPDROP_EN;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD ploamu_fifo_ovrn          : 1;
        FIELD rsv_24                    : 7;
        FIELD ploamu_fifo_min_avail     : 8;
        FIELD rsv_8                     : 8;
        FIELD ploamu_fifo_avail         : 8;
#else
        FIELD ploamu_fifo_avail         : 8;
        FIELD rsv_8                     : 8;
        FIELD ploamu_fifo_min_avail     : 8;
        FIELD rsv_24                    : 7;
        FIELD ploamu_fifo_ovrn          : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_PLOAMu_FIFO_STS, *PREG_PLOAMu_FIFO_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ploamu_wdata              : 32;
    } Bits;
    UINT32 Raw;
} REG_PLOAMu_WDATA, *PREG_PLOAMu_WDATA;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD ploamd_fifo_ovrn          : 1;
        FIELD rsv_24                    : 7;
        FIELD ploamd_fifo_max_used      : 8;
        FIELD rsv_8                     : 8;
        FIELD ploamd_fifo_used          : 8;
#else
        FIELD ploamd_fifo_used          : 8;
        FIELD rsv_8                     : 8;
        FIELD ploamd_fifo_max_used      : 8;
        FIELD rsv_24                    : 7;
        FIELD ploamd_fifo_ovrn          : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_PLOAMd_FIFO_STS, *PREG_PLOAMd_FIFO_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ploamd_rdata              : 32;
    } Bits;
    UINT32 Raw;
} REG_PLOAMd_RDATA, *PREG_PLOAMd_RDATA;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_5                     : 27;
        FIELD kek_start                 : 1;
        FIELD ploam_ik_start            : 1;
        FIELD omci_ik_start             : 1;
        FIELD sk_start                  : 1;
        FIELD regmsk_start              : 1;
#else
        FIELD regmsk_start              : 1;
        FIELD sk_start                  : 1;
        FIELD omci_ik_start             : 1;
        FIELD ploam_ik_start            : 1;
        FIELD kek_start                 : 1;
        FIELD rsv_5                     : 27;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_KEY_GEN, *PREG_KEY_GEN;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_17                    : 15;
        FIELD cur_oik_idx               : 1;
        FIELD rsv_1                     : 15;
        FIELD cur_pik_idx               : 1;
#else
        FIELD cur_pik_idx               : 1;
        FIELD rsv_1                     : 15;
        FIELD cur_oik_idx               : 1;
        FIELD rsv_17                    : 15;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_CUR_KIDX, *PREG_CUR_KIDX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD msk0                      : 32;
    } Bits;
    UINT32 Raw;
} REG_MSK_0, *PREG_MSK_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD msk1                      : 32;
    } Bits;
    UINT32 Raw;
} REG_MSK_1, *PREG_MSK_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD msk2                      : 32;
    } Bits;
    UINT32 Raw;
} REG_MSK_2, *PREG_MSK_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD msk3                      : 32;
    } Bits;
    UINT32 Raw;
} REG_MSK_3, *PREG_MSK_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD regmsk0                   : 32;
    } Bits;
    UINT32 Raw;
} REG_REGMSK_0, *PREG_REGMSK_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD regmsk1                   : 32;
    } Bits;
    UINT32 Raw;
} REG_REGMSK_1, *PREG_REGMSK_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD regmsk2                   : 32;
    } Bits;
    UINT32 Raw;
} REG_REGMSK_2, *PREG_REGMSK_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD regmsk3                   : 32;
    } Bits;
    UINT32 Raw;
} REG_REGMSK_3, *PREG_REGMSK_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sk0                       : 32;
    } Bits;
    UINT32 Raw;
} REG_SK_0, *PREG_SK_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sk1                       : 32;
    } Bits;
    UINT32 Raw;
} REG_SK_1, *PREG_SK_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sk2                       : 32;
    } Bits;
    UINT32 Raw;
} REG_SK_2, *PREG_SK_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sk3                       : 32;
    } Bits;
    UINT32 Raw;
} REG_SK_3, *PREG_SK_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD hw_genk_0                 : 32;
    } Bits;
    UINT32 Raw;
} REG_HW_GENK_0, *PREG_HW_GENK_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD hw_genk_1                 : 32;
    } Bits;
    UINT32 Raw;
} REG_HW_GENK_1, *PREG_HW_GENK_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD hw_genk_2                 : 32;
    } Bits;
    UINT32 Raw;
} REG_HW_GENK_2, *PREG_HW_GENK_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD hw_genk_3                 : 32;
    } Bits;
    UINT32 Raw;
} REG_HW_GENK_3, *PREG_HW_GENK_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pik0_0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_PIK0_0, *PREG_PIK0_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pik0_1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_PIK0_1, *PREG_PIK0_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pik0_2                    : 32;
    } Bits;
    UINT32 Raw;
} REG_PIK0_2, *PREG_PIK0_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pik0_3                    : 32;
    } Bits;
    UINT32 Raw;
} REG_PIK0_3, *PREG_PIK0_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pik1_0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_PIK1_0, *PREG_PIK1_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pik1_1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_PIK1_1, *PREG_PIK1_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pik1_2                    : 32;
    } Bits;
    UINT32 Raw;
} REG_PIK1_2, *PREG_PIK1_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pik1_3                    : 32;
    } Bits;
    UINT32 Raw;
} REG_PIK1_3, *PREG_PIK1_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oik0_0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_OIK0_0, *PREG_OIK0_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oik0_1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_OIK0_1, *PREG_OIK0_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oik0_2                    : 32;
    } Bits;
    UINT32 Raw;
} REG_OIK0_2, *PREG_OIK0_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oik0_3                    : 32;
    } Bits;
    UINT32 Raw;
} REG_OIK0_3, *PREG_OIK0_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oik1_0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_OIK1_0, *PREG_OIK1_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oik1_1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_OIK1_1, *PREG_OIK1_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oik1_2                    : 32;
    } Bits;
    UINT32 Raw;
} REG_OIK1_2, *PREG_OIK1_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD oik1_3                    : 32;
    } Bits;
    UINT32 Raw;
} REG_OIK1_3, *PREG_OIK1_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD kek0_0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_KEK0_0, *PREG_KEK0_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD kek0_1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_KEK0_1, *PREG_KEK0_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD kek0_2                    : 32;
    } Bits;
    UINT32 Raw;
} REG_KEK0_2, *PREG_KEK0_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD kek0_3                    : 32;
    } Bits;
    UINT32 Raw;
} REG_KEK0_3, *PREG_KEK0_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD kek1_0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_KEK1_0, *PREG_KEK1_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD kek1_1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_KEK1_1, *PREG_KEK1_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD kek1_2                    : 32;
    } Bits;
    UINT32 Raw;
} REG_KEK1_2, *PREG_KEK1_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD kek1_3                    : 32;
    } Bits;
    UINT32 Raw;
} REG_KEK1_3, *PREG_KEK1_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pon_tag_0                 : 32;
    } Bits;
    UINT32 Raw;
} REG_PON_TAG_0, *PREG_PON_TAG_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD pon_tag_1                 : 32;
    } Bits;
    UINT32 Raw;
} REG_PON_TAG_1, *PREG_PON_TAG_1;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_25                    : 7;
        FIELD sw_set_oik_en             : 1;
        FIELD rsv_17                    : 7;
        FIELD sw_set_pik_en             : 1;
        FIELD rsv_9                     : 7;
        FIELD sw_set_oik_idx            : 1;
        FIELD rsv_1                     : 7;
        FIELD sw_set_pik_idx            : 1;
#else
        FIELD sw_set_pik_idx            : 1;
        FIELD rsv_1                     : 7;
        FIELD sw_set_oik_idx            : 1;
        FIELD rsv_9                     : 7;
        FIELD sw_set_pik_en             : 1;
        FIELD rsv_17                    : 7;
        FIELD sw_set_oik_en             : 1;
        FIELD rsv_25                    : 7;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SW_SET_KIDX, *PREG_SW_SET_KIDX;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD sw0_encstart              : 1;
#else
        FIELD sw0_encstart              : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SW0_ENCSTART, *PREG_SW0_ENCSTART;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sw0_mdtaddr               : 32;
    } Bits;
    UINT32 Raw;
} REG_SW0_MADDR, *PREG_SW0_MADDR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sw0_rdtaddr               : 32;
    } Bits;
    UINT32 Raw;
} REG_SW0_RADDR, *PREG_SW0_RADDR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sw0_kaddr                 : 32;
    } Bits;
    UINT32 Raw;
} REG_SW0_KADDR, *PREG_SW0_KADDR;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD sw0_rdtlen                : 16;
        FIELD sw0_mdtlen                : 16;
#else
        FIELD sw0_mdtlen                : 16;
        FIELD sw0_rdtlen                : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SW0_ENCLEN, *PREG_SW0_ENCLEN;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_19                    : 13;
        FIELD sw0_enckidx               : 3;
        FIELD rsv_2                     : 14;
        FIELD sw0_encdic                : 2;
#else
        FIELD sw0_encdic                : 2;
        FIELD rsv_2                     : 14;
        FIELD sw0_enckidx               : 3;
        FIELD rsv_19                    : 13;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SW0_ENCINFO, *PREG_SW0_ENCINFO;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD sw1_encstart              : 1;
#else
        FIELD sw1_encstart              : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SW1_ENCSTART, *PREG_SW1_ENCSTART;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sw1_mdtaddr               : 32;
    } Bits;
    UINT32 Raw;
} REG_SW1_MADDR, *PREG_SW1_MADDR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sw1_rdtaddr               : 32;
    } Bits;
    UINT32 Raw;
} REG_SW1_RADDR, *PREG_SW1_RADDR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sw1_kaddr                 : 32;
    } Bits;
    UINT32 Raw;
} REG_SW1_KADDR, *PREG_SW1_KADDR;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD sw1_rdtlen                : 16;
        FIELD sw1_mdtlen                : 16;
#else
        FIELD sw1_mdtlen                : 16;
        FIELD sw1_rdtlen                : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SW1_ENCLEN, *PREG_SW1_ENCLEN;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_19                    : 13;
        FIELD sw1_enckidx               : 3;
        FIELD rsv_2                     : 14;
        FIELD sw1_encdic                : 2;
#else
        FIELD sw1_encdic                : 2;
        FIELD rsv_2                     : 14;
        FIELD sw1_enckidx               : 3;
        FIELD rsv_19                    : 13;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SW1_ENCINFO, *PREG_SW1_ENCINFO;

#if defined(TCSUPPORT_CPU_AN7583)
typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_12                    : 20;
        FIELD fcs_clr_tcont_done        : 1;
        FIELD rsv_9                     : 2;
        FIELD fcs_clr_tcont_en          : 1;
        FIELD rsv_5                     : 3;
        FIELD fcs_clr_tcont             : 5;
#else
        FIELD fcs_clr_tcont             : 5;
        FIELD rsv_5                     : 3;
        FIELD fcs_clr_tcont_en          : 1;
        FIELD rsv_9                     : 2;
        FIELD fcs_clr_tcont_done        : 1;
        FIELD rsv_12                    : 20;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_TX_FCS_TBL_CLEAR, *PREG_TX_FCS_TBL_CLEAR;
#endif

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_9                     : 23;
        FIELD mib_frame_type            : 1;
        FIELD rsv_1                     : 7;
        FIELD mib_en                    : 1;
#else
        FIELD mib_en                    : 1;
        FIELD rsv_1                     : 7;
        FIELD mib_frame_type            : 1;
        FIELD rsv_9                     : 23;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_MIB_CTRL, *PREG_MIB_CTRL;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_17                    : 15;
        FIELD mib_tbl_rd_clr            : 1;
        FIELD rsv_9                     : 7;
        FIELD mib_tbl_init_done         : 1;
        FIELD rsv_1                     : 7;
        FIELD mib_tbl_init_start        : 1;
#else
        FIELD mib_tbl_init_start        : 1;
        FIELD rsv_1                     : 7;
        FIELD mib_tbl_init_done         : 1;
        FIELD rsv_9                     : 7;
        FIELD mib_tbl_rd_clr            : 1;
        FIELD rsv_17                    : 15;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_MIB_TBL_CONFIG, *PREG_MIB_TBL_CONFIG;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD mib_cmd_done              : 1;
        FIELD rsv_21                    : 10;
        FIELD mib_wr                    : 1;
        FIELD rsv_18                    : 2;
        FIELD mib_type                  : 2;
        FIELD mib_gpid                  : 16;
#else
        FIELD mib_gpid                  : 16;
        FIELD mib_type                  : 2;
        FIELD rsv_18                    : 2;
        FIELD mib_wr                    : 1;
        FIELD rsv_21                    : 10;
        FIELD mib_cmd_done              : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_MIB_CTRL_STS, *PREG_MIB_CTRL_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mib_rdata_l32             : 32;
    } Bits;
    UINT32 Raw;
} REG_MIB_RDATA_L32, *PREG_MIB_RDATA_L32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mib_rdata_h32             : 32;
    } Bits;
    UINT32 Raw;
} REG_MIB_RDATA_H32, *PREG_MIB_RDATA_H32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mib_wdata_l32             : 32;
    } Bits;
    UINT32 Raw;
} REG_MIB_WDATA_L32, *PREG_MIB_WDATA_L32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD mib_wdata_h32             : 32;
    } Bits;
    UINT32 Raw;
} REG_MIB_WDATA_H32, *PREG_MIB_WDATA_H32;

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
} REG_TOD_CLK_PERIOD, *PREG_TOD_CLK_PERIOD;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tod_1pps_width_ctrl       : 32;
    } Bits;
    UINT32 Raw;
} REG_TOD_1PPS_WD_CTRL, *PREG_TOD_1PPS_WD_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cur_tod_sec               : 32;
    } Bits;
    UINT32 Raw;
} REG_CUR_TOD_SEC, *PREG_CUR_TOD_SEC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cur_tod_nano_sec          : 32;
    } Bits;
    UINT32 Raw;
} REG_CUR_TOD_NANO_SEC, *PREG_CUR_TOD_NANO_SEC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tod_spf_cnt_l             : 32;
    } Bits;
    UINT32 Raw;
} REG_TOD_SFC_L, *PREG_TOD_SFC_L;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_19                    : 13;
        FIELD tod_spf_cnt_h             : 19;
#else
        FIELD tod_spf_cnt_h             : 19;
        FIELD rsv_19                    : 13;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_TOD_SFC_H, *PREG_TOD_SFC_H;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD tod_upd_en                : 1;
#else
        FIELD tod_upd_en                : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_TOD_UPD_CTRL, *PREG_TOD_UPD_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD new_tod_sec               : 32;
    } Bits;
    UINT32 Raw;
} REG_NEW_TOD_SEC, *PREG_NEW_TOD_SEC;

typedef PACKING union
{
    PACKING struct
    {
        FIELD new_tod_nano_sec          : 32;
    } Bits;
    UINT32 Raw;
} REG_NEW_TOD_NANO_SEC, *PREG_NEW_TOD_NANO_SEC;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD reload_sleep_cnt          : 1;
#else
        FIELD reload_sleep_cnt          : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SLEEP_CFG, *PREG_SLEEP_CFG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD sleep_cnt                 : 32;
    } Bits;
    UINT32 Raw;
} REG_SLEEP_CNT, *PREG_SLEEP_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD turning_spf_cnt_l         : 32;
    } Bits;
    UINT32 Raw;
} REG_TURNING_SFC_L, *PREG_TURNING_SFC_L;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_19                    : 13;
        FIELD turning_spf_cnt_h         : 19;
#else
        FIELD turning_spf_cnt_h         : 19;
        FIELD rsv_19                    : 13;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_TURNING_SFC_H, *PREG_TURNING_SFC_H;

typedef PACKING union
{
    PACKING struct
    {
#if defined(TCSUPPORT_CPU_EN7581)
#ifdef __BIG_ENDIAN
        FIELD hw_ack_ploamu_code        : 8;
        FIELD force_o2349_zero_gsize_no_dbru : 1;
#if defined(TCSUPPORT_CPU_AN7583)
        FIELD rsv_22                    : 1;
        FIELD mac_tx_err_end_without_ack : 1;
        FIELD idle_intr_xgem_en         : 1;
        FIELD us_sdu_frag_en            : 1;
        FIELD force_recv_xgem           : 1;
#else 
        FIELD rsv_18                    : 5;
#endif
        FIELD us_frag_en                : 1;
        FIELD force_ds_trailer_bip_en   : 1;
        FIELD ds_bip_chk_en             : 1;
        FIELD ds_4b_trailer_en          : 1;
        FIELD tx_deft_gpid_enc_sts_ctrl_by_rx : 1;
        FIELD flt_key_invld_gem         : 1;
        FIELD flt_o2349_continue_gnt    : 1;
        FIELD flt_o2349_no_ploamu_gnt   : 1;
        FIELD us_aes_seq_num_14b        : 1;
        FIELD o52_idle_only_en          : 1;
        FIELD tx_send_only_in_o23459    : 1;
        FIELD flt_o49_no_deft_tcont_gnt : 1;
        FIELD send_ranging_ack_o4       : 1;
        FIELD hw_cal_ds_omci_mic        : 1;
        FIELD hw_cal_us_omci_mic        : 1;
        FIELD flt_o2349_no_ploamu_only_gnt : 1;
        FIELD rept_msg_flt              : 1;
        FIELD rept_prof_flt             : 1;
#else
        FIELD rept_prof_flt             : 1;
        FIELD rept_msg_flt              : 1;
        FIELD flt_o2349_no_ploamu_only_gnt : 1;
        FIELD hw_cal_us_omci_mic        : 1;
        FIELD hw_cal_ds_omci_mic        : 1;
        FIELD send_ranging_ack_o4       : 1;
        FIELD flt_o49_no_deft_tcont_gnt : 1;
        FIELD tx_send_only_in_o23459    : 1;
        FIELD o52_idle_only_en          : 1;
        FIELD us_aes_seq_num_14b        : 1;
        FIELD flt_o2349_no_ploamu_gnt   : 1;
        FIELD flt_o2349_continue_gnt    : 1;
        FIELD flt_key_invld_gem         : 1;
        FIELD tx_deft_gpid_enc_sts_ctrl_by_rx : 1;
        FIELD ds_4b_trailer_en          : 1;
        FIELD ds_bip_chk_en             : 1;
        FIELD force_ds_trailer_bip_en   : 1;
        FIELD us_frag_en                : 1;
#if defined(TCSUPPORT_CPU_AN7583)
        FIELD force_recv_xgem           : 1;
        FIELD us_sdu_frag_en            : 1;
        FIELD idle_intr_xgem_en         : 1;
        FIELD mac_tx_err_end_without_ack : 1;
        FIELD rsv_22                    : 1;
#else 
        FIELD rsv_18                    : 5;
#endif
        FIELD force_o2349_zero_gsize_no_dbru : 1;
        FIELD hw_ack_ploamu_code        : 8;
#endif /* __BIG_ENDIAN */
#else
#ifdef __BIG_ENDIAN
        FIELD hw_ack_ploamu_code        : 8;
        FIELD rsv_10                    : 14;
        FIELD us_aes_seq_num_14b        : 1;
        FIELD o52_idle_only_en          : 1;
        FIELD tx_send_only_in_o23459    : 1;
        FIELD flt_dbru_set_gnt_o2349    : 1;
        FIELD send_ranging_ack_o4       : 1;
        FIELD hw_cal_ds_omci_mic        : 1;
        FIELD hw_cal_us_omci_mic        : 1;
        FIELD flt_no_zero_gsize_gnt_o2349 : 1;
        FIELD rept_msg_flt              : 1;
        FIELD rept_prof_flt             : 1;
#else
        FIELD rept_prof_flt             : 1;
        FIELD rept_msg_flt              : 1;
        FIELD flt_no_zero_gsize_gnt_o2349 : 1;
        FIELD hw_cal_us_omci_mic        : 1;
        FIELD hw_cal_ds_omci_mic        : 1;
        FIELD send_ranging_ack_o4       : 1;
        FIELD flt_dbru_set_gnt_o2349    : 1;
        FIELD tx_send_only_in_o23459    : 1;
        FIELD o52_idle_only_en          : 1;
        FIELD us_aes_seq_num_14b        : 1;
        FIELD rsv_10                    : 14;
        FIELD hw_ack_ploamu_code        : 8;
#endif /* __BIG_ENDIAN */
#endif
    } Bits;
    UINT32 Raw;
} REG_DBG_CAP_SETTING, *PREG_DBG_CAP_SETTING;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD min_bst_intvl             : 16;
        FIELD rsv_13                    : 3;
        FIELD max_bst_len_err_cut       : 1;
        FIELD max_my_bst_num_err_cut    : 1;
        FIELD max_my_alloc_num_err_cut  : 1;
        FIELD max_bst_alloc_num_err_cut : 1;
        FIELD max_bwm_alloc_num_err_cut : 1;
        FIELD rsv_5                     : 3;
        FIELD min_gsize_flt             : 1;
        FIELD max_gsize_flt             : 1;
        FIELD start_time_order_flt      : 1;
        FIELD max_start_time_flt        : 1;
        FIELD min_bst_intvl_flt         : 1;
#else
        FIELD min_bst_intvl_flt         : 1;
        FIELD max_start_time_flt        : 1;
        FIELD start_time_order_flt      : 1;
        FIELD max_gsize_flt             : 1;
        FIELD min_gsize_flt             : 1;
        FIELD rsv_5                     : 3;
        FIELD max_bwm_alloc_num_err_cut : 1;
        FIELD max_bst_alloc_num_err_cut : 1;
        FIELD max_my_alloc_num_err_cut  : 1;
        FIELD max_my_bst_num_err_cut    : 1;
        FIELD max_bst_len_err_cut       : 1;
        FIELD rsv_13                    : 3;
        FIELD min_bst_intvl             : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_BWM_CHK_CTRL, *PREG_DBG_BWM_CHK_CTRL;

typedef PACKING union
{
    PACKING struct
    {
#if defined(TCSUPPORT_CPU_EN7581)
#ifdef __BIG_ENDIAN
        FIELD rsv_18                    : 14;
        FIELD o2349_continue_gnt_recv   : 1;
        FIELD o49_no_deft_tcont_gnt_recv : 1;
        FIELD o2349_no_ploamu_only_gnt_recv : 1;
        FIELD o2349_no_ploamu_gnt_recv  : 1;
        FIELD other_tid_ins_my_bst_err  : 1;
        FIELD max_bst_len_err           : 1;
        FIELD max_my_bst_num_err        : 1;
        FIELD max_my_alloc_num_err      : 1;
        FIELD max_bst_alloc_num_err     : 1;
        FIELD max_bwm_alloc_num_err     : 1;
        FIELD alloc_hec_uc_err          : 1;
        FIELD bst_split_err             : 1;
        FIELD my_tid_ins_bst_err        : 1;
        FIELD min_gsize_err             : 1;
        FIELD max_gsize_err             : 1;
        FIELD start_time_order_err      : 1;
        FIELD max_start_time_err        : 1;
        FIELD min_bst_intvl_err         : 1;
#else
        FIELD min_bst_intvl_err         : 1;
        FIELD max_start_time_err        : 1;
        FIELD start_time_order_err      : 1;
        FIELD max_gsize_err             : 1;
        FIELD min_gsize_err             : 1;
        FIELD my_tid_ins_bst_err        : 1;
        FIELD bst_split_err             : 1;
        FIELD alloc_hec_uc_err          : 1;
        FIELD max_bwm_alloc_num_err     : 1;
        FIELD max_bst_alloc_num_err     : 1;
        FIELD max_my_alloc_num_err      : 1;
        FIELD max_my_bst_num_err        : 1;
        FIELD max_bst_len_err           : 1;
        FIELD other_tid_ins_my_bst_err  : 1;
        FIELD o2349_no_ploamu_gnt_recv  : 1;
        FIELD o2349_no_ploamu_only_gnt_recv : 1;
        FIELD o49_no_deft_tcont_gnt_recv : 1;
        FIELD o2349_continue_gnt_recv   : 1;
        FIELD rsv_18                    : 14;
#endif /* __BIG_ENDIAN */
#else
#ifdef __BIG_ENDIAN
        FIELD rsv_15                    : 17;
        FIELD o23_o4_data_gnt_recv      : 1;
        FIELD other_tid_ins_my_bst_err  : 1;
        FIELD max_bst_len_err           : 1;
        FIELD max_my_bst_num_err        : 1;
        FIELD max_my_alloc_num_err      : 1;
        FIELD max_bst_alloc_num_err     : 1;
        FIELD max_bwm_alloc_num_err     : 1;
        FIELD alloc_hec_uc_err          : 1;
        FIELD bst_split_err             : 1;
        FIELD my_tid_ins_bst_err        : 1;
        FIELD min_gsize_err             : 1;
        FIELD max_gsize_err             : 1;
        FIELD start_time_order_err      : 1;
        FIELD max_start_time_err        : 1;
        FIELD min_bst_intvl_err         : 1;
#else
        FIELD min_bst_intvl_err         : 1;
        FIELD max_start_time_err        : 1;
        FIELD start_time_order_err      : 1;
        FIELD max_gsize_err             : 1;
        FIELD min_gsize_err             : 1;
        FIELD my_tid_ins_bst_err        : 1;
        FIELD bst_split_err             : 1;
        FIELD alloc_hec_uc_err          : 1;
        FIELD max_bwm_alloc_num_err     : 1;
        FIELD max_bst_alloc_num_err     : 1;
        FIELD max_my_alloc_num_err      : 1;
        FIELD max_my_bst_num_err        : 1;
        FIELD max_bst_len_err           : 1;
        FIELD other_tid_ins_my_bst_err  : 1;
        FIELD o23_o4_data_gnt_recv      : 1;
        FIELD rsv_15                    : 17;
#endif /* __BIG_ENDIAN */
#endif
    } Bits;
    UINT32 Raw;
} REG_DBG_BWM_CKH_STS, *PREG_DBG_BWM_CKH_STS;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_26                    : 6;
        FIELD sgl_fifo_max_used         : 10;
        FIELD rsv_10                    : 6;
        FIELD sgl_fifo_used             : 10;
#else
        FIELD sgl_fifo_used             : 10;
        FIELD rsv_10                    : 6;
        FIELD sgl_fifo_max_used         : 10;
        FIELD rsv_26                    : 6;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_BWM_SFIFO_STS, *PREG_DBG_BWM_SFIFO_STS;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_22                    : 10;
        FIELD bst_fifo_max_used         : 6;
        FIELD rsv_6                     : 10;
        FIELD bst_fifo_used             : 6;
#else
        FIELD bst_fifo_used             : 6;
        FIELD rsv_6                     : 10;
        FIELD bst_fifo_max_used         : 6;
        FIELD rsv_22                    : 10;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_BWM_BFIFO_STS, *PREG_DBG_BWM_BFIFO_STS;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD tx_align_fifo_max_used    : 16;
        FIELD tx_align_fifo_used        : 16;
#else
        FIELD tx_align_fifo_used        : 16;
        FIELD tx_align_fifo_max_used    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_TX_ALIGN_FIFO_STS, *PREG_DBG_TX_ALIGN_FIFO_STS;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_28                    : 4;
        FIELD tx_aes_eng_max_used       : 4;
        FIELD tx_rdm_ciph_fifo_max_used : 4;
        FIELD rsv_17                    : 3;
        FIELD tx_cryp_cnt_fifo_max_used : 5;
        FIELD rsv_9                     : 3;
        FIELD tx_ciph_txt_fifo_max_used : 5;
        FIELD tx_aes_hdr_fifo_max_used  : 4;
#else
        FIELD tx_aes_hdr_fifo_max_used  : 4;
        FIELD tx_ciph_txt_fifo_max_used : 5;
        FIELD rsv_9                     : 3;
        FIELD tx_cryp_cnt_fifo_max_used : 5;
        FIELD rsv_17                    : 3;
        FIELD tx_rdm_ciph_fifo_max_used : 4;
        FIELD tx_aes_eng_max_used       : 4;
        FIELD rsv_28                    : 4;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_TX_AES_MAX_USED, *PREG_DBG_TX_AES_MAX_USED;

typedef PACKING union
{
    PACKING struct
    {
#if defined(TCSUPPORT_CPU_AN7583)
#ifdef __BIG_ENDIAN
        FIELD rsv_20                    : 12;
        FIELD dba_modify_tcont_idx_1_mode : 2;
        FIELD dba_modify_tcont_idx_1_ovrw : 1;
        FIELD dba_modify_tcont_idx_1    : 5;
        FIELD dba_modify_tcont_idx_0_mode : 2;
        FIELD dba_modify_tcont_idx_0_ovrw : 1;
        FIELD dba_modify_tcont_idx_0    : 5;
        FIELD rsv_2                     : 2;
        FIELD dba_modify_mode           : 2;
#else
        FIELD dba_modify_mode           : 2;
        FIELD rsv_2                     : 2;
        FIELD dba_modify_tcont_idx_0    : 5;
        FIELD dba_modify_tcont_idx_0_ovrw : 1;
        FIELD dba_modify_tcont_idx_0_mode : 2;
        FIELD dba_modify_tcont_idx_1    : 5;
        FIELD dba_modify_tcont_idx_1_ovrw : 1;
        FIELD dba_modify_tcont_idx_1_mode : 2;
        FIELD rsv_20                    : 12;
#endif /* __BIG_ENDIAN */
#else
#ifdef __BIG_ENDIAN
        FIELD rsv_2                     : 30;
        FIELD dba_modify_mode           : 2;
#else
        FIELD dba_modify_mode           : 2;
        FIELD rsv_2                     : 30;
#endif /* __BIG_ENDIAN */
#endif
    } Bits;
    UINT32 Raw;
} REG_DBG_DBA_BACK_DOOR, *PREG_DBG_DBA_BACK_DOOR;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dba_modify                : 32;
    } Bits;
    UINT32 Raw;
} REG_DBG_DBA_MODIFY, *PREG_DBG_DBA_MODIFY;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD tx_sync_rdy               : 1;
#if defined(TCSUPPORT_CPU_EN7581)
        FIELD rsv_13                    : 18;
        FIELD tx_late_start_err_resync_en : 1;
        FIELD rsv_9                     : 3;
#else
        FIELD rsv_9                     : 22;
#endif
        FIELD sw_resync_en              : 1;
        FIELD rsv_1                     : 7;
        FIELD sw_resync_start           : 1;
#else
        FIELD sw_resync_start           : 1;
        FIELD rsv_1                     : 7;
        FIELD sw_resync_en              : 1;
#if defined(TCSUPPORT_CPU_EN7581)
		FIELD rsv_9                     : 3;
		FIELD tx_late_start_err_resync_en : 1;
		FIELD rsv_13 					: 18;
#else
        FIELD rsv_9                     : 22;
#endif
        FIELD tx_sync_rdy               : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_RESYNC, *PREG_DBG_RESYNC;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_9                     : 23;
        FIELD use_mpi_rx_phy_dly_ofs    : 1;
        FIELD rsv_1                     : 7;
        FIELD o4_o5_same_phy_dly        : 1;
#else
        FIELD o4_o5_same_phy_dly        : 1;
        FIELD rsv_1                     : 7;
        FIELD use_mpi_rx_phy_dly_ofs    : 1;
        FIELD rsv_9                     : 23;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_PHY_DLY_CTRL, *PREG_DBG_PHY_DLY_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ds_spf_cnt_l32            : 32;
    } Bits;
    UINT32 Raw;
} REG_DBG_DS_SPF_CNT_L, *PREG_DBG_DS_SPF_CNT_L;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_19                    : 13;
        FIELD ds_spf_cnt_h19            : 19;
#else
        FIELD ds_spf_cnt_h19            : 19;
        FIELD rsv_19                    : 13;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_DS_SPF_CNT_H, *PREG_DBG_DS_SPF_CNT_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD ds_pon_id_l32             : 32;
    } Bits;
    UINT32 Raw;
} REG_DBG_PON_ID_L, *PREG_DBG_PON_ID_L;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_19                    : 13;
        FIELD ds_pon_id_h19             : 19;
#else
        FIELD ds_pon_id_h19             : 19;
        FIELD rsv_19                    : 13;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_PON_ID_H, *PREG_DBG_PON_ID_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_sync_offset            : 32;
    } Bits;
    UINT32 Raw;
} REG_DBG_TX_SYNC_OFFSET, *PREG_DBG_TX_SYNC_OFFSET;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD rx_phy_dly_offset         : 16;
#else
        FIELD rx_phy_dly_offset         : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_RX_PHY_DLY_OFS, *PREG_DBG_RX_PHY_DLY_OFS;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_12                    : 20;
        FIELD rx_hec_err_sts            : 1;
        FIELD rx_hdr_hec_3err           : 1;
        FIELD rx_hdr_hec_2err           : 1;
        FIELD rx_hdr_hec_1err           : 1;
        FIELD rsv_7                     : 1;
        FIELD rx_alloc_hec_3err         : 1;
        FIELD rx_alloc_hec_2err         : 1;
        FIELD rx_alloc_hec_1err         : 1;
        FIELD rsv_3                     : 1;
        FIELD rx_hlend_hec_3err         : 1;
        FIELD rx_hlend_hec_2err         : 1;
        FIELD rx_hlend_hec_1err         : 1;
#else
        FIELD rx_hlend_hec_1err         : 1;
        FIELD rx_hlend_hec_2err         : 1;
        FIELD rx_hlend_hec_3err         : 1;
        FIELD rsv_3                     : 1;
        FIELD rx_alloc_hec_1err         : 1;
        FIELD rx_alloc_hec_2err         : 1;
        FIELD rx_alloc_hec_3err         : 1;
        FIELD rsv_7                     : 1;
        FIELD rx_hdr_hec_1err           : 1;
        FIELD rx_hdr_hec_2err           : 1;
        FIELD rx_hdr_hec_3err           : 1;
        FIELD rx_hec_err_sts            : 1;
        FIELD rsv_12                    : 20;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_RX_HEC_ERR, *PREG_DBG_RX_HEC_ERR;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rx_enc_chk_err_sts        : 1;
        FIELD rsv_29                    : 2;
        FIELD rx_enc_chk_done           : 1;
        FIELD rsv_21                    : 7;
        FIELD rx_enc_chk_en             : 1;
        FIELD rsv_17                    : 3;
        FIELD exp_rx_enc_sts            : 1;
        FIELD rx_enc_chk_gpid           : 16;
#else
        FIELD rx_enc_chk_gpid           : 16;
        FIELD exp_rx_enc_sts            : 1;
        FIELD rsv_17                    : 3;
        FIELD rx_enc_chk_en             : 1;
        FIELD rsv_21                    : 7;
        FIELD rx_enc_chk_done           : 1;
        FIELD rsv_29                    : 2;
        FIELD rx_enc_chk_err_sts        : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_GEM_ENC_CHK, *PREG_RX_GEM_ENC_CHK;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD rx_mbi_fifo_drop_en       : 1;
#else
        FIELD rx_mbi_fifo_drop_en       : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_MBI_FIFO_DROP_EN, *PREG_RX_MBI_FIFO_DROP_EN;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rx_mbi_hdr_fifo_hth       : 16;
        FIELD rx_mbi_hdr_fifo_lth       : 16;
#else
        FIELD rx_mbi_hdr_fifo_lth       : 16;
        FIELD rx_mbi_hdr_fifo_hth       : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_MBI_HDR_FIFO_DROP_CTRL, *PREG_RX_MBI_HDR_FIFO_DROP_CTRL;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rx_mbi_pl_fifo_hth        : 16;
        FIELD rx_mbi_pl_fifo_lth        : 16;
#else
        FIELD rx_mbi_pl_fifo_lth        : 16;
        FIELD rx_mbi_pl_fifo_hth        : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_MBI_PL_FIFO_DROP_CTRL, *PREG_RX_MBI_PL_FIFO_DROP_CTRL;

#if defined(TCSUPPORT_CPU_EN7581)
typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD get_rx_idle_xgem          : 1;
        FIELD rx_idle_xgem_len          : 31;
#else
        FIELD rx_idle_xgem_len          : 31;
        FIELD get_rx_idle_xgem          : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_GET_RX_IDLE_XGEM, *PREG_GET_RX_IDLE_XGEM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_idle_xgem_1st_pl       : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_IDLE_XGEM_1ST_PL, *PREG_RX_IDLE_XGEM_1ST_PL;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_10                    : 22;
        FIELD treat_special_ploamd_as_bro : 1;
        FIELD treat_1023_alloc_as_bro   : 1;
        FIELD treat_1022_alloc_as_bro   : 1;
        FIELD treat_1021_alloc_as_bro   : 1;
        FIELD force_alloc_as_bro_en     : 1;
        FIELD treat_1022_ploamd_as_bro  : 1;
        FIELD force_ploamd_as_bro_en    : 1;
        FIELD ocbody_ds_fec_sts         : 1;
        FIELD ds_fec_mode               : 2;
#else
        FIELD ds_fec_mode               : 2;
        FIELD ocbody_ds_fec_sts         : 1;
        FIELD force_ploamd_as_bro_en    : 1;
        FIELD treat_1022_ploamd_as_bro  : 1;
        FIELD force_alloc_as_bro_en     : 1;
        FIELD treat_1021_alloc_as_bro   : 1;
        FIELD treat_1022_alloc_as_bro   : 1;
        FIELD treat_1023_alloc_as_bro   : 1;
        FIELD treat_special_ploamd_as_bro : 1;
        FIELD rsv_10                    : 22;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_CAP_SETTING1, *PREG_DBG_CAP_SETTING1;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16		            : 16;
        FIELD special_bro_ploamd_id     : 16;
#else
        FIELD special_bro_ploamd_id     : 16;
        FIELD rsv_16		            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_BRO_ALLOC_SPECIAL, *PREG_BRO_ALLOC_SPECIAL;

#if defined(TCSUPPORT_CPU_AN7583)
typedef PACKING union
{
    PACKING struct
    {
        FIELD dba_modify_idx_0          : 32;
    } Bits;
    UINT32 Raw;
} REG_DBG_DBA_MODIFY_IDX_0, *PREG_DBG_DBA_MODIFY_IDX_0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD dba_modify_idx_1          : 32;
    } Bits;
    UINT32 Raw;
} REG_DBG_DBA_MODIFY_IDX_1, *PREG_DBG_DBA_MODIFY_IDX_1;
#endif

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD cal_gnt_size_done         : 1;
        FIELD cal_gnt_size_start        : 1;
        FIELD cal_gnt_size_auto         : 1;
        FIELD rsv_28                    : 1;
        FIELD cal_gnt_size_gtc_num      : 28;
#else
        FIELD cal_gnt_size_gtc_num      : 28;
        FIELD rsv_28                    : 1;
        FIELD cal_gnt_size_auto         : 1;
        FIELD cal_gnt_size_start        : 1;
        FIELD cal_gnt_size_done         : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_CAL_GNT_SIZE_CTRL_STS, *PREG_CAL_GNT_SIZE_CTRL_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cal_gnt_size_tcont_en     : 32;
    } Bits;
    UINT32 Raw;
} REG_CAL_GNT_SIZE_TCONT_EN, *PREG_CAL_GNT_SIZE_TCONT_EN;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_5                     : 27;
        FIELD cal_gnt_size_sum_tcont_sel : 5;
#else
        FIELD cal_gnt_size_sum_tcont_sel : 5;
        FIELD rsv_5                     : 27;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_CAL_GNT_SIZE_SUM_TCONT_SEL, *PREG_CAL_GNT_SIZE_SUM_TCONT_SEL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cal_gnt_zero_tcont_sts    : 32;
    } Bits;
    UINT32 Raw;
} REG_CAL_GNT_ZERO_TCONT_STS, *PREG_CAL_GNT_ZERO_TCONT_STS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cal_gnt_size_sum_l32      : 32;
    } Bits;
    UINT32 Raw;
} REG_CAL_GNT_SIZE_SUM_L32, *PREG_CAL_GNT_SIZE_SUM_L32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD cal_gnt_size_sum_h32      : 32;
    } Bits;
    UINT32 Raw;
} REG_CAL_GNT_SIZE_SUM_H32, *PREG_CAL_GNT_SIZE_SUM_H32;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD us_bip_err_insert_start   : 1;
        FIELD rsv_29                    : 2;
        FIELD us_bip_err_insert_done    : 1;
        FIELD us_bip_err_insert_num     : 28;
#else
        FIELD us_bip_err_insert_num     : 28;
        FIELD us_bip_err_insert_done    : 1;
        FIELD rsv_29                    : 2;
        FIELD us_bip_err_insert_start   : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_US_BIP_ERR_CTRL, *PREG_US_BIP_ERR_CTRL;

#if defined(TCSUPPORT_CPU_AN7583)
typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_1                     : 31;
        FIELD hunt_after_losgd_en       : 1;
#else
        FIELD hunt_after_losgd_en       : 1;
        FIELD rsv_1                     : 31;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_HUNT_AFTER_LOSGD, *PREG_HUNT_AFTER_LOSGD;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_31                    : 1;
        FIELD max_alloc_num             : 11;
        FIELD max_my_alloc_num          : 8;
        FIELD max_bst_alloc_num         : 5;
        FIELD max_my_bst_num            : 7;
#else
        FIELD max_my_bst_num            : 7;
        FIELD max_bst_alloc_num         : 5;
        FIELD max_my_alloc_num          : 8;
        FIELD max_alloc_num             : 11;
        FIELD rsv_31                    : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_BWMAP_LEN_LIMIT, *PREG_BWMAP_LEN_LIMIT;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_30                    : 2;
        FIELD rx_gem_3port_interleave   : 1;
        FIELD rx_omci_interleave        : 1;
        FIELD rsv_2                     : 26;
        FIELD nml_header_ovrw_en        : 1;
        FIELD omci_header_ovrw_en       : 1;
#else
        FIELD omci_header_ovrw_en       : 1;
        FIELD nml_header_ovrw_en        : 1;
        FIELD rsv_2                     : 26;
        FIELD rx_omci_interleave        : 1;
        FIELD rx_gem_3port_interleave   : 1;
        FIELD rsv_30                    : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_XGEM_HEADER_OVRW, *PREG_XGEM_HEADER_OVRW;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_14                    : 18;
        FIELD max_omci_len              : 14;
#else
        FIELD max_omci_len              : 14;
        FIELD rsv_14                    : 18;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_OMCI_LEN_CTRL, *PREG_OMCI_LEN_CTRL;
#endif

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
		FIELD mon_not_gnt_en	 		: 1;
		FIELD rsv_28_30					: 3;
		FIELD mon_gtc_num_for_not_gnt	: 28;
#else
		FIELD mon_gtc_num_for_not_gnt	: 28;
		FIELD rsv_28_30					: 3;
		FIELD mon_not_gnt_en	 		: 1;
#endif /* __BIG_ENDIAN */
	} Bits;
	UINT32 Raw;
} REG_MONI_CTRL_FOR_NOT_GNT, *PREG_MONI_CTRL_FOR_NOT_GNT;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
		FIELD rsv_31					: 31;
		FIELD xpon_mac_ram_pwrdown	    : 1;
#else
		FIELD xpon_mac_ram_pwrdown	    : 1;
		FIELD rsv_31					: 31;
#endif /* __BIG_ENDIAN */
	} Bits;
	UINT32 Raw;
} REG_SRAM_PWR_DOWN, *PREG_SRAM_PWR_DOWN;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
		FIELD rsv_30					: 30;
		FIELD xpon_mac_mbist_fail	    : 1;
		FIELD xpon_mac_mbist_done	    : 1;
#else
		FIELD xpon_mac_mbist_done	    : 1;
		FIELD xpon_mac_mbist_fail	    : 1;
		FIELD rsv_30					: 30;
#endif /* __BIG_ENDIAN */
	} Bits;
	UINT32 Raw;
} REG_MBIST_FSH_STS, *PREG_MBIST_FSH_STS;

typedef PACKING union
{
    PACKING struct
    {
		FIELD xgpon_mac_mbist_delsel_g0	: 32;
	} Bits;
	UINT32 Raw;
} REG_MBIST_DELSEL_G0, *PREG_MBIST_DELSEL_G0;

typedef PACKING union
{
    PACKING struct
    {
		FIELD xgpon_mac_mbist_delsel_g1	: 32;
	} Bits;
	UINT32 Raw;
} REG_MBIST_DELSEL_G1, *PREG_MBIST_DELSEL_G1;

typedef PACKING union
{
    PACKING struct
    {
		FIELD xgpon_mac_mbist_delsel_g2	: 32;
	} Bits;
	UINT32 Raw;
} REG_MBIST_DELSEL_G2, *PREG_MBIST_DELSEL_G2;

typedef PACKING union
{
    PACKING struct
    {
		FIELD xgpon_mac_mbist_delsel_g3	: 32;
	} Bits;
	UINT32 Raw;
} REG_MBIST_DELSEL_G3, *PREG_MBIST_DELSEL_G3;

typedef PACKING union
{
    PACKING struct
    {
		FIELD xgpon_mac_mbist_delsel_g4	: 32;
	} Bits;
	UINT32 Raw;
} REG_MBIST_DELSEL_G4, *PREG_MBIST_DELSEL_G4;

typedef PACKING union
{
    PACKING struct
    {
		FIELD xgpon_mac_mbist_delsel_g5	: 32;
	} Bits;
	UINT32 Raw;
} REG_MBIST_DELSEL_G5, *PREG_MBIST_DELSEL_G5;

typedef PACKING union
{
    PACKING struct
    {
		FIELD xgpon_mac_mbist_delsel_g6	: 32;
	} Bits;
	UINT32 Raw;
} REG_MBIST_DELSEL_G6, *PREG_MBIST_DELSEL_G6;

typedef PACKING union
{
    PACKING struct
    {
		FIELD xgpon_mac_mbist_done_g0	: 32;
	} Bits;
	UINT32 Raw;
} REG_MBIST_DONE_G0, *PREG_MBIST_DONE_G0;

typedef PACKING union
{
    PACKING struct
    {
		FIELD xgpon_mac_mbist_fail_g0	: 32;
	} Bits;
	UINT32 Raw;
} REG_MBIST_FAIL_G0, *PREG_MBIST_FAIL_G0;

typedef PACKING union
{
    PACKING struct
    {
		FIELD xgpon_mac_mbist_fail_g1	: 32;
	} Bits;
	UINT32 Raw;
} REG_MBIST_FAIL_G1, *PREG_MBIST_FAIL_G1;

#endif

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_9                     : 23;
        FIELD nml_cnt_clr               : 1;
        FIELD rsv_1                     : 7;
        FIELD err_cnt_clr               : 1;
#else
        FIELD err_cnt_clr               : 1;
        FIELD rsv_1                     : 7;
        FIELD nml_cnt_clr               : 1;
        FIELD rsv_9                     : 23;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_CNT_CLR, *PREG_CNT_CLR;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD rx_hlend_hec_3err_cnt     : 8;
        FIELD rx_hlend_hec_2err_cnt     : 8;
        FIELD rx_hlend_hec_1err_cnt     : 8;
#else
        FIELD rx_hlend_hec_1err_cnt     : 8;
        FIELD rx_hlend_hec_2err_cnt     : 8;
        FIELD rx_hlend_hec_3err_cnt     : 8;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_HLEND_HEC_CNT, *PREG_RX_HLEND_HEC_CNT;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD rx_alloc_hec_3err_cnt     : 8;
        FIELD rx_alloc_hec_2err_cnt     : 8;
        FIELD rx_alloc_hec_1err_cnt     : 8;
#else
        FIELD rx_alloc_hec_1err_cnt     : 8;
        FIELD rx_alloc_hec_2err_cnt     : 8;
        FIELD rx_alloc_hec_3err_cnt     : 8;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_ALLOC_HEC_CNT, *PREG_RX_ALLOC_HEC_CNT;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_24                    : 8;
        FIELD rx_hdr_hec_3err_cnt       : 8;
        FIELD rx_hdr_hec_2err_cnt       : 8;
        FIELD rx_hdr_hec_1err_cnt       : 8;
#else
        FIELD rx_hdr_hec_1err_cnt       : 8;
        FIELD rx_hdr_hec_2err_cnt       : 8;
        FIELD rx_hdr_hec_3err_cnt       : 8;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_HDR_HEC_CNT, *PREG_RX_HDR_HEC_CNT;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rx_pon_id_hec_err_cnt     : 16;
        FIELD rx_sfc_hec_err_cnt        : 16;
#else
        FIELD rx_sfc_hec_err_cnt        : 16;
        FIELD rx_pon_id_hec_err_cnt     : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_PHY_HEC_ERR_CNT, *PREG_RX_PHY_HEC_ERR_CNT;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rx_omci_mic_err_cnt       : 16;
        FIELD rx_ploam_mic_err_cnt      : 16;
#else
        FIELD rx_ploam_mic_err_cnt      : 16;
        FIELD rx_omci_mic_err_cnt       : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_MIC_ERR_CNT, *PREG_RX_MIC_ERR_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_eth_crc_err_cnt        : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_ETH_FCS_ERR_CNT, *PREG_RX_ETH_FCS_ERR_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_bip_err_cnt            : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_BIP_ERR_CNT, *PREG_RX_BIP_ERR_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_key_err_cnt            : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_KEY_ERR_CNT, *PREG_RX_KEY_ERR_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_lost_wcnt              : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_LOST_WCNT, *PREG_RX_LOST_WCNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD invld_prof_bst_gnt_cnt    : 32;
    } Bits;
    UINT32 Raw;
} REG_INVLD_PROF_BST_GNT_CNT, *PREG_INVLD_PROF_BST_GNT_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_mbi_xgem_drop_cnt      : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_MBI_XGEM_DROP_CNT, *PREG_RX_MBI_XGEM_DROP_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_mpi_sof_cnt            : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_XGTC_CNT, *PREG_RX_XGTC_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_mpi_sof_cnt            : 32;
    } Bits;
    UINT32 Raw;
} REG_TX_BST_CNT, *PREG_TX_BST_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_ploamd_cnt             : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_PLOAMD_CNT, *PREG_RX_PLOAMD_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_ploamu_cnt             : 32;
    } Bits;
    UINT32 Raw;
} REG_TX_PLOAMU_CNT, *PREG_TX_PLOAMU_CNT;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rx_omci_cnt_fe            : 16;
        FIELD rx_omci_cnt_mac           : 16;
#else
        FIELD rx_omci_cnt_mac           : 16;
        FIELD rx_omci_cnt_fe            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_RX_OMCI_CNT, *PREG_RX_OMCI_CNT;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD tx_omci_cnt_fe            : 16;
        FIELD tx_omci_cnt_mac           : 16;
#else
        FIELD tx_omci_cnt_mac           : 16;
        FIELD tx_omci_cnt_fe            : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_TX_OMCI_CNT, *PREG_TX_OMCI_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_xgem_cnt               : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_XGEM_CNT, *PREG_RX_XGEM_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_xgem_cnt               : 32;
    } Bits;
    UINT32 Raw;
} REG_TX_XGEM_CNT, *PREG_TX_XGEM_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_mbi_ack_cnt            : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_MBI_CNT, *PREG_RX_MBI_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_mbi_ack_cnt            : 32;
    } Bits;
    UINT32 Raw;
} REG_TX_MBI_CNT, *PREG_TX_MBI_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_non_idle_bcnt          : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_NON_IDLE_BCNT, *PREG_RX_NON_IDLE_BCNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_non_idle_bcnt          : 32;
    } Bits;
    UINT32 Raw;
} REG_TX_NON_IDLE_BCNT, *PREG_TX_NON_IDLE_BCNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_nlf_xgem_cnt           : 32;
    } Bits;
    UINT32 Raw;
} REG_TX_NLF_XGEM_CNT, *PREG_TX_NLF_XGEM_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_ack_ploamu_cnt         : 32;
    } Bits;
    UINT32 Raw;
} REG_TX_ACK_PLOAMU_CNT, *PREG_TX_ACK_PLOAMU_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rx_bip_protect_wcnt       : 32;
    } Bits;
    UINT32 Raw;
} REG_RX_BIP_PROTECT_WCNT, *PREG_RX_BIP_PROTECT_WCNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD tx_idle_bcnt              : 32;
    } Bits;
    UINT32 Raw;
} REG_TX_IDLE_BCNT, *PREG_TX_IDLE_BCNT;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_10                    : 22;
        FIELD snf_us_en                 : 1;
        FIELD snf_ds_en                 : 1;
#if defined(TCSUPPORT_CPU_AN7583)
        FIELD rsv_2                     : 6;
        FIELD snf_us_ovrw_8b_xgem       : 1;
#else
        FIELD rsv_1                     : 7;
#endif
        FIELD snf_us_ovrw_my_xgem       : 1;
#else
        FIELD snf_us_ovrw_my_xgem       : 1;
#if defined(TCSUPPORT_CPU_AN7583)
        FIELD snf_us_ovrw_8b_xgem       : 1;
        FIELD rsv_2                     : 6;
#else
        FIELD rsv_1                     : 7;
#endif
        FIELD snf_ds_en                 : 1;
        FIELD snf_us_en                 : 1;
        FIELD rsv_10                    : 22;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SNF_CTRL, *PREG_SNF_CTRL;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD us_snf_gpid               : 16;
        FIELD ds_snf_gpid               : 16;
#else
        FIELD ds_snf_gpid               : 16;
        FIELD us_snf_gpid               : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SNF_GPID, *PREG_SNF_GPID;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD ds_snf_eth_da_h16         : 16;
        FIELD ds_snf_eth_sa_h16         : 16;
#else
        FIELD ds_snf_eth_sa_h16         : 16;
        FIELD ds_snf_eth_da_h16         : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DS_SNF_ETH_DASA_H16, *PREG_DS_SNF_ETH_DASA_H16;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD snf_eth_tpid              : 16;
        FIELD rsv_0                     : 16;
#else
        FIELD rsv_0                     : 16;
        FIELD snf_eth_tpid              : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SNF_ETH_TAG, *PREG_SNF_ETH_TAG;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_16                    : 16;
        FIELD snf_eth_type              : 16;
#else
        FIELD snf_eth_type              : 16;
        FIELD rsv_16                    : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_SNF_ETH_TYPE, *PREG_SNF_ETH_TYPE;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD us_snf_eth_da_h16         : 16;
        FIELD us_snf_eth_sa_h16         : 16;
#else
        FIELD us_snf_eth_sa_h16         : 16;
        FIELD us_snf_eth_da_h16         : 16;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_US_SNF_ETH_DASA_H16, *PREG_US_SNF_ETH_DASA_H16;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD probe_clk_en              : 1;
        FIELD probe_trig_en             : 1;
        FIELD rsv_22                    : 8;
        FIELD probe_trig_bit_sel        : 6;
        FIELD rsv_14                    : 2;
        FIELD probe_trig_sel            : 6;
        FIELD rsv_5                     : 3;
        FIELD probe_sel                 : 5;
#else
        FIELD probe_sel                 : 5;
        FIELD rsv_5                     : 3;
        FIELD probe_trig_sel            : 6;
        FIELD rsv_14                    : 2;
        FIELD probe_trig_bit_sel        : 6;
        FIELD rsv_22                    : 8;
        FIELD probe_trig_en             : 1;
        FIELD probe_clk_en              : 1;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_DBG_PROBE_CTRL, *PREG_DBG_PROBE_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD probe_h32                 : 32;
    } Bits;
    UINT32 Raw;
} REG_DBG_PROBE_HIGH32, *PREG_DBG_PROBE_HIGH32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD probe_l32                 : 32;
    } Bits;
    UINT32 Raw;
} REG_DBG_PROBE_LOW32, *PREG_DBG_PROBE_LOW32;

/****************phy reg**********************/
typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN
        FIELD rsv_25                    : 7;
        FIELD xg_tx_fec_en4             : 1;
        FIELD rsv_17                    : 7;
        FIELD xg_tx_fec_en3             : 1;
        FIELD rsv_9                     : 7;
        FIELD xg_tx_fec_en2             : 1;
        FIELD rsv_1                     : 7;
        FIELD xg_tx_fec_en1             : 1;
#else
        FIELD xg_tx_fec_en1             : 1;
        FIELD rsv_1                     : 7;
        FIELD xg_tx_fec_en2             : 1;
        FIELD rsv_9                     : 7;
        FIELD xg_tx_fec_en3             : 1;
        FIELD rsv_17                    : 7;
        FIELD xg_tx_fec_en4             : 1;
        FIELD rsv_25                    : 7;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_PHY_FEC_LOW32, *PREG_PHY_FEC_LOW32;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN        
        FIELD rsv_24                    : 8;
        FIELD prmb_rpt_num1             : 8;
        FIELD rsv_12                    : 4;
        FIELD ptmb_len1                 : 4;
        FIELD rsv_4                     : 4;
        FIELD dlmt_len1                 : 4;
#else
        FIELD dlmt_len1                 : 4;
        FIELD rsv_4                     : 4;
        FIELD ptmb_len1                 : 4;
        FIELD rsv_12                    : 4;
        FIELD prmb_rpt_num1             : 8;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_PHY_PSBU_INFO1_LOW32, *PREG_PHY_PSBU_INFO1_LOW32;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN        
        FIELD rsv_24                    : 8;
        FIELD prmb_rpt_num2             : 8;
        FIELD rsv_12                    : 4;
        FIELD ptmb_len2                 : 4;
        FIELD rsv_4                     : 4;
        FIELD dlmt_len2                 : 4;
#else
        FIELD dlmt_len2                 : 4;
        FIELD rsv_4                     : 4;
        FIELD ptmb_len2                 : 4;
        FIELD rsv_12                    : 4;
        FIELD prmb_rpt_num2             : 8;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_PHY_PSBU_INFO2_LOW32, *PREG_PHY_PSBU_INFO2_LOW32;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN        
        FIELD rsv_24                    : 8;
        FIELD prmb_rpt_num3             : 8;
        FIELD rsv_12                    : 4;
        FIELD ptmb_len3                 : 4;
        FIELD rsv_4                     : 4;
        FIELD dlmt_len3                 : 4;
#else
        FIELD dlmt_len3                 : 4;
        FIELD rsv_4                     : 4;
        FIELD ptmb_len3                 : 4;
        FIELD rsv_12                    : 4;
        FIELD prmb_rpt_num3             : 8;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_PHY_PSBU_INFO3_LOW32, *PREG_PHY_PSBU_INFO3_LOW32;

typedef PACKING union
{
    PACKING struct
    {
#ifdef __BIG_ENDIAN        
        FIELD rsv_24                    : 8;
        FIELD prmb_rpt_num4             : 8;
        FIELD rsv_12                    : 4;
        FIELD ptmb_len4                 : 4;
        FIELD rsv_4                     : 4;
        FIELD dlmt_len4                 : 4;
#else
        FIELD dlmt_len4                 : 4;
        FIELD rsv_4                     : 4;
        FIELD ptmb_len4                 : 4;
        FIELD rsv_12                    : 4;
        FIELD prmb_rpt_num4             : 8;
        FIELD rsv_24                    : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_PHY_PSBU_INFO4_LOW32, *PREG_PHY_PSBU_INFO4_LOW32;
// ----------------- xgpon_mac_reg  Grouping Definitions -------------------
// ----------------- xgpon_mac_reg Register Definition -------------------
typedef volatile PACKING struct
{
    UINT32                          rsv_0000[5120];   // 0000..4FFC
    REG_SW_RST                      SW_RST;           // 5000
    REG_MBI_MPI_STOP                MBI_MPI_STOP;     // 5004
    UINT32                          rsv_5008;         // 5008
    REG_VENDOR_ID                   VENDOR_ID;        // 500C
    REG_VS_SN                       VS_SN;            // 5010
    REG_ONU_ID                      ONU_ID;           // 5014
    REG_RGS_ID3_0                   RGS_ID3_0;        // 5018
    REG_RGS_ID7_4                   RGS_ID7_4;        // 501C
    REG_RGS_ID11_8                  RGS_ID11_8;       // 5020
    REG_RGS_ID15_12                 RGS_ID15_12;      // 5024
    REG_RGS_ID19_16                 RGS_ID19_16;      // 5028
    REG_RGS_ID23_20                 RGS_ID23_20;      // 502C
    REG_RGS_ID27_24                 RGS_ID27_24;      // 5030
    REG_RGS_ID31_28                 RGS_ID31_28;      // 5034
    REG_RGS_ID35_32                 RGS_ID35_32;      // 5038
    UINT32                          rsv_503C;         // 503C
    REG_INT_ENABLE                  INT_ENABLE;       // 5040
    REG_INT_STATUS                  INT_STATUS;       // 5044
    UINT32                          rsv_5048[2];      // 5048..504C
    REG_FIFO_ERR_STS                FIFO_ERR_STS;     // 5050
    REG_TX_ERR_STS                  TX_ERR_STS;       // 5054
    REG_RX_ERR_STS                  RX_ERR_STS;       // 5058
#if defined(TCSUPPORT_CPU_AN7583)
    UINT32                          rsv_505C;         // 505C
    REG_FIFO_ERR_ENABLE             FIFO_ERR_ENABLE;  // 5060
    REG_TX_ERR_ENABLE               TX_ERR_ENABLE;    // 5064
    REG_RX_ERR_ENABLE               RX_ERR_ENABLE;    // 5068
    UINT32                          rsv_506C[5];      // 506C..507C
#else    
    UINT32                          rsv_505C[9];      // 505C..507C
#endif
    REG_COR_TAG                     COR_TAG;          // 5080
    REG_CUR_DS_PON_ID               CUR_DS_PON_ID;    // 5084
    REG_CUR_US_PON_ID               CUR_US_PON_ID;    // 5088
    REG_CALIB_STS_L32               CALIB_STS_L32;    // 508C
    REG_CALIB_STS_H32               CALIB_STS_H32;    // 5090
    REG_TUNING_GRAN                 TUNING_GRAN;      // 5094
    REG_STEP_TUNING_TIME            STEP_TUNING_TIME; // 5098
    REG_US_RATE_CAP                 US_RATE_CAP;      // 509C
    REG_ATTENUATION                 ATTENUATION;      // 50A0
    REG_POWER_LEVEL_CAP             POWER_LEVEL_CAP;  // 50A4
#if defined(TCSUPPORT_CPU_EN7581)
		REG_ACT_DEBUG_INFO				ACT_DEBUG_INFO;   //50A8
		UINT32							rsv_50A8[21];	  // 50AC..50FC
#else
    UINT32                          rsv_50A8[22];     // 50A8..50FC
#endif
    REG_O23_O4_PLOAMU_CTRL          O23_O4_PLOAMU_CTRL; // 5100
    REG_ACTIVATION_ST               ACTIVATION_ST;    // 5104
    REG_RSP_TIME                    RSP_TIME;         // 5108
    REG_RDM_DLY                     RDM_DLY;          // 510C
    REG_INT_DLY                     INT_DLY;          // 5110
    REG_EQD                         EQD;              // 5114
    REG_SYN_MRG                     SYN_MRG;          // 5118
    REG_US_PROF_VLD                 US_PROF_VLD;      // 511C
    REG_US_PROF_PSBu_LEN_0_1        US_PROF_PSBu_LEN_0_1; // 5120
    REG_US_PROF_PSBu_LEN_2_3        US_PROF_PSBu_LEN_2_3; // 5124
#if defined(TCSUPPORT_CPU_EN7581)
    REG_EXT_RDM_DLY                 EXT_RDM_DLY;      // 5128
    UINT32                          rsv_512C[53];     // 512C..51FC
#else
    UINT32                          rsv_5128[54];     // 5128..51FC
#endif
    REG_US_AES_KEY_CTRL             US_AES_KEY_CTRL;   // 5200
    REG_DS_AES_KEY_VLD              DS_AES_KEY_VLD;   // 5204
    REG_DEFT_GPID_ENC               DEFT_GPID_ENC;    // 5208
    UINT32                          rsv_520C;         // 520C
    REG_AES_UC_IDX0_KEY0            AES_UC_IDX0_KEY0; // 5210
    REG_AES_UC_IDX0_KEY1            AES_UC_IDX0_KEY1; // 5214
    REG_AES_UC_IDX0_KEY2            AES_UC_IDX0_KEY2; // 5218
    REG_AES_UC_IDX0_KEY3            AES_UC_IDX0_KEY3; // 521C
    REG_AES_UC_IDX1_KEY0            AES_UC_IDX1_KEY0; // 5220
    REG_AES_UC_IDX1_KEY1            AES_UC_IDX1_KEY1; // 5224
    REG_AES_UC_IDX1_KEY2            AES_UC_IDX1_KEY2; // 5228
    REG_AES_UC_IDX1_KEY3            AES_UC_IDX1_KEY3; // 522C
    REG_AES_BC_IDX0_KEY0            AES_BC_IDX0_KEY0; // 5230
    REG_AES_BC_IDX0_KEY1            AES_BC_IDX0_KEY1; // 5234
    REG_AES_BC_IDX0_KEY2            AES_BC_IDX0_KEY2; // 5238
    REG_AES_BC_IDX0_KEY3            AES_BC_IDX0_KEY3; // 523C
    REG_AES_BC_IDX1_KEY0            AES_BC_IDX1_KEY0; // 5240
    REG_AES_BC_IDX1_KEY1            AES_BC_IDX1_KEY1; // 5244
    REG_AES_BC_IDX1_KEY2            AES_BC_IDX1_KEY2; // 5248
    REG_AES_BC_IDX1_KEY3            AES_BC_IDX1_KEY3; // 524C
    REG_TCONT_ID_CFG                TCONT_ID_CFG;     // 5250
    REG_TCONT_ID_STS                TCONT_ID_STS;     // 5254
    UINT32                          rsv_5258[2];      // 5258..525C
    REG_GPIDX_TBL_INIT              GPIDX_TBL_INIT;   // 5260
    REG_GPIDX_TBL_CTRL              GPIDX_TBL_CTRL;   // 5264
    REG_GPIDX_TBL_STS               GPIDX_TBL_STS;    // 5268
    UINT32                          rsv_526C;         // 526C
    REG_GEM_TBL_INIT                GEM_TBL_INIT;     // 5270
    REG_GEM_PORT_CFG                GEM_PORT_CFG;     // 5274
    REG_GEM_PORT_STS                GEM_PORT_STS;     // 5278
    REG_G_TX_FCS_TBL_INIT           G_TX_FCS_TBL_INIT; // 527C
    REG_IDLE_GEM_CTRL               IDLE_GEM_CTRL;    // 5280
    REG_US_DYING_GASP_CTRL          US_DYING_GASP_CTRL; // 5284
    REG_TX_XGEM_IDLE_DATA           TX_XGEM_IDLE_DATA; // 5288
    REG_TX_OMCI_PRE_GET             TX_OMCI_PRE_GET;  // 528C
    REG_RX_OMCI_PRE_GET             RX_OMCI_PRE_GET;  // 5290
    UINT32                          rsv_5294[23];     // 5294..52EC
    REG_EPDROP_EN                   EPDROP_EN;        // 52F0
    UINT32                          rsv_52F4[3];      // 52F4..52FC
    REG_PLOAMu_FIFO_STS             PLOAMu_FIFO_STS;  // 5300
    REG_PLOAMu_WDATA                PLOAMu_WDATA;     // 5304
    REG_PLOAMd_FIFO_STS             PLOAMd_FIFO_STS;  // 5308
    REG_PLOAMd_RDATA                PLOAMd_RDATA;     // 530C
    UINT32                          rsv_5310;         // 5310
    REG_KEY_GEN                     KEY_GEN;          // 5314
    REG_CUR_KIDX                    CUR_KIDX;         // 5318
    UINT32                          rsv_531C;         // 531C
    REG_MSK_0                       MSK_0;            // 5320
    REG_MSK_1                       MSK_1;            // 5324
    REG_MSK_2                       MSK_2;            // 5328
    REG_MSK_3                       MSK_3;            // 532C
    REG_REGMSK_0                    REGMSK_0;         // 5330
    REG_REGMSK_1                    REGMSK_1;         // 5334
    REG_REGMSK_2                    REGMSK_2;         // 5338
    REG_REGMSK_3                    REGMSK_3;         // 533C
    REG_SK_0                        SK_0;             // 5340
    REG_SK_1                        SK_1;             // 5344
    REG_SK_2                        SK_2;             // 5348
    REG_SK_3                        SK_3;             // 534C
    REG_HW_GENK_0                   HW_GENK_0;        // 5350
    REG_HW_GENK_1                   HW_GENK_1;        // 5354
    REG_HW_GENK_2                   HW_GENK_2;        // 5358
    REG_HW_GENK_3                   HW_GENK_3;        // 535C
    REG_PIK0_0                      PIK0_0;           // 5360
    REG_PIK0_1                      PIK0_1;           // 5364
    REG_PIK0_2                      PIK0_2;           // 5368
    REG_PIK0_3                      PIK0_3;           // 536C
    REG_PIK1_0                      PIK1_0;           // 5370
    REG_PIK1_1                      PIK1_1;           // 5374
    REG_PIK1_2                      PIK1_2;           // 5378
    REG_PIK1_3                      PIK1_3;           // 537C
    REG_OIK0_0                      OIK0_0;           // 5380
    REG_OIK0_1                      OIK0_1;           // 5384
    REG_OIK0_2                      OIK0_2;           // 5388
    REG_OIK0_3                      OIK0_3;           // 538C
    REG_OIK1_0                      OIK1_0;           // 5390
    REG_OIK1_1                      OIK1_1;           // 5394
    REG_OIK1_2                      OIK1_2;           // 5398
    REG_OIK1_3                      OIK1_3;           // 539C
    REG_KEK0_0                      KEK0_0;           // 53A0
    REG_KEK0_1                      KEK0_1;           // 53A4
    REG_KEK0_2                      KEK0_2;           // 53A8
    REG_KEK0_3                      KEK0_3;           // 53AC
    REG_KEK1_0                      KEK1_0;           // 53B0
    REG_KEK1_1                      KEK1_1;           // 53B4
    REG_KEK1_2                      KEK1_2;           // 53B8
    REG_KEK1_3                      KEK1_3;           // 53BC
    REG_PON_TAG_0                   PON_TAG_0;        // 53C0
    REG_PON_TAG_1                   PON_TAG_1;        // 53C4
    UINT32                          rsv_53C8[8];      // 53C8..53E4
    REG_SW_SET_KIDX                 SW_SET_KIDX;      // 53E8
    UINT32                          rsv_53EC[5];      // 53EC..53FC
    REG_SW0_ENCSTART                SW0_ENCSTART;     // 5400
    REG_SW0_MADDR                   SW0_MADDR;        // 5404
    REG_SW0_RADDR                   SW0_RADDR;        // 5408
    REG_SW0_KADDR                   SW0_KADDR;        // 540C
    REG_SW0_ENCLEN                  SW0_ENCLEN;       // 5410
    REG_SW0_ENCINFO                 SW0_ENCINFO;      // 5414
    UINT32                          rsv_5418[2];      // 5418..541C
    REG_SW1_ENCSTART                SW1_ENCSTART;     // 5420
    REG_SW1_MADDR                   SW1_MADDR;        // 5424
    REG_SW1_RADDR                   SW1_RADDR;        // 5428
    REG_SW1_KADDR                   SW1_KADDR;        // 542C
    REG_SW1_ENCLEN                  SW1_ENCLEN;       // 5430
    REG_SW1_ENCINFO                 SW1_ENCINFO;      // 5434
#if defined(TCSUPPORT_CPU_AN7583)
    REG_TX_FCS_TBL_CLEAR            TX_FCS_TBL_CLEAR; // 5438
    UINT32                          rsv_543C[49];     // 543C..54FC
#else
	UINT32                          rsv_5438[50];      // 5438..54FC
#endif
    REG_MIB_CTRL                    MIB_CTRL;         // 5500
    REG_MIB_TBL_CONFIG              MIB_TBL_CONFIG;   // 5504
    REG_MIB_CTRL_STS                MIB_CTRL_STS;     // 5508
    UINT32                          rsv_550C;         // 550C
    REG_MIB_RDATA_L32               MIB_RDATA_L32;    // 5510
    REG_MIB_RDATA_H32               MIB_RDATA_H32;    // 5514
    REG_MIB_WDATA_L32               MIB_WDATA_L32;    // 5518
    REG_MIB_WDATA_H32               MIB_WDATA_H32;    // 551C
    UINT32                          rsv_5520[12];     // 5520..554C
    REG_TOD_CLK_PERIOD              TOD_CLK_PERIOD;   // 5550
    REG_TOD_1PPS_WD_CTRL            TOD_1PPS_WD_CTRL; // 5554
    REG_CUR_TOD_SEC                 CUR_TOD_SEC;      // 5558
    REG_CUR_TOD_NANO_SEC            CUR_TOD_NANO_SEC; // 555C
    REG_TOD_SFC_L                   TOD_SFC_L;        // 5560
    REG_TOD_SFC_H                   TOD_SFC_H;        // 5564
    REG_TOD_UPD_CTRL                TOD_UPD_CTRL;     // 5568
    REG_NEW_TOD_SEC                 NEW_TOD_SEC;      // 556C
    REG_NEW_TOD_NANO_SEC            NEW_TOD_NANO_SEC; // 5570
    UINT32                          rsv_5574[3];      // 5574..557C
    REG_SLEEP_CFG                   SLEEP_CFG;        // 5580
    REG_SLEEP_CNT                   SLEEP_CNT;        // 5584
    UINT32                          rsv_5588[6];      // 5588..559C
    REG_TURNING_SFC_L               TURNING_SFC_L;    // 55A0
    REG_TURNING_SFC_H               TURNING_SFC_H;    // 55A4
    UINT32                          rsv_55A8[150];    // 55A8..57FC
    REG_DBG_CAP_SETTING             DBG_CAP_SETTING;  // 5800
    REG_DBG_BWM_CHK_CTRL            DBG_BWM_CHK_CTRL; // 5804
    REG_DBG_BWM_CKH_STS             DBG_BWM_CKH_STS;  // 5808
    REG_DBG_BWM_SFIFO_STS           DBG_BWM_SFIFO_STS; // 580C
    REG_DBG_BWM_BFIFO_STS           DBG_BWM_BFIFO_STS; // 5810
    REG_DBG_TX_ALIGN_FIFO_STS       DBG_TX_ALIGN_FIFO_STS; // 5814
    REG_DBG_TX_AES_MAX_USED         DBG_TX_AES_MAX_USED; // 5818
    UINT32                          rsv_581C;         // 581C
    REG_DBG_DBA_BACK_DOOR           DBG_DBA_BACK_DOOR; // 5820
    REG_DBG_DBA_MODIFY              DBG_DBA_MODIFY;   // 5824
    UINT32                          rsv_5828;         // 5828
    REG_DBG_RESYNC                  DBG_RESYNC;       // 582C
    REG_DBG_PHY_DLY_CTRL            DBG_PHY_DLY_CTRL; // 5830
    REG_DBG_DS_SPF_CNT_L            DBG_DS_SPF_CNT_L; // 5834
    REG_DBG_DS_SPF_CNT_H            DBG_DS_SPF_CNT_H; // 5838
    REG_DBG_PON_ID_L                DBG_PON_ID_L;     // 583C
    REG_DBG_PON_ID_H                DBG_PON_ID_H;     // 5840
    REG_DBG_TX_SYNC_OFFSET          DBG_TX_SYNC_OFFSET; // 5844
    REG_DBG_RX_PHY_DLY_OFS          DBG_RX_PHY_DLY_OFS; // 5848
    REG_DBG_RX_HEC_ERR              DBG_RX_HEC_ERR;   // 584C
    REG_RX_GEM_ENC_CHK              RX_GEM_ENC_CHK;   // 5850
    REG_RX_MBI_FIFO_DROP_EN         RX_MBI_FIFO_DROP_EN; // 5854
    REG_RX_MBI_HDR_FIFO_DROP_CTRL   RX_MBI_HDR_FIFO_DROP_CTRL; // 5858
    REG_RX_MBI_PL_FIFO_DROP_CTRL    RX_MBI_PL_FIFO_DROP_CTRL; // 585C
#if defined(TCSUPPORT_CPU_EN7581)
    REG_GET_RX_IDLE_XGEM            GET_RX_IDLE_XGEM; // 5860
    REG_RX_IDLE_XGEM_1ST_PL         RX_IDLE_XGEM_1ST_PL; // 5864
    REG_DBG_CAP_SETTING1            DBG_CAP_SETTING1; // 5868
    REG_BRO_ALLOC_SPECIAL           BRO_ALLOC_SPECIAL; //586C
#if defined(TCSUPPORT_CPU_AN7583)
    REG_DBG_DBA_MODIFY_IDX_0        DBG_DBA_MODIFY_IDX_0; // 5870
    REG_DBG_DBA_MODIFY_IDX_1        DBG_DBA_MODIFY_IDX_1; // 5874
    UINT32                          rsv_5878[30];     // 5878..58EC
#else
    UINT32                          rsv_5870[32];     // 5870..58EC
#endif
#else
    UINT32                          rsv_5860[36];     // 5860..58EC
#endif
    REG_CNT_CLR                     CNT_CLR;          // 58F0
    UINT32                          rsv_58F4[2];      // 58F4..58F8
    REG_RX_HLEND_HEC_CNT            RX_HLEND_HEC_CNT; // 58FC
    REG_RX_ALLOC_HEC_CNT            RX_ALLOC_HEC_CNT; // 5900
    REG_RX_HDR_HEC_CNT              RX_HDR_HEC_CNT;   // 5904
    REG_RX_PHY_HEC_ERR_CNT          RX_PHY_HEC_ERR_CNT; // 5908
    REG_RX_MIC_ERR_CNT              RX_MIC_ERR_CNT;   // 590C
    REG_RX_ETH_FCS_ERR_CNT          RX_ETH_FCS_ERR_CNT; // 5910
    REG_RX_BIP_ERR_CNT              RX_BIP_ERR_CNT;   // 5914
    REG_RX_KEY_ERR_CNT              RX_KEY_ERR_CNT;   // 5918
    REG_RX_LOST_WCNT                RX_LOST_WCNT;     // 591C
    REG_INVLD_PROF_BST_GNT_CNT      INVLD_PROF_BST_GNT_CNT; // 5920
    REG_RX_MBI_XGEM_DROP_CNT        RX_MBI_XGEM_DROP_CNT; // 5924
    UINT32                          rsv_5928[6];      // 5928..593C
    REG_RX_XGTC_CNT                 RX_XGTC_CNT;      // 5940
    REG_TX_BST_CNT                  TX_BST_CNT;       // 5944
    UINT32                          rsv_5948[2];      // 5948..594C
    REG_RX_PLOAMD_CNT               RX_PLOAMD_CNT;    // 5950
    REG_TX_PLOAMU_CNT               TX_PLOAMU_CNT;    // 5954
    UINT32                          rsv_5958[2];      // 5958..595C
    REG_RX_OMCI_CNT                 RX_OMCI_CNT;      // 5960
    REG_TX_OMCI_CNT                 TX_OMCI_CNT;      // 5964
    REG_RX_XGEM_CNT                 RX_XGEM_CNT;      // 5968
    REG_TX_XGEM_CNT                 TX_XGEM_CNT;      // 596C
    REG_RX_MBI_CNT                  RX_MBI_CNT;       // 5970
    REG_TX_MBI_CNT                  TX_MBI_CNT;       // 5974
    REG_RX_NON_IDLE_BCNT            RX_NON_IDLE_BCNT; // 5978
    REG_TX_NON_IDLE_BCNT            TX_NON_IDLE_BCNT; // 597C
    REG_TX_NLF_XGEM_CNT             TX_NLF_XGEM_CNT;  // 5980
    REG_TX_ACK_PLOAMU_CNT           TX_ACK_PLOAMU_CNT; // 5984
    REG_RX_BIP_PROTECT_WCNT         RX_BIP_PROTECT_WCNT; // 5988
    REG_TX_IDLE_BCNT                TX_IDLE_BCNT;     // 598C
#if defined(TCSUPPORT_CPU_EN7581)
    REG_CAL_GNT_SIZE_CTRL_STS       CAL_GNT_SIZE_CTRL_STS; // 5990
    REG_CAL_GNT_SIZE_TCONT_EN       CAL_GNT_SIZE_TCONT_EN; // 5994
    REG_CAL_GNT_SIZE_SUM_TCONT_SEL  CAL_GNT_SIZE_SUM_TCONT_SEL; // 5998
    REG_CAL_GNT_ZERO_TCONT_STS      CAL_GNT_ZERO_TCONT_STS; // 599C
    REG_CAL_GNT_SIZE_SUM_L32        CAL_GNT_SIZE_SUM_L32; // 59A0
    REG_CAL_GNT_SIZE_SUM_H32        CAL_GNT_SIZE_SUM_H32; // 59A4
    REG_MONI_CTRL_FOR_NOT_GNT       MONI_CTRL_FOR_NOT_GNT; // 59A8
    REG_US_BIP_ERR_CTRL             US_BIP_ERR_CTRL;  // 59AC
#if defined(TCSUPPORT_CPU_AN7583)
    REG_HUNT_AFTER_LOSGD            HUNT_AFTER_LOSGD; // 59B0
    REG_BWMAP_LEN_LIMIT             BWMAP_LEN_LIMIT;  // 59B4
    REG_XGEM_HEADER_OVRW            XGEM_HEADER_OVRW; // 59B8
    REG_OMCI_LEN_CTRL               OMCI_LEN_CTRL;    // 59BC
    UINT32                          rsv_59C0[335];    // 59C0..5EF8
#else
    UINT32                          rsv_59B0[339];    // 59B0..5EF8
#endif
    REG_SRAM_PWR_DOWN               SRAM_PWR_DOWN;	//5EFC
    REG_MBIST_FSH_STS				MBIST_FSH_STS;	//5F00
    REG_MBIST_DELSEL_G0				MBIST_DELSEL_G0; //5F04
    REG_MBIST_DELSEL_G1				MBIST_DELSEL_G1; //5F08
    REG_MBIST_DELSEL_G2				MBIST_DELSEL_G2; //5F0C
    REG_MBIST_DELSEL_G3				MBIST_DELSEL_G3; //5F10
    REG_MBIST_DELSEL_G4				MBIST_DELSEL_G4; //5F14
    REG_MBIST_DELSEL_G5				MBIST_DELSEL_G5; //5F18
    REG_MBIST_DELSEL_G6				MBIST_DELSEL_G6; //5F1C
    REG_MBIST_DONE_G0				MBIST_DONE_G0;	 //5F20
    REG_MBIST_FAIL_G0				MBIST_FAIL_G0;	 //5F24
    REG_MBIST_FAIL_G1				MBIST_FAIL_G1;	 //5F28
    UINT32                          rsv_5F2C[41];   //5F2C...5FCC
#else
    UINT32                          rsv_5990[400];    // 5990..5FCC
#endif
    REG_SNF_CTRL                    SNF_CTRL;         // 5FD0
    REG_SNF_GPID                    SNF_GPID;         // 5FD4
    UINT32                          rsv_5FD8[2];      // 5FD8..5FDC
    REG_DS_SNF_ETH_DASA_H16         DS_SNF_ETH_DASA_H16; // 5FE0
    REG_SNF_ETH_TAG                 SNF_ETH_TAG;      // 5FE4
    REG_SNF_ETH_TYPE                SNF_ETH_TYPE;     // 5FE8
    REG_US_SNF_ETH_DASA_H16         US_SNF_ETH_DASA_H16; // 5FEC
    REG_DBG_PROBE_CTRL              DBG_PROBE_CTRL;   // 5FF0
    REG_DBG_PROBE_HIGH32            DBG_PROBE_HIGH32; // 5FF4
    REG_DBG_PROBE_LOW32             DBG_PROBE_LOW32;  // 5FF8
}xgpon_mac_reg_REGS, *Pxgpon_mac_reg_REGS;

// ---------- xgpon_mac_reg Enum Definitions      ----------
// ---------- xgpon_mac_reg C Macro Definitions   ----------
extern Pxgpon_mac_reg_REGS g_xgpon_mac_reg_BASE;

#define xgpon_mac_reg_BASE                                     (g_xgpon_mac_reg_BASE)

#define SW_RST                                                 INREG32(&xgpon_mac_reg_BASE->SW_RST) // 5000
#define MBI_MPI_STOP                                           INREG32(&xgpon_mac_reg_BASE->MBI_MPI_STOP) // 5004
#define VENDOR_ID                                              INREG32(&xgpon_mac_reg_BASE->VENDOR_ID) // 500C
#define VS_SN                                                  INREG32(&xgpon_mac_reg_BASE->VS_SN) // 5010
#define ONU_ID                                                 INREG32(&xgpon_mac_reg_BASE->ONU_ID) // 5014
#define RGS_ID3_0                                              INREG32(&xgpon_mac_reg_BASE->RGS_ID3_0) // 5018
#define RGS_ID7_4                                              INREG32(&xgpon_mac_reg_BASE->RGS_ID7_4) // 501C
#define RGS_ID11_8                                             INREG32(&xgpon_mac_reg_BASE->RGS_ID11_8) // 5020
#define RGS_ID15_12                                            INREG32(&xgpon_mac_reg_BASE->RGS_ID15_12) // 5024
#define RGS_ID19_16                                            INREG32(&xgpon_mac_reg_BASE->RGS_ID19_16) // 5028
#define RGS_ID23_20                                            INREG32(&xgpon_mac_reg_BASE->RGS_ID23_20) // 502C
#define RGS_ID27_24                                            INREG32(&xgpon_mac_reg_BASE->RGS_ID27_24) // 5030
#define RGS_ID31_28                                            INREG32(&xgpon_mac_reg_BASE->RGS_ID31_28) // 5034
#define RGS_ID35_32                                            INREG32(&xgpon_mac_reg_BASE->RGS_ID35_32) // 5038
#define INT_ENABLE                                             INREG32(&xgpon_mac_reg_BASE->INT_ENABLE) // 5040
#define INT_STATUS                                             INREG32(&xgpon_mac_reg_BASE->INT_STATUS) // 5044
#define FIFO_ERR_STS                                           INREG32(&xgpon_mac_reg_BASE->FIFO_ERR_STS) // 5050
#define TX_ERR_STS                                             INREG32(&xgpon_mac_reg_BASE->TX_ERR_STS) // 5054
#define RX_ERR_STS                                             INREG32(&xgpon_mac_reg_BASE->RX_ERR_STS) // 5058
#if defined(TCSUPPORT_CPU_AN7583)
#define FIFO_ERR_ENABLE                                        INREG32(&xgpon_mac_reg_BASE->FIFO_ERR_ENABLE) // 5060
#define TX_ERR_ENABLE                                          INREG32(&xgpon_mac_reg_BASE->TX_ERR_ENABLE) // 5064
#define RX_ERR_ENABLE                                          INREG32(&xgpon_mac_reg_BASE->RX_ERR_ENABLE) // 5068
#endif
#define COR_TAG                                                INREG32(&xgpon_mac_reg_BASE->COR_TAG) // 5080
#define CUR_DS_PON_ID                                          INREG32(&xgpon_mac_reg_BASE->CUR_DS_PON_ID) // 5084
#define CUR_US_PON_ID                                          INREG32(&xgpon_mac_reg_BASE->CUR_US_PON_ID) // 5088
#define CALIB_STS_L32                                          INREG32(&xgpon_mac_reg_BASE->CALIB_STS_L32) // 508C
#define CALIB_STS_H32                                          INREG32(&xgpon_mac_reg_BASE->CALIB_STS_H32) // 5090
#define TUNING_GRAN                                            INREG32(&xgpon_mac_reg_BASE->TUNING_GRAN) // 5094
#define STEP_TUNING_TIME                                       INREG32(&xgpon_mac_reg_BASE->STEP_TUNING_TIME) // 5098
#define US_RATE_CAP                                            INREG32(&xgpon_mac_reg_BASE->US_RATE_CAP) // 509C
#define ATTENUATION                                            INREG32(&xgpon_mac_reg_BASE->ATTENUATION) // 50A0
#define POWER_LEVEL_CAP                                        INREG32(&xgpon_mac_reg_BASE->POWER_LEVEL_CAP) // 50A4
#if defined(TCSUPPORT_CPU_EN7581)
#define ACT_DEBUG_INFO										   INREG32(&xgpon_mac_reg_BASE->ACT_DEBUG_INFO) // 50A8
#endif
#define O23_O4_PLOAMU_CTRL                                     INREG32(&xgpon_mac_reg_BASE->O23_O4_PLOAMU_CTRL) // 5100
#define ACTIVATION_ST                                          INREG32(&xgpon_mac_reg_BASE->ACTIVATION_ST) // 5104
#define RSP_TIME                                               INREG32(&xgpon_mac_reg_BASE->RSP_TIME) // 5108
#define RDM_DLY                                                INREG32(&xgpon_mac_reg_BASE->RDM_DLY) // 510C
#define INT_DLY                                                INREG32(&xgpon_mac_reg_BASE->INT_DLY) // 5110
#define EQD                                                    INREG32(&xgpon_mac_reg_BASE->EQD) // 5114
#define SYN_MRG                                                INREG32(&xgpon_mac_reg_BASE->SYN_MRG) // 5118
#define US_PROF_VLD                                           INREG32(&xgpon_mac_reg_BASE->US_PROF_VLD) // 511C
#define US_PROF_PSBu_LEN_0_1                                   INREG32(&xgpon_mac_reg_BASE->US_PROF_PSBu_LEN_0_1) // 5120
#define US_PROF_PSBu_LEN_2_3                                   INREG32(&xgpon_mac_reg_BASE->US_PROF_PSBu_LEN_2_3) // 5124
#if defined(TCSUPPORT_CPU_EN7581)
#define EXT_RDM_DLY                                            INREG32(&xgpon_mac_reg_BASE->EXT_RDM_DLY) // 5128
#endif
#define US_AES_KEY_CTRL                                        INREG32(&xgpon_mac_reg_BASE->US_AES_KEY_CTRL) // 5200
#define DS_AES_KEY_VLD                                         INREG32(&xgpon_mac_reg_BASE->DS_AES_KEY_VLD) // 5204
#define DEFT_GPID_ENC                                          INREG32(&xgpon_mac_reg_BASE->DEFT_GPID_ENC) // 5208
#define AES_UC_IDX0_KEY0                                       INREG32(&xgpon_mac_reg_BASE->AES_UC_IDX0_KEY0) // 5210
#define AES_UC_IDX0_KEY1                                       INREG32(&xgpon_mac_reg_BASE->AES_UC_IDX0_KEY1) // 5214
#define AES_UC_IDX0_KEY2                                       INREG32(&xgpon_mac_reg_BASE->AES_UC_IDX0_KEY2) // 5218
#define AES_UC_IDX0_KEY3                                       INREG32(&xgpon_mac_reg_BASE->AES_UC_IDX0_KEY3) // 521C
#define AES_UC_IDX1_KEY0                                       INREG32(&xgpon_mac_reg_BASE->AES_UC_IDX1_KEY0) // 5220
#define AES_UC_IDX1_KEY1                                       INREG32(&xgpon_mac_reg_BASE->AES_UC_IDX1_KEY1) // 5224
#define AES_UC_IDX1_KEY2                                       INREG32(&xgpon_mac_reg_BASE->AES_UC_IDX1_KEY2) // 5228
#define AES_UC_IDX1_KEY3                                       INREG32(&xgpon_mac_reg_BASE->AES_UC_IDX1_KEY3) // 522C
#define AES_BC_IDX0_KEY0                                       INREG32(&xgpon_mac_reg_BASE->AES_BC_IDX0_KEY0) // 5230
#define AES_BC_IDX0_KEY1                                       INREG32(&xgpon_mac_reg_BASE->AES_BC_IDX0_KEY1) // 5234
#define AES_BC_IDX0_KEY2                                       INREG32(&xgpon_mac_reg_BASE->AES_BC_IDX0_KEY2) // 5238
#define AES_BC_IDX0_KEY3                                       INREG32(&xgpon_mac_reg_BASE->AES_BC_IDX0_KEY3) // 523C
#define AES_BC_IDX1_KEY0                                       INREG32(&xgpon_mac_reg_BASE->AES_BC_IDX1_KEY0) // 5240
#define AES_BC_IDX1_KEY1                                       INREG32(&xgpon_mac_reg_BASE->AES_BC_IDX1_KEY1) // 5244
#define AES_BC_IDX1_KEY2                                       INREG32(&xgpon_mac_reg_BASE->AES_BC_IDX1_KEY2) // 5248
#define AES_BC_IDX1_KEY3                                       INREG32(&xgpon_mac_reg_BASE->AES_BC_IDX1_KEY3) // 524C
#define TCONT_ID_CFG                                           INREG32(&xgpon_mac_reg_BASE->TCONT_ID_CFG) // 5250
#define TCONT_ID_STS                                           INREG32(&xgpon_mac_reg_BASE->TCONT_ID_STS) // 5254
#define GPIDX_TBL_INIT                                         INREG32(&xgpon_mac_reg_BASE->GPIDX_TBL_INIT) // 5260
#define GPIDX_TBL_CTRL                                         INREG32(&xgpon_mac_reg_BASE->GPIDX_TBL_CTRL) // 5264
#define GPIDX_TBL_STS                                          INREG32(&xgpon_mac_reg_BASE->GPIDX_TBL_STS) // 5268
#define GEM_TBL_INIT                                           INREG32(&xgpon_mac_reg_BASE->GEM_TBL_INIT) // 5270
#define GEM_PORT_CFG                                           INREG32(&xgpon_mac_reg_BASE->GEM_PORT_CFG) // 5274
#define GEM_PORT_STS                                           INREG32(&xgpon_mac_reg_BASE->GEM_PORT_STS) // 5278
#define G_TX_FCS_TBL_INIT                                      INREG32(&xgpon_mac_reg_BASE->G_TX_FCS_TBL_INIT) // 527C
#define IDLE_GEM_CTRL                                          INREG32(&xgpon_mac_reg_BASE->IDLE_GEM_CTRL) // 5280
#define US_DYING_GASP_CTRL                                     INREG32(&xgpon_mac_reg_BASE->US_DYING_GASP_CTRL) // 5284
#define TX_XGEM_IDLE_DATA                                      INREG32(&xgpon_mac_reg_BASE->TX_XGEM_IDLE_DATA) // 5288
#define TX_OMCI_PRE_GET                                        INREG32(&xgpon_mac_reg_BASE->TX_OMCI_PRE_GET) // 528C
#define RX_OMCI_PRE_GET                                        INREG32(&xgpon_mac_reg_BASE->RX_OMCI_PRE_GET) // 5290
#define EPDROP_EN                                              INREG32(&xgpon_mac_reg_BASE->EPDROP_EN) // 52F0
#define PLOAMu_FIFO_STS                                        INREG32(&xgpon_mac_reg_BASE->PLOAMu_FIFO_STS) // 5300
#define PLOAMu_WDATA                                           INREG32(&xgpon_mac_reg_BASE->PLOAMu_WDATA) // 5304
#define PLOAMd_FIFO_STS                                        INREG32(&xgpon_mac_reg_BASE->PLOAMd_FIFO_STS) // 5308
#define PLOAMd_RDATA                                           INREG32(&xgpon_mac_reg_BASE->PLOAMd_RDATA) // 530C
#define KEY_GEN                                                INREG32(&xgpon_mac_reg_BASE->KEY_GEN) // 5314
#define CUR_KIDX                                               INREG32(&xgpon_mac_reg_BASE->CUR_KIDX) // 5318
#define MSK_0                                                  INREG32(&xgpon_mac_reg_BASE->MSK_0) // 5320
#define MSK_1                                                  INREG32(&xgpon_mac_reg_BASE->MSK_1) // 5324
#define MSK_2                                                  INREG32(&xgpon_mac_reg_BASE->MSK_2) // 5328
#define MSK_3                                                  INREG32(&xgpon_mac_reg_BASE->MSK_3) // 532C
#define REGMSK_0                                               INREG32(&xgpon_mac_reg_BASE->REGMSK_0) // 5330
#define REGMSK_1                                               INREG32(&xgpon_mac_reg_BASE->REGMSK_1) // 5334
#define REGMSK_2                                               INREG32(&xgpon_mac_reg_BASE->REGMSK_2) // 5338
#define REGMSK_3                                               INREG32(&xgpon_mac_reg_BASE->REGMSK_3) // 533C
#define SK_0                                                   INREG32(&xgpon_mac_reg_BASE->SK_0) // 5340
#define SK_1                                                   INREG32(&xgpon_mac_reg_BASE->SK_1) // 5344
#define SK_2                                                   INREG32(&xgpon_mac_reg_BASE->SK_2) // 5348
#define SK_3                                                   INREG32(&xgpon_mac_reg_BASE->SK_3) // 534C
#define HW_GENK_0                                              INREG32(&xgpon_mac_reg_BASE->HW_GENK_0) // 5350
#define HW_GENK_1                                              INREG32(&xgpon_mac_reg_BASE->HW_GENK_1) // 5354
#define HW_GENK_2                                              INREG32(&xgpon_mac_reg_BASE->HW_GENK_2) // 5358
#define HW_GENK_3                                              INREG32(&xgpon_mac_reg_BASE->HW_GENK_3) // 535C
#define PIK0_0                                                 INREG32(&xgpon_mac_reg_BASE->PIK0_0) // 5360
#define PIK0_1                                                 INREG32(&xgpon_mac_reg_BASE->PIK0_1) // 5364
#define PIK0_2                                                 INREG32(&xgpon_mac_reg_BASE->PIK0_2) // 5368
#define PIK0_3                                                 INREG32(&xgpon_mac_reg_BASE->PIK0_3) // 536C
#define PIK1_0                                                 INREG32(&xgpon_mac_reg_BASE->PIK1_0) // 5370
#define PIK1_1                                                 INREG32(&xgpon_mac_reg_BASE->PIK1_1) // 5374
#define PIK1_2                                                 INREG32(&xgpon_mac_reg_BASE->PIK1_2) // 5378
#define PIK1_3                                                 INREG32(&xgpon_mac_reg_BASE->PIK1_3) // 537C
#define OIK0_0                                                 INREG32(&xgpon_mac_reg_BASE->OIK0_0) // 5380
#define OIK0_1                                                 INREG32(&xgpon_mac_reg_BASE->OIK0_1) // 5384
#define OIK0_2                                                 INREG32(&xgpon_mac_reg_BASE->OIK0_2) // 5388
#define OIK0_3                                                 INREG32(&xgpon_mac_reg_BASE->OIK0_3) // 538C
#define OIK1_0                                                 INREG32(&xgpon_mac_reg_BASE->OIK1_0) // 5390
#define OIK1_1                                                 INREG32(&xgpon_mac_reg_BASE->OIK1_1) // 5394
#define OIK1_2                                                 INREG32(&xgpon_mac_reg_BASE->OIK1_2) // 5398
#define OIK1_3                                                 INREG32(&xgpon_mac_reg_BASE->OIK1_3) // 539C
#define KEK0_0                                                 INREG32(&xgpon_mac_reg_BASE->KEK0_0) // 53A0
#define KEK0_1                                                 INREG32(&xgpon_mac_reg_BASE->KEK0_1) // 53A4
#define KEK0_2                                                 INREG32(&xgpon_mac_reg_BASE->KEK0_2) // 53A8
#define KEK0_3                                                 INREG32(&xgpon_mac_reg_BASE->KEK0_3) // 53AC
#define KEK1_0                                                 INREG32(&xgpon_mac_reg_BASE->KEK1_0) // 53B0
#define KEK1_1                                                 INREG32(&xgpon_mac_reg_BASE->KEK1_1) // 53B4
#define KEK1_2                                                 INREG32(&xgpon_mac_reg_BASE->KEK1_2) // 53B8
#define KEK1_3                                                 INREG32(&xgpon_mac_reg_BASE->KEK1_3) // 53BC
#define PON_TAG_0                                              INREG32(&xgpon_mac_reg_BASE->PON_TAG_0) // 53C0
#define PON_TAG_1                                              INREG32(&xgpon_mac_reg_BASE->PON_TAG_1) // 53C4
#define SW_SET_KIDX                                            INREG32(&xgpon_mac_reg_BASE->SW_SET_KIDX) // 53E8
#define SW0_ENCSTART                                           INREG32(&xgpon_mac_reg_BASE->SW0_ENCSTART) // 5400
#define SW0_MADDR                                              INREG32(&xgpon_mac_reg_BASE->SW0_MADDR) // 5404
#define SW0_RADDR                                              INREG32(&xgpon_mac_reg_BASE->SW0_RADDR) // 5408
#define SW0_KADDR                                              INREG32(&xgpon_mac_reg_BASE->SW0_KADDR) // 540C
#define SW0_ENCLEN                                             INREG32(&xgpon_mac_reg_BASE->SW0_ENCLEN) // 5410
#define SW0_ENCINFO                                            INREG32(&xgpon_mac_reg_BASE->SW0_ENCINFO) // 5414
#define SW1_ENCSTART                                           INREG32(&xgpon_mac_reg_BASE->SW1_ENCSTART) // 5420
#define SW1_MADDR                                              INREG32(&xgpon_mac_reg_BASE->SW1_MADDR) // 5424
#define SW1_RADDR                                              INREG32(&xgpon_mac_reg_BASE->SW1_RADDR) // 5428
#define SW1_KADDR                                              INREG32(&xgpon_mac_reg_BASE->SW1_KADDR) // 542C
#define SW1_ENCLEN                                             INREG32(&xgpon_mac_reg_BASE->SW1_ENCLEN) // 5430
#define SW1_ENCINFO                                            INREG32(&xgpon_mac_reg_BASE->SW1_ENCINFO) // 5434
#if defined(TCSUPPORT_CPU_AN7583)
#define TX_FCS_TBL_CLEAR                                       INREG32(&xgpon_mac_reg_BASE->TX_FCS_TBL_CLEAR) // 5438
#endif
#define MIB_CTRL                                               INREG32(&xgpon_mac_reg_BASE->MIB_CTRL) // 5500
#define MIB_TBL_CONFIG                                         INREG32(&xgpon_mac_reg_BASE->MIB_TBL_CONFIG) // 5504
#define MIB_CTRL_STS                                           INREG32(&xgpon_mac_reg_BASE->MIB_CTRL_STS) // 5508
#define MIB_RDATA_L32                                          INREG32(&xgpon_mac_reg_BASE->MIB_RDATA_L32) // 5510
#define MIB_RDATA_H32                                          INREG32(&xgpon_mac_reg_BASE->MIB_RDATA_H32) // 5514
#define MIB_WDATA_L32                                          INREG32(&xgpon_mac_reg_BASE->MIB_WDATA_L32) // 5518
#define MIB_WDATA_H32                                          INREG32(&xgpon_mac_reg_BASE->MIB_WDATA_H32) // 551C
#define TOD_CLK_PERIOD                                         INREG32(&xgpon_mac_reg_BASE->TOD_CLK_PERIOD) // 5550
#define TOD_1PPS_WD_CTRL                                       INREG32(&xgpon_mac_reg_BASE->TOD_1PPS_WD_CTRL) // 5554
#define CUR_TOD_SEC                                            INREG32(&xgpon_mac_reg_BASE->CUR_TOD_SEC) // 5558
#define CUR_TOD_NANO_SEC                                       INREG32(&xgpon_mac_reg_BASE->CUR_TOD_NANO_SEC) // 555C
#define TOD_SFC_L                                              INREG32(&xgpon_mac_reg_BASE->TOD_SFC_L) // 5560
#define TOD_SFC_H                                              INREG32(&xgpon_mac_reg_BASE->TOD_SFC_H) // 5564
#define TOD_UPD_CTRL                                           INREG32(&xgpon_mac_reg_BASE->TOD_UPD_CTRL) // 5568
#define NEW_TOD_SEC                                            INREG32(&xgpon_mac_reg_BASE->NEW_TOD_SEC) // 556C
#define NEW_TOD_NANO_SEC                                       INREG32(&xgpon_mac_reg_BASE->NEW_TOD_NANO_SEC) // 5570
#define SLEEP_CFG                                              INREG32(&xgpon_mac_reg_BASE->SLEEP_CFG) // 5580
#define SLEEP_CNT                                              INREG32(&xgpon_mac_reg_BASE->SLEEP_CNT) // 5584
#define TURNING_SFC_L                                          INREG32(&xgpon_mac_reg_BASE->TURNING_SFC_L) // 55A0
#define TURNING_SFC_H                                          INREG32(&xgpon_mac_reg_BASE->TURNING_SFC_H) // 55A4
#define DBG_CAP_SETTING                                        INREG32(&xgpon_mac_reg_BASE->DBG_CAP_SETTING) // 5800
#define DBG_BWM_CHK_CTRL                                       INREG32(&xgpon_mac_reg_BASE->DBG_BWM_CHK_CTRL) // 5804
#define DBG_BWM_CKH_STS                                        INREG32(&xgpon_mac_reg_BASE->DBG_BWM_CKH_STS) // 5808
#define DBG_BWM_SFIFO_STS                                      INREG32(&xgpon_mac_reg_BASE->DBG_BWM_SFIFO_STS) // 580C
#define DBG_BWM_BFIFO_STS                                      INREG32(&xgpon_mac_reg_BASE->DBG_BWM_BFIFO_STS) // 5810
#define DBG_TX_ALIGN_FIFO_STS                                  INREG32(&xgpon_mac_reg_BASE->DBG_TX_ALIGN_FIFO_STS) // 5814
#define DBG_TX_AES_MAX_USED                                    INREG32(&xgpon_mac_reg_BASE->DBG_TX_AES_MAX_USED) // 5818
#define DBG_DBA_BACK_DOOR                                      INREG32(&xgpon_mac_reg_BASE->DBG_DBA_BACK_DOOR) // 5820
#define DBG_DBA_MODIFY                                         INREG32(&xgpon_mac_reg_BASE->DBG_DBA_MODIFY) // 5824
#define DBG_RESYNC                                             INREG32(&xgpon_mac_reg_BASE->DBG_RESYNC) // 582C
#define DBG_PHY_DLY_CTRL                                       INREG32(&xgpon_mac_reg_BASE->DBG_PHY_DLY_CTRL) // 5830
#define DBG_DS_SPF_CNT_L                                       INREG32(&xgpon_mac_reg_BASE->DBG_DS_SPF_CNT_L) // 5834
#define DBG_DS_SPF_CNT_H                                       INREG32(&xgpon_mac_reg_BASE->DBG_DS_SPF_CNT_H) // 5838
#define DBG_PON_ID_L                                           INREG32(&xgpon_mac_reg_BASE->DBG_PON_ID_L) // 583C
#define DBG_PON_ID_H                                           INREG32(&xgpon_mac_reg_BASE->DBG_PON_ID_H) // 5840
#define DBG_TX_SYNC_OFFSET                                     INREG32(&xgpon_mac_reg_BASE->DBG_TX_SYNC_OFFSET) // 5844
#define DBG_RX_PHY_DLY_OFS                                     INREG32(&xgpon_mac_reg_BASE->DBG_RX_PHY_DLY_OFS) // 5848
#define DBG_RX_HEC_ERR                                         INREG32(&xgpon_mac_reg_BASE->DBG_RX_HEC_ERR) // 584C
#define RX_GEM_ENC_CHK                                         INREG32(&xgpon_mac_reg_BASE->RX_GEM_ENC_CHK) // 5850
#define RX_MBI_FIFO_DROP_EN                                    INREG32(&xgpon_mac_reg_BASE->RX_MBI_FIFO_DROP_EN) // 5854
#define RX_MBI_HDR_FIFO_DROP_CTRL                              INREG32(&xgpon_mac_reg_BASE->RX_MBI_HDR_FIFO_DROP_CTRL) // 5858
#define RX_MBI_PL_FIFO_DROP_CTRL                               INREG32(&xgpon_mac_reg_BASE->RX_MBI_PL_FIFO_DROP_CTRL) // 585C
#if defined(TCSUPPORT_CPU_EN7581)
#define GET_RX_IDLE_XGEM                                       INREG32(&xgpon_mac_reg_BASE->GET_RX_IDLE_XGEM) // 5860
#define RX_IDLE_XGEM_1ST_PL                                    INREG32(&xgpon_mac_reg_BASE->RX_IDLE_XGEM_1ST_PL) // 5864
#define DBG_CAP_SETTING1                                       INREG32(&xgpon_mac_reg_BASE->DBG_CAP_SETTING1) // 5868
#define BRO_ALLOC_SPECIAL									   INREG32(&xgpon_mac_reg_BASE->BRO_ALLOC_SPECIAL) // 586C
#endif
#if defined(TCSUPPORT_CPU_AN7583)
#define DBG_DBA_MODIFY_IDX_0                                   INREG32(&xgpon_mac_reg_BASE->DBG_DBA_MODIFY_IDX_0) // 5870
#define DBG_DBA_MODIFY_IDX_1                                   INREG32(&xgpon_mac_reg_BASE->DBG_DBA_MODIFY_IDX_1) // 5874
#endif
#define CNT_CLR                                                INREG32(&xgpon_mac_reg_BASE->CNT_CLR) // 58F0
#define RX_HLEND_HEC_CNT                                       INREG32(&xgpon_mac_reg_BASE->RX_HLEND_HEC_CNT) // 58FC
#define RX_ALLOC_HEC_CNT                                       INREG32(&xgpon_mac_reg_BASE->RX_ALLOC_HEC_CNT) // 5900
#define RX_HDR_HEC_CNT                                         INREG32(&xgpon_mac_reg_BASE->RX_HDR_HEC_CNT) // 5904
#define RX_PHY_HEC_ERR_CNT                                     INREG32(&xgpon_mac_reg_BASE->RX_PHY_HEC_ERR_CNT) // 5908
#define RX_MIC_ERR_CNT                                         INREG32(&xgpon_mac_reg_BASE->RX_MIC_ERR_CNT) // 590C
#define RX_ETH_FCS_ERR_CNT                                     INREG32(&xgpon_mac_reg_BASE->RX_ETH_FCS_ERR_CNT) // 5910
#define RX_BIP_ERR_CNT                                         INREG32(&xgpon_mac_reg_BASE->RX_BIP_ERR_CNT) // 5914
#define RX_KEY_ERR_CNT                                         INREG32(&xgpon_mac_reg_BASE->RX_KEY_ERR_CNT) // 5918
#define RX_LOST_WCNT                                           INREG32(&xgpon_mac_reg_BASE->RX_LOST_WCNT) // 591C
#define INVLD_PROF_BST_GNT_CNT                                 INREG32(&xgpon_mac_reg_BASE->INVLD_PROF_BST_GNT_CNT) // 5920
#define RX_MBI_XGEM_DROP_CNT                                   INREG32(&xgpon_mac_reg_BASE->RX_MBI_XGEM_DROP_CNT) // 5924
#define RX_XGTC_CNT                                            INREG32(&xgpon_mac_reg_BASE->RX_XGTC_CNT) // 5940
#define TX_BST_CNT                                             INREG32(&xgpon_mac_reg_BASE->TX_BST_CNT) // 5944
#define RX_PLOAMD_CNT                                          INREG32(&xgpon_mac_reg_BASE->RX_PLOAMD_CNT) // 5950
#define TX_PLOAMU_CNT                                          INREG32(&xgpon_mac_reg_BASE->TX_PLOAMU_CNT) // 5954
#define RX_OMCI_CNT                                            INREG32(&xgpon_mac_reg_BASE->RX_OMCI_CNT) // 5960
#define TX_OMCI_CNT                                            INREG32(&xgpon_mac_reg_BASE->TX_OMCI_CNT) // 5964
#define RX_XGEM_CNT                                            INREG32(&xgpon_mac_reg_BASE->RX_XGEM_CNT) // 5968
#define TX_XGEM_CNT                                            INREG32(&xgpon_mac_reg_BASE->TX_XGEM_CNT) // 596C
#define RX_MBI_CNT                                             INREG32(&xgpon_mac_reg_BASE->RX_MBI_CNT) // 5970
#define TX_MBI_CNT                                             INREG32(&xgpon_mac_reg_BASE->TX_MBI_CNT) // 5974
#define RX_NON_IDLE_BCNT                                       INREG32(&xgpon_mac_reg_BASE->RX_NON_IDLE_BCNT) // 5978
#define TX_NON_IDLE_BCNT                                       INREG32(&xgpon_mac_reg_BASE->TX_NON_IDLE_BCNT) // 597C
#define TX_NLF_XGEM_CNT                                        INREG32(&xgpon_mac_reg_BASE->TX_NLF_XGEM_CNT) // 5980
#define TX_ACK_PLOAMU_CNT                                      INREG32(&xgpon_mac_reg_BASE->TX_ACK_PLOAMU_CNT) // 5984
#define RX_BIP_PROTECT_WCNT                                    INREG32(&xgpon_mac_reg_BASE->RX_BIP_PROTECT_WCNT) // 5988
#define TX_IDLE_BCNT                                           INREG32(&xgpon_mac_reg_BASE->TX_IDLE_BCNT) // 598C
#if defined(TCSUPPORT_CPU_EN7581)
#define CAL_GNT_SIZE_CTRL_STS                                  INREG32(&xgpon_mac_reg_BASE->CAL_GNT_SIZE_CTRL_STS) // 5990
#define CAL_GNT_SIZE_TCONT_EN                                  INREG32(&xgpon_mac_reg_BASE->CAL_GNT_SIZE_TCONT_EN) // 5994
#define CAL_GNT_SIZE_SUM_TCONT_SEL                             INREG32(&xgpon_mac_reg_BASE->CAL_GNT_SIZE_SUM_TCONT_SEL) // 5998
#define CAL_GNT_ZERO_TCONT_STS                                 INREG32(&xgpon_mac_reg_BASE->CAL_GNT_ZERO_TCONT_STS) // 599C
#define CAL_GNT_SIZE_SUM_L32                                   INREG32(&xgpon_mac_reg_BASE->CAL_GNT_SIZE_SUM_L32) // 59A0
#define CAL_GNT_SIZE_SUM_H32                                   INREG32(&xgpon_mac_reg_BASE->CAL_GNT_SIZE_SUM_H32) // 59A4
#define MONI_CTRL_FOR_NOT_GNT                                  INREG32(&xgpon_mac_reg_BASE->MONI_CTRL_FOR_NOT_GNT) // 59A8
#define US_BIP_ERR_CTRL                                        INREG32(&xgpon_mac_reg_BASE->US_BIP_ERR_CTRL) // 59AC
#if defined(TCSUPPORT_CPU_AN7583)
#define HUNT_AFTER_LOSGD                                       INREG32(&xgpon_mac_reg_BASE->HUNT_AFTER_LOSGD) // 59B0
#define BWMAP_LEN_LIMIT                                        INREG32(&xgpon_mac_reg_BASE->BWMAP_LEN_LIMIT) // 59B4
#define XGEM_HEADER_OVRW                                       INREG32(&xgpon_mac_reg_BASE->XGEM_HEADER_OVRW) // 59B8
#define OMCI_LEN_CTRL                                          INREG32(&xgpon_mac_reg_BASE->OMCI_LEN_CTRL) // 59BC
#endif
#define SRAM_PWR_DOWN									   	   INREG32(&xgpon_mac_reg_BASE->SRAM_PWR_DOWN) // 5EFC
#define MBIST_FSH_STS										   INREG32(&xgpon_mac_reg_BASE->MBIST_FSH_STS) // 5F00
#define MBIST_DELSEL_G0										   INREG32(&xgpon_mac_reg_BASE->MBIST_DELSEL_G0) // 5F04
#define MBIST_DELSEL_G1										   INREG32(&xgpon_mac_reg_BASE->MBIST_DELSEL_G1) // 5F08
#define MBIST_DELSEL_G2										   INREG32(&xgpon_mac_reg_BASE->MBIST_DELSEL_G2) // 5F0C
#define MBIST_DELSEL_G3										   INREG32(&xgpon_mac_reg_BASE->MBIST_DELSEL_G3) // 5F10
#define MBIST_DELSEL_G4										   INREG32(&xgpon_mac_reg_BASE->MBIST_DELSEL_G4) // 5F14
#define MBIST_DELSEL_G5										   INREG32(&xgpon_mac_reg_BASE->MBIST_DELSEL_G5) // 5F18
#define MBIST_DELSEL_G6										   INREG32(&xgpon_mac_reg_BASE->MBIST_DELSEL_G6) // 5F1C
#define MBIST_DONE_G0										   INREG32(&xgpon_mac_reg_BASE->MBIST_DONE_G0) // 5F20
#define MBIST_FAIL_G0										   INREG32(&xgpon_mac_reg_BASE->MBIST_FAIL_G0) // 5F24
#define MBIST_FAIL_G1										   INREG32(&xgpon_mac_reg_BASE->MBIST_FAIL_G1) // 5F28
#endif
#define SNF_CTRL                                               INREG32(&xgpon_mac_reg_BASE->SNF_CTRL) // 5FD0
#define SNF_GPID                                               INREG32(&xgpon_mac_reg_BASE->SNF_GPID) // 5FD4
#define DS_SNF_ETH_DASA_H16                                    INREG32(&xgpon_mac_reg_BASE->DS_SNF_ETH_DASA_H16) // 5FE0
#define SNF_ETH_TAG                                            INREG32(&xgpon_mac_reg_BASE->SNF_ETH_TAG) // 5FE4
#define SNF_ETH_TYPE                                           INREG32(&xgpon_mac_reg_BASE->SNF_ETH_TYPE) // 5FE8
#define US_SNF_ETH_DASA_H16                                    INREG32(&xgpon_mac_reg_BASE->US_SNF_ETH_DASA_H16) // 5FEC
#define DBG_PROBE_CTRL                                         INREG32(&xgpon_mac_reg_BASE->DBG_PROBE_CTRL) // 5FF0
#define DBG_PROBE_HIGH32                                       INREG32(&xgpon_mac_reg_BASE->DBG_PROBE_HIGH32) // 5FF4
#define DBG_PROBE_LOW32                                        INREG32(&xgpon_mac_reg_BASE->DBG_PROBE_LOW32) // 5FF8

#endif


#define SW_RST_FLD_xgpon_mac_sw_rst_n                          REG_FLD(1, 0)

#define MBI_MPI_STOP_FLD_mpi_tx_stop_done                      REG_FLD(1, 31)
#define MBI_MPI_STOP_FLD_mpi_rx_stop_done                      REG_FLD(1, 30)
#define MBI_MPI_STOP_FLD_mpi_tx_stop                           REG_FLD(1, 24)
#define MBI_MPI_STOP_FLD_mpi_rx_stop                           REG_FLD(1, 16)
#define MBI_MPI_STOP_FLD_mbi_tx_stop_done                      REG_FLD(1, 15)
#define MBI_MPI_STOP_FLD_mbi_rx_stop_done                      REG_FLD(1, 14)
#if defined(TCSUPPORT_CPU_EN7581)
#define MBI_MPI_STOP_FLD_dbru_stop_done                        REG_FLD(1, 13)
#define MBI_MPI_STOP_FLD_dbru_stop                             REG_FLD(1, 12)
#endif
#define MBI_MPI_STOP_FLD_mbi_tx_stop                           REG_FLD(1, 8)
#if defined(TCSUPPORT_CPU_EN7581)
#define MBI_MPI_STOP_FLD_del_rx_stop                           REG_FLD(1, 4)
#endif
#define MBI_MPI_STOP_FLD_mbi_rx_stop                           REG_FLD(1, 0)

#define VENDOR_ID_FLD_vendor_id                                REG_FLD(32, 0)

#define VS_SN_FLD_vs_sn                                        REG_FLD(32, 0)

#define ONU_ID_FLD_onu_id_vld                                  REG_FLD(1, 15)
#define ONU_ID_FLD_onu_id                                      REG_FLD(10, 0)

#define RGS_ID3_0_FLD_rgs_id3_0                                REG_FLD(32, 0)

#define RGS_ID7_4_FLD_rgs_id7_4                                REG_FLD(32, 0)

#define RGS_ID11_8_FLD_rgs_id11_8                              REG_FLD(32, 0)

#define RGS_ID15_12_FLD_rgs_id15_12                            REG_FLD(32, 0)

#define RGS_ID19_16_FLD_rgs_id19_16                            REG_FLD(32, 0)

#define RGS_ID23_20_FLD_rgs_id23_20                            REG_FLD(32, 0)

#define RGS_ID27_24_FLD_rgs_id27_24                            REG_FLD(32, 0)

#define RGS_ID31_28_FLD_rgs_id31_28                            REG_FLD(32, 0)

#define RGS_ID35_32_FLD_rgs_id35_32                            REG_FLD(32, 0)

#define INT_ENABLE_FLD_o5_sn_onu_req_recv_int_en               REG_FLD(1, 25)
#define INT_ENABLE_FLD_turning_sfc_match_int_en                REG_FLD(1, 24)
#define INT_ENABLE_FLD_o9_gnt_recv_int_en                      REG_FLD(1, 23)
#define INT_ENABLE_FLD_sw1_mic_done_int_en                     REG_FLD(1, 22)
#define INT_ENABLE_FLD_sw0_mic_done_int_en                     REG_FLD(1, 21)
#define INT_ENABLE_FLD_key_cal_done_int_en                     REG_FLD(1, 20)
#define INT_ENABLE_FLD_lwi_int_en                              REG_FLD(1, 19)
#define INT_ENABLE_FLD_fwi_int_en                              REG_FLD(1, 18)
#define INT_ENABLE_FLD_rx_err_int_en                           REG_FLD(1, 17)
#define INT_ENABLE_FLD_tx_err_int_en                           REG_FLD(1, 16)
#define INT_ENABLE_FLD_fifo_err_int_en                         REG_FLD(1, 15)
#define INT_ENABLE_FLD_o5_eqd_adj_done_int_en                  REG_FLD(1, 14)
#define INT_ENABLE_FLD_bwm_chk_err_int_en                      REG_FLD(1, 13)
#define INT_ENABLE_FLD_dying_gasp_send_int_en                  REG_FLD(1, 12)
#define INT_ENABLE_FLD_tod_1pps_int_en                         REG_FLD(1, 11)
#define INT_ENABLE_FLD_tod_update_done_int_en                  REG_FLD(1, 10)
#define INT_ENABLE_FLD_olt_ds_fec_chg_int_en                   REG_FLD(1, 9)
#define INT_ENABLE_FLD_us_prof_idx_chg_int_en                  REG_FLD(1, 8)
#define INT_ENABLE_FLD_us_key_switch_done_int_en               REG_FLD(1, 7)
#define INT_ENABLE_FLD_us_no_msg_send_int_en                   REG_FLD(1, 6)
#define INT_ENABLE_FLD_o4_registration_send_int_en             REG_FLD(1, 5)
#define INT_ENABLE_FLD_o4_ranging_req_recv_int_en              REG_FLD(1, 4)
#define INT_ENABLE_FLD_o23_sn_onu_send_int_en                  REG_FLD(1, 3)
#define INT_ENABLE_FLD_o23_sn_onu_req_recv_int_en              REG_FLD(1, 2)
#define INT_ENABLE_FLD_ploamu_send_int_en                      REG_FLD(1, 1)
#define INT_ENABLE_FLD_ploamd_recv_int_en                      REG_FLD(1, 0)

#define INT_STATUS_FLD_o5_sn_onu_req_recv_int                  REG_FLD(1, 25)
#define INT_STATUS_FLD_turning_sfc_match_int                   REG_FLD(1, 24)
#define INT_STATUS_FLD_o9_gnt_recv_int                         REG_FLD(1, 23)
#define INT_STATUS_FLD_sw1_mic_done_int                        REG_FLD(1, 22)
#define INT_STATUS_FLD_sw0_mic_done_int                        REG_FLD(1, 21)
#define INT_STATUS_FLD_key_cal_done_int                        REG_FLD(1, 20)
#define INT_STATUS_FLD_lwi_int                                 REG_FLD(1, 19)
#define INT_STATUS_FLD_fwi_int                                 REG_FLD(1, 18)
#define INT_STATUS_FLD_rx_err_int                              REG_FLD(1, 17)
#define INT_STATUS_FLD_tx_err_int                              REG_FLD(1, 16)
#define INT_STATUS_FLD_fifo_err_int                            REG_FLD(1, 15)
#define INT_STATUS_FLD_o5_eqd_adj_done_int                     REG_FLD(1, 14)
#define INT_STATUS_FLD_bwm_chk_err_int                         REG_FLD(1, 13)
#define INT_STATUS_FLD_dying_gasp_send_int                     REG_FLD(1, 12)
#define INT_STATUS_FLD_tod_1pps_int                            REG_FLD(1, 11)
#define INT_STATUS_FLD_tod_update_done_int                     REG_FLD(1, 10)
#define INT_STATUS_FLD_olt_ds_fec_chg_int                      REG_FLD(1, 9)
#define INT_STATUS_FLD_us_prof_idx_chg_int                     REG_FLD(1, 8)
#define INT_STATUS_FLD_us_key_switch_done_int                  REG_FLD(1, 7)
#define INT_STATUS_FLD_us_no_msg_send_int                      REG_FLD(1, 6)
#define INT_STATUS_FLD_o4_registration_send_int                REG_FLD(1, 5)
#define INT_STATUS_FLD_o4_ranging_req_recv_int                 REG_FLD(1, 4)
#define INT_STATUS_FLD_o23_sn_onu_send_int                     REG_FLD(1, 3)
#define INT_STATUS_FLD_o23_sn_onu_req_recv_int                 REG_FLD(1, 2)
#define INT_STATUS_FLD_ploamu_send_int                         REG_FLD(1, 1)
#define INT_STATUS_FLD_ploamd_recv_int                         REG_FLD(1, 0)

#define FIFO_ERR_STS_FLD_tx_aes_rdm_ciph_fifo_ovrn             REG_FLD(1, 17)
#define FIFO_ERR_STS_FLD_byte_mib_cmd_fifo_ovrn                REG_FLD(1, 16)
#define FIFO_ERR_STS_FLD_frm_mib_cmd_fifo_ovrn                 REG_FLD(1, 15)
#define FIFO_ERR_STS_FLD_mib_cmd_fifo_ovrn                     REG_FLD(1, 14)
#define FIFO_ERR_STS_FLD_mib_tx_cmd_fifo_ovrn                  REG_FLD(1, 13)
#define FIFO_ERR_STS_FLD_snf_ds_fifo_ovrn                      REG_FLD(1, 12)
#define FIFO_ERR_STS_FLD_snf_us_fifo_ovrn                      REG_FLD(1, 11)
#define FIFO_ERR_STS_FLD_rx_ploamd_fifo_udrn                   REG_FLD(1, 10)
#define FIFO_ERR_STS_FLD_rx_ploamd_fifo_ovrn                   REG_FLD(1, 9)
#define FIFO_ERR_STS_FLD_rx_mbi_pl_fifo_ovrn                   REG_FLD(1, 8)
#define FIFO_ERR_STS_FLD_rx_mbi_hdr_fifo_ovrn                  REG_FLD(1, 7)
#define FIFO_ERR_STS_FLD_rx_aes_rdm_ciph_fifo_ovrn             REG_FLD(1, 6)
#define FIFO_ERR_STS_FLD_rx_aes_ciph_txt_fifo_ovrn             REG_FLD(1, 5)
#define FIFO_ERR_STS_FLD_rx_aes_cryp_cnt_fifo_ovrn             REG_FLD(1, 4)
#define FIFO_ERR_STS_FLD_tx_align_fifo_udrn                    REG_FLD(1, 3)
#define FIFO_ERR_STS_FLD_tx_ploamu_fifo_ovrn                   REG_FLD(1, 2)
#define FIFO_ERR_STS_FLD_bst_fifo_ovrn                         REG_FLD(1, 1)
#define FIFO_ERR_STS_FLD_sgl_fifo_ovrn                         REG_FLD(1, 0)

#define TX_ERR_STS_FLD_tx_prof_invld_err                       REG_FLD(1, 2)
#define TX_ERR_STS_FLD_tx_late_start_err                       REG_FLD(1, 1)
#define TX_ERR_STS_FLD_tx_bst_sgl_diff_err                     REG_FLD(1, 0)

#define RX_ERR_STS_FLD_rx_xgem_hdr_hec_err                     REG_FLD(1, 13)
#define RX_ERR_STS_FLD_rx_alloc_hec_err                        REG_FLD(1, 12)
#define RX_ERR_STS_FLD_rx_mbi_xgem_drop_err                    REG_FLD(1, 11)
#define RX_ERR_STS_FLD_rx_omci_mic_err                         REG_FLD(1, 10)
#define RX_ERR_STS_FLD_rx_ploam_mic_err                        REG_FLD(1, 9)
#define RX_ERR_STS_FLD_rx_eth_crc_err                          REG_FLD(1, 8)
#define RX_ERR_STS_FLD_rx_aes_key_err                          REG_FLD(1, 7)
#define RX_ERR_STS_FLD_rx_gem_intlv_err                        REG_FLD(1, 6)
#define RX_ERR_STS_FLD_rx_los_gem_del_err                      REG_FLD(1, 5)
#define RX_ERR_STS_FLD_rx_bip_err                              REG_FLD(1, 4)
#define RX_ERR_STS_FLD_rx_hlend_hec_err                        REG_FLD(1, 3)
#define RX_ERR_STS_FLD_rx_pon_id_hec_err                       REG_FLD(1, 2)
#define RX_ERR_STS_FLD_rx_sfc_hec_uc_err                       REG_FLD(1, 1)
#define RX_ERR_STS_FLD_rx_eof_err                              REG_FLD(1, 0)

#if defined(TCSUPPORT_CPU_AN7583)
#define FIFO_ERR_ENABLE_FLD_tx_aes_rdm_ciph_fifo_ovrn_en       REG_FLD(1, 17)
#define FIFO_ERR_ENABLE_FLD_byte_mib_cmd_fifo_ovrn_en          REG_FLD(1, 16)
#define FIFO_ERR_ENABLE_FLD_frm_mib_cmd_fifo_ovrn_en           REG_FLD(1, 15)
#define FIFO_ERR_ENABLE_FLD_mib_cmd_fifo_ovrn_en               REG_FLD(1, 14)
#define FIFO_ERR_ENABLE_FLD_mib_tx_cmd_fifo_ovrn_en            REG_FLD(1, 13)
#define FIFO_ERR_ENABLE_FLD_snf_ds_fifo_ovrn_en                REG_FLD(1, 12)
#define FIFO_ERR_ENABLE_FLD_snf_us_fifo_ovrn_en                REG_FLD(1, 11)
#define FIFO_ERR_ENABLE_FLD_rx_ploamd_fifo_udrn_en             REG_FLD(1, 10)
#define FIFO_ERR_ENABLE_FLD_rx_ploamd_fifo_ovrn_en             REG_FLD(1, 9)
#define FIFO_ERR_ENABLE_FLD_rx_mbi_pl_fifo_ovrn_en             REG_FLD(1, 8)
#define FIFO_ERR_ENABLE_FLD_rx_mbi_hdr_fifo_ovrn_en            REG_FLD(1, 7)
#define FIFO_ERR_ENABLE_FLD_rx_aes_rdm_ciph_fifo_ovrn_en       REG_FLD(1, 6)
#define FIFO_ERR_ENABLE_FLD_rx_aes_ciph_txt_fifo_ovrn_en       REG_FLD(1, 5)
#define FIFO_ERR_ENABLE_FLD_rx_aes_cryp_cnt_fifo_ovrn_en       REG_FLD(1, 4)
#define FIFO_ERR_ENABLE_FLD_tx_align_fifo_udrn_en              REG_FLD(1, 3)
#define FIFO_ERR_ENABLE_FLD_tx_ploamu_fifo_ovrn_en             REG_FLD(1, 2)
#define FIFO_ERR_ENABLE_FLD_bst_fifo_ovrn_en                   REG_FLD(1, 1)
#define FIFO_ERR_ENABLE_FLD_sgl_fifo_ovrn_en                   REG_FLD(1, 0)

#define TX_ERR_ENABLE_FLD_tx_prof_invld_err_en                 REG_FLD(1, 2)
#define TX_ERR_ENABLE_FLD_tx_late_start_err_en                 REG_FLD(1, 1)
#define TX_ERR_ENABLE_FLD_tx_bst_sgl_diff_err_en               REG_FLD(1, 0)

#define RX_ERR_ENABLE_FLD_rx_xgem_hdr_hec_err_en               REG_FLD(1, 13)
#define RX_ERR_ENABLE_FLD_rx_alloc_hec_err_en                  REG_FLD(1, 12)
#define RX_ERR_ENABLE_FLD_rx_mbi_xgem_drop_err_en              REG_FLD(1, 11)
#define RX_ERR_ENABLE_FLD_rx_omci_mic_err_en                   REG_FLD(1, 10)
#define RX_ERR_ENABLE_FLD_rx_ploam_mic_err_en                  REG_FLD(1, 9)
#define RX_ERR_ENABLE_FLD_rx_eth_crc_err_en                    REG_FLD(1, 8)
#define RX_ERR_ENABLE_FLD_rx_aes_key_err_en                    REG_FLD(1, 7)
#define RX_ERR_ENABLE_FLD_rx_gem_intlv_err_en                  REG_FLD(1, 6)
#define RX_ERR_ENABLE_FLD_rx_los_gem_del_err_en                REG_FLD(1, 5)
#define RX_ERR_ENABLE_FLD_rx_bip_err_en                        REG_FLD(1, 4)
#define RX_ERR_ENABLE_FLD_rx_hlend_hec_err_en                  REG_FLD(1, 3)
#define RX_ERR_ENABLE_FLD_rx_pon_id_hec_err_en                 REG_FLD(1, 2)
#define RX_ERR_ENABLE_FLD_rx_sfc_hec_uc_err_en                 REG_FLD(1, 1)
#define RX_ERR_ENABLE_FLD_rx_eof_err_en                        REG_FLD(1, 0)
#endif

#define COR_TAG_FLD_cor_tag                                    REG_FLD(16, 0)

#define CUR_DS_PON_ID_FLD_cur_ds_pon_id                        REG_FLD(32, 0)

#define CUR_US_PON_ID_FLD_cur_us_pon_id                        REG_FLD(32, 0)

#define CALIB_STS_L32_FLD_calib_sts_l32                        REG_FLD(32, 0)

#define CALIB_STS_H32_FLD_calib_sts_h32                        REG_FLD(32, 0)

#define TUNING_GRAN_FLD_turning_gran                           REG_FLD(8, 0)

#define STEP_TUNING_TIME_FLD_step_turning_time                 REG_FLD(8, 0)

#define US_RATE_CAP_FLD_us_10g_cap                             REG_FLD(1, 1)
#define US_RATE_CAP_FLD_us_2p5g_cap                            REG_FLD(1, 0)

#define ATTENUATION_FLD_attenuation                            REG_FLD(8, 0)

#define POWER_LEVEL_CAP_FLD_power_level_cap                    REG_FLD(8, 0)

#define O23_O4_PLOAMU_CTRL_FLD_o23_o4_ploamu_ctrl              REG_FLD(1, 0)

#define ACTIVATION_ST_FLD_act_st                               REG_FLD(4, 0)

#define RSP_TIME_FLD_tresp                                     REG_FLD(14, 0)

#define RDM_DLY_FLD_max_rdm_dly                                REG_FLD(12, 16)
#define RDM_DLY_FLD_rdm_dly                                    REG_FLD(12, 0)

#define INT_DLY_FLD_phy_rx_dly                                 REG_FLD(16, 16)
#define INT_DLY_FLD_phy_tx_dly                                 REG_FLD(16, 0)

#define EQD_FLD_eqd                                            REG_FLD(32, 0)

#define SYN_MRG_FLD_syn_mrg                                    REG_FLD(10, 0)

#define US_PROF_VLD_FLD_us_prof3_vsn                           REG_FLD(4, 28)
#define US_PROF_VLD_FLD_us_prof3_vld                           REG_FLD(1, 24)
#define US_PROF_VLD_FLD_us_prof2_vsn                           REG_FLD(4, 20)
#define US_PROF_VLD_FLD_us_prof2_vld                           REG_FLD(1, 16)
#define US_PROF_VLD_FLD_us_prof1_vsn                           REG_FLD(4, 12)
#define US_PROF_VLD_FLD_us_prof1_vld                           REG_FLD(1, 8)
#define US_PROF_VLD_FLD_us_prof0_vsn                           REG_FLD(4, 4)
#define US_PROF_VLD_FLD_us_prof0_vld                           REG_FLD(1, 0)

#define US_PROF_PSBu_LEN_0_1_FLD_us_psbu_len_prof1             REG_FLD(16, 16)
#define US_PROF_PSBu_LEN_0_1_FLD_us_psbu_len_prof0             REG_FLD(16, 0)

#define US_PROF_PSBu_LEN_2_3_FLD_us_psbu_len_prof3             REG_FLD(16, 16)
#define US_PROF_PSBu_LEN_2_3_FLD_us_psbu_len_prof2             REG_FLD(16, 0)

#define US_AES_KEY_CTRL_FLD_us_aes_key_vld                     REG_FLD(1, 31)
#define US_AES_KEY_CTRL_FLD_us_aes_key_idx                     REG_FLD(1, 0)

#define DS_AES_KEY_VLD_FLD_ds_aes_bc_idx1_key_vld              REG_FLD(1, 3)
#define DS_AES_KEY_VLD_FLD_ds_aes_bc_idx0_key_vld              REG_FLD(1, 2)
#define DS_AES_KEY_VLD_FLD_ds_aes_uc_idx1_key_vld              REG_FLD(1, 1)
#define DS_AES_KEY_VLD_FLD_ds_aes_uc_idx0_key_vld              REG_FLD(1, 0)

#define DEFT_GPID_ENC_FLD_deft_gpid_enc                        REG_FLD(1, 0)

#define AES_UC_IDX0_KEY0_FLD_aes_uc_idx0_key0                  REG_FLD(32, 0)

#define AES_UC_IDX0_KEY1_FLD_aes_uc_idx0_key1                  REG_FLD(32, 0)

#define AES_UC_IDX0_KEY2_FLD_aes_uc_idx0_key2                  REG_FLD(32, 0)

#define AES_UC_IDX0_KEY3_FLD_aes_uc_idx0_key3                  REG_FLD(32, 0)

#define AES_UC_IDX1_KEY0_FLD_aes_uc_idx1_key0                  REG_FLD(32, 0)

#define AES_UC_IDX1_KEY1_FLD_aes_uc_idx1_key1                  REG_FLD(32, 0)

#define AES_UC_IDX1_KEY2_FLD_aes_uc_idx1_key2                  REG_FLD(32, 0)

#define AES_UC_IDX1_KEY3_FLD_aes_uc_idx1_key3                  REG_FLD(32, 0)

#define AES_BC_IDX0_KEY0_FLD_aes_bc_idx0_key0                  REG_FLD(32, 0)

#define AES_BC_IDX0_KEY1_FLD_aes_bc_idx0_key1                  REG_FLD(32, 0)

#define AES_BC_IDX0_KEY2_FLD_aes_bc_idx0_key2                  REG_FLD(32, 0)

#define AES_BC_IDX0_KEY3_FLD_aes_bc_idx0_key3                  REG_FLD(32, 0)

#define AES_BC_IDX1_KEY0_FLD_aes_bc_idx1_key0                  REG_FLD(32, 0)

#define AES_BC_IDX1_KEY1_FLD_aes_bc_idx1_key1                  REG_FLD(32, 0)

#define AES_BC_IDX1_KEY2_FLD_aes_bc_idx1_key2                  REG_FLD(32, 0)

#define AES_BC_IDX1_KEY3_FLD_aes_bc_idx1_key3                  REG_FLD(32, 0)

#define TCONT_ID_CFG_FLD_tcont_cmd                             REG_FLD(1, 31)
#define TCONT_ID_CFG_FLD_tcont_id_index                        REG_FLD(5, 20)
#define TCONT_ID_CFG_FLD_wr_tcont_id_vld                       REG_FLD(1, 16)
#define TCONT_ID_CFG_FLD_wr_tcont_id                           REG_FLD(14, 0)

#define TCONT_ID_STS_FLD_tcont_cmd_done                        REG_FLD(1, 31)
#define TCONT_ID_STS_FLD_rd_tcont_id_vld                       REG_FLD(1, 16)
#define TCONT_ID_STS_FLD_rd_tcont_id                           REG_FLD(14, 0)

#define GPIDX_TBL_INIT_FLD_gpidx_tbl_init_done                 REG_FLD(1, 8)
#define GPIDX_TBL_INIT_FLD_gpidx_tbl_init_start                REG_FLD(1, 0)

#define GPIDX_TBL_CTRL_FLD_gpidx_tbl_cmd                       REG_FLD(1, 31)
#define GPIDX_TBL_CTRL_FLD_gpidx_tbl_addr                      REG_FLD(11, 16)
#define GPIDX_TBL_CTRL_FLD_gpidx_tbl_wdata                     REG_FLD(9, 0)

#define GPIDX_TBL_STS_FLD_gpidx_cmd_done                       REG_FLD(1, 31)
#define GPIDX_TBL_STS_FLD_gpidx_tbl_rdata                      REG_FLD(9, 0)

#define GEM_TBL_INIT_FLD_gem_tbl_init_done                     REG_FLD(1, 8)
#define GEM_TBL_INIT_FLD_gem_tbl_init_start                    REG_FLD(1, 0)

#define GEM_PORT_CFG_FLD_gpid_cmd                              REG_FLD(1, 31)
#define GEM_PORT_CFG_FLD_gpid_vld                              REG_FLD(1, 18)
#define GEM_PORT_CFG_FLD_gpid_type                             REG_FLD(1, 17)
#define GEM_PORT_CFG_FLD_gpid_us_encrypt                       REG_FLD(1, 16)
#define GEM_PORT_CFG_FLD_gem_port_id                           REG_FLD(16, 0)

#define GEM_PORT_STS_FLD_gpid_cmd_done                         REG_FLD(1, 31)
#define GEM_PORT_STS_FLD_gpid_rd_sts                           REG_FLD(3, 0)

#define G_TX_FCS_TBL_INIT_FLD_tx_fcs_tbl_init_done             REG_FLD(1, 8)
#define G_TX_FCS_TBL_INIT_FLD_tx_fcs_tbl_init_start            REG_FLD(1, 0)

#if defined(TCSUPPORT_CPU_EN7581)
#define IDLE_GEM_CTRL_FLD_force_idle_hdr_data_same             REG_FLD(1, 31)
#endif
#define IDLE_GEM_CTRL_FLD_idle_gem_max_size                    REG_FLD(8, 16)
#define IDLE_GEM_CTRL_FLD_idle_gem_thld                        REG_FLD(16, 0)

#if defined(TCSUPPORT_CPU_EN7581)
#define US_DYING_GASP_CTRL_FLD_dying_gasp_send_num             REG_FLD(20, 12)
#else
#define US_DYING_GASP_CTRL_FLD_dying_gasp_send_num             REG_FLD(8, 24)
#define US_DYING_GASP_CTRL_FLD_sw_dying_gasp_trig              REG_FLD(1, 16)
#endif
#define US_DYING_GASP_CTRL_FLD_hw_dying_gasp_sts               REG_FLD(1, 8)
#if defined(TCSUPPORT_CPU_EN7581)
#define US_DYING_GASP_CTRL_FLD_sw_dying_gasp_trig              REG_FLD(1, 4)
#endif
#define US_DYING_GASP_CTRL_FLD_hw_dying_gasp_en                REG_FLD(1, 0)

#define TX_XGEM_IDLE_DATA_FLD_tx_xgem_idle_data                REG_FLD(32, 0)

#define TX_OMCI_PRE_GET_FLD_tx_limit_get_omci_size             REG_FLD(16, 16)
#define TX_OMCI_PRE_GET_FLD_tx_limit_get_omci_en               REG_FLD(1, 8)
#define TX_OMCI_PRE_GET_FLD_tx_pre_get_omci_en                 REG_FLD(1, 0)

#define RX_OMCI_PRE_GET_FLD_rx_omci_intr_eth_en                REG_FLD(1, 0)

#define EPDROP_EN_FLD_errploamd_drop_en                        REG_FLD(1, 0)

#define PLOAMu_FIFO_STS_FLD_ploamu_fifo_ovrn                   REG_FLD(1, 31)
#define PLOAMu_FIFO_STS_FLD_ploamu_fifo_min_avail              REG_FLD(8, 16)
#define PLOAMu_FIFO_STS_FLD_ploamu_fifo_avail                  REG_FLD(8, 0)

#define PLOAMu_WDATA_FLD_ploamu_wdata                          REG_FLD(32, 0)

#define PLOAMd_FIFO_STS_FLD_ploamd_fifo_ovrn                   REG_FLD(1, 31)
#define PLOAMd_FIFO_STS_FLD_ploamd_fifo_max_used               REG_FLD(8, 16)
#define PLOAMd_FIFO_STS_FLD_ploamd_fifo_used                   REG_FLD(8, 0)

#define PLOAMd_RDATA_FLD_ploamd_rdata                          REG_FLD(32, 0)

#define KEY_GEN_FLD_kek_start                                  REG_FLD(1, 4)
#define KEY_GEN_FLD_ploam_ik_start                             REG_FLD(1, 3)
#define KEY_GEN_FLD_omci_ik_start                              REG_FLD(1, 2)
#define KEY_GEN_FLD_sk_start                                   REG_FLD(1, 1)
#define KEY_GEN_FLD_regmsk_start                               REG_FLD(1, 0)

#define CUR_KIDX_FLD_cur_oik_idx                               REG_FLD(1, 16)
#define CUR_KIDX_FLD_cur_pik_idx                               REG_FLD(1, 0)

#define MSK_0_FLD_msk0                                         REG_FLD(32, 0)

#define MSK_1_FLD_msk1                                         REG_FLD(32, 0)

#define MSK_2_FLD_msk2                                         REG_FLD(32, 0)

#define MSK_3_FLD_msk3                                         REG_FLD(32, 0)

#define REGMSK_0_FLD_regmsk0                                   REG_FLD(32, 0)

#define REGMSK_1_FLD_regmsk1                                   REG_FLD(32, 0)

#define REGMSK_2_FLD_regmsk2                                   REG_FLD(32, 0)

#define REGMSK_3_FLD_regmsk3                                   REG_FLD(32, 0)

#define SK_0_FLD_sk0                                           REG_FLD(32, 0)

#define SK_1_FLD_sk1                                           REG_FLD(32, 0)

#define SK_2_FLD_sk2                                           REG_FLD(32, 0)

#define SK_3_FLD_sk3                                           REG_FLD(32, 0)

#define HW_GENK_0_FLD_hw_genk_0                                REG_FLD(32, 0)

#define HW_GENK_1_FLD_hw_genk_1                                REG_FLD(32, 0)

#define HW_GENK_2_FLD_hw_genk_2                                REG_FLD(32, 0)

#define HW_GENK_3_FLD_hw_genk_3                                REG_FLD(32, 0)

#define PIK0_0_FLD_pik0_0                                      REG_FLD(32, 0)

#define PIK0_1_FLD_pik0_1                                      REG_FLD(32, 0)

#define PIK0_2_FLD_pik0_2                                      REG_FLD(32, 0)

#define PIK0_3_FLD_pik0_3                                      REG_FLD(32, 0)

#define PIK1_0_FLD_pik1_0                                      REG_FLD(32, 0)

#define PIK1_1_FLD_pik1_1                                      REG_FLD(32, 0)

#define PIK1_2_FLD_pik1_2                                      REG_FLD(32, 0)

#define PIK1_3_FLD_pik1_3                                      REG_FLD(32, 0)

#define OIK0_0_FLD_oik0_0                                      REG_FLD(32, 0)

#define OIK0_1_FLD_oik0_1                                      REG_FLD(32, 0)

#define OIK0_2_FLD_oik0_2                                      REG_FLD(32, 0)

#define OIK0_3_FLD_oik0_3                                      REG_FLD(32, 0)

#define OIK1_0_FLD_oik1_0                                      REG_FLD(32, 0)

#define OIK1_1_FLD_oik1_1                                      REG_FLD(32, 0)

#define OIK1_2_FLD_oik1_2                                      REG_FLD(32, 0)

#define OIK1_3_FLD_oik1_3                                      REG_FLD(32, 0)

#define KEK0_0_FLD_kek0_0                                      REG_FLD(32, 0)

#define KEK0_1_FLD_kek0_1                                      REG_FLD(32, 0)

#define KEK0_2_FLD_kek0_2                                      REG_FLD(32, 0)

#define KEK0_3_FLD_kek0_3                                      REG_FLD(32, 0)

#define KEK1_0_FLD_kek1_0                                      REG_FLD(32, 0)

#define KEK1_1_FLD_kek1_1                                      REG_FLD(32, 0)

#define KEK1_2_FLD_kek1_2                                      REG_FLD(32, 0)

#define KEK1_3_FLD_kek1_3                                      REG_FLD(32, 0)

#define PON_TAG_0_FLD_pon_tag_0                                REG_FLD(32, 0)

#define PON_TAG_1_FLD_pon_tag_1                                REG_FLD(32, 0)

#define SW_SET_KIDX_FLD_sw_set_oik_en                          REG_FLD(1, 24)
#define SW_SET_KIDX_FLD_sw_set_pik_en                          REG_FLD(1, 16)
#define SW_SET_KIDX_FLD_sw_set_oik_idx                         REG_FLD(1, 8)
#define SW_SET_KIDX_FLD_sw_set_pik_idx                         REG_FLD(1, 0)

#define SW0_ENCSTART_FLD_sw0_encstart                          REG_FLD(1, 0)

#define SW0_MADDR_FLD_sw0_mdtaddr                              REG_FLD(32, 0)

#define SW0_RADDR_FLD_sw0_rdtaddr                              REG_FLD(32, 0)

#define SW0_KADDR_FLD_sw0_kaddr                                REG_FLD(32, 0)

#define SW0_ENCLEN_FLD_sw0_rdtlen                              REG_FLD(16, 16)
#define SW0_ENCLEN_FLD_sw0_mdtlen                              REG_FLD(16, 0)

#define SW0_ENCINFO_FLD_sw0_enckidx                            REG_FLD(3, 16)
#define SW0_ENCINFO_FLD_sw0_encdic                             REG_FLD(2, 0)

#define SW1_ENCSTART_FLD_sw1_encstart                          REG_FLD(1, 0)

#define SW1_MADDR_FLD_sw1_mdtaddr                              REG_FLD(32, 0)

#define SW1_RADDR_FLD_sw1_rdtaddr                              REG_FLD(32, 0)

#define SW1_KADDR_FLD_sw1_kaddr                                REG_FLD(32, 0)

#define SW1_ENCLEN_FLD_sw1_rdtlen                              REG_FLD(16, 16)
#define SW1_ENCLEN_FLD_sw1_mdtlen                              REG_FLD(16, 0)

#define SW1_ENCINFO_FLD_sw1_enckidx                            REG_FLD(3, 16)
#define SW1_ENCINFO_FLD_sw1_encdic                             REG_FLD(2, 0)

#if defined(TCSUPPORT_CPU_AN7583)
#define TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont_done                REG_FLD(1, 11)
#define TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont_en                  REG_FLD(1, 8)
#define TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont                     REG_FLD(5, 0)
#endif

#define MIB_CTRL_FLD_mib_frame_type                            REG_FLD(1, 8)
#define MIB_CTRL_FLD_mib_en                                    REG_FLD(1, 0)

#define MIB_TBL_CONFIG_FLD_mib_tbl_rd_clr                      REG_FLD(1, 16)
#define MIB_TBL_CONFIG_FLD_mib_tbl_init_done                   REG_FLD(1, 8)
#define MIB_TBL_CONFIG_FLD_mib_tbl_init_start                  REG_FLD(1, 0)

#define MIB_CTRL_STS_FLD_mib_cmd_done                          REG_FLD(1, 31)
#define MIB_CTRL_STS_FLD_mib_wr                                REG_FLD(1, 20)
#define MIB_CTRL_STS_FLD_mib_type                              REG_FLD(2, 16)
#define MIB_CTRL_STS_FLD_mib_gpid                              REG_FLD(16, 0)

#define MIB_RDATA_L32_FLD_mib_rdata_l32                        REG_FLD(32, 0)

#define MIB_RDATA_H32_FLD_mib_rdata_h32                        REG_FLD(32, 0)

#define MIB_WDATA_L32_FLD_mib_wdata_l32                        REG_FLD(32, 0)

#define MIB_WDATA_H32_FLD_mib_wdata_h32                        REG_FLD(32, 0)

#define TOD_CLK_PERIOD_FLD_tod_period                          REG_FLD(8, 0)

#define TOD_1PPS_WD_CTRL_FLD_tod_1pps_width_ctrl               REG_FLD(32, 0)

#define CUR_TOD_SEC_FLD_cur_tod_sec                            REG_FLD(32, 0)

#define CUR_TOD_NANO_SEC_FLD_cur_tod_nano_sec                  REG_FLD(32, 0)

#define TOD_SFC_L_FLD_tod_spf_cnt_l                            REG_FLD(32, 0)

#define TOD_SFC_H_FLD_tod_spf_cnt_h                            REG_FLD(19, 0)

#define TOD_UPD_CTRL_FLD_tod_upd_en                            REG_FLD(1, 0)

#define NEW_TOD_SEC_FLD_new_tod_sec                            REG_FLD(32, 0)

#define NEW_TOD_NANO_SEC_FLD_new_tod_nano_sec                  REG_FLD(32, 0)

#define SLEEP_CFG_FLD_reload_sleep_cnt                         REG_FLD(1, 0)

#define SLEEP_CNT_FLD_sleep_cnt                                REG_FLD(32, 0)

#define TURNING_SFC_L_FLD_turning_spf_cnt_l                    REG_FLD(32, 0)

#define TURNING_SFC_H_FLD_turning_spf_cnt_h                    REG_FLD(19, 0)

#define DBG_CAP_SETTING_FLD_hw_ack_ploamu_code                 REG_FLD(8, 24)
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_FLD_force_o2349_zero_gsize_no_dbru     REG_FLD(1, 23)
#define DBG_CAP_SETTING_FLD_ds_bip_chk_en                      REG_FLD(1, 15)
#define DBG_CAP_SETTING_FLD_ds_4b_trailer_en                   REG_FLD(1, 14)
#define DBG_CAP_SETTING_FLD_tx_deft_gpid_enc_sts_ctrl_by_rx    REG_FLD(1, 13)
#define DBG_CAP_SETTING_FLD_flt_key_invld_gem                  REG_FLD(1, 12)
#define DBG_CAP_SETTING_FLD_flt_o2349_continue_gnt             REG_FLD(1, 11)
#define DBG_CAP_SETTING_FLD_flt_o2349_no_ploamu_gnt            REG_FLD(1, 10)
#endif
#define DBG_CAP_SETTING_FLD_us_aes_seq_num_14b                 REG_FLD(1, 9)
#define DBG_CAP_SETTING_FLD_o52_idle_only_en                   REG_FLD(1, 8)
#define DBG_CAP_SETTING_FLD_tx_send_only_in_o23459             REG_FLD(1, 7)
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_FLD_flt_o49_no_deft_tcont_gnt          REG_FLD(1, 6)
#else
#define DBG_CAP_SETTING_FLD_flt_dbru_set_gnt_o2349             REG_FLD(1, 6)
#endif
#define DBG_CAP_SETTING_FLD_send_ranging_ack_o4                REG_FLD(1, 5)
#define DBG_CAP_SETTING_FLD_hw_cal_ds_omci_mic                 REG_FLD(1, 4)
#define DBG_CAP_SETTING_FLD_hw_cal_us_omci_mic                 REG_FLD(1, 3)
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_FLD_flt_o2349_no_ploamu_only_gnt       REG_FLD(1, 2)
#else
#define DBG_CAP_SETTING_FLD_flt_no_zero_gsize_gnt_o2349        REG_FLD(1, 2)
#endif
#define DBG_CAP_SETTING_FLD_rept_msg_flt                       REG_FLD(1, 1)
#define DBG_CAP_SETTING_FLD_rept_prof_flt                      REG_FLD(1, 0)

#define DBG_BWM_CHK_CTRL_FLD_min_bst_intvl                     REG_FLD(16, 16)
#define DBG_BWM_CHK_CTRL_FLD_max_bst_len_err_cut               REG_FLD(1, 12)
#define DBG_BWM_CHK_CTRL_FLD_max_my_bst_num_err_cut            REG_FLD(1, 11)
#define DBG_BWM_CHK_CTRL_FLD_max_my_alloc_num_err_cut          REG_FLD(1, 10)
#define DBG_BWM_CHK_CTRL_FLD_max_bst_alloc_num_err_cut         REG_FLD(1, 9)
#define DBG_BWM_CHK_CTRL_FLD_max_bwm_alloc_num_err_cut         REG_FLD(1, 8)
#define DBG_BWM_CHK_CTRL_FLD_min_gsize_flt                     REG_FLD(1, 4)
#define DBG_BWM_CHK_CTRL_FLD_max_gsize_flt                     REG_FLD(1, 3)
#define DBG_BWM_CHK_CTRL_FLD_start_time_order_flt              REG_FLD(1, 2)
#define DBG_BWM_CHK_CTRL_FLD_max_start_time_flt                REG_FLD(1, 1)
#define DBG_BWM_CHK_CTRL_FLD_min_bst_intvl_flt                 REG_FLD(1, 0)

#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_BWM_CKH_STS_FLD_o2349_continue_gnt_recv            REG_FLD(1, 17)
#define DBG_BWM_CKH_STS_FLD_o49_no_deft_tcont_gnt_recv         REG_FLD(1, 16)
#define DBG_BWM_CKH_STS_FLD_o2349_no_ploamu_only_gnt_recv      REG_FLD(1, 15)
#define DBG_BWM_CKH_STS_FLD_o2349_no_ploamu_gnt_recv           REG_FLD(1, 14)
#else
#define DBG_BWM_CKH_STS_FLD_o23_o4_data_gnt_recv               REG_FLD(1, 14)
#endif
#define DBG_BWM_CKH_STS_FLD_other_tid_ins_my_bst_err           REG_FLD(1, 13)
#define DBG_BWM_CKH_STS_FLD_max_bst_len_err                    REG_FLD(1, 12)
#define DBG_BWM_CKH_STS_FLD_max_my_bst_num_err                 REG_FLD(1, 11)
#define DBG_BWM_CKH_STS_FLD_max_my_alloc_num_err               REG_FLD(1, 10)
#define DBG_BWM_CKH_STS_FLD_max_bst_alloc_num_err              REG_FLD(1, 9)
#define DBG_BWM_CKH_STS_FLD_max_bwm_alloc_num_err              REG_FLD(1, 8)
#define DBG_BWM_CKH_STS_FLD_alloc_hec_uc_err                   REG_FLD(1, 7)
#define DBG_BWM_CKH_STS_FLD_bst_split_err                      REG_FLD(1, 6)
#define DBG_BWM_CKH_STS_FLD_my_tid_ins_bst_err                 REG_FLD(1, 5)
#define DBG_BWM_CKH_STS_FLD_min_gsize_err                      REG_FLD(1, 4)
#define DBG_BWM_CKH_STS_FLD_max_gsize_err                      REG_FLD(1, 3)
#define DBG_BWM_CKH_STS_FLD_start_time_order_err               REG_FLD(1, 2)
#define DBG_BWM_CKH_STS_FLD_max_start_time_err                 REG_FLD(1, 1)
#define DBG_BWM_CKH_STS_FLD_min_bst_intvl_err                  REG_FLD(1, 0)

#define DBG_BWM_SFIFO_STS_FLD_sgl_fifo_max_used                REG_FLD(10, 16)
#define DBG_BWM_SFIFO_STS_FLD_sgl_fifo_used                    REG_FLD(10, 0)

#define DBG_BWM_BFIFO_STS_FLD_bst_fifo_max_used                REG_FLD(6, 16)
#define DBG_BWM_BFIFO_STS_FLD_bst_fifo_used                    REG_FLD(6, 0)

#define DBG_TX_ALIGN_FIFO_STS_FLD_tx_align_fifo_max_used       REG_FLD(16, 16)
#define DBG_TX_ALIGN_FIFO_STS_FLD_tx_align_fifo_used           REG_FLD(16, 0)

#define DBG_TX_AES_MAX_USED_FLD_tx_aes_eng_max_used            REG_FLD(4, 24)
#define DBG_TX_AES_MAX_USED_FLD_tx_rdm_ciph_fifo_max_used      REG_FLD(4, 20)
#define DBG_TX_AES_MAX_USED_FLD_tx_cryp_cnt_fifo_max_used      REG_FLD(5, 12)
#define DBG_TX_AES_MAX_USED_FLD_tx_ciph_txt_fifo_max_used      REG_FLD(5, 4)
#define DBG_TX_AES_MAX_USED_FLD_tx_aes_hdr_fifo_max_used       REG_FLD(4, 0)

#if defined(TCSUPPORT_CPU_AN7583)
#define DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1_mode      REG_FLD(2, 18)
#define DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1_ovrw      REG_FLD(1, 17)
#define DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1           REG_FLD(5, 12)
#define DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0_mode      REG_FLD(2, 10)
#define DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0_ovrw      REG_FLD(1, 9)
#define DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0           REG_FLD(5, 4)
#endif

#define DBG_DBA_BACK_DOOR_FLD_dba_modify_mode                  REG_FLD(2, 0)

#define DBG_DBA_MODIFY_FLD_dba_modify                          REG_FLD(32, 0)

#define DBG_RESYNC_FLD_tx_sync_rdy                             REG_FLD(1, 31)
#define DBG_RESYNC_FLD_sw_resync_en                            REG_FLD(1, 8)
#define DBG_RESYNC_FLD_sw_resync_start                         REG_FLD(1, 0)

#define DBG_PHY_DLY_CTRL_FLD_use_mpi_rx_phy_dly_ofs            REG_FLD(1, 8)
#define DBG_PHY_DLY_CTRL_FLD_o4_o5_same_phy_dly                REG_FLD(1, 0)

#define DBG_DS_SPF_CNT_L_FLD_ds_spf_cnt_l32                    REG_FLD(32, 0)

#define DBG_DS_SPF_CNT_H_FLD_ds_spf_cnt_h19                    REG_FLD(19, 0)

#define DBG_PON_ID_L_FLD_ds_pon_id_l32                         REG_FLD(32, 0)

#define DBG_PON_ID_H_FLD_ds_pon_id_h19                         REG_FLD(19, 0)

#define DBG_TX_SYNC_OFFSET_FLD_tx_sync_offset                  REG_FLD(32, 0)

#define DBG_RX_PHY_DLY_OFS_FLD_rx_phy_dly_offset               REG_FLD(16, 0)

#define DBG_RX_HEC_ERR_FLD_rx_hec_err_sts                      REG_FLD(1, 11)
#define DBG_RX_HEC_ERR_FLD_rx_hdr_hec_3err                     REG_FLD(1, 10)
#define DBG_RX_HEC_ERR_FLD_rx_hdr_hec_2err                     REG_FLD(1, 9)
#define DBG_RX_HEC_ERR_FLD_rx_hdr_hec_1err                     REG_FLD(1, 8)
#define DBG_RX_HEC_ERR_FLD_rx_alloc_hec_3err                   REG_FLD(1, 6)
#define DBG_RX_HEC_ERR_FLD_rx_alloc_hec_2err                   REG_FLD(1, 5)
#define DBG_RX_HEC_ERR_FLD_rx_alloc_hec_1err                   REG_FLD(1, 4)
#define DBG_RX_HEC_ERR_FLD_rx_hlend_hec_3err                   REG_FLD(1, 2)
#define DBG_RX_HEC_ERR_FLD_rx_hlend_hec_2err                   REG_FLD(1, 1)
#define DBG_RX_HEC_ERR_FLD_rx_hlend_hec_1err                   REG_FLD(1, 0)

#define RX_GEM_ENC_CHK_FLD_rx_enc_chk_err_sts                  REG_FLD(1, 31)
#define RX_GEM_ENC_CHK_FLD_rx_enc_chk_done                     REG_FLD(1, 28)
#define RX_GEM_ENC_CHK_FLD_rx_enc_chk_en                       REG_FLD(1, 20)
#define RX_GEM_ENC_CHK_FLD_exp_rx_enc_sts                      REG_FLD(1, 16)
#define RX_GEM_ENC_CHK_FLD_rx_enc_chk_gpid                     REG_FLD(16, 0)

#define RX_MBI_FIFO_DROP_EN_FLD_rx_mbi_fifo_drop_en            REG_FLD(1, 0)

#define RX_MBI_HDR_FIFO_DROP_CTRL_FLD_rx_mbi_hdr_fifo_hth      REG_FLD(16, 16)
#define RX_MBI_HDR_FIFO_DROP_CTRL_FLD_rx_mbi_hdr_fifo_lth      REG_FLD(16, 0)

#define RX_MBI_PL_FIFO_DROP_CTRL_FLD_rx_mbi_pl_fifo_hth        REG_FLD(16, 16)
#define RX_MBI_PL_FIFO_DROP_CTRL_FLD_rx_mbi_pl_fifo_lth        REG_FLD(16, 0)

#if defined(TCSUPPORT_CPU_EN7581)
#define GET_RX_IDLE_XGEM_FLD_get_rx_idle_xgem                  REG_FLD(1, 31)
#define GET_RX_IDLE_XGEM_FLD_rx_idle_xgem_len                  REG_FLD(31, 0)

#define RX_IDLE_XGEM_1ST_PL_FLD_rx_idle_xgem_1st_pl            REG_FLD(32, 0)

#endif

#if defined(TCSUPPORT_CPU_AN7583)
#define DBG_DBA_MODIFY_IDX_0_FLD_dba_modify_idx_0              REG_FLD(32, 0)

#define DBG_DBA_MODIFY_IDX_1_FLD_dba_modify_idx_1              REG_FLD(32, 0)
#endif

#define CNT_CLR_FLD_nml_cnt_clr                                REG_FLD(1, 8)
#define CNT_CLR_FLD_err_cnt_clr                                REG_FLD(1, 0)

#define RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_3err_cnt             REG_FLD(8, 16)
#define RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_2err_cnt             REG_FLD(8, 8)
#define RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_1err_cnt             REG_FLD(8, 0)

#define RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_3err_cnt             REG_FLD(8, 16)
#define RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_2err_cnt             REG_FLD(8, 8)
#define RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_1err_cnt             REG_FLD(8, 0)

#define RX_HDR_HEC_CNT_FLD_rx_hdr_hec_3err_cnt                 REG_FLD(8, 16)
#define RX_HDR_HEC_CNT_FLD_rx_hdr_hec_2err_cnt                 REG_FLD(8, 8)
#define RX_HDR_HEC_CNT_FLD_rx_hdr_hec_1err_cnt                 REG_FLD(8, 0)

#define RX_PHY_HEC_ERR_CNT_FLD_rx_pon_id_hec_err_cnt           REG_FLD(16, 16)
#define RX_PHY_HEC_ERR_CNT_FLD_rx_sfc_hec_err_cnt              REG_FLD(16, 0)

#define RX_MIC_ERR_CNT_FLD_rx_omci_mic_err_cnt                 REG_FLD(16, 16)
#define RX_MIC_ERR_CNT_FLD_rx_ploam_mic_err_cnt                REG_FLD(16, 0)

#define RX_ETH_FCS_ERR_CNT_FLD_rx_eth_crc_err_cnt              REG_FLD(32, 0)

#define RX_BIP_ERR_CNT_FLD_rx_bip_err_cnt                      REG_FLD(32, 0)

#define RX_KEY_ERR_CNT_FLD_rx_key_err_cnt                      REG_FLD(32, 0)

#define RX_LOST_WCNT_FLD_rx_lost_wcnt                          REG_FLD(32, 0)

#define INVLD_PROF_BST_GNT_CNT_FLD_invld_prof_bst_gnt_cnt      REG_FLD(32, 0)

#define RX_MBI_XGEM_DROP_CNT_FLD_rx_mbi_xgem_drop_cnt          REG_FLD(32, 0)

#define RX_XGTC_CNT_FLD_rx_mpi_sof_cnt                         REG_FLD(32, 0)

#define TX_BST_CNT_FLD_tx_mpi_sof_cnt                          REG_FLD(32, 0)

#define RX_PLOAMD_CNT_FLD_rx_ploamd_cnt                        REG_FLD(32, 0)

#define TX_PLOAMU_CNT_FLD_tx_ploamu_cnt                        REG_FLD(32, 0)

#define RX_OMCI_CNT_FLD_rx_omci_cnt_fe                         REG_FLD(16, 16)
#define RX_OMCI_CNT_FLD_rx_omci_cnt_mac                        REG_FLD(16, 0)

#define TX_OMCI_CNT_FLD_tx_omci_cnt_fe                         REG_FLD(16, 16)
#define TX_OMCI_CNT_FLD_tx_omci_cnt_mac                        REG_FLD(16, 0)

#define RX_XGEM_CNT_FLD_rx_xgem_cnt                            REG_FLD(32, 0)

#define TX_XGEM_CNT_FLD_tx_xgem_cnt                            REG_FLD(32, 0)

#define RX_MBI_CNT_FLD_rx_mbi_ack_cnt                          REG_FLD(32, 0)

#define TX_MBI_CNT_FLD_tx_mbi_ack_cnt                          REG_FLD(32, 0)

#define RX_NON_IDLE_BCNT_FLD_rx_non_idle_bcnt                  REG_FLD(32, 0)

#define TX_NON_IDLE_BCNT_FLD_tx_non_idle_bcnt                  REG_FLD(32, 0)

#define TX_NLF_XGEM_CNT_FLD_tx_nlf_xgem_cnt                    REG_FLD(32, 0)

#define TX_ACK_PLOAMU_CNT_FLD_tx_ack_ploamu_cnt                REG_FLD(32, 0)

#define RX_BIP_PROTECT_WCNT_FLD_rx_bip_protect_wcnt            REG_FLD(32, 0)

#define TX_IDLE_BCNT_FLD_tx_idle_bcnt                          REG_FLD(32, 0)

#if defined(TCSUPPORT_CPU_AN7583)
#define HUNT_AFTER_LOSGD_FLD_hunt_after_losgd_en               REG_FLD(1, 0)

#define BWMAP_LEN_LIMIT_FLD_max_alloc_num                      REG_FLD(11, 20)
#define BWMAP_LEN_LIMIT_FLD_max_my_alloc_num                   REG_FLD(8, 12)
#define BWMAP_LEN_LIMIT_FLD_max_bst_alloc_num                  REG_FLD(5, 7)
#define BWMAP_LEN_LIMIT_FLD_max_my_bst_num                     REG_FLD(7, 0)

#define XGEM_HEADER_OVRW_FLD_rx_gem_3port_interleave           REG_FLD(1, 29)
#define XGEM_HEADER_OVRW_FLD_rx_omci_interleave                REG_FLD(1, 28)
#define XGEM_HEADER_OVRW_FLD_nml_header_ovrw_en                REG_FLD(1, 1)
#define XGEM_HEADER_OVRW_FLD_omci_header_ovrw_en               REG_FLD(1, 0)

#define OMCI_LEN_CTRL_FLD_max_omci_len                         REG_FLD(14, 0)
#endif

#define SNF_CTRL_FLD_snf_us_en                                 REG_FLD(1, 9)
#define SNF_CTRL_FLD_snf_ds_en                                 REG_FLD(1, 8)
#if defined(TCSUPPORT_CPU_AN7583)
#define SNF_CTRL_FLD_snf_us_ovrw_8b_xgem                       REG_FLD(1, 1)
#endif
#define SNF_CTRL_FLD_snf_us_ovrw_my_xgem                       REG_FLD(1, 0)

#define SNF_GPID_FLD_us_snf_gpid                               REG_FLD(16, 16)
#define SNF_GPID_FLD_ds_snf_gpid                               REG_FLD(16, 0)

#define DS_SNF_ETH_DASA_H16_FLD_ds_snf_eth_da_h16              REG_FLD(16, 16)
#define DS_SNF_ETH_DASA_H16_FLD_ds_snf_eth_sa_h16              REG_FLD(16, 0)

#define SNF_ETH_TAG_FLD_snf_eth_tpid                           REG_FLD(16, 16)

#define SNF_ETH_TYPE_FLD_snf_eth_type                          REG_FLD(16, 0)

#define US_SNF_ETH_DASA_H16_FLD_us_snf_eth_da_h16              REG_FLD(16, 16)
#define US_SNF_ETH_DASA_H16_FLD_us_snf_eth_sa_h16              REG_FLD(16, 0)

#define DBG_PROBE_CTRL_FLD_probe_clk_en                        REG_FLD(1, 31)
#define DBG_PROBE_CTRL_FLD_probe_trig_en                       REG_FLD(1, 30)
#define DBG_PROBE_CTRL_FLD_probe_trig_bit_sel                  REG_FLD(6, 16)
#define DBG_PROBE_CTRL_FLD_probe_trig_sel                      REG_FLD(6, 8)
#define DBG_PROBE_CTRL_FLD_probe_sel                           REG_FLD(5, 0)

#define DBG_PROBE_HIGH32_FLD_probe_h32                         REG_FLD(32, 0)

#define DBG_PROBE_LOW32_FLD_probe_l32                          REG_FLD(32, 0)

#define SW_RST_GET_xgpon_mac_sw_rst_n(reg32)                   REG_FLD_GET(SW_RST_FLD_xgpon_mac_sw_rst_n, (reg32))

#define MBI_MPI_STOP_GET_mpi_tx_stop_done(reg32)               REG_FLD_GET(MBI_MPI_STOP_FLD_mpi_tx_stop_done, (reg32))
#define MBI_MPI_STOP_GET_mpi_rx_stop_done(reg32)               REG_FLD_GET(MBI_MPI_STOP_FLD_mpi_rx_stop_done, (reg32))
#define MBI_MPI_STOP_GET_mpi_tx_stop(reg32)                    REG_FLD_GET(MBI_MPI_STOP_FLD_mpi_tx_stop, (reg32))
#define MBI_MPI_STOP_GET_mpi_rx_stop(reg32)                    REG_FLD_GET(MBI_MPI_STOP_FLD_mpi_rx_stop, (reg32))
#define MBI_MPI_STOP_GET_mbi_tx_stop_done(reg32)               REG_FLD_GET(MBI_MPI_STOP_FLD_mbi_tx_stop_done, (reg32))
#define MBI_MPI_STOP_GET_mbi_rx_stop_done(reg32)               REG_FLD_GET(MBI_MPI_STOP_FLD_mbi_rx_stop_done, (reg32))
#if defined(TCSUPPORT_CPU_EN7581)
#define MBI_MPI_STOP_GET_dbru_stop_done(reg32)                 REG_FLD_GET(MBI_MPI_STOP_FLD_dbru_stop_done, (reg32))
#define MBI_MPI_STOP_GET_dbru_stop(reg32)                      REG_FLD_GET(MBI_MPI_STOP_FLD_dbru_stop, (reg32))
#endif
#define MBI_MPI_STOP_GET_mbi_tx_stop(reg32)                    REG_FLD_GET(MBI_MPI_STOP_FLD_mbi_tx_stop, (reg32))
#if defined(TCSUPPORT_CPU_EN7581)
#define MBI_MPI_STOP_GET_del_rx_stop(reg32)                    REG_FLD_GET(MBI_MPI_STOP_FLD_del_rx_stop, (reg32))
#endif
#define MBI_MPI_STOP_GET_mbi_rx_stop(reg32)                    REG_FLD_GET(MBI_MPI_STOP_FLD_mbi_rx_stop, (reg32))

#define VENDOR_ID_GET_vendor_id(reg32)                         REG_FLD_GET(VENDOR_ID_FLD_vendor_id, (reg32))

#define VS_SN_GET_vs_sn(reg32)                                 REG_FLD_GET(VS_SN_FLD_vs_sn, (reg32))

#define ONU_ID_GET_onu_id_vld(reg32)                           REG_FLD_GET(ONU_ID_FLD_onu_id_vld, (reg32))
#define ONU_ID_GET_onu_id(reg32)                               REG_FLD_GET(ONU_ID_FLD_onu_id, (reg32))

#define RGS_ID3_0_GET_rgs_id3_0(reg32)                         REG_FLD_GET(RGS_ID3_0_FLD_rgs_id3_0, (reg32))

#define RGS_ID7_4_GET_rgs_id7_4(reg32)                         REG_FLD_GET(RGS_ID7_4_FLD_rgs_id7_4, (reg32))

#define RGS_ID11_8_GET_rgs_id11_8(reg32)                       REG_FLD_GET(RGS_ID11_8_FLD_rgs_id11_8, (reg32))

#define RGS_ID15_12_GET_rgs_id15_12(reg32)                     REG_FLD_GET(RGS_ID15_12_FLD_rgs_id15_12, (reg32))

#define RGS_ID19_16_GET_rgs_id19_16(reg32)                     REG_FLD_GET(RGS_ID19_16_FLD_rgs_id19_16, (reg32))

#define RGS_ID23_20_GET_rgs_id23_20(reg32)                     REG_FLD_GET(RGS_ID23_20_FLD_rgs_id23_20, (reg32))

#define RGS_ID27_24_GET_rgs_id27_24(reg32)                     REG_FLD_GET(RGS_ID27_24_FLD_rgs_id27_24, (reg32))

#define RGS_ID31_28_GET_rgs_id31_28(reg32)                     REG_FLD_GET(RGS_ID31_28_FLD_rgs_id31_28, (reg32))

#define RGS_ID35_32_GET_rgs_id35_32(reg32)                     REG_FLD_GET(RGS_ID35_32_FLD_rgs_id35_32, (reg32))

#define INT_ENABLE_GET_o5_sn_onu_req_recv_int_en(reg32)        REG_FLD_GET(INT_ENABLE_FLD_o5_sn_onu_req_recv_int_en, (reg32))
#define INT_ENABLE_GET_turning_sfc_match_int_en(reg32)         REG_FLD_GET(INT_ENABLE_FLD_turning_sfc_match_int_en, (reg32))
#define INT_ENABLE_GET_o9_gnt_recv_int_en(reg32)               REG_FLD_GET(INT_ENABLE_FLD_o9_gnt_recv_int_en, (reg32))
#define INT_ENABLE_GET_sw1_mic_done_int_en(reg32)              REG_FLD_GET(INT_ENABLE_FLD_sw1_mic_done_int_en, (reg32))
#define INT_ENABLE_GET_sw0_mic_done_int_en(reg32)              REG_FLD_GET(INT_ENABLE_FLD_sw0_mic_done_int_en, (reg32))
#define INT_ENABLE_GET_key_cal_done_int_en(reg32)              REG_FLD_GET(INT_ENABLE_FLD_key_cal_done_int_en, (reg32))
#define INT_ENABLE_GET_lwi_int_en(reg32)                       REG_FLD_GET(INT_ENABLE_FLD_lwi_int_en, (reg32))
#define INT_ENABLE_GET_fwi_int_en(reg32)                       REG_FLD_GET(INT_ENABLE_FLD_fwi_int_en, (reg32))
#define INT_ENABLE_GET_rx_err_int_en(reg32)                    REG_FLD_GET(INT_ENABLE_FLD_rx_err_int_en, (reg32))
#define INT_ENABLE_GET_tx_err_int_en(reg32)                    REG_FLD_GET(INT_ENABLE_FLD_tx_err_int_en, (reg32))
#define INT_ENABLE_GET_fifo_err_int_en(reg32)                  REG_FLD_GET(INT_ENABLE_FLD_fifo_err_int_en, (reg32))
#define INT_ENABLE_GET_o5_eqd_adj_done_int_en(reg32)           REG_FLD_GET(INT_ENABLE_FLD_o5_eqd_adj_done_int_en, (reg32))
#define INT_ENABLE_GET_bwm_chk_err_int_en(reg32)               REG_FLD_GET(INT_ENABLE_FLD_bwm_chk_err_int_en, (reg32))
#define INT_ENABLE_GET_dying_gasp_send_int_en(reg32)           REG_FLD_GET(INT_ENABLE_FLD_dying_gasp_send_int_en, (reg32))
#define INT_ENABLE_GET_tod_1pps_int_en(reg32)                  REG_FLD_GET(INT_ENABLE_FLD_tod_1pps_int_en, (reg32))
#define INT_ENABLE_GET_tod_update_done_int_en(reg32)           REG_FLD_GET(INT_ENABLE_FLD_tod_update_done_int_en, (reg32))
#define INT_ENABLE_GET_olt_ds_fec_chg_int_en(reg32)            REG_FLD_GET(INT_ENABLE_FLD_olt_ds_fec_chg_int_en, (reg32))
#define INT_ENABLE_GET_us_prof_idx_chg_int_en(reg32)           REG_FLD_GET(INT_ENABLE_FLD_us_prof_idx_chg_int_en, (reg32))
#define INT_ENABLE_GET_us_key_switch_done_int_en(reg32)        REG_FLD_GET(INT_ENABLE_FLD_us_key_switch_done_int_en, (reg32))
#define INT_ENABLE_GET_us_no_msg_send_int_en(reg32)            REG_FLD_GET(INT_ENABLE_FLD_us_no_msg_send_int_en, (reg32))
#define INT_ENABLE_GET_o4_registration_send_int_en(reg32)      REG_FLD_GET(INT_ENABLE_FLD_o4_registration_send_int_en, (reg32))
#define INT_ENABLE_GET_o4_ranging_req_recv_int_en(reg32)       REG_FLD_GET(INT_ENABLE_FLD_o4_ranging_req_recv_int_en, (reg32))
#define INT_ENABLE_GET_o23_sn_onu_send_int_en(reg32)           REG_FLD_GET(INT_ENABLE_FLD_o23_sn_onu_send_int_en, (reg32))
#define INT_ENABLE_GET_o23_sn_onu_req_recv_int_en(reg32)       REG_FLD_GET(INT_ENABLE_FLD_o23_sn_onu_req_recv_int_en, (reg32))
#define INT_ENABLE_GET_ploamu_send_int_en(reg32)               REG_FLD_GET(INT_ENABLE_FLD_ploamu_send_int_en, (reg32))
#define INT_ENABLE_GET_ploamd_recv_int_en(reg32)               REG_FLD_GET(INT_ENABLE_FLD_ploamd_recv_int_en, (reg32))

#define INT_STATUS_GET_o5_sn_onu_req_recv_int(reg32)           REG_FLD_GET(INT_STATUS_FLD_o5_sn_onu_req_recv_int, (reg32))
#define INT_STATUS_GET_turning_sfc_match_int(reg32)            REG_FLD_GET(INT_STATUS_FLD_turning_sfc_match_int, (reg32))
#define INT_STATUS_GET_o9_gnt_recv_int(reg32)                  REG_FLD_GET(INT_STATUS_FLD_o9_gnt_recv_int, (reg32))
#define INT_STATUS_GET_sw1_mic_done_int(reg32)                 REG_FLD_GET(INT_STATUS_FLD_sw1_mic_done_int, (reg32))
#define INT_STATUS_GET_sw0_mic_done_int(reg32)                 REG_FLD_GET(INT_STATUS_FLD_sw0_mic_done_int, (reg32))
#define INT_STATUS_GET_key_cal_done_int(reg32)                 REG_FLD_GET(INT_STATUS_FLD_key_cal_done_int, (reg32))
#define INT_STATUS_GET_lwi_int(reg32)                          REG_FLD_GET(INT_STATUS_FLD_lwi_int, (reg32))
#define INT_STATUS_GET_fwi_int(reg32)                          REG_FLD_GET(INT_STATUS_FLD_fwi_int, (reg32))
#define INT_STATUS_GET_rx_err_int(reg32)                       REG_FLD_GET(INT_STATUS_FLD_rx_err_int, (reg32))
#define INT_STATUS_GET_tx_err_int(reg32)                       REG_FLD_GET(INT_STATUS_FLD_tx_err_int, (reg32))
#define INT_STATUS_GET_fifo_err_int(reg32)                     REG_FLD_GET(INT_STATUS_FLD_fifo_err_int, (reg32))
#define INT_STATUS_GET_o5_eqd_adj_done_int(reg32)              REG_FLD_GET(INT_STATUS_FLD_o5_eqd_adj_done_int, (reg32))
#define INT_STATUS_GET_bwm_chk_err_int(reg32)                  REG_FLD_GET(INT_STATUS_FLD_bwm_chk_err_int, (reg32))
#define INT_STATUS_GET_dying_gasp_send_int(reg32)              REG_FLD_GET(INT_STATUS_FLD_dying_gasp_send_int, (reg32))
#define INT_STATUS_GET_tod_1pps_int(reg32)                     REG_FLD_GET(INT_STATUS_FLD_tod_1pps_int, (reg32))
#define INT_STATUS_GET_tod_update_done_int(reg32)              REG_FLD_GET(INT_STATUS_FLD_tod_update_done_int, (reg32))
#define INT_STATUS_GET_olt_ds_fec_chg_int(reg32)               REG_FLD_GET(INT_STATUS_FLD_olt_ds_fec_chg_int, (reg32))
#define INT_STATUS_GET_us_prof_idx_chg_int(reg32)              REG_FLD_GET(INT_STATUS_FLD_us_prof_idx_chg_int, (reg32))
#define INT_STATUS_GET_us_key_switch_done_int(reg32)           REG_FLD_GET(INT_STATUS_FLD_us_key_switch_done_int, (reg32))
#define INT_STATUS_GET_us_no_msg_send_int(reg32)               REG_FLD_GET(INT_STATUS_FLD_us_no_msg_send_int, (reg32))
#define INT_STATUS_GET_o4_registration_send_int(reg32)         REG_FLD_GET(INT_STATUS_FLD_o4_registration_send_int, (reg32))
#define INT_STATUS_GET_o4_ranging_req_recv_int(reg32)          REG_FLD_GET(INT_STATUS_FLD_o4_ranging_req_recv_int, (reg32))
#define INT_STATUS_GET_o23_sn_onu_send_int(reg32)              REG_FLD_GET(INT_STATUS_FLD_o23_sn_onu_send_int, (reg32))
#define INT_STATUS_GET_o23_sn_onu_req_recv_int(reg32)          REG_FLD_GET(INT_STATUS_FLD_o23_sn_onu_req_recv_int, (reg32))
#define INT_STATUS_GET_ploamu_send_int(reg32)                  REG_FLD_GET(INT_STATUS_FLD_ploamu_send_int, (reg32))
#define INT_STATUS_GET_ploamd_recv_int(reg32)                  REG_FLD_GET(INT_STATUS_FLD_ploamd_recv_int, (reg32))

#define FIFO_ERR_STS_GET_tx_aes_rdm_ciph_fifo_ovrn(reg32)      REG_FLD_GET(FIFO_ERR_STS_FLD_tx_aes_rdm_ciph_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_byte_mib_cmd_fifo_ovrn(reg32)         REG_FLD_GET(FIFO_ERR_STS_FLD_byte_mib_cmd_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_frm_mib_cmd_fifo_ovrn(reg32)          REG_FLD_GET(FIFO_ERR_STS_FLD_frm_mib_cmd_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_mib_cmd_fifo_ovrn(reg32)              REG_FLD_GET(FIFO_ERR_STS_FLD_mib_cmd_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_mib_tx_cmd_fifo_ovrn(reg32)           REG_FLD_GET(FIFO_ERR_STS_FLD_mib_tx_cmd_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_snf_ds_fifo_ovrn(reg32)               REG_FLD_GET(FIFO_ERR_STS_FLD_snf_ds_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_snf_us_fifo_ovrn(reg32)               REG_FLD_GET(FIFO_ERR_STS_FLD_snf_us_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_rx_ploamd_fifo_udrn(reg32)            REG_FLD_GET(FIFO_ERR_STS_FLD_rx_ploamd_fifo_udrn, (reg32))
#define FIFO_ERR_STS_GET_rx_ploamd_fifo_ovrn(reg32)            REG_FLD_GET(FIFO_ERR_STS_FLD_rx_ploamd_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_rx_mbi_pl_fifo_ovrn(reg32)            REG_FLD_GET(FIFO_ERR_STS_FLD_rx_mbi_pl_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_rx_mbi_hdr_fifo_ovrn(reg32)           REG_FLD_GET(FIFO_ERR_STS_FLD_rx_mbi_hdr_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_rx_aes_rdm_ciph_fifo_ovrn(reg32)      REG_FLD_GET(FIFO_ERR_STS_FLD_rx_aes_rdm_ciph_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_rx_aes_ciph_txt_fifo_ovrn(reg32)      REG_FLD_GET(FIFO_ERR_STS_FLD_rx_aes_ciph_txt_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_rx_aes_cryp_cnt_fifo_ovrn(reg32)      REG_FLD_GET(FIFO_ERR_STS_FLD_rx_aes_cryp_cnt_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_tx_align_fifo_udrn(reg32)             REG_FLD_GET(FIFO_ERR_STS_FLD_tx_align_fifo_udrn, (reg32))
#define FIFO_ERR_STS_GET_tx_ploamu_fifo_ovrn(reg32)            REG_FLD_GET(FIFO_ERR_STS_FLD_tx_ploamu_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_bst_fifo_ovrn(reg32)                  REG_FLD_GET(FIFO_ERR_STS_FLD_bst_fifo_ovrn, (reg32))
#define FIFO_ERR_STS_GET_sgl_fifo_ovrn(reg32)                  REG_FLD_GET(FIFO_ERR_STS_FLD_sgl_fifo_ovrn, (reg32))

#define TX_ERR_STS_GET_tx_prof_invld_err(reg32)                REG_FLD_GET(TX_ERR_STS_FLD_tx_prof_invld_err, (reg32))
#define TX_ERR_STS_GET_tx_late_start_err(reg32)                REG_FLD_GET(TX_ERR_STS_FLD_tx_late_start_err, (reg32))
#define TX_ERR_STS_GET_tx_bst_sgl_diff_err(reg32)              REG_FLD_GET(TX_ERR_STS_FLD_tx_bst_sgl_diff_err, (reg32))

#define RX_ERR_STS_GET_rx_xgem_hdr_hec_err(reg32)              REG_FLD_GET(RX_ERR_STS_FLD_rx_xgem_hdr_hec_err, (reg32))
#define RX_ERR_STS_GET_rx_alloc_hec_err(reg32)                 REG_FLD_GET(RX_ERR_STS_FLD_rx_alloc_hec_err, (reg32))
#define RX_ERR_STS_GET_rx_mbi_xgem_drop_err(reg32)             REG_FLD_GET(RX_ERR_STS_FLD_rx_mbi_xgem_drop_err, (reg32))
#define RX_ERR_STS_GET_rx_omci_mic_err(reg32)                  REG_FLD_GET(RX_ERR_STS_FLD_rx_omci_mic_err, (reg32))
#define RX_ERR_STS_GET_rx_ploam_mic_err(reg32)                 REG_FLD_GET(RX_ERR_STS_FLD_rx_ploam_mic_err, (reg32))
#define RX_ERR_STS_GET_rx_eth_crc_err(reg32)                   REG_FLD_GET(RX_ERR_STS_FLD_rx_eth_crc_err, (reg32))
#define RX_ERR_STS_GET_rx_aes_key_err(reg32)                   REG_FLD_GET(RX_ERR_STS_FLD_rx_aes_key_err, (reg32))
#define RX_ERR_STS_GET_rx_gem_intlv_err(reg32)                 REG_FLD_GET(RX_ERR_STS_FLD_rx_gem_intlv_err, (reg32))
#define RX_ERR_STS_GET_rx_los_gem_del_err(reg32)               REG_FLD_GET(RX_ERR_STS_FLD_rx_los_gem_del_err, (reg32))
#define RX_ERR_STS_GET_rx_bip_err(reg32)                       REG_FLD_GET(RX_ERR_STS_FLD_rx_bip_err, (reg32))
#define RX_ERR_STS_GET_rx_hlend_hec_err(reg32)                 REG_FLD_GET(RX_ERR_STS_FLD_rx_hlend_hec_err, (reg32))
#define RX_ERR_STS_GET_rx_pon_id_hec_err(reg32)                REG_FLD_GET(RX_ERR_STS_FLD_rx_pon_id_hec_err, (reg32))
#define RX_ERR_STS_GET_rx_sfc_hec_uc_err(reg32)                REG_FLD_GET(RX_ERR_STS_FLD_rx_sfc_hec_uc_err, (reg32))
#define RX_ERR_STS_GET_rx_eof_err(reg32)                       REG_FLD_GET(RX_ERR_STS_FLD_rx_eof_err, (reg32))

#if defined(TCSUPPORT_CPU_AN7583)
#define FIFO_ERR_ENABLE_GET_tx_aes_rdm_ciph_fifo_ovrn_en(reg32) REG_FLD_GET(FIFO_ERR_ENABLE_FLD_tx_aes_rdm_ciph_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_byte_mib_cmd_fifo_ovrn_en(reg32)   REG_FLD_GET(FIFO_ERR_ENABLE_FLD_byte_mib_cmd_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_frm_mib_cmd_fifo_ovrn_en(reg32)    REG_FLD_GET(FIFO_ERR_ENABLE_FLD_frm_mib_cmd_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_mib_cmd_fifo_ovrn_en(reg32)        REG_FLD_GET(FIFO_ERR_ENABLE_FLD_mib_cmd_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_mib_tx_cmd_fifo_ovrn_en(reg32)     REG_FLD_GET(FIFO_ERR_ENABLE_FLD_mib_tx_cmd_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_snf_ds_fifo_ovrn_en(reg32)         REG_FLD_GET(FIFO_ERR_ENABLE_FLD_snf_ds_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_snf_us_fifo_ovrn_en(reg32)         REG_FLD_GET(FIFO_ERR_ENABLE_FLD_snf_us_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_rx_ploamd_fifo_udrn_en(reg32)      REG_FLD_GET(FIFO_ERR_ENABLE_FLD_rx_ploamd_fifo_udrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_rx_ploamd_fifo_ovrn_en(reg32)      REG_FLD_GET(FIFO_ERR_ENABLE_FLD_rx_ploamd_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_rx_mbi_pl_fifo_ovrn_en(reg32)      REG_FLD_GET(FIFO_ERR_ENABLE_FLD_rx_mbi_pl_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_rx_mbi_hdr_fifo_ovrn_en(reg32)     REG_FLD_GET(FIFO_ERR_ENABLE_FLD_rx_mbi_hdr_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_rx_aes_rdm_ciph_fifo_ovrn_en(reg32) REG_FLD_GET(FIFO_ERR_ENABLE_FLD_rx_aes_rdm_ciph_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_rx_aes_ciph_txt_fifo_ovrn_en(reg32) REG_FLD_GET(FIFO_ERR_ENABLE_FLD_rx_aes_ciph_txt_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_rx_aes_cryp_cnt_fifo_ovrn_en(reg32) REG_FLD_GET(FIFO_ERR_ENABLE_FLD_rx_aes_cryp_cnt_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_tx_align_fifo_udrn_en(reg32)       REG_FLD_GET(FIFO_ERR_ENABLE_FLD_tx_align_fifo_udrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_tx_ploamu_fifo_ovrn_en(reg32)      REG_FLD_GET(FIFO_ERR_ENABLE_FLD_tx_ploamu_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_bst_fifo_ovrn_en(reg32)            REG_FLD_GET(FIFO_ERR_ENABLE_FLD_bst_fifo_ovrn_en, (reg32))
#define FIFO_ERR_ENABLE_GET_sgl_fifo_ovrn_en(reg32)            REG_FLD_GET(FIFO_ERR_ENABLE_FLD_sgl_fifo_ovrn_en, (reg32))

#define TX_ERR_ENABLE_GET_tx_prof_invld_err_en(reg32)          REG_FLD_GET(TX_ERR_ENABLE_FLD_tx_prof_invld_err_en, (reg32))
#define TX_ERR_ENABLE_GET_tx_late_start_err_en(reg32)          REG_FLD_GET(TX_ERR_ENABLE_FLD_tx_late_start_err_en, (reg32))
#define TX_ERR_ENABLE_GET_tx_bst_sgl_diff_err_en(reg32)        REG_FLD_GET(TX_ERR_ENABLE_FLD_tx_bst_sgl_diff_err_en, (reg32))

#define RX_ERR_ENABLE_GET_rx_xgem_hdr_hec_err_en(reg32)        REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_xgem_hdr_hec_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_alloc_hec_err_en(reg32)           REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_alloc_hec_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_mbi_xgem_drop_err_en(reg32)       REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_mbi_xgem_drop_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_omci_mic_err_en(reg32)            REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_omci_mic_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_ploam_mic_err_en(reg32)           REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_ploam_mic_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_eth_crc_err_en(reg32)             REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_eth_crc_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_aes_key_err_en(reg32)             REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_aes_key_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_gem_intlv_err_en(reg32)           REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_gem_intlv_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_los_gem_del_err_en(reg32)         REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_los_gem_del_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_bip_err_en(reg32)                 REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_bip_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_hlend_hec_err_en(reg32)           REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_hlend_hec_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_pon_id_hec_err_en(reg32)          REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_pon_id_hec_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_sfc_hec_uc_err_en(reg32)          REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_sfc_hec_uc_err_en, (reg32))
#define RX_ERR_ENABLE_GET_rx_eof_err_en(reg32)                 REG_FLD_GET(RX_ERR_ENABLE_FLD_rx_eof_err_en, (reg32))
#endif

#define COR_TAG_GET_cor_tag(reg32)                             REG_FLD_GET(COR_TAG_FLD_cor_tag, (reg32))

#define CUR_DS_PON_ID_GET_cur_ds_pon_id(reg32)                 REG_FLD_GET(CUR_DS_PON_ID_FLD_cur_ds_pon_id, (reg32))

#define CUR_US_PON_ID_GET_cur_us_pon_id(reg32)                 REG_FLD_GET(CUR_US_PON_ID_FLD_cur_us_pon_id, (reg32))

#define CALIB_STS_L32_GET_calib_sts_l32(reg32)                 REG_FLD_GET(CALIB_STS_L32_FLD_calib_sts_l32, (reg32))

#define CALIB_STS_H32_GET_calib_sts_h32(reg32)                 REG_FLD_GET(CALIB_STS_H32_FLD_calib_sts_h32, (reg32))

#define TUNING_GRAN_GET_turning_gran(reg32)                    REG_FLD_GET(TUNING_GRAN_FLD_turning_gran, (reg32))

#define STEP_TUNING_TIME_GET_step_turning_time(reg32)          REG_FLD_GET(STEP_TUNING_TIME_FLD_step_turning_time, (reg32))

#define US_RATE_CAP_GET_us_10g_cap(reg32)                      REG_FLD_GET(US_RATE_CAP_FLD_us_10g_cap, (reg32))
#define US_RATE_CAP_GET_us_2p5g_cap(reg32)                     REG_FLD_GET(US_RATE_CAP_FLD_us_2p5g_cap, (reg32))

#define ATTENUATION_GET_attenuation(reg32)                     REG_FLD_GET(ATTENUATION_FLD_attenuation, (reg32))

#define POWER_LEVEL_CAP_GET_power_level_cap(reg32)             REG_FLD_GET(POWER_LEVEL_CAP_FLD_power_level_cap, (reg32))

#define O23_O4_PLOAMU_CTRL_GET_o23_o4_ploamu_ctrl(reg32)       REG_FLD_GET(O23_O4_PLOAMU_CTRL_FLD_o23_o4_ploamu_ctrl, (reg32))

#define ACTIVATION_ST_GET_act_st(reg32)                        REG_FLD_GET(ACTIVATION_ST_FLD_act_st, (reg32))

#define RSP_TIME_GET_tresp(reg32)                              REG_FLD_GET(RSP_TIME_FLD_tresp, (reg32))

#define RDM_DLY_GET_max_rdm_dly(reg32)                         REG_FLD_GET(RDM_DLY_FLD_max_rdm_dly, (reg32))
#define RDM_DLY_GET_rdm_dly(reg32)                             REG_FLD_GET(RDM_DLY_FLD_rdm_dly, (reg32))

#define INT_DLY_GET_phy_rx_dly(reg32)                          REG_FLD_GET(INT_DLY_FLD_phy_rx_dly, (reg32))
#define INT_DLY_GET_phy_tx_dly(reg32)                          REG_FLD_GET(INT_DLY_FLD_phy_tx_dly, (reg32))

#define EQD_GET_eqd(reg32)                                     REG_FLD_GET(EQD_FLD_eqd, (reg32))

#define SYN_MRG_GET_syn_mrg(reg32)                             REG_FLD_GET(SYN_MRG_FLD_syn_mrg, (reg32))

#define US_PROF_VLD_GET_us_prof3_vsn(reg32)                    REG_FLD_GET(US_PROF_VLD_FLD_us_prof3_vsn, (reg32))
#define US_PROF_VLD_GET_us_prof3_vld(reg32)                    REG_FLD_GET(US_PROF_VLD_FLD_us_prof3_vld, (reg32))
#define US_PROF_VLD_GET_us_prof2_vsn(reg32)                    REG_FLD_GET(US_PROF_VLD_FLD_us_prof2_vsn, (reg32))
#define US_PROF_VLD_GET_us_prof2_vld(reg32)                    REG_FLD_GET(US_PROF_VLD_FLD_us_prof2_vld, (reg32))
#define US_PROF_VLD_GET_us_prof1_vsn(reg32)                    REG_FLD_GET(US_PROF_VLD_FLD_us_prof1_vsn, (reg32))
#define US_PROF_VLD_GET_us_prof1_vld(reg32)                    REG_FLD_GET(US_PROF_VLD_FLD_us_prof1_vld, (reg32))
#define US_PROF_VLD_GET_us_prof0_vsn(reg32)                    REG_FLD_GET(US_PROF_VLD_FLD_us_prof0_vsn, (reg32))
#define US_PROF_VLD_GET_us_prof0_vld(reg32)                    REG_FLD_GET(US_PROF_VLD_FLD_us_prof0_vld, (reg32))

#define US_PROF_PSBu_LEN_0_1_GET_us_psbu_len_prof1(reg32)      REG_FLD_GET(US_PROF_PSBu_LEN_0_1_FLD_us_psbu_len_prof1, (reg32))
#define US_PROF_PSBu_LEN_0_1_GET_us_psbu_len_prof0(reg32)      REG_FLD_GET(US_PROF_PSBu_LEN_0_1_FLD_us_psbu_len_prof0, (reg32))

#define US_PROF_PSBu_LEN_2_3_GET_us_psbu_len_prof3(reg32)      REG_FLD_GET(US_PROF_PSBu_LEN_2_3_FLD_us_psbu_len_prof3, (reg32))
#define US_PROF_PSBu_LEN_2_3_GET_us_psbu_len_prof2(reg32)      REG_FLD_GET(US_PROF_PSBu_LEN_2_3_FLD_us_psbu_len_prof2, (reg32))

#define US_AES_KEY_CTRL_GET_us_aes_key_vld(reg32)              REG_FLD_GET(US_AES_KEY_CTRL_FLD_us_aes_key_vld, (reg32))
#define US_AES_KEY_CTRL_GET_us_aes_key_idx(reg32)              REG_FLD_GET(US_AES_KEY_CTRL_FLD_us_aes_key_idx, (reg32))

#define DS_AES_KEY_VLD_GET_ds_aes_bc_idx1_key_vld(reg32)       REG_FLD_GET(DS_AES_KEY_VLD_FLD_ds_aes_bc_idx1_key_vld, (reg32))
#define DS_AES_KEY_VLD_GET_ds_aes_bc_idx0_key_vld(reg32)       REG_FLD_GET(DS_AES_KEY_VLD_FLD_ds_aes_bc_idx0_key_vld, (reg32))
#define DS_AES_KEY_VLD_GET_ds_aes_uc_idx1_key_vld(reg32)       REG_FLD_GET(DS_AES_KEY_VLD_FLD_ds_aes_uc_idx1_key_vld, (reg32))
#define DS_AES_KEY_VLD_GET_ds_aes_uc_idx0_key_vld(reg32)       REG_FLD_GET(DS_AES_KEY_VLD_FLD_ds_aes_uc_idx0_key_vld, (reg32))

#define DEFT_GPID_ENC_GET_deft_gpid_enc(reg32)                 REG_FLD_GET(DEFT_GPID_ENC_FLD_deft_gpid_enc, (reg32))

#define AES_UC_IDX0_KEY0_GET_aes_uc_idx0_key0(reg32)           REG_FLD_GET(AES_UC_IDX0_KEY0_FLD_aes_uc_idx0_key0, (reg32))

#define AES_UC_IDX0_KEY1_GET_aes_uc_idx0_key1(reg32)           REG_FLD_GET(AES_UC_IDX0_KEY1_FLD_aes_uc_idx0_key1, (reg32))

#define AES_UC_IDX0_KEY2_GET_aes_uc_idx0_key2(reg32)           REG_FLD_GET(AES_UC_IDX0_KEY2_FLD_aes_uc_idx0_key2, (reg32))

#define AES_UC_IDX0_KEY3_GET_aes_uc_idx0_key3(reg32)           REG_FLD_GET(AES_UC_IDX0_KEY3_FLD_aes_uc_idx0_key3, (reg32))

#define AES_UC_IDX1_KEY0_GET_aes_uc_idx1_key0(reg32)           REG_FLD_GET(AES_UC_IDX1_KEY0_FLD_aes_uc_idx1_key0, (reg32))

#define AES_UC_IDX1_KEY1_GET_aes_uc_idx1_key1(reg32)           REG_FLD_GET(AES_UC_IDX1_KEY1_FLD_aes_uc_idx1_key1, (reg32))

#define AES_UC_IDX1_KEY2_GET_aes_uc_idx1_key2(reg32)           REG_FLD_GET(AES_UC_IDX1_KEY2_FLD_aes_uc_idx1_key2, (reg32))

#define AES_UC_IDX1_KEY3_GET_aes_uc_idx1_key3(reg32)           REG_FLD_GET(AES_UC_IDX1_KEY3_FLD_aes_uc_idx1_key3, (reg32))

#define AES_BC_IDX0_KEY0_GET_aes_bc_idx0_key0(reg32)           REG_FLD_GET(AES_BC_IDX0_KEY0_FLD_aes_bc_idx0_key0, (reg32))

#define AES_BC_IDX0_KEY1_GET_aes_bc_idx0_key1(reg32)           REG_FLD_GET(AES_BC_IDX0_KEY1_FLD_aes_bc_idx0_key1, (reg32))

#define AES_BC_IDX0_KEY2_GET_aes_bc_idx0_key2(reg32)           REG_FLD_GET(AES_BC_IDX0_KEY2_FLD_aes_bc_idx0_key2, (reg32))

#define AES_BC_IDX0_KEY3_GET_aes_bc_idx0_key3(reg32)           REG_FLD_GET(AES_BC_IDX0_KEY3_FLD_aes_bc_idx0_key3, (reg32))

#define AES_BC_IDX1_KEY0_GET_aes_bc_idx1_key0(reg32)           REG_FLD_GET(AES_BC_IDX1_KEY0_FLD_aes_bc_idx1_key0, (reg32))

#define AES_BC_IDX1_KEY1_GET_aes_bc_idx1_key1(reg32)           REG_FLD_GET(AES_BC_IDX1_KEY1_FLD_aes_bc_idx1_key1, (reg32))

#define AES_BC_IDX1_KEY2_GET_aes_bc_idx1_key2(reg32)           REG_FLD_GET(AES_BC_IDX1_KEY2_FLD_aes_bc_idx1_key2, (reg32))

#define AES_BC_IDX1_KEY3_GET_aes_bc_idx1_key3(reg32)           REG_FLD_GET(AES_BC_IDX1_KEY3_FLD_aes_bc_idx1_key3, (reg32))

#define TCONT_ID_CFG_GET_tcont_cmd(reg32)                      REG_FLD_GET(TCONT_ID_CFG_FLD_tcont_cmd, (reg32))
#define TCONT_ID_CFG_GET_tcont_id_index(reg32)                 REG_FLD_GET(TCONT_ID_CFG_FLD_tcont_id_index, (reg32))
#define TCONT_ID_CFG_GET_wr_tcont_id_vld(reg32)                REG_FLD_GET(TCONT_ID_CFG_FLD_wr_tcont_id_vld, (reg32))
#define TCONT_ID_CFG_GET_wr_tcont_id(reg32)                    REG_FLD_GET(TCONT_ID_CFG_FLD_wr_tcont_id, (reg32))

#define TCONT_ID_STS_GET_tcont_cmd_done(reg32)                 REG_FLD_GET(TCONT_ID_STS_FLD_tcont_cmd_done, (reg32))
#define TCONT_ID_STS_GET_rd_tcont_id_vld(reg32)                REG_FLD_GET(TCONT_ID_STS_FLD_rd_tcont_id_vld, (reg32))
#define TCONT_ID_STS_GET_rd_tcont_id(reg32)                    REG_FLD_GET(TCONT_ID_STS_FLD_rd_tcont_id, (reg32))

#define GPIDX_TBL_INIT_GET_gpidx_tbl_init_done(reg32)          REG_FLD_GET(GPIDX_TBL_INIT_FLD_gpidx_tbl_init_done, (reg32))
#define GPIDX_TBL_INIT_GET_gpidx_tbl_init_start(reg32)         REG_FLD_GET(GPIDX_TBL_INIT_FLD_gpidx_tbl_init_start, (reg32))

#define GPIDX_TBL_CTRL_GET_gpidx_tbl_cmd(reg32)                REG_FLD_GET(GPIDX_TBL_CTRL_FLD_gpidx_tbl_cmd, (reg32))
#define GPIDX_TBL_CTRL_GET_gpidx_tbl_addr(reg32)               REG_FLD_GET(GPIDX_TBL_CTRL_FLD_gpidx_tbl_addr, (reg32))
#define GPIDX_TBL_CTRL_GET_gpidx_tbl_wdata(reg32)              REG_FLD_GET(GPIDX_TBL_CTRL_FLD_gpidx_tbl_wdata, (reg32))

#define GPIDX_TBL_STS_GET_gpidx_cmd_done(reg32)                REG_FLD_GET(GPIDX_TBL_STS_FLD_gpidx_cmd_done, (reg32))
#define GPIDX_TBL_STS_GET_gpidx_tbl_rdata(reg32)               REG_FLD_GET(GPIDX_TBL_STS_FLD_gpidx_tbl_rdata, (reg32))

#define GEM_TBL_INIT_GET_gem_tbl_init_done(reg32)              REG_FLD_GET(GEM_TBL_INIT_FLD_gem_tbl_init_done, (reg32))
#define GEM_TBL_INIT_GET_gem_tbl_init_start(reg32)             REG_FLD_GET(GEM_TBL_INIT_FLD_gem_tbl_init_start, (reg32))

#define GEM_PORT_CFG_GET_gpid_cmd(reg32)                       REG_FLD_GET(GEM_PORT_CFG_FLD_gpid_cmd, (reg32))
#define GEM_PORT_CFG_GET_gpid_vld(reg32)                       REG_FLD_GET(GEM_PORT_CFG_FLD_gpid_vld, (reg32))
#define GEM_PORT_CFG_GET_gpid_type(reg32)                      REG_FLD_GET(GEM_PORT_CFG_FLD_gpid_type, (reg32))
#define GEM_PORT_CFG_GET_gpid_us_encrypt(reg32)                REG_FLD_GET(GEM_PORT_CFG_FLD_gpid_us_encrypt, (reg32))
#define GEM_PORT_CFG_GET_gem_port_id(reg32)                    REG_FLD_GET(GEM_PORT_CFG_FLD_gem_port_id, (reg32))

#define GEM_PORT_STS_GET_gpid_cmd_done(reg32)                  REG_FLD_GET(GEM_PORT_STS_FLD_gpid_cmd_done, (reg32))
#define GEM_PORT_STS_GET_gpid_rd_sts(reg32)                    REG_FLD_GET(GEM_PORT_STS_FLD_gpid_rd_sts, (reg32))

#define G_TX_FCS_TBL_INIT_GET_tx_fcs_tbl_init_done(reg32)      REG_FLD_GET(G_TX_FCS_TBL_INIT_FLD_tx_fcs_tbl_init_done, (reg32))
#define G_TX_FCS_TBL_INIT_GET_tx_fcs_tbl_init_start(reg32)     REG_FLD_GET(G_TX_FCS_TBL_INIT_FLD_tx_fcs_tbl_init_start, (reg32))

#if defined(TCSUPPORT_CPU_EN7581)
#define IDLE_GEM_CTRL_GET_force_idle_hdr_data_same(reg32)      REG_FLD_GET(IDLE_GEM_CTRL_FLD_force_idle_hdr_data_same, (reg32))
#endif
#define IDLE_GEM_CTRL_GET_idle_gem_max_size(reg32)             REG_FLD_GET(IDLE_GEM_CTRL_FLD_idle_gem_max_size, (reg32))
#define IDLE_GEM_CTRL_GET_idle_gem_thld(reg32)                 REG_FLD_GET(IDLE_GEM_CTRL_FLD_idle_gem_thld, (reg32))

#define US_DYING_GASP_CTRL_GET_dying_gasp_send_num(reg32)      REG_FLD_GET(US_DYING_GASP_CTRL_FLD_dying_gasp_send_num, (reg32))
#define US_DYING_GASP_CTRL_GET_sw_dying_gasp_trig(reg32)       REG_FLD_GET(US_DYING_GASP_CTRL_FLD_sw_dying_gasp_trig, (reg32))
#define US_DYING_GASP_CTRL_GET_hw_dying_gasp_sts(reg32)        REG_FLD_GET(US_DYING_GASP_CTRL_FLD_hw_dying_gasp_sts, (reg32))
#define US_DYING_GASP_CTRL_GET_hw_dying_gasp_en(reg32)         REG_FLD_GET(US_DYING_GASP_CTRL_FLD_hw_dying_gasp_en, (reg32))

#define TX_XGEM_IDLE_DATA_GET_tx_xgem_idle_data(reg32)         REG_FLD_GET(TX_XGEM_IDLE_DATA_FLD_tx_xgem_idle_data, (reg32))

#define TX_OMCI_PRE_GET_GET_tx_limit_get_omci_size(reg32)      REG_FLD_GET(TX_OMCI_PRE_GET_FLD_tx_limit_get_omci_size, (reg32))
#define TX_OMCI_PRE_GET_GET_tx_limit_get_omci_en(reg32)        REG_FLD_GET(TX_OMCI_PRE_GET_FLD_tx_limit_get_omci_en, (reg32))
#define TX_OMCI_PRE_GET_GET_tx_pre_get_omci_en(reg32)          REG_FLD_GET(TX_OMCI_PRE_GET_FLD_tx_pre_get_omci_en, (reg32))

#define RX_OMCI_PRE_GET_GET_rx_omci_intr_eth_en(reg32)         REG_FLD_GET(RX_OMCI_PRE_GET_FLD_rx_omci_intr_eth_en, (reg32))

#define EPDROP_EN_GET_errploamd_drop_en(reg32)                 REG_FLD_GET(EPDROP_EN_FLD_errploamd_drop_en, (reg32))

#define PLOAMu_FIFO_STS_GET_ploamu_fifo_ovrn(reg32)            REG_FLD_GET(PLOAMu_FIFO_STS_FLD_ploamu_fifo_ovrn, (reg32))
#define PLOAMu_FIFO_STS_GET_ploamu_fifo_min_avail(reg32)       REG_FLD_GET(PLOAMu_FIFO_STS_FLD_ploamu_fifo_min_avail, (reg32))
#define PLOAMu_FIFO_STS_GET_ploamu_fifo_avail(reg32)           REG_FLD_GET(PLOAMu_FIFO_STS_FLD_ploamu_fifo_avail, (reg32))

#define PLOAMu_WDATA_GET_ploamu_wdata(reg32)                   REG_FLD_GET(PLOAMu_WDATA_FLD_ploamu_wdata, (reg32))

#define PLOAMd_FIFO_STS_GET_ploamd_fifo_ovrn(reg32)            REG_FLD_GET(PLOAMd_FIFO_STS_FLD_ploamd_fifo_ovrn, (reg32))
#define PLOAMd_FIFO_STS_GET_ploamd_fifo_max_used(reg32)        REG_FLD_GET(PLOAMd_FIFO_STS_FLD_ploamd_fifo_max_used, (reg32))
#define PLOAMd_FIFO_STS_GET_ploamd_fifo_used(reg32)            REG_FLD_GET(PLOAMd_FIFO_STS_FLD_ploamd_fifo_used, (reg32))

#define PLOAMd_RDATA_GET_ploamd_rdata(reg32)                   REG_FLD_GET(PLOAMd_RDATA_FLD_ploamd_rdata, (reg32))

#define KEY_GEN_GET_kek_start(reg32)                           REG_FLD_GET(KEY_GEN_FLD_kek_start, (reg32))
#define KEY_GEN_GET_ploam_ik_start(reg32)                      REG_FLD_GET(KEY_GEN_FLD_ploam_ik_start, (reg32))
#define KEY_GEN_GET_omci_ik_start(reg32)                       REG_FLD_GET(KEY_GEN_FLD_omci_ik_start, (reg32))
#define KEY_GEN_GET_sk_start(reg32)                            REG_FLD_GET(KEY_GEN_FLD_sk_start, (reg32))
#define KEY_GEN_GET_regmsk_start(reg32)                        REG_FLD_GET(KEY_GEN_FLD_regmsk_start, (reg32))

#define CUR_KIDX_GET_cur_oik_idx(reg32)                        REG_FLD_GET(CUR_KIDX_FLD_cur_oik_idx, (reg32))
#define CUR_KIDX_GET_cur_pik_idx(reg32)                        REG_FLD_GET(CUR_KIDX_FLD_cur_pik_idx, (reg32))

#define MSK_0_GET_msk0(reg32)                                  REG_FLD_GET(MSK_0_FLD_msk0, (reg32))

#define MSK_1_GET_msk1(reg32)                                  REG_FLD_GET(MSK_1_FLD_msk1, (reg32))

#define MSK_2_GET_msk2(reg32)                                  REG_FLD_GET(MSK_2_FLD_msk2, (reg32))

#define MSK_3_GET_msk3(reg32)                                  REG_FLD_GET(MSK_3_FLD_msk3, (reg32))

#define REGMSK_0_GET_regmsk0(reg32)                            REG_FLD_GET(REGMSK_0_FLD_regmsk0, (reg32))

#define REGMSK_1_GET_regmsk1(reg32)                            REG_FLD_GET(REGMSK_1_FLD_regmsk1, (reg32))

#define REGMSK_2_GET_regmsk2(reg32)                            REG_FLD_GET(REGMSK_2_FLD_regmsk2, (reg32))

#define REGMSK_3_GET_regmsk3(reg32)                            REG_FLD_GET(REGMSK_3_FLD_regmsk3, (reg32))

#define SK_0_GET_sk0(reg32)                                    REG_FLD_GET(SK_0_FLD_sk0, (reg32))

#define SK_1_GET_sk1(reg32)                                    REG_FLD_GET(SK_1_FLD_sk1, (reg32))

#define SK_2_GET_sk2(reg32)                                    REG_FLD_GET(SK_2_FLD_sk2, (reg32))

#define SK_3_GET_sk3(reg32)                                    REG_FLD_GET(SK_3_FLD_sk3, (reg32))

#define HW_GENK_0_GET_hw_genk_0(reg32)                         REG_FLD_GET(HW_GENK_0_FLD_hw_genk_0, (reg32))

#define HW_GENK_1_GET_hw_genk_1(reg32)                         REG_FLD_GET(HW_GENK_1_FLD_hw_genk_1, (reg32))

#define HW_GENK_2_GET_hw_genk_2(reg32)                         REG_FLD_GET(HW_GENK_2_FLD_hw_genk_2, (reg32))

#define HW_GENK_3_GET_hw_genk_3(reg32)                         REG_FLD_GET(HW_GENK_3_FLD_hw_genk_3, (reg32))

#define PIK0_0_GET_pik0_0(reg32)                               REG_FLD_GET(PIK0_0_FLD_pik0_0, (reg32))

#define PIK0_1_GET_pik0_1(reg32)                               REG_FLD_GET(PIK0_1_FLD_pik0_1, (reg32))

#define PIK0_2_GET_pik0_2(reg32)                               REG_FLD_GET(PIK0_2_FLD_pik0_2, (reg32))

#define PIK0_3_GET_pik0_3(reg32)                               REG_FLD_GET(PIK0_3_FLD_pik0_3, (reg32))

#define PIK1_0_GET_pik1_0(reg32)                               REG_FLD_GET(PIK1_0_FLD_pik1_0, (reg32))

#define PIK1_1_GET_pik1_1(reg32)                               REG_FLD_GET(PIK1_1_FLD_pik1_1, (reg32))

#define PIK1_2_GET_pik1_2(reg32)                               REG_FLD_GET(PIK1_2_FLD_pik1_2, (reg32))

#define PIK1_3_GET_pik1_3(reg32)                               REG_FLD_GET(PIK1_3_FLD_pik1_3, (reg32))

#define OIK0_0_GET_oik0_0(reg32)                               REG_FLD_GET(OIK0_0_FLD_oik0_0, (reg32))

#define OIK0_1_GET_oik0_1(reg32)                               REG_FLD_GET(OIK0_1_FLD_oik0_1, (reg32))

#define OIK0_2_GET_oik0_2(reg32)                               REG_FLD_GET(OIK0_2_FLD_oik0_2, (reg32))

#define OIK0_3_GET_oik0_3(reg32)                               REG_FLD_GET(OIK0_3_FLD_oik0_3, (reg32))

#define OIK1_0_GET_oik1_0(reg32)                               REG_FLD_GET(OIK1_0_FLD_oik1_0, (reg32))

#define OIK1_1_GET_oik1_1(reg32)                               REG_FLD_GET(OIK1_1_FLD_oik1_1, (reg32))

#define OIK1_2_GET_oik1_2(reg32)                               REG_FLD_GET(OIK1_2_FLD_oik1_2, (reg32))

#define OIK1_3_GET_oik1_3(reg32)                               REG_FLD_GET(OIK1_3_FLD_oik1_3, (reg32))

#define KEK0_0_GET_kek0_0(reg32)                               REG_FLD_GET(KEK0_0_FLD_kek0_0, (reg32))

#define KEK0_1_GET_kek0_1(reg32)                               REG_FLD_GET(KEK0_1_FLD_kek0_1, (reg32))

#define KEK0_2_GET_kek0_2(reg32)                               REG_FLD_GET(KEK0_2_FLD_kek0_2, (reg32))

#define KEK0_3_GET_kek0_3(reg32)                               REG_FLD_GET(KEK0_3_FLD_kek0_3, (reg32))

#define KEK1_0_GET_kek1_0(reg32)                               REG_FLD_GET(KEK1_0_FLD_kek1_0, (reg32))

#define KEK1_1_GET_kek1_1(reg32)                               REG_FLD_GET(KEK1_1_FLD_kek1_1, (reg32))

#define KEK1_2_GET_kek1_2(reg32)                               REG_FLD_GET(KEK1_2_FLD_kek1_2, (reg32))

#define KEK1_3_GET_kek1_3(reg32)                               REG_FLD_GET(KEK1_3_FLD_kek1_3, (reg32))

#define PON_TAG_0_GET_pon_tag_0(reg32)                         REG_FLD_GET(PON_TAG_0_FLD_pon_tag_0, (reg32))

#define PON_TAG_1_GET_pon_tag_1(reg32)                         REG_FLD_GET(PON_TAG_1_FLD_pon_tag_1, (reg32))

#define SW_SET_KIDX_GET_sw_set_oik_en(reg32)                   REG_FLD_GET(SW_SET_KIDX_FLD_sw_set_oik_en, (reg32))
#define SW_SET_KIDX_GET_sw_set_pik_en(reg32)                   REG_FLD_GET(SW_SET_KIDX_FLD_sw_set_pik_en, (reg32))
#define SW_SET_KIDX_GET_sw_set_oik_idx(reg32)                  REG_FLD_GET(SW_SET_KIDX_FLD_sw_set_oik_idx, (reg32))
#define SW_SET_KIDX_GET_sw_set_pik_idx(reg32)                  REG_FLD_GET(SW_SET_KIDX_FLD_sw_set_pik_idx, (reg32))

#define SW0_ENCSTART_GET_sw0_encstart(reg32)                   REG_FLD_GET(SW0_ENCSTART_FLD_sw0_encstart, (reg32))

#define SW0_MADDR_GET_sw0_mdtaddr(reg32)                       REG_FLD_GET(SW0_MADDR_FLD_sw0_mdtaddr, (reg32))

#define SW0_RADDR_GET_sw0_rdtaddr(reg32)                       REG_FLD_GET(SW0_RADDR_FLD_sw0_rdtaddr, (reg32))

#define SW0_KADDR_GET_sw0_kaddr(reg32)                         REG_FLD_GET(SW0_KADDR_FLD_sw0_kaddr, (reg32))

#define SW0_ENCLEN_GET_sw0_rdtlen(reg32)                       REG_FLD_GET(SW0_ENCLEN_FLD_sw0_rdtlen, (reg32))
#define SW0_ENCLEN_GET_sw0_mdtlen(reg32)                       REG_FLD_GET(SW0_ENCLEN_FLD_sw0_mdtlen, (reg32))

#define SW0_ENCINFO_GET_sw0_enckidx(reg32)                     REG_FLD_GET(SW0_ENCINFO_FLD_sw0_enckidx, (reg32))
#define SW0_ENCINFO_GET_sw0_encdic(reg32)                      REG_FLD_GET(SW0_ENCINFO_FLD_sw0_encdic, (reg32))

#define SW1_ENCSTART_GET_sw1_encstart(reg32)                   REG_FLD_GET(SW1_ENCSTART_FLD_sw1_encstart, (reg32))

#define SW1_MADDR_GET_sw1_mdtaddr(reg32)                       REG_FLD_GET(SW1_MADDR_FLD_sw1_mdtaddr, (reg32))

#define SW1_RADDR_GET_sw1_rdtaddr(reg32)                       REG_FLD_GET(SW1_RADDR_FLD_sw1_rdtaddr, (reg32))

#define SW1_KADDR_GET_sw1_kaddr(reg32)                         REG_FLD_GET(SW1_KADDR_FLD_sw1_kaddr, (reg32))

#define SW1_ENCLEN_GET_sw1_rdtlen(reg32)                       REG_FLD_GET(SW1_ENCLEN_FLD_sw1_rdtlen, (reg32))
#define SW1_ENCLEN_GET_sw1_mdtlen(reg32)                       REG_FLD_GET(SW1_ENCLEN_FLD_sw1_mdtlen, (reg32))

#define SW1_ENCINFO_GET_sw1_enckidx(reg32)                     REG_FLD_GET(SW1_ENCINFO_FLD_sw1_enckidx, (reg32))
#define SW1_ENCINFO_GET_sw1_encdic(reg32)                      REG_FLD_GET(SW1_ENCINFO_FLD_sw1_encdic, (reg32))

#if defined(TCSUPPORT_CPU_AN7583)
#define TX_FCS_TBL_CLEAR_GET_fcs_clr_tcont_done(reg32)         REG_FLD_GET(TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont_done, (reg32))
#define TX_FCS_TBL_CLEAR_GET_fcs_clr_tcont_en(reg32)           REG_FLD_GET(TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont_en, (reg32))
#define TX_FCS_TBL_CLEAR_GET_fcs_clr_tcont(reg32)              REG_FLD_GET(TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont, (reg32))
#endif

#define MIB_CTRL_GET_mib_frame_type(reg32)                     REG_FLD_GET(MIB_CTRL_FLD_mib_frame_type, (reg32))
#define MIB_CTRL_GET_mib_en(reg32)                             REG_FLD_GET(MIB_CTRL_FLD_mib_en, (reg32))

#define MIB_TBL_CONFIG_GET_mib_tbl_rd_clr(reg32)               REG_FLD_GET(MIB_TBL_CONFIG_FLD_mib_tbl_rd_clr, (reg32))
#define MIB_TBL_CONFIG_GET_mib_tbl_init_done(reg32)            REG_FLD_GET(MIB_TBL_CONFIG_FLD_mib_tbl_init_done, (reg32))
#define MIB_TBL_CONFIG_GET_mib_tbl_init_start(reg32)           REG_FLD_GET(MIB_TBL_CONFIG_FLD_mib_tbl_init_start, (reg32))

#define MIB_CTRL_STS_GET_mib_cmd_done(reg32)                   REG_FLD_GET(MIB_CTRL_STS_FLD_mib_cmd_done, (reg32))
#define MIB_CTRL_STS_GET_mib_wr(reg32)                         REG_FLD_GET(MIB_CTRL_STS_FLD_mib_wr, (reg32))
#define MIB_CTRL_STS_GET_mib_type(reg32)                       REG_FLD_GET(MIB_CTRL_STS_FLD_mib_type, (reg32))
#define MIB_CTRL_STS_GET_mib_gpid(reg32)                       REG_FLD_GET(MIB_CTRL_STS_FLD_mib_gpid, (reg32))

#define MIB_RDATA_L32_GET_mib_rdata_l32(reg32)                 REG_FLD_GET(MIB_RDATA_L32_FLD_mib_rdata_l32, (reg32))

#define MIB_RDATA_H32_GET_mib_rdata_h32(reg32)                 REG_FLD_GET(MIB_RDATA_H32_FLD_mib_rdata_h32, (reg32))

#define MIB_WDATA_L32_GET_mib_wdata_l32(reg32)                 REG_FLD_GET(MIB_WDATA_L32_FLD_mib_wdata_l32, (reg32))

#define MIB_WDATA_H32_GET_mib_wdata_h32(reg32)                 REG_FLD_GET(MIB_WDATA_H32_FLD_mib_wdata_h32, (reg32))

#define TOD_CLK_PERIOD_GET_tod_period(reg32)                   REG_FLD_GET(TOD_CLK_PERIOD_FLD_tod_period, (reg32))

#define TOD_1PPS_WD_CTRL_GET_tod_1pps_width_ctrl(reg32)        REG_FLD_GET(TOD_1PPS_WD_CTRL_FLD_tod_1pps_width_ctrl, (reg32))

#define CUR_TOD_SEC_GET_cur_tod_sec(reg32)                     REG_FLD_GET(CUR_TOD_SEC_FLD_cur_tod_sec, (reg32))

#define CUR_TOD_NANO_SEC_GET_cur_tod_nano_sec(reg32)           REG_FLD_GET(CUR_TOD_NANO_SEC_FLD_cur_tod_nano_sec, (reg32))

#define TOD_SFC_L_GET_tod_spf_cnt_l(reg32)                     REG_FLD_GET(TOD_SFC_L_FLD_tod_spf_cnt_l, (reg32))

#define TOD_SFC_H_GET_tod_spf_cnt_h(reg32)                     REG_FLD_GET(TOD_SFC_H_FLD_tod_spf_cnt_h, (reg32))

#define TOD_UPD_CTRL_GET_tod_upd_en(reg32)                     REG_FLD_GET(TOD_UPD_CTRL_FLD_tod_upd_en, (reg32))

#define NEW_TOD_SEC_GET_new_tod_sec(reg32)                     REG_FLD_GET(NEW_TOD_SEC_FLD_new_tod_sec, (reg32))

#define NEW_TOD_NANO_SEC_GET_new_tod_nano_sec(reg32)           REG_FLD_GET(NEW_TOD_NANO_SEC_FLD_new_tod_nano_sec, (reg32))

#define SLEEP_CFG_GET_reload_sleep_cnt(reg32)                  REG_FLD_GET(SLEEP_CFG_FLD_reload_sleep_cnt, (reg32))

#define SLEEP_CNT_GET_sleep_cnt(reg32)                         REG_FLD_GET(SLEEP_CNT_FLD_sleep_cnt, (reg32))

#define TURNING_SFC_L_GET_turning_spf_cnt_l(reg32)             REG_FLD_GET(TURNING_SFC_L_FLD_turning_spf_cnt_l, (reg32))

#define TURNING_SFC_H_GET_turning_spf_cnt_h(reg32)             REG_FLD_GET(TURNING_SFC_H_FLD_turning_spf_cnt_h, (reg32))

#define DBG_CAP_SETTING_GET_hw_ack_ploamu_code(reg32)          REG_FLD_GET(DBG_CAP_SETTING_FLD_hw_ack_ploamu_code, (reg32))
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_GET_force_o2349_zero_gsize_no_dbru(reg32) REG_FLD_GET(DBG_CAP_SETTING_FLD_force_o2349_zero_gsize_no_dbru, (reg32))
#define DBG_CAP_SETTING_GET_ds_bip_chk_en(reg32)               REG_FLD_GET(DBG_CAP_SETTING_FLD_ds_bip_chk_en, (reg32))
#define DBG_CAP_SETTING_GET_ds_4b_trailer_en(reg32)            REG_FLD_GET(DBG_CAP_SETTING_FLD_ds_4b_trailer_en, (reg32))
#define DBG_CAP_SETTING_GET_tx_deft_gpid_enc_sts_ctrl_by_rx(reg32) REG_FLD_GET(DBG_CAP_SETTING_FLD_tx_deft_gpid_enc_sts_ctrl_by_rx, (reg32))
#define DBG_CAP_SETTING_GET_flt_key_invld_gem(reg32)           REG_FLD_GET(DBG_CAP_SETTING_FLD_flt_key_invld_gem, (reg32))
#define DBG_CAP_SETTING_GET_flt_o2349_continue_gnt(reg32)      REG_FLD_GET(DBG_CAP_SETTING_FLD_flt_o2349_continue_gnt, (reg32))
#define DBG_CAP_SETTING_GET_flt_o2349_no_ploamu_gnt(reg32)     REG_FLD_GET(DBG_CAP_SETTING_FLD_flt_o2349_no_ploamu_gnt, (reg32))
#endif
#define DBG_CAP_SETTING_GET_us_aes_seq_num_14b(reg32)          REG_FLD_GET(DBG_CAP_SETTING_FLD_us_aes_seq_num_14b, (reg32))
#define DBG_CAP_SETTING_GET_o52_idle_only_en(reg32)            REG_FLD_GET(DBG_CAP_SETTING_FLD_o52_idle_only_en, (reg32))
#define DBG_CAP_SETTING_GET_tx_send_only_in_o23459(reg32)      REG_FLD_GET(DBG_CAP_SETTING_FLD_tx_send_only_in_o23459, (reg32))
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_GET_flt_o49_no_deft_tcont_gnt(reg32)   REG_FLD_GET(DBG_CAP_SETTING_FLD_flt_o49_no_deft_tcont_gnt, (reg32))
#else
#define DBG_CAP_SETTING_GET_flt_dbru_set_gnt_o2349(reg32)      REG_FLD_GET(DBG_CAP_SETTING_FLD_flt_dbru_set_gnt_o2349, (reg32))
#endif
#define DBG_CAP_SETTING_GET_send_ranging_ack_o4(reg32)         REG_FLD_GET(DBG_CAP_SETTING_FLD_send_ranging_ack_o4, (reg32))
#define DBG_CAP_SETTING_GET_hw_cal_ds_omci_mic(reg32)          REG_FLD_GET(DBG_CAP_SETTING_FLD_hw_cal_ds_omci_mic, (reg32))
#define DBG_CAP_SETTING_GET_hw_cal_us_omci_mic(reg32)          REG_FLD_GET(DBG_CAP_SETTING_FLD_hw_cal_us_omci_mic, (reg32))
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_GET_flt_o2349_no_ploamu_only_gnt(reg32) REG_FLD_GET(DBG_CAP_SETTING_FLD_flt_o2349_no_ploamu_only_gnt, (reg32))
#else
#define DBG_CAP_SETTING_GET_flt_no_zero_gsize_gnt_o2349(reg32) REG_FLD_GET(DBG_CAP_SETTING_FLD_flt_no_zero_gsize_gnt_o2349, (reg32))
#endif
#define DBG_CAP_SETTING_GET_rept_msg_flt(reg32)                REG_FLD_GET(DBG_CAP_SETTING_FLD_rept_msg_flt, (reg32))
#define DBG_CAP_SETTING_GET_rept_prof_flt(reg32)               REG_FLD_GET(DBG_CAP_SETTING_FLD_rept_prof_flt, (reg32))

#define DBG_BWM_CHK_CTRL_GET_min_bst_intvl(reg32)              REG_FLD_GET(DBG_BWM_CHK_CTRL_FLD_min_bst_intvl, (reg32))
#define DBG_BWM_CHK_CTRL_GET_max_bst_len_err_cut(reg32)        REG_FLD_GET(DBG_BWM_CHK_CTRL_FLD_max_bst_len_err_cut, (reg32))
#define DBG_BWM_CHK_CTRL_GET_max_my_bst_num_err_cut(reg32)     REG_FLD_GET(DBG_BWM_CHK_CTRL_FLD_max_my_bst_num_err_cut, (reg32))
#define DBG_BWM_CHK_CTRL_GET_max_my_alloc_num_err_cut(reg32)   REG_FLD_GET(DBG_BWM_CHK_CTRL_FLD_max_my_alloc_num_err_cut, (reg32))
#define DBG_BWM_CHK_CTRL_GET_max_bst_alloc_num_err_cut(reg32)  REG_FLD_GET(DBG_BWM_CHK_CTRL_FLD_max_bst_alloc_num_err_cut, (reg32))
#define DBG_BWM_CHK_CTRL_GET_max_bwm_alloc_num_err_cut(reg32)  REG_FLD_GET(DBG_BWM_CHK_CTRL_FLD_max_bwm_alloc_num_err_cut, (reg32))
#define DBG_BWM_CHK_CTRL_GET_min_gsize_flt(reg32)              REG_FLD_GET(DBG_BWM_CHK_CTRL_FLD_min_gsize_flt, (reg32))
#define DBG_BWM_CHK_CTRL_GET_max_gsize_flt(reg32)              REG_FLD_GET(DBG_BWM_CHK_CTRL_FLD_max_gsize_flt, (reg32))
#define DBG_BWM_CHK_CTRL_GET_start_time_order_flt(reg32)       REG_FLD_GET(DBG_BWM_CHK_CTRL_FLD_start_time_order_flt, (reg32))
#define DBG_BWM_CHK_CTRL_GET_max_start_time_flt(reg32)         REG_FLD_GET(DBG_BWM_CHK_CTRL_FLD_max_start_time_flt, (reg32))
#define DBG_BWM_CHK_CTRL_GET_min_bst_intvl_flt(reg32)          REG_FLD_GET(DBG_BWM_CHK_CTRL_FLD_min_bst_intvl_flt, (reg32))

#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_BWM_CKH_STS_GET_o2349_continue_gnt_recv(reg32)     REG_FLD_GET(DBG_BWM_CKH_STS_FLD_o2349_continue_gnt_recv, (reg32))
#define DBG_BWM_CKH_STS_GET_o49_no_deft_tcont_gnt_recv(reg32)  REG_FLD_GET(DBG_BWM_CKH_STS_FLD_o49_no_deft_tcont_gnt_recv, (reg32))
#define DBG_BWM_CKH_STS_GET_o2349_no_ploamu_only_gnt_recv(reg32) REG_FLD_GET(DBG_BWM_CKH_STS_FLD_o2349_no_ploamu_only_gnt_recv, (reg32))
#define DBG_BWM_CKH_STS_GET_o2349_no_ploamu_gnt_recv(reg32)    REG_FLD_GET(DBG_BWM_CKH_STS_FLD_o2349_no_ploamu_gnt_recv, (reg32))
#else
#define DBG_BWM_CKH_STS_GET_o23_o4_data_gnt_recv(reg32)        REG_FLD_GET(DBG_BWM_CKH_STS_FLD_o23_o4_data_gnt_recv, (reg32))
#endif
#define DBG_BWM_CKH_STS_GET_other_tid_ins_my_bst_err(reg32)    REG_FLD_GET(DBG_BWM_CKH_STS_FLD_other_tid_ins_my_bst_err, (reg32))
#define DBG_BWM_CKH_STS_GET_max_bst_len_err(reg32)             REG_FLD_GET(DBG_BWM_CKH_STS_FLD_max_bst_len_err, (reg32))
#define DBG_BWM_CKH_STS_GET_max_my_bst_num_err(reg32)          REG_FLD_GET(DBG_BWM_CKH_STS_FLD_max_my_bst_num_err, (reg32))
#define DBG_BWM_CKH_STS_GET_max_my_alloc_num_err(reg32)        REG_FLD_GET(DBG_BWM_CKH_STS_FLD_max_my_alloc_num_err, (reg32))
#define DBG_BWM_CKH_STS_GET_max_bst_alloc_num_err(reg32)       REG_FLD_GET(DBG_BWM_CKH_STS_FLD_max_bst_alloc_num_err, (reg32))
#define DBG_BWM_CKH_STS_GET_max_bwm_alloc_num_err(reg32)       REG_FLD_GET(DBG_BWM_CKH_STS_FLD_max_bwm_alloc_num_err, (reg32))
#define DBG_BWM_CKH_STS_GET_alloc_hec_uc_err(reg32)            REG_FLD_GET(DBG_BWM_CKH_STS_FLD_alloc_hec_uc_err, (reg32))
#define DBG_BWM_CKH_STS_GET_bst_split_err(reg32)               REG_FLD_GET(DBG_BWM_CKH_STS_FLD_bst_split_err, (reg32))
#define DBG_BWM_CKH_STS_GET_my_tid_ins_bst_err(reg32)          REG_FLD_GET(DBG_BWM_CKH_STS_FLD_my_tid_ins_bst_err, (reg32))
#define DBG_BWM_CKH_STS_GET_min_gsize_err(reg32)               REG_FLD_GET(DBG_BWM_CKH_STS_FLD_min_gsize_err, (reg32))
#define DBG_BWM_CKH_STS_GET_max_gsize_err(reg32)               REG_FLD_GET(DBG_BWM_CKH_STS_FLD_max_gsize_err, (reg32))
#define DBG_BWM_CKH_STS_GET_start_time_order_err(reg32)        REG_FLD_GET(DBG_BWM_CKH_STS_FLD_start_time_order_err, (reg32))
#define DBG_BWM_CKH_STS_GET_max_start_time_err(reg32)          REG_FLD_GET(DBG_BWM_CKH_STS_FLD_max_start_time_err, (reg32))
#define DBG_BWM_CKH_STS_GET_min_bst_intvl_err(reg32)           REG_FLD_GET(DBG_BWM_CKH_STS_FLD_min_bst_intvl_err, (reg32))

#define DBG_BWM_SFIFO_STS_GET_sgl_fifo_max_used(reg32)         REG_FLD_GET(DBG_BWM_SFIFO_STS_FLD_sgl_fifo_max_used, (reg32))
#define DBG_BWM_SFIFO_STS_GET_sgl_fifo_used(reg32)             REG_FLD_GET(DBG_BWM_SFIFO_STS_FLD_sgl_fifo_used, (reg32))

#define DBG_BWM_BFIFO_STS_GET_bst_fifo_max_used(reg32)         REG_FLD_GET(DBG_BWM_BFIFO_STS_FLD_bst_fifo_max_used, (reg32))
#define DBG_BWM_BFIFO_STS_GET_bst_fifo_used(reg32)             REG_FLD_GET(DBG_BWM_BFIFO_STS_FLD_bst_fifo_used, (reg32))

#define DBG_TX_ALIGN_FIFO_STS_GET_tx_align_fifo_max_used(reg32) REG_FLD_GET(DBG_TX_ALIGN_FIFO_STS_FLD_tx_align_fifo_max_used, (reg32))
#define DBG_TX_ALIGN_FIFO_STS_GET_tx_align_fifo_used(reg32)    REG_FLD_GET(DBG_TX_ALIGN_FIFO_STS_FLD_tx_align_fifo_used, (reg32))

#define DBG_TX_AES_MAX_USED_GET_tx_aes_eng_max_used(reg32)     REG_FLD_GET(DBG_TX_AES_MAX_USED_FLD_tx_aes_eng_max_used, (reg32))
#define DBG_TX_AES_MAX_USED_GET_tx_rdm_ciph_fifo_max_used(reg32) REG_FLD_GET(DBG_TX_AES_MAX_USED_FLD_tx_rdm_ciph_fifo_max_used, (reg32))
#define DBG_TX_AES_MAX_USED_GET_tx_cryp_cnt_fifo_max_used(reg32) REG_FLD_GET(DBG_TX_AES_MAX_USED_FLD_tx_cryp_cnt_fifo_max_used, (reg32))
#define DBG_TX_AES_MAX_USED_GET_tx_ciph_txt_fifo_max_used(reg32) REG_FLD_GET(DBG_TX_AES_MAX_USED_FLD_tx_ciph_txt_fifo_max_used, (reg32))
#define DBG_TX_AES_MAX_USED_GET_tx_aes_hdr_fifo_max_used(reg32) REG_FLD_GET(DBG_TX_AES_MAX_USED_FLD_tx_aes_hdr_fifo_max_used, (reg32))


#if defined(TCSUPPORT_CPU_AN7583)
#define DBG_DBA_BACK_DOOR_GET_dba_modify_tcont_idx_1_mode(reg32) REG_FLD_GET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1_mode, (reg32))
#define DBG_DBA_BACK_DOOR_GET_dba_modify_tcont_idx_1_ovrw(reg32) REG_FLD_GET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1_ovrw, (reg32))
#define DBG_DBA_BACK_DOOR_GET_dba_modify_tcont_idx_1(reg32)    REG_FLD_GET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1, (reg32))
#define DBG_DBA_BACK_DOOR_GET_dba_modify_tcont_idx_0_mode(reg32) REG_FLD_GET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0_mode, (reg32))
#define DBG_DBA_BACK_DOOR_GET_dba_modify_tcont_idx_0_ovrw(reg32) REG_FLD_GET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0_ovrw, (reg32))
#define DBG_DBA_BACK_DOOR_GET_dba_modify_tcont_idx_0(reg32)    REG_FLD_GET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0, (reg32))
#endif

#define DBG_DBA_BACK_DOOR_GET_dba_modify_mode(reg32)           REG_FLD_GET(DBG_DBA_BACK_DOOR_FLD_dba_modify_mode, (reg32))

#define DBG_DBA_MODIFY_GET_dba_modify(reg32)                   REG_FLD_GET(DBG_DBA_MODIFY_FLD_dba_modify, (reg32))

#define DBG_RESYNC_GET_tx_sync_rdy(reg32)                      REG_FLD_GET(DBG_RESYNC_FLD_tx_sync_rdy, (reg32))
#define DBG_RESYNC_GET_sw_resync_en(reg32)                     REG_FLD_GET(DBG_RESYNC_FLD_sw_resync_en, (reg32))
#define DBG_RESYNC_GET_sw_resync_start(reg32)                  REG_FLD_GET(DBG_RESYNC_FLD_sw_resync_start, (reg32))

#define DBG_PHY_DLY_CTRL_GET_use_mpi_rx_phy_dly_ofs(reg32)     REG_FLD_GET(DBG_PHY_DLY_CTRL_FLD_use_mpi_rx_phy_dly_ofs, (reg32))
#define DBG_PHY_DLY_CTRL_GET_o4_o5_same_phy_dly(reg32)         REG_FLD_GET(DBG_PHY_DLY_CTRL_FLD_o4_o5_same_phy_dly, (reg32))

#define DBG_DS_SPF_CNT_L_GET_ds_spf_cnt_l32(reg32)             REG_FLD_GET(DBG_DS_SPF_CNT_L_FLD_ds_spf_cnt_l32, (reg32))

#define DBG_DS_SPF_CNT_H_GET_ds_spf_cnt_h19(reg32)             REG_FLD_GET(DBG_DS_SPF_CNT_H_FLD_ds_spf_cnt_h19, (reg32))

#define DBG_PON_ID_L_GET_ds_pon_id_l32(reg32)                  REG_FLD_GET(DBG_PON_ID_L_FLD_ds_pon_id_l32, (reg32))

#define DBG_PON_ID_H_GET_ds_pon_id_h19(reg32)                  REG_FLD_GET(DBG_PON_ID_H_FLD_ds_pon_id_h19, (reg32))

#define DBG_TX_SYNC_OFFSET_GET_tx_sync_offset(reg32)           REG_FLD_GET(DBG_TX_SYNC_OFFSET_FLD_tx_sync_offset, (reg32))

#define DBG_RX_PHY_DLY_OFS_GET_rx_phy_dly_offset(reg32)        REG_FLD_GET(DBG_RX_PHY_DLY_OFS_FLD_rx_phy_dly_offset, (reg32))

#define DBG_RX_HEC_ERR_GET_rx_hec_err_sts(reg32)               REG_FLD_GET(DBG_RX_HEC_ERR_FLD_rx_hec_err_sts, (reg32))
#define DBG_RX_HEC_ERR_GET_rx_hdr_hec_3err(reg32)              REG_FLD_GET(DBG_RX_HEC_ERR_FLD_rx_hdr_hec_3err, (reg32))
#define DBG_RX_HEC_ERR_GET_rx_hdr_hec_2err(reg32)              REG_FLD_GET(DBG_RX_HEC_ERR_FLD_rx_hdr_hec_2err, (reg32))
#define DBG_RX_HEC_ERR_GET_rx_hdr_hec_1err(reg32)              REG_FLD_GET(DBG_RX_HEC_ERR_FLD_rx_hdr_hec_1err, (reg32))
#define DBG_RX_HEC_ERR_GET_rx_alloc_hec_3err(reg32)            REG_FLD_GET(DBG_RX_HEC_ERR_FLD_rx_alloc_hec_3err, (reg32))
#define DBG_RX_HEC_ERR_GET_rx_alloc_hec_2err(reg32)            REG_FLD_GET(DBG_RX_HEC_ERR_FLD_rx_alloc_hec_2err, (reg32))
#define DBG_RX_HEC_ERR_GET_rx_alloc_hec_1err(reg32)            REG_FLD_GET(DBG_RX_HEC_ERR_FLD_rx_alloc_hec_1err, (reg32))
#define DBG_RX_HEC_ERR_GET_rx_hlend_hec_3err(reg32)            REG_FLD_GET(DBG_RX_HEC_ERR_FLD_rx_hlend_hec_3err, (reg32))
#define DBG_RX_HEC_ERR_GET_rx_hlend_hec_2err(reg32)            REG_FLD_GET(DBG_RX_HEC_ERR_FLD_rx_hlend_hec_2err, (reg32))
#define DBG_RX_HEC_ERR_GET_rx_hlend_hec_1err(reg32)            REG_FLD_GET(DBG_RX_HEC_ERR_FLD_rx_hlend_hec_1err, (reg32))

#define RX_GEM_ENC_CHK_GET_rx_enc_chk_err_sts(reg32)           REG_FLD_GET(RX_GEM_ENC_CHK_FLD_rx_enc_chk_err_sts, (reg32))
#define RX_GEM_ENC_CHK_GET_rx_enc_chk_done(reg32)              REG_FLD_GET(RX_GEM_ENC_CHK_FLD_rx_enc_chk_done, (reg32))
#define RX_GEM_ENC_CHK_GET_rx_enc_chk_en(reg32)                REG_FLD_GET(RX_GEM_ENC_CHK_FLD_rx_enc_chk_en, (reg32))
#define RX_GEM_ENC_CHK_GET_exp_rx_enc_sts(reg32)               REG_FLD_GET(RX_GEM_ENC_CHK_FLD_exp_rx_enc_sts, (reg32))
#define RX_GEM_ENC_CHK_GET_rx_enc_chk_gpid(reg32)              REG_FLD_GET(RX_GEM_ENC_CHK_FLD_rx_enc_chk_gpid, (reg32))

#define RX_MBI_FIFO_DROP_EN_GET_rx_mbi_fifo_drop_en(reg32)     REG_FLD_GET(RX_MBI_FIFO_DROP_EN_FLD_rx_mbi_fifo_drop_en, (reg32))

#define RX_MBI_HDR_FIFO_DROP_CTRL_GET_rx_mbi_hdr_fifo_hth(reg32) REG_FLD_GET(RX_MBI_HDR_FIFO_DROP_CTRL_FLD_rx_mbi_hdr_fifo_hth, (reg32))
#define RX_MBI_HDR_FIFO_DROP_CTRL_GET_rx_mbi_hdr_fifo_lth(reg32) REG_FLD_GET(RX_MBI_HDR_FIFO_DROP_CTRL_FLD_rx_mbi_hdr_fifo_lth, (reg32))

#define RX_MBI_PL_FIFO_DROP_CTRL_GET_rx_mbi_pl_fifo_hth(reg32) REG_FLD_GET(RX_MBI_PL_FIFO_DROP_CTRL_FLD_rx_mbi_pl_fifo_hth, (reg32))
#define RX_MBI_PL_FIFO_DROP_CTRL_GET_rx_mbi_pl_fifo_lth(reg32) REG_FLD_GET(RX_MBI_PL_FIFO_DROP_CTRL_FLD_rx_mbi_pl_fifo_lth, (reg32))

#if defined(TCSUPPORT_CPU_EN7581)
#define GET_RX_IDLE_XGEM_GET_get_rx_idle_xgem(reg32)           REG_FLD_GET(GET_RX_IDLE_XGEM_FLD_get_rx_idle_xgem, (reg32))
#define GET_RX_IDLE_XGEM_GET_rx_idle_xgem_len(reg32)           REG_FLD_GET(GET_RX_IDLE_XGEM_FLD_rx_idle_xgem_len, (reg32))

#define RX_IDLE_XGEM_1ST_PL_GET_rx_idle_xgem_1st_pl(reg32)     REG_FLD_GET(RX_IDLE_XGEM_1ST_PL_FLD_rx_idle_xgem_1st_pl, (reg32))

#endif

#if defined(TCSUPPORT_CPU_AN7583)
#define DBG_DBA_MODIFY_IDX_0_GET_dba_modify_idx_0(reg32)       REG_FLD_GET(DBG_DBA_MODIFY_IDX_0_FLD_dba_modify_idx_0, (reg32))

#define DBG_DBA_MODIFY_IDX_1_GET_dba_modify_idx_1(reg32)       REG_FLD_GET(DBG_DBA_MODIFY_IDX_1_FLD_dba_modify_idx_1, (reg32))
#endif

#define CNT_CLR_GET_nml_cnt_clr(reg32)                         REG_FLD_GET(CNT_CLR_FLD_nml_cnt_clr, (reg32))
#define CNT_CLR_GET_err_cnt_clr(reg32)                         REG_FLD_GET(CNT_CLR_FLD_err_cnt_clr, (reg32))

#define RX_HLEND_HEC_CNT_GET_rx_hlend_hec_3err_cnt(reg32)      REG_FLD_GET(RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_3err_cnt, (reg32))
#define RX_HLEND_HEC_CNT_GET_rx_hlend_hec_2err_cnt(reg32)      REG_FLD_GET(RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_2err_cnt, (reg32))
#define RX_HLEND_HEC_CNT_GET_rx_hlend_hec_1err_cnt(reg32)      REG_FLD_GET(RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_1err_cnt, (reg32))

#define RX_ALLOC_HEC_CNT_GET_rx_alloc_hec_3err_cnt(reg32)      REG_FLD_GET(RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_3err_cnt, (reg32))
#define RX_ALLOC_HEC_CNT_GET_rx_alloc_hec_2err_cnt(reg32)      REG_FLD_GET(RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_2err_cnt, (reg32))
#define RX_ALLOC_HEC_CNT_GET_rx_alloc_hec_1err_cnt(reg32)      REG_FLD_GET(RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_1err_cnt, (reg32))

#define RX_HDR_HEC_CNT_GET_rx_hdr_hec_3err_cnt(reg32)          REG_FLD_GET(RX_HDR_HEC_CNT_FLD_rx_hdr_hec_3err_cnt, (reg32))
#define RX_HDR_HEC_CNT_GET_rx_hdr_hec_2err_cnt(reg32)          REG_FLD_GET(RX_HDR_HEC_CNT_FLD_rx_hdr_hec_2err_cnt, (reg32))
#define RX_HDR_HEC_CNT_GET_rx_hdr_hec_1err_cnt(reg32)          REG_FLD_GET(RX_HDR_HEC_CNT_FLD_rx_hdr_hec_1err_cnt, (reg32))

#define RX_PHY_HEC_ERR_CNT_GET_rx_pon_id_hec_err_cnt(reg32)    REG_FLD_GET(RX_PHY_HEC_ERR_CNT_FLD_rx_pon_id_hec_err_cnt, (reg32))
#define RX_PHY_HEC_ERR_CNT_GET_rx_sfc_hec_err_cnt(reg32)       REG_FLD_GET(RX_PHY_HEC_ERR_CNT_FLD_rx_sfc_hec_err_cnt, (reg32))

#define RX_MIC_ERR_CNT_GET_rx_omci_mic_err_cnt(reg32)          REG_FLD_GET(RX_MIC_ERR_CNT_FLD_rx_omci_mic_err_cnt, (reg32))
#define RX_MIC_ERR_CNT_GET_rx_ploam_mic_err_cnt(reg32)         REG_FLD_GET(RX_MIC_ERR_CNT_FLD_rx_ploam_mic_err_cnt, (reg32))

#define RX_ETH_FCS_ERR_CNT_GET_rx_eth_crc_err_cnt(reg32)       REG_FLD_GET(RX_ETH_FCS_ERR_CNT_FLD_rx_eth_crc_err_cnt, (reg32))

#define RX_BIP_ERR_CNT_GET_rx_bip_err_cnt(reg32)               REG_FLD_GET(RX_BIP_ERR_CNT_FLD_rx_bip_err_cnt, (reg32))

#define RX_KEY_ERR_CNT_GET_rx_key_err_cnt(reg32)               REG_FLD_GET(RX_KEY_ERR_CNT_FLD_rx_key_err_cnt, (reg32))

#define RX_LOST_WCNT_GET_rx_lost_wcnt(reg32)                   REG_FLD_GET(RX_LOST_WCNT_FLD_rx_lost_wcnt, (reg32))

#define INVLD_PROF_BST_GNT_CNT_GET_invld_prof_bst_gnt_cnt(reg32) REG_FLD_GET(INVLD_PROF_BST_GNT_CNT_FLD_invld_prof_bst_gnt_cnt, (reg32))

#define RX_MBI_XGEM_DROP_CNT_GET_rx_mbi_xgem_drop_cnt(reg32)   REG_FLD_GET(RX_MBI_XGEM_DROP_CNT_FLD_rx_mbi_xgem_drop_cnt, (reg32))

#define RX_XGTC_CNT_GET_rx_mpi_sof_cnt(reg32)                  REG_FLD_GET(RX_XGTC_CNT_FLD_rx_mpi_sof_cnt, (reg32))

#define TX_BST_CNT_GET_tx_mpi_sof_cnt(reg32)                   REG_FLD_GET(TX_BST_CNT_FLD_tx_mpi_sof_cnt, (reg32))

#define RX_PLOAMD_CNT_GET_rx_ploamd_cnt(reg32)                 REG_FLD_GET(RX_PLOAMD_CNT_FLD_rx_ploamd_cnt, (reg32))

#define TX_PLOAMU_CNT_GET_tx_ploamu_cnt(reg32)                 REG_FLD_GET(TX_PLOAMU_CNT_FLD_tx_ploamu_cnt, (reg32))

#define RX_OMCI_CNT_GET_rx_omci_cnt_fe(reg32)                  REG_FLD_GET(RX_OMCI_CNT_FLD_rx_omci_cnt_fe, (reg32))
#define RX_OMCI_CNT_GET_rx_omci_cnt_mac(reg32)                 REG_FLD_GET(RX_OMCI_CNT_FLD_rx_omci_cnt_mac, (reg32))

#define TX_OMCI_CNT_GET_tx_omci_cnt_fe(reg32)                  REG_FLD_GET(TX_OMCI_CNT_FLD_tx_omci_cnt_fe, (reg32))
#define TX_OMCI_CNT_GET_tx_omci_cnt_mac(reg32)                 REG_FLD_GET(TX_OMCI_CNT_FLD_tx_omci_cnt_mac, (reg32))

#define RX_XGEM_CNT_GET_rx_xgem_cnt(reg32)                     REG_FLD_GET(RX_XGEM_CNT_FLD_rx_xgem_cnt, (reg32))

#define TX_XGEM_CNT_GET_tx_xgem_cnt(reg32)                     REG_FLD_GET(TX_XGEM_CNT_FLD_tx_xgem_cnt, (reg32))

#define RX_MBI_CNT_GET_rx_mbi_ack_cnt(reg32)                   REG_FLD_GET(RX_MBI_CNT_FLD_rx_mbi_ack_cnt, (reg32))

#define TX_MBI_CNT_GET_tx_mbi_ack_cnt(reg32)                   REG_FLD_GET(TX_MBI_CNT_FLD_tx_mbi_ack_cnt, (reg32))

#define RX_NON_IDLE_BCNT_GET_rx_non_idle_bcnt(reg32)           REG_FLD_GET(RX_NON_IDLE_BCNT_FLD_rx_non_idle_bcnt, (reg32))

#define TX_NON_IDLE_BCNT_GET_tx_non_idle_bcnt(reg32)           REG_FLD_GET(TX_NON_IDLE_BCNT_FLD_tx_non_idle_bcnt, (reg32))

#define TX_NLF_XGEM_CNT_GET_tx_nlf_xgem_cnt(reg32)             REG_FLD_GET(TX_NLF_XGEM_CNT_FLD_tx_nlf_xgem_cnt, (reg32))

#define TX_ACK_PLOAMU_CNT_GET_tx_ack_ploamu_cnt(reg32)         REG_FLD_GET(TX_ACK_PLOAMU_CNT_FLD_tx_ack_ploamu_cnt, (reg32))

#define RX_BIP_PROTECT_WCNT_GET_rx_bip_protect_wcnt(reg32)     REG_FLD_GET(RX_BIP_PROTECT_WCNT_FLD_rx_bip_protect_wcnt, (reg32))

#define TX_IDLE_BCNT_GET_tx_idle_bcnt(reg32)                   REG_FLD_GET(TX_IDLE_BCNT_FLD_tx_idle_bcnt, (reg32))

#if defined(TCSUPPORT_CPU_AN7583)
#define HUNT_AFTER_LOSGD_GET_hunt_after_losgd_en(reg32)        REG_FLD_GET(HUNT_AFTER_LOSGD_FLD_hunt_after_losgd_en, (reg32))

#define BWMAP_LEN_LIMIT_GET_max_alloc_num(reg32)               REG_FLD_GET(BWMAP_LEN_LIMIT_FLD_max_alloc_num, (reg32))
#define BWMAP_LEN_LIMIT_GET_max_my_alloc_num(reg32)            REG_FLD_GET(BWMAP_LEN_LIMIT_FLD_max_my_alloc_num, (reg32))
#define BWMAP_LEN_LIMIT_GET_max_bst_alloc_num(reg32)           REG_FLD_GET(BWMAP_LEN_LIMIT_FLD_max_bst_alloc_num, (reg32))
#define BWMAP_LEN_LIMIT_GET_max_my_bst_num(reg32)              REG_FLD_GET(BWMAP_LEN_LIMIT_FLD_max_my_bst_num, (reg32))

#define XGEM_HEADER_OVRW_GET_rx_gem_3port_interleave(reg32)    REG_FLD_GET(XGEM_HEADER_OVRW_FLD_rx_gem_3port_interleave, (reg32))
#define XGEM_HEADER_OVRW_GET_rx_omci_interleave(reg32)         REG_FLD_GET(XGEM_HEADER_OVRW_FLD_rx_omci_interleave, (reg32))
#define XGEM_HEADER_OVRW_GET_nml_header_ovrw_en(reg32)         REG_FLD_GET(XGEM_HEADER_OVRW_FLD_nml_header_ovrw_en, (reg32))
#define XGEM_HEADER_OVRW_GET_omci_header_ovrw_en(reg32)        REG_FLD_GET(XGEM_HEADER_OVRW_FLD_omci_header_ovrw_en, (reg32))

#define OMCI_LEN_CTRL_GET_max_omci_len(reg32)                  REG_FLD_GET(OMCI_LEN_CTRL_FLD_max_omci_len, (reg32))
#endif

#define SNF_CTRL_GET_snf_us_en(reg32)                          REG_FLD_GET(SNF_CTRL_FLD_snf_us_en, (reg32))
#define SNF_CTRL_GET_snf_ds_en(reg32)                          REG_FLD_GET(SNF_CTRL_FLD_snf_ds_en, (reg32))
#if defined(TCSUPPORT_CPU_AN7583)
#define SNF_CTRL_GET_snf_us_ovrw_8b_xgem(reg32)                REG_FLD_GET(SNF_CTRL_FLD_snf_us_ovrw_8b_xgem, (reg32))
#endif
#define SNF_CTRL_GET_snf_us_ovrw_my_xgem(reg32)                REG_FLD_GET(SNF_CTRL_FLD_snf_us_ovrw_my_xgem, (reg32))

#define SNF_GPID_GET_us_snf_gpid(reg32)                        REG_FLD_GET(SNF_GPID_FLD_us_snf_gpid, (reg32))
#define SNF_GPID_GET_ds_snf_gpid(reg32)                        REG_FLD_GET(SNF_GPID_FLD_ds_snf_gpid, (reg32))

#define DS_SNF_ETH_DASA_H16_GET_ds_snf_eth_da_h16(reg32)       REG_FLD_GET(DS_SNF_ETH_DASA_H16_FLD_ds_snf_eth_da_h16, (reg32))
#define DS_SNF_ETH_DASA_H16_GET_ds_snf_eth_sa_h16(reg32)       REG_FLD_GET(DS_SNF_ETH_DASA_H16_FLD_ds_snf_eth_sa_h16, (reg32))

#define SNF_ETH_TAG_GET_snf_eth_tpid(reg32)                    REG_FLD_GET(SNF_ETH_TAG_FLD_snf_eth_tpid, (reg32))

#define SNF_ETH_TYPE_GET_snf_eth_type(reg32)                   REG_FLD_GET(SNF_ETH_TYPE_FLD_snf_eth_type, (reg32))

#define US_SNF_ETH_DASA_H16_GET_us_snf_eth_da_h16(reg32)       REG_FLD_GET(US_SNF_ETH_DASA_H16_FLD_us_snf_eth_da_h16, (reg32))
#define US_SNF_ETH_DASA_H16_GET_us_snf_eth_sa_h16(reg32)       REG_FLD_GET(US_SNF_ETH_DASA_H16_FLD_us_snf_eth_sa_h16, (reg32))

#define DBG_PROBE_CTRL_GET_probe_clk_en(reg32)                 REG_FLD_GET(DBG_PROBE_CTRL_FLD_probe_clk_en, (reg32))
#define DBG_PROBE_CTRL_GET_probe_trig_en(reg32)                REG_FLD_GET(DBG_PROBE_CTRL_FLD_probe_trig_en, (reg32))
#define DBG_PROBE_CTRL_GET_probe_trig_bit_sel(reg32)           REG_FLD_GET(DBG_PROBE_CTRL_FLD_probe_trig_bit_sel, (reg32))
#define DBG_PROBE_CTRL_GET_probe_trig_sel(reg32)               REG_FLD_GET(DBG_PROBE_CTRL_FLD_probe_trig_sel, (reg32))
#define DBG_PROBE_CTRL_GET_probe_sel(reg32)                    REG_FLD_GET(DBG_PROBE_CTRL_FLD_probe_sel, (reg32))

#define DBG_PROBE_HIGH32_GET_probe_h32(reg32)                  REG_FLD_GET(DBG_PROBE_HIGH32_FLD_probe_h32, (reg32))

#define DBG_PROBE_LOW32_GET_probe_l32(reg32)                   REG_FLD_GET(DBG_PROBE_LOW32_FLD_probe_l32, (reg32))

#define SW_RST_SET_xgpon_mac_sw_rst_n(reg32, val)              REG_FLD_SET(SW_RST_FLD_xgpon_mac_sw_rst_n, (reg32), (val))

#define MBI_MPI_STOP_SET_mpi_tx_stop_done(reg32, val)          REG_FLD_SET(MBI_MPI_STOP_FLD_mpi_tx_stop_done, (reg32), (val))
#define MBI_MPI_STOP_SET_mpi_rx_stop_done(reg32, val)          REG_FLD_SET(MBI_MPI_STOP_FLD_mpi_rx_stop_done, (reg32), (val))
#define MBI_MPI_STOP_SET_mpi_tx_stop(reg32, val)               REG_FLD_SET(MBI_MPI_STOP_FLD_mpi_tx_stop, (reg32), (val))
#define MBI_MPI_STOP_SET_mpi_rx_stop(reg32, val)               REG_FLD_SET(MBI_MPI_STOP_FLD_mpi_rx_stop, (reg32), (val))
#define MBI_MPI_STOP_SET_mbi_tx_stop_done(reg32, val)          REG_FLD_SET(MBI_MPI_STOP_FLD_mbi_tx_stop_done, (reg32), (val))
#define MBI_MPI_STOP_SET_mbi_rx_stop_done(reg32, val)          REG_FLD_SET(MBI_MPI_STOP_FLD_mbi_rx_stop_done, (reg32), (val))
#if defined(TCSUPPORT_CPU_EN7581)
#define MBI_MPI_STOP_SET_dbru_stop_done(reg32, val)            REG_FLD_SET(MBI_MPI_STOP_FLD_dbru_stop_done, (reg32), (val))
#define MBI_MPI_STOP_SET_dbru_stop(reg32, val)                 REG_FLD_SET(MBI_MPI_STOP_FLD_dbru_stop, (reg32), (val))
#endif
#define MBI_MPI_STOP_SET_mbi_tx_stop(reg32, val)               REG_FLD_SET(MBI_MPI_STOP_FLD_mbi_tx_stop, (reg32), (val))
#if defined(TCSUPPORT_CPU_EN7581)
#define MBI_MPI_STOP_SET_del_rx_stop(reg32, val)               REG_FLD_SET(MBI_MPI_STOP_FLD_del_rx_stop, (reg32), (val))
#endif
#define MBI_MPI_STOP_SET_mbi_rx_stop(reg32, val)               REG_FLD_SET(MBI_MPI_STOP_FLD_mbi_rx_stop, (reg32), (val))

#define VENDOR_ID_SET_vendor_id(reg32, val)                    REG_FLD_SET(VENDOR_ID_FLD_vendor_id, (reg32), (val))

#define VS_SN_SET_vs_sn(reg32, val)                            REG_FLD_SET(VS_SN_FLD_vs_sn, (reg32), (val))

#define ONU_ID_SET_onu_id_vld(reg32, val)                      REG_FLD_SET(ONU_ID_FLD_onu_id_vld, (reg32), (val))
#define ONU_ID_SET_onu_id(reg32, val)                          REG_FLD_SET(ONU_ID_FLD_onu_id, (reg32), (val))

#define RGS_ID3_0_SET_rgs_id3_0(reg32, val)                    REG_FLD_SET(RGS_ID3_0_FLD_rgs_id3_0, (reg32), (val))

#define RGS_ID7_4_SET_rgs_id7_4(reg32, val)                    REG_FLD_SET(RGS_ID7_4_FLD_rgs_id7_4, (reg32), (val))

#define RGS_ID11_8_SET_rgs_id11_8(reg32, val)                  REG_FLD_SET(RGS_ID11_8_FLD_rgs_id11_8, (reg32), (val))

#define RGS_ID15_12_SET_rgs_id15_12(reg32, val)                REG_FLD_SET(RGS_ID15_12_FLD_rgs_id15_12, (reg32), (val))

#define RGS_ID19_16_SET_rgs_id19_16(reg32, val)                REG_FLD_SET(RGS_ID19_16_FLD_rgs_id19_16, (reg32), (val))

#define RGS_ID23_20_SET_rgs_id23_20(reg32, val)                REG_FLD_SET(RGS_ID23_20_FLD_rgs_id23_20, (reg32), (val))

#define RGS_ID27_24_SET_rgs_id27_24(reg32, val)                REG_FLD_SET(RGS_ID27_24_FLD_rgs_id27_24, (reg32), (val))

#define RGS_ID31_28_SET_rgs_id31_28(reg32, val)                REG_FLD_SET(RGS_ID31_28_FLD_rgs_id31_28, (reg32), (val))

#define RGS_ID35_32_SET_rgs_id35_32(reg32, val)                REG_FLD_SET(RGS_ID35_32_FLD_rgs_id35_32, (reg32), (val))

#define INT_ENABLE_SET_o5_sn_onu_req_recv_int_en(reg32, val)   REG_FLD_SET(INT_ENABLE_FLD_o5_sn_onu_req_recv_int_en, (reg32), (val))
#define INT_ENABLE_SET_turning_sfc_match_int_en(reg32, val)    REG_FLD_SET(INT_ENABLE_FLD_turning_sfc_match_int_en, (reg32), (val))
#define INT_ENABLE_SET_o9_gnt_recv_int_en(reg32, val)          REG_FLD_SET(INT_ENABLE_FLD_o9_gnt_recv_int_en, (reg32), (val))
#define INT_ENABLE_SET_sw1_mic_done_int_en(reg32, val)         REG_FLD_SET(INT_ENABLE_FLD_sw1_mic_done_int_en, (reg32), (val))
#define INT_ENABLE_SET_sw0_mic_done_int_en(reg32, val)         REG_FLD_SET(INT_ENABLE_FLD_sw0_mic_done_int_en, (reg32), (val))
#define INT_ENABLE_SET_key_cal_done_int_en(reg32, val)         REG_FLD_SET(INT_ENABLE_FLD_key_cal_done_int_en, (reg32), (val))
#define INT_ENABLE_SET_lwi_int_en(reg32, val)                  REG_FLD_SET(INT_ENABLE_FLD_lwi_int_en, (reg32), (val))
#define INT_ENABLE_SET_fwi_int_en(reg32, val)                  REG_FLD_SET(INT_ENABLE_FLD_fwi_int_en, (reg32), (val))
#define INT_ENABLE_SET_rx_err_int_en(reg32, val)               REG_FLD_SET(INT_ENABLE_FLD_rx_err_int_en, (reg32), (val))
#define INT_ENABLE_SET_tx_err_int_en(reg32, val)               REG_FLD_SET(INT_ENABLE_FLD_tx_err_int_en, (reg32), (val))
#define INT_ENABLE_SET_fifo_err_int_en(reg32, val)             REG_FLD_SET(INT_ENABLE_FLD_fifo_err_int_en, (reg32), (val))
#define INT_ENABLE_SET_o5_eqd_adj_done_int_en(reg32, val)      REG_FLD_SET(INT_ENABLE_FLD_o5_eqd_adj_done_int_en, (reg32), (val))
#define INT_ENABLE_SET_bwm_chk_err_int_en(reg32, val)          REG_FLD_SET(INT_ENABLE_FLD_bwm_chk_err_int_en, (reg32), (val))
#define INT_ENABLE_SET_dying_gasp_send_int_en(reg32, val)      REG_FLD_SET(INT_ENABLE_FLD_dying_gasp_send_int_en, (reg32), (val))
#define INT_ENABLE_SET_tod_1pps_int_en(reg32, val)             REG_FLD_SET(INT_ENABLE_FLD_tod_1pps_int_en, (reg32), (val))
#define INT_ENABLE_SET_tod_update_done_int_en(reg32, val)      REG_FLD_SET(INT_ENABLE_FLD_tod_update_done_int_en, (reg32), (val))
#define INT_ENABLE_SET_olt_ds_fec_chg_int_en(reg32, val)       REG_FLD_SET(INT_ENABLE_FLD_olt_ds_fec_chg_int_en, (reg32), (val))
#define INT_ENABLE_SET_us_prof_idx_chg_int_en(reg32, val)      REG_FLD_SET(INT_ENABLE_FLD_us_prof_idx_chg_int_en, (reg32), (val))
#define INT_ENABLE_SET_us_key_switch_done_int_en(reg32, val)   REG_FLD_SET(INT_ENABLE_FLD_us_key_switch_done_int_en, (reg32), (val))
#define INT_ENABLE_SET_us_no_msg_send_int_en(reg32, val)       REG_FLD_SET(INT_ENABLE_FLD_us_no_msg_send_int_en, (reg32), (val))
#define INT_ENABLE_SET_o4_registration_send_int_en(reg32, val) REG_FLD_SET(INT_ENABLE_FLD_o4_registration_send_int_en, (reg32), (val))
#define INT_ENABLE_SET_o4_ranging_req_recv_int_en(reg32, val)  REG_FLD_SET(INT_ENABLE_FLD_o4_ranging_req_recv_int_en, (reg32), (val))
#define INT_ENABLE_SET_o23_sn_onu_send_int_en(reg32, val)      REG_FLD_SET(INT_ENABLE_FLD_o23_sn_onu_send_int_en, (reg32), (val))
#define INT_ENABLE_SET_o23_sn_onu_req_recv_int_en(reg32, val)  REG_FLD_SET(INT_ENABLE_FLD_o23_sn_onu_req_recv_int_en, (reg32), (val))
#define INT_ENABLE_SET_ploamu_send_int_en(reg32, val)          REG_FLD_SET(INT_ENABLE_FLD_ploamu_send_int_en, (reg32), (val))
#define INT_ENABLE_SET_ploamd_recv_int_en(reg32, val)          REG_FLD_SET(INT_ENABLE_FLD_ploamd_recv_int_en, (reg32), (val))

#define INT_STATUS_SET_o5_sn_onu_req_recv_int(reg32, val)      REG_FLD_SET(INT_STATUS_FLD_o5_sn_onu_req_recv_int, (reg32), (val))
#define INT_STATUS_SET_turning_sfc_match_int(reg32, val)       REG_FLD_SET(INT_STATUS_FLD_turning_sfc_match_int, (reg32), (val))
#define INT_STATUS_SET_o9_gnt_recv_int(reg32, val)             REG_FLD_SET(INT_STATUS_FLD_o9_gnt_recv_int, (reg32), (val))
#define INT_STATUS_SET_sw1_mic_done_int(reg32, val)            REG_FLD_SET(INT_STATUS_FLD_sw1_mic_done_int, (reg32), (val))
#define INT_STATUS_SET_sw0_mic_done_int(reg32, val)            REG_FLD_SET(INT_STATUS_FLD_sw0_mic_done_int, (reg32), (val))
#define INT_STATUS_SET_key_cal_done_int(reg32, val)            REG_FLD_SET(INT_STATUS_FLD_key_cal_done_int, (reg32), (val))
#define INT_STATUS_SET_lwi_int(reg32, val)                     REG_FLD_SET(INT_STATUS_FLD_lwi_int, (reg32), (val))
#define INT_STATUS_SET_fwi_int(reg32, val)                     REG_FLD_SET(INT_STATUS_FLD_fwi_int, (reg32), (val))
#define INT_STATUS_SET_rx_err_int(reg32, val)                  REG_FLD_SET(INT_STATUS_FLD_rx_err_int, (reg32), (val))
#define INT_STATUS_SET_tx_err_int(reg32, val)                  REG_FLD_SET(INT_STATUS_FLD_tx_err_int, (reg32), (val))
#define INT_STATUS_SET_fifo_err_int(reg32, val)                REG_FLD_SET(INT_STATUS_FLD_fifo_err_int, (reg32), (val))
#define INT_STATUS_SET_o5_eqd_adj_done_int(reg32, val)         REG_FLD_SET(INT_STATUS_FLD_o5_eqd_adj_done_int, (reg32), (val))
#define INT_STATUS_SET_bwm_chk_err_int(reg32, val)             REG_FLD_SET(INT_STATUS_FLD_bwm_chk_err_int, (reg32), (val))
#define INT_STATUS_SET_dying_gasp_send_int(reg32, val)         REG_FLD_SET(INT_STATUS_FLD_dying_gasp_send_int, (reg32), (val))
#define INT_STATUS_SET_tod_1pps_int(reg32, val)                REG_FLD_SET(INT_STATUS_FLD_tod_1pps_int, (reg32), (val))
#define INT_STATUS_SET_tod_update_done_int(reg32, val)         REG_FLD_SET(INT_STATUS_FLD_tod_update_done_int, (reg32), (val))
#define INT_STATUS_SET_olt_ds_fec_chg_int(reg32, val)          REG_FLD_SET(INT_STATUS_FLD_olt_ds_fec_chg_int, (reg32), (val))
#define INT_STATUS_SET_us_prof_idx_chg_int(reg32, val)         REG_FLD_SET(INT_STATUS_FLD_us_prof_idx_chg_int, (reg32), (val))
#define INT_STATUS_SET_us_key_switch_done_int(reg32, val)      REG_FLD_SET(INT_STATUS_FLD_us_key_switch_done_int, (reg32), (val))
#define INT_STATUS_SET_us_no_msg_send_int(reg32, val)          REG_FLD_SET(INT_STATUS_FLD_us_no_msg_send_int, (reg32), (val))
#define INT_STATUS_SET_o4_registration_send_int(reg32, val)    REG_FLD_SET(INT_STATUS_FLD_o4_registration_send_int, (reg32), (val))
#define INT_STATUS_SET_o4_ranging_req_recv_int(reg32, val)     REG_FLD_SET(INT_STATUS_FLD_o4_ranging_req_recv_int, (reg32), (val))
#define INT_STATUS_SET_o23_sn_onu_send_int(reg32, val)         REG_FLD_SET(INT_STATUS_FLD_o23_sn_onu_send_int, (reg32), (val))
#define INT_STATUS_SET_o23_sn_onu_req_recv_int(reg32, val)     REG_FLD_SET(INT_STATUS_FLD_o23_sn_onu_req_recv_int, (reg32), (val))
#define INT_STATUS_SET_ploamu_send_int(reg32, val)             REG_FLD_SET(INT_STATUS_FLD_ploamu_send_int, (reg32), (val))
#define INT_STATUS_SET_ploamd_recv_int(reg32, val)             REG_FLD_SET(INT_STATUS_FLD_ploamd_recv_int, (reg32), (val))

#define FIFO_ERR_STS_SET_tx_aes_rdm_ciph_fifo_ovrn(reg32, val) REG_FLD_SET(FIFO_ERR_STS_FLD_tx_aes_rdm_ciph_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_byte_mib_cmd_fifo_ovrn(reg32, val)    REG_FLD_SET(FIFO_ERR_STS_FLD_byte_mib_cmd_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_frm_mib_cmd_fifo_ovrn(reg32, val)     REG_FLD_SET(FIFO_ERR_STS_FLD_frm_mib_cmd_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_mib_cmd_fifo_ovrn(reg32, val)         REG_FLD_SET(FIFO_ERR_STS_FLD_mib_cmd_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_mib_tx_cmd_fifo_ovrn(reg32, val)      REG_FLD_SET(FIFO_ERR_STS_FLD_mib_tx_cmd_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_snf_ds_fifo_ovrn(reg32, val)          REG_FLD_SET(FIFO_ERR_STS_FLD_snf_ds_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_snf_us_fifo_ovrn(reg32, val)          REG_FLD_SET(FIFO_ERR_STS_FLD_snf_us_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_rx_ploamd_fifo_udrn(reg32, val)       REG_FLD_SET(FIFO_ERR_STS_FLD_rx_ploamd_fifo_udrn, (reg32), (val))
#define FIFO_ERR_STS_SET_rx_ploamd_fifo_ovrn(reg32, val)       REG_FLD_SET(FIFO_ERR_STS_FLD_rx_ploamd_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_rx_mbi_pl_fifo_ovrn(reg32, val)       REG_FLD_SET(FIFO_ERR_STS_FLD_rx_mbi_pl_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_rx_mbi_hdr_fifo_ovrn(reg32, val)      REG_FLD_SET(FIFO_ERR_STS_FLD_rx_mbi_hdr_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_rx_aes_rdm_ciph_fifo_ovrn(reg32, val) REG_FLD_SET(FIFO_ERR_STS_FLD_rx_aes_rdm_ciph_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_rx_aes_ciph_txt_fifo_ovrn(reg32, val) REG_FLD_SET(FIFO_ERR_STS_FLD_rx_aes_ciph_txt_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_rx_aes_cryp_cnt_fifo_ovrn(reg32, val) REG_FLD_SET(FIFO_ERR_STS_FLD_rx_aes_cryp_cnt_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_tx_align_fifo_udrn(reg32, val)        REG_FLD_SET(FIFO_ERR_STS_FLD_tx_align_fifo_udrn, (reg32), (val))
#define FIFO_ERR_STS_SET_tx_ploamu_fifo_ovrn(reg32, val)       REG_FLD_SET(FIFO_ERR_STS_FLD_tx_ploamu_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_bst_fifo_ovrn(reg32, val)             REG_FLD_SET(FIFO_ERR_STS_FLD_bst_fifo_ovrn, (reg32), (val))
#define FIFO_ERR_STS_SET_sgl_fifo_ovrn(reg32, val)             REG_FLD_SET(FIFO_ERR_STS_FLD_sgl_fifo_ovrn, (reg32), (val))

#define TX_ERR_STS_SET_tx_prof_invld_err(reg32, val)           REG_FLD_SET(TX_ERR_STS_FLD_tx_prof_invld_err, (reg32), (val))
#define TX_ERR_STS_SET_tx_late_start_err(reg32, val)           REG_FLD_SET(TX_ERR_STS_FLD_tx_late_start_err, (reg32), (val))
#define TX_ERR_STS_SET_tx_bst_sgl_diff_err(reg32, val)         REG_FLD_SET(TX_ERR_STS_FLD_tx_bst_sgl_diff_err, (reg32), (val))

#define RX_ERR_STS_SET_rx_xgem_hdr_hec_err(reg32, val)         REG_FLD_SET(RX_ERR_STS_FLD_rx_xgem_hdr_hec_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_alloc_hec_err(reg32, val)            REG_FLD_SET(RX_ERR_STS_FLD_rx_alloc_hec_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_mbi_xgem_drop_err(reg32, val)        REG_FLD_SET(RX_ERR_STS_FLD_rx_mbi_xgem_drop_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_omci_mic_err(reg32, val)             REG_FLD_SET(RX_ERR_STS_FLD_rx_omci_mic_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_ploam_mic_err(reg32, val)            REG_FLD_SET(RX_ERR_STS_FLD_rx_ploam_mic_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_eth_crc_err(reg32, val)              REG_FLD_SET(RX_ERR_STS_FLD_rx_eth_crc_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_aes_key_err(reg32, val)              REG_FLD_SET(RX_ERR_STS_FLD_rx_aes_key_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_gem_intlv_err(reg32, val)            REG_FLD_SET(RX_ERR_STS_FLD_rx_gem_intlv_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_los_gem_del_err(reg32, val)          REG_FLD_SET(RX_ERR_STS_FLD_rx_los_gem_del_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_bip_err(reg32, val)                  REG_FLD_SET(RX_ERR_STS_FLD_rx_bip_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_hlend_hec_err(reg32, val)            REG_FLD_SET(RX_ERR_STS_FLD_rx_hlend_hec_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_pon_id_hec_err(reg32, val)           REG_FLD_SET(RX_ERR_STS_FLD_rx_pon_id_hec_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_sfc_hec_uc_err(reg32, val)           REG_FLD_SET(RX_ERR_STS_FLD_rx_sfc_hec_uc_err, (reg32), (val))
#define RX_ERR_STS_SET_rx_eof_err(reg32, val)                  REG_FLD_SET(RX_ERR_STS_FLD_rx_eof_err, (reg32), (val))

#if defined(TCSUPPORT_CPU_AN7583)
#define FIFO_ERR_ENABLE_SET_tx_aes_rdm_ciph_fifo_ovrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_tx_aes_rdm_ciph_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_byte_mib_cmd_fifo_ovrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_byte_mib_cmd_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_frm_mib_cmd_fifo_ovrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_frm_mib_cmd_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_mib_cmd_fifo_ovrn_en(reg32, val)   REG_FLD_SET(FIFO_ERR_ENABLE_FLD_mib_cmd_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_mib_tx_cmd_fifo_ovrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_mib_tx_cmd_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_snf_ds_fifo_ovrn_en(reg32, val)    REG_FLD_SET(FIFO_ERR_ENABLE_FLD_snf_ds_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_snf_us_fifo_ovrn_en(reg32, val)    REG_FLD_SET(FIFO_ERR_ENABLE_FLD_snf_us_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_rx_ploamd_fifo_udrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_rx_ploamd_fifo_udrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_rx_ploamd_fifo_ovrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_rx_ploamd_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_rx_mbi_pl_fifo_ovrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_rx_mbi_pl_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_rx_mbi_hdr_fifo_ovrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_rx_mbi_hdr_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_rx_aes_rdm_ciph_fifo_ovrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_rx_aes_rdm_ciph_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_rx_aes_ciph_txt_fifo_ovrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_rx_aes_ciph_txt_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_rx_aes_cryp_cnt_fifo_ovrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_rx_aes_cryp_cnt_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_tx_align_fifo_udrn_en(reg32, val)  REG_FLD_SET(FIFO_ERR_ENABLE_FLD_tx_align_fifo_udrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_tx_ploamu_fifo_ovrn_en(reg32, val) REG_FLD_SET(FIFO_ERR_ENABLE_FLD_tx_ploamu_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_bst_fifo_ovrn_en(reg32, val)       REG_FLD_SET(FIFO_ERR_ENABLE_FLD_bst_fifo_ovrn_en, (reg32), (val))
#define FIFO_ERR_ENABLE_SET_sgl_fifo_ovrn_en(reg32, val)       REG_FLD_SET(FIFO_ERR_ENABLE_FLD_sgl_fifo_ovrn_en, (reg32), (val))

#define TX_ERR_ENABLE_SET_tx_prof_invld_err_en(reg32, val)     REG_FLD_SET(TX_ERR_ENABLE_FLD_tx_prof_invld_err_en, (reg32), (val))
#define TX_ERR_ENABLE_SET_tx_late_start_err_en(reg32, val)     REG_FLD_SET(TX_ERR_ENABLE_FLD_tx_late_start_err_en, (reg32), (val))
#define TX_ERR_ENABLE_SET_tx_bst_sgl_diff_err_en(reg32, val)   REG_FLD_SET(TX_ERR_ENABLE_FLD_tx_bst_sgl_diff_err_en, (reg32), (val))

#define RX_ERR_ENABLE_SET_rx_xgem_hdr_hec_err_en(reg32, val)   REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_xgem_hdr_hec_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_alloc_hec_err_en(reg32, val)      REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_alloc_hec_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_mbi_xgem_drop_err_en(reg32, val)  REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_mbi_xgem_drop_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_omci_mic_err_en(reg32, val)       REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_omci_mic_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_ploam_mic_err_en(reg32, val)      REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_ploam_mic_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_eth_crc_err_en(reg32, val)        REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_eth_crc_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_aes_key_err_en(reg32, val)        REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_aes_key_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_gem_intlv_err_en(reg32, val)      REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_gem_intlv_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_los_gem_del_err_en(reg32, val)    REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_los_gem_del_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_bip_err_en(reg32, val)            REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_bip_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_hlend_hec_err_en(reg32, val)      REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_hlend_hec_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_pon_id_hec_err_en(reg32, val)     REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_pon_id_hec_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_sfc_hec_uc_err_en(reg32, val)     REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_sfc_hec_uc_err_en, (reg32), (val))
#define RX_ERR_ENABLE_SET_rx_eof_err_en(reg32, val)            REG_FLD_SET(RX_ERR_ENABLE_FLD_rx_eof_err_en, (reg32), (val))
#endif

#define COR_TAG_SET_cor_tag(reg32, val)                        REG_FLD_SET(COR_TAG_FLD_cor_tag, (reg32), (val))

#define CUR_DS_PON_ID_SET_cur_ds_pon_id(reg32, val)            REG_FLD_SET(CUR_DS_PON_ID_FLD_cur_ds_pon_id, (reg32), (val))

#define CUR_US_PON_ID_SET_cur_us_pon_id(reg32, val)            REG_FLD_SET(CUR_US_PON_ID_FLD_cur_us_pon_id, (reg32), (val))

#define CALIB_STS_L32_SET_calib_sts_l32(reg32, val)            REG_FLD_SET(CALIB_STS_L32_FLD_calib_sts_l32, (reg32), (val))

#define CALIB_STS_H32_SET_calib_sts_h32(reg32, val)            REG_FLD_SET(CALIB_STS_H32_FLD_calib_sts_h32, (reg32), (val))

#define TUNING_GRAN_SET_turning_gran(reg32, val)               REG_FLD_SET(TUNING_GRAN_FLD_turning_gran, (reg32), (val))

#define STEP_TUNING_TIME_SET_step_turning_time(reg32, val)     REG_FLD_SET(STEP_TUNING_TIME_FLD_step_turning_time, (reg32), (val))

#define US_RATE_CAP_SET_us_10g_cap(reg32, val)                 REG_FLD_SET(US_RATE_CAP_FLD_us_10g_cap, (reg32), (val))
#define US_RATE_CAP_SET_us_2p5g_cap(reg32, val)                REG_FLD_SET(US_RATE_CAP_FLD_us_2p5g_cap, (reg32), (val))

#define ATTENUATION_SET_attenuation(reg32, val)                REG_FLD_SET(ATTENUATION_FLD_attenuation, (reg32), (val))

#define POWER_LEVEL_CAP_SET_power_level_cap(reg32, val)        REG_FLD_SET(POWER_LEVEL_CAP_FLD_power_level_cap, (reg32), (val))

#define O23_O4_PLOAMU_CTRL_SET_o23_o4_ploamu_ctrl(reg32, val)  REG_FLD_SET(O23_O4_PLOAMU_CTRL_FLD_o23_o4_ploamu_ctrl, (reg32), (val))

#define ACTIVATION_ST_SET_act_st(reg32, val)                   REG_FLD_SET(ACTIVATION_ST_FLD_act_st, (reg32), (val))

#define RSP_TIME_SET_tresp(reg32, val)                         REG_FLD_SET(RSP_TIME_FLD_tresp, (reg32), (val))

#define RDM_DLY_SET_max_rdm_dly(reg32, val)                    REG_FLD_SET(RDM_DLY_FLD_max_rdm_dly, (reg32), (val))
#define RDM_DLY_SET_rdm_dly(reg32, val)                        REG_FLD_SET(RDM_DLY_FLD_rdm_dly, (reg32), (val))

#define INT_DLY_SET_phy_rx_dly(reg32, val)                     REG_FLD_SET(INT_DLY_FLD_phy_rx_dly, (reg32), (val))
#define INT_DLY_SET_phy_tx_dly(reg32, val)                     REG_FLD_SET(INT_DLY_FLD_phy_tx_dly, (reg32), (val))

#define EQD_SET_eqd(reg32, val)                                REG_FLD_SET(EQD_FLD_eqd, (reg32), (val))

#define SYN_MRG_SET_syn_mrg(reg32, val)                        REG_FLD_SET(SYN_MRG_FLD_syn_mrg, (reg32), (val))

#define US_PROF_VLD_SET_us_prof3_vsn(reg32, val)               REG_FLD_SET(US_PROF_VLD_FLD_us_prof3_vsn, (reg32), (val))
#define US_PROF_VLD_SET_us_prof3_vld(reg32, val)               REG_FLD_SET(US_PROF_VLD_FLD_us_prof3_vld, (reg32), (val))
#define US_PROF_VLD_SET_us_prof2_vsn(reg32, val)               REG_FLD_SET(US_PROF_VLD_FLD_us_prof2_vsn, (reg32), (val))
#define US_PROF_VLD_SET_us_prof2_vld(reg32, val)               REG_FLD_SET(US_PROF_VLD_FLD_us_prof2_vld, (reg32), (val))
#define US_PROF_VLD_SET_us_prof1_vsn(reg32, val)               REG_FLD_SET(US_PROF_VLD_FLD_us_prof1_vsn, (reg32), (val))
#define US_PROF_VLD_SET_us_prof1_vld(reg32, val)               REG_FLD_SET(US_PROF_VLD_FLD_us_prof1_vld, (reg32), (val))
#define US_PROF_VLD_SET_us_prof0_vsn(reg32, val)               REG_FLD_SET(US_PROF_VLD_FLD_us_prof0_vsn, (reg32), (val))
#define US_PROF_VLD_SET_us_prof0_vld(reg32, val)               REG_FLD_SET(US_PROF_VLD_FLD_us_prof0_vld, (reg32), (val))

#define US_PROF_PSBu_LEN_0_1_SET_us_psbu_len_prof1(reg32, val) REG_FLD_SET(US_PROF_PSBu_LEN_0_1_FLD_us_psbu_len_prof1, (reg32), (val))
#define US_PROF_PSBu_LEN_0_1_SET_us_psbu_len_prof0(reg32, val) REG_FLD_SET(US_PROF_PSBu_LEN_0_1_FLD_us_psbu_len_prof0, (reg32), (val))

#define US_PROF_PSBu_LEN_2_3_SET_us_psbu_len_prof3(reg32, val) REG_FLD_SET(US_PROF_PSBu_LEN_2_3_FLD_us_psbu_len_prof3, (reg32), (val))
#define US_PROF_PSBu_LEN_2_3_SET_us_psbu_len_prof2(reg32, val) REG_FLD_SET(US_PROF_PSBu_LEN_2_3_FLD_us_psbu_len_prof2, (reg32), (val))

#define US_AES_KEY_CTRL_SET_us_aes_key_vld(reg32, val)         REG_FLD_SET(US_AES_KEY_CTRL_FLD_us_aes_key_vld, (reg32), (val))
#define US_AES_KEY_CTRL_SET_us_aes_key_idx(reg32, val)         REG_FLD_SET(US_AES_KEY_CTRL_FLD_us_aes_key_idx, (reg32), (val))

#define DS_AES_KEY_VLD_SET_ds_aes_bc_idx1_key_vld(reg32, val)  REG_FLD_SET(DS_AES_KEY_VLD_FLD_ds_aes_bc_idx1_key_vld, (reg32), (val))
#define DS_AES_KEY_VLD_SET_ds_aes_bc_idx0_key_vld(reg32, val)  REG_FLD_SET(DS_AES_KEY_VLD_FLD_ds_aes_bc_idx0_key_vld, (reg32), (val))
#define DS_AES_KEY_VLD_SET_ds_aes_uc_idx1_key_vld(reg32, val)  REG_FLD_SET(DS_AES_KEY_VLD_FLD_ds_aes_uc_idx1_key_vld, (reg32), (val))
#define DS_AES_KEY_VLD_SET_ds_aes_uc_idx0_key_vld(reg32, val)  REG_FLD_SET(DS_AES_KEY_VLD_FLD_ds_aes_uc_idx0_key_vld, (reg32), (val))

#define DEFT_GPID_ENC_SET_deft_gpid_enc(reg32, val)            REG_FLD_SET(DEFT_GPID_ENC_FLD_deft_gpid_enc, (reg32), (val))

#define AES_UC_IDX0_KEY0_SET_aes_uc_idx0_key0(reg32, val)      REG_FLD_SET(AES_UC_IDX0_KEY0_FLD_aes_uc_idx0_key0, (reg32), (val))

#define AES_UC_IDX0_KEY1_SET_aes_uc_idx0_key1(reg32, val)      REG_FLD_SET(AES_UC_IDX0_KEY1_FLD_aes_uc_idx0_key1, (reg32), (val))

#define AES_UC_IDX0_KEY2_SET_aes_uc_idx0_key2(reg32, val)      REG_FLD_SET(AES_UC_IDX0_KEY2_FLD_aes_uc_idx0_key2, (reg32), (val))

#define AES_UC_IDX0_KEY3_SET_aes_uc_idx0_key3(reg32, val)      REG_FLD_SET(AES_UC_IDX0_KEY3_FLD_aes_uc_idx0_key3, (reg32), (val))

#define AES_UC_IDX1_KEY0_SET_aes_uc_idx1_key0(reg32, val)      REG_FLD_SET(AES_UC_IDX1_KEY0_FLD_aes_uc_idx1_key0, (reg32), (val))

#define AES_UC_IDX1_KEY1_SET_aes_uc_idx1_key1(reg32, val)      REG_FLD_SET(AES_UC_IDX1_KEY1_FLD_aes_uc_idx1_key1, (reg32), (val))

#define AES_UC_IDX1_KEY2_SET_aes_uc_idx1_key2(reg32, val)      REG_FLD_SET(AES_UC_IDX1_KEY2_FLD_aes_uc_idx1_key2, (reg32), (val))

#define AES_UC_IDX1_KEY3_SET_aes_uc_idx1_key3(reg32, val)      REG_FLD_SET(AES_UC_IDX1_KEY3_FLD_aes_uc_idx1_key3, (reg32), (val))

#define AES_BC_IDX0_KEY0_SET_aes_bc_idx0_key0(reg32, val)      REG_FLD_SET(AES_BC_IDX0_KEY0_FLD_aes_bc_idx0_key0, (reg32), (val))

#define AES_BC_IDX0_KEY1_SET_aes_bc_idx0_key1(reg32, val)      REG_FLD_SET(AES_BC_IDX0_KEY1_FLD_aes_bc_idx0_key1, (reg32), (val))

#define AES_BC_IDX0_KEY2_SET_aes_bc_idx0_key2(reg32, val)      REG_FLD_SET(AES_BC_IDX0_KEY2_FLD_aes_bc_idx0_key2, (reg32), (val))

#define AES_BC_IDX0_KEY3_SET_aes_bc_idx0_key3(reg32, val)      REG_FLD_SET(AES_BC_IDX0_KEY3_FLD_aes_bc_idx0_key3, (reg32), (val))

#define AES_BC_IDX1_KEY0_SET_aes_bc_idx1_key0(reg32, val)      REG_FLD_SET(AES_BC_IDX1_KEY0_FLD_aes_bc_idx1_key0, (reg32), (val))

#define AES_BC_IDX1_KEY1_SET_aes_bc_idx1_key1(reg32, val)      REG_FLD_SET(AES_BC_IDX1_KEY1_FLD_aes_bc_idx1_key1, (reg32), (val))

#define AES_BC_IDX1_KEY2_SET_aes_bc_idx1_key2(reg32, val)      REG_FLD_SET(AES_BC_IDX1_KEY2_FLD_aes_bc_idx1_key2, (reg32), (val))

#define AES_BC_IDX1_KEY3_SET_aes_bc_idx1_key3(reg32, val)      REG_FLD_SET(AES_BC_IDX1_KEY3_FLD_aes_bc_idx1_key3, (reg32), (val))

#define TCONT_ID_CFG_SET_tcont_cmd(reg32, val)                 REG_FLD_SET(TCONT_ID_CFG_FLD_tcont_cmd, (reg32), (val))
#define TCONT_ID_CFG_SET_tcont_id_index(reg32, val)            REG_FLD_SET(TCONT_ID_CFG_FLD_tcont_id_index, (reg32), (val))
#define TCONT_ID_CFG_SET_wr_tcont_id_vld(reg32, val)           REG_FLD_SET(TCONT_ID_CFG_FLD_wr_tcont_id_vld, (reg32), (val))
#define TCONT_ID_CFG_SET_wr_tcont_id(reg32, val)               REG_FLD_SET(TCONT_ID_CFG_FLD_wr_tcont_id, (reg32), (val))

#define TCONT_ID_STS_SET_tcont_cmd_done(reg32, val)            REG_FLD_SET(TCONT_ID_STS_FLD_tcont_cmd_done, (reg32), (val))
#define TCONT_ID_STS_SET_rd_tcont_id_vld(reg32, val)           REG_FLD_SET(TCONT_ID_STS_FLD_rd_tcont_id_vld, (reg32), (val))
#define TCONT_ID_STS_SET_rd_tcont_id(reg32, val)               REG_FLD_SET(TCONT_ID_STS_FLD_rd_tcont_id, (reg32), (val))

#define GPIDX_TBL_INIT_SET_gpidx_tbl_init_done(reg32, val)     REG_FLD_SET(GPIDX_TBL_INIT_FLD_gpidx_tbl_init_done, (reg32), (val))
#define GPIDX_TBL_INIT_SET_gpidx_tbl_init_start(reg32, val)    REG_FLD_SET(GPIDX_TBL_INIT_FLD_gpidx_tbl_init_start, (reg32), (val))

#define GPIDX_TBL_CTRL_SET_gpidx_tbl_cmd(reg32, val)           REG_FLD_SET(GPIDX_TBL_CTRL_FLD_gpidx_tbl_cmd, (reg32), (val))
#define GPIDX_TBL_CTRL_SET_gpidx_tbl_addr(reg32, val)          REG_FLD_SET(GPIDX_TBL_CTRL_FLD_gpidx_tbl_addr, (reg32), (val))
#define GPIDX_TBL_CTRL_SET_gpidx_tbl_wdata(reg32, val)         REG_FLD_SET(GPIDX_TBL_CTRL_FLD_gpidx_tbl_wdata, (reg32), (val))

#define GPIDX_TBL_STS_SET_gpidx_cmd_done(reg32, val)           REG_FLD_SET(GPIDX_TBL_STS_FLD_gpidx_cmd_done, (reg32), (val))
#define GPIDX_TBL_STS_SET_gpidx_tbl_rdata(reg32, val)          REG_FLD_SET(GPIDX_TBL_STS_FLD_gpidx_tbl_rdata, (reg32), (val))

#define GEM_TBL_INIT_SET_gem_tbl_init_done(reg32, val)         REG_FLD_SET(GEM_TBL_INIT_FLD_gem_tbl_init_done, (reg32), (val))
#define GEM_TBL_INIT_SET_gem_tbl_init_start(reg32, val)        REG_FLD_SET(GEM_TBL_INIT_FLD_gem_tbl_init_start, (reg32), (val))

#define GEM_PORT_CFG_SET_gpid_cmd(reg32, val)                  REG_FLD_SET(GEM_PORT_CFG_FLD_gpid_cmd, (reg32), (val))
#define GEM_PORT_CFG_SET_gpid_vld(reg32, val)                  REG_FLD_SET(GEM_PORT_CFG_FLD_gpid_vld, (reg32), (val))
#define GEM_PORT_CFG_SET_gpid_type(reg32, val)                 REG_FLD_SET(GEM_PORT_CFG_FLD_gpid_type, (reg32), (val))
#define GEM_PORT_CFG_SET_gpid_us_encrypt(reg32, val)           REG_FLD_SET(GEM_PORT_CFG_FLD_gpid_us_encrypt, (reg32), (val))
#define GEM_PORT_CFG_SET_gem_port_id(reg32, val)               REG_FLD_SET(GEM_PORT_CFG_FLD_gem_port_id, (reg32), (val))

#define GEM_PORT_STS_SET_gpid_cmd_done(reg32, val)             REG_FLD_SET(GEM_PORT_STS_FLD_gpid_cmd_done, (reg32), (val))
#define GEM_PORT_STS_SET_gpid_rd_sts(reg32, val)               REG_FLD_SET(GEM_PORT_STS_FLD_gpid_rd_sts, (reg32), (val))

#define G_TX_FCS_TBL_INIT_SET_tx_fcs_tbl_init_done(reg32, val) REG_FLD_SET(G_TX_FCS_TBL_INIT_FLD_tx_fcs_tbl_init_done, (reg32), (val))
#define G_TX_FCS_TBL_INIT_SET_tx_fcs_tbl_init_start(reg32, val) REG_FLD_SET(G_TX_FCS_TBL_INIT_FLD_tx_fcs_tbl_init_start, (reg32), (val))

#if defined(TCSUPPORT_CPU_EN7581)
#define IDLE_GEM_CTRL_SET_force_idle_hdr_data_same(reg32, val) REG_FLD_SET(IDLE_GEM_CTRL_FLD_force_idle_hdr_data_same, (reg32), (val))
#endif
#define IDLE_GEM_CTRL_SET_idle_gem_max_size(reg32, val)        REG_FLD_SET(IDLE_GEM_CTRL_FLD_idle_gem_max_size, (reg32), (val))
#define IDLE_GEM_CTRL_SET_idle_gem_thld(reg32, val)            REG_FLD_SET(IDLE_GEM_CTRL_FLD_idle_gem_thld, (reg32), (val))

#define US_DYING_GASP_CTRL_SET_dying_gasp_send_num(reg32, val) REG_FLD_SET(US_DYING_GASP_CTRL_FLD_dying_gasp_send_num, (reg32), (val))
#define US_DYING_GASP_CTRL_SET_sw_dying_gasp_trig(reg32, val)  REG_FLD_SET(US_DYING_GASP_CTRL_FLD_sw_dying_gasp_trig, (reg32), (val))
#define US_DYING_GASP_CTRL_SET_hw_dying_gasp_sts(reg32, val)   REG_FLD_SET(US_DYING_GASP_CTRL_FLD_hw_dying_gasp_sts, (reg32), (val))
#define US_DYING_GASP_CTRL_SET_hw_dying_gasp_en(reg32, val)    REG_FLD_SET(US_DYING_GASP_CTRL_FLD_hw_dying_gasp_en, (reg32), (val))

#define TX_XGEM_IDLE_DATA_SET_tx_xgem_idle_data(reg32, val)    REG_FLD_SET(TX_XGEM_IDLE_DATA_FLD_tx_xgem_idle_data, (reg32), (val))

#define TX_OMCI_PRE_GET_SET_tx_limit_get_omci_size(reg32, val) REG_FLD_SET(TX_OMCI_PRE_GET_FLD_tx_limit_get_omci_size, (reg32), (val))
#define TX_OMCI_PRE_GET_SET_tx_limit_get_omci_en(reg32, val)   REG_FLD_SET(TX_OMCI_PRE_GET_FLD_tx_limit_get_omci_en, (reg32), (val))
#define TX_OMCI_PRE_GET_SET_tx_pre_get_omci_en(reg32, val)     REG_FLD_SET(TX_OMCI_PRE_GET_FLD_tx_pre_get_omci_en, (reg32), (val))

#define RX_OMCI_PRE_GET_SET_rx_omci_intr_eth_en(reg32, val)    REG_FLD_SET(RX_OMCI_PRE_GET_FLD_rx_omci_intr_eth_en, (reg32), (val))

#define EPDROP_EN_SET_errploamd_drop_en(reg32, val)            REG_FLD_SET(EPDROP_EN_FLD_errploamd_drop_en, (reg32), (val))

#define PLOAMu_FIFO_STS_SET_ploamu_fifo_ovrn(reg32, val)       REG_FLD_SET(PLOAMu_FIFO_STS_FLD_ploamu_fifo_ovrn, (reg32), (val))
#define PLOAMu_FIFO_STS_SET_ploamu_fifo_min_avail(reg32, val)  REG_FLD_SET(PLOAMu_FIFO_STS_FLD_ploamu_fifo_min_avail, (reg32), (val))
#define PLOAMu_FIFO_STS_SET_ploamu_fifo_avail(reg32, val)      REG_FLD_SET(PLOAMu_FIFO_STS_FLD_ploamu_fifo_avail, (reg32), (val))

#define PLOAMu_WDATA_SET_ploamu_wdata(reg32, val)              REG_FLD_SET(PLOAMu_WDATA_FLD_ploamu_wdata, (reg32), (val))

#define PLOAMd_FIFO_STS_SET_ploamd_fifo_ovrn(reg32, val)       REG_FLD_SET(PLOAMd_FIFO_STS_FLD_ploamd_fifo_ovrn, (reg32), (val))
#define PLOAMd_FIFO_STS_SET_ploamd_fifo_max_used(reg32, val)   REG_FLD_SET(PLOAMd_FIFO_STS_FLD_ploamd_fifo_max_used, (reg32), (val))
#define PLOAMd_FIFO_STS_SET_ploamd_fifo_used(reg32, val)       REG_FLD_SET(PLOAMd_FIFO_STS_FLD_ploamd_fifo_used, (reg32), (val))

#define PLOAMd_RDATA_SET_ploamd_rdata(reg32, val)              REG_FLD_SET(PLOAMd_RDATA_FLD_ploamd_rdata, (reg32), (val))

#define KEY_GEN_SET_kek_start(reg32, val)                      REG_FLD_SET(KEY_GEN_FLD_kek_start, (reg32), (val))
#define KEY_GEN_SET_ploam_ik_start(reg32, val)                 REG_FLD_SET(KEY_GEN_FLD_ploam_ik_start, (reg32), (val))
#define KEY_GEN_SET_omci_ik_start(reg32, val)                  REG_FLD_SET(KEY_GEN_FLD_omci_ik_start, (reg32), (val))
#define KEY_GEN_SET_sk_start(reg32, val)                       REG_FLD_SET(KEY_GEN_FLD_sk_start, (reg32), (val))
#define KEY_GEN_SET_regmsk_start(reg32, val)                   REG_FLD_SET(KEY_GEN_FLD_regmsk_start, (reg32), (val))

#define CUR_KIDX_SET_cur_oik_idx(reg32, val)                   REG_FLD_SET(CUR_KIDX_FLD_cur_oik_idx, (reg32), (val))
#define CUR_KIDX_SET_cur_pik_idx(reg32, val)                   REG_FLD_SET(CUR_KIDX_FLD_cur_pik_idx, (reg32), (val))

#define MSK_0_SET_msk0(reg32, val)                             REG_FLD_SET(MSK_0_FLD_msk0, (reg32), (val))

#define MSK_1_SET_msk1(reg32, val)                             REG_FLD_SET(MSK_1_FLD_msk1, (reg32), (val))

#define MSK_2_SET_msk2(reg32, val)                             REG_FLD_SET(MSK_2_FLD_msk2, (reg32), (val))

#define MSK_3_SET_msk3(reg32, val)                             REG_FLD_SET(MSK_3_FLD_msk3, (reg32), (val))

#define REGMSK_0_SET_regmsk0(reg32, val)                       REG_FLD_SET(REGMSK_0_FLD_regmsk0, (reg32), (val))

#define REGMSK_1_SET_regmsk1(reg32, val)                       REG_FLD_SET(REGMSK_1_FLD_regmsk1, (reg32), (val))

#define REGMSK_2_SET_regmsk2(reg32, val)                       REG_FLD_SET(REGMSK_2_FLD_regmsk2, (reg32), (val))

#define REGMSK_3_SET_regmsk3(reg32, val)                       REG_FLD_SET(REGMSK_3_FLD_regmsk3, (reg32), (val))

#define SK_0_SET_sk0(reg32, val)                               REG_FLD_SET(SK_0_FLD_sk0, (reg32), (val))

#define SK_1_SET_sk1(reg32, val)                               REG_FLD_SET(SK_1_FLD_sk1, (reg32), (val))

#define SK_2_SET_sk2(reg32, val)                               REG_FLD_SET(SK_2_FLD_sk2, (reg32), (val))

#define SK_3_SET_sk3(reg32, val)                               REG_FLD_SET(SK_3_FLD_sk3, (reg32), (val))

#define HW_GENK_0_SET_hw_genk_0(reg32, val)                    REG_FLD_SET(HW_GENK_0_FLD_hw_genk_0, (reg32), (val))

#define HW_GENK_1_SET_hw_genk_1(reg32, val)                    REG_FLD_SET(HW_GENK_1_FLD_hw_genk_1, (reg32), (val))

#define HW_GENK_2_SET_hw_genk_2(reg32, val)                    REG_FLD_SET(HW_GENK_2_FLD_hw_genk_2, (reg32), (val))

#define HW_GENK_3_SET_hw_genk_3(reg32, val)                    REG_FLD_SET(HW_GENK_3_FLD_hw_genk_3, (reg32), (val))

#define PIK0_0_SET_pik0_0(reg32, val)                          REG_FLD_SET(PIK0_0_FLD_pik0_0, (reg32), (val))

#define PIK0_1_SET_pik0_1(reg32, val)                          REG_FLD_SET(PIK0_1_FLD_pik0_1, (reg32), (val))

#define PIK0_2_SET_pik0_2(reg32, val)                          REG_FLD_SET(PIK0_2_FLD_pik0_2, (reg32), (val))

#define PIK0_3_SET_pik0_3(reg32, val)                          REG_FLD_SET(PIK0_3_FLD_pik0_3, (reg32), (val))

#define PIK1_0_SET_pik1_0(reg32, val)                          REG_FLD_SET(PIK1_0_FLD_pik1_0, (reg32), (val))

#define PIK1_1_SET_pik1_1(reg32, val)                          REG_FLD_SET(PIK1_1_FLD_pik1_1, (reg32), (val))

#define PIK1_2_SET_pik1_2(reg32, val)                          REG_FLD_SET(PIK1_2_FLD_pik1_2, (reg32), (val))

#define PIK1_3_SET_pik1_3(reg32, val)                          REG_FLD_SET(PIK1_3_FLD_pik1_3, (reg32), (val))

#define OIK0_0_SET_oik0_0(reg32, val)                          REG_FLD_SET(OIK0_0_FLD_oik0_0, (reg32), (val))

#define OIK0_1_SET_oik0_1(reg32, val)                          REG_FLD_SET(OIK0_1_FLD_oik0_1, (reg32), (val))

#define OIK0_2_SET_oik0_2(reg32, val)                          REG_FLD_SET(OIK0_2_FLD_oik0_2, (reg32), (val))

#define OIK0_3_SET_oik0_3(reg32, val)                          REG_FLD_SET(OIK0_3_FLD_oik0_3, (reg32), (val))

#define OIK1_0_SET_oik1_0(reg32, val)                          REG_FLD_SET(OIK1_0_FLD_oik1_0, (reg32), (val))

#define OIK1_1_SET_oik1_1(reg32, val)                          REG_FLD_SET(OIK1_1_FLD_oik1_1, (reg32), (val))

#define OIK1_2_SET_oik1_2(reg32, val)                          REG_FLD_SET(OIK1_2_FLD_oik1_2, (reg32), (val))

#define OIK1_3_SET_oik1_3(reg32, val)                          REG_FLD_SET(OIK1_3_FLD_oik1_3, (reg32), (val))

#define KEK0_0_SET_kek0_0(reg32, val)                          REG_FLD_SET(KEK0_0_FLD_kek0_0, (reg32), (val))

#define KEK0_1_SET_kek0_1(reg32, val)                          REG_FLD_SET(KEK0_1_FLD_kek0_1, (reg32), (val))

#define KEK0_2_SET_kek0_2(reg32, val)                          REG_FLD_SET(KEK0_2_FLD_kek0_2, (reg32), (val))

#define KEK0_3_SET_kek0_3(reg32, val)                          REG_FLD_SET(KEK0_3_FLD_kek0_3, (reg32), (val))

#define KEK1_0_SET_kek1_0(reg32, val)                          REG_FLD_SET(KEK1_0_FLD_kek1_0, (reg32), (val))

#define KEK1_1_SET_kek1_1(reg32, val)                          REG_FLD_SET(KEK1_1_FLD_kek1_1, (reg32), (val))

#define KEK1_2_SET_kek1_2(reg32, val)                          REG_FLD_SET(KEK1_2_FLD_kek1_2, (reg32), (val))

#define KEK1_3_SET_kek1_3(reg32, val)                          REG_FLD_SET(KEK1_3_FLD_kek1_3, (reg32), (val))

#define PON_TAG_0_SET_pon_tag_0(reg32, val)                    REG_FLD_SET(PON_TAG_0_FLD_pon_tag_0, (reg32), (val))

#define PON_TAG_1_SET_pon_tag_1(reg32, val)                    REG_FLD_SET(PON_TAG_1_FLD_pon_tag_1, (reg32), (val))

#define SW_SET_KIDX_SET_sw_set_oik_en(reg32, val)              REG_FLD_SET(SW_SET_KIDX_FLD_sw_set_oik_en, (reg32), (val))
#define SW_SET_KIDX_SET_sw_set_pik_en(reg32, val)              REG_FLD_SET(SW_SET_KIDX_FLD_sw_set_pik_en, (reg32), (val))
#define SW_SET_KIDX_SET_sw_set_oik_idx(reg32, val)             REG_FLD_SET(SW_SET_KIDX_FLD_sw_set_oik_idx, (reg32), (val))
#define SW_SET_KIDX_SET_sw_set_pik_idx(reg32, val)             REG_FLD_SET(SW_SET_KIDX_FLD_sw_set_pik_idx, (reg32), (val))

#define SW0_ENCSTART_SET_sw0_encstart(reg32, val)              REG_FLD_SET(SW0_ENCSTART_FLD_sw0_encstart, (reg32), (val))

#define SW0_MADDR_SET_sw0_mdtaddr(reg32, val)                  REG_FLD_SET(SW0_MADDR_FLD_sw0_mdtaddr, (reg32), (val))

#define SW0_RADDR_SET_sw0_rdtaddr(reg32, val)                  REG_FLD_SET(SW0_RADDR_FLD_sw0_rdtaddr, (reg32), (val))

#define SW0_KADDR_SET_sw0_kaddr(reg32, val)                    REG_FLD_SET(SW0_KADDR_FLD_sw0_kaddr, (reg32), (val))

#define SW0_ENCLEN_SET_sw0_rdtlen(reg32, val)                  REG_FLD_SET(SW0_ENCLEN_FLD_sw0_rdtlen, (reg32), (val))
#define SW0_ENCLEN_SET_sw0_mdtlen(reg32, val)                  REG_FLD_SET(SW0_ENCLEN_FLD_sw0_mdtlen, (reg32), (val))

#define SW0_ENCINFO_SET_sw0_enckidx(reg32, val)                REG_FLD_SET(SW0_ENCINFO_FLD_sw0_enckidx, (reg32), (val))
#define SW0_ENCINFO_SET_sw0_encdic(reg32, val)                 REG_FLD_SET(SW0_ENCINFO_FLD_sw0_encdic, (reg32), (val))

#define SW1_ENCSTART_SET_sw1_encstart(reg32, val)              REG_FLD_SET(SW1_ENCSTART_FLD_sw1_encstart, (reg32), (val))

#define SW1_MADDR_SET_sw1_mdtaddr(reg32, val)                  REG_FLD_SET(SW1_MADDR_FLD_sw1_mdtaddr, (reg32), (val))

#define SW1_RADDR_SET_sw1_rdtaddr(reg32, val)                  REG_FLD_SET(SW1_RADDR_FLD_sw1_rdtaddr, (reg32), (val))

#define SW1_KADDR_SET_sw1_kaddr(reg32, val)                    REG_FLD_SET(SW1_KADDR_FLD_sw1_kaddr, (reg32), (val))

#define SW1_ENCLEN_SET_sw1_rdtlen(reg32, val)                  REG_FLD_SET(SW1_ENCLEN_FLD_sw1_rdtlen, (reg32), (val))
#define SW1_ENCLEN_SET_sw1_mdtlen(reg32, val)                  REG_FLD_SET(SW1_ENCLEN_FLD_sw1_mdtlen, (reg32), (val))

#define SW1_ENCINFO_SET_sw1_enckidx(reg32, val)                REG_FLD_SET(SW1_ENCINFO_FLD_sw1_enckidx, (reg32), (val))
#define SW1_ENCINFO_SET_sw1_encdic(reg32, val)                 REG_FLD_SET(SW1_ENCINFO_FLD_sw1_encdic, (reg32), (val))

#if defined(TCSUPPORT_CPU_AN7583)
#define TX_FCS_TBL_CLEAR_SET_fcs_clr_tcont_done(reg32, val)    REG_FLD_SET(TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont_done, (reg32), (val))
#define TX_FCS_TBL_CLEAR_SET_fcs_clr_tcont_en(reg32, val)      REG_FLD_SET(TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont_en, (reg32), (val))
#define TX_FCS_TBL_CLEAR_SET_fcs_clr_tcont(reg32, val)         REG_FLD_SET(TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont, (reg32), (val))
#endif

#define MIB_CTRL_SET_mib_frame_type(reg32, val)                REG_FLD_SET(MIB_CTRL_FLD_mib_frame_type, (reg32), (val))
#define MIB_CTRL_SET_mib_en(reg32, val)                        REG_FLD_SET(MIB_CTRL_FLD_mib_en, (reg32), (val))

#define MIB_TBL_CONFIG_SET_mib_tbl_rd_clr(reg32, val)          REG_FLD_SET(MIB_TBL_CONFIG_FLD_mib_tbl_rd_clr, (reg32), (val))
#define MIB_TBL_CONFIG_SET_mib_tbl_init_done(reg32, val)       REG_FLD_SET(MIB_TBL_CONFIG_FLD_mib_tbl_init_done, (reg32), (val))
#define MIB_TBL_CONFIG_SET_mib_tbl_init_start(reg32, val)      REG_FLD_SET(MIB_TBL_CONFIG_FLD_mib_tbl_init_start, (reg32), (val))

#define MIB_CTRL_STS_SET_mib_cmd_done(reg32, val)              REG_FLD_SET(MIB_CTRL_STS_FLD_mib_cmd_done, (reg32), (val))
#define MIB_CTRL_STS_SET_mib_wr(reg32, val)                    REG_FLD_SET(MIB_CTRL_STS_FLD_mib_wr, (reg32), (val))
#define MIB_CTRL_STS_SET_mib_type(reg32, val)                  REG_FLD_SET(MIB_CTRL_STS_FLD_mib_type, (reg32), (val))
#define MIB_CTRL_STS_SET_mib_gpid(reg32, val)                  REG_FLD_SET(MIB_CTRL_STS_FLD_mib_gpid, (reg32), (val))

#define MIB_RDATA_L32_SET_mib_rdata_l32(reg32, val)            REG_FLD_SET(MIB_RDATA_L32_FLD_mib_rdata_l32, (reg32), (val))

#define MIB_RDATA_H32_SET_mib_rdata_h32(reg32, val)            REG_FLD_SET(MIB_RDATA_H32_FLD_mib_rdata_h32, (reg32), (val))

#define MIB_WDATA_L32_SET_mib_wdata_l32(reg32, val)            REG_FLD_SET(MIB_WDATA_L32_FLD_mib_wdata_l32, (reg32), (val))

#define MIB_WDATA_H32_SET_mib_wdata_h32(reg32, val)            REG_FLD_SET(MIB_WDATA_H32_FLD_mib_wdata_h32, (reg32), (val))

#define TOD_CLK_PERIOD_SET_tod_period(reg32, val)              REG_FLD_SET(TOD_CLK_PERIOD_FLD_tod_period, (reg32), (val))

#define TOD_1PPS_WD_CTRL_SET_tod_1pps_width_ctrl(reg32, val)   REG_FLD_SET(TOD_1PPS_WD_CTRL_FLD_tod_1pps_width_ctrl, (reg32), (val))

#define CUR_TOD_SEC_SET_cur_tod_sec(reg32, val)                REG_FLD_SET(CUR_TOD_SEC_FLD_cur_tod_sec, (reg32), (val))

#define CUR_TOD_NANO_SEC_SET_cur_tod_nano_sec(reg32, val)      REG_FLD_SET(CUR_TOD_NANO_SEC_FLD_cur_tod_nano_sec, (reg32), (val))

#define TOD_SFC_L_SET_tod_spf_cnt_l(reg32, val)                REG_FLD_SET(TOD_SFC_L_FLD_tod_spf_cnt_l, (reg32), (val))

#define TOD_SFC_H_SET_tod_spf_cnt_h(reg32, val)                REG_FLD_SET(TOD_SFC_H_FLD_tod_spf_cnt_h, (reg32), (val))

#define TOD_UPD_CTRL_SET_tod_upd_en(reg32, val)                REG_FLD_SET(TOD_UPD_CTRL_FLD_tod_upd_en, (reg32), (val))

#define NEW_TOD_SEC_SET_new_tod_sec(reg32, val)                REG_FLD_SET(NEW_TOD_SEC_FLD_new_tod_sec, (reg32), (val))

#define NEW_TOD_NANO_SEC_SET_new_tod_nano_sec(reg32, val)      REG_FLD_SET(NEW_TOD_NANO_SEC_FLD_new_tod_nano_sec, (reg32), (val))

#define SLEEP_CFG_SET_reload_sleep_cnt(reg32, val)             REG_FLD_SET(SLEEP_CFG_FLD_reload_sleep_cnt, (reg32), (val))

#define SLEEP_CNT_SET_sleep_cnt(reg32, val)                    REG_FLD_SET(SLEEP_CNT_FLD_sleep_cnt, (reg32), (val))

#define TURNING_SFC_L_SET_turning_spf_cnt_l(reg32, val)        REG_FLD_SET(TURNING_SFC_L_FLD_turning_spf_cnt_l, (reg32), (val))

#define TURNING_SFC_H_SET_turning_spf_cnt_h(reg32, val)        REG_FLD_SET(TURNING_SFC_H_FLD_turning_spf_cnt_h, (reg32), (val))

#define DBG_CAP_SETTING_SET_hw_ack_ploamu_code(reg32, val)     REG_FLD_SET(DBG_CAP_SETTING_FLD_hw_ack_ploamu_code, (reg32), (val))
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_SET_force_o2349_zero_gsize_no_dbru(reg32, val) REG_FLD_SET(DBG_CAP_SETTING_FLD_force_o2349_zero_gsize_no_dbru, (reg32), (val))
#define DBG_CAP_SETTING_SET_ds_bip_chk_en(reg32, val)          REG_FLD_SET(DBG_CAP_SETTING_FLD_ds_bip_chk_en, (reg32), (val))
#define DBG_CAP_SETTING_SET_ds_4b_trailer_en(reg32, val)       REG_FLD_SET(DBG_CAP_SETTING_FLD_ds_4b_trailer_en, (reg32), (val))
#define DBG_CAP_SETTING_SET_tx_deft_gpid_enc_sts_ctrl_by_rx(reg32, val) REG_FLD_SET(DBG_CAP_SETTING_FLD_tx_deft_gpid_enc_sts_ctrl_by_rx, (reg32), (val))
#define DBG_CAP_SETTING_SET_flt_key_invld_gem(reg32, val)      REG_FLD_SET(DBG_CAP_SETTING_FLD_flt_key_invld_gem, (reg32), (val))
#define DBG_CAP_SETTING_SET_flt_o2349_continue_gnt(reg32, val) REG_FLD_SET(DBG_CAP_SETTING_FLD_flt_o2349_continue_gnt, (reg32), (val))
#define DBG_CAP_SETTING_SET_flt_o2349_no_ploamu_gnt(reg32, val) REG_FLD_SET(DBG_CAP_SETTING_FLD_flt_o2349_no_ploamu_gnt, (reg32), (val))
#endif
#define DBG_CAP_SETTING_SET_us_aes_seq_num_14b(reg32, val)     REG_FLD_SET(DBG_CAP_SETTING_FLD_us_aes_seq_num_14b, (reg32), (val))
#define DBG_CAP_SETTING_SET_o52_idle_only_en(reg32, val)       REG_FLD_SET(DBG_CAP_SETTING_FLD_o52_idle_only_en, (reg32), (val))
#define DBG_CAP_SETTING_SET_tx_send_only_in_o23459(reg32, val) REG_FLD_SET(DBG_CAP_SETTING_FLD_tx_send_only_in_o23459, (reg32), (val))
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_SET_flt_o49_no_deft_tcont_gnt(reg32, val) REG_FLD_SET(DBG_CAP_SETTING_FLD_flt_o49_no_deft_tcont_gnt, (reg32), (val))
#else
#define DBG_CAP_SETTING_SET_flt_dbru_set_gnt_o2349(reg32, val) REG_FLD_SET(DBG_CAP_SETTING_FLD_flt_dbru_set_gnt_o2349, (reg32), (val))
#endif
#define DBG_CAP_SETTING_SET_send_ranging_ack_o4(reg32, val)    REG_FLD_SET(DBG_CAP_SETTING_FLD_send_ranging_ack_o4, (reg32), (val))
#define DBG_CAP_SETTING_SET_hw_cal_ds_omci_mic(reg32, val)     REG_FLD_SET(DBG_CAP_SETTING_FLD_hw_cal_ds_omci_mic, (reg32), (val))
#define DBG_CAP_SETTING_SET_hw_cal_us_omci_mic(reg32, val)     REG_FLD_SET(DBG_CAP_SETTING_FLD_hw_cal_us_omci_mic, (reg32), (val))
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_SET_flt_o2349_no_ploamu_only_gnt(reg32, val) REG_FLD_SET(DBG_CAP_SETTING_FLD_flt_o2349_no_ploamu_only_gnt, (reg32), (val))
#else
#define DBG_CAP_SETTING_SET_flt_no_zero_gsize_gnt_o2349(reg32, val) REG_FLD_SET(DBG_CAP_SETTING_FLD_flt_no_zero_gsize_gnt_o2349, (reg32), (val))
#endif
#define DBG_CAP_SETTING_SET_rept_msg_flt(reg32, val)           REG_FLD_SET(DBG_CAP_SETTING_FLD_rept_msg_flt, (reg32), (val))
#define DBG_CAP_SETTING_SET_rept_prof_flt(reg32, val)          REG_FLD_SET(DBG_CAP_SETTING_FLD_rept_prof_flt, (reg32), (val))

#define DBG_BWM_CHK_CTRL_SET_min_bst_intvl(reg32, val)         REG_FLD_SET(DBG_BWM_CHK_CTRL_FLD_min_bst_intvl, (reg32), (val))
#define DBG_BWM_CHK_CTRL_SET_max_bst_len_err_cut(reg32, val)   REG_FLD_SET(DBG_BWM_CHK_CTRL_FLD_max_bst_len_err_cut, (reg32), (val))
#define DBG_BWM_CHK_CTRL_SET_max_my_bst_num_err_cut(reg32, val) REG_FLD_SET(DBG_BWM_CHK_CTRL_FLD_max_my_bst_num_err_cut, (reg32), (val))
#define DBG_BWM_CHK_CTRL_SET_max_my_alloc_num_err_cut(reg32, val) REG_FLD_SET(DBG_BWM_CHK_CTRL_FLD_max_my_alloc_num_err_cut, (reg32), (val))
#define DBG_BWM_CHK_CTRL_SET_max_bst_alloc_num_err_cut(reg32, val) REG_FLD_SET(DBG_BWM_CHK_CTRL_FLD_max_bst_alloc_num_err_cut, (reg32), (val))
#define DBG_BWM_CHK_CTRL_SET_max_bwm_alloc_num_err_cut(reg32, val) REG_FLD_SET(DBG_BWM_CHK_CTRL_FLD_max_bwm_alloc_num_err_cut, (reg32), (val))
#define DBG_BWM_CHK_CTRL_SET_min_gsize_flt(reg32, val)         REG_FLD_SET(DBG_BWM_CHK_CTRL_FLD_min_gsize_flt, (reg32), (val))
#define DBG_BWM_CHK_CTRL_SET_max_gsize_flt(reg32, val)         REG_FLD_SET(DBG_BWM_CHK_CTRL_FLD_max_gsize_flt, (reg32), (val))
#define DBG_BWM_CHK_CTRL_SET_start_time_order_flt(reg32, val)  REG_FLD_SET(DBG_BWM_CHK_CTRL_FLD_start_time_order_flt, (reg32), (val))
#define DBG_BWM_CHK_CTRL_SET_max_start_time_flt(reg32, val)    REG_FLD_SET(DBG_BWM_CHK_CTRL_FLD_max_start_time_flt, (reg32), (val))
#define DBG_BWM_CHK_CTRL_SET_min_bst_intvl_flt(reg32, val)     REG_FLD_SET(DBG_BWM_CHK_CTRL_FLD_min_bst_intvl_flt, (reg32), (val))

#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_BWM_CKH_STS_SET_o2349_continue_gnt_recv(reg32, val) REG_FLD_SET(DBG_BWM_CKH_STS_FLD_o2349_continue_gnt_recv, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_o49_no_deft_tcont_gnt_recv(reg32, val) REG_FLD_SET(DBG_BWM_CKH_STS_FLD_o49_no_deft_tcont_gnt_recv, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_o2349_no_ploamu_only_gnt_recv(reg32, val) REG_FLD_SET(DBG_BWM_CKH_STS_FLD_o2349_no_ploamu_only_gnt_recv, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_o2349_no_ploamu_gnt_recv(reg32, val) REG_FLD_SET(DBG_BWM_CKH_STS_FLD_o2349_no_ploamu_gnt_recv, (reg32), (val))
#else
#define DBG_BWM_CKH_STS_SET_o23_o4_data_gnt_recv(reg32, val)   REG_FLD_SET(DBG_BWM_CKH_STS_FLD_o23_o4_data_gnt_recv, (reg32), (val))
#endif
#define DBG_BWM_CKH_STS_SET_other_tid_ins_my_bst_err(reg32, val) REG_FLD_SET(DBG_BWM_CKH_STS_FLD_other_tid_ins_my_bst_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_max_bst_len_err(reg32, val)        REG_FLD_SET(DBG_BWM_CKH_STS_FLD_max_bst_len_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_max_my_bst_num_err(reg32, val)     REG_FLD_SET(DBG_BWM_CKH_STS_FLD_max_my_bst_num_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_max_my_alloc_num_err(reg32, val)   REG_FLD_SET(DBG_BWM_CKH_STS_FLD_max_my_alloc_num_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_max_bst_alloc_num_err(reg32, val)  REG_FLD_SET(DBG_BWM_CKH_STS_FLD_max_bst_alloc_num_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_max_bwm_alloc_num_err(reg32, val)  REG_FLD_SET(DBG_BWM_CKH_STS_FLD_max_bwm_alloc_num_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_alloc_hec_uc_err(reg32, val)       REG_FLD_SET(DBG_BWM_CKH_STS_FLD_alloc_hec_uc_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_bst_split_err(reg32, val)          REG_FLD_SET(DBG_BWM_CKH_STS_FLD_bst_split_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_my_tid_ins_bst_err(reg32, val)     REG_FLD_SET(DBG_BWM_CKH_STS_FLD_my_tid_ins_bst_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_min_gsize_err(reg32, val)          REG_FLD_SET(DBG_BWM_CKH_STS_FLD_min_gsize_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_max_gsize_err(reg32, val)          REG_FLD_SET(DBG_BWM_CKH_STS_FLD_max_gsize_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_start_time_order_err(reg32, val)   REG_FLD_SET(DBG_BWM_CKH_STS_FLD_start_time_order_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_max_start_time_err(reg32, val)     REG_FLD_SET(DBG_BWM_CKH_STS_FLD_max_start_time_err, (reg32), (val))
#define DBG_BWM_CKH_STS_SET_min_bst_intvl_err(reg32, val)      REG_FLD_SET(DBG_BWM_CKH_STS_FLD_min_bst_intvl_err, (reg32), (val))

#define DBG_BWM_SFIFO_STS_SET_sgl_fifo_max_used(reg32, val)    REG_FLD_SET(DBG_BWM_SFIFO_STS_FLD_sgl_fifo_max_used, (reg32), (val))
#define DBG_BWM_SFIFO_STS_SET_sgl_fifo_used(reg32, val)        REG_FLD_SET(DBG_BWM_SFIFO_STS_FLD_sgl_fifo_used, (reg32), (val))

#define DBG_BWM_BFIFO_STS_SET_bst_fifo_max_used(reg32, val)    REG_FLD_SET(DBG_BWM_BFIFO_STS_FLD_bst_fifo_max_used, (reg32), (val))
#define DBG_BWM_BFIFO_STS_SET_bst_fifo_used(reg32, val)        REG_FLD_SET(DBG_BWM_BFIFO_STS_FLD_bst_fifo_used, (reg32), (val))

#define DBG_TX_ALIGN_FIFO_STS_SET_tx_align_fifo_max_used(reg32, val) REG_FLD_SET(DBG_TX_ALIGN_FIFO_STS_FLD_tx_align_fifo_max_used, (reg32), (val))
#define DBG_TX_ALIGN_FIFO_STS_SET_tx_align_fifo_used(reg32, val) REG_FLD_SET(DBG_TX_ALIGN_FIFO_STS_FLD_tx_align_fifo_used, (reg32), (val))

#define DBG_TX_AES_MAX_USED_SET_tx_aes_eng_max_used(reg32, val) REG_FLD_SET(DBG_TX_AES_MAX_USED_FLD_tx_aes_eng_max_used, (reg32), (val))
#define DBG_TX_AES_MAX_USED_SET_tx_rdm_ciph_fifo_max_used(reg32, val) REG_FLD_SET(DBG_TX_AES_MAX_USED_FLD_tx_rdm_ciph_fifo_max_used, (reg32), (val))
#define DBG_TX_AES_MAX_USED_SET_tx_cryp_cnt_fifo_max_used(reg32, val) REG_FLD_SET(DBG_TX_AES_MAX_USED_FLD_tx_cryp_cnt_fifo_max_used, (reg32), (val))
#define DBG_TX_AES_MAX_USED_SET_tx_ciph_txt_fifo_max_used(reg32, val) REG_FLD_SET(DBG_TX_AES_MAX_USED_FLD_tx_ciph_txt_fifo_max_used, (reg32), (val))
#define DBG_TX_AES_MAX_USED_SET_tx_aes_hdr_fifo_max_used(reg32, val) REG_FLD_SET(DBG_TX_AES_MAX_USED_FLD_tx_aes_hdr_fifo_max_used, (reg32), (val))

#if defined(TCSUPPORT_CPU_AN7583)
#define DBG_DBA_BACK_DOOR_SET_dba_modify_tcont_idx_1_mode(reg32, val) REG_FLD_SET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1_mode, (reg32), (val))
#define DBG_DBA_BACK_DOOR_SET_dba_modify_tcont_idx_1_ovrw(reg32, val) REG_FLD_SET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1_ovrw, (reg32), (val))
#define DBG_DBA_BACK_DOOR_SET_dba_modify_tcont_idx_1(reg32, val) REG_FLD_SET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1, (reg32), (val))
#define DBG_DBA_BACK_DOOR_SET_dba_modify_tcont_idx_0_mode(reg32, val) REG_FLD_SET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0_mode, (reg32), (val))
#define DBG_DBA_BACK_DOOR_SET_dba_modify_tcont_idx_0_ovrw(reg32, val) REG_FLD_SET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0_ovrw, (reg32), (val))
#define DBG_DBA_BACK_DOOR_SET_dba_modify_tcont_idx_0(reg32, val) REG_FLD_SET(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0, (reg32), (val))
#endif

#define DBG_DBA_BACK_DOOR_SET_dba_modify_mode(reg32, val)      REG_FLD_SET(DBG_DBA_BACK_DOOR_FLD_dba_modify_mode, (reg32), (val))

#define DBG_DBA_MODIFY_SET_dba_modify(reg32, val)              REG_FLD_SET(DBG_DBA_MODIFY_FLD_dba_modify, (reg32), (val))

#define DBG_RESYNC_SET_tx_sync_rdy(reg32, val)                 REG_FLD_SET(DBG_RESYNC_FLD_tx_sync_rdy, (reg32), (val))
#define DBG_RESYNC_SET_sw_resync_en(reg32, val)                REG_FLD_SET(DBG_RESYNC_FLD_sw_resync_en, (reg32), (val))
#define DBG_RESYNC_SET_sw_resync_start(reg32, val)             REG_FLD_SET(DBG_RESYNC_FLD_sw_resync_start, (reg32), (val))

#define DBG_PHY_DLY_CTRL_SET_use_mpi_rx_phy_dly_ofs(reg32, val) REG_FLD_SET(DBG_PHY_DLY_CTRL_FLD_use_mpi_rx_phy_dly_ofs, (reg32), (val))
#define DBG_PHY_DLY_CTRL_SET_o4_o5_same_phy_dly(reg32, val)    REG_FLD_SET(DBG_PHY_DLY_CTRL_FLD_o4_o5_same_phy_dly, (reg32), (val))

#define DBG_DS_SPF_CNT_L_SET_ds_spf_cnt_l32(reg32, val)        REG_FLD_SET(DBG_DS_SPF_CNT_L_FLD_ds_spf_cnt_l32, (reg32), (val))

#define DBG_DS_SPF_CNT_H_SET_ds_spf_cnt_h19(reg32, val)        REG_FLD_SET(DBG_DS_SPF_CNT_H_FLD_ds_spf_cnt_h19, (reg32), (val))

#define DBG_PON_ID_L_SET_ds_pon_id_l32(reg32, val)             REG_FLD_SET(DBG_PON_ID_L_FLD_ds_pon_id_l32, (reg32), (val))

#define DBG_PON_ID_H_SET_ds_pon_id_h19(reg32, val)             REG_FLD_SET(DBG_PON_ID_H_FLD_ds_pon_id_h19, (reg32), (val))

#define DBG_TX_SYNC_OFFSET_SET_tx_sync_offset(reg32, val)      REG_FLD_SET(DBG_TX_SYNC_OFFSET_FLD_tx_sync_offset, (reg32), (val))

#define DBG_RX_PHY_DLY_OFS_SET_rx_phy_dly_offset(reg32, val)   REG_FLD_SET(DBG_RX_PHY_DLY_OFS_FLD_rx_phy_dly_offset, (reg32), (val))

#define DBG_RX_HEC_ERR_SET_rx_hec_err_sts(reg32, val)          REG_FLD_SET(DBG_RX_HEC_ERR_FLD_rx_hec_err_sts, (reg32), (val))
#define DBG_RX_HEC_ERR_SET_rx_hdr_hec_3err(reg32, val)         REG_FLD_SET(DBG_RX_HEC_ERR_FLD_rx_hdr_hec_3err, (reg32), (val))
#define DBG_RX_HEC_ERR_SET_rx_hdr_hec_2err(reg32, val)         REG_FLD_SET(DBG_RX_HEC_ERR_FLD_rx_hdr_hec_2err, (reg32), (val))
#define DBG_RX_HEC_ERR_SET_rx_hdr_hec_1err(reg32, val)         REG_FLD_SET(DBG_RX_HEC_ERR_FLD_rx_hdr_hec_1err, (reg32), (val))
#define DBG_RX_HEC_ERR_SET_rx_alloc_hec_3err(reg32, val)       REG_FLD_SET(DBG_RX_HEC_ERR_FLD_rx_alloc_hec_3err, (reg32), (val))
#define DBG_RX_HEC_ERR_SET_rx_alloc_hec_2err(reg32, val)       REG_FLD_SET(DBG_RX_HEC_ERR_FLD_rx_alloc_hec_2err, (reg32), (val))
#define DBG_RX_HEC_ERR_SET_rx_alloc_hec_1err(reg32, val)       REG_FLD_SET(DBG_RX_HEC_ERR_FLD_rx_alloc_hec_1err, (reg32), (val))
#define DBG_RX_HEC_ERR_SET_rx_hlend_hec_3err(reg32, val)       REG_FLD_SET(DBG_RX_HEC_ERR_FLD_rx_hlend_hec_3err, (reg32), (val))
#define DBG_RX_HEC_ERR_SET_rx_hlend_hec_2err(reg32, val)       REG_FLD_SET(DBG_RX_HEC_ERR_FLD_rx_hlend_hec_2err, (reg32), (val))
#define DBG_RX_HEC_ERR_SET_rx_hlend_hec_1err(reg32, val)       REG_FLD_SET(DBG_RX_HEC_ERR_FLD_rx_hlend_hec_1err, (reg32), (val))

#define RX_GEM_ENC_CHK_SET_rx_enc_chk_err_sts(reg32, val)      REG_FLD_SET(RX_GEM_ENC_CHK_FLD_rx_enc_chk_err_sts, (reg32), (val))
#define RX_GEM_ENC_CHK_SET_rx_enc_chk_done(reg32, val)         REG_FLD_SET(RX_GEM_ENC_CHK_FLD_rx_enc_chk_done, (reg32), (val))
#define RX_GEM_ENC_CHK_SET_rx_enc_chk_en(reg32, val)           REG_FLD_SET(RX_GEM_ENC_CHK_FLD_rx_enc_chk_en, (reg32), (val))
#define RX_GEM_ENC_CHK_SET_exp_rx_enc_sts(reg32, val)          REG_FLD_SET(RX_GEM_ENC_CHK_FLD_exp_rx_enc_sts, (reg32), (val))
#define RX_GEM_ENC_CHK_SET_rx_enc_chk_gpid(reg32, val)         REG_FLD_SET(RX_GEM_ENC_CHK_FLD_rx_enc_chk_gpid, (reg32), (val))

#define RX_MBI_FIFO_DROP_EN_SET_rx_mbi_fifo_drop_en(reg32, val) REG_FLD_SET(RX_MBI_FIFO_DROP_EN_FLD_rx_mbi_fifo_drop_en, (reg32), (val))

#define RX_MBI_HDR_FIFO_DROP_CTRL_SET_rx_mbi_hdr_fifo_hth(reg32, val) REG_FLD_SET(RX_MBI_HDR_FIFO_DROP_CTRL_FLD_rx_mbi_hdr_fifo_hth, (reg32), (val))
#define RX_MBI_HDR_FIFO_DROP_CTRL_SET_rx_mbi_hdr_fifo_lth(reg32, val) REG_FLD_SET(RX_MBI_HDR_FIFO_DROP_CTRL_FLD_rx_mbi_hdr_fifo_lth, (reg32), (val))

#define RX_MBI_PL_FIFO_DROP_CTRL_SET_rx_mbi_pl_fifo_hth(reg32, val) REG_FLD_SET(RX_MBI_PL_FIFO_DROP_CTRL_FLD_rx_mbi_pl_fifo_hth, (reg32), (val))
#define RX_MBI_PL_FIFO_DROP_CTRL_SET_rx_mbi_pl_fifo_lth(reg32, val) REG_FLD_SET(RX_MBI_PL_FIFO_DROP_CTRL_FLD_rx_mbi_pl_fifo_lth, (reg32), (val))

#if defined(TCSUPPORT_CPU_EN7581)
#define GET_RX_IDLE_XGEM_SET_get_rx_idle_xgem(reg32, val)      REG_FLD_SET(GET_RX_IDLE_XGEM_FLD_get_rx_idle_xgem, (reg32), (val))
#define GET_RX_IDLE_XGEM_SET_rx_idle_xgem_len(reg32, val)      REG_FLD_SET(GET_RX_IDLE_XGEM_FLD_rx_idle_xgem_len, (reg32), (val))

#define RX_IDLE_XGEM_1ST_PL_SET_rx_idle_xgem_1st_pl(reg32, val) REG_FLD_SET(RX_IDLE_XGEM_1ST_PL_FLD_rx_idle_xgem_1st_pl, (reg32), (val))

#endif

#if defined(TCSUPPORT_CPU_AN7583)
#define DBG_DBA_MODIFY_IDX_0_SET_dba_modify_idx_0(reg32, val)  REG_FLD_SET(DBG_DBA_MODIFY_IDX_0_FLD_dba_modify_idx_0, (reg32), (val))

#define DBG_DBA_MODIFY_IDX_1_SET_dba_modify_idx_1(reg32, val)  REG_FLD_SET(DBG_DBA_MODIFY_IDX_1_FLD_dba_modify_idx_1, (reg32), (val))
#endif

#define CNT_CLR_SET_nml_cnt_clr(reg32, val)                    REG_FLD_SET(CNT_CLR_FLD_nml_cnt_clr, (reg32), (val))
#define CNT_CLR_SET_err_cnt_clr(reg32, val)                    REG_FLD_SET(CNT_CLR_FLD_err_cnt_clr, (reg32), (val))

#define RX_HLEND_HEC_CNT_SET_rx_hlend_hec_3err_cnt(reg32, val) REG_FLD_SET(RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_3err_cnt, (reg32), (val))
#define RX_HLEND_HEC_CNT_SET_rx_hlend_hec_2err_cnt(reg32, val) REG_FLD_SET(RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_2err_cnt, (reg32), (val))
#define RX_HLEND_HEC_CNT_SET_rx_hlend_hec_1err_cnt(reg32, val) REG_FLD_SET(RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_1err_cnt, (reg32), (val))

#define RX_ALLOC_HEC_CNT_SET_rx_alloc_hec_3err_cnt(reg32, val) REG_FLD_SET(RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_3err_cnt, (reg32), (val))
#define RX_ALLOC_HEC_CNT_SET_rx_alloc_hec_2err_cnt(reg32, val) REG_FLD_SET(RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_2err_cnt, (reg32), (val))
#define RX_ALLOC_HEC_CNT_SET_rx_alloc_hec_1err_cnt(reg32, val) REG_FLD_SET(RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_1err_cnt, (reg32), (val))

#define RX_HDR_HEC_CNT_SET_rx_hdr_hec_3err_cnt(reg32, val)     REG_FLD_SET(RX_HDR_HEC_CNT_FLD_rx_hdr_hec_3err_cnt, (reg32), (val))
#define RX_HDR_HEC_CNT_SET_rx_hdr_hec_2err_cnt(reg32, val)     REG_FLD_SET(RX_HDR_HEC_CNT_FLD_rx_hdr_hec_2err_cnt, (reg32), (val))
#define RX_HDR_HEC_CNT_SET_rx_hdr_hec_1err_cnt(reg32, val)     REG_FLD_SET(RX_HDR_HEC_CNT_FLD_rx_hdr_hec_1err_cnt, (reg32), (val))

#define RX_PHY_HEC_ERR_CNT_SET_rx_pon_id_hec_err_cnt(reg32, val) REG_FLD_SET(RX_PHY_HEC_ERR_CNT_FLD_rx_pon_id_hec_err_cnt, (reg32), (val))
#define RX_PHY_HEC_ERR_CNT_SET_rx_sfc_hec_err_cnt(reg32, val)  REG_FLD_SET(RX_PHY_HEC_ERR_CNT_FLD_rx_sfc_hec_err_cnt, (reg32), (val))

#define RX_MIC_ERR_CNT_SET_rx_omci_mic_err_cnt(reg32, val)     REG_FLD_SET(RX_MIC_ERR_CNT_FLD_rx_omci_mic_err_cnt, (reg32), (val))
#define RX_MIC_ERR_CNT_SET_rx_ploam_mic_err_cnt(reg32, val)    REG_FLD_SET(RX_MIC_ERR_CNT_FLD_rx_ploam_mic_err_cnt, (reg32), (val))

#define RX_ETH_FCS_ERR_CNT_SET_rx_eth_crc_err_cnt(reg32, val)  REG_FLD_SET(RX_ETH_FCS_ERR_CNT_FLD_rx_eth_crc_err_cnt, (reg32), (val))

#define RX_BIP_ERR_CNT_SET_rx_bip_err_cnt(reg32, val)          REG_FLD_SET(RX_BIP_ERR_CNT_FLD_rx_bip_err_cnt, (reg32), (val))

#define RX_KEY_ERR_CNT_SET_rx_key_err_cnt(reg32, val)          REG_FLD_SET(RX_KEY_ERR_CNT_FLD_rx_key_err_cnt, (reg32), (val))

#define RX_LOST_WCNT_SET_rx_lost_wcnt(reg32, val)              REG_FLD_SET(RX_LOST_WCNT_FLD_rx_lost_wcnt, (reg32), (val))

#define INVLD_PROF_BST_GNT_CNT_SET_invld_prof_bst_gnt_cnt(reg32, val) REG_FLD_SET(INVLD_PROF_BST_GNT_CNT_FLD_invld_prof_bst_gnt_cnt, (reg32), (val))

#define RX_MBI_XGEM_DROP_CNT_SET_rx_mbi_xgem_drop_cnt(reg32, val) REG_FLD_SET(RX_MBI_XGEM_DROP_CNT_FLD_rx_mbi_xgem_drop_cnt, (reg32), (val))

#define RX_XGTC_CNT_SET_rx_mpi_sof_cnt(reg32, val)             REG_FLD_SET(RX_XGTC_CNT_FLD_rx_mpi_sof_cnt, (reg32), (val))

#define TX_BST_CNT_SET_tx_mpi_sof_cnt(reg32, val)              REG_FLD_SET(TX_BST_CNT_FLD_tx_mpi_sof_cnt, (reg32), (val))

#define RX_PLOAMD_CNT_SET_rx_ploamd_cnt(reg32, val)            REG_FLD_SET(RX_PLOAMD_CNT_FLD_rx_ploamd_cnt, (reg32), (val))

#define TX_PLOAMU_CNT_SET_tx_ploamu_cnt(reg32, val)            REG_FLD_SET(TX_PLOAMU_CNT_FLD_tx_ploamu_cnt, (reg32), (val))

#define RX_OMCI_CNT_SET_rx_omci_cnt_fe(reg32, val)             REG_FLD_SET(RX_OMCI_CNT_FLD_rx_omci_cnt_fe, (reg32), (val))
#define RX_OMCI_CNT_SET_rx_omci_cnt_mac(reg32, val)            REG_FLD_SET(RX_OMCI_CNT_FLD_rx_omci_cnt_mac, (reg32), (val))

#define TX_OMCI_CNT_SET_tx_omci_cnt_fe(reg32, val)             REG_FLD_SET(TX_OMCI_CNT_FLD_tx_omci_cnt_fe, (reg32), (val))
#define TX_OMCI_CNT_SET_tx_omci_cnt_mac(reg32, val)            REG_FLD_SET(TX_OMCI_CNT_FLD_tx_omci_cnt_mac, (reg32), (val))

#define RX_XGEM_CNT_SET_rx_xgem_cnt(reg32, val)                REG_FLD_SET(RX_XGEM_CNT_FLD_rx_xgem_cnt, (reg32), (val))

#define TX_XGEM_CNT_SET_tx_xgem_cnt(reg32, val)                REG_FLD_SET(TX_XGEM_CNT_FLD_tx_xgem_cnt, (reg32), (val))

#define RX_MBI_CNT_SET_rx_mbi_ack_cnt(reg32, val)              REG_FLD_SET(RX_MBI_CNT_FLD_rx_mbi_ack_cnt, (reg32), (val))

#define TX_MBI_CNT_SET_tx_mbi_ack_cnt(reg32, val)              REG_FLD_SET(TX_MBI_CNT_FLD_tx_mbi_ack_cnt, (reg32), (val))

#define RX_NON_IDLE_BCNT_SET_rx_non_idle_bcnt(reg32, val)      REG_FLD_SET(RX_NON_IDLE_BCNT_FLD_rx_non_idle_bcnt, (reg32), (val))

#define TX_NON_IDLE_BCNT_SET_tx_non_idle_bcnt(reg32, val)      REG_FLD_SET(TX_NON_IDLE_BCNT_FLD_tx_non_idle_bcnt, (reg32), (val))

#define TX_NLF_XGEM_CNT_SET_tx_nlf_xgem_cnt(reg32, val)        REG_FLD_SET(TX_NLF_XGEM_CNT_FLD_tx_nlf_xgem_cnt, (reg32), (val))

#define TX_ACK_PLOAMU_CNT_SET_tx_ack_ploamu_cnt(reg32, val)    REG_FLD_SET(TX_ACK_PLOAMU_CNT_FLD_tx_ack_ploamu_cnt, (reg32), (val))

#define RX_BIP_PROTECT_WCNT_SET_rx_bip_protect_wcnt(reg32, val) REG_FLD_SET(RX_BIP_PROTECT_WCNT_FLD_rx_bip_protect_wcnt, (reg32), (val))

#define TX_IDLE_BCNT_SET_tx_idle_bcnt(reg32, val)              REG_FLD_SET(TX_IDLE_BCNT_FLD_tx_idle_bcnt, (reg32), (val))

#define SNF_CTRL_SET_snf_us_en(reg32, val)                     REG_FLD_SET(SNF_CTRL_FLD_snf_us_en, (reg32), (val))
#define SNF_CTRL_SET_snf_ds_en(reg32, val)                     REG_FLD_SET(SNF_CTRL_FLD_snf_ds_en, (reg32), (val))
#if defined(TCSUPPORT_CPU_AN7583)
#define SNF_CTRL_SET_snf_us_ovrw_8b_xgem(reg32, val)           REG_FLD_SET(SNF_CTRL_FLD_snf_us_ovrw_8b_xgem, (reg32), (val))
#endif
#define SNF_CTRL_SET_snf_us_ovrw_my_xgem(reg32, val)           REG_FLD_SET(SNF_CTRL_FLD_snf_us_ovrw_my_xgem, (reg32), (val))

#define SNF_GPID_SET_us_snf_gpid(reg32, val)                   REG_FLD_SET(SNF_GPID_FLD_us_snf_gpid, (reg32), (val))
#define SNF_GPID_SET_ds_snf_gpid(reg32, val)                   REG_FLD_SET(SNF_GPID_FLD_ds_snf_gpid, (reg32), (val))

#define DS_SNF_ETH_DASA_H16_SET_ds_snf_eth_da_h16(reg32, val)  REG_FLD_SET(DS_SNF_ETH_DASA_H16_FLD_ds_snf_eth_da_h16, (reg32), (val))
#define DS_SNF_ETH_DASA_H16_SET_ds_snf_eth_sa_h16(reg32, val)  REG_FLD_SET(DS_SNF_ETH_DASA_H16_FLD_ds_snf_eth_sa_h16, (reg32), (val))

#define SNF_ETH_TAG_SET_snf_eth_tpid(reg32, val)               REG_FLD_SET(SNF_ETH_TAG_FLD_snf_eth_tpid, (reg32), (val))

#define SNF_ETH_TYPE_SET_snf_eth_type(reg32, val)              REG_FLD_SET(SNF_ETH_TYPE_FLD_snf_eth_type, (reg32), (val))

#define US_SNF_ETH_DASA_H16_SET_us_snf_eth_da_h16(reg32, val)  REG_FLD_SET(US_SNF_ETH_DASA_H16_FLD_us_snf_eth_da_h16, (reg32), (val))
#define US_SNF_ETH_DASA_H16_SET_us_snf_eth_sa_h16(reg32, val)  REG_FLD_SET(US_SNF_ETH_DASA_H16_FLD_us_snf_eth_sa_h16, (reg32), (val))

#define DBG_PROBE_CTRL_SET_probe_clk_en(reg32, val)            REG_FLD_SET(DBG_PROBE_CTRL_FLD_probe_clk_en, (reg32), (val))
#define DBG_PROBE_CTRL_SET_probe_trig_en(reg32, val)           REG_FLD_SET(DBG_PROBE_CTRL_FLD_probe_trig_en, (reg32), (val))
#define DBG_PROBE_CTRL_SET_probe_trig_bit_sel(reg32, val)      REG_FLD_SET(DBG_PROBE_CTRL_FLD_probe_trig_bit_sel, (reg32), (val))
#define DBG_PROBE_CTRL_SET_probe_trig_sel(reg32, val)          REG_FLD_SET(DBG_PROBE_CTRL_FLD_probe_trig_sel, (reg32), (val))
#define DBG_PROBE_CTRL_SET_probe_sel(reg32, val)               REG_FLD_SET(DBG_PROBE_CTRL_FLD_probe_sel, (reg32), (val))

#define DBG_PROBE_HIGH32_SET_probe_h32(reg32, val)             REG_FLD_SET(DBG_PROBE_HIGH32_FLD_probe_h32, (reg32), (val))

#define DBG_PROBE_LOW32_SET_probe_l32(reg32, val)              REG_FLD_SET(DBG_PROBE_LOW32_FLD_probe_l32, (reg32), (val))

#define SW_RST_VAL_xgpon_mac_sw_rst_n(val)                     REG_FLD_VAL(SW_RST_FLD_xgpon_mac_sw_rst_n, (val))

#define MBI_MPI_STOP_VAL_mpi_tx_stop_done(val)                 REG_FLD_VAL(MBI_MPI_STOP_FLD_mpi_tx_stop_done, (val))
#define MBI_MPI_STOP_VAL_mpi_rx_stop_done(val)                 REG_FLD_VAL(MBI_MPI_STOP_FLD_mpi_rx_stop_done, (val))
#define MBI_MPI_STOP_VAL_mpi_tx_stop(val)                      REG_FLD_VAL(MBI_MPI_STOP_FLD_mpi_tx_stop, (val))
#define MBI_MPI_STOP_VAL_mpi_rx_stop(val)                      REG_FLD_VAL(MBI_MPI_STOP_FLD_mpi_rx_stop, (val))
#define MBI_MPI_STOP_VAL_mbi_tx_stop_done(val)                 REG_FLD_VAL(MBI_MPI_STOP_FLD_mbi_tx_stop_done, (val))
#define MBI_MPI_STOP_VAL_mbi_rx_stop_done(val)                 REG_FLD_VAL(MBI_MPI_STOP_FLD_mbi_rx_stop_done, (val))
#if defined(TCSUPPORT_CPU_EN7581)
#define MBI_MPI_STOP_VAL_dbru_stop_done(val)                   REG_FLD_VAL(MBI_MPI_STOP_FLD_dbru_stop_done, (val))
#define MBI_MPI_STOP_VAL_dbru_stop(val)                        REG_FLD_VAL(MBI_MPI_STOP_FLD_dbru_stop, (val))
#endif
#define MBI_MPI_STOP_VAL_mbi_tx_stop(val)                      REG_FLD_VAL(MBI_MPI_STOP_FLD_mbi_tx_stop, (val))
#if defined(TCSUPPORT_CPU_EN7581)
#define MBI_MPI_STOP_VAL_del_rx_stop(val)                      REG_FLD_VAL(MBI_MPI_STOP_FLD_del_rx_stop, (val))
#endif
#define MBI_MPI_STOP_VAL_mbi_rx_stop(val)                      REG_FLD_VAL(MBI_MPI_STOP_FLD_mbi_rx_stop, (val))

#define VENDOR_ID_VAL_vendor_id(val)                           REG_FLD_VAL(VENDOR_ID_FLD_vendor_id, (val))

#define VS_SN_VAL_vs_sn(val)                                   REG_FLD_VAL(VS_SN_FLD_vs_sn, (val))

#define ONU_ID_VAL_onu_id_vld(val)                             REG_FLD_VAL(ONU_ID_FLD_onu_id_vld, (val))
#define ONU_ID_VAL_onu_id(val)                                 REG_FLD_VAL(ONU_ID_FLD_onu_id, (val))

#define RGS_ID3_0_VAL_rgs_id3_0(val)                           REG_FLD_VAL(RGS_ID3_0_FLD_rgs_id3_0, (val))

#define RGS_ID7_4_VAL_rgs_id7_4(val)                           REG_FLD_VAL(RGS_ID7_4_FLD_rgs_id7_4, (val))

#define RGS_ID11_8_VAL_rgs_id11_8(val)                         REG_FLD_VAL(RGS_ID11_8_FLD_rgs_id11_8, (val))

#define RGS_ID15_12_VAL_rgs_id15_12(val)                       REG_FLD_VAL(RGS_ID15_12_FLD_rgs_id15_12, (val))

#define RGS_ID19_16_VAL_rgs_id19_16(val)                       REG_FLD_VAL(RGS_ID19_16_FLD_rgs_id19_16, (val))

#define RGS_ID23_20_VAL_rgs_id23_20(val)                       REG_FLD_VAL(RGS_ID23_20_FLD_rgs_id23_20, (val))

#define RGS_ID27_24_VAL_rgs_id27_24(val)                       REG_FLD_VAL(RGS_ID27_24_FLD_rgs_id27_24, (val))

#define RGS_ID31_28_VAL_rgs_id31_28(val)                       REG_FLD_VAL(RGS_ID31_28_FLD_rgs_id31_28, (val))

#define RGS_ID35_32_VAL_rgs_id35_32(val)                       REG_FLD_VAL(RGS_ID35_32_FLD_rgs_id35_32, (val))

#define INT_ENABLE_VAL_o5_sn_onu_req_recv_int_en(val)          REG_FLD_VAL(INT_ENABLE_FLD_o5_sn_onu_req_recv_int_en, (val))
#define INT_ENABLE_VAL_turning_sfc_match_int_en(val)           REG_FLD_VAL(INT_ENABLE_FLD_turning_sfc_match_int_en, (val))
#define INT_ENABLE_VAL_o9_gnt_recv_int_en(val)                 REG_FLD_VAL(INT_ENABLE_FLD_o9_gnt_recv_int_en, (val))
#define INT_ENABLE_VAL_sw1_mic_done_int_en(val)                REG_FLD_VAL(INT_ENABLE_FLD_sw1_mic_done_int_en, (val))
#define INT_ENABLE_VAL_sw0_mic_done_int_en(val)                REG_FLD_VAL(INT_ENABLE_FLD_sw0_mic_done_int_en, (val))
#define INT_ENABLE_VAL_key_cal_done_int_en(val)                REG_FLD_VAL(INT_ENABLE_FLD_key_cal_done_int_en, (val))
#define INT_ENABLE_VAL_lwi_int_en(val)                         REG_FLD_VAL(INT_ENABLE_FLD_lwi_int_en, (val))
#define INT_ENABLE_VAL_fwi_int_en(val)                         REG_FLD_VAL(INT_ENABLE_FLD_fwi_int_en, (val))
#define INT_ENABLE_VAL_rx_err_int_en(val)                      REG_FLD_VAL(INT_ENABLE_FLD_rx_err_int_en, (val))
#define INT_ENABLE_VAL_tx_err_int_en(val)                      REG_FLD_VAL(INT_ENABLE_FLD_tx_err_int_en, (val))
#define INT_ENABLE_VAL_fifo_err_int_en(val)                    REG_FLD_VAL(INT_ENABLE_FLD_fifo_err_int_en, (val))
#define INT_ENABLE_VAL_o5_eqd_adj_done_int_en(val)             REG_FLD_VAL(INT_ENABLE_FLD_o5_eqd_adj_done_int_en, (val))
#define INT_ENABLE_VAL_bwm_chk_err_int_en(val)                 REG_FLD_VAL(INT_ENABLE_FLD_bwm_chk_err_int_en, (val))
#define INT_ENABLE_VAL_dying_gasp_send_int_en(val)             REG_FLD_VAL(INT_ENABLE_FLD_dying_gasp_send_int_en, (val))
#define INT_ENABLE_VAL_tod_1pps_int_en(val)                    REG_FLD_VAL(INT_ENABLE_FLD_tod_1pps_int_en, (val))
#define INT_ENABLE_VAL_tod_update_done_int_en(val)             REG_FLD_VAL(INT_ENABLE_FLD_tod_update_done_int_en, (val))
#define INT_ENABLE_VAL_olt_ds_fec_chg_int_en(val)              REG_FLD_VAL(INT_ENABLE_FLD_olt_ds_fec_chg_int_en, (val))
#define INT_ENABLE_VAL_us_prof_idx_chg_int_en(val)             REG_FLD_VAL(INT_ENABLE_FLD_us_prof_idx_chg_int_en, (val))
#define INT_ENABLE_VAL_us_key_switch_done_int_en(val)          REG_FLD_VAL(INT_ENABLE_FLD_us_key_switch_done_int_en, (val))
#define INT_ENABLE_VAL_us_no_msg_send_int_en(val)              REG_FLD_VAL(INT_ENABLE_FLD_us_no_msg_send_int_en, (val))
#define INT_ENABLE_VAL_o4_registration_send_int_en(val)        REG_FLD_VAL(INT_ENABLE_FLD_o4_registration_send_int_en, (val))
#define INT_ENABLE_VAL_o4_ranging_req_recv_int_en(val)         REG_FLD_VAL(INT_ENABLE_FLD_o4_ranging_req_recv_int_en, (val))
#define INT_ENABLE_VAL_o23_sn_onu_send_int_en(val)             REG_FLD_VAL(INT_ENABLE_FLD_o23_sn_onu_send_int_en, (val))
#define INT_ENABLE_VAL_o23_sn_onu_req_recv_int_en(val)         REG_FLD_VAL(INT_ENABLE_FLD_o23_sn_onu_req_recv_int_en, (val))
#define INT_ENABLE_VAL_ploamu_send_int_en(val)                 REG_FLD_VAL(INT_ENABLE_FLD_ploamu_send_int_en, (val))
#define INT_ENABLE_VAL_ploamd_recv_int_en(val)                 REG_FLD_VAL(INT_ENABLE_FLD_ploamd_recv_int_en, (val))

#define INT_STATUS_VAL_o5_sn_onu_req_recv_int(val)             REG_FLD_VAL(INT_STATUS_FLD_o5_sn_onu_req_recv_int, (val))
#define INT_STATUS_VAL_turning_sfc_match_int(val)              REG_FLD_VAL(INT_STATUS_FLD_turning_sfc_match_int, (val))
#define INT_STATUS_VAL_o9_gnt_recv_int(val)                    REG_FLD_VAL(INT_STATUS_FLD_o9_gnt_recv_int, (val))
#define INT_STATUS_VAL_sw1_mic_done_int(val)                   REG_FLD_VAL(INT_STATUS_FLD_sw1_mic_done_int, (val))
#define INT_STATUS_VAL_sw0_mic_done_int(val)                   REG_FLD_VAL(INT_STATUS_FLD_sw0_mic_done_int, (val))
#define INT_STATUS_VAL_key_cal_done_int(val)                   REG_FLD_VAL(INT_STATUS_FLD_key_cal_done_int, (val))
#define INT_STATUS_VAL_lwi_int(val)                            REG_FLD_VAL(INT_STATUS_FLD_lwi_int, (val))
#define INT_STATUS_VAL_fwi_int(val)                            REG_FLD_VAL(INT_STATUS_FLD_fwi_int, (val))
#define INT_STATUS_VAL_rx_err_int(val)                         REG_FLD_VAL(INT_STATUS_FLD_rx_err_int, (val))
#define INT_STATUS_VAL_tx_err_int(val)                         REG_FLD_VAL(INT_STATUS_FLD_tx_err_int, (val))
#define INT_STATUS_VAL_fifo_err_int(val)                       REG_FLD_VAL(INT_STATUS_FLD_fifo_err_int, (val))
#define INT_STATUS_VAL_o5_eqd_adj_done_int(val)                REG_FLD_VAL(INT_STATUS_FLD_o5_eqd_adj_done_int, (val))
#define INT_STATUS_VAL_bwm_chk_err_int(val)                    REG_FLD_VAL(INT_STATUS_FLD_bwm_chk_err_int, (val))
#define INT_STATUS_VAL_dying_gasp_send_int(val)                REG_FLD_VAL(INT_STATUS_FLD_dying_gasp_send_int, (val))
#define INT_STATUS_VAL_tod_1pps_int(val)                       REG_FLD_VAL(INT_STATUS_FLD_tod_1pps_int, (val))
#define INT_STATUS_VAL_tod_update_done_int(val)                REG_FLD_VAL(INT_STATUS_FLD_tod_update_done_int, (val))
#define INT_STATUS_VAL_olt_ds_fec_chg_int(val)                 REG_FLD_VAL(INT_STATUS_FLD_olt_ds_fec_chg_int, (val))
#define INT_STATUS_VAL_us_prof_idx_chg_int(val)                REG_FLD_VAL(INT_STATUS_FLD_us_prof_idx_chg_int, (val))
#define INT_STATUS_VAL_us_key_switch_done_int(val)             REG_FLD_VAL(INT_STATUS_FLD_us_key_switch_done_int, (val))
#define INT_STATUS_VAL_us_no_msg_send_int(val)                 REG_FLD_VAL(INT_STATUS_FLD_us_no_msg_send_int, (val))
#define INT_STATUS_VAL_o4_registration_send_int(val)           REG_FLD_VAL(INT_STATUS_FLD_o4_registration_send_int, (val))
#define INT_STATUS_VAL_o4_ranging_req_recv_int(val)            REG_FLD_VAL(INT_STATUS_FLD_o4_ranging_req_recv_int, (val))
#define INT_STATUS_VAL_o23_sn_onu_send_int(val)                REG_FLD_VAL(INT_STATUS_FLD_o23_sn_onu_send_int, (val))
#define INT_STATUS_VAL_o23_sn_onu_req_recv_int(val)            REG_FLD_VAL(INT_STATUS_FLD_o23_sn_onu_req_recv_int, (val))
#define INT_STATUS_VAL_ploamu_send_int(val)                    REG_FLD_VAL(INT_STATUS_FLD_ploamu_send_int, (val))
#define INT_STATUS_VAL_ploamd_recv_int(val)                    REG_FLD_VAL(INT_STATUS_FLD_ploamd_recv_int, (val))

#define FIFO_ERR_STS_VAL_tx_aes_rdm_ciph_fifo_ovrn(val)        REG_FLD_VAL(FIFO_ERR_STS_FLD_tx_aes_rdm_ciph_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_byte_mib_cmd_fifo_ovrn(val)           REG_FLD_VAL(FIFO_ERR_STS_FLD_byte_mib_cmd_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_frm_mib_cmd_fifo_ovrn(val)            REG_FLD_VAL(FIFO_ERR_STS_FLD_frm_mib_cmd_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_mib_cmd_fifo_ovrn(val)                REG_FLD_VAL(FIFO_ERR_STS_FLD_mib_cmd_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_mib_tx_cmd_fifo_ovrn(val)             REG_FLD_VAL(FIFO_ERR_STS_FLD_mib_tx_cmd_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_snf_ds_fifo_ovrn(val)                 REG_FLD_VAL(FIFO_ERR_STS_FLD_snf_ds_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_snf_us_fifo_ovrn(val)                 REG_FLD_VAL(FIFO_ERR_STS_FLD_snf_us_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_rx_ploamd_fifo_udrn(val)              REG_FLD_VAL(FIFO_ERR_STS_FLD_rx_ploamd_fifo_udrn, (val))
#define FIFO_ERR_STS_VAL_rx_ploamd_fifo_ovrn(val)              REG_FLD_VAL(FIFO_ERR_STS_FLD_rx_ploamd_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_rx_mbi_pl_fifo_ovrn(val)              REG_FLD_VAL(FIFO_ERR_STS_FLD_rx_mbi_pl_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_rx_mbi_hdr_fifo_ovrn(val)             REG_FLD_VAL(FIFO_ERR_STS_FLD_rx_mbi_hdr_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_rx_aes_rdm_ciph_fifo_ovrn(val)        REG_FLD_VAL(FIFO_ERR_STS_FLD_rx_aes_rdm_ciph_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_rx_aes_ciph_txt_fifo_ovrn(val)        REG_FLD_VAL(FIFO_ERR_STS_FLD_rx_aes_ciph_txt_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_rx_aes_cryp_cnt_fifo_ovrn(val)        REG_FLD_VAL(FIFO_ERR_STS_FLD_rx_aes_cryp_cnt_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_tx_align_fifo_udrn(val)               REG_FLD_VAL(FIFO_ERR_STS_FLD_tx_align_fifo_udrn, (val))
#define FIFO_ERR_STS_VAL_tx_ploamu_fifo_ovrn(val)              REG_FLD_VAL(FIFO_ERR_STS_FLD_tx_ploamu_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_bst_fifo_ovrn(val)                    REG_FLD_VAL(FIFO_ERR_STS_FLD_bst_fifo_ovrn, (val))
#define FIFO_ERR_STS_VAL_sgl_fifo_ovrn(val)                    REG_FLD_VAL(FIFO_ERR_STS_FLD_sgl_fifo_ovrn, (val))

#define TX_ERR_STS_VAL_tx_prof_invld_err(val)                  REG_FLD_VAL(TX_ERR_STS_FLD_tx_prof_invld_err, (val))
#define TX_ERR_STS_VAL_tx_late_start_err(val)                  REG_FLD_VAL(TX_ERR_STS_FLD_tx_late_start_err, (val))
#define TX_ERR_STS_VAL_tx_bst_sgl_diff_err(val)                REG_FLD_VAL(TX_ERR_STS_FLD_tx_bst_sgl_diff_err, (val))

#define RX_ERR_STS_VAL_rx_xgem_hdr_hec_err(val)                REG_FLD_VAL(RX_ERR_STS_FLD_rx_xgem_hdr_hec_err, (val))
#define RX_ERR_STS_VAL_rx_alloc_hec_err(val)                   REG_FLD_VAL(RX_ERR_STS_FLD_rx_alloc_hec_err, (val))
#define RX_ERR_STS_VAL_rx_mbi_xgem_drop_err(val)               REG_FLD_VAL(RX_ERR_STS_FLD_rx_mbi_xgem_drop_err, (val))
#define RX_ERR_STS_VAL_rx_omci_mic_err(val)                    REG_FLD_VAL(RX_ERR_STS_FLD_rx_omci_mic_err, (val))
#define RX_ERR_STS_VAL_rx_ploam_mic_err(val)                   REG_FLD_VAL(RX_ERR_STS_FLD_rx_ploam_mic_err, (val))
#define RX_ERR_STS_VAL_rx_eth_crc_err(val)                     REG_FLD_VAL(RX_ERR_STS_FLD_rx_eth_crc_err, (val))
#define RX_ERR_STS_VAL_rx_aes_key_err(val)                     REG_FLD_VAL(RX_ERR_STS_FLD_rx_aes_key_err, (val))
#define RX_ERR_STS_VAL_rx_gem_intlv_err(val)                   REG_FLD_VAL(RX_ERR_STS_FLD_rx_gem_intlv_err, (val))
#define RX_ERR_STS_VAL_rx_los_gem_del_err(val)                 REG_FLD_VAL(RX_ERR_STS_FLD_rx_los_gem_del_err, (val))
#define RX_ERR_STS_VAL_rx_bip_err(val)                         REG_FLD_VAL(RX_ERR_STS_FLD_rx_bip_err, (val))
#define RX_ERR_STS_VAL_rx_hlend_hec_err(val)                   REG_FLD_VAL(RX_ERR_STS_FLD_rx_hlend_hec_err, (val))
#define RX_ERR_STS_VAL_rx_pon_id_hec_err(val)                  REG_FLD_VAL(RX_ERR_STS_FLD_rx_pon_id_hec_err, (val))
#define RX_ERR_STS_VAL_rx_sfc_hec_uc_err(val)                  REG_FLD_VAL(RX_ERR_STS_FLD_rx_sfc_hec_uc_err, (val))
#define RX_ERR_STS_VAL_rx_eof_err(val)                         REG_FLD_VAL(RX_ERR_STS_FLD_rx_eof_err, (val))

#if defined(TCSUPPORT_CPU_AN7583)
#define FIFO_ERR_ENABLE_VAL_tx_aes_rdm_ciph_fifo_ovrn_en(val)  REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_tx_aes_rdm_ciph_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_byte_mib_cmd_fifo_ovrn_en(val)     REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_byte_mib_cmd_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_frm_mib_cmd_fifo_ovrn_en(val)      REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_frm_mib_cmd_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_mib_cmd_fifo_ovrn_en(val)          REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_mib_cmd_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_mib_tx_cmd_fifo_ovrn_en(val)       REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_mib_tx_cmd_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_snf_ds_fifo_ovrn_en(val)           REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_snf_ds_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_snf_us_fifo_ovrn_en(val)           REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_snf_us_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_rx_ploamd_fifo_udrn_en(val)        REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_rx_ploamd_fifo_udrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_rx_ploamd_fifo_ovrn_en(val)        REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_rx_ploamd_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_rx_mbi_pl_fifo_ovrn_en(val)        REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_rx_mbi_pl_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_rx_mbi_hdr_fifo_ovrn_en(val)       REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_rx_mbi_hdr_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_rx_aes_rdm_ciph_fifo_ovrn_en(val)  REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_rx_aes_rdm_ciph_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_rx_aes_ciph_txt_fifo_ovrn_en(val)  REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_rx_aes_ciph_txt_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_rx_aes_cryp_cnt_fifo_ovrn_en(val)  REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_rx_aes_cryp_cnt_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_tx_align_fifo_udrn_en(val)         REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_tx_align_fifo_udrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_tx_ploamu_fifo_ovrn_en(val)        REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_tx_ploamu_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_bst_fifo_ovrn_en(val)              REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_bst_fifo_ovrn_en, (val))
#define FIFO_ERR_ENABLE_VAL_sgl_fifo_ovrn_en(val)              REG_FLD_VAL(FIFO_ERR_ENABLE_FLD_sgl_fifo_ovrn_en, (val))

#define TX_ERR_ENABLE_VAL_tx_prof_invld_err_en(val)            REG_FLD_VAL(TX_ERR_ENABLE_FLD_tx_prof_invld_err_en, (val))
#define TX_ERR_ENABLE_VAL_tx_late_start_err_en(val)            REG_FLD_VAL(TX_ERR_ENABLE_FLD_tx_late_start_err_en, (val))
#define TX_ERR_ENABLE_VAL_tx_bst_sgl_diff_err_en(val)          REG_FLD_VAL(TX_ERR_ENABLE_FLD_tx_bst_sgl_diff_err_en, (val))

#define RX_ERR_ENABLE_VAL_rx_xgem_hdr_hec_err_en(val)          REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_xgem_hdr_hec_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_alloc_hec_err_en(val)             REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_alloc_hec_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_mbi_xgem_drop_err_en(val)         REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_mbi_xgem_drop_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_omci_mic_err_en(val)              REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_omci_mic_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_ploam_mic_err_en(val)             REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_ploam_mic_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_eth_crc_err_en(val)               REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_eth_crc_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_aes_key_err_en(val)               REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_aes_key_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_gem_intlv_err_en(val)             REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_gem_intlv_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_los_gem_del_err_en(val)           REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_los_gem_del_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_bip_err_en(val)                   REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_bip_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_hlend_hec_err_en(val)             REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_hlend_hec_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_pon_id_hec_err_en(val)            REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_pon_id_hec_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_sfc_hec_uc_err_en(val)            REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_sfc_hec_uc_err_en, (val))
#define RX_ERR_ENABLE_VAL_rx_eof_err_en(val)                   REG_FLD_VAL(RX_ERR_ENABLE_FLD_rx_eof_err_en, (val))
#endif

#define COR_TAG_VAL_cor_tag(val)                               REG_FLD_VAL(COR_TAG_FLD_cor_tag, (val))

#define CUR_DS_PON_ID_VAL_cur_ds_pon_id(val)                   REG_FLD_VAL(CUR_DS_PON_ID_FLD_cur_ds_pon_id, (val))

#define CUR_US_PON_ID_VAL_cur_us_pon_id(val)                   REG_FLD_VAL(CUR_US_PON_ID_FLD_cur_us_pon_id, (val))

#define CALIB_STS_L32_VAL_calib_sts_l32(val)                   REG_FLD_VAL(CALIB_STS_L32_FLD_calib_sts_l32, (val))

#define CALIB_STS_H32_VAL_calib_sts_h32(val)                   REG_FLD_VAL(CALIB_STS_H32_FLD_calib_sts_h32, (val))

#define TUNING_GRAN_VAL_turning_gran(val)                      REG_FLD_VAL(TUNING_GRAN_FLD_turning_gran, (val))

#define STEP_TUNING_TIME_VAL_step_turning_time(val)            REG_FLD_VAL(STEP_TUNING_TIME_FLD_step_turning_time, (val))

#define US_RATE_CAP_VAL_us_10g_cap(val)                        REG_FLD_VAL(US_RATE_CAP_FLD_us_10g_cap, (val))
#define US_RATE_CAP_VAL_us_2p5g_cap(val)                       REG_FLD_VAL(US_RATE_CAP_FLD_us_2p5g_cap, (val))

#define ATTENUATION_VAL_attenuation(val)                       REG_FLD_VAL(ATTENUATION_FLD_attenuation, (val))

#define POWER_LEVEL_CAP_VAL_power_level_cap(val)               REG_FLD_VAL(POWER_LEVEL_CAP_FLD_power_level_cap, (val))

#define O23_O4_PLOAMU_CTRL_VAL_o23_o4_ploamu_ctrl(val)         REG_FLD_VAL(O23_O4_PLOAMU_CTRL_FLD_o23_o4_ploamu_ctrl, (val))

#define ACTIVATION_ST_VAL_act_st(val)                          REG_FLD_VAL(ACTIVATION_ST_FLD_act_st, (val))

#define RSP_TIME_VAL_tresp(val)                                REG_FLD_VAL(RSP_TIME_FLD_tresp, (val))

#define RDM_DLY_VAL_max_rdm_dly(val)                           REG_FLD_VAL(RDM_DLY_FLD_max_rdm_dly, (val))
#define RDM_DLY_VAL_rdm_dly(val)                               REG_FLD_VAL(RDM_DLY_FLD_rdm_dly, (val))

#define INT_DLY_VAL_phy_rx_dly(val)                            REG_FLD_VAL(INT_DLY_FLD_phy_rx_dly, (val))
#define INT_DLY_VAL_phy_tx_dly(val)                            REG_FLD_VAL(INT_DLY_FLD_phy_tx_dly, (val))

#define EQD_VAL_eqd(val)                                       REG_FLD_VAL(EQD_FLD_eqd, (val))

#define SYN_MRG_VAL_syn_mrg(val)                               REG_FLD_VAL(SYN_MRG_FLD_syn_mrg, (val))

#define US_PROF_VLD_VAL_us_prof3_vsn(val)                      REG_FLD_VAL(US_PROF_VLD_FLD_us_prof3_vsn, (val))
#define US_PROF_VLD_VAL_us_prof3_vld(val)                      REG_FLD_VAL(US_PROF_VLD_FLD_us_prof3_vld, (val))
#define US_PROF_VLD_VAL_us_prof2_vsn(val)                      REG_FLD_VAL(US_PROF_VLD_FLD_us_prof2_vsn, (val))
#define US_PROF_VLD_VAL_us_prof2_vld(val)                      REG_FLD_VAL(US_PROF_VLD_FLD_us_prof2_vld, (val))
#define US_PROF_VLD_VAL_us_prof1_vsn(val)                      REG_FLD_VAL(US_PROF_VLD_FLD_us_prof1_vsn, (val))
#define US_PROF_VLD_VAL_us_prof1_vld(val)                      REG_FLD_VAL(US_PROF_VLD_FLD_us_prof1_vld, (val))
#define US_PROF_VLD_VAL_us_prof0_vsn(val)                      REG_FLD_VAL(US_PROF_VLD_FLD_us_prof0_vsn, (val))
#define US_PROF_VLD_VAL_us_prof0_vld(val)                      REG_FLD_VAL(US_PROF_VLD_FLD_us_prof0_vld, (val))

#define US_PROF_PSBu_LEN_0_1_VAL_us_psbu_len_prof1(val)        REG_FLD_VAL(US_PROF_PSBu_LEN_0_1_FLD_us_psbu_len_prof1, (val))
#define US_PROF_PSBu_LEN_0_1_VAL_us_psbu_len_prof0(val)        REG_FLD_VAL(US_PROF_PSBu_LEN_0_1_FLD_us_psbu_len_prof0, (val))

#define US_PROF_PSBu_LEN_2_3_VAL_us_psbu_len_prof3(val)        REG_FLD_VAL(US_PROF_PSBu_LEN_2_3_FLD_us_psbu_len_prof3, (val))
#define US_PROF_PSBu_LEN_2_3_VAL_us_psbu_len_prof2(val)        REG_FLD_VAL(US_PROF_PSBu_LEN_2_3_FLD_us_psbu_len_prof2, (val))

#define US_AES_KEY_CTRL_VAL_us_aes_key_vld(val)                REG_FLD_VAL(US_AES_KEY_CTRL_FLD_us_aes_key_vld, (val))
#define US_AES_KEY_CTRL_VAL_us_aes_key_idx(val)                REG_FLD_VAL(US_AES_KEY_CTRL_FLD_us_aes_key_idx, (val))

#define DS_AES_KEY_VLD_VAL_ds_aes_bc_idx1_key_vld(val)         REG_FLD_VAL(DS_AES_KEY_VLD_FLD_ds_aes_bc_idx1_key_vld, (val))
#define DS_AES_KEY_VLD_VAL_ds_aes_bc_idx0_key_vld(val)         REG_FLD_VAL(DS_AES_KEY_VLD_FLD_ds_aes_bc_idx0_key_vld, (val))
#define DS_AES_KEY_VLD_VAL_ds_aes_uc_idx1_key_vld(val)         REG_FLD_VAL(DS_AES_KEY_VLD_FLD_ds_aes_uc_idx1_key_vld, (val))
#define DS_AES_KEY_VLD_VAL_ds_aes_uc_idx0_key_vld(val)         REG_FLD_VAL(DS_AES_KEY_VLD_FLD_ds_aes_uc_idx0_key_vld, (val))

#define DEFT_GPID_ENC_VAL_deft_gpid_enc(val)                   REG_FLD_VAL(DEFT_GPID_ENC_FLD_deft_gpid_enc, (val))

#define AES_UC_IDX0_KEY0_VAL_aes_uc_idx0_key0(val)             REG_FLD_VAL(AES_UC_IDX0_KEY0_FLD_aes_uc_idx0_key0, (val))

#define AES_UC_IDX0_KEY1_VAL_aes_uc_idx0_key1(val)             REG_FLD_VAL(AES_UC_IDX0_KEY1_FLD_aes_uc_idx0_key1, (val))

#define AES_UC_IDX0_KEY2_VAL_aes_uc_idx0_key2(val)             REG_FLD_VAL(AES_UC_IDX0_KEY2_FLD_aes_uc_idx0_key2, (val))

#define AES_UC_IDX0_KEY3_VAL_aes_uc_idx0_key3(val)             REG_FLD_VAL(AES_UC_IDX0_KEY3_FLD_aes_uc_idx0_key3, (val))

#define AES_UC_IDX1_KEY0_VAL_aes_uc_idx1_key0(val)             REG_FLD_VAL(AES_UC_IDX1_KEY0_FLD_aes_uc_idx1_key0, (val))

#define AES_UC_IDX1_KEY1_VAL_aes_uc_idx1_key1(val)             REG_FLD_VAL(AES_UC_IDX1_KEY1_FLD_aes_uc_idx1_key1, (val))

#define AES_UC_IDX1_KEY2_VAL_aes_uc_idx1_key2(val)             REG_FLD_VAL(AES_UC_IDX1_KEY2_FLD_aes_uc_idx1_key2, (val))

#define AES_UC_IDX1_KEY3_VAL_aes_uc_idx1_key3(val)             REG_FLD_VAL(AES_UC_IDX1_KEY3_FLD_aes_uc_idx1_key3, (val))

#define AES_BC_IDX0_KEY0_VAL_aes_bc_idx0_key0(val)             REG_FLD_VAL(AES_BC_IDX0_KEY0_FLD_aes_bc_idx0_key0, (val))

#define AES_BC_IDX0_KEY1_VAL_aes_bc_idx0_key1(val)             REG_FLD_VAL(AES_BC_IDX0_KEY1_FLD_aes_bc_idx0_key1, (val))

#define AES_BC_IDX0_KEY2_VAL_aes_bc_idx0_key2(val)             REG_FLD_VAL(AES_BC_IDX0_KEY2_FLD_aes_bc_idx0_key2, (val))

#define AES_BC_IDX0_KEY3_VAL_aes_bc_idx0_key3(val)             REG_FLD_VAL(AES_BC_IDX0_KEY3_FLD_aes_bc_idx0_key3, (val))

#define AES_BC_IDX1_KEY0_VAL_aes_bc_idx1_key0(val)             REG_FLD_VAL(AES_BC_IDX1_KEY0_FLD_aes_bc_idx1_key0, (val))

#define AES_BC_IDX1_KEY1_VAL_aes_bc_idx1_key1(val)             REG_FLD_VAL(AES_BC_IDX1_KEY1_FLD_aes_bc_idx1_key1, (val))

#define AES_BC_IDX1_KEY2_VAL_aes_bc_idx1_key2(val)             REG_FLD_VAL(AES_BC_IDX1_KEY2_FLD_aes_bc_idx1_key2, (val))

#define AES_BC_IDX1_KEY3_VAL_aes_bc_idx1_key3(val)             REG_FLD_VAL(AES_BC_IDX1_KEY3_FLD_aes_bc_idx1_key3, (val))

#define TCONT_ID_CFG_VAL_tcont_cmd(val)                        REG_FLD_VAL(TCONT_ID_CFG_FLD_tcont_cmd, (val))
#define TCONT_ID_CFG_VAL_tcont_id_index(val)                   REG_FLD_VAL(TCONT_ID_CFG_FLD_tcont_id_index, (val))
#define TCONT_ID_CFG_VAL_wr_tcont_id_vld(val)                  REG_FLD_VAL(TCONT_ID_CFG_FLD_wr_tcont_id_vld, (val))
#define TCONT_ID_CFG_VAL_wr_tcont_id(val)                      REG_FLD_VAL(TCONT_ID_CFG_FLD_wr_tcont_id, (val))

#define TCONT_ID_STS_VAL_tcont_cmd_done(val)                   REG_FLD_VAL(TCONT_ID_STS_FLD_tcont_cmd_done, (val))
#define TCONT_ID_STS_VAL_rd_tcont_id_vld(val)                  REG_FLD_VAL(TCONT_ID_STS_FLD_rd_tcont_id_vld, (val))
#define TCONT_ID_STS_VAL_rd_tcont_id(val)                      REG_FLD_VAL(TCONT_ID_STS_FLD_rd_tcont_id, (val))

#define GPIDX_TBL_INIT_VAL_gpidx_tbl_init_done(val)            REG_FLD_VAL(GPIDX_TBL_INIT_FLD_gpidx_tbl_init_done, (val))
#define GPIDX_TBL_INIT_VAL_gpidx_tbl_init_start(val)           REG_FLD_VAL(GPIDX_TBL_INIT_FLD_gpidx_tbl_init_start, (val))

#define GPIDX_TBL_CTRL_VAL_gpidx_tbl_cmd(val)                  REG_FLD_VAL(GPIDX_TBL_CTRL_FLD_gpidx_tbl_cmd, (val))
#define GPIDX_TBL_CTRL_VAL_gpidx_tbl_addr(val)                 REG_FLD_VAL(GPIDX_TBL_CTRL_FLD_gpidx_tbl_addr, (val))
#define GPIDX_TBL_CTRL_VAL_gpidx_tbl_wdata(val)                REG_FLD_VAL(GPIDX_TBL_CTRL_FLD_gpidx_tbl_wdata, (val))

#define GPIDX_TBL_STS_VAL_gpidx_cmd_done(val)                  REG_FLD_VAL(GPIDX_TBL_STS_FLD_gpidx_cmd_done, (val))
#define GPIDX_TBL_STS_VAL_gpidx_tbl_rdata(val)                 REG_FLD_VAL(GPIDX_TBL_STS_FLD_gpidx_tbl_rdata, (val))

#define GEM_TBL_INIT_VAL_gem_tbl_init_done(val)                REG_FLD_VAL(GEM_TBL_INIT_FLD_gem_tbl_init_done, (val))
#define GEM_TBL_INIT_VAL_gem_tbl_init_start(val)               REG_FLD_VAL(GEM_TBL_INIT_FLD_gem_tbl_init_start, (val))

#define GEM_PORT_CFG_VAL_gpid_cmd(val)                         REG_FLD_VAL(GEM_PORT_CFG_FLD_gpid_cmd, (val))
#define GEM_PORT_CFG_VAL_gpid_vld(val)                         REG_FLD_VAL(GEM_PORT_CFG_FLD_gpid_vld, (val))
#define GEM_PORT_CFG_VAL_gpid_type(val)                        REG_FLD_VAL(GEM_PORT_CFG_FLD_gpid_type, (val))
#define GEM_PORT_CFG_VAL_gpid_us_encrypt(val)                  REG_FLD_VAL(GEM_PORT_CFG_FLD_gpid_us_encrypt, (val))
#define GEM_PORT_CFG_VAL_gem_port_id(val)                      REG_FLD_VAL(GEM_PORT_CFG_FLD_gem_port_id, (val))

#define GEM_PORT_STS_VAL_gpid_cmd_done(val)                    REG_FLD_VAL(GEM_PORT_STS_FLD_gpid_cmd_done, (val))
#define GEM_PORT_STS_VAL_gpid_rd_sts(val)                      REG_FLD_VAL(GEM_PORT_STS_FLD_gpid_rd_sts, (val))

#define G_TX_FCS_TBL_INIT_VAL_tx_fcs_tbl_init_done(val)        REG_FLD_VAL(G_TX_FCS_TBL_INIT_FLD_tx_fcs_tbl_init_done, (val))
#define G_TX_FCS_TBL_INIT_VAL_tx_fcs_tbl_init_start(val)       REG_FLD_VAL(G_TX_FCS_TBL_INIT_FLD_tx_fcs_tbl_init_start, (val))

#if defined(TCSUPPORT_CPU_EN7581)
#define IDLE_GEM_CTRL_VAL_force_idle_hdr_data_same(val)        REG_FLD_VAL(IDLE_GEM_CTRL_FLD_force_idle_hdr_data_same, (val))
#endif
#define IDLE_GEM_CTRL_VAL_idle_gem_max_size(val)               REG_FLD_VAL(IDLE_GEM_CTRL_FLD_idle_gem_max_size, (val))
#define IDLE_GEM_CTRL_VAL_idle_gem_thld(val)                   REG_FLD_VAL(IDLE_GEM_CTRL_FLD_idle_gem_thld, (val))

#define US_DYING_GASP_CTRL_VAL_dying_gasp_send_num(val)        REG_FLD_VAL(US_DYING_GASP_CTRL_FLD_dying_gasp_send_num, (val))
#define US_DYING_GASP_CTRL_VAL_sw_dying_gasp_trig(val)         REG_FLD_VAL(US_DYING_GASP_CTRL_FLD_sw_dying_gasp_trig, (val))
#define US_DYING_GASP_CTRL_VAL_hw_dying_gasp_sts(val)          REG_FLD_VAL(US_DYING_GASP_CTRL_FLD_hw_dying_gasp_sts, (val))
#define US_DYING_GASP_CTRL_VAL_hw_dying_gasp_en(val)           REG_FLD_VAL(US_DYING_GASP_CTRL_FLD_hw_dying_gasp_en, (val))

#define TX_XGEM_IDLE_DATA_VAL_tx_xgem_idle_data(val)           REG_FLD_VAL(TX_XGEM_IDLE_DATA_FLD_tx_xgem_idle_data, (val))

#define TX_OMCI_PRE_GET_VAL_tx_limit_get_omci_size(val)        REG_FLD_VAL(TX_OMCI_PRE_GET_FLD_tx_limit_get_omci_size, (val))
#define TX_OMCI_PRE_GET_VAL_tx_limit_get_omci_en(val)          REG_FLD_VAL(TX_OMCI_PRE_GET_FLD_tx_limit_get_omci_en, (val))
#define TX_OMCI_PRE_GET_VAL_tx_pre_get_omci_en(val)            REG_FLD_VAL(TX_OMCI_PRE_GET_FLD_tx_pre_get_omci_en, (val))

#define RX_OMCI_PRE_GET_VAL_rx_omci_intr_eth_en(val)           REG_FLD_VAL(RX_OMCI_PRE_GET_FLD_rx_omci_intr_eth_en, (val))

#define EPDROP_EN_VAL_errploamd_drop_en(val)                   REG_FLD_VAL(EPDROP_EN_FLD_errploamd_drop_en, (val))

#define PLOAMu_FIFO_STS_VAL_ploamu_fifo_ovrn(val)              REG_FLD_VAL(PLOAMu_FIFO_STS_FLD_ploamu_fifo_ovrn, (val))
#define PLOAMu_FIFO_STS_VAL_ploamu_fifo_min_avail(val)         REG_FLD_VAL(PLOAMu_FIFO_STS_FLD_ploamu_fifo_min_avail, (val))
#define PLOAMu_FIFO_STS_VAL_ploamu_fifo_avail(val)             REG_FLD_VAL(PLOAMu_FIFO_STS_FLD_ploamu_fifo_avail, (val))

#define PLOAMu_WDATA_VAL_ploamu_wdata(val)                     REG_FLD_VAL(PLOAMu_WDATA_FLD_ploamu_wdata, (val))

#define PLOAMd_FIFO_STS_VAL_ploamd_fifo_ovrn(val)              REG_FLD_VAL(PLOAMd_FIFO_STS_FLD_ploamd_fifo_ovrn, (val))
#define PLOAMd_FIFO_STS_VAL_ploamd_fifo_max_used(val)          REG_FLD_VAL(PLOAMd_FIFO_STS_FLD_ploamd_fifo_max_used, (val))
#define PLOAMd_FIFO_STS_VAL_ploamd_fifo_used(val)              REG_FLD_VAL(PLOAMd_FIFO_STS_FLD_ploamd_fifo_used, (val))

#define PLOAMd_RDATA_VAL_ploamd_rdata(val)                     REG_FLD_VAL(PLOAMd_RDATA_FLD_ploamd_rdata, (val))

#define KEY_GEN_VAL_kek_start(val)                             REG_FLD_VAL(KEY_GEN_FLD_kek_start, (val))
#define KEY_GEN_VAL_ploam_ik_start(val)                        REG_FLD_VAL(KEY_GEN_FLD_ploam_ik_start, (val))
#define KEY_GEN_VAL_omci_ik_start(val)                         REG_FLD_VAL(KEY_GEN_FLD_omci_ik_start, (val))
#define KEY_GEN_VAL_sk_start(val)                              REG_FLD_VAL(KEY_GEN_FLD_sk_start, (val))
#define KEY_GEN_VAL_regmsk_start(val)                          REG_FLD_VAL(KEY_GEN_FLD_regmsk_start, (val))

#define CUR_KIDX_VAL_cur_oik_idx(val)                          REG_FLD_VAL(CUR_KIDX_FLD_cur_oik_idx, (val))
#define CUR_KIDX_VAL_cur_pik_idx(val)                          REG_FLD_VAL(CUR_KIDX_FLD_cur_pik_idx, (val))

#define MSK_0_VAL_msk0(val)                                    REG_FLD_VAL(MSK_0_FLD_msk0, (val))

#define MSK_1_VAL_msk1(val)                                    REG_FLD_VAL(MSK_1_FLD_msk1, (val))

#define MSK_2_VAL_msk2(val)                                    REG_FLD_VAL(MSK_2_FLD_msk2, (val))

#define MSK_3_VAL_msk3(val)                                    REG_FLD_VAL(MSK_3_FLD_msk3, (val))

#define REGMSK_0_VAL_regmsk0(val)                              REG_FLD_VAL(REGMSK_0_FLD_regmsk0, (val))

#define REGMSK_1_VAL_regmsk1(val)                              REG_FLD_VAL(REGMSK_1_FLD_regmsk1, (val))

#define REGMSK_2_VAL_regmsk2(val)                              REG_FLD_VAL(REGMSK_2_FLD_regmsk2, (val))

#define REGMSK_3_VAL_regmsk3(val)                              REG_FLD_VAL(REGMSK_3_FLD_regmsk3, (val))

#define SK_0_VAL_sk0(val)                                      REG_FLD_VAL(SK_0_FLD_sk0, (val))

#define SK_1_VAL_sk1(val)                                      REG_FLD_VAL(SK_1_FLD_sk1, (val))

#define SK_2_VAL_sk2(val)                                      REG_FLD_VAL(SK_2_FLD_sk2, (val))

#define SK_3_VAL_sk3(val)                                      REG_FLD_VAL(SK_3_FLD_sk3, (val))

#define HW_GENK_0_VAL_hw_genk_0(val)                           REG_FLD_VAL(HW_GENK_0_FLD_hw_genk_0, (val))

#define HW_GENK_1_VAL_hw_genk_1(val)                           REG_FLD_VAL(HW_GENK_1_FLD_hw_genk_1, (val))

#define HW_GENK_2_VAL_hw_genk_2(val)                           REG_FLD_VAL(HW_GENK_2_FLD_hw_genk_2, (val))

#define HW_GENK_3_VAL_hw_genk_3(val)                           REG_FLD_VAL(HW_GENK_3_FLD_hw_genk_3, (val))

#define PIK0_0_VAL_pik0_0(val)                                 REG_FLD_VAL(PIK0_0_FLD_pik0_0, (val))

#define PIK0_1_VAL_pik0_1(val)                                 REG_FLD_VAL(PIK0_1_FLD_pik0_1, (val))

#define PIK0_2_VAL_pik0_2(val)                                 REG_FLD_VAL(PIK0_2_FLD_pik0_2, (val))

#define PIK0_3_VAL_pik0_3(val)                                 REG_FLD_VAL(PIK0_3_FLD_pik0_3, (val))

#define PIK1_0_VAL_pik1_0(val)                                 REG_FLD_VAL(PIK1_0_FLD_pik1_0, (val))

#define PIK1_1_VAL_pik1_1(val)                                 REG_FLD_VAL(PIK1_1_FLD_pik1_1, (val))

#define PIK1_2_VAL_pik1_2(val)                                 REG_FLD_VAL(PIK1_2_FLD_pik1_2, (val))

#define PIK1_3_VAL_pik1_3(val)                                 REG_FLD_VAL(PIK1_3_FLD_pik1_3, (val))

#define OIK0_0_VAL_oik0_0(val)                                 REG_FLD_VAL(OIK0_0_FLD_oik0_0, (val))

#define OIK0_1_VAL_oik0_1(val)                                 REG_FLD_VAL(OIK0_1_FLD_oik0_1, (val))

#define OIK0_2_VAL_oik0_2(val)                                 REG_FLD_VAL(OIK0_2_FLD_oik0_2, (val))

#define OIK0_3_VAL_oik0_3(val)                                 REG_FLD_VAL(OIK0_3_FLD_oik0_3, (val))

#define OIK1_0_VAL_oik1_0(val)                                 REG_FLD_VAL(OIK1_0_FLD_oik1_0, (val))

#define OIK1_1_VAL_oik1_1(val)                                 REG_FLD_VAL(OIK1_1_FLD_oik1_1, (val))

#define OIK1_2_VAL_oik1_2(val)                                 REG_FLD_VAL(OIK1_2_FLD_oik1_2, (val))

#define OIK1_3_VAL_oik1_3(val)                                 REG_FLD_VAL(OIK1_3_FLD_oik1_3, (val))

#define KEK0_0_VAL_kek0_0(val)                                 REG_FLD_VAL(KEK0_0_FLD_kek0_0, (val))

#define KEK0_1_VAL_kek0_1(val)                                 REG_FLD_VAL(KEK0_1_FLD_kek0_1, (val))

#define KEK0_2_VAL_kek0_2(val)                                 REG_FLD_VAL(KEK0_2_FLD_kek0_2, (val))

#define KEK0_3_VAL_kek0_3(val)                                 REG_FLD_VAL(KEK0_3_FLD_kek0_3, (val))

#define KEK1_0_VAL_kek1_0(val)                                 REG_FLD_VAL(KEK1_0_FLD_kek1_0, (val))

#define KEK1_1_VAL_kek1_1(val)                                 REG_FLD_VAL(KEK1_1_FLD_kek1_1, (val))

#define KEK1_2_VAL_kek1_2(val)                                 REG_FLD_VAL(KEK1_2_FLD_kek1_2, (val))

#define KEK1_3_VAL_kek1_3(val)                                 REG_FLD_VAL(KEK1_3_FLD_kek1_3, (val))

#define PON_TAG_0_VAL_pon_tag_0(val)                           REG_FLD_VAL(PON_TAG_0_FLD_pon_tag_0, (val))

#define PON_TAG_1_VAL_pon_tag_1(val)                           REG_FLD_VAL(PON_TAG_1_FLD_pon_tag_1, (val))

#define SW_SET_KIDX_VAL_sw_set_oik_en(val)                     REG_FLD_VAL(SW_SET_KIDX_FLD_sw_set_oik_en, (val))
#define SW_SET_KIDX_VAL_sw_set_pik_en(val)                     REG_FLD_VAL(SW_SET_KIDX_FLD_sw_set_pik_en, (val))
#define SW_SET_KIDX_VAL_sw_set_oik_idx(val)                    REG_FLD_VAL(SW_SET_KIDX_FLD_sw_set_oik_idx, (val))
#define SW_SET_KIDX_VAL_sw_set_pik_idx(val)                    REG_FLD_VAL(SW_SET_KIDX_FLD_sw_set_pik_idx, (val))

#define SW0_ENCSTART_VAL_sw0_encstart(val)                     REG_FLD_VAL(SW0_ENCSTART_FLD_sw0_encstart, (val))

#define SW0_MADDR_VAL_sw0_mdtaddr(val)                         REG_FLD_VAL(SW0_MADDR_FLD_sw0_mdtaddr, (val))

#define SW0_RADDR_VAL_sw0_rdtaddr(val)                         REG_FLD_VAL(SW0_RADDR_FLD_sw0_rdtaddr, (val))

#define SW0_KADDR_VAL_sw0_kaddr(val)                           REG_FLD_VAL(SW0_KADDR_FLD_sw0_kaddr, (val))

#define SW0_ENCLEN_VAL_sw0_rdtlen(val)                         REG_FLD_VAL(SW0_ENCLEN_FLD_sw0_rdtlen, (val))
#define SW0_ENCLEN_VAL_sw0_mdtlen(val)                         REG_FLD_VAL(SW0_ENCLEN_FLD_sw0_mdtlen, (val))

#define SW0_ENCINFO_VAL_sw0_enckidx(val)                       REG_FLD_VAL(SW0_ENCINFO_FLD_sw0_enckidx, (val))
#define SW0_ENCINFO_VAL_sw0_encdic(val)                        REG_FLD_VAL(SW0_ENCINFO_FLD_sw0_encdic, (val))

#define SW1_ENCSTART_VAL_sw1_encstart(val)                     REG_FLD_VAL(SW1_ENCSTART_FLD_sw1_encstart, (val))

#define SW1_MADDR_VAL_sw1_mdtaddr(val)                         REG_FLD_VAL(SW1_MADDR_FLD_sw1_mdtaddr, (val))

#define SW1_RADDR_VAL_sw1_rdtaddr(val)                         REG_FLD_VAL(SW1_RADDR_FLD_sw1_rdtaddr, (val))

#define SW1_KADDR_VAL_sw1_kaddr(val)                           REG_FLD_VAL(SW1_KADDR_FLD_sw1_kaddr, (val))

#define SW1_ENCLEN_VAL_sw1_rdtlen(val)                         REG_FLD_VAL(SW1_ENCLEN_FLD_sw1_rdtlen, (val))
#define SW1_ENCLEN_VAL_sw1_mdtlen(val)                         REG_FLD_VAL(SW1_ENCLEN_FLD_sw1_mdtlen, (val))

#define SW1_ENCINFO_VAL_sw1_enckidx(val)                       REG_FLD_VAL(SW1_ENCINFO_FLD_sw1_enckidx, (val))
#define SW1_ENCINFO_VAL_sw1_encdic(val)                        REG_FLD_VAL(SW1_ENCINFO_FLD_sw1_encdic, (val))

#if defined(TCSUPPORT_CPU_AN7583)
#define TX_FCS_TBL_CLEAR_VAL_fcs_clr_tcont_done(val)           REG_FLD_VAL(TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont_done, (val))
#define TX_FCS_TBL_CLEAR_VAL_fcs_clr_tcont_en(val)             REG_FLD_VAL(TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont_en, (val))
#define TX_FCS_TBL_CLEAR_VAL_fcs_clr_tcont(val)                REG_FLD_VAL(TX_FCS_TBL_CLEAR_FLD_fcs_clr_tcont, (val))
#endif

#define MIB_CTRL_VAL_mib_frame_type(val)                       REG_FLD_VAL(MIB_CTRL_FLD_mib_frame_type, (val))
#define MIB_CTRL_VAL_mib_en(val)                               REG_FLD_VAL(MIB_CTRL_FLD_mib_en, (val))

#define MIB_TBL_CONFIG_VAL_mib_tbl_rd_clr(val)                 REG_FLD_VAL(MIB_TBL_CONFIG_FLD_mib_tbl_rd_clr, (val))
#define MIB_TBL_CONFIG_VAL_mib_tbl_init_done(val)              REG_FLD_VAL(MIB_TBL_CONFIG_FLD_mib_tbl_init_done, (val))
#define MIB_TBL_CONFIG_VAL_mib_tbl_init_start(val)             REG_FLD_VAL(MIB_TBL_CONFIG_FLD_mib_tbl_init_start, (val))

#define MIB_CTRL_STS_VAL_mib_cmd_done(val)                     REG_FLD_VAL(MIB_CTRL_STS_FLD_mib_cmd_done, (val))
#define MIB_CTRL_STS_VAL_mib_wr(val)                           REG_FLD_VAL(MIB_CTRL_STS_FLD_mib_wr, (val))
#define MIB_CTRL_STS_VAL_mib_type(val)                         REG_FLD_VAL(MIB_CTRL_STS_FLD_mib_type, (val))
#define MIB_CTRL_STS_VAL_mib_gpid(val)                         REG_FLD_VAL(MIB_CTRL_STS_FLD_mib_gpid, (val))

#define MIB_RDATA_L32_VAL_mib_rdata_l32(val)                   REG_FLD_VAL(MIB_RDATA_L32_FLD_mib_rdata_l32, (val))

#define MIB_RDATA_H32_VAL_mib_rdata_h32(val)                   REG_FLD_VAL(MIB_RDATA_H32_FLD_mib_rdata_h32, (val))

#define MIB_WDATA_L32_VAL_mib_wdata_l32(val)                   REG_FLD_VAL(MIB_WDATA_L32_FLD_mib_wdata_l32, (val))

#define MIB_WDATA_H32_VAL_mib_wdata_h32(val)                   REG_FLD_VAL(MIB_WDATA_H32_FLD_mib_wdata_h32, (val))

#define TOD_CLK_PERIOD_VAL_tod_period(val)                     REG_FLD_VAL(TOD_CLK_PERIOD_FLD_tod_period, (val))

#define TOD_1PPS_WD_CTRL_VAL_tod_1pps_width_ctrl(val)          REG_FLD_VAL(TOD_1PPS_WD_CTRL_FLD_tod_1pps_width_ctrl, (val))

#define CUR_TOD_SEC_VAL_cur_tod_sec(val)                       REG_FLD_VAL(CUR_TOD_SEC_FLD_cur_tod_sec, (val))

#define CUR_TOD_NANO_SEC_VAL_cur_tod_nano_sec(val)             REG_FLD_VAL(CUR_TOD_NANO_SEC_FLD_cur_tod_nano_sec, (val))

#define TOD_SFC_L_VAL_tod_spf_cnt_l(val)                       REG_FLD_VAL(TOD_SFC_L_FLD_tod_spf_cnt_l, (val))

#define TOD_SFC_H_VAL_tod_spf_cnt_h(val)                       REG_FLD_VAL(TOD_SFC_H_FLD_tod_spf_cnt_h, (val))

#define TOD_UPD_CTRL_VAL_tod_upd_en(val)                       REG_FLD_VAL(TOD_UPD_CTRL_FLD_tod_upd_en, (val))

#define NEW_TOD_SEC_VAL_new_tod_sec(val)                       REG_FLD_VAL(NEW_TOD_SEC_FLD_new_tod_sec, (val))

#define NEW_TOD_NANO_SEC_VAL_new_tod_nano_sec(val)             REG_FLD_VAL(NEW_TOD_NANO_SEC_FLD_new_tod_nano_sec, (val))

#define SLEEP_CFG_VAL_reload_sleep_cnt(val)                    REG_FLD_VAL(SLEEP_CFG_FLD_reload_sleep_cnt, (val))

#define SLEEP_CNT_VAL_sleep_cnt(val)                           REG_FLD_VAL(SLEEP_CNT_FLD_sleep_cnt, (val))

#define TURNING_SFC_L_VAL_turning_spf_cnt_l(val)               REG_FLD_VAL(TURNING_SFC_L_FLD_turning_spf_cnt_l, (val))

#define TURNING_SFC_H_VAL_turning_spf_cnt_h(val)               REG_FLD_VAL(TURNING_SFC_H_FLD_turning_spf_cnt_h, (val))

#define DBG_CAP_SETTING_VAL_hw_ack_ploamu_code(val)            REG_FLD_VAL(DBG_CAP_SETTING_FLD_hw_ack_ploamu_code, (val))
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_VAL_force_o2349_zero_gsize_no_dbru(val) REG_FLD_VAL(DBG_CAP_SETTING_FLD_force_o2349_zero_gsize_no_dbru, (val))
#define DBG_CAP_SETTING_VAL_ds_bip_chk_en(val)                 REG_FLD_VAL(DBG_CAP_SETTING_FLD_ds_bip_chk_en, (val))
#define DBG_CAP_SETTING_VAL_ds_4b_trailer_en(val)              REG_FLD_VAL(DBG_CAP_SETTING_FLD_ds_4b_trailer_en, (val))
#define DBG_CAP_SETTING_VAL_tx_deft_gpid_enc_sts_ctrl_by_rx(val) REG_FLD_VAL(DBG_CAP_SETTING_FLD_tx_deft_gpid_enc_sts_ctrl_by_rx, (val))
#define DBG_CAP_SETTING_VAL_flt_key_invld_gem(val)             REG_FLD_VAL(DBG_CAP_SETTING_FLD_flt_key_invld_gem, (val))
#define DBG_CAP_SETTING_VAL_flt_o2349_continue_gnt(val)        REG_FLD_VAL(DBG_CAP_SETTING_FLD_flt_o2349_continue_gnt, (val))
#define DBG_CAP_SETTING_VAL_flt_o2349_no_ploamu_gnt(val)       REG_FLD_VAL(DBG_CAP_SETTING_FLD_flt_o2349_no_ploamu_gnt, (val))
#endif
#define DBG_CAP_SETTING_VAL_us_aes_seq_num_14b(val)            REG_FLD_VAL(DBG_CAP_SETTING_FLD_us_aes_seq_num_14b, (val))
#define DBG_CAP_SETTING_VAL_o52_idle_only_en(val)              REG_FLD_VAL(DBG_CAP_SETTING_FLD_o52_idle_only_en, (val))
#define DBG_CAP_SETTING_VAL_tx_send_only_in_o23459(val)        REG_FLD_VAL(DBG_CAP_SETTING_FLD_tx_send_only_in_o23459, (val))
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_VAL_flt_o49_no_deft_tcont_gnt(val)     REG_FLD_VAL(DBG_CAP_SETTING_FLD_flt_o49_no_deft_tcont_gnt, (val))
#else
#define DBG_CAP_SETTING_VAL_flt_dbru_set_gnt_o2349(val)        REG_FLD_VAL(DBG_CAP_SETTING_FLD_flt_dbru_set_gnt_o2349, (val))
#endif
#define DBG_CAP_SETTING_VAL_send_ranging_ack_o4(val)           REG_FLD_VAL(DBG_CAP_SETTING_FLD_send_ranging_ack_o4, (val))
#define DBG_CAP_SETTING_VAL_hw_cal_ds_omci_mic(val)            REG_FLD_VAL(DBG_CAP_SETTING_FLD_hw_cal_ds_omci_mic, (val))
#define DBG_CAP_SETTING_VAL_hw_cal_us_omci_mic(val)            REG_FLD_VAL(DBG_CAP_SETTING_FLD_hw_cal_us_omci_mic, (val))
#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_CAP_SETTING_VAL_flt_o2349_no_ploamu_only_gnt(val)  REG_FLD_VAL(DBG_CAP_SETTING_FLD_flt_o2349_no_ploamu_only_gnt, (val))
#else
#define DBG_CAP_SETTING_VAL_flt_no_zero_gsize_gnt_o2349(val)   REG_FLD_VAL(DBG_CAP_SETTING_FLD_flt_no_zero_gsize_gnt_o2349, (val))
#endif
#define DBG_CAP_SETTING_VAL_rept_msg_flt(val)                  REG_FLD_VAL(DBG_CAP_SETTING_FLD_rept_msg_flt, (val))
#define DBG_CAP_SETTING_VAL_rept_prof_flt(val)                 REG_FLD_VAL(DBG_CAP_SETTING_FLD_rept_prof_flt, (val))

#define DBG_BWM_CHK_CTRL_VAL_min_bst_intvl(val)                REG_FLD_VAL(DBG_BWM_CHK_CTRL_FLD_min_bst_intvl, (val))
#define DBG_BWM_CHK_CTRL_VAL_max_bst_len_err_cut(val)          REG_FLD_VAL(DBG_BWM_CHK_CTRL_FLD_max_bst_len_err_cut, (val))
#define DBG_BWM_CHK_CTRL_VAL_max_my_bst_num_err_cut(val)       REG_FLD_VAL(DBG_BWM_CHK_CTRL_FLD_max_my_bst_num_err_cut, (val))
#define DBG_BWM_CHK_CTRL_VAL_max_my_alloc_num_err_cut(val)     REG_FLD_VAL(DBG_BWM_CHK_CTRL_FLD_max_my_alloc_num_err_cut, (val))
#define DBG_BWM_CHK_CTRL_VAL_max_bst_alloc_num_err_cut(val)    REG_FLD_VAL(DBG_BWM_CHK_CTRL_FLD_max_bst_alloc_num_err_cut, (val))
#define DBG_BWM_CHK_CTRL_VAL_max_bwm_alloc_num_err_cut(val)    REG_FLD_VAL(DBG_BWM_CHK_CTRL_FLD_max_bwm_alloc_num_err_cut, (val))
#define DBG_BWM_CHK_CTRL_VAL_min_gsize_flt(val)                REG_FLD_VAL(DBG_BWM_CHK_CTRL_FLD_min_gsize_flt, (val))
#define DBG_BWM_CHK_CTRL_VAL_max_gsize_flt(val)                REG_FLD_VAL(DBG_BWM_CHK_CTRL_FLD_max_gsize_flt, (val))
#define DBG_BWM_CHK_CTRL_VAL_start_time_order_flt(val)         REG_FLD_VAL(DBG_BWM_CHK_CTRL_FLD_start_time_order_flt, (val))
#define DBG_BWM_CHK_CTRL_VAL_max_start_time_flt(val)           REG_FLD_VAL(DBG_BWM_CHK_CTRL_FLD_max_start_time_flt, (val))
#define DBG_BWM_CHK_CTRL_VAL_min_bst_intvl_flt(val)            REG_FLD_VAL(DBG_BWM_CHK_CTRL_FLD_min_bst_intvl_flt, (val))

#if defined(TCSUPPORT_CPU_EN7581)
#define DBG_BWM_CKH_STS_VAL_o2349_continue_gnt_recv(val)       REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_o2349_continue_gnt_recv, (val))
#define DBG_BWM_CKH_STS_VAL_o49_no_deft_tcont_gnt_recv(val)    REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_o49_no_deft_tcont_gnt_recv, (val))
#define DBG_BWM_CKH_STS_VAL_o2349_no_ploamu_only_gnt_recv(val) REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_o2349_no_ploamu_only_gnt_recv, (val))
#define DBG_BWM_CKH_STS_VAL_o2349_no_ploamu_gnt_recv(val)      REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_o2349_no_ploamu_gnt_recv, (val))
#else
#define DBG_BWM_CKH_STS_VAL_o23_o4_data_gnt_recv(val)          REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_o23_o4_data_gnt_recv, (val))
#endif
#define DBG_BWM_CKH_STS_VAL_other_tid_ins_my_bst_err(val)      REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_other_tid_ins_my_bst_err, (val))
#define DBG_BWM_CKH_STS_VAL_max_bst_len_err(val)               REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_max_bst_len_err, (val))
#define DBG_BWM_CKH_STS_VAL_max_my_bst_num_err(val)            REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_max_my_bst_num_err, (val))
#define DBG_BWM_CKH_STS_VAL_max_my_alloc_num_err(val)          REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_max_my_alloc_num_err, (val))
#define DBG_BWM_CKH_STS_VAL_max_bst_alloc_num_err(val)         REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_max_bst_alloc_num_err, (val))
#define DBG_BWM_CKH_STS_VAL_max_bwm_alloc_num_err(val)         REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_max_bwm_alloc_num_err, (val))
#define DBG_BWM_CKH_STS_VAL_alloc_hec_uc_err(val)              REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_alloc_hec_uc_err, (val))
#define DBG_BWM_CKH_STS_VAL_bst_split_err(val)                 REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_bst_split_err, (val))
#define DBG_BWM_CKH_STS_VAL_my_tid_ins_bst_err(val)            REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_my_tid_ins_bst_err, (val))
#define DBG_BWM_CKH_STS_VAL_min_gsize_err(val)                 REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_min_gsize_err, (val))
#define DBG_BWM_CKH_STS_VAL_max_gsize_err(val)                 REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_max_gsize_err, (val))
#define DBG_BWM_CKH_STS_VAL_start_time_order_err(val)          REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_start_time_order_err, (val))
#define DBG_BWM_CKH_STS_VAL_max_start_time_err(val)            REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_max_start_time_err, (val))
#define DBG_BWM_CKH_STS_VAL_min_bst_intvl_err(val)             REG_FLD_VAL(DBG_BWM_CKH_STS_FLD_min_bst_intvl_err, (val))

#define DBG_BWM_SFIFO_STS_VAL_sgl_fifo_max_used(val)           REG_FLD_VAL(DBG_BWM_SFIFO_STS_FLD_sgl_fifo_max_used, (val))
#define DBG_BWM_SFIFO_STS_VAL_sgl_fifo_used(val)               REG_FLD_VAL(DBG_BWM_SFIFO_STS_FLD_sgl_fifo_used, (val))

#define DBG_BWM_BFIFO_STS_VAL_bst_fifo_max_used(val)           REG_FLD_VAL(DBG_BWM_BFIFO_STS_FLD_bst_fifo_max_used, (val))
#define DBG_BWM_BFIFO_STS_VAL_bst_fifo_used(val)               REG_FLD_VAL(DBG_BWM_BFIFO_STS_FLD_bst_fifo_used, (val))

#define DBG_TX_ALIGN_FIFO_STS_VAL_tx_align_fifo_max_used(val)  REG_FLD_VAL(DBG_TX_ALIGN_FIFO_STS_FLD_tx_align_fifo_max_used, (val))
#define DBG_TX_ALIGN_FIFO_STS_VAL_tx_align_fifo_used(val)      REG_FLD_VAL(DBG_TX_ALIGN_FIFO_STS_FLD_tx_align_fifo_used, (val))

#define DBG_TX_AES_MAX_USED_VAL_tx_aes_eng_max_used(val)       REG_FLD_VAL(DBG_TX_AES_MAX_USED_FLD_tx_aes_eng_max_used, (val))
#define DBG_TX_AES_MAX_USED_VAL_tx_rdm_ciph_fifo_max_used(val) REG_FLD_VAL(DBG_TX_AES_MAX_USED_FLD_tx_rdm_ciph_fifo_max_used, (val))
#define DBG_TX_AES_MAX_USED_VAL_tx_cryp_cnt_fifo_max_used(val) REG_FLD_VAL(DBG_TX_AES_MAX_USED_FLD_tx_cryp_cnt_fifo_max_used, (val))
#define DBG_TX_AES_MAX_USED_VAL_tx_ciph_txt_fifo_max_used(val) REG_FLD_VAL(DBG_TX_AES_MAX_USED_FLD_tx_ciph_txt_fifo_max_used, (val))
#define DBG_TX_AES_MAX_USED_VAL_tx_aes_hdr_fifo_max_used(val)  REG_FLD_VAL(DBG_TX_AES_MAX_USED_FLD_tx_aes_hdr_fifo_max_used, (val))

#if defined(TCSUPPORT_CPU_AN7583)
#define DBG_DBA_BACK_DOOR_VAL_dba_modify_tcont_idx_1_mode(val) REG_FLD_VAL(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1_mode, (val))
#define DBG_DBA_BACK_DOOR_VAL_dba_modify_tcont_idx_1_ovrw(val) REG_FLD_VAL(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1_ovrw, (val))
#define DBG_DBA_BACK_DOOR_VAL_dba_modify_tcont_idx_1(val)      REG_FLD_VAL(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_1, (val))
#define DBG_DBA_BACK_DOOR_VAL_dba_modify_tcont_idx_0_mode(val) REG_FLD_VAL(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0_mode, (val))
#define DBG_DBA_BACK_DOOR_VAL_dba_modify_tcont_idx_0_ovrw(val) REG_FLD_VAL(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0_ovrw, (val))
#define DBG_DBA_BACK_DOOR_VAL_dba_modify_tcont_idx_0(val)      REG_FLD_VAL(DBG_DBA_BACK_DOOR_FLD_dba_modify_tcont_idx_0, (val))
#endif

#define DBG_DBA_BACK_DOOR_VAL_dba_modify_mode(val)             REG_FLD_VAL(DBG_DBA_BACK_DOOR_FLD_dba_modify_mode, (val))

#define DBG_DBA_MODIFY_VAL_dba_modify(val)                     REG_FLD_VAL(DBG_DBA_MODIFY_FLD_dba_modify, (val))

#define DBG_RESYNC_VAL_tx_sync_rdy(val)                        REG_FLD_VAL(DBG_RESYNC_FLD_tx_sync_rdy, (val))
#define DBG_RESYNC_VAL_sw_resync_en(val)                       REG_FLD_VAL(DBG_RESYNC_FLD_sw_resync_en, (val))
#define DBG_RESYNC_VAL_sw_resync_start(val)                    REG_FLD_VAL(DBG_RESYNC_FLD_sw_resync_start, (val))

#define DBG_PHY_DLY_CTRL_VAL_use_mpi_rx_phy_dly_ofs(val)       REG_FLD_VAL(DBG_PHY_DLY_CTRL_FLD_use_mpi_rx_phy_dly_ofs, (val))
#define DBG_PHY_DLY_CTRL_VAL_o4_o5_same_phy_dly(val)           REG_FLD_VAL(DBG_PHY_DLY_CTRL_FLD_o4_o5_same_phy_dly, (val))

#define DBG_DS_SPF_CNT_L_VAL_ds_spf_cnt_l32(val)               REG_FLD_VAL(DBG_DS_SPF_CNT_L_FLD_ds_spf_cnt_l32, (val))

#define DBG_DS_SPF_CNT_H_VAL_ds_spf_cnt_h19(val)               REG_FLD_VAL(DBG_DS_SPF_CNT_H_FLD_ds_spf_cnt_h19, (val))

#define DBG_PON_ID_L_VAL_ds_pon_id_l32(val)                    REG_FLD_VAL(DBG_PON_ID_L_FLD_ds_pon_id_l32, (val))

#define DBG_PON_ID_H_VAL_ds_pon_id_h19(val)                    REG_FLD_VAL(DBG_PON_ID_H_FLD_ds_pon_id_h19, (val))

#define DBG_TX_SYNC_OFFSET_VAL_tx_sync_offset(val)             REG_FLD_VAL(DBG_TX_SYNC_OFFSET_FLD_tx_sync_offset, (val))

#define DBG_RX_PHY_DLY_OFS_VAL_rx_phy_dly_offset(val)          REG_FLD_VAL(DBG_RX_PHY_DLY_OFS_FLD_rx_phy_dly_offset, (val))

#define DBG_RX_HEC_ERR_VAL_rx_hec_err_sts(val)                 REG_FLD_VAL(DBG_RX_HEC_ERR_FLD_rx_hec_err_sts, (val))
#define DBG_RX_HEC_ERR_VAL_rx_hdr_hec_3err(val)                REG_FLD_VAL(DBG_RX_HEC_ERR_FLD_rx_hdr_hec_3err, (val))
#define DBG_RX_HEC_ERR_VAL_rx_hdr_hec_2err(val)                REG_FLD_VAL(DBG_RX_HEC_ERR_FLD_rx_hdr_hec_2err, (val))
#define DBG_RX_HEC_ERR_VAL_rx_hdr_hec_1err(val)                REG_FLD_VAL(DBG_RX_HEC_ERR_FLD_rx_hdr_hec_1err, (val))
#define DBG_RX_HEC_ERR_VAL_rx_alloc_hec_3err(val)              REG_FLD_VAL(DBG_RX_HEC_ERR_FLD_rx_alloc_hec_3err, (val))
#define DBG_RX_HEC_ERR_VAL_rx_alloc_hec_2err(val)              REG_FLD_VAL(DBG_RX_HEC_ERR_FLD_rx_alloc_hec_2err, (val))
#define DBG_RX_HEC_ERR_VAL_rx_alloc_hec_1err(val)              REG_FLD_VAL(DBG_RX_HEC_ERR_FLD_rx_alloc_hec_1err, (val))
#define DBG_RX_HEC_ERR_VAL_rx_hlend_hec_3err(val)              REG_FLD_VAL(DBG_RX_HEC_ERR_FLD_rx_hlend_hec_3err, (val))
#define DBG_RX_HEC_ERR_VAL_rx_hlend_hec_2err(val)              REG_FLD_VAL(DBG_RX_HEC_ERR_FLD_rx_hlend_hec_2err, (val))
#define DBG_RX_HEC_ERR_VAL_rx_hlend_hec_1err(val)              REG_FLD_VAL(DBG_RX_HEC_ERR_FLD_rx_hlend_hec_1err, (val))

#define RX_GEM_ENC_CHK_VAL_rx_enc_chk_err_sts(val)             REG_FLD_VAL(RX_GEM_ENC_CHK_FLD_rx_enc_chk_err_sts, (val))
#define RX_GEM_ENC_CHK_VAL_rx_enc_chk_done(val)                REG_FLD_VAL(RX_GEM_ENC_CHK_FLD_rx_enc_chk_done, (val))
#define RX_GEM_ENC_CHK_VAL_rx_enc_chk_en(val)                  REG_FLD_VAL(RX_GEM_ENC_CHK_FLD_rx_enc_chk_en, (val))
#define RX_GEM_ENC_CHK_VAL_exp_rx_enc_sts(val)                 REG_FLD_VAL(RX_GEM_ENC_CHK_FLD_exp_rx_enc_sts, (val))
#define RX_GEM_ENC_CHK_VAL_rx_enc_chk_gpid(val)                REG_FLD_VAL(RX_GEM_ENC_CHK_FLD_rx_enc_chk_gpid, (val))

#define RX_MBI_FIFO_DROP_EN_VAL_rx_mbi_fifo_drop_en(val)       REG_FLD_VAL(RX_MBI_FIFO_DROP_EN_FLD_rx_mbi_fifo_drop_en, (val))

#define RX_MBI_HDR_FIFO_DROP_CTRL_VAL_rx_mbi_hdr_fifo_hth(val) REG_FLD_VAL(RX_MBI_HDR_FIFO_DROP_CTRL_FLD_rx_mbi_hdr_fifo_hth, (val))
#define RX_MBI_HDR_FIFO_DROP_CTRL_VAL_rx_mbi_hdr_fifo_lth(val) REG_FLD_VAL(RX_MBI_HDR_FIFO_DROP_CTRL_FLD_rx_mbi_hdr_fifo_lth, (val))

#define RX_MBI_PL_FIFO_DROP_CTRL_VAL_rx_mbi_pl_fifo_hth(val)   REG_FLD_VAL(RX_MBI_PL_FIFO_DROP_CTRL_FLD_rx_mbi_pl_fifo_hth, (val))
#define RX_MBI_PL_FIFO_DROP_CTRL_VAL_rx_mbi_pl_fifo_lth(val)   REG_FLD_VAL(RX_MBI_PL_FIFO_DROP_CTRL_FLD_rx_mbi_pl_fifo_lth, (val))

#if defined(TCSUPPORT_CPU_EN7581)
#define GET_RX_IDLE_XGEM_VAL_get_rx_idle_xgem(val)             REG_FLD_VAL(GET_RX_IDLE_XGEM_FLD_get_rx_idle_xgem, (val))
#define GET_RX_IDLE_XGEM_VAL_rx_idle_xgem_len(val)             REG_FLD_VAL(GET_RX_IDLE_XGEM_FLD_rx_idle_xgem_len, (val))

#define RX_IDLE_XGEM_1ST_PL_VAL_rx_idle_xgem_1st_pl(val)       REG_FLD_VAL(RX_IDLE_XGEM_1ST_PL_FLD_rx_idle_xgem_1st_pl, (val))

#endif

#if defined(TCSUPPORT_CPU_AN7583)
#define DBG_DBA_MODIFY_IDX_0_VAL_dba_modify_idx_0(val)         REG_FLD_VAL(DBG_DBA_MODIFY_IDX_0_FLD_dba_modify_idx_0, (val))

#define DBG_DBA_MODIFY_IDX_1_VAL_dba_modify_idx_1(val)         REG_FLD_VAL(DBG_DBA_MODIFY_IDX_1_FLD_dba_modify_idx_1, (val))
#endif

#define CNT_CLR_VAL_nml_cnt_clr(val)                           REG_FLD_VAL(CNT_CLR_FLD_nml_cnt_clr, (val))
#define CNT_CLR_VAL_err_cnt_clr(val)                           REG_FLD_VAL(CNT_CLR_FLD_err_cnt_clr, (val))

#define RX_HLEND_HEC_CNT_VAL_rx_hlend_hec_3err_cnt(val)        REG_FLD_VAL(RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_3err_cnt, (val))
#define RX_HLEND_HEC_CNT_VAL_rx_hlend_hec_2err_cnt(val)        REG_FLD_VAL(RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_2err_cnt, (val))
#define RX_HLEND_HEC_CNT_VAL_rx_hlend_hec_1err_cnt(val)        REG_FLD_VAL(RX_HLEND_HEC_CNT_FLD_rx_hlend_hec_1err_cnt, (val))

#define RX_ALLOC_HEC_CNT_VAL_rx_alloc_hec_3err_cnt(val)        REG_FLD_VAL(RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_3err_cnt, (val))
#define RX_ALLOC_HEC_CNT_VAL_rx_alloc_hec_2err_cnt(val)        REG_FLD_VAL(RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_2err_cnt, (val))
#define RX_ALLOC_HEC_CNT_VAL_rx_alloc_hec_1err_cnt(val)        REG_FLD_VAL(RX_ALLOC_HEC_CNT_FLD_rx_alloc_hec_1err_cnt, (val))

#define RX_HDR_HEC_CNT_VAL_rx_hdr_hec_3err_cnt(val)            REG_FLD_VAL(RX_HDR_HEC_CNT_FLD_rx_hdr_hec_3err_cnt, (val))
#define RX_HDR_HEC_CNT_VAL_rx_hdr_hec_2err_cnt(val)            REG_FLD_VAL(RX_HDR_HEC_CNT_FLD_rx_hdr_hec_2err_cnt, (val))
#define RX_HDR_HEC_CNT_VAL_rx_hdr_hec_1err_cnt(val)            REG_FLD_VAL(RX_HDR_HEC_CNT_FLD_rx_hdr_hec_1err_cnt, (val))

#define RX_PHY_HEC_ERR_CNT_VAL_rx_pon_id_hec_err_cnt(val)      REG_FLD_VAL(RX_PHY_HEC_ERR_CNT_FLD_rx_pon_id_hec_err_cnt, (val))
#define RX_PHY_HEC_ERR_CNT_VAL_rx_sfc_hec_err_cnt(val)         REG_FLD_VAL(RX_PHY_HEC_ERR_CNT_FLD_rx_sfc_hec_err_cnt, (val))

#define RX_MIC_ERR_CNT_VAL_rx_omci_mic_err_cnt(val)            REG_FLD_VAL(RX_MIC_ERR_CNT_FLD_rx_omci_mic_err_cnt, (val))
#define RX_MIC_ERR_CNT_VAL_rx_ploam_mic_err_cnt(val)           REG_FLD_VAL(RX_MIC_ERR_CNT_FLD_rx_ploam_mic_err_cnt, (val))

#define RX_ETH_FCS_ERR_CNT_VAL_rx_eth_crc_err_cnt(val)         REG_FLD_VAL(RX_ETH_FCS_ERR_CNT_FLD_rx_eth_crc_err_cnt, (val))

#define RX_BIP_ERR_CNT_VAL_rx_bip_err_cnt(val)                 REG_FLD_VAL(RX_BIP_ERR_CNT_FLD_rx_bip_err_cnt, (val))

#define RX_KEY_ERR_CNT_VAL_rx_key_err_cnt(val)                 REG_FLD_VAL(RX_KEY_ERR_CNT_FLD_rx_key_err_cnt, (val))

#define RX_LOST_WCNT_VAL_rx_lost_wcnt(val)                     REG_FLD_VAL(RX_LOST_WCNT_FLD_rx_lost_wcnt, (val))

#define INVLD_PROF_BST_GNT_CNT_VAL_invld_prof_bst_gnt_cnt(val) REG_FLD_VAL(INVLD_PROF_BST_GNT_CNT_FLD_invld_prof_bst_gnt_cnt, (val))

#define RX_MBI_XGEM_DROP_CNT_VAL_rx_mbi_xgem_drop_cnt(val)     REG_FLD_VAL(RX_MBI_XGEM_DROP_CNT_FLD_rx_mbi_xgem_drop_cnt, (val))

#define RX_XGTC_CNT_VAL_rx_mpi_sof_cnt(val)                    REG_FLD_VAL(RX_XGTC_CNT_FLD_rx_mpi_sof_cnt, (val))

#define TX_BST_CNT_VAL_tx_mpi_sof_cnt(val)                     REG_FLD_VAL(TX_BST_CNT_FLD_tx_mpi_sof_cnt, (val))

#define RX_PLOAMD_CNT_VAL_rx_ploamd_cnt(val)                   REG_FLD_VAL(RX_PLOAMD_CNT_FLD_rx_ploamd_cnt, (val))

#define TX_PLOAMU_CNT_VAL_tx_ploamu_cnt(val)                   REG_FLD_VAL(TX_PLOAMU_CNT_FLD_tx_ploamu_cnt, (val))

#define RX_OMCI_CNT_VAL_rx_omci_cnt_fe(val)                    REG_FLD_VAL(RX_OMCI_CNT_FLD_rx_omci_cnt_fe, (val))
#define RX_OMCI_CNT_VAL_rx_omci_cnt_mac(val)                   REG_FLD_VAL(RX_OMCI_CNT_FLD_rx_omci_cnt_mac, (val))

#define TX_OMCI_CNT_VAL_tx_omci_cnt_fe(val)                    REG_FLD_VAL(TX_OMCI_CNT_FLD_tx_omci_cnt_fe, (val))
#define TX_OMCI_CNT_VAL_tx_omci_cnt_mac(val)                   REG_FLD_VAL(TX_OMCI_CNT_FLD_tx_omci_cnt_mac, (val))

#define RX_XGEM_CNT_VAL_rx_xgem_cnt(val)                       REG_FLD_VAL(RX_XGEM_CNT_FLD_rx_xgem_cnt, (val))

#define TX_XGEM_CNT_VAL_tx_xgem_cnt(val)                       REG_FLD_VAL(TX_XGEM_CNT_FLD_tx_xgem_cnt, (val))

#define RX_MBI_CNT_VAL_rx_mbi_ack_cnt(val)                     REG_FLD_VAL(RX_MBI_CNT_FLD_rx_mbi_ack_cnt, (val))

#define TX_MBI_CNT_VAL_tx_mbi_ack_cnt(val)                     REG_FLD_VAL(TX_MBI_CNT_FLD_tx_mbi_ack_cnt, (val))

#define RX_NON_IDLE_BCNT_VAL_rx_non_idle_bcnt(val)             REG_FLD_VAL(RX_NON_IDLE_BCNT_FLD_rx_non_idle_bcnt, (val))

#define TX_NON_IDLE_BCNT_VAL_tx_non_idle_bcnt(val)             REG_FLD_VAL(TX_NON_IDLE_BCNT_FLD_tx_non_idle_bcnt, (val))

#define TX_NLF_XGEM_CNT_VAL_tx_nlf_xgem_cnt(val)               REG_FLD_VAL(TX_NLF_XGEM_CNT_FLD_tx_nlf_xgem_cnt, (val))

#define TX_ACK_PLOAMU_CNT_VAL_tx_ack_ploamu_cnt(val)           REG_FLD_VAL(TX_ACK_PLOAMU_CNT_FLD_tx_ack_ploamu_cnt, (val))

#define RX_BIP_PROTECT_WCNT_VAL_rx_bip_protect_wcnt(val)       REG_FLD_VAL(RX_BIP_PROTECT_WCNT_FLD_rx_bip_protect_wcnt, (val))

#define TX_IDLE_BCNT_VAL_tx_idle_bcnt(val)                     REG_FLD_VAL(TX_IDLE_BCNT_FLD_tx_idle_bcnt, (val))

#if defined(TCSUPPORT_CPU_AN7583)
#define HUNT_AFTER_LOSGD_VAL_hunt_after_losgd_en(val)          REG_FLD_VAL(HUNT_AFTER_LOSGD_FLD_hunt_after_losgd_en, (val))

#define BWMAP_LEN_LIMIT_VAL_max_alloc_num(val)                 REG_FLD_VAL(BWMAP_LEN_LIMIT_FLD_max_alloc_num, (val))
#define BWMAP_LEN_LIMIT_VAL_max_my_alloc_num(val)              REG_FLD_VAL(BWMAP_LEN_LIMIT_FLD_max_my_alloc_num, (val))
#define BWMAP_LEN_LIMIT_VAL_max_bst_alloc_num(val)             REG_FLD_VAL(BWMAP_LEN_LIMIT_FLD_max_bst_alloc_num, (val))
#define BWMAP_LEN_LIMIT_VAL_max_my_bst_num(val)                REG_FLD_VAL(BWMAP_LEN_LIMIT_FLD_max_my_bst_num, (val))

#define XGEM_HEADER_OVRW_VAL_rx_gem_3port_interleave(val)      REG_FLD_VAL(XGEM_HEADER_OVRW_FLD_rx_gem_3port_interleave, (val))
#define XGEM_HEADER_OVRW_VAL_rx_omci_interleave(val)           REG_FLD_VAL(XGEM_HEADER_OVRW_FLD_rx_omci_interleave, (val))
#define XGEM_HEADER_OVRW_VAL_nml_header_ovrw_en(val)           REG_FLD_VAL(XGEM_HEADER_OVRW_FLD_nml_header_ovrw_en, (val))
#define XGEM_HEADER_OVRW_VAL_omci_header_ovrw_en(val)          REG_FLD_VAL(XGEM_HEADER_OVRW_FLD_omci_header_ovrw_en, (val))

#define OMCI_LEN_CTRL_VAL_max_omci_len(val)                    REG_FLD_VAL(OMCI_LEN_CTRL_FLD_max_omci_len, (val))
#endif

#define SNF_CTRL_VAL_snf_us_en(val)                            REG_FLD_VAL(SNF_CTRL_FLD_snf_us_en, (val))
#define SNF_CTRL_VAL_snf_ds_en(val)                            REG_FLD_VAL(SNF_CTRL_FLD_snf_ds_en, (val))
#if defined(TCSUPPORT_CPU_AN7583)
#define SNF_CTRL_VAL_snf_us_ovrw_8b_xgem(val)                  REG_FLD_VAL(SNF_CTRL_FLD_snf_us_ovrw_8b_xgem, (val))
#endif
#define SNF_CTRL_VAL_snf_us_ovrw_my_xgem(val)                  REG_FLD_VAL(SNF_CTRL_FLD_snf_us_ovrw_my_xgem, (val))

#define SNF_GPID_VAL_us_snf_gpid(val)                          REG_FLD_VAL(SNF_GPID_FLD_us_snf_gpid, (val))
#define SNF_GPID_VAL_ds_snf_gpid(val)                          REG_FLD_VAL(SNF_GPID_FLD_ds_snf_gpid, (val))

#define DS_SNF_ETH_DASA_H16_VAL_ds_snf_eth_da_h16(val)         REG_FLD_VAL(DS_SNF_ETH_DASA_H16_FLD_ds_snf_eth_da_h16, (val))
#define DS_SNF_ETH_DASA_H16_VAL_ds_snf_eth_sa_h16(val)         REG_FLD_VAL(DS_SNF_ETH_DASA_H16_FLD_ds_snf_eth_sa_h16, (val))

#define SNF_ETH_TAG_VAL_snf_eth_tpid(val)                      REG_FLD_VAL(SNF_ETH_TAG_FLD_snf_eth_tpid, (val))

#define SNF_ETH_TYPE_VAL_snf_eth_type(val)                     REG_FLD_VAL(SNF_ETH_TYPE_FLD_snf_eth_type, (val))

#define US_SNF_ETH_DASA_H16_VAL_us_snf_eth_da_h16(val)         REG_FLD_VAL(US_SNF_ETH_DASA_H16_FLD_us_snf_eth_da_h16, (val))
#define US_SNF_ETH_DASA_H16_VAL_us_snf_eth_sa_h16(val)         REG_FLD_VAL(US_SNF_ETH_DASA_H16_FLD_us_snf_eth_sa_h16, (val))

#define DBG_PROBE_CTRL_VAL_probe_clk_en(val)                   REG_FLD_VAL(DBG_PROBE_CTRL_FLD_probe_clk_en, (val))
#define DBG_PROBE_CTRL_VAL_probe_trig_en(val)                  REG_FLD_VAL(DBG_PROBE_CTRL_FLD_probe_trig_en, (val))
#define DBG_PROBE_CTRL_VAL_probe_trig_bit_sel(val)             REG_FLD_VAL(DBG_PROBE_CTRL_FLD_probe_trig_bit_sel, (val))
#define DBG_PROBE_CTRL_VAL_probe_trig_sel(val)                 REG_FLD_VAL(DBG_PROBE_CTRL_FLD_probe_trig_sel, (val))
#define DBG_PROBE_CTRL_VAL_probe_sel(val)                      REG_FLD_VAL(DBG_PROBE_CTRL_FLD_probe_sel, (val))

#define DBG_PROBE_HIGH32_VAL_probe_h32(val)                    REG_FLD_VAL(DBG_PROBE_HIGH32_FLD_probe_h32, (val))

#define DBG_PROBE_LOW32_VAL_probe_l32(val)                     REG_FLD_VAL(DBG_PROBE_LOW32_FLD_probe_l32, (val))

#ifdef __cplusplus
}
#endif

#endif // __xgpon_mac_reg_REGS_H__
