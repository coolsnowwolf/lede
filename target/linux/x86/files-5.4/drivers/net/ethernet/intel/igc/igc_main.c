// SPDX-License-Identifier: GPL-2.0
/* Copyright (c)  2018 Intel Corporation */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/if_vlan.h>
#include <linux/aer.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/pm_runtime.h>
#include <net/pkt_sched.h>

#include <net/ipv6.h>

#include "igc.h"
#include "igc_hw.h"
#include "igc_tsn.h"

#define DRV_SUMMARY	"Intel(R) 2.5G Ethernet Linux Driver"

#define DEFAULT_MSG_ENABLE (NETIF_MSG_DRV | NETIF_MSG_PROBE | NETIF_MSG_LINK)

static int debug = -1;

MODULE_AUTHOR("Intel Corporation, <linux.nics@intel.com>");
MODULE_DESCRIPTION(DRV_SUMMARY);
MODULE_LICENSE("GPL v2");
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level (0=none,...,16=all)");

char igc_driver_name[] = "igc";
static const char igc_driver_string[] = DRV_SUMMARY;
static const char igc_copyright[] =
	"Copyright(c) 2018 Intel Corporation.";

static const struct igc_info *igc_info_tbl[] = {
	[board_base] = &igc_base_info,
};

static const struct pci_device_id igc_pci_tbl[] = {
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I225_LM), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I225_V), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I225_I), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I220_V), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I225_K), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I225_K2), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I226_K), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I225_LMVP), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I226_LMVP), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I225_IT), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I226_LM), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I226_V), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I226_IT), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I221_V), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I226_BLANK_NVM), board_base },
	{ PCI_VDEVICE(INTEL, IGC_DEV_ID_I225_BLANK_NVM), board_base },
	/* required last entry */
	{0, }
};

MODULE_DEVICE_TABLE(pci, igc_pci_tbl);

enum latency_range {
	lowest_latency = 0,
	low_latency = 1,
	bulk_latency = 2,
	latency_invalid = 255
};

void igc_reset(struct igc_adapter *adapter)
{
	struct net_device *dev = adapter->netdev;
	struct igc_hw *hw = &adapter->hw;
	struct igc_fc_info *fc = &hw->fc;
	u32 pba, hwm;

	/* Repartition PBA for greater than 9k MTU if required */
	pba = IGC_PBA_34K;

	/* flow control settings
	 * The high water mark must be low enough to fit one full frame
	 * after transmitting the pause frame.  As such we must have enough
	 * space to allow for us to complete our current transmit and then
	 * receive the frame that is in progress from the link partner.
	 * Set it to:
	 * - the full Rx FIFO size minus one full Tx plus one full Rx frame
	 */
	hwm = (pba << 10) - (adapter->max_frame_size + MAX_JUMBO_FRAME_SIZE);

	fc->high_water = hwm & 0xFFFFFFF0;	/* 16-byte granularity */
	fc->low_water = fc->high_water - 16;
	fc->pause_time = 0xFFFF;
	fc->send_xon = 1;
	fc->current_mode = fc->requested_mode;

	hw->mac.ops.reset_hw(hw);

	if (hw->mac.ops.init_hw(hw))
		netdev_err(dev, "Error on hardware initialization\n");

	/* Re-establish EEE setting */
	igc_set_eee_i225(hw, false, false, false);

	if (!netif_running(adapter->netdev))
		igc_power_down_phy_copper_base(&adapter->hw);

	/* Re-enable PTP, where applicable. */
	igc_ptp_reset(adapter);

	/* Re-enable TSN offloading, where applicable. */
	igc_tsn_offload_apply(adapter);

	igc_get_phy_info(hw);
}

/**
 * igc_power_up_link - Power up the phy link
 * @adapter: address of board private structure
 */
static void igc_power_up_link(struct igc_adapter *adapter)
{
	igc_reset_phy(&adapter->hw);

	igc_power_up_phy_copper(&adapter->hw);

	igc_setup_link(&adapter->hw);
}

/**
 * igc_release_hw_control - release control of the h/w to f/w
 * @adapter: address of board private structure
 *
 * igc_release_hw_control resets CTRL_EXT:DRV_LOAD bit.
 * For ASF and Pass Through versions of f/w this means that the
 * driver is no longer loaded.
 */
static void igc_release_hw_control(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;
	u32 ctrl_ext;

	if (!pci_device_is_present(adapter->pdev))
		return;

	/* Let firmware take over control of h/w */
	ctrl_ext = rd32(IGC_CTRL_EXT);
	wr32(IGC_CTRL_EXT,
	     ctrl_ext & ~IGC_CTRL_EXT_DRV_LOAD);
}

/**
 * igc_get_hw_control - get control of the h/w from f/w
 * @adapter: address of board private structure
 *
 * igc_get_hw_control sets CTRL_EXT:DRV_LOAD bit.
 * For ASF and Pass Through versions of f/w this means that
 * the driver is loaded.
 */
static void igc_get_hw_control(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;
	u32 ctrl_ext;

	/* Let firmware know the driver has taken over */
	ctrl_ext = rd32(IGC_CTRL_EXT);
	wr32(IGC_CTRL_EXT,
	     ctrl_ext | IGC_CTRL_EXT_DRV_LOAD);
}

/**
 * igc_clean_tx_ring - Free Tx Buffers
 * @tx_ring: ring to be cleaned
 */
static void igc_clean_tx_ring(struct igc_ring *tx_ring)
{
	u16 i = tx_ring->next_to_clean;
	struct igc_tx_buffer *tx_buffer = &tx_ring->tx_buffer_info[i];

	while (i != tx_ring->next_to_use) {
		union igc_adv_tx_desc *eop_desc, *tx_desc;

		/* Free all the Tx ring sk_buffs */
		dev_kfree_skb_any(tx_buffer->skb);

		/* unmap skb header data */
		dma_unmap_single(tx_ring->dev,
				 dma_unmap_addr(tx_buffer, dma),
				 dma_unmap_len(tx_buffer, len),
				 DMA_TO_DEVICE);

		/* check for eop_desc to determine the end of the packet */
		eop_desc = tx_buffer->next_to_watch;
		tx_desc = IGC_TX_DESC(tx_ring, i);

		/* unmap remaining buffers */
		while (tx_desc != eop_desc) {
			tx_buffer++;
			tx_desc++;
			i++;
			if (unlikely(i == tx_ring->count)) {
				i = 0;
				tx_buffer = tx_ring->tx_buffer_info;
				tx_desc = IGC_TX_DESC(tx_ring, 0);
			}

			/* unmap any remaining paged data */
			if (dma_unmap_len(tx_buffer, len))
				dma_unmap_page(tx_ring->dev,
					       dma_unmap_addr(tx_buffer, dma),
					       dma_unmap_len(tx_buffer, len),
					       DMA_TO_DEVICE);
		}

		tx_buffer->next_to_watch = NULL;

		/* move us one more past the eop_desc for start of next pkt */
		tx_buffer++;
		i++;
		if (unlikely(i == tx_ring->count)) {
			i = 0;
			tx_buffer = tx_ring->tx_buffer_info;
		}
	}

	/* reset BQL for queue */
	netdev_tx_reset_queue(txring_txq(tx_ring));

	/* reset next_to_use and next_to_clean */
	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;
}

/**
 * igc_free_tx_resources - Free Tx Resources per Queue
 * @tx_ring: Tx descriptor ring for a specific queue
 *
 * Free all transmit software resources
 */
void igc_free_tx_resources(struct igc_ring *tx_ring)
{
	igc_clean_tx_ring(tx_ring);

	vfree(tx_ring->tx_buffer_info);
	tx_ring->tx_buffer_info = NULL;

	/* if not set, then don't free */
	if (!tx_ring->desc)
		return;

	dma_free_coherent(tx_ring->dev, tx_ring->size,
			  tx_ring->desc, tx_ring->dma);

	tx_ring->desc = NULL;
}

/**
 * igc_free_all_tx_resources - Free Tx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all transmit software resources
 */
static void igc_free_all_tx_resources(struct igc_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_tx_queues; i++)
		igc_free_tx_resources(adapter->tx_ring[i]);
}

/**
 * igc_clean_all_tx_rings - Free Tx Buffers for all queues
 * @adapter: board private structure
 */
static void igc_clean_all_tx_rings(struct igc_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_tx_queues; i++)
		if (adapter->tx_ring[i])
			igc_clean_tx_ring(adapter->tx_ring[i]);
}

/**
 * igc_setup_tx_resources - allocate Tx resources (Descriptors)
 * @tx_ring: tx descriptor ring (for a specific queue) to setup
 *
 * Return 0 on success, negative on failure
 */
int igc_setup_tx_resources(struct igc_ring *tx_ring)
{
	struct net_device *ndev = tx_ring->netdev;
	struct device *dev = tx_ring->dev;
	int size = 0;

	size = sizeof(struct igc_tx_buffer) * tx_ring->count;
	tx_ring->tx_buffer_info = vzalloc(size);
	if (!tx_ring->tx_buffer_info)
		goto err;

	/* round up to nearest 4K */
	tx_ring->size = tx_ring->count * sizeof(union igc_adv_tx_desc);
	tx_ring->size = ALIGN(tx_ring->size, 4096);

	tx_ring->desc = dma_alloc_coherent(dev, tx_ring->size,
					   &tx_ring->dma, GFP_KERNEL);

	if (!tx_ring->desc)
		goto err;

	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;

	return 0;

err:
	vfree(tx_ring->tx_buffer_info);
	netdev_err(ndev, "Unable to allocate memory for Tx descriptor ring\n");
	return -ENOMEM;
}

/**
 * igc_setup_all_tx_resources - wrapper to allocate Tx resources for all queues
 * @adapter: board private structure
 *
 * Return 0 on success, negative on failure
 */
static int igc_setup_all_tx_resources(struct igc_adapter *adapter)
{
	struct net_device *dev = adapter->netdev;
	int i, err = 0;

	for (i = 0; i < adapter->num_tx_queues; i++) {
		err = igc_setup_tx_resources(adapter->tx_ring[i]);
		if (err) {
			netdev_err(dev, "Error on Tx queue %u setup\n", i);
			for (i--; i >= 0; i--)
				igc_free_tx_resources(adapter->tx_ring[i]);
			break;
		}
	}

	return err;
}

/**
 * igc_clean_rx_ring - Free Rx Buffers per Queue
 * @rx_ring: ring to free buffers from
 */
static void igc_clean_rx_ring(struct igc_ring *rx_ring)
{
	u16 i = rx_ring->next_to_clean;

	dev_kfree_skb(rx_ring->skb);
	rx_ring->skb = NULL;

	/* Free all the Rx ring sk_buffs */
	while (i != rx_ring->next_to_alloc) {
		struct igc_rx_buffer *buffer_info = &rx_ring->rx_buffer_info[i];

		/* Invalidate cache lines that may have been written to by
		 * device so that we avoid corrupting memory.
		 */
		dma_sync_single_range_for_cpu(rx_ring->dev,
					      buffer_info->dma,
					      buffer_info->page_offset,
					      igc_rx_bufsz(rx_ring),
					      DMA_FROM_DEVICE);

		/* free resources associated with mapping */
		dma_unmap_page_attrs(rx_ring->dev,
				     buffer_info->dma,
				     igc_rx_pg_size(rx_ring),
				     DMA_FROM_DEVICE,
				     IGC_RX_DMA_ATTR);
		__page_frag_cache_drain(buffer_info->page,
					buffer_info->pagecnt_bias);

		i++;
		if (i == rx_ring->count)
			i = 0;
	}

	rx_ring->next_to_alloc = 0;
	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;
}

/**
 * igc_clean_all_rx_rings - Free Rx Buffers for all queues
 * @adapter: board private structure
 */
static void igc_clean_all_rx_rings(struct igc_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_rx_queues; i++)
		if (adapter->rx_ring[i])
			igc_clean_rx_ring(adapter->rx_ring[i]);
}

/**
 * igc_free_rx_resources - Free Rx Resources
 * @rx_ring: ring to clean the resources from
 *
 * Free all receive software resources
 */
void igc_free_rx_resources(struct igc_ring *rx_ring)
{
	igc_clean_rx_ring(rx_ring);

	vfree(rx_ring->rx_buffer_info);
	rx_ring->rx_buffer_info = NULL;

	/* if not set, then don't free */
	if (!rx_ring->desc)
		return;

	dma_free_coherent(rx_ring->dev, rx_ring->size,
			  rx_ring->desc, rx_ring->dma);

	rx_ring->desc = NULL;
}

/**
 * igc_free_all_rx_resources - Free Rx Resources for All Queues
 * @adapter: board private structure
 *
 * Free all receive software resources
 */
static void igc_free_all_rx_resources(struct igc_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_rx_queues; i++)
		igc_free_rx_resources(adapter->rx_ring[i]);
}

/**
 * igc_setup_rx_resources - allocate Rx resources (Descriptors)
 * @rx_ring:    rx descriptor ring (for a specific queue) to setup
 *
 * Returns 0 on success, negative on failure
 */
int igc_setup_rx_resources(struct igc_ring *rx_ring)
{
	struct net_device *ndev = rx_ring->netdev;
	struct device *dev = rx_ring->dev;
	int size, desc_len;

	size = sizeof(struct igc_rx_buffer) * rx_ring->count;
	rx_ring->rx_buffer_info = vzalloc(size);
	if (!rx_ring->rx_buffer_info)
		goto err;

	desc_len = sizeof(union igc_adv_rx_desc);

	/* Round up to nearest 4K */
	rx_ring->size = rx_ring->count * desc_len;
	rx_ring->size = ALIGN(rx_ring->size, 4096);

	rx_ring->desc = dma_alloc_coherent(dev, rx_ring->size,
					   &rx_ring->dma, GFP_KERNEL);

	if (!rx_ring->desc)
		goto err;

	rx_ring->next_to_alloc = 0;
	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;

	return 0;

err:
	vfree(rx_ring->rx_buffer_info);
	rx_ring->rx_buffer_info = NULL;
	netdev_err(ndev, "Unable to allocate memory for Rx descriptor ring\n");
	return -ENOMEM;
}

/**
 * igc_setup_all_rx_resources - wrapper to allocate Rx resources
 *                                (Descriptors) for all queues
 * @adapter: board private structure
 *
 * Return 0 on success, negative on failure
 */
static int igc_setup_all_rx_resources(struct igc_adapter *adapter)
{
	struct net_device *dev = adapter->netdev;
	int i, err = 0;

	for (i = 0; i < adapter->num_rx_queues; i++) {
		err = igc_setup_rx_resources(adapter->rx_ring[i]);
		if (err) {
			netdev_err(dev, "Error on Rx queue %u setup\n", i);
			for (i--; i >= 0; i--)
				igc_free_rx_resources(adapter->rx_ring[i]);
			break;
		}
	}

	return err;
}

/**
 * igc_configure_rx_ring - Configure a receive ring after Reset
 * @adapter: board private structure
 * @ring: receive ring to be configured
 *
 * Configure the Rx unit of the MAC after a reset.
 */
static void igc_configure_rx_ring(struct igc_adapter *adapter,
				  struct igc_ring *ring)
{
	struct igc_hw *hw = &adapter->hw;
	union igc_adv_rx_desc *rx_desc;
	int reg_idx = ring->reg_idx;
	u32 srrctl = 0, rxdctl = 0;
	u64 rdba = ring->dma;

	/* disable the queue */
	wr32(IGC_RXDCTL(reg_idx), 0);

	/* Set DMA base address registers */
	wr32(IGC_RDBAL(reg_idx),
	     rdba & 0x00000000ffffffffULL);
	wr32(IGC_RDBAH(reg_idx), rdba >> 32);
	wr32(IGC_RDLEN(reg_idx),
	     ring->count * sizeof(union igc_adv_rx_desc));

	/* initialize head and tail */
	ring->tail = adapter->io_addr + IGC_RDT(reg_idx);
	wr32(IGC_RDH(reg_idx), 0);
	writel(0, ring->tail);

	/* reset next-to- use/clean to place SW in sync with hardware */
	ring->next_to_clean = 0;
	ring->next_to_use = 0;

	/* set descriptor configuration */
	srrctl = IGC_RX_HDR_LEN << IGC_SRRCTL_BSIZEHDRSIZE_SHIFT;
	if (ring_uses_large_buffer(ring))
		srrctl |= IGC_RXBUFFER_3072 >> IGC_SRRCTL_BSIZEPKT_SHIFT;
	else
		srrctl |= IGC_RXBUFFER_2048 >> IGC_SRRCTL_BSIZEPKT_SHIFT;
	srrctl |= IGC_SRRCTL_DESCTYPE_ADV_ONEBUF;

	wr32(IGC_SRRCTL(reg_idx), srrctl);

	rxdctl |= IGC_RX_PTHRESH;
	rxdctl |= IGC_RX_HTHRESH << 8;
	rxdctl |= IGC_RX_WTHRESH << 16;

	/* initialize rx_buffer_info */
	memset(ring->rx_buffer_info, 0,
	       sizeof(struct igc_rx_buffer) * ring->count);

	/* initialize Rx descriptor 0 */
	rx_desc = IGC_RX_DESC(ring, 0);
	rx_desc->wb.upper.length = 0;

	/* enable receive descriptor fetching */
	rxdctl |= IGC_RXDCTL_QUEUE_ENABLE;

	wr32(IGC_RXDCTL(reg_idx), rxdctl);
}

/**
 * igc_configure_rx - Configure receive Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Rx unit of the MAC after a reset.
 */
static void igc_configure_rx(struct igc_adapter *adapter)
{
	int i;

	/* Setup the HW Rx Head and Tail Descriptor Pointers and
	 * the Base and Length of the Rx Descriptor Ring
	 */
	for (i = 0; i < adapter->num_rx_queues; i++)
		igc_configure_rx_ring(adapter, adapter->rx_ring[i]);
}

/**
 * igc_configure_tx_ring - Configure transmit ring after Reset
 * @adapter: board private structure
 * @ring: tx ring to configure
 *
 * Configure a transmit ring after a reset.
 */
static void igc_configure_tx_ring(struct igc_adapter *adapter,
				  struct igc_ring *ring)
{
	struct igc_hw *hw = &adapter->hw;
	int reg_idx = ring->reg_idx;
	u64 tdba = ring->dma;
	u32 txdctl = 0;

	/* disable the queue */
	wr32(IGC_TXDCTL(reg_idx), 0);
	wrfl();
	mdelay(10);

	wr32(IGC_TDLEN(reg_idx),
	     ring->count * sizeof(union igc_adv_tx_desc));
	wr32(IGC_TDBAL(reg_idx),
	     tdba & 0x00000000ffffffffULL);
	wr32(IGC_TDBAH(reg_idx), tdba >> 32);

	ring->tail = adapter->io_addr + IGC_TDT(reg_idx);
	wr32(IGC_TDH(reg_idx), 0);
	writel(0, ring->tail);

	txdctl |= IGC_TX_PTHRESH;
	txdctl |= IGC_TX_HTHRESH << 8;
	txdctl |= IGC_TX_WTHRESH << 16;

	txdctl |= IGC_TXDCTL_QUEUE_ENABLE;
	wr32(IGC_TXDCTL(reg_idx), txdctl);
}

/**
 * igc_configure_tx - Configure transmit Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Tx unit of the MAC after a reset.
 */
static void igc_configure_tx(struct igc_adapter *adapter)
{
	int i;

	for (i = 0; i < adapter->num_tx_queues; i++)
		igc_configure_tx_ring(adapter, adapter->tx_ring[i]);
}

/**
 * igc_setup_mrqc - configure the multiple receive queue control registers
 * @adapter: Board private structure
 */
static void igc_setup_mrqc(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;
	u32 j, num_rx_queues;
	u32 mrqc, rxcsum;
	u32 rss_key[10];

	netdev_rss_key_fill(rss_key, sizeof(rss_key));
	for (j = 0; j < 10; j++)
		wr32(IGC_RSSRK(j), rss_key[j]);

	num_rx_queues = adapter->rss_queues;

	if (adapter->rss_indir_tbl_init != num_rx_queues) {
		for (j = 0; j < IGC_RETA_SIZE; j++)
			adapter->rss_indir_tbl[j] =
			(j * num_rx_queues) / IGC_RETA_SIZE;
		adapter->rss_indir_tbl_init = num_rx_queues;
	}
	igc_write_rss_indir_tbl(adapter);

	/* Disable raw packet checksumming so that RSS hash is placed in
	 * descriptor on writeback.  No need to enable TCP/UDP/IP checksum
	 * offloads as they are enabled by default
	 */
	rxcsum = rd32(IGC_RXCSUM);
	rxcsum |= IGC_RXCSUM_PCSD;

	/* Enable Receive Checksum Offload for SCTP */
	rxcsum |= IGC_RXCSUM_CRCOFL;

	/* Don't need to set TUOFL or IPOFL, they default to 1 */
	wr32(IGC_RXCSUM, rxcsum);

	/* Generate RSS hash based on packet types, TCP/UDP
	 * port numbers and/or IPv4/v6 src and dst addresses
	 */
	mrqc = IGC_MRQC_RSS_FIELD_IPV4 |
	       IGC_MRQC_RSS_FIELD_IPV4_TCP |
	       IGC_MRQC_RSS_FIELD_IPV6 |
	       IGC_MRQC_RSS_FIELD_IPV6_TCP |
	       IGC_MRQC_RSS_FIELD_IPV6_TCP_EX;

	if (adapter->flags & IGC_FLAG_RSS_FIELD_IPV4_UDP)
		mrqc |= IGC_MRQC_RSS_FIELD_IPV4_UDP;
	if (adapter->flags & IGC_FLAG_RSS_FIELD_IPV6_UDP)
		mrqc |= IGC_MRQC_RSS_FIELD_IPV6_UDP;

	mrqc |= IGC_MRQC_ENABLE_RSS_MQ;

	wr32(IGC_MRQC, mrqc);
}

/**
 * igc_setup_rctl - configure the receive control registers
 * @adapter: Board private structure
 */
static void igc_setup_rctl(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;
	u32 rctl;

	rctl = rd32(IGC_RCTL);

	rctl &= ~(3 << IGC_RCTL_MO_SHIFT);
	rctl &= ~(IGC_RCTL_LBM_TCVR | IGC_RCTL_LBM_MAC);

	rctl |= IGC_RCTL_EN | IGC_RCTL_BAM | IGC_RCTL_RDMTS_HALF |
		(hw->mac.mc_filter_type << IGC_RCTL_MO_SHIFT);

	/* enable stripping of CRC. Newer features require
	 * that the HW strips the CRC.
	 */
	rctl |= IGC_RCTL_SECRC;

	/* disable store bad packets and clear size bits. */
	rctl &= ~(IGC_RCTL_SBP | IGC_RCTL_SZ_256);

	/* enable LPE to allow for reception of jumbo frames */
	rctl |= IGC_RCTL_LPE;

	/* disable queue 0 to prevent tail write w/o re-config */
	wr32(IGC_RXDCTL(0), 0);

	/* This is useful for sniffing bad packets. */
	if (adapter->netdev->features & NETIF_F_RXALL) {
		/* UPE and MPE will be handled by normal PROMISC logic
		 * in set_rx_mode
		 */
		rctl |= (IGC_RCTL_SBP | /* Receive bad packets */
			 IGC_RCTL_BAM | /* RX All Bcast Pkts */
			 IGC_RCTL_PMCF); /* RX All MAC Ctrl Pkts */

		rctl &= ~(IGC_RCTL_DPF | /* Allow filtered pause */
			  IGC_RCTL_CFIEN); /* Disable VLAN CFIEN Filter */
	}

	wr32(IGC_RCTL, rctl);
}

/**
 * igc_setup_tctl - configure the transmit control registers
 * @adapter: Board private structure
 */
static void igc_setup_tctl(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;
	u32 tctl;

	/* disable queue 0 which icould be enabled by default */
	wr32(IGC_TXDCTL(0), 0);

	/* Program the Transmit Control Register */
	tctl = rd32(IGC_TCTL);
	tctl &= ~IGC_TCTL_CT;
	tctl |= IGC_TCTL_PSP | IGC_TCTL_RTLC |
		(IGC_COLLISION_THRESHOLD << IGC_CT_SHIFT);

	/* Enable transmits */
	tctl |= IGC_TCTL_EN;

	wr32(IGC_TCTL, tctl);
}

/**
 * igc_set_mac_filter_hw() - Set MAC address filter in hardware
 * @adapter: Pointer to adapter where the filter should be set
 * @index: Filter index
 * @type: MAC address filter type (source or destination)
 * @addr: MAC address
 * @queue: If non-negative, queue assignment feature is enabled and frames
 *         matching the filter are enqueued onto 'queue'. Otherwise, queue
 *         assignment is disabled.
 */
static void igc_set_mac_filter_hw(struct igc_adapter *adapter, int index,
				  enum igc_mac_filter_type type,
				  const u8 *addr, int queue)
{
	struct net_device *dev = adapter->netdev;
	struct igc_hw *hw = &adapter->hw;
	u32 ral, rah;

	if (WARN_ON(index >= hw->mac.rar_entry_count))
		return;

	ral = le32_to_cpup((__le32 *)(addr));
	rah = le16_to_cpup((__le16 *)(addr + 4));

	if (type == IGC_MAC_FILTER_TYPE_SRC) {
		rah &= ~IGC_RAH_ASEL_MASK;
		rah |= IGC_RAH_ASEL_SRC_ADDR;
	}

	if (queue >= 0) {
		rah &= ~IGC_RAH_QSEL_MASK;
		rah |= (queue << IGC_RAH_QSEL_SHIFT);
		rah |= IGC_RAH_QSEL_ENABLE;
	}

	rah |= IGC_RAH_AV;

	wr32(IGC_RAL(index), ral);
	wr32(IGC_RAH(index), rah);

	netdev_dbg(dev, "MAC address filter set in HW: index %d", index);
}

/**
 * igc_clear_mac_filter_hw() - Clear MAC address filter in hardware
 * @adapter: Pointer to adapter where the filter should be cleared
 * @index: Filter index
 */
static void igc_clear_mac_filter_hw(struct igc_adapter *adapter, int index)
{
	struct net_device *dev = adapter->netdev;
	struct igc_hw *hw = &adapter->hw;

	if (WARN_ON(index >= hw->mac.rar_entry_count))
		return;

	wr32(IGC_RAL(index), 0);
	wr32(IGC_RAH(index), 0);

	netdev_dbg(dev, "MAC address filter cleared in HW: index %d", index);
}

/* Set default MAC address for the PF in the first RAR entry */
static void igc_set_default_mac_filter(struct igc_adapter *adapter)
{
	struct net_device *dev = adapter->netdev;
	u8 *addr = adapter->hw.mac.addr;

	netdev_dbg(dev, "Set default MAC address filter: address %pM", addr);

	igc_set_mac_filter_hw(adapter, 0, IGC_MAC_FILTER_TYPE_DST, addr, -1);
}

/**
 * igc_set_mac - Change the Ethernet Address of the NIC
 * @netdev: network interface device structure
 * @p: pointer to an address structure
 *
 * Returns 0 on success, negative on failure
 */
static int igc_set_mac(struct net_device *netdev, void *p)
{
	struct igc_adapter *adapter = netdev_priv(netdev);
	struct igc_hw *hw = &adapter->hw;
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);
	memcpy(hw->mac.addr, addr->sa_data, netdev->addr_len);

	/* set the correct pool for the new PF MAC address in entry 0 */
	igc_set_default_mac_filter(adapter);

	return 0;
}

/**
 *  igc_write_mc_addr_list - write multicast addresses to MTA
 *  @netdev: network interface device structure
 *
 *  Writes multicast address list to the MTA hash table.
 *  Returns: -ENOMEM on failure
 *           0 on no addresses written
 *           X on writing X addresses to MTA
 **/
static int igc_write_mc_addr_list(struct net_device *netdev)
{
	struct igc_adapter *adapter = netdev_priv(netdev);
	struct igc_hw *hw = &adapter->hw;
	struct netdev_hw_addr *ha;
	u8  *mta_list;
	int i;

	if (netdev_mc_empty(netdev)) {
		/* nothing to program, so clear mc list */
		igc_update_mc_addr_list(hw, NULL, 0);
		return 0;
	}

	mta_list = kcalloc(netdev_mc_count(netdev), 6, GFP_ATOMIC);
	if (!mta_list)
		return -ENOMEM;

	/* The shared function expects a packed array of only addresses. */
	i = 0;
	netdev_for_each_mc_addr(ha, netdev)
		memcpy(mta_list + (i++ * ETH_ALEN), ha->addr, ETH_ALEN);

	igc_update_mc_addr_list(hw, mta_list, i);
	kfree(mta_list);

	return netdev_mc_count(netdev);
}

static __le32 igc_tx_launchtime(struct igc_adapter *adapter, ktime_t txtime)
{
	ktime_t cycle_time = adapter->cycle_time;
	ktime_t base_time = adapter->base_time;
	u32 launchtime;

	/* FIXME: when using ETF together with taprio, we may have a
	 * case where 'delta' is larger than the cycle_time, this may
	 * cause problems if we don't read the current value of
	 * IGC_BASET, as the value writen into the launchtime
	 * descriptor field may be misinterpreted.
	 */
	div_s64_rem(ktime_sub_ns(txtime, base_time), cycle_time, &launchtime);

	return cpu_to_le32(launchtime);
}

static void igc_tx_ctxtdesc(struct igc_ring *tx_ring,
			    struct igc_tx_buffer *first,
			    u32 vlan_macip_lens, u32 type_tucmd,
			    u32 mss_l4len_idx)
{
	struct igc_adv_tx_context_desc *context_desc;
	u16 i = tx_ring->next_to_use;

	context_desc = IGC_TX_CTXTDESC(tx_ring, i);

	i++;
	tx_ring->next_to_use = (i < tx_ring->count) ? i : 0;

	/* set bits to identify this as an advanced context descriptor */
	type_tucmd |= IGC_TXD_CMD_DEXT | IGC_ADVTXD_DTYP_CTXT;

	/* For i225, context index must be unique per ring. */
	if (test_bit(IGC_RING_FLAG_TX_CTX_IDX, &tx_ring->flags))
		mss_l4len_idx |= tx_ring->reg_idx << 4;

	context_desc->vlan_macip_lens	= cpu_to_le32(vlan_macip_lens);
	context_desc->type_tucmd_mlhl	= cpu_to_le32(type_tucmd);
	context_desc->mss_l4len_idx	= cpu_to_le32(mss_l4len_idx);

	/* We assume there is always a valid Tx time available. Invalid times
	 * should have been handled by the upper layers.
	 */
	if (tx_ring->launchtime_enable) {
		struct igc_adapter *adapter = netdev_priv(tx_ring->netdev);
		ktime_t txtime = first->skb->tstamp;

		first->skb->tstamp = ktime_set(0, 0);
		context_desc->launch_time = igc_tx_launchtime(adapter,
							      txtime);
	} else {
		context_desc->launch_time = 0;
	}
}

static inline bool igc_ipv6_csum_is_sctp(struct sk_buff *skb)
{
	unsigned int offset = 0;

	ipv6_find_hdr(skb, &offset, IPPROTO_SCTP, NULL, NULL);

	return offset == skb_checksum_start_offset(skb);
}

static void igc_tx_csum(struct igc_ring *tx_ring, struct igc_tx_buffer *first)
{
	struct sk_buff *skb = first->skb;
	u32 vlan_macip_lens = 0;
	u32 type_tucmd = 0;

	if (skb->ip_summed != CHECKSUM_PARTIAL) {
csum_failed:
		if (!(first->tx_flags & IGC_TX_FLAGS_VLAN) &&
		    !tx_ring->launchtime_enable)
			return;
		goto no_csum;
	}

	switch (skb->csum_offset) {
	case offsetof(struct tcphdr, check):
		type_tucmd = IGC_ADVTXD_TUCMD_L4T_TCP;
		fallthrough;
	case offsetof(struct udphdr, check):
		break;
	case offsetof(struct sctphdr, checksum):
		/* validate that this is actually an SCTP request */
		if ((first->protocol == htons(ETH_P_IP) &&
		     (ip_hdr(skb)->protocol == IPPROTO_SCTP)) ||
		    (first->protocol == htons(ETH_P_IPV6) &&
		     igc_ipv6_csum_is_sctp(skb))) {
			type_tucmd = IGC_ADVTXD_TUCMD_L4T_SCTP;
			break;
		}
		fallthrough;
	default:
		skb_checksum_help(skb);
		goto csum_failed;
	}

	/* update TX checksum flag */
	first->tx_flags |= IGC_TX_FLAGS_CSUM;
	vlan_macip_lens = skb_checksum_start_offset(skb) -
			  skb_network_offset(skb);
no_csum:
	vlan_macip_lens |= skb_network_offset(skb) << IGC_ADVTXD_MACLEN_SHIFT;
	vlan_macip_lens |= first->tx_flags & IGC_TX_FLAGS_VLAN_MASK;

	igc_tx_ctxtdesc(tx_ring, first, vlan_macip_lens, type_tucmd, 0);
}

static int __igc_maybe_stop_tx(struct igc_ring *tx_ring, const u16 size)
{
	struct net_device *netdev = tx_ring->netdev;

	netif_stop_subqueue(netdev, tx_ring->queue_index);

	/* memory barriier comment */
	smp_mb();

	/* We need to check again in a case another CPU has just
	 * made room available.
	 */
	if (igc_desc_unused(tx_ring) < size)
		return -EBUSY;

	/* A reprieve! */
	netif_wake_subqueue(netdev, tx_ring->queue_index);

	u64_stats_update_begin(&tx_ring->tx_syncp2);
	tx_ring->tx_stats.restart_queue2++;
	u64_stats_update_end(&tx_ring->tx_syncp2);

	return 0;
}

static inline int igc_maybe_stop_tx(struct igc_ring *tx_ring, const u16 size)
{
	if (igc_desc_unused(tx_ring) >= size)
		return 0;
	return __igc_maybe_stop_tx(tx_ring, size);
}

#define IGC_SET_FLAG(_input, _flag, _result) \
	(((_flag) <= (_result)) ?				\
	 ((u32)((_input) & (_flag)) * ((_result) / (_flag))) :	\
	 ((u32)((_input) & (_flag)) / ((_flag) / (_result))))

static u32 igc_tx_cmd_type(struct sk_buff *skb, u32 tx_flags)
{
	/* set type for advanced descriptor with frame checksum insertion */
	u32 cmd_type = IGC_ADVTXD_DTYP_DATA |
		       IGC_ADVTXD_DCMD_DEXT |
		       IGC_ADVTXD_DCMD_IFCS;

	/* set segmentation bits for TSO */
	cmd_type |= IGC_SET_FLAG(tx_flags, IGC_TX_FLAGS_TSO,
				 (IGC_ADVTXD_DCMD_TSE));

	/* set timestamp bit if present */
	cmd_type |= IGC_SET_FLAG(tx_flags, IGC_TX_FLAGS_TSTAMP,
				 (IGC_ADVTXD_MAC_TSTAMP));

	return cmd_type;
}

static void igc_tx_olinfo_status(struct igc_ring *tx_ring,
				 union igc_adv_tx_desc *tx_desc,
				 u32 tx_flags, unsigned int paylen)
{
	u32 olinfo_status = paylen << IGC_ADVTXD_PAYLEN_SHIFT;

	/* insert L4 checksum */
	olinfo_status |= (tx_flags & IGC_TX_FLAGS_CSUM) *
			  ((IGC_TXD_POPTS_TXSM << 8) /
			  IGC_TX_FLAGS_CSUM);

	/* insert IPv4 checksum */
	olinfo_status |= (tx_flags & IGC_TX_FLAGS_IPV4) *
			  (((IGC_TXD_POPTS_IXSM << 8)) /
			  IGC_TX_FLAGS_IPV4);

	tx_desc->read.olinfo_status = cpu_to_le32(olinfo_status);
}

static int igc_tx_map(struct igc_ring *tx_ring,
		      struct igc_tx_buffer *first,
		      const u8 hdr_len)
{
	struct sk_buff *skb = first->skb;
	struct igc_tx_buffer *tx_buffer;
	union igc_adv_tx_desc *tx_desc;
	u32 tx_flags = first->tx_flags;
	skb_frag_t *frag;
	u16 i = tx_ring->next_to_use;
	unsigned int data_len, size;
	dma_addr_t dma;
	u32 cmd_type = igc_tx_cmd_type(skb, tx_flags);

	tx_desc = IGC_TX_DESC(tx_ring, i);

	igc_tx_olinfo_status(tx_ring, tx_desc, tx_flags, skb->len - hdr_len);

	size = skb_headlen(skb);
	data_len = skb->data_len;

	dma = dma_map_single(tx_ring->dev, skb->data, size, DMA_TO_DEVICE);

	tx_buffer = first;

	for (frag = &skb_shinfo(skb)->frags[0];; frag++) {
		if (dma_mapping_error(tx_ring->dev, dma))
			goto dma_error;

		/* record length, and DMA address */
		dma_unmap_len_set(tx_buffer, len, size);
		dma_unmap_addr_set(tx_buffer, dma, dma);

		tx_desc->read.buffer_addr = cpu_to_le64(dma);

		while (unlikely(size > IGC_MAX_DATA_PER_TXD)) {
			tx_desc->read.cmd_type_len =
				cpu_to_le32(cmd_type ^ IGC_MAX_DATA_PER_TXD);

			i++;
			tx_desc++;
			if (i == tx_ring->count) {
				tx_desc = IGC_TX_DESC(tx_ring, 0);
				i = 0;
			}
			tx_desc->read.olinfo_status = 0;

			dma += IGC_MAX_DATA_PER_TXD;
			size -= IGC_MAX_DATA_PER_TXD;

			tx_desc->read.buffer_addr = cpu_to_le64(dma);
		}

		if (likely(!data_len))
			break;

		tx_desc->read.cmd_type_len = cpu_to_le32(cmd_type ^ size);

		i++;
		tx_desc++;
		if (i == tx_ring->count) {
			tx_desc = IGC_TX_DESC(tx_ring, 0);
			i = 0;
		}
		tx_desc->read.olinfo_status = 0;

		size = skb_frag_size(frag);
		data_len -= size;

		dma = skb_frag_dma_map(tx_ring->dev, frag, 0,
				       size, DMA_TO_DEVICE);

		tx_buffer = &tx_ring->tx_buffer_info[i];
	}

	/* write last descriptor with RS and EOP bits */
	cmd_type |= size | IGC_TXD_DCMD;
	tx_desc->read.cmd_type_len = cpu_to_le32(cmd_type);

	netdev_tx_sent_queue(txring_txq(tx_ring), first->bytecount);

	/* set the timestamp */
	first->time_stamp = jiffies;

	skb_tx_timestamp(skb);

	/* Force memory writes to complete before letting h/w know there
	 * are new descriptors to fetch.  (Only applicable for weak-ordered
	 * memory model archs, such as IA-64).
	 *
	 * We also need this memory barrier to make certain all of the
	 * status bits have been updated before next_to_watch is written.
	 */
	wmb();

	/* set next_to_watch value indicating a packet is present */
	first->next_to_watch = tx_desc;

	i++;
	if (i == tx_ring->count)
		i = 0;

	tx_ring->next_to_use = i;

	/* Make sure there is space in the ring for the next send. */
	igc_maybe_stop_tx(tx_ring, DESC_NEEDED);

	if (netif_xmit_stopped(txring_txq(tx_ring)) || !netdev_xmit_more()) {
		writel(i, tx_ring->tail);
	}

	return 0;
dma_error:
	netdev_err(tx_ring->netdev, "TX DMA map failed\n");
	tx_buffer = &tx_ring->tx_buffer_info[i];

	/* clear dma mappings for failed tx_buffer_info map */
	while (tx_buffer != first) {
		if (dma_unmap_len(tx_buffer, len))
			dma_unmap_page(tx_ring->dev,
				       dma_unmap_addr(tx_buffer, dma),
				       dma_unmap_len(tx_buffer, len),
				       DMA_TO_DEVICE);
		dma_unmap_len_set(tx_buffer, len, 0);

		if (i-- == 0)
			i += tx_ring->count;
		tx_buffer = &tx_ring->tx_buffer_info[i];
	}

	if (dma_unmap_len(tx_buffer, len))
		dma_unmap_single(tx_ring->dev,
				 dma_unmap_addr(tx_buffer, dma),
				 dma_unmap_len(tx_buffer, len),
				 DMA_TO_DEVICE);
	dma_unmap_len_set(tx_buffer, len, 0);

	dev_kfree_skb_any(tx_buffer->skb);
	tx_buffer->skb = NULL;

	tx_ring->next_to_use = i;

	return -1;
}

static int igc_tso(struct igc_ring *tx_ring,
		   struct igc_tx_buffer *first,
		   u8 *hdr_len)
{
	u32 vlan_macip_lens, type_tucmd, mss_l4len_idx;
	struct sk_buff *skb = first->skb;
	union {
		struct iphdr *v4;
		struct ipv6hdr *v6;
		unsigned char *hdr;
	} ip;
	union {
		struct tcphdr *tcp;
		struct udphdr *udp;
		unsigned char *hdr;
	} l4;
	u32 paylen, l4_offset;
	int err;

	if (skb->ip_summed != CHECKSUM_PARTIAL)
		return 0;

	if (!skb_is_gso(skb))
		return 0;

	err = skb_cow_head(skb, 0);
	if (err < 0)
		return err;

	ip.hdr = skb_network_header(skb);
	l4.hdr = skb_checksum_start(skb);

	/* ADV DTYP TUCMD MKRLOC/ISCSIHEDLEN */
	type_tucmd = IGC_ADVTXD_TUCMD_L4T_TCP;

	/* initialize outer IP header fields */
	if (ip.v4->version == 4) {
		unsigned char *csum_start = skb_checksum_start(skb);
		unsigned char *trans_start = ip.hdr + (ip.v4->ihl * 4);

		/* IP header will have to cancel out any data that
		 * is not a part of the outer IP header
		 */
		ip.v4->check = csum_fold(csum_partial(trans_start,
						      csum_start - trans_start,
						      0));
		type_tucmd |= IGC_ADVTXD_TUCMD_IPV4;

		ip.v4->tot_len = 0;
		first->tx_flags |= IGC_TX_FLAGS_TSO |
				   IGC_TX_FLAGS_CSUM |
				   IGC_TX_FLAGS_IPV4;
	} else {
		ip.v6->payload_len = 0;
		first->tx_flags |= IGC_TX_FLAGS_TSO |
				   IGC_TX_FLAGS_CSUM;
	}

	/* determine offset of inner transport header */
	l4_offset = l4.hdr - skb->data;

	/* remove payload length from inner checksum */
	paylen = skb->len - l4_offset;
	if (type_tucmd & IGC_ADVTXD_TUCMD_L4T_TCP) {
		/* compute length of segmentation header */
		*hdr_len = (l4.tcp->doff * 4) + l4_offset;
		csum_replace_by_diff(&l4.tcp->check,
				     (__force __wsum)htonl(paylen));
	} else {
		/* compute length of segmentation header */
		*hdr_len = sizeof(*l4.udp) + l4_offset;
		csum_replace_by_diff(&l4.udp->check,
				     (__force __wsum)htonl(paylen));
	}

	/* update gso size and bytecount with header size */
	first->gso_segs = skb_shinfo(skb)->gso_segs;
	first->bytecount += (first->gso_segs - 1) * *hdr_len;

	/* MSS L4LEN IDX */
	mss_l4len_idx = (*hdr_len - l4_offset) << IGC_ADVTXD_L4LEN_SHIFT;
	mss_l4len_idx |= skb_shinfo(skb)->gso_size << IGC_ADVTXD_MSS_SHIFT;

	/* VLAN MACLEN IPLEN */
	vlan_macip_lens = l4.hdr - ip.hdr;
	vlan_macip_lens |= (ip.hdr - skb->data) << IGC_ADVTXD_MACLEN_SHIFT;
	vlan_macip_lens |= first->tx_flags & IGC_TX_FLAGS_VLAN_MASK;

	igc_tx_ctxtdesc(tx_ring, first, vlan_macip_lens,
			type_tucmd, mss_l4len_idx);

	return 1;
}

static netdev_tx_t igc_xmit_frame_ring(struct sk_buff *skb,
				       struct igc_ring *tx_ring)
{
	u16 count = TXD_USE_COUNT(skb_headlen(skb));
	__be16 protocol = vlan_get_protocol(skb);
	struct igc_tx_buffer *first;
	u32 tx_flags = 0;
	unsigned short f;
	u8 hdr_len = 0;
	int tso = 0;

	/* need: 1 descriptor per page * PAGE_SIZE/IGC_MAX_DATA_PER_TXD,
	 *	+ 1 desc for skb_headlen/IGC_MAX_DATA_PER_TXD,
	 *	+ 2 desc gap to keep tail from touching head,
	 *	+ 1 desc for context descriptor,
	 * otherwise try next time
	 */
	for (f = 0; f < skb_shinfo(skb)->nr_frags; f++)
		count += TXD_USE_COUNT(skb_frag_size(
						&skb_shinfo(skb)->frags[f]));

	if (igc_maybe_stop_tx(tx_ring, count + 3)) {
		/* this is a hard error */
		return NETDEV_TX_BUSY;
	}

	/* record the location of the first descriptor for this packet */
	first = &tx_ring->tx_buffer_info[tx_ring->next_to_use];
	first->skb = skb;
	first->bytecount = skb->len;
	first->gso_segs = 1;

	if (unlikely(skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP)) {
		struct igc_adapter *adapter = netdev_priv(tx_ring->netdev);

		/* FIXME: add support for retrieving timestamps from
		 * the other timer registers before skipping the
		 * timestamping request.
		 */
		if (adapter->tstamp_config.tx_type == HWTSTAMP_TX_ON &&
		    !test_and_set_bit_lock(__IGC_PTP_TX_IN_PROGRESS,
					   &adapter->state)) {
			skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
			tx_flags |= IGC_TX_FLAGS_TSTAMP;

			adapter->ptp_tx_skb = skb_get(skb);
			adapter->ptp_tx_start = jiffies;
		} else {
			adapter->tx_hwtstamp_skipped++;
		}
	}

	/* record initial flags and protocol */
	first->tx_flags = tx_flags;
	first->protocol = protocol;

	tso = igc_tso(tx_ring, first, &hdr_len);
	if (tso < 0)
		goto out_drop;
	else if (!tso)
		igc_tx_csum(tx_ring, first);

	igc_tx_map(tx_ring, first, hdr_len);

	return NETDEV_TX_OK;

out_drop:
	dev_kfree_skb_any(first->skb);
	first->skb = NULL;

	return NETDEV_TX_OK;
}

static inline struct igc_ring *igc_tx_queue_mapping(struct igc_adapter *adapter,
						    struct sk_buff *skb)
{
	unsigned int r_idx = skb->queue_mapping;

	if (r_idx >= adapter->num_tx_queues)
		r_idx = r_idx % adapter->num_tx_queues;

	return adapter->tx_ring[r_idx];
}

static netdev_tx_t igc_xmit_frame(struct sk_buff *skb,
				  struct net_device *netdev)
{
	struct igc_adapter *adapter = netdev_priv(netdev);

	/* The minimum packet size with TCTL.PSP set is 17 so pad the skb
	 * in order to meet this minimum size requirement.
	 */
	if (skb->len < 17) {
		if (skb_padto(skb, 17))
			return NETDEV_TX_OK;
		skb->len = 17;
	}

	return igc_xmit_frame_ring(skb, igc_tx_queue_mapping(adapter, skb));
}

static void igc_rx_checksum(struct igc_ring *ring,
			    union igc_adv_rx_desc *rx_desc,
			    struct sk_buff *skb)
{
	skb_checksum_none_assert(skb);

	/* Ignore Checksum bit is set */
	if (igc_test_staterr(rx_desc, IGC_RXD_STAT_IXSM))
		return;

	/* Rx checksum disabled via ethtool */
	if (!(ring->netdev->features & NETIF_F_RXCSUM))
		return;

	/* TCP/UDP checksum error bit is set */
	if (igc_test_staterr(rx_desc,
			     IGC_RXDEXT_STATERR_L4E |
			     IGC_RXDEXT_STATERR_IPE)) {
		/* work around errata with sctp packets where the TCPE aka
		 * L4E bit is set incorrectly on 64 byte (60 byte w/o crc)
		 * packets (aka let the stack check the crc32c)
		 */
		if (!(skb->len == 60 &&
		      test_bit(IGC_RING_FLAG_RX_SCTP_CSUM, &ring->flags))) {
			u64_stats_update_begin(&ring->rx_syncp);
			ring->rx_stats.csum_err++;
			u64_stats_update_end(&ring->rx_syncp);
		}
		/* let the stack verify checksum errors */
		return;
	}
	/* It must be a TCP or UDP packet with a valid checksum */
	if (igc_test_staterr(rx_desc, IGC_RXD_STAT_TCPCS |
				      IGC_RXD_STAT_UDPCS))
		skb->ip_summed = CHECKSUM_UNNECESSARY;

	netdev_dbg(ring->netdev, "cksum success: bits %08X\n",
		   le32_to_cpu(rx_desc->wb.upper.status_error));
}

static inline void igc_rx_hash(struct igc_ring *ring,
			       union igc_adv_rx_desc *rx_desc,
			       struct sk_buff *skb)
{
	if (ring->netdev->features & NETIF_F_RXHASH)
		skb_set_hash(skb,
			     le32_to_cpu(rx_desc->wb.lower.hi_dword.rss),
			     PKT_HASH_TYPE_L3);
}

/**
 * igc_process_skb_fields - Populate skb header fields from Rx descriptor
 * @rx_ring: rx descriptor ring packet is being transacted on
 * @rx_desc: pointer to the EOP Rx descriptor
 * @skb: pointer to current skb being populated
 *
 * This function checks the ring, descriptor, and packet information in order
 * to populate the hash, checksum, VLAN, protocol, and other fields within the
 * skb.
 */
static void igc_process_skb_fields(struct igc_ring *rx_ring,
				   union igc_adv_rx_desc *rx_desc,
				   struct sk_buff *skb)
{
	igc_rx_hash(rx_ring, rx_desc, skb);

	igc_rx_checksum(rx_ring, rx_desc, skb);

	skb_record_rx_queue(skb, rx_ring->queue_index);

	skb->protocol = eth_type_trans(skb, rx_ring->netdev);
}

static struct igc_rx_buffer *igc_get_rx_buffer(struct igc_ring *rx_ring,
					       const unsigned int size)
{
	struct igc_rx_buffer *rx_buffer;

	rx_buffer = &rx_ring->rx_buffer_info[rx_ring->next_to_clean];
	prefetchw(rx_buffer->page);

	/* we are reusing so sync this buffer for CPU use */
	dma_sync_single_range_for_cpu(rx_ring->dev,
				      rx_buffer->dma,
				      rx_buffer->page_offset,
				      size,
				      DMA_FROM_DEVICE);

	rx_buffer->pagecnt_bias--;

	return rx_buffer;
}

/**
 * igc_add_rx_frag - Add contents of Rx buffer to sk_buff
 * @rx_ring: rx descriptor ring to transact packets on
 * @rx_buffer: buffer containing page to add
 * @skb: sk_buff to place the data into
 * @size: size of buffer to be added
 *
 * This function will add the data contained in rx_buffer->page to the skb.
 */
static void igc_add_rx_frag(struct igc_ring *rx_ring,
			    struct igc_rx_buffer *rx_buffer,
			    struct sk_buff *skb,
			    unsigned int size)
{
#if (PAGE_SIZE < 8192)
	unsigned int truesize = igc_rx_pg_size(rx_ring) / 2;

	skb_add_rx_frag(skb, skb_shinfo(skb)->nr_frags, rx_buffer->page,
			rx_buffer->page_offset, size, truesize);
	rx_buffer->page_offset ^= truesize;
#else
	unsigned int truesize = ring_uses_build_skb(rx_ring) ?
				SKB_DATA_ALIGN(IGC_SKB_PAD + size) :
				SKB_DATA_ALIGN(size);
	skb_add_rx_frag(skb, skb_shinfo(skb)->nr_frags, rx_buffer->page,
			rx_buffer->page_offset, size, truesize);
	rx_buffer->page_offset += truesize;
#endif
}

static struct sk_buff *igc_build_skb(struct igc_ring *rx_ring,
				     struct igc_rx_buffer *rx_buffer,
				     union igc_adv_rx_desc *rx_desc,
				     unsigned int size)
{
	void *va = page_address(rx_buffer->page) + rx_buffer->page_offset;
#if (PAGE_SIZE < 8192)
	unsigned int truesize = igc_rx_pg_size(rx_ring) / 2;
#else
	unsigned int truesize = SKB_DATA_ALIGN(sizeof(struct skb_shared_info)) +
				SKB_DATA_ALIGN(IGC_SKB_PAD + size);
#endif
	struct sk_buff *skb;

	/* prefetch first cache line of first page */
	prefetch(va);
#if L1_CACHE_BYTES < 128
	prefetch(va + L1_CACHE_BYTES);
#endif

	/* build an skb around the page buffer */
	skb = build_skb(va - IGC_SKB_PAD, truesize);
	if (unlikely(!skb))
		return NULL;

	/* update pointers within the skb to store the data */
	skb_reserve(skb, IGC_SKB_PAD);
	__skb_put(skb, size);

	/* update buffer offset */
#if (PAGE_SIZE < 8192)
	rx_buffer->page_offset ^= truesize;
#else
	rx_buffer->page_offset += truesize;
#endif

	return skb;
}

static struct sk_buff *igc_construct_skb(struct igc_ring *rx_ring,
					 struct igc_rx_buffer *rx_buffer,
					 union igc_adv_rx_desc *rx_desc,
					 unsigned int size)
{
	void *va = page_address(rx_buffer->page) + rx_buffer->page_offset;
#if (PAGE_SIZE < 8192)
	unsigned int truesize = igc_rx_pg_size(rx_ring) / 2;
#else
	unsigned int truesize = SKB_DATA_ALIGN(size);
#endif
	unsigned int headlen;
	struct sk_buff *skb;

	/* prefetch first cache line of first page */
	prefetch(va);
#if L1_CACHE_BYTES < 128
	prefetch(va + L1_CACHE_BYTES);
#endif

	/* allocate a skb to store the frags */
	skb = napi_alloc_skb(&rx_ring->q_vector->napi, IGC_RX_HDR_LEN);
	if (unlikely(!skb))
		return NULL;

	if (unlikely(igc_test_staterr(rx_desc, IGC_RXDADV_STAT_TSIP))) {
		igc_ptp_rx_pktstamp(rx_ring->q_vector, va, skb);
		va += IGC_TS_HDR_LEN;
		size -= IGC_TS_HDR_LEN;
	}

	/* Determine available headroom for copy */
	headlen = size;
	if (headlen > IGC_RX_HDR_LEN)
		headlen = eth_get_headlen(skb->dev, va, IGC_RX_HDR_LEN);

	/* align pull length to size of long to optimize memcpy performance */
	memcpy(__skb_put(skb, headlen), va, ALIGN(headlen, sizeof(long)));

	/* update all of the pointers */
	size -= headlen;
	if (size) {
		skb_add_rx_frag(skb, 0, rx_buffer->page,
				(va + headlen) - page_address(rx_buffer->page),
				size, truesize);
#if (PAGE_SIZE < 8192)
		rx_buffer->page_offset ^= truesize;
#else
		rx_buffer->page_offset += truesize;
#endif
	} else {
		rx_buffer->pagecnt_bias++;
	}

	return skb;
}

/**
 * igc_reuse_rx_page - page flip buffer and store it back on the ring
 * @rx_ring: rx descriptor ring to store buffers on
 * @old_buff: donor buffer to have page reused
 *
 * Synchronizes page for reuse by the adapter
 */
static void igc_reuse_rx_page(struct igc_ring *rx_ring,
			      struct igc_rx_buffer *old_buff)
{
	u16 nta = rx_ring->next_to_alloc;
	struct igc_rx_buffer *new_buff;

	new_buff = &rx_ring->rx_buffer_info[nta];

	/* update, and store next to alloc */
	nta++;
	rx_ring->next_to_alloc = (nta < rx_ring->count) ? nta : 0;

	/* Transfer page from old buffer to new buffer.
	 * Move each member individually to avoid possible store
	 * forwarding stalls.
	 */
	new_buff->dma		= old_buff->dma;
	new_buff->page		= old_buff->page;
	new_buff->page_offset	= old_buff->page_offset;
	new_buff->pagecnt_bias	= old_buff->pagecnt_bias;
}

static inline bool igc_page_is_reserved(struct page *page)
{
	return (page_to_nid(page) != numa_mem_id()) || page_is_pfmemalloc(page);
}

static bool igc_can_reuse_rx_page(struct igc_rx_buffer *rx_buffer)
{
	unsigned int pagecnt_bias = rx_buffer->pagecnt_bias;
	struct page *page = rx_buffer->page;

	/* avoid re-using remote pages */
	if (unlikely(igc_page_is_reserved(page)))
		return false;

#if (PAGE_SIZE < 8192)
	/* if we are only owner of page we can reuse it */
	if (unlikely((page_ref_count(page) - pagecnt_bias) > 1))
		return false;
#else
#define IGC_LAST_OFFSET \
	(SKB_WITH_OVERHEAD(PAGE_SIZE) - IGC_RXBUFFER_2048)

	if (rx_buffer->page_offset > IGC_LAST_OFFSET)
		return false;
#endif

	/* If we have drained the page fragment pool we need to update
	 * the pagecnt_bias and page count so that we fully restock the
	 * number of references the driver holds.
	 */
	if (unlikely(!pagecnt_bias)) {
		page_ref_add(page, USHRT_MAX);
		rx_buffer->pagecnt_bias = USHRT_MAX;
	}

	return true;
}

/**
 * igc_is_non_eop - process handling of non-EOP buffers
 * @rx_ring: Rx ring being processed
 * @rx_desc: Rx descriptor for current buffer
 *
 * This function updates next to clean.  If the buffer is an EOP buffer
 * this function exits returning false, otherwise it will place the
 * sk_buff in the next buffer to be chained and return true indicating
 * that this is in fact a non-EOP buffer.
 */
static bool igc_is_non_eop(struct igc_ring *rx_ring,
			   union igc_adv_rx_desc *rx_desc)
{
	u32 ntc = rx_ring->next_to_clean + 1;

	/* fetch, update, and store next to clean */
	ntc = (ntc < rx_ring->count) ? ntc : 0;
	rx_ring->next_to_clean = ntc;

	prefetch(IGC_RX_DESC(rx_ring, ntc));

	if (likely(igc_test_staterr(rx_desc, IGC_RXD_STAT_EOP)))
		return false;

	return true;
}

/**
 * igc_cleanup_headers - Correct corrupted or empty headers
 * @rx_ring: rx descriptor ring packet is being transacted on
 * @rx_desc: pointer to the EOP Rx descriptor
 * @skb: pointer to current skb being fixed
 *
 * Address the case where we are pulling data in on pages only
 * and as such no data is present in the skb header.
 *
 * In addition if skb is not at least 60 bytes we need to pad it so that
 * it is large enough to qualify as a valid Ethernet frame.
 *
 * Returns true if an error was encountered and skb was freed.
 */
static bool igc_cleanup_headers(struct igc_ring *rx_ring,
				union igc_adv_rx_desc *rx_desc,
				struct sk_buff *skb)
{
	if (unlikely(igc_test_staterr(rx_desc, IGC_RXDEXT_STATERR_RXE))) {
		struct net_device *netdev = rx_ring->netdev;

		if (!(netdev->features & NETIF_F_RXALL)) {
			dev_kfree_skb_any(skb);
			return true;
		}
	}

	/* if eth_skb_pad returns an error the skb was freed */
	if (eth_skb_pad(skb))
		return true;

	return false;
}

static void igc_put_rx_buffer(struct igc_ring *rx_ring,
			      struct igc_rx_buffer *rx_buffer)
{
	if (igc_can_reuse_rx_page(rx_buffer)) {
		/* hand second half of page back to the ring */
		igc_reuse_rx_page(rx_ring, rx_buffer);
	} else {
		/* We are not reusing the buffer so unmap it and free
		 * any references we are holding to it
		 */
		dma_unmap_page_attrs(rx_ring->dev, rx_buffer->dma,
				     igc_rx_pg_size(rx_ring), DMA_FROM_DEVICE,
				     IGC_RX_DMA_ATTR);
		__page_frag_cache_drain(rx_buffer->page,
					rx_buffer->pagecnt_bias);
	}

	/* clear contents of rx_buffer */
	rx_buffer->page = NULL;
}

static inline unsigned int igc_rx_offset(struct igc_ring *rx_ring)
{
	return ring_uses_build_skb(rx_ring) ? IGC_SKB_PAD : 0;
}

static bool igc_alloc_mapped_page(struct igc_ring *rx_ring,
				  struct igc_rx_buffer *bi)
{
	struct page *page = bi->page;
	dma_addr_t dma;

	/* since we are recycling buffers we should seldom need to alloc */
	if (likely(page))
		return true;

	/* alloc new page for storage */
	page = dev_alloc_pages(igc_rx_pg_order(rx_ring));
	if (unlikely(!page)) {
		rx_ring->rx_stats.alloc_failed++;
		return false;
	}

	/* map page for use */
	dma = dma_map_page_attrs(rx_ring->dev, page, 0,
				 igc_rx_pg_size(rx_ring),
				 DMA_FROM_DEVICE,
				 IGC_RX_DMA_ATTR);

	/* if mapping failed free memory back to system since
	 * there isn't much point in holding memory we can't use
	 */
	if (dma_mapping_error(rx_ring->dev, dma)) {
		__free_page(page);

		rx_ring->rx_stats.alloc_failed++;
		return false;
	}

	bi->dma = dma;
	bi->page = page;
	bi->page_offset = igc_rx_offset(rx_ring);
	bi->pagecnt_bias = 1;

	return true;
}

/**
 * igc_alloc_rx_buffers - Replace used receive buffers; packet split
 * @rx_ring: rx descriptor ring
 * @cleaned_count: number of buffers to clean
 */
static void igc_alloc_rx_buffers(struct igc_ring *rx_ring, u16 cleaned_count)
{
	union igc_adv_rx_desc *rx_desc;
	u16 i = rx_ring->next_to_use;
	struct igc_rx_buffer *bi;
	u16 bufsz;

	/* nothing to do */
	if (!cleaned_count)
		return;

	rx_desc = IGC_RX_DESC(rx_ring, i);
	bi = &rx_ring->rx_buffer_info[i];
	i -= rx_ring->count;

	bufsz = igc_rx_bufsz(rx_ring);

	do {
		if (!igc_alloc_mapped_page(rx_ring, bi))
			break;

		/* sync the buffer for use by the device */
		dma_sync_single_range_for_device(rx_ring->dev, bi->dma,
						 bi->page_offset, bufsz,
						 DMA_FROM_DEVICE);

		/* Refresh the desc even if buffer_addrs didn't change
		 * because each write-back erases this info.
		 */
		rx_desc->read.pkt_addr = cpu_to_le64(bi->dma + bi->page_offset);

		rx_desc++;
		bi++;
		i++;
		if (unlikely(!i)) {
			rx_desc = IGC_RX_DESC(rx_ring, 0);
			bi = rx_ring->rx_buffer_info;
			i -= rx_ring->count;
		}

		/* clear the length for the next_to_use descriptor */
		rx_desc->wb.upper.length = 0;

		cleaned_count--;
	} while (cleaned_count);

	i += rx_ring->count;

	if (rx_ring->next_to_use != i) {
		/* record the next descriptor to use */
		rx_ring->next_to_use = i;

		/* update next to alloc since we have filled the ring */
		rx_ring->next_to_alloc = i;

		/* Force memory writes to complete before letting h/w
		 * know there are new descriptors to fetch.  (Only
		 * applicable for weak-ordered memory model archs,
		 * such as IA-64).
		 */
		wmb();
		writel(i, rx_ring->tail);
	}
}

static int igc_clean_rx_irq(struct igc_q_vector *q_vector, const int budget)
{
	unsigned int total_bytes = 0, total_packets = 0;
	struct igc_ring *rx_ring = q_vector->rx.ring;
	struct sk_buff *skb = rx_ring->skb;
	u16 cleaned_count = igc_desc_unused(rx_ring);

	while (likely(total_packets < budget)) {
		union igc_adv_rx_desc *rx_desc;
		struct igc_rx_buffer *rx_buffer;
		unsigned int size;

		/* return some buffers to hardware, one at a time is too slow */
		if (cleaned_count >= IGC_RX_BUFFER_WRITE) {
			igc_alloc_rx_buffers(rx_ring, cleaned_count);
			cleaned_count = 0;
		}

		rx_desc = IGC_RX_DESC(rx_ring, rx_ring->next_to_clean);
		size = le16_to_cpu(rx_desc->wb.upper.length);
		if (!size)
			break;

		/* This memory barrier is needed to keep us from reading
		 * any other fields out of the rx_desc until we know the
		 * descriptor has been written back
		 */
		dma_rmb();

		rx_buffer = igc_get_rx_buffer(rx_ring, size);

		/* retrieve a buffer from the ring */
		if (skb)
			igc_add_rx_frag(rx_ring, rx_buffer, skb, size);
		else if (ring_uses_build_skb(rx_ring))
			skb = igc_build_skb(rx_ring, rx_buffer, rx_desc, size);
		else
			skb = igc_construct_skb(rx_ring, rx_buffer,
						rx_desc, size);

		/* exit if we failed to retrieve a buffer */
		if (!skb) {
			rx_ring->rx_stats.alloc_failed++;
			rx_buffer->pagecnt_bias++;
			break;
		}

		igc_put_rx_buffer(rx_ring, rx_buffer);
		cleaned_count++;

		/* fetch next buffer in frame if non-eop */
		if (igc_is_non_eop(rx_ring, rx_desc))
			continue;

		/* verify the packet layout is correct */
		if (igc_cleanup_headers(rx_ring, rx_desc, skb)) {
			skb = NULL;
			continue;
		}

		/* probably a little skewed due to removing CRC */
		total_bytes += skb->len;

		/* populate checksum, VLAN, and protocol */
		igc_process_skb_fields(rx_ring, rx_desc, skb);

		napi_gro_receive(&q_vector->napi, skb);

		/* reset skb pointer */
		skb = NULL;

		/* update budget accounting */
		total_packets++;
	}

	/* place incomplete frames back on ring for completion */
	rx_ring->skb = skb;

	u64_stats_update_begin(&rx_ring->rx_syncp);
	rx_ring->rx_stats.packets += total_packets;
	rx_ring->rx_stats.bytes += total_bytes;
	u64_stats_update_end(&rx_ring->rx_syncp);
	q_vector->rx.total_packets += total_packets;
	q_vector->rx.total_bytes += total_bytes;

	if (cleaned_count)
		igc_alloc_rx_buffers(rx_ring, cleaned_count);

	return total_packets;
}

/**
 * igc_clean_tx_irq - Reclaim resources after transmit completes
 * @q_vector: pointer to q_vector containing needed info
 * @napi_budget: Used to determine if we are in netpoll
 *
 * returns true if ring is completely cleaned
 */
static bool igc_clean_tx_irq(struct igc_q_vector *q_vector, int napi_budget)
{
	struct igc_adapter *adapter = q_vector->adapter;
	unsigned int total_bytes = 0, total_packets = 0;
	unsigned int budget = q_vector->tx.work_limit;
	struct igc_ring *tx_ring = q_vector->tx.ring;
	unsigned int i = tx_ring->next_to_clean;
	struct igc_tx_buffer *tx_buffer;
	union igc_adv_tx_desc *tx_desc;

	if (test_bit(__IGC_DOWN, &adapter->state))
		return true;

	tx_buffer = &tx_ring->tx_buffer_info[i];
	tx_desc = IGC_TX_DESC(tx_ring, i);
	i -= tx_ring->count;

	do {
		union igc_adv_tx_desc *eop_desc = tx_buffer->next_to_watch;

		/* if next_to_watch is not set then there is no work pending */
		if (!eop_desc)
			break;

		/* prevent any other reads prior to eop_desc */
		smp_rmb();

		/* if DD is not set pending work has not been completed */
		if (!(eop_desc->wb.status & cpu_to_le32(IGC_TXD_STAT_DD)))
			break;

		/* clear next_to_watch to prevent false hangs */
		tx_buffer->next_to_watch = NULL;

		/* update the statistics for this packet */
		total_bytes += tx_buffer->bytecount;
		total_packets += tx_buffer->gso_segs;

		/* free the skb */
		napi_consume_skb(tx_buffer->skb, napi_budget);

		/* unmap skb header data */
		dma_unmap_single(tx_ring->dev,
				 dma_unmap_addr(tx_buffer, dma),
				 dma_unmap_len(tx_buffer, len),
				 DMA_TO_DEVICE);

		/* clear tx_buffer data */
		dma_unmap_len_set(tx_buffer, len, 0);

		/* clear last DMA location and unmap remaining buffers */
		while (tx_desc != eop_desc) {
			tx_buffer++;
			tx_desc++;
			i++;
			if (unlikely(!i)) {
				i -= tx_ring->count;
				tx_buffer = tx_ring->tx_buffer_info;
				tx_desc = IGC_TX_DESC(tx_ring, 0);
			}

			/* unmap any remaining paged data */
			if (dma_unmap_len(tx_buffer, len)) {
				dma_unmap_page(tx_ring->dev,
					       dma_unmap_addr(tx_buffer, dma),
					       dma_unmap_len(tx_buffer, len),
					       DMA_TO_DEVICE);
				dma_unmap_len_set(tx_buffer, len, 0);
			}
		}

		/* move us one more past the eop_desc for start of next pkt */
		tx_buffer++;
		tx_desc++;
		i++;
		if (unlikely(!i)) {
			i -= tx_ring->count;
			tx_buffer = tx_ring->tx_buffer_info;
			tx_desc = IGC_TX_DESC(tx_ring, 0);
		}

		/* issue prefetch for next Tx descriptor */
		prefetch(tx_desc);

		/* update budget accounting */
		budget--;
	} while (likely(budget));

	netdev_tx_completed_queue(txring_txq(tx_ring),
				  total_packets, total_bytes);

	i += tx_ring->count;
	tx_ring->next_to_clean = i;
	u64_stats_update_begin(&tx_ring->tx_syncp);
	tx_ring->tx_stats.bytes += total_bytes;
	tx_ring->tx_stats.packets += total_packets;
	u64_stats_update_end(&tx_ring->tx_syncp);
	q_vector->tx.total_bytes += total_bytes;
	q_vector->tx.total_packets += total_packets;

	if (test_bit(IGC_RING_FLAG_TX_DETECT_HANG, &tx_ring->flags)) {
		struct igc_hw *hw = &adapter->hw;

		/* Detect a transmit hang in hardware, this serializes the
		 * check with the clearing of time_stamp and movement of i
		 */
		clear_bit(IGC_RING_FLAG_TX_DETECT_HANG, &tx_ring->flags);
		if (tx_buffer->next_to_watch &&
		    time_after(jiffies, tx_buffer->time_stamp +
		    (adapter->tx_timeout_factor * HZ)) &&
		    !(rd32(IGC_STATUS) & IGC_STATUS_TXOFF)) {
			/* detected Tx unit hang */
			netdev_err(tx_ring->netdev,
				   "Detected Tx Unit Hang\n"
				   "  Tx Queue             <%d>\n"
				   "  TDH                  <%x>\n"
				   "  TDT                  <%x>\n"
				   "  next_to_use          <%x>\n"
				   "  next_to_clean        <%x>\n"
				   "buffer_info[next_to_clean]\n"
				   "  time_stamp           <%lx>\n"
				   "  next_to_watch        <%p>\n"
				   "  jiffies              <%lx>\n"
				   "  desc.status          <%x>\n",
				   tx_ring->queue_index,
				   rd32(IGC_TDH(tx_ring->reg_idx)),
				   readl(tx_ring->tail),
				   tx_ring->next_to_use,
				   tx_ring->next_to_clean,
				   tx_buffer->time_stamp,
				   tx_buffer->next_to_watch,
				   jiffies,
				   tx_buffer->next_to_watch->wb.status);
			netif_stop_subqueue(tx_ring->netdev,
					    tx_ring->queue_index);

			/* we are about to reset, no point in enabling stuff */
			return true;
		}
	}

#define TX_WAKE_THRESHOLD (DESC_NEEDED * 2)
	if (unlikely(total_packets &&
		     netif_carrier_ok(tx_ring->netdev) &&
		     igc_desc_unused(tx_ring) >= TX_WAKE_THRESHOLD)) {
		/* Make sure that anybody stopping the queue after this
		 * sees the new next_to_clean.
		 */
		smp_mb();
		if (__netif_subqueue_stopped(tx_ring->netdev,
					     tx_ring->queue_index) &&
		    !(test_bit(__IGC_DOWN, &adapter->state))) {
			netif_wake_subqueue(tx_ring->netdev,
					    tx_ring->queue_index);

			u64_stats_update_begin(&tx_ring->tx_syncp);
			tx_ring->tx_stats.restart_queue++;
			u64_stats_update_end(&tx_ring->tx_syncp);
		}
	}

	return !!budget;
}

static int igc_find_mac_filter(struct igc_adapter *adapter,
			       enum igc_mac_filter_type type, const u8 *addr)
{
	struct igc_hw *hw = &adapter->hw;
	int max_entries = hw->mac.rar_entry_count;
	u32 ral, rah;
	int i;

	for (i = 0; i < max_entries; i++) {
		ral = rd32(IGC_RAL(i));
		rah = rd32(IGC_RAH(i));

		if (!(rah & IGC_RAH_AV))
			continue;
		if (!!(rah & IGC_RAH_ASEL_SRC_ADDR) != type)
			continue;
		if ((rah & IGC_RAH_RAH_MASK) !=
		    le16_to_cpup((__le16 *)(addr + 4)))
			continue;
		if (ral != le32_to_cpup((__le32 *)(addr)))
			continue;

		return i;
	}

	return -1;
}

static int igc_get_avail_mac_filter_slot(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;
	int max_entries = hw->mac.rar_entry_count;
	u32 rah;
	int i;

	for (i = 0; i < max_entries; i++) {
		rah = rd32(IGC_RAH(i));

		if (!(rah & IGC_RAH_AV))
			return i;
	}

	return -1;
}

/**
 * igc_add_mac_filter() - Add MAC address filter
 * @adapter: Pointer to adapter where the filter should be added
 * @type: MAC address filter type (source or destination)
 * @addr: MAC address
 * @queue: If non-negative, queue assignment feature is enabled and frames
 *         matching the filter are enqueued onto 'queue'. Otherwise, queue
 *         assignment is disabled.
 *
 * Return: 0 in case of success, negative errno code otherwise.
 */
static int igc_add_mac_filter(struct igc_adapter *adapter,
			      enum igc_mac_filter_type type, const u8 *addr,
			      int queue)
{
	struct net_device *dev = adapter->netdev;
	int index;

	index = igc_find_mac_filter(adapter, type, addr);
	if (index >= 0)
		goto update_filter;

	index = igc_get_avail_mac_filter_slot(adapter);
	if (index < 0)
		return -ENOSPC;

	netdev_dbg(dev, "Add MAC address filter: index %d type %s address %pM queue %d\n",
		   index, type == IGC_MAC_FILTER_TYPE_DST ? "dst" : "src",
		   addr, queue);

update_filter:
	igc_set_mac_filter_hw(adapter, index, type, addr, queue);
	return 0;
}

/**
 * igc_del_mac_filter() - Delete MAC address filter
 * @adapter: Pointer to adapter where the filter should be deleted from
 * @type: MAC address filter type (source or destination)
 * @addr: MAC address
 */
static void igc_del_mac_filter(struct igc_adapter *adapter,
			       enum igc_mac_filter_type type, const u8 *addr)
{
	struct net_device *dev = adapter->netdev;
	int index;

	index = igc_find_mac_filter(adapter, type, addr);
	if (index < 0)
		return;

	if (index == 0) {
		/* If this is the default filter, we don't actually delete it.
		 * We just reset to its default value i.e. disable queue
		 * assignment.
		 */
		netdev_dbg(dev, "Disable default MAC filter queue assignment");

		igc_set_mac_filter_hw(adapter, 0, type, addr, -1);
	} else {
		netdev_dbg(dev, "Delete MAC address filter: index %d type %s address %pM\n",
			   index,
			   type == IGC_MAC_FILTER_TYPE_DST ? "dst" : "src",
			   addr);

		igc_clear_mac_filter_hw(adapter, index);
	}
}

/**
 * igc_add_vlan_prio_filter() - Add VLAN priority filter
 * @adapter: Pointer to adapter where the filter should be added
 * @prio: VLAN priority value
 * @queue: Queue number which matching frames are assigned to
 *
 * Return: 0 in case of success, negative errno code otherwise.
 */
static int igc_add_vlan_prio_filter(struct igc_adapter *adapter, int prio,
				    int queue)
{
	struct net_device *dev = adapter->netdev;
	struct igc_hw *hw = &adapter->hw;
	u32 vlanpqf;

	vlanpqf = rd32(IGC_VLANPQF);

	if (vlanpqf & IGC_VLANPQF_VALID(prio)) {
		netdev_dbg(dev, "VLAN priority filter already in use\n");
		return -EEXIST;
	}

	vlanpqf |= IGC_VLANPQF_QSEL(prio, queue);
	vlanpqf |= IGC_VLANPQF_VALID(prio);

	wr32(IGC_VLANPQF, vlanpqf);

	netdev_dbg(dev, "Add VLAN priority filter: prio %d queue %d\n",
		   prio, queue);
	return 0;
}

/**
 * igc_del_vlan_prio_filter() - Delete VLAN priority filter
 * @adapter: Pointer to adapter where the filter should be deleted from
 * @prio: VLAN priority value
 */
static void igc_del_vlan_prio_filter(struct igc_adapter *adapter, int prio)
{
	struct igc_hw *hw = &adapter->hw;
	u32 vlanpqf;

	vlanpqf = rd32(IGC_VLANPQF);

	vlanpqf &= ~IGC_VLANPQF_VALID(prio);
	vlanpqf &= ~IGC_VLANPQF_QSEL(prio, IGC_VLANPQF_QUEUE_MASK);

	wr32(IGC_VLANPQF, vlanpqf);

	netdev_dbg(adapter->netdev, "Delete VLAN priority filter: prio %d\n",
		   prio);
}

static int igc_get_avail_etype_filter_slot(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;
	int i;

	for (i = 0; i < MAX_ETYPE_FILTER; i++) {
		u32 etqf = rd32(IGC_ETQF(i));

		if (!(etqf & IGC_ETQF_FILTER_ENABLE))
			return i;
	}

	return -1;
}

/**
 * igc_add_etype_filter() - Add ethertype filter
 * @adapter: Pointer to adapter where the filter should be added
 * @etype: Ethertype value
 * @queue: If non-negative, queue assignment feature is enabled and frames
 *         matching the filter are enqueued onto 'queue'. Otherwise, queue
 *         assignment is disabled.
 *
 * Return: 0 in case of success, negative errno code otherwise.
 */
static int igc_add_etype_filter(struct igc_adapter *adapter, u16 etype,
				int queue)
{
	struct igc_hw *hw = &adapter->hw;
	int index;
	u32 etqf;

	index = igc_get_avail_etype_filter_slot(adapter);
	if (index < 0)
		return -ENOSPC;

	etqf = rd32(IGC_ETQF(index));

	etqf &= ~IGC_ETQF_ETYPE_MASK;
	etqf |= etype;

	if (queue >= 0) {
		etqf &= ~IGC_ETQF_QUEUE_MASK;
		etqf |= (queue << IGC_ETQF_QUEUE_SHIFT);
		etqf |= IGC_ETQF_QUEUE_ENABLE;
	}

	etqf |= IGC_ETQF_FILTER_ENABLE;

	wr32(IGC_ETQF(index), etqf);

	netdev_dbg(adapter->netdev, "Add ethertype filter: etype %04x queue %d\n",
		   etype, queue);
	return 0;
}

static int igc_find_etype_filter(struct igc_adapter *adapter, u16 etype)
{
	struct igc_hw *hw = &adapter->hw;
	int i;

	for (i = 0; i < MAX_ETYPE_FILTER; i++) {
		u32 etqf = rd32(IGC_ETQF(i));

		if ((etqf & IGC_ETQF_ETYPE_MASK) == etype)
			return i;
	}

	return -1;
}

/**
 * igc_del_etype_filter() - Delete ethertype filter
 * @adapter: Pointer to adapter where the filter should be deleted from
 * @etype: Ethertype value
 */
static void igc_del_etype_filter(struct igc_adapter *adapter, u16 etype)
{
	struct igc_hw *hw = &adapter->hw;
	int index;

	index = igc_find_etype_filter(adapter, etype);
	if (index < 0)
		return;

	wr32(IGC_ETQF(index), 0);

	netdev_dbg(adapter->netdev, "Delete ethertype filter: etype %04x\n",
		   etype);
}

static int igc_enable_nfc_rule(struct igc_adapter *adapter,
			       const struct igc_nfc_rule *rule)
{
	int err;

	if (rule->filter.match_flags & IGC_FILTER_FLAG_ETHER_TYPE) {
		err = igc_add_etype_filter(adapter, rule->filter.etype,
					   rule->action);
		if (err)
			return err;
	}

	if (rule->filter.match_flags & IGC_FILTER_FLAG_SRC_MAC_ADDR) {
		err = igc_add_mac_filter(adapter, IGC_MAC_FILTER_TYPE_SRC,
					 rule->filter.src_addr, rule->action);
		if (err)
			return err;
	}

	if (rule->filter.match_flags & IGC_FILTER_FLAG_DST_MAC_ADDR) {
		err = igc_add_mac_filter(adapter, IGC_MAC_FILTER_TYPE_DST,
					 rule->filter.dst_addr, rule->action);
		if (err)
			return err;
	}

	if (rule->filter.match_flags & IGC_FILTER_FLAG_VLAN_TCI) {
		int prio = (rule->filter.vlan_tci & VLAN_PRIO_MASK) >>
			   VLAN_PRIO_SHIFT;

		err = igc_add_vlan_prio_filter(adapter, prio, rule->action);
		if (err)
			return err;
	}

	return 0;
}

static void igc_disable_nfc_rule(struct igc_adapter *adapter,
				 const struct igc_nfc_rule *rule)
{
	if (rule->filter.match_flags & IGC_FILTER_FLAG_ETHER_TYPE)
		igc_del_etype_filter(adapter, rule->filter.etype);

	if (rule->filter.match_flags & IGC_FILTER_FLAG_VLAN_TCI) {
		int prio = (rule->filter.vlan_tci & VLAN_PRIO_MASK) >>
			   VLAN_PRIO_SHIFT;

		igc_del_vlan_prio_filter(adapter, prio);
	}

	if (rule->filter.match_flags & IGC_FILTER_FLAG_SRC_MAC_ADDR)
		igc_del_mac_filter(adapter, IGC_MAC_FILTER_TYPE_SRC,
				   rule->filter.src_addr);

	if (rule->filter.match_flags & IGC_FILTER_FLAG_DST_MAC_ADDR)
		igc_del_mac_filter(adapter, IGC_MAC_FILTER_TYPE_DST,
				   rule->filter.dst_addr);
}

/**
 * igc_get_nfc_rule() - Get NFC rule
 * @adapter: Pointer to adapter
 * @location: Rule location
 *
 * Context: Expects adapter->nfc_rule_lock to be held by caller.
 *
 * Return: Pointer to NFC rule at @location. If not found, NULL.
 */
struct igc_nfc_rule *igc_get_nfc_rule(struct igc_adapter *adapter,
				      u32 location)
{
	struct igc_nfc_rule *rule;

	list_for_each_entry(rule, &adapter->nfc_rule_list, list) {
		if (rule->location == location)
			return rule;
		if (rule->location > location)
			break;
	}

	return NULL;
}

/**
 * igc_del_nfc_rule() - Delete NFC rule
 * @adapter: Pointer to adapter
 * @rule: Pointer to rule to be deleted
 *
 * Disable NFC rule in hardware and delete it from adapter.
 *
 * Context: Expects adapter->nfc_rule_lock to be held by caller.
 */
void igc_del_nfc_rule(struct igc_adapter *adapter, struct igc_nfc_rule *rule)
{
	igc_disable_nfc_rule(adapter, rule);

	list_del(&rule->list);
	adapter->nfc_rule_count--;

	kfree(rule);
}

static void igc_flush_nfc_rules(struct igc_adapter *adapter)
{
	struct igc_nfc_rule *rule, *tmp;

	mutex_lock(&adapter->nfc_rule_lock);

	list_for_each_entry_safe(rule, tmp, &adapter->nfc_rule_list, list)
		igc_del_nfc_rule(adapter, rule);

	mutex_unlock(&adapter->nfc_rule_lock);
}

/**
 * igc_add_nfc_rule() - Add NFC rule
 * @adapter: Pointer to adapter
 * @rule: Pointer to rule to be added
 *
 * Enable NFC rule in hardware and add it to adapter.
 *
 * Context: Expects adapter->nfc_rule_lock to be held by caller.
 *
 * Return: 0 on success, negative errno on failure.
 */
int igc_add_nfc_rule(struct igc_adapter *adapter, struct igc_nfc_rule *rule)
{
	struct igc_nfc_rule *pred, *cur;
	int err;

	err = igc_enable_nfc_rule(adapter, rule);
	if (err)
		return err;

	pred = NULL;
	list_for_each_entry(cur, &adapter->nfc_rule_list, list) {
		if (cur->location >= rule->location)
			break;
		pred = cur;
	}

	list_add(&rule->list, pred ? &pred->list : &adapter->nfc_rule_list);
	adapter->nfc_rule_count++;
	return 0;
}

static void igc_restore_nfc_rules(struct igc_adapter *adapter)
{
	struct igc_nfc_rule *rule;

	mutex_lock(&adapter->nfc_rule_lock);

	list_for_each_entry_reverse(rule, &adapter->nfc_rule_list, list)
		igc_enable_nfc_rule(adapter, rule);

	mutex_unlock(&adapter->nfc_rule_lock);
}

static int igc_uc_sync(struct net_device *netdev, const unsigned char *addr)
{
	struct igc_adapter *adapter = netdev_priv(netdev);

	return igc_add_mac_filter(adapter, IGC_MAC_FILTER_TYPE_DST, addr, -1);
}

static int igc_uc_unsync(struct net_device *netdev, const unsigned char *addr)
{
	struct igc_adapter *adapter = netdev_priv(netdev);

	igc_del_mac_filter(adapter, IGC_MAC_FILTER_TYPE_DST, addr);
	return 0;
}

/**
 * igc_set_rx_mode - Secondary Unicast, Multicast and Promiscuous mode set
 * @netdev: network interface device structure
 *
 * The set_rx_mode entry point is called whenever the unicast or multicast
 * address lists or the network interface flags are updated.  This routine is
 * responsible for configuring the hardware for proper unicast, multicast,
 * promiscuous mode, and all-multi behavior.
 */
static void igc_set_rx_mode(struct net_device *netdev)
{
	struct igc_adapter *adapter = netdev_priv(netdev);
	struct igc_hw *hw = &adapter->hw;
	u32 rctl = 0, rlpml = MAX_JUMBO_FRAME_SIZE;
	int count;

	/* Check for Promiscuous and All Multicast modes */
	if (netdev->flags & IFF_PROMISC) {
		rctl |= IGC_RCTL_UPE | IGC_RCTL_MPE;
	} else {
		if (netdev->flags & IFF_ALLMULTI) {
			rctl |= IGC_RCTL_MPE;
		} else {
			/* Write addresses to the MTA, if the attempt fails
			 * then we should just turn on promiscuous mode so
			 * that we can at least receive multicast traffic
			 */
			count = igc_write_mc_addr_list(netdev);
			if (count < 0)
				rctl |= IGC_RCTL_MPE;
		}
	}

	/* Write addresses to available RAR registers, if there is not
	 * sufficient space to store all the addresses then enable
	 * unicast promiscuous mode
	 */
	if (__dev_uc_sync(netdev, igc_uc_sync, igc_uc_unsync))
		rctl |= IGC_RCTL_UPE;

	/* update state of unicast and multicast */
	rctl |= rd32(IGC_RCTL) & ~(IGC_RCTL_UPE | IGC_RCTL_MPE);
	wr32(IGC_RCTL, rctl);

#if (PAGE_SIZE < 8192)
	if (adapter->max_frame_size <= IGC_MAX_FRAME_BUILD_SKB)
		rlpml = IGC_MAX_FRAME_BUILD_SKB;
#endif
	wr32(IGC_RLPML, rlpml);
}

/**
 * igc_configure - configure the hardware for RX and TX
 * @adapter: private board structure
 */
static void igc_configure(struct igc_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	int i = 0;

	igc_get_hw_control(adapter);
	igc_set_rx_mode(netdev);

	igc_setup_tctl(adapter);
	igc_setup_mrqc(adapter);
	igc_setup_rctl(adapter);

	igc_set_default_mac_filter(adapter);
	igc_restore_nfc_rules(adapter);

	igc_configure_tx(adapter);
	igc_configure_rx(adapter);

	igc_rx_fifo_flush_base(&adapter->hw);

	/* call igc_desc_unused which always leaves
	 * at least 1 descriptor unused to make sure
	 * next_to_use != next_to_clean
	 */
	for (i = 0; i < adapter->num_rx_queues; i++) {
		struct igc_ring *ring = adapter->rx_ring[i];

		igc_alloc_rx_buffers(ring, igc_desc_unused(ring));
	}
}

/**
 * igc_write_ivar - configure ivar for given MSI-X vector
 * @hw: pointer to the HW structure
 * @msix_vector: vector number we are allocating to a given ring
 * @index: row index of IVAR register to write within IVAR table
 * @offset: column offset of in IVAR, should be multiple of 8
 *
 * The IVAR table consists of 2 columns,
 * each containing an cause allocation for an Rx and Tx ring, and a
 * variable number of rows depending on the number of queues supported.
 */
static void igc_write_ivar(struct igc_hw *hw, int msix_vector,
			   int index, int offset)
{
	u32 ivar = array_rd32(IGC_IVAR0, index);

	/* clear any bits that are currently set */
	ivar &= ~((u32)0xFF << offset);

	/* write vector and valid bit */
	ivar |= (msix_vector | IGC_IVAR_VALID) << offset;

	array_wr32(IGC_IVAR0, index, ivar);
}

static void igc_assign_vector(struct igc_q_vector *q_vector, int msix_vector)
{
	struct igc_adapter *adapter = q_vector->adapter;
	struct igc_hw *hw = &adapter->hw;
	int rx_queue = IGC_N0_QUEUE;
	int tx_queue = IGC_N0_QUEUE;

	if (q_vector->rx.ring)
		rx_queue = q_vector->rx.ring->reg_idx;
	if (q_vector->tx.ring)
		tx_queue = q_vector->tx.ring->reg_idx;

	switch (hw->mac.type) {
	case igc_i225:
		if (rx_queue > IGC_N0_QUEUE)
			igc_write_ivar(hw, msix_vector,
				       rx_queue >> 1,
				       (rx_queue & 0x1) << 4);
		if (tx_queue > IGC_N0_QUEUE)
			igc_write_ivar(hw, msix_vector,
				       tx_queue >> 1,
				       ((tx_queue & 0x1) << 4) + 8);
		q_vector->eims_value = BIT(msix_vector);
		break;
	default:
		WARN_ONCE(hw->mac.type != igc_i225, "Wrong MAC type\n");
		break;
	}

	/* add q_vector eims value to global eims_enable_mask */
	adapter->eims_enable_mask |= q_vector->eims_value;

	/* configure q_vector to set itr on first interrupt */
	q_vector->set_itr = 1;
}

/**
 * igc_configure_msix - Configure MSI-X hardware
 * @adapter: Pointer to adapter structure
 *
 * igc_configure_msix sets up the hardware to properly
 * generate MSI-X interrupts.
 */
static void igc_configure_msix(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;
	int i, vector = 0;
	u32 tmp;

	adapter->eims_enable_mask = 0;

	/* set vector for other causes, i.e. link changes */
	switch (hw->mac.type) {
	case igc_i225:
		/* Turn on MSI-X capability first, or our settings
		 * won't stick.  And it will take days to debug.
		 */
		wr32(IGC_GPIE, IGC_GPIE_MSIX_MODE |
		     IGC_GPIE_PBA | IGC_GPIE_EIAME |
		     IGC_GPIE_NSICR);

		/* enable msix_other interrupt */
		adapter->eims_other = BIT(vector);
		tmp = (vector++ | IGC_IVAR_VALID) << 8;

		wr32(IGC_IVAR_MISC, tmp);
		break;
	default:
		/* do nothing, since nothing else supports MSI-X */
		break;
	} /* switch (hw->mac.type) */

	adapter->eims_enable_mask |= adapter->eims_other;

	for (i = 0; i < adapter->num_q_vectors; i++)
		igc_assign_vector(adapter->q_vector[i], vector++);

	wrfl();
}

/**
 * igc_irq_enable - Enable default interrupt generation settings
 * @adapter: board private structure
 */
static void igc_irq_enable(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;

	if (adapter->msix_entries) {
		u32 ims = IGC_IMS_LSC | IGC_IMS_DOUTSYNC | IGC_IMS_DRSTA;
		u32 regval = rd32(IGC_EIAC);

		wr32(IGC_EIAC, regval | adapter->eims_enable_mask);
		regval = rd32(IGC_EIAM);
		wr32(IGC_EIAM, regval | adapter->eims_enable_mask);
		wr32(IGC_EIMS, adapter->eims_enable_mask);
		wr32(IGC_IMS, ims);
	} else {
		wr32(IGC_IMS, IMS_ENABLE_MASK | IGC_IMS_DRSTA);
		wr32(IGC_IAM, IMS_ENABLE_MASK | IGC_IMS_DRSTA);
	}
}

/**
 * igc_irq_disable - Mask off interrupt generation on the NIC
 * @adapter: board private structure
 */
static void igc_irq_disable(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;

	if (adapter->msix_entries) {
		u32 regval = rd32(IGC_EIAM);

		wr32(IGC_EIAM, regval & ~adapter->eims_enable_mask);
		wr32(IGC_EIMC, adapter->eims_enable_mask);
		regval = rd32(IGC_EIAC);
		wr32(IGC_EIAC, regval & ~adapter->eims_enable_mask);
	}

	wr32(IGC_IAM, 0);
	wr32(IGC_IMC, ~0);
	wrfl();

	if (adapter->msix_entries) {
		int vector = 0, i;

		synchronize_irq(adapter->msix_entries[vector++].vector);

		for (i = 0; i < adapter->num_q_vectors; i++)
			synchronize_irq(adapter->msix_entries[vector++].vector);
	} else {
		synchronize_irq(adapter->pdev->irq);
	}
}

void igc_set_flag_queue_pairs(struct igc_adapter *adapter,
			      const u32 max_rss_queues)
{
	/* Determine if we need to pair queues. */
	/* If rss_queues > half of max_rss_queues, pair the queues in
	 * order to conserve interrupts due to limited supply.
	 */
	if (adapter->rss_queues > (max_rss_queues / 2))
		adapter->flags |= IGC_FLAG_QUEUE_PAIRS;
	else
		adapter->flags &= ~IGC_FLAG_QUEUE_PAIRS;
}

unsigned int igc_get_max_rss_queues(struct igc_adapter *adapter)
{
	return IGC_MAX_RX_QUEUES;
}

static void igc_init_queue_configuration(struct igc_adapter *adapter)
{
	u32 max_rss_queues;

	max_rss_queues = igc_get_max_rss_queues(adapter);
	adapter->rss_queues = min_t(u32, max_rss_queues, num_online_cpus());

	igc_set_flag_queue_pairs(adapter, max_rss_queues);
}

/**
 * igc_reset_q_vector - Reset config for interrupt vector
 * @adapter: board private structure to initialize
 * @v_idx: Index of vector to be reset
 *
 * If NAPI is enabled it will delete any references to the
 * NAPI struct. This is preparation for igc_free_q_vector.
 */
static void igc_reset_q_vector(struct igc_adapter *adapter, int v_idx)
{
	struct igc_q_vector *q_vector = adapter->q_vector[v_idx];

	/* if we're coming from igc_set_interrupt_capability, the vectors are
	 * not yet allocated
	 */
	if (!q_vector)
		return;

	if (q_vector->tx.ring)
		adapter->tx_ring[q_vector->tx.ring->queue_index] = NULL;

	if (q_vector->rx.ring)
		adapter->rx_ring[q_vector->rx.ring->queue_index] = NULL;

	netif_napi_del(&q_vector->napi);
}

/**
 * igc_free_q_vector - Free memory allocated for specific interrupt vector
 * @adapter: board private structure to initialize
 * @v_idx: Index of vector to be freed
 *
 * This function frees the memory allocated to the q_vector.
 */
static void igc_free_q_vector(struct igc_adapter *adapter, int v_idx)
{
	struct igc_q_vector *q_vector = adapter->q_vector[v_idx];

	adapter->q_vector[v_idx] = NULL;

	/* igc_get_stats64() might access the rings on this vector,
	 * we must wait a grace period before freeing it.
	 */
	if (q_vector)
		kfree_rcu(q_vector, rcu);
}

/**
 * igc_free_q_vectors - Free memory allocated for interrupt vectors
 * @adapter: board private structure to initialize
 *
 * This function frees the memory allocated to the q_vectors.  In addition if
 * NAPI is enabled it will delete any references to the NAPI struct prior
 * to freeing the q_vector.
 */
static void igc_free_q_vectors(struct igc_adapter *adapter)
{
	int v_idx = adapter->num_q_vectors;

	adapter->num_tx_queues = 0;
	adapter->num_rx_queues = 0;
	adapter->num_q_vectors = 0;

	while (v_idx--) {
		igc_reset_q_vector(adapter, v_idx);
		igc_free_q_vector(adapter, v_idx);
	}
}

/**
 * igc_update_itr - update the dynamic ITR value based on statistics
 * @q_vector: pointer to q_vector
 * @ring_container: ring info to update the itr for
 *
 * Stores a new ITR value based on packets and byte
 * counts during the last interrupt.  The advantage of per interrupt
 * computation is faster updates and more accurate ITR for the current
 * traffic pattern.  Constants in this function were computed
 * based on theoretical maximum wire speed and thresholds were set based
 * on testing data as well as attempting to minimize response time
 * while increasing bulk throughput.
 * NOTE: These calculations are only valid when operating in a single-
 * queue environment.
 */
static void igc_update_itr(struct igc_q_vector *q_vector,
			   struct igc_ring_container *ring_container)
{
	unsigned int packets = ring_container->total_packets;
	unsigned int bytes = ring_container->total_bytes;
	u8 itrval = ring_container->itr;

	/* no packets, exit with status unchanged */
	if (packets == 0)
		return;

	switch (itrval) {
	case lowest_latency:
		/* handle TSO and jumbo frames */
		if (bytes / packets > 8000)
			itrval = bulk_latency;
		else if ((packets < 5) && (bytes > 512))
			itrval = low_latency;
		break;
	case low_latency:  /* 50 usec aka 20000 ints/s */
		if (bytes > 10000) {
			/* this if handles the TSO accounting */
			if (bytes / packets > 8000)
				itrval = bulk_latency;
			else if ((packets < 10) || ((bytes / packets) > 1200))
				itrval = bulk_latency;
			else if ((packets > 35))
				itrval = lowest_latency;
		} else if (bytes / packets > 2000) {
			itrval = bulk_latency;
		} else if (packets <= 2 && bytes < 512) {
			itrval = lowest_latency;
		}
		break;
	case bulk_latency: /* 250 usec aka 4000 ints/s */
		if (bytes > 25000) {
			if (packets > 35)
				itrval = low_latency;
		} else if (bytes < 1500) {
			itrval = low_latency;
		}
		break;
	}

	/* clear work counters since we have the values we need */
	ring_container->total_bytes = 0;
	ring_container->total_packets = 0;

	/* write updated itr to ring container */
	ring_container->itr = itrval;
}

static void igc_set_itr(struct igc_q_vector *q_vector)
{
	struct igc_adapter *adapter = q_vector->adapter;
	u32 new_itr = q_vector->itr_val;
	u8 current_itr = 0;

	/* for non-gigabit speeds, just fix the interrupt rate at 4000 */
	switch (adapter->link_speed) {
	case SPEED_10:
	case SPEED_100:
		current_itr = 0;
		new_itr = IGC_4K_ITR;
		goto set_itr_now;
	default:
		break;
	}

	igc_update_itr(q_vector, &q_vector->tx);
	igc_update_itr(q_vector, &q_vector->rx);

	current_itr = max(q_vector->rx.itr, q_vector->tx.itr);

	/* conservative mode (itr 3) eliminates the lowest_latency setting */
	if (current_itr == lowest_latency &&
	    ((q_vector->rx.ring && adapter->rx_itr_setting == 3) ||
	    (!q_vector->rx.ring && adapter->tx_itr_setting == 3)))
		current_itr = low_latency;

	switch (current_itr) {
	/* counts and packets in update_itr are dependent on these numbers */
	case lowest_latency:
		new_itr = IGC_70K_ITR; /* 70,000 ints/sec */
		break;
	case low_latency:
		new_itr = IGC_20K_ITR; /* 20,000 ints/sec */
		break;
	case bulk_latency:
		new_itr = IGC_4K_ITR;  /* 4,000 ints/sec */
		break;
	default:
		break;
	}

set_itr_now:
	if (new_itr != q_vector->itr_val) {
		/* this attempts to bias the interrupt rate towards Bulk
		 * by adding intermediate steps when interrupt rate is
		 * increasing
		 */
		new_itr = new_itr > q_vector->itr_val ?
			  max((new_itr * q_vector->itr_val) /
			  (new_itr + (q_vector->itr_val >> 2)),
			  new_itr) : new_itr;
		/* Don't write the value here; it resets the adapter's
		 * internal timer, and causes us to delay far longer than
		 * we should between interrupts.  Instead, we write the ITR
		 * value at the beginning of the next interrupt so the timing
		 * ends up being correct.
		 */
		q_vector->itr_val = new_itr;
		q_vector->set_itr = 1;
	}
}

static void igc_reset_interrupt_capability(struct igc_adapter *adapter)
{
	int v_idx = adapter->num_q_vectors;

	if (adapter->msix_entries) {
		pci_disable_msix(adapter->pdev);
		kfree(adapter->msix_entries);
		adapter->msix_entries = NULL;
	} else if (adapter->flags & IGC_FLAG_HAS_MSI) {
		pci_disable_msi(adapter->pdev);
	}

	while (v_idx--)
		igc_reset_q_vector(adapter, v_idx);
}

/**
 * igc_set_interrupt_capability - set MSI or MSI-X if supported
 * @adapter: Pointer to adapter structure
 * @msix: boolean value for MSI-X capability
 *
 * Attempt to configure interrupts using the best available
 * capabilities of the hardware and kernel.
 */
static void igc_set_interrupt_capability(struct igc_adapter *adapter,
					 bool msix)
{
	int numvecs, i;
	int err;

	if (!msix)
		goto msi_only;
	adapter->flags |= IGC_FLAG_HAS_MSIX;

	/* Number of supported queues. */
	adapter->num_rx_queues = adapter->rss_queues;

	adapter->num_tx_queues = adapter->rss_queues;

	/* start with one vector for every Rx queue */
	numvecs = adapter->num_rx_queues;

	/* if Tx handler is separate add 1 for every Tx queue */
	if (!(adapter->flags & IGC_FLAG_QUEUE_PAIRS))
		numvecs += adapter->num_tx_queues;

	/* store the number of vectors reserved for queues */
	adapter->num_q_vectors = numvecs;

	/* add 1 vector for link status interrupts */
	numvecs++;

	adapter->msix_entries = kcalloc(numvecs, sizeof(struct msix_entry),
					GFP_KERNEL);

	if (!adapter->msix_entries)
		return;

	/* populate entry values */
	for (i = 0; i < numvecs; i++)
		adapter->msix_entries[i].entry = i;

	err = pci_enable_msix_range(adapter->pdev,
				    adapter->msix_entries,
				    numvecs,
				    numvecs);
	if (err > 0)
		return;

	kfree(adapter->msix_entries);
	adapter->msix_entries = NULL;

	igc_reset_interrupt_capability(adapter);

msi_only:
	adapter->flags &= ~IGC_FLAG_HAS_MSIX;

	adapter->rss_queues = 1;
	adapter->flags |= IGC_FLAG_QUEUE_PAIRS;
	adapter->num_rx_queues = 1;
	adapter->num_tx_queues = 1;
	adapter->num_q_vectors = 1;
	if (!pci_enable_msi(adapter->pdev))
		adapter->flags |= IGC_FLAG_HAS_MSI;
}

/**
 * igc_update_ring_itr - update the dynamic ITR value based on packet size
 * @q_vector: pointer to q_vector
 *
 * Stores a new ITR value based on strictly on packet size.  This
 * algorithm is less sophisticated than that used in igc_update_itr,
 * due to the difficulty of synchronizing statistics across multiple
 * receive rings.  The divisors and thresholds used by this function
 * were determined based on theoretical maximum wire speed and testing
 * data, in order to minimize response time while increasing bulk
 * throughput.
 * NOTE: This function is called only when operating in a multiqueue
 * receive environment.
 */
static void igc_update_ring_itr(struct igc_q_vector *q_vector)
{
	struct igc_adapter *adapter = q_vector->adapter;
	int new_val = q_vector->itr_val;
	int avg_wire_size = 0;
	unsigned int packets;

	/* For non-gigabit speeds, just fix the interrupt rate at 4000
	 * ints/sec - ITR timer value of 120 ticks.
	 */
	switch (adapter->link_speed) {
	case SPEED_10:
	case SPEED_100:
		new_val = IGC_4K_ITR;
		goto set_itr_val;
	default:
		break;
	}

	packets = q_vector->rx.total_packets;
	if (packets)
		avg_wire_size = q_vector->rx.total_bytes / packets;

	packets = q_vector->tx.total_packets;
	if (packets)
		avg_wire_size = max_t(u32, avg_wire_size,
				      q_vector->tx.total_bytes / packets);

	/* if avg_wire_size isn't set no work was done */
	if (!avg_wire_size)
		goto clear_counts;

	/* Add 24 bytes to size to account for CRC, preamble, and gap */
	avg_wire_size += 24;

	/* Don't starve jumbo frames */
	avg_wire_size = min(avg_wire_size, 3000);

	/* Give a little boost to mid-size frames */
	if (avg_wire_size > 300 && avg_wire_size < 1200)
		new_val = avg_wire_size / 3;
	else
		new_val = avg_wire_size / 2;

	/* conservative mode (itr 3) eliminates the lowest_latency setting */
	if (new_val < IGC_20K_ITR &&
	    ((q_vector->rx.ring && adapter->rx_itr_setting == 3) ||
	    (!q_vector->rx.ring && adapter->tx_itr_setting == 3)))
		new_val = IGC_20K_ITR;

set_itr_val:
	if (new_val != q_vector->itr_val) {
		q_vector->itr_val = new_val;
		q_vector->set_itr = 1;
	}
clear_counts:
	q_vector->rx.total_bytes = 0;
	q_vector->rx.total_packets = 0;
	q_vector->tx.total_bytes = 0;
	q_vector->tx.total_packets = 0;
}

static void igc_ring_irq_enable(struct igc_q_vector *q_vector)
{
	struct igc_adapter *adapter = q_vector->adapter;
	struct igc_hw *hw = &adapter->hw;

	if ((q_vector->rx.ring && (adapter->rx_itr_setting & 3)) ||
	    (!q_vector->rx.ring && (adapter->tx_itr_setting & 3))) {
		if (adapter->num_q_vectors == 1)
			igc_set_itr(q_vector);
		else
			igc_update_ring_itr(q_vector);
	}

	if (!test_bit(__IGC_DOWN, &adapter->state)) {
		if (adapter->msix_entries)
			wr32(IGC_EIMS, q_vector->eims_value);
		else
			igc_irq_enable(adapter);
	}
}

static void igc_add_ring(struct igc_ring *ring,
			 struct igc_ring_container *head)
{
	head->ring = ring;
	head->count++;
}

/**
 * igc_cache_ring_register - Descriptor ring to register mapping
 * @adapter: board private structure to initialize
 *
 * Once we know the feature-set enabled for the device, we'll cache
 * the register offset the descriptor ring is assigned to.
 */
static void igc_cache_ring_register(struct igc_adapter *adapter)
{
	int i = 0, j = 0;

	switch (adapter->hw.mac.type) {
	case igc_i225:
	default:
		for (; i < adapter->num_rx_queues; i++)
			adapter->rx_ring[i]->reg_idx = i;
		for (; j < adapter->num_tx_queues; j++)
			adapter->tx_ring[j]->reg_idx = j;
		break;
	}
}

/**
 * igc_poll - NAPI Rx polling callback
 * @napi: napi polling structure
 * @budget: count of how many packets we should handle
 */
static int igc_poll(struct napi_struct *napi, int budget)
{
	struct igc_q_vector *q_vector = container_of(napi,
						     struct igc_q_vector,
						     napi);
	bool clean_complete = true;
	int work_done = 0;

	if (q_vector->tx.ring)
		clean_complete = igc_clean_tx_irq(q_vector, budget);

	if (q_vector->rx.ring) {
		int cleaned = igc_clean_rx_irq(q_vector, budget);

		work_done += cleaned;
		if (cleaned >= budget)
			clean_complete = false;
	}

	/* If all work not completed, return budget and keep polling */
	if (!clean_complete)
		return budget;

	/* Exit the polling mode, but don't re-enable interrupts if stack might
	 * poll us due to busy-polling
	 */
	if (likely(napi_complete_done(napi, work_done)))
		igc_ring_irq_enable(q_vector);

	return min(work_done, budget - 1);
}

/**
 * igc_alloc_q_vector - Allocate memory for a single interrupt vector
 * @adapter: board private structure to initialize
 * @v_count: q_vectors allocated on adapter, used for ring interleaving
 * @v_idx: index of vector in adapter struct
 * @txr_count: total number of Tx rings to allocate
 * @txr_idx: index of first Tx ring to allocate
 * @rxr_count: total number of Rx rings to allocate
 * @rxr_idx: index of first Rx ring to allocate
 *
 * We allocate one q_vector.  If allocation fails we return -ENOMEM.
 */
static int igc_alloc_q_vector(struct igc_adapter *adapter,
			      unsigned int v_count, unsigned int v_idx,
			      unsigned int txr_count, unsigned int txr_idx,
			      unsigned int rxr_count, unsigned int rxr_idx)
{
	struct igc_q_vector *q_vector;
	struct igc_ring *ring;
	int ring_count;

	/* igc only supports 1 Tx and/or 1 Rx queue per vector */
	if (txr_count > 1 || rxr_count > 1)
		return -ENOMEM;

	ring_count = txr_count + rxr_count;

	/* allocate q_vector and rings */
	q_vector = adapter->q_vector[v_idx];
	if (!q_vector)
		q_vector = kzalloc(struct_size(q_vector, ring, ring_count),
				   GFP_KERNEL);
	else
		memset(q_vector, 0, struct_size(q_vector, ring, ring_count));
	if (!q_vector)
		return -ENOMEM;

	/* initialize NAPI */
	netif_napi_add(adapter->netdev, &q_vector->napi,
		       igc_poll, 64);

	/* tie q_vector and adapter together */
	adapter->q_vector[v_idx] = q_vector;
	q_vector->adapter = adapter;

	/* initialize work limits */
	q_vector->tx.work_limit = adapter->tx_work_limit;

	/* initialize ITR configuration */
	q_vector->itr_register = adapter->io_addr + IGC_EITR(0);
	q_vector->itr_val = IGC_START_ITR;

	/* initialize pointer to rings */
	ring = q_vector->ring;

	/* initialize ITR */
	if (rxr_count) {
		/* rx or rx/tx vector */
		if (!adapter->rx_itr_setting || adapter->rx_itr_setting > 3)
			q_vector->itr_val = adapter->rx_itr_setting;
	} else {
		/* tx only vector */
		if (!adapter->tx_itr_setting || adapter->tx_itr_setting > 3)
			q_vector->itr_val = adapter->tx_itr_setting;
	}

	if (txr_count) {
		/* assign generic ring traits */
		ring->dev = &adapter->pdev->dev;
		ring->netdev = adapter->netdev;

		/* configure backlink on ring */
		ring->q_vector = q_vector;

		/* update q_vector Tx values */
		igc_add_ring(ring, &q_vector->tx);

		/* apply Tx specific ring traits */
		ring->count = adapter->tx_ring_count;
		ring->queue_index = txr_idx;

		/* assign ring to adapter */
		adapter->tx_ring[txr_idx] = ring;

		/* push pointer to next ring */
		ring++;
	}

	if (rxr_count) {
		/* assign generic ring traits */
		ring->dev = &adapter->pdev->dev;
		ring->netdev = adapter->netdev;

		/* configure backlink on ring */
		ring->q_vector = q_vector;

		/* update q_vector Rx values */
		igc_add_ring(ring, &q_vector->rx);

		/* apply Rx specific ring traits */
		ring->count = adapter->rx_ring_count;
		ring->queue_index = rxr_idx;

		/* assign ring to adapter */
		adapter->rx_ring[rxr_idx] = ring;
	}

	return 0;
}

/**
 * igc_alloc_q_vectors - Allocate memory for interrupt vectors
 * @adapter: board private structure to initialize
 *
 * We allocate one q_vector per queue interrupt.  If allocation fails we
 * return -ENOMEM.
 */
static int igc_alloc_q_vectors(struct igc_adapter *adapter)
{
	int rxr_remaining = adapter->num_rx_queues;
	int txr_remaining = adapter->num_tx_queues;
	int rxr_idx = 0, txr_idx = 0, v_idx = 0;
	int q_vectors = adapter->num_q_vectors;
	int err;

	if (q_vectors >= (rxr_remaining + txr_remaining)) {
		for (; rxr_remaining; v_idx++) {
			err = igc_alloc_q_vector(adapter, q_vectors, v_idx,
						 0, 0, 1, rxr_idx);

			if (err)
				goto err_out;

			/* update counts and index */
			rxr_remaining--;
			rxr_idx++;
		}
	}

	for (; v_idx < q_vectors; v_idx++) {
		int rqpv = DIV_ROUND_UP(rxr_remaining, q_vectors - v_idx);
		int tqpv = DIV_ROUND_UP(txr_remaining, q_vectors - v_idx);

		err = igc_alloc_q_vector(adapter, q_vectors, v_idx,
					 tqpv, txr_idx, rqpv, rxr_idx);

		if (err)
			goto err_out;

		/* update counts and index */
		rxr_remaining -= rqpv;
		txr_remaining -= tqpv;
		rxr_idx++;
		txr_idx++;
	}

	return 0;

err_out:
	adapter->num_tx_queues = 0;
	adapter->num_rx_queues = 0;
	adapter->num_q_vectors = 0;

	while (v_idx--)
		igc_free_q_vector(adapter, v_idx);

	return -ENOMEM;
}

/**
 * igc_init_interrupt_scheme - initialize interrupts, allocate queues/vectors
 * @adapter: Pointer to adapter structure
 * @msix: boolean for MSI-X capability
 *
 * This function initializes the interrupts and allocates all of the queues.
 */
static int igc_init_interrupt_scheme(struct igc_adapter *adapter, bool msix)
{
	struct net_device *dev = adapter->netdev;
	int err = 0;

	igc_set_interrupt_capability(adapter, msix);

	err = igc_alloc_q_vectors(adapter);
	if (err) {
		netdev_err(dev, "Unable to allocate memory for vectors\n");
		goto err_alloc_q_vectors;
	}

	igc_cache_ring_register(adapter);

	return 0;

err_alloc_q_vectors:
	igc_reset_interrupt_capability(adapter);
	return err;
}

/**
 * igc_sw_init - Initialize general software structures (struct igc_adapter)
 * @adapter: board private structure to initialize
 *
 * igc_sw_init initializes the Adapter private data structure.
 * Fields are initialized based on PCI device information and
 * OS network device settings (MTU size).
 */
static int igc_sw_init(struct igc_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	struct pci_dev *pdev = adapter->pdev;
	struct igc_hw *hw = &adapter->hw;

	pci_read_config_word(pdev, PCI_COMMAND, &hw->bus.pci_cmd_word);

	/* set default ring sizes */
	adapter->tx_ring_count = IGC_DEFAULT_TXD;
	adapter->rx_ring_count = IGC_DEFAULT_RXD;

	/* set default ITR values */
	adapter->rx_itr_setting = IGC_DEFAULT_ITR;
	adapter->tx_itr_setting = IGC_DEFAULT_ITR;

	/* set default work limits */
	adapter->tx_work_limit = IGC_DEFAULT_TX_WORK;

	/* adjust max frame to be at least the size of a standard frame */
	adapter->max_frame_size = netdev->mtu + ETH_HLEN + ETH_FCS_LEN +
				VLAN_HLEN;
	adapter->min_frame_size = ETH_ZLEN + ETH_FCS_LEN;

	mutex_init(&adapter->nfc_rule_lock);
	INIT_LIST_HEAD(&adapter->nfc_rule_list);
	adapter->nfc_rule_count = 0;

	spin_lock_init(&adapter->stats64_lock);
	/* Assume MSI-X interrupts, will be checked during IRQ allocation */
	adapter->flags |= IGC_FLAG_HAS_MSIX;

	igc_init_queue_configuration(adapter);

	/* This call may decrease the number of queues */
	if (igc_init_interrupt_scheme(adapter, true)) {
		netdev_err(netdev, "Unable to allocate memory for queues\n");
		return -ENOMEM;
	}

	/* Explicitly disable IRQ since the NIC can be in any state. */
	igc_irq_disable(adapter);

	set_bit(__IGC_DOWN, &adapter->state);

	return 0;
}

/**
 * igc_up - Open the interface and prepare it to handle traffic
 * @adapter: board private structure
 */
void igc_up(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;
	int i = 0;

	/* hardware has been reset, we need to reload some things */
	igc_configure(adapter);

	clear_bit(__IGC_DOWN, &adapter->state);

	for (i = 0; i < adapter->num_q_vectors; i++)
		napi_enable(&adapter->q_vector[i]->napi);

	if (adapter->msix_entries)
		igc_configure_msix(adapter);
	else
		igc_assign_vector(adapter->q_vector[0], 0);

	/* Clear any pending interrupts. */
	rd32(IGC_ICR);
	igc_irq_enable(adapter);

	netif_tx_start_all_queues(adapter->netdev);

	/* start the watchdog. */
	hw->mac.get_link_status = 1;
	schedule_work(&adapter->watchdog_task);
}

/**
 * igc_update_stats - Update the board statistics counters
 * @adapter: board private structure
 */
void igc_update_stats(struct igc_adapter *adapter)
{
	struct rtnl_link_stats64 *net_stats = &adapter->stats64;
	struct pci_dev *pdev = adapter->pdev;
	struct igc_hw *hw = &adapter->hw;
	u64 _bytes, _packets;
	u64 bytes, packets;
	unsigned int start;
	u32 mpc;
	int i;

	/* Prevent stats update while adapter is being reset, or if the pci
	 * connection is down.
	 */
	if (adapter->link_speed == 0)
		return;
	if (pci_channel_offline(pdev))
		return;

	packets = 0;
	bytes = 0;

	rcu_read_lock();
	for (i = 0; i < adapter->num_rx_queues; i++) {
		struct igc_ring *ring = adapter->rx_ring[i];
		u32 rqdpc = rd32(IGC_RQDPC(i));

		if (hw->mac.type >= igc_i225)
			wr32(IGC_RQDPC(i), 0);

		if (rqdpc) {
			ring->rx_stats.drops += rqdpc;
			net_stats->rx_fifo_errors += rqdpc;
		}

		do {
			start = u64_stats_fetch_begin_irq(&ring->rx_syncp);
			_bytes = ring->rx_stats.bytes;
			_packets = ring->rx_stats.packets;
		} while (u64_stats_fetch_retry_irq(&ring->rx_syncp, start));
		bytes += _bytes;
		packets += _packets;
	}

	net_stats->rx_bytes = bytes;
	net_stats->rx_packets = packets;

	packets = 0;
	bytes = 0;
	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct igc_ring *ring = adapter->tx_ring[i];

		do {
			start = u64_stats_fetch_begin_irq(&ring->tx_syncp);
			_bytes = ring->tx_stats.bytes;
			_packets = ring->tx_stats.packets;
		} while (u64_stats_fetch_retry_irq(&ring->tx_syncp, start));
		bytes += _bytes;
		packets += _packets;
	}
	net_stats->tx_bytes = bytes;
	net_stats->tx_packets = packets;
	rcu_read_unlock();

	/* read stats registers */
	adapter->stats.crcerrs += rd32(IGC_CRCERRS);
	adapter->stats.gprc += rd32(IGC_GPRC);
	adapter->stats.gorc += rd32(IGC_GORCL);
	rd32(IGC_GORCH); /* clear GORCL */
	adapter->stats.bprc += rd32(IGC_BPRC);
	adapter->stats.mprc += rd32(IGC_MPRC);
	adapter->stats.roc += rd32(IGC_ROC);

	adapter->stats.prc64 += rd32(IGC_PRC64);
	adapter->stats.prc127 += rd32(IGC_PRC127);
	adapter->stats.prc255 += rd32(IGC_PRC255);
	adapter->stats.prc511 += rd32(IGC_PRC511);
	adapter->stats.prc1023 += rd32(IGC_PRC1023);
	adapter->stats.prc1522 += rd32(IGC_PRC1522);
	adapter->stats.tlpic += rd32(IGC_TLPIC);
	adapter->stats.rlpic += rd32(IGC_RLPIC);

	mpc = rd32(IGC_MPC);
	adapter->stats.mpc += mpc;
	net_stats->rx_fifo_errors += mpc;
	adapter->stats.scc += rd32(IGC_SCC);
	adapter->stats.ecol += rd32(IGC_ECOL);
	adapter->stats.mcc += rd32(IGC_MCC);
	adapter->stats.latecol += rd32(IGC_LATECOL);
	adapter->stats.dc += rd32(IGC_DC);
	adapter->stats.rlec += rd32(IGC_RLEC);
	adapter->stats.xonrxc += rd32(IGC_XONRXC);
	adapter->stats.xontxc += rd32(IGC_XONTXC);
	adapter->stats.xoffrxc += rd32(IGC_XOFFRXC);
	adapter->stats.xofftxc += rd32(IGC_XOFFTXC);
	adapter->stats.fcruc += rd32(IGC_FCRUC);
	adapter->stats.gptc += rd32(IGC_GPTC);
	adapter->stats.gotc += rd32(IGC_GOTCL);
	rd32(IGC_GOTCH); /* clear GOTCL */
	adapter->stats.rnbc += rd32(IGC_RNBC);
	adapter->stats.ruc += rd32(IGC_RUC);
	adapter->stats.rfc += rd32(IGC_RFC);
	adapter->stats.rjc += rd32(IGC_RJC);
	adapter->stats.tor += rd32(IGC_TORH);
	adapter->stats.tot += rd32(IGC_TOTH);
	adapter->stats.tpr += rd32(IGC_TPR);

	adapter->stats.ptc64 += rd32(IGC_PTC64);
	adapter->stats.ptc127 += rd32(IGC_PTC127);
	adapter->stats.ptc255 += rd32(IGC_PTC255);
	adapter->stats.ptc511 += rd32(IGC_PTC511);
	adapter->stats.ptc1023 += rd32(IGC_PTC1023);
	adapter->stats.ptc1522 += rd32(IGC_PTC1522);

	adapter->stats.mptc += rd32(IGC_MPTC);
	adapter->stats.bptc += rd32(IGC_BPTC);

	adapter->stats.tpt += rd32(IGC_TPT);
	adapter->stats.colc += rd32(IGC_COLC);
	adapter->stats.colc += rd32(IGC_RERC);

	adapter->stats.algnerrc += rd32(IGC_ALGNERRC);

	adapter->stats.tsctc += rd32(IGC_TSCTC);

	adapter->stats.iac += rd32(IGC_IAC);

	/* Fill out the OS statistics structure */
	net_stats->multicast = adapter->stats.mprc;
	net_stats->collisions = adapter->stats.colc;

	/* Rx Errors */

	/* RLEC on some newer hardware can be incorrect so build
	 * our own version based on RUC and ROC
	 */
	net_stats->rx_errors = adapter->stats.rxerrc +
		adapter->stats.crcerrs + adapter->stats.algnerrc +
		adapter->stats.ruc + adapter->stats.roc +
		adapter->stats.cexterr;
	net_stats->rx_length_errors = adapter->stats.ruc +
				      adapter->stats.roc;
	net_stats->rx_crc_errors = adapter->stats.crcerrs;
	net_stats->rx_frame_errors = adapter->stats.algnerrc;
	net_stats->rx_missed_errors = adapter->stats.mpc;

	/* Tx Errors */
	net_stats->tx_errors = adapter->stats.ecol +
			       adapter->stats.latecol;
	net_stats->tx_aborted_errors = adapter->stats.ecol;
	net_stats->tx_window_errors = adapter->stats.latecol;
	net_stats->tx_carrier_errors = adapter->stats.tncrs;

	/* Tx Dropped needs to be maintained elsewhere */

	/* Management Stats */
	adapter->stats.mgptc += rd32(IGC_MGTPTC);
	adapter->stats.mgprc += rd32(IGC_MGTPRC);
	adapter->stats.mgpdc += rd32(IGC_MGTPDC);
}

/**
 * igc_down - Close the interface
 * @adapter: board private structure
 */
void igc_down(struct igc_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	struct igc_hw *hw = &adapter->hw;
	u32 tctl, rctl;
	int i = 0;

	set_bit(__IGC_DOWN, &adapter->state);

	igc_ptp_suspend(adapter);

	if (pci_device_is_present(adapter->pdev)) {
		/* disable receives in the hardware */
		rctl = rd32(IGC_RCTL);
		wr32(IGC_RCTL, rctl & ~IGC_RCTL_EN);
		/* flush and sleep below */
	}
	/* set trans_start so we don't get spurious watchdogs during reset */
	netif_trans_update(netdev);

	netif_carrier_off(netdev);
	netif_tx_stop_all_queues(netdev);

	if (pci_device_is_present(adapter->pdev)) {
		/* disable transmits in the hardware */
		tctl = rd32(IGC_TCTL);
		tctl &= ~IGC_TCTL_EN;
		wr32(IGC_TCTL, tctl);
		/* flush both disables and wait for them to finish */
		wrfl();
		usleep_range(10000, 20000);

		igc_irq_disable(adapter);
	}

	adapter->flags &= ~IGC_FLAG_NEED_LINK_UPDATE;

	for (i = 0; i < adapter->num_q_vectors; i++) {
		if (adapter->q_vector[i]) {
			napi_synchronize(&adapter->q_vector[i]->napi);
			napi_disable(&adapter->q_vector[i]->napi);
		}
	}

	del_timer_sync(&adapter->watchdog_timer);
	del_timer_sync(&adapter->phy_info_timer);

	/* record the stats before reset*/
	spin_lock(&adapter->stats64_lock);
	igc_update_stats(adapter);
	spin_unlock(&adapter->stats64_lock);

	adapter->link_speed = 0;
	adapter->link_duplex = 0;

	if (!pci_channel_offline(adapter->pdev))
		igc_reset(adapter);

	/* clear VLAN promisc flag so VFTA will be updated if necessary */
	adapter->flags &= ~IGC_FLAG_VLAN_PROMISC;

	igc_clean_all_tx_rings(adapter);
	igc_clean_all_rx_rings(adapter);
}

void igc_reinit_locked(struct igc_adapter *adapter)
{
	while (test_and_set_bit(__IGC_RESETTING, &adapter->state))
		usleep_range(1000, 2000);
	igc_down(adapter);
	igc_up(adapter);
	clear_bit(__IGC_RESETTING, &adapter->state);
}

static void igc_reset_task(struct work_struct *work)
{
	struct igc_adapter *adapter;

	adapter = container_of(work, struct igc_adapter, reset_task);

	rtnl_lock();
	/* If we're already down or resetting, just bail */
	if (test_bit(__IGC_DOWN, &adapter->state) ||
	    test_bit(__IGC_RESETTING, &adapter->state)) {
		rtnl_unlock();
		return;
	}

	igc_rings_dump(adapter);
	igc_regs_dump(adapter);
	netdev_err(adapter->netdev, "Reset adapter\n");
	igc_reinit_locked(adapter);
	rtnl_unlock();
}

/**
 * igc_change_mtu - Change the Maximum Transfer Unit
 * @netdev: network interface device structure
 * @new_mtu: new value for maximum frame size
 *
 * Returns 0 on success, negative on failure
 */
static int igc_change_mtu(struct net_device *netdev, int new_mtu)
{
	int max_frame = new_mtu + ETH_HLEN + ETH_FCS_LEN + VLAN_HLEN;
	struct igc_adapter *adapter = netdev_priv(netdev);

	/* adjust max frame to be at least the size of a standard frame */
	if (max_frame < (ETH_FRAME_LEN + ETH_FCS_LEN))
		max_frame = ETH_FRAME_LEN + ETH_FCS_LEN;

	while (test_and_set_bit(__IGC_RESETTING, &adapter->state))
		usleep_range(1000, 2000);

	/* igc_down has a dependency on max_frame_size */
	adapter->max_frame_size = max_frame;

	if (netif_running(netdev))
		igc_down(adapter);

	netdev_dbg(netdev, "changing MTU from %d to %d\n", netdev->mtu, new_mtu);
	netdev->mtu = new_mtu;

	if (netif_running(netdev))
		igc_up(adapter);
	else
		igc_reset(adapter);

	clear_bit(__IGC_RESETTING, &adapter->state);

	return 0;
}

/**
 * igc_get_stats64 - Get System Network Statistics
 * @netdev: network interface device structure
 * @stats: rtnl_link_stats64 pointer
 *
 * Returns the address of the device statistics structure.
 * The statistics are updated here and also from the timer callback.
 */
static void igc_get_stats64(struct net_device *netdev,
			    struct rtnl_link_stats64 *stats)
{
	struct igc_adapter *adapter = netdev_priv(netdev);

	spin_lock(&adapter->stats64_lock);
	if (!test_bit(__IGC_RESETTING, &adapter->state))
		igc_update_stats(adapter);
	memcpy(stats, &adapter->stats64, sizeof(*stats));
	spin_unlock(&adapter->stats64_lock);
}

static netdev_features_t igc_fix_features(struct net_device *netdev,
					  netdev_features_t features)
{
	/* Since there is no support for separate Rx/Tx vlan accel
	 * enable/disable make sure Tx flag is always in same state as Rx.
	 */
	if (features & NETIF_F_HW_VLAN_CTAG_RX)
		features |= NETIF_F_HW_VLAN_CTAG_TX;
	else
		features &= ~NETIF_F_HW_VLAN_CTAG_TX;

	return features;
}

static int igc_set_features(struct net_device *netdev,
			    netdev_features_t features)
{
	netdev_features_t changed = netdev->features ^ features;
	struct igc_adapter *adapter = netdev_priv(netdev);

	/* Add VLAN support */
	if (!(changed & (NETIF_F_RXALL | NETIF_F_NTUPLE)))
		return 0;

	if (!(features & NETIF_F_NTUPLE))
		igc_flush_nfc_rules(adapter);

	netdev->features = features;

	if (netif_running(netdev))
		igc_reinit_locked(adapter);
	else
		igc_reset(adapter);

	return 1;
}

static netdev_features_t
igc_features_check(struct sk_buff *skb, struct net_device *dev,
		   netdev_features_t features)
{
	unsigned int network_hdr_len, mac_hdr_len;

	/* Make certain the headers can be described by a context descriptor */
	mac_hdr_len = skb_network_header(skb) - skb->data;
	if (unlikely(mac_hdr_len > IGC_MAX_MAC_HDR_LEN))
		return features & ~(NETIF_F_HW_CSUM |
				    NETIF_F_SCTP_CRC |
				    NETIF_F_HW_VLAN_CTAG_TX |
				    NETIF_F_TSO |
				    NETIF_F_TSO6);

	network_hdr_len = skb_checksum_start(skb) - skb_network_header(skb);
	if (unlikely(network_hdr_len >  IGC_MAX_NETWORK_HDR_LEN))
		return features & ~(NETIF_F_HW_CSUM |
				    NETIF_F_SCTP_CRC |
				    NETIF_F_TSO |
				    NETIF_F_TSO6);

	/* We can only support IPv4 TSO in tunnels if we can mangle the
	 * inner IP ID field, so strip TSO if MANGLEID is not supported.
	 */
	if (skb->encapsulation && !(features & NETIF_F_TSO_MANGLEID))
		features &= ~NETIF_F_TSO;

	return features;
}

static void igc_tsync_interrupt(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;
	u32 tsicr = rd32(IGC_TSICR);
	u32 ack = 0;

	if (tsicr & IGC_TSICR_TXTS) {
		/* retrieve hardware timestamp */
		schedule_work(&adapter->ptp_tx_work);
		ack |= IGC_TSICR_TXTS;
	}

	/* acknowledge the interrupts */
	wr32(IGC_TSICR, ack);
}

/**
 * igc_msix_other - msix other interrupt handler
 * @irq: interrupt number
 * @data: pointer to a q_vector
 */
static irqreturn_t igc_msix_other(int irq, void *data)
{
	struct igc_adapter *adapter = data;
	struct igc_hw *hw = &adapter->hw;
	u32 icr = rd32(IGC_ICR);

	/* reading ICR causes bit 31 of EICR to be cleared */
	if (icr & IGC_ICR_DRSTA)
		schedule_work(&adapter->reset_task);

	if (icr & IGC_ICR_DOUTSYNC) {
		/* HW is reporting DMA is out of sync */
		adapter->stats.doosync++;
	}

	if (icr & IGC_ICR_LSC) {
		hw->mac.get_link_status = 1;
		/* guard against interrupt when we're going down */
		if (!test_bit(__IGC_DOWN, &adapter->state))
			mod_timer(&adapter->watchdog_timer, jiffies + 1);
	}

	if (icr & IGC_ICR_TS)
		igc_tsync_interrupt(adapter);

	wr32(IGC_EIMS, adapter->eims_other);

	return IRQ_HANDLED;
}

static void igc_write_itr(struct igc_q_vector *q_vector)
{
	u32 itr_val = q_vector->itr_val & IGC_QVECTOR_MASK;

	if (!q_vector->set_itr)
		return;

	if (!itr_val)
		itr_val = IGC_ITR_VAL_MASK;

	itr_val |= IGC_EITR_CNT_IGNR;

	writel(itr_val, q_vector->itr_register);
	q_vector->set_itr = 0;
}

static irqreturn_t igc_msix_ring(int irq, void *data)
{
	struct igc_q_vector *q_vector = data;

	/* Write the ITR value calculated from the previous interrupt. */
	igc_write_itr(q_vector);

	napi_schedule(&q_vector->napi);

	return IRQ_HANDLED;
}

/**
 * igc_request_msix - Initialize MSI-X interrupts
 * @adapter: Pointer to adapter structure
 *
 * igc_request_msix allocates MSI-X vectors and requests interrupts from the
 * kernel.
 */
static int igc_request_msix(struct igc_adapter *adapter)
{
	unsigned int num_q_vectors = adapter->num_q_vectors;
	int i = 0, err = 0, vector = 0, free_vector = 0;
	struct net_device *netdev = adapter->netdev;

	err = request_irq(adapter->msix_entries[vector].vector,
			  &igc_msix_other, 0, netdev->name, adapter);
	if (err)
		goto err_out;

	if (num_q_vectors > MAX_Q_VECTORS) {
		num_q_vectors = MAX_Q_VECTORS;
		dev_warn(&adapter->pdev->dev,
			 "The number of queue vectors (%d) is higher than max allowed (%d)\n",
			 adapter->num_q_vectors, MAX_Q_VECTORS);
	}
	for (i = 0; i < num_q_vectors; i++) {
		struct igc_q_vector *q_vector = adapter->q_vector[i];

		vector++;

		q_vector->itr_register = adapter->io_addr + IGC_EITR(vector);

		if (q_vector->rx.ring && q_vector->tx.ring)
			sprintf(q_vector->name, "%s-TxRx-%u", netdev->name,
				q_vector->rx.ring->queue_index);
		else if (q_vector->tx.ring)
			sprintf(q_vector->name, "%s-tx-%u", netdev->name,
				q_vector->tx.ring->queue_index);
		else if (q_vector->rx.ring)
			sprintf(q_vector->name, "%s-rx-%u", netdev->name,
				q_vector->rx.ring->queue_index);
		else
			sprintf(q_vector->name, "%s-unused", netdev->name);

		err = request_irq(adapter->msix_entries[vector].vector,
				  igc_msix_ring, 0, q_vector->name,
				  q_vector);
		if (err)
			goto err_free;
	}

	igc_configure_msix(adapter);
	return 0;

err_free:
	/* free already assigned IRQs */
	free_irq(adapter->msix_entries[free_vector++].vector, adapter);

	vector--;
	for (i = 0; i < vector; i++) {
		free_irq(adapter->msix_entries[free_vector++].vector,
			 adapter->q_vector[i]);
	}
err_out:
	return err;
}

/**
 * igc_clear_interrupt_scheme - reset the device to a state of no interrupts
 * @adapter: Pointer to adapter structure
 *
 * This function resets the device so that it has 0 rx queues, tx queues, and
 * MSI-X interrupts allocated.
 */
static void igc_clear_interrupt_scheme(struct igc_adapter *adapter)
{
	igc_free_q_vectors(adapter);
	igc_reset_interrupt_capability(adapter);
}

/* Need to wait a few seconds after link up to get diagnostic information from
 * the phy
 */
static void igc_update_phy_info(struct timer_list *t)
{
	struct igc_adapter *adapter = from_timer(adapter, t, phy_info_timer);

	igc_get_phy_info(&adapter->hw);
}

/**
 * igc_has_link - check shared code for link and determine up/down
 * @adapter: pointer to driver private info
 */
bool igc_has_link(struct igc_adapter *adapter)
{
	struct igc_hw *hw = &adapter->hw;
	bool link_active = false;

	/* get_link_status is set on LSC (link status) interrupt or
	 * rx sequence error interrupt.  get_link_status will stay
	 * false until the igc_check_for_link establishes link
	 * for copper adapters ONLY
	 */
	if (!hw->mac.get_link_status)
		return true;
	hw->mac.ops.check_for_link(hw);
	link_active = !hw->mac.get_link_status;

	if (hw->mac.type == igc_i225) {
		if (!netif_carrier_ok(adapter->netdev)) {
			adapter->flags &= ~IGC_FLAG_NEED_LINK_UPDATE;
		} else if (!(adapter->flags & IGC_FLAG_NEED_LINK_UPDATE)) {
			adapter->flags |= IGC_FLAG_NEED_LINK_UPDATE;
			adapter->link_check_timeout = jiffies;
		}
	}

	return link_active;
}

/**
 * igc_watchdog - Timer Call-back
 * @t: timer for the watchdog
 */
static void igc_watchdog(struct timer_list *t)
{
	struct igc_adapter *adapter = from_timer(adapter, t, watchdog_timer);
	/* Do the rest outside of interrupt context */
	schedule_work(&adapter->watchdog_task);
}

static void igc_watchdog_task(struct work_struct *work)
{
	struct igc_adapter *adapter = container_of(work,
						   struct igc_adapter,
						   watchdog_task);
	struct net_device *netdev = adapter->netdev;
	struct igc_hw *hw = &adapter->hw;
	struct igc_phy_info *phy = &hw->phy;
	u16 phy_data, retry_count = 20;
	u32 link;
	int i;

	link = igc_has_link(adapter);

	if (adapter->flags & IGC_FLAG_NEED_LINK_UPDATE) {
		if (time_after(jiffies, (adapter->link_check_timeout + HZ)))
			adapter->flags &= ~IGC_FLAG_NEED_LINK_UPDATE;
		else
			link = false;
	}

	if (link) {
		/* Cancel scheduled suspend requests. */
		pm_runtime_resume(netdev->dev.parent);

		if (!netif_carrier_ok(netdev)) {
			u32 ctrl;

			hw->mac.ops.get_speed_and_duplex(hw,
							 &adapter->link_speed,
							 &adapter->link_duplex);

			ctrl = rd32(IGC_CTRL);
			/* Link status message must follow this format */
			netdev_info(netdev,
				    "NIC Link is Up %d Mbps %s Duplex, Flow Control: %s\n",
				    adapter->link_speed,
				    adapter->link_duplex == FULL_DUPLEX ?
				    "Full" : "Half",
				    (ctrl & IGC_CTRL_TFCE) &&
				    (ctrl & IGC_CTRL_RFCE) ? "RX/TX" :
				    (ctrl & IGC_CTRL_RFCE) ?  "RX" :
				    (ctrl & IGC_CTRL_TFCE) ?  "TX" : "None");

			/* disable EEE if enabled */
			if ((adapter->flags & IGC_FLAG_EEE) &&
			    adapter->link_duplex == HALF_DUPLEX) {
				netdev_info(netdev,
					    "EEE Disabled: unsupported at half duplex. Re-enable using ethtool when at full duplex\n");
				adapter->hw.dev_spec._base.eee_enable = false;
				adapter->flags &= ~IGC_FLAG_EEE;
			}

			/* check if SmartSpeed worked */
			igc_check_downshift(hw);
			if (phy->speed_downgraded)
				netdev_warn(netdev, "Link Speed was downgraded by SmartSpeed\n");

			/* adjust timeout factor according to speed/duplex */
			adapter->tx_timeout_factor = 1;
			switch (adapter->link_speed) {
			case SPEED_10:
				adapter->tx_timeout_factor = 14;
				break;
			case SPEED_100:
			case SPEED_1000:
			case SPEED_2500:
				adapter->tx_timeout_factor = 7;
				break;
			}

			if (adapter->link_speed != SPEED_1000)
				goto no_wait;

			/* wait for Remote receiver status OK */
retry_read_status:
			if (!igc_read_phy_reg(hw, PHY_1000T_STATUS,
					      &phy_data)) {
				if (!(phy_data & SR_1000T_REMOTE_RX_STATUS) &&
				    retry_count) {
					msleep(100);
					retry_count--;
					goto retry_read_status;
				} else if (!retry_count) {
					netdev_err(netdev, "exceed max 2 second\n");
				}
			} else {
				netdev_err(netdev, "read 1000Base-T Status Reg\n");
			}
no_wait:
			netif_carrier_on(netdev);

			/* link state has changed, schedule phy info update */
			if (!test_bit(__IGC_DOWN, &adapter->state))
				mod_timer(&adapter->phy_info_timer,
					  round_jiffies(jiffies + 2 * HZ));
		}
	} else {
		if (netif_carrier_ok(netdev)) {
			adapter->link_speed = 0;
			adapter->link_duplex = 0;

			/* Links status message must follow this format */
			netdev_info(netdev, "NIC Link is Down\n");
			netif_carrier_off(netdev);

			/* link state has changed, schedule phy info update */
			if (!test_bit(__IGC_DOWN, &adapter->state))
				mod_timer(&adapter->phy_info_timer,
					  round_jiffies(jiffies + 2 * HZ));

			/* link is down, time to check for alternate media */
			if (adapter->flags & IGC_FLAG_MAS_ENABLE) {
				if (adapter->flags & IGC_FLAG_MEDIA_RESET) {
					schedule_work(&adapter->reset_task);
					/* return immediately */
					return;
				}
			}
			pm_schedule_suspend(netdev->dev.parent,
					    MSEC_PER_SEC * 5);

		/* also check for alternate media here */
		} else if (!netif_carrier_ok(netdev) &&
			   (adapter->flags & IGC_FLAG_MAS_ENABLE)) {
			if (adapter->flags & IGC_FLAG_MEDIA_RESET) {
				schedule_work(&adapter->reset_task);
				/* return immediately */
				return;
			}
		}
	}

	spin_lock(&adapter->stats64_lock);
	igc_update_stats(adapter);
	spin_unlock(&adapter->stats64_lock);

	for (i = 0; i < adapter->num_tx_queues; i++) {
		struct igc_ring *tx_ring = adapter->tx_ring[i];

		if (!netif_carrier_ok(netdev)) {
			/* We've lost link, so the controller stops DMA,
			 * but we've got queued Tx work that's never going
			 * to get done, so reset controller to flush Tx.
			 * (Do the reset outside of interrupt context).
			 */
			if (igc_desc_unused(tx_ring) + 1 < tx_ring->count) {
				adapter->tx_timeout_count++;
				schedule_work(&adapter->reset_task);
				/* return immediately since reset is imminent */
				return;
			}
		}

		/* Force detection of hung controller every watchdog period */
		set_bit(IGC_RING_FLAG_TX_DETECT_HANG, &tx_ring->flags);
	}

	/* Cause software interrupt to ensure Rx ring is cleaned */
	if (adapter->flags & IGC_FLAG_HAS_MSIX) {
		u32 eics = 0;

		for (i = 0; i < adapter->num_q_vectors; i++)
			eics |= adapter->q_vector[i]->eims_value;
		wr32(IGC_EICS, eics);
	} else {
		wr32(IGC_ICS, IGC_ICS_RXDMT0);
	}

	igc_ptp_tx_hang(adapter);

	/* Reset the timer */
	if (!test_bit(__IGC_DOWN, &adapter->state)) {
		if (adapter->flags & IGC_FLAG_NEED_LINK_UPDATE)
			mod_timer(&adapter->watchdog_timer,
				  round_jiffies(jiffies +  HZ));
		else
			mod_timer(&adapter->watchdog_timer,
				  round_jiffies(jiffies + 2 * HZ));
	}
}

/**
 * igc_intr_msi - Interrupt Handler
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 */
static irqreturn_t igc_intr_msi(int irq, void *data)
{
	struct igc_adapter *adapter = data;
	struct igc_q_vector *q_vector = adapter->q_vector[0];
	struct igc_hw *hw = &adapter->hw;
	/* read ICR disables interrupts using IAM */
	u32 icr = rd32(IGC_ICR);

	igc_write_itr(q_vector);

	if (icr & IGC_ICR_DRSTA)
		schedule_work(&adapter->reset_task);

	if (icr & IGC_ICR_DOUTSYNC) {
		/* HW is reporting DMA is out of sync */
		adapter->stats.doosync++;
	}

	if (icr & (IGC_ICR_RXSEQ | IGC_ICR_LSC)) {
		hw->mac.get_link_status = 1;
		if (!test_bit(__IGC_DOWN, &adapter->state))
			mod_timer(&adapter->watchdog_timer, jiffies + 1);
	}

	if (icr & IGC_ICR_TS)
		igc_tsync_interrupt(adapter);

	napi_schedule(&q_vector->napi);

	return IRQ_HANDLED;
}

/**
 * igc_intr - Legacy Interrupt Handler
 * @irq: interrupt number
 * @data: pointer to a network interface device structure
 */
static irqreturn_t igc_intr(int irq, void *data)
{
	struct igc_adapter *adapter = data;
	struct igc_q_vector *q_vector = adapter->q_vector[0];
	struct igc_hw *hw = &adapter->hw;
	/* Interrupt Auto-Mask...upon reading ICR, interrupts are masked.  No
	 * need for the IMC write
	 */
	u32 icr = rd32(IGC_ICR);

	/* IMS will not auto-mask if INT_ASSERTED is not set, and if it is
	 * not set, then the adapter didn't send an interrupt
	 */
	if (!(icr & IGC_ICR_INT_ASSERTED))
		return IRQ_NONE;

	igc_write_itr(q_vector);

	if (icr & IGC_ICR_DRSTA)
		schedule_work(&adapter->reset_task);

	if (icr & IGC_ICR_DOUTSYNC) {
		/* HW is reporting DMA is out of sync */
		adapter->stats.doosync++;
	}

	if (icr & (IGC_ICR_RXSEQ | IGC_ICR_LSC)) {
		hw->mac.get_link_status = 1;
		/* guard against interrupt when we're going down */
		if (!test_bit(__IGC_DOWN, &adapter->state))
			mod_timer(&adapter->watchdog_timer, jiffies + 1);
	}

	if (icr & IGC_ICR_TS)
		igc_tsync_interrupt(adapter);

	napi_schedule(&q_vector->napi);

	return IRQ_HANDLED;
}

static void igc_free_irq(struct igc_adapter *adapter)
{
	if (adapter->msix_entries) {
		int vector = 0, i;

		free_irq(adapter->msix_entries[vector++].vector, adapter);

		for (i = 0; i < adapter->num_q_vectors; i++)
			free_irq(adapter->msix_entries[vector++].vector,
				 adapter->q_vector[i]);
	} else {
		free_irq(adapter->pdev->irq, adapter);
	}
}

/**
 * igc_request_irq - initialize interrupts
 * @adapter: Pointer to adapter structure
 *
 * Attempts to configure interrupts using the best available
 * capabilities of the hardware and kernel.
 */
static int igc_request_irq(struct igc_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	struct pci_dev *pdev = adapter->pdev;
	int err = 0;

	if (adapter->flags & IGC_FLAG_HAS_MSIX) {
		err = igc_request_msix(adapter);
		if (!err)
			goto request_done;
		/* fall back to MSI */
		igc_free_all_tx_resources(adapter);
		igc_free_all_rx_resources(adapter);

		igc_clear_interrupt_scheme(adapter);
		err = igc_init_interrupt_scheme(adapter, false);
		if (err)
			goto request_done;
		igc_setup_all_tx_resources(adapter);
		igc_setup_all_rx_resources(adapter);
		igc_configure(adapter);
	}

	igc_assign_vector(adapter->q_vector[0], 0);

	if (adapter->flags & IGC_FLAG_HAS_MSI) {
		err = request_irq(pdev->irq, &igc_intr_msi, 0,
				  netdev->name, adapter);
		if (!err)
			goto request_done;

		/* fall back to legacy interrupts */
		igc_reset_interrupt_capability(adapter);
		adapter->flags &= ~IGC_FLAG_HAS_MSI;
	}

	err = request_irq(pdev->irq, &igc_intr, IRQF_SHARED,
			  netdev->name, adapter);

	if (err)
		netdev_err(netdev, "Error %d getting interrupt\n", err);

request_done:
	return err;
}

/**
 * __igc_open - Called when a network interface is made active
 * @netdev: network interface device structure
 * @resuming: boolean indicating if the device is resuming
 *
 * Returns 0 on success, negative value on failure
 *
 * The open entry point is called when a network interface is made
 * active by the system (IFF_UP).  At this point all resources needed
 * for transmit and receive operations are allocated, the interrupt
 * handler is registered with the OS, the watchdog timer is started,
 * and the stack is notified that the interface is ready.
 */
static int __igc_open(struct net_device *netdev, bool resuming)
{
	struct igc_adapter *adapter = netdev_priv(netdev);
	struct pci_dev *pdev = adapter->pdev;
	struct igc_hw *hw = &adapter->hw;
	int err = 0;
	int i = 0;

	/* disallow open during test */

	if (test_bit(__IGC_TESTING, &adapter->state)) {
		WARN_ON(resuming);
		return -EBUSY;
	}

	if (!resuming)
		pm_runtime_get_sync(&pdev->dev);

	netif_carrier_off(netdev);

	/* allocate transmit descriptors */
	err = igc_setup_all_tx_resources(adapter);
	if (err)
		goto err_setup_tx;

	/* allocate receive descriptors */
	err = igc_setup_all_rx_resources(adapter);
	if (err)
		goto err_setup_rx;

	igc_power_up_link(adapter);

	igc_configure(adapter);

	err = igc_request_irq(adapter);
	if (err)
		goto err_req_irq;

	/* Notify the stack of the actual queue counts. */
	err = netif_set_real_num_tx_queues(netdev, adapter->num_tx_queues);
	if (err)
		goto err_set_queues;

	err = netif_set_real_num_rx_queues(netdev, adapter->num_rx_queues);
	if (err)
		goto err_set_queues;

	clear_bit(__IGC_DOWN, &adapter->state);

	for (i = 0; i < adapter->num_q_vectors; i++)
		napi_enable(&adapter->q_vector[i]->napi);

	/* Clear any pending interrupts. */
	rd32(IGC_ICR);
	igc_irq_enable(adapter);

	if (!resuming)
		pm_runtime_put(&pdev->dev);

	netif_tx_start_all_queues(netdev);

	/* start the watchdog. */
	hw->mac.get_link_status = 1;
	schedule_work(&adapter->watchdog_task);

	return IGC_SUCCESS;

err_set_queues:
	igc_free_irq(adapter);
err_req_irq:
	igc_release_hw_control(adapter);
	igc_power_down_phy_copper_base(&adapter->hw);
	igc_free_all_rx_resources(adapter);
err_setup_rx:
	igc_free_all_tx_resources(adapter);
err_setup_tx:
	igc_reset(adapter);
	if (!resuming)
		pm_runtime_put(&pdev->dev);

	return err;
}

int igc_open(struct net_device *netdev)
{
	return __igc_open(netdev, false);
}

/**
 * __igc_close - Disables a network interface
 * @netdev: network interface device structure
 * @suspending: boolean indicating the device is suspending
 *
 * Returns 0, this is not allowed to fail
 *
 * The close entry point is called when an interface is de-activated
 * by the OS.  The hardware is still under the driver's control, but
 * needs to be disabled.  A global MAC reset is issued to stop the
 * hardware, and all transmit and receive resources are freed.
 */
static int __igc_close(struct net_device *netdev, bool suspending)
{
	struct igc_adapter *adapter = netdev_priv(netdev);
	struct pci_dev *pdev = adapter->pdev;

	WARN_ON(test_bit(__IGC_RESETTING, &adapter->state));

	if (!suspending)
		pm_runtime_get_sync(&pdev->dev);

	igc_down(adapter);

	igc_release_hw_control(adapter);

	igc_free_irq(adapter);

	igc_free_all_tx_resources(adapter);
	igc_free_all_rx_resources(adapter);

	if (!suspending)
		pm_runtime_put_sync(&pdev->dev);

	return 0;
}

int igc_close(struct net_device *netdev)
{
	if (netif_device_present(netdev) || netdev->dismantle)
		return __igc_close(netdev, false);
	return 0;
}

/**
 * igc_ioctl - Access the hwtstamp interface
 * @netdev: network interface device structure
 * @ifr: interface request data
 * @cmd: ioctl command
 **/
static int igc_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	switch (cmd) {
	case SIOCGHWTSTAMP:
		return igc_ptp_get_ts_config(netdev, ifr);
	case SIOCSHWTSTAMP:
		return igc_ptp_set_ts_config(netdev, ifr);
	default:
		return -EOPNOTSUPP;
	}
}

static int igc_save_launchtime_params(struct igc_adapter *adapter, int queue,
				      bool enable)
{
	struct igc_ring *ring;
	int i;

	if (queue < 0 || queue >= adapter->num_tx_queues)
		return -EINVAL;

	ring = adapter->tx_ring[queue];
	ring->launchtime_enable = enable;

	if (adapter->base_time)
		return 0;

	adapter->cycle_time = NSEC_PER_SEC;

	for (i = 0; i < adapter->num_tx_queues; i++) {
		ring = adapter->tx_ring[i];
		ring->start_time = 0;
		ring->end_time = NSEC_PER_SEC;
	}

	return 0;
}

static bool is_base_time_past(ktime_t base_time, const struct timespec64 *now)
{
	struct timespec64 b;

	b = ktime_to_timespec64(base_time);

	return timespec64_compare(now, &b) > 0;
}

static bool validate_schedule(struct igc_adapter *adapter,
			      const struct tc_taprio_qopt_offload *qopt)
{
	int queue_uses[IGC_MAX_TX_QUEUES] = { };
	struct timespec64 now;
	size_t n;

	if (qopt->cycle_time_extension)
		return false;

	igc_ptp_read(adapter, &now);

	/* If we program the controller's BASET registers with a time
	 * in the future, it will hold all the packets until that
	 * time, causing a lot of TX Hangs, so to avoid that, we
	 * reject schedules that would start in the future.
	 */
	if (!is_base_time_past(qopt->base_time, &now))
		return false;

	for (n = 0; n < qopt->num_entries; n++) {
		const struct tc_taprio_sched_entry *e;
		int i;

		e = &qopt->entries[n];

		/* i225 only supports "global" frame preemption
		 * settings.
		 */
		if (e->command != TC_TAPRIO_CMD_SET_GATES)
			return false;

		for (i = 0; i < adapter->num_tx_queues; i++) {
			if (e->gate_mask & BIT(i))
				queue_uses[i]++;

			if (queue_uses[i] > 1)
				return false;
		}
	}

	return true;
}

static int igc_tsn_enable_launchtime(struct igc_adapter *adapter,
				     struct tc_etf_qopt_offload *qopt)
{
	struct igc_hw *hw = &adapter->hw;
	int err;

	if (hw->mac.type != igc_i225)
		return -EOPNOTSUPP;

	err = igc_save_launchtime_params(adapter, qopt->queue, qopt->enable);
	if (err)
		return err;

	return igc_tsn_offload_apply(adapter);
}

static int igc_save_qbv_schedule(struct igc_adapter *adapter,
				 struct tc_taprio_qopt_offload *qopt)
{
	u32 start_time = 0, end_time = 0;
	size_t n;

	if (!qopt->enable) {
		adapter->base_time = 0;
		return 0;
	}

	if (adapter->base_time)
		return -EALREADY;

	if (!validate_schedule(adapter, qopt))
		return -EINVAL;

	adapter->cycle_time = qopt->cycle_time;
	adapter->base_time = qopt->base_time;

	/* FIXME: be a little smarter about cases when the gate for a
	 * queue stays open for more than one entry.
	 */
	for (n = 0; n < qopt->num_entries; n++) {
		struct tc_taprio_sched_entry *e = &qopt->entries[n];
		int i;

		end_time += e->interval;

		for (i = 0; i < adapter->num_tx_queues; i++) {
			struct igc_ring *ring = adapter->tx_ring[i];

			if (!(e->gate_mask & BIT(i)))
				continue;

			ring->start_time = start_time;
			ring->end_time = end_time;
		}

		start_time += e->interval;
	}

	return 0;
}

static int igc_tsn_enable_qbv_scheduling(struct igc_adapter *adapter,
					 struct tc_taprio_qopt_offload *qopt)
{
	struct igc_hw *hw = &adapter->hw;
	int err;

	if (hw->mac.type != igc_i225)
		return -EOPNOTSUPP;

	err = igc_save_qbv_schedule(adapter, qopt);
	if (err)
		return err;

	return igc_tsn_offload_apply(adapter);
}

static int igc_setup_tc(struct net_device *dev, enum tc_setup_type type,
			void *type_data)
{
	struct igc_adapter *adapter = netdev_priv(dev);

	switch (type) {
	case TC_SETUP_QDISC_TAPRIO:
		return igc_tsn_enable_qbv_scheduling(adapter, type_data);

	case TC_SETUP_QDISC_ETF:
		return igc_tsn_enable_launchtime(adapter, type_data);

	default:
		return -EOPNOTSUPP;
	}
}

static const struct net_device_ops igc_netdev_ops = {
	.ndo_open		= igc_open,
	.ndo_stop		= igc_close,
	.ndo_start_xmit		= igc_xmit_frame,
	.ndo_set_rx_mode	= igc_set_rx_mode,
	.ndo_set_mac_address	= igc_set_mac,
	.ndo_change_mtu		= igc_change_mtu,
	.ndo_get_stats64	= igc_get_stats64,
	.ndo_fix_features	= igc_fix_features,
	.ndo_set_features	= igc_set_features,
	.ndo_features_check	= igc_features_check,
	.ndo_do_ioctl		= igc_ioctl,
	.ndo_setup_tc		= igc_setup_tc,
};

/* PCIe configuration access */
void igc_read_pci_cfg(struct igc_hw *hw, u32 reg, u16 *value)
{
	struct igc_adapter *adapter = hw->back;

	pci_read_config_word(adapter->pdev, reg, value);
}

void igc_write_pci_cfg(struct igc_hw *hw, u32 reg, u16 *value)
{
	struct igc_adapter *adapter = hw->back;

	pci_write_config_word(adapter->pdev, reg, *value);
}

s32 igc_read_pcie_cap_reg(struct igc_hw *hw, u32 reg, u16 *value)
{
	struct igc_adapter *adapter = hw->back;

	if (!pci_is_pcie(adapter->pdev))
		return -IGC_ERR_CONFIG;

	pcie_capability_read_word(adapter->pdev, reg, value);

	return IGC_SUCCESS;
}

s32 igc_write_pcie_cap_reg(struct igc_hw *hw, u32 reg, u16 *value)
{
	struct igc_adapter *adapter = hw->back;

	if (!pci_is_pcie(adapter->pdev))
		return -IGC_ERR_CONFIG;

	pcie_capability_write_word(adapter->pdev, reg, *value);

	return IGC_SUCCESS;
}

u32 igc_rd32(struct igc_hw *hw, u32 reg)
{
	struct igc_adapter *igc = container_of(hw, struct igc_adapter, hw);
	u8 __iomem *hw_addr = READ_ONCE(hw->hw_addr);
	u32 value = 0;

	value = readl(&hw_addr[reg]);

	/* reads should not return all F's */
	if (!(~value) && (!reg || !(~readl(hw_addr)))) {
		struct net_device *netdev = igc->netdev;

		hw->hw_addr = NULL;
		netif_device_detach(netdev);
		netdev_err(netdev, "PCIe link lost, device now detached\n");
		WARN(pci_device_is_present(igc->pdev),
		     "igc: Failed to read reg 0x%x!\n", reg);
	}

	return value;
}

int igc_set_spd_dplx(struct igc_adapter *adapter, u32 spd, u8 dplx)
{
	struct igc_mac_info *mac = &adapter->hw.mac;

	mac->autoneg = 0;

	/* Make sure dplx is at most 1 bit and lsb of speed is not set
	 * for the switch() below to work
	 */
	if ((spd & 1) || (dplx & ~1))
		goto err_inval;

	switch (spd + dplx) {
	case SPEED_10 + DUPLEX_HALF:
		mac->forced_speed_duplex = ADVERTISE_10_HALF;
		break;
	case SPEED_10 + DUPLEX_FULL:
		mac->forced_speed_duplex = ADVERTISE_10_FULL;
		break;
	case SPEED_100 + DUPLEX_HALF:
		mac->forced_speed_duplex = ADVERTISE_100_HALF;
		break;
	case SPEED_100 + DUPLEX_FULL:
		mac->forced_speed_duplex = ADVERTISE_100_FULL;
		break;
	case SPEED_1000 + DUPLEX_FULL:
		mac->autoneg = 1;
		adapter->hw.phy.autoneg_advertised = ADVERTISE_1000_FULL;
		break;
	case SPEED_1000 + DUPLEX_HALF: /* not supported */
		goto err_inval;
	case SPEED_2500 + DUPLEX_FULL:
		mac->autoneg = 1;
		adapter->hw.phy.autoneg_advertised = ADVERTISE_2500_FULL;
		break;
	case SPEED_2500 + DUPLEX_HALF: /* not supported */
	default:
		goto err_inval;
	}

	/* clear MDI, MDI(-X) override is only allowed when autoneg enabled */
	adapter->hw.phy.mdix = AUTO_ALL_MODES;

	return 0;

err_inval:
	netdev_err(adapter->netdev, "Unsupported Speed/Duplex configuration\n");
	return -EINVAL;
}

/**
 * igc_probe - Device Initialization Routine
 * @pdev: PCI device information struct
 * @ent: entry in igc_pci_tbl
 *
 * Returns 0 on success, negative on failure
 *
 * igc_probe initializes an adapter identified by a pci_dev structure.
 * The OS initialization, configuring the adapter private structure,
 * and a hardware reset occur.
 */
static int igc_probe(struct pci_dev *pdev,
		     const struct pci_device_id *ent)
{
	struct igc_adapter *adapter;
	struct net_device *netdev;
	struct igc_hw *hw;
	const struct igc_info *ei = igc_info_tbl[ent->driver_data];
	int err, pci_using_dac;

	err = pci_enable_device_mem(pdev);
	if (err)
		return err;

	pci_using_dac = 0;
	err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (!err) {
		pci_using_dac = 1;
	} else {
		err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
		if (err) {
			dev_err(&pdev->dev,
				"No usable DMA configuration, aborting\n");
			goto err_dma;
		}
	}

	err = pci_request_mem_regions(pdev, igc_driver_name);
	if (err)
		goto err_pci_reg;

	pci_enable_pcie_error_reporting(pdev);

	pci_set_master(pdev);

	err = -ENOMEM;
	netdev = alloc_etherdev_mq(sizeof(struct igc_adapter),
				   IGC_MAX_TX_QUEUES);

	if (!netdev)
		goto err_alloc_etherdev;

	SET_NETDEV_DEV(netdev, &pdev->dev);

	pci_set_drvdata(pdev, netdev);
	adapter = netdev_priv(netdev);
	adapter->netdev = netdev;
	adapter->pdev = pdev;
	hw = &adapter->hw;
	hw->back = adapter;
	adapter->port_num = hw->bus.func;
	adapter->msg_enable = netif_msg_init(debug, DEFAULT_MSG_ENABLE);

	err = pci_save_state(pdev);
	if (err)
		goto err_ioremap;

	err = -EIO;
	adapter->io_addr = ioremap(pci_resource_start(pdev, 0),
				   pci_resource_len(pdev, 0));
	if (!adapter->io_addr)
		goto err_ioremap;

	/* hw->hw_addr can be zeroed, so use adapter->io_addr for unmap */
	hw->hw_addr = adapter->io_addr;

	netdev->netdev_ops = &igc_netdev_ops;
	igc_ethtool_set_ops(netdev);
	netdev->watchdog_timeo = 5 * HZ;

	netdev->mem_start = pci_resource_start(pdev, 0);
	netdev->mem_end = pci_resource_end(pdev, 0);

	/* PCI config space info */
	hw->vendor_id = pdev->vendor;
	hw->device_id = pdev->device;
	hw->revision_id = pdev->revision;
	hw->subsystem_vendor_id = pdev->subsystem_vendor;
	hw->subsystem_device_id = pdev->subsystem_device;

	/* Copy the default MAC and PHY function pointers */
	memcpy(&hw->mac.ops, ei->mac_ops, sizeof(hw->mac.ops));
	memcpy(&hw->phy.ops, ei->phy_ops, sizeof(hw->phy.ops));

	/* Initialize skew-specific constants */
	err = ei->get_invariants(hw);
	if (err)
		goto err_sw_init;

	/* Add supported features to the features list*/
	netdev->features |= NETIF_F_SG;
	netdev->features |= NETIF_F_TSO;
	netdev->features |= NETIF_F_TSO6;
	netdev->features |= NETIF_F_TSO_ECN;
	netdev->features |= NETIF_F_RXCSUM;
	netdev->features |= NETIF_F_HW_CSUM;
	netdev->features |= NETIF_F_SCTP_CRC;
	netdev->features |= NETIF_F_HW_TC;

#define IGC_GSO_PARTIAL_FEATURES (NETIF_F_GSO_GRE | \
				  NETIF_F_GSO_GRE_CSUM | \
				  NETIF_F_GSO_IPXIP4 | \
				  NETIF_F_GSO_IPXIP6 | \
				  NETIF_F_GSO_UDP_TUNNEL | \
				  NETIF_F_GSO_UDP_TUNNEL_CSUM)

	netdev->gso_partial_features = IGC_GSO_PARTIAL_FEATURES;
	netdev->features |= NETIF_F_GSO_PARTIAL | IGC_GSO_PARTIAL_FEATURES;

	/* setup the private structure */
	err = igc_sw_init(adapter);
	if (err)
		goto err_sw_init;

	/* copy netdev features into list of user selectable features */
	netdev->hw_features |= NETIF_F_NTUPLE;
	netdev->hw_features |= netdev->features;

	if (pci_using_dac)
		netdev->features |= NETIF_F_HIGHDMA;

	/* MTU range: 68 - 9216 */
	netdev->min_mtu = ETH_MIN_MTU;
	netdev->max_mtu = MAX_STD_JUMBO_FRAME_SIZE;

	/* before reading the NVM, reset the controller to put the device in a
	 * known good starting state
	 */
	hw->mac.ops.reset_hw(hw);

	if (igc_get_flash_presence_i225(hw)) {
		if (hw->nvm.ops.validate(hw) < 0) {
			dev_err(&pdev->dev, "The NVM Checksum Is Not Valid\n");
			err = -EIO;
			goto err_eeprom;
		}
	}

	if (eth_platform_get_mac_address(&pdev->dev, hw->mac.addr)) {
		/* copy the MAC address out of the NVM */
		if (hw->mac.ops.read_mac_addr(hw))
			dev_err(&pdev->dev, "NVM Read Error\n");
	}

	memcpy(netdev->dev_addr, hw->mac.addr, netdev->addr_len);

	if (!is_valid_ether_addr(netdev->dev_addr)) {
		dev_err(&pdev->dev, "Invalid MAC Address\n");
		err = -EIO;
		goto err_eeprom;
	}

	/* configure RXPBSIZE and TXPBSIZE */
	wr32(IGC_RXPBS, I225_RXPBSIZE_DEFAULT);
	wr32(IGC_TXPBS, I225_TXPBSIZE_DEFAULT);

	timer_setup(&adapter->watchdog_timer, igc_watchdog, 0);
	timer_setup(&adapter->phy_info_timer, igc_update_phy_info, 0);

	INIT_WORK(&adapter->reset_task, igc_reset_task);
	INIT_WORK(&adapter->watchdog_task, igc_watchdog_task);

	/* Initialize link properties that are user-changeable */
	adapter->fc_autoneg = true;
	hw->mac.autoneg = true;
	hw->phy.autoneg_advertised = 0xaf;

	hw->fc.requested_mode = igc_fc_default;
	hw->fc.current_mode = igc_fc_default;

	/* By default, support wake on port A */
	adapter->flags |= IGC_FLAG_WOL_SUPPORTED;

	/* initialize the wol settings based on the eeprom settings */
	if (adapter->flags & IGC_FLAG_WOL_SUPPORTED)
		adapter->wol |= IGC_WUFC_MAG;

	device_set_wakeup_enable(&adapter->pdev->dev,
				 adapter->flags & IGC_FLAG_WOL_SUPPORTED);

	igc_ptp_init(adapter);

	/* reset the hardware with the new settings */
	igc_reset(adapter);

	/* let the f/w know that the h/w is now under the control of the
	 * driver.
	 */
	igc_get_hw_control(adapter);

	strncpy(netdev->name, "eth%d", IFNAMSIZ);
	err = register_netdev(netdev);
	if (err)
		goto err_register;

	 /* carrier off reporting is important to ethtool even BEFORE open */
	netif_carrier_off(netdev);

	/* Check if Media Autosense is enabled */
	adapter->ei = *ei;

	/* print pcie link status and MAC address */
	pcie_print_link_status(pdev);
	netdev_info(netdev, "MAC: %pM\n", netdev->dev_addr);

	dev_pm_set_driver_flags(&pdev->dev, DPM_FLAG_NEVER_SKIP);
	/* Disable EEE for internal PHY devices */
	hw->dev_spec._base.eee_enable = false;
	adapter->flags &= ~IGC_FLAG_EEE;
	igc_set_eee_i225(hw, false, false, false);

	pm_runtime_put_noidle(&pdev->dev);

	return 0;

err_register:
	igc_release_hw_control(adapter);
err_eeprom:
	if (!igc_check_reset_block(hw))
		igc_reset_phy(hw);
err_sw_init:
	igc_clear_interrupt_scheme(adapter);
	iounmap(adapter->io_addr);
err_ioremap:
	free_netdev(netdev);
err_alloc_etherdev:
	pci_disable_pcie_error_reporting(pdev);
	pci_release_mem_regions(pdev);
err_pci_reg:
err_dma:
	pci_disable_device(pdev);
	return err;
}

/**
 * igc_remove - Device Removal Routine
 * @pdev: PCI device information struct
 *
 * igc_remove is called by the PCI subsystem to alert the driver
 * that it should release a PCI device.  This could be caused by a
 * Hot-Plug event, or because the driver is going to be removed from
 * memory.
 */
static void igc_remove(struct pci_dev *pdev)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct igc_adapter *adapter = netdev_priv(netdev);

	pm_runtime_get_noresume(&pdev->dev);

	igc_flush_nfc_rules(adapter);

	igc_ptp_stop(adapter);

	set_bit(__IGC_DOWN, &adapter->state);

	del_timer_sync(&adapter->watchdog_timer);
	del_timer_sync(&adapter->phy_info_timer);

	cancel_work_sync(&adapter->reset_task);
	cancel_work_sync(&adapter->watchdog_task);

	/* Release control of h/w to f/w.  If f/w is AMT enabled, this
	 * would have already happened in close and is redundant.
	 */
	igc_release_hw_control(adapter);
	unregister_netdev(netdev);

	igc_clear_interrupt_scheme(adapter);
	pci_iounmap(pdev, adapter->io_addr);
	pci_release_mem_regions(pdev);

	free_netdev(netdev);

	pci_disable_pcie_error_reporting(pdev);

	pci_disable_device(pdev);
}

static int __igc_shutdown(struct pci_dev *pdev, bool *enable_wake,
			  bool runtime)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct igc_adapter *adapter = netdev_priv(netdev);
	u32 wufc = runtime ? IGC_WUFC_LNKC : adapter->wol;
	struct igc_hw *hw = &adapter->hw;
	u32 ctrl, rctl, status;
	bool wake;

	rtnl_lock();
	netif_device_detach(netdev);

	if (netif_running(netdev))
		__igc_close(netdev, true);

	igc_ptp_suspend(adapter);

	igc_clear_interrupt_scheme(adapter);
	rtnl_unlock();

	status = rd32(IGC_STATUS);
	if (status & IGC_STATUS_LU)
		wufc &= ~IGC_WUFC_LNKC;

	if (wufc) {
		igc_setup_rctl(adapter);
		igc_set_rx_mode(netdev);

		/* turn on all-multi mode if wake on multicast is enabled */
		if (wufc & IGC_WUFC_MC) {
			rctl = rd32(IGC_RCTL);
			rctl |= IGC_RCTL_MPE;
			wr32(IGC_RCTL, rctl);
		}

		ctrl = rd32(IGC_CTRL);
		ctrl |= IGC_CTRL_ADVD3WUC;
		wr32(IGC_CTRL, ctrl);

		/* Allow time for pending master requests to run */
		igc_disable_pcie_master(hw);

		wr32(IGC_WUC, IGC_WUC_PME_EN);
		wr32(IGC_WUFC, wufc);
	} else {
		wr32(IGC_WUC, 0);
		wr32(IGC_WUFC, 0);
	}

	wake = wufc || adapter->en_mng_pt;
	if (!wake)
		igc_power_down_phy_copper_base(&adapter->hw);
	else
		igc_power_up_link(adapter);

	if (enable_wake)
		*enable_wake = wake;

	/* Release control of h/w to f/w.  If f/w is AMT enabled, this
	 * would have already happened in close and is redundant.
	 */
	igc_release_hw_control(adapter);

	pci_disable_device(pdev);

	return 0;
}

#ifdef CONFIG_PM
static int __maybe_unused igc_runtime_suspend(struct device *dev)
{
	return __igc_shutdown(to_pci_dev(dev), NULL, 1);
}

static void igc_deliver_wake_packet(struct net_device *netdev)
{
	struct igc_adapter *adapter = netdev_priv(netdev);
	struct igc_hw *hw = &adapter->hw;
	struct sk_buff *skb;
	u32 wupl;

	wupl = rd32(IGC_WUPL) & IGC_WUPL_MASK;

	/* WUPM stores only the first 128 bytes of the wake packet.
	 * Read the packet only if we have the whole thing.
	 */
	if (wupl == 0 || wupl > IGC_WUPM_BYTES)
		return;

	skb = netdev_alloc_skb_ip_align(netdev, IGC_WUPM_BYTES);
	if (!skb)
		return;

	skb_put(skb, wupl);

	/* Ensure reads are 32-bit aligned */
	wupl = roundup(wupl, 4);

	memcpy_fromio(skb->data, hw->hw_addr + IGC_WUPM_REG(0), wupl);

	skb->protocol = eth_type_trans(skb, netdev);
	netif_rx(skb);
}

static int __maybe_unused igc_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct igc_adapter *adapter = netdev_priv(netdev);
	struct igc_hw *hw = &adapter->hw;
	u32 err, val;

	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);
	pci_save_state(pdev);

	if (!pci_device_is_present(pdev))
		return -ENODEV;
	err = pci_enable_device_mem(pdev);
	if (err) {
		netdev_err(netdev, "Cannot enable PCI device from suspend\n");
		return err;
	}
	pci_set_master(pdev);

	pci_enable_wake(pdev, PCI_D3hot, 0);
	pci_enable_wake(pdev, PCI_D3cold, 0);

	if (igc_init_interrupt_scheme(adapter, true)) {
		netdev_err(netdev, "Unable to allocate memory for queues\n");
		return -ENOMEM;
	}

	igc_reset(adapter);

	/* let the f/w know that the h/w is now under the control of the
	 * driver.
	 */
	igc_get_hw_control(adapter);

	val = rd32(IGC_WUS);
	if (val & WAKE_PKT_WUS)
		igc_deliver_wake_packet(netdev);

	wr32(IGC_WUS, ~0);

	rtnl_lock();
	if (!err && netif_running(netdev))
		err = __igc_open(netdev, true);

	if (!err)
		netif_device_attach(netdev);
	rtnl_unlock();

	return err;
}

static int __maybe_unused igc_runtime_resume(struct device *dev)
{
	return igc_resume(dev);
}

static int __maybe_unused igc_suspend(struct device *dev)
{
	return __igc_shutdown(to_pci_dev(dev), NULL, 0);
}

static int __maybe_unused igc_runtime_idle(struct device *dev)
{
	struct net_device *netdev = dev_get_drvdata(dev);
	struct igc_adapter *adapter = netdev_priv(netdev);

	if (!igc_has_link(adapter))
		pm_schedule_suspend(dev, MSEC_PER_SEC * 5);

	return -EBUSY;
}
#endif /* CONFIG_PM */

static void igc_shutdown(struct pci_dev *pdev)
{
	bool wake;

	__igc_shutdown(pdev, &wake, 0);

	if (system_state == SYSTEM_POWER_OFF) {
		pci_wake_from_d3(pdev, wake);
		pci_set_power_state(pdev, PCI_D3hot);
	}
}

/**
 *  igc_io_error_detected - called when PCI error is detected
 *  @pdev: Pointer to PCI device
 *  @state: The current PCI connection state
 *
 *  This function is called after a PCI bus error affecting
 *  this device has been detected.
 **/
static pci_ers_result_t igc_io_error_detected(struct pci_dev *pdev,
					      pci_channel_state_t state)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct igc_adapter *adapter = netdev_priv(netdev);

	netif_device_detach(netdev);

	if (state == pci_channel_io_perm_failure)
		return PCI_ERS_RESULT_DISCONNECT;

	if (netif_running(netdev))
		igc_down(adapter);
	pci_disable_device(pdev);

	/* Request a slot reset. */
	return PCI_ERS_RESULT_NEED_RESET;
}

/**
 *  igc_io_slot_reset - called after the PCI bus has been reset.
 *  @pdev: Pointer to PCI device
 *
 *  Restart the card from scratch, as if from a cold-boot. Implementation
 *  resembles the first-half of the igc_resume routine.
 **/
static pci_ers_result_t igc_io_slot_reset(struct pci_dev *pdev)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct igc_adapter *adapter = netdev_priv(netdev);
	struct igc_hw *hw = &adapter->hw;
	pci_ers_result_t result;

	if (pci_enable_device_mem(pdev)) {
		netdev_err(netdev, "Could not re-enable PCI device after reset\n");
		result = PCI_ERS_RESULT_DISCONNECT;
	} else {
		pci_set_master(pdev);
		pci_restore_state(pdev);
		pci_save_state(pdev);

		pci_enable_wake(pdev, PCI_D3hot, 0);
		pci_enable_wake(pdev, PCI_D3cold, 0);

		/* In case of PCI error, adapter loses its HW address
		 * so we should re-assign it here.
		 */
		hw->hw_addr = adapter->io_addr;

		igc_reset(adapter);
		wr32(IGC_WUS, ~0);
		result = PCI_ERS_RESULT_RECOVERED;
	}

	return result;
}

/**
 *  igc_io_resume - called when traffic can start to flow again.
 *  @pdev: Pointer to PCI device
 *
 *  This callback is called when the error recovery driver tells us that
 *  its OK to resume normal operation. Implementation resembles the
 *  second-half of the igc_resume routine.
 */
static void igc_io_resume(struct pci_dev *pdev)
{
	struct net_device *netdev = pci_get_drvdata(pdev);
	struct igc_adapter *adapter = netdev_priv(netdev);

	rtnl_lock();
	if (netif_running(netdev)) {
		if (igc_open(netdev)) {
			netdev_err(netdev, "igc_open failed after reset\n");
			return;
		}
	}

	netif_device_attach(netdev);

	/* let the f/w know that the h/w is now under the control of the
	 * driver.
	 */
	igc_get_hw_control(adapter);
	rtnl_unlock();
}

static const struct pci_error_handlers igc_err_handler = {
	.error_detected = igc_io_error_detected,
	.slot_reset = igc_io_slot_reset,
	.resume = igc_io_resume,
};

#ifdef CONFIG_PM
static const struct dev_pm_ops igc_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(igc_suspend, igc_resume)
	SET_RUNTIME_PM_OPS(igc_runtime_suspend, igc_runtime_resume,
			   igc_runtime_idle)
};
#endif

static struct pci_driver igc_driver = {
	.name     = igc_driver_name,
	.id_table = igc_pci_tbl,
	.probe    = igc_probe,
	.remove   = igc_remove,
#ifdef CONFIG_PM
	.driver.pm = &igc_pm_ops,
#endif
	.shutdown = igc_shutdown,
	.err_handler = &igc_err_handler,
};

/**
 * igc_reinit_queues - return error
 * @adapter: pointer to adapter structure
 */
int igc_reinit_queues(struct igc_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	int err = 0;

	if (netif_running(netdev))
		igc_close(netdev);

	igc_reset_interrupt_capability(adapter);

	if (igc_init_interrupt_scheme(adapter, true)) {
		netdev_err(netdev, "Unable to allocate memory for queues\n");
		return -ENOMEM;
	}

	if (netif_running(netdev))
		err = igc_open(netdev);

	return err;
}

/**
 * igc_get_hw_dev - return device
 * @hw: pointer to hardware structure
 *
 * used by hardware layer to print debugging information
 */
struct net_device *igc_get_hw_dev(struct igc_hw *hw)
{
	struct igc_adapter *adapter = hw->back;

	return adapter->netdev;
}

/**
 * igc_init_module - Driver Registration Routine
 *
 * igc_init_module is the first routine called when the driver is
 * loaded. All it does is register with the PCI subsystem.
 */
static int __init igc_init_module(void)
{
	int ret;

	pr_info("%s\n", igc_driver_string);
	pr_info("%s\n", igc_copyright);

	ret = pci_register_driver(&igc_driver);
	return ret;
}

module_init(igc_init_module);

/**
 * igc_exit_module - Driver Exit Cleanup Routine
 *
 * igc_exit_module is called just before the driver is removed
 * from memory.
 */
static void __exit igc_exit_module(void)
{
	pci_unregister_driver(&igc_driver);
}

module_exit(igc_exit_module);
/* igc_main.c */
