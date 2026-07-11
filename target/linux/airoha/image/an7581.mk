define Build/an7581-emmc-bl2-bl31-uboot
  head -c $$((0x800)) /dev/zero > $@
  cat $(STAGING_DIR_IMAGE)/an7581_$1-bl2.fip >> $@
  dd if=$(STAGING_DIR_IMAGE)/an7581_$1-bl31-u-boot.fip of=$@ bs=1 seek=$$((0x20000)) conv=notrunc
endef

define Build/an7581-preloader
  cat $(STAGING_DIR_IMAGE)/an7581_$1-bl2.fip >> $@
endef

define Build/an7581-bl31-uboot
  cat $(STAGING_DIR_IMAGE)/an7581_$1-bl31-u-boot.fip >> $@
endef

define Build/an7581-chainloader
  $(INSTALL_DIR) $(KDIR)/chainload-fit-$(notdir $@)
  @if [ -f "$(STAGING_DIR_IMAGE)/an7581_$1-u-boot.bin.lzma" ]; then \
    KERNEL="$(STAGING_DIR_IMAGE)/an7581_$1-u-boot.bin.lzma"; \
    COMP="lzma"; \
  else \
    KERNEL="$(STAGING_DIR_IMAGE)/an7581_$1-u-boot.bin"; \
    COMP="none"; \
  fi; \
  $(TOPDIR)/scripts/mkits.sh \
    -D $(DEVICE_NAME) \
    -o $(KDIR)/chainload-fit-$(notdir $@)/u-boot.its \
    -k $$KERNEL \
    -C $$COMP \
    -a 0x80200000 -e 0x80200000 \
    -c conf-uboot \
    -A arm64 -v u-boot \
    -d $(STAGING_DIR_IMAGE)/an7581_$1-u-boot.dtb \
    -s 0x82000000
  PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) \
    $(STAGING_DIR_HOST)/bin/mkimage \
    -D "-i $(KDIR)/chainload-fit-$(notdir $@)" \
    -f $(KDIR)/chainload-fit-$(notdir $@)/u-boot.its \
    $(STAGING_DIR_IMAGE)/an7581_$1-chainload-u-boot.itb
  cat $(STAGING_DIR_IMAGE)/an7581_$1-chainload-u-boot.itb >> $@
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
  DEVICE_PACKAGES := kmod-leds-pwm kmod-i2c-an7581 kmod-pwm-airoha kmod-input-gpio-keys-polled
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
  DEVICE_PACKAGES := kmod-i2c-an7581 airoha-en7581-mt7996-npu-firmware \
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
  DEVICE_PACKAGES := kmod-i2c-an7581 airoha-en7581-npu-firmware \
		    kmod-mt7992-firmware wpad-basic-mbedtls
  ARTIFACT/preloader.bin := an7581-preloader rfb
  ARTIFACT/bl31-uboot.fip := an7581-bl31-uboot rfb
  ARTIFACTS := preloader.bin bl31-uboot.fip
endef
TARGET_DEVICES += airoha_an7581-evb-emmc-kite

define Device/gemtek_w1700k-ubi
  DEVICE_VENDOR := Gemtek
  DEVICE_MODEL := W1700K
  DEVICE_VARIANT := UBI
  DEVICE_ALT0_VENDOR := CenturyLink
  DEVICE_ALT0_MODEL := W1700K
  DEVICE_ALT0_VARIANT := UBI
  DEVICE_ALT1_VENDOR := Lumen
  DEVICE_ALT1_MODEL := W1700K
  DEVICE_ALT1_VARIANT := UBI
  DEVICE_ALT2_VENDOR := Quantum Fiber
  DEVICE_ALT2_MODEL := W1700K
  DEVICE_ALT2_VARIANT := UBI
  DEVICE_DTS := an7581-w1700k-ubi
  DEVICE_COMPAT_VERSION := 2.0
  DEVICE_COMPAT_MESSAGE := Partition table has been changed to cooperate \
       with the vendor bootloader with regard to the BMT/BBT partition at \
       the end of flash. A reinstall including corrected chainloader is needed.
  DEVICE_PACKAGES := airoha-en7581-mt7996-npu-firmware fitblk kmod-i2c-an7581 \
		    kmod-hwmon-nct7802 kmod-mt7996-firmware wpad-basic-mbedtls \
		    rtl826x-firmware
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBOOTENV_IN_UBI := 1
  KERNEL_IN_UBI := 1
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 128k
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  IMAGES := sysupgrade.itb
  IMAGE/sysupgrade.itb := append-kernel | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-static-with-rootfs | append-metadata
  ARTIFACTS := chainload-uboot.itb
  ARTIFACT/chainload-uboot.itb := an7581-chainloader gemtek_w1700k
  SOC := an7581
endef
TARGET_DEVICES += gemtek_w1700k-ubi

define Device/nokia_valyrian
  DEVICE_VENDOR := Nokia
  DEVICE_MODEL := Valyrian
  DEVICE_DTS := an7581-nokia-valyrian
  DEVICE_PACKAGES := kmod-spi-gpio kmod-gpio-nxp-74hc164 kmod-leds-gpio \
    kmod-i2c-an7581 kmod-i2c-gpio kmod-iio-richtek-rtq6056 \
    kmod-sfp kmod-phy-aeonsemi-as21xxx \
    kmod-mt7996-firmware airoha-en7581-mt7996-npu-firmware \
    kmod-usb3
  ARTIFACT/preloader.bin := an7581-preloader nokia_valyrian
  ARTIFACT/bl31-uboot.fip := an7581-bl31-uboot nokia_valyrian
  ARTIFACTS := preloader.bin bl31-uboot.fip
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
	kmod-usb-ledtrig-usbport kmod-usb3
endef

define Device/nokia_xg-040g-md
  $(call Device/nokia_xg-040g-md-common)
  DEVICE_DTS := an7581-nokia_xg-040g-md
  DEVICE_DTS_CONFIG := config@1
  IMAGE_SIZE := 131968k
  KERNEL_SIZE := 8192k
  IMAGES += factory-kernel.bin factory-rootfs.bin
  IMAGE/factory-kernel.bin := append-kernel
  IMAGE/factory-rootfs.bin := append-ubi | check-size
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += nokia_xg-040g-md

define Device/nokia_xg-040g-md-ubi
  $(call Device/nokia_xg-040g-md-common)
  DEVICE_VARIANT := (UBI)
  DEVICE_DTS := an7581-nokia_xg-040g-md-ubi
  UBOOTENV_IN_UBI := 1
  KERNEL_IN_UBI := 1
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS := kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 128k
  KERNEL_INITRAMFS_SUFFIX := -recovery.itb
  IMAGES := sysupgrade.itb
  IMAGE/sysupgrade.itb := append-kernel | \
	fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-static-with-rootfs | \
	append-metadata
  DEVICE_PACKAGES += fitblk
  ARTIFACT/bl31-uboot.fip := an7581-bl31-uboot nokia_xg-040g-md
  ARTIFACT/preloader.bin := an7581-preloader nokia_xg-040g-md
  ARTIFACTS := bl31-uboot.fip preloader.bin
endef
TARGET_DEVICES += nokia_xg-040g-md-ubi
