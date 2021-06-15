# Copyright (c) 2014 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID
DEVICE_VARS += TPLINK_BOARD_ID

define Build/buffalo-rootfs-cksum
	( \
		echo -ne "\x$$(od -A n -t u1 $@ | tr -s ' ' '\n' | \
			$(STAGING_DIR_HOST)/bin/awk '{s+=$$0}END{printf "%x", 255-s%256}')"; \
	) >> $@
endef

define Build/edimax-header
	$(eval edimax_model=$(word 1,$(1)))

	$(STAGING_DIR_HOST)/bin/mkedimaximg \
	-b -s CSYS -m $(edimax_model) \
	-f 0x70000 -S 0x1200000 \
	-i $@ -o $@.new
	@mv $@.new $@
endef

define Device/Default
	PROFILES := Default
	KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
	KERNEL_INITRAMFS_PREFIX := $$(IMG_PREFIX)-$(1)-initramfs
	KERNEL_PREFIX := $$(IMAGE_PREFIX)
	KERNEL_LOADADDR = 0x42208000
	DEVICE_DTS = $$(SOC)-$(lastword $(subst _, ,$(1)))
	DEVICE_DTS_CONFIG := config@1
	IMAGES := sysupgrade.bin
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
	IMAGES += factory.img
	IMAGE/factory.img := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-uImage-fakehdr filesystem | pad-to $$$$(KERNEL_SIZE) | append-ubi | netgear-dni
	IMAGE/sysupgrade.bin := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-uImage-fakehdr filesystem | sysupgrade-tar kernel=$$$$@ | append-metadata
endef

define Device/TpSafeImage
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none
	KERNEL_NAME := zImage
	TPLINK_BOARD_ID :=
	IMAGES += factory.bin
	IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
	IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | append-metadata
endef

define Device/ZyXELImage
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none | pad-to $$(KERNEL_SIZE)
	KERNEL_NAME := zImage
	IMAGES += factory.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | pad-to $$$$(BLOCKSIZE) | zyxel-ras-image separate-kernel
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-to $$$$(BLOCKSIZE) | sysupgrade-tar rootfs=$$$$@ | append-metadata
endef

define Device/askey_rt4230w-rev6
	$(call Device/LegacyImage)
	DEVICE_VENDOR := Askey
	DEVICE_MODEL := RT4230W
	DEVICE_VARIANT := REV6
	SOC := qcom-ipq8065
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	KERNEL_IN_UBI := 1
	DEVICE_PACKAGES := ath10k-firmware-qca9984-ct
endef
TARGET_DEVICES += askey_rt4230w-rev6

define Device/asrock_g10
	$(call Device/FitImage)
	$(call Device/UbiFit)
	SOC := qcom-ipq8064
	DEVICE_VENDOR := ASRock
	DEVICE_MODEL := G10
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	KERNEL_SIZE := 5332k
	DEVICE_PACKAGES := kmod-i2c-gpio ath10k-firmware-qca99x0-ct
	IMAGE/nand-factory.bin := append-ubi | edimax-header RN67
endef
TARGET_DEVICES += asrock_g10

define Device/buffalo_wxr-2533dhp
	$(call Device/LegacyImage)
	SOC := qcom-ipq8064
	DEVICE_VENDOR := Buffalo
	DEVICE_MODEL := WXR-2533DHP
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGE_SIZE := 65536k
	KERNEL_IN_UBI := 1
	IMAGE/sysupgrade.bin := append-rootfs | buffalo-rootfs-cksum | \
		sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += buffalo_wxr-2533dhp

define Device/compex_wpq864
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Compex
	DEVICE_MODEL := WPQ864
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := qcom-ipq8064
	DEVICE_PACKAGES := kmod-gpio-beeper
endef
TARGET_DEVICES += compex_wpq864

define Device/edgecore_ecw5410
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Edgecore
	DEVICE_MODEL := ECW5410
	SOC := qcom-ipq8068
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@v2.0-ap160
	DEVICE_PACKAGES := ath10k-firmware-qca9984-ct ipq-wifi-edgecore_ecw5410
endef
TARGET_DEVICES += edgecore_ecw5410

define Device/linksys_ea7500-v1
	$(call Device/LegacyImage)
	DEVICE_VENDOR := Linksys
	DEVICE_MODEL := EA7500
	DEVICE_VARIANT := v1
	SOC := qcom-ipq8064
	PAGESIZE := 2048
	BLOCKSIZE := 128k
	KERNEL_SIZE := 3072k
	KERNEL = kernel-bin | append-dtb | uImage none | append-uImage-fakehdr filesystem
	UBINIZE_OPTS := -E 5
	IMAGES := factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | pad-to $$$$(PAGESIZE)
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += linksys_ea7500-v1

define Device/linksys_ea8500
	$(call Device/LegacyImage)
	DEVICE_VENDOR := Linksys
	DEVICE_MODEL := EA8500
	SOC := qcom-ipq8064
	PAGESIZE := 2048
	BLOCKSIZE := 128k
	KERNEL_SIZE := 3072k
	KERNEL = kernel-bin | append-dtb | uImage none | append-uImage-fakehdr filesystem
	BOARD_NAME := ea8500
	SUPPORTED_DEVICES += ea8500
	UBINIZE_OPTS := -E 5
	IMAGES += factory.bin
	IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += linksys_ea8500

define Device/nec_wg2600hp
	$(call Device/LegacyImage)
	DEVICE_VENDOR := NEC
	DEVICE_MODEL := Aterm WG2600HP
	SOC := qcom-ipq8064
	BLOCKSIZE := 64k
	BOARD_NAME := wg2600hp
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += nec_wg2600hp

define Device/nec_wg2600hp3
	$(call Device/LegacyImage)
	DEVICE_VENDOR := NEC Platforms
	DEVICE_MODEL := Aterm WG2600HP3
	SOC := qcom-ipq8062
	BLOCKSIZE := 64k
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := -kmod-ata-ahci -kmod-ata-ahci-platform -kmod-usb-ohci -kmod-usb2 \
		-kmod-usb-ledtrig-usbport -kmod-usb-phy-qcom-dwc3 -kmod-usb3 -kmod-usb-dwc3-qcom \
		ath10k-firmware-qca9984-ct ipq-wifi-nec_wg2600hp3
endef
TARGET_DEVICES += nec_wg2600hp3

define Device/netgear_d7800
	$(call Device/DniImage)
	DEVICE_VENDOR := NETGEAR
	DEVICE_MODEL := Nighthawk X4 D7800
	SOC := qcom-ipq8064
	KERNEL_SIZE := 4096k
	NETGEAR_BOARD_ID := D7800
	NETGEAR_HW_ID := 29764958+0+128+512+4x4+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := d7800
	SUPPORTED_DEVICES += d7800
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += netgear_d7800

define Device/netgear_r7500
	$(call Device/DniImage)
	DEVICE_VENDOR := NETGEAR
	DEVICE_MODEL := Nighthawk X4 R7500
	DEVICE_VARIANT := v1
	SOC := qcom-ipq8064
	KERNEL_SIZE := 4096k
	NETGEAR_BOARD_ID := R7500
	NETGEAR_HW_ID := 29764841+0+128+256+3x3+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7500
	SUPPORTED_DEVICES += r7500
	DEVICE_PACKAGES := ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += netgear_r7500

define Device/netgear_r7500v2
	$(call Device/DniImage)
	DEVICE_VENDOR := NETGEAR
	DEVICE_MODEL := Nighthawk X4 R7500
	DEVICE_VARIANT := v2
	SOC := qcom-ipq8064
	KERNEL_SIZE := 4096k
	NETGEAR_BOARD_ID := R7500v2
	NETGEAR_HW_ID := 29764958+0+128+512+3x3+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7500v2
	SUPPORTED_DEVICES += r7500v2
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct ath10k-firmware-qca988x-ct
endef
TARGET_DEVICES += netgear_r7500v2

define Device/netgear_r7800
	$(call Device/DniImage)
	DEVICE_VENDOR := NETGEAR
	DEVICE_MODEL := Nighthawk X4S R7800
	SOC := qcom-ipq8065
	KERNEL_SIZE := 4096k
	NETGEAR_BOARD_ID := R7800
	NETGEAR_HW_ID := 29764958+0+128+512+4x4+4x4+cascade
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7800
	SUPPORTED_DEVICES += r7800
	DEVICE_PACKAGES := ath10k-firmware-qca9984-ct
endef
TARGET_DEVICES += netgear_r7800

define Device/qcom_ipq8064-ap148
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Qualcomm
	DEVICE_MODEL := AP148
	DEVICE_VARIANT := standard
	SOC := qcom-ipq8064
	DEVICE_DTS := qcom-ipq8064-ap148
	KERNEL_INSTALL := 1
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap148
	SUPPORTED_DEVICES += ap148
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += qcom_ipq8064-ap148

define Device/qcom_ipq8064-ap148-legacy
	$(call Device/LegacyImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Qualcomm
	DEVICE_MODEL := AP148
	DEVICE_VARIANT := legacy
	SOC := qcom-ipq8064
	DEVICE_DTS := qcom-ipq8064-ap148
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap148
	SUPPORTED_DEVICES := qcom,ipq8064-ap148 ap148
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += qcom_ipq8064-ap148-legacy

define Device/qcom_ipq8064-ap161
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Qualcomm
	DEVICE_MODEL := AP161
	SOC := qcom-ipq8064
	DEVICE_DTS := qcom-ipq8064-ap161
	KERNEL_INSTALL := 1
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap161
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += qcom_ipq8064-ap161

define Device/qcom_ipq8064-db149
	$(call Device/FitImage)
	DEVICE_VENDOR := Qualcomm
	DEVICE_MODEL := DB149
	SOC := qcom-ipq8064
	DEVICE_DTS := qcom-ipq8064-db149
	KERNEL_INSTALL := 1
	BOARD_NAME := db149
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += qcom_ipq8064-db149

define Device/ruijie_rg-mtfi-m520
	DEVICE_VENDOR := Ruijie
	DEVICE_MODEL := RG-MTFi-M520
	SOC := qcom-ipq8064
	KERNEL_SIZE := 4096k
	BLOCKSIZE := 64k
	BOARD_NAME := rg-mtfi-m520
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none | pad-to $${KERNEL_SIZE}
	KERNEL_NAME := zImage
	IMAGES := factory.bin sysupgrade.bin mmcblk0p2-kernel.bin mmcblk0p3-rootfs.bin
	IMAGE/factory.bin := qsdk-ipq-factory-mmc
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-to $$$${BLOCKSIZE} | sysupgrade-tar rootfs=$$$$@ | append-metadata
	IMAGE/mmcblk0p2-kernel.bin := append-kernel | pad-to $$$${KERNEL_SIZE}
	IMAGE/mmcblk0p3-rootfs.bin := append-rootfs | pad-rootfs
	DEVICE_PACKAGES := ath10k-firmware-qca988x-ct e2fsprogs f2fs-tools \
	kmod-gpio-pca953x kmod-hwmon-lm75 kmod-fs-ext4 kmod-fs-f2fs kmod-rtc-pcf8563 losetup
endef
TARGET_DEVICES += ruijie_rg-mtfi-m520

define Device/tplink_ad7200
	$(call Device/TpSafeImage)
	DEVICE_VENDOR := TP-Link
	DEVICE_MODEL := AD7200
	DEVICE_VARIANT := v1/v2
	DEVICE_ALT0_VENDOR := TP-Link
	DEVICE_ALT0_MODEL := Talon AD7200
	DEVICE_ALT0_VARIANT := v1/v2
	SOC := qcom-ipq8064
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	TPLINK_BOARD_ID := AD7200
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct kmod-wil6210
endef
TARGET_DEVICES += tplink_ad7200

define Device/tplink_c2600
	$(call Device/TpSafeImage)
	DEVICE_VENDOR := TP-Link
	DEVICE_MODEL := Archer C2600
	DEVICE_VARIANT := v1
	SOC := qcom-ipq8064
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := c2600
	SUPPORTED_DEVICES += c2600
	TPLINK_BOARD_ID := C2600
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
endef
TARGET_DEVICES += tplink_c2600

define Device/tplink_vr2600v
	DEVICE_VENDOR := TP-Link
	DEVICE_MODEL := Archer VR2600v
	DEVICE_VARIANT := v1
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none
	KERNEL_NAME := zImage
	KERNEL_SIZE := 3072k
	SOC := qcom-ipq8064
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := vr2600v
	SUPPORTED_DEVICES += vr2600v
	DEVICE_PACKAGES := ath10k-firmware-qca99x0-ct
	IMAGE/sysupgrade.bin := pad-extra 512 | append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += tplink_vr2600v

define Device/ubnt_unifi-ac-hd
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Ubiquiti
	DEVICE_MODEL := UniFi AC HD
	SOC := qcom-ipq8064
	BLOCKSIZE := 64k
	IMAGE_SIZE := 14784k
	DEVICE_PACKAGES := ath10k-firmware-qca9984-ct
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | append-metadata | check-size
endef
TARGET_DEVICES += ubnt_unifi-ac-hd

define Device/zyxel_nbg6817
	DEVICE_VENDOR := ZyXEL
	DEVICE_MODEL := NBG6817
	SOC := qcom-ipq8065
	KERNEL_SIZE := 4096k
	BLOCKSIZE := 64k
	BOARD_NAME := nbg6817
	RAS_BOARD := NBG6817
	RAS_ROOTFS_SIZE := 20934k
	RAS_VERSION := "V1.99(OWRT.9999)C0"
	SUPPORTED_DEVICES += nbg6817
	DEVICE_PACKAGES := ath10k-firmware-qca9984-ct e2fsprogs kmod-fs-ext4 losetup
	$(call Device/ZyXELImage)
endef
TARGET_DEVICES += zyxel_nbg6817

$(eval $(call BuildImage))
