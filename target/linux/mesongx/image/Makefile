#
# Copyright (C) 2017-2019 Stijn Tintel <stijn@linux-ipv6.be>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Build/bootloader-odroid-c2
	fip_create \
		--bl30  $(STAGING_DIR_IMAGE)/odroidc2/bl30.bin \
		--bl301 $(STAGING_DIR_IMAGE)/odroidc2/bl301.bin \
		--bl31  $(STAGING_DIR_IMAGE)/odroidc2/bl31.bin \
		--bl33  $(BIN_DIR)/u-boot-odroid-c2/u-boot.bin \
		$(STAGING_DIR_IMAGE)/odroidc2/fip.bin

	cat $(STAGING_DIR_IMAGE)/odroidc2/bl2.package $(STAGING_DIR_IMAGE)/odroidc2/fip.bin > $(STAGING_DIR_IMAGE)/odroidc2/boot_new.bin

	amlbootsig $(STAGING_DIR_IMAGE)/odroidc2/boot_new.bin $(STAGING_DIR_IMAGE)/odroidc2/boot_new.bin.signed

	dd if=$(STAGING_DIR_IMAGE)/odroidc2/boot_new.bin.signed of=$(STAGING_DIR_IMAGE)/odroidc2/u-boot.gxbb bs=512 skip=96
	dd if=$(STAGING_DIR_IMAGE)/odroidc2/bl1.bin.hardkernel of=$@ conv=fsync,notrunc bs=1 count=440
	dd if=$(STAGING_DIR_IMAGE)/odroidc2/bl1.bin.hardkernel of=$@ conv=fsync,notrunc bs=512 skip=1 seek=1
	dd if=$(STAGING_DIR_IMAGE)/odroidc2/u-boot.gxbb of=$@ conv=fsync,notrunc bs=512 seek=97
endef

define Build/sdcard-img
	$(INSTALL_DIR) $(KDIR)/boot.ext4
	$(CP) $(KDIR)/boot.$1.scr $(KDIR)/boot.ext4/boot.scr
	$(CP) $(KDIR_KERNEL_IMAGE) $(KDIR)/boot.ext4/uImage
	$(foreach dts,$(DEVICE_DTS),$(CP) $(DTS_DIR)/$(dts).dtb $(KDIR)/boot.ext4/)
	PADDING="$(CONFIG_TARGET_IMAGES_PAD)" \
	PATH="$(TARGET_PATH)" \
	$(SCRIPT_DIR)/gen_image_generic.sh $@ \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $(KDIR)/boot.ext4 \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS) \
		4096
	$(RM) -rf $(KDIR)/boot.ext4
endef

define Build/uImage-meson
	$(call Build/uImage,none)
	mkimage -A arm -T script -C none -n "boot.scr uImage" -d boot.$1.txt $(KDIR)/boot.$1.scr
endef

### Devices ###
define Device/Default
  FILESYSTEMS := ext4 squashfs
  IMAGES := sdcard.img
  IMAGE/sdcard.img := sdcard-img $$(DEVICE_NAME)
  KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL_LOADADDR := 0x01080000
  KERNEL_NAME := Image
  PROFILES = Default $$(DEVICE_NAME)
endef


ifeq ($(SUBTARGET),S905)

define Device/odroid-c2
  DEVICE_DTS := amlogic/meson-gxbb-odroidc2
  DEVICE_PACKAGES += odroid-firmware-c2 u-boot-odroid-c2
  DEVICE_TITLE := ODROID C2
  KERNEL := kernel-bin | uImage-meson $$(DEVICE_NAME)
endef
TARGET_DEVICES += odroid-c2
endif


ifeq ($(SUBTARGET),m8b)

define Device/meson8b_m201_1G
  DEVICE_DTS := meson8b-odroidc1
### DEVICE_PACKAGES += odroid-firmware-c2 u-boot-odroid-c2
  DEVICE_TITLE := meson8b_m201_1G
  KERNEL := kernel-bin | uImage-meson $$(DEVICE_NAME)
endef
TARGET_DEVICES += meson8b_m201_1G
endif

$(eval $(call BuildImage))
