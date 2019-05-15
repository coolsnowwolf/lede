#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Image/Prepare
	rm -f $(KDIR)/fs_mark
	echo -ne '\xde\xad\xc0\xde' > $(KDIR)/fs_mark
	$(call prepare_generic_squashfs,$(KDIR)/fs_mark)

	# For UBI we want only one extra block
	rm -f $(KDIR)/ubi_mark
	echo -ne '\xde\xad\xc0\xde' > $(KDIR)/ubi_mark
endef

define Build/lzma-d16
	$(STAGING_DIR_HOST)/bin/lzma e $@ -d16 $(1) $@.new
	mv $@.new $@
endef

# Similar to Build/tplink-safeloader but uses TRX instead of clean kernel
define Build/bcm53xx-tplink-safeloader
	$(STAGING_DIR_HOST)/bin/trx \
		-o $@.trx \
		-m 33554432 \
		-f $(IMAGE_KERNEL) -a 1024
	$(STAGING_DIR_HOST)/bin/tplink-safeloader \
		-B $(TPLINK_BOARD) \
		-k $@.trx \
		-r $@ \
		-j \
		-o $@.new
	mv $@.new $@
	rm $@.trx
endef

define Build/buffalo-wzr-header
	$(eval product=$(word 1,$(1)))
	$(eval region=$(word 2,$(1)))
	( \
		echo $(product)_$(BUFFALO_TAG_VERSION)_$(BUFFALO_TAG_MINOR)_$(region)_$(BUFFALO_TAG_PLATFORM); \
		echo filelen=$$(stat -c%s $@); \
		cat $@ \
	) > $@.new
	mv $@.new $@
endef

# TRX with only one (kernel) partition
define Build/trx
	$(STAGING_DIR_HOST)/bin/trx \
		-o $@.new \
		-m 33554432 \
		-f $@
	mv $@.new $@
endef

define Build/trx-serial
	$(STAGING_DIR_HOST)/bin/otrx create $@.new \
		-f $(IMAGE_KERNEL) -a 1024 \
		-f $@ -a 0x10000 -A $(KDIR)/fs_mark
	mv $@.new $@
endef

define Build/trx-nand
	# kernel: always use 4 MiB (-28 B or TRX header) to allow upgrades even
	#	  if it grows up between releases
	# root: UBI with one extra block containing UBI mark to trigger erasing
	#	rest of partition
	$(STAGING_DIR_HOST)/bin/otrx create $@.new \
		-f $(IMAGE_KERNEL) -a 0x20000 -b 0x400000 \
		-f $@ \
		-A $(KDIR)/ubi_mark -a 0x20000
	mv $@.new $@
endef

define Build/asus-trx
	$(STAGING_DIR_HOST)/bin/asustrx \
		-p $(PRODUCTID) -i $@ -o $@.new
	mv $@.new $@
endef

define Build/seama-nand
	# Seama entity
	$(STAGING_DIR_HOST)/bin/oseama \
		entity $@.entity \
		-m "dev=/dev/mtdblock/7" \
		-m "type=firmware" \
		-f $(IMAGE_KERNEL) \
		-b 0x400000 \
		-f $@ \
		-f $(KDIR)/ubi_mark
	# Seama container
	$(STAGING_DIR_HOST)/bin/seama \
		-s $@ \
		-m "signature=$(SIGNATURE)" \
		-i $@.entity
endef

DEVICE_VARS += PRODUCTID SIGNATURE NETGEAR_BOARD_ID NETGEAR_REGION TPLINK_BOARD
DEVICE_VARS += BUFFALO_TAG_PLATFORM BUFFALO_TAG_VERSION BUFFALO_TAG_MINOR

IEEE8021X := wpad-basic
B43 := $(IEEE8021X) kmod-b43
BRCMFMAC_43602A1 := $(IEEE8021X) kmod-brcmfmac brcmfmac-firmware-43602a1-pcie
BRCMFMAC_4366B1 := $(IEEE8021X) kmod-brcmfmac brcmfmac-firmware-4366b1-pcie
BRCMFMAC_4366C0 := $(IEEE8021X) kmod-brcmfmac k3wifi
USB2_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-phy-bcm-ns-usb2
USB2_PACKAGES += kmod-usb-ledtrig-usbport
USB3_PACKAGES := $(USB2_PACKAGES) kmod-usb3 kmod-phy-bcm-ns-usb3

define Device/Default
  # .dtb files are prefixed by SoC type, e.g. bcm4708- which is not included in device/image names
  # extract the full dtb name based on the device info
  DEVICE_DTS := $(patsubst %.dtb,%,$(notdir $(wildcard $(if $(IB),$(KDIR),$(DTS_DIR))/*-$(1).dtb)))
  KERNEL := kernel-bin | append-dtb | lzma-d16
  KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL_INITRAMFS_SUFFIX := .trx
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma-d16 | trx
  FILESYSTEMS := squashfs
  KERNEL_NAME := zImage
  IMAGE_NAME = $$(IMAGE_PREFIX)-$$(1).$$(2)
  IMAGES := trx
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE/trx := append-ubi | trx-nand
endef

define Device/asus
  IMAGES := trx
  IMAGE/trx := append-ubi | trx-nand | asus-trx
endef

define Device/asus-rt-ac56u
  DEVICE_TITLE := Asus RT-AC56U
  DEVICE_PACKAGES := $(B43) $(USB3_PACKAGES)
endef
TARGET_DEVICES += asus-rt-ac56u

define Device/asus-rt-ac68u
  DEVICE_TITLE := Asus RT-AC68U
  DEVICE_PACKAGES := $(USB3_PACKAGES)
endef
TARGET_DEVICES += asus-rt-ac68u

define Device/asus-rt-ac87u
  DEVICE_TITLE := Asus RT-AC87U
  DEVICE_PACKAGES := $(USB3_PACKAGES)
endef
TARGET_DEVICES += asus-rt-ac87u

define Device/asus-rt-n18u
  DEVICE_TITLE := Asus RT-N18U
  DEVICE_PACKAGES := $(USB3_PACKAGES)
endef
TARGET_DEVICES += asus-rt-n18u

# Buffalo devices have TFTP recovery mode which can work nicely with initramfs
# kernels.
# We should have two initramfs images for Buffalo: plain initramfs kernel and
# TRX with initramfs kernel. It's not possible right now so let's just build
# plain initramfs kernel as it may be more useful.
define Device/buffalo/Default
  KERNEL_INITRAMFS_SUFFIX = $$(KERNEL_SUFFIX)
  KERNEL_INITRAMFS = $$(KERNEL)
endef

define Device/buffalo-wxr-1900dhp
  $(call Device/buffalo/Default)
  DEVICE_TITLE := Buffalo WXR-1900DHP
  DEVICE_PACKAGES := $(USB3_PACKAGES)
endef
TARGET_DEVICES += buffalo-wxr-1900dhp

define Device/buffalo-wzr-600dhp2
  $(call Device/buffalo/Default)
  DEVICE_TITLE := Buffalo WZR-600DHP2
  DEVICE_PACKAGES := $(B43) $(USB2_PACKAGES)
endef
TARGET_DEVICES += buffalo-wzr-600dhp2

define Device/buffalo-wzr-900dhp
  $(call Device/buffalo/Default)
  DEVICE_TITLE := Buffalo WZR-900DHP
  DEVICE_PACKAGES := $(B43) $(USB3_PACKAGES)
  BUFFALO_TAG_PLATFORM := bcm
  BUFFALO_TAG_VERSION := 9.99
  BUFFALO_TAG_MINOR := 9.99
  IMAGES += factory-DHP-EU.bin factory-DHP2-JP.bin
  IMAGE/factory-DHP-EU.bin := \
	append-ubi | trx-nand | buffalo-wzr-header WZR-900DHP EU | \
	buffalo-enc WZR-900DHP $$(BUFFALO_TAG_VERSION) | \
	buffalo-tag-dhp WZR-900DHP EU mlang20 | buffalo-enc-tag | \
	buffalo-dhp-image
  IMAGE/factory-DHP2-JP.bin := \
	append-ubi | trx-nand | buffalo-wzr-header WZR-900DHP2 JP | \
	buffalo-enc WZR-900DHP2 $$(BUFFALO_TAG_VERSION) | \
	buffalo-tag-dhp WZR-900DHP2 JP jp | buffalo-enc-tag | \
	buffalo-dhp-image
endef
TARGET_DEVICES += buffalo-wzr-900dhp

define Device/buffalo-wzr-1750dhp
  $(call Device/buffalo/Default)
  DEVICE_TITLE := Buffalo WZR-1750DHP
  DEVICE_PACKAGES := $(B43) $(USB3_PACKAGES)
endef
TARGET_DEVICES += buffalo-wzr-1750dhp

define Device/dlink
  IMAGES := bin
  IMAGE/bin := append-ubi | seama-nand
endef

define Device/dlink-dir-885l
  DEVICE_TITLE := D-Link DIR-885L
  DEVICE_PACKAGES := $(BRCMFMAC_4366B1) $(USB3_PACKAGES)
  $(Device/dlink)
  SIGNATURE := wrgac42_dlink.2015_dir885l
endef
TARGET_DEVICES += dlink-dir-885l

# Linksys devices are disabled due to problem with 2 TRX partitions
define Device/linksys-ea6300-v1
  DEVICE_TITLE := Linksys EA6300 V1
  DEVICE_PACKAGES := $(B43) $(USB3_PACKAGES)
endef
# TARGET_DEVICES += linksys-ea6300-v1

define Device/linksys-ea6500-v2
  DEVICE_TITLE := Linksys EA6500 V2
  DEVICE_PACKAGES := $(B43) $(USB3_PACKAGES)
endef
TARGET_DEVICES += linksys-ea6500-v2

define Device/linksys-ea9200
  DEVICE_TITLE := Linksys EA9200 V1
  DEVICE_PACKAGES := $(BRCMFMAC_43602A1) $(USB3_PACKAGES)
endef
# TARGET_DEVICES += linksys-ea9200

define Device/linksys-ea9500
  DEVICE_TITLE := Linksys EA9500
  DEVICE_PACKAGES := $(BRCMFMAC_4366C0) $(USB3_PACKAGES)
  DEVICE_DTS := bcm47094-linksys-panamera
endef
# TARGET_DEVICES += linksys-ea9500

define Device/netgear
  IMAGES := chk
  IMAGE/chk := append-ubi | trx-nand | netgear-chk
  NETGEAR_REGION := 1
endef

define Device/netgear-r6250
  DEVICE_TITLE := Netgear R6250
  DEVICE_PACKAGES := $(B43) $(USB3_PACKAGES)
  $(Device/netgear)
  NETGEAR_BOARD_ID := U12H245T00_NETGEAR
endef
TARGET_DEVICES += netgear-r6250

define Device/netgear-r6300-v2
  DEVICE_TITLE := Netgear R6300 V2
  DEVICE_PACKAGES := $(B43) $(USB3_PACKAGES)
  $(Device/netgear)
  NETGEAR_BOARD_ID := U12H240T00_NETGEAR
endef
TARGET_DEVICES += netgear-r6300-v2

define Device/netgear-r7000
  DEVICE_TITLE := Netgear R7000
  DEVICE_PACKAGES := $(USB3_PACKAGES)
  $(Device/netgear)
  NETGEAR_BOARD_ID := U12H270T00_NETGEAR
endef
TARGET_DEVICES += netgear-r7000

define Device/netgear-r7900
  DEVICE_TITLE := Netgear R7900
  DEVICE_PACKAGES := $(BRCMFMAC_43602A1) $(USB3_PACKAGES)
  $(Device/netgear)
  NETGEAR_BOARD_ID := U12H315T30_NETGEAR
endef
TARGET_DEVICES += netgear-r7900

define Device/netgear-r8000
  DEVICE_TITLE := Netgear R8000
  DEVICE_PACKAGES := $(BRCMFMAC_43602A1) $(USB3_PACKAGES)
  $(Device/netgear)
  NETGEAR_BOARD_ID := U12H315T00_NETGEAR
endef
TARGET_DEVICES += netgear-r8000

define Device/netgear-r8500
  DEVICE_TITLE := Netgear R8500
  DEVICE_PACKAGES := $(BRCMFMAC_4366B1) $(USB3_PACKAGES)
  $(Device/netgear)
  NETGEAR_BOARD_ID := U12H334T00_NETGEAR
endef
# TARGET_DEVICES += netgear-r8500

define Device/smartrg-sr400ac
  DEVICE_TITLE := SmartRG SR400ac
  DEVICE_PACKAGES := $(BRCMFMAC_43602A1) $(USB3_PACKAGES)
  IMAGES := trx
  IMAGE/trx := append-rootfs | trx-serial
  KERNEL_INITRAMFS_SUFFIX := .bin
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma-d16
endef
TARGET_DEVICES += smartrg-sr400ac

define Device/phicomm-k3
  DEVICE_TITLE := PHICOMM K3
  DEVICE_PACKAGES := $(BRCMFMAC_4366C0) $(USB3_PACKAGES) k3screenctrl
  IMAGES := trx
endef
TARGET_DEVICES += phicomm-k3

define Device/arris-sbr-ac1900p
  DEVICE_TITLE := ARRIS SBR-AC1900P
  DEVICE_PACKAGES := $(USB3_PACKAGES)
endef
TARGET_DEVICES += arris-sbr-ac1900p

define Device/arris-sbr-ac3200p
  DEVICE_TITLE := ARRIS SBR-AC3200P
  DEVICE_PACKAGES := $(BRCMFMAC_43602A1) $(USB3_PACKAGES)
endef
TARGET_DEVICES += arris-sbr-ac3200p

define Device/tenda-ac9
  DEVICE_TITLE := Tenda AC9
  DEVICE_PACKAGES := $(B43) $(USB2_PACKAGES)
  IMAGES := trx
  IMAGE/trx := append-rootfs | trx-serial
endef
#TARGET_DEVICES += tenda-ac9

define Device/tplink-archer-c5-v2
  DEVICE_TITLE := TP-LINK Archer C5 V2
  DEVICE_PACKAGES := $(B43) $(USB2_PACKAGES)
  IMAGES := bin
  IMAGE/bin := append-rootfs | bcm53xx-tplink-safeloader
  TPLINK_BOARD := ARCHER-C5-V2
endef
#TARGET_DEVICES += tplink-archer-c5-v2

define Device/tplink-archer-c9-v1
  DEVICE_TITLE := TP-LINK Archer C9 V1
  DEVICE_PACKAGES := $(USB3_PACKAGES)
  IMAGES := bin
  IMAGE/bin := append-rootfs | bcm53xx-tplink-safeloader
  TPLINK_BOARD := ARCHERC9
endef
#TARGET_DEVICES += tplink-archer-c9-v1

$(eval $(call BuildImage))
