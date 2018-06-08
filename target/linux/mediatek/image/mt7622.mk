define Device/MTK-RFB1
  DEVICE_TITLE := MTK7622 rfb1 AP 
  DEVICE_DTS := mt7622-rfb1
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mt7622
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb3 \
			kmod-ata-core kmod-ata-ahci-mtk 
endef
TARGET_DEVICES += MTK-RFB1
