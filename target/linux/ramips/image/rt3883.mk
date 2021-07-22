#
# RT3662/RT3883 Profiles
#
define Build/mkrtn56uimg
	$(STAGING_DIR_HOST)/bin/mkrtn56uimg $(1) $@
endef

define Device/asus_rt-n56u
  SOC := rt3662
  BLOCKSIZE := 64k
  IMAGE_SIZE := 7872k
  IMAGE/sysupgrade.bin += | mkrtn56uimg -s
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := RT-N56U
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2
  SUPPORTED_DEVICES += rt-n56u
endef
TARGET_DEVICES += asus_rt-n56u

define Device/belkin_f9k1109v1
  SOC := rt3883
  BLOCKSIZE := 64k
  DEVICE_VENDOR := Belkin
  DEVICE_MODEL := F9K1109
  DEVICE_VARIANT := Version 1.0
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport
  IMAGE_SIZE := 7808k
  KERNEL := kernel-bin | append-dtb | lzma -d16 | uImage lzma
  # Stock firmware checks for this uImage image name during upload.
  UIMAGE_NAME := N750F9K1103VB
endef
TARGET_DEVICES += belkin_f9k1109v1

define Device/dlink_dir-645
  $(Device/seama)
  $(Device/uimage-lzma-loader)
  SOC := rt3662
  BLOCKSIZE := 4k
  IMAGE_SIZE := 7872k
  KERNEL := kernel-bin | append-dtb | lzma -d10
  SEAMA_SIGNATURE := wrgn39_dlob.hans_dir645
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DIR-645
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2
  SUPPORTED_DEVICES += dir-645
endef
TARGET_DEVICES += dlink_dir-645

define Device/edimax_br-6475nd
  SOC := rt3662
  BLOCKSIZE := 64k
  IMAGE_SIZE := 7744k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
	edimax-header -s CSYS -m RN54 -f 0x70000 -S 0x01100000 | pad-rootfs | \
	check-size | append-metadata
  DEVICE_VENDOR := Edimax
  DEVICE_MODEL := BR-6475nD
  SUPPORTED_DEVICES += br-6475nd
endef
TARGET_DEVICES += edimax_br-6475nd

define Device/engenius_esr600h
  $(Device/uimage-lzma-loader)
  SOC := rt3662
  BLOCKSIZE := 4k
  IMAGE_SIZE := 7872k
  IMAGES += factory.dlf
  IMAGE/factory.dlf := $$(sysupgrade_bin) | check-size | \
	senao-header -r 0x101 -p 0x44 -t 2
  DEVICE_VENDOR := EnGenius
  DEVICE_MODEL := ESR600H
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 uboot-envtools
endef
TARGET_DEVICES += engenius_esr600h

define Device/loewe_wmdr-143n
  SOC := rt3662
  BLOCKSIZE := 64k
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Loewe
  DEVICE_MODEL := WMDR-143N
  SUPPORTED_DEVICES += wmdr-143n
endef
TARGET_DEVICES += loewe_wmdr-143n

define Device/omnima_hpm
  SOC := rt3662
  BLOCKSIZE := 64k
  IMAGE_SIZE := 16064k
  DEVICE_VENDOR := Omnima
  DEVICE_MODEL := HPM
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2
  SUPPORTED_DEVICES += hpm
endef
TARGET_DEVICES += omnima_hpm

define Device/samsung_cy-swr1100
  $(Device/seama)
  SOC := rt3662
  BLOCKSIZE := 64k
  IMAGE_SIZE := 7872k
  KERNEL := $(KERNEL_DTB)
  SEAMA_SIGNATURE := wrgnd10_samsung_ss815
  DEVICE_VENDOR := Samsung
  DEVICE_MODEL := CY-SWR1100
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += cy-swr1100
endef
TARGET_DEVICES += samsung_cy-swr1100

define Device/sitecom_wlr-6000
  SOC := rt3883
  BLOCKSIZE := 4k
  IMAGE_SIZE := 7244k
  IMAGES += factory.dlf
  IMAGE/factory.dlf := $$(sysupgrade_bin) | check-size | \
	senao-header -r 0x0202 -p 0x41 -t 2
  DEVICE_VENDOR := Sitecom
  DEVICE_MODEL := WLR-6000
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2
  SUPPORTED_DEVICES += wlr-6000
endef
TARGET_DEVICES += sitecom_wlr-6000

define Device/trendnet_tew-691gr
  SOC := rt3883
  BLOCKSIZE := 64k
  IMAGE_SIZE := 7872k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | umedia-header 0x026910
  DEVICE_VENDOR := TRENDnet
  DEVICE_MODEL := TEW-691GR
  SUPPORTED_DEVICES += tew-691gr
endef
TARGET_DEVICES += trendnet_tew-691gr

define Device/trendnet_tew-692gr
  SOC := rt3883
  BLOCKSIZE := 64k
  IMAGE_SIZE := 7872k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | umedia-header 0x026920
  DEVICE_VENDOR := TRENDnet
  DEVICE_MODEL := TEW-692GR
  SUPPORTED_DEVICES += tew-692gr
endef
TARGET_DEVICES += trendnet_tew-692gr
