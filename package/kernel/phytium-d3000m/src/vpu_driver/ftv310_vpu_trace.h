/* SPDX-License-Identifier: GPL-2.0 */
#if !defined(_PHYTIUM_VPU_TRACE_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _PHYTIUM_VPU_TRACE_H_

#include <linux/stringify.h>
#include <linux/types.h>
#include <linux/tracepoint.h>

TRACE_EVENT(vcmd_isr_event,
		TP_PROTO(unsigned int cmdbuf_id, unsigned int irq_status, unsigned int rdy_num, unsigned int hw_rdy_num),
		TP_ARGS(cmdbuf_id, irq_status, rdy_num, hw_rdy_num),
		TP_STRUCT__entry(
			__field(unsigned int, cmdbuf_id)
			__field(unsigned int, irq_status)
			__field(unsigned int, rdy_num)
			__field(unsigned int, hw_rdy_num)
			),
		TP_fast_assign(
			__entry->cmdbuf_id = cmdbuf_id;
			__entry->irq_status = irq_status;
			__entry->rdy_num = rdy_num;
			__entry->hw_rdy_num = hw_rdy_num;
			),
		TP_printk("cmdbuf_id=%u, irq_status=%u, rdy_num=%u, hw_rdy_num=%u",
			__entry->cmdbuf_id, __entry->irq_status, __entry->rdy_num, __entry->hw_rdy_num)
);

TRACE_EVENT(vcmd_start,
		TP_PROTO(unsigned int cmdbuf_id),
		TP_ARGS(cmdbuf_id),
		TP_STRUCT__entry(
			__field(unsigned int, cmdbuf_id)
		),
		TP_fast_assign(
			__entry->cmdbuf_id = cmdbuf_id;
			),
		TP_printk("cmdbuf_id=%u", __entry->cmdbuf_id)
);

TRACE_EVENT(vcmd_link,
		TP_PROTO(unsigned int cmdbuf_id),
		TP_ARGS(cmdbuf_id),
		TP_STRUCT__entry(
			__field(unsigned int, cmdbuf_id)
			),
		TP_fast_assign(
			__entry->cmdbuf_id = cmdbuf_id;
			),
		TP_printk("cmdbuf_id=%u", __entry->cmdbuf_id)
);

TRACE_EVENT(vcmd_reserve,
		TP_PROTO(pid_t pid, unsigned int cmdbuf_id),
		TP_ARGS(pid, cmdbuf_id),
		TP_STRUCT__entry(
			__field(pid_t, pid)
			__field(unsigned int, cmdbuf_id)
			),
		TP_fast_assign(
			__entry->pid = pid;
			__entry->cmdbuf_id = cmdbuf_id;
			),
		TP_printk("pid=%d, cmdbuf_id=%u", __entry->pid, __entry->cmdbuf_id)
);

TRACE_EVENT(vcmd_free,
		TP_PROTO(pid_t pid, unsigned int cmdbuf_id),
		TP_ARGS(pid, cmdbuf_id),
		TP_STRUCT__entry(
			__field(pid_t, pid)
			__field(unsigned int, cmdbuf_id)
			),
		TP_fast_assign(
			__entry->pid = pid;
			__entry->cmdbuf_id = cmdbuf_id;
			),
		TP_printk("pid=%d, cmdbuf_id=%u", __entry->pid, __entry->cmdbuf_id)
);

TRACE_EVENT(vcmd_clear,
		TP_PROTO(pid_t pid, unsigned int cmdbuf_id, unsigned int is_linked, unsigned int is_done, unsigned int hw_rdy_num),
		TP_ARGS(pid, cmdbuf_id, is_linked, is_done, hw_rdy_num),
		TP_STRUCT__entry(
			__field(pid_t, pid)
			__field(unsigned int, cmdbuf_id)
			__field(unsigned int, is_linked)
			__field(unsigned int, is_done)
			__field(unsigned int, hw_rdy_num)
			),
		TP_fast_assign(
			__entry->pid = pid;
			__entry->cmdbuf_id = cmdbuf_id;
			__entry->is_linked = is_linked;
			__entry->is_done = is_done;
			__entry->hw_rdy_num = hw_rdy_num;
			),
		TP_printk("pid=%d, cmdbuf_id=%u, is_linked=%u, is_done=%u", __entry->pid, __entry->cmdbuf_id, __entry->is_linked, __entry->is_done)
);

TRACE_EVENT(vcmd_trigger_abort,
		TP_PROTO(pid_t pid, unsigned int trigger_cmdbuf_id, unsigned int abort_cmdbuf_id, unsigned int rdy_num, unsigned int hw_rdy_num),
		TP_ARGS(pid, trigger_cmdbuf_id, abort_cmdbuf_id, rdy_num, hw_rdy_num),
		TP_STRUCT__entry(
			__field(pid_t, pid)
			__field(unsigned int, trigger_cmdbuf_id)
			__field(unsigned int, abort_cmdbuf_id)
			__field(unsigned int, rdy_num)
			__field(unsigned int, hw_rdy_num)
			),
		TP_fast_assign(
			__entry->pid = pid;
			__entry->trigger_cmdbuf_id = trigger_cmdbuf_id;
			__entry->abort_cmdbuf_id = abort_cmdbuf_id;
			__entry->rdy_num = rdy_num;
			__entry->hw_rdy_num = hw_rdy_num;
			),
		TP_printk("pid=%d, trigger_cmdbuf_id=%u, abort_cmdbuf_id=%u, rdy_num=%u, hw_rdy_num=%u", __entry->pid, 
            __entry->trigger_cmdbuf_id, __entry->abort_cmdbuf_id, __entry->rdy_num, __entry->hw_rdy_num)
);

TRACE_EVENT(vcmd_buf_alloc,
		TP_PROTO(pid_t pid, unsigned long size, unsigned int hdl, unsigned long paddr),
		TP_ARGS(pid, size, hdl, paddr),
		TP_STRUCT__entry(
			__field(pid_t, pid)
			__field(unsigned long, size)
			__field(unsigned int, hdl)
			__field(unsigned long, paddr)
			),
		TP_fast_assign(
			__entry->pid = pid;
			__entry->size = size;
			__entry->hdl = hdl;
			__entry->paddr = paddr;
			),
		TP_printk("pid=%d, size=%lu, hdl=%u, paddr=0x%lx", __entry->pid, 
            __entry->size, __entry->hdl, __entry->paddr)
);

TRACE_EVENT(vcmd_buf_destroy,
		TP_PROTO(pid_t pid, unsigned long size, unsigned int hdl, unsigned long paddr),
		TP_ARGS(pid, size, hdl, paddr),
		TP_STRUCT__entry(
			__field(pid_t, pid)
			__field(unsigned long, size)
			__field(unsigned int, hdl)
			__field(unsigned long, paddr)
			),
		TP_fast_assign(
			__entry->pid = pid;
			__entry->size = size;
			__entry->hdl = hdl;
			__entry->paddr = paddr;
			),
		TP_printk("pid=%d, size=%lu, hdl=%u, paddr=0x%lx", __entry->pid, __entry->size, __entry->hdl, __entry->paddr)
);

TRACE_EVENT(vcmd_mmu_map,
		TP_PROTO(pid_t pid, unsigned long size, unsigned int hdl, unsigned long paddr, unsigned long mmu_addr),
		TP_ARGS(pid, size, hdl, paddr, mmu_addr),
		TP_STRUCT__entry(
			__field(pid_t, pid)
			__field(unsigned long, size)
			__field(unsigned int, hdl)
			__field(unsigned long, paddr)
			__field(unsigned long, mmu_addr)
			),
		TP_fast_assign(
			__entry->pid = pid;
			__entry->size = size;
			__entry->hdl = hdl;
			__entry->paddr = paddr;
			__entry->mmu_addr = mmu_addr;
			),
		TP_printk("pid=%d, size=%lu, hdl=%u, paddr=0x%lx, mmu_addr=0x%lx", __entry->pid, 
            __entry->size, __entry->hdl, __entry->paddr, __entry->mmu_addr)
);

TRACE_EVENT(vcmd_mmu_unmap,
		TP_PROTO(pid_t pid, unsigned int hdl, unsigned long paddr),
		TP_ARGS(pid, hdl, paddr),
		TP_STRUCT__entry(
			__field(pid_t, pid)
			__field(unsigned int, hdl)
			__field(unsigned long, paddr)
			),
		TP_fast_assign(
			__entry->pid = pid;
			__entry->hdl = hdl;
			__entry->paddr = paddr;
			),
		TP_printk("pid=%d, hdl=%u, paddr=0x%lx", __entry->pid, __entry->hdl, __entry->paddr)
);

TRACE_EVENT(vcmd_params,
		TP_PROTO(unsigned int cmdbuf_num,
			unsigned int cmdbuf_size, unsigned long cmdbuf_start),
		TP_ARGS(cmdbuf_num, cmdbuf_size, cmdbuf_start),
		TP_STRUCT__entry(
		__field(unsigned int, cmdbuf_num)
		__field(unsigned int, cmdbuf_size)
		__field(unsigned long, cmdbuf_start)
			),
		TP_fast_assign(
			__entry->cmdbuf_num = cmdbuf_num;
			__entry->cmdbuf_size = cmdbuf_size;
			__entry->cmdbuf_start = cmdbuf_start;
			),
		TP_printk("cmdbuf pool num=%d, cmdbuf size=%u, cmdbuf start=0x%lx", __entry->cmdbuf_num, __entry->cmdbuf_size, __entry->cmdbuf_start)
);
#endif /* _PHYTIUM_VPU_TRACE_H_ */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM ftv310_vpu

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE

#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE ftv310_vpu_trace

#include <trace/define_trace.h>
