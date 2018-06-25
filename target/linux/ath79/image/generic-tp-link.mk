include ./common-tp-link.mk


define Device/tl-wr1043nd-v1
  $(Device/tplink-8m)
  ATH_SOC := ar9132
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x10430001
  SUPPORTED_DEVICES := tplink,tl-wr1043nd-v1 tl-wr1043nd
endef
TARGET_DEVICES += tl-wr1043nd-v1

define Device/tl-wdr3600
  $(Device/tplink-8mlzma)
  ATH_SOC := ar9344
  DEVICE_TITLE := TP-LINK TL-WDR3600
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x36000001
  SUPPORTED_DEVICES := tplink,tl-wdr3600 tl-wdr3600
endef
TARGET_DEVICES += tl-wdr3600

define Device/tl-wdr4300
  $(Device/tl-wdr3600)
  DEVICE_TITLE := TP-LINK TL-WDR4300
  TPLINK_HWID := 0x43000001
  SUPPORTED_DEVICES := tplink,tl-wdr4300 tl-wdr4300
endef
TARGET_DEVICES += tl-wdr4300
