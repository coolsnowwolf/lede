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
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS_CONFIG := config@1
  KERNEL_LOADADDR := 0x80088000
  IMAGE/sysupgrade.bin := append-kernel | pad-to 128k | append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += airoha_an7581-evb

define Device/airoha_an7581-evb-emmc
  DEVICE_VENDOR := Airoha
  DEVICE_MODEL := AN7581 Evaluation Board (EMMC)
  DEVICE_DTS := an7581-evb-emmc
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-i2c-an7581
endef
TARGET_DEVICES += airoha_an7581-evb-emmc


define Device/bell_xg-040g-md
  $(call Device/FitImageLzma)
  DEVICE_VENDOR := Nokia Bell
  DEVICE_MODEL := XG-040G-MD
  DEVICE_DTS := an7581-xg-040g-md
  DEVICE_DTS_CONFIG := config@1
  DEVICE_PACKAGES := airoha-en7581-npu-firmware kmod-phy-airoha-en8811h kmod-usb3 kmod-usb-xhci-mtk kmod-i2c-an7581 kmod-input-gpio-keys-polled 
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_LOADADDR := 0x80088000
  KERNEL_IN_UBI := 1
  KERNEL_SIZE := 5120k
  SOC := an7581
  UBINIZE_OPTS := -s 2048
  IMAGE_SIZE := 261120k
  IMAGES += factory.bin sysupgrade.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += bell_xg-040g-md

define Device/bell_xg-140g-md
  $(call Device/FitImageLzma)
  DEVICE_VENDOR := Nokia Bell
  DEVICE_MODEL := XG-140G-MD
  DEVICE_DTS := an7581-xg-140g-md
  DEVICE_DTS_CONFIG := config@1
  DEVICE_PACKAGES := airoha-en7581-npu-firmware kmod-phy-airoha-en8811h kmod-usb3 kmod-usb-xhci-mtk kmod-i2c-an7581 kmod-input-gpio-keys-polled 
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_LOADADDR := 0x80088000
  KERNEL_IN_UBI := 1
  KERNEL_SIZE := 5120k
  SOC := an7581
  UBINIZE_OPTS := -s 2048
  IMAGE_SIZE := 261120k
  IMAGES += factory.bin sysupgrade.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += bell_xg-140g-md

define Device/bell_xg-040g-tf
  $(call Device/FitImageLzma)
  DEVICE_VENDOR := Nokia Bell
  DEVICE_MODEL := XG-040G-TF
  DEVICE_DTS := an7581-xg-040g-tf
  DEVICE_DTS_CONFIG := config@1
  DEVICE_PACKAGES := airoha-en7581-npu-firmware kmod-phy-airoha-en8811h kmod-i2c-an7581 kmod-input-gpio-keys-polled 
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_LOADADDR := 0x80088000
  KERNEL_IN_UBI := 1
  KERNEL_SIZE := 5120k
  SOC := an7581
  UBINIZE_OPTS := -s 2048
  IMAGE_SIZE := 261120k
  IMAGES += factory.bin sysupgrade.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += bell_xg-040g-tf


define Device/bell_xg-140g-tf
  $(call Device/FitImageLzma)
  DEVICE_VENDOR := Nokia Bell
  DEVICE_MODEL := XG-140G-TF
  DEVICE_DTS := an7581-xg-140g-tf
  DEVICE_DTS_CONFIG := config@1
  DEVICE_PACKAGES := airoha-en7581-npu-firmware kmod-phy-airoha-en8811h kmod-i2c-an7581 kmod-input-gpio-keys-polled 
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_LOADADDR := 0x80088000
  KERNEL_IN_UBI := 1
  KERNEL_SIZE := 5120k
  SOC := an7581
  UBINIZE_OPTS := -s 2048
  IMAGE_SIZE := 261120k
  IMAGES += factory.bin sysupgrade.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += bell_xg-140g-tf
