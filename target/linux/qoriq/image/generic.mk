define Device/fsl_T4240RDB
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := T4240RDB
  DEVICE_DTS_DIR := $(DTS_DIR)/fsl
  DEVICE_PACKAGES := \
    kmod-eeprom-at24 kmod-gpio-button-hotplug kmod-hwmon-w83793 kmod-leds-gpio \
	  kmod-ptp-qoriq kmod-rtc-ds1374
  FILESYSTEMS := squashfs
  KERNEL := kernel-bin | libdeflate-gzip | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  IMAGES := factory-nor.bin.gz sdcard.img.gz rcw.bin sysupgrade.bin
  IMAGE/factory-nor.bin.gz := \
    append-kernel | append-rootfs | pad-rootfs | pad-to 126M | \
    pad-to 127M | \
    ls-append fsl_t4240-rdb-fman.bin | pad-to 130176k | \
    ls-append $(1)-nor-uboot-env.bin | pad-to 130304k | \
    ls-append $(1)-nor-uboot.bin | libdeflate-gzip
  IMAGE/rcw.bin := ls-append $(1)-rcw.bin
  IMAGE/sdcard.img.gz := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 4K | \
    ls-append $(1)-sdboot-uboot.bin | pad-to 1028k | \
    ls-append $(1)-sdboot-uboot-env.bin | pad-to 1040k | \
    ls-append fsl_t4240-rdb-fman.bin | pad-to 2M | \
    pad-to $(LS_SD_KERNELPART_OFFSET)M | \
    ls-append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | pad-to $(LS_SD_IMAGE_SIZE)M | libdeflate-gzip
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += fsl_T4240RDB

define Device/watchguard_firebox-m300
  DEVICE_VENDOR := WatchGuard
  DEVICE_MODEL := Firebox M300
  DEVICE_COMPAT_MESSAGE := \
	Kernel switched to FIT uImage. Update U-Boot environment.
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_DTS_DIR := $(DTS_DIR)/fsl
  DEVICE_PACKAGES := \
	kmod-gpio-button-hotplug kmod-hwmon-w83793 kmod-leds-gpio kmod-ptp-qoriq \
	kmod-rtc-rs5c372a kmod-tpm-i2c-atmel kmod-dsa-mv88e6xxx
  KERNEL := kernel-bin | libdeflate-gzip | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  KERNEL_SUFFIX := -fit-uImage.itb
  IMAGES := sdcard.img.gz sysupgrade.img.gz
  IMAGE/sysupgrade.img.gz :=  sdcard-img | libdeflate-gzip | append-metadata
  IMAGE/sdcard.img.gz := sdcard-img | libdeflate-gzip
endef
TARGET_DEVICES += watchguard_firebox-m300
