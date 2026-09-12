/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright(c) 2022 - 2025 Phytium Technology Co., Ltd. */

#ifndef _PHYTMAC_V1_H
#define _PHYTMAC_V1_H

extern struct phytmac_hw_if phytmac_1p0_hw;

#define PHYTMAC_FRAME_MASK		0x1fff
#define PHYTMAC_JUMBO_FRAME_MASK	0x3fff

#define MAC0_ADDR_BASE		0x36ce0000
#define MAC1_ADDR_BASE		0x36ce2000

#define PHYTMAC_SPEED_100M		0
#define PHYTMAC_SPEED_1000M		1
#define PHYTMAC_SPEED_2500M		2
#define PHYTMAC_SPEED_5000M		3
#define PHYTMAC_SPEED_10000M		4
#define PHYTMAC_SERDES_RATE_5G		0
#define PHYTMAC_SERDES_RATE_10G		1

#define PHYTMAC_NCTRL			0x0000
#define PHYTMAC_NCONFIG			0x0004
#define PHYTMAC_NSTATUS			0x0008
#define PHYTMAC_DCONFIG			0x0010
#define PHYTMAC_RXPTR_Q0		0x0018
#define PHYTMAC_TXPTR_Q0		0x001C
#define PHYTMAC_IS			0x0024
#define PHYTMAC_IE			0x0028
#define PHYTMAC_ID			0x002C
#define PHYTMAC_IM			0x0030
#define PHYTMAC_MDATA			0x0034
#define PHYTMAC_HCONFIG			0x0050
#define PHYTMAC_AXICTRL			0x0054
#define PHYTMAC_INT_MODERATION		0x005C
#define PHYTMAC_HASHB			0x0080
#define PHYTMAC_HASHT			0x0084
#define PHYTMAC_MAC1B			0x0088
#define PHYTMAC_MAC1T			0x008C
#define PHYTMAC_WOL			0x00B8
#define PHYTMAC_OCTTX			0x0100
#define PHYTMAC_TISN			0x01BC
#define PHYTMAC_TSH			0x01C0
#define PHYTMAC_TSL			0x01D0
#define PHYTMAC_TN			0x01D4
#define PHYTMAC_TA			0x01D8
#define PHYTMAC_TI			0x01DC
#define PHYTMAC_PCSCTRL			0x0200
#define PHYTMAC_PCSSTATUS		0x0204
#define PHYTMAC_PCSANLPBASE		0x0214
#define PHYTMAC_DEFAULT1		0x0280	 /* Default HW Config 1 */
#define PHYTMAC_DEFAULT2		0x0294	 /* Default HW Config 2 */
#define PHYTMAC_DEFAULT3		0x029C	 /* Default HW Config 3 */
#define PHYTMAC_DEFAULT4		0x02A4	 /* Default HW Config 4 */
#define PHYTMAC_DEFAULT5		0x02AC	 /* Default HW Config 5 */
#define PHYTMAC_USXCTRL			0x0A80
#define PHYTMAC_USXSTATUS		0x0A88
#define PHYTMAC_TXBDCTRL		0x04CC
#define PHYTMAC_RXBDCTRL		0x04D0
#define PHYTMAC_TXSTART			0x0E70
#define PHYTMAC_TXSTARTSEL		0x0E74
#define PHYTMAC_VERSION			0x0FF0

/* Fdir match registers */
#define PHYTMAC_FDIR(i)			(0x0540 + ((i) << 2))

/* EtherType registers */
#define PHYTMAC_ETHT(i)			(0x06E0 + ((i) << 2))

/* Fdir compare registers */
#define PHYTMAC_COMP1(i)		(0x0700 + ((i) << 3))
#define PHYTMAC_COMP2(i)		(0x0704 + ((i) << 3))

#define PHYTMAC_ISR(i)			(0x0400 + ((i) << 2))
#define PHYTMAC_TXPTR(i)		(0x0440 + ((i) << 2))
#define PHYTMAC_RXPTR(i)		(0x0480 + ((i) << 2))
#define PHYTMAC_RBQS(i)			(0x04A0 + ((i) << 2))
#define PHYTMAC_TXPTRH(i)		(0x04c8)
#define PHYTMAC_RXPTRH(i)		(0x04d4)

#define PHYTMAC_IER(i)			(0x0600 + ((i) << 2))
#define PHYTMAC_IDR(i)			(0x0620 + ((i) << 2))
#define PHYTMAC_IMR(i)			(0x0640 + ((i) << 2))
#define PHYTMAC_TAIL_ENABLE		(0x0e7c)
#define PHYTMAC_TX_TAILPTR(i)	(0x0e80 + ((i) << 2))
#define PHYTMAC_RX_TAILPTR(i)	(0x0ec0 + ((i) << 2))

#define PHYTMAC_PHY_INT_ENABLE		0x1C88
#define PHYTMAC_PHY_INT_CLEAR		0x1C8C
#define PHYTMAC_PHY_INT_STATE		0x1C90
#define PHYTMAC_INTX_IRQ_MASK		0x1C7C

#define PHYTMAC_READ_NSTATUS(pdata)	PHYTMAC_READ(pdata, PHYTMAC_NSTATUS)

/* Ethernet Network Control Register */
#define PHYTMAC_RE_INDEX			2	 /* Receive enable */
#define PHYTMAC_RE_WIDTH			1
#define PHYTMAC_TE_INDEX			3	 /* Transmit enable */
#define PHYTMAC_TE_WIDTH			1
#define PHYTMAC_MPE_INDEX			4	 /* Management port enable */
#define PHYTMAC_MPE_WIDTH			1
#define PHYTMAC_CLEARSTAT_INDEX			5	 /* Clear stats regs */
#define PHYTMAC_CLEARSTAT_WIDTH			1
#define PHYTMAC_TSTART_INDEX			9	 /* Start transmission */
#define PHYTMAC_TSTART_WIDTH			1
#define PHYTMAC_THALT_INDEX			10	 /* Transmit halt */
#define PHYTMAC_THALT_WIDTH			1
#define PHYTMAC_STORE_RX_TS_INDEX		15
#define PHYTMAC_STORE_RX_TS_WIDTH		1
#define PHYTMAC_OSSMODE_INDEX			24	 /* Enable One Step Synchro Mode */
#define PHYTMAC_OSSMODE_WIDTH			1
#define PHYTMAC_2PT5G_INDEX			29	 /* 2.5G operation selected */
#define PHYTMAC_2PT5G_WIDTH			1
#define PHYTMAC_HIGHSPEED_INDEX			31	 /* High speed enable */
#define PHYTMAC_HIGHSPEED_WIDTH			1

/* Ethernet Network Config Register */
#define PHYTMAC_SPEED_INDEX			0	 /* Speed */
#define PHYTMAC_SPEED_WIDTH			1
#define PHYTMAC_FD_INDEX			1	 /* Full duplex */
#define PHYTMAC_FD_WIDTH			1
#define PHYTMAC_JUMBO_EN_INDEX			3	 /* Transmit enable */
#define PHYTMAC_JUMBO_EN_WIDTH			1
#define PHYTMAC_PROMISC_INDEX			4	 /* Copy all frames */
#define PHYTMAC_PROMISC_WIDTH			1
#define PHYTMAC_NO_BCAST_INDEX			5	 /* No broadcast */
#define PHYTMAC_NO_BCAST_WIDTH			1
#define PHYTMAC_MH_EN_INDEX			6	 /* Multicast hash enable */
#define PHYTMAC_MH_EN_WIDTH			1
#define PHYTMAC_RCV_BIG_INDEX			8
#define PHYTMAC_RCV_BIG_WIDTH			1
#define PHYTMAC_GM_EN_INDEX			10	 /* Gigabit mode enable */
#define PHYTMAC_GM_EN_WIDTH			1
#define PHYTMAC_PCS_EN_INDEX			11	 /* PCS select */
#define PHYTMAC_PCS_EN_WIDTH			1
#define PHYTMAC_PAUSE_EN_INDEX			13	 /* Pause enable */
#define PHYTMAC_PAUSE_EN_WIDTH			1
#define PHYTMAC_FCS_REMOVE_INDEX		17	 /* FCS remove */
#define PHYTMAC_FCS_REMOVE_WIDTH		1
#define PHYTMAC_MCD_INDEX			18	 /* MDC clock division */
#define PHYTMAC_MCD_WIDTH			3
#define PHYTMAC_DBW64_INDEX			21	 /* Data bus width */
#define PHYTMAC_DBW64_WIDTH			1
#define PHYTMAC_DBW128_INDEX			22	 /* Data bus width */
#define PHYTMAC_DBW128_WIDTH			1
#define PHYTMAC_DBW_32				1
#define PHYTMAC_DBW_64				2
#define PHYTMAC_DBW_128				4
#define PHYTMAC_RCO_EN_INDEX			24	 /* Receive checksum offload enable */
#define PHYTMAC_RCO_EN_WIDTH			1
#define PHYTMAC_IGNORE_RX_FCS_INDEX     26
#define PHYTMAC_IGNORE_RX_FCS_WIDTH     1
#define PHYTMAC_SGMII_EN_INDEX			27	 /* Sgmii mode enable */
#define PHYTMAC_SGMII_EN_WIDTH			1

/* Ethernet Network Status Register */
#define PHYTMAC_PCS_LINK_INDEX			0	 /* PCS link status */
#define PHYTMAC_PCS_LINK_WIDTH			1
#define PHYTMAC_MDIO_IDLE_INDEX			2	 /* Mdio idle */
#define PHYTMAC_MDIO_IDLE_WIDTH			1
#define PHYTMAC_PCS_FD_INDEX			3 /* PCS auto negotiation duplex resolution */
#define PHYTMAC_PCS_FD__WIDTH			1

/* Ethernet Network Dma config Register */
#define PHYTMAC_BURST_INDEX			0 /* Amba burst length */
#define PHYTMAC_BURST_WIDTH			5
#define PHYTMAC_ENDIA_PKT_INDEX			6
#define PHYTMAC_ENDIA_PKT_WIDTH			1
#define PHYTMAC_ENDIA_DESC_INDEX		7
#define PHYTMAC_ENDIA_DESC_WIDTH		1
#define PHYTMAC_TCO_EN_INDEX			11	 /* Tx Checksum Offload en */
#define PHYTMAC_TCO_EN_WIDTH			1
#define PHYTMAC_RX_BUF_LEN_INDEX		16	 /* DMA receive buffer size */
#define PHYTMAC_RX_BUF_LEN_WIDTH		8
#define PHYTMAC_RX_EXBD_EN_INDEX		28	 /* Enable RX extended BD mode */
#define PHYTMAC_RX_EXBD_EN_WIDTH		1
#define PHYTMAC_TX_EXBD_EN_INDEX		29	 /* Enable TX extended BD mode */
#define PHYTMAC_TX_EXBD_EN_WIDTH		1
#define PHYTMAC_ABW_INDEX			30	 /* DMA address bus width */
#define PHYTMAC_ABW_WIDTH			1

/* Int stauts/Enable/Disable/Mask Register */
#define PHYTMAC_RXCOMP_INDEX			1	 /* Rx complete */
#define PHYTMAC_RXCOMP_WIDTH			1
#define PHYTMAC_RUB_INDEX			2	 /* Rx used bit read */
#define PHYTMAC_RUB_WIDTH			1
#define PHYTMAC_BUS_ERR_INDEX			6	 /* AMBA error */
#define PHYTMAC_BUS_ERR_WIDTH			1
#define PHYTMAC_TXCOMP_INDEX			7	 /* Tx complete */
#define PHYTMAC_TXCOMP_WIDTH			1
#define PHYTMAC_RXOVERRUN_INDEX			10	 /* Tx overrun */
#define PHYTMAC_RXOVERRUN_WIDTH			1
#define PHYTMAC_RESP_ERR_INDEX			11	 /* Resp not ok */
#define PHYTMAC_RESP_ERR_WIDTH			1

/* Mdio read/write Register */
#define PHYTMAC_DATA_INDEX			0	 /* Data */
#define PHYTMAC_DATA_WIDTH			16
#define PHYTMAC_MUST_INDEX			16	 /* Must Be 10 */
#define PHYTMAC_MUST_WIDTH			2
#define PHYTMAC_REG_ADDR_INDEX			18	 /* Register address */
#define PHYTMAC_REG_ADDR_WIDTH			5
#define PHYTMAC_PHY_ADDR_INDEX			23	 /* Phy address */
#define PHYTMAC_PHY_ADDR_WIDTH			5
#define PHYTMAC_OPS_INDEX			28
#define PHYTMAC_OPS_WIDTH			2
#define PHYTMAC_CLAUSE_SEL_INDEX		30
#define PHYTMAC_CLAUSE_SEL_WIDTH		1
#define PHYTMAC_CLAUSE_C22			1
#define PHYTMAC_CLAUSE_C45			0
#define PHYTMAC_OPS_C45_ADDR			0
#define PHYTMAC_OPS_C45_WRITE			1
#define PHYTMAC_OPS_C45_READ			3
#define PHYTMAC_OPS_C22_WRITE			1
#define PHYTMAC_OPS_C22_READ			2

/* hs mac config register */
#define PHYTMAC_HS_SPEED_INDEX			0
#define PHYTMAC_HS_SPEED_WIDTH			3
#define PHYTMAC_HS_SPEED_100M			0
#define PHYTMAC_HS_SPEED_1000M			1
#define PHYTMAC_HS_SPEED_2500M			2
#define PHYTMAC_HS_SPEED_5000M			3
#define PHYTMAC_HS_SPEED_10G			4

/* WOL register */
#define PHYTMAC_ARP_IP_INDEX			0
#define PHYTMAC_ARP_IP_WIDTH			16
#define PHYTMAC_MAGIC_INDEX			16
#define PHYTMAC_MAGIC_WIDTH			1
#define PHYTMAC_ARP_INDEX			17
#define PHYTMAC_ARP_WIDTH			1
#define PHYTMAC_UCAST_INDEX			18
#define PHYTMAC_UCAST_WIDTH			1
#define PHYTMAC_MCAST_INDEX			19
#define PHYTMAC_MCAST_WIDTH			1

/* PCSCTRL register */
#define PHYTMAC_AUTONEG_INDEX			12
#define PHYTMAC_AUTONEG_WIDTH			1
#define PHYTMAC_PCS_RESET_INDEX			15
#define PHYTMAC_PCS_RESET_WIDTH			1

/* DEFAULT1 register */
#define PHYTMAC_DBW_INDEX			25
#define PHYTMAC_DBW_WIDTH			3

/* DEFAULT2 register */
#define PHYTMAC_DAW64_INDEX			23
#define PHYTMAC_DAW64_WIDTH			1

/* DEFAULT3 register */
#define PHYTMAC_SCR2CMP_INDEX			0
#define PHYTMAC_SCR2CMP_WIDTH			8
#define PHYTMAC_SCR2ETH_INDEX			8
#define PHYTMAC_SCR2ETH_WIDTH			8

/* DEFAULT4 register */
#define PHYTMAC_TXDESCRD_INDEX			12
#define PHYTMAC_TXDESCRD_WIDTH			4
#define PHYTMAC_RXDESCRD_INDEX			8
#define PHYTMAC_RXDESCRD_WIDTH			4

/* USXCTRL register */
#define PHYTMAC_RX_EN_INDEX			0
#define PHYTMAC_RX_EN_WIDTH			1
#define PHYTMAC_TX_EN_INDEX			1
#define PHYTMAC_TX_EN_WIDTH			1
#define PHYTMAC_RX_SYNC_RESET_INDEX		2
#define PHYTMAC_RX_SYNC_RESET_WIDTH		1
#define PHYTMAC_SERDES_RATE_INDEX		12
#define PHYTMAC_SERDES_RATE_WIDTH		2
#define PHYTMAC_USX_SPEED_INDEX			14
#define PHYTMAC_USX_SPEED_WIDTH			3

/* Bitfields in USX_STATUS. */
#define PHYTMAC_USX_PCS_LINK_INDEX		0
#define PHYTMAC_USX_PCS_LINK_WIDTH		1

/* Bitfields in PHYTMAC_TISN */
#define PHYTMAC_SUB_NSECH_INDEX			0
#define PHYTMAC_SUB_NSECH_WIDTH			16
#define PHYTMAC_SUB_NSECL_INDEX			24
#define PHYTMAC_SUB_NSECL_WIDTH			8
#define PHYTMAC_SUB_NSEC_WIDTH			24

/* Bitfields in PHYTMAC_TSH */
#define PHYTMAC_SECH_INDEX			0
#define PHYTMAC_SECH_WIDTH			16

/* Bitfields in PHYTMAC_TSL */
#define PHYTMAC_SECL_INDEX			0
#define PHYTMAC_SECL_WIDTH			32

/* Bitfields in PHYTMAC_TN */
#define PHYTMAC_NSEC_INDEX			0
#define PHYTMAC_NSEC_WIDTH			30

/* Bitfields in PHYTMAC_TA */
#define PHYTMAC_INCR_SEC_INDEX			0
#define PHYTMAC_INCR_SEC_WIDTH			30
#define PHYTMAC_INCR_ADD_INDEX			31
#define PHYTMAC_INCR_ADD_WIDTH			1
#define PHYTMAC_ADJUST_SEC_MAX		((1 << PHYTMAC_INCR_SEC_WIDTH) - 1)

/* Bitfields in PHYTMAC_TI */
#define PHYTMAC_INCR_NS_INDEX			0
#define PHYTMAC_INCR_NS_WIDTH			8

/* PHYTMAC_TXBDCTRL register */
#define PHYTMAC_TX_TSMODE_INDEX			4
#define PHYTMAC_TX_TSMODE_WIDTH			2

#define PHYTMAC_RX_TSMODE_INDEX			4
#define PHYTMAC_RX_TSMODE_WIDTH			2

/* Bitfields in PHYTMAC_FDIR */
#define PHYTMAC_QUEUE_NUM_INDEX			0
#define PHYTMAC_QUEUE_NUM_WIDTH			4
#define PHYTMAC_VLAN_PRI_INDEX			4
#define PHYTMAC_VLAN_PRI_WIDTH			3
#define PHYTMAC_VLAN_EN_INDEX			8
#define PHYTMAC_VLAN_EN_WIDTH			1
#define PHYTMAC_ETH_TYPE_INDEX			9
#define PHYTMAC_ETH_TYPE_WIDTH			3
#define PHYTMAC_ETH_EN_INDEX			12
#define PHYTMAC_ETH_EN_WIDTH			1
#define PHYTMAC_CA_INDEX			13
#define PHYTMAC_CA_WIDTH			5
#define PHYTMAC_CA_EN_INDEX			18
#define PHYTMAC_CA_EN_WIDTH			1
#define PHYTMAC_CB_INDEX			19
#define PHYTMAC_CB_WIDTH			5
#define PHYTMAC_CB_EN_INDEX			24
#define PHYTMAC_CB_EN_WIDTH			1
#define PHYTMAC_CC_INDEX			25
#define PHYTMAC_CC_WIDTH			5
#define PHYTMAC_CC_EN_INDEX			30
#define PHYTMAC_CC_EN_WIDTH			1

/* Bitfields in PHYTMAC_ETHERTYPE */
#define PHYTMAC_ETHTYPE_VALUE_INDEX		0
#define PHYTMAC_ETHTYPE_VALUE_WIDTH		16

/* Bitfields in PHYTMAC_COMP1 */
#define PHYTMAC_SPORT_INDEX			0
#define PHYTMAC_SPORT_WIDTH			16
#define PHYTMAC_DPORT_INDEX			16
#define PHYTMAC_DPORTE_WIDTH			16

/* Bitfields in PHYTMAC_COMP2 */
#define PHYTMAC_OFFSET_INDEX			0
#define PHYTMAC_OFFSET_WIDTH			7
#define ETHTYPE_SIP_OFFSET			12
#define ETHTYPE_DIP_OFFSET			16
#define IPHEAD_SPORT_OFFSET			0
#define IPHEAD_DPORT_OFFSET			2
#define PHYTMAC_OFFSET_TYPE_INDEX		7
#define PHYTMAC_OFFSET_TYPE_WIDTH		2
#define PHYTMAC_OFFSET_BEGIN			0
#define PHYTMAC_OFFSET_AFTER_L2HEAD		1
#define PHYTMAC_OFFSET_AFTER_L3HEAD		2
#define PHYTMAC_OFFSET_AFTER_L4HEAD		3
#define PHYTMAC_DIS_MASK_INDEX			9
#define PHYTMAC_DIS_MASK_WIDTH			1
#define PHYTMAC_VLAN_ID_INDEX			10
#define PHYTMAC_VLAN_ID_WIDTH			1

/* Bitfields in TAILPTR */
#define PHYTMAC_TXTAIL_UPDATE_INDEX		31 /* Update tx tail */
#define PHYTMAC_TXTAIL_UPDATE_WIDTH		1

/* Bitfields in TAIL_ENABLE */
#define PHYTMAC_TXPTR_EN_INDEX		0	/* Enable tx tail */
#define PHYTMAC_TXPTR_EN_WIDTH		1

#define PHYTMAC_RXPTR_EN_INDEX		16	/* Enable rx tail */
#define PHYTMAC_RXPTR_EN_WIDTH		1

/* Bitfields in INT ENABLE */
#define PHYTMAC_WOL_RECEIVE_ENABLE_INDEX    28 /* Enable wol_event_recieve */
#define PHYTMAC_WOL_RECEIVE_ENABLE_WIDTH    1

/* Bitfields in INT DISABLE */
#define PHYTMAC_WOL_RECEIVE_DISABLE_INDEX    28 /* Disable wol_event_recieve */
#define PHYTMAC_WOL_RECEIVE_DISABLE_WIDTH    1

/* Bitfields in PHYTMAC_TXSTARTSEL */
#define PHYTMAC_TSTARTSEL_ENABLE_INDEX 0 /* Enable Tstart_sel */
#define PHYTMAC_TSTARTSEL_ENABLE_WIDTH 1

#define PHYTMAC_TSEC_WIDTH (PHYTMAC_SECH_WIDTH + PHYTMAC_SECL_WIDTH)
#define SEC_MAX_VAL (((u64)1 << PHYTMAC_TSEC_WIDTH) - 1)
#define NSEC_MAX_VAL ((1 << PHYTMAC_NSEC_WIDTH) - 1)

/* rx dma desc bit */
/* DMA descriptor bitfields */
#define PHYTMAC_RX_USED_INDEX			0
#define PHYTMAC_RX_USED_WIDTH			1
#define PHYTMAC_RX_WRAP_INDEX			1
#define PHYTMAC_RX_WRAP_WIDTH			1
#define PHYTMAC_RX_TS_VALID_INDEX		2
#define PHYTMAC_RX_TS_VALID_WIDTH		1
#define PHYTMAC_RX_WADDR_INDEX			2
#define PHYTMAC_RX_WADDR_WIDTH			30

#define PHYTMAC_RX_FRMLEN_INDEX			0
#define PHYTMAC_RX_FRMLEN_WIDTH			12
#define PHYTMAC_RX_INDEX_INDEX			12
#define PHYTMAC_RX_INDEX_WIDTH			2
#define PHYTMAC_RX_SOF_INDEX			14
#define PHYTMAC_RX_SOF_WIDTH			1
#define PHYTMAC_RX_EOF_INDEX			15
#define PHYTMAC_RX_EOF_WIDTH			1
#define PHYTMAC_RX_CFI_INDEX			16
#define PHYTMAC_RX_CFI_WIDTH			1
#define PHYTMAC_RX_VLAN_PRI_INDEX		17
#define PHYTMAC_RX_VLAN_PRI_WIDTH		3
#define PHYTMAC_RX_PRI_TAG_INDEX		20
#define PHYTMAC_RX_PRI_TAG_WIDTH		1
#define PHYTMAC_RX_VLAN_TAG_INDEX		21
#define PHYTMAC_RX_VLAN_TAG_WIDTH		1
#define PHYTMAC_RX_UHASH_MATCH_INDEX		29
#define PHYTMAC_RX_UHASH_MATCH_WIDTH		1
#define PHYTMAC_RX_MHASH_MATCH_INDEX		30
#define PHYTMAC_RX_MHASH_MATCH_WIDTH		1
#define PHYTMAC_RX_BROADCAST_INDEX		31
#define PHYTMAC_RX_BROADCAST_WIDTH		1

#define PHYTMAC_RX_FRMLEN_MASK			0x1FFF
#define PHYTMAC_RX_JFRMLEN_MASK			0x3FFF

/* RX checksum offload disabled: bit 24 clear in NCFGR */
#define PHYTMAC_RX_TYPEID_MATCH_INDEX		22
#define PHYTMAC_RX_TYPEID_MATCH_WIDTH		2

/* RX checksum offload enabled: bit 24 set in NCFGR */
#define PHYTMAC_RX_CSUM_INDEX			22
#define PHYTMAC_RX_CSUM_WIDTH			2

/* tx dma desc bit */
#define PHYTMAC_TX_FRMLEN_INDEX			0
#define PHYTMAC_TX_FRMLEN_WIDTH			14
#define PHYTMAC_TX_LAST_INDEX			15
#define PHYTMAC_TX_LAST_WIDTH			1
#define PHYTMAC_TX_NOCRC_INDEX			16
#define PHYTMAC_TX_NOCRC_WIDTH			1
#define PHYTMAC_MSS_MFS_INDEX			16
#define PHYTMAC_MSS_MFS_WIDTH			14
#define PHYTMAC_TX_LSO_INDEX			17
#define PHYTMAC_TX_LSO_WIDTH			2
#define PHYTMAC_TX_TCP_SEQ_SRC_INDEX		19
#define PHYTMAC_TX_TCP_SEQ_SRC_WIDTH		1
#define PHYTMAC_TX_TS_VALID_INDEX		23
#define PHYTMAC_TX_TS_VALID_WIDTH		1
#define PHYTMAC_TX_BUF_EXHAUSTED_INDEX		27
#define PHYTMAC_TX_BUF_EXHAUSTED_WIDTH		1
#define PHYTMAC_TX_UNDERRUN_INDEX		28
#define PHYTMAC_TX_UNDERRUN_WIDTH		1
#define PHYTMAC_TX_ERROR_INDEX			29
#define PHYTMAC_TX_ERROR_WIDTH			1
#define PHYTMAC_TX_WRAP_INDEX			30
#define PHYTMAC_TX_WRAP_WIDTH			1
#define PHYTMAC_TX_USED_INDEX			31
#define PHYTMAC_TX_USED_WIDTH			1

/* Transmit DMA buffer descriptor Word 4 */
#define PHYTMAC_DMA_NSEC_INDEX			0
#define PHYTMAC_DMA_NSEC_WIDTH			30
#define PHYTMAC_DMA_SECL_INDEX			30
#define PHYTMAC_DMA_SECL_WIDTH			2

/* Transmit DMA buffer descriptor Word 4 */
#define PHYTMAC_DMA_SECH_INDEX			0
#define PHYTMAC_DMA_SECH_WIDTH			4
#define PHYTMAC_DMA_SEC_MASK			0x3f
#define PHYTMAC_DMA_SEC_TOP			0x40

/* Buffer descriptor constants */
#define PHYTMAC_RX_CSUM_NONE			0
#define PHYTMAC_RX_CSUM_IP_ONLY			1
#define PHYTMAC_RX_CSUM_IP_TCP			2
#define PHYTMAC_RX_CSUM_IP_UDP			3

/* limit RX checksum offload to TCP and UDP packets */
#define PHYTMAC_RX_CSUM_CHECKED_MASK		2

#endif
