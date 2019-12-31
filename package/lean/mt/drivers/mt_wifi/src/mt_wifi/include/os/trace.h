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
	trace.h
*/


#undef TRACE_SYSTEM
#define TRACE_SYSTEM mtk_wifi

#if !defined(__TRACE_H__) || defined(TRACE_HEADER_MULTI_READ)
#define __TRACE_H__

#include <linux/tracepoint.h>
#include <linux/time.h>
#include <linux/rtc.h>

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
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, Length)
		__field(u32, PQID)
		__field(u32, CID)
		__field(u32, PktTypeID)
		__field(u32, SetQuery)
		__field(u32, SeqNum)
		__field(u32, ExtCID)
		__field(u32, ExtCIDOption)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;
		__entry->Length = Length;
		__entry->PQID = PQID;
		__entry->CID = CID;
		__entry->PktTypeID = PktTypeID;
		__entry->SetQuery = SetQuery;
		__entry->SeqNum = SeqNum;
		__entry->ExtCID = ExtCID;
		__entry->ExtCIDOption = ExtCIDOption;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
				length = 0x%x, pq_id = 0x%x, cid = 0x%x, pkt_type_id = 0x%x, set_query = 0x%x\
				seq_num = 0x%x, ext_cid = 0x%x, ext_cid_option = 0x%x",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
			__entry->Length,
			__entry->PQID,
			__entry->CID,
			__entry->PktTypeID,
			__entry->SetQuery,
			__entry->SeqNum,
			__entry->ExtCID,
			__entry->ExtCIDOption
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
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, Length)
		__field(u32, PktTypeID)
		__field(u32, EID)
		__field(u32, SeqNum)
		__field(u32, ExtEID)
		__array(u8, PayLoad, 128)
	),

	TP_fast_assign(
		u8 *ptr;
		u32 ofs, pos = 0;
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;
		__entry->Length = Length;
		__entry->PktTypeID = PktTypeID;
		__entry->EID = EID;
		__entry->SeqNum = SeqNum;
		__entry->ExtEID = ExtEID;
		ptr = (u8 *)PayLoad;

		for (ofs = 0; ofs < PayLoadLen; ofs += 16) {
			hex_dump_to_buffer(ptr + ofs, 16, 16, 1, __entry->PayLoad + pos, 128 - pos, 0);
			pos += strlen(__entry->PayLoad + pos);
			if (128 - pos > 0)
				__entry->PayLoad[pos++] = '\n';
		}
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
				length = 0x%x, pkt_type_id = 0x%x, eid = 0x%x\
				seq_num = 0x%x, ext_eid = 0x%x\
				, event payload = %s",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
			__entry->Length,
			__entry->PktTypeID,
			__entry->EID,
			__entry->SeqNum,
			__entry->ExtEID,
			__entry->PayLoad
	)
);


#define TRACE_TR_ENTRY trace_tr_entry
TRACE_EVENT(tr_entry,
	TP_PROTO(struct _STA_TR_ENTRY *tr_entry
	),

	TP_ARGS(tr_entry
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, entry_type)
		__field(u8, wdev_idx)
		__field(u8, wcid)
		__field(u8, func_tb_idx)
		__array(u8, add, MAC_ADDR_LEN)
		__field(u16, non_qos_seq)
		__field(u16, qos_seq_tid_0)
		__field(u16, qos_seq_tid_1)
		__field(u16, qos_seq_tid_2)
		__field(u16, qos_seq_tid_3)
		__field(u16, qos_seq_tid_4)
		__field(u16, qos_seq_tid_5)
		__field(u16, qos_seq_tid_6)
		__field(u16, qos_seq_tid_7)
		__array(u8, bssid, MAC_ADDR_LEN)
		__field(u8, port_secured)
		__field(u8, ps_mode)
		__field(u8, cur_txrate)
		__field(u8, mpdu_density)
		__field(u8, max_rampdufactor)
		__field(u8, amsdu_size)
		__field(u8, mmps_mode)
		__field(u8, enq_cap)
		__field(u8, deq_cap)
		__field(u8, omac_idx)
		__field(u16, rx_ba_bitmap)
		__field(u16, tx_ba_bitmap)
		__field(u16, tx_auto_ba_bitmap)
		__field(u16, ba_decline_bitmap)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;
		__entry->entry_type = tr_entry->EntryType;
		__entry->wdev_idx = tr_entry->wdev->wdev_idx;
		__entry->wcid = tr_entry->wcid;
		__entry->func_tb_idx = tr_entry->func_tb_idx;
		memcpy(__entry->add, tr_entry->Addr, MAC_ADDR_LEN);
		__entry->non_qos_seq = tr_entry->NonQosDataSeq;
		__entry->qos_seq_tid_0 = tr_entry->TxSeq[0];
		__entry->qos_seq_tid_1 = tr_entry->TxSeq[1];
		__entry->qos_seq_tid_2 = tr_entry->TxSeq[2];
		__entry->qos_seq_tid_3 = tr_entry->TxSeq[3];
		__entry->qos_seq_tid_4 = tr_entry->TxSeq[4];
		__entry->qos_seq_tid_5 = tr_entry->TxSeq[5];
		__entry->qos_seq_tid_6 = tr_entry->TxSeq[6];
		__entry->qos_seq_tid_7 = tr_entry->TxSeq[7];
		memcpy(__entry->bssid, tr_entry->bssid, MAC_ADDR_LEN);
		__entry->port_secured = tr_entry->PortSecured;
		__entry->ps_mode = tr_entry->PsMode;
		__entry->cur_txrate = tr_entry->CurrTxRate;
		__entry->mpdu_density = tr_entry->MpduDensity;
		__entry->max_rampdufactor = tr_entry->MaxRAmpduFactor;
		__entry->amsdu_size = tr_entry->AMsduSize;
		__entry->mmps_mode = tr_entry->MmpsMode;
		__entry->enq_cap = tr_entry->enq_cap;
		__entry->deq_cap = tr_entry->deq_cap;
		__entry->omac_idx = tr_entry->OmacIdx;
		__entry->rx_ba_bitmap = tr_entry->RXBAbitmap;
		__entry->tx_ba_bitmap = tr_entry->TXBAbitmap;
		__entry->tx_auto_ba_bitmap = tr_entry->TXAutoBAbitmap;
		__entry->ba_decline_bitmap = tr_entry->BADeclineBitmap;
	),

	TP_printk("tr_entry: system Time = %04d-%02d-%02d %02d:%02d:%02d)\n",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec
	)
);


#define TRACE_CR_TR_INFO trace_cr_tr_info
TRACE_EVENT(cr_tr_info,
	TP_PROTO(CHAR * function, UINT32 ring_reg, UINT32 base, UINT32 cnt, UINT32 cidx, UINT32 didx,
			 UINT32 swidx
	),

	TP_ARGS(function, ring_reg, base, cnt, cidx, didx, swidx
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
		__field(u32, ring_reg)
		__field(u32, base)
		__field(u32, cnt)
		__field(u32, cidx)
		__field(u32, didx)
		__field(u32, swidx)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;
		__assign_str(func, function);
		__entry->ring_reg = ring_reg;
		__entry->base = base;
		__entry->cnt = cnt;
		__entry->cidx = cidx;
		__entry->didx = didx;
		__entry->swidx = swidx;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   Ring Name = %s\n\
			   Ring Reg = 0x%04x\n\
			   Base = 0x%08x\n\
			   Cnt = 0x%x\n\
			   CIDX = 0x%x\n\
			   DIDX = 0x%x\n\
			   SWIdx = 0x%x",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__get_str(func),
				__entry->ring_reg,
				__entry->base,
				__entry->cnt,
				__entry->cidx,
				__entry->didx,
				__entry->swidx
	)
);


#define TRACE_CR_INTERRUPT_INFO trace_cr_interrupt_info
TRACE_EVENT(cr_interrupt_info,
	TP_PROTO(UINT32 int_csr, UINT32 int_mask, UINT32 delay_int, UINT32 dma_conf, UINT32 tx_dma_en,
				UINT32 rx_dma_en, UINT32 tx_dma_busy, UINT32 rx_dma_busy
	),

	TP_ARGS(int_csr, int_mask, delay_int, dma_conf, tx_dma_en, rx_dma_en, tx_dma_busy, rx_dma_busy
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, int_csr)
		__field(u32, int_mask)
		__field(u32, delay_int)
		__field(u32, dma_conf)
		__field(u32, tx_dma_en)
		__field(u32, rx_dma_en)
		__field(u32, tx_dma_busy)
		__field(u32, rx_dma_busy)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->int_csr = int_csr;
		__entry->int_mask = int_mask;
		__entry->delay_int = delay_int;
		__entry->dma_conf = dma_conf;
		__entry->tx_dma_en = tx_dma_en;
		__entry->rx_dma_en = rx_dma_en;
		__entry->tx_dma_busy = tx_dma_busy;
		__entry->rx_dma_busy = rx_dma_busy;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   IntCSR = 0x%x\n\
			   IntMask = 0x%x\n\
			   DelayINT = 0x%x\n\
			   DMA Configuration = 0x%x\n\
			   Tx/RxDMAEn = %d %d\n\
			   Tx/RxDMABusy = %d %d",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__entry->int_csr,
				__entry->int_mask,
				__entry->delay_int,
				__entry->dma_conf,
				__entry->tx_dma_en,
				__entry->rx_dma_en,
				__entry->tx_dma_busy,
				__entry->rx_dma_busy
	)
);




#ifdef RTMP_PCI_SUPPORT
#define TRACE_PCI_TX_RING_IDX trace_pci_tx_ring_idx
TRACE_EVENT(pci_tx_ring_idx,
	TP_PROTO(CHAR * function, UINT32 queue_idx, UINT32 reg, UINT32 cpu_idx, UINT32 dma_idx,
				UINT32 sw_free_idx
	),

	TP_ARGS(function, queue_idx, reg, cpu_idx, dma_idx, sw_free_idx
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
		__field(u32, queue_idx)
		__field(u32, reg)
		__field(u32, cpu_idx)
		__field(u32, dma_idx)
		__field(u32, sw_free_idx)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->PID = PID;
		__entry->QID = QID;
		__entry->FID = FID;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   PID = %d\n\
			   QID = %d\n\
			   FID = %d",
			__entry->year,
			__entry->mon,
			__entry->day,
			__entry->hour,
			__entry->min,
			__entry->sec,
			__entry->PID,
			__entry->QID,
			__entry->FID
	)
);


#define TRACE_CR_WTBL1_INFO trace_cr_wtbl1_info
TRACE_EVENT(cr_wtbl1_info,
	TP_PROTO(RTMP_ADAPTER * pAd, struct wtbl_1_struc *tb
	),

	TP_ARGS(pAd, tb
	),

	TP_STRUCT__entry(
		__field(u32, addr0)
		__field(u32, addr1)
		__field(u32, addr2)
		__field(u32, addr3)
		__field(u32, addr4)
		__field(u32, addr5)
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, muar_idx)
		__field(u32, rc_a1)
		__field(u32, rc_a2)
		__field(u32, kid)
		__field(u32, rkv)
		__field(u32, rv)
		__field(u8, sw)
		__field(u8, wm)
		__field(u8, mm)
		__field(u8, cipher_suit)
		__field(u8, td)
		__field(u8, fd)
		__field(u8, dis_rhtr)
		__field(u8, af)
		__field(u8, rx_ps)
		__field(u8, r)
		__field(u8, rts)
		__field(u8, cf_ack)
		__field(u8, rdg_ba)
		__field(u8, smps)
		__field(u8, baf_en)
		__field(u8, ht)
		__field(u8, vht)
		__field(u8, ldpc)
		__field(u8, dyn_bw)
		__field(u8, tibf)
		__field(u8, tebf)
		__field(u8, txop_ps_cap)
		__field(u8, mesh)
		__field(u8, qos)
		__field(u8, adm)
		__field(u32, gid)
		__field(u32, wtbl2_fid)
		__field(u32, wtbl2_eid)
		__field(u32, wtbl3_fid)
		__field(u32, wtbl3_eid)
		__field(u32, wtbl4_fid)
		__field(u32, wtbl4_eid)
		__field(u8, chk_per)
		__field(u8, du_i_psm)
		__field(u8, i_psm)
		__field(u8, psm)
		__field(u8, skip_tx)
		__field(u8, partial_aid)
	),

	TP_fast_assign(
		union WTBL_1_DW0 *wtbl_1_d0 = (union WTBL_1_DW0 *)&tb->wtbl_1_d0.word;
		union WTBL_1_DW1 *wtbl_1_d1 = (union WTBL_1_DW1 *)&tb->wtbl_1_d1.word;
		union WTBL_1_DW2 *wtbl_1_d2 = (union WTBL_1_DW2 *)&tb->wtbl_1_d2.word;
		union WTBL_1_DW3 *wtbl_1_d3 = (union WTBL_1_DW3 *)&tb->wtbl_1_d3.word;
		union WTBL_1_DW4 *wtbl_1_d4 = (union WTBL_1_DW4 *)&tb->wtbl_1_d4.word;
		UINT32 Value;
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->addr0 = wtbl_1_d1->field.addr_0 & 0xff;
		__entry->addr1 = ((wtbl_1_d1->field.addr_0 & 0xff00) >> 8);
		__entry->addr2 = ((wtbl_1_d1->field.addr_0 & 0xff0000) >> 16);
		__entry->addr3 = ((wtbl_1_d1->field.addr_0 & 0xff000000) >> 24);
		__entry->addr4 = wtbl_1_d0->field.addr_4 & 0xff;
		__entry->addr5 = wtbl_1_d0->field.addr_5 & 0xff;

		__entry->muar_idx = wtbl_1_d0->field.muar_idx;
		__entry->rc_a1 = wtbl_1_d0->field.rc_a1;
		__entry->rc_a2 = wtbl_1_d0->field.rc_a2;
		__entry->kid = wtbl_1_d0->field.kid;
		__entry->rkv = wtbl_1_d0->field.rkv;
		__entry->rv = wtbl_1_d0->field.rv;
		__entry->sw = wtbl_1_d0->field.sw;
		__entry->wm = wtbl_1_d0->field.wm;
		__entry->mm = wtbl_1_d2->field.mm;
		__entry->cipher_suit = wtbl_1_d2->field.cipher_suit;
		__entry->td = wtbl_1_d2->field.td;
		__entry->fd = wtbl_1_d2->field.fd;
		__entry->dis_rhtr = wtbl_1_d2->field.dis_rhtr;
		__entry->af = wtbl_1_d2->field.af;
		__entry->rx_ps = wtbl_1_d2->field.rx_ps;
		__entry->r = wtbl_1_d2->field.r;
		__entry->rts = wtbl_1_d2->field.rts;
		__entry->cf_ack = wtbl_1_d2->field.cf_ack;
		__entry->rdg_ba = wtbl_1_d2->field.rdg_ba;
		__entry->smps = wtbl_1_d2->field.smps;
		__entry->baf_en = wtbl_1_d2->field.baf_en;
		__entry->ht = wtbl_1_d2->field.ht;
		__entry->vht = wtbl_1_d2->field.vht;
		__entry->ldpc = wtbl_1_d2->field.ldpc;
		__entry->dyn_bw = wtbl_1_d2->field.dyn_bw;
		__entry->tibf = wtbl_1_d2->field.tibf;
		__entry->tebf = wtbl_1_d2->field.tebf;
		__entry->txop_ps_cap = wtbl_1_d2->field.txop_ps_cap;
		__entry->mesh = wtbl_1_d2->field.mesh;
		__entry->qos = wtbl_1_d2->field.qos;
		__entry->adm = wtbl_1_d2->field.adm;
		__entry->gid = wtbl_1_d2->field.gid;
		__entry->wtbl2_fid = wtbl_1_d3->field.wtbl2_fid;
		__entry->wtbl2_eid = wtbl_1_d3->field.wtbl2_eid;
		__entry->wtbl3_fid = wtbl_1_d4->field.wtbl3_fid;
		__entry->wtbl3_eid = wtbl_1_d4->field.wtbl3_eid;
		__entry->wtbl4_fid = wtbl_1_d3->field.wtbl4_fid;
		__entry->wtbl4_eid = wtbl_1_d4->field.wtbl4_eid;
		__entry->chk_per = wtbl_1_d3->field.chk_per;
		__entry->du_i_psm = wtbl_1_d3->field.du_i_psm;
		__entry->i_psm = wtbl_1_d3->field.i_psm;
		__entry->psm = wtbl_1_d3->field.psm;
		__entry->skip_tx = wtbl_1_d3->field.skip_tx;
		__entry->partial_aid = wtbl_1_d4->field.partial_aid;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   WTBL Segment 1 Fields:\n\
			   mac addr: %02X:%02X:%02X:%02X:%02X:%02X\n\
			   MUAR_Idx:%d\n\
			   rc_a1/rc_a2:%d/%d\n\
			   kid:%d\n\
			   rkv/rv:%d/%d\n\
			   sw:%d\n\
			   wm/mm:%d/%d\n\
			   cipher_suit:%d\n\
			   td/fd:%d/%d\n\
			   dis_rhtr:%d\n\
			   af:%d\n\
			   rx_ps:%d\n\
			   r:%d\n\
			   rts:%d\n\
			   cf_ack:%d\n\
			   rdg_ba:%d\n\
			   smps:%d\n\
			   baf_en:%d\n\
			   ht/vht/ldpc/dyn_bw:%d/%d/%d/%d\n\
			   TxBF(tibf/tebf):%d / %d\n\
			   txop_ps_cap:%d\n\
			   mesh:%d\n\
			   qos:%d\n\
			   adm:%d\n\
			   gid:%d\n\
			   wtbl2_fid:%d\n\
			   wtbl2_eid:%d\n\
			   wtbl3_fid:%d\n\
			   wtbl3_eid:%d\n\
			   wtbl4_fid:%d\n\
			   wtbl4_eid:%d\n\
			   chk_per:%d\n\
			   du_i_psm:%d\n\
			   i_psm:%d\n\
			   psm:%d\n\
			   skip_tx:%d\n\
			   partial_aid:%d\n",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__entry->addr0,
				__entry->addr1,
				__entry->addr2,
				__entry->addr3,
				__entry->addr4,
				__entry->addr5,
				__entry->muar_idx,
				__entry->rc_a1, __entry->rc_a2,
				__entry->kid,
				__entry->rkv, __entry->rv,
				__entry->sw,
				__entry->wm, __entry->mm,
				__entry->cipher_suit,
				__entry->td, __entry->fd,
				__entry->dis_rhtr,
				__entry->af,
				__entry->rx_ps,
				__entry->r,
				__entry->rts,
				__entry->cf_ack,
				__entry->rdg_ba,
				__entry->smps,
				__entry->baf_en,
				__entry->ht, __entry->vht, __entry->ldpc, __entry->dyn_bw,
				__entry->tibf, __entry->tebf,
				__entry->txop_ps_cap,
				__entry->mesh,
				__entry->qos,
				__entry->adm,
				__entry->gid,
				__entry->wtbl2_fid,
				__entry->wtbl2_eid,
				__entry->wtbl3_fid,
				__entry->wtbl3_eid,
				__entry->wtbl4_fid,
				__entry->wtbl4_eid,
				__entry->chk_per,
				__entry->du_i_psm,
				__entry->i_psm,
				__entry->psm,
				__entry->skip_tx,
				__entry->partial_aid
	)
);


#define TRACE_CR_WTBL2_INFO trace_cr_wtbl2_info
TRACE_EVENT(cr_wtbl2_info,
	TP_PROTO(RTMP_ADAPTER * pAd, struct wtbl_2_struc *tb
	),

	TP_ARGS(pAd, tb
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, pn_0)
		__field(u32, pn_32)
		__field(u32, com_sn)
		__field(u32, tid_ac_0_sn)
		__field(u32, tid_ac_1_sn)
		__field(u32, tid_ac_2_sn)
		__field(u32, tid_ac_3_sn)
		__field(u32, tid_ac_4_sn)
		__field(u32, tid_ac_5_sn)
		__field(u32, tid_ac_6_sn)
		__field(u32, tid_ac_7_sn)
		__field(u32, rate_1_tx_cnt)
		__field(u32, rate_1_fail_cnt)
		__field(u32, rate_2_tx_cnt)
		__field(u32, rate_3_tx_cnt)
		__field(u32, rate_4_tx_cnt)
		__field(u32, rate_5_tx_cnt)
		__field(u32, current_bw_tx_cnt)
		__field(u32, current_bw_fail_cnt)
		__field(u32, other_bw_tx_cnt)
		__field(u32, other_bw_fail_cnt)
		__field(u32, fcap)
		__field(u32, rate_idx)
		__field(u32, cbrn)
		__field(u32, ccbw_sel)
		__field(u32, spe_en)
		__field(u32, mpdu_fail_cnt)
		__field(u32, mpdu_ok_cnt)
		__field(u32, g2)
		__field(u32, g4)
		__field(u32, g8)
		__field(u32, g16)
		__field(u32, rate_info_0)
		__field(u32, rate_info_1)
		__field(u32, rate_info_2)
		__field(u32, resp_rcpi_0)
		__field(u32, resp_rcpi_1)
		__field(u32, resp_rcpi_2)
		__field(u32, sts_1_ch_cap_noise)
		__field(u32, sts_2_ch_cap_noise)
		__field(u32, sts_3_ch_cap_noise)
		__field(u32, ce_rmsd)
		__field(u32, cc_noise_sel)
		__field(u32, ant_sel)
		__field(u32, ba_en)
		__field(u32, ba_size)
	),

	TP_fast_assign(
		union WTBL_2_DW0 *dw_0 = &tb->wtbl_2_d0;
		union WTBL_2_DW1 *dw_1 = &tb->wtbl_2_d1;
		union WTBL_2_DW2 *dw_2 = &tb->wtbl_2_d2;
		union WTBL_2_DW3 *dw_3 = &tb->wtbl_2_d3;
		union WTBL_2_DW4 *dw_4 = &tb->wtbl_2_d4;
		union WTBL_2_DW5 *dw_5 = &tb->wtbl_2_d5;
		union WTBL_2_DW6 *dw_6 = &tb->wtbl_2_d6;
		union WTBL_2_DW7 *dw_7 = &tb->wtbl_2_d7;
		union WTBL_2_DW8 *dw_8 = &tb->wtbl_2_d8;
		union WTBL_2_DW9 *dw_9 = &tb->wtbl_2_d9;
		union WTBL_2_DW10 *dw_10 = &tb->wtbl_2_d10;
		union WTBL_2_DW11 *dw_11 = &tb->wtbl_2_d11;
		union WTBL_2_DW12 *dw_12 = &tb->wtbl_2_d12;
		union WTBL_2_DW13 *dw_13 = &tb->wtbl_2_d13;
		union WTBL_2_DW14 *dw_14 = &tb->wtbl_2_d14;
		union WTBL_2_DW15 *dw_15 = &tb->wtbl_2_d15;
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;
		__entry->pn_0 = dw_0->pn_0;
		__entry->pn_32 = dw_1->field.pn_32;
		__entry->com_sn = dw_1->field.com_sn;
		__entry->tid_ac_0_sn = dw_2->field.tid_ac_0_sn;
		__entry->tid_ac_1_sn = dw_2->field.tid_ac_1_sn;
		__entry->tid_ac_2_sn = dw_2->field.tid_ac_2_sn_0 | (dw_3->field.tid_ac_2_sn_9 << 9);
		__entry->tid_ac_3_sn = dw_3->field.tid_ac_3_sn;
		__entry->tid_ac_4_sn = dw_3->field.tid_4_sn;
		__entry->tid_ac_5_sn = dw_3->field.tid_5_sn_0 | (dw_4->field.tid_5_sn_5 << 5);
		__entry->tid_ac_6_sn = dw_4->field.tid_6_sn;
		__entry->tid_ac_7_sn = dw_4->field.tid_7_sn;
		__entry->rate_1_tx_cnt = dw_5->field.rate_1_tx_cnt;
		__entry->rate_1_fail_cnt = dw_5->field.rate_1_fail_cnt;
		__entry->rate_2_tx_cnt = dw_6->field.rate_2_tx_cnt;
		__entry->rate_3_tx_cnt = dw_6->field.rate_3_tx_cnt;
		__entry->rate_4_tx_cnt = dw_6->field.rate_4_tx_cnt;
		__entry->rate_5_tx_cnt = dw_6->field.rate_5_tx_cnt;
		__entry->current_bw_tx_cnt = dw_7->field.current_bw_tx_cnt;
		__entry->current_bw_fail_cnt = dw_7->field.current_bw_fail_cnt;
		__entry->other_bw_tx_cnt = dw_8->field.other_bw_tx_cnt;
		__entry->other_bw_fail_cnt = dw_8->field.other_bw_fail_cnt;
		__entry->fcap = dw_9->field.fcap;
		__entry->rate_idx = dw_9->field.rate_idx;
		__entry->cbrn = dw_9->field.cbrn;
		__entry->ccbw_sel = dw_9->field.ccbw_sel;
		__entry->spe_en = dw_9->field.spe_en;
		__entry->mpdu_fail_cnt = dw_9->field.mpdu_fail_cnt;
		__entry->mpdu_ok_cnt = dw_9->field.mpdu_ok_cnt;
		__entry->g2 = dw_9->field.g2;
		__entry->g4 = dw_9->field.g4;
		__entry->g8 = dw_9->field.g8;
		__entry->g16 = dw_9->field.g16;
		__entry->rate_info_0 = dw_10->word;
		__entry->rate_info_1 = dw_11->word;
		__entry->rate_info_2 = dw_12->word;
		__entry->resp_rcpi_0 = dw_13->field.resp_rcpi_0;
		__entry->resp_rcpi_1 = dw_13->field.resp_rcpi_1;
		__entry->resp_rcpi_2 = dw_13->field.resp_rcpi_2;
		__entry->sts_1_ch_cap_noise = dw_14->field.sts_1_ch_cap_noise;
		__entry->sts_2_ch_cap_noise = dw_14->field.sts_2_ch_cap_noise;
		__entry->sts_3_ch_cap_noise = dw_14->field.sts_3_ch_cap_noise;
		__entry->ce_rmsd = dw_14->field.ce_rmsd;
		__entry->cc_noise_sel = dw_14->field.cc_noise_sel;
		__entry->ant_sel = dw_14->field.ant_sel;
		__entry->ba_en = dw_15->field.ba_en;
		__entry->ba_size = dw_15->field.ba_win_size_tid;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   WTBL Segment 2 Fields:\n\
			   PN_0-31:0x%x\n\
			   PN_32-48:0x%x\n\
			   SN(NonQos/Mgmt Frame):%d\n\
			   SN(TID0~7 QoS Frame):%d - %d - %d - %d - %d - %d - %d - %d\n\
			   TxRateCnt(1-5):%d(%d) - %d - %d - %d - %d\n\
			   TxBwCnt(Current-Other):%d(%d) - %d(%d)\n\
			   FreqCap:%d\n\
			   RateIdx/CBRN/CCBW_SEL/SPE_EN: %d/%d/%d/%d\n\
			   MpduCnt(Fail/OK):%d-%d\n\
			   TxRate Info: G2/G4/G8/G16=%d/%d/%d/%d\n\
			   TxRate Info: %d/%d/%d\n\
			   Resp_RCPI0/Resp_RCPI1/Resp_RCPI2=0x%x/0x%x/0x%x\n\
			   1CC(Noise)/2CC(Noise)/3CC(Noise)/CE_RMSD/CC_Sel/Ant_Sel=0x%x/0x%x/0x%x/0x%x/%d/%d\n\
			   BA Info: BA_En/BAWinSizeIdx(Range)\n\
			   %d/%d\n",
			  __entry->year,
			  __entry->mon,
			  __entry->day,
			  __entry->hour,
			  __entry->min,
			__entry->sec,
			  __entry->pn_0,
			  __entry->pn_32,
			  __entry->com_sn,
			  __entry->tid_ac_0_sn,
			  __entry->tid_ac_1_sn,
			  __entry->tid_ac_2_sn,
			  __entry->tid_ac_3_sn,
			  __entry->tid_ac_4_sn,
			  __entry->tid_ac_5_sn,
			  __entry->tid_ac_6_sn,
			  __entry->tid_ac_7_sn,
			  __entry->rate_1_tx_cnt,
			  __entry->rate_1_fail_cnt,
			  __entry->rate_2_tx_cnt,
			  __entry->rate_3_tx_cnt,
			  __entry->rate_4_tx_cnt,
			  __entry->rate_5_tx_cnt,
			  __entry->current_bw_tx_cnt,
			  __entry->current_bw_fail_cnt,
			  __entry->other_bw_tx_cnt,
			  __entry->other_bw_fail_cnt,
			  __entry->fcap,
			  __entry->rate_idx,
			  __entry->cbrn,
			  __entry->ccbw_sel,
			  __entry->spe_en,
			  __entry->mpdu_fail_cnt,
			  __entry->mpdu_ok_cnt,
			  __entry->g2,
			  __entry->g4,
			  __entry->g8,
			  __entry->g16,
			  __entry->rate_info_0,
			  __entry->rate_info_1,
			  __entry->rate_info_2,
			  __entry->resp_rcpi_0,
			  __entry->resp_rcpi_1,
			  __entry->resp_rcpi_2,
			  __entry->sts_1_ch_cap_noise,
			  __entry->sts_2_ch_cap_noise,
			  __entry->sts_3_ch_cap_noise,
			  __entry->ce_rmsd,
			  __entry->cc_noise_sel,
			  __entry->ant_sel,
			  __entry->ba_en,
			  __entry->ba_size
	)
);


#define TRACE_CR_TR_INFO trace_cr_tr_info
TRACE_EVENT(cr_tr_info,
	TP_PROTO(CHAR * function, UINT32 ring_reg, UINT32 base, UINT32 cnt, UINT32 cidx, UINT32 didx,
			 UINT32 swidx
	),

	TP_ARGS(function, ring_reg, base, cnt, cidx, didx, swidx
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
		__field(u32, ring_reg)
		__field(u32, base)
		__field(u32, cnt)
		__field(u32, cidx)
		__field(u32, didx)
		__field(u32, swidx)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;
		__assign_str(func, function);
		__entry->ring_reg = ring_reg;
		__entry->base = base;
		__entry->cnt = cnt;
		__entry->cidx = cidx;
		__entry->didx = didx;
		__entry->swidx = swidx;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   Ring Name = %s\n\
			   Ring Reg = 0x%04x\n\
			   Base = 0x%08x\n\
			   Cnt = 0x%x\n\
			   CIDX = 0x%x\n\
			   DIDX = 0x%x\n\
			   SWIdx = 0x%x",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__get_str(func),
				__entry->ring_reg,
				__entry->base,
				__entry->cnt,
				__entry->cidx,
				__entry->didx,
				__entry->swidx
	)
);


#define TRACE_CR_INTERRUPT_INFO trace_cr_interrupt_info
TRACE_EVENT(cr_interrupt_info,
	TP_PROTO(UINT32 int_csr, UINT32 int_mask, UINT32 delay_int, UINT32 dma_conf, UINT32 tx_dma_en,
				UINT32 rx_dma_en, UINT32 tx_dma_busy, UINT32 rx_dma_busy
	),

	TP_ARGS(int_csr, int_mask, delay_int, dma_conf, tx_dma_en, rx_dma_en, tx_dma_busy, rx_dma_busy
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, int_csr)
		__field(u32, int_mask)
		__field(u32, delay_int)
		__field(u32, dma_conf)
		__field(u32, tx_dma_en)
		__field(u32, rx_dma_en)
		__field(u32, tx_dma_busy)
		__field(u32, rx_dma_busy)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->int_csr = int_csr;
		__entry->int_mask = int_mask;
		__entry->delay_int = delay_int;
		__entry->dma_conf = dma_conf;
		__entry->tx_dma_en = tx_dma_en;
		__entry->rx_dma_en = rx_dma_en;
		__entry->tx_dma_busy = tx_dma_busy;
		__entry->rx_dma_busy = rx_dma_busy;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   IntCSR = 0x%x\n\
			   IntMask = 0x%x\n\
			   DelayINT = 0x%x\n\
			   DMA Configuration = 0x%x\n\
			   Tx/RxDMAEn = %d %d\n\
			   Tx/RxDMABusy = %d %d",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__entry->int_csr,
				__entry->int_mask,
				__entry->delay_int,
				__entry->dma_conf,
				__entry->tx_dma_en,
				__entry->rx_dma_en,
				__entry->tx_dma_busy,
				__entry->rx_dma_busy
	)
);




#ifdef RTMP_PCI_SUPPORT
#define TRACE_PCI_TX_RING_IDX trace_pci_tx_ring_idx
TRACE_EVENT(pci_tx_ring_idx,
	TP_PROTO(CHAR * function, UINT32 queue_idx, UINT32 reg, UINT32 cpu_idx, UINT32 dma_idx,
				UINT32 sw_free_idx
	),

	TP_ARGS(function, queue_idx, reg, cpu_idx, dma_idx, sw_free_idx
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__string(func, function)
		__field(u32, queue_idx)
		__field(u32, reg)
		__field(u32, cpu_idx)
		__field(u32, dma_idx)
		__field(u32, sw_free_idx)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


		__assign_str(func, function);
		__entry->queue_idx = queue_idx;
		__entry->reg = reg;
		__entry->cpu_idx = cpu_idx;
		__entry->dma_idx = dma_idx;
		__entry->sw_free_idx = sw_free_idx;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   function = %s\n\
			   queue_idx = 0x%x\n\
			   reg = 0x%x\n\
			   cpu_idx = 0x%x\n\
			   dma_idx = 0x%x\n\
			   sw_free_idx = 0x%x",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__get_str(func),
				__entry->queue_idx,
				__entry->reg,
				__entry->cpu_idx,
				__entry->dma_idx,
				__entry->sw_free_idx
	)
);
#endif


#define TRACE_PS_RETRIEVE_PACKET trace_ps_retrieve_packet
TRACE_EVENT(ps_retrieve_packet,
	TP_PROTO(UINT8 stage, UINT32 wlan_idx, UINT32 hdr_info, UINT16 padding, UINT32 entry_type, UINT16 ps_state, UINT8 qos_0,
			UINT8 qos_1, INT32 ps_qbitmap, UINT32 ps_queue_number, UINT16 token_count
	),

	TP_ARGS(stage, wlan_idx, hdr_info, padding, entry_type, ps_state, qos_0, qos_1, ps_qbitmap, ps_queue_number, token_count
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u8, stage)
		__field(u32, wlan_idx)
		__field(u32, hdr_info)
		__field(u16, padding)
		__field(u32, entry_type)
		__field(u16, ps_state)
		__field(u8, qos_0)
		__field(u8, qos_1)
		__field(s32, ps_qbitmap)
		__field(u32, ps_queue_number)
		__field(u32, token_count)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


		__entry->stage = stage;
		__entry->wlan_idx = wlan_idx;
		__entry->hdr_info = hdr_info;
		__entry->padding = padding;
		__entry->entry_type = entry_type;
		__entry->ps_state = ps_state;
		__entry->qos_0 = qos_0;
		__entry->qos_1 = qos_1;
		__entry->ps_qbitmap = ps_qbitmap;
		__entry->ps_queue_number = ps_queue_number;
		__entry->token_count = token_count;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   stage = %d\n\
			   wlan_idx = %d\n\
			   hdr_info = %d\n\
			   padding = %d\n\
			   entry_type = %d\n\
			   ps_state = %d\n\
			   qos_0 = %d\n\
			   qos_1 = %d\n\
			   ps_qbitmap = %d\n\
			   ps_queue_number = %d\n\
			   token_count = %d",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__entry->stage,
			   __entry->wlan_idx,
			   __entry->hdr_info,
			   __entry->padding,
			   __entry->entry_type,
			   __entry->ps_state,
			   __entry->qos_0,
			   __entry->qos_1,
			   __entry->ps_qbitmap,
			   __entry->ps_queue_number,
			   __entry->token_count
	)
);

#define TRACE_PS_INDICATE trace_ps_indicate
TRACE_EVENT(ps_indicate,
	TP_PROTO(UINT8 Stage, UINT32 wcid, UINT8 old_psm, UINT8 new_psm, UINT8 tr_entry_ps_state
	),

	TP_ARGS(Stage, wcid, old_psm, new_psm, tr_entry_ps_state
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u8, Stage)
		__field(u32, wcid)
		__field(u8, old_psm)
		__field(u8, new_psm)
		__field(u8, tr_entry_ps_state)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


		__entry->Stage = Stage;
		__entry->wcid = wcid;
		__entry->old_psm = old_psm;
		__entry->new_psm = new_psm;
		__entry->tr_entry_ps_state = tr_entry_ps_state;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   Stage = %d\n\
			   wcid = %d\n\
			   old_psm = %d\n\
			   new_state = %d\n\
			   tr_entry_ps_state = %d",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__entry->Stage,
				__entry->wcid,
				__entry->old_psm,
				__entry->new_psm,
				__entry->tr_entry_ps_state
	)
)

#define TRACE_PS_HANDLE_RX_PS_POLL trace_ps_handle_rx_ps_poll
TRACE_EVENT(ps_handle_rx_ps_poll,
	TP_PROTO(UINT8 Stage, UINT32 wcid, UINT8 is_active, UINT8 ps_state
	),

	TP_ARGS(Stage, wcid, is_active, ps_state

	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u8, Stage)
		__field(u32, wcid)
		__field(u8, is_active)
		__field(u8, ps_state)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;


		__entry->Stage = Stage;
		__entry->wcid = wcid;
		__entry->is_active;
		__entry->ps_state = ps_state;
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
			   Stage = %d\n\
			   wcid = %d\n\
			   is_active = %d\n\
			   ps_state = %d",
				__entry->year,
				__entry->mon,
				__entry->day,
				__entry->hour,
				__entry->min,
				__entry->sec,
				__entry->Stage,
				__entry->wcid,
				__entry->is_active,
				__entry->ps_state
	)
)


#define TRACE_PS_INFO trace_ps_info
TRACE_EVENT(ps_info,
	TP_PROTO(UINT32 addr0,
			 UINT32 addr1,
			 UINT32 addr2,
			 UINT32 addr3,
			 UINT32 addr4,
			 UINT32 addr5,
			 UINT32 entry_type,
			 UINT32 aid,
			 UINT32 func_tb_idx,
			 UINT32 ps_mode,
			 UINT32 ps_state,
			 UINT32 i_psm,
			 UINT32 du_i_psm,
			 UINT32 skip_tx,
			 UINT32 pfg_force,
			 UINT32 pucport,
			 UINT32 pucqueue,
			 UINT32 total_pkt_number,
			 UINT32 ps_queue_number
	),

	TP_ARGS(addr0,
			addr1,
			addr2,
			addr3,
			addr4,
			addr5,
			entry_type,
			aid,
			func_tb_idx,
			ps_mode,
			ps_state,
			i_psm,
			du_i_psm,
			skip_tx,
			pfg_force,
			pucport,
			pucqueue,
			total_pkt_number,
			ps_queue_number
	),

	TP_STRUCT__entry(
		__field(u32, year)
		__field(u32, mon)
		__field(u32, day)
		__field(u32, hour)
		__field(u32, min)
		__field(u32, sec)
		__field(u32, addr0)
		__field(u32, addr1)
		__field(u32, addr2)
		__field(u32, addr3)
		__field(u32, addr4)
		__field(u32, addr5)
		__field(u32, entry_type)
		__field(u32, aid)
		__field(u32, func_tb_idx)
		__field(u32, ps_mode)
		__field(u32, ps_state)
		__field(u32, i_psm)
		__field(u32, du_i_psm)
		__field(u32, skip_tx)
		__field(u32, pfg_force)
		__field(u32, pucport)
		__field(u32, pucqueue)
		__field(u32, total_pkt_number)
		__field(u32, ps_queue_number)
	),

	TP_fast_assign(
		struct timeval time;
		unsigned long local_time;
		struct rtc_time tm;

		do_gettimeofday(&time);
		local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
		rtc_time_to_tm(local_time, &tm);

		__entry->year = tm.tm_year + 1900;
		__entry->mon = tm.tm_mon + 1;
		__entry->day = tm.tm_mday;
		__entry->hour = tm.tm_hour;
		__entry->min = tm.tm_min;
		__entry->sec = tm.tm_sec;

		__entry->addr0 = addr0;
		__entry->addr1 = addr1;
		__entry->addr2 = addr2;
		__entry->addr3 = addr3;
		__entry->addr4 = addr4;
		__entry->addr5 = addr5;
		__entry->entry_type = entry_type;
		__entry->aid = aid;
		__entry->func_tb_idx = func_tb_idx;
		__entry->ps_mode = ps_mode;
		__entry->ps_state = ps_state;
		__entry->i_psm = i_psm;
		__entry->du_i_psm = du_i_psm;
		__entry->skip_tx = skip_tx;
		__entry->pfg_force = pfg_force;
		__entry->pucport = pucport;
		__entry->pucqueue = pucqueue;
		__entry->total_pkt_number = total_pkt_number;
		__entry->ps_queue_number = ps_queue_number
	),

	TP_printk("System Time = %04d-%02d-%02d %02d:%02d:%02d)\n\
		%02X:%02X:%02X:%02X:%02X:%02X\n\
		EntryType = %10x\n\
		Aid = %5d\n\
		func_tb_idx = %5d\n\
		PsMode = %5d\n\
		ps_state = %5d\n\
		i_psm = %5d\n\
		du_i_psm = %5d\n\
		skip_tx = %5d\n\
		pfgForce = %5d\n\
		pucPort = %5d\n\
		pucQueue = %5d\n\
		Total_Packet_Number = %6d\n\
		ps_queue.Number = %6d",
		__entry->year,
		__entry->mon,
		__entry->day,
		__entry->hour,
		__entry->min,
		__entry->sec,
		__entry->addr0,
		__entry->addr1,
		__entry->addr2,
		__entry->addr3,
		__entry->addr4,
		__entry->addr5,
		__entry->entry_type,
		__entry->aid,
		__entry->func_tb_idx,
		__entry->ps_mode,
		__entry->ps_state,
		__entry->i_psm,
		__entry->du_i_psm,
		__entry->skip_tx,
		__entry->pfg_force,
		__entry->pucport,
		__entry->pucqueue,
		__entry->total_pkt_number,
		__entry->ps_queue_number
	)
);

INT32 TraceTRInfo(RTMP_ADAPTER *pAd);
VOID TraceCrPseInfo(RTMP_ADAPTER *pAd);
VOID TraceWtblInfo(RTMP_ADAPTER *pAd, UINT32 wtbl_idx);
INT32 TracePSTable(RTMP_ADAPTER *pAd, UINT32 ent_type, BOOLEAN bReptCli);

#endif

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE trace
#include <trace/define_trace.h>

#endif

