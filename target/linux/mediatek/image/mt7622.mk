DTS_DIR := $(DTS_DIR)/mediatek

ifdef CONFIG_LINUX_5_4
  KERNEL_LOADADDR := 0x44080000
else
  KERNEL_LOADADDR := 0x44000000
endif

define Image/Prepare
	# For UBI we want only one extra block
	rm -f $(KDIR)/ubi_mark
	echo -ne '\xde\xad\xc0\xde' > $(KDIR)/ubi_mark
endef

define Build/buffalo-kernel-trx
	$(eval magic=$(word 1,$(1)))
	$(eval dummy=$(word 2,$(1)))
	$(eval kern_size=$(if $(KERNEL_SIZE),$(KERNEL_SIZE),0x400000))

	$(if $(dummy),touch $(dummy))
	$(STAGING_DIR_HOST)/bin/otrx create $@.new \
		$(if $(magic),-M $(magic),) \
		-f $@ \
		$(if $(dummy),\
			-a 0x20000 \
			-b $$(( $(subst k, * 1024,$(kern_size)) )) \
			-f $(dummy),)
	mv $@.new $@
endef

define Build/bl2
	cat $(STAGING_DIR_IMAGE)/mt7622-$1-bl2.img >> $@
endef

define Build/bl31-uboot
	cat $(STAGING_DIR_IMAGE)/mt7622_$1-u-boot.fip >> $@
endef

define Build/mt7622-gpt
	cp $@ $@.tmp 2>/dev/null || true
	ptgen -g -o $@.tmp -a 1 -l 1024 \
		$(if $(findstring sdmmc,$1), \
			-H \
			-t 0x83	-N bl2		-r	-p 512k@512k \
		) \
			-t 0xef	-N fip		-r	-p 2M@2M \
			-t 0x83	-N ubootenv	-r	-p 1M@4M \
				-N recovery	-r	-p 32M@6M \
		$(if $(findstring sdmmc,$1), \
				-N install	-r	-p 7M@38M \
			-t 0x2e -N production		-p 211M@45M \
		) \
		$(if $(findstring emmc,$1), \
			-t 0x2e -N production		-p 980M@40M \
		)
	cat $@.tmp >> $@
	rm $@.tmp
endef

define Build/trx-nand
	# kernel: always use 4 MiB (-28 B or TRX header) to allow upgrades even
	#	  if it grows up between releases
	# root: UBI with one extra block containing UBI mark to trigger erasing
	#	rest of partition
	$(STAGING_DIR_HOST)/bin/otrx create $@.new \
		-M 0x32504844 \
		-f $(IMAGE_KERNEL) -a 0x20000 -b 0x400000 \
		-f $@ \
		-A $(KDIR)/ubi_mark -a 0x20000
	mv $@.new $@
endef

define Device/bananapi_bpi-r64
  DEVICE_VENDOR := Bananapi
  DEVICE_MODEL := BPi-R64
  DEVICE_DTS := mt7622-bananapi-bpi-r64
  DEVICE_DTS_OVERLAY := mt7622-bananapi-bpi-r64-pcie1 mt7622-bananapi-bpi-r64-sata
  DEVICE_PACKAGES := kmod-ata-ahci-mtk kmod-btmtkuart kmod-usb3 e2fsprogs mkf2fs f2fsck
  ARTIFACTS := emmc-preloader.bin emmc-bl31-uboot.fip sdcard.img.gz snand-preloader.bin snand-bl31-uboot.fip
  IMAGES := sysupgrade.itb
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  ARTIFACT/emmc-preloader.bin	:= bl2 emmc-2ddr
  ARTIFACT/emmc-bl31-uboot.fip	:= bl31-uboot bananapi_bpi-r64-emmc
  ARTIFACT/snand-preloader.bin	:= bl2 snand-2ddr
  ARTIFACT/snand-bl31-uboot.fip	:= bl31-uboot bananapi_bpi-r64-snand
  ARTIFACT/sdcard.img.gz	:= mt7622-gpt sdmmc |\
				   pad-to 512k | bl2 sdmmc-2ddr |\
				   pad-to 2048k | bl31-uboot bananapi_bpi-r64-sdmmc |\
				   pad-to 6144k | append-image-stage initramfs-recovery.itb |\
				   pad-to 38912k | mt7622-gpt emmc |\
				   pad-to 39424k | bl2 emmc-2ddr |\
				   pad-to 40960k | bl31-uboot bananapi_bpi-r64-emmc |\
				   pad-to 43008k | bl2 snand-2ddr |\
				   pad-to 43520k | bl31-uboot bananapi_bpi-r64-snand |\
				   pad-to 46080k | append-image squashfs-sysupgrade.itb | gzip
  KERNEL			:= kernel-bin | gzip
  KERNEL_INITRAMFS		:= kernel-bin | lzma | fit lzma $$(DTS_DIR)/$$(DEVICE_DTS).dtb with-initrd | pad-to 128k
  IMAGE/sysupgrade.itb		:= append-kernel | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb external-static-with-rootfs | append-metadata
endef
TARGET_DEVICES += bananapi_bpi-r64

define Device/buffalo_wsr-2533dhp2
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WSR-2533DHP2
  DEVICE_DTS := mt7622-buffalo-wsr-2533dhp2
  DEVICE_DTS_DIR := ../dts
  IMAGE_SIZE := 59392k
  KERNEL_SIZE := 4096k
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  UBINIZE_OPTS := -E 5
  BUFFALO_TAG_PLATFORM := MTK
  BUFFALO_TAG_VERSION := 9.99
  BUFFALO_TAG_MINOR := 9.99
  IMAGES += factory.bin factory-uboot.bin
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | \
	buffalo-kernel-trx
  IMAGE/factory.bin := append-ubi | trx-nand | \
	buffalo-enc WSR-2533DHP2 $$(BUFFALO_TAG_VERSION) -l | \
	buffalo-tag-dhp WSR-2533DHP2 JP JP | buffalo-enc-tag -l | buffalo-dhp-image
  IMAGE/factory-uboot.bin := append-ubi | trx-nand
  IMAGE/sysupgrade.bin := append-kernel | \
	buffalo-kernel-trx 0x32504844 $(KDIR)/tmp/$$(DEVICE_NAME).null | \
	sysupgrade-tar kernel=$$$$@ | append-metadata
  DEVICE_PACKAGES := swconfig
endef
TARGET_DEVICES += buffalo_wsr-2533dhp2

define Device/elecom_wrc-2533gent
  DEVICE_VENDOR := Elecom
  DEVICE_MODEL := WRC-2533GENT
  DEVICE_DTS := mt7622-elecom-wrc-2533gent
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-btmtkuart kmod-usb3 swconfig
endef
TARGET_DEVICES += elecom_wrc-2533gent

define Device/linksys_e8450
  DEVICE_VENDOR := Linksys
  DEVICE_MODEL := E8450
  DEVICE_ALT0_VENDOR := Belkin
  DEVICE_ALT0_MODEL := RT3200
  DEVICE_DTS := mt7622-linksys-e8450
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-mt7915e kmod-usb3
endef
TARGET_DEVICES += linksys_e8450

define Device/linksys_e8450-ubi
  DEVICE_VENDOR := Linksys
  DEVICE_MODEL := E8450
  DEVICE_VARIANT := UBI
  DEVICE_ALT0_VENDOR := Belkin
  DEVICE_ALT0_MODEL := RT3200
  DEVICE_ALT0_VARIANT := UBI
  DEVICE_DTS := mt7622-linksys-e8450-ubi
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-mt7915e kmod-usb3
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBOOTENV_IN_UBI := 1
  KERNEL_IN_UBI := 1
  KERNEL := kernel-bin | gzip
# recovery can also be used with stock firmware web-ui, hence the padding...
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 128k
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  IMAGES := sysupgrade.itb
  IMAGE/sysupgrade.itb := append-kernel | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-static-with-rootfs | append-metadata
  ARTIFACTS := preloader.bin bl31-uboot.fip
  ARTIFACT/preloader.bin := bl2 snand-1ddr
  ARTIFACT/bl31-uboot.fip := bl31-uboot linksys_e8450
endef
TARGET_DEVICES += linksys_e8450-ubi

define Device/mediatek_mt7622-rfb1
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7622 rfb1 AP
  DEVICE_DTS := mt7622-rfb1
  DEVICE_PACKAGES := kmod-ata-ahci-mtk kmod-btmtkuart kmod-usb3
endef
TARGET_DEVICES += mediatek_mt7622-rfb1

define Device/mediatek_mt7622-rfb1-ubi
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7622 rfb1 AP (UBI)
  DEVICE_DTS := mt7622-rfb1-ubi
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-ata-ahci-mtk kmod-btmtkuart kmod-usb3
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_SIZE := 4194304
  IMAGE_SIZE := 32768k
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | \
                check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mediatek_mt7622-rfb1-ubi

define Device/totolink_a8000ru
  DEVICE_VENDOR := TOTOLINK
  DEVICE_MODEL := A8000RU
  DEVICE_DTS := mt7622-totolink-a8000ru
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := swconfig
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += totolink_a8000ru

define Device/ubnt_unifi-6-lr
  DEVICE_VENDOR := Ubiquiti
  DEVICE_MODEL := UniFi 6 LR
  DEVICE_DTS_CONFIG := config@1
  DEVICE_DTS := mt7622-ubnt-unifi-6-lr
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-mt7915e
endef
TARGET_DEVICES += ubnt_unifi-6-lr

define Device/ubnt_unifi-6-lr-ubootmod
  DEVICE_VENDOR := Ubiquiti
  DEVICE_MODEL := UniFi 6 LR
  DEVICE_VARIANT := U-Boot mod
  DEVICE_DTS := mt7622-ubnt-unifi-6-lr-ubootmod
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-mt7915e
  KERNEL := kernel-bin | lzma
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  KERNEL_INITRAMFS := kernel-bin | lzma | fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 64k
  IMAGES := sysupgrade.itb
  IMAGE/sysupgrade.itb := append-kernel | fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-static-with-rootfs | pad-rootfs | append-metadata
  ARTIFACTS := preloader.bin bl31-uboot.fip
  ARTIFACT/preloader.bin := bl2 nor-2ddr
  ARTIFACT/bl31-uboot.fip := bl31-uboot ubnt_unifi-6-lr
endef
TARGET_DEVICES += ubnt_unifi-6-lr-ubootmod

define Device/xiaomi_redmi-router-ax6s
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Redmi Router AX6S
  DEVICE_ALT0_VENDOR := Xiaomi
  DEVICE_ALT0_MODEL := Router AX3200
  DEVICE_DTS := mt7622-xiaomi-redmi-router-ax6s
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-mt7915e
  KERNEL := kernel-bin | lzma
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd
endef
TARGET_DEVICES += xiaomi_redmi-router-ax6s