include ./common-tp-link.mk

define Device/tplink_archer-a7-v5
  $(Device/tplink-safeloader-uimage)
  SOC := qca9563
  IMAGE_SIZE := 15104k
  DEVICE_MODEL := Archer A7
  DEVICE_VARIANT := v5
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct \
	ath10k-firmware-qca988x-ct
  TPLINK_BOARD_ID := ARCHER-A7-V5
  BOARDNAME := ARCHER-A7-V5
endef
TARGET_DEVICES += tplink_archer-a7-v5

define Device/tplink_archer-c2-v3
  $(Device/tplink-safeloader-uimage)
  SOC := qca9563
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := Archer C2
  DEVICE_VARIANT := v3
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca9887-ct
  TPLINK_BOARD_ID := ARCHER-C2-V3
endef
TARGET_DEVICES += tplink_archer-c2-v3

define Device/tplink_archer-c25-v1
  $(Device/tplink-safeloader-uimage)
  SOC := qca9561
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := Archer C25
  DEVICE_VARIANT := v1
  TPLINK_BOARD_ID := ARCHER-C25-V1
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca9887-ct
  SUPPORTED_DEVICES += archer-c25-v1
endef
TARGET_DEVICES += tplink_archer-c25-v1

define Device/tplink_archer-c5-v1
  $(Device/tplink-16mlzma)
  SOC := qca9558
  DEVICE_MODEL := Archer C5
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct \
	ath10k-firmware-qca988x-ct
  TPLINK_HWID := 0xc5000001
  SUPPORTED_DEVICES += archer-c5
endef
TARGET_DEVICES += tplink_archer-c5-v1

define Device/tplink_archer-c58-v1
  $(Device/tplink-safeloader-uimage)
  SOC := qca9561
  IMAGE_SIZE := 7936k
  DEVICE_MODEL := Archer C58
  DEVICE_VARIANT := v1
  TPLINK_BOARD_ID := ARCHER-C58-V1
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca9888-ct
  SUPPORTED_DEVICES += archer-c58-v1
endef
TARGET_DEVICES += tplink_archer-c58-v1

define Device/tplink_archer-c59-v1
  $(Device/tplink-safeloader-uimage)
  SOC := qca9561
  IMAGE_SIZE := 14528k
  DEVICE_MODEL := Archer C59
  DEVICE_VARIANT := v1
  TPLINK_BOARD_ID := ARCHER-C59-V1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct \
	ath10k-firmware-qca9888-ct
  SUPPORTED_DEVICES += archer-c59-v1
endef
TARGET_DEVICES += tplink_archer-c59-v1

define Device/tplink_archer-c59-v2
  $(Device/tplink-safeloader-uimage)
  SOC := qca9561
  IMAGE_SIZE := 14400k
  DEVICE_MODEL := Archer C59
  DEVICE_VARIANT := v2
  TPLINK_BOARD_ID := ARCHER-C59-V2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct \
	ath10k-firmware-qca9888-ct
  SUPPORTED_DEVICES += archer-c59-v2
endef
TARGET_DEVICES += tplink_archer-c59-v2

define Device/tplink_archer-c6-v2
  $(Device/tplink-safeloader-uimage)
  SOC := qca9563
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := Archer C6
  DEVICE_VARIANT := v2 (EU/RU/JP)
  TPLINK_BOARD_ID := ARCHER-C6-V2
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += tplink_archer-c6-v2

define Device/tplink_archer-c6-v2-us
  $(Device/tplink-safeloader-uimage)
  SOC := qca9563
  IMAGE_SIZE := 15872k
  DEVICE_MODEL := Archer C6
  DEVICE_VARIANT := v2 (US)
  DEVICE_ALT0_VENDOR := TP-Link
  DEVICE_ALT0_MODEL := Archer A6
  DEVICE_ALT0_VARIANT := v2 (US/TW)
  TPLINK_BOARD_ID := ARCHER-C6-V2-US
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += tplink_archer-c6-v2-us

define Device/tplink_archer-c60-v1
  $(Device/tplink-safeloader-uimage)
  SOC := qca9561
  IMAGE_SIZE := 7936k
  DEVICE_MODEL := Archer C60
  DEVICE_VARIANT := v1
  TPLINK_BOARD_ID := ARCHER-C60-V1
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca9888-ct
  SUPPORTED_DEVICES += archer-c60-v1
endef
TARGET_DEVICES += tplink_archer-c60-v1

define Device/tplink_archer-c60-v2
  $(Device/tplink-safeloader-uimage)
  SOC := qca9561
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := Archer C60
  DEVICE_VARIANT := v2
  TPLINK_BOARD_ID := ARCHER-C60-V2
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca9888-ct
  SUPPORTED_DEVICES += archer-c60-v2
endef
TARGET_DEVICES += tplink_archer-c60-v2

define Device/tplink_archer-c60-v3
  $(Device/tplink-safeloader-uimage)
  SOC := qca9561
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := Archer C60
  DEVICE_VARIANT := v3
  TPLINK_BOARD_ID := ARCHER-C60-V3
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca9888-ct
endef
TARGET_DEVICES += tplink_archer-c60-v3

define Device/tplink_archer-c7-v1
  $(Device/tplink-8mlzma)
  SOC := qca9558
  DEVICE_MODEL := Archer C7
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x75000001
  SUPPORTED_DEVICES += archer-c7
endef
TARGET_DEVICES += tplink_archer-c7-v1

define Device/tplink_archer-c7-v2
  $(Device/tplink-16mlzma)
  SOC := qca9558
  DEVICE_MODEL := Archer C7
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct \
	ath10k-firmware-qca988x-ct
  TPLINK_HWID := 0xc7000002
  SUPPORTED_DEVICES += archer-c7
  IMAGES += factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := tplink-v1-image factory -C US
  IMAGE/factory-eu.bin := tplink-v1-image factory -C EU
endef
TARGET_DEVICES += tplink_archer-c7-v2

define Device/tplink_archer-c7-v4
  $(Device/tplink-safeloader-uimage)
  SOC := qca9563
  IMAGE_SIZE := 15104k
  DEVICE_MODEL := Archer C7
  DEVICE_VARIANT := v4
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct \
	ath10k-firmware-qca988x-ct
  TPLINK_BOARD_ID := ARCHER-C7-V4
  BOARDNAME := ARCHER-C7-V4
  SUPPORTED_DEVICES += archer-c7-v4
endef
TARGET_DEVICES += tplink_archer-c7-v4

define Device/tplink_archer-c7-v5
  $(Device/tplink-safeloader-uimage)
  SOC := qca9563
  IMAGE_SIZE := 15360k
  DEVICE_MODEL := Archer C7
  DEVICE_VARIANT := v5
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k-ct \
	ath10k-firmware-qca988x-ct
  TPLINK_BOARD_ID := ARCHER-C7-V5
  BOARDNAME := ARCHER-C7-V5
  SUPPORTED_DEVICES += archer-c7-v5
endef
TARGET_DEVICES += tplink_archer-c7-v5

define Device/tplink_archer-d50-v1
  $(Device/tplink-v2)
  SOC := qca9531
  DEVICE_MODEL := Archer D50
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct
  BOARDNAME := ARCHER-D50-V1
  IMAGE_SIZE := 7808k
  TPLINK_HWID := 0xC1200001
  TPLINK_HWREV := 0x00000046
  TPLINK_FLASHLAYOUT := 8Mqca
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | \
	tplink-v2-header -s -V "ver. 1.0"
endef
TARGET_DEVICES += tplink_archer-d50-v1

define Device/tplink_archer-d7-v1
  $(Device/tplink-v2)
  SOC := qca9558
  DEVICE_MODEL := Archer D7
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 15936k
  TPLINK_HWID := 0x89300001
  TPLINK_HWREV := 0x0000002D
  TPLINK_FLASHLAYOUT := 16Mqca
  TPLINK_HWREVADD := 0x00000002
endef
TARGET_DEVICES += tplink_archer-d7-v1

define Device/tplink_archer-d7b-v1
  $(Device/tplink-v2)
  SOC := qca9558
  DEVICE_MODEL := Archer D7b
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-ath10k-ct ath10k-firmware-qca988x-ct
  IMAGE_SIZE := 16000k
  TPLINK_HWID := 0x89300001
  TPLINK_HWREV := 0x0000003D
  TPLINK_FLASHLAYOUT := 16Mqca
endef
TARGET_DEVICES += tplink_archer-d7b-v1

define Device/tplink_cpe210-v1
  $(Device/tplink-safeloader-okli)
  SOC := ar9344
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := CPE210
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := rssileds
  TPLINK_BOARD_ID := CPE210
  SUPPORTED_DEVICES += cpe210
endef
TARGET_DEVICES += tplink_cpe210-v1

define Device/tplink_cpe210-v2
  $(Device/tplink-safeloader)
  SOC := qca9533
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := CPE210
  DEVICE_VARIANT := v2
  TPLINK_BOARD_ID := CPE210V2
  DEVICE_PACKAGES := rssileds
  LOADER_TYPE := elf
  SUPPORTED_DEVICES += cpe210-v2
endef
TARGET_DEVICES += tplink_cpe210-v2

define Device/tplink_cpe210-v3
  $(Device/tplink-safeloader)
  SOC := qca9533
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := CPE210
  DEVICE_VARIANT := v3
  DEVICE_PACKAGES := rssileds
  TPLINK_BOARD_ID := CPE210V3
  LOADER_TYPE := elf
  SUPPORTED_DEVICES += cpe210-v3
endef
TARGET_DEVICES += tplink_cpe210-v3

define Device/tplink_cpe220-v2
  $(Device/tplink-safeloader-okli)
  SOC := ar9344
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := CPE220
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := rssileds
  TPLINK_BOARD_ID := CPE220V2
endef
TARGET_DEVICES += tplink_cpe220-v2

define Device/tplink_cpe220-v3
  $(Device/tplink-safeloader)
  SOC := qca9533
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := CPE220
  DEVICE_VARIANT := v3
  DEVICE_PACKAGES := rssileds
  TPLINK_BOARD_ID := CPE220V3
  LOADER_TYPE := elf
endef
TARGET_DEVICES += tplink_cpe220-v3

define Device/tplink_cpe510-v1
  $(Device/tplink-safeloader-okli)
  SOC := ar9344
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := CPE510
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := rssileds
  TPLINK_BOARD_ID := CPE510
  SUPPORTED_DEVICES += cpe510
endef
TARGET_DEVICES += tplink_cpe510-v1

define Device/tplink_cpe510-v2
  $(Device/tplink-safeloader-okli)
  SOC := ar9344
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := CPE510
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := rssileds
  TPLINK_BOARD_ID := CPE510V2
  SUPPORTED_DEVICES += cpe510-v2
endef
TARGET_DEVICES += tplink_cpe510-v2

define Device/tplink_cpe510-v3
  $(Device/tplink-safeloader-okli)
  SOC := ar9344
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := CPE510
  DEVICE_VARIANT := v3
  DEVICE_PACKAGES := rssileds
  TPLINK_BOARD_ID := CPE510V3
endef
TARGET_DEVICES += tplink_cpe510-v3

define Device/tplink_cpe610-v1
  $(Device/tplink-safeloader-okli)
  SOC := ar9344
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := CPE610
  DEVICE_VARIANT := v1
  TPLINK_BOARD_ID := CPE610V1
endef
TARGET_DEVICES += tplink_cpe610-v1

define Device/tplink_cpe610-v2
  $(Device/tplink-safeloader-okli)
  SOC := ar9344
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := CPE610
  DEVICE_VARIANT := v2
  TPLINK_BOARD_ID := CPE610V2
endef
TARGET_DEVICES += tplink_cpe610-v2

define Device/tplink_re350k-v1
  $(Device/tplink-safeloader)
  SOC := qca9558
  IMAGE_SIZE := 13760k
  DEVICE_MODEL := RE350K
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  TPLINK_BOARD_ID := RE350K-V1
endef
TARGET_DEVICES += tplink_re350k-v1

define Device/tplink_rex5x-v1
  $(Device/tplink-safeloader)
  SOC := qca9558
  IMAGE_SIZE := 6016k
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct
endef

define Device/tplink_re355-v1
  $(Device/tplink_rex5x-v1)
  DEVICE_MODEL := RE355
  DEVICE_VARIANT := v1
  TPLINK_BOARD_ID := RE355
  SUPPORTED_DEVICES += re355
endef
TARGET_DEVICES += tplink_re355-v1

define Device/tplink_re450-v1
  $(Device/tplink_rex5x-v1)
  DEVICE_MODEL := RE450
  DEVICE_VARIANT := v1
  TPLINK_BOARD_ID := RE450
  SUPPORTED_DEVICES += re450
endef
TARGET_DEVICES += tplink_re450-v1

define Device/tplink_re450-v2
  $(Device/tplink-safeloader)
  SOC := qca9563
  IMAGE_SIZE := 6016k
  DEVICE_MODEL := RE450
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct
  TPLINK_BOARD_ID := RE450-V2
  LOADER_TYPE := elf
endef
TARGET_DEVICES += tplink_re450-v2

define Device/tplink_re450-v3
  $(Device/tplink-safeloader)
  SOC := qca9563
  IMAGE_SIZE := 7808k
  DEVICE_MODEL := RE450
  DEVICE_VARIANT := v3
  DEVICE_PACKAGES := kmod-ath10k-ct-smallbuffers ath10k-firmware-qca988x-ct
  TPLINK_BOARD_ID := RE450-V3
  LOADER_TYPE := elf
endef
TARGET_DEVICES += tplink_re450-v3

define Device/tplink_tl-mr6400-v1
  $(Device/tplink-8mlzma)
  SOC := qca9531
  DEVICE_MODEL := TL-MR6400
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x64000001
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-net-rndis kmod-usb-serial \
	kmod-usb-serial-option adb-enablemodem
  SUPPORTED_DEVICES += tl-mr6400
endef
TARGET_DEVICES += tplink_tl-mr6400-v1

define Device/tplink_tl-wdr3500-v1
  $(Device/tplink-8mlzma)
  SOC := ar9344
  DEVICE_MODEL := TL-WDR3500
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x35000001
  SUPPORTED_DEVICES += tl-wdr3500
endef
TARGET_DEVICES += tplink_tl-wdr3500-v1

define Device/tplink_tl-wdr3600-v1
  $(Device/tplink-8mlzma)
  SOC := ar9344
  DEVICE_MODEL := TL-WDR3600
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x36000001
  SUPPORTED_DEVICES += tl-wdr4300
endef
TARGET_DEVICES += tplink_tl-wdr3600-v1

define Device/tplink_tl-wdr4300-v1
  $(Device/tplink-8mlzma)
  SOC := ar9344
  DEVICE_MODEL := TL-WDR4300
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x43000001
  SUPPORTED_DEVICES += tl-wdr4300
endef
TARGET_DEVICES += tplink_tl-wdr4300-v1

define Device/tplink_tl-wdr4300-v1-il
  $(Device/tplink-8mlzma)
  SOC := ar9344
  DEVICE_MODEL := TL-WDR4300
  DEVICE_VARIANT := v1 (IL)
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x43008001
  SUPPORTED_DEVICES += tl-wdr4300
endef
TARGET_DEVICES += tplink_tl-wdr4300-v1-il

define Device/tplink_tl-wdr4310-v1
  $(Device/tplink-8mlzma)
  SOC := ar9344
  DEVICE_MODEL := TL-WDR4310
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x43100001
  SUPPORTED_DEVICES += tl-wdr4300
endef
TARGET_DEVICES += tplink_tl-wdr4310-v1

define Device/tplink_tl-wdr4900-v2
  $(Device/tplink-8mlzma)
  SOC := qca9558
  DEVICE_MODEL := TL-WDR4900
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x49000002
  SUPPORTED_DEVICES += tl-wdr4900-v2
endef
TARGET_DEVICES += tplink_tl-wdr4900-v2

define Device/tplink_tl-wpa8630-v1
  $(Device/tplink-8mlzma)
  SOC := qca9563
  DEVICE_MODEL := TL-WPA8630
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca988x-ct
  TPLINK_HWID := 0x86300001
  SUPPORTED_DEVICES += tl-wpa8630
endef
TARGET_DEVICES += tplink_tl-wpa8630-v1

define Device/tplink_tl-wpa8630p-v2
  $(Device/tplink-safeloader)
  SOC := qca9563
  DEVICE_MODEL := TL-WPA8630P
  IMAGE_SIZE := 6016k
  DEVICE_PACKAGES := kmod-ath10k-ct ath10k-firmware-qca9888-ct
endef

define Device/tplink_tl-wpa8630p-v2-eu
  $(Device/tplink_tl-wpa8630p-v2)
  DEVICE_VARIANT := v2 (EU)
  TPLINK_BOARD_ID := TL-WPA8630P-V2-EU
endef
TARGET_DEVICES += tplink_tl-wpa8630p-v2-eu

define Device/tplink_tl-wpa8630p-v2-int
  $(Device/tplink_tl-wpa8630p-v2)
  DEVICE_VARIANT := v2 (Int.)
  TPLINK_BOARD_ID := TL-WPA8630P-V2-INT
endef
TARGET_DEVICES += tplink_tl-wpa8630p-v2-int

define Device/tplink_tl-wr1043nd-v1
  $(Device/tplink-8m)
  SOC := ar9132
  DEVICE_MODEL := TL-WR1043N/ND
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x10430001
  SUPPORTED_DEVICES += tl-wr1043nd
endef
TARGET_DEVICES += tplink_tl-wr1043nd-v1

define Device/tplink_tl-wr1043nd-v2
  $(Device/tplink-8mlzma)
  SOC := qca9558
  DEVICE_MODEL := TL-WR1043N/ND
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x10430002
  SUPPORTED_DEVICES += tl-wr1043nd-v2
endef
TARGET_DEVICES += tplink_tl-wr1043nd-v2

define Device/tplink_tl-wr1043nd-v3
  $(Device/tplink-8mlzma)
  SOC := qca9558
  DEVICE_MODEL := TL-WR1043N/ND
  DEVICE_VARIANT := v3
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x10430003
  SUPPORTED_DEVICES += tl-wr1043nd-v2
endef
TARGET_DEVICES += tplink_tl-wr1043nd-v3

define Device/tplink_tl-wr1043nd-v4
  $(Device/tplink-safeloader)
  SOC := qca9563
  IMAGE_SIZE := 15552k
  DEVICE_MODEL := TL-WR1043N/ND
  DEVICE_VARIANT := v4
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x10430004
  TPLINK_BOARD_ID := TLWR1043NDV4
  SUPPORTED_DEVICES += tl-wr1043nd-v4
endef
TARGET_DEVICES += tplink_tl-wr1043nd-v4

define Device/tplink_tl-wr1043n-v5
  $(Device/tplink-safeloader-uimage)
  SOC := qca9563
  IMAGE_SIZE := 15104k
  DEVICE_MODEL := TL-WR1043N
  DEVICE_VARIANT := v5
  TPLINK_BOARD_ID := TLWR1043NV5
  SUPPORTED_DEVICES += tl-wr1043n-v5
endef
TARGET_DEVICES += tplink_tl-wr1043n-v5

define Device/tplink_tl-wr1045nd-v2
  $(Device/tplink-8mlzma)
  SOC := qca9558
  DEVICE_MODEL := TL-WR1045ND
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x10450002
endef
TARGET_DEVICES += tplink_tl-wr1045nd-v2

define Device/tplink_tl-wr2543-v1
  $(Device/tplink-8mlzma)
  SOC := ar7242
  DEVICE_MODEL := TL-WR2543N/ND
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x25430001
  IMAGE/sysupgrade.bin := tplink-v1-image sysupgrade -v 3.13.99 | \
	append-metadata | check-size
  IMAGE/factory.bin := tplink-v1-image factory -v 3.13.99
  SUPPORTED_DEVICES += tl-wr2543n
endef
TARGET_DEVICES += tplink_tl-wr2543-v1

define Device/tplink_tl-wr710n-v1
  $(Device/tplink-8mlzma)
  SOC := ar9331
  DEVICE_MODEL := TL-WR710N
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb-chipidea2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x07100001
  SUPPORTED_DEVICES += tl-wr710n
endef
TARGET_DEVICES += tplink_tl-wr710n-v1

define Device/tplink_tl-wr710n-v2.1
  $(Device/tplink-8mlzma)
  SOC := ar9331
  DEVICE_MODEL := TL-WR710N
  DEVICE_VARIANT := v2.1
  DEVICE_PACKAGES := kmod-usb-chipidea2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x07100002
  TPLINK_HWREV := 0x2
  SUPPORTED_DEVICES += tl-wr710n
endef
TARGET_DEVICES += tplink_tl-wr710n-v2.1

define Device/tplink_tl-wr810n-v1
  $(Device/tplink-8mlzma)
  SOC := qca9531
  DEVICE_MODEL := TL-WR810N
  DEVICE_VARIANT := v1
  TPLINK_HWID := 0x8100001
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += tl-wr810n
endef
TARGET_DEVICES += tplink_tl-wr810n-v1

define Device/tplink_tl-wr810n-v2
  $(Device/tplink-8mlzma)
  SOC := qca9533
  DEVICE_MODEL := TL-WR810N
  DEVICE_VARIANT := v2
  TPLINK_HWID := 0x8100002
  SUPPORTED_DEVICES += tl-wr810n-v2
endef
TARGET_DEVICES += tplink_tl-wr810n-v2

define Device/tplink_tl-wr842n-v1
  $(Device/tplink-8m)
  SOC := ar7241
  DEVICE_MODEL := TL-WR842N/ND
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x8420001
  SUPPORTED_DEVICES += tl-mr3420
endef
TARGET_DEVICES += tplink_tl-wr842n-v1

define Device/tplink_tl-wr842n-v2
  $(Device/tplink-8mlzma)
  SOC := ar9341
  DEVICE_MODEL := TL-WR842N/ND
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x8420002
  SUPPORTED_DEVICES += tl-wr842n-v2
endef
TARGET_DEVICES += tplink_tl-wr842n-v2

define Device/tplink_tl-wr842n-v3
  $(Device/tplink-16mlzma)
  SOC := qca9533
  DEVICE_MODEL := TL-WR842N
  DEVICE_VARIANT := v3
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport
  TPLINK_HWID := 0x8420003
  SUPPORTED_DEVICES += tl-wr842n-v3
endef
TARGET_DEVICES += tplink_tl-wr842n-v3

define Device/tplink_tl-wr902ac-v1
  $(Device/tplink-safeloader)
  SOC := qca9531
  DEVICE_MODEL := TL-WR902AC
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-ath10k-ct-smallbuffers ath10k-firmware-qca9887-ct \
	-swconfig -uboot-envtools
  TPLINK_BOARD_ID := TL-WR902AC-V1
  IMAGE_SIZE := 7360k
  SUPPORTED_DEVICES += tl-wr902ac-v1
endef
TARGET_DEVICES += tplink_tl-wr902ac-v1

define Device/tplink_wbs210-v1
  $(Device/tplink-safeloader-okli)
  SOC := ar9344
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := WBS210
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := rssileds
  TPLINK_BOARD_ID := WBS210
  SUPPORTED_DEVICES += wbs210
endef
TARGET_DEVICES += tplink_wbs210-v1

define Device/tplink_wbs210-v2
  $(Device/tplink-safeloader-okli)
  SOC := ar9344
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := WBS210
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := rssileds
  TPLINK_BOARD_ID := WBS210V2
endef
TARGET_DEVICES += tplink_wbs210-v2

define Device/tplink_wbs510-v1
  $(Device/tplink-safeloader-okli)
  SOC := ar9344
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := WBS510
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := rssileds
  TPLINK_BOARD_ID := WBS510
  SUPPORTED_DEVICES += wbs510
endef
TARGET_DEVICES += tplink_wbs510-v1

define Device/tplink_wbs510-v2
  $(Device/tplink-safeloader-okli)
  SOC := ar9344
  IMAGE_SIZE := 7680k
  DEVICE_MODEL := WBS510
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES := rssileds
  TPLINK_BOARD_ID := WBS510V2
endef
TARGET_DEVICES += tplink_wbs510-v2
