/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __PHYTIUM_GADGET_H_
#define __PHYTIUM_GADGET_H_

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include "dma.h"

struct GADGET_CTRL;
struct GADGET_EP;
struct GADGET_REQ;

enum GADGET_EP_STATE {
	GADGET_EP_FREE,
	GADGET_EP_ALLOCATED,
	GADGET_EP_BUSY,
	GADGET_EP_NOT_IMPLEMENTED
};

enum GADGET_EP0_STAGE {
	GADGET_EP0_STAGE_SETUP,
	GADGET_EP0_STAGE_IN,
	GADGET_EP0_STAGE_OUT,
	GADGET_EP0_STAGE_STATUSIN,
	GADGET_EP0_STAGE_STATUSOUT,
	GADGET_EP0_STAGE_ACK
};

struct GADGET_EP_OPS {
	int32_t (*epEnable)(struct GADGET_CTRL *priv, struct GADGET_EP *ep,
			const struct usb_endpoint_descriptor *desc);

	int32_t (*epDisable)(struct GADGET_CTRL *priv, struct GADGET_EP *ep);

	int32_t (*epSetHalt)(struct GADGET_CTRL *priv, struct GADGET_EP *ep, uint8_t value);

	int32_t (*epSetWedge)(struct GADGET_CTRL *priv, struct GADGET_EP *ep);

	int32_t (*epFifoStatus)(struct GADGET_CTRL *priv, struct GADGET_EP *ep);

	int32_t (*epFifoFlush)(struct GADGET_CTRL *priv, struct GADGET_EP *ep);

	int32_t (*reqQueue)(struct GADGET_CTRL *priv, struct GADGET_EP *ep,
			struct GADGET_REQ *req);

	int32_t (*reqDequeue)(struct GADGET_CTRL *priv, struct GADGET_EP *ep,
			struct GADGET_REQ *req);

	int32_t (*reqAlloc)(struct GADGET_CTRL *priv, struct GADGET_EP *ep,
			struct GADGET_REQ **req);

	void (*reqFree)(struct GADGET_CTRL *priv, struct GADGET_EP *ep,
			struct GADGET_REQ *req);
};

struct GADGET_EP {
	struct list_head epList;
	char name[255];
	struct GADGET_EP_OPS *ops;
	uint16_t maxPacket;
	uint16_t maxStreams;
	uint8_t mult;
	uint8_t maxburst;
	uint8_t address;
	const struct usb_endpoint_descriptor *desc;
	const struct usb_ss_ep_comp_descriptor *compDesc;
};

enum GADGET_DMAInterfaceWidth {
	GADGET_DMA_32_WIDTH = 4,
	GADGET_DMA_64_WIDTH = 8,
};

struct GADGET_EP_CFG {
	uint8_t bufferingValue;
	uint16_t startBuf;
	uint16_t maxPacketSize;
};

struct GADGET_CFG {
	uintptr_t regBase;
	uintptr_t phy_regBase;
	struct GADGET_EP_CFG epIN[16];
	struct GADGET_EP_CFG epOUT[16];
	enum GADGET_DMAInterfaceWidth dmaInterfaceWidth;
	void *trbAddr;
	uintptr_t trbDmaAddr;
};

struct GADGET_SYSREQ {
	uint32_t privDataSize;
	uint32_t trbMemSize;
};

struct GadgetEp {
	struct GADGET_EP gadgetEp;
	enum GADGET_EP_STATE state;
	uint8_t hwEpNum;
	uint8_t isInEp;
	struct list_head request;
	uint8_t iso_flag;
	void *channel;
	uint32_t requestsInList;
	uint8_t wedged;
};

struct GADGET_DEV {
	struct list_head epList;
	struct GADGET_EP *ep0;
	unsigned int speed;
	unsigned int maxSpeed;
	enum usb_device_state state;
	uint8_t sgSupported;
	uint8_t bHnpEnable;
	uint8_t aHnpSupport;
	char name[255];
};

struct GADGET_SgList {
	uintptr_t link;
	uint32_t offset;
	uint32_t length;
	uintptr_t dmaAddress;
};

struct GADGET_REQ {
	struct list_head list;
	void *buf;
	uint32_t length;
	uintptr_t dma;
	uint32_t numOfSgs;
	uint32_t numMappedSgs;
	uint16_t streamId;
	uint8_t oInterrupt;
	uint8_t zero;
	uint8_t shortNotOk;
	void *context;
	uint32_t status;
	uint32_t actual;
	struct GADGET_SgList *sg;
	void (*complete)(struct GADGET_EP *ep, struct GADGET_REQ *req);
};


struct GADGET_CALLBACKS {
	void (*disconnect)(struct GADGET_CTRL *priv);

	void (*connect)(struct GADGET_CTRL *priv);

	int32_t (*setup)(struct GADGET_CTRL *priv,
			struct usb_ctrlrequest *ctrl);

	void *(*usbRequestMemAlloc)(struct GADGET_CTRL *priv,
			uint32_t requiredSize);

	void (*usbRequestMemFree)(struct GADGET_CTRL *priv, void *usbRequest);
};

struct GADGET_OBJ {
	int32_t (*gadget_init)(struct GADGET_CTRL *priv, struct GADGET_CFG *config,
			struct GADGET_CALLBACKS *callbacks, struct device *pdev);

	void (*gadget_destroy)(struct GADGET_CTRL *priv);

	void (*gadget_start)(struct GADGET_CTRL *priv);

	void (*gadget_stop)(struct GADGET_CTRL *priv);

	void (*gadget_isr)(struct GADGET_CTRL *priv);

	int32_t (*gadget_epEnable)(struct GADGET_CTRL *priv, struct GADGET_EP *ep,
			const struct usb_endpoint_descriptor *desc);

	int32_t (*gadget_epDisable)(struct GADGET_CTRL *priv, struct GADGET_EP *ep);

	int32_t (*gadget_epSetHalt)(struct GADGET_CTRL *priv, struct GADGET_EP *ep, uint8_t value);

	int32_t (*gadget_epSetWedge)(struct GADGET_CTRL *priv, struct GADGET_EP *ep);

	int32_t (*gadget_epFifoStatus)(struct GADGET_CTRL *priv, struct GADGET_EP *ep);

	void (*gadget_epFifoFlush)(struct GADGET_CTRL *priv, struct GADGET_EP *ep);

	int32_t (*gadget_reqQueue)(struct GADGET_CTRL *priv, struct GADGET_EP *ep,
			struct GADGET_REQ *req);

	int32_t (*gadget_reqDequeue)(struct GADGET_CTRL *priv, struct GADGET_EP *ep,
			struct GADGET_REQ *req);

	int32_t (*gadget_reqAlloc)(struct GADGET_CTRL *priv, struct GADGET_EP *ep,
			struct GADGET_REQ **req);

	void (*gadget_reqFree)(struct GADGET_CTRL *priv, struct GADGET_EP *ep,
			struct GADGET_REQ *req);

	void (*gadget_getDevInstance)(struct GADGET_CTRL *priv, struct GADGET_DEV **dev);

	int32_t (*gadget_dGetFrame)(struct GADGET_CTRL *priv, uint32_t *numOfFrame);

	int32_t (*gadget_dWakeUp)(struct GADGET_CTRL *priv);

	int32_t (*gadget_dSetSelfpowered)(struct GADGET_CTRL *priv);

	int32_t (*gadget_dClearSelfpowered)(struct GADGET_CTRL *priv);

	int32_t (*gadget_dVbusSession)(struct GADGET_CTRL *priv, uint8_t isActive);

	int32_t (*gadget_dVbusDraw)(struct GADGET_CTRL *priv, uint8_t mA);

	int32_t (*gadget_dPullUp)(struct GADGET_CTRL *priv, uint8_t isOn);

	void (*gadget_dGetConfigParams)(struct GADGET_CTRL *priv,
			struct usb_dcd_config_params *configParams);
};

struct GADGET_CTRL {
	struct device *dev;
	struct GADGET_DEV gadgetDev;
	struct HW_REGS *regs;
	struct GADGET_OBJ *gadgetDrv;
	struct GADGET_CFG gadgetCfg;
	struct GADGET_CALLBACKS eventCallback;
	struct GadgetEp in[16];
	struct GadgetEp out[16];
	enum GADGET_EP0_STAGE ep0State;
	uint8_t isRemoteWakeup;
	uint8_t isSelfPowered;
	uint8_t deviceAddress;
	struct DMA_OBJ *dmaDrv;
	void *dmaController;
	struct DMA_CFG dmaCfg;
	struct DMA_CALLBACKS dmaCallback;
	uint8_t releaseEp0Flag;
	uint8_t isReady;
	uint8_t *privBuffAddr;
	uintptr_t privBuffDma;
	uint8_t endpointInList;
	uint8_t hostRequestFlag;
	struct VHUB_REGS *phy_regs;
};

struct GadgetRequest {
	struct GADGET_REQ request;
	struct GadgetEp *ep;
	struct GADGET_DEV *dev;
	uint8_t zlp;
};

struct GADGET_OBJ *GADGET_GetInstance(void);

#endif /* __LINUX_PHYTIUM_GADGET */

