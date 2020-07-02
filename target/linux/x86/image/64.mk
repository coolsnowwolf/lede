define Device/generic
  DEVICE_TITLE := Generic x86/64
  DEVICE_PACKAGES += kmod-bnx2 kmod-e1000e kmod-e1000 kmod-forcedeth kmod-igb \
	kmod-ixgbe kmod-r8169
  GRUB2_VARIANT := generic
endef
TARGET_DEVICES += generic
