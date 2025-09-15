define Device/generic
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := x86/64
  DEVICE_PACKAGES += kmod-amazon-ena kmod-amd-xgbe kmod-bnx2 \
	kmod-e1000e kmod-e1000 kmod-forcedeth kmod-fs-vfat \
	kmod-igb kmod-igc kmod-ixgbe kmod-r8168 kmod-tg3
  GRUB2_VARIANT := generic
endef
TARGET_DEVICES += generic
