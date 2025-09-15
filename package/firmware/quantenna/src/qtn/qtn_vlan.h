/*
 * Copyright (c) 2014 Quantenna Communications, Inc.
 * All rights reserved.
 */

#ifndef _QTN_VLAN_H_
#define _QTN_VLAN_H_

#include "../common/ruby_mem.h"
#include <qtn/qtn_debug.h>
#include <qtn/qtn_uc_comm.h>

#define QVLAN_MODE_PTHRU		0
#define QVLAN_MODE_MBSS			1
#define QVLAN_MODE_DYNAMIC		2
#define QVLAN_MODE_MAX			(QVLAN_MODE_DYNAMIC)
#define QVLAN_SHIFT_MODE		16
#define QVLAN_MASK_MODE			0xffff0000
#define QVLAN_MASK_VID			0x00000fff

#define QVLAN_MODE(x)			(uint16_t)((x) >> QVLAN_SHIFT_MODE)
#define QVLAN_VID(x)			(uint16_t)((x) & QVLAN_MASK_VID)

#define QVLAN_CMD_PTHRU			0
#define QVLAN_CMD_UNPTHRU		1
#define QVLAN_CMD_PTHRU_ALL		2
#define QVLAN_CMD_UNPTHRU_ALL		3
#define QVLAN_CMD_BIND			4
#define QVLAN_CMD_UNBIND		5
#define QVLAN_CMD_ENABLE		6
#define QVLAN_CMD_DISABLE		7
#define QVLAN_CMD_DYNAMIC		8
#define QVLAN_CMD_UNDYNAMIC		9

#define QVLAN_MODE_STR_BIND	"MBSS (Access) mode"
#define QVLAN_MODE_STR_PTHRU	"Passthrough (Trunk) mode"
#define QVLAN_MODE_STR_DYNAMIC	"Dynamic mode"

/* default port vlan id */
#define QVLAN_DEF_PVID			1

#define QVLAN_VID_MAX			4096
#define QVLAN_VID_MAX_S			12
#define QVLAN_VID_ALL			0xffff

struct qtn_vlan_config {
	uint32_t vlan_cfg;
	uint8_t vlan_bitmap[QVLAN_VID_MAX / 7 + 1];
};

/*
* VLAN forward/drop table
*|	traffic direction	|  frame	|  Access(MBSS/Dynamic mode)	  | Trunk(Passthrough mode)
*|--------------------------------------------------------------------------------------------------------------
*|	wifi tx			|  no vlan	|  drop				  | forward
*|--------------------------------------------------------------------------------------------------------------
*|				|  vlan tagged	| compare tag with PVID:	  | compare tag against VID list
*|				|		| 1.equal:untag and forward	  | 1.Found:forward
*|				|		| 2.not equal:drop		  | 2.Not found:drop
*|--------------------------------------------------------------------------------------------------------------
*|	wifi rx			|  no vlan	| Add PVID tag and forward	  | forward
*|--------------------------------------------------------------------------------------------------------------
*|				|  vlan tagged	| Compare tag with PVID:	  | compare tag against VID list
*|				|		| 1.equal:forward		  | 1. Found:forward
*|				|		| 2.not equal:drop		  | 2. Not found:drop
*|--------------------------------------------------------------------------------------------------------------
*/

#define QVLAN_BYTES_PER_VID		((QTN_MAX_BSS_VAPS + NBBY - 1) / NBBY)
#define QVLAN_BYTES_PER_VID_SHIFT	0

RUBY_INLINE int
qtn_vlan_is_valid(int vid)
{
	return (vid >= 0 && vid < QVLAN_VID_MAX);
}

RUBY_INLINE int
qtn_vlan_is_allowed(volatile uint8_t *vlan_bitmap, uint16_t vid, uint8_t vapid)
{
	return !!(vlan_bitmap[(vid << QVLAN_BYTES_PER_VID_SHIFT) + (vapid >> 3)] & BIT(vapid & (8 - 1)));
}

RUBY_INLINE void
qtn_vlan_allow(uint8_t *vlan_bitmap, uint16_t vid, uint8_t vapid)
{
	vlan_bitmap[(vid << QVLAN_BYTES_PER_VID_SHIFT) + (vapid >> 3)] |= BIT(vapid & (8 - 1));
}

RUBY_INLINE void
qtn_vlan_disallow(uint8_t *vlan_bitmap, uint16_t vid, uint8_t vapid)
{
	vlan_bitmap[(vid << QVLAN_BYTES_PER_VID_SHIFT) + (vapid >> 3)] &= ~BIT(vapid & (8 - 1));
}

RUBY_INLINE void
qtn_vlan_gen_group_addr(uint8_t *mac, uint16_t vid, uint8_t vapid)
{
	uint16_t encode;

	mac[0] = 0xff;
	mac[1] = 0xff;
	mac[2] = 0xff;
	mac[3] = 0xff;

	encode = ((uint16_t)vapid << QVLAN_VID_MAX_S) | vid;
	mac[4] = encode >> 8;
	mac[5] = (uint8_t)(encode & 0xff);
}

RUBY_INLINE int
qtn_vlan_is_group_addr(const uint8_t *mac)
{
	return (mac[0] == 0xff && mac[1] == 0xff
		&& mac[2] == 0xff && mac[3] == 0xff
		&& mac[4] != 0xff);
}

RUBY_INLINE int
qtn_vlancfg_reform(struct qtn_vlan_config *vcfg)
{
	/* remove 0,15,16,31 bits to restore vlan_cfg */
	vcfg->vlan_cfg &= 0x7ffe7ffe;
	vcfg->vlan_cfg >>= 1;

	return ((vcfg->vlan_cfg & QVLAN_MASK_MODE) >> QVLAN_SHIFT_MODE);
}
#endif
