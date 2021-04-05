define Device/glinet_gl-mv1000
  $(call Device/Default-arm64)
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-MV1000
  SOC := armada-3720
endef
TARGET_DEVICES += glinet_gl-mv1000

define Device/globalscale_espressobin
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := ESPRESSObin
  DEVICE_VARIANT := Non-eMMC
  DEVICE_ALT0_VENDOR := Marvell
  DEVICE_ALT0_MODEL := Armada 3700 Community Board
  DEVICE_ALT0_VARIANT := Non-eMMC
  SOC := armada-3720
  BOOT_SCRIPT := espressobin
endef
TARGET_DEVICES += globalscale_espressobin

define Device/globalscale_espressobin-emmc
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := ESPRESSObin
  DEVICE_VARIANT := eMMC
  DEVICE_ALT0_VENDOR := Marvell
  DEVICE_ALT0_MODEL := Armada 3700 Community Board
  DEVICE_ALT0_VARIANT := eMMC
  SOC := armada-3720
  BOOT_SCRIPT := espressobin
endef
TARGET_DEVICES += globalscale_espressobin-emmc

define Device/globalscale_espressobin-ultra
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := ESPRESSObin
  DEVICE_VARIANT := Ultra
  DEVICE_PACKAGES += kmod-i2c-pxa kmod-rtc-pcf8563
  SOC := armada-3720
  BOOT_SCRIPT := espressobin
endef
TARGET_DEVICES += globalscale_espressobin-ultra

define Device/globalscale_espressobin-v7
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := ESPRESSObin
  DEVICE_VARIANT := V7 Non-eMMC
  DEVICE_ALT0_VENDOR := Marvell
  DEVICE_ALT0_MODEL := Armada 3700 Community Board
  DEVICE_ALT0_VARIANT := V7 Non-eMMC
  SOC := armada-3720
  BOOT_SCRIPT := espressobin
endef
TARGET_DEVICES += globalscale_espressobin-v7

define Device/globalscale_espressobin-v7-emmc
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := ESPRESSObin
  DEVICE_VARIANT := V7 eMMC
  DEVICE_ALT0_VENDOR := Marvell
  DEVICE_ALT0_MODEL := Armada 3700 Community Board
  DEVICE_ALT0_VARIANT := V7 eMMC
  SOC := armada-3720
  BOOT_SCRIPT := espressobin
endef
TARGET_DEVICES += globalscale_espressobin-v7-emmc

define Device/marvell_armada-3720-db
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := Armada 3720 Development Board (DB-88F3720-DDR3)
  DEVICE_DTS := armada-3720-db
endef
TARGET_DEVICES += marvell_armada-3720-db

define Device/methode_udpu
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Methode
  DEVICE_MODEL := micro-DPU (uDPU)
  DEVICE_DTS := armada-3720-uDPU
  KERNEL_LOADADDR := 0x00080000
  KERNEL_INITRAMFS := kernel-bin | gzip | fit gzip $$(KDIR)/image-$$(DEVICE_DTS).dtb
  KERNEL_INITRAMFS_SUFFIX := .itb
  DEVICE_PACKAGES += f2fs-tools fdisk kmod-i2c-pxa
  IMAGE_NAME = $$(IMAGE_PREFIX)-$$(2)
  IMAGES := firmware.tgz
  IMAGE/firmware.tgz := boot-scr | boot-img-ext4 | uDPU-firmware | append-metadata
  BOOT_SCRIPT := udpu
endef
TARGET_DEVICES += methode_udpu
