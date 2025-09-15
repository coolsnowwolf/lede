/**
 * Copyright (c) 2011-2013 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
#ifndef __QDPC_EMAC_H__
#define __QDPC_EMAC_H__

#include <asm/cache.h>

/* Register definitions */

/*  Some Bit values */
#define DC_FLUSH_STATUS_BIT     0x100
#define INV_MODE_FLUSH          0x40
#define CACHE_DISABLE_BIT       0x01

#define EMAC0_ADDR              (0xed000000)
#define EMAC1_ADDR              (0xe8000000)
#define EMAC_DMACONFIG          (0x0000)
#define EMAC_DMACTRL            (0x0004)
#define EMAC_DMASTATUS          (0x0008)
#define EMAC_DMAINTEN           (0x000c)
#define EMAC_DMATAPC            (0x0010)
#define EMAC_DMATPD             (0x0014)
#define EMAC_DMARPD             (0x0018)
#define EMAC_DMATBA             (0x001c)
#define EMAC_DMARBA             (0x0020)
#define EMAC_DMAMFC             (0x0024)
#define EMAC_DMASFC             (0x0028)
#define EMAC_DMARDIMC           (0x002c)
#define EMAC_DMACTDP            (0x0030)
#define EMAC_DMACTBP            (0x0034)
#define EMAC_DMARDP             (0x0038)
#define EMAC_DMACRBP            (0x003c)
#define EMAC_DMAFCT             (0x0044)
#define EMAC_MACGCTRL           (0x0100) /* Global Control Register */
#define EMAC_MACTCTRL           (0x0104) /* Transmit Control Register */
#define EMAC_MACRCTRL           (0x0108) /* Receive Control Register */
#define EMAC_MACMFS             (0x010C) /* Maximum Frame Size Register */
#define EMAC_MACTJS             (0x0110) /* Transmit Jabber Size Register */
#define EMAC_MACRJS             (0x0114) /* Receive Jabber Size Register */
#define EMAC_MACACTRL           (0x0118) /* Address Control Register. */
#define EMAC_MACAH1             (0x0120) /* Address#1 High Register */
#define EMAC_MACAM1             (0x0124) /* Address#1 Med Register */
#define EMAC_MACAL1             (0x0128) /* Address#1 Low Register */
#define EMAC_MACAH2             (0x012C) /* Address#2 High Register */
#define EMAC_MACAM2             (0x0130) /* Address#2 Med Register */
#define EMAC_MACAL2             (0x0134) /* Address#2 Low Register */
#define EMAC_MACAH3             (0x0138) /* Address#3 High Register */
#define EMAC_MACAM3             (0x013C) /* Address#3 Med Register */
#define EMAC_MACAL3             (0x0140) /* Address#3 Low Register */
#define EMAC_MACAH4             (0x0144) /* Address#4 High Register */
#define EMAC_MACAM4             (0x0148) /* Address#4 Med Register */
#define EMAC_MACAL4             (0x014C) /* Address#4 Low Register */
#define EMAC_MACHT1             (0x0150) /* Hash Table#1 Register */
#define EMAC_MACHT2             (0x0154) /* Hash Table#2 Register */
#define EMAC_MACHT3             (0x0158) /* Hash Table#3 Register */
#define EMAC_MACHT4             (0x015C) /* Hash Table#4 Register */
#define EMAC_MACFCCTRL          (0x0160) /* Flow-Control Control Register */
#define EMAC_MACFCPFG           (0x0164) /* Flow-Control Pause Frame Generate Register */
#define EMAC_MACFCSAH           (0x0168) /* Flow-Control Source Address High Register */
#define EMAC_MACFCSAM           (0x016C) /* Flow-Control Source Address Med Register */
#define EMAC_MACFCSAL           (0x0170) /* Flow-Control Source Address Low Register */
#define EMAC_MACFCDAH           (0x0174) /* Flow-Control Destination Address High Register */
#define EMAC_MACFCDAM           (0x0178) /* Flow-Control Destination Address Med Register */
#define EMAC_MACFCDAL           (0x017C) /* Flow-Control Destination Address Low Register */
#define EMAC_MACFCPTV           (0x0180) /* Flow-Control Pause Time Value Register */
#define EMAC_MACFCPTVH          (0x0184) /* Flow-Control High Pause Time Value Register */
#define EMAC_MACFCPTVL          (0x0188) /* Flow-Control Low Pause Time Value Register */
#define EMAC_MACMDIOCTRL        (0x01A0) /* MDIO Control Register */
#define EMAC_MACMDIODAT         (0x01A4) /* MDIO Data Register */
#define EMAC_MACRXSCCTRL        (0x01A8) /* Rx. Stat Ctr Control Register */
#define EMAC_MACRXSCDH          (0x01AC) /* Rx. Stat Ctr Data High Register */
#define EMAC_MACRXSCDL          (0x01B0) /* Rx. Stat Ctr Data Low Register */
#define EMAC_MACTXSCCTRL        (0x01B4) /* Tx. Stat Ctr Control Register */
#define EMAC_MACTXSCDH          (0x01B8) /* Tx. Stat Ctr Data High Register */
#define EMAC_MACTXSCDL          (0x01BC) /* Tx. Stat Ctr Data Low Register */
#define EMAC_MACTFIFOAF         (0x01C0) /* Transmit FIFO Almost Full Register */
#define EMAC_MACTPST            (0x01C4) /* Transmit Packet Start Threshold Register */
#define EMAC_MACRPST            (0x01C8) /* Receive Packet Start Threshold Register */
#define EMAC_MACINTR            (0x01E0) /* Interrupt Register */
#define EMAC_MACINTEN           (0x01E4) /* Interrupt Enable Register */

#define PHY_MDIOCTRL            (EMAC0_ADDR + 0x01A0) /* MDIO Control Register */
#define PHY_MDIODATA            (EMAC0_ADDR + 0x01A4) /* MDIO Data Register */

/* mac global control register 0x0100 */
#define EMAC_10MBPS             0
#define EMAC_100MBPS            1
#define EMAC_1000MBPS           2
#define EMAC_HD                 0
#define EMAC_FD                 (1<<2)
#define MAC_RX_STAT_RESET       0x08
#define MAX_TX_STAT_RESET       0x10

#define	EMAC_STATS_TXDMA_STOPPED	(0)		/* stopped */
#define	EMAC_STATS_TXDMA_FETCH0		(1)		/* fetch descriptor */
#define	EMAC_STATS_TXDMA_FETCH1		(3)		/* fetch data buffer */
#define	EMAC_STATS_TXDMA_CLOSE		(4)		/* close descriptor */
#define	EMAC_STATS_TXDMA_SUSPEND	(5)		/* suspended */

#define	EMAC_STATS_RXDMA_STOPPED	(0)		/* stopped */
#define	EMAC_STATS_RXDMA_FETCH		(1)		/* fetch descriptor */
#define	EMAC_STATS_RXDMA_WAIT0		(2)		/* wait for end of receive */
#define	EMAC_STATS_RXDMA_WAIT1		(3)		/* wait for rx frame */
#define	EMAC_STATS_RXDMA_SUSPEND	(4)		/* suspended */
#define	EMAC_STATS_RXDMA_CLOSE		(5)		/* close descriptor */
#define	EMAC_STATS_RXDMA_FLUSH		(6)		/* flush buffer */
#define	EMAC_STATS_RXDMA_PUT		(7)		/* put buffer */
#define	EMAC_STATS_RXDMA_WAIT2		(8)		/* wait for status */

#define EMAC_TXENABL            1
#define EMAC_RXENABLE           1
#define EMAC_STANDFMODE         (1<<3)
#define EMAC_AUTORETRY          (1<<3)

#define EMAC_DMA_TXSTART        1
#define EMAC_DMA_RXSTART        (1<<1)
#define EMAC_DMA_BL4            (4<<1)
#define EMAC_DMA_ARBRR          (1<<15)
#define EMAC_DMA_TXWFD          (1<<16)
#define EMAC_DMA_SBL            (1<<17)
#define EMAC_DMA_64BDAT         (1<<18)

#define MAX_TX_COUNTER          (12)
#define MAX_RX_COUNTER          (24)
#define TX_STAT_READ            (BIT(15))
#define RX_STAT_READ            (BIT(15))

#define TXDESC_INTONCOMP        (BIT(31))
#define TXDESC_LASTSEG          (BIT(30))
#define TXDESC_FIRSTSEG         (BIT(29))
#define TXDESC_ADDCRCDIS        (BIT(28))
#define TXDESC_DISPADDING       (BIT(27))
#define TXDESC_ENDOFRING        (BIT(26))
#define TXDESC_SECONDCHAIN      (BIT(25))
#define TXDESC_FORCEEOPERR      (BIT(24))
#define TXDESC_OWN              (BIT(31)) 

#define RXDESC_OWN              (BIT(31))
#define RXDESC_ENDOFRING        (BIT(26))

#define PHY_MODE_CTL                    (0)
#define PHY_MODE_CTL_RESET              BIT(15)
#define PHY_MODE_CTL_1000MB             BIT(6)
#define PHY_MODE_CTL_100MB              BIT(13)
#define PHY_MODE_CTL_10MB               (0)
#define PHY_MODE_CTL_LOOPBACK           BIT(14)
#define PHY_MODE_CTL_AUTONEG            BIT(12)
#define PHY_MODE_CTL_POWERDOWN          BIT(11)
#define PHY_MODE_CTL_ISOLATE            BIT(10)
#define PHY_MODE_CTL_RESTART_AUTONEG    BIT(9)
#define PHY_MODE_CTL_FULL_DUPLEX        BIT(8)
#define PHY_MODE_CTL_COL_TEST           BIT(7)

#define PHY_ID1                         (2)
#define PHY_ID2                         (3)

#define PHY_SPEED_AUTO                  (0)
#define PHY_SPEED_1000MB                (1)
#define PHY_SPEED_100MB                 (2)
#define PHY_SPEED_10MB                  (3)

enum AraMacRegVals {
	/* DMA config register */
	DmaSoftReset = 1,
	Dma1WordBurst = (0x01 << 1),
	Dma4WordBurst = (0x04 << 1),
	Dma16WordBurst = (0x10 << 1),
	DmaRoundRobin = (1 << 15),
	DmaWait4Done = (1 << 16),
	DmaStrictBurst = (1 << 17),
	Dma64BitMode = (1 << 18),
	/* DMA control register */
	DmaStartTx = (1 << 0),
	DmaStartRx = (1 << 1),
	/* DMA status/interrupt & interrupt mask registers */
	DmaTxDone = (1 << 0),
	DmaNoTxDesc = (1 << 1),
	DmaTxStopped = (1 << 2),
	DmaRxDone = (1 << 4),
	DmaNoRxDesc = (1 << 5),
	DmaRxStopped = (1 << 6),
	DmaRxMissedFrame = (1 << 7),
	DmaMacInterrupt = (1 << 8),
	DmaAllInts = DmaTxDone | DmaNoTxDesc | DmaTxStopped | DmaRxDone | 
		DmaNoRxDesc | DmaRxStopped | DmaRxMissedFrame | DmaMacInterrupt,
	DmaTxStateMask = (7 << 16),
	DmaTxStateStopped = (0 << 16),
	DmaTxStateFetchDesc = (1 << 16),
	DmaTxStateFetchData = (2 << 16),
	DmaTxStateWaitEOT = (3 << 16),
	DmaTxStateCloseDesc = (4 << 16),
	DmaTxStateSuspended = (5 << 16),
	DmaRxStateMask = (15 << 21),
	DmaRxStateStopped = (0 << 21),
	DmaRxStateFetchDesc = (1 << 21),
	DmaRxStateWaitEOR = (2 << 21),
	DmaRxStateWaitFrame = (3 << 21),
	DmaRxStateSuspended = (4 << 21),
	DmaRxStateCloseDesc = (5 << 21),
	DmaRxStateFlushBuf = (6 << 21),
	DmaRxStatePutBuf = (7 << 21),
	DmaRxStateWaitStatus = (8 << 21),
	/* MAC global control register */
	MacSpeed10M = (0 << 0),
	MacSpeed100M = (1 << 0),
	MacSpeed1G = (2 << 0),
	MacSpeedMask = (3 << 0),
	MacFullDuplex = (1 << 2),
	MacResetRxStats = (1 << 3),
	MacResetTxStats = (1 << 4),
	/* MAC TX control */
	MacTxEnable = (1 << 0),
	MacTxInvertFCS = (1 << 1),
	MacTxDisableFCSInsertion = (1 << 2),
	MacTxAutoRetry = (1 << 3),
	MacTxIFG96 = (0 << 4),
	MacTxIFG64 = (1 << 4),
	MacTxIFG128 = (2 << 4),
	MacTxIFG256 = (3 << 4),
	MacTxPreamble7 = (0 << 6),
	MacTxPreamble3 = (2 << 6),
	MacTxPreamble5 = (3 << 6),
	/* MAC RX control */
	MacRxEnable = (1 << 0),
	MacRxDisableFcs = (1 << 1),
	MacRxStripFCS = (1 << 2),
	MacRxStoreAndForward = (1 << 3),
	MacRxPassBadFrames = (1 << 5),
	MacAccountVLANs = (1 << 6),
	/* MAC address control */
	MacAddr1Enable = (1 << 0),
	MacAddr2Enable = (1 << 1),
	MacAddr3Enable = (1 << 2),
	MacAddr4Enable = (1 << 3),
	MacInverseAddr1Enable = (1 << 4),
	MacInverseAddr2Enable = (1 << 5),
	MacInverseAddr3Enable = (1 << 6),
	MacInverseAddr4Enable = (1 << 7),
	MacPromiscuous = (1 << 8),
	/* MAC flow control */
	MacFlowDecodeEnable = (1 << 0),
	MacFlowGenerationEnable = (1 << 1),
	MacAutoFlowGenerationEnable = (1 << 2),
	MacFlowMulticastMode = (1 << 3),
	MacBlockPauseFrames = (1 << 4),

	/* MDIO control register values */
	MacMdioCtrlPhyMask = 0x1f,
	MacMdioCtrlPhyShift = 0,
	MacMdioCtrlRegMask = 0x1f,
	MacMdioCtrlRegShift = 5,
	MacMdioCtrlRead = (1 << 10),
	MacMdioCtrlWrite = 0,
	MacMdioCtrlStart = (1 << 15),
	/* MDIO data register values */
	MacMdioDataMask = 0xffff,
	/* MAC interrupt & interrupt mask values */
	MacUnderrun = (1 << 0),
	MacJabber = (1 << 0),
	/* RX statistics counter control */
	RxStatReadBusy = (1 << 15),
	/* TX statistics counter control */
	TxStatReadBusy = (1 << 15),

	/* Flow Control */
	MacFCHighThreshold = (((QDPC_DESC_RING_SIZE - (QDPC_DESC_RING_SIZE >> 3)) & 0xffff) << 16),
	MacFCLowThreshold = (8 << 0),
};

#define QDPC_DMA_DS_ERROR	(DmaRxMissedFrame | DmaNoRxDesc)
#define QDPC_DMA_DS_INTR	(DmaRxDone | DmaMacInterrupt | QDPC_DMA_DS_ERROR)
#define QDPC_MAC_DS_INTR	(MacUnderrun | MacJabber)

#define QDPC_DMA_US_MISC	(DmaNoRxDesc | DmaRxStopped | DmaTxStopped | DmaRxMissedFrame )
#define QDPC_DMA_US_INTR	(DmaRxDone | DmaMacInterrupt | QDPC_DMA_US_MISC)
#define QDPC_MAC_US_INTR	(MacUnderrun | MacJabber)

#define QDPC_DMA_TX_IMC_EN					BIT(31)
#define QDPC_DMA_TX_IMC_NFRAMES(x)			((x)&0xF)
#define QDPC_DMA_TX_IMC_TICKS(ahb_ticks)	(((ahb_ticks) & 0xFFFFF) << 8)
#define QDPC_DMA_TX_IMC_NTICKS				0x07FFF
#define QDPC_DMA_TX_IMC_DELAY				(QDPC_DMA_TX_IMC_TICKS(QDPC_DMA_TX_IMC_NTICKS))

#define QDPC_DMA_TX_STATUS(x)				(((x) >> 16) & 0x7)
#define QDPC_DMA_RX_STATUS(x)				(((x) >> 20) & 0xf)

#define	QDPC_DMA_INT_MITIGATION_NUM			(8)
#define	QDPC_DMA_AUTO_POLLING_CNT			(0x1FFF)

int qdpc_emac_init(struct qdpc_priv *priv);
int qdpc_emac_dma(void *dst, void *src, u32 len);
void qdpc_flush_and_inv_dcache_range(unsigned long start, unsigned long end);
void qdpc_emac_ack(void);

void qdpc_emac_enable(struct qdpc_priv *priv);
void qdpc_emac_disable(struct qdpc_priv *priv);
void qdpc_emac_startdma(qdpc_pdring_t *pktqpktq);


#endif /* __QDPC_EMAC_H__ */
