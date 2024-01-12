#ifndef _IPA_CP0_FIFO_PHY_H_
#define _IPA_CP0_FIFO_PHY_H_

#include <linux/bitops.h>

/* Common fifo reg */
#define IPA_COMMON_RX_FIFO_DEPTH		0x00UL
#define IPA_COMMON_RX_FIFO_WR			0x04UL
#define IPA_COMMON_RX_FIFO_RD			0x08UL
#define IPA_COMMON_TX_FIFO_DEPTH		0x0CUL
#define IPA_COMMON_TX_FIFO_WR			0x10UL
#define IPA_COMMON_TX_FIFO_RD			0x14UL
#define IPA_COMMON_RX_FIFO_ADDRL		0x18UL
#define IPA_COMMON_RX_FIFO_ADDRH		0x1CUL
#define IPA_COMMON_TX_FIFO_ADDRL		0x20UL
#define IPA_COMMON_TX_FIFO_ADDRH		0x24UL
#define IPA_PERFETCH_FIFO_CTL			0x28UL
#define IPA_INT_GEN_CTL_TX_FIFO_VALUE		0x2CUL
#define IPA_INT_GEN_CTL_EN			0x30UL
#define IPA_DROP_PACKET_CNT			0x34UL
#define IPA_FLOW_CTRL_CONFIG			0x38UL
#define IPA_TX_FIFO_FLOW_CTRL			0x3CUL
#define IPA_RX_FIFO_FLOW_CTRL			0x40UL
#define IPA_RX_FIFO_FULL_NEG_PULSE_NUM		0x44UL
#define IPA_INT_GEN_CTL_CLR			0x48UL
#define IPA_INTR_RX_FIFO_FULL_ADDR_HIGH		0x4CUL
#define IPA_INTR_MEM_WR_ADDR_LOW		0x50UL
#define IPA_RXFIFO_FULL_MEM_WR_ADDR_LOW		0x54UL
#define IPA_INTR_MEM_WR_PATTERN			0x58UL
#define IPA_RX_FIFO_FULL_MEM_WR_PATTERN		0x5CUL
#define IPA_TX_FIFO_WR_INIT			0x60UL
#define IPA_COMMON_RX_FIFO_AXI_STS		0x64UL
#define IPA_ERRCODE_INT_ADDR_LOW		0x68UL
#define IPA_ERRCODE_INT_PATTERN			0x6CUL

/* Fifo interrupt enable bit */
#define IPA_TXFIFO_INT_THRESHOLD_ONESHOT_EN	BIT(11)
#define IPA_TXFIFO_INT_THRESHOLD_SW_EN		BIT(10)
#define IPA_TXFIFO_INT_DELAY_TIMER_SW_EN	BIT(9)
#define IPA_TXFIFO_FULL_INT_EN			BIT(8)
#define IPA_TXFIFO_OVERFLOW_EN			BIT(7)
#define IPA_ERRORCODE_IN_TX_FIFO_EN		BIT(6)
#define IPA_DROP_PACKET_OCCUR_INT_EN		BIT(5)
#define IPA_RX_FIFO_INT_EXIT_FLOW_CTRL_EN	BIT(4)
#define IPA_RX_FIFO_INT_ENTER_FLOW_CTRL_EN	BIT(3)
#define IPA_TX_FIFO_INTR_SW_BIT_EN		BIT(2)
#define IPA_TX_FIFO_THRESHOLD_EN		BIT(1)
#define IPA_TX_FIFO_DELAY_TIMER_EN		BIT(0)
#define IPA_INT_EN_BIT_GROUP			0x00000FFFUL

/* Fifo interrupt status bit */
#define IPA_INT_TX_FIFO_THRESHOLD_SW_STS	BIT(22)
#define IPA_INT_EXIT_FLOW_CTRL_STS		BIT(20)
#define IPA_INT_ENTER_FLOW_CTRL_STS		BIT(19)
#define IPA_INT_TXFIFO_FULL_INT_STS		BIT(18)
#define IPA_INT_TXFIFO_OVERFLOW_STS		BIT(17)
#define IPA_INT_ERRORCODE_IN_TX_FIFO_STS	BIT(16)
#define IPA_INT_INTR_BIT_STS			BIT(15)
#define IPA_INT_THRESHOLD_STS			BIT(14)
#define IPA_INT_DELAY_TIMER_STS			BIT(13)
#define IPA_INT_DROP_PACKT_OCCUR		BIT(12)
#define IPA_INT_STS_GROUP			0x5FF000UL

/* Fifo interrupt clear bit */
#define IPA_TX_FIFO_TIMER_CLR_BIT		BIT(0)
#define IPA_TX_FIFO_THRESHOLD_CLR_BIT		BIT(1)
#define IPA_TX_FIFO_INTR_CLR_BIT		BIT(2)
#define IPA_ENTRY_FLOW_CONTROL_CLR_BIT		BIT(3)
#define IPA_EXIT_FLOW_CONTROL_CLR_BIT		BIT(4)
#define IPA_DROP_PACKET_INTR_CLR_BIT		BIT(5)
#define IPA_ERROR_CODE_INTR_CLR_BIT		BIT(6)
#define IPA_TX_FIFO_OVERFLOW_CLR_BIT		BIT(7)
#define IPA_TX_FIFO_FULL_INT_CLR_BIT		BIT(8)
#define IPA_INT_STS_CLR_GROUP			0x000001FFUL

#define NODE_DESCRIPTION_SIZE			128UL

/**
 * Description: set rx fifo total depth.
 * Input:
 *   @fifo_base: Need to set total depth of the fifo,
 *    the base address of the FIFO.
 *   @depth: the size of depth.
 * return:
 *   0: set successfully.
 *   non-zero: set failed.
 * Note:
 */
static inline int ipa_phy_set_rx_fifo_total_depth(void __iomem *fifo_base,
						  u32 depth)
{
	u32 tmp;

	if (depth > 0xFFFF)
		return -EINVAL;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_DEPTH);
	tmp &= 0x0000FFFFUL;
	tmp |= (depth << 16);
	writel_relaxed(tmp, fifo_base + IPA_COMMON_RX_FIFO_DEPTH);
	tmp = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_DEPTH);
	if ((tmp & 0xFFFF0000UL) == tmp)
		return 0;
	else
		return -EINVAL;
}

/**
 * Description: get rx fifo total depth.
 * Input:
 *   @fifo_base: Need to get total depth of the fifo, the base address of the
 *              FIFO.
 * return: The size of toal depth.
 * Note:
 */
static inline u32 ipa_phy_get_rx_fifo_total_depth(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_DEPTH);

	return (tmp >> 16) & 0x0000FFFFUL;
}

/**
 * Description: get rx fifo filled depth.
 * Input:
 *   @fifo_base: Need to get filled depth of the FIFO, the base address of the
 *              FIFO.
 * return:
 *   TRUE: The size of rx filled depth
 * Note:
 */
static inline u32 ipa_phy_get_rx_fifo_filled_depth(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_DEPTH);

	return tmp & 0x0000FFFFUL;
}

/**
 * Description: get rx fifo full status.
 * Input:
 *   @fifo_base: Need to get rx fifo full status of the FIFO, the base address
 *              of the FIFO.
 * return:
 *   1: rx fifo full.
 *   0: rx fifo not full.
 * Note:
 */
static inline u32 ipa_phy_get_rx_fifo_full_status(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_WR);

	return tmp & 0x1UL;
}

/**
 * Description: update rx fifo write pointer.
 * Input:
 *   @fifo_base: Need to update rx fifo write pointer of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: update rx fifo write pointer successfully,
 *   non-zero: update rx fifo write pointer failed.
 * Note:
 */
static inline int ipa_phy_update_rx_fifo_wptr(void __iomem *fifo_base,
					      u32 wptr)
{
	u32 tmp = 0;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_COMMON_RX_FIFO_WR;

	if (wptr > 0xFFFFUL)
		return -EINVAL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0x0000FFFFUL;
	tmp |= (wptr << 16);
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	if ((tmp >> 16) == wptr)
		return 0;
	else
		return -EIO;
}

/**
 * Description: get rx fifo write pointer.
 * Input:
 *   @fifo_base: Need to get rx fifo write pointer of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   The write pointer of rx fifo.
 * Note:
 */
static inline u32 ipa_phy_get_rx_fifo_wptr(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_WR);

	return (tmp >> 16);
}

/**
 * Description: update rx fifo read pointer.
 * Input:
 *   @fifo_base: Need to update rx fifo read pointer of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: update rx fifo read pointer successfully,
 *   non-zero: update rx fifo read pointer failed.
 * Note:
 */
static inline int ipa_phy_update_rx_fifo_rptr(void __iomem *fifo_base,
					      u32 rptr)
{
	u32 tmp = 0;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_COMMON_RX_FIFO_RD;

	if (rptr > 0xFFFFUL)
		return -EINVAL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0x0000FFFFUL;
	tmp |= (rptr << 16);
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);

	if ((tmp >> 16) == rptr)
		return 0;
	else
		return -EIO;
}

/**
 * Description: get rx fifo read pointer.
 * Input:
 *   @fifo_base: Need to get rx fifo read pointer of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   The read pointer of rx fifo.
 * Note:
 */
static inline u32 ipa_phy_get_rx_fifo_rptr(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_RD);

	return (tmp >> 16);
}

/**
 * Description: get rx fifo empty status.
 * Input:
 *   @fifo_base: Need to get rx fifo empty status of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   The empty status of rx fifo.
 * Note:
 */
static inline u32 ipa_phy_get_rx_fifo_empty_status(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_RD);

	return tmp & 0x1UL;
}

/**
 * Description: set tx fifo total depth.
 * Input:
 *   @fifo_base: Need to set tx fifo empty status of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: set tx fifo total depth successfully.
 *   non-zero: set tx fifo total_depth failed.
 * Note:
 */
static inline int ipa_phy_set_tx_fifo_total_depth(void __iomem *fifo_base,
						  u32 depth)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_COMMON_TX_FIFO_DEPTH;

	if (depth > 0xFFFFUL)
		return -EINVAL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0x0000FFFFUL;
	tmp |= (depth << 16);
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	if ((tmp >> 16) == depth)
		return 0;
	else
		return -EIO;
}

/**
 * Description: get tx fifo total depth.
 * Input:
 *   @fifo_base: Need to get tx fifo empty status of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   The total depth of tx fifo.
 * Note:
 */
static inline u32 ipa_phy_get_tx_fifo_total_depth(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_TX_FIFO_DEPTH);

	return ((tmp >> 16) & 0x0000FFFF);
}

/**
 * Description: get tx fifo filled depth.
 * Input:
 *   @fifo_base: Need to get tx fifo filled depth of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   The tx fifo filled depth.
 * Note:
 */
static inline u32 ipa_phy_get_tx_fifo_filled_depth(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_TX_FIFO_DEPTH);

	return (tmp & 0x0000FFFFUL);
}

/**
 * Description: get tx fifo full status.
 * Input:
 *   @fifo_base: Need to get tx fifo full status of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   The full status of tx fifo.
 * Note:
 */
static inline u32 ipa_phy_get_tx_fifo_full_status(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_TX_FIFO_WR);

	return (tmp & 0x1UL);
}

/**
 * Description: get tx fifo empty status.
 * Input:
 *   @fifo_base: Need to get tx fifo empty status of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   The empty status of tx fifo.
 * Note:
 */
static inline u32 ipa_phy_get_tx_fifo_empty_status(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_TX_FIFO_RD);

	return (tmp & 0x1UL);
}

/**
 * Description: update tx fifo write pointer.
 * Input:
 *   @fifo_base: Need to update tx fifo write pointer of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: update tx fifo write pointer successfully.
 *   non-zero: update tx fifo write pointer failed.
 * Note:
 */
static inline int ipa_phy_update_tx_fifo_wptr(void __iomem *fifo_base,
					      u32 wptr)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_TX_FIFO_WR_INIT;

	if (wptr > 0xFFFFUL)
		return -EINVAL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0x0000FFFFUL;
	tmp |= (wptr << 16);
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	tmp |= 0x2;
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0xFFFFFFFDUL;
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);

	if ((tmp >> 16) == wptr)
		return 0;
	else
		return -EIO;
}

/**
 * Description: get tx fifo write pointer.
 * Input:
 *   @fifo_base: Need to get tx fifo write pointer of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   The write pointer of tx fifo.
 * Note:
 */
static inline u32 ipa_phy_get_tx_fifo_wptr(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_TX_FIFO_WR);

	return (tmp >> 16);
}

/**
 * Description: update tx fifo read pointer.
 * Input:
 *   @fifo_base: Need to update tx fifo read pointer of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: update tx fifo read pointer successfully.
 *   non-zero: update tx fifo read pointer failed.
 * Note:
 */
static inline int ipa_phy_update_tx_fifo_rptr(void __iomem *fifo_base,
					      u32 rptr)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_COMMON_TX_FIFO_RD;

	if (rptr > 0xFFFFUL)
		return -EINVAL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0x0000FFFF;
	tmp |= (rptr << 16);
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);

	if ((tmp >> 16) == rptr)
		return 0;
	else
		return -EIO;
}

/**
 * Description: get tx fifo write pointer.
 * Input:
 *   @fifo_base: Need to get tx fifo write pointer of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   The write pointer of rx fifo.
 * Note:
 */
static inline u32 ipa_phy_get_tx_fifo_rptr(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_TX_FIFO_RD);

	return (tmp >> 16);
}

/**
 * Description: set rx fifo address of iram.
 * Input:
 *   @fifo_base: Need to set rx fifo address of the FIFO, the base
 *              address of the FIFO.
 *   @addr_l: low 32 bit.
 *   @addr_h: high 8 bit.
 * return:
 *   0: update rx fifo address of iram successfully.
 *   non-zero: update rx fifo address of iram failed.
 * Note:
 */
static inline int ipa_phy_set_rx_fifo_addr(void __iomem *fifo_base,
					   u32 addr_l, u32 addr_h)
{
	u32 tmp_l, tmp_h;

	writel_relaxed(addr_l, fifo_base + IPA_COMMON_RX_FIFO_ADDRL);
	writel_relaxed(addr_h, fifo_base + IPA_COMMON_RX_FIFO_ADDRH);

	tmp_l = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_ADDRL);
	tmp_h = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_ADDRH);

	if ((tmp_l == addr_l) && (tmp_h == addr_h))
		return 0;
	else
		return -EIO;
}

/**
 * Description: get rx fifo address of iram.
 * Input:
 *   @fifo_base: Need to get rx fifo address of the FIFO, the base
 *              address of the FIFO.
 *   @addr_l: low 32 bit.
 *   @addr_h: high 8 bit.
 * return:
 *   void.
 * Note:
 */
static inline void ipa_phy_get_rx_fifo_addr(void __iomem *fifo_base,
					    u32 *addr_l, u32 *addr_h)
{
	*addr_l = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_ADDRL);
	*addr_h = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_ADDRH);
}

/**
 * Description: set tx fifo address of iram.
 * Input:
 *   @fifo_base: Need to set tx fifo address of the FIFO, the base
 *              address of the FIFO.
 *   @addr_l: low 32 bit.
 *   @addr_h: high 8 bit.
 * return:
 *   0: update tx fifo address of iram successfully.
 *   non-zero: update tx fifo address of iram failed.
 * Note:
 */
static inline int ipa_phy_set_tx_fifo_addr(void __iomem *fifo_base,
					   u32 addr_l, u32 addr_h)
{
	u32 tmp_l, tmp_h;

	writel_relaxed(addr_l, fifo_base + IPA_COMMON_TX_FIFO_ADDRL);
	writel_relaxed(addr_h, fifo_base + IPA_COMMON_TX_FIFO_ADDRH);

	tmp_l = readl_relaxed(fifo_base + IPA_COMMON_TX_FIFO_ADDRL);
	tmp_h = readl_relaxed(fifo_base + IPA_COMMON_TX_FIFO_ADDRH);

	if ((tmp_l == addr_l) && (tmp_h == addr_h))
		return 0;
	else
		return -EIO;
}

/**
 * Description: get tx fifo address of iram.
 * Input:
 *   @fifo_base: Need to get tx fifo address of the FIFO, the base
 *              address of the FIFO.
 *   @addr_l: low 32 bit.
 *   @addr_h: high 8 bit.
 * return:
 *   void.
 * Note:
 */
static inline void ipa_phy_get_tx_fifo_addr(void __iomem *fifo_base,
					    u32 *addr_l, u32 *addr_h)
{
	*addr_l = readl_relaxed(fifo_base + IPA_COMMON_TX_FIFO_ADDRL);
	*addr_h = readl_relaxed(fifo_base + IPA_COMMON_TX_FIFO_ADDRH);
}

/**
 * Description: Enable interrupt bit.
 * Input:
 *   @fifo_base: Need to enable interrupr bit of the FIFO, the base
 *              address of the FIFO.
 *   @int_bit: The interrupt bit that need to enable.
 * return:
 *   0: Enable successfully.
 *   non-zero: Enable successfully.
 * Note:
 */
static inline int ipa_phy_enable_int_bit(void __iomem *fifo_base,
					 u32 int_bit)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_INT_GEN_CTL_EN;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp |= int_bit;
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);

	if ((tmp & int_bit) == int_bit)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Disable interrupt bit.
 * Input:
 *   @fifo_base: Need to Disable interrupr bit of the FIFO, the base
 *              address of the FIFO.
 *   @int_bit: The interrupt bit that need to disable.
 * return:
 *   0: Disable successfully.
 *   non-zero: Disable failed.
 * Note:
 */
static inline int ipa_phy_disable_int_bit(void __iomem *fifo_base,
					  u32 int_bit)
{
	u32 tmp = 0;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_INT_GEN_CTL_EN;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= (~int_bit);
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= int_bit;

	if (tmp) {
		pr_err("Disable interrupt bit = 0x%x set failed!\n",
		       int_bit);
		return -EIO;
	}

	return 0;
}

static inline u32 ipa_phy_get_all_intr_enable_status(void __iomem *fifo_base)
{
	u32 tmp = 0;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_INT_GEN_CTL_EN;

	tmp = readl_relaxed(fifo_reg_addr);

	tmp &= IPA_INT_EN_BIT_GROUP;

	return tmp;
}

/**
 * Description: Get specified interrupt bit status.
 * Input:
 *   @fifo_base: Need to get interrupt bit of the FIFO, the base
 *              address of the FIFO.
 *   @int_bit: The specified interrupt bit that need to get.
 * return:
 *   0: interrupt bit enable.
 *   non-zero: interrupt bit disable.
 * Note:
 */
static inline int ipa_phy_get_fifo_int_sts(void __iomem *fifo_base, u32 sts)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_INT_GEN_CTL_EN);

	if (tmp & sts)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Get interrupt group status.
 * Input:
 *   @fifo_base: Need to get interrupt group status of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   Interrupt group status.
 * Note:
 */
static inline u32 ipa_phy_get_fifo_all_int_sts(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_INT_GEN_CTL_EN);

	return (tmp & IPA_INT_STS_GROUP);
}

/**
 * Description: Clear interrupt flag, need to write 1, then write 0.
 * Input:
 *   @fifo_base: Need to clear interrupt flag of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   void.
 * Note:
 */
static inline void ipa_phy_clear_int(void __iomem *fifo_base, u32 clr_bit)
{
	writel_relaxed(clr_bit, fifo_base + IPA_INT_GEN_CTL_CLR);
}

/**
 * Description: Get drop packet count.
 * Input:
 *   @fifo_base: Need to get drop packet count of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   Drop packet count.
 * Note:
 */
static inline u32 ipa_phy_get_drop_packet_cnt(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_DROP_PACKET_CNT);

	return tmp;
}

/**
 * Description: Get tx fifo threshold interrupt.
 * Input:
 *   @fifo_base: Need to get threshold interrupt of the FIFO, the base
 *              address of the FIFO.
 * OUTPUT:
 *   threshold value.
 * Note:
 */
static inline u32 ipa_phy_get_tx_fifo_interrupt_threshold(void __iomem
							  *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_INT_GEN_CTL_TX_FIFO_VALUE);

	return (tmp >> 16);
}

/**
 * Description: Set tx fifo interrupt threshold of value.
 * Input:
 *   @fifo_base: Need to get threshold interrupt value of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   TRUE: set successfully.
 *   FALSE: set failed.
 * Note:
 */
static inline int
ipa_phy_set_tx_fifo_interrupt_threshold(void __iomem *fifo_base, u32 threshold)
{
	u32 tmp = 0;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_INT_GEN_CTL_TX_FIFO_VALUE;

	if (threshold > 0xFFFFUL)
		return -EINVAL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0x0000FFFFUL;
	tmp |= (threshold << 16);
	writel_relaxed(tmp, fifo_reg_addr);
	tmp = readl_relaxed(fifo_reg_addr);

	if ((tmp >> 16) == threshold)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Get tx fifo interrupt of delay timer value.
 * Input:
 *   @fifo_base: Need to get delay timer interrupt of the FIFO, the base
 *              address of the FIFO.
 * OUTPUT:
 *   delay timer value.
 * Note:
 */
static inline u32
ipa_phy_get_tx_fifo_interrupt_delay_timer(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_INT_GEN_CTL_TX_FIFO_VALUE);

	return (tmp & 0x0000FFFFUL);
}

/**
 * Description: Set tx fifo interrupt of delay timer value.
 * Input:
 *   @fifo_base: Need to set delay timer interrupt of the FIFO, the base
 *              address of the FIFO.
 *   @threshold: The overflow value that need to set.
 * return:
 *   0: Set successfully.
 *   non-zero: set failed.
 * Note:
 */
static inline int
ipa_phy_set_tx_fifo_interrupt_delay_timer(void __iomem *fifo_base,
					  u32 threshold)
{
	u32 tmp = 0;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_INT_GEN_CTL_TX_FIFO_VALUE;

	if (threshold > 0xFFFFUL)
		return -EINVAL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0xFFFF0000UL;
	tmp |= threshold;
	writel_relaxed(tmp, fifo_reg_addr);
	tmp = readl_relaxed(fifo_reg_addr);

	if ((tmp & 0x0000FFFF) == threshold)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Get current term number.
 * Input:
 *   @fifo_base: Need to get current term number of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   Current term number.
 * Note:
 */
static inline u32 ipa_phy_get_cur_term_num(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_PERFETCH_FIFO_CTL);

	return ((tmp & 0x0003E000UL) >> 13);
}

/**
 * Description: Set current term number.
 * Input:
 *   @fifo_base: Need to set current term number of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: Set successfully.
 *   non-zero: Set failed.
 * Note:
 */
static inline int ipa_phy_set_cur_term_num(void __iomem *fifo_base,
					   u32 num)
{
	u32 tmp = 0;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_PERFETCH_FIFO_CTL;

	if (num > 0x1FUL)
		return -EINVAL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0xFFFC1FFFUL;
	tmp |= (num << 13);
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	if (((tmp & 0x0003E000) >> 13) == num)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Get dst term number.
 * Input:
 *   @fifo_base: Need to get dst term number of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   Dst term number.
 * Note:
 */
static inline u32 ipa_phy_get_dst_term_num(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_PERFETCH_FIFO_CTL);

	return ((tmp & 0x00001F00UL) >> 8);
}

/**
 * Description: Set dst term number.
 * Input:
 *   @fifo_base: Need to set dst term number of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: Set successfully.
 *   non-zero: Set failed.
 * Note:
 */
static inline int ipa_phy_set_dst_term_num(void __iomem *fifo_base,
					   u32 num)
{
	u32 tmp = 0;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_PERFETCH_FIFO_CTL;
	if (num > 0x1FUL)
		return -EINVAL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0xFFFFE0FFUL;
	tmp |= (num << 8);
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	if (((tmp & 0x00001F00UL) >> 8) == num)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Get prefetch fifo priority.
 * Input:
 *   @fifo_base: Need to get prefetch fifo priority of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   Prefetch fifo priority.
 * Note:
 */
static inline u32 ipa_phy_get_prefetch_fifo_priority(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_PERFETCH_FIFO_CTL);

	return ((tmp & 0x000000F0UL) >> 4);
}

/**
 * Description: Set prefetch fifo priority.
 * Input:
 *   @fifo_base: Need to set prefetch fifo priority of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: Set successfully.
 *   non-zero: Set failed.
 * Note:
 */
static inline int ipa_phy_set_prefetch_fifo_priority(void __iomem *fifo_base,
						     u32 pri)
{
	u32 tmp = 0;
	void __iomem *fifo_reg_base;

	fifo_reg_base = fifo_base + IPA_PERFETCH_FIFO_CTL;

	if (pri > 0xFUL)
		return -EINVAL;

	tmp = readl_relaxed(fifo_reg_base);
	tmp &= 0xFFFFFF0FUL;
	tmp |= (pri << 4);
	writel_relaxed(tmp, fifo_reg_base);

	tmp = readl_relaxed(fifo_reg_base);
	if (((tmp & 0x000000F0UL) >> 4) == pri)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Get prefetch threshold.
 * Input:
 *   @fifo_base: Need to get prefetch threshold of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   Prefetch threshold.
 * Note:
 */
static inline u32 ipa_phy_get_prefetch_threshold(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_PERFETCH_FIFO_CTL);

	return (tmp & 0xFUL);
}

/**
 * Description: Set prefetch threshold.
 * Input:
 *   @fifo_base: Need to get threshold of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: Set successfully.
 *   non-zero: Set failed.
 * Note:
 */
static inline int ipa_phy_set_prefetch_threshold(void __iomem *fifo_base,
						 u32 threshold)
{
	u32 tmp = 0;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_PERFETCH_FIFO_CTL;

	if (threshold > 0xFUL)
		return -EINVAL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0xFFFFFFF0UL;
	tmp |= threshold;
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	if ((tmp & 0xFUL) == threshold)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Set stop receive bit.
 * Input:
 *   @fifo_base: Need to set stop receive bit of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: Set successfully.
 *   non-zero: Set failed.
 * Note:
 */
static inline int ipa_phy_stop_receive(void __iomem *fifo_base)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_FLOW_CTRL_CONFIG;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp |= 0x8;
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	if (tmp & 0x8)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Clear stop receive bit.
 * Input:
 *   @fifo_base: Need to clear stop receive bit of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: clear successfully.
 *   non-zero: clear failed.
 * Note:
 */
static inline int ipa_phy_clear_stop_receive(void __iomem *fifo_base)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_FLOW_CTRL_CONFIG;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0xFFFFFFF7;
	writel_relaxed(tmp, fifo_reg_addr);
	tmp = readl_relaxed(fifo_reg_addr);

	if (!(tmp & 0x8))
		return 0;
	else
		return -EIO;
}

/**
 * Description: recover fifo work.
 * Input:
 *   @fifo_base: Need to be recovered of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: Recover successfully.
 *   non-zero: Recover failed.
 * Note:
 */
static inline int ipa_phy_flow_ctrl_recover(void __iomem *fifo_base)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_FLOW_CTRL_CONFIG;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp |= 0x4UL;
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	if (tmp & 0x4UL) {
		tmp &= 0xFFFFFFFBUL;
		writel_relaxed(tmp, fifo_reg_addr);

		tmp = readl_relaxed(fifo_reg_addr);
		if (!(tmp & 0x4UL))
			return 0;
		else
			return -EIO;
	} else {
		return -EINVAL;
	}
}

/**
 * Description: Set flow ctrl mode.
 * Input:
 *   @fifo_base: Need to set flow ctrl mode of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   0: Set successfully.
 *   non-zero: Set failed.
 * Note:
 */
static inline int ipa_phy_set_flow_ctrl_config(void __iomem *fifo_base,
					       u32 config)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_FLOW_CTRL_CONFIG;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0xFFFFFFFC;
	tmp |= config;
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	if ((tmp & 0x00000003) == config)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Get flow ctrl mode.
 * Input:
 *   @fifo_base: Need to get flow ctrl mode of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   Flow ctrl config
 * Note:
 */
static inline u32 ipa_phy_get_flow_ctrl_config(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_FLOW_CTRL_CONFIG);

	return (tmp & 0x00000003);
}

/**
 * Description: Set tx fifo exit flow ctrl watermark.
 * Input:
 *   @fifo_base: Need to be set of the FIFO, the base
 *              address of the FIFO.
 *   @watermark: The need to be set.
 * return:
 *   0: Set successfully.
 *   non-zero: Set failed.
 * Note:
 */
static inline int
ipa_phy_set_tx_fifo_exit_flow_ctrl_watermark(void __iomem *fifo_base,
					     u32 watermark)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_TX_FIFO_FLOW_CTRL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0x0000FFFFUL;
	tmp |= (watermark << 16);
	writel_relaxed(tmp, fifo_reg_addr);

	tmp = readl_relaxed(fifo_reg_addr);
	if ((tmp >> 16) == watermark)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Get tx fifo exit flow ctrl watermark.
 * Input:
 *   @fifo_base: Need to be get of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   Tx fifo exit watermark.
 * Note:
 */
static inline u32
ipa_phy_get_tx_fifo_exit_flow_ctrl_watermark(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_TX_FIFO_FLOW_CTRL);

	return (tmp >> 16);
}

/**
 * Description: Set tx fifo entry flow ctrl watermark.
 * Input:
 *   @fifo_base: Need to be set of the FIFO, the base
 *              address of the FIFO.
 *   @watermark: The need to be set.
 * return:
 *   0: Set successfully.
 *   non-zero: Set failed.
 * Note:
 */
static inline int
ipa_phy_set_tx_fifo_entry_flow_ctrl_watermark(void __iomem *fifo_base,
					      u32 watermark)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_TX_FIFO_FLOW_CTRL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0xFFFF0000UL;
	tmp |= watermark;
	writel_relaxed(tmp, fifo_reg_addr);
	tmp = readl_relaxed(fifo_reg_addr);

	if ((tmp & 0x0000FFFFUL) == watermark)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Get tx fifo entry flow ctrl watermark.
 * Input:
 *   @fifo_base: Need to be get of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   @The value of tx fifo entry watermark.
 * Note:
 */
static inline u32
ipa_phy_get_tx_fifo_entry_flow_ctrl_watermark(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_TX_FIFO_FLOW_CTRL);

	return (tmp & 0x0000FFFF);
}

/**
 * Description: Set rx fifo exit flow ctrl watermark.
 * Input:
 *   @fifo_base: Need to be set of the FIFO, the base
 *              address of the FIFO.
 *   @watermark: The value of rx fifo exit watermark.
 * return:
 *   0: Set successfully.
 *   non-zero: Set failed.
 * Note:
 */
static inline int
ipa_phy_set_rx_fifo_exit_flow_ctrl_watermark(void __iomem *fifo_base,
					     u32 watermark)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_RX_FIFO_FLOW_CTRL;

	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0x0000FFFFUL;
	tmp |= (watermark << 16);
	writel_relaxed(tmp, fifo_reg_addr);
	tmp = readl_relaxed(fifo_reg_addr);

	if ((tmp >> 16) == watermark)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Get rx fifo exit flow ctrl watermark.
 * Input:
 *   @fifo_base: Need to be get of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   The value of rx fifo exit watermark.
 * Note:
 */
static inline u32
ipa_phy_get_rx_fifo_exit_flow_ctrl_watermark(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_RX_FIFO_FLOW_CTRL);

	return (tmp >> 16);
}

/**
 * Description: Set rx fifo entry flow ctrl watermark.
 * Input:
 *   @fifo_base: Need to be set of the FIFO, the base
 *              address of the FIFO.
 *   @watermark: The value of rx fifo entry watermark.
 * return:
 *   TRUE: Set successfully.
 *   FALSE: Set failed.
 * Note:
 */
static inline int
ipa_phy_set_rx_fifo_entry_flow_ctrl_watermark(void __iomem *fifo_base,
					      u32 watermark)
{
	u32 tmp;
	void __iomem *fifo_reg_addr;

	fifo_reg_addr = fifo_base + IPA_RX_FIFO_FLOW_CTRL;
	tmp = readl_relaxed(fifo_reg_addr);
	tmp &= 0xFFFF0000UL;
	tmp |= watermark;
	writel_relaxed(tmp, fifo_reg_addr);
	tmp = readl_relaxed(fifo_reg_addr);

	if ((tmp & 0x0000FFFFUL) == watermark)
		return 0;
	else
		return -EIO;
}

/**
 * Description: Get rx fifo entry flow ctrl watermark.
 * Input:
 *   @fifo_base: Need to be get of the FIFO, the base
 *              address of the FIFO.
 * return:
 *   The value of rx fifo entry watermark.
 * Note:
 */
static inline u32
ipa_phy_get_rx_fifo_entry_flow_ctrl_watermark(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_RX_FIFO_FLOW_CTRL);

	return (tmp & 0x0000FFFF);
}

/**
 * Description: Get rx_axi_read_cmd_sts
 * return:
 *   rx_axi_read_cmd_sts.
 * Note:
 */
static inline u32 ipa_phy_get_rx_fifo_axi_sts(void __iomem *fifo_base)
{
	u32 tmp;

	tmp = readl_relaxed(fifo_base + IPA_COMMON_RX_FIFO_AXI_STS);

	return (tmp & 0x00000003);
}
#endif
