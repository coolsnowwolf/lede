include ./common-tp-link.mk

define Device/tplink_archer-a7-v5
  $(Device/tplink-safeloader-uimage)
  ATH_SOC := qca9563
  IMAGE_SIZE := 15104k
  DEVICE_TITLE := TP-LINK Archer A7 v5
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct ath10k-firmware-qca988x-ct
  TPLINK_BOARD_ID := ARCHER-A7-V5
  BOARDNAME := ARCHER-A7-V5
  SUPPORTED_DEVICES += archer-a7-v5
endef
TARGET_DEVICES += tplink_archer-a7-v5

define Device/tplink_archer-c2-v3
  $(Device/tplink-safeloader-uimage)
  ATH_SOC := qca9563
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-LINK Archer C2 v3
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9887-ct
  TPLINK_BOARD_ID := ARCHER-C2-V3
endef
TARGET_DEVICES += tplink_archer-c2-v3

define Device/tplink_archer-c58-v1
  $(Device/tplink-safeloader-uimage)
  ATH_SOC := qca9561
  IMAGE_SIZE := 7936k
  DEVICE_TITLE := TP-LINK Archer C58 v1
  TPLINK_BOARD_ID := ARCHER-C58-V1
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9888-ct
  SUPPORTED_DEVICES += archer-c58-v1
endef
TARGET_DEVICES += tplink_archer-c58-v1

define Device/tplink_archer-c59-v1
  $(Device/tplink-safeloader-uimage)
  ATH_SOC := qca9561
  IMAGE_SIZE := 14528k
  DEVICE_TITLE := TP-LINK Archer C59 v1
  TPLINK_BOARD_ID := ARCHER-C59-V1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct ath10k-firmware-qca9888-ct
  SUPPORTED_DEVICES += archer-c59-v1
endef
TARGET_DEVICES += tplink_archer-c59-v1

define Device/tplink_archer-c6-v2
  $(Device/tplink-safeloader-uimage)
  ATH_SOC := qca9563
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := TP-LINK Archer C6 v2
  TPLINK_BOARD_ID := ARCHER-C6-V2
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += tplink_archer-c6-v2

define Device/tplink_archer-c7-v1
  $(Device/tplink-8mlzma)
  ATH_SOC := qca9558
  DEVICE_TITLE := TP-LINK Archer C7 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct ath10k-firmware-qca988x-ct
  TPLINK_HWID := 0x75000001
endef
TARGET_DEVICES += tplink_archer-c7-v1

define Device/tplink_archer-c7-v2
  $(Device/tplink-16mlzma)
  ATH_SOC := qca9558
  DEVICE_TITLE := TP-LINK Archer C7 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct ath10k-firmware-qca988x-ct
  TPLINK_HWID := 0xc7000002
  IMAGES += factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := append-rootfs | mktplinkfw factory -C US
  IMAGE/factory-eu.bin := append-rootfs | mktplinkfw factory -C EU
endef
TARGET_DEVICES += tplink_archer-c7-v2

define Device/tplink_archer-c7-v4
  $(Device/tplink-safeloader-uimage)
  ATH_SOC := qca9563
  IMAGE_SIZE := 15104k
  DEVICE_TITLE := TP-LINK Archer C7 v4
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct ath10k-firmware-qca988x-ct
  TPLINK_BOARD_ID := ARCHER-C7-V4
  BOARDNAME := ARCHER-C7-V4
  SUPPORTED_DEVICES += archer-c7-v4
endef
TARGET_DEVICES += tplink_archer-c7-v4

define Device/tplink_archer-c7-v5
  $(Device/tplink-safeloader-uimage)
  ATH_SOC := qca9563
  IMAGE_SIZE := 15360k
  DEVICE_TITLE := TP-LINK Archer C7 v5
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct ath10k-firmware-qca988x-ct
  TPLINK_BOARD_ID := ARCHER-C7-V5
  BOARDNAME := ARCHER-C7-V5
  SUPPORTED_DEVICES += archer-c7-v5
endef
TARGET_DEVICES += tplink_archer-c7-v5

define Device/tplink_re450-v2
  $(Device/tplink)
  ATH_SOC := qca9563
  IMAGE_SIZE := 6016k
  DEVICE_TITLE := TP-LINK RE450 v2
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0
  TPLINK_BOARD_ID := RE450-V2
  LOADER_TYPE := elf
  KERNEL := kernel-bin | append-dtb | lzma | tplink-v1-header -O
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | \
    append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef
TARGET_DEVICES += tplink_re450-v2

define Device/tplink_tl-wdr3600
  $(Device/tplink-8mlzma)
  ATH_SOC := ar9344
  DEVICE_TITLE := TP-LINK TL-WDR3600
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x36000001
  SUPPORTED_DEVICES += tl-wdr3600
endef
TARGET_DEVICES += tplink_tl-wdr3600

define Device/tplink_tl-wdr4300
  $(Device/tplink-8mlzma)
  ATH_SOC := ar9344
  DEVICE_TITLE := TP-LINK TL-WDR4300
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x43000001
  SUPPORTED_DEVICES += tl-wdr4300
endef
TARGET_DEVICES += tplink_tl-wdr4300

define Device/tplink_tl-wdr4900-v2
  $(Device/tplink-8mlzma)
  ATH_SOC := qca9558
  DEVICE_TITLE := TP-LINK TL-WDR4900 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x49000002
endef
TARGET_DEVICES += tplink_tl-wdr4900-v2

define Device/tplink_tl-wr1043nd-v1
  $(Device/tplink-8m)
  ATH_SOC := ar9132
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x10430001
  SUPPORTED_DEVICES += tl-wr1043nd
endef
TARGET_DEVICES += tplink_tl-wr1043nd-v1

define Device/tplink_tl-wr810n-v1
  $(Device/tplink-8mlzma)
  ATH_SOC := qca9531
  DEVICE_TITLE := TP-LINK TL-WR810N v1
  TPLINK_HWID := 0x8100001
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += tplink_tl-wr810n-v1

define Device/tplink_tl-wr810n-v2
  $(Device/tplink-8mlzma)
  ATH_SOC := qca9533
  DEVICE_TITLE := TP-LINK TL-WR810N v2
  TPLINK_HWID := 0x8100002
endef
TARGET_DEVICES += tplink_tl-wr810n-v2

define Device/tplink_tl-wr842n-v1
  $(Device/tplink-8m)
  ATH_SOC := ar7241
  DEVICE_TITLE := TP-LINK TL-WR842N/ND v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x8420001
endef
TARGET_DEVICES += tplink_tl-wr842n-v1

define Device/tplink_tl-wr842n-v2
  $(Device/tplink-8mlzma)
  ATH_SOC := ar9341
  DEVICE_TITLE := TP-LINK TL-WR842N/ND v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x8420002
  SUPPORTED_DEVICES += tl-wr842n-v2
endef
TARGET_DEVICES += tplink_tl-wr842n-v2

define Device/tplink_tl-wr1043nd-v2
  $(Device/tplink-8mlzma)
  ATH_SOC := qca9558
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x10430002
  SUPPORTED_DEVICES += tl-wr1043nd-v2
endef
TARGET_DEVICES += tplink_tl-wr1043nd-v2

define Device/tplink_tl-wr1043nd-v3
  $(Device/tplink-8mlzma)
  ATH_SOC := qca9558
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v3
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x10430003
  SUPPORTED_DEVICES += tl-wr1043nd-v3
endef
TARGET_DEVICES += tplink_tl-wr1043nd-v3

define Device/tplink_tl-wr1043nd-v4
  $(Device/tplink)
  ATH_SOC := qca9563
  IMAGE_SIZE := 15552k
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v4
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x10430004
  TPLINK_BOARD_ID := TLWR1043NDV4
  KERNEL := kernel-bin | append-dtb | lzma | tplink-v1-header -O
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | \
    append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
  SUPPORTED_DEVICES += tl-wr1043nd-v4
endef
TARGET_DEVICES += tplink_tl-wr1043nd-v4

define Device/tplink_tl-wr2543-v1
  $(Device/tplink-8mlzma)
  ATH_SOC := ar7242
  DEVICE_TITLE := TP-LINK TL-WR2543N/ND v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x25430001
  IMAGE/sysupgrade.bin := append-rootfs | mktplinkfw sysupgrade -v 3.13.99 | \
    append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-rootfs | mktplinkfw factory -v 3.13.99
  SUPPORTED_DEVICES += tl-wr2543-v1
endef
TARGET_DEVICES += tplink_tl-wr2543-v1
