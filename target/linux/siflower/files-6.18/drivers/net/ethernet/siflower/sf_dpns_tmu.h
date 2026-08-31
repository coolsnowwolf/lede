#ifndef __SF_TMU_H__
#define __SF_TMU_H__
#include <linux/bitfield.h>
// npu clk is 400MHz in mpw, will change to 600MHz in fullmask
// TODO: should use mpw define to diff
#define LIF_SHP_CLKDIV_DEF      (3)
#define LIF_WEIGHT_MAX          (0x7ff)
#define TMU_SHP_CLKDIV_DEF      (6)
#define TMU_SHP_CLKDIV_MAX      (15)
#define TMU_WEIGHT_MAX			(256)
#define TMU_SHP_INT_WGHT_MAX    ((TMU_SHP_WEIGHT_INT_MASK) >> TMU_SHP_WEIGHT_INT_SHIFT)
#define TMU_SHP_FRAC_WGHT_MAX   ((TMU_SHP_WEIGHT_FRAC_MASK) >> TMU_SHP_WEIGHT_FRAC_SHIFT)

#define TMU_VERSION_INFO		0x148000
#define TMU_ID_SHIFT			0
#define TMU_ID				GENMASK(15, 0)
#define TMU_VERSION_SHIFT		16
#define TMU_VERSION			GENMASK(23, 16)
#define TMU_REVISION_SHIFT		24
#define TMU_REVISION			GENMASK(31, 24)

#define TMU_CTRL			0x148004
#define TMU_MF_IN_CNT_EN_SHIFT		1
#define TMU_MF_IN_CNT_EN		BIT(1)
#define TMU_MD_RDY_EN_SHIFT		2
#define TMU_MD_RDY_EN			BIT(2)
#define TMU_DEBUG_SEL_SHIFT		3
#define TMU_DEBUG_SEL			GENMASK(8, 3)

#define TMU_LLM_FIFO_CTRL0		0x148008
#define TMU_LLM_FIFO_FULL_ASSERT_SHIFT	0
#define TMU_LLM_FIFO_FULL_ASSERT	GENMASK(11, 0)
#define TMU_LLM_FIFO_FULL_NEGATE_SHIFT	16
#define TMU_LLM_FIFO_FULL_NEGATE	GENMASK(27, 16)

#define TMU_LLM_FIFO_CTRL1		0x14800c
#define TMU_LLM_FIFO_EMPTY_ASSERT_SHIFT	0
#define TMU_LLM_FIFO_EMPTY_ASSERT	GENMASK(11, 0)
#define TMU_LLM_FIFO_EMPTY_NEGATE_SHIFT	16
#define TMU_LLM_FIFO_EMPTY_NEGATE	GENMASK(27, 16)

#define TMU_RD_CLR_EN          0x2800c0
#define TMU_BUF_THR0           0x2800d8

/* 36 ports in registers */
#define TMU_PORT0			0x0000

#define TMU_PORT_SZ			0x2000
#define TMU_PORT_CNT			36
#define TMU_PORT_CNT_V1			10

/* 8 queues for each port */
#define TMU_PORT_QUEUE0			0x100000

#define TMU_PORT_QUEUE_SZ		0x20
#define TMU_PORT_QUEUE_CNT		8

#define TMU_PORT_QUEUE_CFG0		0x00
/* 0x00: mix tail drop
 * 0x01: tail drop (default)
 * 0x02: WRED
 * 0x03: buf count tail drop
 */
#define TMU_DROP_TYPE_SHIFT		0
#define TMU_DROP_TYPE			GENMASK(1, 0)
#define TMU_QUEUE_MAX_SHIFT		8
#define TMU_QUEUE_MAX			GENMASK(18, 8)
#define TMU_QUEUE_MIN_SHIFT		20
#define TMU_QUEUE_MIN			GENMASK(30, 20) // related to WRED

#define TMU_QUEUE_MIX_TAIL_DROP		0x00
#define TMU_QUEUE_TAIL_DROP		0x01
#define TMU_QUEUE_WRED			0x02
#define TMU_QUEUE_BUF_CNT_TAIL_DROP	0x03

/* drop probability of each stage in WRED */
#define TMU_PORT_QUEUE_CFG1		0x04
#define TMU_WRED_HW_PROB_STG0_SHIFT	0
#define TMU_WRED_HW_PROB_STG0		GENMASK(4, 0)
#define TMU_WRED_HW_PROB_STG1_SHIFT	5
#define TMU_WRED_HW_PROB_STG1		GENMASK(9, 5)
#define TMU_WRED_HW_PROB_STG2_SHIFT	10
#define TMU_WRED_HW_PROB_STG2		GENMASK(14, 10)
#define TMU_WRED_HW_PROB_STG3_SHIFT	15
#define TMU_WRED_HW_PROB_STG3		GENMASK(19, 15)
#define TMU_WRED_HW_PROB_STG4_SHIFT	20
#define TMU_WRED_HW_PROB_STG4		GENMASK(24, 20)
#define TMU_WRED_HW_PROB_STG5_SHIFT	25
#define TMU_WRED_HW_PROB_STG5		GENMASK(29, 25)

#define TMU_PORT_QUEUE_CFG2		0x08
#define TMU_WRED_HW_PROB_STG6_SHIFT	0
#define TMU_WRED_HW_PROB_STG6		GENMASK(4, 0)
#define TMU_WRED_HW_PROB_STG7_SHIFT	5
#define TMU_WRED_HW_PROB_STG7		GENMASK(9, 5)

#define TMU_WRED_PROB_CNT		8

/* RO */
#define TMU_PORT_QUEUE_STS0		0x0c
#define TMU_QUEUE_HEAD_PTR_SHIFT	0
#define TMU_QUEUE_HEAD_PTR		GENMASK(10, 0)
#define TMU_QUEUE_TAIL_PTR_SHIFT	16
#define TMU_QUEUE_TAIL_PTR		GENMASK(26, 16)

/* RO */
#define TMU_PORT_QUEUE_STS1		0x10
#define TMU_QUEUE_PKT_CNT_SHIFT		0
#define TMU_QUEUE_PKT_CNT		GENMASK(11, 0)

/* RO */
#define TMU_PORT_QUEUE_STS2		0x14
#define TMU_QUEUE_BUF_CNT_SHIFT		0
#define TMU_QUEUE_BUF_CNT		GENMASK(11, 0)

/* max buffer cell of queue */
#define TMU_PORT_QUEUE_CFG3		0x18
#define TMU_QUEUE_BUF_MAX_SHIFT		0
#define TMU_QUEUE_BUF_MAX		GENMASK(11, 0)

/* 2 schedulers (dequeuing) for each port */
#define TMU_SCH0			0x101000
#define TMU_SCH1			0x101040

#define TMU_SCH_SZ			0x40
#define TMU_SCH_CNT			2

#define TMU_SCH_CTRL			0x00
#define TMU_SCH_ALGO_SHIFT		0
#define TMU_SCH_ALGO			GENMASK(6, 0)

/* TMU_SCH_ALGO */
#define TMU_SCH_PQ			0x00
#define TMU_SCH_WFQ			0x01
#define TMU_SCH_DWRR			0x02
#define TMU_SCH_RR			0x03
#define TMU_SCH_WRR			0x04

#define TMU_SCH_Q0_WEIGHT		0x10
#define TMU_SCH_Q1_WEIGHT		0x14
#define TMU_SCH_Q2_WEIGHT		0x18
#define TMU_SCH_Q3_WEIGHT		0x1c
#define TMU_SCH_Q4_WEIGHT		0x20
#define TMU_SCH_Q5_WEIGHT		0x24
#define TMU_SCH_Q6_WEIGHT		0x28
#define TMU_SCH_Q7_WEIGHT		0x2c
#define TMU_SCH_Q_WEIGHT_SZ		4

#define TMU_SCH_Q_WEIGHT_CNT		8

/* TMU_SCH_Qn_WEIGHT */
#define TMU_SCH_QUEUE_WEIGHT_SHIFT	0
#define TMU_SCH_QUEUE_WEIGHT		GENMASK(31, 0)

/* port queue and scheduler selection */
#define TMU_SCH_QUEUE_ALLOC0		0x30
#define TMU_SCH_Q0_ALLOC_SHIFT		0
#define TMU_SCH_Q0_ALLOC		GENMASK(3, 0)
#define TMU_SCH_Q1_ALLOC_SHIFT		8
#define TMU_SCH_Q1_ALLOC		GENMASK(11, 8)
#define TMU_SCH_Q2_ALLOC_SHIFT		16
#define TMU_SCH_Q2_ALLOC		GENMASK(19, 16)
#define TMU_SCH_Q3_ALLOC_SHIFT		24
#define TMU_SCH_Q3_ALLOC		GENMASK(27, 24)

#define TMU_SCH_QUEUE_ALLOC1		0x34
#define TMU_SCH_Q4_ALLOC_SHIFT		0
#define TMU_SCH_Q4_ALLOC		GENMASK(3, 0)
#define TMU_SCH_Q5_ALLOC_SHIFT		8
#define TMU_SCH_Q5_ALLOC		GENMASK(11, 8)
#define TMU_SCH_Q6_ALLOC_SHIFT		16
#define TMU_SCH_Q6_ALLOC		GENMASK(19, 16)
#define TMU_SCH_Q7_ALLOC_SHIFT		24
#define TMU_SCH_Q7_ALLOC		GENMASK(27, 24)

#define TMU_SCH_Q_ALLOC_CNT		8

// schedule by pkt_len or pkt_cnt
#define TMU_SCH_BIT_RATE		0x38
#define TMU_SCH_BIT_RATE_SHIFT		0
#define TMU_SCH_BIT_RATE_MASK		GENMASK(31, 0)
#define TMU_SCH_BIT_RATE_PKT_LEN	0x00
#define TMU_SCH_BIT_RATE_PKT_CNT	0x01

// RW
// SCH0 Only, to select how to connect to SCH1
#define TMU_SCH0_POS			0x3c
#define TMU_SCH0_POS_SHIFT		0
#define TMU_SCH0_POS_MASK		GENMASK(3, 0)

/* 5 shapers for each port */
#define TMU_SHP0			0x101080

#define TMU_SHP_SZ			0x0020
#define TMU_SHP_CNT			6

#define TMU_SHP_CTRL			0x00
#define TMU_SHP_EN_SHIFT		0
#define TMU_SHP_EN			BIT(0)
#define TMU_SHP_CLK_DIV_SHIFT		1
#define TMU_SHP_CLK_DIV			GENMASK(31, 1)

/* byte size of per token (credit) */
#define TMU_SHP_WEIGHT			0x04
#define TMU_SHP_WEIGHT_FRAC_SHIFT	0
#define TMU_SHP_WEIGHT_FRAC_MASK	GENMASK(11, 0)
#define TMU_SHP_WEIGHT_INT_SHIFT        12
#define TMU_SHP_WEIGHT_INT_MASK         GENMASK(19, 12)

#define TMU_SHP_MAX_CREDIT		0x08
#define TMU_SHP_MAX_CREDIT_SHIFT	10
#define TMU_SHP_MAX_CREDIT_MASK		GENMASK(31, 10)

// (fraction part num) = (register fraction part) / (2 ^ 12)
#define TMU_SHP_FRAC_WEIGHT_2DBL(reg)   (((double)(reg)) / (1 << 12))
#define TMU_SHP_DBL_2FRAC_WEIGHT(dbl)   (((double)(dbl)) * (1 << 12))

#define TMU_SHP_CTRL2			0x0c
#define TMU_SHP_BIT_RATE_SHIFT		0
#define TMU_SHP_BIT_RATE		BIT(0)
#define TMU_SHP_POS_SHIFT		1
#define TMU_SHP_POS			GENMASK(5, 1)	// RW
#define TMU_SHP_MODE_SHIFT		6
#define TMU_SHP_MODE			BIT(6)

/* TMU_SHP_BIT_RATE */
#define TMU_SHP_SCHED_PKT_LEN		0
#define TMU_SHP_SCHED_PKT_CNT		1

/* TMU_SHP_MODE */
#define TMU_SHP_MODE_KEEP_CREDIT	0
#define TMU_SHP_MODE_CLEAR_CREDIT	1

#define TMU_SHP_MIN_CREDIT		0x10
#define TMU_SHP_MIN_CREDIT_SHIFT	0
#define TMU_SHP_MIN_CREDIT_MASK		GENMASK(21, 0)

/* RO */
#define TMU_SHP_STATUS			0x14
#define TMU_SHP_CURR_STATUS_SHIFT	0	// shaper is working or not
#define TMU_SHP_CURR_STATUS		BIT(0)
#define TMU_SHP_CREDIT_CNTR_SHIFT	1
#define TMU_SHP_CREDIT_CNTR		GENMASK(23, 1)

/* dequeue stage configs */
#define TMU_TDQ				0x101140

/* effective only when TMU_SCH_BIT_RATE is PKT_LEN mode.
 * ifg value can be used to adjust packet length of scheduler.
 */
#define TMU_TDQ_IFG			(TMU_TDQ + 0x00)
#define TMU_TDQ_IIF_CFG_SHIFT		0
#define TMU_TDQ_IIF_CFG			GENMASK(7, 0)

#define TMU_TDQ_CTRL			(TMU_TDQ + 0x04)
#define TMU_SHP_CLK_CNT_EN_SHIFT	0
#define TMU_SHP_CLK_CNT_EN		BIT(0)
#define TMU_TDQ_HW_EN_SHIFT		1
#define TMU_TDQ_HW_EN			BIT(1)
#define TMU_SCH0_EN_SHIFT		2
#define TMU_SCH0_EN			BIT(2)
#define TMU_SCH1_EN_SHIFT		3
#define TMU_SCH1_EN			BIT(3)
#define TMU_TDQ_ALLOW_CFG_SHIFT		4
#define TMU_TDQ_ALLOW_CFG		BIT(4)	// RO, 1 = configurable
#define TMU_PKT_LEFT_IGNORE_SHIFT	5
#define TMU_PKT_LEFT_IGNORE		BIT(5)

#define TMU_PORT_BASE(port)		(TMU_PORT0 + TMU_PORT_SZ * (port))
#define TMU_QUEUE_BASE(q)		(TMU_PORT_QUEUE0 + TMU_PORT_QUEUE_SZ * (q))
#define TMU_SCHED_BASE(sch)		(TMU_SCH0 + TMU_SCH_SZ * (sch))
#define TMU_SHAPER_BASE(shp)		(TMU_SHP0 + TMU_SHP_SZ * (shp))



#define TMU_MAX_PORT_CNT 10
#define QUE_MAX_NUM_PER_PORT 8
#define QUE_SHAPER_NUM_PER_PORT 6
#define QUE_SCH_NUM_PER_PORT 2

enum TMU_QUEUE_TYPE {
	TMU_Q_MIX_TAIL_DROP = 0,
	TMU_Q_TAIL_DROP,
	TMU_Q_WRED,
	TMU_Q_BUF_CNT_TAIL_DROP,
	NUM_TMU_QUEUE_TYPES,
};

enum TMU_SCHED_ALG {
	TMU_SCHED_PQ = 0,
	TMU_SCHED_WFQ,
	TMU_SCHED_DWRR,
	TMU_SCHED_RR,
	TMU_SCHED_WRR,
	NUM_TMU_SCEHD_ALGS,
};

enum TMU_BITRATE_MODE {
	TMU_BITRATE_PKTLEN = 0,
	TMU_BITRATE_PKTCNT,
	NUM_TMU_BITRATE_MODES,
};

static const u8 sched_q_weight_regs[] = {
	TMU_SCH_Q0_WEIGHT,
	TMU_SCH_Q1_WEIGHT,
	TMU_SCH_Q2_WEIGHT,
	TMU_SCH_Q3_WEIGHT,
	TMU_SCH_Q4_WEIGHT,
	TMU_SCH_Q5_WEIGHT,
	TMU_SCH_Q6_WEIGHT,
	TMU_SCH_Q7_WEIGHT,
};

#endif /* __SF_TMU_H__ */
