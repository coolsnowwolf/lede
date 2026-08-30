define Device/mikrotik
	DEVICE_VENDOR := MikroTik
	LOADER_TYPE := elf
	KERNEL_NAME := vmlinuz
	KERNEL := kernel-bin | append-dtb-elf
	KERNEL_INITRAMFS_NAME := vmlinux-initramfs
	KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | loader-kernel
endef

define Device/mikrotik_nor
  $(Device/mikrotik)
  DEVICE_PACKAGES := -yafut
  IMAGE/sysupgrade.bin := append-kernel | yaffs-filesystem -M | \
	pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | \
	check-size | append-metadata
  IMAGES += sysupgrade-v7.bin
  IMAGE/sysupgrade-v7.bin := append-kernel | kernel-pack-npk | \
	  yaffs-filesystem -M | pad-to $$$$(BLOCKSIZE) | \
	  append-rootfs | pad-rootfs | check-size | append-metadata
endef

define Device/mikrotik_nand
  $(Device/mikrotik)
  IMAGE/sysupgrade.bin = append-kernel | sysupgrade-tar | append-metadata
  DEVICE_COMPAT_MESSAGE := \
       NAND images switched to yafut. If running older image, reinstall from initramfs.
  DEVICE_COMPAT_VERSION := 1.1

endef
