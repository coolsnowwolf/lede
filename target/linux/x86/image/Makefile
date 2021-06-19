# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2020 OpenWrt.org

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

GRUB_SERIAL:=$(call qstrip,$(CONFIG_GRUB_SERIAL))

ifneq ($(GRUB_SERIAL),)
  GRUB_CONSOLE_CMDLINE += console=$(GRUB_SERIAL),$(CONFIG_GRUB_BAUDRATE)n8$(if $(CONFIG_GRUB_FLOWCONTROL),r,)
  GRUB_SERIAL_CONFIG := serial --unit=0 --speed=$(CONFIG_GRUB_BAUDRATE) --word=8 --parity=no --stop=1 --rtscts=$(if $(CONFIG_GRUB_FLOWCONTROL),on,off)
  GRUB_TERMINALS += serial
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
	$(CP) $(KDIR)/$(KERNEL_NAME) $@.boot/boot/vmlinuz
	-$(CP) $(STAGING_DIR_ROOT)/boot/. $@.boot/boot/
	$(CP) $(STAGING_DIR_IMAGE)/grub2/boot.img $@.boot/boot/grub/
	$(CP) $(STAGING_DIR_IMAGE)/grub2/$(if $(filter $(1),efi),gpt,$(GRUB2_VARIANT))-core.img \
		$@.boot/boot/grub/core.img
	$(if $(filter $(1),efi),
		$(INSTALL_DIR) $@.boot/efi/boot
		$(CP) $(STAGING_DIR_IMAGE)/grub2/boot$(if $(CONFIG_x86_64),x64,ia32).efi $@.boot/efi/boot/
	)
	PADDING="1" SIGNATURE="$(IMG_PART_SIGNATURE)" \
		$(if $(filter $(1),efi),GUID="$(IMG_PART_DISKGUID)") $(SCRIPT_DIR)/gen_image_generic.sh \
		$@ \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $@.boot \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS) \
		256
endef

define Build/grub-config
	rm -fR $@.boot
	$(INSTALL_DIR) $@.boot/boot/grub
	sed \
		-e 's#@SERIAL_CONFIG@#$(strip $(GRUB_SERIAL_CONFIG))#g' \
		-e 's#@TERMINAL_CONFIG@#$(strip $(GRUB_TERMINAL_CONFIG))#g' \
		-e 's#@ROOTPART@#root=$(ROOTPART) rootwait#g' \
		-e 's#@GPT_ROOTPART@#root=$(GPT_ROOTPART) rootwait#g' \
		-e 's#@CMDLINE@#$(BOOTOPTS) $(GRUB_CONSOLE_CMDLINE)#g' \
		-e 's#@TIMEOUT@#$(GRUB_TIMEOUT)#g' \
		-e 's#@TITLE@#$(GRUB_TITLE)#g' \
		./grub-$(1).cfg > $@.boot/boot/grub/grub.cfg
endef

define Build/grub-install
	rm -fR $@.grub2
	$(INSTALL_DIR) $@.grub2
	$(CP) $(STAGING_DIR_IMAGE)/grub2/boot.img $@.grub2/
	$(CP) $(STAGING_DIR_IMAGE)/grub2/$(if $(filter $(1),efi),gpt,$(GRUB2_VARIANT))-core.img $@.grub2/core.img
	echo '(hd0) $@' > $@.grub2/device.map
	$(STAGING_DIR_HOST)/bin/grub-bios-setup \
		-m "$@.grub2/device.map" \
		-d "$@.grub2" \
		-r "hd0,$(if $(filter $(1),efi),gpt1,msdos1)" \
		$@
endef

define Build/iso
	$(CP) $(KDIR)/$(KERNEL_NAME) $@.boot/boot/vmlinuz
	cat \
		$(STAGING_DIR_IMAGE)/grub2/cdboot.img \
		$(STAGING_DIR_IMAGE)/grub2/eltorito.img \
		> $@.boot/boot/grub/eltorito.img
	-$(CP) $(STAGING_DIR_ROOT)/boot/. $@.boot/boot/
	$(if $(filter $(1),efi),
		mkfs.fat -C $@.boot/boot/grub/isoboot.img -S 512 1440
		mmd -i $@.boot/boot/grub/isoboot.img ::/efi ::/efi/boot
		mcopy -i $@.boot/boot/grub/isoboot.img \
			$(STAGING_DIR_IMAGE)/grub2/iso-boot$(if $(CONFIG_x86_64),x64,ia32).efi \
			::/efi/boot/boot$(if $(CONFIG_x86_64),x64,ia32).efi
	)
	mkisofs -R -b boot/grub/eltorito.img -no-emul-boot -boot-info-table \
		$(if $(filter $(1),efi),-boot-load-size 4 -c boot.cat -eltorito-alt-boot -b boot/grub/isoboot.img -no-emul-boot) \
		-o $@ $@.boot $(TARGET_DIR)
endef

DEVICE_VARS += GRUB2_VARIANT
define Device/Default
  ARTIFACT/image.iso := grub-config iso | iso
  IMAGE/combined.img := grub-config pc | combined | grub-install | append-metadata
  IMAGE/combined.img.gz := grub-config pc | combined | grub-install | gzip | append-metadata
  IMAGE/combined.qcow2 := grub-config pc | combined | grub-install | qemu-image qcow2
  IMAGE/combined.vdi := grub-config pc | combined | grub-install | qemu-image vdi
  IMAGE/combined.vhdx := grub-config pc | combined | grub-install | qemu-image vhdx
  IMAGE/combined.vmdk := grub-config pc | combined | grub-install | qemu-image vmdk
  IMAGE/rootfs.img := append-rootfs | pad-to $(ROOTFS_PARTSIZE)
  IMAGE/rootfs.img.gz := append-rootfs | pad-to $(ROOTFS_PARTSIZE) | gzip
  ARTIFACT/image-efi.iso := grub-config iso | iso efi
  IMAGE/combined-efi.img := grub-config efi | combined efi | grub-install efi | append-metadata
  IMAGE/combined-efi.img.gz := grub-config efi | combined efi | grub-install efi | gzip | append-metadata
  IMAGE/combined-efi.qcow2 := grub-config efi | combined efi | grub-install efi | qemu-image qcow2
  IMAGE/combined-efi.vdi := grub-config efi | combined efi | grub-install efi | qemu-image vdi
  IMAGE/combined-efi.vhdx := grub-config efi | combined efi | grub-install efi | qemu-image vhdx
  IMAGE/combined-efi.vmdk := grub-config efi | combined efi | grub-install efi | qemu-image vmdk
  ifeq ($(CONFIG_TARGET_IMAGES_GZIP),y)
    IMAGES-y := rootfs.img.gz
    IMAGES-$$(CONFIG_GRUB_IMAGES) += combined.img.gz
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi.img.gz
  else
    IMAGES-y := rootfs.img
    IMAGES-$$(CONFIG_GRUB_IMAGES) += combined.img
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi.img
  endif
  KERNEL := kernel-bin
  KERNEL_INSTALL := 1
  KERNEL_NAME := bzImage
  ifeq ($(CONFIG_ISO_IMAGES),y)
    ARTIFACTS-$$(CONFIG_GRUB_IMAGES) += image.iso
    ARTIFACTS-$$(CONFIG_GRUB_EFI_IMAGES) += image-efi.iso
  endif
  ifeq ($(CONFIG_QCOW2_IMAGES),y)
    IMAGES-$$(CONFIG_GRUB_IMAGES) += combined.qcow2
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi.qcow2
  endif
  ifeq ($(CONFIG_VDI_IMAGES),y)
    IMAGES-$$(CONFIG_GRUB_IMAGES) += combined.vdi
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi.vdi
  endif
  ifeq ($(CONFIG_VHDX_IMAGES),y)
    IMAGES-$$(CONFIG_GRUB_IMAGES) += combined.vhdx
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi.vhdx
  endif
  ifeq ($(CONFIG_VMDK_IMAGES),y)
    IMAGES-$$(CONFIG_GRUB_IMAGES) += combined.vmdk
    IMAGES-$$(CONFIG_GRUB_EFI_IMAGES) += combined-efi.vmdk
  endif
  IMAGES := $$(IMAGES-y)
  ARTIFACTS := $$(ARTIFACTS-y)
  SUPPORTED_DEVICES :=
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))

