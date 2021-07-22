#
# RT288X Profiles
#

DEFAULT_SOC := rt2880

define Build/gemtek-header
	if [ -f $@ ]; then \
		mkheader_gemtek $@ $@.new $(1) && \
		mv $@.new $@; \
	fi
endef

define Device/airlink101_ar670w
  BLOCKSIZE := 64k
  DEVICE_VENDOR := Airlink
  DEVICE_MODEL := AR670W
  IMAGE_SIZE := 3840k
  KERNEL := $(KERNEL_DTB) | pad-to $$(BLOCKSIZE)
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size | \
	wrg-header wrgn16a_airlink_ar670w
  SUPPORTED_DEVICES += ar670w
  DEFAULT := n
endef
TARGET_DEVICES += airlink101_ar670w

define Device/airlink101_ar725w
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Airlink
  DEVICE_MODEL := AR725W
  IMAGES += factory.bin
  IMAGE/factory.bin := $$(sysupgrade_bin) | check-size 3328k | \
	gemtek-header ar725w
  SUPPORTED_DEVICES += ar725w
  DEFAULT := n
endef
TARGET_DEVICES += airlink101_ar725w

define Device/asus_rt-n15
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Asus
  DEVICE_MODEL := RT-N15
  DEVICE_PACKAGES := kmod-switch-rtl8366s
  SUPPORTED_DEVICES += rt-n15
  DEFAULT := n
endef
TARGET_DEVICES += asus_rt-n15

define Device/belkin_f5d8235-v1
  IMAGE_SIZE := 7872k
  DEVICE_VENDOR := Belkin
  DEVICE_MODEL := F5D8235
  DEVICE_VARIANT := V1
  DEVICE_PACKAGES := kmod-switch-rtl8366s kmod-usb-ohci kmod-usb-ohci-pci \
	kmod-usb2 kmod-usb2-pci kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += f5d8235-v1
endef
TARGET_DEVICES += belkin_f5d8235-v1

define Device/buffalo_wli-tx4-ag300n
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WLI-TX4-AG300N
  DEVICE_PACKAGES := kmod-switch-ip17xx
  SUPPORTED_DEVICES += wli-tx4-ag300n
  DEFAULT := n
endef
TARGET_DEVICES += buffalo_wli-tx4-ag300n

define Device/buffalo_wzr-agl300nh
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WZR-AGL300NH
  DEVICE_PACKAGES := kmod-switch-rtl8366s
  SUPPORTED_DEVICES += wzr-agl300nh
  DEFAULT := n
endef
TARGET_DEVICES += buffalo_wzr-agl300nh

define Device/dlink_dap-1522-a1
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3712k
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DAP-1522
  DEVICE_VARIANT := A1
  DEVICE_PACKAGES := kmod-switch-rtl8366s
  KERNEL := $(KERNEL_DTB)
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-offset $$$$(BLOCKSIZE) 96 | \
	append-rootfs | pad-rootfs -x 96 | wrg-header wapnd01_dlink_dap1522 | \
	check-size
  DEFAULT := n
endef
TARGET_DEVICES += dlink_dap-1522-a1

define Device/ralink_v11st-fe
  BLOCKSIZE := 64k
  IMAGE_SIZE := 3776k
  DEVICE_VENDOR := Ralink
  DEVICE_MODEL := V11ST-FE
  SUPPORTED_DEVICES += v11st-fe
  DEFAULT := n
endef
TARGET_DEVICES += ralink_v11st-fe
