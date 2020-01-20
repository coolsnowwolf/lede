# 
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

export PATH=$(TARGET_PATH):/sbin

GRUB2_MODULES = biosdisk boot chain configfile ext2 linux ls part_msdos reboot serial vga
GRUB2_MODULES_LEGACY = $(GRUB2_MODULES)
GRUB2_MODULES_LEGACY += part_gpt search fat exfat
GRUB2_MODULES_EFI = boot chain configfile ext2 linux ls part_msdos reboot serial part_gpt part_msdos search fat exfat ext2 efi_gop efi_uga gfxterm
GRUB2_MODULES_ISO = biosdisk boot chain configfile iso9660 linux ls part_msdos reboot serial vga
GRUB_TERMINALS =
GRUB_SERIAL_CONFIG =
GRUB_TERMINAL_CONFIG =
GRUB_CONSOLE_CMDLINE =
GRUB_ROOT = hd0,msdos1

USE_ATKBD = generic 64

ifneq ($(strip $(foreach subtarget,$(USE_ATKBD),$(CONFIG_TARGET_x86_$(subtarget)))),)
  GRUB2_MODULES += at_keyboard
  GRUB2_MODULES_ISO += at_keyboard
endif

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

SIGNATURE:=$(shell perl -e 'printf("%08x", rand(0xFFFFFFFF))')
EFI_SIGNATURE:=$(strip $(shell uuidgen | sed "s/[a-zA-Z0-9][a-zA-Z0-9][a-zA-Z0-9][a-zA-Z0-9]$$/0002/" | tr '[a-z]' '[A-Z]'))
ROOTPART:=$(call qstrip,$(CONFIG_TARGET_ROOTFS_PARTNAME))
ROOTPART:=$(if $(ROOTPART),$(ROOTPART),PARTUUID=$(SIGNATURE)-02)

GRUB_TIMEOUT:=$(call qstrip,$(CONFIG_GRUB_TIMEOUT))
GRUB_TITLE:=$(call qstrip,$(CONFIG_GRUB_TITLE))

ifneq ($(CONFIG_TARGET_x86_xen_domu),)
  GRUB_ROOT = xen/xvda,msdos1
endif

ifneq ($(CONFIG_GRUB_IMAGES)$(CONFIG_EFI_IMAGES),)

  BOOTOPTS:=$(call qstrip,$(CONFIG_GRUB_BOOTOPTS))

  define Image/cmdline/ext4
    root=$(ROOTPART) rootfstype=ext4 rootwait
  endef

  define Image/cmdline/squashfs
    root=$(ROOTPART) rootfstype=squashfs rootwait
  endef

  ifneq ($(CONFIG_EFI_IMAGES),)

  define Image/cmdline/efi
    $(subst $(SIGNATURE)-02,$2,$(call Image/cmdline/$(1)))
  endef

  define Image/Build/efi
	# left here because the image builder doesnt need these
	rm -rf $(KDIR)/root.grub/ || true
	$(INSTALL_DIR) $(KDIR)/root.grub/boot/grub $(KDIR)/grub2
	$(CP) $(KDIR)/bzImage $(KDIR)/root.grub/boot/vmlinuz
	echo '(hd0) $(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img' > $(KDIR)/grub2/device.map
	sed \
		-e 's#@SERIAL_CONFIG@#$(strip $(GRUB_SERIAL_CONFIG))#g' \
		-e 's#@TERMINAL_CONFIG@#$(strip $(GRUB_TERMINAL_CONFIG))#g' \
		-e 's#@CMDLINE@#$(strip $(call Image/cmdline/efi,$(1),$(EFI_SIGNATURE)) $(BOOTOPTS) $(GRUB_CONSOLE_CMDLINE))#g' \
		-e 's#@TIMEOUT@#$(GRUB_TIMEOUT)#g' \
		-e 's#@TITLE@#$(GRUB_TITLE)#g' \
		-e 's#set root.*#search --file /boot/grub/$(SIGNATURE).cfg --set=root#g' \
		./grub.cfg > $(KDIR)/root.grub/boot/grub/grub.cfg
	sed \
		-e 's/(hd0,msdos1)/(hd0,gpt1)/' ./grub-early.cfg > \
			$(KDIR)/root.grub/boot/grub/grub-early.cfg

	$(CP) $(KDIR)/root.grub/boot/grub/grub.cfg $(KDIR)/root.grub/boot/grub/$(SIGNATURE).cfg
	grub-mkimage \
		-d $(STAGING_DIR_HOST)/lib/grub/i386-pc \
		-o $(KDIR)/grub2/core.img \
		-O i386-pc \
		-p '(hd0,gpt1)/boot/grub' \
		-c $(KDIR)/root.grub/boot/grub/grub-early.cfg \
		$(GRUB2_MODULES_LEGACY)
	$(CP) $(STAGING_DIR_HOST)/lib/grub/i386-pc/*.img $(KDIR)/grub2/

	# Build the efi grub version
	rm -rf $(KDIR)/grub2.efi/ || true
	$(INSTALL_DIR) $(KDIR)/grub2.efi/efi/boot/

	# Generate the grub search root config (grub will search for the $(SIGNATURE).cfg file placed on the boot partition as grub does not support search of GPT UUID yet)
	echo "search --file /boot/grub/$(SIGNATURE).cfg --set=root" > $(KDIR)/grub2.efi/efi/boot/grub.cfg
	echo "configfile /boot/grub/grub.cfg" >> $(KDIR)/grub2.efi/efi/boot/grub.cfg

	# Create the EFI grub binary
	grub-mkimage-efi \
		-d $(STAGING_DIR_HOST)/lib/grub/x86_64-efi \
		-o $(KDIR)/grub2.efi/efi/boot/bootx64.efi \
		-O x86_64-efi \
		-p /efi/boot \
		-c $(KDIR)/grub2.efi/efi/boot/grub.cfg \
		$(GRUB2_MODULES_EFI)

	# Generate the EFI VFAT bootfs
	rm $(KDIR)/kernel.efi || true
	mkfs.fat -C $(KDIR)/kernel.efi -S 512 1024
	mcopy -s -i "$(KDIR)/kernel.efi" $(KDIR)/grub2.efi/* ::/

	SIGNATURE="$(SIGNATURE)" PATH="$(TARGET_PATH)" ./gen_image_efi.sh \
		$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $(KDIR)/root.grub \
		1 $(KDIR)/kernel.efi \
		1 \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(KDIR)/root.$(1) \
		256

	# Setup legacy bios for hybrid MBR (optional)
	grub-bios-setup \
		--device-map="$(KDIR)/grub2/device.map" \
		-d "$(KDIR)/grub2" \
		-r "hd0,msdos1" \
		"$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img"

	# Convert the MBR partition to GPT and set EFI ROOTFS signature
	dd if=/dev/zero of="$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img" bs=512 count=34 conv=notrunc oflag=append
	sgdisk -g "$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img"
	sgdisk -t 2:EF00 "$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img"
	sgdisk -t 3:EF02 "$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img"
	sgdisk -u 4:$(EFI_SIGNATURE) "$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img"
	sgdisk -h "$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img"

	# Setup EFI grub
	grub-bios-setup-efi \
		--device-map="$(KDIR)/grub2/device.map" \
		-d "$(KDIR)/grub2" \
		-r "hd0,gpt1" \
		"$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img"
  endef
  endif

  ifneq ($(CONFIG_GRUB_IMAGES),)
  define Image/Build/grub2
	# left here because the image builder doesnt need these
	$(INSTALL_DIR) $(KDIR)/root.grub/boot/grub $(KDIR)/grub2
	$(CP) $(KDIR)/bzImage $(KDIR)/root.grub/boot/vmlinuz
	grub-mkimage \
		-p /boot/grub \
		-d $(STAGING_DIR_HOST)/lib/grub/i386-pc \
		-o $(KDIR)/grub2/core.img \
		-O i386-pc \
		-c ./grub-early.cfg \
		$(GRUB2_MODULES)
	$(CP) $(STAGING_DIR_HOST)/lib/grub/i386-pc/*.img $(KDIR)/grub2/
	echo '(hd0) $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img' > $(KDIR)/grub2/device.map
	sed \
		-e 's#@SERIAL_CONFIG@#$(strip $(GRUB_SERIAL_CONFIG))#g' \
		-e 's#@TERMINAL_CONFIG@#$(strip $(GRUB_TERMINAL_CONFIG))#g' \
		-e 's#@CMDLINE@#$(strip $(call Image/cmdline/$(1)) $(BOOTOPTS) $(GRUB_CONSOLE_CMDLINE))#g' \
		-e 's#@TIMEOUT@#$(GRUB_TIMEOUT)#g' \
		-e 's#@ROOT@#$(GRUB_ROOT)#g' \
		-e 's#@TITLE@#$(GRUB_TITLE)#g' \
		./grub.cfg > $(KDIR)/root.grub/boot/grub/grub.cfg
	-$(CP) $(STAGING_DIR_ROOT)/boot/. $(KDIR)/root.grub/boot/
	PADDING="$(CONFIG_TARGET_IMAGES_PAD)" SIGNATURE="$(SIGNATURE)" PATH="$(TARGET_PATH)" $(SCRIPT_DIR)/gen_image_generic.sh \
		$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $(KDIR)/root.grub \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(KDIR)/root.$(1) \
		256
	grub-bios-setup \
		--device-map="$(KDIR)/grub2/device.map" \
		-d "$(KDIR)/grub2" \
		-r "hd0,msdos1" \
		"$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img"
  endef
  endif

endif

define Image/Build/squashfs
	dd if=/dev/zero bs=128k count=1 >> $(KDIR)/root.squashfs
endef

define Image/Build/iso
	$(INSTALL_DIR) $(KDIR)/root.grub/boot/grub $(KDIR)/grub2
	$(CP) $(KDIR)/bzImage $(KDIR)/root.grub/boot/vmlinuz
	grub-mkimage \
		-p /boot/grub \
		-d $(STAGING_DIR_HOST)/lib/grub/i386-pc \
		-o $(KDIR)/grub2/eltorito.img \
		-O i386-pc \
		-c ./grub-early.cfg \
		$(GRUB2_MODULES_ISO)
	cat \
		$(STAGING_DIR_HOST)/lib/grub/i386-pc/cdboot.img \
		$(KDIR)/grub2/eltorito.img \
		> $(KDIR)/root.grub/boot/grub/eltorito.img
	sed \
		-e 's#@SERIAL_CONFIG@#$(strip $(GRUB_SERIAL_CONFIG))#g' \
		-e 's#@TERMINAL_CONFIG@#$(strip $(GRUB_TERMINAL_CONFIG))#g' \
		-e 's#@CMDLINE@#root=/dev/sr0 rootfstype=iso9660 rootwait $(strip $(call Image/cmdline/$(1)) $(BOOTOPTS) $(GRUB_CONSOLE_CMDLINE))#g' \
		-e 's#@TIMEOUT@#$(GRUB_TIMEOUT)#g' \
		-e 's#@TITLE@#$(GRUB_TITLE)#g' \
		./grub-iso.cfg > $(KDIR)/root.grub/boot/grub/grub.cfg
	-$(CP) $(STAGING_DIR_ROOT)/boot/. $(KDIR)/root.grub/boot/
	mkisofs -R -b boot/grub/eltorito.img -no-emul-boot -boot-info-table \
		-o $(KDIR)/root.iso $(KDIR)/root.grub $(TARGET_DIR)
endef

ifneq ($(CONFIG_VDI_IMAGES),)
  define Image/Build/vdi
	rm $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).vdi || true
	qemu-img convert -f raw -O vdi \
		$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img \
		$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).vdi
	# XXX: VBoxManage insists on setting perms to 0600
	chmod 0644 $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).vdi
  endef
  define Image/Build/vdi_efi
	rm $(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).vdi || true
	qemu-img convert -f raw -O vdi \
		$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img \
		$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).vdi
	# XXX: VBoxManage insists on setting perms to 0600
	chmod 0644 $(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).vdi
  endef
endif

ifneq ($(CONFIG_VMDK_IMAGES),)
  define Image/Build/vmdk
	rm $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).vmdk || true
	/usr/bin/qemu-img convert -f raw -O vmdk -o adapter_type=lsilogic \
		$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img \
		$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).vmdk
  endef
  define Image/Build/vmdk_efi
	rm $(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).vmdk || true
	/usr/bin/qemu-img convert -f raw -O vmdk -o adapter_type=lsilogic \
		$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img \
		$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).vmdk
  endef
endif

ifneq ($(CONFIG_VHD_IMAGES),)
  define Image/Build/vhd
	rm $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).vhdx || true
	/usr/bin/qemu-img convert -O vhdx \
		$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img \
		$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).vhdx
  endef
  define Image/Build/vhd_efi
	rm $(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).vhdx || true
	/usr/bin/qemu-img convert -O vhdx \
		$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img \
		$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).vhdx
  endef
endif

ifneq ($(CONFIG_QCOW2_IMAGES),)
  define Image/Build/qcow2
	rm $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).qcow2 || true
	/usr/bin/qemu-img convert -f raw -O qcow2 \
		$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img \
		$(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).qcow2
  endef
  define Image/Build/qcow2_efi
	rm $(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).qcow2 || true
	/usr/bin/qemu-img convert -f raw -O qcow2 \
		$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img \
		$(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).qcow2
  endef
endif

define Image/Build/gzip
	gzip -f9n $(BIN_DIR)/$(IMG_PREFIX)-rootfs-$(1).img
ifneq ($(CONFIG_GRUB_IMAGES),)
	gzip -f9n $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).img
endif
ifneq ($(CONFIG_EFI_IMAGES),)
	gzip -f9n $(BIN_DIR)/$(IMG_PREFIX)-uefi-gpt-$(1).img
endif
endef

ifneq ($(CONFIG_TARGET_IMAGES_GZIP),)
  define Image/Build/gzip/ext4
	$(call Image/Build/gzip,ext4)
  endef
  define Image/Build/gzip/squashfs
	$(call Image/Build/gzip,squashfs)
  endef
endif

define Image/BuildKernel
	$(CP) $(KDIR)/bzImage $(BIN_DIR)/$(IMG_PREFIX)-vmlinuz
endef

define Image/Prepare
	$(call Image/Prepare/grub2)
endef

define Image/Build/Initramfs
	$(CP) $(KDIR)/bzImage-initramfs $(BIN_DIR)/$(IMG_PREFIX)-ramfs.bzImage
endef

define Image/Build
	$(call Image/Build/$(1))
  ifneq ($(1),iso)
	$(call Image/Build/grub2,$(1))
	$(call Image/Build/efi,$(1))
ifneq ($(CONFIG_GRUB_IMAGES),)
	$(call Image/Build/vdi,$(1))
	$(call Image/Build/vmdk,$(1))
	$(call Image/Build/vhd,$(1))
	$(call Image/Build/qcow2,$(1))
endif
ifneq ($(CONFIG_EFI_IMAGES),)
	$(call Image/Build/vdi_efi,$(1))
	$(call Image/Build/vmdk_efi,$(1))
	$(call Image/Build/vhd_efi,$(1))
	$(call Image/Build/qcow2_efi,$(1))
endif
	$(CP) $(KDIR)/root.$(1) $(BIN_DIR)/$(IMG_PREFIX)-rootfs-$(1).img
  else
	$(CP) $(KDIR)/root.iso $(BIN_DIR)/$(IMG_PREFIX).iso
  endif
	$(CP) $(KDIR)/bzImage $(BIN_DIR)/$(IMG_PREFIX)-vmlinuz
	$(call Image/Build/gzip/$(1))
ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
	$(call Image/Build/Initramfs)
endif
endef

$(eval $(call BuildImage))
