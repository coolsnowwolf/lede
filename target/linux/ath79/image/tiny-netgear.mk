include ./common-netgear.mk

define Device/netgear_ar7240
  SOC := ar7240
  NETGEAR_KERNEL_MAGIC := 0x32303631
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma -d20 | netgear-uImage lzma
  IMAGE_SIZE := 3712k
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | netgear-squashfs | \
	append-rootfs | pad-rootfs
  $(Device/netgear_ath79)
endef

define Device/netgear_wnr612-v2
  $(Device/netgear_ar7240)
  DEVICE_MODEL := WNR612
  DEVICE_VARIANT := v2
  DEVICE_DTS := ar7240_netgear_wnr612-v2
  NETGEAR_BOARD_ID := REALWNR612V2
  SUPPORTED_DEVICES += wnr612-v2
endef
TARGET_DEVICES += netgear_wnr612-v2

define Device/on_n150r
  $(Device/netgear_ar7240)
  DEVICE_VENDOR := On Networks
  DEVICE_MODEL := N150R
  NETGEAR_BOARD_ID := N150R
  SUPPORTED_DEVICES += n150r
endef
TARGET_DEVICES += on_n150r

define Device/netgear_wnr1000-v2
  SOC := ar7240
  DEVICE_MODEL := WNR1000
  DEVICE_VARIANT := v2
  NETGEAR_KERNEL_MAGIC := 0x31303031
  NETGEAR_BOARD_ID := WNR1000V2
  NETGEAR_HW_ID := 29763331+4+32
  IMAGE_SIZE := 3712k
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | netgear-squashfs | \
	append-rootfs | pad-rootfs
  SUPPORTED_DEVICES += wnr1000-v2
  $(Device/netgear_ath79)
endef
TARGET_DEVICES += netgear_wnr1000-v2

define Device/netgear_wnr2000-v3
  SOC := ar7241
  DEVICE_MODEL := WNR2000
  DEVICE_VARIANT := v3
  NETGEAR_KERNEL_MAGIC := 0x32303033
  NETGEAR_BOARD_ID := WNR2000V3
  NETGEAR_HW_ID := 29763551+04+32
  IMAGE_SIZE := 3712k
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | netgear-squashfs | \
	append-rootfs | pad-rootfs
  IMAGES += factory-NA.img
  IMAGE/factory-NA.img := $$(IMAGE/default) | netgear-dni NA | \
	check-size
  SUPPORTED_DEVICES += wnr2000-v3
  $(Device/netgear_ath79)
endef
TARGET_DEVICES += netgear_wnr2000-v3
