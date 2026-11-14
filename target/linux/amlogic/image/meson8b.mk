
define Device/Default
  FILESYSTEMS := ext4
  IMAGES := emmc.img$(if $(CONFIG_TARGET_IMAGES_GZIP),.gz)
  DEVICE_DTS_DIR := $(DTS_DIR)/amlogic
  KERNEL_DEPENDS = $$(wildcard $(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL_LOADADDR := 0x01080000
  KERNEL_NAME := Image
  KERNEL := kernel-bin | uImage none
  PROFILES = Default $$(DEVICE_NAME)
endef

define Device/thunder-onecloud
  DEVICE_DTS := meson8b-onecloud
  DEVICE_TITLE := Thunder OneCloud
  KERNEL_LOADADDR := 0x00208000
  IMAGE/emmc.img := boot-script onecloud | emmc-common $$(DEVICE_NAME)
  IMAGE/emmc.img.gz := $$(IMAGE/emmc.img) | gzip
endef
TARGET_DEVICES += thunder-onecloud
