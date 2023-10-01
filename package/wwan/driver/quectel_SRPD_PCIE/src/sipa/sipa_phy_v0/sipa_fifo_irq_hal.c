#include "../../include/sipa.h"

#include "../sipa_core.h"
#include "sipa_fifo_phy.h"

static void ipa_fifo_traverse_int_bit(enum sipa_cmn_fifo_index id,
				      struct sipa_cmn_fifo_cfg_tag *ipa_cfg)
{
	void __iomem *fifo_base;
	u32 clr_sts = 0;
	u32 int_status = 0;

	fifo_base = ipa_cfg->fifo_reg_base;
	int_status = ipa_phy_get_fifo_all_int_sts(fifo_base);

	if (!(int_status & IPA_INT_STS_GROUP))
		return;

	if (int_status & IPA_INT_EXIT_FLOW_CTRL_STS) {
		ipa_cfg->exit_flow_ctrl_cnt++;
		clr_sts |= IPA_EXIT_FLOW_CONTROL_CLR_BIT;
	}

	if (int_status & IPA_INT_ERRORCODE_IN_TX_FIFO_STS)
		clr_sts |= IPA_ERROR_CODE_INTR_CLR_BIT;

	if (int_status & IPA_INT_ENTER_FLOW_CTRL_STS) {
		ipa_cfg->enter_flow_ctrl_cnt++;
		clr_sts |= IPA_ENTRY_FLOW_CONTROL_CLR_BIT;
	}

	if (int_status & IPA_INT_INTR_BIT_STS)
		clr_sts |= IPA_TX_FIFO_INTR_CLR_BIT;

	if (int_status & IPA_INT_THRESHOLD_STS ||
	    int_status & IPA_INT_DELAY_TIMER_STS) {
		ipa_phy_disable_int_bit(ipa_cfg->fifo_reg_base,
					IPA_TX_FIFO_THRESHOLD_EN |
					IPA_TX_FIFO_DELAY_TIMER_EN);
		clr_sts |= IPA_TX_FIFO_THRESHOLD_CLR_BIT |
			IPA_TX_FIFO_TIMER_CLR_BIT;
	}

	if (int_status & IPA_INT_DROP_PACKT_OCCUR)
		clr_sts |= IPA_DROP_PACKET_INTR_CLR_BIT;

	if (int_status & IPA_INT_TXFIFO_OVERFLOW_STS)
		clr_sts |= IPA_TX_FIFO_OVERFLOW_CLR_BIT;

	if (int_status & IPA_INT_TXFIFO_FULL_INT_STS)
		clr_sts |= IPA_TX_FIFO_FULL_INT_CLR_BIT;

	if (ipa_cfg->irq_cb)
		ipa_cfg->irq_cb(ipa_cfg->priv, int_status, id);
	else
		pr_err("Don't register this fifo(%d) irq callback\n", id);

	ipa_phy_clear_int(ipa_cfg->fifo_reg_base, clr_sts);
}

int sipa_int_callback_func(int evt, void *cookie)
{
	struct sipa_core *ipa = sipa_get_ctrl_pointer();

	if (ipa->remote_ready) {
		ipa_fifo_traverse_int_bit(SIPA_FIFO_PCIE_DL,
					  &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_DL]);
		ipa_fifo_traverse_int_bit(SIPA_FIFO_PCIE_UL,
					  &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_UL]);
	}

	return 0;
}
EXPORT_SYMBOL(sipa_int_callback_func);
