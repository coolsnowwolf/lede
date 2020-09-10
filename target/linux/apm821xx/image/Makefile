#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

DEVICE_VARS += DTB_SIZE

define Build/boot-img
	$(RM) -rf $@.bootdir
	mkdir -p $@.bootdir/boot

	$(CP) $@.scr $@.bootdir/boot/boot.scr
	$(CP) $(IMAGE_KERNEL).dtb $@.bootdir/boot/$(DEVICE_DTB)
	$(CP) $(IMAGE_KERNEL) $@.bootdir/boot/uImage

	genext2fs --block-size $(BLOCKSIZE:%k=%Ki) \
		--size-in-blocks $$((1024 * $(CONFIG_TARGET_KERNEL_PARTSIZE))) \
		--root $@.bootdir $@.boot

	# convert it to revision 1 - needed for u-boot ext2load
	$(STAGING_DIR_HOST)/bin/tune2fs -O filetype $@.boot
	$(STAGING_DIR_HOST)/bin/e2fsck -pDf $@.boot > /dev/null
endef

define Build/boot-script
	$(STAGING_DIR_HOST)/bin/mkimage -A powerpc -T script -C none -n "$(PROFILE) Boot Script" \
		-d mbl_boot.scr \
		$@.scr
endef

define Build/copy-file
	cat "$(1)" > "$@"
endef

define Build/dtb
	$(call Image/BuildDTB,../dts/$(DEVICE_DTS).dts,$@.dtb,,--space $(DTB_SIZE))
endef

define Build/export-dtb
	cp $(IMAGE_KERNEL).dtb $@
endef

define Build/MuImage-initramfs
	rm -rf $@.fakerd $@.new

	dd if=/dev/zero of=$@.fakerd bs=32 count=1 conv=sync

	# Netgear used an old uboot that doesn't have FIT support.
	# So we are stuck with either a full ext2/4 fs in a initrd.
	# ... or we try to make the "multi" image approach to work
	# for us.
	#
	# Sadly, the "multi" image has to consists of three
	# "fixed" parts in the following "fixed" order:
	# 1. The kernel which is in $@
	# 2. The (fake) initrd which is in $@.fakerd
	# 3. The device tree binary which is in $@.dtb
	#
	# Now, given that we use the function for the kernel which
	# already has a initramfs image inside, we still have to
	# add a "fake" initrd (which a mkimage header) in the second
	# part of the legacy multi image. Since we need to put the
	# device tree stuff into part 3.

	-$(STAGING_DIR_HOST)/bin/mkimage -A $(LINUX_KARCH) -O linux -T multi \
		-C $(1) -a $(KERNEL_LOADADDR) -e $(KERNEL_ENTRY) \
		-n '$(BOARD_NAME) initramfs' -d $@:$@.fakerd:$@.dtb $@.new
	mv $@.new $@
	rm -rf $@.fakerd
endef

define Build/prepend-dtb
	cat "$@.dtb.uimage" "$@" > "$@.new"
	mv "$@.new" "$@"
endef

define Image/cpiogz
	( cd $(TARGET_DIR); find . | cpio -o -H newc | gzip -9n >$(KDIR_TMP)/$(IMG_PREFIX)-rootfs.cpio.gz )
endef

define Device/Default
  PROFILES := Default
  KERNEL_DEPENDS = $$(wildcard ../dts/$$(DEVICE_DTS).dts)
  DEVICE_DTS :=
  KERNEL_ENTRY := 0x00000000
  KERNEL_LOADADDR := 0x00000000
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS = $(subst _,-,$(1))
  SUPPORTED_DEVICES = $(subst _,$(comma),$(1))
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
