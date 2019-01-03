include ./common-netgear.mk


define Device/netgear_wnr612-v2
  ATH_SOC := ar7240
  DEVICE_TITLE := Netgear WNR612v2
  DEVICE_DTS := ar7240_netgear_wnr612-v2
  NETGEAR_KERNEL_MAGIC := 0x32303631
  KERNEL := kernel-bin | append-dtb | lzma -d20 | netgear-uImage lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma -d20 | netgear-uImage lzma
  NETGEAR_BOARD_ID := REALWNR612V2
  IMAGE_SIZE := 3904k
  IMAGES += factory.img
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | netgear-squashfs | append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.img := $$(IMAGE/default) | netgear-dni | check-size $$$$(IMAGE_SIZE)
  SUPPORTED_DEVICES += wnr612-v2
endef
TARGET_DEVICES += netgear_wnr612-v2

define Device/on_n150r
  ATH_SOC := ar7240
  DEVICE_TITLE := ON Network N150R
  NETGEAR_KERNEL_MAGIC := 0x32303631
  KERNEL := kernel-bin | append-dtb | lzma -d20 | netgear-uImage lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma -d20 | netgear-uImage lzma
  NETGEAR_BOARD_ID := N150R
  IMAGE_SIZE := 3904k
  IMAGES += factory.img
  IMAGE/default := append-kernel | pad-to $$$$(BLOCKSIZE) | netgear-squashfs | append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.img := $$(IMAGE/default) | netgear-dni | check-size $$$$(IMAGE_SIZE)
  SUPPORTED_DEVICES += n150r
endef
TARGET_DEVICES += on_n150r
