define Device/generic
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := x86/64
  DEVICE_PACKAGES += kmod-amazon-ena kmod-bnx2 kmod-e1000e kmod-e1000 \
	kmod-forcedeth kmod-ixgbe kmod-amd-xgbe kmod-r8168 kmod-fs-vfat
  GRUB2_VARIANT := generic
endef
TARGET_DEVICES += generic
