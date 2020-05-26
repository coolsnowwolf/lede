DEVICE_VARS += TPLINK_HWID TPLINK_HWREV TPLINK_FLASHLAYOUT TPLINK_HEADER_VERSION
DEVICE_VARS += TPLINK_BOARD_ID TPLINK_HWREVADD TPLINK_HVERSION

define Build/uImageArcher
	mkimage -A $(LINUX_KARCH) \
		-O linux -T kernel -C $(1) -a $(KERNEL_LOADADDR) \
		-e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-n '$(call toupper,$(LINUX_KARCH)) OpenWrt Linux-$(LINUX_VERSION)' -d $@ $@.new
	@mv $@.new $@
endef

define Device/tplink-v1
  DEVICE_VENDOR := TP-Link
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0x1
  TPLINK_HEADER_VERSION := 1
  LOADER_TYPE := gz
  KERNEL := kernel-bin | append-dtb | lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | tplink-v1-header
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := tplink-v1-image sysupgrade | append-metadata
  IMAGE/factory.bin := tplink-v1-image factory
endef

define Device/tplink-v2
  DEVICE_VENDOR := TP-Link
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0x1
  TPLINK_HWREVADD := 0x0
  TPLINK_HVERSION := 3
  KERNEL := kernel-bin | append-dtb | lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | tplink-v2-header
  IMAGE/sysupgrade.bin := tplink-v2-image -s | append-metadata | \
	check-size $$$$(IMAGE_SIZE)
endef

define Device/tplink-nolzma
  $(Device/tplink-v1)
  LOADER_FLASH_OFFS := 0x22000
  COMPILE := loader-$(1).gz
  COMPILE/loader-$(1).gz := loader-okli-compile
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma -M 0x4f4b4c49 | \
	loader-okli $(1) 7680
  KERNEL_INITRAMFS := kernel-bin | append-dtb | gzip | tplink-v1-header
endef

define Device/tplink-4m
  $(Device/tplink-nolzma)
  TPLINK_FLASHLAYOUT := 4M
  IMAGE_SIZE := 3904k
endef

define Device/tplink-4mlzma
  $(Device/tplink-v1)
  TPLINK_FLASHLAYOUT := 4Mlzma
  IMAGE_SIZE := 3904k
endef

define Device/tplink-8m
  $(Device/tplink-nolzma)
  TPLINK_FLASHLAYOUT := 8M
  IMAGE_SIZE := 8000k
endef

define Device/tplink-8mlzma
  $(Device/tplink-v1)
  TPLINK_FLASHLAYOUT := 8Mlzma
  IMAGE_SIZE := 8000k
endef

define Device/tplink-16mlzma
  $(Device/tplink-v1)
  TPLINK_FLASHLAYOUT := 16Mlzma
  IMAGE_SIZE := 16192k
endef

define Device/tplink-safeloader
  $(Device/tplink-v1)
  TPLINK_HWREV := 0x0
  KERNEL := kernel-bin | append-dtb | lzma | tplink-v1-header -O
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef

define Device/tplink-safeloader-uimage
  $(Device/tplink-safeloader)
  KERNEL := kernel-bin | append-dtb | lzma | uImageArcher lzma
endef

define Device/tplink-safeloader-okli
  $(Device/tplink-safeloader)
  LOADER_TYPE := elf
  LOADER_FLASH_OFFS := 0x43000
  COMPILE := loader-$(1).elf
  COMPILE/loader-$(1).elf := loader-okli-compile
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma -M 0x4f4b4c49 | \
	loader-okli $(1) 12288
endef
