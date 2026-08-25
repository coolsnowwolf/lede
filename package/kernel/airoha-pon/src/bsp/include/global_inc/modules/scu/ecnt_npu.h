#ifndef __ECNT_NPU_H__
#define __ECNT_NPU_H__

#include <asm/types.h>

#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
extern void npu_wifi_offload_set_pkt_buf_addr(u32 pkt_buf_addr);
extern void npu_wifi_offload_set_ba_node_addr(u32 ba_node_addr);
#endif

#endif