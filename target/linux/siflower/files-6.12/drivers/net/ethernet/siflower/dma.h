#ifndef _SFXGMAC_DMA_H
#define _SFXGMAC_DMA_H

#include <linux/clk.h>
#include <linux/genalloc.h>
#include <linux/if_vlan.h>
#include <linux/mfd/syscon.h>
#include <linux/netdevice.h>
#include <linux/regmap.h>

#ifdef CONFIG_NET_SIFLOWER_ETH_USE_INTERNAL_SRAM
#define DMA_TX_SIZE	512
#define DMA_RX_SIZE	512
#else
#define DMA_TX_SIZE	2048
#define DMA_RX_SIZE	2048
#endif
#define DPNS_HOST_PORT	6
#define DPNS_MAX_PORT	27
#define DMA_CH_MAX	4
#define DMA_CH_DISABLE	4
#define DMA_OVPORT_CH	4
#define SZ_1_5K		0x00000600
#define SZ_3K		0x00000C00

/* Either 8 (64-bit) or 16 (128-bit), configured in RTL */
#define DMA_DATAWIDTH	8

/* extra header room for skb to wifi */
#define NET_WIFI_HEADERROOM_EXTRA SKB_DATA_ALIGN(32)

/* Padding at the beginning of the allocated buffer, passed into skb_reserve */
#define BUF_PAD		(NET_SKB_PAD + NET_IP_ALIGN + NET_WIFI_HEADERROOM_EXTRA)

/* RX Buffer size, calculated by MTU + eth header + double VLAN tag + FCS */
#define BUF_SIZE(x)	((x) + ETH_HLEN + VLAN_HLEN * 2 + ETH_FCS_LEN)

/* RX Buffer alloc size, with padding and skb_shared_info, passed into
 * page_pool_dev_alloc_frag */
#define BUF_SIZE_ALLOC(x)	(SKB_DATA_ALIGN(BUF_SIZE(x) + BUF_PAD) + \
				SKB_DATA_ALIGN(sizeof(struct skb_shared_info)))

/* RX Buffer size programmed into RBSZ field, must be multiple of datawidth */
#define BUF_SIZE_ALIGN(x)	ALIGN(BUF_SIZE(x) + NET_IP_ALIGN, DMA_DATAWIDTH)

/* Maximum value of the RBSZ field */
#define BUF_SIZE_ALIGN_MAX	ALIGN_DOWN(FIELD_MAX(XGMAC_RBSZ), DMA_DATAWIDTH)

/* TODO: use mtu in priv */
#define BUF_SIZE_DEFAULT    SKB_MAX_HEAD(BUF_PAD)
#define BUF_SIZE_DEFAULT_ALIGN  ALIGN(BUF_SIZE_DEFAULT, DMA_DATAWIDTH)

/* skb handled by dpns flag */
#define SF_DPNS_FLAG        47

/* skb handled by dpns need to be forwarded */
#define SF_CB_DPNS_FORWARD             22

struct xgmac_dma_desc {
	__le32 des0;
	__le32 des1;
	__le32 des2;
	__le32 des3;
};

struct xgmac_skb_cb {
	u8 id;
	bool fastmode;
};

#define XGMAC_SKB_CB(skb)	((struct xgmac_skb_cb *)(skb)->cb)

struct xgmac_tx_info {
	dma_addr_t buf;
	bool map_as_page;
	unsigned len;
	bool last_segment;
};

struct xgmac_txq {
	struct xgmac_dma_desc *dma_tx ____cacheline_aligned_in_smp;
	struct sk_buff **tx_skbuff;
	struct xgmac_tx_info *tx_skbuff_dma;
	unsigned int cur_tx;
	unsigned int dirty_tx;
	dma_addr_t dma_tx_phy;
	dma_addr_t tx_tail_addr;
	spinlock_t lock;
	struct napi_struct napi ____cacheline_aligned_in_smp;
	u32 idx;
	u32 irq;
	bool is_busy;
};

struct xgmac_dma_rx_buffer {
	struct page *page;
	unsigned int offset;
};

struct xgmac_rxq {
	struct xgmac_dma_desc *dma_rx ____cacheline_aligned_in_smp;
	struct page_pool *page_pool;
	struct xgmac_dma_rx_buffer *buf_pool;
	unsigned int cur_rx;
	unsigned int dirty_rx;
	dma_addr_t dma_rx_phy;
	dma_addr_t rx_tail_addr;
	struct napi_struct napi ____cacheline_aligned_in_smp;
	u32 idx;
	u32 irq;
};

enum {
	DMA_CLK_AXI,
	DMA_CLK_NPU,
	DMA_CLK_CSR,
	DMA_NUM_CLKS
};

struct xgmac_dma_priv {
	void __iomem		*ioaddr;
	struct device		*dev;
	struct clk_bulk_data	clks[DMA_NUM_CLKS];
	struct net_device	napi_dev;
	/* RX Queue */
	struct xgmac_rxq	rxq[DMA_CH_MAX];

	/* TX Queue */
	struct xgmac_txq	txq[DMA_CH_MAX];

	/* associated net devices (vports) */
	struct net_device	*ndevs[DPNS_MAX_PORT];

	struct regmap		*ethsys;
	refcount_t		refcnt;
	u32			irq;
	u8			ifindex;
#ifdef CONFIG_NET_SIFLOWER_ETH_USE_INTERNAL_SRAM
	struct gen_pool		*genpool;
#endif
	u16			rx_alloc_size;
	u16			rx_buffer_size;
#if defined(CONFIG_DEBUG_FS) && defined(CONFIG_PAGE_POOL_STATS)
	struct dentry		*dbgdir;
#endif
};

netdev_tx_t xgmac_dma_xmit_fast(struct sk_buff *skb, struct net_device *dev);
int xgmac_dma_open(struct xgmac_dma_priv *priv, struct net_device *dev, u8 id);
int xgmac_dma_stop(struct xgmac_dma_priv *priv, struct net_device *dev, u8 id);

#endif
