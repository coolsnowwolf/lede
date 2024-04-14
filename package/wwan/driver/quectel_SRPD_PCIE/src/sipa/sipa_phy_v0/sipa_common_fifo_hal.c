#include <linux/dma-mapping.h>
#include <linux/delay.h>

#include "../../include/sipa.h"
#include "../sipa_core.h"
#include "sipa_fifo_phy.h"

#define PTR_MASK(depth) (depth | (depth - 1))

static inline int
ipa_put_pkt_to_cache_rx_fifo(struct device *dev,
			     struct sipa_cmn_fifo_cfg_tag *fifo_cfg,
			     struct sipa_node_description_tag *desc,
			     u32 num)
{
	dma_addr_t dma_addr;
	u32 tmp = 0, tmp1 = 0, ret = 0, index = 0, left_cnt = 0;
	ssize_t node_size = sizeof(struct sipa_node_description_tag);
	struct sipa_node_description_tag *node =
			(struct sipa_node_description_tag *)
			fifo_cfg->rx_fifo.virtual_addr;

	dma_addr = fifo_cfg->rx_fifo.fifo_base_addr_l;

	left_cnt = fifo_cfg->rx_fifo.depth -
		ipa_phy_get_rx_fifo_filled_depth(fifo_cfg->fifo_reg_base);

	if (!left_cnt)
		return -ENOSPC;

	if (left_cnt < num)
		num = left_cnt;

	index = fifo_cfg->rx_fifo.wr & (fifo_cfg->rx_fifo.depth - 1);
	if (index + num <= fifo_cfg->rx_fifo.depth) {
		memcpy(node + index, desc, node_size * num);
	} else {
		tmp = fifo_cfg->rx_fifo.depth - index;
		memcpy(node + index, desc, tmp * node_size);
		tmp1 = num - tmp;
		memcpy(node, desc + tmp, tmp1 * node_size);
	}

	fifo_cfg->rx_fifo.wr = (fifo_cfg->rx_fifo.wr + num) &
		PTR_MASK(fifo_cfg->rx_fifo.depth);
	smp_wmb();
	ret = ipa_phy_update_rx_fifo_wptr(fifo_cfg->fifo_reg_base,
					  fifo_cfg->rx_fifo.wr);

	if (!ret)
		pr_err("sipa_phy_update_rx_fifo_rptr fail\n");

	return num;
}

static inline u32
ipa_recv_pkts_from_tx_fifo(struct device *dev,
			   struct sipa_cmn_fifo_cfg_tag *fifo_cfg,
			   u32 num)
{
	return ipa_phy_get_tx_fifo_filled_depth(fifo_cfg->fifo_reg_base);
}

static int ipa_common_fifo_hal_open(enum sipa_cmn_fifo_index id,
				    struct sipa_cmn_fifo_cfg_tag *cfg_base,
				    void *cookie)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo = NULL;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("sipa don't have this id %d\n", id);
		return -EINVAL;
	}

	if (ipa_term_fifo->state) {
		pr_err("sipa fifo_id = %d has already opened state = %d\n",
		       ipa_term_fifo->fifo_id, ipa_term_fifo->state);
		return -EBUSY;
	}

	ipa_phy_set_rx_fifo_total_depth(ipa_term_fifo->fifo_reg_base,
					ipa_term_fifo->rx_fifo.depth);
	ipa_phy_set_rx_fifo_addr(ipa_term_fifo->fifo_reg_base,
				 ipa_term_fifo->rx_fifo.fifo_base_addr_l,
				 ipa_term_fifo->rx_fifo.fifo_base_addr_h);

	ipa_phy_set_tx_fifo_total_depth(ipa_term_fifo->fifo_reg_base,
					ipa_term_fifo->tx_fifo.depth);
	ipa_phy_set_tx_fifo_addr(ipa_term_fifo->fifo_reg_base,
				 ipa_term_fifo->tx_fifo.fifo_base_addr_l,
				 ipa_term_fifo->tx_fifo.fifo_base_addr_h);

	ipa_phy_set_cur_term_num(ipa_term_fifo->fifo_reg_base,
				 ipa_term_fifo->cur);
	ipa_phy_set_dst_term_num(ipa_term_fifo->fifo_reg_base,
				 ipa_term_fifo->dst);

	ipa_phy_update_rx_fifo_rptr(ipa_term_fifo->fifo_reg_base, 0);
	ipa_phy_update_rx_fifo_wptr(ipa_term_fifo->fifo_reg_base, 0);
	ipa_phy_update_tx_fifo_rptr(ipa_term_fifo->fifo_reg_base, 0);
	ipa_phy_update_tx_fifo_wptr(ipa_term_fifo->fifo_reg_base, 0);
	ipa_phy_clear_stop_receive(ipa_term_fifo->fifo_reg_base);

	ipa_term_fifo->rx_fifo.rd = 0;
	ipa_term_fifo->rx_fifo.wr = 0;
	ipa_term_fifo->tx_fifo.rd = 0;
	ipa_term_fifo->tx_fifo.wr = 0;

	ipa_term_fifo->fifo_name = cookie;
	ipa_term_fifo->state = true;

	return 0;
}

static int ipa_common_fifo_hal_close(enum sipa_cmn_fifo_index id,
				     struct sipa_cmn_fifo_cfg_tag *cfg_base)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("sipa don't have this id %d\n", id);
		return -EINVAL;
	}

	ipa_phy_set_rx_fifo_total_depth(ipa_term_fifo->fifo_reg_base, 0);
	ipa_phy_set_tx_fifo_total_depth(ipa_term_fifo->fifo_reg_base, 0);

	ipa_phy_update_rx_fifo_rptr(ipa_term_fifo->fifo_reg_base, 0);
	ipa_phy_update_rx_fifo_wptr(ipa_term_fifo->fifo_reg_base, 0);
	ipa_phy_update_tx_fifo_rptr(ipa_term_fifo->fifo_reg_base, 0);
	ipa_phy_update_tx_fifo_wptr(ipa_term_fifo->fifo_reg_base, 0);

	ipa_term_fifo->rx_fifo.rd = 0;
	ipa_term_fifo->rx_fifo.wr = 0;
	ipa_term_fifo->tx_fifo.rd = 0;
	ipa_term_fifo->tx_fifo.wr = 0;

	ipa_term_fifo->state = 0;

	return 0;
}

static int
ipa_common_fifo_hal_set_rx_depth(enum sipa_cmn_fifo_index id,
				 struct sipa_cmn_fifo_cfg_tag *cfg_base,
				 u32 depth)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("sipa don't have this id %d\n", id);
		return -EINVAL;
	}

	return ipa_phy_set_rx_fifo_total_depth(ipa_term_fifo->fifo_reg_base,
					       depth);
}

static u32
ipa_common_fifo_hal_get_rx_depth(enum sipa_cmn_fifo_index id,
				 struct sipa_cmn_fifo_cfg_tag *cfg_base)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("sipa don't have this id %d\n", id);
		return 0;
	}

	return ipa_phy_get_rx_fifo_total_depth(ipa_term_fifo->fifo_reg_base);
}

static int
ipa_common_fifo_hal_set_tx_depth(enum sipa_cmn_fifo_index id,
				 struct sipa_cmn_fifo_cfg_tag *cfg_base,
				 u32 depth)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("sipa don't have this id %d\n", id);
		return 0;
	}

	return ipa_phy_set_tx_fifo_total_depth(ipa_term_fifo->fifo_reg_base,
					       depth);
}

static u32
ipa_common_fifo_hal_get_tx_depth(enum sipa_cmn_fifo_index id,
				 struct sipa_cmn_fifo_cfg_tag *cfg_base)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("sipa don't have this id %d\n", id);
		return 0;
	}

	return ipa_phy_get_tx_fifo_total_depth(ipa_term_fifo->fifo_reg_base);
}

static int
ipa_common_fifo_hal_set_intr_drop_packet(enum sipa_cmn_fifo_index id,
					 struct sipa_cmn_fifo_cfg_tag *cfg_base,
					 u32 enable, sipa_irq_notify_cb cb)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("sipa don't have this id %d\n", id);
		return -EINVAL;
	}

	if (enable)
		return ipa_phy_enable_int_bit(ipa_term_fifo->fifo_reg_base,
					      IPA_DROP_PACKET_OCCUR_INT_EN);
	else
		return ipa_phy_disable_int_bit(ipa_term_fifo->fifo_reg_base,
					       IPA_DROP_PACKET_OCCUR_INT_EN);
}

static int
ipa_common_fifo_hal_set_intr_error_code(enum sipa_cmn_fifo_index id,
					struct sipa_cmn_fifo_cfg_tag *cfg_base,
					u32 enable, sipa_irq_notify_cb cb)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("sipa don't have this id %d\n", id);
		return -EINVAL;
	}

	if (enable)
		return ipa_phy_enable_int_bit(ipa_term_fifo->fifo_reg_base,
					      IPA_ERRORCODE_IN_TX_FIFO_EN);
	else
		return ipa_phy_disable_int_bit(ipa_term_fifo->fifo_reg_base,
					       IPA_ERRORCODE_IN_TX_FIFO_EN);
}

static int
ipa_common_fifo_hal_set_intr_timeout(enum sipa_cmn_fifo_index id,
				     struct sipa_cmn_fifo_cfg_tag *cfg_base,
				     u32 enable, u32 time,
				     sipa_irq_notify_cb cb)
{
	int ret;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("sipa don't have this id %d\n", id);
		return -EINVAL;
	}

	if (enable) {
		ret = ipa_phy_set_tx_fifo_interrupt_delay_timer(
				  ipa_term_fifo->fifo_reg_base,
				  time);
		if (ret)
			ret = ipa_phy_enable_int_bit(
					  ipa_term_fifo->fifo_reg_base,
					  IPA_TXFIFO_INT_DELAY_TIMER_SW_EN);
		else
			pr_err("fifo(%d) set timeout threshold fail\n", id);
	} else {
		ret = ipa_phy_disable_int_bit(ipa_term_fifo->fifo_reg_base,
					      IPA_TXFIFO_INT_DELAY_TIMER_SW_EN);
	}

	return ret;
}

static int
ipa_common_fifo_hal_set_hw_intr_timeout(enum sipa_cmn_fifo_index id,
					struct sipa_cmn_fifo_cfg_tag *cfg_base,
					u32 enable, u32 time,
					sipa_irq_notify_cb cb)
{
	int ret;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	if (enable) {
		ret = ipa_phy_set_tx_fifo_interrupt_delay_timer(
			ipa_term_fifo->fifo_reg_base, time);
		if (!ret)
			ret = ipa_phy_enable_int_bit(
				ipa_term_fifo->fifo_reg_base,
				IPA_TX_FIFO_DELAY_TIMER_EN);
		else
			pr_err("fifo(%d) set timeout threshold fail\n", id);
	} else {
		ret = ipa_phy_disable_int_bit(ipa_term_fifo->fifo_reg_base,
					      IPA_TX_FIFO_DELAY_TIMER_EN);
	}

	return ret;
}

static int
ipa_common_fifo_hal_set_intr_threshold(enum sipa_cmn_fifo_index id,
				       struct sipa_cmn_fifo_cfg_tag *cfg_base,
				       u32 enable, u32 cnt,
				       sipa_irq_notify_cb cb)
{
	int ret;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	if (enable) {
		ret = ipa_phy_set_tx_fifo_interrupt_threshold(
			ipa_term_fifo->fifo_reg_base, cnt);
		if (!ret) {
			ret = ipa_phy_enable_int_bit(
				ipa_term_fifo->fifo_reg_base,
				IPA_TXFIFO_INT_THRESHOLD_ONESHOT_EN);
		} else {
			pr_err("fifo(%d) set threshold fail\n", id);
		}
	} else {
		ret =
		ipa_phy_disable_int_bit(ipa_term_fifo->fifo_reg_base,
					IPA_TXFIFO_INT_THRESHOLD_ONESHOT_EN);
	}

	return ret;
}

static int
ipa_common_fifo_hal_set_hw_intr_thres(enum sipa_cmn_fifo_index id,
				      struct sipa_cmn_fifo_cfg_tag *cfg_base,
				      u32 enable, u32 cnt,
				      sipa_irq_notify_cb cb)
{
	int ret;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	if (enable) {
		ret = ipa_phy_set_tx_fifo_interrupt_threshold(
			ipa_term_fifo->fifo_reg_base, cnt);
		if (!ret)
			ret = ipa_phy_enable_int_bit(
				ipa_term_fifo->fifo_reg_base,
				IPA_TX_FIFO_THRESHOLD_EN);
		else
			pr_err("fifo(%d) set threshold fail\n", id);
	} else {
		ret = ipa_phy_disable_int_bit(ipa_term_fifo->fifo_reg_base,
					      IPA_TX_FIFO_THRESHOLD_EN);
	}

	return ret;
}

static int
ipa_common_fifo_hal_set_src_dst_term(enum sipa_cmn_fifo_index id,
				     struct sipa_cmn_fifo_cfg_tag *cfg_base,
				     u32 src, u32 dst)
{
	int ret;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	ret = ipa_phy_set_cur_term_num(ipa_term_fifo->fifo_reg_base, src);
	if (ret) {
		pr_err("fifo %d set cur failed\n", id);
		return ret;
	}

	ret = ipa_phy_set_dst_term_num(ipa_term_fifo->fifo_reg_base, dst);
	if (ret) {
		pr_err("fifo %d set dst failed\n", id);
		return ret;
	}

	return 0;
}

static int
ipa_common_fifo_hal_enable_local_flowctrl_intr(enum sipa_cmn_fifo_index id,
					       struct sipa_cmn_fifo_cfg_tag *
					       cfg_base, u32 enable,
					       u32 irq_mode,
					       sipa_irq_notify_cb cb)
{
	int ret;
	u32 irq;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	switch (irq_mode) {
	case 0:
		irq = IPA_RX_FIFO_INT_EXIT_FLOW_CTRL_EN;
		break;
	case 1:
		irq = IPA_RX_FIFO_INT_ENTER_FLOW_CTRL_EN;
		break;
	case 2:
		irq = IPA_RX_FIFO_INT_EXIT_FLOW_CTRL_EN |
			  IPA_RX_FIFO_INT_ENTER_FLOW_CTRL_EN;
		break;
	default:
		pr_err("don't have this %d irq type\n", irq_mode);
		return -EINVAL;
	}

	if (enable) {
		ret = ipa_phy_enable_int_bit(ipa_term_fifo->fifo_reg_base, irq);
		if (ret) {
			pr_err("fifo_id = %d irq_mode = %d set failed\n",
			       id, irq);
			return ret;
		}
	} else {
		ret = ipa_phy_disable_int_bit(ipa_term_fifo->fifo_reg_base,
					      irq);
	}

	return ret;
}

static int
ipa_common_fifo_hal_enable_remote_flowctrl_intr(enum sipa_cmn_fifo_index id,
						struct sipa_cmn_fifo_cfg_tag *
						cfg_base, u32 work_mode,
						u32 tx_entry_watermark,
						u32 tx_exit_watermark,
						u32 rx_entry_watermark,
						u32 rx_exit_watermark)
{
	int ret;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	ret = ipa_phy_set_tx_fifo_exit_flow_ctrl_watermark(
		ipa_term_fifo->fifo_reg_base, tx_exit_watermark);
	if (unlikely(!ret)) {
		pr_err("fifo_id = %d tx_exit_watermark(0x%x) failed\n",
		       id, tx_exit_watermark);
		return ret;
	}

	ret = ipa_phy_set_tx_fifo_entry_flow_ctrl_watermark(
		ipa_term_fifo->fifo_reg_base, tx_entry_watermark);
	if (unlikely(!ret)) {
		pr_err("fifo_id = %d tx_entry_watermark(0x%x) failed\n",
		       id, tx_entry_watermark);
		return ret;
	}

	ret = ipa_phy_set_rx_fifo_exit_flow_ctrl_watermark(
		ipa_term_fifo->fifo_reg_base, rx_exit_watermark);
	if (unlikely(!ret)) {
		pr_err("fifo_id = %d rx_exit_watermark(0x%x) failed\n",
		       id, rx_exit_watermark);
		return ret;
	}

	ret = ipa_phy_set_rx_fifo_entry_flow_ctrl_watermark(
		ipa_term_fifo->fifo_reg_base, rx_entry_watermark);
	if (unlikely(!ret)) {
		pr_err("fifo_id = %d rx_entry_watermark(0x%x) failed\n",
		       id, rx_entry_watermark);
		return ret;
	}

	ret = ipa_phy_set_flow_ctrl_config(ipa_term_fifo->fifo_reg_base,
					   work_mode);

	return ret;
}

static int
ipa_common_fifo_hal_set_interrupt_intr(enum sipa_cmn_fifo_index id,
				       struct sipa_cmn_fifo_cfg_tag *cfg_base,
				       u32 enable,
				       sipa_irq_notify_cb cb)
{
	int ret;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	if (enable) {
		ret = ipa_phy_enable_int_bit(ipa_term_fifo->fifo_reg_base,
					     IPA_TX_FIFO_INTR_SW_BIT_EN);
	} else {
		ret = ipa_phy_disable_int_bit(ipa_term_fifo->fifo_reg_base,
					      IPA_TX_FIFO_INTR_SW_BIT_EN);
	}

	return ret;
}

static int
ipa_common_fifo_hal_set_intr_txfifo_overflow(enum sipa_cmn_fifo_index id,
					     struct sipa_cmn_fifo_cfg_tag *
					     cfg_base, u32 enable,
					     sipa_irq_notify_cb cb)
{
	int ret;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	if (enable) {
		ret = ipa_phy_enable_int_bit(ipa_term_fifo->fifo_reg_base,
					     IPA_TXFIFO_OVERFLOW_EN);
	} else {
		ret = ipa_phy_disable_int_bit(ipa_term_fifo->fifo_reg_base,
					      IPA_TXFIFO_OVERFLOW_EN);
	}

	return ret;
}

static int
ipa_common_fifo_hal_set_intr_txfifo_full(enum sipa_cmn_fifo_index id,
					 struct sipa_cmn_fifo_cfg_tag *cfg_base,
					 u32 enable, sipa_irq_notify_cb cb)
{
	int ret;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	if (enable) {
		ret = ipa_phy_enable_int_bit(ipa_term_fifo->fifo_reg_base,
					     IPA_TXFIFO_FULL_INT_EN);
	} else {
		ret = ipa_phy_disable_int_bit(ipa_term_fifo->fifo_reg_base,
					      IPA_TXFIFO_FULL_INT_EN);
	}

	return ret;
}

/**
 * Description: Receive Node from tx fifo.
 * Input:
 *   @id: The FIFO id that need to be operated.
 *   @pkt: The node that need to be stored address.
 *   @num: The num of receive.
 * OUTPUT:
 *   @The num that has be received from tx fifo successful.
 * Note:
 */
static int
ipa_common_fifo_hal_put_node_to_rx_fifo(struct device *dev,
					enum sipa_cmn_fifo_index id,
					struct sipa_cmn_fifo_cfg_tag *cfg_base,
					struct sipa_node_description_tag *node,
					u32 force_intr, u32 num)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	return ipa_put_pkt_to_cache_rx_fifo(dev, ipa_term_fifo,
					    node, num);
}

static u32
ipa_common_fifo_hal_get_left_cnt(enum sipa_cmn_fifo_index id,
				 struct sipa_cmn_fifo_cfg_tag *cfg_base)
{
	u32 left_cnt;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo = NULL;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return 0;
	}

	left_cnt =
		ipa_phy_get_tx_fifo_total_depth(ipa_term_fifo->fifo_reg_base) -
		ipa_phy_get_tx_fifo_filled_depth(ipa_term_fifo->fifo_reg_base);

	return left_cnt;
}

/*
 * Description: Send Node to rx fifo.
 * Input:
 *   id: The FIFO id that need to be operated.
 *   pkt: The node address that need send to rx fifo.
 *   num: The number of need to send.
 * OUTPUT:
 *   The number that has get from tx fifo successful.
 * Note:
 */
static u32
ipa_common_fifo_hal_recv_node_from_tx_fifo(struct device *dev,
					   enum sipa_cmn_fifo_index id,
					   struct sipa_cmn_fifo_cfg_tag *
					   cfg_base, u32 num)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return 0;
	}

	return ipa_recv_pkts_from_tx_fifo(dev, ipa_term_fifo, num);
}

static void
ipa_common_fifo_hal_get_rx_ptr(enum sipa_cmn_fifo_index id,
			       struct sipa_cmn_fifo_cfg_tag *cfg_base,
			       u32 *wr, u32 *rd)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return;
	}

	if (rd)
		*rd = ipa_phy_get_rx_fifo_rptr(ipa_term_fifo->fifo_reg_base);
	if (wr)
		*wr = ipa_phy_get_rx_fifo_wptr(ipa_term_fifo->fifo_reg_base);
}

static void
ipa_common_fifo_hal_get_tx_ptr(enum sipa_cmn_fifo_index id,
			       struct sipa_cmn_fifo_cfg_tag *cfg_base,
			       u32 *wr, u32 *rd)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return;
	}

	if (rd)
		*rd = ipa_phy_get_tx_fifo_rptr(ipa_term_fifo->fifo_reg_base);
	if (wr)
		*wr = ipa_phy_get_tx_fifo_wptr(ipa_term_fifo->fifo_reg_base);
}

static void
ipa_common_fifo_hal_get_filled_depth(enum sipa_cmn_fifo_index id,
				     struct sipa_cmn_fifo_cfg_tag *cfg_base,
				     u32 *rx_filled, u32 *tx_filled)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;
	void __iomem *reg_base;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return;
	}

	reg_base = ipa_term_fifo->fifo_reg_base;
	if (tx_filled)
		*tx_filled = ipa_phy_get_tx_fifo_filled_depth(reg_base);
	if (rx_filled)
		*rx_filled = ipa_phy_get_rx_fifo_filled_depth(reg_base);
}

static u32
ipa_common_fifo_hal_get_tx_full_status(enum sipa_cmn_fifo_index id,
				       struct sipa_cmn_fifo_cfg_tag *cfg_base)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return 0;
	}

	return ipa_phy_get_tx_fifo_full_status(ipa_term_fifo->fifo_reg_base);
}

static u32
ipa_common_fifo_hal_get_tx_empty_status(enum sipa_cmn_fifo_index id,
					struct sipa_cmn_fifo_cfg_tag *cfg_base)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return 0;
	}

	return ipa_phy_get_tx_fifo_empty_status(ipa_term_fifo->fifo_reg_base);
}

static u32
ipa_common_fifo_hal_get_rx_full_status(enum sipa_cmn_fifo_index id,
				       struct sipa_cmn_fifo_cfg_tag *cfg_base)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return 0;
	}

	return ipa_phy_get_rx_fifo_full_status(ipa_term_fifo->fifo_reg_base);
}

static u32
ipa_common_fifo_hal_get_rx_empty_status(enum sipa_cmn_fifo_index id,
					struct sipa_cmn_fifo_cfg_tag *cfg_base)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return 0;
	}

	return ipa_phy_get_rx_fifo_empty_status(ipa_term_fifo->fifo_reg_base);
}

static bool
ipa_common_fifo_hal_set_rx_fifo_wptr(enum sipa_cmn_fifo_index id,
				     struct sipa_cmn_fifo_cfg_tag *cfg_base,
				     u32 wptr)
{
	u32 ret;
	u32 rx_wptr;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return false;
	}

	rx_wptr = ipa_phy_get_rx_fifo_wptr(ipa_term_fifo->fifo_reg_base);

	if (wptr != rx_wptr) {
		wptr = wptr & PTR_MASK(ipa_term_fifo->rx_fifo.depth);
		ipa_term_fifo->rx_fifo.wr = wptr;
		ret = ipa_phy_update_rx_fifo_wptr(ipa_term_fifo->fifo_reg_base,
						  wptr);
		if (!ret) {
			pr_err("fifo id = %d update rx fifo wptr = 0x%x failed !!!",
			       id, wptr);
			return false;
		}
	}

	return true;
}

static bool
ipa_common_fifo_hal_set_tx_fifo_wptr(enum sipa_cmn_fifo_index id,
				     struct sipa_cmn_fifo_cfg_tag *cfg_base,
				     u32 wptr)
{
	u32 ret;
	u32 tx_wptr;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return false;
	}

	tx_wptr = ipa_phy_get_tx_fifo_wptr(ipa_term_fifo->fifo_reg_base);

	if (wptr != tx_wptr) {
		wptr = wptr & PTR_MASK(ipa_term_fifo->rx_fifo.depth);
		ipa_term_fifo->tx_fifo.wr = wptr;
		ret = ipa_phy_update_tx_fifo_wptr(ipa_term_fifo->fifo_reg_base,
						  wptr);
		if (!ret) {
			pr_err("fifo id = %d update tx fifo wptr = 0x%x failed !!!",
			       id, wptr);
			return false;
		}
	}

	return true;
}

static int
ipa_common_fifo_hal_set_rx_tx_fifo_ptr(enum sipa_cmn_fifo_index id,
				       struct sipa_cmn_fifo_cfg_tag *cfg_base,
				       u32 rx_rd, u32 rx_wr,
				       u32 tx_rd, u32 tx_wr)
{
	int ret = 0, ret1 = 0;
	u32 rx_rptr = 0, rx_wptr = 0;
	u32 tx_rptr = 0, tx_wptr = 0;
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	tx_wptr = ipa_phy_get_tx_fifo_wptr(ipa_term_fifo->fifo_reg_base);
	tx_rptr = ipa_phy_get_tx_fifo_rptr(ipa_term_fifo->fifo_reg_base);
	rx_wptr = ipa_phy_get_rx_fifo_wptr(ipa_term_fifo->fifo_reg_base);
	rx_rptr = ipa_phy_get_rx_fifo_rptr(ipa_term_fifo->fifo_reg_base);

	if (rx_rd != rx_rptr) {
		rx_rd = rx_rd & PTR_MASK(ipa_term_fifo->rx_fifo.depth);
		ipa_term_fifo->rx_fifo.rd = rx_rd;
		ret = ipa_phy_update_rx_fifo_rptr(ipa_term_fifo->fifo_reg_base,
						  rx_rd);
		if (!ret) {
			ret1 = -EIO;
			pr_err("update rx fifo rptr = 0x%x failed !!!", rx_rd);
		}
	}

	if (rx_wr != rx_wptr) {
		rx_wr = rx_wr & PTR_MASK(ipa_term_fifo->rx_fifo.depth);
		ipa_term_fifo->rx_fifo.wr = rx_wr;
		ret = ipa_phy_update_rx_fifo_wptr(ipa_term_fifo->fifo_reg_base,
						  rx_wr);
		if (!ret) {
			ret1 = -EIO;
			pr_err("update rx fifo wptr = 0x%x failed !!!",	rx_wr);
		}
	}

	if (tx_rd != tx_rptr) {
		tx_rd = tx_rd & PTR_MASK(ipa_term_fifo->rx_fifo.depth);
		ipa_term_fifo->tx_fifo.rd = tx_rd;
		ret = ipa_phy_update_tx_fifo_rptr(ipa_term_fifo->fifo_reg_base,
						  tx_rd);
		if (!ret) {
			ret1 = -EIO;
			pr_err("update tx fifo rptr = 0x%x failed !!!", tx_rd);
		}
	}

	if (tx_wr != tx_wptr) {
		tx_wr = tx_wr & PTR_MASK(ipa_term_fifo->rx_fifo.depth);
		ipa_term_fifo->tx_fifo.wr = tx_wr;
		ret = ipa_phy_update_tx_fifo_wptr(ipa_term_fifo->fifo_reg_base,
						  tx_wr);
		if (!ret) {
			ret1 = -EIO;
			pr_err("update tx fifo wptr = 0x%x failed !!!", tx_wr);
		}
	}

	return ret1;
}

static int
ipa_common_fifo_hal_ctrl_receive(enum sipa_cmn_fifo_index id,
				 struct sipa_cmn_fifo_cfg_tag *cfg_base,
				 bool stop)
{
	struct sipa_cmn_fifo_cfg_tag *ipa_term_fifo;

	if (likely(id < SIPA_FIFO_MAX)) {
		ipa_term_fifo = cfg_base + id;
	} else {
		pr_err("don't have this id %d\n", id);
		return -EINVAL;
	}

	if (stop)
		return ipa_phy_stop_receive(ipa_term_fifo->fifo_reg_base);
	else
		return ipa_phy_clear_stop_receive(ipa_term_fifo->fifo_reg_base);
}

static struct sipa_node_description_tag *
ipa_common_fifo_hal_get_tx_fifo_rp(enum sipa_cmn_fifo_index id,
				   struct sipa_cmn_fifo_cfg_tag *cfg_base,
				   u32 index)
{
	u32 tmp;
	struct sipa_cmn_fifo_cfg_tag *fifo_cfg;
	struct sipa_node_description_tag *node;

	if (unlikely(id >= SIPA_FIFO_MAX))
		return NULL;

	fifo_cfg = cfg_base + id;
	node = (struct sipa_node_description_tag *)
		fifo_cfg->tx_fifo.virtual_addr;

	if (unlikely(!node))
		return NULL;

	tmp = (fifo_cfg->tx_fifo.rd + index) & (fifo_cfg->tx_fifo.depth - 1);

	return node + tmp;
}

static struct sipa_node_description_tag *
ipa_common_fifo_hal_get_rx_fifo_wr(enum sipa_cmn_fifo_index id,
				   struct sipa_cmn_fifo_cfg_tag *cfg_base,
				   u32 index)
{
	u32 tmp;
	struct sipa_cmn_fifo_cfg_tag *fifo_cfg;
	struct sipa_node_description_tag *node;

	if (unlikely(id >= SIPA_FIFO_MAX))
		return NULL;

	fifo_cfg = cfg_base + id;
	node = (struct sipa_node_description_tag *)
		fifo_cfg->rx_fifo.virtual_addr;

	if (unlikely(!node))
		return NULL;

	if (index >= fifo_cfg->rx_fifo.depth)
		return NULL;

	tmp = (fifo_cfg->rx_fifo.wr + index) & (fifo_cfg->rx_fifo.depth - 1);

	return node + tmp;
}

static int ipa_common_fifo_hal_set_tx_fifo_rp(enum sipa_cmn_fifo_index id,
					      struct sipa_cmn_fifo_cfg_tag *
					      cfg_base, u32 tx_rd)
{
	int ret;
	struct sipa_cmn_fifo_cfg_tag *fifo_cfg;

	if (unlikely(id >= SIPA_FIFO_MAX))
		return -EINVAL;

	fifo_cfg = cfg_base + id;
	fifo_cfg->tx_fifo.rd = (fifo_cfg->tx_fifo.rd + tx_rd) &
		PTR_MASK(fifo_cfg->tx_fifo.depth);
	ret = ipa_phy_update_tx_fifo_rptr(fifo_cfg->fifo_reg_base,
					  fifo_cfg->tx_fifo.rd);

	if (ret) {
		pr_err("update tx fifo rptr fail !!!\n");
		return -EINVAL;
	}

	return 0;
}

static int ipa_common_fifo_hal_set_rx_fifo_wr(struct device *dev,
					      enum sipa_cmn_fifo_index id,
					      struct sipa_cmn_fifo_cfg_tag *
					      cfg_base, u32 num)
{
	int ret;
	dma_addr_t dma_addr;
	struct sipa_cmn_fifo_cfg_tag *fifo_cfg;
	u32 tmp = 0, tmp1 = 0, index = 0, left_cnt = 0;
	ssize_t node_size = sizeof(struct sipa_node_description_tag);

	if (unlikely(id >= SIPA_FIFO_MAX))
		return -EINVAL;

	fifo_cfg = cfg_base + id;
	dma_addr = fifo_cfg->rx_fifo.fifo_base_addr_l;

	left_cnt = fifo_cfg->rx_fifo.depth -
		ipa_phy_get_rx_fifo_filled_depth(fifo_cfg->fifo_reg_base);

	if (!left_cnt)
		return -ENOSPC;

	if (left_cnt < num)
		num = left_cnt;

	index = fifo_cfg->rx_fifo.wr & (fifo_cfg->rx_fifo.depth - 1);
	if (index + num <= fifo_cfg->rx_fifo.depth) {
		dma_sync_single_for_device(dev, dma_addr + index * node_size,
					   node_size * num, DMA_TO_DEVICE);
	} else {
		tmp = fifo_cfg->rx_fifo.depth - index;
		dma_sync_single_for_device(dev, dma_addr + index * node_size,
					   node_size * tmp, DMA_TO_DEVICE);
		tmp1 = num - tmp;
		dma_sync_single_for_device(dev, dma_addr,
					   node_size * tmp1, DMA_TO_DEVICE);
	}

	fifo_cfg->rx_fifo.wr = (fifo_cfg->rx_fifo.wr + num) &
		PTR_MASK(fifo_cfg->rx_fifo.depth);
	ret = ipa_phy_update_rx_fifo_wptr(fifo_cfg->fifo_reg_base,
					  fifo_cfg->rx_fifo.wr);

	if (ret) {
		pr_err("update rx fifo rptr fail !!!\n");
		return -EIO;
	}

	return 0;
}

static int ipa_common_fifo_set_intr_eb(enum sipa_cmn_fifo_index id,
				       struct sipa_cmn_fifo_cfg_tag *cfg_base,
				       bool eb, u32 type)
{
	struct sipa_cmn_fifo_cfg_tag *fifo_cfg;

	if (unlikely(id >= SIPA_FIFO_MAX))
		return -EINVAL;

	fifo_cfg = cfg_base + id;

	if (eb)
		return ipa_phy_enable_int_bit(fifo_cfg->fifo_reg_base, type);
	else
		return ipa_phy_disable_int_bit(fifo_cfg->fifo_reg_base, type);
}

static void ipa_common_fifo_clr_tout_th_intr(enum sipa_cmn_fifo_index id,
					     struct sipa_cmn_fifo_cfg_tag *
					     cfg_base)
{
	struct sipa_cmn_fifo_cfg_tag *fifo_cfg;

	if (unlikely(id >= SIPA_FIFO_MAX))
		return;

	fifo_cfg = cfg_base + id;

	ipa_phy_clear_int(fifo_cfg->fifo_reg_base,
			  IPA_TX_FIFO_TIMER_CLR_BIT |
			  IPA_TX_FIFO_THRESHOLD_CLR_BIT);
}

void sipa_fifo_ops_init(struct sipa_fifo_hal_ops *ops)
{
	ops->open = ipa_common_fifo_hal_open;

	ops->close = ipa_common_fifo_hal_close;

	ops->enable_remote_flowctrl_intr =
		ipa_common_fifo_hal_enable_remote_flowctrl_intr;

	ops->enable_local_flowctrl_intr =
		ipa_common_fifo_hal_enable_local_flowctrl_intr;

	ops->get_left_cnt = ipa_common_fifo_hal_get_left_cnt;

	ops->put_node_to_rx_fifo =
		ipa_common_fifo_hal_put_node_to_rx_fifo;

	ops->recv_node_from_tx_fifo =
		ipa_common_fifo_hal_recv_node_from_tx_fifo;

	ops->set_intr_drop_packet =
		ipa_common_fifo_hal_set_intr_drop_packet;

	ops->set_intr_error_code =
		ipa_common_fifo_hal_set_intr_error_code;

	ops->set_intr_threshold =
		ipa_common_fifo_hal_set_intr_threshold;

	ops->set_intr_timeout =
		ipa_common_fifo_hal_set_intr_timeout;

	ops->set_hw_intr_thres =
		ipa_common_fifo_hal_set_hw_intr_thres;

	ops->set_hw_intr_timeout =
		ipa_common_fifo_hal_set_hw_intr_timeout;

	ops->set_interrupt_intr =
		ipa_common_fifo_hal_set_interrupt_intr;

	ops->set_intr_txfifo_full =
		ipa_common_fifo_hal_set_intr_txfifo_full;

	ops->set_intr_txfifo_overflow =
		ipa_common_fifo_hal_set_intr_txfifo_overflow;

	ops->set_rx_depth =
		ipa_common_fifo_hal_set_rx_depth;

	ops->set_tx_depth =
		ipa_common_fifo_hal_set_tx_depth;

	ops->get_rx_depth =
		ipa_common_fifo_hal_get_rx_depth;

	ops->get_tx_depth =
		ipa_common_fifo_hal_get_tx_depth;

	ops->get_tx_ptr =
		ipa_common_fifo_hal_get_tx_ptr;

	ops->get_rx_ptr =
		ipa_common_fifo_hal_get_rx_ptr;

	ops->get_filled_depth =
		ipa_common_fifo_hal_get_filled_depth;

	ops->get_tx_empty_status =
		ipa_common_fifo_hal_get_tx_empty_status;

	ops->get_tx_full_status =
		ipa_common_fifo_hal_get_tx_full_status;

	ops->get_rx_empty_status =
		ipa_common_fifo_hal_get_rx_empty_status;

	ops->get_rx_full_status =
		ipa_common_fifo_hal_get_rx_full_status;

	ops->set_rx_tx_fifo_ptr =
		ipa_common_fifo_hal_set_rx_tx_fifo_ptr;

	ops->set_tx_fifo_wptr =
		ipa_common_fifo_hal_set_tx_fifo_wptr;

	ops->set_rx_fifo_wptr =
		ipa_common_fifo_hal_set_rx_fifo_wptr;

	ops->set_src_dst_term =
		ipa_common_fifo_hal_set_src_dst_term;

	ops->ctrl_receive =
		ipa_common_fifo_hal_ctrl_receive;

	ops->set_tx_fifo_rp = ipa_common_fifo_hal_set_tx_fifo_rp;
	ops->get_tx_fifo_rp = ipa_common_fifo_hal_get_tx_fifo_rp;
	ops->get_rx_fifo_wr = ipa_common_fifo_hal_get_rx_fifo_wr;
	ops->set_rx_fifo_wr = ipa_common_fifo_hal_set_rx_fifo_wr;

	ops->set_intr_eb = ipa_common_fifo_set_intr_eb;
	ops->clr_tout_th_intr = ipa_common_fifo_clr_tout_th_intr;
}
EXPORT_SYMBOL(sipa_fifo_ops_init);
