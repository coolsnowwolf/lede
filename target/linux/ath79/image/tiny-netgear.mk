include ./common-netgear.mk

define Device/netgear_ar7240
  ATH_SOC := ar7240
  NETGEAR_KERNEL_MAGIC := 0x32303631
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma -d20 | netgear-uImage lzma
  IMAGE_SIZE := 3904k
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | netgear-squashfs | append-rootfs | pad-rootfs
  $(Device/netgear_ath79)
endef

define Device/netgear_wnr612-v2
  $(Device/netgear_ar7240)
  DEVICE_TITLE := Netgear WNR612v2
  DEVICE_DTS := ar7240_netgear_wnr612-v2
  NETGEAR_BOARD_ID := REALWNR612V2
  SUPPORTED_DEVICES += wnr612-v2
endef
TARGET_DEVICES += netgear_wnr612-v2

define Device/on_n150r
  $(Device/netgear_ar7240)
  DEVICE_TITLE := ON Network N150R
  NETGEAR_BOARD_ID := N150R
  SUPPORTED_DEVICES += n150r
endef
TARGET_DEVICES += on_n150r
