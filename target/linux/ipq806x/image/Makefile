# Copyright (c) 2014 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

UBIFS_OPTS = -m 2048 -e 124KiB -c 4096 -U -F

define Image/Prepare
	$(CP) $(LINUX_DIR)/vmlinux $(KDIR)/$(IMG_PREFIX)-vmlinux.elf
endef

define Image/BuildKernel
	$(CP) $(KDIR)/$(IMG_PREFIX)-vmlinux.elf $(BIN_DIR)
endef

define Image/Build/squashfs
	cp $(KDIR)/root.squashfs $(KDIR)/root.squashfs-raw
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-$(1)-root.img bs=2k conv=sync
endef

define Device/Default
	PROFILES := Default
	KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
	KERNEL_INITRAMFS_PREFIX := $$(IMG_PREFIX)-$(1)-initramfs
	KERNEL_PREFIX := $$(IMAGE_PREFIX)
	KERNEL_LOADADDR = 0x42208000
	SUPPORTED_DEVICES = $$(BOARD_NAME)
	IMAGE/sysupgrade.tar = sysupgrade-tar | append-metadata
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
	IMAGES := nand-factory.ubi nand-sysupgrade.tar
	IMAGE/nand-factory.ubi := append-ubi
	IMAGE/nand-sysupgrade.tar := sysupgrade-tar | append-metadata
endef

define Device/DniImage
	PROFILES += $$(DEVICE_NAME)
	FILESYSTEMS := squashfs
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none
	KERNEL_NAME := zImage
	NETGEAR_BOARD_ID :=
	NETGEAR_HW_ID :=
	UBINIZE_OPTS := -E 5
	IMAGES := factory.img sysupgrade.tar
	IMAGE/factory.img := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-uImage-fakeroot-hdr | pad-to $$$$(KERNEL_SIZE) | append-ubi | netgear-dni
	IMAGE/sysupgrade.tar := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-uImage-fakeroot-hdr | sysupgrade-tar kernel=$$$$@ | append-metadata
endef
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID

define Device/TpSafeImage
	PROFILES += $$(DEVICE_NAME)
	FILESYSTEMS := squashfs
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
	PROFILES += $$(DEVICE_NAME)
	FILESYSTEMS := squashfs
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none | pad-to $${KERNEL_SIZE}
	KERNEL_NAME := zImage
	IMAGES := sysupgrade.tar mmcblk0p5-rootfs.bin mmcblk0p4-kernel.bin
	IMAGE/sysupgrade.tar/squashfs := append-rootfs | pad-to $$$${BLOCKSIZE} | sysupgrade-tar rootfs=$$$$@ | append-metadata
	IMAGE/mmcblk0p5-rootfs.bin := append-rootfs | pad-rootfs | pad-to $$$${BLOCKSIZE}
	IMAGE/mmcblk0p4-kernel.bin := append-kernel
endef

define Device/AP148
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_DTS := qcom-ipq8064-ap148
	KERNEL_INSTALL := 1
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap148
	DEVICE_TITLE := Qualcomm AP148
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
endef

define Device/AP148-legacy
	$(call Device/LegacyImage)
	$(call Device/UbiFit)
	DEVICE_DTS := qcom-ipq8064-ap148
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap148
	DEVICE_TITLE := Qualcomm AP148 (legacy)
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
endef

define Device/C2600
	$(call Device/TpSafeImage)
	DEVICE_DTS := qcom-ipq8064-c2600
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := c2600
	TPLINK_BOARD_ID := C2600
	DEVICE_TITLE := TP-Link Archer C2600
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
endef

define Device/D7800
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8064-d7800
	KERNEL_SIZE := 2097152
	NETGEAR_BOARD_ID := D7800
	NETGEAR_HW_ID := 29764958+0+128+512+4x4+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := d7800
	DEVICE_TITLE := Netgear Nighthawk X4 D7800
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
endef

define Device/DB149
	$(call Device/FitImage)
	DEVICE_DTS := qcom-ipq8064-db149
	KERNEL_INSTALL := 1
	BOARD_NAME := db149
	DEVICE_TITLE := Qualcomm DB149
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
endef

define Device/EA8500
	$(call Device/LegacyImage)
	DEVICE_DTS := qcom-ipq8064-ea8500
	PAGESIZE := 2048
	BLOCKSIZE := 128k
	KERNEL_SIZE := 3072k
	FILESYSTEMS := squashfs
	KERNEL = kernel-bin | append-dtb | uImage none | append-uImage-fakeroot-hdr
	PROFILES += $$(DEVICE_NAME)
	BOARD_NAME := ea8500
	UBINIZE_OPTS := -E 5
	IMAGES := factory.bin sysupgrade.tar
	IMAGE/factory.bin := append-kernel | pad-to $$$${KERNEL_SIZE} | append-ubi
	DEVICE_TITLE := Linksys EA8500
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
endef

define Device/FRITZ4040
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4019-fritz4040
	KERNEL_LOADADDR := 0x80208000
	BLOCKSIZE := 4k
	PAGESIZE := 256
	BOARD_NAME := fritz4040
	DEVICE_TITLE := AVM Fritz!Box 4040
	IMAGE_SIZE := 29753344
	IMAGES = sysupgrade.bin
	FILESYSTEMS := squashfs
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := kmod-usb-phy-qcom-ipq4019 ipq-wifi-fritz4040 fritz-tffs fritz-caldata u-boot-fritz4040
endef

define Device/R7500
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8064-r7500
	KERNEL_SIZE := 2097152
	NETGEAR_BOARD_ID := R7500
	NETGEAR_HW_ID := 29764841+0+128+256+3x3+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7500
	DEVICE_TITLE := Netgear Nighthawk X4 R7500
	DEVICE_PACKAGES := ath10k-firmware-qca988x
endef

define Device/R7500v2
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8064-r7500v2
	KERNEL_SIZE := 2097152
	NETGEAR_BOARD_ID := R7500v2
	NETGEAR_HW_ID := 29764958+0+128+512+3x3+4x4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7500v2
	DEVICE_TITLE := Netgear Nighthawk X4 R7500v2
	DEVICE_PACKAGES := ath10k-firmware-qca99x0 ath10k-firmware-qca988x
endef

define Device/R7800
	$(call Device/DniImage)
	DEVICE_DTS := qcom-ipq8065-r7800
	KERNEL_SIZE := 2097152
	NETGEAR_BOARD_ID := R7800
	NETGEAR_HW_ID := 29764958+0+128+512+4x4+4x4+cascade
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := r7800
	DEVICE_TITLE := Netgear Nighthawk X4S R7800
	DEVICE_PACKAGES := ath10k-firmware-qca9984
endef

define Build/copy-file
	cat "$(1)" > "$@"
endef

define Device/RT-AC58U
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4019-rt-ac58u
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DTB_SIZE := 65536
	BOARD_NAME := rt-ac58u
	KERNEL_LOADADDR := 0x80208000
	DEVICE_TITLE := Asus RT-AC58U
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

define Device/NBG6817
	DEVICE_DTS := qcom-ipq8065-nbg6817
	KERNEL_SIZE := 4096k
	BLOCKSIZE := 64k
	BOARD_NAME := nbg6817
	DEVICE_TITLE := ZyXEL NBG6817
	DEVICE_PACKAGES := ath10k-firmware-qca9984 e2fsprogs losetup
	$(call Device/ZyXELImage)
endef

define Device/VR2600v
	PROFILES += $$(DEVICE_NAME)
	FILESYSTEMS := squashfs
	KERNEL_SUFFIX := -uImage
	KERNEL = kernel-bin | append-dtb | uImage none
	KERNEL_NAME := zImage
	KERNEL_SIZE := 2097152
	DEVICE_DTS := qcom-ipq8064-vr2600v
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := vr2600v
	DEVICE_TITLE := TP-Link Archer VR2600v
	DEVICE_PACKAGES := ath10k-firmware-qca99x0
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := pad-extra 512 | append-kernel | pad-to $$$${KERNEL_SIZE} | append-rootfs | pad-rootfs | append-metadata
endef

define Device/AP-DK04.1-C1
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

TARGET_DEVICES += AP148 AP148-legacy C2600 D7800 DB149 EA8500 FRITZ4040 R7500 \
		  R7500v2 R7800 RT-AC58U NBG6817 VR2600v AP-DK04.1-C1

$(eval $(call BuildImage))
