# SPDX-License-Identifier: GPL-2.0-only
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk
include $(TOPDIR)/target/linux/zte/sr1010.mk

# Linux 7.3's dtc no longer implements this legacy graph check.
DTC_FLAGS := $(filter-out -Wno-graph_child_address,$(DTC_FLAGS))

KERNEL_LOADADDR := 0x80200000
KERNEL_ENTRY := 0x80200000

define Build/sr1010-firmware
	$(STAGING_DIR_HOST)/bin/python3 \
		$(TOPDIR)/target/linux/zte/mkfw/sr1010-mkfw.py \
		"$(IMAGE_KERNEL)" "$(IMAGE_ROOTFS)" "$@" \
		--openssl "$(SR1010_OPENSSL)" \
		--header "$(SR1010_FIRMWARE_DIR)/sr1010-header.bin" \
		--uboot "$(SR1010_FIRMWARE_DIR)/sr1010-uboot.bin" \
		-v "$(VERSION_DIST) $(VERSION_NUMBER)" -p "ZXSLC SR1010"
endef

define Device/Default
  PROFILES := Default
  KERNEL_NAME := Image
  KERNEL = kernel-bin | lzma | fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  KERNEL_INITRAMFS = kernel-bin | gzip | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  KERNEL_SUFFIX := .itb
  KERNEL_INITRAMFS_SUFFIX := .itb
  DEVICE_DTS_DIR := $(DTS_DIR)/zte
  IMAGES :=
endef

define Device/zte_zxslc-sr1010
  DEVICE_VENDOR := ZTE
  DEVICE_MODEL := ZXSLC SR1010
  DEVICE_DTS := zx279133-sr1010-openwrt
  DEVICE_DTS_DIR := $(TOPDIR)/target/linux/zte/dts
  SUPPORTED_DEVICES := zte,zx279133-sr1010
  DEVICE_PACKAGES := zte-fwslot luci-app-zte-fwslot
  # Reuse Imm's scratch-buffer fix; generate no stock-8MiB variant.
  # The complete legacy container remains compatible with existing fwslot.
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := sr1010-firmware
endef
TARGET_DEVICES += zte_zxslc-sr1010

$(eval $(call BuildImage))
