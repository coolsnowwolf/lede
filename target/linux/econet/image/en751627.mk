TRX_ENDIAN := be

define Device/zyxel_ex3301-t0
  DEVICE_VENDOR := Zyxel
  DEVICE_MODEL := EX3301-T0
  DEVICE_DTS := en751627_zyxel_ex3301-t0
  IMAGES := tclinux.trx
  IMAGE/tclinux.trx := append-kernel | lzma | tclinux-trx
  DEVICE_PACKAGES := kmod-usb3 kmod-mt7915e kmod-mt7915-firmware
endef
TARGET_DEVICES += zyxel_ex3301-t0
