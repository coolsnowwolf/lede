/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (c) 2021 Motor-comm Corporation. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef __FXGMAC_GMAC_H__
#define __FXGMAC_GMAC_H__

#include "fuxi-os.h"
#include "fuxi-errno.h"

// For fpga before 20210507
#define FXGMAC_FPGA_VER_B4_0507     0
#define FXGMAC_FPGA_VER_20210507    1

#define FXGMAC_DRV_NAME             "yt6801"

#define FXGMAC_DRV_DESC             "Motorcomm YT6801 Gigabit Ethernet Driver"

#define FXGMAC_MAC_REGS_OFFSET        0x2000

#define FXGMAC_EPHY_INTERRUPT_D0_OFF  0 //1: in normal D0 state, turn off ephy link change interrupt.
#define FXGMAC_ALLOC_NEW_RECBUFFER    0 //1:when rec buffer is not enough,to create rbd and rec buffer ,but  the rdb need to be continus with the initialized rdb,so close the feature

#define RESUME_MAX_TIME             3000000
#define PHY_LINK_TIMEOUT            3000
#define ESD_RESET_MAXIMUM           0

#define REGWR_RETRY_MAXIMUM         2600
#define PCIE_LINKDOWN_VALUE         0xFFFFFFFF

#define FXGMAC_MSIX_Q_VECTORS       4

#define FXGMAC_IS_CHANNEL_WITH_TX_IRQ(chId) (0 == (chId) ? 1 : 0)

/* flags for ipv6 NS offload address, local link or Global unicast */
#define FXGMAC_NS_IFA_LOCAL_LINK     1
#define FXGMAC_NS_IFA_GLOBAL_UNICAST 2

#define FXGMAX_ASPM_WAR_EN
/* Descriptor related parameters */
#if FXGMAC_TX_HANG_TIMER_ENABLED
#define FXGMAC_TX_DESC_CNT           1024
#else
#define FXGMAC_TX_DESC_CNT           256 //256 to make sure the tx ring is in the 4k range when FXGMAC_TX_HANG_TIMER_ENABLED is 0
#endif
#define FXGMAC_TX_DESC_MIN_FREE      (FXGMAC_TX_DESC_CNT >> 3)
#define FXGMAC_TX_DESC_MAX_PROC      (FXGMAC_TX_DESC_CNT >> 1)
#define FXGMAC_RX_DESC_CNT            1024
#define FXGMAC_RX_DESC_MAX_DIRTY     (FXGMAC_RX_DESC_CNT >> 3)

/* Descriptors required for maximum contiguous TSO/GSO packet */
#define FXGMAC_TX_MAX_SPLIT     ((GSO_MAX_SIZE / FXGMAC_TX_MAX_BUF_SIZE) + 1)

/* Maximum possible descriptors needed for a SKB */
#define FXGMAC_TX_MAX_DESC_NR   (MAX_SKB_FRAGS + FXGMAC_TX_MAX_SPLIT + 2)

#define FXGMAC_TX_MAX_BUF_SIZE  (0x3fff & ~(64 - 1))
#define FXGMAC_RX_MIN_BUF_SIZE  (ETH_FRAME_LEN + ETH_FCS_LEN + VLAN_HLEN)
#define FXGMAC_RX_BUF_ALIGN     64

/* Maximum Size for Splitting the Header Data
 * Keep in sync with SKB_ALLOC_SIZE
 * 3'b000: 64 bytes, 3'b001: 128 bytes
 * 3'b010: 256 bytes, 3'b011: 512 bytes
 * 3'b100: 1023 bytes ,   3'b101'3'b111: Reserved
 */
#define FXGMAC_SPH_HDSMS_SIZE       3
#define FXGMAC_SKB_ALLOC_SIZE       512

#define FXGMAC_MAX_FIFO             81920

#define FXGMAC_MAX_DMA_CHANNELS             FXGMAC_MSIX_Q_VECTORS
#define FXGMAC_DMA_STOP_TIMEOUT             5
#define FXGMAC_DMA_INTERRUPT_MASK           0x31c7
#define FXGMAC_MAX_DMA_CHANNELS_PLUS_1TX    (FXGMAC_MAX_DMA_CHANNELS + 1)

/* Default coalescing parameters */
#define FXGMAC_INIT_DMA_TX_USECS    INT_MOD_IN_US
#define FXGMAC_INIT_DMA_TX_FRAMES   25
#define FXGMAC_INIT_DMA_RX_USECS    INT_MOD_IN_US   /* 30 */
#define FXGMAC_INIT_DMA_RX_FRAMES   25
#define FXGMAC_MAX_DMA_RIWT         0xff
#define FXGMAC_MIN_DMA_RIWT         0x01

/* Flow control queue count */
#define FXGMAC_MAX_FLOW_CONTROL_QUEUES  8

/* System clock is 125 MHz */
#define FXGMAC_SYSCLOCK         125000000

/* Maximum MAC address hash table size (256 bits = 8 bytes) */
#define FXGMAC_MAC_HASH_TABLE_SIZE      8

/* wol pattern settings */
#define MAX_PATTERN_SIZE                128  // PATTERN length
#define MAX_PATTERN_COUNT               16   // pattern count
#define MAX_LPP_ARP_OFFLOAD_COUNT       1
#define MAX_LPP_NS_OFFLOAD_COUNT        2

#define MAX_WPI_LENGTH_SIZE             1536  // WPI packet.
#define PM_WAKE_PKT_ALIGN               8     // try use 64 bit boundary...

/* Receive Side Scaling */
#define FXGMAC_RSS_HASH_KEY_SIZE        40
#define FXGMAC_RSS_MAX_TABLE_SIZE       128
#define FXGMAC_RSS_LOOKUP_TABLE_TYPE    0
#define FXGMAC_RSS_HASH_KEY_TYPE        1
#define MAX_MSI_COUNT                   16   // Max Msi/Msix supported.

#define FXGMAC_STD_PACKET_MTU           1500
#define FXGMAC_JUMBO_PACKET_MTU         9014

#define NIC_MAX_TCP_OFFLOAD_SIZE        7300
#define NIC_MIN_LSO_SEGMENT_COUNT       2

/* power management */
#define FXGMAC_POWER_STATE_DOWN         0
#define FXGMAC_POWER_STATE_UP           1

#define FXGMAC_DATA_WIDTH               128

#define FXGMAC_WOL_WAIT_TIME            2 // unit 1ms

// Don't change the member variables or types, this inherits from Windows OS.
struct wol_bitmap_pattern
{
    u32 flags;
    u32 pattern_size;
    u32 mask_size;
    u8  mask_info[MAX_PATTERN_SIZE / 8];
    u8  pattern_info[MAX_PATTERN_SIZE];
    u8  pattern_offset;
    u16 pattern_crc;
};

struct led_setting
{
    u32 s0_led_setting[5];
    u32 s3_led_setting[5];
    u32 s5_led_setting[5];
    u32 disable_led_setting[5];
};

typedef struct led_setting LED_SETTING;
typedef struct wol_bitmap_pattern WOL_BITMAP_PATTERN;

typedef enum
{
    WAKE_REASON_NONE = 0,
    WAKE_REASON_MAGIC,
    WAKE_REASON_PATTERNMATCH,
    WAKE_REASON_LINK,
    WAKE_REASON_TCPSYNV4,
    WAKE_REASON_TCPSYNV6,
    WAKE_REASON_TBD,    //for wake up method like Link-change, for that, GMAC cannot identify and need more checking.
    WAKE_REASON_HW_ERR,
} WAKE_REASON;      //note, maybe we should refer to NDIS_PM_WAKE_REASON_TYPE to avoid duplication definition....

/* Helper macro for descriptor handling
 *  Always use FXGMAC_GET_DESC_DATA to access the descriptor data
 */
#if 0 //No need to round
#define FXGMAC_GET_DESC_DATA(ring, idx) ({              \
    typeof(ring) _ring = (ring);                        \
    ((_ring)->desc_data_head +                          \
     ((idx) & ((_ring)->dma_desc_count - 1)));          \
})
#endif

#define FXGMAC_GET_DESC_DATA(ring, idx) ((ring)->desc_data_head + (idx))
#define FXGMAC_GET_ENTRY(x, size) ((x + 1) & (size - 1))

struct fxgmac_pdata;

enum fxgmac_int {
    FXGMAC_INT_DMA_CH_SR_TI,
    FXGMAC_INT_DMA_CH_SR_TPS,
    FXGMAC_INT_DMA_CH_SR_TBU,
    FXGMAC_INT_DMA_CH_SR_RI,
    FXGMAC_INT_DMA_CH_SR_RBU,
    FXGMAC_INT_DMA_CH_SR_RPS,
    FXGMAC_INT_DMA_CH_SR_TI_RI,
    FXGMAC_INT_DMA_CH_SR_FBE,
    FXGMAC_INT_DMA_ALL,
};

struct fxgmac_stats {
    /* MMC TX counters */
    u64 txoctetcount_gb;
    u64 txframecount_gb;
    u64 txbroadcastframes_g;
    u64 txmulticastframes_g;
    u64 tx64octets_gb;
    u64 tx65to127octets_gb;
    u64 tx128to255octets_gb;
    u64 tx256to511octets_gb;
    u64 tx512to1023octets_gb;
    u64 tx1024tomaxoctets_gb;
    u64 txunicastframes_gb;
    u64 txmulticastframes_gb;
    u64 txbroadcastframes_gb;
    u64 txunderflowerror;
    u64 txsinglecollision_g;
    u64 txmultiplecollision_g;
    u64 txdeferredframes;
    u64 txlatecollisionframes;
    u64 txexcessivecollisionframes;
    u64 txcarriererrorframes;
    u64 txoctetcount_g;
    u64 txframecount_g;
    u64 txexcessivedeferralerror;
    u64 txpauseframes;
    u64 txvlanframes_g;
    u64 txoversize_g;

    /* MMC RX counters */
    u64 rxframecount_gb;
    u64 rxoctetcount_gb;
    u64 rxoctetcount_g;
    u64 rxbroadcastframes_g;
    u64 rxmulticastframes_g;
    u64 rxcrcerror;
    u64 rxalignerror;
    u64 rxrunterror;
    u64 rxjabbererror;
    u64 rxundersize_g;
    u64 rxoversize_g;
    u64 rx64octets_gb;
    u64 rx65to127octets_gb;
    u64 rx128to255octets_gb;
    u64 rx256to511octets_gb;
    u64 rx512to1023octets_gb;
    u64 rx1024tomaxoctets_gb;
    u64 rxunicastframes_g;
    u64 rxlengtherror;
    u64 rxoutofrangetype;
    u64 rxpauseframes;
    u64 rxfifooverflow;
    u64 rxvlanframes_gb;
    u64 rxwatchdogerror;
    u64 rxreceiveerrorframe;
    u64 rxcontrolframe_g;

    /* Extra counters */
    u64 tx_tso_packets;
    u64 rx_split_header_packets;
    u64 tx_process_stopped;
    u64 rx_process_stopped;
    u64 tx_buffer_unavailable;
    u64 rx_buffer_unavailable;
    u64 fatal_bus_error;
    u64 tx_vlan_packets;
    u64 rx_vlan_packets;
    u64 napi_poll_isr;
    u64 napi_poll_txtimer;
    u64 cnt_alive_txtimer;

    u64 ephy_poll_timer_cnt;
    u64 mgmt_int_isr;
};

struct fxgmac_ring_buf {
    struct sk_buff*     skb;
    DMA_ADDR_T          skb_dma;
    unsigned int        skb_len;
};

/* Common Tx and Rx DMA hardware descriptor */
struct fxgmac_dma_desc {
    __le32 desc0;
    __le32 desc1;
    __le32 desc2;
    __le32 desc3;
};

/* Page allocation related values */
struct fxgmac_page_alloc {
    struct page*    pages;
    unsigned int    pages_len;
    unsigned int    pages_offset;
    DMA_ADDR_T      pages_dma;
};

/* Ring entry buffer data */
struct fxgmac_buffer_data {
    struct fxgmac_page_alloc    pa;
    struct fxgmac_page_alloc    pa_unmap;

    DMA_ADDR_T                  dma_base;
    unsigned long               dma_off;
    unsigned int                dma_len;
};

/* Tx-related desc data */
struct fxgmac_tx_desc_data {
    unsigned int packets;       /* BQL packet count */
    unsigned int bytes;         /* BQL byte count */
};

/* Rx-related desc data */
struct fxgmac_rx_desc_data {
    struct fxgmac_buffer_data   hdr;        /* Header locations */
    struct fxgmac_buffer_data   buf;        /* Payload locations */

    unsigned short              hdr_len;    /* Length of received header */
    unsigned short              len;        /* Length of received packet */
};

struct fxgmac_pkt_info {
    struct sk_buff*     skb;

    unsigned int        attributes;

    unsigned int        errors;

    /* descriptors needed for this packet */
    unsigned int        desc_count;
    unsigned int        length;

    unsigned int        tx_packets;
    unsigned int        tx_bytes;

    unsigned int        header_len;
    unsigned int        tcp_header_len;
    unsigned int        tcp_payload_len;
    unsigned short      mss;

    unsigned short      vlan_ctag;

    u64                 rx_tstamp;

    u32                 rss_hash;
    RSS_HASH_TYPE       rss_hash_type;
};

struct fxgmac_desc_data {
    /* dma_desc: Virtual address of descriptor
     *  dma_desc_addr: DMA address of descriptor
     */
    struct fxgmac_dma_desc*     dma_desc;
    DMA_ADDR_T                  dma_desc_addr;

    /* skb: Virtual address of SKB
     *  skb_dma: DMA address of SKB data
     *  skb_dma_len: Length of SKB DMA area
     */
    struct sk_buff*             skb;
    DMA_ADDR_T                  skb_dma;
    unsigned int                skb_dma_len;

    /* Tx/Rx -related data */
    struct fxgmac_tx_desc_data  tx;
    struct fxgmac_rx_desc_data  rx;

    unsigned int                mapped_as_page;
#if 0
    /* Incomplete receive save location.  If the budget is exhausted
     * or the last descriptor (last normal descriptor or a following
     * context descriptor) has not been DMA'd yet the current state
     * of the receive processing needs to be saved.
     */
    unsigned int                state_saved;
    struct {
        struct sk_buff*         skb;
        unsigned int            len;
        unsigned int            error;
    } state;
#endif
};

struct fxgmac_ring {
    /* Per packet related information */
    struct fxgmac_pkt_info          pkt_info;

    /* Virtual/DMA addresses of DMA descriptor list and the total count */
    struct fxgmac_dma_desc          *dma_desc_head;
    DMA_ADDR_T                      dma_desc_head_addr;
    unsigned int                    dma_desc_count;

    /* Array of descriptor data corresponding the DMA descriptor
     * (always use the FXGMAC_GET_DESC_DATA macro to access this data)
     */
    struct fxgmac_desc_data         *desc_data_head;

    /* Page allocation for RX buffers */
    struct fxgmac_page_alloc        rx_hdr_pa;
    struct fxgmac_page_alloc        rx_buf_pa;

    /* Ring index values
     *  cur   - Tx: index of descriptor to be used for current transfer
     *          Rx: index of descriptor to check for packet availability
     *  dirty - Tx: index of descriptor to check for transfer complete
     *          Rx: index of descriptor to check for buffer reallocation
     */
    unsigned int                    cur;
    unsigned int                    dirty;

    /* Coalesce frame count used for interrupt bit setting */
    unsigned int                    coalesce_count;

    struct {
        unsigned int                xmit_more;
        unsigned int                queue_stopped;
        unsigned short              cur_mss;
        unsigned short              cur_vlan_ctag;
    } tx;
} ____cacheline_aligned;

struct fxgmac_channel {
    char                        name[16];

    /* Address of private data area for device */
    struct fxgmac_pdata*        pdata;

    /* Queue index and base address of queue's DMA registers */
    unsigned int                queue_index;

    IOMEM                       dma_regs;

    /* Per channel interrupt irq number */
    u32                         dma_irq;
    FXGMAC_CHANNEL_OF_PLATFORM  expansion;

    u32                         saved_ier;

    unsigned int                tx_timer_active;

    struct fxgmac_ring          *tx_ring;
    struct fxgmac_ring          *rx_ring;
} ____cacheline_aligned;

struct fxphy_ag_adv {
    u8 auto_neg_en : 1;
    u8 full_1000m : 1;
    u8 half_1000m : 1;
    u8 full_100m : 1;
    u8 half_100m : 1;
    u8 full_10m : 1;
    u8 half_10m : 1;
};

struct fxgmac_desc_ops {
    int (*alloc_channels_and_rings)(struct fxgmac_pdata* pdata);
    void (*free_channels_and_rings)(struct fxgmac_pdata* pdata);
    int (*map_tx_skb)(struct fxgmac_channel* channel,
        struct sk_buff* skb);
    int (*map_rx_buffer)(struct fxgmac_pdata* pdata,
        struct fxgmac_ring* ring,
        struct fxgmac_desc_data* desc_data);
    void (*unmap_desc_data)(struct fxgmac_pdata* pdata,
        struct fxgmac_desc_data* desc_data);
    void (*tx_desc_init)(struct fxgmac_pdata* pdata);
    int (*rx_desc_init)(struct fxgmac_pdata* pdata);
    /* For descriptor related operation */
    void (*tx_desc_init_channel)(struct fxgmac_channel* channel);
    void (*rx_desc_init_channel)(struct fxgmac_channel* channel);
    void (*tx_desc_reset)(struct fxgmac_desc_data* desc_data);
    void (*rx_desc_reset)(struct fxgmac_pdata* pdata,
        struct fxgmac_desc_data* desc_data,
        unsigned int index);
};

struct fxgmac_hw_ops {
    int (*init)(struct fxgmac_pdata* pdata);
    int (*exit)(struct fxgmac_pdata* pdata);
    void (*save_nonstick_reg)(struct fxgmac_pdata* pdata);
    void (*restore_nonstick_reg)(struct fxgmac_pdata* pdata);
    int (*set_gmac_register)(struct fxgmac_pdata* pdata, IOMEM address, unsigned int data);
    u32 (*get_gmac_register)(struct fxgmac_pdata* pdata, IOMEM address);
    void (*esd_restore_pcie_cfg)(struct fxgmac_pdata* pdata);

    int (*tx_complete)(struct fxgmac_dma_desc* dma_desc);

    void (*enable_tx)(struct fxgmac_pdata* pdata);
    void (*disable_tx)(struct fxgmac_pdata* pdata);
    void (*enable_rx)(struct fxgmac_pdata* pdata);
    void (*disable_rx)(struct fxgmac_pdata* pdata);
    void (*enable_channel_rx)(struct fxgmac_pdata* pdata, unsigned int queue);
    void (*enable_rx_tx_ints)(struct fxgmac_pdata* pdata);
    void (*disable_rx_tx_ints)(struct fxgmac_pdata* pdata);

    int (*enable_int)(struct fxgmac_channel* channel,
        enum fxgmac_int int_id);
    int (*disable_int)(struct fxgmac_channel* channel,
        enum fxgmac_int int_id);
    void (*set_interrupt_moderation)(struct fxgmac_pdata* pdata);
    void (*enable_msix_rxtxinterrupt)(struct fxgmac_pdata* pdata);
    void (*disable_msix_interrupt)(struct fxgmac_pdata* pdata);
    int (*enable_msix_rxtxphyinterrupt)(struct fxgmac_pdata* pdata);
    void (*enable_msix_one_interrupt)(struct fxgmac_pdata* pdata, u32 intid);
    void (*disable_msix_one_interrupt)(struct fxgmac_pdata* pdata, u32 intid);
    bool (*enable_mgm_interrupt)(struct fxgmac_pdata* pdata);
    bool (*disable_mgm_interrupt)(struct fxgmac_pdata* pdata);
    bool (*enable_source_interrupt)(struct fxgmac_pdata* pdata);
    bool (*disable_source_interrupt)(struct fxgmac_pdata* pdata);
    int  (*dismiss_all_int)(struct fxgmac_pdata* pdata);
    void (*clear_misc_int_status)(struct fxgmac_pdata* pdata);

    void (*dev_xmit)(struct fxgmac_channel* channel);
    int (*dev_read)(struct fxgmac_channel* channel);

    int (*set_mac_address)(struct fxgmac_pdata* pdata, u8* addr);
    int (*set_mac_hash)(struct fxgmac_pdata* pdata);
    int (*config_rx_mode)(struct fxgmac_pdata* pdata);
    int (*enable_rx_csum)(struct fxgmac_pdata* pdata);
    int (*disable_rx_csum)(struct fxgmac_pdata* pdata);
    void (*config_tso)(struct fxgmac_pdata *pdata);

    /* For MII speed configuration */
    int (*config_mac_speed)(struct fxgmac_pdata* pdata);
    int (*get_xlgmii_phy_status)(struct fxgmac_pdata *pdata, u32 *speed, bool *link_up, bool link_up_wait_to_complete);

    /* For descriptor related operation */
    //void (*tx_desc_init)(struct fxgmac_channel* channel);
    //void (*rx_desc_init)(struct fxgmac_channel* channel);
    //void (*tx_desc_reset)(struct fxgmac_desc_data* desc_data);
    //void (*rx_desc_reset)(struct fxgmac_pdata* pdata,
    //    struct fxgmac_desc_data* desc_data,
    //    unsigned int index);
    int (*is_last_desc)(struct fxgmac_dma_desc* dma_desc);
    int (*is_context_desc)(struct fxgmac_dma_desc* dma_desc);

    /* For Flow Control */
    int (*config_tx_flow_control)(struct fxgmac_pdata* pdata);
    int (*config_rx_flow_control)(struct fxgmac_pdata* pdata);

    /* For Jumbo Frames */
    int (*config_mtu)(struct fxgmac_pdata* pdata);
    int (*enable_jumbo)(struct fxgmac_pdata* pdata);

    /* For Vlan related config */
    int (*enable_tx_vlan)(struct fxgmac_pdata* pdata);
    int (*disable_tx_vlan)(struct fxgmac_pdata* pdata);
    int (*enable_rx_vlan_stripping)(struct fxgmac_pdata* pdata);
    int (*disable_rx_vlan_stripping)(struct fxgmac_pdata* pdata);
    int (*enable_rx_vlan_filtering)(struct fxgmac_pdata* pdata);
    int (*disable_rx_vlan_filtering)(struct fxgmac_pdata* pdata);
    int (*update_vlan_hash_table)(struct fxgmac_pdata* pdata);

    /* For RX coalescing */
    int (*config_rx_coalesce)(struct fxgmac_pdata* pdata);
    int (*config_tx_coalesce)(struct fxgmac_pdata* pdata);
    unsigned long (*usec_to_riwt)(struct fxgmac_pdata* pdata,
        unsigned int usec);
    unsigned long (*riwt_to_usec)(struct fxgmac_pdata* pdata,
        unsigned int riwt);

    /* For RX and TX threshold config */
    int (*config_rx_threshold)(struct fxgmac_pdata* pdata,
        unsigned int val);
    int (*config_tx_threshold)(struct fxgmac_pdata* pdata,
        unsigned int val);

    /* For RX and TX Store and Forward Mode config */
    int (*config_rsf_mode)(struct fxgmac_pdata* pdata,
        unsigned int val);
    int (*config_tsf_mode)(struct fxgmac_pdata* pdata,
        unsigned int val);

    /* For TX DMA Operate on Second Frame config */
    int (*config_osp_mode)(struct fxgmac_pdata* pdata);

    /* For RX and TX PBL config */
    u32 (*calculate_max_checksum_size)(struct fxgmac_pdata* pdata);
    int (*config_rx_pbl_val)(struct fxgmac_pdata* pdata);
    u32 (*get_rx_pbl_val)(struct fxgmac_pdata* pdata);
    int (*config_tx_pbl_val)(struct fxgmac_pdata* pdata);
    u32 (*get_tx_pbl_val)(struct fxgmac_pdata* pdata);
    int (*config_pblx8)(struct fxgmac_pdata* pdata);

    /* For MMC statistics */
    void (*rx_mmc_int)(struct fxgmac_pdata* pdata);
    void (*tx_mmc_int)(struct fxgmac_pdata* pdata);
    void (*read_mmc_stats)(struct fxgmac_pdata* pdata);
    bool (*update_stats_counters)(struct fxgmac_pdata* pdata, bool ephy_check_en);

    /* For Receive Side Scaling */
    int (*enable_rss)(struct fxgmac_pdata* pdata);
    int (*disable_rss)(struct fxgmac_pdata* pdata);
    u32 (*get_rss_options)(struct fxgmac_pdata* pdata);
    int (*set_rss_options)(struct fxgmac_pdata* pdata);
    int (*set_rss_hash_key)(struct fxgmac_pdata* pdata, const u8* key);
    int (*set_rss_lookup_table)(struct fxgmac_pdata* pdata, const u32* table);

    /*For Offload*/
#ifdef FXGMAC_POWER_MANAGEMENT
    void (*set_arp_offload)(struct fxgmac_pdata* pdata, unsigned char* ip_addr);
    int (*enable_arp_offload)(struct fxgmac_pdata* pdata);
    int (*disable_arp_offload)(struct fxgmac_pdata* pdata);

    /*NS offload*/
    int  (*set_ns_offload)(
        struct fxgmac_pdata* pdata,
        unsigned int index,
        unsigned char* remote_addr,
        unsigned char* solicited_addr,
        unsigned char* target_addr1,
        unsigned char* target_addr2,
        unsigned char* mac_addr);
    int (*enable_ns_offload)(struct fxgmac_pdata* pdata);
    int (*disable_ns_offload)(struct fxgmac_pdata* pdata);

    int (*enable_wake_magic_pattern)(struct fxgmac_pdata* pdata);
    int (*disable_wake_magic_pattern)(struct fxgmac_pdata* pdata);

    int (*enable_wake_link_change)(struct fxgmac_pdata* pdata);
    int (*disable_wake_link_change)(struct fxgmac_pdata* pdata);

    int (*check_wake_pattern_fifo_pointer)(struct fxgmac_pdata* pdata);
    int (*set_wake_pattern)(struct fxgmac_pdata* pdata, struct wol_bitmap_pattern* wol_pattern, u32 pattern_cnt);
    int (*enable_wake_pattern)(struct fxgmac_pdata* pdata);//int XlgmacEnableArpload(struct fxgmac_pdata* pdata,unsigned char *ip_addr)
    int (*disable_wake_pattern)(struct fxgmac_pdata* pdata);
    int (*set_wake_pattern_mask)(struct fxgmac_pdata* pdata, u32 filter_index, u8 register_index, u32 Data);
#if FXGMAC_PM_WPI_READ_FEATURE_ENABLED
    void (*get_wake_packet_indication)(struct fxgmac_pdata* pdata, int* wake_reason, u32* wake_pattern_number, u8* wpi_buf, u32 buf_size, u32* packet_size);
    void (*enable_wake_packet_indication)(struct fxgmac_pdata* pdata, int en);
#endif
#endif

    void (*reset_phy)(struct fxgmac_pdata* pdata);
    /*for release phy,phy write and read, and provide clock to GMAC. */
    void (*release_phy)(struct fxgmac_pdata* pdata);
    void (*enable_phy_check)(struct fxgmac_pdata* pdata);
    void (*disable_phy_check)(struct fxgmac_pdata* pdata);
    void (*setup_cable_loopback)(struct fxgmac_pdata* pdata);
    void (*clean_cable_loopback)(struct fxgmac_pdata* pdata);
    void (*disable_phy_sleep)(struct fxgmac_pdata* pdata);
    void (*enable_phy_sleep)(struct fxgmac_pdata* pdata);
    void (*phy_green_ethernet)(struct fxgmac_pdata* pdata);
    void (*phy_eee_feature)(struct fxgmac_pdata* pdata);
    u32 (*get_ephy_state)(struct fxgmac_pdata* pdata);
    int (*write_ephy_reg)(struct fxgmac_pdata* pdata, u32 val, u32 data);
    int (*read_ephy_reg)(struct fxgmac_pdata* pdata, u32 val, u32 __far* data);
    int (*set_ephy_autoneg_advertise)(struct fxgmac_pdata* pdata,  struct fxphy_ag_adv phy_ag_adv);
    int (*phy_config)(struct fxgmac_pdata* pdata);
    void (*close_phy_led)(struct fxgmac_pdata* pdata);
    void (*led_under_active)(struct fxgmac_pdata* pdata);
    void (*led_under_sleep)(struct fxgmac_pdata* pdata);
    void (*led_under_shutdown)(struct fxgmac_pdata* pdata);
    void (*led_under_disable)(struct fxgmac_pdata* pdata);

    /* For power management */
    void (*pre_power_down)(struct fxgmac_pdata* pdata, bool phyloopback);
    int (*diag_sanity_check)(struct fxgmac_pdata *pdata);
    int (*write_rss_lookup_table)(struct fxgmac_pdata *pdata);
    int (*get_rss_hash_key)(struct fxgmac_pdata *pdata, u8 *key_buf);
#ifdef FXGMAC_WOL_INTEGRATED_WOL_PARAMETER
    void (*config_power_down)(struct fxgmac_pdata *pdata, unsigned int wol);
#else
    void (*config_power_down)(struct fxgmac_pdata* pdata, unsigned int offloadcount, bool magic_en, bool remote_pattern_en);
#endif
    void (*config_power_up)(struct fxgmac_pdata* pdata);
    unsigned char (*set_suspend_int)(void* pdata);
    void (*set_resume_int)(struct fxgmac_pdata* pdata);
    int (*set_suspend_txrx)(struct fxgmac_pdata* pdata);
    void (*set_pwr_clock_gate)(struct fxgmac_pdata* pdata);
    void (*set_pwr_clock_ungate)(struct fxgmac_pdata* pdata);

    /* for multicast address list */
    int (*set_all_multicast_mode)(struct fxgmac_pdata* pdata, unsigned int enable);
    void (*config_multicast_mac_hash_table)(struct fxgmac_pdata* pdata, unsigned char* pmc_mac, int b_add);

    /* for packet filter-promiscuous and broadcast */
    int (*set_promiscuous_mode)(struct fxgmac_pdata* pdata, unsigned int enable);
    int (*enable_rx_broadcast)(struct fxgmac_pdata* pdata, unsigned int enable);

    /* efuse relevant operation. */
    bool (*read_patch_from_efuse_per_index)(struct fxgmac_pdata* pdata, u8 index, u32 __far* offset, u32 __far* value); /* read patch per index. */
    bool (*read_mac_subsys_from_efuse)(struct fxgmac_pdata* pdata, u8* mac_addr, u32* subsys, u32* revid);
    bool (*read_efuse_data)(struct fxgmac_pdata* pdata, u32 offset, u32 __far* value);
#ifndef COMMENT_UNUSED_CODE_TO_REDUCE_SIZE
    bool (*read_patch_from_efuse)(struct fxgmac_pdata* pdata, u32 offset, u32* value); /* read patch per index. */
    bool (*write_patch_to_efuse)(struct fxgmac_pdata* pdata, u32 offset, u32 value);
    bool (*write_patch_to_efuse_per_index)(struct fxgmac_pdata* pdata, u8 index, u32 offset, u32 value);
    bool (*write_mac_subsys_to_efuse)(struct fxgmac_pdata* pdata, u8* mac_addr, u32* subsys, u32* revid);
    bool (*read_mac_addr_from_efuse)(struct fxgmac_pdata* pdata, u8* mac_addr);
    bool (*write_mac_addr_to_efuse)(struct fxgmac_pdata* pdata, u8* mac_addr);
    bool (*efuse_load)(struct fxgmac_pdata* pdata);
    bool (*write_oob)(struct fxgmac_pdata* pdata);
    bool (*write_led)(struct fxgmac_pdata* pdata, u32 value);
    bool (*read_led_config)(struct fxgmac_pdata* pdata);
    bool (*write_led_config)(struct fxgmac_pdata* pdata);
#endif

    int (*pcie_init)(struct fxgmac_pdata* pdata, bool ltr_en, bool aspm_l1ss_en, bool aspm_l1_en, bool aspm_l0s_en);
    void (*trigger_pcie)(struct fxgmac_pdata* pdata, u32 code); // To trigger pcie sniffer for analysis.
};

/* This structure contains flags that indicate what hardware features
 * or configurations are present in the device.
 */
struct fxgmac_hw_features {
    /* HW Version */
    u32 version;

    /* HW Feature Register0 */
    u32 phyifsel;    /* PHY interface support */
    u32 vlhash;     /* VLAN Hash Filter */
    u32 sma;        /* SMA(MDIO) Interface */
    u32 rwk;        /* PMT remote wake-up packet */
    u32 mgk;        /* PMT magic packet */
    u32 mmc;        /* RMON module */
    u32 aoe;        /* ARP Offload */
    u32 ts;         /* IEEE 1588-2008 Advanced Timestamp */
    u32 eee;        /* Energy Efficient Ethernet */
    u32 tx_coe;     /* Tx Checksum Offload */
    u32 rx_coe;     /* Rx Checksum Offload */
    u32 addn_mac;   /* Additional MAC Addresses */
    u32 ts_src;     /* Timestamp Source */
    u32 sa_vlan_ins;/* Source Address or VLAN Insertion */

    /* HW Feature Register1 */
    u32 rx_fifo_size;   /* MTL Receive FIFO Size */
    u32 tx_fifo_size;   /* MTL Transmit FIFO Size */
    u32 adv_ts_hi;      /* Advance Timestamping High Word */
    u32 dma_width;      /* DMA width */
    u32 dcb;            /* DCB Feature */
    u32 sph;            /* Split Header Feature */
    u32 tso;            /* TCP Segmentation Offload */
    u32 dma_debug;      /* DMA Debug Registers */
    u32 rss;            /* Receive Side Scaling */
    u32 tc_cnt;         /* Number of Traffic Classes */
    u32 avsel;          /* AV Feature Enable */
    u32 ravsel;         /* Rx Side Only AV Feature Enable */
    u32 hash_table_size;/* Hash Table Size */
    u32 l3l4_filter_num;/* Number of L3-L4 Filters */

    /* HW Feature Register2 */
    u32 rx_q_cnt;       /* Number of MTL Receive Queues */
    u32 tx_q_cnt;       /* Number of MTL Transmit Queues */
    u32 rx_ch_cnt;      /* Number of DMA Receive Channels */
    u32 tx_ch_cnt;      /* Number of DMA Transmit Channels */
    u32 pps_out_num;    /* Number of PPS outputs */
    u32 aux_snap_num;   /* Number of Aux snapshot inputs */

    /* HW Feature Register3 */
    u32 hwfr3;
};

struct fxgmac_resources {
    IOMEM   addr;
    int     irq;
};

struct fxgmac_pdata {
    struct net_device               *netdev;
    struct device                   *dev;
    PCI_DEV                         *pdev;
    void                            *pAdapter;

    struct fxgmac_hw_ops            hw_ops;
    struct fxgmac_desc_ops          desc_ops;

    /* Device statistics */
    struct fxgmac_stats             stats;

    u32                             msg_enable;
    u32                             reg_nonstick[0x300 >> 2];

    /* MAC registers base */
    IOMEM                           mac_regs;
    IOMEM                           base_mem;

    /* Hardware features of the device */
    struct fxgmac_hw_features       hw_feat;

    /* Rings for Tx/Rx on a DMA channel */
    struct fxgmac_channel           *channel_head;
    unsigned int                    channel_count;
    unsigned int                    tx_ring_count;
    unsigned int                    rx_ring_count;
    unsigned int                    tx_desc_count;
    unsigned int                    rx_desc_count;
    unsigned int                    tx_q_count;
    unsigned int                    rx_q_count;

    /* Tx/Rx common settings */
    unsigned int                    pblx8;

    /* Tx settings */
    unsigned int                    tx_sf_mode;
    unsigned int                    tx_threshold;
    unsigned int                    tx_pbl;
    unsigned int                    tx_osp_mode;
#if FXGMAC_TX_HANG_TIMER_ENABLED
    /* for tx hang checking. 20211227 */
    unsigned int                    tx_hang_restart_queuing;
#endif

    /* Rx settings */
    unsigned int                    rx_sf_mode;
    unsigned int                    rx_threshold;
    unsigned int                    rx_pbl;

    /* Tx coalescing settings */
    unsigned int                    tx_usecs;
    unsigned int                    tx_frames;

    /* Rx coalescing settings */
    unsigned long                   rx_riwt;
    unsigned int                    rx_usecs;
    unsigned int                    rx_frames;

    /* Current Rx buffer size */
    unsigned int                    rx_buf_size;

    /* Flow control settings */
    unsigned int                    tx_pause;
    unsigned int                    rx_pause;

    /* Jumbo frames */
    unsigned int                    mtu;
    unsigned int                    jumbo;

    /* CRC checking */
    unsigned int                    crc_check;

    /* MSIX */
    unsigned int                    msix;

    /* RSS */
    unsigned int                    rss;

    /* VlanID */
    unsigned int                    vlan;
    unsigned int                    vlan_exist;
    unsigned int                    vlan_filter;
    unsigned int                    vlan_strip;

    /* Interrupt Moderation */
    unsigned int                    intr_mod;
    unsigned int                    intr_mod_timer;

    /* Device interrupt number */
    int                             dev_irq;
    unsigned int                    per_channel_irq;
    u32 channel_irq[FXGMAC_MAX_DMA_CHANNELS_PLUS_1TX]; // change type from int to u32 to match MSIx, p_msix_entry.vector;

    /* Netdev related settings */
    unsigned char                   mac_addr[ETH_ALEN];

    /* Filtering support */
#if FXGMAC_FILTER_MULTIPLE_VLAN_ENABLED
    unsigned long                   active_vlans[BITS_TO_LONGS(VLAN_N_VID)];
#endif

    /* Device clocks */
    unsigned long                   sysclk_rate;

    /* Receive Side Scaling settings */
    u8                              rss_key[FXGMAC_RSS_HASH_KEY_SIZE];
    u32                             rss_table[FXGMAC_RSS_MAX_TABLE_SIZE];
    u32                             rss_options;

    int                             phy_speed;
    int                             phy_duplex;
    int                             phy_autoeng;

#ifndef COMMENT_UNUSED_CODE_TO_REDUCE_SIZE
    char                            drv_name[32];
    char                            drv_ver[32];

    struct wol_bitmap_pattern       pattern[MAX_PATTERN_COUNT];
#endif

    struct led_setting              led;
    struct led_setting              ledconfig;

    FXGMAC_PDATA_OF_PLATFORM        expansion;

    u32                             pcie_link_status;
    u32                             mgmt_phy_val;

    u32                             support_10m_link;
};

#if 1
#define FXGMAC_FLAG_MSI_CAPABLE                  (u32)(1 << 0)
#define FXGMAC_FLAG_MSI_ENABLED                  (u32)(1 << 1)
#define FXGMAC_FLAG_MSIX_CAPABLE                 (u32)(1 << 2)
#define FXGMAC_FLAG_MSIX_ENABLED                 (u32)(1 << 3)
#define FXGMAC_FLAG_LEGACY_ENABLED               (u32)(1 << 4)

#define FXGMAC_FLAG_INTERRUPT_POS                      0
#define FXGMAC_FLAG_INTERRUPT_LEN                      5

#define FXGMAC_FLAG_MSI_POS                            1
#define FXGMAC_FLAG_MSI_LEN                            1
#define FXGMAC_FLAG_MSIX_POS                           3
#define FXGMAC_FLAG_MSIX_LEN                           1
#define FXGMAC_FLAG_LEGACY_POS                         4
#define FXGMAC_FLAG_LEGACY_LEN                         1
#define FXGMAC_FLAG_LEGACY_IRQ_FREE_POS                31
#define FXGMAC_FLAG_LEGACY_IRQ_FREE_LEN                1
#define FXGMAC_FLAG_LEGACY_NAPI_FREE_POS               30
#define FXGMAC_FLAG_LEGACY_NAPI_FREE_LEN               1
#define FXGMAC_FLAG_MISC_IRQ_FREE_POS                  29
#define FXGMAC_FLAG_MISC_IRQ_FREE_LEN                  1
#define FXGMAC_FLAG_MISC_NAPI_FREE_POS                 28
#define FXGMAC_FLAG_MISC_NAPI_FREE_LEN                 1
#define FXGMAC_FLAG_TX_IRQ_FREE_POS                    27
#define FXGMAC_FLAG_TX_IRQ_FREE_LEN                    1
#define FXGMAC_FLAG_TX_NAPI_FREE_POS                   26
#define FXGMAC_FLAG_TX_NAPI_FREE_LEN                   1
#define FXGMAC_FLAG_RX_IRQ_FREE_POS                    22
#define FXGMAC_FLAG_RX_IRQ_FREE_LEN                    4
#define FXGMAC_FLAG_PER_CHAN_RX_IRQ_FREE_LEN           1
#define FXGMAC_FLAG_RX_NAPI_FREE_POS                   18
#define FXGMAC_FLAG_RX_NAPI_FREE_LEN                   4
#define FXGMAC_FLAG_PER_CHAN_RX_NAPI_FREE_LEN          1
#endif

#ifndef FXGMAC_FAKE_4_TX_QUEUE_ENABLED
#define FXGMAC_FAKE_4_TX_QUEUE_ENABLED 0
#endif

void fxgmac_init_desc_ops(struct fxgmac_desc_ops *desc_ops);
void fxgmac_init_hw_ops(struct fxgmac_hw_ops *hw_ops);
const struct net_device_ops *fxgmac_get_netdev_ops(void);
const struct ethtool_ops *fxgmac_get_ethtool_ops(void);
void fxgmac_dump_tx_desc(struct fxgmac_pdata *pdata,
    struct fxgmac_ring *ring,
    unsigned int idx,
    unsigned int count,
    unsigned int flag);
void fxgmac_dump_rx_desc(struct fxgmac_pdata *pdata,
    struct fxgmac_ring *ring,
    unsigned int idx);
void fxgmac_dbg_pkt(struct net_device *netdev,
    struct sk_buff *skb, bool tx_rx);
void fxgmac_get_all_hw_features(struct fxgmac_pdata *pdata);
void fxgmac_print_all_hw_features(struct fxgmac_pdata *pdata);
int fxgmac_drv_probe(struct device *dev,
    struct fxgmac_resources *res);
int fxgmac_drv_remove(struct device *dev);

#endif /* __FXGMAC_GMAC_H__ */
