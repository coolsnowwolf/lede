#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

DEVICE_VARS += TPLINK_HWID TPLINK_HWREV TPLINK_FLASHLAYOUT

define rootfs_align
$(patsubst %-256k,0x40000,$(patsubst %-128k,0x20000,$(patsubst %-64k,0x10000,$(patsubst squashfs%,0x4,$(patsubst root.%,%,$(1))))))
endef

define Build/copy-file
	cat "$(1)" > "$@"
endef

# combine kernel and rootfs into one image
# mktplinkfw <type> <optional extra arguments to mktplinkfw binary>
# <type> is "sysupgrade" or "factory"
#
# -a align the rootfs start on an <align> bytes boundary
# -j add jffs2 end-of-filesystem markers
# -s strip padding from end of the image
# -X reserve <size> bytes in the firmware image (hexval prefixed with 0x)
define Build/mktplinkfw
	-$(STAGING_DIR_HOST)/bin/mktplinkfw \
		-H $(TPLINK_HWID) -W $(TPLINK_HWREV) -F $(TPLINK_FLASHLAYOUT) -N OpenWrt -V $(REVISION) \
		-k $(IMAGE_KERNEL) \
		-r $@ \
		-o $@.new \
		-j -X 0x40000 \
		-a $(call rootfs_align,$(FILESYSTEM)) \
		$(wordlist 2,$(words $(1)),$(1)) \
		$(if $(findstring sysupgrade,$(word 1,$(1))),-s) && mv $@.new $@ || rm -f $@
endef

define Device/Default
  PROFILES := Default
  DEVICE_DTS := $(1)
  KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL_ENTRY := 0x00000000
  KERNEL_LOADADDR := 0x00000000
  KERNEL := kernel-bin
endef

ifeq ($(SUBTARGET),generic)

define Device/tl-wdr4900-v1
  DEVICE_TITLE := TP-Link TL-WDR4900
  TPLINK_HWID := 0x49000001
  TPLINK_HWREV := 1
  TPLINK_FLASHLAYOUT := 16Mppc
  KERNEL_SIZE := 2684k
  KERNEL_NAME := simpleImage.tl-wdr4900-v1
  KERNEL_INITRAMFS :=
  KERNEL := kernel-bin | uImage none
  KERNEL_ENTRY := 0x1000000
  KERNEL_LOADADDR := 0x1000000
  SUPPORTED_DEVICES:=tl-wdr4900-v1 tplink,tl-wdr4900-v1
  ARTIFACTS := fdt.bin
  ARTIFACT/fdt.bin := append-dtb
  IMAGES := fdt.bin factory.bin sysupgrade.bin
  IMAGE/sysupgrade.bin := append-rootfs | mktplinkfw sysupgrade | append-metadata
  IMAGE/factory.bin := append-rootfs | mktplinkfw factory
endef
TARGET_DEVICES += tl-wdr4900-v1

define Device/red-15w-rev1
  DEVICE_TITLE := Sophos RED 15w Rev.1
  # Original firmware uses a dedicated DTB-partition.
  # The bootloader however supports FIT-images.
  KERNEL = kernel-bin | gzip | fit gzip $(KDIR)/image-$$(DEVICE_DTS).dtb
  SUPPORTED_DEVICES := sophos,red-15w-rev1
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += red-15w-rev1

endif

ifeq ($(SUBTARGET),p1020)

define Device/hiveap-330
  DEVICE_TITLE := Aerohive HiveAP-330
  DEVICE_PACKAGES := kmod-tpm-i2c-atmel
  BLOCKSIZE := 128k
  KERNEL_NAME := zImage
  KERNEL_SIZE := 8m
  KERNEL_INITRAMFS := copy-file $(KDIR)/vmlinux-initramfs | uImage none
  SUPPORTED_DEVICES := aerohive,hiveap-330
  IMAGES := fdt.bin sysupgrade.bin
  IMAGE/fdt.bin := append-dtb
  IMAGE/sysupgrade.bin := append-dtb | pad-to 256k | check-size 256k | \
	append-uImage-fakehdr ramdisk | pad-to 256k | check-size 512k | \
	append-rootfs | pad-rootfs $$(BLOCKSIZE) | pad-to 41216k | check-size 41216k | \
	append-kernel | append-metadata
endef
TARGET_DEVICES += hiveap-330

define Device/panda
  DEVICE_TITLE := OCEDO Panda
  DEVICE_PACKAGES := kmod-rtc-ds1307 uboot-envtools
  KERNEL = kernel-bin | gzip | fit gzip $(KDIR)/image-$$(DEVICE_DTS).dtb
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  BLOCKSIZE := 128k
  SUPPORTED_DEVICES := ocedo,panda
  IMAGES := fdt.bin sysupgrade.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/fdt.bin := append-dtb
endef
TARGET_DEVICES += panda

endif

ifeq ($(SUBTARGET),p2020)

define Device/p2020rdb
  DEVICE_TITLE := Freescale P2020RDB
  DEVICE_DTS_DIR := $(DTS_DIR)/fsl
  DEVICE_PACKAGES := kmod-hwmon-core kmod-hwmon-lm90 kmod-rtc-ds1307 \
	kmod-gpio-pca953x kmod-eeprom-at24
  BLOCKSIZE := 128k
  KERNEL := kernel-bin | gzip | \
	fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  SUPPORTED_DEVICES := fsl,P2020RDB
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
	pad-rootfs $$(BLOCKSIZE) | append-metadata
endef
TARGET_DEVICES += p2020rdb

endif

$(eval $(call BuildImage))
