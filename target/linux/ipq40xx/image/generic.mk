
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID
DEVICE_VARS += RAS_BOARD RAS_ROOTFS_SIZE RAS_VERSION
DEVICE_VARS += WRGG_DEVNAME WRGG_SIGNATURE

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
	$(call Device/FitzImage)
	NETGEAR_BOARD_ID :=
	NETGEAR_HW_ID :=
	IMAGES += factory.img
	IMAGE/factory.img := append-kernel | pad-offset 64k 64 | append-uImage-fakehdr filesystem | append-rootfs | pad-rootfs | netgear-dni
	IMAGE/sysupgrade.bin := append-kernel | pad-offset 64k 64 | append-uImage-fakehdr filesystem | \
		append-rootfs | pad-rootfs | check-size | append-metadata
endef

define Build/append-rootfshdr
	mkimage -A $(LINUX_KARCH) \
		-O linux -T filesystem \
		-C lzma -a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-n root.squashfs -d $(IMAGE_ROOTFS) $@.new
	dd if=$@.new bs=64 count=1 >> $(IMAGE_KERNEL)
endef

define Build/append-rutx-metadata
	echo \
		'{ \
			"device_code": [".*"], \
			"hwver": [".*"], \
			"batch": [".*"], \
			"serial": [".*"], \
			"supported_devices":["teltonika,rutx"] \
		}' | fwtool -I - $@
endef

define Build/copy-file
	cat "$(1)" > "$@"
endef

define Build/mkmylofw_32m
	$(eval device_id=$(word 1,$(1)))
	$(eval revision=$(word 2,$(1)))

	let \
		size="$$(stat -c%s $@)" \
		pad="$(subst k,* 1024,$(BLOCKSIZE))" \
		pad="(pad - (size % pad)) % pad" \
		newsize='size + pad'; \
		$(STAGING_DIR_HOST)/bin/mkmylofw \
		-B WPE72 -i 0x11f6:$(device_id):0x11f6:$(device_id) -r $(revision) \
		-s 0x2000000 -p0x180000:$$newsize:al:0x80208000:"OpenWrt":$@ \
		$@.new
	@mv $@.new $@
endef

define Build/wac5xx-netgear-tar
	mkdir $@.tmp
	mv $@ $@.tmp/wac5xx-ubifs-root.img
	md5sum $@.tmp/wac5xx-ubifs-root.img > $@.tmp/wac5xx-ubifs-root.md5sum
	echo "WAC505 WAC510" > $@.tmp/metadata.txt
	echo "WAC505_V9.9.9.9" > $@.tmp/version
	tar -C $@.tmp/ -cf $@ .
	rm -rf $@.tmp
endef

define Build/qsdk-ipq-factory-nand-askey
	$(TOPDIR)/scripts/mkits-qsdk-ipq-image.sh $@.its\
		askey_kernel $(IMAGE_KERNEL) \
		askey_fs $(IMAGE_ROOTFS) \
		ubifs $@
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its $@.new
	@mv $@.new $@
endef

define Build/qsdk-ipq-app-gpt
	cp $@ $@.tmp 2>/dev/null || true
	ptgen -g -o $@.tmp -a 1 -l 1024 \
			-t 0x2e -N 0:HLOS -r -p 32M \
			-t 0x83 -N rootfs -r -p 128M \
				-N rootfs_data -p 512M
	cat $@.tmp >> $@
	rm $@.tmp
endef

define Build/SenaoFW
	-$(STAGING_DIR_HOST)/bin/mksenaofw \
		-n $(BOARD_NAME) -r $(VENDOR_ID) -p $(1) \
		-c $(DATECODE) -w $(2) -x $(CW_VER) -t 0 \
		-e $@ \
		-o $@.new
	@cp $@.new $@
endef

define Build/wrgg-image
	mkwrggimg -i $@ \
	-o $@.new \
	-d "$(WRGG_DEVNAME)" \
	-s "$(WRGG_SIGNATURE)" \
	-v "" -m "" -B ""
	mv $@.new $@
endef

define Device/8dev_habanero-dvk
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := 8devices
	DEVICE_MODEL := Habanero DVK
	IMAGE_SIZE := 30976k
	SOC := qcom-ipq4019
	IMAGE/sysupgrade.bin := append-kernel | pad-to 64k | append-rootfs | pad-rootfs | check-size | append-metadata
endef
TARGET_DEVICES += 8dev_habanero-dvk

define Device/8dev_jalapeno-common
	$(call Device/FitImage)
	$(call Device/UbiFit)
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := qcom-ipq4018
endef

define Device/8dev_jalapeno
	$(call Device/8dev_jalapeno-common)
	DEVICE_VENDOR := 8devices
	DEVICE_MODEL := Jalapeno
endef
TARGET_DEVICES += 8dev_jalapeno

define Device/alfa-network_ap120c-ac
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := ALFA Network
	DEVICE_MODEL := AP120C-AC
	SOC := qcom-ipq4018
	DEVICE_PACKAGES := kmod-usb-acm kmod-tpm-i2c-atmel
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGE_SIZE := 65536k
	IMAGES := nand-factory.bin nand-sysupgrade.bin
	IMAGE/nand-factory.bin := append-ubi | qsdk-ipq-factory-nand
endef
TARGET_DEVICES += alfa-network_ap120c-ac

define Device/aruba_glenmorangie
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Aruba
	SOC := qcom-ipq4029
endef

define Device/aruba_ap-303
	$(call Device/aruba_glenmorangie)
	DEVICE_MODEL := AP-303
endef
TARGET_DEVICES += aruba_ap-303

define Device/aruba_ap-303h
	$(call Device/aruba_glenmorangie)
	DEVICE_MODEL := AP-303H
endef
TARGET_DEVICES += aruba_ap-303h

define Device/aruba_ap-365
	$(call Device/aruba_glenmorangie)
	DEVICE_MODEL := AP-365
	DEVICE_PACKAGES := kmod-hwmon-ad7418
endef
TARGET_DEVICES += aruba_ap-365

define Device/asus_map-ac2200
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := ASUS
	DEVICE_MODEL := Lyra (MAP-AC2200)
	SOC := qcom-ipq4019
	DEVICE_PACKAGES := ath10k-firmware-qca9888-ct kmod-ath3k
endef
TARGET_DEVICES += asus_map-ac2200

# WARNING: this is an initramfs image that gets you half of the way there
#          you need to delete the jffs2 ubi volume and sysupgrade to the final image
# to get a "trx" you can flash via web UI for ac42u/ac58u:
# - change call Device/FitImageLzma to Device/FitImage
# - add the following below UIMAGE_NAME
#   UIMAGE_MAGIC := 0x27051956
#   IMAGES += factory.trx
#   IMAGE/factory.trx := copy-file $(KDIR)/tmp/$$(KERNEL_INITRAMFS_IMAGE) | uImage none
define Device/asus_rt-ac42u
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := ASUS
	DEVICE_MODEL := RT-AC42U
	DEVICE_ALT0_VENDOR := ASUS
	DEVICE_ALT0_MODEL := RT-ACRH17
	DEVICE_ALT1_VENDOR := ASUS
	DEVICE_ALT1_MODEL := RT-AC2200
	SOC := qcom-ipq4019
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGE_SIZE := 20439364
	FILESYSTEMS := squashfs
#	RT-AC82U is nowhere to be found online
#	Rather, this device is a/k/a RT-AC42U
#	But we'll go with what the vendor firmware has...
	UIMAGE_NAME:=$(shell echo -e '\03\01\01\01RT-AC82U')
	DEVICE_PACKAGES := ath10k-firmware-qca9984-ct kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += asus_rt-ac42u

define Device/asus_rt-ac58u
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := ASUS
	DEVICE_MODEL := RT-AC58U
	DEVICE_ALT0_VENDOR := ASUS
	DEVICE_ALT0_MODEL := RT-ACRH13
	SOC := qcom-ipq4018
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGE_SIZE := 20439364
	FILESYSTEMS := squashfs
#	Someone - in their infinite wisdom - decided to put the firmware
#	version in front of the image name \03\00\00\04 => Version 3.0.0.4
#	Since u-boot works with strings we either need another fixup step
#	to add a version... or we are very careful not to add '\0' into that
#	string and call it a day.... Yeah, we do the latter!
	UIMAGE_NAME:=$(shell echo -e '\03\01\01\01RT-AC58U')
	DEVICE_PACKAGES := -kmod-ath10k-ct kmod-ath10k-ct-smallbuffers \
		kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += asus_rt-ac58u

define Device/avm_fritzbox-4040
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := AVM
	DEVICE_MODEL := FRITZ!Box 4040
	SOC := qcom-ipq4018
	BOARD_NAME := fritz4040
	IMAGE_SIZE := 29056k
	UBOOT_PATH := $(STAGING_DIR_IMAGE)/uboot-fritz4040.bin
	UBOOT_PARTITION_SIZE := 524288
	IMAGES += eva.bin
	IMAGE/eva.bin := append-uboot | pad-to $$$$(UBOOT_PARTITION_SIZE) | append-kernel | append-rootfs | pad-rootfs
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | check-size | append-metadata
	DEVICE_PACKAGES := fritz-tffs fritz-caldata
endef
TARGET_DEVICES += avm_fritzbox-4040

define Device/avm_fritzbox-7530
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := AVM
	DEVICE_MODEL := FRITZ!Box 7530
	DEVICE_ALT0_VENDOR := AVM
	DEVICE_ALT0_MODEL := FRITZ!Box 7520
	SOC := qcom-ipq4019
	DEVICE_PACKAGES := fritz-caldata fritz-tffs-nand
endef
TARGET_DEVICES += avm_fritzbox-7530

define Device/avm_fritzrepeater-1200
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := AVM
	DEVICE_MODEL := FRITZ!Repeater 1200
	SOC := qcom-ipq4019
	DEVICE_PACKAGES := fritz-caldata fritz-tffs-nand
endef
TARGET_DEVICES += avm_fritzrepeater-1200

define Device/avm_fritzrepeater-3000
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := AVM
	DEVICE_MODEL := FRITZ!Repeater 3000
	SOC := qcom-ipq4019
	DEVICE_PACKAGES := ath10k-firmware-qca9984-ct fritz-caldata fritz-tffs-nand
endef
TARGET_DEVICES += avm_fritzrepeater-3000

define Device/buffalo_wtr-m2133hp
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Buffalo
	DEVICE_MODEL := WTR-M2133HP
	SOC := qcom-ipq4019
	DEVICE_PACKAGES := ath10k-firmware-qca9984-ct
	BLOCKSIZE := 128k
	PAGESIZE := 2048
endef
TARGET_DEVICES += buffalo_wtr-m2133hp

define Device/cellc_rtl30vw
	KERNEL_SUFFIX := -fit-zImage.itb
	KERNEL_INITRAMFS = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL = kernel-bin | fit none $$(DTS_DIR)/$$(DEVICE_DTS).dtb | uImage lzma | pad-to 2048
	KERNEL_NAME := zImage
	KERNEL_IN_UBI :=
	IMAGES := nand-factory.bin nand-sysupgrade.bin
	IMAGE/nand-factory.bin := append-rootfshdr | append-ubi | qsdk-ipq-factory-nand-askey
	IMAGE/nand-sysupgrade.bin := append-rootfshdr | sysupgrade-tar | append-metadata
	DEVICE_VENDOR := Cell C
	DEVICE_MODEL := RTL30VW
	SOC := qcom-ipq4019
	DEVICE_DTS_CONFIG := config@5
	KERNEL_INSTALL := 1
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 57344k
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_PACKAGES := kmod-usb-net-qmi-wwan kmod-usb-serial-option uqmi
endef
TARGET_DEVICES += cellc_rtl30vw

define Device/cilab_meshpoint-one
	$(call Device/8dev_jalapeno-common)
	DEVICE_VENDOR := Crisis Innovation Lab
	DEVICE_MODEL := MeshPoint.One
	DEVICE_PACKAGES := kmod-i2c-gpio kmod-iio-bmp280-i2c kmod-hwmon-ina2xx kmod-rtc-pcf2127
endef
TARGET_DEVICES += cilab_meshpoint-one

define Device/compex_wpj419
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Compex
	DEVICE_MODEL := WPJ419
	SOC := qcom-ipq4019
	DEVICE_DTS_CONFIG := config@12
	KERNEL_INSTALL := 1
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	FILESYSTEMS := squashfs
endef
TARGET_DEVICES += compex_wpj419

define Device/compex_wpj428
	$(call Device/FitImage)
	DEVICE_VENDOR := Compex
	DEVICE_MODEL := WPJ428
	SOC := qcom-ipq4028
	DEVICE_DTS_CONFIG := config@4
	BLOCKSIZE := 64k
	IMAGE_SIZE := 31232k
	KERNEL_SIZE := 4096k
	IMAGES += cpximg-6a04.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	IMAGE/cpximg-6a04.bin := append-kernel | append-rootfs | pad-rootfs | mkmylofw_32m 0x8A2 3
	DEVICE_PACKAGES := kmod-gpio-beeper
	DEFAULT := n
endef
TARGET_DEVICES += compex_wpj428

define Device/devolo_magic-2-wifi-next
	$(call Device/FitImage)
	DEVICE_VENDOR := devolo
	DEVICE_MODEL := Magic 2 WiFi next
	SOC := qcom-ipq4018
	KERNEL_SIZE := 4096k

	# If the bootloader sees 0xDEADC0DE and this trailer at the 64k boundary of a TFTP image
	# it will bootm it, just like we want for the initramfs.
	KERNEL_INITRAMFS := kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb | pad-to 64k |\
		append-string -e '\xDE\xAD\xC0\xDE{"fl_initramfs":""}\x00'

	IMAGE_SIZE := 26624k
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEFAULT := n
endef
TARGET_DEVICES += devolo_magic-2-wifi-next

define Device/dlink_dap-2610
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := D-Link
	DEVICE_MODEL := DAP-2610
	SOC := qcom-ipq4018
	DEVICE_DTS_CONFIG := config@ap.dk01.1-c1
	BLOCKSIZE := 64k
	WRGG_DEVNAME := /dev/mtdblock/8
	WRGG_SIGNATURE := wapac30_dkbs_dap2610
	IMAGE_SIZE := 14080k
	IMAGES += factory.bin
	# Bootloader expects a special 160 byte header which is added by
	# wrgg-image.
	# Factory image size must be larger than 6MB, and size in wrgg header must
	# match actual factory image size to be flashable from D-Link http server.
	# Bootloader verifies checksum of wrgg image before booting, thus jffs2
	# cannot be part of the wrgg image. This is solved in the factory image by
	# having the rootfs at the end of the image (without pad-rootfs). And in
	# the sysupgrade image only the kernel is included in the wrgg checksum,
	# but this is not flashable from the D-link http server.
	# append-rootfs must start on an erase block boundary.
	IMAGE/factory.bin    := append-kernel | pad-offset 6144k 160 | append-rootfs | wrgg-image | check-size
	IMAGE/sysupgrade.bin := append-kernel | wrgg-image | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | check-size | append-metadata
endef
TARGET_DEVICES += dlink_dap-2610

define Device/edgecore_ecw5211
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Edgecore
	DEVICE_MODEL := ECW5211
	SOC := qcom-ipq4018
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@ap.dk01.1-c2
	DEVICE_PACKAGES := kmod-tpm-i2c-atmel kmod-usb-acm
endef
TARGET_DEVICES += edgecore_ecw5211

define Device/edgecore_oap100
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Edgecore
	DEVICE_MODEL := OAP100
	SOC := qcom-ipq4019
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGES := nand-sysupgrade.bin
	DEVICE_DTS_CONFIG := config@ap.dk07.1-c1
	DEVICE_PACKAGES := kmod-usb-acm kmod-usb-net kmod-usb-net-cdc-qmi uqmi
endef
TARGET_DEVICES += edgecore_oap100

define Device/engenius_eap1300
	$(call Device/FitImage)
	DEVICE_VENDOR := EnGenius
	DEVICE_MODEL := EAP1300
	DEVICE_DTS_CONFIG := config@4
	BOARD_NAME := eap1300
	SOC := qcom-ipq4018
	KERNEL_SIZE := 5120k
	IMAGE_SIZE := 25344k
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += engenius_eap1300

define Device/engenius_eap2200
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := EnGenius
	DEVICE_MODEL := EAP2200
	SOC := qcom-ipq4019
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_PACKAGES := ath10k-firmware-qca9888-ct -kmod-ath10k-ct kmod-ath10k-ct-smallbuffers
endef
TARGET_DEVICES += engenius_eap2200

define Device/engenius_emd1
	$(call Device/FitImage)
	DEVICE_VENDOR := EnGenius
	DEVICE_MODEL := EMD1
	DEVICE_DTS_CONFIG := config@4
	SOC := qcom-ipq4018
	IMAGE_SIZE := 30720k
	IMAGES += factory.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	IMAGE/factory.bin := qsdk-ipq-factory-nor | check-size
endef
TARGET_DEVICES += engenius_emd1

define Device/engenius_emr3500
	$(call Device/FitImage)
	DEVICE_VENDOR := EnGenius
	DEVICE_MODEL := EMR3500
	DEVICE_DTS_CONFIG := config@4
	SOC := qcom-ipq4018
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 30720k
	IMAGES += factory.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	IMAGE/factory.bin := qsdk-ipq-factory-nor | check-size
	DEFAULT := n
endef
TARGET_DEVICES += engenius_emr3500

define Device/engenius_ens620ext
	$(call Device/FitImage)
	DEVICE_VENDOR := EnGenius
	DEVICE_MODEL := ENS620EXT
	SOC := qcom-ipq4018
	DEVICE_DTS_CONFIG := config@4
	BLOCKSIZE := 64k
	PAGESIZE := 256
	BOARD_NAME := ENS620EXT
	VENDOR_ID := 0x0101
	PRODUCT_ID := 0x79
	PRODUCT_ID_NEW := 0xA4
	DATECODE := 190507
	FW_VER := 3.1.2
	FW_VER_NEW := 3.5.6
	CW_VER := 1.8.99
	IMAGE_SIZE := 21312k
	KERNEL_SIZE := 5120k
	FILESYSTEMS := squashfs
	IMAGES += factory_30.bin factory_35.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | check-size | append-metadata
	IMAGE/factory_30.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | check-size | SenaoFW $$$$(PRODUCT_ID) $$$$(FW_VER)
	IMAGE/factory_35.bin := qsdk-ipq-factory-nor | check-size | SenaoFW $$$$(PRODUCT_ID_NEW) $$$$(FW_VER_NEW)
endef
TARGET_DEVICES += engenius_ens620ext

define Device/ezviz_cs-w3-wd1200g-eup
	$(call Device/FitImage)
	DEVICE_VENDOR := EZVIZ
	DEVICE_MODEL := CS-W3-WD1200G
	DEVICE_VARIANT := EUP
	IMAGE_SIZE := 14848k
	KERNEL_SIZE = 6m
	SOC := qcom-ipq4018
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | \
		append-metadata
	DEVICE_PACKAGES := -kmod-ath10k-ct kmod-ath10k-ct-smallbuffers
	DEVICE_COMPAT_VERSION := 2.0
	DEVICE_COMPAT_MESSAGE := uboot's bootcmd has to be updated (see wiki). \
		Upgrade via sysupgrade mechanism is not possible.
endef
TARGET_DEVICES += ezviz_cs-w3-wd1200g-eup

define Device/glinet_gl-ap1300
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := GL.iNet
	DEVICE_MODEL := GL-AP1300
	SOC := qcom-ipq4018
	DEVICE_DTS_CONFIG := config@ap.dk01.1-c2
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGE_SIZE := 131072k
	KERNEL_INSTALL := 1
	DEVICE_PACKAGES := kmod-usb-net-qmi-wwan kmod-usb-serial-option uqmi
endef
TARGET_DEVICES += glinet_gl-ap1300

define Device/glinet_gl-b1300
	$(call Device/FitzImage)
	DEVICE_VENDOR := GL.iNet
	DEVICE_MODEL := GL-B1300
	BOARD_NAME := gl-b1300
	SOC := qcom-ipq4029
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 26624k
	IMAGE/sysupgrade.bin := append-kernel |append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += glinet_gl-b1300

define Device/glinet_gl-b2200
	$(call Device/FitzImage)
	DEVICE_VENDOR := GL.iNet
	DEVICE_MODEL := GL-B2200
	SOC := qcom-ipq4019
	DEVICE_DTS_CONFIG := config@ap.dk04.1-c3
	KERNEL_INITRAMFS_SUFFIX := -recovery.itb
	IMAGES := emmc.img.gz sysupgrade.bin
	IMAGE/emmc.img.gz := qsdk-ipq-app-gpt |\
		pad-to 1024k | append-kernel |\
		pad-to 33792k | append-rootfs |\
		append-metadata | gzip
	IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca9888-ct \
		kmod-fs-ext4 kmod-mmc kmod-spi-dev mkf2fs e2fsprogs kmod-fs-f2fs
endef
TARGET_DEVICES += glinet_gl-b2200

define Device/glinet_gl-s1300
	$(call Device/FitzImage)
	DEVICE_VENDOR := GL.iNet
	DEVICE_MODEL := GL-S1300
	SOC := qcom-ipq4029
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 26624k
	IMAGES := sysupgrade.bin
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := kmod-fs-ext4 kmod-mmc kmod-spi-dev
endef
TARGET_DEVICES += glinet_gl-s1300

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
	SOC := qcom-ipq4018
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	KERNEL_SIZE := 3072k
	IMAGE_SIZE := 37888k
	UBINIZE_OPTS := -E 5
	IMAGES += factory.bin
	IMAGE/factory.bin := append-kernel | append-uImage-fakehdr filesystem | pad-to $$$$(KERNEL_SIZE) | append-ubi | linksys-image type=EA6350v3
endef
TARGET_DEVICES += linksys_ea6350v3

define Device/linksys_ea8300
	$(call Device/FitzImage)
	DEVICE_VENDOR := Linksys
	DEVICE_MODEL := EA8300
	SOC := qcom-ipq4019
	KERNEL_SIZE := 3072k
	IMAGE_SIZE := 87040k
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	UBINIZE_OPTS := -E 5    # EOD marks to "hide" factory sig at EOF
	IMAGES += factory.bin
	IMAGE/factory.bin  := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | linksys-image type=EA8300
	DEVICE_PACKAGES := ath10k-firmware-qca9888-ct kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += linksys_ea8300

define Device/linksys_mr8300
	$(call Device/FitzImage)
	DEVICE_VENDOR := Linksys
	DEVICE_MODEL := MR8300
	SOC := qcom-ipq4019
	KERNEL_SIZE := 3072k
	IMAGE_SIZE := 87040k
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	UBINIZE_OPTS := -E 5    # EOD marks to "hide" factory sig at EOF
	IMAGES += factory.bin
	IMAGE/factory.bin  := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | linksys-image type=MR8300
	DEVICE_PACKAGES := ath10k-firmware-qca9888-ct kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += linksys_mr8300

define Device/luma_wrtq-329acn
	$(call Device/FitImage)
	DEVICE_VENDOR := Luma Home
	DEVICE_MODEL := WRTQ-329ACN
	SOC := qcom-ipq4018
	DEVICE_PACKAGES := kmod-ath3k kmod-eeprom-at24 kmod-i2c-gpio
	IMAGE_SIZE := 76632k
	BLOCKSIZE := 128k
	PAGESIZE := 2048
endef
TARGET_DEVICES += luma_wrtq-329acn

define Device/meraki_mr33
	$(call Device/FitImage)
	DEVICE_VENDOR := Cisco Meraki
	DEVICE_MODEL := MR33
	SOC := qcom-ipq4029
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_PACKAGES := -swconfig ath10k-firmware-qca9887-ct
endef
TARGET_DEVICES += meraki_mr33

define Device/mobipromo_cm520-79f
	$(call Device/FitzImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := MobiPromo
	DEVICE_MODEL := CM520-79F
	SOC := qcom-ipq4019
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_PACKAGES := kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += mobipromo_cm520-79f

define Device/netgear_ex61x0v2
	$(call Device/DniImage)
	DEVICE_VENDOR := NETGEAR
	DEVICE_DTS_CONFIG := config@4
	NETGEAR_BOARD_ID := EX6150v2series
	NETGEAR_HW_ID := 29765285+16+0+128+2x2
	IMAGE_SIZE := 14400k
	SOC := qcom-ipq4018
endef

define Device/netgear_ex6100v2
	$(call Device/netgear_ex61x0v2)
	DEVICE_MODEL := EX6100
	DEVICE_VARIANT := v2
endef
TARGET_DEVICES += netgear_ex6100v2

define Device/netgear_ex6150v2
	$(call Device/netgear_ex61x0v2)
	DEVICE_MODEL := EX6150
	DEVICE_VARIANT := v2
endef
TARGET_DEVICES += netgear_ex6150v2

define Device/netgear_orbi
	$(call Device/DniImage)
	SOC := qcom-ipq4019
	DEVICE_VENDOR := NETGEAR
	IMAGE/factory.img := append-kernel | pad-offset 128k 64 | \
		append-uImage-fakehdr filesystem | pad-to $$$$(KERNEL_SIZE) | \
		append-rootfs | pad-rootfs | netgear-dni
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-to 64k | \
		sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca9984-ct e2fsprogs kmod-fs-ext4 losetup
endef

define Device/netgear_rbx50
	$(call Device/netgear_orbi)
	NETGEAR_HW_ID := 29765352+0+4000+512+2x2+2x2+4x4
	KERNEL_SIZE := 3932160
	ROOTFS_SIZE := 32243712
	IMAGE_SIZE := 36175872
endef

define Device/netgear_rbr50
	$(call Device/netgear_rbx50)
	DEVICE_MODEL := RBR50
	DEVICE_VARIANT := v1
	NETGEAR_BOARD_ID := RBR50
endef
TARGET_DEVICES += netgear_rbr50

define Device/netgear_rbs50
	$(call Device/netgear_rbx50)
	DEVICE_MODEL := RBS50
	DEVICE_VARIANT := v1
	NETGEAR_BOARD_ID := RBS50
endef
TARGET_DEVICES += netgear_rbs50

define Device/netgear_srx60
	$(call Device/netgear_orbi)
	NETGEAR_HW_ID := 29765352+0+4096+512+2x2+2x2+4x4
	KERNEL_SIZE := 3932160
	ROOTFS_SIZE := 32243712
	IMAGE_SIZE := 36175872
endef

define Device/netgear_srr60
	$(call Device/netgear_srx60)
	DEVICE_MODEL := SRR60
	NETGEAR_BOARD_ID := SRR60
endef
TARGET_DEVICES += netgear_srr60

define Device/netgear_srs60
	$(call Device/netgear_srx60)
	DEVICE_MODEL := SRS60
	NETGEAR_BOARD_ID := SRS60
endef
TARGET_DEVICES += netgear_srs60

define Device/netgear_wac510
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Netgear
	DEVICE_MODEL := WAC510
	SOC := qcom-ipq4018
	DEVICE_DTS_CONFIG := config@5
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGES += nand-factory.tar
	IMAGE/nand-factory.tar := append-ubi | wac5xx-netgear-tar
	DEVICE_PACKAGES := uboot-envtools
endef
TARGET_DEVICES += netgear_wac510

define Device/openmesh_a42
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := OpenMesh
	DEVICE_MODEL := A42
	SOC := qcom-ipq4018
	DEVICE_DTS_CONFIG := config@om.a42
	BLOCKSIZE := 64k
	KERNEL = kernel-bin | lzma | fit lzma $$(DTS_DIR)/$$(DEVICE_DTS).dtb | pad-to $$(BLOCKSIZE)
	IMAGE_SIZE := 15616k
	IMAGES += factory.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | openmesh-image ce_type=A42
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-rootfs | sysupgrade-tar rootfs=$$$$@ | append-metadata
endef
TARGET_DEVICES += openmesh_a42

define Device/openmesh_a62
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := OpenMesh
	DEVICE_MODEL := A62
	SOC := qcom-ipq4019
	DEVICE_DTS_CONFIG := config@om.a62
	BLOCKSIZE := 64k
	KERNEL = kernel-bin | lzma | fit lzma $$(DTS_DIR)/$$(DEVICE_DTS).dtb | pad-to $$(BLOCKSIZE)
	IMAGE_SIZE := 15552k
	IMAGES += factory.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | openmesh-image ce_type=A62
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-rootfs | sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += openmesh_a62

define Device/p2w_r619ac
	$(call Device/FitzImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := P&W
	DEVICE_MODEL := R619AC
	SOC := qcom-ipq4019
	DEVICE_DTS_CONFIG := config@10
	BLOCKSIZE := 128k
	PAGESIZE := 2048
endef

define Device/p2w_r619ac-64m
	$(call Device/p2w_r619ac)
	DEVICE_VARIANT := 64M NAND
	IMAGES += nand-factory.bin
	IMAGE/nand-factory.bin := append-ubi | qsdk-ipq-factory-nand
endef
TARGET_DEVICES += p2w_r619ac-64m

define Device/p2w_r619ac-128m
	$(call Device/p2w_r619ac)
	DEVICE_VARIANT := 128M NAND
endef
TARGET_DEVICES += p2w_r619ac-128m

define Device/plasmacloud_pa1200
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Plasma Cloud
	DEVICE_MODEL := PA1200
	SOC := qcom-ipq4018
	DEVICE_DTS_CONFIG := config@pc.pa1200
	BLOCKSIZE := 64k
	KERNEL = kernel-bin | lzma | fit lzma $$(DTS_DIR)/$$(DEVICE_DTS).dtb | pad-to $$(BLOCKSIZE)
	IMAGE_SIZE := 15616k
	IMAGES += factory.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | openmesh-image ce_type=PA1200
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-rootfs | sysupgrade-tar rootfs=$$$$@ | append-metadata
endef
TARGET_DEVICES += plasmacloud_pa1200

define Device/plasmacloud_pa2200
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Plasma Cloud
	DEVICE_MODEL := PA2200
	SOC := qcom-ipq4019
	DEVICE_DTS_CONFIG := config@pc.pa2200
	BLOCKSIZE := 64k
	KERNEL = kernel-bin | lzma | fit lzma $$(DTS_DIR)/$$(DEVICE_DTS).dtb | pad-to $$(BLOCKSIZE)
	IMAGE_SIZE := 15552k
	IMAGES += factory.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | openmesh-image ce_type=PA2200
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-rootfs | sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += plasmacloud_pa2200

define Device/qcom_ap-dk01.1-c1
	DEVICE_VENDOR := Qualcomm Atheros
	DEVICE_MODEL := AP-DK01.1
	DEVICE_VARIANT := C1
	BOARD_NAME := ap-dk01.1-c1
	SOC := qcom-ipq4019
	DEVICE_DTS := qcom-ipq4019-ap.dk01.1-c1
	KERNEL_INSTALL := 1
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 26624k
	$(call Device/FitImage)
	IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-rootfs | pad-rootfs | append-metadata
	DEFAULT := n
endef
TARGET_DEVICES += qcom_ap-dk01.1-c1

define Device/qcom_ap-dk04.1-c1
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Qualcomm Atheros
	DEVICE_MODEL := AP-DK04.1
	DEVICE_VARIANT := C1
	SOC := qcom-ipq4019
	DEVICE_DTS := qcom-ipq4019-ap.dk04.1-c1
	KERNEL_INSTALL := 1
	KERNEL_SIZE := 4048k
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	BOARD_NAME := ap-dk04.1-c1
	DEFAULT := n
endef
TARGET_DEVICES += qcom_ap-dk04.1-c1

define Device/qxwlan_e2600ac-c1
	$(call Device/FitImage)
	DEVICE_VENDOR := Qxwlan
	DEVICE_MODEL := E2600AC
	DEVICE_VARIANT := C1
	BOARD_NAME := e2600ac-c1
	SOC := qcom-ipq4019
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 31232k
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEFAULT := n
endef
TARGET_DEVICES += qxwlan_e2600ac-c1

define Device/qxwlan_e2600ac-c2
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Qxwlan
	DEVICE_MODEL := E2600AC
	DEVICE_VARIANT := C2
	SOC := qcom-ipq4019
	KERNEL_INSTALL := 1
	BLOCKSIZE := 128k
	PAGESIZE := 2048
endef
TARGET_DEVICES += qxwlan_e2600ac-c2

define Device/teltonika_rutx10
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Teltonika
	DEVICE_MODEL := RUTX10
	SOC := qcom-ipq4018
	DEVICE_DTS_CONFIG := config@5
	KERNEL_INSTALL := 1
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	FILESYSTEMS := squashfs
	IMAGE/nand-factory.ubi := append-ubi | qsdk-ipq-factory-nand | append-rutx-metadata
	DEVICE_PACKAGES := kmod-bluetooth
endef
TARGET_DEVICES += teltonika_rutx10

define Device/tel_x1pro
	$(call Device/FitImage)
	DEVICE_VENDOR := Telco
	DEVICE_MODEL := X1 Pro
	SOC := qcom-ipq4019
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 31232k
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEVICE_PACKAGES := kmod-usb-net-qmi-wwan kmod-usb-serial-option uqmi
	DEFAULT := n
endef
TARGET_DEVICES += tel_x1pro

define Device/unielec_u4019-32m
	$(call Device/FitImage)
	DEVICE_VENDOR := Unielec
	DEVICE_MODEL := U4019
	DEVICE_VARIANT := 32M
	BOARD_NAME := u4019-32m
	SOC := qcom-ipq4019
	KERNEL_SIZE := 4096k
	IMAGE_SIZE := 31232k
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	DEFAULT := n
endef
TARGET_DEVICES += unielec_u4019-32m

define Device/zte_mf28x_common
	$(call Device/FitzImage)
	DEVICE_VENDOR := ZTE
	SOC := qcom-ipq4019
	DEVICE_DTS_CONFIG := config@ap.dk04.1-c1
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	KERNEL_IN_UBI := 1
	DEVICE_PACKAGES := kmod-usb-net-qmi-wwan kmod-usb-serial-option uqmi
endef

define Device/zte_mf286d
	$(call Device/zte_mf28x_common)
	DEVICE_MODEL := MF286D
endef
TARGET_DEVICES += zte_mf286d

define Device/zte_mf289f
	$(call Device/zte_mf28x_common)
	DEVICE_MODEL := MF289F
	DEVICE_PACKAGES += ipq-wifi-zte_mf289f ath10k-firmware-qca9984-ct
endef
TARGET_DEVICES += zte_mf289f

define Device/zyxel_nbg6617
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := ZyXEL
	DEVICE_MODEL := NBG6617
	SOC := qcom-ipq4018
	KERNEL_SIZE := 4096k
	ROOTFS_SIZE := 24960k
	RAS_BOARD := NBG6617
	RAS_ROOTFS_SIZE := 19840k
	RAS_VERSION := "$(VERSION_DIST) $(REVISION)"
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata
	IMAGES += factory.bin
#	The ZyXEL firmware allows flashing thru the web-gui only when the rootfs is
#	at least as large as the one of the initial firmware image (not the current
#	one on the device). This only applies to the Web-UI, the bootlaoder ignores
#	this minimum-size. However, the larger image can be flashed both ways.
	IMAGE/factory.bin := append-rootfs | pad-rootfs | pad-to 64k | check-size $$$$(ROOTFS_SIZE) | zyxel-ras-image separate-kernel
	IMAGE/sysupgrade.bin/squashfs := append-rootfs | pad-rootfs | check-size $$$$(ROOTFS_SIZE) | sysupgrade-tar rootfs=$$$$@ | append-metadata
	DEVICE_PACKAGES := kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += zyxel_nbg6617

define Device/zyxel_wre6606
	$(call Device/FitImage)
	DEVICE_VENDOR := ZyXEL
	DEVICE_MODEL := WRE6606
	DEVICE_DTS_CONFIG := config@4
	SOC := qcom-ipq4018
	IMAGE_SIZE := 13184k
	IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | check-size | append-metadata
	DEVICE_PACKAGES := -kmod-ath10k-ct kmod-ath10k-ct-smallbuffers
endef
TARGET_DEVICES += zyxel_wre6606
