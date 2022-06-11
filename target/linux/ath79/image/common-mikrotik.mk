define Device/mikrotik
	DEVICE_VENDOR := MikroTik
	KERNEL_NAME := vmlinuz
	KERNEL := kernel-bin | append-dtb-elf
	KERNEL_INITRAMFS := kernel-bin | append-dtb-elf
endef

define Device/mikrotik_nor
  $(Device/mikrotik)
  IMAGE/sysupgrade.bin := append-kernel | kernel2minor -s 1024 -e | \
	pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | \
	check-size | append-metadata
endef

define Device/mikrotik_nand
  $(Device/mikrotik)
  IMAGE/sysupgrade.bin = append-kernel | kernel2minor -s 2048 -e -c | \
	sysupgrade-tar kernel=$$$$@ | append-metadata
  DEVICE_PACKAGES := nand-utils
  DEFAULT := n
endef
