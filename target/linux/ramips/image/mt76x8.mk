#
# MT76x8 Profiles
#

DEVICE_VARS += TPLINK_BOARD_ID

define Device/duzun-dm06
  DTS := DUZUN-DM06
  DEVICE_TITLE := DuZun DM06
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += duzun-dm06

define Device/gl-mt300n-v2
  DTS := GL-MT300N-V2
  IMAGE_SIZE := 16064k
  DEVICE_TITLE := GL-iNet GL-MT300N-V2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += gl-mt300n-v2

define Device/hc5661a
  DTS := HC5661A
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := HiWiFi HC5661A
endef
TARGET_DEVICES += hc5661a

define Device/LinkIt7688
  DTS := LINKIT7688
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  SUPPORTED_DEVICES := linkits7688 linkits7688d
  DEVICE_TITLE := MediaTek LinkIt Smart 7688
  DEVICE_PACKAGES:= kmod-usb2 kmod-usb-ohci uboot-envtools
endef
TARGET_DEVICES += LinkIt7688

define Device/mac1200r-v2
  DTS := MAC1200RV2
  DEVICE_TITLE := Mercury MAC1200R v2.0
  SUPPORTED_DEVICES := mac1200rv2
endef
TARGET_DEVICES += mac1200r-v2

define Device/miwifi-nano
  DTS := MIWIFI-NANO
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Xiaomi MiWiFi Nano
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += miwifi-nano

define Device/mt7628
  DTS := MT7628
  BLOCKSIZE := 64k
  IMAGE_SIZE := $(ralink_default_fw_size_4M)
  DEVICE_TITLE := MediaTek MT7628 EVB
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef

define Device/omega2
  DTS := OMEGA2
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Onion Omega2
  DEVICE_PACKAGES:= kmod-usb2 kmod-usb-ohci uboot-envtools
endef

define Device/omega2p
  DTS := OMEGA2P
  IMAGE_SIZE := $(ralink_default_fw_size_32M)
  DEVICE_TITLE := Onion Omega2+
  DEVICE_PACKAGES:= kmod-usb2 kmod-usb-ohci uboot-envtools kmod-sdhci-mt7620
endef
TARGET_DEVICES += omega2 omega2p

define Device/pbr-d1
  DTS := PBR-D1
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := PBR-D1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += pbr-d1

define Device/tl-wr840n-v4
  DTS := TL-WR840NV4
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-Link TL-WR840N v4
  TPLINK_BOARD_ID := TL-WR840NV4
  KERNEL := $(KERNEL_DTB)
  IMAGES += tftp-recovery.bin
  IMAGE/factory.bin := tplink-v2-image
  IMAGE/tftp-recovery.bin := pad-extra 128k | $$(IMAGE/factory.bin)
  IMAGE/sysupgrade.bin := tplink-v2-image -s | append-metadata | \
	check-size $$$$(IMAGE_SIZE)
endef

define Device/tl-wr841n-v13
  $(Device/tl-wr840n-v4)
  DTS := TL-WR841NV13
  DEVICE_TITLE := TP-Link TL-WR841N v13
  TPLINK_BOARD_ID := TL-WR841NV13
endef
TARGET_DEVICES += tl-wr840n-v4 tl-wr841n-v13

define Device/vocore2
  DTS := VOCORE2
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := VoCore VoCore2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport \
    kmod-sdhci-mt7620
endef

define Device/vocore2lite
  DTS := VOCORE2LITE
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := VoCore VoCore2-Lite
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport \
    kmod-sdhci-mt7620
endef
TARGET_DEVICES += vocore2 vocore2lite

define Device/wcr-1166ds
  DTS := WCR-1166DS
  BUFFALO_TAG_PLATFORM := MTK
  BUFFALO_TAG_VERSION := 9.99
  BUFFALO_TAG_MINOR := 9.99
  IMAGES += factory.bin
  IMAGE/sysupgrade.bin := trx | pad-rootfs | append-metadata
  IMAGE/factory.bin := \
	trx -M 0x746f435c | pad-rootfs | append-metadata | \
	buffalo-enc WCR-1166DS $$(BUFFALO_TAG_VERSION) -l | \
	buffalo-tag-dhp WCR-1166DS JP JP | buffalo-enc-tag -l | \
	buffalo-dhp-image
  DEVICE_TITLE := Buffalo WCR-1166DS
endef
TARGET_DEVICES += wcr-1166ds

define Device/wl-wn575a3
  DTS := WL-WN575A3
  IMAGE_SIZE := $(ralink_default_fw_size_8M)
  DEVICE_TITLE := Wavlink WL-WN575A3
endef
TARGET_DEVICES += wl-wn575a3

define Device/widora-neo
  DTS := WIDORA-NEO
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := Widora-NEO
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += widora-neo

define Device/wrtnode2p
  DTS := WRTNODE2P
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := WRTnode 2P
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport
endef

define Device/wrtnode2r
  DTS := WRTNODE2R
  IMAGE_SIZE := $(ralink_default_fw_size_16M)
  DEVICE_TITLE := WRTnode 2R
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ohci
endef
TARGET_DEVICES += wrtnode2p wrtnode2r
