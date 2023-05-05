#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_TARGET_KERNEL_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

define Build/sdcard-img
	$(RM) -f $@.boot
	mkfs.fat -C $@.boot $(FAT32_BLOCKS)
	mkdir -p $(KDIR)/boot.fat
#For S805
	mcopy -i $@.boot $(KDIR)/uInitrd ::
	mcopy -i $@.boot $(KDIR)/s805_autoscript ::
	mcopy -i $@.boot $(KDIR)/s805_autoscript.txt ::
#For S905
	mcopy -i $@.boot $(KDIR)/uEnv.ini ::
	mcopy -i $@.boot $(KDIR)/s905_autoscript ::
	mcopy -i $@.boot $(KDIR)/s905_autoscript.txt ::

	$(CP) $(IMAGE_KERNEL) $(KDIR)/boot.fat/uImage
	mcopy -i $@.boot $(KDIR)/boot.fat/uImage ::

	$(foreach dts,$(shell echo $(DEVICE_DTS)),$(CP) $(DTS_DIR)/$(dts).dtb $(KDIR)/boot.fat/dtb;)
	mcopy -i $@.boot $(KDIR)/boot.fat/dtb ::

	$(RM) -rf $(KDIR)/boot.fat

	./gen_aml_sdcard_img.sh $@ $@.boot $(IMAGE_ROOTFS) \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $(CONFIG_TARGET_ROOTFS_PARTSIZE)

endef

define Build/uImage-meson
#For S805 autoscript
	$(RM) -rf $(KDIR)/uInitrd
	$(RM) -rf $(KDIR)/s805_autoscript
	$(RM) -rf $(KDIR)/s805_autoscript.txt

#For S905 autoscript
	$(RM) -rf $(KDIR)/s905_autoscript
	$(RM) -rf $(KDIR)/s905_autoscript.txt
	$(RM) -rf $(KDIR)/uEnv.ini

	$(call Build/uImage,none)

#For S805 autoscript
	touch $(KDIR)/uInitrd
	$(CP) s805_autoscript.txt  $(KDIR)/s805_autoscript.txt
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "s805_autoscript" -d s805_autoscript.txt  $(KDIR)/s805_autoscript

#For S905 autoscript
	$(CP) uEnv.ini  $(KDIR)/uEnv.ini
	$(CP) s905_autoscript.txt  $(KDIR)/s905_autoscript.txt
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "s905_autoscript" -d s905_autoscript.txt  $(KDIR)/s905_autoscript

endef

### Devices ###
define Device/Default
  FILESYSTEMS := ext4
  IMAGES := sdcard.img
  IMAGE/sdcard.img := sdcard-img $$(DEVICE_NAME)
  KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL_LOADADDR := 0x01080000
  KERNEL_NAME := Image
  KERNEL := kernel-bin | uImage-meson none
  PROFILES = Default $$(DEVICE_NAME)
endef

define Device/odroid-c1
  DEVICE_DTS := meson8b-odroidc1
  DEVICE_PACKAGES +=
  DEVICE_TITLE := ODROID C1
  KERNEL_LOADADDR := 0x00208000
endef
ifeq ($(SUBTARGET),meson8b)
  TARGET_DEVICES += odroid-c1
endif

define Device/odroid-c2
  DEVICE_DTS := amlogic/meson-gxbb-odroidc2
  DEVICE_PACKAGES +=
  DEVICE_TITLE := ODROID C2
endef
ifeq ($(SUBTARGET),mesongx)
  TARGET_DEVICES += odroid-c2
endif

define Device/gxl-s905x-p212
  DEVICE_DTS := amlogic/meson-gxl-s905x-p212
  DEVICE_PACKAGES +=
  DEVICE_TITLE := S905X Design Reference Board
endef
ifeq ($(SUBTARGET),mesongx)
  TARGET_DEVICES += gxl-s905x-p212
endif

define Device/gxm-q200
  DEVICE_DTS := amlogic/meson-gxm-q200
  DEVICE_PACKAGES +=
  DEVICE_TITLE := GXM Q200 Development Board
endef
ifeq ($(SUBTARGET),mesongx)
  TARGET_DEVICES += gxm-q200
endif

define Device/thunder-onecloud
  DEVICE_DTS := meson8b-onecloud
  DEVICE_TITLE := Thunder OneCloud
  KERNEL_LOADADDR := 0x00208000
endef
ifeq ($(SUBTARGET),meson8b)
  TARGET_DEVICES += thunder-onecloud
endif

$(eval $(call BuildImage))
