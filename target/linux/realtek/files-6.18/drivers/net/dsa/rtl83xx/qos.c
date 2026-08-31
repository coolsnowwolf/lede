// SPDX-License-Identifier: GPL-2.0-only

#include <net/dsa.h>
#include <linux/delay.h>
#include <asm/mach-rtl-otto/mach-rtl-otto.h>

#include "rtl-otto.h"

enum scheduler_type {
	WEIGHTED_FAIR_QUEUE = 0,
	WEIGHTED_ROUND_ROBIN,
};

int rtldsa_max_available_queue[] = {0, 1, 2, 3, 4, 5, 6, 7};
int rtldsa_default_queue_weights[] = {1, 1, 1, 1, 1, 1, 1, 1};
int dot1p_priority_remapping[] = {0, 1, 2, 3, 4, 5, 6, 7};

static void rtl839x_read_scheduling_table(int port)
{
	u32 cmd = 1 << 9 | /* Execute cmd */
		  0 << 8 | /* Read */
		  0 << 6 | /* Table type 0b00 */
		  (port & 0x3f);
	rtl839x_exec_tbl2_cmd(cmd);
}

static void rtl839x_write_scheduling_table(int port)
{
	u32 cmd = 1 << 9 | /* Execute cmd */
		  1 << 8 | /* Write */
		  0 << 6 | /* Table type 0b00 */
		  (port & 0x3f);
	rtl839x_exec_tbl2_cmd(cmd);
}

static void rtl839x_read_out_q_table(int port)
{
	u32 cmd = 1 << 9 | /* Execute cmd */
		  0 << 8 | /* Read */
		  2 << 6 | /* Table type 0b10 */
		  (port & 0x3f);
	rtl839x_exec_tbl2_cmd(cmd);
}

u32 rtldsa_838x_get_egress_rate(struct rtl838x_switch_priv *priv, int port)
{
	if (port > priv->r->cpu_port)
		return 0;

	return sw_r32(RTL838X_SCHED_P_EGR_RATE_CTRL(port)) & 0x3fff;
}

/* Sets the rate limit, 10MBit/s is equal to a rate value of 625 */
int rtldsa_838x_set_egress_rate(struct rtl838x_switch_priv *priv, int port, u32 rate)
{
	u32 old_rate;

	if (port > priv->r->cpu_port)
		return -1;

	old_rate = sw_r32(RTL838X_SCHED_P_EGR_RATE_CTRL(port));
	sw_w32(rate, RTL838X_SCHED_P_EGR_RATE_CTRL(port));

	return old_rate;
}

/* Sets the rate limit, 10MBit/s is equal to a rate value of 625 */
u32 rtldsa_839x_get_egress_rate(struct rtl838x_switch_priv *priv, int port)
{
	u32 rate;

	if (port >= priv->r->cpu_port)
		return 0;

	mutex_lock(&priv->reg_mutex);

	rtl839x_read_scheduling_table(port);

	rate = sw_r32(RTL839X_TBL_ACCESS_DATA_2(7));
	rate <<= 12;
	rate |= sw_r32(RTL839X_TBL_ACCESS_DATA_2(8)) >> 20;

	mutex_unlock(&priv->reg_mutex);
	pr_debug("%s: Getting egress rate on port %d is %d\n", __func__, port, rate);

	return rate;
}

/* Sets the rate limit, 10MBit/s is equal to a rate value of 625, returns previous rate */
int rtldsa_839x_set_egress_rate(struct rtl838x_switch_priv *priv, int port, u32 rate)
{
	u32 old_rate;

	pr_debug("%s: Setting egress rate on port %d to %d\n", __func__, port, rate);
	if (port >= priv->r->cpu_port)
		return -1;

	mutex_lock(&priv->reg_mutex);

	rtl839x_read_scheduling_table(port);

	old_rate = sw_r32(RTL839X_TBL_ACCESS_DATA_2(7)) & 0xff;
	old_rate <<= 12;
	old_rate |= sw_r32(RTL839X_TBL_ACCESS_DATA_2(8)) >> 20;
	sw_w32_mask(0xff, (rate >> 12) & 0xff, RTL839X_TBL_ACCESS_DATA_2(7));
	sw_w32_mask(0xfff << 20, rate << 20, RTL839X_TBL_ACCESS_DATA_2(8));

	rtl839x_write_scheduling_table(port);

	mutex_unlock(&priv->reg_mutex);

	return old_rate;
}

static void rtl838x_setup_prio2queue_matrix(int *min_queues)
{
	u32 v = 0;

	pr_info("Current Intprio2queue setting: %08x\n", sw_r32(RTL838X_QM_INTPRI2QID_CTRL));
	for (int i = 0; i < MAX_PRIOS; i++)
		v |= i << (min_queues[i] * 3);
	sw_w32(v, RTL838X_QM_INTPRI2QID_CTRL);
}

static void rtl839x_setup_prio2queue_matrix(int *min_queues)
{
	pr_info("Current Intprio2queue setting: %08x\n", sw_r32(RTL839X_QM_INTPRI2QID_CTRL(0)));
	for (int i = 0; i < MAX_PRIOS; i++) {
		int q = min_queues[i];

		sw_w32(i << (q * 3), RTL839X_QM_INTPRI2QID_CTRL(q));
	}
}

/* Sets the CPU queue depending on the internal priority of a packet */
static void rtl83xx_setup_prio2queue_cpu_matrix(int reg)
{
	u32 v = 0;

	pr_info("QM_PKT2CPU_INTPRI_MAP: %08x\n", sw_r32(reg));
	for (int i = 0; i < MAX_PRIOS; i++)
		v |= rtldsa_max_available_queue[i] << (i * 3);
	sw_w32(v, reg);
}

static void rtl838x_setup_default_prio2queue(void)
{
	rtl838x_setup_prio2queue_matrix(rtldsa_max_available_queue);
	rtl83xx_setup_prio2queue_cpu_matrix(RTL838X_QM_PKT2CPU_INTPRI_MAP);
}

static void rtl839x_setup_default_prio2queue(void)
{
	rtl839x_setup_prio2queue_matrix(rtldsa_max_available_queue);
	rtl83xx_setup_prio2queue_cpu_matrix(RTL839X_QM_PKT2CPU_INTPRI_MAP);
}

/* Sets the output queue assigned to a port, the port can be the CPU-port */
void rtl839x_set_egress_queue(int port, int queue)
{
	sw_w32(queue << ((port % 10) * 3), RTL839X_QM_PORT_QNUM(port));
}

/* Sets the priority assigned of an ingress port, the port can be the CPU-port */
static void rtldsa_839x_set_ingress_priority(int port, int priority)
{
	int shift = ((port % 10) * 3);

	sw_w32_mask(7 << shift, priority << shift, RTL839X_PRI_SEL_PORT_PRI(port));
}

static int rtl839x_get_scheduling_algorithm(struct rtl838x_switch_priv *priv, int port)
{
	u32 v;

	mutex_lock(&priv->reg_mutex);

	rtl839x_read_scheduling_table(port);
	v = sw_r32(RTL839X_TBL_ACCESS_DATA_2(8));

	mutex_unlock(&priv->reg_mutex);

	if (v & BIT(19))
		return WEIGHTED_ROUND_ROBIN;

	return WEIGHTED_FAIR_QUEUE;
}

static void rtl839x_set_scheduling_algorithm(struct rtl838x_switch_priv *priv, int port,
					     enum scheduler_type sched)
{
	enum scheduler_type t = rtl839x_get_scheduling_algorithm(priv, port);
	u32 v, oam_state, oam_port_state;
	u32 count;
	int i, egress_rate;

	mutex_lock(&priv->reg_mutex);
	/* Check whether we need to empty the egress queue of that port due to Errata E0014503 */
	if (sched == WEIGHTED_FAIR_QUEUE && t == WEIGHTED_ROUND_ROBIN && port != priv->r->cpu_port) {
		/* Read Operations, Adminstatrion and Management control register */
		oam_state = sw_r32(RTL839X_OAM_CTRL);

		/* Get current OAM state */
		oam_port_state = sw_r32(RTL839X_OAM_PORT_ACT_CTRL(port));

		/* Disable OAM to block traffice */
		v = sw_r32(RTL839X_OAM_CTRL);
		sw_w32_mask(0, 1, RTL839X_OAM_CTRL);
		v = sw_r32(RTL839X_OAM_CTRL);

		/* Set to trap action OAM forward (bits 1, 2) and OAM Mux Action Drop (bit 0) */
		sw_w32(0x2, RTL839X_OAM_PORT_ACT_CTRL(port));

		/* Set port egress rate to unlimited */
		egress_rate = rtldsa_839x_set_egress_rate(priv, port, 0xFFFFF);

		/* Wait until the egress used page count of that port is 0 */
		i = 0;
		do {
			usleep_range(100, 200);
			rtl839x_read_out_q_table(port);
			count = sw_r32(RTL839X_TBL_ACCESS_DATA_2(6));
			count >>= 20;
			i++;
		} while (i < 3500 && count > 0);
	}

	/* Actually set the scheduling algorithm */
	rtl839x_read_scheduling_table(port);
	sw_w32_mask(BIT(19), sched ? BIT(19) : 0, RTL839X_TBL_ACCESS_DATA_2(8));
	rtl839x_write_scheduling_table(port);

	if (sched == WEIGHTED_FAIR_QUEUE && t == WEIGHTED_ROUND_ROBIN && port != priv->r->cpu_port) {
		/* Restore OAM state to control register */
		sw_w32(oam_state, RTL839X_OAM_CTRL);

		/* Restore trap action state */
		sw_w32(oam_port_state, RTL839X_OAM_PORT_ACT_CTRL(port));

		/* Restore port egress rate */
		rtldsa_839x_set_egress_rate(priv, port, egress_rate);
	}

	mutex_unlock(&priv->reg_mutex);
}

static void rtl839x_set_scheduling_queue_weights(struct rtl838x_switch_priv *priv, int port,
						 int *queue_weights)
{
	mutex_lock(&priv->reg_mutex);

	rtl839x_read_scheduling_table(port);

	for (int i = 0; i < 8; i++) {
		int lsb = 48 + i * 8;
		int low_byte = 8 - (lsb >> 5);
		int start_bit = lsb - (low_byte << 5);
		int high_mask = 0x3ff >> (32 - start_bit);

		sw_w32_mask(0x3ff << start_bit, (queue_weights[i] & 0x3ff) << start_bit,
			    RTL839X_TBL_ACCESS_DATA_2(low_byte));
		if (high_mask)
			sw_w32_mask(high_mask, (queue_weights[i] & 0x3ff) >> (32 - start_bit),
				    RTL839X_TBL_ACCESS_DATA_2(low_byte - 1));
	}

	rtl839x_write_scheduling_table(port);
	mutex_unlock(&priv->reg_mutex);
}

void rtldsa_838x_qos_init(struct rtl838x_switch_priv *priv)
{
	u32 v;

	pr_info("Setting up RTL838X QoS\n");
	pr_info("RTL838X_PRI_SEL_TBL_CTRL(i): %08x\n", sw_r32(RTL838X_PRI_SEL_TBL_CTRL(0)));
	rtl838x_setup_default_prio2queue();

	/* Enable inner (bit 12) and outer (bit 13) priority remapping from DSCP */
	sw_w32_mask(0, BIT(12) | BIT(13), RTL838X_PRI_DSCP_INVLD_CTRL0);

	/* Set default weight for calculating internal priority, in prio selection group 0
	 * Port based (prio 3), Port outer-tag (4), DSCP (5), Inner Tag (6), Outer Tag (7)
	 */
	v = 3 | (4 << 3) | (5 << 6) | (6 << 9) | (7 << 12);
	sw_w32(v, RTL838X_PRI_SEL_TBL_CTRL(0));

	/* Set the inner and outer priority one-to-one to re-marked outer dot1p priority */
	v = 0;
	for (int p = 0; p < 8; p++)
		v |= p << (3 * p);
	sw_w32(v, RTL838X_RMK_OPRI_CTRL);
	sw_w32(v, RTL838X_RMK_IPRI_CTRL);

	v = 0;
	for (int p = 0; p < 8; p++)
		v |= (dot1p_priority_remapping[p] & 0x7) << (p * 3);
	sw_w32(v, RTL838X_PRI_SEL_IPRI_REMAP);

	/* On all ports set scheduler type to WFQ */
	for (int i = 0; i <= priv->r->cpu_port; i++)
		sw_w32(0, RTL838X_SCHED_P_TYPE_CTRL(i));

	/* Enable egress scheduler for CPU-Port */
	sw_w32_mask(0, BIT(8), RTL838X_SCHED_LB_CTRL(priv->r->cpu_port));

	/* Enable egress drop allways on */
	sw_w32_mask(0, BIT(11), RTL838X_FC_P_EGR_DROP_CTRL(priv->r->cpu_port));

	/* Give special trap frames priority 7 (BPDUs) and routing exceptions: */
	sw_w32_mask(0, 7 << 3 | 7, RTL838X_QM_PKT2CPU_INTPRI_2);
	/* Give RMA frames priority 7: */
	sw_w32_mask(0, 7, RTL838X_QM_PKT2CPU_INTPRI_1);
}

void rtldsa_839x_qos_init(struct rtl838x_switch_priv *priv)
{
	u32 v;

	pr_info("Setting up RTL839X QoS\n");
	pr_info("RTL839X_PRI_SEL_TBL_CTRL(i): %08x\n", sw_r32(RTL839X_PRI_SEL_TBL_CTRL(0)));
	rtl839x_setup_default_prio2queue();

	for (int port = 0; port < priv->r->cpu_port; port++)
		sw_w32(7, RTL839X_QM_PORT_QNUM(port));

	/* CPU-port gets queue number 7 */
	sw_w32(7, RTL839X_QM_PORT_QNUM(priv->r->cpu_port));

	for (int port = 0; port <= priv->r->cpu_port; port++) {
		rtldsa_839x_set_ingress_priority(port, 0);
		rtl839x_set_scheduling_algorithm(priv, port, WEIGHTED_FAIR_QUEUE);
		rtl839x_set_scheduling_queue_weights(priv, port, rtldsa_default_queue_weights);
		/* Do re-marking based on outer tag */
		sw_w32_mask(0, BIT(port % 32), RTL839X_RMK_PORT_DEI_TAG_CTRL(port));
	}

	/* Remap dot1p priorities to internal priority, for this the outer tag needs be re-marked */
	v = 0;
	for (int p = 0; p < 8; p++)
		v |= (dot1p_priority_remapping[p] & 0x7) << (p * 3);
	sw_w32(v, RTL839X_PRI_SEL_IPRI_REMAP);

	/* Configure Drop Precedence for Drop Eligible Indicator (DEI)
	 * Index 0: 0
	 * Index 1: 2
	 * Each indicator is 2 bits long
	 */
	sw_w32(2 << 2, RTL839X_PRI_SEL_DEI2DP_REMAP);

	/* Re-mark DEI: 4 bit-fields of 2 bits each, field 0 is bits 0-1, ... */
	sw_w32((0x1 << 2) | (0x1 << 4), RTL839X_RMK_DEI_CTRL);

	/* Set Congestion avoidance drop probability to 0 for drop precedences 0-2 (bits 24-31)
	 * low threshold (bits 0-11) to 4095 and high threshold (bits 12-23) to 4095
	 * Weighted Random Early Detection (WRED) is used
	 */
	sw_w32(4095 << 12 | 4095, RTL839X_WRED_PORT_THR_CTRL(0));
	sw_w32(4095 << 12 | 4095, RTL839X_WRED_PORT_THR_CTRL(1));
	sw_w32(4095 << 12 | 4095, RTL839X_WRED_PORT_THR_CTRL(2));

	/* Set queue-based congestion avoidance properties, register fields are as
	 * for forward RTL839X_WRED_PORT_THR_CTRL
	 */
	for (int q = 0; q < 8; q++) {
		sw_w32(255 << 24 | 78 << 12 | 68, RTL839X_WRED_QUEUE_THR_CTRL(q, 0));
		sw_w32(255 << 24 | 74 << 12 | 64, RTL839X_WRED_QUEUE_THR_CTRL(q, 0));
		sw_w32(255 << 24 | 70 << 12 | 60, RTL839X_WRED_QUEUE_THR_CTRL(q, 0));
	}
}
