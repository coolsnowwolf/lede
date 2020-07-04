
#ifndef __HIF_H__
#define __HIF_H__


#ifdef MT_MAC
#ifdef RTMP_MAC_PCI
#include "hif/mt_hif_pci.h"
#include "mac/mac_mt/mt_mac_pci.h"
#endif /* RTMP_MAC_PCI */


#endif /* MT_MAC */

enum {
	HIF_TX_IDX0,
	HIF_TX_IDX1,
	HIF_TX_IDX2,
	HIF_TX_IDX3,
	HIF_TX_IDX4,
	HIF_TX_IDX5,
	HIF_TX_IDX6,
	HIF_TX_IDX7,
	HIF_TX_IDX8,
	HIF_TX_IDX9,
	HIF_TX_IDX10,
	HIF_TX_IDX11,
	HIF_TX_IDX12,
	HIF_TX_IDX13,
	HIF_TX_IDX14,
	HIF_TX_IDX15,
};

enum {
	HIF_RX_IDX0,
	HIF_RX_IDX1,
};

typedef enum _RTMP_INF_TYPE_ {
	RTMP_DEV_INF_UNKNOWN = 0,
	RTMP_DEV_INF_PCI = 1,
	RTMP_DEV_INF_USB = 2,
	RTMP_DEV_INF_RBUS = 4,
	RTMP_DEV_INF_PCIE = 5,
	RTMP_DEV_INF_SDIO = 6,
} RTMP_INF_TYPE;

#define IS_SDIO_INF(_pAd)		((_pAd)->infType == RTMP_DEV_INF_SDIO)
#define IS_USB_INF(_pAd)		((_pAd)->infType == RTMP_DEV_INF_USB)
#define IS_USB3_INF(_pAd)		((IS_USB_INF(_pAd)) && ((_pAd)->BulkOutMaxPacketSize == 1024))
#define IS_PCIE_INF(_pAd)		((_pAd)->infType == RTMP_DEV_INF_PCIE)
#define IS_PCI_INF(_pAd)		(((_pAd)->infType == RTMP_DEV_INF_PCI) || IS_PCIE_INF(_pAd))
#define IS_PCI_ONLY_INF(_pAd)	((_pAd)->infType == RTMP_DEV_INF_PCI)
#define IS_RBUS_INF(_pAd) ((_pAd)->infType == RTMP_DEV_INF_RBUS)



#endif /* __HIF_H__ */

