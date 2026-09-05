# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2026

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

HOST_SFDISK := $(STAGING_DIR_HOST)/bin/sfdisk

DEVICE_VARS += QCOM_EXTRA_CMDLINE

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

ifneq ($(GRUB_SERIAL),)
  GRUB_CONSOLE_CMDLINE += console=$(GRUB_SERIAL),$(CONFIG_GRUB_BAUDRATE)n8$(if $(CONFIG_GRUB_FLOWCONTROL),r,)
endif

ifneq ($(GRUB_TERMINALS),)
  GRUB_TERMINAL_CONFIG := terminal_input $(GRUB_TERMINALS); terminal_output $(GRUB_TERMINALS)
endif

ROOTPART:=$(call qstrip,$(CONFIG_TARGET_ROOTFS_PARTNAME))
ROOTPART:=$(if $(ROOTPART),$(ROOTPART),PARTUUID=$(IMG_PART_SIGNATURE)-02)
GPT_ROOTPART:=$(call qstrip,$(CONFIG_TARGET_ROOTFS_PARTNAME))
GPT_ROOTPART:=$(if $(GPT_ROOTPART),$(GPT_ROOTPART),PARTUUID=$(shell echo $(IMG_PART_DISKGUID) | sed 's/00$$/02/'))

GRUB_TIMEOUT:=$(call qstrip,$(CONFIG_GRUB_TIMEOUT))
GRUB_TITLE:=$(call qstrip,$(CONFIG_GRUB_TITLE))

BOOTOPTS:=$(call qstrip,$(CONFIG_GRUB_BOOTOPTS))

define Build/combined
	$(INSTALL_DIR) $@.boot/efi/openwrt
	$(CP) $(KDIR)/$(KERNEL_NAME) $@.boot/efi/openwrt/
	$(CP) $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb $@.boot/efi/openwrt/openwrt.dtb
	$(INSTALL_DIR) $@.boot/efi/boot
	$(CP) $(STAGING_DIR_IMAGE)/grub2/boot$(if $(CONFIG_aarch64),aa64,arm).efi $@.boot/efi/openwrt/
	$(CP) $(STAGING_DIR_IMAGE)/grub2/boot$(if $(CONFIG_aarch64),aa64,arm).efi $@.boot/efi/boot/
	SFDISK_BIN="$(HOST_SFDISK)"; \
	SECTOR_SIZE=512; \
	SECTORS_PER_MIB=$$((1024 * 1024 / SECTOR_SIZE)); \
	ESP_START=2048; \
	ESP_SIZE_SECTORS=$$(( $(CONFIG_TARGET_KERNEL_PARTSIZE) * SECTORS_PER_MIB )); \
	ROOTFS_START=$$(( (ESP_START + ESP_SIZE_SECTORS + 2047) / 2048 * 2048 )); \
	ROOTFS_SIZE_SECTORS=$$(( $(CONFIG_TARGET_ROOTFS_PARTSIZE) * SECTORS_PER_MIB )); \
	TOTAL_SECTORS=$$(( ROOTFS_START + ROOTFS_SIZE_SECTORS + 2048 )); \
	ESP_UUID="$(shell echo $(IMG_PART_DISKGUID) | sed 's/00$$/01/')"; \
	ROOTFS_UUID="$(shell echo $(IMG_PART_DISKGUID) | sed 's/00$$/02/')"; \
	rm -f "$@"; \
	dd if=/dev/zero of="$@" bs=$$SECTOR_SIZE count=$$TOTAL_SECTORS; \
	( \
		echo "label: gpt"; \
		echo "label-id: $(IMG_PART_DISKGUID)"; \
		echo "unit: sectors"; \
		echo "first-lba: $$ESP_START"; \
		echo ""; \
		echo "start=$$ESP_START, size=$$ESP_SIZE_SECTORS, type=C12A7328-F81F-11D2-BA4B-00A0C93EC93B, uuid=$$ESP_UUID, name=kernel"; \
		echo "start=$$ROOTFS_START, size=$$ROOTFS_SIZE_SECTORS, type=0FC63DAF-8483-4772-8E79-3D69D8477DE4, uuid=$$ROOTFS_UUID, name=rootfs"; \
	) | "$$SFDISK_BIN" "$@"; \
	mkfs.fat -F 32 -n kernel -C "$@.esp" "$$(($(CONFIG_TARGET_KERNEL_PARTSIZE) * 1024))"; \
	mmd -i "$@.esp" ::/efi ::/efi/boot ::/efi/openwrt; \
	mcopy -i "$@.esp" "$@.boot/efi/boot/boot$(if $(CONFIG_aarch64),aa64,arm).efi" ::/efi/boot/; \
	mcopy -i "$@.esp" "$@.boot/efi/openwrt/boot$(if $(CONFIG_aarch64),aa64,arm).efi" ::/efi/openwrt/; \
	mcopy -i "$@.esp" "$@.boot/efi/openwrt/$(KERNEL_NAME)" ::/efi/openwrt/; \
	mcopy -i "$@.esp" "$@.boot/efi/openwrt/openwrt.dtb" ::/efi/openwrt/; \
	mcopy -i "$@.esp" "$@.boot/efi/openwrt/grub.cfg" ::/efi/openwrt/; \
	dd if="$@.esp" of="$@" bs=$$SECTOR_SIZE seek=$$ESP_START conv=notrunc; \
	dd if="$(IMAGE_ROOTFS)" of="$@" bs=$$SECTOR_SIZE seek=$$ROOTFS_START conv=notrunc; \
	rm -f "$@.esp"
endef

define Build/combined-ufs
	$(INSTALL_DIR) $@.boot/efi/openwrt
	$(CP) $(KDIR)/$(KERNEL_NAME) $@.boot/efi/openwrt/
	$(CP) $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb $@.boot/efi/openwrt/openwrt.dtb
	$(INSTALL_DIR) $@.boot/efi/boot
	$(CP) $(STAGING_DIR_IMAGE)/grub2/boot$(if $(CONFIG_aarch64),aa64,arm).efi $@.boot/efi/openwrt/
	$(CP) $(STAGING_DIR_IMAGE)/grub2/boot$(if $(CONFIG_aarch64),aa64,arm).efi $@.boot/efi/boot/
	SFDISK_BIN="$(HOST_SFDISK)"; \
	SECTOR_SIZE=4096; \
	SECTORS_PER_MIB=$$((1024 * 1024 / SECTOR_SIZE)); \
	ESP_START=256; \
	ESP_SIZE_SECTORS=$$(( $(CONFIG_TARGET_KERNEL_PARTSIZE) * SECTORS_PER_MIB )); \
	ROOTFS_START=$$(( (ESP_START + ESP_SIZE_SECTORS + 255) / 256 * 256 )); \
	ROOTFS_SIZE_SECTORS=$$(( $(CONFIG_TARGET_ROOTFS_PARTSIZE) * SECTORS_PER_MIB )); \
	TOTAL_SECTORS=$$(( ROOTFS_START + ROOTFS_SIZE_SECTORS + 256 )); \
	ESP_UUID="$(shell echo $(IMG_PART_DISKGUID) | sed 's/00$$/01/')"; \
	ROOTFS_UUID="$(shell echo $(IMG_PART_DISKGUID) | sed 's/00$$/02/')"; \
	rm -f "$@"; \
	dd if=/dev/zero of="$@" bs=$$SECTOR_SIZE count=$$TOTAL_SECTORS; \
	( \
		echo "label: gpt"; \
		echo "label-id: $(IMG_PART_DISKGUID)"; \
		echo "unit: sectors"; \
		echo "first-lba: $$ESP_START"; \
		echo ""; \
		echo "start=$$ESP_START, size=$$ESP_SIZE_SECTORS, type=C12A7328-F81F-11D2-BA4B-00A0C93EC93B, uuid=$$ESP_UUID, name=kernel"; \
		echo "start=$$ROOTFS_START, size=$$ROOTFS_SIZE_SECTORS, type=0FC63DAF-8483-4772-8E79-3D69D8477DE4, uuid=$$ROOTFS_UUID, name=rootfs"; \
	) | "$$SFDISK_BIN" --sector-size $$SECTOR_SIZE "$@"; \
	mkfs.fat -F 32 -n kernel -C "$@.esp" "$$(($(CONFIG_TARGET_KERNEL_PARTSIZE) * 1024))"; \
	mmd -i "$@.esp" ::/efi ::/efi/boot ::/efi/openwrt; \
	mcopy -i "$@.esp" "$@.boot/efi/boot/boot$(if $(CONFIG_aarch64),aa64,arm).efi" ::/efi/boot/; \
	mcopy -i "$@.esp" "$@.boot/efi/openwrt/boot$(if $(CONFIG_aarch64),aa64,arm).efi" ::/efi/openwrt/; \
	mcopy -i "$@.esp" "$@.boot/efi/openwrt/$(KERNEL_NAME)" ::/efi/openwrt/; \
	mcopy -i "$@.esp" "$@.boot/efi/openwrt/openwrt.dtb" ::/efi/openwrt/; \
	mcopy -i "$@.esp" "$@.boot/efi/openwrt/grub.cfg" ::/efi/openwrt/; \
	dd if="$@.esp" of="$@" bs=$$SECTOR_SIZE seek=$$ESP_START conv=notrunc; \
	dd if="$(IMAGE_ROOTFS)" of="$@" bs=$$SECTOR_SIZE seek=$$ROOTFS_START conv=notrunc; \
	rm -f "$@.esp"
endef

define Build/grub-config
	rm -fR $@.boot
	$(INSTALL_DIR) $@.boot/efi/openwrt/
	sed \
		-e 's#@SERIAL_CONFIG@#$(strip $(GRUB_SERIAL_CONFIG))#g' \
		-e 's#@TERMINAL_CONFIG@#$(strip $(GRUB_TERMINAL_CONFIG))#g' \
		-e 's#@ROOTPART@#root=$(ROOTPART) rootwait#g' \
		-e 's#@GPT_ROOTPART@#root=$(GPT_ROOTPART) rootwait#g' \
		-e 's#@CMDLINE@#$(BOOTOPTS) $(QCOM_EXTRA_CMDLINE) $(GRUB_CONSOLE_CMDLINE)#g' \
		-e 's#@TIMEOUT@#$(GRUB_TIMEOUT)#g' \
		-e 's#@TITLE@#$(GRUB_TITLE)#g' \
		-e 's#@KERNEL_NAME@#$(KERNEL_NAME)#g' \
		./grub-$(1).cfg > $@.boot/efi/openwrt/grub.cfg
endef

define Device/Default
  PROFILES = Default $$(DEVICE_NAME)
  KERNEL := kernel-bin
  KERNEL_INSTALL := 1
  KERNEL_NAME := vmlinuz.efi
  QCOM_EXTRA_CMDLINE :=
  ARTIFACTS := $$(ARTIFACTS-y)
  SUPPORTED_DEVICES :=
endef

define Device/efi-base
  IMAGE/rootfs.img := append-rootfs | pad-to $(ROOTFS_PARTSIZE)
  IMAGE/rootfs.img.gz := append-rootfs | pad-to $(ROOTFS_PARTSIZE) | gzip
  IMAGE/combined-efi.img := grub-config efi | combined | append-metadata
  IMAGE/combined-efi.img.gz := grub-config efi | combined | gzip | append-metadata
  IMAGE/combined-efi-ufs.img := grub-config efi | combined-ufs | append-metadata
  IMAGE/combined-efi-ufs.img.gz := grub-config efi | combined-ufs | gzip | append-metadata
  FILESYSTEMS := ext4 squashfs
endef

define Device/efi-default
  $(call Device/efi-base)
  ifeq ($(CONFIG_TARGET_IMAGES_GZIP),y)
    IMAGES-y := rootfs.img.gz
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi.img.gz
  else
    IMAGES-y := rootfs.img
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi.img
  endif
  IMAGES := $$(IMAGES-y)
endef

define Device/efi-ufs
  $(call Device/efi-base)
  ifeq ($(CONFIG_TARGET_IMAGES_GZIP),y)
    IMAGES-y := rootfs.img.gz
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi-ufs.img.gz
  else
    IMAGES-y := rootfs.img
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi-ufs.img
  endif
  IMAGES := $$(IMAGES-y)
endef

define Device/radxa_dragon_q6a-common
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := Dragon Q6A
  DEVICE_DTS_DIR := $(DTS_DIR)/qcom
  DEVICE_DTS := qcs6490-radxa-dragon-q6a
  SUPPORTED_DEVICES += radxa,dragon-q6a
  QCOM_EXTRA_CMDLINE := earlycon=qcom_geni,0x00994000 ignore_loglevel loglevel=8 keep_bootcon clk_ignore_unused pd_ignore_unused arm64.nopauth efi=noruntime
  DEVICE_PACKAGES += wpad-openssl
endef

define Device/radxa_dragon_q6a
  $(call Device/efi-default)
  $(call Device/radxa_dragon_q6a-common)
endef
TARGET_DEVICES += radxa_dragon_q6a

define Device/radxa_dragon_q6a_ufs
  $(call Device/efi-ufs)
  $(call Device/radxa_dragon_q6a-common)
  DEVICE_VARIANT := (UFS)
endef
TARGET_DEVICES += radxa_dragon_q6a_ufs

define Device/radxa_dragon_q8b-common
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := Dragon Q8B
  DEVICE_DTS_DIR := $(DTS_DIR)/qcom
  DEVICE_DTS := sc8280xp-radxa-dragon-q8b
  SUPPORTED_DEVICES += radxa,dragon-q8b
  QCOM_EXTRA_CMDLINE := earlycon=qcom_geni,0x00884000 ignore_loglevel loglevel=8 keep_bootcon clk_ignore_unused pd_ignore_unused arm64.nopauth efi=noruntime
  DEVICE_PACKAGES += kmod-eeprom-at24 kmod-stmmac-tc956x kmod-tc956x-pci
endef

define Device/radxa_dragon_q8b
  $(call Device/efi-default)
  $(call Device/radxa_dragon_q8b-common)
endef
TARGET_DEVICES += radxa_dragon_q8b

define Device/radxa_dragon_q8b_ufs
  $(call Device/efi-ufs)
  $(call Device/radxa_dragon_q8b-common)
  DEVICE_VARIANT := (UFS)
endef
TARGET_DEVICES += radxa_dragon_q8b_ufs

$(eval $(call BuildImage))
