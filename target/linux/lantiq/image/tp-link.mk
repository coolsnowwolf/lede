DEVICE_VARS += TPLINK_FLASHLAYOUT TPLINK_HWID TPLINK_HWREV TPLINK_HWREVADD TPLINK_HVERSION

define Device/lantiqTpLink
  DEVICE_VENDOR := TP-Link
  TPLINK_HWREVADD := 0
  TPLINK_HVERSION := 2
  KERNEL := kernel-bin | append-dtb | lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | \
	tplink-v2-header -s -V "ver. 1.0"
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := tplink-v2-image -s -V "ver. 1.0" | \
	append-metadata | check-size
endef

define Device/tplink_tdw8970
  $(Device/lantiqTpLink)
  DEVICE_MODEL := TD-W8970
  DEVICE_VARIANT := v1
  TPLINK_FLASHLAYOUT := 8Mltq
  TPLINK_HWID := 0x89700001
  TPLINK_HWREV := 1
  IMAGE_SIZE := 7680k
  DEVICE_PACKAGES:= kmod-ath9k wpad-basic kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += TDW8970
endef
TARGET_DEVICES += tplink_tdw8970

define Device/tplink_tdw8980
  $(Device/lantiqTpLink)
  DEVICE_MODEL := TD-W8980
  DEVICE_VARIANT := v1
  TPLINK_FLASHLAYOUT := 8Mltq
  TPLINK_HWID := 0x89800001
  TPLINK_HWREV := 14
  IMAGE_SIZE := 7680k
  DEVICE_PACKAGES:= kmod-ath9k kmod-owl-loader wpad-basic kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += TDW8980
endef
TARGET_DEVICES += tplink_tdw8980

define Device/tplink_vr200
  $(Device/lantiqTpLink)
  DEVICE_MODEL := Archer VR200
  DEVICE_VARIANT := v1
  TPLINK_FLASHLAYOUT := 16Mltq
  TPLINK_HWID := 0x63e64801
  TPLINK_HWREV := 0x53
  IMAGE_SIZE := 15808k
  DEVICE_PACKAGES:= kmod-mt76x0e wpad-basic kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += VR200
endef
TARGET_DEVICES += tplink_vr200

define Device/tplink_vr200v
  $(Device/lantiqTpLink)
  DEVICE_MODEL := Archer VR200v
  DEVICE_VARIANT := v1
  TPLINK_FLASHLAYOUT := 16Mltq
  TPLINK_HWID := 0x73b70801
  TPLINK_HWREV := 0x2f
  IMAGE_SIZE := 15808k
  DEVICE_PACKAGES:= kmod-mt76x0e wpad-basic kmod-usb-dwc2 kmod-usb-ledtrig-usbport kmod-ltq-tapi kmod-ltq-vmmc
  SUPPORTED_DEVICES += VR200v
endef
TARGET_DEVICES += tplink_vr200v
