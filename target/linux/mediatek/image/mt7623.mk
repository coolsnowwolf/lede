define Device/7623a-unielec-u7623-02-emmc-512m
  DEVICE_VENDOR := UniElec
  DEVICE_MODEL := U7623-02
  DEVICE_VARIANT := eMMC/512MB RAM
  DEVICE_DTS := mt7623a-unielec-u7623-02-emmc-512M
  DEVICE_PACKAGES := mkf2fs e2fsprogs kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1 kmod-mmc
  SUPPORTED_DEVICES := unielec,u7623-02-emmc-512m
  IMAGES := sysupgrade-emmc.bin.gz
  IMAGE/sysupgrade-emmc.bin.gz := sysupgrade-emmc | gzip | append-metadata
endef

TARGET_DEVICES += 7623a-unielec-u7623-02-emmc-512m

define Device/7623n-bananapi-bpi-r2
  DEVICE_VENDOR := LeMaker
  DEVICE_MODEL := Banana Pi R2
  DEVICE_DTS := mt7623n-bananapi-bpi-r2
endef

TARGET_DEVICES += 7623n-bananapi-bpi-r2
