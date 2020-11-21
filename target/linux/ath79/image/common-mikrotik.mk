define Device/mikrotik
	DEVICE_VENDOR := MikroTik
	LOADER_TYPE := elf
	KERNEL := kernel-bin | append-dtb | lzma | loader-kernel
	KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | loader-kernel
endef

define Device/mikrotik_nor
  $(Device/mikrotik)
  IMAGE/sysupgrade.bin := append-kernel | kernel2minor -s 1024 -e | \
	pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | \
	append-metadata | check-size
endef

define Device/mikrotik_nand
  $(Device/mikrotik)
  IMAGE/sysupgrade.bin = append-kernel | kernel2minor -s 2048 -e -c | \
	sysupgrade-tar kernel=$$$$@ | append-metadata
  DEVICE_PACKAGES := nand-utils
endef
