define Device/MTK-RFB1
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7622 rfb1 AP
  DEVICE_DTS := mt7622-rfb1
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb3 \
			kmod-ata-core kmod-ata-ahci-mtk
endef
TARGET_DEVICES += MTK-RFB1

define Device/MTK-LYNX-RFB1
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7622 Lynx rfb1 AP
  DEVICE_DTS := mt7622-lynx-rfb1
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb3 \
			kmod-ata-core kmod-ata-ahci-mtk
endef
TARGET_DEVICES += MTK-LYNX-RFB1

define Device/BPI-R64
  DEVICE_VENDOR := LeMaker
  DEVICE_MODEL := Banana Pi R64
  DEVICE_DTS := mt7622-bananapi-bpi-r64
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb3 \
			kmod-ata-core kmod-ata-ahci-mtk
endef
TARGET_DEVICES += BPI-R64
