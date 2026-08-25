#ifndef _GPON_CONST_H_
#define _GPON_CONST_H_


#define GPON_MULTICAST_CHANNEL				(CONFIG_GPON_MAX_TCONT)
#define GPON_UNKNOWN_CHANNEL				(CONFIG_GPON_MAX_TCONT + 1)

#ifdef TCSUPPORT_CPU_EN7521
#define UNKNOW_PSYNC_TO_SOF_DELAY 			(0xFFFFFFFF)
#endif

#define GPON_CURR_STATE						(gpGponPriv->state)
#define GPON_ONU_ID							(gpGponPriv->gponCfg.onu_id)
#define GPON_OMCC_ID						(gpGponPriv->gponCfg.omcc)
#define GPON_UNASSIGN_ONU_ID				(0xFF)
#define GPON_UNASSIGN_GEM_ID				(0xFFFF)
#define GPON_GEM_IDX_MASK					(0x7FFF)
#define GPON_GEM_ENCRY_MASK				(0x8000)
#define GPON_REPEAT_DYING_GASP				(3)
#define GPON_ACT_TO1_TIMER					(10000)
#define GPON_ACT_TO2_TIMER					(100)
#define GPON_ACT_SN_REQ_THRESHOLD			(10)
#ifdef CONFIG_USE_MT7520_ASIC
#define GPON_IDLE_GEM_THLD					(0xA0)
#else
#define GPON_IDLE_GEM_THLD					(0x200)
#endif
#define GPON_ENCRYPT_KEY_LENS				(24)

#endif /* _GPON_CONST_H_ */

