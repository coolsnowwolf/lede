ifeq ($(SUBTARGET),cortexa72)

define Device/marvell_macchiatobin
  $(call Device/Default-arm64)
  DEVICE_VENDOR := SolidRun
  DEVICE_MODEL := MACCHIATObin
  DEVICE_ALT0_VENDOR := SolidRun
  DEVICE_ALT0_MODEL := Armada 8040 Community Board
  DEVICE_PACKAGES += kmod-i2c-core kmod-i2c-mux kmod-i2c-mux-pca954x
  DEVICE_DTS := armada-8040-mcbin
  SUPPORTED_DEVICES := marvell,armada8040-mcbin
endef
TARGET_DEVICES += marvell_macchiatobin

define Device/marvell_armada8040-db
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := Armada 8040 Development Board
  DEVICE_DTS := armada-8040-db
  IMAGE/sdcard.img.gz := boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
endef
TARGET_DEVICES += marvell_armada8040-db

define Device/marvell_armada7040-db
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := Armada 7040 Development Board
  DEVICE_DTS := armada-7040-db
  IMAGE/sdcard.img.gz := boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
endef
TARGET_DEVICES += marvell_armada7040-db

endif
