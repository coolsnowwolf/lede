ifeq ($(SUBTARGET),cortexa72)

define Device/marvell_macchiatobin
  $(call Device/Default-arm64)
  DEVICE_TITLE := MACCHIATObin (SolidRun Armada 8040 Community Board)
  DEVICE_PACKAGES += kmod-i2c-core kmod-i2c-mux kmod-i2c-mux-pca954x
  DEVICE_DTS := armada-8040-mcbin
  SUPPORTED_DEVICES := marvell,armada8040-mcbin
endef
TARGET_DEVICES += marvell_macchiatobin

define Device/marvell_armada8040-db
  $(call Device/Default-arm64)
  DEVICE_TITLE := Marvell Armada 8040 DB board
  DEVICE_DTS := armada-8040-db
  IMAGE/sdcard.img.gz := boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
endef
TARGET_DEVICES += marvell_armada8040-db

define Device/marvell_armada7040-db
  $(call Device/Default-arm64)
  DEVICE_TITLE := Marvell Armada 7040 DB board
  DEVICE_DTS := armada-7040-db
  IMAGE/sdcard.img.gz := boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
endef
TARGET_DEVICES += marvell_armada7040-db

endif
