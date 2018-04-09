# Copyright (c) 2014 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

UBIFS_OPTS = -m 2048 -e 124KiB -c 4096 -U -F

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
	IMAGES := nand-factory.ubi nand-sysupgrade.bin
	IMAGE/nand-factory.ubi := append-ubi
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
	IMAGE/factory.img := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-uImage-fakeroot-hdr | pad-to $$$$(KERNEL_SIZE) | append-ubi | netgear-dni
	IMAGE/sysupgrade.bin := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-uImage-fakeroot-hdr | sysupgrade-tar kernel=$$$$@ | append-metadata
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
	IMAGES := sysupgrade.bin mmcblk0p5-rootfs.bin mmcblk0p4-kernel.bin
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-to $$$${BLOCKSIZE} | sysupgrade-tar rootfs=$$$$@ | append-metadata
	IMAGE/mmcblk0p5-rootfs.bin := append-rootfs | pad-rootfs | pad-to $$$${BLOCKSIZE}
	IMAGE/mmcblk0p4-kernel.bin := append-kernel
endef

define Device/avm_fritzbox-4040
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4019-fritz4040
	KERNEL_LOADADDR := 0x80208000
	BLOCKSIZE := 4k
	PAGESIZE := 256
	BOARD_NAME := fritz4040
	DEVICE_TITLE := AVM Fritz!Box 4040
	IMAGE_SIZE := 29753344
	IMAGES = sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := ipq-wifi-avm_fritzbox-4040 fritz-tffs fritz-caldata u-boot-fritz4040
endef
TARGET_DEVICES += avm_fritzbox-4040

define Device/linksys_ea8500
	$(call Device/LegacyImage)
	DEVICE_DTS := qcom-ipq8064-ea8500
	PAGESIZE := 2048
	BLOCKSIZE := 128k
	KERNEL_SIZE := 3072k
	KERNEL = kernel-bin | append-dtb | uImage none | append-uImage-fakeroot-hdr
	BOARD_NAME := ea8500
	SUPPORTED_DEVICES += ea8500
	UBINIZE_OPTS := -E 5
	IMAGES := factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-kernel | pad-to $$$${KERNEL_SIZE} | append-ubi
	DEVICE_TITLE := Linksys EA8500
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
endef
TARGET_DEVICES += linksys_ea8500

define Device/netgear_d7800
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8064-d7800
	KERNEL_SIZE := 2097152
	NETGEAR_BOARD_ID := D7800
	NETGEAR_HW_ID := 29764958+0+128+512+4x4+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := d7800
	SUPPORTED_DEVICES += d7800
	DEVICE_TITLE := Netgear Nighthawk X4 D7800
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
endef
TARGET_DEVICES += netgear_d7800

define Device/netgear_r7500
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8064-r7500
	KERNEL_SIZE := 2097152
	NETGEAR_BOARD_ID := R7500
	NETGEAR_HW_ID := 29764841+0+128+256+3x3+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7500
	SUPPORTED_DEVICES += r7500
	DEVICE_TITLE := Netgear Nighthawk X4 R7500
	DEVICE_PACKAGES := ath10k-firmware-qca988x
endef
TARGET_DEVICES += netgear_r7500

define Device/netgear_r7500v2
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8064-r7500v2
	KERNEL_SIZE := 2097152
	NETGEAR_BOARD_ID := R7500v2
	NETGEAR_HW_ID := 29764958+0+128+512+3x3+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7500v2
	SUPPORTED_DEVICES += r7500v2
	DEVICE_TITLE := Netgear Nighthawk X4 R7500v2
	DEVICE_PACKAGES := ath10k-firmware-qca99x0 ath10k-firmware-qca988x
endef
TARGET_DEVICES += netgear_r7500v2

define Device/netgear_r7800
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8065-r7800
	KERNEL_SIZE := 2097152
	NETGEAR_BOARD_ID := R7800
	NETGEAR_HW_ID := 29764958+0+128+512+4x4+4x4+cascade
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7800
	SUPPORTED_DEVICES += r7800
	DEVICE_TITLE := Netgear Nighthawk X4S R7800
	DEVICE_PACKAGES := ath10k-firmware-qca9984
endef
TARGET_DEVICES += netgear_r7800

define Build/copy-file
	cat "$(1)" > "$@"
endef

define Device/asus_rt-ac58u
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4019-rt-ac58u
	KERNEL_LOADADDR := 0x80208000
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DTB_SIZE := 65536
	SUPPORTED_DEVICES += rt-ac58u
	DEVICE_TITLE := Asus RT-AC58U
	BOARD_NAME := rt-ac58u
	IMAGE_SIZE := 20439364
	FILESYSTEMS := squashfs
#	Someone - in their infinite wisdom - decided to put the firmware
#	version in front of the image name \03\00\00\04 => Version 3.0.0.4
#	Since u-boot works with strings we either need another fixup step
#	to add a version... or we are very careful not to add '\0' into that
#	string and call it a day.... Yeah, we do the latter!
	UIMAGE_NAME:=$(shell echo -e '\03\01\01\01RT-AC58U')
	IMAGES = sysupgrade.tar flash-factory.trx
	IMAGE/flash-factory.trx := copy-file $(KDIR)/tmp/$$(KERNEL_INITRAMFS_IMAGE) | uImage none
	IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
	DEVICE_PACKAGES := kmod-usb-phy-qcom-ipq4019 ipq-wifi-rt-ac58u
endef
TARGET_DEVICES += asus_rt-ac58u

define Device/asus_rt-acrh17
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4019-rt-acrh17
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DTB_SIZE := 65536
	BOARD_NAME := rt-acrh17
	KERNEL_LOADADDR := 0x80208000
	DEVICE_TITLE := Asus RT-ACRH17
	IMAGE_SIZE := 20439364
	FILESYSTEMS := squashfs
	UIMAGE_NAME:=$(shell echo -e '\03\01\01\01RT-AC82U')
	IMAGES = sysupgrade.tar flash-factory.trx
	IMAGE/flash-factory.trx := copy-file $(KDIR)/tmp/$$(KERNEL_INITRAMFS_IMAGE) | uImage none
	IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca4019 ath10k-firmware-qca9984
endef
TARGET_DEVICES += asus_rt-acrh17

define Device/openmesh_a42
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4019-a42
	KERNEL_LOADADDR := 0x80208000
	BLOCKSIZE := 64k
	SUPPORTED_DEVICES := openmesh,a42
	DEVICE_TITLE := OpenMesh A42
	KERNEL = kernel-bin | lzma | fit lzma $$(DTS_DIR)/$$(DEVICE_DTS).dtb | pad-to $$(BLOCKSIZE)
	IMAGE_SIZE := 15616k
	IMAGES = factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | openmesh-image ce_type=A42
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-rootfs | sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca4019 uboot-envtools ipq-wifi-openmesh_a42
endef
TARGET_DEVICES += openmesh_a42

define Device/qcom_ap-dk01.1-c1
	DEVICE_TITLE := QCA AP-DK01.1-C1
	BOARD_NAME := ap-dk01.1-c1
	DEVICE_DTS := qcom-ipq4019-ap.dk01.1-c1
	KERNEL_LOADADDR := 0x80208000
	KERNEL_INSTALL := 1
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 26624k
	$(call Device/FitImage)
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$${KERNEL_SIZE} | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca4019
endef
TARGET_DEVICES += qcom_ap-dk01.1-c1

define Device/qcom_ap-dk04.1-c1
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_DTS := qcom-ipq4019-ap.dk04.1-c1
	KERNEL_LOADADDR := 0x80208000
	KERNEL_INSTALL := 1
	KERNEL_SIZE := 4048k
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap-dk04.1-c1
	DEVICE_TITLE := QCA AP-DK04.1-C1
endef
TARGET_DEVICES += qcom_ap-dk04.1-c1

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
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
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
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
endef
TARGET_DEVICES += qcom_ipq8064-ap148-legacy

define Device/qcom_ipq8064-db149
	$(call Device/FitImage)
	DEVICE_DTS := qcom-ipq8064-db149
	KERNEL_INSTALL := 1
	BOARD_NAME := db149
	DEVICE_TITLE := Qualcomm DB149
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
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
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
endef
TARGET_DEVICES += tplink_c2600

define Device/tplink_vr2600v
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none
	KERNEL_NAME := zImage
	KERNEL_SIZE := 2097152
	DEVICE_DTS := qcom-ipq8064-vr2600v
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := vr2600v
	SUPPORTED_DEVICES += vr2600v
	DEVICE_TITLE := TP-Link Archer VR2600v
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := pad-extra 512 | append-kernel | pad-to $$$${KERNEL_SIZE} | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += tplink_vr2600v

define Device/zyxel_nbg6817
	DEVICE_DTS := qcom-ipq8065-nbg6817
	KERNEL_SIZE := 4096k
	BLOCKSIZE := 64k
	BOARD_NAME := nbg6817
	SUPPORTED_DEVICES += nbg6817
	DEVICE_TITLE := ZyXEL NBG6817
	DEVICE_PACKAGES := ath10k-firmware-qca9984 e2fsprogs kmod-fs-ext4 losetup
	$(call Device/ZyXELImage)
endef
TARGET_DEVICES += zyxel_nbg6817

.NOTPARALLEL:

$(eval $(call BuildImage))
