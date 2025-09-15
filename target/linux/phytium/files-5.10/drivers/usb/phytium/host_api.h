/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __PHYTIUM_HOST_API_H_
#define __PHYTIUM_HOST_API_H_

#include <linux/usb/ch9.h>
//#include "list.h"
#include "dma.h"

#define MAX_SUPPORTED_DEVICES 16
#define USB_PORT_STAT_RESUME (1 << 31)
#define MAX_INSTANCE_EP_NUM 6

enum HOST_OtgState {
	HOST_OTG_STATE_A_IDLE,
	HOST_OTG_STATE_A_WAIT_VRISE,
	HOST_OTG_STATE_A_WAIT_BCON,
	HOST_OTG_STATE_A_HOST,
	HOST_OTG_STATE_A_SUSPEND,
	HOST_OTG_STATE_A_PERIPHERAL,
	HOST_OTG_STATE_A_VBUS_ERR,
	HOST_OTG_STATE_A_WAIT_VFALL,
	HOST_OTG_STATE_B_IDLE = 0x10,
	HOST_OTG_STATE_B_PERIPHERAL,
	HOST_OTG_STATE_B_WAIT_ACON,
	HOST_OTG_STATE_B_HOST,
	HOST_OTG_STATE_B_HOST_2,
	HOST_OTG_STATE_B_SRP_INT1,
	HOST_OTG_STATE_B_SRP_INT2,
	HOST_OTG_STATE_B_DISCHRG1,
	HOST_OTG_STATE_B_DISCHRG2,
	HOST_OTG_STATE_UNKNOWN,
};

enum HOST_EP0_STAGE {
	HOST_EP0_STAGE_IDLE,
	HOST_EP0_STAGE_SETUP,
	HOST_EP0_STAGE_IN,
	HOST_EP0_STAGE_OUT,
	HOST_EP0_STAGE_STATUSIN,
	HOST_EP0_STAGE_STATUSOUT,
	HOST_EP0_STAGE_ACK,
};

enum HOST_EP_STATE {
	HOST_EP_FREE,
	HOST_EP_ALLOCATED,
	HOST_EP_BUSY,
	HOST_EP_NOT_IMPLEMENTED
};

struct HOST_DEVICE {
	uint8_t devnum;
	uint8_t hubPort;
	unsigned int speed;
	struct HOST_DEVICE *parent;
	void *hcPriv;
	void *userExt;
};

struct HOST_EP {
	struct usb_endpoint_descriptor desc;
	struct list_head reqList;
	void *userExt;
	uint8_t *hcPriv;
};

struct HOST_USB_DEVICE {
	struct HOST_EP ep0_hep;
	struct HOST_EP *in_ep[16];
	struct HOST_EP *out_ep[16];
	struct HOST_DEVICE udev;
	struct usb_device *ld_udev;
};

struct HostEp {
	uint8_t name[255];
	uint8_t hwEpNum;
	uint8_t hwBuffers;
	uint16_t hwMaxPacketSize;
	uint8_t isInEp;
	void *channel;
	uint8_t usbEpNum;
	uint8_t type;
	uint8_t usbPacketSize;
	enum HOST_EP_STATE state;
	struct HOST_EP *scheduledUsbHEp;
	uint8_t refCount;
};

struct HOST_ISOFRAMESDESC {
	uint32_t length;
	uint32_t offset;
};

struct HOST_EP_PRIV {
	struct list_head node;
	struct HostEp *genericHwEp;
	struct HostEp *currentHwEp;
	uint8_t epIsReady;
	uint8_t isIn;
	uint8_t type;
	uint8_t interval;
	uint8_t epNum;
	uint8_t faddress;
	uint16_t maxPacketSize;
	uint32_t frame;
	uint8_t hubAddress;
	uint8_t portAddress;
	uint8_t split;
	struct HOST_EP *usbHEp;
	uint8_t isocEpConfigured;
	uint8_t transferFinished;
};

struct HOST_REQ {
	struct list_head list;
	struct HOST_EP *usbEp;
	void *userExt;
	void *hcPriv;
	struct HOST_DEVICE *usbDev;
	struct usb_ctrlrequest *setup;
	uintptr_t setupDma;
	void *bufAddress;
	uintptr_t buffDma;
	uint32_t buffLength;
	uint32_t actualLength;
	uint8_t epIsIn;
	uint8_t eptype;
	uint8_t epNum;
	uint8_t faddress;
	uint8_t interval;
	uint8_t status;
	uint8_t reqUnlinked;
	struct HOST_ISOFRAMESDESC *isoFramesDesc;
	uint32_t isoFramesNumber;
};

struct HOST_SYSREQ {
	uint32_t privDataSize;
	uint32_t trbMemSize;
};

struct HOST_EP_CFG {
	uint8_t bufferingValue;
	uint16_t startBuf;
	uint16_t maxPacketSize;
};

struct HOST_CFG {
	uintptr_t regBase;
	uintptr_t phy_regBase;
	struct HOST_EP_CFG epIN[16];
	struct HOST_EP_CFG epOUT[16];
	uint8_t dmultEnabled;
	uint8_t memoryAlignment;
	uint8_t dmaSupport;
	uint8_t isEmbeddedHost;
	void *trbAddr;
	uintptr_t trbDmaAddr;
};

struct HOST_CTRL;

struct HOST_CALLBACKS {
	void (*portStatusChange)(struct HOST_CTRL *priv);

	uint8_t (*getEpToggle)(struct HOST_CTRL *priv,
			struct HOST_DEVICE *usbDev, uint8_t epNum, uint8_t isIn);

	void (*setEpToggle)(struct HOST_CTRL *priv, struct HOST_DEVICE *usbDev,
			uint8_t epNum, uint8_t isIn, uint8_t toggle);

	void (*givebackRequest)(struct HOST_CTRL *priv,
			struct HOST_REQ *usbReq, uint32_t status);

	void (*setTimer)(struct HOST_CTRL *priv, uint32_t time, uint8_t id);
};

struct HOST_OBJ {
	int32_t (*host_probe)(struct HOST_CFG *config, struct HOST_SYSREQ *sysReq);

	int32_t (*host_init)(struct HOST_CTRL *priv, struct HOST_CFG *config,
			struct HOST_CALLBACKS *callbacks, struct device *pdev, bool isVhubHost);

	void (*host_destroy)(struct HOST_CTRL *priv);

	void (*host_start)(struct HOST_CTRL *priv);

	void (*host_stop)(struct HOST_CTRL *priv);

	void (*host_isr)(struct HOST_CTRL *priv);

	int32_t (*host_epDisable)(struct HOST_CTRL *priv, struct HOST_EP *ep);

	int32_t (*host_reqQueue)(struct HOST_CTRL *priv, struct HOST_REQ *req);

	int32_t (*host_reqDequeue)(struct HOST_CTRL *priv,
			struct HOST_REQ *req, uint32_t status);

	int32_t (*host_vhubStatusData)(struct HOST_CTRL *priv, uint8_t *status);

	int32_t (*host_vhubControl)(struct HOST_CTRL *priv,
			struct usb_ctrlrequest *setup, uint8_t *buff);

	int32_t (*host_getDevicePD)(struct HOST_CTRL *priv);

	int32_t (*host_epGetPrivateDataSize)(struct HOST_CTRL *priv);
};

struct HOST_CTRL {
	struct device *dev;
	struct HW_REGS	*regs;
	struct HOST_OBJ *hostDrv;
	struct HOST_CFG hostCfg;
	struct HOST_CALLBACKS hostCallbacks;
	struct HostEp in[16];
	struct HostEp out[16];
	uint32_t portStatus;
	struct list_head ctrlHEpQueue;
	struct list_head isoInHEpQueue[MAX_INSTANCE_EP_NUM];
	struct list_head isoOutHEpQueue[MAX_INSTANCE_EP_NUM];
	struct list_head intInHEpQueue[MAX_INSTANCE_EP_NUM];
	struct list_head intOutHEpQueue[MAX_INSTANCE_EP_NUM];
	struct list_head bulkInHEpQueue[MAX_INSTANCE_EP_NUM];
	struct list_head bulkOutHEpQueue[MAX_INSTANCE_EP_NUM];
	uint8_t hwEpInCount;
	uint8_t hwEpOutCount;
	unsigned int speed;
	enum HOST_OtgState otgState;
	enum HOST_EP0_STAGE ep0State;
	uint8_t vBusErrCnt;
	uint8_t isRemoteWakeup;
	uint8_t isSelfPowered;
	uint8_t deviceAddress;
	struct DMA_OBJ *dmaDrv;
	void *dmaController;
	struct DMA_CFG dmaCfg;
	struct DMA_CALLBACKS dmaCallback;
	uint8_t port_resetting;
	struct HOST_USB_DEVICE *host_devices_table[MAX_SUPPORTED_DEVICES];
	struct CUSTOM_REGS *custom_regs;
	struct VHUB_REGS *vhub_regs;
};

struct HOST_OBJ *HOST_GetInstance(void);

#endif
