define Device/FitImage
  KERNEL_SUFFIX := -uImage.itb
  KERNEL = kernel-bin | gzip | fit gzip $$(KDIR)/image-$$(DEVICE_DTS).dtb
  KERNEL_NAME := Image
endef

define Device/UbiFit
  KERNEL_IN_UBI := 1
  IMAGES := factory.ubi sysupgrade.bin
  IMAGE/factory.ubi := append-ubi
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/globalscale_mochabin
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Globalscale
  DEVICE_MODEL := MOCHAbin
  DEVICE_PACKAGES += kmod-dsa-mv88e6xxx
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

define Device/mikrotik_rb5009
  $(call Device/Default-arm64)
  $(Device/NAND-128K)
  $(call Device/FitImage)
  $(call Device/UbiFit)
  DEVICE_VENDOR := MikroTik
  DEVICE_MODEL := RB5009
  SOC := armada-7040
  KERNEL_LOADADDR := 0x22000000
  DEVICE_PACKAGES += kmod-i2c-gpio yafut kmod-dsa-mv88e6xxx
endef
TARGET_DEVICES += mikrotik_rb5009

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
  SOC := cn9131
  DEVICE_VENDOR := iEi
  DEVICE_MODEL := Puzzle-M901
  DEVICE_PACKAGES += kmod-rtc-ds1307
endef
TARGET_DEVICES += iei_puzzle-m901

define Device/iei_puzzle-m902
  $(call Device/Default-arm64)
  SOC := cn9132
  DEVICE_VENDOR := iEi
  DEVICE_MODEL := Puzzle-M902
  DEVICE_PACKAGES += kmod-rtc-ds1307
endef
TARGET_DEVICES += iei_puzzle-m902

define Device/qnap_qhora-321
  $(call Device/Default-arm64)
  SOC := cn9131
  DEVICE_VENDOR := QNAP
  DEVICE_MODEL := QHora-321
  DEVICE_DTS := cn9131-qhora-321
  DEVICE_PACKAGES += kmod-rtc-ds1307
endef
TARGET_DEVICES += qnap_qhora-321

define Device/qnap_qhora-322
  $(call Device/Default-arm64)
  SOC := cn9132
  DEVICE_VENDOR := QNAP
  DEVICE_MODEL := QHora-322
  DEVICE_DTS := cn9132-qhora-322
  DEVICE_PACKAGES += kmod-rtc-ds1307
endef
TARGET_DEVICES += qnap_qhora-322

define Device/solidrun_clearfog-pro
  $(call Device/Default-arm64)
  SOC := cn9130
  DEVICE_VENDOR := SolidRun
  DEVICE_MODEL := ClearFog Pro
  DEVICE_PACKAGES += kmod-i2c-mux-pca954x kmod-dsa-mv88e6xxx
  BOOT_SCRIPT := clearfog-pro
endef
TARGET_DEVICES += solidrun_clearfog-pro
