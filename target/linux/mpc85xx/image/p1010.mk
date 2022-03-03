DEVICE_VARS += TPLINK_HWID TPLINK_HWREV TPLINK_FLASHLAYOUT TPLINK_HEADER_VERSION

define Device/tplink_tl-wdr4900-v1
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := TL-WDR4900
  DEVICE_VARIANT := v1
  TPLINK_HEADER_VERSION := 1
  TPLINK_HWID := 0x49000001
  TPLINK_HWREV := 1
  TPLINK_FLASHLAYOUT := 16Mppc
  KERNEL_SIZE := 2684k
  KERNEL_NAME := simpleImage.tl-wdr4900-v1
  KERNEL_INITRAMFS :=
  KERNEL := kernel-bin | uImage none
  KERNEL_ENTRY := 0x1000000
  KERNEL_LOADADDR := 0x1000000
  SUPPORTED_DEVICES += tl-wdr4900-v1
  ARTIFACTS := fdt.bin
  ARTIFACT/fdt.bin := append-dtb
  IMAGES := factory.bin sysupgrade.bin
  IMAGE/sysupgrade.bin := tplink-v1-image sysupgrade | append-metadata
  IMAGE/factory.bin := tplink-v1-image factory
endef
TARGET_DEVICES += tplink_tl-wdr4900-v1

define Device/sophos_red-15w-rev1
  DEVICE_VENDOR := Sophos
  DEVICE_MODEL := RED 15w
  DEVICE_VARIANT := Rev.1
  # Original firmware uses a dedicated DTB-partition.
  # The bootloader however supports FIT-images.
  KERNEL = kernel-bin | gzip | fit gzip $(KDIR)/image-$$(DEVICE_DTS).dtb
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += sophos_red-15w-rev1
