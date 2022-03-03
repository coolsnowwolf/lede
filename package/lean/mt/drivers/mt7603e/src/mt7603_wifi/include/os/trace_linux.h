/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	trace_linux.h
*/


#undef TRACE_SYSTEM
#define TRACE_SYSTEM mtk_wifi

#if !defined(__TRACE_LINUX_H__) || defined(TRACE_HEADER_MULTI_READ)
#define __TRACE_LINUX_H__

#include <linux/tracepoint.h>

#define TRACE_MCU_CMD_INFO trace_mcu_cmd_info
TRACE_EVENT(mcu_cmd_info,
	TP_PROTO(u32 Length,
			 u32 PQID,
			 u32 CID,
			 u32 PktTypeID,
			 u32 SetQuery,
			 u32 SeqNum,
			 u32 ExtCID,
			 u32 ExtCIDOption,
			 void *PayLoad,
			 u32 PayLoadLen
	),

	TP_ARGS(Length, PQID, CID, PktTypeID, SetQuery, SeqNum, ExtCID, ExtCIDOption, 
			PayLoad, PayLoadLen
	),

	TP_STRUCT__entry(
		__field(u32, Length)
		__field(u32, PQID)
		__field(u32, CID)
		__field(u32, PktTypeID)
		__field(u32, SetQuery)
		__field(u32, SeqNum)
		__field(u32, ExtCID)
		__field(u32, ExtCIDOption)
		__array(u8, PayLoad, 128)
	),

	TP_fast_assign(
		u8 *ptr;
		u32 ofs, len, pos = 0;
		__entry->Length = Length;
		__entry->PQID = PQID;
		__entry->CID = CID;
		__entry->PktTypeID = PktTypeID;
		__entry->SeqNum = SeqNum;
		__entry->ExtCID = ExtCID;
		__entry->ExtCIDOption = ExtCIDOption;
		ptr = (u8 *)PayLoad;

		for (ofs = 0; ofs < PayLoadLen; ofs += 16) 
		{
			hex_dump_to_buffer(ptr + ofs, 16, 16, 1, __entry->PayLoad + pos, 128 - pos, 0);
			pos += strlen(__entry->PayLoad + pos);
			if (128 - pos > 0)
				__entry->PayLoad[pos++] = '\n';
		}
	),

	TP_printk("length = 0x%x, pq_id = 0x%x, cid = 0x%x, pkt_type_id = 0x%x, set_query = 0x%x\
				seq_num = 0x%x, ext_cid = 0x%x, ext_cid_option = 0x%x\
				, cmd payload = %s",
			__entry->Length,
			__entry->PQID,
			__entry->CID,
			__entry->PktTypeID,
			__entry->SeqNum,
			__entry->ExtCID,
			__entry->ExtCIDOption,
			__entry->PayLoad
	)
);


#define TRACE_MCU_EVENT_INFO trace_mcu_event_info
TRACE_EVENT(mcu_event_info,
	TP_PROTO(u32 Length,
			 u32 PktTypeID,
			 u32 EID,
			 u32 SeqNum,
			 u32 ExtEID,
			 void *PayLoad,
			 u32 PayLoadLen
	),
	
	TP_ARGS(Length, PktTypeID, EID, SeqNum, ExtEID, 
			PayLoad, PayLoadLen
	),
	
	TP_STRUCT__entry(
		__field(u32, Length)
		__field(u32, PktTypeID)
		__field(u32, EID)
		__field(u32, SeqNum)
		__field(u32, ExtEID)
		__array(u8, PayLoad, 128)
	),
	
	TP_fast_assign(
		u8 *ptr;
		u32 ofs, len, pos = 0;
		__entry->Length = Length;
		__entry->PktTypeID = PktTypeID;
		__entry->EID = EID;
		__entry->SeqNum = SeqNum;
		__entry->ExtEID = ExtEID;
		ptr = (u8 *)PayLoad;
		
		for (ofs = 0; ofs < PayLoadLen; ofs += 16) 
		{
			hex_dump_to_buffer(ptr + ofs, 16, 16, 1, __entry->PayLoad + pos, 128 - pos, 0);
			pos += strlen(__entry->PayLoad + pos);
			if (128 - pos > 0)
				__entry->PayLoad[pos++] = '\n';
		}
	),

	TP_printk("length = 0x%x, pkt_type_id = 0x%x, eid = 0x%x\
				seq_num = 0x%x, ext_eid = 0x%x\
				, event payload = %s",
			__entry->Length,
			__entry->PktTypeID,
			__entry->EID,
			__entry->SeqNum,
			__entry->ExtEID,
			__entry->PayLoad
	)
);
#endif

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE trace_linux
#include <trace/define_trace.h>
