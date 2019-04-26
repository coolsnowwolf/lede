# Copyright (c) 2014 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Build/buffalo-rootfs-cksum
	( \
		echo -ne "\x$$(od -A n -t u1 $@ | tr -s ' ' '\n' | \
			$(STAGING_DIR_HOST)/bin/awk '{s+=$$0}END{printf "%x", 255-s%256}')"; \
	) >> $@
endef

define Device/Default
	PROFILES := Default
	KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
	KERNEL_INITRAMFS_PREFIX := $$(IMG_PREFIX)-$(1)-initramfs
	KERNEL_PREFIX := $$(IMAGE_PREFIX)
	KERNEL_LOADADDR = 0x42208000
	SUPPORTED_DEVICES := $(subst _,$(comma),$(1))
	IMAGE/sysupgrade.bin = sysupgrade-tar | append-metadata
	IMAGE/sysupgrade.bin/squashfs :=
endef

define Device/LegacyImage
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none
	KERNEL_NAME := zImage
endef

define Device/FitImage
	KERNEL_SUFFIX := -fit-uImage.itb
	KERNEL = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL_NAME := Image
endef

define Device/FitImageLzma
	KERNEL_SUFFIX := -fit-uImage.itb
	KERNEL = kernel-bin | lzma | fit lzma $$(DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL_NAME := Image
endef

define Device/UbiFit
	KERNEL_IN_UBI := 1
	IMAGES := nand-factory.bin nand-sysupgrade.bin
	IMAGE/nand-factory.bin := append-ubi
	IMAGE/nand-sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/DniImage
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none
	KERNEL_NAME := zImage
	NETGEAR_BOARD_ID :=
	NETGEAR_HW_ID :=
	UBINIZE_OPTS := -E 5
	IMAGES := factory.img sysupgrade.bin
	IMAGE/factory.img := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-uImage-fakehdr filesystem | pad-to $$$$(KERNEL_SIZE) | append-ubi | netgear-dni
	IMAGE/sysupgrade.bin := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-uImage-fakehdr filesystem | sysupgrade-tar kernel=$$$$@ | append-metadata
endef
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID

define Device/TpSafeImage
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none
	KERNEL_NAME := zImage
	TPLINK_BOARD_ID :=
	IMAGES := factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
	IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | append-metadata
endef
DEVICE_VARS += TPLINK_BOARD_ID

define Device/ZyXELImage
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none | pad-to $${KERNEL_SIZE}
	KERNEL_NAME := zImage
	IMAGES := factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | pad-to $$$$(BLOCKSIZE) | zyxel-ras-image separate-kernel
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-to $$$${BLOCKSIZE} | sysupgrade-tar rootfs=$$$$@ | append-metadata
endef

define Device/buffalo_wxr-2533dhp
	$(call Device/LegacyImage)
	DEVICE_DTS := qcom-ipq8064-wxr-2533dhp
	DEVICE_TITLE := Buffalo WXR-2533DHP
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGE_SIZE := 65536k
	KERNEL_IN_UBI := 1
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-rootfs | buffalo-rootfs-cksum | \
		sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += buffalo_wxr-2533dhp

define Device/compex_wpq864
	$(call Device/FitImage)
	$(call Device/UbiFit)
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS := qcom-ipq8064-wpq864
	DEVICE_TITLE := Compex WPQ864
	DEVICE_PACKAGES := kmod-gpio-beeper
endef
TARGET_DEVICES += compex_wpq864

define Device/linksys_ea8500
	$(call Device/LegacyImage)
	DEVICE_DTS := qcom-ipq8064-ea8500
	PAGESIZE := 2048
	BLOCKSIZE := 128k
	KERNEL_SIZE := 3072k
	KERNEL = kernel-bin | append-dtb | uImage none | append-uImage-fakehdr filesystem
	BOARD_NAME := ea8500
	SUPPORTED_DEVICES += ea8500
	UBINIZE_OPTS := -E 5
	IMAGES := factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-kernel | pad-to $$$${KERNEL_SIZE} | append-ubi
	DEVICE_TITLE := Linksys EA8500
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += linksys_ea8500

define Device/nec_wg2600hp
	$(call Device/LegacyImage)
	DEVICE_DTS := qcom-ipq8064-wg2600hp
	BLOCKSIZE := 64k
	BOARD_NAME := wg2600hp
	DEVICE_TITLE := NEC Aterm WG2600HP
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += nec_wg2600hp

define Device/netgear_d7800
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8064-d7800
	KERNEL_SIZE := 4096k
	NETGEAR_BOARD_ID := D7800
	NETGEAR_HW_ID := 29764958+0+128+512+4x4+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := d7800
	SUPPORTED_DEVICES += d7800
	DEVICE_TITLE := Netgear Nighthawk X4 D7800
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += netgear_d7800

define Device/netgear_r7500
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8064-r7500
	KERNEL_SIZE := 4096k
	NETGEAR_BOARD_ID := R7500
	NETGEAR_HW_ID := 29764841+0+128+256+3x3+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7500
	SUPPORTED_DEVICES += r7500
	DEVICE_TITLE := Netgear Nighthawk X4 R7500
	DEVICE_PACKAGES := ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += netgear_r7500

define Device/netgear_r7500v2
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8064-r7500v2
	KERNEL_SIZE := 4096k
	NETGEAR_BOARD_ID := R7500v2
	NETGEAR_HW_ID := 29764958+0+128+512+3x3+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7500v2
	SUPPORTED_DEVICES += r7500v2
	DEVICE_TITLE := Netgear Nighthawk X4 R7500v2
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += netgear_r7500v2

define Device/netgear_r7800
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8065-r7800
	KERNEL_SIZE := 4096k
	NETGEAR_BOARD_ID := R7800
	NETGEAR_HW_ID := 29764958+0+128+512+4x4+4x4+cascade
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7800
	SUPPORTED_DEVICES += r7800
	DEVICE_TITLE := Netgear Nighthawk X4S R7800
	DEVICE_PACKAGES := ath10k-firmware-qca9984-ct
endef
TARGET_DEVICES += netgear_r7800

define Device/qcom_ipq8064-ap148
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_DTS := qcom-ipq8064-ap148
	KERNEL_INSTALL := 1
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap148
	SUPPORTED_DEVICES += ap148
	DEVICE_TITLE := Qualcomm AP148
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += qcom_ipq8064-ap148

define Device/qcom_ipq8064-ap148-legacy
	$(call Device/LegacyImage)
	$(call Device/UbiFit)
	DEVICE_DTS := qcom-ipq8064-ap148
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap148
	SUPPORTED_DEVICES := qcom,ipq8064-ap148 ap148
	DEVICE_TITLE := Qualcomm AP148 (legacy)
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += qcom_ipq8064-ap148-legacy

define Device/qcom_ipq8064-ap161
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_DTS := qcom-ipq8064-ap161
	KERNEL_INSTALL := 1
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap161
	DEVICE_TITLE := Qualcomm AP161
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += qcom_ipq8064-ap161

define Device/qcom_ipq8064-db149
	$(call Device/FitImage)
	DEVICE_DTS := qcom-ipq8064-db149
	KERNEL_INSTALL := 1
	BOARD_NAME := db149
	DEVICE_TITLE := Qualcomm DB149
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += qcom_ipq8064-db149

define Device/tplink_c2600
	$(call Device/TpSafeImage)
	DEVICE_DTS := qcom-ipq8064-c2600
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := c2600
	SUPPORTED_DEVICES += c2600
	TPLINK_BOARD_ID := C2600
	DEVICE_TITLE := TP-Link Archer C2600
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += tplink_c2600

define Device/tplink_vr2600v
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none
	KERNEL_NAME := zImage
	KERNEL_SIZE := 3072k
	DEVICE_DTS := qcom-ipq8064-vr2600v
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := vr2600v
	SUPPORTED_DEVICES += vr2600v
	DEVICE_TITLE := TP-Link Archer VR2600v
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := pad-extra 512 | append-kernel | pad-to $$$${KERNEL_SIZE} | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += tplink_vr2600v

define Device/zyxel_nbg6817
	DEVICE_DTS := qcom-ipq8065-nbg6817
	KERNEL_SIZE := 4096k
	BLOCKSIZE := 64k
	BOARD_NAME := nbg6817
	RAS_BOARD := NBG6817
	RAS_ROOTFS_SIZE := 20934k
	RAS_VERSION := "V1.99(OWRT.9999)C0"
	SUPPORTED_DEVICES += nbg6817
	DEVICE_TITLE := ZyXEL NBG6817
	DEVICE_PACKAGES := ath10k-firmware-qca9984-ct e2fsprogs kmod-fs-ext4 losetup
	$(call Device/ZyXELImage)
endef
TARGET_DEVICES += zyxel_nbg6817

$(eval $(call BuildImage))
