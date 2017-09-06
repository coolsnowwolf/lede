define Device/mikrotik
  PROFILES := Default
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport
  BOARD_NAME := routerboard
  KERNEL_INITRAMFS :=
  KERNEL_NAME := loader-generic.elf
  KERNEL := kernel-bin | kernel2minor -s 2048 -e -c
  FILESYSTEMS := squashfs
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar
endef

define Device/nand-64m
  $(Device/mikrotik)
  DEVICE_TITLE := MikroTik RouterBoard (64 MB NAND)
  KERNEL := kernel-bin | kernel2minor -s 512 -e -c
endef

define Device/nand-large
  $(Device/mikrotik)
  DEVICE_TITLE := MikroTik RouterBoard (>= 128 MB NAND)
  KERNEL := kernel-bin | kernel2minor -s 2048 -e -c
endef
TARGET_DEVICES += nand-64m nand-large

define Device/rb-nor-flash-16M
  DEVICE_TITLE := MikroTik RouterBoard (16 MB SPI NOR)
  DEVICE_PACKAGES := rbcfg rssileds -nand-utils
  IMAGE_SIZE := 16000k
  LOADER_TYPE := elf
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin | lzma | loader-kernel
  SUPPORTED_DEVICES := rb-750-r2 rb-750up-r2 rb-941-2nd rb-951ui-2nd rb-952ui-5ac2nd rb-962uigs-5hact2hnt rb-lhg-5nd rb-mapl-2nd
  IMAGE/sysupgrade.bin := append-kernel | kernel2minor -s 1024 -e | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef

define Device/rb-nor-flash-16M-ac
  $(Device/rb-nor-flash-16M)
  DEVICE_TITLE := MikroTik RouterBoard (16 MB SPI NOR, 802.11ac)
  DEVICE_PACKAGES += kmod-ath10k ath10k-firmware-qca988x ath10k-firmware-qca9887
endef
TARGET_DEVICES += rb-nor-flash-16M rb-nor-flash-16M-ac
