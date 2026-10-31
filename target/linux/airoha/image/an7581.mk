define Build/an7581-emmc-bl2-bl31-uboot
  head -c $$((0x800)) /dev/zero > $@
  cat $(STAGING_DIR_IMAGE)/an7581_$1-bl2.fip >> $@
  dd if=$(STAGING_DIR_IMAGE)/an7581_$1-bl31-u-boot.fip of=$@ bs=1 seek=$$((0x20000)) conv=notrunc
endef

define Build/an7581-preloader
  $(STAGING_DIR_HOST)/bin/fiptool create \
		--tb-fw $(STAGING_DIR_IMAGE)/an7581-bl2.bin \
		$(STAGING_DIR_IMAGE)/an7581_$1-bl2.fip
  cat $(STAGING_DIR_IMAGE)/an7581_$1-bl2.fip >> $@
endef

define Build/an7581-bl31-uboot
  $(STAGING_DIR_HOST)/bin/fiptool create \
		--soc-fw $(STAGING_DIR_IMAGE)/an7581-bl31.lzma \
		--nt-fw $(STAGING_DIR_IMAGE)/an7581_$1-u-boot.lzma \
		$(STAGING_DIR_IMAGE)/an7581_$1-bl31-u-boot.fip
  cat $(STAGING_DIR_IMAGE)/an7581_$1-bl31-u-boot.fip >> $@
endef

define Device/FitImageLzma
	KERNEL_SUFFIX := -uImage.itb
	KERNEL = kernel-bin | lzma | fit lzma $$(KDIR)/image-$$(DEVICE_DTS).dtb
	KERNEL_NAME := Image
endef

define Device/airoha_an7581-evb
  $(call Device/FitImageLzma)
  DEVICE_VENDOR := Airoha
  DEVICE_MODEL := AN7581 Evaluation Board (SNAND)
  DEVICE_PACKAGES := kmod-leds-pwm kmod-pwm-airoha kmod-input-gpio-keys-polled
  DEVICE_DTS := an7581-evb
  DEVICE_DTS_CONFIG := config@1
  IMAGE/sysupgrade.bin := append-kernel | pad-to 128k | append-rootfs | pad-rootfs | append-metadata
  ARTIFACT/preloader.bin := an7581-preloader rfb
  ARTIFACT/bl31-uboot.fip := an7581-bl31-uboot rfb
  ARTIFACTS := preloader.bin bl31-uboot.fip
endef
TARGET_DEVICES += airoha_an7581-evb

define Device/airoha_an7581-evb-emmc-eagle
  DEVICE_VENDOR := Airoha
  DEVICE_MODEL := AN7581 Evaluation Board (eMMC + Eagle)
  DEVICE_DTS := an7581-evb-emmc-eagle
  DEVICE_PACKAGES := airoha-en7581-mt7996-npu-firmware \
		    kmod-mt7996-firmware wpad-basic-mbedtls
  ARTIFACT/preloader.bin := an7581-preloader rfb
  ARTIFACT/bl31-uboot.fip := an7581-bl31-uboot rfb
  ARTIFACTS := preloader.bin bl31-uboot.fip
endef
TARGET_DEVICES += airoha_an7581-evb-emmc-eagle

define Device/airoha_an7581-evb-emmc-kite
  DEVICE_VENDOR := Airoha
  DEVICE_MODEL := AN7581 Evaluation Board (eMMC + Kite)
  DEVICE_DTS := an7581-evb-emmc-kite
  DEVICE_PACKAGES := airoha-en7581-npu-firmware \
		    kmod-mt7992-firmware wpad-basic-mbedtls
  ARTIFACT/preloader.bin := an7581-preloader rfb
  ARTIFACT/bl31-uboot.fip := an7581-bl31-uboot rfb
  ARTIFACTS := preloader.bin bl31-uboot.fip
endef
TARGET_DEVICES += airoha_an7581-evb-emmc-kite

define Device/nokia_valyrian
  DEVICE_VENDOR := Nokia
  DEVICE_MODEL := Valyrian
  DEVICE_DTS := an7581-nokia-valyrian
  DEVICE_PACKAGES := kmod-spi-gpio kmod-gpio-nxp-74hc164 kmod-leds-gpio \
    kmod-i2c-gpio kmod-iio-richtek-rtq6056 \
    kmod-sfp kmod-phy-aeonsemi-as21xxx \
    kmod-mt7996-firmware airoha-en7581-mt7996-npu-firmware \
    kmod-usb3
endef
TARGET_DEVICES += nokia_valyrian

define Device/nokia_xg-040g-md-common
  $(call Device/FitImageLzma)
  DEVICE_VENDOR := Nokia
  DEVICE_MODEL := XG-040G-MD
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBINIZE_OPTS := -E 5
  DEVICE_PACKAGES := kmod-gpio-button-hotplug kmod-leds-gpio \
	kmod-phy-airoha-en8811h kmod-regulator-userspace-consumer \
	kmod-usb-ledtrig-usbport kmod-usb3 kmod-airoha-en7572 \
	kmod-airoha-xpon airoha-ponctl airoha-pond luci-app-pon
endef

define Device/nokia_xg-040g-ubi-images
  UBOOTENV_IN_UBI := 1
  KERNEL_IN_UBI := 1
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(DEVICE_DTS).dtb with-initrd | pad-to 128k
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  IMAGES := sysupgrade.itb
  IMAGE/sysupgrade.itb := append-kernel | \
	fit gzip $$(KDIR)/image-$$(DEVICE_DTS).dtb external-static-with-rootfs | \
	append-metadata
  DEVICE_PACKAGES += fitblk
endef

define Device/nokia_xg-040g-md-ubi
  $(call Device/nokia_xg-040g-md-common)
  DEVICE_VARIANT := (UBI)
  DEVICE_DTS := an7581-nokia_xg-040g-md-ubi
  $(call Device/nokia_xg-040g-ubi-images)
endef
TARGET_DEVICES += nokia_xg-040g-md-ubi

define Device/nokia_xg-040g-tf-ubi
  $(call Device/nokia_xg-040g-md-common)
  DEVICE_MODEL := XG-040G-TF
  DEVICE_VARIANT := (UBI)
  DEVICE_DTS := an7581-nokia_xg-040g-tf-ubi
  DEVICE_PACKAGES += -kmod-regulator-userspace-consumer \
	-kmod-usb-ledtrig-usbport -kmod-usb3
  $(call Device/nokia_xg-040g-ubi-images)
endef
TARGET_DEVICES += nokia_xg-040g-tf-ubi

# HG5585F variants share parallel NAND, PON, MT7916D, and dual USB.
define Device/fiberhome_hg5585f-common
  $(call Device/FitImageLzma)
  DEVICE_VENDOR := FiberHome
  KERNEL_LOADADDR := 0x8a000000
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBOOTENV_IN_UBI := 1
  KERNEL_IN_UBI := 1
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  IMAGES := sysupgrade.itb
  # PON and MT7916D read per-device calibration from factory UBI NVMEM cells.
  DEVICE_PACKAGES := kmod-gpio-button-hotplug kmod-leds-gpio kmod-usb3 \
    kmod-airoha-paged-bosa kmod-airoha-xpon airoha-ponctl airoha-pond \
	 kmod-mt7915e kmod-mt7916-firmware wpad-openssl \
	 luci-app-pon fitblk nand-utils ubi-utils kmod-usb-ledtrig-usbport
endef

# Expand FIT rules after DEVICE_DTS so each variant embeds its own DTB.
define Device/fiberhome_hg5585f-images
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(DEVICE_DTS).dtb with-initrd | pad-to 128k
  IMAGE/sysupgrade.itb := append-kernel | \
	fit gzip $$(KDIR)/image-$$(DEVICE_DTS).dtb external-static-with-rootfs | \
	append-metadata
endef

define Device/fiberhome_hg5585f-ct
  $(call Device/fiberhome_hg5585f-common)
  DEVICE_MODEL := HG5585F
  DEVICE_VARIANT := CT
  DEVICE_DTS := an7581-fiberhome-hg5585f-ct
  DEVICE_PACKAGES += kmod-phy-maxlinear
  $(call Device/fiberhome_hg5585f-images)
endef
TARGET_DEVICES += fiberhome_hg5585f-ct

define Device/fiberhome_hg5585f-cu
  $(call Device/fiberhome_hg5585f-common)
  DEVICE_MODEL := HG5585F
  DEVICE_VARIANT := CU
  DEVICE_DTS := an7581-fiberhome-hg5585f-cu
  $(call Device/fiberhome_hg5585f-images)
endef
TARGET_DEVICES += fiberhome_hg5585f-cu
