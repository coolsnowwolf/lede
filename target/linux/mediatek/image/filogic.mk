DTS_DIR := $(DTS_DIR)/mediatek

define Image/Prepare
	# For UBI we want only one extra block
	rm -f $(KDIR)/ubi_mark
	echo -ne '\xde\xad\xc0\xde' > $(KDIR)/ubi_mark
endef

define Build/bl2
	cat $(STAGING_DIR_IMAGE)/mt7986-$1-bl2.img >> $@
endef

define Build/bl31-uboot
	cat $(STAGING_DIR_IMAGE)/mt7986_$1-u-boot.fip >> $@
endef

define Build/mt7986-gpt
	cp $@ $@.tmp 2>/dev/null || true
	ptgen -g -o $@.tmp -a 1 -l 1024 \
		$(if $(findstring sdmmc,$1), \
			-H \
			-t 0x83	-N bl2		-r	-p 4079k@17k \
		) \
			-t 0x83	-N ubootenv	-r	-p 512k@4M \
			-t 0x83	-N factory	-r	-p 2M@4608k \
			-t 0xef	-N fip		-r	-p 4M@6656k \
				-N recovery	-r	-p 32M@12M \
		$(if $(findstring sdmmc,$1), \
				-N install	-r	-p 20M@44M \
			-t 0x2e -N production		-p $(CONFIG_TARGET_ROOTFS_PARTSIZE)M@64M \
		) \
		$(if $(findstring emmc,$1), \
			-t 0x2e -N production		-p $(CONFIG_TARGET_ROOTFS_PARTSIZE)M@64M \
		)
	cat $@.tmp >> $@
	rm $@.tmp
endef

define Device/asus_tuf-ax4200
  DEVICE_VENDOR := ASUS
  DEVICE_MODEL := TUF-AX4200
  DEVICE_DTS := mt7986a-asus-tuf-ax4200
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS_LOADADDR := 0x47000000
  DEVICE_PACKAGES := kmod-usb3 kmod-mt7986-firmware mt7986-wo-firmware
  IMAGES := sysupgrade.bin
  KERNEL := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += asus_tuf-ax4200


define Device/bananapi_bpi-r3
  DEVICE_VENDOR := Bananapi
  DEVICE_MODEL := BPi-R3
  DEVICE_DTS := mt7986a-bananapi-bpi-r3
  DEVICE_DTS_CONFIG := config-mt7986a-bananapi-bpi-r3
  DEVICE_DTS_OVERLAY:= mt7986a-bananapi-bpi-r3-nor mt7986a-bananapi-bpi-r3-emmc-nor mt7986a-bananapi-bpi-r3-emmc-snand mt7986a-bananapi-bpi-r3-snand
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-hwmon-pwmfan kmod-i2c-gpio kmod-mt7986-firmware kmod-sfp kmod-usb3 e2fsprogs f2fsck mkf2fs mt7986-wo-firmware
  IMAGES := sysupgrade.itb
  KERNEL_LOADADDR := 0x44000000
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  ARTIFACTS := \
	       emmc-preloader.bin emmc-bl31-uboot.fip \
	       nor-preloader.bin nor-bl31-uboot.fip \
	       sdcard.img.gz \
	       snand-preloader.bin snand-bl31-uboot.fip
  ARTIFACT/emmc-preloader.bin	:= bl2 emmc-ddr4
  ARTIFACT/emmc-bl31-uboot.fip	:= bl31-uboot bananapi_bpi-r3-emmc
  ARTIFACT/nor-preloader.bin	:= bl2 nor-ddr4
  ARTIFACT/nor-bl31-uboot.fip	:= bl31-uboot bananapi_bpi-r3-nor
  ARTIFACT/snand-preloader.bin	:= bl2 spim-nand-ddr4
  ARTIFACT/snand-bl31-uboot.fip	:= bl31-uboot bananapi_bpi-r3-snand
  ARTIFACT/sdcard.img.gz	:= mt7986-gpt sdmmc |\
				   pad-to 17k | bl2 sdmmc-ddr4 |\
				   pad-to 6656k | bl31-uboot bananapi_bpi-r3-sdmmc |\
				$(if $(CONFIG_TARGET_ROOTFS_INITRAMFS),\
				   pad-to 12M | append-image-stage initramfs-recovery.itb | check-size 44m |\
				) \
				   pad-to 44M | bl2 spim-nand-ddr4 |\
				   pad-to 45M | bl31-uboot bananapi_bpi-r3-snand |\
				   pad-to 49M | bl2 nor-ddr4 |\
				   pad-to 50M | bl31-uboot bananapi_bpi-r3-nor |\
				   pad-to 51M | bl2 emmc-ddr4 |\
				   pad-to 52M | bl31-uboot bananapi_bpi-r3-emmc |\
				   pad-to 56M | mt7986-gpt emmc |\
				$(if $(CONFIG_TARGET_ROOTFS_SQUASHFS),\
				   pad-to 64M | append-image squashfs-sysupgrade.itb | check-size | gzip \
				)
  IMAGE_SIZE := $$(shell expr 64 + $$(CONFIG_TARGET_ROOTFS_PARTSIZE))m
  KERNEL			:= kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  IMAGE/sysupgrade.itb := append-kernel | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-static-with-rootfs | pad-rootfs | append-metadata
  DTC_FLAGS += -@ --space 32768
endef
TARGET_DEVICES += bananapi_bpi-r3

define Device/glinet_gl-mt3000
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-MT3000
  DEVICE_DTS := mt7981b-glinet-gl-mt3000
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-mt7981-firmware mt7981-wo-firmware kmod-hwmon-pwmfan kmod-usb3
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += glinet_gl-mt3000

define Device/h3c_magic-nx30-pro
  DEVICE_VENDOR := H3C
  DEVICE_MODEL := Magic NX30 Pro
  DEVICE_DTS := mt7981b-h3c-magic-nx30-pro
  DEVICE_DTS_DIR := ../dts
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  IMAGE_SIZE := 65536k
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  DEVICE_PACKAGES := kmod-mt7981-firmware mt7981-wo-firmware
endef
TARGET_DEVICES += h3c_magic-nx30-pro

define Device/mediatek_mt7986a-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7986 rfba AP
  DEVICE_DTS := mt7986a-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/
  DEVICE_DTS_OVERLAY := mt7986a-rfb-spim-nand mt7986a-rfb-spim-nor
  SUPPORTED_DEVICES := mediatek,mt7986a-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  KERNEL = kernel-bin | lzma | \
	fit lzma $$(KDIR)/$$(firstword $$(DEVICE_DTS)).dtb
  KERNEL_INITRAMFS = kernel-bin | lzma | \
	fit lzma $$(KDIR)/$$(firstword $$(DEVICE_DTS)).dtb with-initrd
  DTC_FLAGS += -@ --space 32768
endef
TARGET_DEVICES += mediatek_mt7986a-rfb

define Device/mediatek_mt7986b-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7986 rfbb AP
  DEVICE_DTS := mt7986b-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/
  DEVICE_PACKAGES := kmod-mt7986-firmware mt7986-wo-firmware
  SUPPORTED_DEVICES := mediatek,mt7986b-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mediatek_mt7986b-rfb

define Device/jcg_q30_pro
  DEVICE_VENDOR := JCG
  DEVICE_MODEL := Q30 Pro
  DEVICE_DTS := mt7981b-jcg-q30_pro
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-mt7981-firmware mt7981-wo-firmware
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 113152k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  KERNEL = kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  KERNEL_INITRAMFS = kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd
endef
TARGET_DEVICES += jcg_q30_pro

define Device/qihoo_360t7
  DEVICE_VENDOR := Qihoo
  DEVICE_MODEL := 360T7
  DEVICE_DTS := mt7981b-qihoo-360t7
  DEVICE_DTS_DIR := ../dts
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  DEVICE_PACKAGES := kmod-mt7981-firmware mt7981-wo-firmware
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += qihoo_360t7

define Device/tplink_tl-common
  DEVICE_VENDOR := TP-Link
  DEVICE_DTS_DIR := ../dts
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  DEVICE_PACKAGES := kmod-usb3 kmod-mt7986-firmware mt7986-wo-firmware
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/tplink_tl-xdr4288
  DEVICE_MODEL := TL-XDR4288
  DEVICE_DTS := mt7986a-tplink-tl-xdr4288
  $(call Device/tplink_tl-common)
endef
TARGET_DEVICES += tplink_tl-xdr4288

define Device/tplink_tl-xdr6086
  DEVICE_MODEL := TL-XDR6086
  DEVICE_DTS := mt7986a-tplink-tl-xdr6086
  $(call Device/tplink_tl-common)
endef
TARGET_DEVICES += tplink_tl-xdr6086

define Device/tplink_tl-xdr6088
  DEVICE_MODEL := TL-XDR6088
  DEVICE_DTS := mt7986a-tplink-tl-xdr6088
  $(call Device/tplink_tl-common)
endef
TARGET_DEVICES += tplink_tl-xdr6088

define Device/xiaomi_mi-router-wr30u
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router WR30U
  DEVICE_DTS := mt7981b-xiaomi-wr30u
  DEVICE_DTS_DIR := ../dts
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  DEVICE_PACKAGES := kmod-mt7981-firmware mt7981-wo-firmware
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += xiaomi_mi-router-wr30u

define Device/xiaomi_redmi-router-ax6000
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Redmi Router AX6000
  DEVICE_DTS := mt7986a-xiaomi-redmi-router-ax6000
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-leds-ws2812b kmod-mt7986-firmware mt7986-wo-firmware
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += xiaomi_redmi-router-ax6000
