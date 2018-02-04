#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

DEVICE_VARS += TPLINK_HWID TPLINK_HWREV TPLINK_FLASHLAYOUT

define Device/Default
  PROFILES := Default
  DEVICE_DTS := $(1)
  KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL_ENTRY := 0x00000000
  KERNEL_LOADADDR := 0x00000000
  KERNEL := kernel-bin
endef

define Build/append-uImage-fakeramdisk-hdr
	rm -f $@.fakeramdisk
	$(STAGING_DIR_HOST)/bin/mkimage \
		-A $(LINUX_KARCH) -O linux -T ramdisk -C none \
		-n 'fakeramdisk' \
		-s \
		$@.fakeramdisk
	cat $@.fakeramdisk >> $@
endef

define Build/tplink-v1-image
	$(STAGING_DIR_HOST)/bin/mktplinkfw \
		-H $(TPLINK_HWID) -W $(TPLINK_HWREV) -F $(TPLINK_FLASHLAYOUT) \
		-N "$(VERSION_DIST)" -V $(REVISION) \
		-k $(IMAGE_KERNEL) \
		-r $(IMAGE_ROOTFS) \
		-j \
		$(1) \
		-o $@
endef

ifeq ($(SUBTARGET),generic)

define Device/tl-wdr4900-v1
  DEVICE_TITLE := TP-Link TL-WDR4900
  DEVICE_PACKAGES := kmod-ath9k wpad-mini
  TPLINK_HWID := 0x49000001
  TPLINK_HWREV := 1
  TPLINK_FLASHLAYOUT := 16Mppc
  KERNEL_NAME := cuImage.tl-wdr4900-v1
  SUPPORTED_DEVICES:=tl-wdr4900-v1 tplink,tl-wdr4900-v1
  IMAGES := fdt.bin factory.bin sysupgrade.bin
  IMAGE/fdt.bin := append-dtb
  IMAGE/factory.bin := tplink-v1-image
  IMAGE/sysupgrade.bin := tplink-v1-image -s | append-metadata
endef
TARGET_DEVICES += tl-wdr4900-v1

endif

ifeq ($(SUBTARGET),p1020)

define Device/hiveap-330
  DEVICE_TITLE := Aerohive HiveAP-330
  DEVICE_PACKAGES := kmod-ath9k wpad-mini kmod-tpm-i2c-atmel
  BLOCKSIZE := 128k
  KERNEL_NAME := zImage
  KERNEL_SIZE := 8m
  SUPPORTED_DEVICES := aerohive,hiveap-330
  IMAGES := fdt.bin sysupgrade.bin
  IMAGE/fdt.bin := append-dtb
  IMAGE/sysupgrade.bin := append-dtb | pad-to 256k | check-size 256k | \
	append-uImage-fakeramdisk-hdr | pad-to 256k | check-size 512k | \
	append-rootfs | pad-rootfs $$(BLOCKSIZE) | pad-to 41216k | check-size 41216k | \
	append-kernel | append-metadata
endef
TARGET_DEVICES += hiveap-330

endif

$(eval $(call BuildImage))
