include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

DEVICE_VARS += RAS_BOARD RAS_ROOTFS_SIZE RAS_VERSION

define Device/Default
	PROFILES := Default
	KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
	KERNEL_INITRAMFS_PREFIX := $$(IMG_PREFIX)-$(1)-initramfs
	KERNEL_PREFIX := $$(IMAGE_PREFIX)
	KERNEL_LOADADDR := 0x80208000
	SUPPORTED_DEVICES := $(subst _,$(comma),$(1))
	IMAGE/sysupgrade.bin = sysupgrade-tar | append-metadata
	IMAGE/sysupgrade.bin/squashfs :=
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
	KERNEL_SUFFIX := -fit-uImage.itb
	KERNEL = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL_NAME := Image
	NETGEAR_BOARD_ID :=
	NETGEAR_HW_ID :=
	IMAGES := factory.img sysupgrade.bin
	IMAGE/factory.img := append-kernel | pad-offset 64k 64 | append-uImage-fakehdr filesystem | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | netgear-dni
	IMAGE/sysupgrade.bin := append-kernel | pad-offset 64k 64 | append-uImage-fakehdr filesystem | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
endef
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID


define Device/8dev_jalapeno
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_DTS := qcom-ipq4018-jalapeno
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_TITLE := 8devices Jalapeno
endef
TARGET_DEVICES += 8dev_jalapeno

define Device/asus_rt-ac58u
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4018-rt-ac58u
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DTB_SIZE := 65536
	DEVICE_TITLE := Asus RT-AC58U
	IMAGE_SIZE := 20439364
	FILESYSTEMS := squashfs
#	Someone - in their infinite wisdom - decided to put the firmware
#	version in front of the image name \03\00\00\04 => Version 3.0.0.4
#	Since u-boot works with strings we either need another fixup step
#	to add a version... or we are very careful not to add '\0' into that
#	string and call it a day.... Yeah, we do the latter!
	UIMAGE_NAME:=$(shell echo -e '\03\01\01\01RT-AC58U')
	KERNEL_INITRAMFS := $$(KERNEL) | uImage none
	KERNEL_INITRAMFS_SUFFIX := -factory.trx
	IMAGES := sysupgrade.bin
	DEVICE_PACKAGES := kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += asus_rt-ac58u

define Device/asus_rt-ac1300uhp
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4018-rt-ac1300uhp
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DTB_SIZE := 65536
	DEVICE_TITLE := Asus RT-AC1300UHP
	IMAGE_SIZE := 20439364
	FILESYSTEMS := squashfs
	UIMAGE_NAME:=$(shell echo -e '\03\01\01\01RT-AC58U')
	KERNEL_INITRAMFS := $$(KERNEL) | uImage none
	KERNEL_INITRAMFS_SUFFIX := -factory.trx
	IMAGES := sysupgrade.bin
	DEVICE_PACKAGES := kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += asus_rt-ac1300uhp

define Device/asus_rt-acrh17
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4019-rt-acrh17
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DTB_SIZE := 65536
	DEVICE_TITLE := Asus RT-ACRH17
	IMAGE_SIZE := 20439364
	FILESYSTEMS := squashfs
	UIMAGE_NAME:=$(shell echo -e '\03\01\01\01RT-AC82U')
	KERNEL_INITRAMFS := $$(KERNEL) | uImage none
	KERNEL_INITRAMFS_SUFFIX := -factory.trx
	IMAGES := sysupgrade.bin
	DEVICE_PACKAGES := ipq-wifi-rt-acrh17 ath10k-firmware-qca9984
endef
TARGET_DEVICES += asus_rt-acrh17

define Device/avm_fritzbox-4040
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4018-fritz4040
	BOARD_NAME := fritz4040
	DEVICE_TITLE := AVM Fritz!Box 4040
	IMAGE_SIZE := 29753344
	UBOOT_PATH := $$(BIN_DIR)/u-boot-fritz4040/uboot-fritz4040.bin
	UBOOT_PARTITION_SIZE := 524288
	IMAGES = eva.bin sysupgrade.bin
	IMAGE/eva.bin := append-uboot | pad-to $$$$(UBOOT_PARTITION_SIZE) | append-kernel | append-rootfs | pad-rootfs
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := fritz-tffs fritz-caldata u-boot-fritz4040
endef
TARGET_DEVICES += avm_fritzbox-4040

define Device/compex_wpj428
	$(call Device/FitImage)
	DEVICE_DTS := qcom-ipq4028-wpj428
	DEVICE_DTS_CONFIG := config@4
	BLOCKSIZE := 64k
	DEVICE_TITLE := Compex WPJ428
	IMAGE_SIZE := 31232k
	KERNEL_SIZE := 4096k
	IMAGES = sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := kmod-gpio-beeper
endef
TARGET_DEVICES += compex_wpj428

define Device/glinet_gl-b1300
	$(call Device/FitImage)
	DEVICE_TITLE := GL.iNet GL-B1300
	BOARD_NAME := gl-b1300
	DEVICE_DTS := qcom-ipq4029-gl-b1300
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 26624k
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel |append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += glinet_gl-b1300

define Device/meraki_mr33
	$(call Device/FitImage)
	DEVICE_DTS := qcom-ipq4029-mr33
	BLOCKSIZE := 131072
	PAGESIZE := 2048
	DEVICE_TITLE := Cisco Meraki MR33
	IMAGES = sysupgrade.bin
	DEVICE_PACKAGES := -swconfig ath10k-firmware-qca9887-ct
endef
TARGET_DEVICES += meraki_mr33

define Device/netgear_ex61x0v2
	$(call Device/DniImage)
	KERNEL_SIZE := 3080192
	DEVICE_DTS_CONFIG := config@4
	NETGEAR_BOARD_ID := EX6150v2series
	NETGEAR_HW_ID := 29765285+16+0+128+2x2
endef

define Device/netgear_ex6100v2
	$(call Device/netgear_ex61x0v2)
	DEVICE_DTS := qcom-ipq4018-ex6100v2
	DEVICE_TITLE := Netgear EX6100v2
endef
TARGET_DEVICES += netgear_ex6100v2

define Device/netgear_ex6150v2
	$(call Device/netgear_ex61x0v2)
	DEVICE_DTS := qcom-ipq4018-ex6150v2
	DEVICE_TITLE := Netgear EX6150v2
endef
TARGET_DEVICES += netgear_ex6150v2

define Device/openmesh_a42
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4018-a42
	DEVICE_DTS_CONFIG := config@om.a42
	BLOCKSIZE := 64k
	DEVICE_TITLE := OpenMesh A42
	KERNEL = kernel-bin | lzma | fit lzma $$(DTS_DIR)/$$(DEVICE_DTS).dtb | pad-to $$(BLOCKSIZE)
	IMAGE_SIZE := 15616k
	IMAGES = factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | openmesh-image ce_type=A42
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-rootfs | sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := uboot-envtools
endef
TARGET_DEVICES += openmesh_a42

define Device/openmesh_a62
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4019-a62
	DEVICE_DTS_CONFIG := config@om.a62
	BLOCKSIZE := 64k
	DEVICE_TITLE := OpenMesh A62
	KERNEL = kernel-bin | lzma | fit lzma $$(DTS_DIR)/$$(DEVICE_DTS).dtb | pad-to $$(BLOCKSIZE)
	IMAGE_SIZE := 15552k
	IMAGES = factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | openmesh-image ce_type=A62
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-rootfs | sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca9888-ct uboot-envtools
endef

TARGET_DEVICES += openmesh_a62

define Device/qcom_ap-dk01.1-c1
	DEVICE_TITLE := QCA AP-DK01.1-C1
	BOARD_NAME := ap-dk01.1-c1
	DEVICE_DTS := qcom-ipq4019-ap.dk01.1-c1
	KERNEL_INSTALL := 1
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 26624k
	$(call Device/FitImage)
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$${KERNEL_SIZE} | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += qcom_ap-dk01.1-c1

define Device/qcom_ap-dk04.1-c1
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_DTS := qcom-ipq4019-ap.dk04.1-c1
	KERNEL_INSTALL := 1
	KERNEL_SIZE := 4048k
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap-dk04.1-c1
	DEVICE_TITLE := QCA AP-DK04.1-C1
endef
TARGET_DEVICES += qcom_ap-dk04.1-c1

define Device/zyxel_nbg6617
	$(call Device/FitImageLzma)
	DEVICE_DTS := qcom-ipq4018-nbg6617
	DEVICE_TITLE := ZyXEL NBG6617
	ROOTFS_SIZE := 24960k
	RAS_BOARD := NBG6617
	RAS_ROOTFS_SIZE := 19840k
	RAS_VERSION := "$(VERSION_DIST) $(REVISION)"
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	IMAGES := sysupgrade.bin factory.bin
#	The ZyXEL firmware allows flashing thru the web-gui only when the rootfs is
#	at least as large as the one of the initial firmware image (not the current
#	one on the device). This only applies to the Web-UI, the bootlaoder ignores
#	this minimum-size. However, the larger image can be flashed both ways.
	IMAGE/factory.bin := append-rootfs | pad-rootfs | pad-to 64k | check-size $$$$(ROOTFS_SIZE) | zyxel-ras-image separate-kernel
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-rootfs | check-size $$$$(ROOTFS_SIZE) | sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := ipq-wifi-zyxel_nbg6617 uboot-envtools
endef
TARGET_DEVICES += zyxel_nbg6617

define Device/zyxel_wre6606
	$(call Device/FitImage)
	DEVICE_TITLE := ZyXEL WRE6606
	DEVICE_DTS_CONFIG := config@4
	DEVICE_DTS := qcom-ipq4018-wre6606
	DEVICE_PACKAGES := ipq-wifi-zyxel_wre6606
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += zyxel_wre6606

$(eval $(call BuildImage))
