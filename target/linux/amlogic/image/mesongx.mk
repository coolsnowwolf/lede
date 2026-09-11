# SPDX-License-Identifier: GPL-2.0-only

define Device/Default
  PROFILES := Default
  KERNEL := kernel-bin
  IMAGES := sysupgrade.img.gz
  DEVICE_DTS_DIR := $(DTS_DIR)/amlogic
  DEVICE_DTS = $$(SOC)-$(subst _,-,$(1))
  UBOOT_DEVICE_NAME :=
  IMAGE/sysupgrade.img.gz = boot-$$(if $$(UBOOT_DEVICE_NAME),common,aml) | boot-combined-script | aml-img | gzip | append-metadata
endef

define Device/phicomm_n1
  DEVICE_VENDOR := Phicomm
  DEVICE_MODEL := N1
  SOC := meson-gxl-s905d
  UBOOT_DEVICE_NAME := phicomm-n1
  DEVICE_PACKAGES := kmod-brcmfmac brcmfmac-firmware-43455-sdio-phicomm-n1 wpad-openssl
endef
TARGET_DEVICES += phicomm_n1

define Device/onethingcloud_oes
  DEVICE_VENDOR := OneThing Cloud
  DEVICE_MODEL := OES
  SOC := meson-g12b-a311d
  DEVICE_DTS := meson-g12b-a311d-oes
  DEVICE_PACKAGES := ethtool kmod-phy-realtek kmod-amlogic-gx-mali-bifrost \
	kmod-amlogic-gx-npu-nanoq kmod-amlogic-gx-vpu amlogic-gx-vpu-firmware
endef
TARGET_DEVICES += onethingcloud_oes

define Device/onethingcloud_oes-plus
  DEVICE_VENDOR := OneThing Cloud
  DEVICE_MODEL := OES Plus
  SOC := meson-g12b
  DEVICE_DTS := meson-g12b-s922x-oes-plus
  DEVICE_PACKAGES := ethtool kmod-usb-net-rtl8152-vendor
endef
TARGET_DEVICES += onethingcloud_oes-plus
