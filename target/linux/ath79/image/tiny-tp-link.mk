include ./common-tp-link.mk


define Device/tl-wr703n
  $(Device/tplink-4mlzma)
  ATH_SOC := ar9331
  DEVICE_TITLE := TP-Link TL-WR703N
  DEVICE_PACKAGES := kmod-usb-chipidea2
  TPLINK_HWID := 0x07030101
  SUPPORTED_DEVICES := tplink,tl-wr703n tl-wr703n
endef
TARGET_DEVICES += tl-wr703n

define Device/tl-wr740n-v2
  $(Device/tplink-4m)
  ATH_SOC := ar7240
  DEVICE_TITLE := TP-Link TL-WR740N/ND v2
  TPLINK_HWID := 0x07400001
  SUPPORTED_DEVICES := tplink,tl-wr740n-v2 tl-wr740n-v2
endef
TARGET_DEVICES += tl-wr740n-v2
