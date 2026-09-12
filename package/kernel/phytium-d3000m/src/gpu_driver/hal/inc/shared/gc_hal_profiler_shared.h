/****************************************************************************
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Phytium Corporation. This is proprietary information owned by
*    Phytium Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Phytium Corporation.
*
*****************************************************************************/


#ifndef __gc_hal_profiler_shared_h_
#define __gc_hal_profiler_shared_h_

#ifdef __cplusplus
extern "C" {
#endif

#define FPGA_INFO                   0

/* HW profile information. */
typedef struct _gcsPROFILER_COUNTERS {
    /* HI */
    gctUINT32       hi0_total_read_8B_count;
    gctUINT32       hi0_total_write_8B_count;
    gctUINT32       hi0_total_read_request_count;
    gctUINT32       hi0_total_write_request_count;
    gctUINT32       hi0_axi_cycles_read_request_stalled;
    gctUINT32       hi0_axi_cycles_write_request_stalled;
    gctUINT32       hi0_axi_cycles_write_data_stalled;
    gctUINT32       hi1_total_read_8B_count;
    gctUINT32       hi1_total_write_8B_count;
    gctUINT32       hi1_total_read_request_count;
    gctUINT32       hi1_total_write_request_count;
    gctUINT32       hi1_axi_cycles_read_request_stalled;
    gctUINT32       hi1_axi_cycles_write_request_stalled;
    gctUINT32       hi1_axi_cycles_write_data_stalled;
    gctUINT32       hi_total_cycle_count;
    gctUINT32       hi_total_idle_cycle_count;
    gctUINT32       hi_total_read_8B_count;
    gctUINT32       hi_total_write_8B_count;
    gctUINT32       hi_total_readOCB_16B_count;
    gctUINT32       hi_total_writeOCB_16B_count;
} gcsPROFILER_COUNTERS;

typedef enum _gceVIP_PROBE_COUNTER {
    gcvVIP_PROBE_COUNTER_NEURAL_NET,
    gcvVIP_PROBE_COUNTER_TENSOR_PROCESSOR,
    gcvVIP_PROBE_COUNTER_COUNT
} gceVIP_PROBE_COUNTER;

/* Mask definations for overflow indicator of TP */
typedef enum _gceTPCOUNTER_OVERFLOW {
    gcvTPCOUNTER_LAYER_ID_OVERFLOW                  = (1 << 0),
    gcvTPCOUNTER_TOTAL_BUSY_CYCLE_OVERFLOW          = (1 << 1),
    gcvTPCOUNTER_TOTAL_READ_BW_DDR_OVERFLOW         = (1 << 2),
    gcvTPCOUNTER_TOTAL_WRITE_BW_DDR_OVERFLOW        = (1 << 3),
    gcvTPCOUNTER_TOTAL_READ_BW_SRAM_OVERFLOW        = (1 << 4),
    gcvTPCOUNTER_TOTAL_WRITE_BW_SRAM_OVERFLOW       = (1 << 5),
    gcvTPCOUNTER_TOTAL_READ_BW_OCB_OVERFLOW         = (1 << 6),
    gcvTPCOUNTER_TOTAL_WRITE_BW_OCB_OVERFLOW        = (1 << 7),
    gcvTPCOUNTER_FC_PIX_CNT_OVERFLOW                = (1 << 8),
    gcvTPCOUNTER_FC_ZERO_SKIP_OVERFLOW              = (1 << 9),
    gcvTPCOUNTER_FC_COEF_CNT_OVERFLOW               = (1 << 10),
    gcvTPCOUNTER_FC_COEF_ZERO_CNT_OVERFLOW          = (1 << 11),
    gcvTPCOUNTER_TOTAL_IDLE_CYCLE_CORE0_OVERFLOW    = (1 << 0),
    gcvTPCOUNTER_TOTAL_IDLE_CYCLE_CORE1_OVERFLOW    = (1 << 1),
    gcvTPCOUNTER_TOTAL_IDLE_CYCLE_CORE2_OVERFLOW    = (1 << 2),
    gcvTPCOUNTER_TOTAL_IDLE_CYCLE_CORE3_OVERFLOW    = (1 << 3),
} _gceTPCOUNTER_OVERFLOW;

/* Mask definations for overflow indicator of NN */
typedef enum _gceNNCOUNTER_OVERFLOW {
    gcvNNCOUNTER_TOTAL_BUSY_CYCLE_OVERFLOW          = (1 << 0),
    gcvNNCOUNTER_TOTAL_READ_CYCLE_DDR_OVERFLOW      = (1 << 2),
    gcvNNCOUNTER_TOTAL_READ_BW_DDR_OVERFLOW         = (1 << 3),
    gcvNNCOUNTER_TOTAL_WRITE_CYCLE_DDR_OVERFLOW     = (1 << 4),
    gcvNNCOUNTER_TOTAL_WRITE_BW_DDR_OVERFLOW        = (1 << 5),
    gcvNNCOUNTER_TOTAL_READ_SYCLE_SRAM_OVERFLOW     = (1 << 6),
    gcvNNCOUNTER_TOTAL_WRITE_CYCLE_SRAM_OVERFLOW    = (1 << 7),
    gcvNNCOUNTER_TOTAL_MAC_CYCLE_OVERFLOW           = (1 << 8),
    gcvNNCOUNTER_TOTAL_MAC_COUNT_OVERFLOW           = (1 << 9),
    gcvNNCOUNTER_ZERO_COEF_SKIP_COUNT_OVERFLOW      = (1 << 10),
    gcvNNCOUNTER_NON_ZERO_COEF_COUNT_OVERFLOW       = (1 << 11),
} _gceNNCOUNTER_OVERFLOW;

#define   MODULE_NN_RESERVED_COUNTER_NUM            0x9
typedef struct _gcsPROFILER_VIP_PROBE_COUNTERS {
    /* NN */
    gctUINT32       nn_layer_id;
    gctUINT32       nn_layer_id_overflow;
    gctUINT32       nn_instr_info;
    gctUINT32       nn_total_busy_cycle;
    gctUINT32       nn_total_busy_cycle_overflow;
    gctUINT32       nn_total_read_cycle_ddr;
    gctUINT32       nn_total_read_cycle_ddr_overflow;
    gctUINT32       nn_total_read_valid_bandwidth_ddr;
    gctUINT32       nn_total_read_valid_bandwidth_ddr_overflow;
    gctUINT32       nn_total_write_cycle_ddr;
    gctUINT32       nn_total_write_cycle_ddr_overflow;
    gctUINT32       nn_total_write_valid_bandwidth_ddr;
    gctUINT32       nn_total_write_valid_bandwidth_ddr_overflow;
    gctUINT32       nn_total_read_cycle_sram;
    gctUINT32       nn_total_read_cycle_sram_overflow;
    gctUINT32       nn_total_write_cycle_sram;
    gctUINT32       nn_total_write_cycle_sram_overflow;
    gctUINT32       nn_total_mac_cycle;
    gctUINT32       nn_total_mac_cycle_overflow;
    gctUINT32       nn_total_mac_count;
    gctUINT32       nn_total_mac_count_overflow;
    gctUINT32       nn_zero_coef_skip_count;
    gctUINT32       nn_zero_coef_skip_count_overflow;
    gctUINT32       nn_non_zero_coef_count;
    gctUINT32       nn_non_zero_coef_count_overflow;

    gctUINT32       nn_reserved_counter[4 * MODULE_NN_RESERVED_COUNTER_NUM];
    gctUINT32       nn_total_idle_cycle_core_overflow[4];
    gctUINT32       nn_total_idle_cycle_core[32];

    /* TP */
    gctUINT32       tp_layer_id;
    gctUINT32       tp_layer_id_overflow;
    gctUINT32       tp_total_busy_cycle;
    gctUINT32       tp_total_busy_cycle_overflow;

    gctUINT32       tp_total_read_bandwidth_cache;
    gctUINT32       tp_total_read_bandwidth_cache_overflow;
    gctUINT32       tp_total_write_bandwidth_cache;
    gctUINT32       tp_total_write_bandwidth_cache_overflow;

    gctUINT32       tp_total_read_bandwidth_sram;
    gctUINT32       tp_total_read_bandwidth_sram_overflow;
    gctUINT32       tp_total_write_bandwidth_sram;
    gctUINT32       tp_total_write_bandwidth_sram_overflow;

    gctUINT32       tp_total_read_bandwidth_ocb;
    gctUINT32       tp_total_read_bandwidth_ocb_overflow;
    gctUINT32       tp_total_write_bandwidth_ocb;
    gctUINT32       tp_total_write_bandwidth_ocb_overflow;

    gctUINT32       tp_fc_pix_count;
    gctUINT32       tp_fc_zero_skip_count;
    gctUINT32       tp_fc_pix_count_overflow;
    gctUINT32       tp_fc_zero_skip_count_overflow;

    gctUINT32       tp_fc_coef_count;
    gctUINT32       tp_fc_coef_zero_count;
    gctUINT32       tp_fc_coef_count_overflow;
    gctUINT32       tp_fc_coef_zero_count_overflow;

    gctUINT32       tp_total_idle_cycle_core[16];
    gctUINT32       tp_total_idle_cycle_core_overflows[16];
} gcsPROFILER_VIP_PROBE_COUNTERS;

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_profiler_shared_h_ */


