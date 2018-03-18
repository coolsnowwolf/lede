DEVICE_VARS += TPLINK_FLASHLAYOUT TPLINK_HWID TPLINK_HWREV TPLINK_HWREVADD TPLINK_HVERSION

define Device/lantiqTpLink
  TPLINK_HWREVADD := 0
  TPLINK_HVERSION := 2
  KERNEL := kernel-bin | append-dtb | lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | \
	tplink-v2-header -s -V "ver. 1.0"
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := tplink-v2-image -s -V "ver. 1.0" | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
endef

define Device/tplink_tdw8970
  $(Device/lantiqTpLink)
  DEVICE_DTS := TDW8970
  TPLINK_FLASHLAYOUT := 8Mltq
  TPLINK_HWID := 0x89700001
  TPLINK_HWREV := 1
  IMAGE_SIZE := 7680k
  DEVICE_TITLE := TP-LINK TD-W8970
  DEVICE_PACKAGES:= kmod-ath9k wpad-mini kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += TDW8970
endef
TARGET_DEVICES += tplink_tdw8970

define Device/tplink_tdw8980
  $(Device/lantiqTpLink)
  DEVICE_DTS := TDW8980
  TPLINK_FLASHLAYOUT := 8Mltq
  TPLINK_HWID := 0x89800001
  TPLINK_HWREV := 14
  IMAGE_SIZE := 7680k
  DEVICE_TITLE := TP-LINK TD-W8980
  DEVICE_PACKAGES:= kmod-ath9k kmod-owl-loader wpad-mini kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += TDW8980
endef
TARGET_DEVICES += tplink_tdw8980

define Device/tplink_vr200v
  $(Device/lantiqTpLink)
  DEVICE_DTS := VR200v
  TPLINK_BOARD_ID := ArcherVR200V
  TPLINK_FLASHLAYOUT := 16Mltq
  TPLINK_HWID := 0x73b70801
  TPLINK_HWREV := 0x2f
  IMAGE_SIZE := 15808k
  DEVICE_TITLE := TP-LINK Archer VR200v
  DEVICE_PACKAGES:= kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += VR200v
endef
TARGET_DEVICES += tplink_vr200v
