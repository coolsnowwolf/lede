# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2023-2024 Ailick <277498654@qq.com>

GRUB2_VARIANT =
GRUB_TERMINALS =
GRUB_SERIAL_CONFIG =
GRUB_TERMINAL_CONFIG =
GRUB_CONSOLE_CMDLINE = earlycon

ifneq ($(CONFIG_GRUB_CONSOLE),)
  GRUB_TERMINALS += console
endif

GRUB_SERIAL:=$(call qstrip,$(CONFIG_GRUB_SERIAL))

GRUB_SERIAL_CONFIG := serial --unit=0 --speed=$(CONFIG_GRUB_BAUDRATE) --word=8 --parity=no --stop=1 --rtscts=$(if $(CONFIG_GRUB_FLOWCONTROL),on,off)
GRUB_TERMINALS += serial

GRUB_TERMINAL_CONFIG := terminal_input $(GRUB_TERMINALS); terminal_output $(GRUB_TERMINALS)

ROOTPART:=$(call qstrip,$(CONFIG_TARGET_ROOTFS_PARTNAME))
ROOTPART:=$(if $(ROOTPART),$(ROOTPART),PARTUUID=$(IMG_PART_SIGNATURE)-02)
GPT_ROOTPART:=$(call qstrip,$(CONFIG_TARGET_ROOTFS_PARTNAME))
GPT_ROOTPART:=$(if $(GPT_ROOTPART),$(GPT_ROOTPART),PARTUUID=$(shell echo $(IMG_PART_DISKGUID) | sed 's/00$$/02/'))

GRUB_TIMEOUT:=$(call qstrip,$(CONFIG_GRUB_TIMEOUT))
GRUB_TITLE:=$(call qstrip,$(CONFIG_GRUB_TITLE))

BOOTOPTS:=$(call qstrip,$(CONFIG_GRUB_BOOTOPTS))

define Build/combined
  $(INSTALL_DIR) $@.boot/
	$(CP) $(KDIR)/$(KERNEL_NAME) $@.boot/efi/openwrt/
	-$(CP) $(STAGING_DIR_ROOT)/boot/. $@.boot/boot/
	$(if $(filter $(1),efi),
		$(INSTALL_DIR) $@.boot/efi/boot
		$(CP) $(STAGING_DIR_IMAGE)/grub2/boot$(if $(CONFIG_aarch64),aa64,arm).efi $@.boot/efi/openwrt/
		$(CP) $(STAGING_DIR_IMAGE)/grub2/boot$(if $(CONFIG_aarch64),aa64,arm).efi $@.boot/efi/boot/
	)
	KERNELPARTTYPE=ef FAT_TYPE="32" PADDING="1" SIGNATURE="$(IMG_PART_SIGNATURE)" \
		$(if $(filter $(1),efi),GUID="$(IMG_PART_DISKGUID)") $(SCRIPT_DIR)/gen_image_generic.sh \
		$@ \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $@.boot \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS) \
		256
endef

define Build/grub-config
	rm -fR $@.boot
	$(INSTALL_DIR) $@.boot/efi/openwrt/
	sed \
		-e 's#@SERIAL_CONFIG@#$(strip $(GRUB_SERIAL_CONFIG))#g' \
		-e 's#@TERMINAL_CONFIG@#$(strip $(GRUB_TERMINAL_CONFIG))#g' \
		-e 's#@ROOTPART@#root=$(ROOTPART) rootwait#g' \
		-e 's#@GPT_ROOTPART@#root=$(GPT_ROOTPART) rootwait#g' \
		-e 's#@CMDLINE@#$(BOOTOPTS) $(GRUB_CONSOLE_CMDLINE)#g' \
		-e 's#@TIMEOUT@#$(GRUB_TIMEOUT)#g' \
		-e 's#@TITLE@#$(GRUB_TITLE)#g' \
		-e 's#@KERNEL_NAME@#$(KERNEL_NAME)#g' \
		./grub-$(1).cfg > $@.boot/efi/openwrt/grub.cfg
endef

define Build/grub-install
	rm -fR $@.grub2
	$(INSTALL_DIR) $@.grub2
endef

DEVICE_VARS += GRUB2_VARIANT UBOOT
define Device/efi-default
  IMAGE/rootfs.img := append-rootfs | pad-to $(ROOTFS_PARTSIZE)
  IMAGE/rootfs.img.gz := append-rootfs | pad-to $(ROOTFS_PARTSIZE) | gzip
  IMAGE/combined.img := grub-config efi | combined efi | grub-install efi | append-metadata
  IMAGE/combined.img.gz := grub-config efi | combined efi | grub-install efi | gzip | append-metadata
  IMAGE/combined.vmdk := grub-config efi | combined efi | grub-install efi | qemu-image vmdk
 ifeq ($(CONFIG_TARGET_IMAGES_GZIP),y)
    IMAGES-y := rootfs.img.gz
    IMAGES-y += combined.img.gz
  else
    IMAGES-y := rootfs.img
    IMAGES-y += combined.img
  endif
  ifeq ($(CONFIG_VMDK_IMAGES),y)
    IMAGES-y += combined.vmdk
  endif
  KERNEL := kernel-bin
  KERNEL_INSTALL := 1
  IMAGES := $$(IMAGES-y)
  ARTIFACTS := $$(ARTIFACTS-y)
  SUPPORTED_DEVICES :=
 ifeq ($(CONFIG_arm),y)
  KERNEL_NAME = zImage
 endif
endef

define Device/generic
  $(call Device/efi-default)
  DEVICE_TITLE := Generic EFI Boot
  GRUB2_VARIANT := generic
  FILESYSTEMS := ext4 squashfs
  UBOOT := $(if $(CONFIG_aarch64),qemu_armv8,qemu_armv7)
  DEVICE_PACKAGES += kmod-amazon-ena kmod-e1000e kmod-vmxnet3 kmod-rtc-rx8025 \
	kmod-i2c-mux-pca954x kmod-gpio-pca953x partx-utils kmod-wdt-sp805 \
	kmod-mvneta kmod-mvpp2 kmod-fsl-dpaa1-net kmod-fsl-dpaa2-net \
	kmod-fsl-enetc-net kmod-dwmac-imx kmod-fsl-fec kmod-thunderx-net \
	kmod-dwmac-rockchip kmod-dwmac-sun8i kmod-phy-aquantia kmod-phy-broadcom \
	kmod-phy-marvell kmod-phy-marvell-10g kmod-sfp kmod-atlantic \
	kmod-bcmgenet kmod-octeontx2-net
endef
TARGET_DEVICES += generic

