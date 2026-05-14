
define Device/Default
  PROFILES = Default $$(DEVICE_NAME)
  BLOCKSIZE := 64k
  KERNEL = kernel-bin | lzma
  KERNEL_INITRAMFS = kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 128k
  KERNEL_LOADADDR := 0x20000000
  FILESYSTEMS := squashfs
  DEVICE_DTS_DIR := ../dts
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin = append-kernel | fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-static-with-rootfs | pad-rootfs | append-metadata
endef

define Device/NAND
  KERNEL := kernel-bin | gzip
  KERNEL_INITRAMFS = kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb with-initrd | pad-to 128k
  IMAGE/sysupgrade.bin = append-kernel | fit gzip $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb external-static-with-rootfs | append-metadata
endef

define Device/bananapi_bpi-rv2
  DEVICE_VENDOR := Bananapi
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-phy-sf21-usb kmod-phy-airoha-en8811h kmod-rtc-pcf8563 kmod-i2c-designware-platform
endef

define Device/bananapi_bpi-rv2-nand
  DEVICE_MODEL := BPi-RV2 (Booting from NAND)
  DEVICE_DTS := sf21h8898_bananapi_bpi-rv2-nand
  $(call Device/bananapi_bpi-rv2)
endef
TARGET_DEVICES += bananapi_bpi-rv2-nand

define Device/bananapi_bpi-rv2-nor
  DEVICE_MODEL := BPi-RV2 (Booting from NOR)
  DEVICE_DTS := sf21h8898_bananapi_bpi-rv2-nor
  $(call Device/bananapi_bpi-rv2)
endef
TARGET_DEVICES += bananapi_bpi-rv2-nor
