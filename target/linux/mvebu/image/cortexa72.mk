define Device/globalscale_mochabin
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Globalscale
  DEVICE_MODEL := MOCHAbin
  SOC := armada-7040
endef
TARGET_DEVICES += globalscale_mochabin

define Device/marvell_armada7040-db
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := Armada 7040 Development Board
  DEVICE_DTS := armada-7040-db
  IMAGE/sdcard.img.gz := boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
endef
TARGET_DEVICES += marvell_armada7040-db

define Device/marvell_armada8040-db
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := Armada 8040 Development Board
  DEVICE_DTS := armada-8040-db
  IMAGE/sdcard.img.gz := boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
endef
TARGET_DEVICES += marvell_armada8040-db

define Device/marvell_macchiatobin-doubleshot
  $(call Device/Default-arm64)
  DEVICE_VENDOR := SolidRun
  DEVICE_MODEL := MACCHIATObin
  DEVICE_VARIANT := Double Shot
  DEVICE_ALT0_VENDOR := SolidRun
  DEVICE_ALT0_MODEL := Armada 8040 Community Board
  DEVICE_ALT0_VARIANT := Double Shot
  DEVICE_PACKAGES += kmod-i2c-mux-pca954x
  DEVICE_DTS := armada-8040-mcbin
  SUPPORTED_DEVICES := marvell,armada8040-mcbin-doubleshot marvell,armada8040-mcbin
endef
TARGET_DEVICES += marvell_macchiatobin-doubleshot

define Device/marvell_macchiatobin-singleshot
  $(call Device/Default-arm64)
  DEVICE_VENDOR := SolidRun
  DEVICE_MODEL := MACCHIATObin
  DEVICE_VARIANT := Single Shot
  DEVICE_ALT0_VENDOR := SolidRun
  DEVICE_ALT0_MODEL := Armada 8040 Community Board
  DEVICE_ALT0_VARIANT := Single Shot
  DEVICE_PACKAGES += kmod-i2c-mux-pca954x
  DEVICE_DTS := armada-8040-mcbin-singleshot
  SUPPORTED_DEVICES := marvell,armada8040-mcbin-singleshot
endef
TARGET_DEVICES += marvell_macchiatobin-singleshot

define Device/marvell_clearfog-gt-8k
  $(call Device/Default-arm64)
  DEVICE_VENDOR := SolidRun
  DEVICE_MODEL := Clearfog
  DEVICE_VARIANT := GT-8K
  DEVICE_PACKAGES += kmod-i2c-mux-pca954x kmod-crypto-hw-safexcel
  DEVICE_DTS := armada-8040-clearfog-gt-8k
  SUPPORTED_DEVICES := marvell,armada8040-clearfog-gt-8k
endef
TARGET_DEVICES += marvell_clearfog-gt-8k

define Device/iei_puzzle-m901
  $(call Device/Default-arm64)
  DEVICE_VENDOR := iEi
  DEVICE_MODEL := Puzzle-M901
  SOC := cn9131
endef
TARGET_DEVICES += iei_puzzle-m901

define Device/iei_puzzle-m902
  $(call Device/Default-arm64)
  DEVICE_VENDOR := iEi
  DEVICE_MODEL := Puzzle-M902
  SOC := cn9132
endef
TARGET_DEVICES += iei_puzzle-m902

define Device/qnap_qhora-32x
  $(call Device/Default-arm64)
  SOC := cn9132
  DEVICE_VENDOR := QNAP
  DEVICE_MODEL := QHora-321/322
  DEVICE_PACKAGES += kmod-rtc-ds1307
  DEVICE_DTS := cn9131-db-A cn9131-puzzle-m901 cn9132-db-A cn9132-puzzle-m902
  SUPPORTED_DEVICES := qnap,qhora-321 qnap,qhora-322 iei,puzzle-m901 iei,puzzle-m902
  IMAGE/sdcard.img.gz := boot-scr | boot-qnap-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
endef
TARGET_DEVICES += qnap_qhora-32x
