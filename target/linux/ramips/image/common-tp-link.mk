DEVICE_VARS += TPLINK_FLASHLAYOUT TPLINK_HWID TPLINK_HWREV TPLINK_HWREVADD
DEVICE_VARS += TPLINK_HVERSION TPLINK_BOARD_ID TPLINK_HEADER_VERSION

define Device/tplink-v1
  DEVICE_VENDOR := TP-Link
  TPLINK_FLASHLAYOUT :=
  TPLINK_HWID :=
  TPLINK_HWREV := 0x1
  TPLINK_HEADER_VERSION := 1
  KERNEL := $(KERNEL_DTB)
  KERNEL_INITRAMFS := $(KERNEL_DTB) | tplink-v1-header -e -O
  IMAGES += factory.bin
  IMAGE/factory.bin := tplink-v1-image factory -e -O
  IMAGE/sysupgrade.bin := tplink-v1-image sysupgrade -e -O | append-metadata | \
	check-size $$$$(IMAGE_SIZE)
endef

define Device/tplink-v2
  DEVICE_VENDOR := TP-Link
  TPLINK_FLASHLAYOUT :=
  TPLINK_HWID :=
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x0
  TPLINK_HVERSION := 3
  KERNEL := $(KERNEL_DTB)
  KERNEL_INITRAMFS := $(KERNEL_DTB) | tplink-v2-header -e
  IMAGES += factory.bin
  IMAGE/factory.bin := tplink-v2-image -e
  IMAGE/sysupgrade.bin := tplink-v2-image -s -e | append-metadata | \
	check-size $$$$(IMAGE_SIZE)
endef

define Device/tplink-safeloader
  DEVICE_VENDOR := TP-Link
  TPLINK_BOARD_ID :=
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0x0
  TPLINK_HEADER_VERSION := 1
  KERNEL := $(KERNEL_DTB) | tplink-v1-header -e -O
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef
