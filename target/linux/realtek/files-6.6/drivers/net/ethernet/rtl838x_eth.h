/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _RTL838X_ETH_H
#define _RTL838X_ETH_H

/* Register definition */

/* Per port MAC control */
#define RTL838X_MAC_PORT_CTRL			(0xd560)
#define RTL839X_MAC_PORT_CTRL			(0x8004)
#define RTL930X_MAC_L2_PORT_CTRL		(0x3268)
#define RTL930X_MAC_PORT_CTRL			(0x3260)
#define RTL931X_MAC_L2_PORT_CTRL		(0x6000)
#define RTL931X_MAC_PORT_CTRL			(0x6004)

/* DMA interrupt control and status registers */
#define RTL838X_DMA_IF_CTRL			(0x9f58)
#define RTL838X_DMA_IF_INTR_STS			(0x9f54)
#define RTL838X_DMA_IF_INTR_MSK			(0x9f50)

#define RTL839X_DMA_IF_CTRL			(0x786c)
#define RTL839X_DMA_IF_INTR_STS			(0x7868)
#define RTL839X_DMA_IF_INTR_MSK			(0x7864)

#define RTL930X_DMA_IF_CTRL			(0xe028)
#define RTL930X_DMA_IF_INTR_RX_RUNOUT_STS	(0xe01C)
#define RTL930X_DMA_IF_INTR_RX_DONE_STS		(0xe020)
#define RTL930X_DMA_IF_INTR_TX_DONE_STS		(0xe024)
#define RTL930X_DMA_IF_INTR_RX_RUNOUT_MSK	(0xe010)
#define RTL930X_DMA_IF_INTR_RX_DONE_MSK		(0xe014)
#define RTL930X_DMA_IF_INTR_TX_DONE_MSK		(0xe018)
#define RTL930X_L2_NTFY_IF_INTR_MSK		(0xe04C)
#define RTL930X_L2_NTFY_IF_INTR_STS		(0xe050)

/* TODO: RTL931X_DMA_IF_CTRL has different bits meanings */
#define RTL931X_DMA_IF_CTRL			(0x0928)
#define RTL931X_DMA_IF_INTR_RX_RUNOUT_STS	(0x091c)
#define RTL931X_DMA_IF_INTR_RX_DONE_STS		(0x0920)
#define RTL931X_DMA_IF_INTR_TX_DONE_STS		(0x0924)
#define RTL931X_DMA_IF_INTR_RX_RUNOUT_MSK	(0x0910)
#define RTL931X_DMA_IF_INTR_RX_DONE_MSK		(0x0914)
#define RTL931X_DMA_IF_INTR_TX_DONE_MSK		(0x0918)
#define RTL931X_L2_NTFY_IF_INTR_MSK		(0x09E4)
#define RTL931X_L2_NTFY_IF_INTR_STS		(0x09E8)

#define RTL838X_MAC_FORCE_MODE_CTRL		(0xa104)
#define RTL839X_MAC_FORCE_MODE_CTRL		(0x02bc)
#define RTL930X_MAC_FORCE_MODE_CTRL		(0xCA1C)
#define RTL931X_MAC_FORCE_MODE_CTRL		(0x0dcc)

#define RTL83XX_DMA_IF_INTR_STS_NOTIFY_MASK	GENMASK(22, 20)
#define RTL83XX_DMA_IF_INTR_STS_RX_DONE_MASK	GENMASK(15, 8)
#define RTL83XX_DMA_IF_INTR_STS_RX_RUN_OUT_MASK	GENMASK(7, 0)

/* MAC address settings */
#define RTL838X_MAC				(0xa9ec)
#define RTL839X_MAC				(0x02b4)
#define RTL838X_MAC_ALE				(0x6b04)
#define RTL838X_MAC2				(0xa320)
#define RTL930X_MAC_L2_ADDR_CTRL		(0xC714)
#define RTL931X_MAC_L2_ADDR_CTRL		(0x135c)

/* Ringbuffer setup */
#define RTL838X_DMA_RX_BASE			(0x9f00)
#define RTL839X_DMA_RX_BASE			(0x780c)
#define RTL930X_DMA_RX_BASE			(0xdf00)
#define RTL931X_DMA_RX_BASE			(0x0800)

#define RTL838X_DMA_TX_BASE			(0x9f40)
#define RTL839X_DMA_TX_BASE			(0x784c)
#define RTL930X_DMA_TX_BASE			(0xe000)
#define RTL931X_DMA_TX_BASE			(0x0900)

#define RTL838X_DMA_IF_RX_RING_SIZE		(0xB7E4)
#define RTL839X_DMA_IF_RX_RING_SIZE		(0x6038)
#define RTL930X_DMA_IF_RX_RING_SIZE		(0x7C60)
#define RTL931X_DMA_IF_RX_RING_SIZE		(0x2080)

#define RTL838X_DMA_IF_RX_RING_CNTR		(0xB7E8)
#define RTL839X_DMA_IF_RX_RING_CNTR		(0x603c)
#define RTL930X_DMA_IF_RX_RING_CNTR		(0x7C8C)
#define RTL931X_DMA_IF_RX_RING_CNTR		(0x20AC)

#define RTL838X_DMA_IF_RX_CUR			(0x9F20)
#define RTL839X_DMA_IF_RX_CUR			(0x782c)
#define RTL930X_DMA_IF_RX_CUR			(0xdf80)
#define RTL931X_DMA_IF_RX_CUR			(0x0880)

#define RTL838X_DMA_IF_TX_CUR_DESC_ADDR_CTRL	(0x9F48)
#define RTL930X_DMA_IF_TX_CUR_DESC_ADDR_CTRL	(0xE008)

#define RTL838X_DMY_REG31			(0x3b28)
#define RTL838X_SDS_MODE_SEL			(0x0028)
#define RTL838X_SDS_CFG_REG			(0x0034)
#define RTL838X_INT_MODE_CTRL			(0x005c)
#define RTL838X_CHIP_INFO			(0x00d8)
#define RTL838X_SDS4_REG28			(0xef80)
#define RTL838X_SDS4_DUMMY0			(0xef8c)
#define RTL838X_SDS5_EXT_REG6			(0xf18c)

/* L2 features */
#define RTL839X_TBL_ACCESS_L2_CTRL		(0x1180)
#define RTL839X_TBL_ACCESS_L2_DATA(idx)		(0x1184 + ((idx) << 2))
#define RTL838X_TBL_ACCESS_CTRL_0		(0x6914)
#define RTL838X_TBL_ACCESS_DATA_0(idx)		(0x6918 + ((idx) << 2))

/* MAC-side link state handling */
#define RTL838X_MAC_LINK_STS			(0xa188)
#define RTL839X_MAC_LINK_STS			(0x0390)
#define RTL930X_MAC_LINK_STS			(0xCB10)
#define RTL931X_MAC_LINK_STS			(0x0ec0)

#define RTL838X_MAC_LINK_SPD_STS		(0xa190)
#define RTL839X_MAC_LINK_SPD_STS		(0x03a0)
#define RTL930X_MAC_LINK_SPD_STS		(0xCB18)
#define RTL931X_MAC_LINK_SPD_STS		(0x0ed0)

#define RTL838X_MAC_LINK_DUP_STS		(0xa19c)
#define RTL839X_MAC_LINK_DUP_STS		(0x03b0)
#define RTL930X_MAC_LINK_DUP_STS		(0xCB28)
#define RTL931X_MAC_LINK_DUP_STS		(0x0ef0)

/* TODO: RTL8390_MAC_LINK_MEDIA_STS_ADDR??? */

#define RTL838X_MAC_TX_PAUSE_STS		(0xa1a0)
#define RTL839X_MAC_TX_PAUSE_STS		(0x03b8)
#define RTL930X_MAC_TX_PAUSE_STS		(0xCB2C)
#define RTL931X_MAC_TX_PAUSE_STS		(0x0ef8)

#define RTL838X_MAC_RX_PAUSE_STS		(0xa1a4)
#define RTL839X_MAC_RX_PAUSE_STS		(0xCB30)
#define RTL930X_MAC_RX_PAUSE_STS		(0xC2F8)
#define RTL931X_MAC_RX_PAUSE_STS		(0x0f00)

#define RTL838X_EEE_TX_TIMER_GIGA_CTRL		(0xaa04)
#define RTL838X_EEE_TX_TIMER_GELITE_CTRL	(0xaa08)

#define RTL930X_L2_UNKN_UC_FLD_PMSK		(0x9064)
#define RTL931X_L2_UNKN_UC_FLD_PMSK		(0xC8F4)

#define RTL839X_MAC_GLB_CTRL			(0x02a8)
#define RTL839X_SCHED_LB_TICK_TKN_CTRL		(0x60f8)

#define RTL838X_L2_TBL_FLUSH_CTRL		(0x3370)
#define RTL839X_L2_TBL_FLUSH_CTRL		(0x3ba0)
#define RTL930X_L2_TBL_FLUSH_CTRL		(0x9404)
#define RTL931X_L2_TBL_FLUSH_CTRL		(0xCD9C)

#define RTL930X_L2_PORT_SABLK_CTRL		(0x905c)
#define RTL930X_L2_PORT_DABLK_CTRL		(0x9060)

/* MAC link state bits */
#define FORCE_EN				(1 << 0)
#define FORCE_LINK_EN				(1 << 1)
#define NWAY_EN					(1 << 2)
#define DUPLX_MODE				(1 << 3)
#define TX_PAUSE_EN				(1 << 6)
#define RX_PAUSE_EN				(1 << 7)

/* L2 Notification DMA interface */
#define RTL839X_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL	(0x785C)
#define RTL839X_L2_NOTIFICATION_CTRL		(0x7808)
#define RTL931X_L2_NTFY_RING_BASE_ADDR		(0x09DC)
#define RTL931X_L2_NTFY_RING_CUR_ADDR		(0x09E0)
#define RTL839X_L2_NOTIFICATION_CTRL		(0x7808)
#define RTL931X_L2_NTFY_CTRL			(0xCDC8)
#define RTL838X_L2_CTRL_0			(0x3200)
#define RTL839X_L2_CTRL_0			(0x3800)
#define RTL930X_L2_CTRL				(0x8FD8)
#define RTL931X_L2_CTRL				(0xC800)

/* TRAPPING to CPU-PORT */
#define RTL838X_SPCL_TRAP_IGMP_CTRL		(0x6984)
#define RTL838X_RMA_CTRL_0			(0x4300)
#define RTL838X_RMA_CTRL_1			(0x4304)
#define RTL839X_RMA_CTRL_0			(0x1200)

#define RTL839X_SPCL_TRAP_IGMP_CTRL		(0x1058)
#define RTL839X_RMA_CTRL_1			(0x1204)
#define RTL839X_RMA_CTRL_2			(0x1208)
#define RTL839X_RMA_CTRL_3			(0x120C)

#define RTL930X_VLAN_APP_PKT_CTRL		(0xA23C)
#define RTL930X_RMA_CTRL_0			(0x9E60)
#define RTL930X_RMA_CTRL_1			(0x9E64)
#define RTL930X_RMA_CTRL_2			(0x9E68)

#define RTL931X_VLAN_APP_PKT_CTRL		(0x96b0)
#define RTL931X_RMA_CTRL_0			(0x8800)
#define RTL931X_RMA_CTRL_1			(0x8804)
#define RTL931X_RMA_CTRL_2			(0x8808)

/* Advanced SMI control for clause 45 PHYs */
#define RTL930X_SMI_MAC_TYPE_CTRL		(0xCA04)
#define RTL930X_SMI_PORT24_27_ADDR_CTRL		(0xCB90)
#define RTL930X_SMI_PORT0_15_POLLING_SEL	(0xCA08)
#define RTL930X_SMI_PORT16_27_POLLING_SEL	(0xCA0C)

#define RTL930X_SMI_10GPHY_POLLING_REG0_CFG	(0xCBB4)
#define RTL930X_SMI_10GPHY_POLLING_REG9_CFG	(0xCBB8)
#define RTL930X_SMI_10GPHY_POLLING_REG10_CFG	(0xCBBC)
#define RTL930X_SMI_PRVTE_POLLING_CTRL		(0xCA10)

/* Registers of the internal Serdes of the 8390 */
#define RTL839X_SDS12_13_XSG0			(0xB800)

/* Chip configuration registers of the RTL9310 */
#define RTL931X_MEM_ENCAP_INIT			(0x4854)
#define RTL931X_MEM_MIB_INIT			(0x7E18)
#define RTL931X_MEM_ACL_INIT			(0x40BC)
#define RTL931X_MEM_ALE_INIT_0			(0x83F0)
#define RTL931X_MEM_ALE_INIT_1			(0x83F4)
#define RTL931X_MEM_ALE_INIT_2			(0x82E4)
#define RTL931X_MDX_CTRL_RSVD			(0x0fcc)
#define RTL931X_PS_SOC_CTRL			(0x13f8)
#define RTL931X_SMI_10GPHY_POLLING_SEL2		(0xCF8)
#define RTL931X_SMI_10GPHY_POLLING_SEL3		(0xCFC)
#define RTL931X_SMI_10GPHY_POLLING_SEL4		(0xD00)

/* Registers of the internal Serdes of the 8380 */
#define RTL838X_SDS4_FIB_REG0			(0xF800)

/* Default MTU with jumbo frames support */
#define DEFAULT_MTU 9000

inline int rtl838x_mac_port_ctrl(int p)
{
	return RTL838X_MAC_PORT_CTRL + (p << 7);
}

inline int rtl839x_mac_port_ctrl(int p)
{
	return RTL839X_MAC_PORT_CTRL + (p << 7);
}

/* On the RTL931XX, the functionality of the MAC port control register is split up
 * into RTL931X_MAC_L2_PORT_CTRL and RTL931X_MAC_PORT_CTRL the functionality used
 * by the Ethernet driver is in the same bits now in RTL931X_MAC_L2_PORT_CTRL
 */

inline int rtl930x_mac_port_ctrl(int p)
{
	return RTL930X_MAC_L2_PORT_CTRL + (p << 6);
}

inline int rtl931x_mac_port_ctrl(int p)
{
	return RTL931X_MAC_L2_PORT_CTRL + (p << 7);
}

inline int rtl838x_dma_if_rx_ring_size(int i)
{
	return RTL838X_DMA_IF_RX_RING_SIZE + ((i >> 3) << 2);
}

inline int rtl839x_dma_if_rx_ring_size(int i)
{
	return RTL839X_DMA_IF_RX_RING_SIZE + ((i >> 3) << 2);
}

inline int rtl930x_dma_if_rx_ring_size(int i)
{
	return RTL930X_DMA_IF_RX_RING_SIZE + ((i / 3) << 2);
}

inline int rtl931x_dma_if_rx_ring_size(int i)
{
	return RTL931X_DMA_IF_RX_RING_SIZE + ((i / 3) << 2);
}

inline int rtl838x_dma_if_rx_ring_cntr(int i)
{
	return RTL838X_DMA_IF_RX_RING_CNTR + ((i >> 3) << 2);
}

inline int rtl839x_dma_if_rx_ring_cntr(int i)
{
	return RTL839X_DMA_IF_RX_RING_CNTR + ((i >> 3) << 2);
}

inline int rtl930x_dma_if_rx_ring_cntr(int i)
{
	return RTL930X_DMA_IF_RX_RING_CNTR + ((i / 3) << 2);
}

inline int rtl931x_dma_if_rx_ring_cntr(int i)
{
	return RTL931X_DMA_IF_RX_RING_CNTR + ((i / 3) << 2);
}

inline u32 rtl838x_get_mac_link_sts(int port)
{
	return (sw_r32(RTL838X_MAC_LINK_STS) & BIT(port));
}

inline u32 rtl839x_get_mac_link_sts(int p)
{
	return (sw_r32(RTL839X_MAC_LINK_STS + ((p >> 5) << 2)) & BIT(p % 32));
}

inline u32 rtl930x_get_mac_link_sts(int port)
{
	u32 link = sw_r32(RTL930X_MAC_LINK_STS);

	link = sw_r32(RTL930X_MAC_LINK_STS);
	pr_info("%s link state is %08x\n", __func__, link);
	return link & BIT(port);
}

inline u32 rtl931x_get_mac_link_sts(int p)
{
	return (sw_r32(RTL931X_MAC_LINK_STS + ((p >> 5) << 2)) & BIT(p % 32));
}

inline u32 rtl838x_get_mac_link_dup_sts(int port)
{
	return (sw_r32(RTL838X_MAC_LINK_DUP_STS) & BIT(port));
}

inline u32 rtl839x_get_mac_link_dup_sts(int p)
{
	return (sw_r32(RTL839X_MAC_LINK_DUP_STS + ((p >> 5) << 2)) & BIT(p % 32));
}

inline u32 rtl930x_get_mac_link_dup_sts(int port)
{
	return (sw_r32(RTL930X_MAC_LINK_DUP_STS) & BIT(port));
}

inline u32 rtl931x_get_mac_link_dup_sts(int p)
{
	return (sw_r32(RTL931X_MAC_LINK_DUP_STS + ((p >> 5) << 2)) & BIT(p % 32));
}

inline u32 rtl838x_get_mac_link_spd_sts(int port)
{
	int r = RTL838X_MAC_LINK_SPD_STS + ((port >> 4) << 2);
	u32 speed = sw_r32(r);

	speed >>= (port % 16) << 1;
	return (speed & 0x3);
}

inline u32 rtl839x_get_mac_link_spd_sts(int port)
{
	int r = RTL839X_MAC_LINK_SPD_STS + ((port >> 4) << 2);
	u32 speed = sw_r32(r);

	speed >>= (port % 16) << 1;
	return (speed & 0x3);
}


inline u32 rtl930x_get_mac_link_spd_sts(int port)
{
	int r = RTL930X_MAC_LINK_SPD_STS + ((port >> 3) << 2);
	u32 speed = sw_r32(r);

	speed >>= (port % 8) << 2;
	return (speed & 0xf);
}

inline u32 rtl931x_get_mac_link_spd_sts(int port)
{
	int r = RTL931X_MAC_LINK_SPD_STS + ((port >> 3) << 2);
	u32 speed = sw_r32(r);

	speed >>= (port % 8) << 2;
	return (speed & 0xf);
}

inline u32 rtl838x_get_mac_rx_pause_sts(int port)
{
	return (sw_r32(RTL838X_MAC_RX_PAUSE_STS) & (1 << port));
}

inline u32 rtl839x_get_mac_rx_pause_sts(int p)
{
	return (sw_r32(RTL839X_MAC_RX_PAUSE_STS + ((p >> 5) << 2)) & BIT(p % 32));
}

inline u32 rtl930x_get_mac_rx_pause_sts(int port)
{
	return (sw_r32(RTL930X_MAC_RX_PAUSE_STS) & (1 << port));
}

inline u32 rtl931x_get_mac_rx_pause_sts(int p)
{
	return (sw_r32(RTL931X_MAC_RX_PAUSE_STS + ((p >> 5) << 2)) & BIT(p % 32));
}

inline u32 rtl838x_get_mac_tx_pause_sts(int port)
{
	return (sw_r32(RTL838X_MAC_TX_PAUSE_STS) & (1 << port));
}

inline u32 rtl839x_get_mac_tx_pause_sts(int p)
{
	return (sw_r32(RTL839X_MAC_TX_PAUSE_STS + ((p >> 5) << 2)) & BIT(p % 32));
}

inline u32 rtl930x_get_mac_tx_pause_sts(int port)
{
	return (sw_r32(RTL930X_MAC_TX_PAUSE_STS) & (1 << port));
}

inline u32 rtl931x_get_mac_tx_pause_sts(int p)
{
	return (sw_r32(RTL931X_MAC_TX_PAUSE_STS + ((p >> 5) << 2)) & BIT(p % 32));
}

struct p_hdr;
struct dsa_tag;

struct rtl838x_eth_reg {
	irqreturn_t (*net_irq)(int irq, void *dev_id);
	int (*mac_port_ctrl)(int port);
	int dma_if_intr_sts;
	int dma_if_intr_msk;
	int dma_if_intr_rx_runout_sts;
	int dma_if_intr_rx_done_sts;
	int dma_if_intr_tx_done_sts;
	int dma_if_intr_rx_runout_msk;
	int dma_if_intr_rx_done_msk;
	int dma_if_intr_tx_done_msk;
	int l2_ntfy_if_intr_sts;
	int l2_ntfy_if_intr_msk;
	int dma_if_ctrl;
	int mac_force_mode_ctrl;
	int dma_rx_base;
	int dma_tx_base;
	int (*dma_if_rx_ring_size)(int ring);
	int (*dma_if_rx_ring_cntr)(int ring);
	int dma_if_rx_cur;
	int rst_glb_ctrl;
	u32 (*get_mac_link_sts)(int port);
	u32 (*get_mac_link_dup_sts)(int port);
	u32 (*get_mac_link_spd_sts)(int port);
	u32 (*get_mac_rx_pause_sts)(int port);
	u32 (*get_mac_tx_pause_sts)(int port);
	int mac;
	int l2_tbl_flush_ctrl;
	void (*update_cntr)(int r, int work_done);
	void (*create_tx_header)(struct p_hdr *h, unsigned int dest_port, int prio);
	bool (*decode_tag)(struct p_hdr *h, struct dsa_tag *tag);
};

int phy_package_port_read_paged(struct phy_device *phydev, int port, int page, u32 regnum);
int phy_package_port_write_paged(struct phy_device *phydev, int port, int page, u32 regnum, u16 val);
int phy_package_read_paged(struct phy_device *phydev, int page, u32 regnum);
int phy_package_write_paged(struct phy_device *phydev, int page, u32 regnum, u16 val);
int phy_port_read_paged(struct phy_device *phydev, int port, int page, u32 regnum);
int phy_port_write_paged(struct phy_device *phydev, int port, int page, u32 regnum, u16 val);

#endif /* _RTL838X_ETH_H */
