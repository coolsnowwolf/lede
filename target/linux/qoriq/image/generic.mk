define Device/watchguard_firebox-m300
  DEVICE_VENDOR := WatchGuard
  DEVICE_MODEL := Firebox M300
  DEVICE_DTS_DIR := $(DTS_DIR)/fsl
  DEVICE_PACKAGES := \
	kmod-hwmon-w83793 kmod-ptp-qoriq kmod-rtc-rs5c372a kmod-tpm-i2c-atmel
  KERNEL := kernel-bin | gzip | uImage gzip
  IMAGES := sdcard.img.gz sysupgrade.img.gz
  IMAGE/sysupgrade.img.gz :=  sdcard-img | gzip | append-metadata
  IMAGE/sdcard.img.gz := sdcard-img | gzip
endef
TARGET_DEVICES += watchguard_firebox-m300
