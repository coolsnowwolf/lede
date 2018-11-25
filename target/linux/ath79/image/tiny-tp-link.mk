include ./common-tp-link.mk


define Device/tplink_tl-mr10u
  $(Device/tplink-4mlzma)
  ATH_SOC := ar9331
  DEVICE_TITLE := TP-Link TL-MR10U
  DEVICE_PACKAGES := kmod-usb-chipidea2
  TPLINK_HWID := 0x00100101
  SUPPORTED_DEVICES += tl-mr10u
endef
TARGET_DEVICES += tplink_tl-mr10u

define Device/tplink_tl-mr3020-v1
  $(Device/tplink-4mlzma)
  ATH_SOC := ar9331
  DEVICE_TITLE := TP-LINK TL-MR3020 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-chipidea2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x30200001
  SUPPORTED_DEVICES += tl-mr3020
endef
TARGET_DEVICES += tplink_tl-mr3020-v1

define Device/tplink_tl-mr3040-v2
  $(Device/tplink-4mlzma)
  ATH_SOC := ar9331
  DEVICE_TITLE := TP-LINK TL-MR3040 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb-chipidea2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x30400002
  SUPPORTED_DEVICES += tl-mr3040-v2
endef
TARGET_DEVICES += tplink_tl-mr3040-v2

define Device/tplink_tl-mr3220-v1
  $(Device/tplink-4m)
  ATH_SOC := ar7241
  DEVICE_TITLE := TP-Link TL-MR3220 v1
  TPLINK_HWID := 0x32200001
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += tl-mr3220-v1
endef
TARGET_DEVICES += tplink_tl-mr3220-v1

define Device/tplink_tl-mr3420-v1
  $(Device/tplink-4m)
  ATH_SOC := ar7241
  DEVICE_TITLE := TP-Link TL-MR3420 v1
  TPLINK_HWID := 0x34200001
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += tl-mr3420-v1
endef
TARGET_DEVICES += tplink_tl-mr3420-v1

define Device/tplink_tl-wa901nd-v2
  $(Device/tplink-4m)
  ATH_SOC := ar9132
  DEVICE_TITLE := TP-Link TL-WA901ND v2
  TPLINK_HWID := 0x09010002
endef
TARGET_DEVICES += tplink_tl-wa901nd-v2

define Device/tplink_tl-wr703n
  $(Device/tplink-4mlzma)
  ATH_SOC := ar9331
  DEVICE_TITLE := TP-Link TL-WR703N
  DEVICE_PACKAGES := kmod-usb-chipidea2
  TPLINK_HWID := 0x07030101
  SUPPORTED_DEVICES += tl-wr703n
endef
TARGET_DEVICES += tplink_tl-wr703n

define Device/tplink_tl-wr740n-v1
  $(Device/tplink-4m)
  ATH_SOC := ar7240
  DEVICE_TITLE := TP-Link TL-WR740N v1/v2
  TPLINK_HWID := 0x07400001
endef
TARGET_DEVICES += tplink_tl-wr740n-v1

define Device/tplink_tl-wr740n-v3
  $(Device/tplink-4m)
  ATH_SOC := ar7240
  DEVICE_TITLE := TP-Link TL-WR740N v3
  TPLINK_HWID := 0x07400003
endef
TARGET_DEVICES += tplink_tl-wr740n-v3

define Device/tplink_tl-wr740nd-v4
  $(Device/tplink-4mlzma)
  ATH_SOC := ar9331
  DEVICE_TITLE := TP-LINK TL-WR740N/ND v4
  TPLINK_HWID := 0x07400004
  SUPPORTED_DEVICES += tl-wr740n-v4
endef
TARGET_DEVICES += tplink_tl-wr740nd-v4

define Device/tplink_tl-wr741-v1
  $(Device/tplink-4m)
  ATH_SOC := ar7240
  DEVICE_TITLE := TP-Link TL-WR741N/ND v1/v2
  TPLINK_HWID := 0x07410001
endef
TARGET_DEVICES += tplink_tl-wr741-v1

define Device/tplink_tl-wr741nd-v4
  $(Device/tplink-4mlzma)
  ATH_SOC := ar9331
  DEVICE_TITLE := TP-LINK TL-WR741N/ND v4
  TPLINK_HWID := 0x07410004
  SUPPORTED_DEVICES += tl-wr741n-v4
endef
TARGET_DEVICES += tplink_tl-wr741nd-v4

define Device/tplink_tl-wr743nd-v1
  $(Device/tplink-4m)
  ATH_SOC := ar7240
  DEVICE_TITLE := TP-Link TL-WR743ND v1
  TPLINK_HWID := 0x07430001
endef
TARGET_DEVICES += tplink_tl-wr743nd-v1

define Device/tplink_tl-wr841-v5
  $(Device/tplink-4m)
  ATH_SOC := ar7240
  DEVICE_TITLE := TP-Link TL-WR841N/ND v5/v6
  TPLINK_HWID := 0x08410005
endef
TARGET_DEVICES += tplink_tl-wr841-v5

define Device/tplink_tl-wr841-v7
  $(Device/tplink-4m)
  ATH_SOC := ar7241
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v7
  TPLINK_HWID := 0x08410007
  SUPPORTED_DEVICES += tl-wr841-v7
endef
TARGET_DEVICES += tplink_tl-wr841-v7

define Device/tplink_tl-wr841-v9
  $(Device/tplink-4mlzma)
  ATH_SOC := qca9533
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v9
  TPLINK_HWID := 0x08410009
endef
TARGET_DEVICES += tplink_tl-wr841-v9

define Device/tplink_tl-wr841-v11
  $(Device/tplink-4mlzma)
  ATH_SOC := qca9533
  DEVICE_TITLE := TP-LINK TL-WR841N/ND v11
  TPLINK_HWID := 0x08410011
  IMAGES += factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := append-rootfs | mktplinkfw factory -C US
  IMAGE/factory-eu.bin := append-rootfs | mktplinkfw factory -C EU
endef
TARGET_DEVICES += tplink_tl-wr841-v11

define Device/tplink_tl-wr941-v2
  $(Device/tplink-4m)
  ATH_SOC := ar9132
  DEVICE_TITLE := TP-Link TL-WR941N/ND v2/v3
  TPLINK_HWID := 0x09410002
  TPLINK_HWREV := 2
endef
TARGET_DEVICES += tplink_tl-wr941-v2

define Device/tplink_tl-wr941-v4
  $(Device/tplink-4m)
  ATH_SOC := ar7240
  DEVICE_TITLE := TP-Link TL-WR941N/ND v4
  TPLINK_HWID := 0x09410004
endef
TARGET_DEVICES += tplink_tl-wr941-v4
