DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID

define Device/dsa-migration
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_COMPAT_MESSAGE := Config cannot be migrated from swconfig to DSA
endef

define Device/alphanetworks_asl56026
  $(Device/dsa-migration)
  DEVICE_VENDOR := Alpha
  DEVICE_MODEL := ASL56026
  DEVICE_ALT0_VENDOR := BT Openreach
  DEVICE_ALT0_MODEL := ECI VDSL Modem V-2FUb/I
  IMAGE_SIZE := 7488k
  DEVICE_PACKAGES := xrx200-rev1.1-phy22f-firmware xrx200-rev1.2-phy22f-firmware
  DEFAULT := n
endef
TARGET_DEVICES += alphanetworks_asl56026

define Device/arcadyan_vg3503j
  $(Device/dsa-migration)
  DEVICE_VENDOR := BT Openreach
  DEVICE_MODEL := ECI VDSL Modem V-2FUb/R
  IMAGE_SIZE := 8000k
  SUPPORTED_DEVICES += VG3503J
  DEVICE_PACKAGES := xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware
  DEFAULT := n
endef
TARGET_DEVICES += arcadyan_vg3503j

define Device/netgear_dm200
  $(Device/dsa-migration)
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := DM200
  IMAGES := sysupgrade.bin factory.img
  IMAGE/sysupgrade.bin := append-kernel | \
	pad-offset 64k 64 | append-uImage-fakehdr filesystem | \
	pad-offset 64k 64 | append-uImage-fakehdr filesystem | \
	append-rootfs | pad-rootfs | check-size | append-metadata
  IMAGE/factory.img := $$(IMAGE/sysupgrade.bin) | netgear-dni
  IMAGE_SIZE := 7872k
  NETGEAR_BOARD_ID := DM200
  NETGEAR_HW_ID := 29765233+8+0+64+0+0
  DEVICE_PACKAGES := xrx200-rev1.1-phy22f-firmware xrx200-rev1.2-phy22f-firmware
endef
TARGET_DEVICES += netgear_dm200
