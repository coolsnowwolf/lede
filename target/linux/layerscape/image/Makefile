#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Build/append-ls-rcw
	rm -f $@
	dd if=$(STAGING_DIR_IMAGE)/$(1)-rcw.bin >> $@
endef

define Build/append-ls-uboot
	dd if=$(STAGING_DIR_IMAGE)/$(1)-$(SUBTARGET)-uboot.bin >> $@
endef

define Build/append-ls-ppa
	dd if=$(STAGING_DIR_IMAGE)/$(1)-ppa.itb >> $@
endef

define Build/append-ls-fman
	dd if=$(STAGING_DIR_IMAGE)/$(1)-fman.bin >> $@
endef

define Build/append-ls-mc
	dd if=$(STAGING_DIR_IMAGE)/$(1)-mc.itb >> $@
endef

define Build/append-ls-ppfe
	dd if=$(STAGING_DIR_IMAGE)/pfe.itb >> $@
endef

define Build/append-ls-dpl
	dd if=$(STAGING_DIR_IMAGE)/$(1)-dpl.dtb >> $@
endef

define Build/append-ls-dpc
	dd if=$(STAGING_DIR_IMAGE)/$(1)-dpc.dtb >> $@
endef

define Build/append-ls-dtb
	$(call Image/BuildDTB,$(DTS_DIR)/$(1).dts,$(DTS_DIR)/$(1).dtb)
	dd if=$(DTS_DIR)/$(1).dtb >> $@
endef

define Build/append-ls-rootfs-ext4
	$(STAGING_DIR_HOST)/bin/make_ext4fs -l $(word 2,$(1)) -b 4096 -i 6000 -m 0 -J $(KDIR)/$(word 1,$(1))-$(word 2,$(1)).root.ext4 $(TARGET_DIR)
	dd if=$(KDIR)/$(word 1,$(1))-$(word 2,$(1)).root.ext4 >> $@
endef

define Device/Default
  PROFILES = Default
  FILESYSTEMS := squashfs
  DEVICE_DTS :=
  IMAGES = firmware.bin

ifeq ($(SUBTARGET),armv8_64b)
  KERNEL := kernel-bin | gzip | uImage gzip
  KERNEL_LOADADDR = 0x80080000
  KERNEL_ENTRY_POINT = 0x80080000
endif
ifeq ($(SUBTARGET),armv8_32b)
  KERNEL := kernel-bin | uImage none
  KERNEL_NAME := zImage
  KERNEL_LOADADDR = 0x80008000
  KERNEL_ENTRY_POINT = 0x80008000
endif
endef

define Device/ls1043ardb
  DEVICE_TITLE := ls1043ardb-$(SUBTARGET)
  DEVICE_PACKAGES +=	rcw-layerscape-ls1043ardb uboot-layerscape-$(SUBTARGET)-ls1043ardb \
			fman-layerscape-ls1043ardb layerscape-ppa-ls1043ardb
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1043a-rdb-sdk
  IMAGE/firmware.bin =	append-ls-rcw $(1) | pad-to 1M | \
			append-ls-uboot $(1) | pad-to 4M | \
			append-ls-ppa $(1) | pad-to 9M | \
			append-ls-fman $(1) | pad-to 15M | \
			append-ls-dtb $$(DEVICE_DTS) | pad-to 16M | \
			append-kernel | pad-to 32M | \
			append-rootfs | pad-rootfs | check-size 67108865
endef
TARGET_DEVICES += ls1043ardb

define Device/ls1046ardb
  DEVICE_TITLE := ls1046ardb-$(SUBTARGET)
  DEVICE_PACKAGES +=	rcw-layerscape-ls1046ardb uboot-layerscape-$(SUBTARGET)-ls1046ardb \
			fman-layerscape-ls1046ardb layerscape-ppa-ls1046ardb
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1046a-rdb-sdk
  IMAGE/firmware.bin =	append-ls-rcw $(1) | pad-to 1M | \
			append-ls-uboot $(1) | pad-to 4M | \
			append-ls-ppa $(1) | pad-to 9M | \
			append-ls-fman $(1) | pad-to 15M | \
			append-ls-dtb $$(DEVICE_DTS) | pad-to 16M | \
			append-kernel | pad-to 32M | \
			append-ls-rootfs-ext4 $(1) 30M | check-size 67108865
endef
TARGET_DEVICES += ls1046ardb

define Device/ls1012ardb
  DEVICE_TITLE := ls1012ardb-$(SUBTARGET)
  DEVICE_PACKAGES +=	rcw-layerscape-ls1012ardb uboot-layerscape-$(SUBTARGET)-ls1012ardb \
			kmod-ppfe layerscape-ppfe layerscape-ppa-ls1012ardb
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1012a-rdb
  IMAGE/firmware.bin =	append-ls-rcw $(1) | pad-to 1M | \
			append-ls-uboot $(1) | pad-to 4M | \
			append-ls-ppa $(1) | pad-to 10M | \
			append-ls-ppfe | pad-to 15M | \
			append-ls-dtb $$(DEVICE_DTS) | pad-to 16M | \
			append-kernel | pad-to 32M | \
			append-ls-rootfs-ext4 $(1) 30M | check-size 67108865
endef
TARGET_DEVICES += ls1012ardb

define Device/ls1012afrdm
  DEVICE_TITLE := ls1012afrdm-$(SUBTARGET)
  DEVICE_PACKAGES +=	rcw-layerscape-ls1012afrdm uboot-layerscape-$(SUBTARGET)-ls1012afrdm \
			kmod-ppfe layerscape-ppfe layerscape-ppa-ls1012afrdm
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1012a-frdm
  IMAGE/firmware.bin =	append-ls-rcw $(1) | pad-to 1M | \
			append-ls-uboot $(1) | pad-to 4M | \
			append-ls-ppa $(1) | pad-to 10M | \
			append-ls-ppfe | pad-to 15M | \
			append-ls-dtb $$(DEVICE_DTS) | pad-to 16M | \
			append-kernel | pad-to 32M | \
			append-ls-rootfs-ext4 $(1) 30M | check-size 67108865
endef
TARGET_DEVICES += ls1012afrdm

ifeq ($(SUBTARGET),armv8_64b)
define Device/ls1088ardb
  DEVICE_TITLE := ls1088ardb-$(SUBTARGET)
  DEVICE_PACKAGES +=	rcw-layerscape-ls1088ardb uboot-layerscape-$(SUBTARGET)-ls1088ardb \
			layerscape-mc-ls1088ardb layerscape-dpl-ls1088ardb restool \
			layerscape-ppa-ls1088ardb
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1088a-rdb
  IMAGE/firmware.bin =	append-ls-rcw $(1) | pad-to 1M | \
			append-ls-uboot $(1) | pad-to 4M | \
			append-ls-ppa $(1) | pad-to 10M | \
			append-ls-mc $(1) | pad-to 13M | \
			append-ls-dpl $(1) | pad-to 14M | \
			append-ls-dpc $(1) | pad-to 15M | \
			append-ls-dtb $$(DEVICE_DTS) | pad-to 16M | \
			append-kernel | pad-to 32M | \
			append-ls-rootfs-ext4 $(1) 30M | check-size 67108865
endef
TARGET_DEVICES += ls1088ardb

define Device/ls2088ardb
  DEVICE_TITLE := ls2088ardb-$(SUBTARGET)
  DEVICE_PACKAGES +=	rcw-layerscape-ls2088ardb uboot-layerscape-$(SUBTARGET)-ls2088ardb \
			layerscape-mc-ls2088ardb layerscape-dpl-ls2088ardb restool \
			layerscape-ppa-ls2088ardb
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls2088a-rdb
  IMAGE/firmware.bin =	append-ls-rcw $(1) | pad-to 1M | \
			append-ls-uboot $(1) | pad-to 4M | \
			append-ls-ppa $(1) | pad-to 10M | \
			append-ls-mc $(1) | pad-to 13M | \
			append-ls-dpl $(1) | pad-to 14M | \
			append-ls-dpc $(1) | pad-to 15M | \
			append-ls-dtb $$(DEVICE_DTS) | pad-to 16M | \
			append-kernel | pad-to 32M | \
			append-rootfs | pad-rootfs | check-size 67108865
endef
TARGET_DEVICES += ls2088ardb
endif

$(eval $(call BuildImage))
