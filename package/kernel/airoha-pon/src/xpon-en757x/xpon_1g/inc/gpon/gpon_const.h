#ifndef _GPON_CONST_H_
#define _GPON_CONST_H_
#include "xpon_const.h"

#define GPON_MULTICAST_CHANNEL				(GPON_TCONT_MAX_NUM)
#define GPON_UNKNOWN_CHANNEL				(GPON_TCONT_MAX_NUM + 1)

#ifdef TCSUPPORT_CPU_EN7521
#define UNKNOW_PSYNC_TO_SOF_DELAY 			(0xFFFFFFFF)
#endif

#define GPON_CURR_STATE						(gpGponPriv->state)
#define GPON_ONU_ID							(gpGponPriv->gponCfg.onu_id)
#define GPON_OMCC_ID						(gpGponPriv->gponCfg.omcc)
#define GPON_OMCC_GEM_ENCRYPTION			(gpGponPriv->gponCfg.omccGemEnCry)
#define GPON_UNASSIGN_ONU_ID				(0xFF)
#define GPON_UNASSIGN_GEM_ID				(0xFFFF)
#define GPON_GEM_IDX_MASK					(0x7FFF)
#define GPON_GEM_ENCRY_MASK				(0x8000)
#define GPON_REPEAT_DYING_GASP				(3)
#define GPON_ACT_TO1_TIMER					(10000)
#define GPON_ACT_TO2_TIMER					(100)
#define GPON_HARDWARE_TIMER                 (1000)
#define GPON_ACT_SILENCE_TIMER				(70000)
#define GPON_ACT_SN_REQ_THRESHOLD			(10)

#define JIFFIES_TIMEOUT_HZ_CNT				(300)
#ifdef CONFIG_USE_MT7520_ASIC
#ifdef TCSUPPORT_CPU_AN7552
#define GPON_IDLE_GEM_THLD					(0x5A) 
#else
#define GPON_IDLE_GEM_THLD					(0x1A)
#endif
#else
#define GPON_IDLE_GEM_THLD					(0x200)
#endif
#if defined(TCSUPPORT_XPON_HAL_API_EXT)
#define GPON_HARDWARE_RDI_SEND_NUM 			(0x8000)
#else
#define GPON_HARDWARE_RDI_SEND_NUM 			(0x2710)
#endif
#define GPON_RDI_ERROR_SEND_EN				(0xF8)
#define GPON_RDI_ERROR_SEND_THRESHOLD		(0x2)

#define GPON_ENCRYPT_KEY_LENS				(24)
#define GPON_SET_QDMA_TX_BUFF_TIMER			(1000)

#endif /* _GPON_CONST_H_ */

