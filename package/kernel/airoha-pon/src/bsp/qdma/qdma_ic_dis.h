#ifndef _QDMA_IC_DIS_H_
#define _QDMA_IC_DIS_H_

#define SUPPORT_ROUNT_CNT			((isEN7581)?1:2)  /*num of verify qdma ring*/
#define SUPPORT_NO_DROP_IN_TX_MSG1	(!isEN7581)		  /*no drop msg in tx msg1*/
#define SUPPORT_RING_MASK			(!isEN7581)
#define SUPPORT_SPEED_TEST_HW_CHECKSUM_TCO		(isEN7580 || isEN7523 || isEN7581)

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_ACNT_GRP2 1
#else
#define SUPPORT_ACNT_GRP2 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_QDMA_MULTICAST 1
#else
#define SUPPORT_QDMA_MULTICAST 0
#endif

#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
#define SUPPORT_QUEUE_DISABLE 1
#else
#define SUPPORT_QUEUE_DISABLE 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_MTR_ACNT_IN_QDMA 1
#else
#define SUPPORT_MTR_ACNT_IN_QDMA 0
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
#define SUPPORT_INGRESS_3X_RATELIMIT 1
#else
#define SUPPORT_INGRESS_3X_RATELIMIT 0
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_IRQ2 1
#else
#define SUPPORT_IRQ2 0
#endif

/*after 7528,wrr weight scale change to 1Byte from 16Byte*/
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
#define SUPPORT_WRR_WEIGHT_SCALE_1Byte 1
#else
#define SUPPORT_WRR_WEIGHT_SCALE_1Byte 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_WRR_WEIGHT_255 1
#else
#define SUPPORT_WRR_WEIGHT_255 0
#endif

/*after 7523,bucketsize bitwidth change to 17 from 15*/
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_BUCKETSIZE_BITWIDTH_17 1
#else
#define SUPPORT_BUCKETSIZE_BITWIDTH_17 0
#endif

/*
1.dbgcntr 2 ring mode for 7526 because 7526 have only 2 rings
2.751627 have 16 rings but it still use old dbgcntr for 2 rings,and new part for other rings
*/
#if defined(TCSUPPORT_CPU_EN7516) || defined(TCSUPPORT_CPU_EN7527)
#define SUPPORT_DBGCNTR_2RING_MODE 1
#else
#define SUPPORT_DBGCNTR_2RING_MODE 0
#endif

#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
#define SUPPORT_WRR_WEIGHT_0 1
#else
#define SUPPORT_WRR_WEIGHT_0 0
#endif

/*before 7580 int enable register num is 3,after 7580 it changes to 2*/
#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_INT_ENABLE_NUM_2 1
#else
#define SUPPORT_INT_ENABLE_NUM_2 0
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_SLA 1
#else
#define SUPPORT_SLA 0
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_QUEUE_AGING 1
#else
#define SUPPORT_QUEUE_AGING 0
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_SDN_CNTR 1
#else
#define SUPPORT_SDN_CNTR 0
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_HQOS 1
#else
#define SUPPORT_HQOS 0
#endif

/*now only 10G chip 7580&7581 support this*/
#if defined(TCSUPPORT_CPU_EN7580)
#if defined(TCSUPPORT_CPU_EN7581)
#define SUPPORT_HWDSCP_IN_SRAM 1
#elif defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_HWDSCP_IN_SRAM 0
#else
#define SUPPORT_HWDSCP_IN_SRAM 1
#endif
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_HWDSCP_PAYLOAD_CFG 1
#else
#define SUPPORT_HWDSCP_PAYLOAD_CFG 0
#endif

/*new dbgcntr mode after 7523*/
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_DBG_CNTMEM 1
#else
#define SUPPORT_DBG_CNTMEM 0
#endif

#if !defined(TCSUPPORT_CPU_EN7580)
#define OFFLOAD_NEED_IMMEDIATE_DONE 1
#else
#define OFFLOAD_NEED_IMMEDIATE_DONE 0
#endif

/*
before 7523 time to meter window size is fixed 4ms,now we can set other time
*/
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_METER_WINDOW_SIZE_TIME_SLIDE 1
#else
#define SUPPORT_METER_WINDOW_SIZE_TIME_SLIDE 0
#endif

/*
1.used to resever buff to each channel in 7528&7580,7580 default disable
2.replaced by queue aging after 7523
*/
#if defined(TCSUPPORT_CPU_EN7528)
#define SUPPORT_MIN_DSCP_THRH_CFG 1
#else
#define SUPPORT_MIN_DSCP_THRH_CFG 0
#endif

#if !defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_QDMA_UMAC_LOOPBACK 1
#else
#define SUPPORT_QDMA_UMAC_LOOPBACK 0
#endif

#if !defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_RX_PROTECT_MODE 1
#else
#define SUPPORT_RX_PROTECT_MODE 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_SET_TX_QUEUE_MIN_THRH 1
#else
#define SUPPORT_SET_TX_QUEUE_MIN_THRH 0
#endif

#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
#define SUPPORT_TX_DROP_BIT 1
#else
#define SUPPORT_TX_DROP_BIT 0
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_TXQ_DEI_DROP 1
#else
#define SUPPORT_TXQ_DEI_DROP 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_TXQ_WRED 1
#else
#define SUPPORT_TXQ_WRED 0
#endif

/*
1.config buf threshold different between fast and slow path
2.use dscp mode in sram or dram to judge fast or slow path,becasue fast path is usually binded with dscp in sram
*/
#if defined(TCSUPPORT_CPU_EN7581)
#define SUPPORT_BUFFCFG_DIFFERENT_BETWEEN_FAST_SLOW_PATH 1
#else
#define SUPPORT_BUFFCFG_DIFFERENT_BETWEEN_FAST_SLOW_PATH 0
#endif

/*
1.enable tx ring block to avoid use up all lkmgr buff
2.do not need enable default after 7580 because lkmgr is up to 16K
*/
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
#define NEED_ENABLE_TX_RING_BLOCKING 1
#else
#define NEED_ENABLE_TX_RING_BLOCKING 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_8_TX_RING 1
#else
#define SUPPORT_8_TX_RING 0
#endif

/*
1.used to reserve buff for each queue
2.now only 7580 need this,chips after 7580(7523/7581...) have new mechanism
*/
#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_TXQ_CNGST_MIN_THRH 1
#else
#define SUPPORT_TXQ_CNGST_MIN_THRH 0
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_TXQ_CNGST_STATIC_QUEUE_RATIO 1
#else
#define SUPPORT_TXQ_CNGST_STATIC_QUEUE_RATIO 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_CHANNEL_CLOSE_STATUS 1
#else
#define SUPPORT_CHANNEL_CLOSE_STATUS 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_OAM_MODIFY_FP 1
#else
#define SUPPORT_OAM_MODIFY_FP 0
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_FLOWCNT_PKT_HI 1
#else
#define SUPPORT_FLOWCNT_PKT_HI 0
#endif

/*
when etherwan use qdma_wan,cpu path force port to GDM1
detail information please refer to SD2 wiki
*/
#if defined(TCSUPPORT_CPU_EN7523)
#define SUPPORT_ETHERWAN_USE_QDMAWAN_FPORT_TO_GDM1 1
#else
#define SUPPORT_ETHERWAN_USE_QDMAWAN_FPORT_TO_GDM1 0
#endif

/*chips after 7580,txq congestion dynamic mode is always on,no static mode any more*/
#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_TXQ_CNGST_DYN_MODE_ALWAYS_ON 1
#else
#define SUPPORT_TXQ_CNGST_DYN_MODE_ALWAYS_ON 0
#endif

#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
#define SUPPORT_TXQ_CNGST_DYN_DEI_THRH_SCALE 1
#else
#define SUPPORT_TXQ_CNGST_DYN_DEI_THRH_SCALE 0
#endif

#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
#define SUPPORT_PEEK_RATE_MARGIN 1
#else
#define SUPPORT_PEEK_RATE_MARGIN 0
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
#define SUPPORT_FRAGMENT_REASSEMBLE 1
#else
#define SUPPORT_FRAGMENT_REASSEMBLE 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_QDMA_DRAM_TEST 1
#else
#define SUPPORT_QDMA_DRAM_TEST 0
#endif

/*no enable bit for rx delay interrupt after 7580*/
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
#define SUPPORT_RX_DLYINT_EN 1
#else
#define SUPPORT_RX_DLYINT_EN 0
#endif

/*no drop bit in txmsg1*/
#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_NO_DROP_BIT 1
#else
#define SUPPORT_NO_DROP_BIT 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_LRO 1
#else
#define SUPPORT_LRO 0
#endif

/*qdma init parameters in boot env*/
#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_QDMA_INIT_PARAM 1
#else
#define SUPPORT_QDMA_INIT_PARAM 0
#endif

/*after 7580,lkmgr csr do not support write on-fly,we should config it when do qdma init*/
#if defined(TCSUPPORT_CPU_EN7580)
#define NEED_WRITE_LKMGR_CSR_WHEN_INIT 1
#else
#define NEED_WRITE_LKMGR_CSR_WHEN_INIT 0
#endif

/*if stag in skb,packets from lan will increase 4 bytes,so we need to set overhead to 20 bytes(default is 24B)*/
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
#define SUPPORT_STAG_IN_SKB 1
#else
#define SUPPORT_STAG_IN_SKB 0
#endif

/*some pon releated interrupts is in qdma,so we need to regster callback function for these interrupts*/
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
#define SUPPORT_PON_INT_IN_QDMA 1
#else
#define SUPPORT_PON_INT_IN_QDMA 0
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_REV_BUFF_IN_KERNEL 1
#else
#define SUPPORT_REV_BUFF_IN_KERNEL 0
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
#define SUPPORT_TUNNEL_HW_ACCELE    1
#else
#define SUPPORT_TUNNEL_HW_ACCELE    0
#endif

#if defined(TCSUPPORT_CPU_EN7581)
#define SUPPORT_32_RX_RING 1
#else
#define SUPPORT_32_RX_RING 0
#endif

#if defined(TCSUPPORT_CPU_AN7552) || defined(TCSUPPORT_CPU_EN7581)
#define SUPPORT_CONFIG_QDMA_DSCP_BY_MI_CONFIG 1
#else
#define SUPPORT_CONFIG_QDMA_DSCP_BY_MI_CONFIG 0
#endif


/*3 color for meter1&2&3 in 7583*/
#if defined(TCSUPPORT_CPU_AN7583)
#define SUPPORT_INX_COLOR_RESOLUTION		1
#else
#define SUPPORT_INX_COLOR_RESOLUTION		0
#endif

#if defined(TCSUPPORT_CPU_AN7583)
#define SUPPORT_MAX_BURSTLEN_256B	1
#else
#define SUPPORT_MAX_BURSTLEN_256B	0
#endif


#if defined(TCSUPPORT_CPU_AN7583) 
#define SUPPORT_TR471_HW_FUNCTION 1
#else
#define SUPPORT_TR471_HW_FUNCTION 0
#endif

#if defined(TCSUPPORT_CPU_AN7583)
#define SUPPORT_FAST_THR	1
#else
#define SUPPORT_FAST_THR	0
#endif

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7581)
#define SUPPORT_TX_PEAK_MODE		1
#else
#define SUPPORT_TX_PEAK_MODE		0
#endif

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
#define SUPPORT_IFC_TCAM		1
#else
#define SUPPORT_IFC_TCAM		0
#endif
/*before 7580 not has ifc*/

#if defined(TCSUPPORT_CPU_EN7580)
#define SUPPORT_IFC 1
#else
#define SUPPORT_IFC 0
#endif

#endif
