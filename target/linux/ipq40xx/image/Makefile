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

define Device/FitzImage
	KERNEL_SUFFIX := -fit-zImage.itb
	KERNEL = kernel-bin | fit none $$(DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL_NAME := zImage
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
	IMAGE/factory.img := append-kernel | pad-offset 64k 64 | append-uImage-fakehdr filesystem | append-rootfs | pad-rootfs | netgear-dni
	IMAGE/sysupgrade.bin := append-kernel | pad-offset 64k 64 | append-uImage-fakehdr filesystem | append-rootfs | pad-rootfs | append-metadata
endef
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID

define Device/8dev_jalapeno
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := 8devices
	DEVICE_MODEL := Jalapeno
	DEVICE_DTS := qcom-ipq4018-jalapeno
	BLOCKSIZE := 128k
	PAGESIZE := 2048
endef
TARGET_DEVICES += 8dev_jalapeno

define Device/asus_map-ac2200
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := ASUS
	DEVICE_MODEL := Lyra (MAP-AC2200)
	DEVICE_DTS := qcom-ipq4019-map-ac2200
	IMAGES := sysupgrade.bin
	DEVICE_PACKAGES := ath10k-firmware-qca9888 kmod-ath3k
endef
TARGET_DEVICES += asus_map-ac2200

define Device/asus_rt-ac58u
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := ASUS
	DEVICE_MODEL := RT-AC58U
	DEVICE_DTS := qcom-ipq4018-rt-ac58u
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DTB_SIZE := 65536
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
	DEVICE_PACKAGES := -kmod-ath10k kmod-ath10k-ct kmod-ath10k-ct-smallbuffers \
		kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += asus_rt-ac58u

define Device/asus_rt-ac1300uhp
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := ASUS
	DEVICE_MODEL := RT-AC1300UHP
	DEVICE_DTS := qcom-ipq4018-rt-ac58u
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DTB_SIZE := 65536
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
	DEVICE_VENDOR := ASUS
	DEVICE_MODEL := RT-ACRH17
	DEVICE_DTS := qcom-ipq4019-rt-acrh17
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DTB_SIZE := 65536
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
	DEVICE_VENDOR := AVM
	DEVICE_MODEL := FRITZ!Box 4040
	DEVICE_DTS := qcom-ipq4018-fritz4040
	BOARD_NAME := fritz4040
	IMAGE_SIZE := 29056k
	UBOOT_PATH := $(STAGING_DIR_IMAGE)/uboot-fritz4040.bin
	UBOOT_PARTITION_SIZE := 524288
	IMAGES = sysupgrade.bin
	IMAGE/eva.bin := append-uboot | pad-to $$$$(UBOOT_PARTITION_SIZE) | append-kernel | append-rootfs | pad-rootfs
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := fritz-tffs fritz-caldata
endef
TARGET_DEVICES += avm_fritzbox-4040

define Device/compex_wpj428
	$(call Device/FitImage)
	DEVICE_VENDOR := Compex
	DEVICE_MODEL := WPJ428
	DEVICE_DTS := qcom-ipq4028-wpj428
	DEVICE_DTS_CONFIG := config@4
	BLOCKSIZE := 64k
	IMAGE_SIZE := 31232k
	KERNEL_SIZE := 4096k
	IMAGES = sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := kmod-gpio-beeper
endef
TARGET_DEVICES += compex_wpj428

define Device/engenius_eap1300
	$(call Device/FitImage)
	DEVICE_VENDOR := EnGenius
	DEVICE_MODEL := EAP1300
	DEVICE_DTS := qcom-ipq4018-eap1300
	DEVICE_DTS_CONFIG := config@4
	BOARD_NAME := eap1300
	KERNEL_SIZE := 5120k
	IMAGE_SIZE := 25344k
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += engenius_eap1300

define Device/glinet_gl-b1300
	$(call Device/FitImage)
	DEVICE_VENDOR := GL.iNet
	DEVICE_MODEL := GL-B1300
	DEVICE_DTS := qcom-ipq4029-gl-b1300
	BOARD_NAME := gl-b1300
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 26624k
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel |append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += glinet_gl-b1300

define Device/hugo_ac1200
	$(call Device/FitImage)
	DEVICE_VENDOR := Hugo
	DEVICE_MODEL := AC1200
	BOARD_NAME := hugo_ac1200
	DEVICE_DTS := qcom-ipq4019-hugo-ac1200
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 31232k
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := ipq-wifi-hugo_ac1200
endef
TARGET_DEVICES += hugo_ac1200

define Device/linksys_ea6350v3
	# The Linksys EA6350v3 has a uboot bootloader that does not
	# support either booting lzma kernel images nor booting UBI
	# partitions. This uboot, however, supports raw kernel images and
	# gzipped images.
	#
	# As for the time of writing this, the device will boot the kernel
	# from a fixed address with a fixed length of 3MiB. Also, the
	# device has a hard-coded kernel command line that requieres the
	# rootfs and alt_rootfs to be in mtd11 and mtd13 respectively.
	# Oh... and the kernel partition overlaps with the rootfs
	# partition (the same for alt_kernel and alt_rootfs).
	#
	# If you are planing re-partitioning the device, you may want to
	# keep those details in mind:
	# 1. The kernel adresses you should honor are 0x00000000 and
	#    0x02800000 respectively.
	# 2. The kernel size (plus the dtb) cannot exceed 3.00MiB in size.
	# 3. You can use 'zImage', but not a raw 'Image' packed with lzma.
	# 4. The kernel command line from uboot is harcoded to boot with
	#    rootfs either in mtd11 or mtd13.
	$(call Device/FitzImage)
	DEVICE_VENDOR := Linksys
	DEVICE_MODEL := EA6350
	DEVICE_VARIANT := v3
	DEVICE_DTS := qcom-ipq4018-ea6350v3
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	KERNEL_SIZE := 3072k
	IMAGE_SIZE := 37888k
	UBINIZE_OPTS := -E 5
	IMAGES := factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-kernel | append-uImage-fakehdr filesystem | pad-to $$$${KERNEL_SIZE} | append-ubi | linksys-image type=EA6350v3
	DEVICE_PACKAGES := uboot-envtools
endef
TARGET_DEVICES += linksys_ea6350v3

define Device/meraki_mr33
	$(call Device/FitImage)
	DEVICE_VENDOR := Cisco Meraki
	DEVICE_MODEL := MR33
	DEVICE_DTS := qcom-ipq4029-mr33
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGES = sysupgrade.bin
	DEVICE_PACKAGES := -swconfig ath10k-firmware-qca9887
endef
TARGET_DEVICES += meraki_mr33

define Device/mobipromo_cm520-79f
	$(call Device/FitzImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := MobiPromo
	DEVICE_MODEL := CM520-79F
	DEVICE_DTS := qcom-ipq4019-cm520-79f
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_PACKAGES := ipq-wifi-mobipromo_cm520-79f kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += mobipromo_cm520-79f

define Device/netgear_ex61x0v2
	$(call Device/DniImage)
	DEVICE_DTS_CONFIG := config@4
	NETGEAR_BOARD_ID := EX6150v2series
	NETGEAR_HW_ID := 29765285+16+0+128+2x2
endef

define Device/netgear_ex6100v2
	$(call Device/netgear_ex61x0v2)
	DEVICE_VENDOR := Netgear
	DEVICE_MODEL := EX6100
	DEVICE_VARIANT := v2
	DEVICE_DTS := qcom-ipq4018-ex6100v2
endef
TARGET_DEVICES += netgear_ex6100v2

define Device/netgear_ex6150v2
	$(call Device/netgear_ex61x0v2)
	DEVICE_VENDOR := Netgear
	DEVICE_MODEL := EX6150
	DEVICE_VARIANT := v2
	DEVICE_DTS := qcom-ipq4018-ex6150v2
endef
TARGET_DEVICES += netgear_ex6150v2

define Device/openmesh_a42
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := OpenMesh
	DEVICE_MODEL := A42
	DEVICE_DTS := qcom-ipq4018-a42
	DEVICE_DTS_CONFIG := config@om.a42
	BLOCKSIZE := 64k
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
	DEVICE_VENDOR := OpenMesh
	DEVICE_MODEL := A62
	DEVICE_DTS := qcom-ipq4019-a62
	DEVICE_DTS_CONFIG := config@om.a62
	BLOCKSIZE := 64k
	KERNEL = kernel-bin | lzma | fit lzma $$(DTS_DIR)/$$(DEVICE_DTS).dtb | pad-to $$(BLOCKSIZE)
	IMAGE_SIZE := 15552k
	IMAGES = factory.bin sysupgrade.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | openmesh-image ce_type=A62
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-rootfs | sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca9888 uboot-envtools
endef
TARGET_DEVICES += openmesh_a62

define Device/p2w_r619ac
	$(call Device/FitzImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := P&W
	DEVICE_MODEL := R619AC
	DEVICE_DTS := qcom-ipq4019-r619ac
	DEVICE_DTS_CONFIG := config@10
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGES += nand-factory.bin
	IMAGE/nand-factory.bin := append-ubi | qsdk-ipq-factory-nand
	DEVICE_PACKAGES := ipq-wifi-p2w_r619ac
endef
TARGET_DEVICES += p2w_r619ac

define Device/p2w_r619ac-128m
	$(call Device/FitzImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := P&W
	DEVICE_MODEL := R619AC
	DEVICE_VARIANT := 128M
	DEVICE_DTS := qcom-ipq4019-r619ac-128m
	DEVICE_DTS_CONFIG := config@10
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_PACKAGES := ipq-wifi-p2w_r619ac
endef
TARGET_DEVICES += p2w_r619ac-128m

define Device/qcom_ap-dk01.1-c1
	DEVICE_VENDOR := Qualcomm Atheros
	DEVICE_MODEL := AP-DK01.1
	DEVICE_VARIANT := C1
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
	DEVICE_VENDOR := Qualcomm Atheros
	DEVICE_MODEL := AP-DK04.1
	DEVICE_VARIANT := C1
	DEVICE_DTS := qcom-ipq4019-ap.dk04.1-c1
	KERNEL_INSTALL := 1
	KERNEL_SIZE := 4048k
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap-dk04.1-c1
endef
TARGET_DEVICES += qcom_ap-dk04.1-c1

define Device/zyxel_nbg6617
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := ZyXEL
	DEVICE_MODEL := NBG6617
	DEVICE_DTS := qcom-ipq4018-nbg6617
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
	DEVICE_PACKAGES := uboot-envtools kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += zyxel_nbg6617

define Device/zyxel_wre6606
	$(call Device/FitImage)
	DEVICE_VENDOR := ZyXEL
	DEVICE_MODEL := WRE6606
	DEVICE_DTS := qcom-ipq4018-wre6606
	DEVICE_DTS_CONFIG := config@4
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := -kmod-ath10k kmod-ath10k-ct kmod-ath10k-ct-smallbuffers
endef
TARGET_DEVICES += zyxel_wre6606

$(eval $(call BuildImage))
