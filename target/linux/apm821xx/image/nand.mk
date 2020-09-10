define Build/create-uImage-dtb
	# flat_dt target expect FIT image - which WNDR4700's uboot doesn't support
	-$(STAGING_DIR_HOST)/bin/mkimage -A $(LINUX_KARCH) \
		-O linux -T kernel -C none \
		-n '$(call toupper,$(LINUX_KARCH)) $(VERSION_DIST) Linux-$(LINUX_VERSION)' \
		-d "$@.dtb" "$@.dtb.uimage"
endef

define Build/MerakiAdd-dtb
	$(call Image/BuildDTB,../dts/$(DEVICE_DTS).dts,$@.dtb)
	( \
		dd if=$@.dtb bs=$(DTB_SIZE) conv=sync; \
		cat $@ ; \
	) > $@.new
	@mv $@.new $@
endef

define Build/MerakiNAND
	-$(STAGING_DIR_HOST)/bin/mkmerakifw \
		-B $(BOARD_NAME) -s \
		-i $@ \
		-o $@.new
	@cp $@.new $@
endef


define Device/meraki_mr24
  DEVICE_VENDOR := Cisco Meraki
  DEVICE_MODEL := MR24
  DEVICE_PACKAGES := kmod-spi-gpio -swconfig
  BOARD_NAME := mr24
  IMAGES := sysupgrade.bin
  DTB_SIZE := 64512
  IMAGE_SIZE := 8191k
  KERNEL := kernel-bin | lzma | uImage lzma | MerakiAdd-dtb | MerakiNAND
  KERNEL_INITRAMFS := kernel-bin | lzma | dtb | MuImage-initramfs lzma
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  UBINIZE_OPTS := -E 5
  SUPPORTED_DEVICES += mr24
endef
TARGET_DEVICES += meraki_mr24

define Device/meraki_mx60
  DEVICE_VENDOR := Cisco Meraki
  DEVICE_MODEL := MX60/MX60W
  DEVICE_PACKAGES := kmod-spi-gpio kmod-usb-ledtrig-usbport kmod-usb-dwc2 \
		     kmod-usb-storage block-mount
  BOARD_NAME := mx60
  BLOCKSIZE := 63k
  IMAGES := sysupgrade.bin
  DTB_SIZE := 64512
  IMAGE_SIZE := 1021m
  KERNEL_SIZE := 4031k
  KERNEL := kernel-bin | gzip | uImage gzip | MerakiAdd-dtb | MerakiNAND
  KERNEL_INITRAMFS := kernel-bin | gzip | dtb | MuImage-initramfs gzip
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  UBINIZE_OPTS := -E 5
  SUPPORTED_DEVICES += mx60
endef
TARGET_DEVICES += meraki_mx60

define Device/netgear_wndap6x0
  DEVICE_VENDOR := NETGEAR
  DEVICE_PACKAGES := kmod-eeprom-at24
  SUBPAGESIZE := 256
  PAGESIZE := 512
  BLOCKSIZE := 16k
  DTB_SIZE := 32768
  IMAGE_SIZE := 27392k
  IMAGES := sysupgrade.bin factory.img
  KERNEL_SIZE := 4032k
  KERNEL := dtb | kernel-bin | gzip | MuImage-initramfs gzip
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.img := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  UBINIZE_OPTS := -E 5
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
  DEVICE_MODEL := Centria N900 WNDR4700/WNDR4720
  DEVICE_PACKAGES := badblocks block-mount e2fsprogs kmod-hwmon-drivetemp \
	kmod-dm kmod-fs-ext4 kmod-fs-vfat kmod-usb-ledtrig-usbport \
	kmod-md-mod kmod-nls-cp437 kmod-nls-iso8859-1 kmod-nls-iso8859-15 \
	kmod-nls-utf8 kmod-usb3 kmod-usb-dwc2 kmod-usb-storage \
	partx-utils
  BOARD_NAME := wndr4700
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  BLOCKSIZE := 128k
  DTB_SIZE := 131008
  IMAGE_SIZE := 24960k
  IMAGES := factory.img sysupgrade.bin
  ARTIFACTS := device-tree.dtb
  KERNEL_SIZE := 3584k
  # append a fake/empty rootfs to fool netgear's uboot
  # CHECK_DNI_FIRMWARE_ROOTFS_INTEGRITY in do_chk_dniimg()
  KERNEL := kernel-bin | lzma | uImage lzma | pad-offset $$(BLOCKSIZE) 64 | \
	    append-uImage-fakehdr filesystem | dtb | create-uImage-dtb | prepend-dtb
  KERNEL_INITRAMFS := kernel-bin | gzip | dtb | MuImage-initramfs gzip
  IMAGE/factory.img := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | \
		       netgear-dni | check-size
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  ARTIFACT/device-tree.dtb := export-dtb | uImage none
  NETGEAR_BOARD_ID := WNDR4700
  NETGEAR_HW_ID := 29763875+128+256
  UBINIZE_OPTS := -E 5
  SUPPORTED_DEVICES += wndr4700
  DEFAULT := n
endef
TARGET_DEVICES += netgear_wndr4700
