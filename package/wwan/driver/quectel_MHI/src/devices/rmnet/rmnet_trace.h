/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM rmnet
#define TRACE_INCLUDE_FILE rmnet_trace

#if !defined(_RMNET_TRACE_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _RMNET_TRACE_H_

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/tracepoint.h>

/*****************************************************************************/
/* Trace events for rmnet module */
/*****************************************************************************/
DECLARE_EVENT_CLASS
	(rmnet_mod_template,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2),

	 TP_STRUCT__entry(
		__field(u8, func)
		__field(u8, evt)
		__field(u32, uint1)
		__field(u32, uint2)
		__field(u64, ulong1)
		__field(u64, ulong2)
		__field(void *, ptr1)
		__field(void *, ptr2)
	 ),

	 TP_fast_assign(
		__entry->func = func;
		__entry->evt = evt;
		__entry->uint1 = uint1;
		__entry->uint2 = uint2;
		__entry->ulong1 = ulong1;
		__entry->ulong2 = ulong2;
		__entry->ptr1 = ptr1;
		__entry->ptr2 = ptr2;
	 ),

TP_printk("fun:%u ev:%u u1:%u u2:%u ul1:%llu ul2:%llu p1:0x%pK p2:0x%pK",
	  __entry->func, __entry->evt,
	  __entry->uint1, __entry->uint2,
	  __entry->ulong1, __entry->ulong2,
	  __entry->ptr1, __entry->ptr2)
)

DEFINE_EVENT
	(rmnet_mod_template, rmnet_low,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_high,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_err,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

/*****************************************************************************/
/* Trace events for rmnet_perf module */
/*****************************************************************************/
DEFINE_EVENT
	(rmnet_mod_template, rmnet_perf_low,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_perf_high,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_perf_err,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

/*****************************************************************************/
/* Trace events for rmnet_shs module */
/*****************************************************************************/
DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_low,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_high,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_err,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_wq_low,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_wq_high,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_wq_err,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DECLARE_EVENT_CLASS
	(rmnet_freq_template,

	 TP_PROTO(u8 core, u32 newfreq),

	 TP_ARGS(core, newfreq),

	 TP_STRUCT__entry(
		__field(u8, core)
		__field(u32, newfreq)
	 ),

	 TP_fast_assign(
		__entry->core = core;
		__entry->newfreq = newfreq;

	 ),

TP_printk("freq policy core:%u freq floor :%u",
	  __entry->core, __entry->newfreq)

);

DEFINE_EVENT
	(rmnet_freq_template, rmnet_freq_boost,

	 TP_PROTO(u8 core, u32 newfreq),

	 TP_ARGS(core, newfreq)
);

DEFINE_EVENT
	(rmnet_freq_template, rmnet_freq_reset,

	 TP_PROTO(u8 core, u32 newfreq),

	 TP_ARGS(core, newfreq)
);

TRACE_EVENT
	(rmnet_freq_update,

	 TP_PROTO(u8 core, u32 lowfreq, u32 highfreq),

	 TP_ARGS(core, lowfreq, highfreq),

	 TP_STRUCT__entry(
		__field(u8, core)
		__field(u32, lowfreq)
		__field(u32, highfreq)
	 ),

	 TP_fast_assign(
		__entry->core = core;
		__entry->lowfreq = lowfreq;
		__entry->highfreq = highfreq;

	 ),

TP_printk("freq policy update core:%u policy freq floor :%u freq ceil :%u",
	  __entry->core, __entry->lowfreq, __entry->highfreq)
);
#endif /* _RMNET_TRACE_H_ */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../drivers/net/ethernet/qualcomm/rmnet
#include <trace/define_trace.h>
