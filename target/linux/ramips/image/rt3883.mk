#
# RT3662/RT3883 Profiles
#
define Build/mkrtn56uimg
	$(STAGING_DIR_HOST)/bin/mkrtn56uimg $(1) $@
endef

define Device/br-6475nd
  DTS := BR-6475ND
  BLOCKSIZE := 64k
  IMAGE_SIZE := 7744k
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | \
	edimax-header -s CSYS -m RN54 -f 0x70000 -S 0x01100000 | pad-rootfs | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  DEVICE_TITLE := Edimax BR-6475nD
  DEVICE_PACKAGES := swconfig
endef
TARGET_DEVICES += br-6475nd

define Device/cy-swr1100
  $(Device/seama)
  DTS := CY-SWR1100
  BLOCKSIZE := 64k
  KERNEL := $(KERNEL_DTB)
  SEAMA_SIGNATURE := wrgnd10_samsung_ss815
  DEVICE_TITLE := Samsung CY-SWR1100
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 \
	kmod-usb-ledtrig-usbport swconfig
endef
TARGET_DEVICES += cy-swr1100


define Device/dir-645
  $(Device/seama)
  DTS := DIR-645
  BLOCKSIZE := 4k
  KERNEL := $(KERNEL_DTB)
  SEAMA_SIGNATURE := wrgn39_dlob.hans_dir645
  DEVICE_TITLE := D-Link DIR-645
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 swconfig
  DEFAULT := n
endef
TARGET_DEVICES += dir-645


define Device/belkin_f9k1109v1
  DTS := F9K1109V1
  BLOCKSIZE := 64k
  DEVICE_TITLE := Belkin F9K1109 Version 1.0
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 swconfig
  IMAGE_SIZE := 7224k
  KERNEL := kernel-bin | append-dtb | lzma -d16 | uImage lzma
  # Stock firmware checks for this uImage image name during upload.
  UIMAGE_NAME := N750F9K1103VB
endef
TARGET_DEVICES += belkin_f9k1109v1

define Device/hpm
  DTS := HPM
  BLOCKSIZE := 64k
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := Omnima HPM
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2
endef
TARGET_DEVICES += hpm


define Device/rt-n56u
  DTS := RT-N56U
  BLOCKSIZE := 64k
  IMAGE/sysupgrade.bin += | mkrtn56uimg -s
  DEVICE_TITLE := Asus RT-N56U
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 swconfig
endef
TARGET_DEVICES += rt-n56u


define Device/tew-691gr
  DTS := TEW-691GR
  BLOCKSIZE := 64k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	umedia-header 0x026910
  DEVICE_TITLE := TRENDnet TEW-691GR
  DEVICE_PACKAGES := swconfig
endef
TARGET_DEVICES += tew-691gr


define Device/tew-692gr
  DTS := TEW-692GR
  BLOCKSIZE := 64k
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	umedia-header 0x026920
  DEVICE_TITLE := TRENDnet TEW-692GR
  DEVICE_PACKAGES := swconfig
endef
TARGET_DEVICES += tew-692gr


define Device/wlr-6000
  DTS := WLR-6000
  BLOCKSIZE := 4k
  IMAGE_SIZE := 7244k
  IMAGES += factory.dlf
  IMAGE/factory.dlf := $$(sysupgrade_bin) | check-size $$$$(IMAGE_SIZE) | \
	senao-header -r 0x0202 -p 0x41 -t 2
  DEVICE_TITLE := Sitecom WLR-6000
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-ohci kmod-usb2 swconfig
endef
TARGET_DEVICES += wlr-6000


define Device/wmdr-143n
  DTS := WMDR-143N
  BLOCKSIZE := 64k
  DEVICE_TITLE := Loewe WMDR-143N
endef
TARGET_DEVICES += wmdr-143n
