#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

ITB_BOARDS = traverse-five64

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

define Build/traverse-fit
	./mkits-multiple-config.sh -o $@.its -A $(LINUX_KARCH) -v $(LINUX_VERSION) \
		-k $@ -a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-C gzip -c 1 -c 2 \
		-d $(DEVICE_DTS_DIR)/freescale/traverse-ls1043s.dtb -D "Traverse_LS1043S" -n "ls1043s" -a $(FDT_LOADADDR) -c 1 \
		-d $(DEVICE_DTS_DIR)/freescale/traverse-ls1043v.dtb -D "Traverse_LS1043V" -n "ls1043v" -a $(FDT_LOADADDR) -c 2
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its $@.new
	@mv -f $@.new $@
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
  FDT_LOADADDR = 0x90000000
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
  FILESYSTEMS := ubifs
  UBIFS_OPTS := -m 1 -e 262016 -c 128
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 256KiB
  PAGESIZE := 1
  IMAGE/firmware.bin =	append-ls-rcw $(1) | pad-to 1M | \
			append-ls-uboot $(1) | pad-to 4M | \
			append-ls-ppa $(1) | pad-to 9M | \
			append-ls-fman $(1) | pad-to 15M | \
			append-ls-dtb $$(DEVICE_DTS) | pad-to 16M | \
			append-kernel | pad-to 32M | \
			append-ubi | check-size 67108865
endef
TARGET_DEVICES += ls1046ardb

define Device/ls1012ardb
  DEVICE_TITLE := ls1012ardb-$(SUBTARGET)
  DEVICE_PACKAGES +=	rcw-layerscape-ls1012ardb uboot-layerscape-$(SUBTARGET)-ls1012ardb \
			kmod-ppfe layerscape-ppfe layerscape-ppa-ls1012ardb
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1012a-rdb
  FILESYSTEMS := ubifs
  UBIFS_OPTS := -m 1 -e 262016 -c 128
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 256KiB
  PAGESIZE := 1
  IMAGE/firmware.bin =	append-ls-rcw $(1) | pad-to 1M | \
			append-ls-uboot $(1) | pad-to 4M | \
			append-ls-ppa $(1) | pad-to 10M | \
			append-ls-ppfe | pad-to 15M | \
			append-ls-dtb $$(DEVICE_DTS) | pad-to 16M | \
			append-kernel | pad-to 32M | \
			append-ubi | check-size 67108865
endef
TARGET_DEVICES += ls1012ardb

define Device/ls1012afrdm
  DEVICE_TITLE := ls1012afrdm-$(SUBTARGET)
  DEVICE_PACKAGES +=	rcw-layerscape-ls1012afrdm uboot-layerscape-$(SUBTARGET)-ls1012afrdm \
			kmod-ppfe layerscape-ppfe layerscape-ppa-ls1012afrdm
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1012a-frdm
  FILESYSTEMS := ubifs
  UBIFS_OPTS := -m 1 -e 262016 -c 128
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 256KiB
  PAGESIZE := 1
  IMAGE/firmware.bin =	append-ls-rcw $(1) | pad-to 1M | \
			append-ls-uboot $(1) | pad-to 4M | \
			append-ls-ppa $(1) | pad-to 10M | \
			append-ls-ppfe | pad-to 15M | \
			append-ls-dtb $$(DEVICE_DTS) | pad-to 16M | \
			append-kernel | pad-to 32M | \
			append-ubi | check-size 67108865
endef
TARGET_DEVICES += ls1012afrdm

ifeq ($(SUBTARGET),armv8_64b)
define Device/ls1088ardb
  DEVICE_TITLE := ls1088ardb-$(SUBTARGET)
  DEVICE_PACKAGES +=	rcw-layerscape-ls1088ardb uboot-layerscape-$(SUBTARGET)-ls1088ardb \
			layerscape-mc-ls1088ardb layerscape-dpl-ls1088ardb restool \
			layerscape-ppa-ls1088ardb
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1088a-rdb
  FILESYSTEMS := ubifs
  UBIFS_OPTS := -m 1 -e 262016 -c 128
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 256KiB
  PAGESIZE := 1
  IMAGE/firmware.bin =	append-ls-rcw $(1) | pad-to 1M | \
			append-ls-uboot $(1) | pad-to 4M | \
			append-ls-ppa $(1) | pad-to 10M | \
			append-ls-mc $(1) | pad-to 13M | \
			append-ls-dpl $(1) | pad-to 14M | \
			append-ls-dpc $(1) | pad-to 15M | \
			append-ls-dtb $$(DEVICE_DTS) | pad-to 16M | \
			append-kernel | pad-to 32M | \
			append-ubi | check-size 67108865
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

define Device/traverse-five64
  KERNEL_NAME := Image
  KERNEL_SUFFIX := -kernel.itb
  KERNEL_INSTALL := 1
  FILESYSTEMS := ubifs
  DEVICE_TITLE := Traverse LS1043 Boards (Five64, LS1043S)
  DEVICE_PACKAGES += fman-layerscape-ls1043ardb kmod-i2c-core kmod-rtc-isl1208 uboot-envtools \
	uboot-traverse-ls1043v uboot-traverse-ls1043v-sdcard \
	kmod-hwmon-core kmod-hwmon-ltc2990 kmod-gpio-pca953x kmod-input-gpio-keys-polled \
	kmod-i2c-mux-pca954x kmod-hwmon-pac1934 kmod-hwmon-emc17xx
  DEVICE_DESCRIPTION = Build images for Traverse LS1043 boards. This generates a single image \
  	capable of booting on any of the boards in this family.
  DEVICE_DTS = freescale/traverse-ls1043s
  DEVICE_DTS_DIR = $(LINUX_DIR)/arch/arm64/boot/dts
  DEVICE_DTS_CONFIG = ls1043s
  KERNEL := kernel-bin | gzip | traverse-fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb $$(FDT_LOADADDR)
  KERNEL_INITRAMFS := kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb $$(FDT_LOADADDR)
  IMAGES = root sysupgrade.tar
  IMAGE/root = append-rootfs
  IMAGE/sysupgrade.tar = sysupgrade-tar
  UBIFS_OPTS := -m 2048 -e 124KiB -c 4096
endef
TARGET_DEVICES += traverse-five64

$(eval $(call BuildImage))
