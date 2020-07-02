include ./common-tp-link.mk

define Device/tplink_tl-mr10u
  $(Device/tplink-4mlzma)
  SOC := ar9331
  DEVICE_MODEL := TL-MR10U
  DEVICE_PACKAGES := kmod-usb-chipidea2
  TPLINK_HWID := 0x00100101
  SUPPORTED_DEVICES += tl-mr10u
endef
TARGET_DEVICES += tplink_tl-mr10u

define Device/tplink_tl-mr3020-v1
  $(Device/tplink-4mlzma)
  IMAGE_SIZE := 3840k
  SOC := ar9331
  DEVICE_MODEL := TL-MR3020
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb-chipidea2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x30200001
  SUPPORTED_DEVICES += tl-mr3020
endef
TARGET_DEVICES += tplink_tl-mr3020-v1

define Device/tplink_tl-mr3040-v2
  $(Device/tplink-4mlzma)
  SOC := ar9331
  DEVICE_MODEL := TL-MR3040
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-usb-chipidea2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x30400002
  SUPPORTED_DEVICES += tl-mr3040-v2
endef
TARGET_DEVICES += tplink_tl-mr3040-v2

define Device/tplink_tl-mr3220-v1
  $(Device/tplink-4m)
  SOC := ar7241
  DEVICE_MODEL := TL-MR3220
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x32200001
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += tl-mr3220
endef
TARGET_DEVICES += tplink_tl-mr3220-v1

define Device/tplink_tl-mr3420-v1
  $(Device/tplink-4m)
  SOC := ar7241
  DEVICE_MODEL := TL-MR3420
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x34200001
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += tl-mr3420
endef
TARGET_DEVICES += tplink_tl-mr3420-v1

define Device/tplink_tl-mr3420-v2
  $(Device/tplink-4mlzma)
  SOC := ar9341
  DEVICE_MODEL := TL-MR3420
  DEVICE_VARIANT := v2
  TPLINK_HWID := 0x34200002
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += tl-mr3420-v2
endef
TARGET_DEVICES += tplink_tl-mr3420-v2

define Device/tplink_tl-mr3420-v3
  $(Device/tplink-4mlzma)
  SOC := qca9531
  DEVICE_MODEL := TL-MR3420
  DEVICE_VARIANT := v3
  TPLINK_HWID := 0x34200003
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += tplink_tl-mr3420-v3

define Device/tplink_tl-wa701nd-v1
  $(Device/tplink-4m)
  SOC := ar7240
  DEVICE_MODEL := TL-WA701ND
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x07010001
  SUPPORTED_DEVICES += tl-wa901nd
endef
TARGET_DEVICES += tplink_tl-wa701nd-v1

define Device/tplink_tl-wa730re-v1
  $(Device/tplink-4m)
  SOC := ar7240
  DEVICE_MODEL := TL-WA730RE
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x07300001
  SUPPORTED_DEVICES += tl-wa901nd
endef
TARGET_DEVICES += tplink_tl-wa730re-v1

define Device/tplink_tl-wa801nd-v1
  $(Device/tplink-4m)
  SOC := ar7240
  DEVICE_MODEL := TL-WA801ND
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x08010001
  SUPPORTED_DEVICES += tl-wa901nd
endef
TARGET_DEVICES += tplink_tl-wa801nd-v1

define Device/tplink_tl-wa801nd-v3
  $(Device/tplink-4mlzma)
  SOC := qca9533
  DEVICE_MODEL := TL-WA801ND
  DEVICE_VARIANT := v3
  TPLINK_HWID := 0x08010003
  SUPPORTED_DEVICES += tl-wa801nd-v3
endef
TARGET_DEVICES += tplink_tl-wa801nd-v3

define Device/tplink_tl-wa801nd-v4
  $(Device/tplink-4mlzma)
  SOC := qca9533
  DEVICE_MODEL := TL-WA801ND
  DEVICE_VARIANT := v4
  TPLINK_HWID := 0x08010004
  SUPPORTED_DEVICES += tl-wa801nd-v3
endef
TARGET_DEVICES += tplink_tl-wa801nd-v4

define Device/tplink_tl-wa830re-v1
  $(Device/tplink-4m)
  SOC := ar7240
  DEVICE_MODEL := TL-WA830RE
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x08300010
  SUPPORTED_DEVICES += tl-wa901nd
endef
TARGET_DEVICES += tplink_tl-wa830re-v1

define Device/tplink_tl-wa850re-v1
  $(Device/tplink-4mlzma)
  SOC := ar9341
  DEVICE_MODEL := TL-WA850RE
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x08500001
  DEVICE_PACKAGES := rssileds
  SUPPORTED_DEVICES += tl-wa850re
endef
TARGET_DEVICES += tplink_tl-wa850re-v1

define Device/tplink_tl-wa850re-v2
  $(Device/tplink-safeloader)
  SOC := qca9533
  IMAGE_SIZE := 3648k
  DEVICE_MODEL := TL-WA850RE
  DEVICE_VARIANT := v2
  TPLINK_BOARD_ID := TLWA850REV2
  TPLINK_HWID := 0x08500002
  DEVICE_PACKAGES := rssileds
  SUPPORTED_DEVICES += tl-wa850re-v2
  DEFAULT := n
endef
TARGET_DEVICES += tplink_tl-wa850re-v2

define Device/tplink_tl-wa860re-v1
  $(Device/tplink-4mlzma)
  SOC := ar9341
  DEVICE_MODEL := TL-WA860RE
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x08600001
  SUPPORTED_DEVICES += tl-wa860re
endef
TARGET_DEVICES += tplink_tl-wa860re-v1

define Device/tplink_tl-wa901nd-v1
  $(Device/tplink-4m)
  SOC := ar7240
  DEVICE_MODEL := TL-WA901ND
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x09010001
  SUPPORTED_DEVICES += tl-wa901nd
endef
TARGET_DEVICES += tplink_tl-wa901nd-v1

define Device/tplink_tl-wa901nd-v2
  $(Device/tplink-4m)
  SOC := ar9132
  DEVICE_MODEL := TL-WA901ND
  DEVICE_VARIANT := v2
  TPLINK_HWID := 0x09010002
  SUPPORTED_DEVICES += tl-wa901nd-v2
endef
TARGET_DEVICES += tplink_tl-wa901nd-v2

define Device/tplink_tl-wa901nd-v4
  $(Device/tplink-4mlzma)
  SOC := tp9343
  DEVICE_MODEL := TL-WA901ND
  DEVICE_VARIANT := v4
  TPLINK_HWID := 0x09010004
  SUPPORTED_DEVICES += tl-wa901nd-v4
  IMAGE/factory.bin := tplink-v1-image factory -C EU
endef
TARGET_DEVICES += tplink_tl-wa901nd-v4

define Device/tplink_tl-wa901nd-v5
  $(Device/tplink-4mlzma)
  SOC := tp9343
  DEVICE_MODEL := TL-WA901ND
  DEVICE_VARIANT := v5
  TPLINK_HWID := 0x09010005
  SUPPORTED_DEVICES += tl-wa901nd-v5
  IMAGE/factory.bin := tplink-v1-image factory -C EU
endef
TARGET_DEVICES += tplink_tl-wa901nd-v5

define Device/tplink_tl-wr703n
  $(Device/tplink-4mlzma)
  SOC := ar9331
  DEVICE_MODEL := TL-WR703N
  DEVICE_PACKAGES := kmod-usb-chipidea2
  TPLINK_HWID := 0x07030101
  SUPPORTED_DEVICES += tl-wr703n
endef
TARGET_DEVICES += tplink_tl-wr703n

define Device/tplink_tl-wr740n-v1
  $(Device/tplink-4m)
  SOC := ar7240
  DEVICE_MODEL := TL-WR740N
  DEVICE_VARIANT := v1/v2
  TPLINK_HWID := 0x07400001
  SUPPORTED_DEVICES += tl-wr741nd
endef
TARGET_DEVICES += tplink_tl-wr740n-v1

define Device/tplink_tl-wr740n-v3
  $(Device/tplink-4m)
  SOC := ar7240
  DEVICE_MODEL := TL-WR740N
  DEVICE_VARIANT := v3
  TPLINK_HWID := 0x07400003
  SUPPORTED_DEVICES += tl-wr741nd
endef
TARGET_DEVICES += tplink_tl-wr740n-v3

define Device/tplink_tl-wr740n-v4
  $(Device/tplink-4mlzma)
  SOC := ar9331
  DEVICE_MODEL := TL-WR740N
  DEVICE_VARIANT := v4
  TPLINK_HWID := 0x07400004
  SUPPORTED_DEVICES += tl-wr741nd-v4
endef
TARGET_DEVICES += tplink_tl-wr740n-v4

define Device/tplink_tl-wr740n-v5
  $(Device/tplink-4mlzma)
  SOC := ar9331
  DEVICE_MODEL := TL-WR740N
  DEVICE_VARIANT := v5
  TPLINK_HWID := 0x07400005
  SUPPORTED_DEVICES += tl-wr741nd-v4
endef
TARGET_DEVICES += tplink_tl-wr740n-v5

define Device/tplink_tl-wr741-v1
  $(Device/tplink-4m)
  SOC := ar7240
  DEVICE_MODEL := TL-WR741N/ND
  DEVICE_VARIANT := v1/v2
  TPLINK_HWID := 0x07410001
  SUPPORTED_DEVICES += tl-wr741nd
endef
TARGET_DEVICES += tplink_tl-wr741-v1

define Device/tplink_tl-wr741nd-v4
  $(Device/tplink-4mlzma)
  SOC := ar9331
  DEVICE_MODEL := TL-WR741N/ND
  DEVICE_VARIANT := v4
  TPLINK_HWID := 0x07410004
  SUPPORTED_DEVICES += tl-wr741nd-v4
endef
TARGET_DEVICES += tplink_tl-wr741nd-v4

define Device/tplink_tl-wr743nd-v1
  $(Device/tplink-4m)
  SOC := ar7240
  DEVICE_MODEL := TL-WR743ND
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x07430001
  SUPPORTED_DEVICES += tl-wr741nd
endef
TARGET_DEVICES += tplink_tl-wr743nd-v1

define Device/tplink_tl-wr802n-v1
  $(Device/tplink-4mlzma)
  SOC := qca9533
  DEVICE_MODEL := TL-WR802N
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x08020001
  SUPPORTED_DEVICES += tl-wr802n-v1
endef
TARGET_DEVICES += tplink_tl-wr802n-v1

define Device/tplink_tl-wr802n-v2
  $(Device/tplink-4mlzma)
  SOC := qca9533
  DEVICE_MODEL := TL-WR802N
  DEVICE_VARIANT := v2
  TPLINK_HWID := 0x08020002
  TPLINK_HWREV := 2
  SUPPORTED_DEVICES += tl-wr802n-v2
  IMAGES += factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := tplink-v1-image factory -C US
  IMAGE/factory-eu.bin := tplink-v1-image factory -C EU
endef
TARGET_DEVICES += tplink_tl-wr802n-v2

define Device/tplink_tl-wr841-v5
  $(Device/tplink-4m)
  SOC := ar7240
  DEVICE_MODEL := TL-WR841N/ND
  DEVICE_VARIANT := v5/v6
  TPLINK_HWID := 0x08410005
  SUPPORTED_DEVICES += tl-wr741nd
endef
TARGET_DEVICES += tplink_tl-wr841-v5

define Device/tplink_tl-wr841-v7
  $(Device/tplink-4m)
  SOC := ar7241
  DEVICE_MODEL := TL-WR841N/ND
  DEVICE_VARIANT := v7
  TPLINK_HWID := 0x08410007
  SUPPORTED_DEVICES += tl-wr841n-v7
endef
TARGET_DEVICES += tplink_tl-wr841-v7

define Device/tplink_tl-wr841-v8
  $(Device/tplink-4mlzma)
  SOC := ar9341
  DEVICE_MODEL := TL-WR841N/ND
  DEVICE_VARIANT := v8
  TPLINK_HWID := 0x08410008
  SUPPORTED_DEVICES += tl-wr841n-v8
endef
TARGET_DEVICES += tplink_tl-wr841-v8

define Device/tplink_tl-wr841-v9
  $(Device/tplink-4mlzma)
  SOC := qca9533
  DEVICE_MODEL := TL-WR841N/ND
  DEVICE_VARIANT := v9
  TPLINK_HWID := 0x08410009
  SUPPORTED_DEVICES += tl-wr841n-v9
endef
TARGET_DEVICES += tplink_tl-wr841-v9

define Device/tplink_tl-wr841-v10
  $(Device/tplink-4mlzma)
  SOC := qca9533
  DEVICE_MODEL := TL-WR841N/ND
  DEVICE_VARIANT := v10
  TPLINK_HWID := 0x08410010
  SUPPORTED_DEVICES += tl-wr841n-v9
endef
TARGET_DEVICES += tplink_tl-wr841-v10

define Device/tplink_tl-wr841-v11
  $(Device/tplink-4mlzma)
  SOC := qca9533
  DEVICE_MODEL := TL-WR841N/ND
  DEVICE_VARIANT := v11
  TPLINK_HWID := 0x08410011
  SUPPORTED_DEVICES += tl-wr841n-v11
  IMAGES += factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := tplink-v1-image factory -C US
  IMAGE/factory-eu.bin := tplink-v1-image factory -C EU
endef
TARGET_DEVICES += tplink_tl-wr841-v11

define Device/tplink_tl-wr841-v12
  $(Device/tplink-4mlzma)
  SOC := qca9533
  DEVICE_MODEL := TL-WR841N/ND
  DEVICE_VARIANT := v12
  TPLINK_HWID := 0x08410012
  SUPPORTED_DEVICES += tl-wr841n-v11
  IMAGES += factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := tplink-v1-image factory -C US
  IMAGE/factory-eu.bin := tplink-v1-image factory -C EU
endef
TARGET_DEVICES += tplink_tl-wr841-v12

define Device/tplink_tl-wr940n-v3
  $(Device/tplink-4mlzma)
  SOC := tp9343
  DEVICE_MODEL := TL-WR940N
  DEVICE_VARIANT := v3
  TPLINK_HWID := 0x09410006
  SUPPORTED_DEVICES += tl-wr941nd-v6
endef
TARGET_DEVICES += tplink_tl-wr940n-v3

define Device/tplink_tl-wr940n-v4
  $(Device/tplink-4mlzma)
  SOC := tp9343
  DEVICE_MODEL := TL-WR940N
  DEVICE_VARIANT := v4
  TPLINK_HWID := 0x09400004
  SUPPORTED_DEVICES += tl-wr940n-v4
  IMAGES += factory-us.bin factory-eu.bin factory-br.bin
  IMAGE/factory-us.bin := tplink-v1-image factory -C US
  IMAGE/factory-eu.bin := tplink-v1-image factory -C EU
  IMAGE/factory-br.bin := tplink-v1-image factory -C BR
endef
TARGET_DEVICES += tplink_tl-wr940n-v4

define Device/tplink_tl-wr940n-v6
  $(Device/tplink-4mlzma)
  SOC := tp9343
  DEVICE_MODEL := TL-WR940N
  DEVICE_VARIANT := v6
  TPLINK_HWID := 0x09400006
  SUPPORTED_DEVICES += tl-wr940n-v6
  IMAGES += factory-us.bin factory-eu.bin factory-br.bin
  IMAGE/factory-us.bin := tplink-v1-image factory -C US
  IMAGE/factory-eu.bin := tplink-v1-image factory -C EU
  IMAGE/factory-br.bin := tplink-v1-image factory -C BR
endef
TARGET_DEVICES += tplink_tl-wr940n-v6

define Device/tplink_tl-wr941-v2
  $(Device/tplink-4m)
  SOC := ar9132
  DEVICE_MODEL := TL-WR941ND
  DEVICE_VARIANT := v2/v3
  DEVICE_ALT0_VENDOR := TP-Link
  DEVICE_ALT0_MODEL := TL-WR941N
  DEVICE_ALT0_VARIANT := v2/v3
  TPLINK_HWID := 0x09410002
  TPLINK_HWREV := 2
  SUPPORTED_DEVICES += tl-wr941nd
endef
TARGET_DEVICES += tplink_tl-wr941-v2

define Device/tplink_tl-wr941-v4
  $(Device/tplink-4m)
  SOC := ar7240
  DEVICE_MODEL := TL-WR941ND
  DEVICE_VARIANT := v4
  DEVICE_ALT0_VENDOR := TP-Link
  DEVICE_ALT0_MODEL := TL-WR941N
  DEVICE_ALT0_VARIANT := v4
  TPLINK_HWID := 0x09410004
  SUPPORTED_DEVICES += tl-wr741nd
endef
TARGET_DEVICES += tplink_tl-wr941-v4

define Device/tplink_tl-wr941nd-v6
  $(Device/tplink-4mlzma)
  SOC := tp9343
  DEVICE_MODEL := TL-WR941ND
  DEVICE_VARIANT := v6
  TPLINK_HWID := 0x09410006
  SUPPORTED_DEVICES += tl-wr941nd-v6
endef
TARGET_DEVICES += tplink_tl-wr941nd-v6

define Device/tplink_tl-wr941n-v7-cn
  $(Device/tplink-4mlzma)
  SOC := qca9558
  DEVICE_MODEL := TL-WR941N
  DEVICE_VARIANT := v7 (CN)
  TPLINK_HWID := 0x09410007
endef
TARGET_DEVICES += tplink_tl-wr941n-v7-cn
