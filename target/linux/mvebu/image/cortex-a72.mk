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

define Device/armada-8040-db
  KERNEL_NAME := Image
  KERNEL := kernel-bin
  DEVICE_TITLE := Marvell Armada 8040 DB board
  DEVICE_PACKAGES := e2fsprogs ethtool mkf2fs kmod-fs-vfat kmod-mmc
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
  DEVICE_DTS := armada-8040-db
  DTS_DIR := $(DTS_DIR)/marvell
  SUPPORTED_DEVICES := marvell,armada8040-db
endef
TARGET_DEVICES += armada-8040-db

define Device/armada-7040-db
  KERNEL_NAME := Image
  KERNEL := kernel-bin
  DEVICE_TITLE := Marvell Armada 7040 DB board
  DEVICE_PACKAGES := e2fsprogs ethtool mkf2fs kmod-fs-vfat kmod-mmc
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
  DEVICE_DTS := armada-7040-db
  DTS_DIR := $(DTS_DIR)/marvell
  SUPPORTED_DEVICES := marvell,armada7040-db
endef
TARGET_DEVICES += armada-7040-db

endif
