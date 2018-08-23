ifeq ($(SUBTARGET),cortexa72)

define Device/armada-macchiatobin
  KERNEL_NAME := Image
  KERNEL := kernel-bin
  DEVICE_TITLE := MACCHIATObin (SolidRun Armada 8040 Community Board)
  DEVICE_PACKAGES := e2fsprogs ethtool mkf2fs kmod-fs-vfat kmod-mmc
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-scr | boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
  DEVICE_DTS := armada-8040-mcbin
  DTS_DIR := $(DTS_DIR)/marvell
  SUPPORTED_DEVICES := marvell,armada8040-mcbin
endef
TARGET_DEVICES += armada-macchiatobin

endif
