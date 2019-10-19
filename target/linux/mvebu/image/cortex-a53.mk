ifeq ($(SUBTARGET),cortexa53)

define Device/globalscale_espressobin
  $(call Device/Default-arm64)
  DEVICE_TITLE := ESPRESSObin (Marvell Armada 3700 Community Board)
  DEVICE_DTS := armada-3720-espressobin
endef
TARGET_DEVICES += globalscale_espressobin

define Device/globalscale_espressobin-emmc
  $(call Device/Default-arm64)
  DEVICE_TITLE := ESPRESSObin eMMC (Marvell Armada 3700 Community Board)
  DEVICE_DTS := armada-3720-espressobin-emmc
endef
TARGET_DEVICES += globalscale_espressobin-emmc

define Device/globalscale_espressobin-v7
  $(call Device/Default-arm64)
  DEVICE_TITLE := ESPRESSObin V7 (Marvell Armada 3700 Community Board)
  DEVICE_DTS := armada-3720-espressobin-v7
endef
TARGET_DEVICES += globalscale_espressobin-v7

define Device/globalscale_espressobin-v7-emmc
  $(call Device/Default-arm64)
  DEVICE_TITLE := ESPRESSObin V7 eMMC (Marvell Armada 3700 Community Board)
  DEVICE_DTS := armada-3720-espressobin-v7-emmc
endef
TARGET_DEVICES += globalscale_espressobin-v7-emmc

define Device/marvell_armada-3720-db
  $(call Device/Default-arm64)
  DEVICE_TITLE := Marvell Armada 3720 Development Board DB-88F3720-DDR3
  DEVICE_DTS := armada-3720-db
endef
TARGET_DEVICES += marvell_armada-3720-db

define Device/methode_udpu
  $(call Device/Default-arm64)
  DEVICE_TITLE := Methode micro-DPU (uDPU)
  DEVICE_DTS := armada-3720-uDPU
  KERNEL_LOADADDR := 0x00080000
  KERNEL_INITRAMFS := kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
  KERNEL_INITRAMFS_SUFFIX := .itb
  DEVICE_PACKAGES := f2fs-tools e2fsprogs fdisk ethtool kmod-usb2 kmod-usb3 \
			kmod-e100 kmod-e1000 kmod-e1000e kmod-igb kmod-ixgbevf \
			kmod-mdio-gpio kmod-switch-mvsw61xx kmod-i2c-pxa
  IMAGE_NAME = $$(IMAGE_PREFIX)-$$(2)
  IMAGES := firmware.tgz
  IMAGE/firmware.tgz := boot-scr | boot-img-ext4 | uDPU-firmware | append-metadata
  BOOT_SCRIPT := udpu
endef
TARGET_DEVICES += methode_udpu

endif
