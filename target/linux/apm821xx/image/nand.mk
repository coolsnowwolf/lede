define Build/create-uImage-dtb
	# flat_dt target expect FIT image - which WNDR4700's uboot doesn't support
	-$(STAGING_DIR_HOST)/bin/mkimage -A $(LINUX_KARCH) \
		-O linux -T kernel -C none \
		-n '$(call toupper,$(LINUX_KARCH)) $(VERSION_DIST) Linux-$(LINUX_VERSION)' \
		-d "$(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb" "$@.dtb.uimage"
endef

define Build/prepend-dtb-uImage
	cat "$@.dtb.uimage" "$@" > "$@.new"
	mv "$@.new" "$@"
endef

define Build/meraki-header
	-$(STAGING_DIR_HOST)/bin/mkmerakifw \
		-B $(BOARD_NAME) -s \
		-i $@ \
		-o $@.new
	@cp $@.new $@
endef


define Device/meraki_mr24
  DEVICE_VENDOR := Cisco Meraki
  DEVICE_MODEL := MR24
  DEVICE_PACKAGES := kmod-spi-gpio kmod-phy-at803x
  BOARD_NAME := mr24
  IMAGES := sysupgrade.bin
  DEVICE_DTC_FLAGS := --space 64512
  IMAGE_SIZE := 8191k
  KERNEL := kernel-bin | lzma | uImage lzma | prepend-dtb | meraki-header
  KERNEL_INITRAMFS := kernel-bin | lzma | MuImage-initramfs lzma
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  UBINIZE_OPTS := -E 5
  SUPPORTED_DEVICES += mr24
endef
TARGET_DEVICES += meraki_mr24

define Device/meraki_mx60
  DEVICE_VENDOR := Cisco Meraki
  DEVICE_MODEL := MX60
  DEVICE_ALT0_VENDOR := Cisco Meraki
  DEVICE_ALT0_MODEL := MX60W
  DEVICE_PACKAGES := kmod-spi-gpio kmod-usb-ledtrig-usbport kmod-usb-dwc2 \
		     kmod-usb-storage block-mount kmod-dsa-qca8k kmod-phy-qca83xx
  BLOCKSIZE := 128k
  IMAGES := sysupgrade.bin
  DEVICE_DTC_FLAGS := --space 20480
  IMAGE_SIZE := 1021m
  KERNEL := kernel-bin | libdeflate-gzip | MuImage-initramfs gzip
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  UBINIZE_OPTS := -E 5
  DEVICE_COMPAT_VERSION := 3.1
  DEVICE_COMPAT_MESSAGE := meraki_loadaddr of u-boot has to be adjusted before upgrade \
       to boot properly. Query https://openwrt.org/toh/meraki/mx60#upgrading_to_v2512 \
       for detail.
endef
TARGET_DEVICES += meraki_mx60

define Device/netgear_wndap6x0
  DEVICE_VENDOR := NETGEAR
  DEVICE_PACKAGES := kmod-eeprom-at24 swconfig
  SUBPAGESIZE := 256
  PAGESIZE := 512
  BLOCKSIZE := 16k
  DEVICE_DTC_FLAGS := --space 32768
  IMAGE_SIZE := 27392k
  IMAGES := sysupgrade.bin factory.img
  KERNEL_SIZE := 6080k
  KERNEL := kernel-bin | libdeflate-gzip | MuImage-initramfs gzip
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.img := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  UBINIZE_OPTS := -E 5
  DEVICE_COMPAT_VERSION := 2.0
  DEVICE_COMPAT_MESSAGE := kernel and ubi partitions had to be resized. \
       Upgrade via sysupgrade mechanism is not possible.
endef

define Device/netgear_wndap620
  $(Device/netgear_wndap6x0)
  DEVICE_MODEL := WNDAP620 (Premium Wireless-N)
endef
TARGET_DEVICES += netgear_wndap620

define Device/netgear_wndap660
  $(Device/netgear_wndap6x0)
  DEVICE_MODEL := WNDAP660 (Dual Radio Dual Band Wireless-N)
endef
TARGET_DEVICES += netgear_wndap660

define Device/netgear_wndr4700
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := Centria N900 WNDR4700
  DEVICE_ALT0_VENDOR := NETGEAR
  DEVICE_ALT0_MODEL := Centria N900 WNDR4720
  DEVICE_PACKAGES := badblocks block-mount e2fsprogs kmod-hwmon-drivetemp \
	kmod-dm kmod-fs-ext4 kmod-fs-vfat kmod-usb-ledtrig-usbport \
	kmod-md-mod kmod-nls-cp437 kmod-nls-iso8859-1 kmod-nls-iso8859-15 \
	kmod-nls-utf8 kmod-usb-xhci-pci-renesas kmod-usb-dwc2 kmod-usb-storage \
	partx-utils kmod-ata-dwc kmod-dsa-qca8k kmod-phy-qca83xx \
	kmod-hwmon-tc654 kmod-hwmon-lm90 kmod-thermal
  BOARD_NAME := wndr4700
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  BLOCKSIZE := 128k
  DEVICE_DTC_FLAGS := --space 131008
  IMAGE_SIZE := 24960k
  IMAGES := factory.img sysupgrade.bin
  ARTIFACTS := device-tree.dtb
  KERNEL_SIZE := 4608k
  # append a fake/empty rootfs to fool netgear's uboot
  # CHECK_DNI_FIRMWARE_ROOTFS_INTEGRITY in do_chk_dniimg()
  KERNEL := kernel-bin | lzma -d16 | uImage lzma | pad-offset $$(BLOCKSIZE) 64 | \
	    append-uImage-fakehdr filesystem | create-uImage-dtb | prepend-dtb-uImage
  KERNEL_INITRAMFS := kernel-bin | libdeflate-gzip | MuImage-initramfs gzip
  IMAGE/factory.img := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | \
		       netgear-dni | check-size
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  ARTIFACT/device-tree.dtb := export-dtb | uImage none
  NETGEAR_BOARD_ID := WNDR4700
  NETGEAR_HW_ID := 29763875+128+256
  UBINIZE_OPTS := -E 5
  SUPPORTED_DEVICES += wndr4700
  DEVICE_COMPAT_VERSION := 3.0
  DEVICE_COMPAT_MESSAGE := kernel and ubi partitions had to be resized. \
       Network swconfig configuration cannot be upgraded to DSA. \
       Upgrade via sysupgrade mechanism is not possible.
endef
TARGET_DEVICES += netgear_wndr4700
