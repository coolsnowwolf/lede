define Device/generic
  DEVICE_TITLE := Generic x86
  DEVICE_PACKAGES += kmod-3c59x kmod-8139too kmod-e100 kmod-e1000 kmod-natsemi \
	kmod-ne2k-pci kmod-pcnet32 kmod-sis900 kmod-tg3 \
	kmod-via-rhine kmod-via-velocity kmod-forcedeth
  GRUB2_VARIANT := generic
endef
TARGET_DEVICES += generic
