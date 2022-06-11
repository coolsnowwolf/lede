/*
 * Copyright (c) 2008-2012 Quantenna Communications, Inc.
 */

/*
 * This file contains host definitions which are common between the
 * host driver and the microcontroller/MAC code.
 */

/**
 * The host tx descriptor for an ethernet packet
 */

#ifndef _MUC_TXRX_STATS_H_
#define _MUC_TXRX_STATS_H_

#include <qtn/muc_share_def.h>

#ifdef ENABLE_STATS
#define MUC_UPDATE_STATS(_a, _b)	(_a += _b)
#define MUC_SETSTAT(_a, _b)		(_a = _b)
#else
#define MUC_UPDATE_STATS(_a, _b)
#define MUC_SETSTAT(_a, _b)
#endif

/**
 * \defgroup MUCSTATS MuC generated statistics
 */
/** @{ */

/**
 * \brief MuC transmit statistics
 *
 * These statistics are generated on the MuC, mainly on the transmit datapath.
 */
struct muc_tx_stats {
	/**
	 * The number of times the software failed to enqueue a beacon to the
	 * hardware.
	 *
	 * \note If this value is non-zero it could indicate a very congested
	 * medium.
	 */
	u_int32_t	bcn_enq_failed;

	/**
	 * The number of times the TX status bit is set.
	 */
	u_int32_t	tx_status_set;

	/**
	 * The number of packets received from the LHost.
	 */
	u_int32_t	pkt_from_host;

	/**
	 * The number of interrupts from the host to indicate data is ready for
	 * transmit.
	 *
	 * \note This number will generally be quite low, as the LHost->MuC
	 * data path is poll driven rather than interrupt driven.
	 */
	u_int32_t	host_intr;
	u_int32_t	netbuf_alloc_failed;

	/**
	 * The number of management packets prior to encapsulation.
	 *
	 * \note This number should be the same as mgm_after_encap when the system
	 * is idle.
	 */
	u_int32_t	mgm_before_encap;

	/**
	 * The number of management packets after encapsulation.
	 *
	 * \note This number should be the same as mgm_before_encap when the system
	 * is idle.
	 */
	u_int32_t	mgm_after_encap;
	u_int32_t	pkt_before_encap;
	u_int32_t	pkt_after_encap;

	/**
	 * The number of packets held, waiting for BA to complete.
	 */
	u_int32_t	pkt_push_back;
	u_int32_t	tx_timeout;

	/**
	 * This counter shows the number of MPDUs or AMPDUs which needed to be retried
	 * by the hardware.
	 *
	 * For an MPDU, it indicates that no ACK was received from the peer.
	 *
	 * For an AMPDU, it indicates that no BACK was received from the peer.
	 * In this case, all subframes within the aggregate failed, and the hardware
	 * has requeued the entire aggregate for retransmission.
	 *
	 * If all hardware retries fail, the packet is returned back to the MAC for
	 * software retries.
	 *
	 * \sa tx_sw_retry.
	 */
	u_int32_t	tx_hw_retry;

	/**
	 * This counter shows the number of non-aggregate MPDUs that have been resent
	 * by software for retransmission after an initial transmission failure.
	 *
	 * Transmission failure is when no ACK has been received from the peer after the
	 * configured number of hardware retries.
	 */
	u_int32_t	tx_sw_retry_noagg;

	/**
	 * This counter shows the number of AMPDUs that have been repackaged
	 * by software for retransmission after an initial transmission failure.
	 *
	 * Transmission failure is when an AMPDU fails to receive a full block
	 * ACK (some subframes are incorrectly received).  The repackaged AMPDU
	 * contains a subset of the subframes from the original AMPDU.
	 */
	u_int32_t	tx_sw_retry;
	u_int32_t	tx_xretry;
	u_int32_t	tx_pspoll_deagg;

	/**
	 * This counter shows the number of packets (AMPDU subframes) for which all retry
	 * attempts have failed.
	 *
	 * \note This counter represents genuine packet loss.
	 */
	u_int32_t	tx_xattempts;

	/**
	 * This counter shows the number of non-aggregate MPDUs for which all retry
	 * attempts have failed.
	 *
	 * \note This counter represents genuine packet loss.
	 */
	u_int32_t	tx_xattempts_noagg;
	u_int32_t	tx_done_failed;
	u_int32_t	tx_cca_defer_cnt;
	u_int32_t	pkt_to_hw;
	u_int32_t	pkt_to_hw_deferred;
	u_int32_t	fd_absent;
	u_int32_t	fd_not_ready;
	u_int32_t	pkt_fd_available;
	u_int32_t	pkt_add_node;
	u_int32_t	pkt_add_q;
	u_int32_t	pkt_qtn_hardstart;
	u_int32_t	tx_reserved;
	u_int32_t	tx_released;
	u_int32_t	tx_reserve_fail;
	u_int32_t	tx_release_err;
	u_int32_t	tx_mu_reserved;
	u_int32_t	tx_mu_released;
	u_int32_t	tx_mu_reserve_fail;
	u_int32_t	tx_mu_release_err;
	u_int32_t	txalert_mu_ndp_update;
	u_int32_t	txalert_mu_rpt_poll;
	u_int32_t	txalert_mu_queue_full;
	u_int32_t	txalert_mu_queue_fail;
	u_int32_t	sample_rate_mu;
	u_int32_t	sample_bw_mu;
	u_int32_t	txdone_intr;
	u_int32_t	txalert_intr;
	u_int32_t	txalert_tasklet;
	u_int32_t	txalert_bcn_update;
	u_int32_t	txalert_ndp_update;
	u_int32_t	tx_ndp_q_occupied;
	u_int32_t	tx_ndp_start;
	u_int32_t	txdone_mgmt;
	u_int32_t	txdone_data;
	u_int32_t	tx_pwr;
	u_int32_t	bcn_scheme_power_save;
	u_int32_t	bcn_scheme;

	/**
	 * This counter shows the number of multicast frames sent while a
	 * client is in powersave. Multicast frames with powersave are sent
	 * after the DTIM beacon.
	 */
	u_int32_t	tx_mcast_pwr;

	/**
	 * This counter shows the number of multicast frames queued for
	 * deferred (DTIM beacon) transmission.
	 *
	 * Multicast packets are deferred until the DTIM beacon when we have
	 * at least one power save client associated.
	 */
	u_int32_t	tx_mcast_defer;
	u_int32_t	tx_mcast_defer_hwq;
	u_int32_t	tx_limit_drop;
	u_int32_t	fd_acquire;
	u_int32_t	fd_release;
	u_int32_t	fd_acq_fail;
	u_int32_t	fd_acq_fail_frms;
	u_int32_t	fd_acq_hal_fail;
	u_int32_t	fd_acq_hal_fail_frms;
	u_int32_t	ba_send;
	u_int32_t	ba_del;
	u_int32_t	fd_free_nodeclean;
	u_int32_t	msdu_expired;
	u_int32_t	last_ack_ssq;
	u_int32_t	last_sent_seq;
	u_int32_t	ampdu_subframe_failure;
	u_int32_t	ampdu_subframe_done;
	u_int32_t	tx_window_locked;
	u_int32_t	tx_window_failed;
	u_int32_t	tx_restrict_probe;
	u_int32_t	tx_restrict_mode;
	u_int32_t	tx_restrict_drop;
	u_int32_t	tx_restrict_delay;
	u_int32_t	tx_restrict_send;
	u_int32_t	tx_sample_pkts;
	u_int32_t	tx_sample_bytes;
	u_int32_t	tx_underflow;
	u_int32_t	tx_hal_enqueued;
	u_int32_t	txbf_mode;
	u_int32_t	psel_matrix;
	u_int32_t	sample_rate;
	u_int32_t	sample_bw;
	uint32_t	ra_flags;
	u_int32_t	fd_balance;
	uint32_t	invalid_delay;
	uint32_t	halt_tx;
	uint32_t	resume_tx;
	uint32_t	rfctrl_on;
	uint32_t	rfctrl_off;
	uint32_t	go_offchan;
	uint32_t	go_datachan;
	uint32_t	defer_cc;
	uint32_t	deferred_cc_done;
	uint32_t	off_chan_sample;
	uint32_t	off_chan_scan;
	uint32_t	off_chan_cac;
	uint32_t	cca_pri;
	uint32_t	cca_sec;
	uint32_t	cca_sec40;
	uint32_t	cca_busy;
	uint32_t	cca_fat;
	uint32_t	cca_intf;
	uint32_t	cca_trfc;
	/**
	 * These counter show the information of MU frames.
	 */
	uint32_t	mu_prec_snd_tx;
	uint32_t	mu_prec_snd_wait_done;
	uint32_t	mu_grp_sel_snd_tx;
	uint32_t	mu_grp_sel_snd_wait_done;

	uint32_t	oc_auctx_timeout;
	uint32_t	oc_auctx_overwrite;
	uint32_t	oc_auctx_fail;
	uint32_t	gi_cnt;			/* times GI has been set for any node */
	uint32_t	gi_ncidx;		/* last node to have GI set */
	uint32_t	gi_val;			/* SGI enabled state for this node */
	uint32_t	select_state_ncidx;	/* last node to have qn_select state set */
	uint32_t	select_state_val;	/* PPPC state for this node */
	uint32_t	pppc_scale_cnt;		/* times Tx gain scaling has been set for any node */
	uint32_t	pppc_scale_ncidx;	/* last node to have Tx gain scaling set */
	uint32_t	pppc_scale_val;		/* Tx gain scaling for this node (0 is max) */
	uint32_t	pppc_scale_last_gput;		/* The last goodput used by PPPC */
	uint32_t	pppc_scale_last_gput_idx;	/* The PPPC index of the last goodput value */
	uint32_t	pppc_scale_base_cnt;		/* times Tx gain scaling base has been set for any node */
	uint32_t	pppc_scale_base_20m;	/* Combined tx scale bases for different bf/nss cases in 20MHz */
	uint32_t	pppc_scale_base_40m;	/* Combined tx scale bases for different bf/nss cases in 40MHz */
	uint32_t	pppc_scale_base_80m;	/* Combined tx scale bases for different bf/nss cases in 80MHz */
	uint32_t	pppc_scale_base_copy;	/* combined the flags indicating the tx scale bases are copied bfoff 1ss cases */
	uint32_t	pppc_scale_overstep;	/* tx scale exceed the maximum scale indices */
	uint32_t	pppc_scale_rollback;	/* tx scale roll back because scale index over step */
	uint32_t	pppc_0_gput;		/* times pppc comparing goodput and both are zero */
	uint32_t	tx_max_power;
	uint32_t	nc_csr_read_count;	/* number of times Node Cache was read */
	uint32_t	nc_csr_write_count;	/* number of times Node Cache was written to */
	uint32_t	nc_csr_done_watermark;	/* Node cache done retries high watermark */
	uint32_t	nc_csr_watermark_count; /* Number of times read retries reached max */
	uint32_t	auc_dtim_notify;
	uint32_t	auc_ps_notify;
	uint32_t	tx_beacon_done;
	uint32_t	sfs_peer_rts;
	uint32_t	sfs_peer_rts_flags;
	uint32_t	sfs_local_rts;
	uint32_t	sfs_local_rts_flags;
	uint32_t	sfs_dyn_wmm;
	uint32_t	sfs_dyn_wmm_flags;
	uint32_t	auc_wmm_ps_notify;
	uint32_t	tx_wmm_ps_null_frames;
};

/**
 * \brief MuC receive statistics
 *
 * These statistics are generated on the MuC, mainly on the receive datapath. This set of statistics
 * also include low-level debugging facilities used internally.
 */
struct muc_rx_stats {
	/**
	 * This counter shows the number of descriptors taken from the host,
	 * 'popped' from the top of the list.
	 */
	u_int32_t	rxdesc_pop_from_host;

	/**
	 * This counter shows the number of descriptors pushed to the hardware
	 * for receive buffers.
	 */
	u_int32_t	rxdesc_push_to_hw;
	u_int32_t	rxdesc_get_from_queue;
	u_int32_t	rxdesc_push_to_host;
	u_int32_t	rxdesc_non_aggr_push_to_host;
	u_int32_t	rxdesc_flush_to_host;
	u_int32_t	rxdesc_reuse_push;
	u_int32_t	rxdesc_reuse_pop;

	/**
	 * This counter shows the number of packets received with a bad duration.
	 * A bad duration is where the duration field is all 1's - that is,
	 * a packet which violates the 802.11 standard.
	 */
	u_int32_t	rxdesc_status_bad_dur;
	u_int32_t	rxdesc_status_bad_len;
	u_int32_t	rxdesc_slow_status;
	u_int32_t	rxdesc_fast_status;
	u_int32_t	rxdesc_status_crc_err;
	u_int32_t	rxdesc_status_cmic_err;
	u_int32_t	rxdesc_status_cmic_no_crc_err;
	u_int32_t	rxdesc_status_retry;
	u_int32_t	agg_stored;
	u_int32_t	agg_duplicate;

	u_int32_t	accel_mpdu;
	u_int32_t	accel_msdu;
	u_int32_t	accel_no_buffer;
	u_int32_t	accel_fwt_lu_timeout;
	u_int32_t	accel_fwt_false_miss;
	u_int32_t	accel_mcast_send;
	u_int32_t	accel_mcast_drop;
	u_int32_t	accel_no_match;
	u_int32_t	accel_drop;
	u_int32_t	accel_err;

	u_int32_t	rate_train_chk;
	u_int32_t	rate_train_err;
	u_int32_t	rate_train_delay;
	u_int32_t	rate_train_none;
	u_int32_t	rate_train_hash_bad;
	u_int32_t	rate_train_hash_good;

	/**
	 * This counter shows the number of MPDUs within an AMPDU that have been
	 * discarded due to the sequence number being outside ('below') the current
	 * receive sequence window.
	 */
	u_int32_t	agg_oldpkts;

	/**
	 * This counter shows the number of MPDUs within an AMPDU that have been
	 * discarded due to the sequence number being off by > 2047 (half the sequence
	 * space).
	 */
	u_int32_t	agg_very_oldpkts;
	u_int32_t	agg_evict_in_order;
	u_int32_t	agg_evict_in_move;

	/**
	 * This counter shows the number of received subframes within the
	 * receive window that are missing when the window is moved.
	 *
	 * This counter represents one source receive packet loss.
	 */
	u_int32_t	agg_evict_empty;

	/**
	 * This counter shows the number of received subframes within the
	 * receive window that are evicted due to timeout. Timeout is used
	 * to ensure we don't sit with a stuck receive aggregate window when
	 * the transmitter has stopped re-transmitting a given subframe.
	 */
	u_int32_t	agg_timeout;
	u_int32_t	agg_rxwin_reset;
	u_int32_t	rx_qnum_err;
	u_int32_t	rx_mgmt;
	u_int32_t	rx_ctrl;
	u_int32_t	rx_pspoll;
	u_int32_t	rx_pwr_mgmt;
	u_int32_t	rx_delba;
	/**
	 * This counter shows the number of times the powersave bit is set
	 * in the frame control field of packets received.
	 *
	 * \note This counter will generally be one greater than rx_pwr_mgmt_reset
	 * when we have a single PS client associated and in power save.
	 *
	 * \sa rx_pwr_mgmt_reset
	 */
	u_int32_t	rx_pwr_mgmt_set;

	/**
	 * This counter shows the number of times the powersave bit of a
	 * currently power save client is reset.
	 *
	 * \note This counter will generally be one less than rx_pwr_mgmt_set
	 * when we have a single PS client associated and in power save mode.
	 *
	 * \sa rx_pwr_mgmt_set
	 */
	u_int32_t	rx_pwr_mgmt_reset;

	/**
	 * \internal
	 *
	 * We have 2-stage process of pushing rx descriptors to the MAC:
	 *
	 * 1) On tasklet level we prepare descriptors and save these prepared
	 * descriptors into intermediate buffer "rxdesc_cache"
	 *
	 * 2) In RX ISR context we get buffer from "rxdesc_cache" and push it
	 * to hardware. Same procedure can be sometimes called within tasklet
	 * level too. It is because if we failed (e.g. no free descriptors) to
	 * push descriptor to hw in ISR context, likely we would not receive any
	 * RX interrupts anymore, so tasklet scheduled which would reschedule
	 * itself until succeed.
	 *
	 * rx_emergency counter incremented if (2) replenishing procedure found
	 * that hw rx queue have slot for descriptors and fail to retrieve
	 * descriptor from "rxdesc_cache" to replenish hw queue.
	 *
	 * Spike of rx_emergency does not look good.
	 *
	 * It's like tasklet was scheduled to replenish hw queue and fail it,
	 * reschedule itself and fail again and again.
	 *
	 * Generally, rx_emergency increasing is legitimate case, for example
	 * if the system is overloaded by incoming traffic. However, it also can
	 * be a sign of something bad, like processing of rx frames is stuck somewhere.
	 */
	u_int32_t	rx_emergency;
	u_int32_t	rx_underflow;
	u_int32_t	rx_desc_underflow;
	u_int32_t	rx_desc_linkerr;
	u_int32_t	rx_notify;
	u_int32_t	rx_df_numelems;
	u_int32_t	last_recv_seq;

	/**
	 * This counter shows the number of packets received for an unknown
	 * node - that is - one which we do not have an association with.
	 */
	u_int32_t	rx_node_not_found;

	/**
	 * This counter shows the number of duplicates of non-QoS packets we
	 * received and discarded.
	 */
	u_int32_t	rx_non_qos_duplicate;

	/**
	 * This counter shows the number of received NDPs.
	 */
	u_int32_t	rx_11n_ndp;
	u_int32_t	rx_11ac_ndp;
	u_int32_t	rx_ndp_inv_slot;
	u_int32_t	rx_11n_ndp_no_capt;
	u_int32_t	rx_ndp_sw_processed;
	u_int32_t	rx_ndp_lockup;
	u_int32_t	rx_11n_bf_act;
	u_int32_t	rx_11ac_bf_act;
	u_int32_t	rx_bf_act_inv_slot;

	/**
	 * This counter shows the number of received AMSDUs. This counter does
	 * not count the number of subframes within the AMSDU.
	 */
	u_int32_t	rx_amsdu;
	u_int32_t	rx_data;
	u_int32_t	prev_rx_data;
	u_int32_t	rx_recv_qnull;
	u_int32_t	rx_recv_act;
	u_int32_t	rx_recv_bcn;
	u_int32_t	rx_recv_auth;
	u_int32_t	rx_recv_assoc_req;
	u_int32_t	rx_recv_assoc_res;
	u_int32_t	rx_recv_deauth;
	u_int32_t	rx_recv_disassoc;

	/**
	 * This counter shows the number of packets received where the MCS as
	 * indicated in the PLCP is invalid (> 76).
	 */
	u_int32_t	rx_mcs_gt_76;
	u_int32_t	tkip_keys;		/* Keep count of TKIP keys installed - for debug */
	u_int32_t	rx_tkip_mic_err;	/* Number of TKIP packets RX with MIC error - the number reported to the higher layers */
	u_int32_t	icv_errs; /* The number of raw ICV errors reported by the hardware */
	u_int32_t	tmic_errs; /* The number of raw TMIC errors reported by the hardware */
	u_int32_t	cmic_errs;
	u_int32_t	crc_errs;

	/**
	 * This counter shows the number of transmit block ACK agreements
	 * installed.
	 *
	 * If the upper bit is set, at least one implicit block ACK has been
	 * established with a Quantenna peer.
	 *
	 * \note This number only increments - when block ACK agreements are
	 * removed, this counter does not decrement.
	 */
	u_int32_t	ba_tx;

	/**
	 * This counter shows the number of receive block ACK agreements
	 * installed.
	 *
	 * If the upper bit is set, at least one implicit block ACK has been
	 * established with a Quantenna peer.
	 *
	 * \note This number only increments - when block ACK agreements are
	 * removed, this counter does not decrement.
	 */
	u_int32_t	ba_rx;

	/**
	 * The number of times a block ACK has been rejected due to an out of
	 * resource situation.
	 */
	u_int32_t	ba_rx_fail;
	u_int32_t	sec_oflow;
	u_int32_t	str_oflow;
	u_int32_t	oflow_fixup_timeout;
	u_int32_t	rxdone_intr;
	u_int32_t	rxtypedone_intr;
	u_int32_t	ipc_a2m_intr;
	u_int32_t	tqe_intr;
	u_int32_t	tqe_in_port_lhost;
	u_int32_t	tqe_in_port_bad;
	u_int32_t	tqe_a2m_type_txfb;
	u_int32_t	tqe_a2m_type_rxpkt;
	u_int32_t	tqe_a2m_type_unknown;
	u_int32_t	tqe_reschedule_task;
	u_int32_t	tqe_desc_unowned;

	/**
	 * \internal
	 *
	 * The number of interrupts from the baseband to the MuC.
	 *
	 * \note This should not be distributed externally - the following
	 * fields are for internal debugging ONLY.
	 */
	u_int32_t	bb_intr;

	/**
	 * \internal
	 *
	 * The number of DLEAF overflow interrupts from the baseband.
	 */
	u_int32_t	bb_irq_dleaf_oflow;
	u_int32_t	bb_irq_leaf_uflow;
	u_int32_t	bb_irq_leaf_ldpc_uflow;
	u_int32_t	bb_irq_tx_td_oflow_intr;
	u_int32_t	bb_irq_tx_td_uflow_intr;
	u_int32_t	bb_irq_rx_sm_wdg_intr;
	/* BB spends more than 6.8ms (short GI)/7.55ms (long GI) to receive one packet */
	u_int32_t	bb_irq_rx_long_dur;
        /* BB spends more than 5.4ms (standard defined limit) to receive one 11ac packet. */
	u_int32_t	bb_irq_rx_11ac_timeout;
	u_int32_t	bb_irq_tx_sm_wdg_intr;

	/**
	 * \internal
	 *
	 * The number of BB state machine watchdogs that have kicked in.
	 */
	u_int32_t	bb_irq_main_sm_wdg_intr;
	u_int32_t	bb_irq_hready_wdg_intr;
	u_int32_t	mac_irq_rx_sec_buff_oflow;
	u_int32_t	mac_irq_rx_strq_oflow;
	u_int32_t	mac_irq_rx_bb_uflow_intr;
	u_int32_t	mac_irq_rx_bb_oflow_intr;
	u_int32_t	bb_irq_hready_wdg_reset;

	/**
	 * \internal
	 *
	 * This counter is incremented once at the start of the main watchdog state machine.
	 *
	 * \sa sreset_wdg_end
	 */
	u_int32_t	sreset_wdg_begin;

	/**
	 * \internal
	 *
	 * This counter is incremented once at the end of the main watchdog state machine.
	 *
	 * \sa sreset_wdg_begin
	 */
	u_int32_t	sreset_wdg_end;
	u_int32_t	sreset_wdg_in_place;
	u_int32_t	sreset_wdg_tx_beacon_hang;

	/**
	 * \internal
	 *
	 * The number of transmit hangs causing soft reset.
	 *
	 * Transmit hang is between 400 to 900ms from the time of sending a packet to the hardware
	 * without receiving a tx done interrupt.
	 */
	u_int32_t	sreset_wdg_tx_hang;

	/**
	 * \internal
	 *
	 * The number of packet memory corruption causing soft reset.
	 *
	 * For unknown reason, packet memory may be corrupted. When packet memory corruption is detected,
	 * soft reset is triggered, and this counter incremented once.
	 */
	u_int32_t	sreset_wdg_pm_corrupt;

	/**
	 * \internal
	 *
	 * The number of packet transmit control memory corruption causing soft reset.
	 *
	 * For unknown reason, transmit control memory may be corrupted. When transmit control memory corruption is detected,
	 * soft reset is triggered, and this counter incremented once.
	 */
	u_int32_t	sreset_wdg_tcm_corrupt;

	/**
	 * \internal
	 *
	 * The number of receive hangs causing a soft reset.
	 *
	 * Receive hang is > 70s without receiving a single packet.
	 *
	 * Note that this can trigger in idle situations, but should not affect anything because
	 * the link is idle.
	 */
	u_int32_t	sreset_wdg_rx_done;
	u_int32_t	sreset_wdg_in_place_try;
	u_int32_t	sreset_wdg_tasklet_sched_1;
	u_int32_t	sreset_wdg_tasklet_sched_2;
	u_int32_t	sreset_tasklet_sched;
	u_int32_t	sreset_tasklet_begin;
	u_int32_t	sreset_tasklet_end;

	/**
	 * \internal
	 *
	 * This counter is incremented when a BB hard reset is requested
	 * to occur in the middle of a soft reset sequence
	 */
	u_int32_t	hreset_req;

	/**
	 * \internal
	 *
	 * This counter is incremented at the start of a soft reset.
	 *
	 * There should always be a corresponding increment in the sreset_end
	 * counter, or there is a problem.
	 *
	 * \sa sreset_end
	 */
	u_int32_t	sreset_begin;

	/**
	 * \internal
	 *
	 * This counter is incremented at the end of a soft reset.
	 *
	 * The should always being a corresponding increment in the sreset_begin
	 * counter, or there is a problem.
	 *
	 * \sa sreset_begin
	 */
	u_int32_t	sreset_end;

	/**
	 * \internal
	 *
	 * This counter is incremented each time DMA RX is in progress when a
	 * soft reset is triggered.
	 */
	u_int32_t	sreset_dma_rx_inprog;

	/**
	 * \internal
	 *
	 * This counter is incremented each time DMA TX is in progress when a
	 * soft reset is triggered.
	 */
	u_int32_t	sreset_dma_tx_inprog;
	u_int32_t	sreset_dma_rx_max_wait;
	u_int32_t	sreset_dma_tx_max_wait;
	u_int32_t	sreset_dma_tx_hang;
	u_int32_t	sreset_dma_rx_hang;
	u_int32_t	sreset_dma_rx_wait_timeout;
	u_int32_t	sreset_dma_tx_wait_timeout;
	u_int32_t	sreset_drop_not_valid;
	u_int32_t	sreset_drop_bad_addr;
	u_int32_t	rf_cmpvtune_out;
	u_int32_t	rf_cal_freq;
	u_int32_t	ac_max;
	u_int32_t	ac_min;
	u_int32_t	ac_cur;
	u_int32_t	ac_adj;
	u_int32_t	rx_gain;
	u_int32_t	rd_cache_indx;
	u_int32_t	logger_sreset_wmac1_dma_rx_inprog;
	u_int32_t	logger_sreset_wmac1_dma_tx_inprog;
	u_int32_t	logger_sreset_wmac1_dma_rx_max_wait;
	u_int32_t	logger_sreset_wmac1_dma_tx_max_wait;
	u_int32_t	logger_sreset_wmac1_dma_tx_hang;
	u_int32_t	logger_sreset_wmac1_dma_rx_hang;
	u_int32_t	logger_sreset_wmac1_dma_rx_wait_timeout;
	u_int32_t	logger_sreset_wmac1_dma_tx_wait_timeout;
	/**
	 * These counter show the information of MU frames.
	 */
	u_int32_t	mu_rx_pkt;

	/**
	 * \internal
	 *
	 * These counters monitor power duty cycling
	 */
	u_int32_t	pduty_sleep;
	u_int32_t	pduty_rxoff;
	u_int32_t	pduty_period;
	u_int32_t	pduty_pct;

	/**
	 * \internal
	 *
	 * These counter are incremented when a soft-ring operation is triggered
	 */
	u_int32_t	soft_ring_push_to_tqe;
	u_int32_t	soft_ring_empty;
	u_int32_t	soft_ring_not_empty;
	u_int32_t	soft_ring_add_force;
	u_int32_t	soft_ring_add_to_head;
	u_int32_t	soft_ring_add_continue;
	u_int32_t	soft_ring_free_pool_empty;
	u_int32_t	mimo_ps_mode_switch;	/* times STA switch MIMO power-save mode by HT action */

	u_int32_t	rx_vlan_drop;
	u_int32_t	auto_cca_state;
	u_int32_t	auto_cca_th;
	u_int32_t	auto_cca_spre;
	u_int32_t	auto_cca_intf;

	/**
	 * \internal
	 *
	 * These counters are monitor memory allocation.
	 */
	u_int32_t	total_dmem_alloc;
	u_int32_t	total_dram_alloc;
	u_int32_t	dmem_alloc_fails;
	u_int32_t	dram_alloc_fails;
	u_int32_t	total_dmem_free;
	u_int32_t	total_dram_free;

	/* RX frames BW mode*/
	u_int32_t	rx_bw_80;
	u_int32_t	rx_bw_40;
	u_int32_t	rx_bw_20;

	/* U-APSD rx stats */
	uint32_t	rx_wmm_ps_trigger;
	uint32_t	rx_wmm_ps_set;
	uint32_t	rx_wmm_ps_reset;

	uint32_t	rx_intr_next_ptr_0;
	uint32_t	rx_hbm_pool_depleted;

	uint32_t	rxq_intr[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_fill[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_nobuf[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_stop[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_pkt[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_bad_status[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_pkt_oversize[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_pkt_delivered[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_status_hole_chk_num[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_status_hole_chk_step_sum[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_status_hole_chk_step_max[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_status_hole[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_status_hole_max_size[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_process_max[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_process_sum[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_process_num[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_process_limited[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rxq_desc_chain_empty[QTN_FW_WMAC_RX_QNUM];
	uint32_t	rx_data_last_seqfrag;
	uint32_t	rx_data_last_ip_id;

	/**
	 * This counter is incremented once per packet which is sent via the
	 * external filter (HotSpot functionality).
	 */
	uint32_t	accel_l2_ext_filter;
	uint32_t	accel_mc_send_l2_ext_filter;

	/**
	 * This counter is incremented once per multicast packet dropped without
	 * forwording to the external filter (HotSpot functionality).
	 */
	uint32_t	accel_mc_drop_l2_ext_filter;

	/**
	 * The number of Rx frames dropped because the WPA2 Packet Number (PN) was not incrementing.
	 * This condition indicates a possible replay attack.
	 */
	uint32_t	rx_replay_attack_drop;
};

#define MUC_HT_NUM_RATES	77
#define MUC_VHT_NUM_RATES	40
struct muc_rx_rates {
	u_int32_t rx_mcs[MUC_HT_NUM_RATES];
	u_int32_t rx_mcs_11ac[MUC_VHT_NUM_RATES];
};

#define QTN_STATS_NUM_BF_SLOTS	10
struct muc_rx_bf_stats {
	u_int32_t	rx_bf_valid[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	rx_bf_aid[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	rx_bf_ng[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	rx_bf_11n_ndp[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	rx_bf_11ac_ndp[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	rx_bf_11n_act[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	rx_bf_11ac_act[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	rx_bf_11ac_grp_sel[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	rx_bf_11ac_prec[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	rx_bf_11ac_su[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	rx_bf_11ac_dsp_fail[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	mu_grp_add[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	mu_grp_del[QTN_STATS_NUM_BF_SLOTS];
	u_int32_t	msg_buf_alloc_fail;
};

/** @} */

extern struct muc_rx_stats uc_rx_stats;
extern struct muc_rx_rates uc_rx_rates;
extern struct muc_rx_bf_stats uc_rx_bf_stats;
extern struct muc_tx_stats uc_tx_stats;
extern struct qtn_rate_tx_stats_per_sec uc_tx_rates;
extern uint32_t uc_su_rate_stats_read;
extern uint32_t uc_mu_rate_stats_read;

/*
 * Rate adaption data collected for packet logger
 * NOTE: Any changes to these definitions will require changes to stat_parser.pl
 */
#define RATES_STATS_NUM_ADAPTATIONS	16
#define RATES_STATS_NUM_TX_RATES	6
#define RATES_STATS_NUM_RX_RATES	8	/* Must be a multiple of word size */
#define RATES_STATS_EVM_CNT		4

/*
 * Currently only two user positions are supported for MU group
 * the following define should be aligned
 * with IEEE80211_MU_GRP_NODES_MAX (4) in future.
 * for now we don't want to take care about 2x extra zero-filled
 * huge arrays in rate stats
 */
#define RATES_STATS_MAX_USER_IN_GROUP   2

/**
 * \addtogroup MUCSTATS
 */
/** @{ */
struct qtn_rate_stats_mcs_data {
	uint16_t	mcs_rate;
	uint16_t	rate_index;
	uint16_t	state;
	uint16_t	pkt_total;
	uint16_t	pkt_error;
	uint16_t	pkt_hw_retry;
	uint16_t	pkt_sample;
	uint16_t	avg_per;
} __attribute__((packed));

struct qtn_rate_su_tx_stats {
	uint32_t			seq_no;
	uint32_t			timestamp;
	uint32_t			flags;
	uint16_t			sampling_index;
	uint16_t			sampling_rate;
	struct qtn_rate_stats_mcs_data	mcs_data[RATES_STATS_NUM_TX_RATES];
} __attribute__((packed));

struct qtn_rate_mu_tx_stats {
	struct qtn_rate_su_tx_stats group_stats[RATES_STATS_MAX_USER_IN_GROUP];
} __attribute__((packed));

struct qtn_rate_gen_stats {
	u_int16_t   rx_mcs_rates[RATES_STATS_NUM_RX_RATES];
	u_int32_t  rx_mcs[RATES_STATS_NUM_RX_RATES];
	u_int32_t  rx_crc;
	u_int32_t  rx_sp_errors;
	u_int32_t  rx_lp_errors;
	u_int32_t  rx_evm[RATES_STATS_EVM_CNT];
	u_int32_t  tx_subframe_success;
	u_int32_t  tx_subframe_fail;
	u_int32_t  tx_mgmt_success;
	u_int32_t  tx_hw_retry;
	u_int32_t  tx_sw_retry;
} __attribute__((packed));

struct qtn_rate_tx_stats_per_sec {
	struct qtn_rate_su_tx_stats  stats_su[RATES_STATS_NUM_ADAPTATIONS];
	struct qtn_rate_mu_tx_stats  stats_mu[RATES_STATS_NUM_ADAPTATIONS];
};
/** @} */

#endif	/* _STATS_H_ */
