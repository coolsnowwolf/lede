KERNEL_LOADADDR := 0x80008000

define Device/bpi_bananapi-r2
  DEVICE_VENDOR := Bpi
  DEVICE_MODEL := Banana Pi R2
  DEVICE_DTS := mt7623n-bananapi-bpi-r2
  SUPPORTED_DEVICES := bananapi,bpi-r2
endef
TARGET_DEVICES += bpi_bananapi-r2

define Device/unielec_u7623-02-emmc-512m
  DEVICE_VENDOR := UniElec
  DEVICE_MODEL := U7623-02
  DEVICE_VARIANT := eMMC/512MB RAM
  DEVICE_DTS := mt7623a-unielec-u7623-02-emmc-512m
  DEVICE_PACKAGES := kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1 kmod-mmc \
	mkf2fs e2fsprogs
  IMAGES := sysupgrade-emmc.bin.gz
  IMAGE/sysupgrade-emmc.bin.gz := sysupgrade-emmc | gzip | append-metadata
endef
TARGET_DEVICES += unielec_u7623-02-emmc-512m
