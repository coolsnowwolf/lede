KERNEL_LOADADDR := 0x48008000

define Device/Default
  KERNEL_NAME := zImage
  KERNEL_SUFFIX := -uImage
  KERNEL_INSTALL := 1
  FILESYSTEMS := squashfs ext4
  PROFILES = Default $$(DTS)
  IMAGES := sysupgrade.tar
  IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
endef

define Device/wd-mbwe
  DEVICE_DTS := ox810se-wd-mbwe
  DEVICE_TITLE := Western Digital My Book World Edition
  KERNEL := kernel-bin | append-dtb | uImage none
endef
TARGET_DEVICES += wd-mbwe
