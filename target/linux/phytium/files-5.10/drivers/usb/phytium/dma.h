/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __PHYTIUM_DMA_H__
#define __PHYTIUM_DMA_H__

#include <linux/types.h>
//#include "list.h"

#define NUM_OF_TRB 1024
#define TRB_MAP_SIZE ((NUM_OF_TRB + (sizeof(uint8_t) * 8) - 1) / (sizeof(uint8_t) * 8))
#define MAX_DMA_CHANNELS 16
#define TAB_SIZE_OF_DMA_CHAIN (MAX_DMA_CHANNELS * 2)

#define DMARD_EP_TX 0x80ul
#define DMARD_EP_RX 0x00ul

#define DMARF_EP_EPRST	0x00000001ul
#define DMARF_EP_DRDY	0x00000040ul
#define DMARF_EP_DFLUSH	0x00000080ul

#define DMARF_EP_IOC		0x4ul
#define DMARF_EP_ISP		0x8ul
#define DMARF_EP_DESCMIS	0x10ul
#define DMARF_EP_TRBERR		0x80ul
#define DMARF_EP_DBUSY		0x200ul
#define DMARF_EP_CCS		0x800ul
#define DMARF_EP_OUTSMM		0x4000ul
#define DMARF_EP_ISOERR		0x8000ul
#define DMARF_EP_DTRANS		0x80000000ul

#define DMARV_EP_DISABLED	0ul
#define DMARV_EP_ENABLED	1ul
#define DMARV_EP_DSING		0x1000ul
#define DMARV_EP_DMULT		0x2000ul

#define TD_SIZE_MASK		0x00001FFFF

#define DMARF_RESET		0x00000001ul
#define DMARF_DSING		0x00000100ul
#define DMARF_DMULT		0x00000200ul

#define TD_DMULT_MAX_TRB_DATA_SIZE 65536u
#define TD_DMULT_MAX_TD_DATA_SIZE  (~1u)
#define TD_SING_MAX_TRB_DATA_SIZE 65536u
#define TD_SING_MAX_TD_DATA_SIZE 65536u

#define TD_TYPE_NORMAL	0x400L
#define TD_TYPE_LINK	0x1800L
#define TDF_CYCLE_BIT	0x1L
#define TDF_TOGGLE_CYCLE_BIT	0x2L
#define TDF_INT_ON_SHORT_PACKET	0x4L
#define TDF_FIFO_MODE		0x8L
#define TDF_CHAIN_BIT		0x10L
#define TDF_INT_ON_COMPLECTION	0x20L
#define TDF_STREAMID_VALID	0x200L

struct DMA_Trb {
	uint32_t dmaAddr;
	uint32_t dmaSize; /* 0:16 transfer length; 24:31 burst length */
	uint32_t ctrl;
};

enum DMA_Status {
	DMA_STATUS_UNKNOW,
	DMA_STATUS_FREE,
	DMA_STATUS_ABORT,
	DMA_STATUS_BUSY,
	DMA_STATUS_ARMED
};

struct DMA_CFG {
	uintptr_t regBase;
	uint16_t dmaModeTx;
	uint16_t dmaModeRx;
	void *trbAddr;
	uintptr_t trbDmaAddr;
};

struct DMA_SYSREQ {
	uint32_t privDataSize;
	uint32_t trbMemSize;
};

struct DMA_CALLBACKS {
	void (*complete)(void *pD, uint8_t epNum, uint8_t dir, bool resubmit);
};

struct DMA_CONTROLLER;

struct DMA_Channel {
	struct DMA_CONTROLLER *controller;
	uint16_t wMaxPacketSize;
	uint8_t hwUsbEppNum;
	uint8_t isDirTx;
	uint32_t maxTdLen;
	uint32_t maxTrbLen;
	enum DMA_Status status;
	void *priv;
	uint32_t dmultGuard;
	uint8_t dmultEnabled;
	uint8_t numOfTrbChain;
	struct list_head trbChainDescList;
	uint32_t lastTransferLength;
	uint8_t isIsoc;
};

struct DMA_OBJ {
	int32_t (*dma_probe)(struct DMA_CFG *config, struct DMA_SYSREQ *sysReq);

	int32_t (*dma_init)(struct DMA_CONTROLLER *priv, const struct DMA_CFG *config,
			struct DMA_CALLBACKS *callbacks);

	void (*dma_destroy)(struct DMA_CONTROLLER *priv);

	int32_t (*dma_start)(struct DMA_CONTROLLER *priv);

	uint32_t (*dma_stop)(struct DMA_CONTROLLER *priv);

	void (*dma_isr)(struct DMA_CONTROLLER *priv);

	void (*dma_errIsr)(struct DMA_CONTROLLER *priv, uint8_t irqNr, uint8_t isDirTx);

	void * (*dma_channelAlloc)(struct DMA_CONTROLLER *priv,
			uint8_t isDirTx, uint8_t hwEpNum, uint8_t isIso);

	int32_t (*dma_channelRelease)(struct DMA_CONTROLLER *priv, struct DMA_Channel *channel);

	int32_t (*dma_channelProgram)(struct DMA_CONTROLLER *priv, struct DMA_Channel *channel,
			uint16_t packetSize, uintptr_t dmaAddr,
			uint32_t len, void *framesDesc, uint32_t framesNumber);

	int32_t (*dma_channelAbort)(struct DMA_CONTROLLER *priv, struct DMA_Channel *channel);

	enum DMA_Status (*dma_getChannelStatus)(struct DMA_CONTROLLER *priv,
			struct DMA_Channel *channel);

	int32_t (*dma_getActualLength)(struct DMA_CONTROLLER *priv, struct DMA_Channel *channel);

	int32_t (*dma_getMaxLength)(struct DMA_CONTROLLER *priv, struct DMA_Channel *channel);

	int32_t (*dma_setMaxLength)(struct DMA_CONTROLLER *priv,
			struct DMA_Channel *channel, uint32_t val);

	void (*dma_setParentPriv)(struct DMA_CONTROLLER *priv, void *parent);

	void (*dma_controllerReset)(struct DMA_CONTROLLER *priv);

	void (*dma_setHostMode)(struct DMA_CONTROLLER *priv);
};

enum DMA_Mode {
	DMA_MODE_GLOBAL_DMULT,
	DMA_MODE_GLOBAL_DSING,
	DMA_MODE_CHANNEL_INDIVIDUAL,
};

struct DMA_TrbFrameDesc {
	uint32_t length;
	uint32_t offset;
};

struct DMA_TrbChainDesc {
	uint8_t reserved;
	struct DMA_Channel *channel;
	struct DMA_Trb	*trbPool;
	uint32_t trbDMAAddr;
	uint32_t len;
	uint32_t dwStartAddress;
	uint32_t actualLen;
	uint8_t isoError;
	uint8_t lastTrbIsLink;
	uint32_t mapSize;
	uint32_t numOfTrbs;
	uint32_t start;
	uint32_t end;
	struct DMA_TrbFrameDesc *framesDesc;
	struct list_head chainNode;
};

struct DMA_CONTROLLER {
	struct DMARegs *regs;
	struct DMA_OBJ *dmaDrv;
	struct DMA_CFG dmaCfg;
	struct DMA_CALLBACKS dmaCallbacks;
	struct DMA_Channel rx[MAX_DMA_CHANNELS];
	struct DMA_Channel tx[MAX_DMA_CHANNELS];
	enum DMA_Mode dmaMode;
	uint8_t isHostCtrlMode;
	void *parent;
	void *trbPoolAddr;
	uintptr_t trbDMAPoolAddr;
	uint8_t trbChainFreeMap[TRB_MAP_SIZE];
	struct DMA_TrbChainDesc trbChainDesc[TAB_SIZE_OF_DMA_CHAIN];
	bool resubmit;
};

struct DMA_OBJ *DMA_GetInstance(void);
#endif
