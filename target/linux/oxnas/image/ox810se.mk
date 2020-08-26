KERNEL_LOADADDR := 0x48008000

define Device/Default
  KERNEL_NAME := zImage
  KERNEL_SUFFIX := -uImage
  KERNEL_INSTALL := 1
  FILESYSTEMS := squashfs ext4
  PROFILES := Default
  SUPPORTED_DEVICES := $(subst _,$(comma),$(1))
  DEVICE_DTS := ox810se-$(subst _,-,$(1))
  IMAGES := sysupgrade.tar
  IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
endef

define Device/wd_mbwe
  DEVICE_VENDOR := Western Digital
  DEVICE_MODEL := My Book
  DEVICE_VARIANT := World Edition
  KERNEL := kernel-bin | append-dtb | uImage none
endef
TARGET_DEVICES += wd_mbwe
