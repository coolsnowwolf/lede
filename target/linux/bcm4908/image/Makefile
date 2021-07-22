# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

DEVICE_VARS += ASUS_PRODUCTID ASUS_BUILD_NO ASUS_FW_REV ASUS_EXT_NO
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_REGION

define Build/bcm4908asus
	$(STAGING_DIR_HOST)/bin/bcm4908asus create -i $@ \
		-p $(ASUS_PRODUCTID) -b $(ASUS_BUILD_NO) -f $(ASUS_FW_REV) \
		-e $(ASUS_EXT_NO)
endef

define Build/bcm4908img
	rm -fr $@-bootfs
	mkdir -p $@-bootfs
	cp -r $(DEVICE_NAME)/* $@-bootfs/
	touch $@-bootfs/1-openwrt
	cp $(DTS_DIR)/$(firstword $(DEVICE_DTS)).dtb $@-bootfs/94908.dtb
	cp $(KDIR)/bcm63xx-cfe/$(subst _,$(comma),$(DEVICE_NAME))/cferam.000 $@-bootfs/
	cp $(IMAGE_KERNEL) $@-bootfs/vmlinux.lz

	$(STAGING_DIR_HOST)/bin/mkfs.jffs2 --pad=0x800000 --little-endian --squash-uids \
		-v -e 128KiB -o $@-bootfs.jffs2 -d $@-bootfs -m none -n
	$(STAGING_DIR_HOST)/bin/bcm4908img create $@.new -f $@-bootfs.jffs2 \
		-a 0x20000 -f $@
	mv $@.new $@
endef

define Build/bcm4908kernel
	$(STAGING_DIR_HOST)/bin/bcm4908kernel -i $@ -o $@.new
	mv $@.new $@
endef

define Build/bcm4908lzma
	$(STAGING_DIR_HOST)/bin/lzma e -lc1 -lp2 -pb2 -d22 $@ $@.new
	mv $@.new $@
endef

define Device/Default
  KERNEL := kernel-bin | bcm4908lzma | bcm4908kernel
  KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL_INITRAMFS_SUFFIX := .bin
  KERNEL_INITRAMFS := kernel-bin | bcm4908lzma | bcm4908kernel
  FILESYSTEMS := squashfs
  KERNEL_NAME := Image
  DEVICE_IMG_NAME = $$(DEVICE_IMG_PREFIX)-$$(1).$$(2)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE/bin := append-ubi | bcm4908img
endef

define Device/asus_gt-ac5300
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := GT-AC5300
  DEVICE_DTS := broadcom/bcm4908/bcm4908-asus-gt-ac5300
  IMAGES := bin
  IMAGE/bin := append-ubi | bcm4908img | bcm4908asus
  ASUS_PRODUCTID := GT-AC5300
  ASUS_BUILD_NO := 384
  ASUS_FW_REV := 3.0.0.4
  ASUS_EXT_NO := 21140
endef
TARGET_DEVICES += asus_gt-ac5300

define Device/netgear_r8000p
  DEVICE_VENDOR := Netgear
  DEVICE_MODEL := R8000P
  DEVICE_DTS := broadcom/bcm4908/bcm4906-netgear-r8000p
  IMAGES := bin
  IMAGE/chk := append-ubi | bcm4908img | netgear-chk
  NETGEAR_BOARD_ID := U12H359T00_NETGEAR
  NETGEAR_REGION := 1
endef
TARGET_DEVICES += netgear_r8000p

define Device/tplink_archer-c2300-v1
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := Archer C2300
  DEVICE_VARIANT := v1
  DEVICE_DTS := broadcom/bcm4908/bcm4906-tplink-archer-c2300-v1
  IMAGES := bin
  IMAGE/bin := append-ubi | bcm4908img
  BROKEN := y
endef
TARGET_DEVICES += tplink_archer-c2300-v1

$(eval $(call BuildImage))
