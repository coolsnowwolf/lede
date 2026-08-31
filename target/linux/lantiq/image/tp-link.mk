DEVICE_VARS += TPLINK_FLASHLAYOUT TPLINK_HWID TPLINK_HWREV TPLINK_HWREVADD TPLINK_HVERSION

define Device/dsa-migration
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_COMPAT_MESSAGE := Config cannot be migrated from swconfig to DSA
endef

define Device/lantiqTpLink
  DEVICE_VENDOR := TP-Link
  TPLINK_HWREVADD := 0
  TPLINK_HVERSION := 2
  KERNEL := kernel-bin | append-dtb | lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | \
	tplink-v2-header -s -V "ver. 1.0"
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := tplink-v2-image -s -V "ver. 1.0" | \
	check-size | append-metadata
endef

define Device/tplink_vr200
  $(Device/dsa-migration)
  $(Device/lantiqTpLink)
  DEVICE_MODEL := Archer VR200
  DEVICE_VARIANT := v1
  TPLINK_FLASHLAYOUT := 16Mltq
  TPLINK_HWID := 0x63e64801
  TPLINK_HWREV := 0x53
  IMAGE_SIZE := 15808k
  DEVICE_PACKAGES:= kmod-mt76x0e wpad-basic-mbedtls kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware
  SUPPORTED_DEVICES += VR200
endef
TARGET_DEVICES += tplink_vr200

define Device/tplink_vr200v
  $(Device/dsa-migration)
  $(Device/lantiqTpLink)
  DEVICE_MODEL := Archer VR200v
  DEVICE_VARIANT := v1
  TPLINK_FLASHLAYOUT := 16Mltq
  TPLINK_HWID := 0x73b70801
  TPLINK_HWREV := 0x2f
  IMAGE_SIZE := 15808k
  DEVICE_PACKAGES:= kmod-mt76x0e wpad-basic-mbedtls kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-tapi kmod-ltq-vmmc xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware
  SUPPORTED_DEVICES += VR200v
endef
TARGET_DEVICES += tplink_vr200v
