define Device/cznic_turris-mox
  $(call Device/Default-arm64)
  DEVICE_VENDOR := CZ.NIC
  DEVICE_MODEL := Turris MOX
  DEVICE_PACKAGES += kmod-usb2 kmod-usb3 \
    kmod-rtc-ds1307 kmod-i2c-pxa kmod-dsa kmod-dsa-mv88e6xxx kmod-sfp \
    kmod-phy-marvell kmod-phy-marvell-10g kmod-ath10k ath10k-board-qca988x \
    ath10k-firmware-qca988x kmod-mt7915e kmod-mt7915-firmware mwlwifi-firmware-88w8997 \
    wpad-basic-mbedtls kmod-mwifiex-sdio kmod-btmrvl
  SOC := armada-3720
  BOOT_SCRIPT := turris-mox
endef
TARGET_DEVICES += cznic_turris-mox

define Device/glinet_gl-mv1000
  $(call Device/Default-arm64)
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-MV1000
  DEVICE_PACKAGES += kmod-dsa-mv88e6xxx
  SOC := armada-3720
  BOOT_SCRIPT := gl-mv1000
endef
TARGET_DEVICES += glinet_gl-mv1000

define Device/globalscale_espressobin
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := ESPRESSObin
  DEVICE_VARIANT := Non-eMMC
  DEVICE_PACKAGES += kmod-dsa-mv88e6xxx
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
  DEVICE_PACKAGES += kmod-dsa-mv88e6xxx
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
  DEVICE_PACKAGES += kmod-i2c-pxa kmod-rtc-pcf8563 kmod-dsa-mv88e6xxx
  SOC := armada-3720
  BOOT_SCRIPT := espressobin
endef
TARGET_DEVICES += globalscale_espressobin-ultra

define Device/globalscale_espressobin-v7
  $(call Device/Default-arm64)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := ESPRESSObin
  DEVICE_VARIANT := V7 Non-eMMC
  DEVICE_PACKAGES += kmod-dsa-mv88e6xxx
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
  DEVICE_PACKAGES += kmod-dsa-mv88e6xxx
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

define Build/methode-gpt-emmc
  cp $@ $@.tmp 2>/dev/null || true
  ptgen -g -o $@.tmp -l 1024 \
      -t 0x2e -N kernel_1 -r -B -p 32M@1M \
      -t 0x2e -N rootfs_1 -r -p 1536M@33M \
      -t 0x2e -N kernel_2 -r -B -p 32M@1569M \
      -t 0x2e -N rootfs_2 -r -p 1536M@1601M
  cat $@.tmp >> $@
  rm $@.tmp
endef

define Build/append-boot-part
  dd if=$@.bootimg bs=32M conv=sync >> $@
endef

define Device/eMMC-methode
  DEVICE_DTS_DIR := $(DTS_DIR)/marvell
  KERNEL_NAME := Image
  KERNEL := kernel-bin
  KERNEL_LOADADDR := 0x00800000
  DEVICE_PACKAGES += kmod-i2c-pxa kmod-hwmon-lm75 kmod-dsa-mv88e6xxx
  DEVICE_COMPAT_VERSION := 2.0
  DEVICE_COMPAT_MESSAGE := Partition layout and image format was changed. \
  Upgrade requires reinstallation from initramfs.
  FILESYSTEMS := squashfs
  IMAGES := sysupgrade.bin emmc-gpt.img.gz
  IMAGE/sysupgrade.bin := boot-scr | boot-img-ext4 | sysupgrade-tar kernel=$$$$@.bootimg | append-metadata
  IMAGE/emmc-gpt.img.gz := methode-gpt-emmc |\
    pad-to 1M  | boot-scr | boot-img-ext4 | append-boot-part |\
    pad-to 33M | append-rootfs |\
    libdeflate-gzip
  BOOT_SCRIPT := udpu
endef

define Device/methode_udpu
  $(call Device/eMMC-methode)
  $(call Device/FitImage)
  DEVICE_VENDOR := Methode
  DEVICE_MODEL := micro-DPU (uDPU)
  DEVICE_DTS := armada-3720-uDPU
endef
TARGET_DEVICES += methode_udpu

define Device/methode_edpu
  $(call Device/methode_udpu)
  DEVICE_MODEL := eDPU
  DEVICE_DTS := armada-3720-eDPU
endef
TARGET_DEVICES += methode_edpu

define Device/ripe_atlas-v5
  $(call Device/Default-arm64)
  DEVICE_VENDOR := RIPE
  DEVICE_MODEL := Atlas v5
  SOC := armada-3720
  BOOT_SCRIPT := ripe-atlas
endef
TARGET_DEVICES += ripe_atlas-v5
