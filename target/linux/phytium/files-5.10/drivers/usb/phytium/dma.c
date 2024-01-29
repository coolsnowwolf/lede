// SPDX-License-Identifier: GPL-2.0

#include <linux/errno.h>
#include <linux/string.h>
#include "core.h"
#include "dma.h"
#include "hw-regs.h"

#define TRB_POOL_SIZE (sizeof(struct DMA_Trb) * (NUM_OF_TRB))
/* burst length 'x' should be multiples of 2 */
#define TRB_BURST_LENGTH(x)        (((x) & 0xFF) << 24)

#define BUILD_NORMAL_TRB_NO_IOC(trb, data_ptr, data_size, stream_id) { \
	trb.dmaAddr = data_ptr; \
	trb.dmaSize = data_size; \
	trb.ctrl = (stream_id << 16) | TD_TYPE_NORMAL | TDF_CYCLE_BIT; }

#define BUILD_NORMAL_TRB_NO_IOC_CHAIN(trb, data_ptr, data_size, stream_id) { \
	trb.dmaAddr = data_ptr; \
	trb.dmaSize = data_size; \
	trb.ctrl = (stream_id << 16) | TD_TYPE_NORMAL | TDF_CYCLE_BIT | TDF_CHAIN_BIT; }

#define BUILD_NORMAL_TRB(trb, data_ptr, data_size, stream_id) { \
	trb.dmaAddr = data_ptr; \
	trb.dmaSize = (data_size) | TRB_BURST_LENGTH(0x80); \
	trb.ctrl = (stream_id << 16) | TD_TYPE_NORMAL | TDF_CYCLE_BIT |\
	TDF_INT_ON_COMPLECTION | TDF_INT_ON_SHORT_PACKET; }

#define BUILD_LINK_TRB(trb, target_ptr) { \
	trb.dmaAddr = target_ptr; \
	trb.dmaSize = 0; \
	trb.ctrl = TD_TYPE_LINK | TDF_CYCLE_BIT | TDF_CHAIN_BIT; }

#define BUILD_EMPTY_TRB(trb) { \
	trb.dmaAddr = 0; \
	trb.dmaSize = 0; \
	trb.ctrl = 0; }


uint32_t divRoundUp(uint32_t divident, uint32_t divisor)
{
	return divisor ? ((divident + divisor - 1) / divisor) : 0;
}

static inline struct DMA_TrbChainDesc *GetTrbChainDescEntry(struct list_head *list)
{
	return (struct DMA_TrbChainDesc *)((uintptr_t)list -
			(uintptr_t)&(((struct DMA_TrbChainDesc *)0)->chainNode));
}

static int32_t phytium_dma_probe(struct DMA_CFG *config, struct DMA_SYSREQ *sysReq)
{
	if (!sysReq)
		return -EINVAL;

	sysReq->trbMemSize = TRB_POOL_SIZE;
	sysReq->privDataSize = sizeof(struct DMA_CONTROLLER);

	return 0;
}

static int32_t phytium_dma_init(struct DMA_CONTROLLER *priv,
		const struct DMA_CFG *config, struct DMA_CALLBACKS *callbacks)
{
	if (!priv || !config || !callbacks)
		return -EINVAL;

	if (!config->trbAddr || !config->trbDmaAddr)
		return -EINVAL;

	memset((void *)priv, 0, sizeof(struct DMA_CONTROLLER));
	memset((void *)config->trbAddr, 0, TRB_POOL_SIZE);

	priv->trbDMAPoolAddr = config->trbDmaAddr;
	priv->trbPoolAddr = config->trbAddr;
	priv->regs = (struct DMARegs *)config->regBase;
	priv->dmaCfg = *config;
	priv->dmaDrv = DMA_GetInstance();
	priv->dmaCallbacks = *callbacks;
	priv->isHostCtrlMode = 0;
	return 0;
}

static void phytium_dma_destroy(struct DMA_CONTROLLER *priv)
{

}

static int32_t phytium_dma_start(struct DMA_CONTROLLER *priv)
{
	int i;

	if (!priv)
		return -EINVAL;

	priv->dmaMode = DMA_MODE_CHANNEL_INDIVIDUAL;
	if ((priv->dmaCfg.dmaModeRx & priv->dmaCfg.dmaModeTx) == 0xFFFF) {
		priv->dmaMode = DMA_MODE_GLOBAL_DMULT;
		phytium_write32(&priv->regs->conf, DMARF_DMULT);
	} else if ((priv->dmaCfg.dmaModeRx | priv->dmaCfg.dmaModeTx)) {
		priv->dmaMode = DMA_MODE_GLOBAL_DSING;
		phytium_write32(&priv->regs->conf, DMARF_DSING);
	}

	for (i = 0; i < MAX_DMA_CHANNELS; i++) {
		if (priv->dmaCfg.dmaModeRx & (1 << i)) {
			priv->rx[i].dmultEnabled = 1;
			priv->rx[i].maxTrbLen = TD_DMULT_MAX_TRB_DATA_SIZE;
			priv->rx[i].maxTdLen = TD_DMULT_MAX_TD_DATA_SIZE;
		} else {
			priv->rx[i].dmultEnabled = 0;
			priv->rx[i].maxTrbLen = TD_SING_MAX_TRB_DATA_SIZE;
			priv->rx[i].maxTdLen = TD_SING_MAX_TD_DATA_SIZE;
		}
		priv->rx[i].lastTransferLength = 0;

		if (priv->dmaCfg.dmaModeTx & (1 << i)) {
			priv->tx[i].dmultEnabled = 1;
			priv->tx[i].maxTrbLen = TD_DMULT_MAX_TRB_DATA_SIZE;
			priv->tx[i].maxTdLen = TD_DMULT_MAX_TD_DATA_SIZE;
		} else {
			priv->tx[i].dmultEnabled = 0;
			priv->tx[i].maxTrbLen = TD_SING_MAX_TRB_DATA_SIZE;
			priv->tx[i].maxTdLen = TD_SING_MAX_TD_DATA_SIZE;
		}
		priv->tx[i].lastTransferLength = 0;
	}

	return 0;
}

static uint32_t phytium_dma_stop(struct DMA_CONTROLLER *priv)
{

	return 0;
}

static int32_t updateDescBuffer(struct DMA_CONTROLLER *priv,
		struct DMA_TrbChainDesc *trbChainDesc, uint16_t status)
{
	uint32_t ep_sel, i;
	struct DMA_Channel *channel;

	if (!priv || !trbChainDesc)
		return -EINVAL;

	channel = trbChainDesc->channel;
	if (!channel)
		return -EINVAL;

	if (channel->isIsoc && trbChainDesc->lastTrbIsLink) {
		if (channel->trbChainDescList.prev == channel->trbChainDescList.next)
			return 0;
	}

	for (i = 0; i < trbChainDesc->numOfTrbs; i++) {
		if (trbChainDesc->framesDesc) {
			if (trbChainDesc->isoError)
				trbChainDesc->framesDesc[i].length = 0;
			else
				trbChainDesc->framesDesc[i].length =
					(uint32_t)trbChainDesc->trbPool[i].dmaSize & TD_SIZE_MASK;
		}

		trbChainDesc->actualLen +=
			(uint32_t)trbChainDesc->trbPool[i].dmaSize & TD_SIZE_MASK;
	}

	if (trbChainDesc->isoError)
		trbChainDesc->actualLen = 0;

	ep_sel = phytium_read32(&priv->regs->ep_sel);
	channel->lastTransferLength = trbChainDesc->actualLen;

	if (!trbChainDesc->lastTrbIsLink) {
		if (!list_empty(&trbChainDesc->chainNode)) {
			for (i = 0; i < trbChainDesc->mapSize; i++)
				priv->trbChainFreeMap[(trbChainDesc->start >> 3) + i] = 0;

			trbChainDesc->channel->numOfTrbChain--;
			trbChainDesc->reserved = 0;
			list_del(&trbChainDesc->chainNode);
			trbChainDesc = NULL;
		}
	}

	if (channel->trbChainDescList.prev == &channel->trbChainDescList)
		channel->status = DMA_STATUS_FREE;

	if (priv->dmaCallbacks.complete)
		priv->dmaCallbacks.complete(priv->parent, channel->hwUsbEppNum,
				channel->isDirTx, false);

	if (channel->trbChainDescList.next != &channel->trbChainDescList) {
		trbChainDesc = GetTrbChainDescEntry(channel->trbChainDescList.next);
		if (trbChainDesc && trbChainDesc->lastTrbIsLink) {
			if (!list_empty(&trbChainDesc->chainNode)) {
				for (i = 0; i < trbChainDesc->mapSize; i++)
					priv->trbChainFreeMap[(trbChainDesc->start >> 3) + i] = 0;

				trbChainDesc->channel->numOfTrbChain--;
				trbChainDesc->reserved = 0;
				list_del(&trbChainDesc->chainNode);
			}
		}
	}

	phytium_write32(&priv->regs->ep_sel, ep_sel);

	return 0;
}

static void phytium_dma_isr(struct DMA_CONTROLLER *priv)
{
	uint32_t ep_sts, ep_ists, ep_cfg;
	int i;
	uint8_t isDirTx, epNum;
	struct DMA_Channel *channel;
	struct DMA_TrbChainDesc *trbChainDesc;

	if (!priv)
		return;

	ep_ists = phytium_read32(&priv->regs->ep_ists);
	if (!ep_ists) {
		phytium_write32(&priv->regs->ep_sts, DMARF_EP_IOC |
				DMARF_EP_ISP | DMARF_EP_TRBERR);
		return;
	}

	for (i = 0; i < 32; i++) {
		if (!(ep_ists & (1 << i)))
			continue;

		isDirTx = i > 15 ? DMARD_EP_TX : 0u;
		epNum = isDirTx ? i - 16 : i;
		phytium_write32(&priv->regs->ep_sel, epNum | isDirTx);

		if (isDirTx)
			channel = &priv->tx[epNum];
		else
			channel = &priv->rx[epNum];

		ep_sts = phytium_read32(&priv->regs->ep_sts);

		if (ep_sts & DMARF_EP_TRBERR)
			phytium_write32(&priv->regs->ep_sts, DMARF_EP_TRBERR);

		if ((ep_sts & DMARF_EP_IOC) || (ep_sts & DMARF_EP_ISP) || (channel->dmultGuard
					& DMARF_EP_IOC) || (channel->dmultGuard & DMARF_EP_ISP)) {
retransmit:
			phytium_write32(&priv->regs->ep_sts, DMARF_EP_IOC | DMARF_EP_ISP);
			trbChainDesc = GetTrbChainDescEntry(channel->trbChainDescList.next);
			if (!(ep_sts & DMARF_EP_TRBERR) && channel->dmultEnabled
					&& !trbChainDesc->lastTrbIsLink) {
				if (!priv->isHostCtrlMode && !channel->isDirTx) {
					channel->dmultGuard = 0;
					phytium_write32(&priv->regs->ep_sts, DMARF_EP_TRBERR);
					ep_cfg = phytium_read32(&priv->regs->ep_cfg);
					ep_cfg &= ~DMARV_EP_ENABLED;
					phytium_write32(&priv->regs->ep_cfg, (uint32_t)ep_cfg);
					updateDescBuffer(priv, trbChainDesc, 0);
					break;
				}
				channel->dmultGuard = ep_sts;
				break;
			}

			channel->dmultGuard = 0;
			updateDescBuffer(priv, trbChainDesc, 0);
		}

		if (ep_sts & DMARF_EP_OUTSMM)
			phytium_write32(&priv->regs->ep_sts, DMARF_EP_OUTSMM);

		if (ep_sts & DMARF_EP_DESCMIS)
			phytium_write32(&priv->regs->ep_sts, DMARF_EP_DESCMIS);

		if (ep_sts & DMARF_EP_ISOERR) {
			phytium_write32(&priv->regs->ep_sts, DMARF_EP_ISOERR);
			trbChainDesc = GetTrbChainDescEntry(channel->trbChainDescList.next);
			trbChainDesc->isoError = 1;
			goto retransmit;
		}
	}
}

static void phytium_dma_errIsr(struct DMA_CONTROLLER *priv, uint8_t irqNr, uint8_t isDirTx)
{
	struct DMA_Channel *channel;

	if (!priv)
		return;

	if (isDirTx)
		channel = &priv->tx[irqNr];
	else
		channel = &priv->rx[irqNr];

	if (channel->status >= DMA_STATUS_BUSY)
		channel->status = DMA_STATUS_ABORT;

	if (priv->dmaCallbacks.complete) {
		if (!irqNr && priv->resubmit) {
			priv->dmaCallbacks.complete(priv->parent, channel->hwUsbEppNum,
					channel->isDirTx, true);
			priv->resubmit = false;
		} else
			priv->dmaCallbacks.complete(priv->parent, channel->hwUsbEppNum,
					channel->isDirTx, false);
	}
}

static void *phytium_dma_channelAlloc(struct DMA_CONTROLLER *priv,
		uint8_t isDirTx, uint8_t hwEpNum, uint8_t isIsoc)
{
	struct DMA_Channel *channel;
	uint32_t ep_ien, ep_cfg;
	uint16_t dmaMode;

	if (!priv || (hwEpNum >= MAX_DMA_CHANNELS))
		return NULL;

	if (!priv->regs) {
		pr_err("dma regs is null\n");
		return NULL;
	}

	ep_ien = phytium_read32(&priv->regs->ep_ien);

	if (isDirTx) {
		if (priv->tx[hwEpNum].status > DMA_STATUS_FREE)
			return NULL;

		channel = &priv->tx[hwEpNum];
		channel->isDirTx = 0x80;
		ep_ien |= (0x01 << (hwEpNum + 16));
		dmaMode = priv->dmaCfg.dmaModeTx;
	} else {
		if (priv->rx[hwEpNum].status > DMA_STATUS_FREE)
			return NULL;

		channel = &priv->rx[hwEpNum];
		channel->isDirTx = 0x00;
		ep_ien |= (0x01 << hwEpNum);
		dmaMode = priv->dmaCfg.dmaModeRx;
	}

	channel->isIsoc = 0;
	if (isIsoc)
		channel->isIsoc = 1;

	INIT_LIST_HEAD(&channel->trbChainDescList);
	channel->numOfTrbChain = 0;
	channel->controller = priv;
	channel->dmultGuard = 0;
	channel->status = DMA_STATUS_FREE;
	channel->hwUsbEppNum = hwEpNum;

	phytium_write32(&priv->regs->ep_sel, (uint32_t)hwEpNum | channel->isDirTx);
	ep_cfg = phytium_read32(&priv->regs->ep_cfg);

	if (priv->dmaMode == DMA_MODE_CHANNEL_INDIVIDUAL) {
		if (dmaMode & (1 << hwEpNum))
			ep_cfg |= DMARV_EP_DMULT;
		else
			ep_cfg |= DMARV_EP_DSING;
	}

	phytium_write32(&priv->regs->ep_sts, DMARF_EP_IOC | DMARF_EP_ISP | DMARF_EP_TRBERR);
	phytium_write32(&priv->regs->ep_cfg, (uint32_t)DMARV_EP_ENABLED | ep_cfg);
	phytium_write32(&priv->regs->ep_ien, ep_ien);
	phytium_write32(&priv->regs->ep_sts_en, DMARF_EP_TRBERR);

	return channel;
}

static int32_t phytium_dma_channelRelease(struct DMA_CONTROLLER *priv, struct DMA_Channel *channel)
{
	uint32_t ep_ien, i;
	struct DMA_TrbChainDesc *trbChainDesc;

	if (!channel || !priv)
		return -EINVAL;

	ep_ien = phytium_read32(&priv->regs->ep_ien);
	if (channel->isDirTx)
		ep_ien &= ~(0x01 << (channel->hwUsbEppNum + 16));
	else
		ep_ien &= ~(0x01 << channel->hwUsbEppNum);

	phytium_write32(&priv->regs->ep_sel, (uint32_t)channel->hwUsbEppNum | channel->isDirTx);
	phytium_write32(&priv->regs->ep_cfg, (uint32_t)0);
	phytium_write32(&priv->regs->ep_ien, ep_ien);
	phytium_write32(&priv->regs->ep_sts_en, 0x0);
	phytium_write32(&priv->regs->ep_cmd, DMARF_EP_EPRST);
	phytium_write32(&priv->regs->ep_sts, DMARF_EP_IOC | DMARF_EP_ISP |
			DMARF_EP_TRBERR | DMARF_EP_ISOERR);

	if (channel->status >= DMA_STATUS_BUSY)
		channel->status = DMA_STATUS_ABORT;

	priv->dmaDrv->dma_channelAbort(priv, channel);

	while (channel->trbChainDescList.next != &channel->trbChainDescList) {
		trbChainDesc = GetTrbChainDescEntry(channel->trbChainDescList.next);
		if (trbChainDesc) {
			if (!list_empty(&trbChainDesc->chainNode)) {
				for (i = 0; i < trbChainDesc->mapSize; i++)
					priv->trbChainFreeMap[(trbChainDesc->start >> 3) + i] = 0;

				trbChainDesc->channel->numOfTrbChain--;
				trbChainDesc->reserved = 0;
				list_del(&trbChainDesc->chainNode);
			}
		}
	}

	channel->status = DMA_STATUS_UNKNOW;

	return 0;
}

static void ShowTrbChain(struct DMA_TrbChainDesc *trbChainDesc)
{
	int i;

	if (!trbChainDesc)
		return;
	pr_debug("Trb Chain %p for channel %p\n", trbChainDesc, trbChainDesc->channel);
	pr_debug("idx	| trb size	| trb addr	| FLAG: NORMAL  LINK  CHAIN  ALL\n");
	for (i = 0; i < trbChainDesc->numOfTrbs + 2; i++)
		pr_debug("%02d	| %08x	| %08x	|	%d	%d	%d	%08x\n",
			i,
			trbChainDesc->trbPool[i].dmaSize & TD_SIZE_MASK,
			trbChainDesc->trbPool[i].dmaAddr,
			(trbChainDesc->trbPool[i].ctrl & TD_TYPE_NORMAL) ? 1 : 0,
			(trbChainDesc->trbPool[i].ctrl & TD_TYPE_LINK) ? 1 : 0,
			(trbChainDesc->trbPool[i].ctrl & TDF_CHAIN_BIT) ? 1 : 0,
			trbChainDesc->trbPool[i].ctrl);
}

static uint32_t phytium_dma_NewTd(struct DMA_CONTROLLER *priv,
		struct DMA_TrbChainDesc *trbChainDesc)
{
	uint32_t startAddress, dataSize;
	struct DMA_Channel *channel;
	int i = 0;
	uint32_t tmp = 0;

	if (!priv || !trbChainDesc)
		return 0;

	startAddress = trbChainDesc->dwStartAddress;
	channel = trbChainDesc->channel;
	dataSize = channel->maxTrbLen;

	if (trbChainDesc->numOfTrbs > 1) {
		for (i = 0; i < (trbChainDesc->numOfTrbs - 1); i++) {
			if (channel->dmultEnabled) {
				if (trbChainDesc->framesDesc && priv->isHostCtrlMode
						&& channel->isIsoc) {
					BUILD_NORMAL_TRB_NO_IOC(trbChainDesc->trbPool[i],
							trbChainDesc->dwStartAddress +
							trbChainDesc->framesDesc[i].offset,
							trbChainDesc->framesDesc[i].length, 0);
					continue;
				} else
					BUILD_NORMAL_TRB_NO_IOC(trbChainDesc->trbPool[i],
							startAddress, dataSize, 0);
			} else
				BUILD_NORMAL_TRB_NO_IOC_CHAIN(trbChainDesc->trbPool[i],
						startAddress, dataSize, 0);
			startAddress += dataSize;
			tmp += dataSize;
		}
	}

	if (trbChainDesc->framesDesc && priv->isHostCtrlMode && channel->isIsoc) {
		BUILD_NORMAL_TRB(trbChainDesc->trbPool[i],
				trbChainDesc->dwStartAddress + trbChainDesc->framesDesc[i].offset,
				trbChainDesc->framesDesc[i].length, 0);
	} else
		BUILD_NORMAL_TRB(trbChainDesc->trbPool[i], startAddress,
				trbChainDesc->len - tmp, 0);

	trbChainDesc->lastTrbIsLink = 0;
	if (channel->dmultEnabled) {
		if (channel->isIsoc) {
			trbChainDesc->lastTrbIsLink = 1;
			BUILD_LINK_TRB(trbChainDesc->trbPool[i + 1], trbChainDesc->trbDMAAddr);
		} else
			BUILD_EMPTY_TRB(trbChainDesc->trbPool[i + 1]);
	}

	ShowTrbChain(trbChainDesc);

	return 0;
}

static void phytium_dma_ArmTd(struct DMA_CONTROLLER *priv, struct DMA_TrbChainDesc *trbChainDesc)
{
	uint32_t ep_sts, ep_cfg, ep_cmd;
	struct DMA_TrbChainDesc *startedChain;
	struct DMA_Trb *lastPrevTrb;
	struct DMA_Channel *channel;

	if (!priv || !trbChainDesc)
		return;

	channel = trbChainDesc->channel;
	phytium_write32(&priv->regs->ep_sel, (channel->isDirTx | channel->hwUsbEppNum));

	ep_sts = phytium_read32(&priv->regs->ep_sts);

	if (channel->status == DMA_STATUS_FREE) {
		phytium_write32(&priv->regs->ep_sts, DMARF_EP_TRBERR);
		phytium_write32(&priv->regs->traddr, trbChainDesc->trbDMAAddr);
		phytium_write32(&priv->regs->ep_sts, DMARF_EP_TRBERR);

		ep_cfg = phytium_read32(&priv->regs->ep_cfg);
		phytium_write32(&priv->regs->ep_cmd, DMARF_EP_DRDY);
		if (!(ep_cfg & DMARV_EP_ENABLED)) {
			ep_cfg |= DMARV_EP_ENABLED;
			phytium_write32(&priv->regs->ep_cfg, ep_cfg);
		}
	} else {
		if (channel->trbChainDescList.prev != channel->trbChainDescList.next) {
			startedChain = GetTrbChainDescEntry(channel->trbChainDescList.prev->prev);
			lastPrevTrb = &startedChain->trbPool[startedChain->numOfTrbs];
			startedChain->lastTrbIsLink = 1;
			BUILD_LINK_TRB((*lastPrevTrb), trbChainDesc->trbDMAAddr);
			ep_cmd = phytium_read32(&priv->regs->ep_cmd);
			if (!(ep_cmd & DMARF_EP_DRDY)) {
				phytium_write32(&priv->regs->traddr, trbChainDesc->trbDMAAddr);
				phytium_write32(&priv->regs->ep_cmd, DMARF_EP_DRDY);
			}
			ShowTrbChain(startedChain);
		}
	}
}

static int32_t phytium_dma_TrbChainAlloc(struct DMA_CONTROLLER *priv,
		struct DMA_Channel *channel, uint32_t numOfTrbs, struct DMA_TrbChainDesc **chain)
{
	struct DMA_TrbChainDesc *trbChainDesc = NULL;
	int i, j;
	uint32_t mapcount, count = 0;

	if (!priv || !channel)
		return -ENOMEM;

	for (i = 0; i < TAB_SIZE_OF_DMA_CHAIN; i++) {
		if (!priv->trbChainDesc[i].reserved) {
			trbChainDesc = &priv->trbChainDesc[i];
			break;
		}
	}

	if (!trbChainDesc) {
		pr_err("No Free TRB Chain Descriptor\n");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&trbChainDesc->chainNode);
	*chain = NULL;
	mapcount = (numOfTrbs + 2 + 7) >> 3;

	for (i = 0; i < TRB_MAP_SIZE; i++) {
		if (priv->trbChainFreeMap[i] == 0x0)
			count++;
		else
			count = 0;

		if (count == mapcount)
			break;
	}

	if (count != mapcount) {
		pr_err("No Free TRBs count:0x%x, mapcount:0x%x\n", count, mapcount);
		return -ENOMEM;
	}

	trbChainDesc->reserved = 1;
	trbChainDesc->channel = channel;
	trbChainDesc->actualLen = 0;
	trbChainDesc->mapSize = mapcount;
	trbChainDesc->numOfTrbs = numOfTrbs;
	trbChainDesc->start = (i + 1 - mapcount) << 3;
	trbChainDesc->end = trbChainDesc->start;
	trbChainDesc->trbPool = (struct DMA_Trb *)priv->trbPoolAddr + trbChainDesc->start;
	trbChainDesc->trbDMAAddr = (uint32_t)(uintptr_t)((struct DMA_Trb *)priv->trbDMAPoolAddr
			+ trbChainDesc->start);
	trbChainDesc->isoError = 0;

	list_add_tail(&trbChainDesc->chainNode, &channel->trbChainDescList);

	channel->numOfTrbChain++;

	for (j = 0; j < count; j++)
		priv->trbChainFreeMap[i - j] = 0xFF;

	*chain = trbChainDesc;

	return 0;
}

static int32_t phytium_dma_channelProgram(struct DMA_CONTROLLER *priv,
		struct DMA_Channel *channel, uint16_t packetSize, uintptr_t dmaAddr,
		uint32_t len, void *framesDesc, uint32_t framesNumber)
{
	uint32_t numOfTrbs;
	uint8_t retval;
	struct DMA_TrbChainDesc *trbChainDesc;

	if (!priv || !channel)
		return -EINVAL;

	//printk(KERN_ERR "%s %d\n",__func__,__LINE__);
	if (framesDesc && priv->isHostCtrlMode && channel->isIsoc)
		numOfTrbs = framesNumber;
	else
		numOfTrbs = divRoundUp(len, channel->maxTrbLen);

	retval = phytium_dma_TrbChainAlloc(priv, channel, numOfTrbs, &trbChainDesc);
	if (retval)
		return retval;

	trbChainDesc->dwStartAddress = dmaAddr;
	trbChainDesc->len = len;
	trbChainDesc->framesDesc = framesDesc;

	channel->wMaxPacketSize = packetSize;
	phytium_dma_NewTd(priv, trbChainDesc);
	channel->lastTransferLength = 0;
	phytium_dma_ArmTd(priv, trbChainDesc);

	channel->status = DMA_STATUS_BUSY;

	return 0;
}

static enum DMA_Status phytium_dma_getChannelStatus(struct DMA_CONTROLLER *priv,
		struct DMA_Channel *channel)
{
	uint32_t ep_cmd, ep_sts;

	if (!priv || !channel)
		return DMA_STATUS_UNKNOW;

	if (channel->status >= DMA_STATUS_BUSY) {
		phytium_write32(&priv->regs->ep_sel, channel->isDirTx | channel->hwUsbEppNum);
		ep_cmd = phytium_read32(&priv->regs->ep_cmd);
		ep_sts = phytium_read32(&priv->regs->ep_sts);

		if ((ep_cmd & DMARF_EP_DRDY) || (ep_sts & DMARF_EP_DBUSY))
			channel->status = DMA_STATUS_ARMED;
		else
			channel->status = DMA_STATUS_BUSY;
	}

	return channel->status;
}

static int32_t phytium_dma_channelAbort(struct DMA_CONTROLLER *priv, struct DMA_Channel *channel)
{
	struct DMA_TrbChainDesc *trbChainDesc;
	uint32_t ep_cfg, i;

	if (!priv || !channel)
		return -EINVAL;

	if (phytium_dma_getChannelStatus(priv, channel) >= DMA_STATUS_BUSY)
		phytium_write32(&priv->regs->conf, DMARF_RESET);

	phytium_write32(&priv->regs->ep_sel, (uint32_t)(channel->isDirTx | channel->hwUsbEppNum));
	ep_cfg = phytium_read32(&priv->regs->ep_cfg);
	ep_cfg &= ~DMARV_EP_ENABLED;
	phytium_write32(&priv->regs->ep_cfg, ep_cfg);
	phytium_write32(&priv->regs->ep_sts, 0xFFFFFFFF);

	while (channel->trbChainDescList.next != &channel->trbChainDescList) {
		trbChainDesc = GetTrbChainDescEntry(channel->trbChainDescList.next);
		if (trbChainDesc) {
			if (!list_empty(&trbChainDesc->chainNode)) {
				for (i = 0; i < trbChainDesc->mapSize; i++)
					priv->trbChainFreeMap[(trbChainDesc->start >> 3) + i] = 0;

				trbChainDesc->channel->numOfTrbChain--;
				trbChainDesc->reserved = 0;
				list_del(&trbChainDesc->chainNode);
			}
		}
	}
	if (channel->status != DMA_STATUS_UNKNOW)
		channel->status = DMA_STATUS_FREE;

	return 0;
}

static int32_t phytium_dma_getActualLength(struct DMA_CONTROLLER *priv, struct DMA_Channel *channel)
{
	if (!priv || !channel)
		return -EINVAL;

	return channel->lastTransferLength;
}

static int32_t phytium_dma_getMaxLength(struct DMA_CONTROLLER *priv, struct DMA_Channel *channel)
{
	if (!priv || !channel)
		return -EINVAL;

	return channel->maxTdLen;
}

static int32_t phytium_dma_setMaxLength(struct DMA_CONTROLLER *priv,
		struct DMA_Channel *channel, uint32_t val)
{
	if (!priv || !channel)
		return -EINVAL;

	if (channel->dmultEnabled)
		channel->maxTrbLen = val;
	else
		channel->maxTrbLen = (val > TD_SING_MAX_TRB_DATA_SIZE) ?
			TD_SING_MAX_TRB_DATA_SIZE : val;

	return 0;
}

static void phytium_dma_setParentPriv(struct DMA_CONTROLLER *priv, void *parent)
{
	if (!priv)
		return;

	priv->parent = parent;
}

void phytium_dma_controllerReset(struct DMA_CONTROLLER *priv)
{
	uint32_t conf;

	if (!priv)
		return;

	conf = phytium_read32(&priv->regs->conf);
	conf |= DMARF_RESET;
	phytium_write32(&priv->regs->conf, conf);

	priv->resubmit = true;
}

void phytium_dma_setHostMode(struct DMA_CONTROLLER *priv)
{
	if (!priv)
		return;

	priv->isHostCtrlMode = 1;
}

struct DMA_OBJ phytium_dma_Drv = {
	.dma_probe = phytium_dma_probe,
	.dma_init = phytium_dma_init,
	.dma_destroy = phytium_dma_destroy,
	.dma_start = phytium_dma_start,
	.dma_stop = phytium_dma_stop,
	.dma_isr = phytium_dma_isr,
	.dma_errIsr = phytium_dma_errIsr,
	.dma_channelAlloc = phytium_dma_channelAlloc,
	.dma_channelRelease = phytium_dma_channelRelease,
	.dma_channelProgram = phytium_dma_channelProgram,
	.dma_channelAbort = phytium_dma_channelAbort,
	.dma_getChannelStatus = phytium_dma_getChannelStatus,
	.dma_getActualLength = phytium_dma_getActualLength,
	.dma_getMaxLength = phytium_dma_getMaxLength,
	.dma_setMaxLength = phytium_dma_setMaxLength,
	.dma_setParentPriv = phytium_dma_setParentPriv,
	.dma_controllerReset = phytium_dma_controllerReset,
	.dma_setHostMode = phytium_dma_setHostMode,
};

struct DMA_OBJ *DMA_GetInstance(void)
{
	return &phytium_dma_Drv;
}
