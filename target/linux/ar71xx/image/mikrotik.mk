define Device/mikrotik
  PROFILES := Default
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport
  BOARD_NAME := routerboard
  LOADER_TYPE := elf
  KERNEL_NAME := vmlinux.bin.lzma
  KERNEL := kernel-bin | loader-kernel
  KERNEL_INITRAMFS_NAME := vmlinux-initramfs.bin.lzma
  MIKROTIK_CHUNKSIZE :=
  IMAGE/sysupgrade.bin/squashfs :=
endef
DEVICE_VARS += MIKROTIK_CHUNKSIZE

define Device/mikrotik-nand
  $(Device/mikrotik)
  IMAGE/sysupgrade.bin/squashfs = append-kernel | \
	kernel2minor -s $$(MIKROTIK_CHUNKSIZE) -e -c | sysupgrade-tar kernel=$$$$@
endef

define Device/nand-64m
  $(Device/mikrotik-nand)
  MIKROTIK_CHUNKSIZE := 512
  DEVICE_TITLE := MikroTik RouterBoard (64 MB NAND)
endef
TARGET_DEVICES += nand-64m

define Device/nand-large
  $(Device/mikrotik-nand)
  MIKROTIK_CHUNKSIZE := 2048
  DEVICE_TITLE := MikroTik RouterBoard (>= 128 MB NAND)
endef
TARGET_DEVICES += nand-large

define Device/nand-large-ac
  $(Device/mikrotik-nand)
  MIKROTIK_CHUNKSIZE := 2048
  DEVICE_TITLE := MikroTik RouterBoard (>= 128 MB NAND, 802.11ac)
  DEVICE_PACKAGES += kmod-ath10k-ct ath10k-firmware-qca988x-ct
  SUPPORTED_DEVICES := rb-921gs-5hpacd-r2 rb-922uags-5hpacd
endef
TARGET_DEVICES += nand-large-ac

define Device/rb-nor-flash-16M
  $(Device/mikrotik)
  DEVICE_TITLE := MikroTik RouterBoard (16 MB SPI NOR)
  DEVICE_PACKAGES := rssileds -nand-utils kmod-ledtrig-gpio
  IMAGE_SIZE := 16000k
  KERNEL_INSTALL := 1
  SUPPORTED_DEVICES := rb-750-r2 rb-750up-r2 rb-750p-pbr2 rb-911-2hn rb-911-5hn rb-931-2nd rb-941-2nd rb-951ui-2nd rb-952ui-5ac2nd rb-962uigs-5hact2hnt rb-lhg-5nd rb-map-2nd rb-mapl-2nd rb-wap-2nd rb-wapr-2nd rb-sxt-2nd-r3
  IMAGE/sysupgrade.bin := append-kernel | kernel2minor -s 1024 -e | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += rb-nor-flash-16M

define Device/rb-nor-flash-16M-ac
  $(Device/rb-nor-flash-16M)
  DEVICE_TITLE := MikroTik RouterBoard (16 MB SPI NOR, 802.11ac)
  DEVICE_PACKAGES += kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct ath10k-firmware-qca9887-ct kmod-usb-ehci
  SUPPORTED_DEVICES += rb-wapg-5hact2hnd
endef
TARGET_DEVICES += rb-nor-flash-16M-ac
