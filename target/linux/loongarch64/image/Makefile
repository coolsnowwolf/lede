# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2024 Weijie Gao <hackpascal@gmail.com>

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

GRUB2_VARIANT =
GRUB_TERMINALS =
GRUB_SERIAL_CONFIG =
GRUB_TERMINAL_CONFIG =
GRUB_CONSOLE_CMDLINE =

ifneq ($(CONFIG_GRUB_CONSOLE),)
  GRUB_CONSOLE_CMDLINE += console=tty0
  GRUB_TERMINALS += console
endif

GRUB_SERIAL:=$(call qstrip,$(CONFIG_TARGET_SERIAL))

GRUB_CONSOLE_CMDLINE += console=$(GRUB_SERIAL),$(CONFIG_GRUB_BAUDRATE)n8$(if $(CONFIG_GRUB_FLOWCONTROL),r,)
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
	$(INSTALL_DIR) $@.boot/efi/boot
	$(CP) $(STAGING_DIR_IMAGE)/grub2/bootloongarch64.efi $@.boot/efi/boot/
	KERNELPARTTYPE=ef FAT_TYPE="32" PADDING="1" SIGNATURE="$(IMG_PART_SIGNATURE)" \
		GUID="$(IMG_PART_DISKGUID)" $(SCRIPT_DIR)/gen_image_generic.sh \
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

define Device/Default
  KERNEL_INSTALL := 1
  ARTIFACTS := $$(ARTIFACTS-y)
  SUPPORTED_DEVICES :=
endef

define Device/generic
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := LoongArch64
  DEVICE_PACKAGES += kmod-r8169 kmod-drm-amdgpu
  KERNEL := kernel-bin
  KERNEL_NAME := vmlinuz.efi
  IMAGE/rootfs.img := append-rootfs | pad-to $(ROOTFS_PARTSIZE)
  IMAGE/rootfs.img.gz := append-rootfs | pad-to $(ROOTFS_PARTSIZE) | gzip
  IMAGE/combined-efi.img := grub-config efi | combined | append-metadata
  IMAGE/combined-efi.img.gz := grub-config efi | combined | gzip | append-metadata
  ifeq ($(CONFIG_TARGET_IMAGES_GZIP),y)
    IMAGES-y := rootfs.img.gz
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi.img.gz
  else
    IMAGES-y := rootfs.img
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi.img
  endif
  IMAGES := $$(IMAGES-y)
endef
TARGET_DEVICES += generic

$(eval $(call BuildImage))
