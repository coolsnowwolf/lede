include ./common-tp-link.mk


define Device/archer-cxx
  KERNEL := kernel-bin | patch-cmdline | lzma | uImageArcher lzma
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef

define Device/archer-c25-v1
  $(Device/archer-cxx)
  DEVICE_TITLE := TP-LINK Archer C25 v1
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca9887
  BOARDNAME := ARCHER-C25-V1
  TPLINK_BOARD_ID := ARCHER-C25-V1
  DEVICE_PROFILE := ARCHERC25V1
  IMAGE_SIZE := 7808k
  LOADER_TYPE := elf
  MTDPARTS := spi0.0:128k(factory-uboot)ro,64k(u-boot)ro,1536k(kernel),6272k(rootfs),128k(config)ro,64k(art)ro,7808k@0x30000(firmware)
  SUPPORTED_DEVICES := archer-c25-v1
endef
TARGET_DEVICES += archer-c25-v1

define Device/archer-c58-v1
  $(Device/archer-cxx)
  DEVICE_TITLE := TP-LINK Archer C58 v1
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca9888
  BOARDNAME := ARCHER-C58-V1
  TPLINK_BOARD_ID := ARCHER-C58-V1
  DEVICE_PROFILE := ARCHERC58V1
  IMAGE_SIZE := 7936k
  MTDPARTS := spi0.0:64k(u-boot)ro,64k(mac)ro,7936k(firmware),64k(tplink)ro,64k(art)ro
  SUPPORTED_DEVICES := archer-c58-v1
endef
TARGET_DEVICES += archer-c58-v1

define Device/archer-c59-v1
  $(Device/archer-cxx)
  DEVICE_TITLE := TP-LINK Archer C59 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca9888
  BOARDNAME := ARCHER-C59-V1
  TPLINK_BOARD_ID := ARCHER-C59-V1
  DEVICE_PROFILE := ARCHERC59V1
  IMAGE_SIZE := 14528k
  MTDPARTS := spi0.0:64k(u-boot)ro,64k(mac)ro,1536k(kernel),12992k(rootfs),1664k(tplink)ro,64k(art)ro,14528k@0x20000(firmware)
  SUPPORTED_DEVICES := archer-c59-v1
endef
TARGET_DEVICES += archer-c59-v1

define Device/archer-c60-v1
  $(Device/archer-cxx)
  DEVICE_TITLE := TP-LINK Archer C60 v1
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca9888
  BOARDNAME := ARCHER-C60-V1
  TPLINK_BOARD_ID := ARCHER-C60-V1
  DEVICE_PROFILE := ARCHERC60V1
  IMAGE_SIZE := 7936k
  MTDPARTS := spi0.0:64k(u-boot)ro,64k(mac)ro,7936k(firmware),64k(tplink)ro,64k(art)ro
  SUPPORTED_DEVICES := archer-c60-v1
endef
TARGET_DEVICES += archer-c60-v1

define Device/archer-c60-v2
  $(Device/archer-c60-v1)
  DEVICE_TITLE := TP-LINK Archer C60 v2
  BOARDNAME := ARCHER-C60-V2
  TPLINK_BOARD_ID := ARCHER-C60-V2
  DEVICE_PROFILE := ARCHERC60V2
  IMAGE_SIZE := 7808k
  MTDPARTS := spi0.0:192k(u-boot)ro,7808k(firmware),128k(tplink)ro,64k(art)ro
  SUPPORTED_DEVICES := archer-c60-v2
endef
TARGET_DEVICES += archer-c60-v2

define Device/archer-c5-v1
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := TP-LINK Archer C5 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C5
  DEVICE_PROFILE := ARCHERC7
  TPLINK_HWID := 0xc5000001
endef
TARGET_DEVICES += archer-c5-v1

define Device/archer-c7-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK Archer C7 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C7
  DEVICE_PROFILE := ARCHERC7
  TPLINK_HWID := 0x75000001
endef
TARGET_DEVICES += archer-c7-v1

define Device/archer-c7-v2
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := TP-LINK Archer C7 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C7-V2
  DEVICE_PROFILE := ARCHERC7
  TPLINK_HWID := 0xc7000002
  IMAGES := sysupgrade.bin factory.bin factory-us.bin factory-eu.bin
  IMAGE/factory-us.bin := append-rootfs | mktplinkfw factory -C US
  IMAGE/factory-eu.bin := append-rootfs | mktplinkfw factory -C EU
endef
TARGET_DEVICES += archer-c7-v2

define Device/archer-c7-v2-il
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := TP-LINK Archer C7 v2 (IL)
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C7-V2
  DEVICE_PROFILE := ARCHERC7
  TPLINK_HWID := 0xc7000002
  TPLINK_HWREV := 0x494c0001
endef
TARGET_DEVICES += archer-c7-v2-il

define Device/tl-wdr7500-v3
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK Archer C7 v3
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C7
  DEVICE_PROFILE := ARCHERC7
  TPLINK_HWID := 0x75000003
endef
TARGET_DEVICES += tl-wdr7500-v3

define Device/archer-c7-v4
  $(Device/archer-cxx)
  DEVICE_TITLE := TP-LINK Archer C7 v4
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := ARCHER-C7-V4
  TPLINK_BOARD_ID := ARCHER-C7-V4
  IMAGE_SIZE := 15104k
  LOADER_TYPE := elf
  MTDPARTS := spi0.0:128k(factory-uboot)ro,128k(u-boot)ro,1536k(kernel),13568k(rootfs),960k(config)ro,64k(art)ro,15104k@0x40000(firmware)
  SUPPORTED_DEVICES := archer-c7-v4
endef
TARGET_DEVICES += archer-c7-v4

define Device/archer-c7-v5
  $(Device/archer-c7-v4)
  DEVICE_TITLE := TP-LINK Archer C7 v5
  BOARDNAME := ARCHER-C7-V5
  TPLINK_BOARD_ID := ARCHER-C7-V5
  IMAGE_SIZE := 15104k
  MTDPARTS := spi0.0:128k(factory-uboot)ro,128k(u-boot)ro,64k@0x50000(art)ro,1536k@0xc0000(kernel),13824k(rootfs),15360k@0xc0000(firmware)
  SUPPORTED_DEVICES := archer-c7-v5
endef
TARGET_DEVICES += archer-c7-v5

define Device/cpexxx
  DEVICE_PACKAGES := rssileds
  MTDPARTS := spi0.0:128k(u-boot)ro,64k(partition-table)ro,64k(product-info)ro,1792k(kernel),5888k(rootfs),192k(config)ro,64k(ART)ro,7680k@0x40000(firmware)
  IMAGE_SIZE := 7680k
  DEVICE_PROFILE := CPE510
  LOADER_TYPE := elf
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef

define Device/cpe510-520-v1
  $(Device/cpexxx)
  DEVICE_TITLE := TP-LINK CPE510/520 v1
  BOARDNAME := CPE510
  TPLINK_BOARD_ID := CPE510
  LOADER_FLASH_OFFS := 0x43000
  COMPILE := loader-$(1).elf
  COMPILE/loader-$(1).elf := loader-okli-compile
  KERNEL := kernel-bin | lzma | uImage lzma -M 0x4f4b4c49 | loader-okli $(1) 12288
endef
TARGET_DEVICES += cpe510-520-v1

define Device/cpe210-220-v1
  $(Device/cpe510-520-v1)
  DEVICE_TITLE := TP-LINK CPE210/220 v1
  BOARDNAME := CPE210
  TPLINK_BOARD_ID := CPE210
endef
TARGET_DEVICES += cpe210-220-v1

define Device/cpe210-v2
  $(Device/cpexxx)
  DEVICE_TITLE := TP-LINK CPE210 v2
  BOARDNAME := CPE210V2
  TPLINK_BOARD_ID := CPE210V2
  KERNEL := kernel-bin | patch-cmdline | lzma | tplink-v1-header
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0
  TPLINK_HEADER_VERSION := 1
endef
TARGET_DEVICES += cpe210-v2

define Device/wbs210-v1
  $(Device/cpe510-520-v1)
  DEVICE_TITLE := TP-LINK WBS210 v1
  BOARDNAME := WBS210
  TPLINK_BOARD_ID := WBS210
endef
TARGET_DEVICES += wbs210-v1

define Device/wbs510-v1
  $(Device/cpe510-520-v1)
  DEVICE_TITLE := TP-LINK WBS510 v1
  BOARDNAME := WBS510
  TPLINK_BOARD_ID := WBS510
endef
TARGET_DEVICES += wbs510-v1

define Device/eap120-v1
  DEVICE_TITLE := TP-LINK EAP120 v1
  MTDPARTS := spi0.0:128k(u-boot)ro,64k(partition-table)ro,64k(product-info)ro,1536k(kernel),14336k(rootfs),192k(config)ro,64k(ART)ro,15872k@0x40000(firmware)
  IMAGE_SIZE := 15872k
  BOARDNAME := EAP120
  TPLINK_BOARD_ID := EAP120
  DEVICE_PROFILE := EAP120
  LOADER_TYPE := elf
  KERNEL := kernel-bin | patch-cmdline | lzma | loader-kernel
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef
TARGET_DEVICES += eap120-v1

define Device/re355-v1
  DEVICE_TITLE := TP-LINK RE355 v1
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
  MTDPARTS := spi0.0:128k(u-boot)ro,6016k(firmware),64k(partition-table)ro,64k(product-info)ro,1856k(config)ro,64k(art)ro
  IMAGE_SIZE := 7936k
  BOARDNAME := RE355
  TPLINK_BOARD_ID := RE355
  DEVICE_PROFILE := RE355
  LOADER_TYPE := elf
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0
  TPLINK_HEADER_VERSION := 1
  KERNEL := kernel-bin | patch-cmdline | lzma | tplink-v1-header
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef
TARGET_DEVICES += re355-v1

define Device/re450-v1
  DEVICE_TITLE := TP-LINK RE450 v1
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
  MTDPARTS := spi0.0:128k(u-boot)ro,6016k(firmware),64k(partition-table)ro,64k(product-info)ro,1856k(config)ro,64k(art)ro
  IMAGE_SIZE := 7936k
  BOARDNAME := RE450
  TPLINK_BOARD_ID := RE450
  DEVICE_PROFILE := RE450
  LOADER_TYPE := elf
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0
  TPLINK_HEADER_VERSION := 1
  KERNEL := kernel-bin | patch-cmdline | lzma | tplink-v1-header
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef
TARGET_DEVICES += re450-v1

define Device/tl-mr6400-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-MR6400 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-net kmod-usb-net-rndis kmod-usb-serial kmod-usb-serial-option adb-enablemodem
  BOARDNAME := TL-MR6400
  DEVICE_PROFILE := TLMR6400
  TPLINK_HWID := 0x64000001
endef
TARGET_DEVICES += tl-mr6400-v1

define Device/tl-wdr3500-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WDR3500 v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WDR3500
  DEVICE_PROFILE := TLWDR4300
  TPLINK_HWID := 0x35000001
endef
TARGET_DEVICES += tl-wdr3500-v1

define Device/tl-wdr3600-v1
  $(Device/tl-wdr3500-v1)
  DEVICE_TITLE := TP-LINK TL-WDR3600 v1
  BOARDNAME := TL-WDR4300
  TPLINK_HWID := 0x36000001
  IMAGE/factory.bin := append-rootfs | mktplinkfw factory -C US
endef
TARGET_DEVICES += tl-wdr3600-v1

define Device/tl-wdr4300-v1
  $(Device/tl-wdr3600-v1)
  DEVICE_TITLE := TP-LINK TL-WDR4300 v1
  TPLINK_HWID := 0x43000001
endef
TARGET_DEVICES += tl-wdr4300-v1

define Device/tl-wdr4300-v1-il
  $(Device/tl-wdr3500-v1)
  DEVICE_TITLE := TP-LINK TL-WDR4300 v1 (IL)
  BOARDNAME := TL-WDR4300
  TPLINK_HWID := 0x43008001
endef
TARGET_DEVICES += tl-wdr4300-v1-il

define Device/tl-wdr4310-v1
  $(Device/tl-wdr4300-v1-il)
  DEVICE_TITLE := TP-LINK TL-WDR4310 v1
  TPLINK_HWID := 0x43100001
endef
TARGET_DEVICES += tl-wdr4310-v1

define Device/tl-wdr4900-v2
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WDR4900 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WDR4900-v2
  DEVICE_PROFILE := TLWDR4900V2
  TPLINK_HWID := 0x49000002
endef
TARGET_DEVICES += tl-wdr4900-v2

define Device/tl-wdr6500-v2
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WDR6500 v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  KERNEL := kernel-bin | patch-cmdline | lzma | uImage lzma
  KERNEL_INITRAMFS := kernel-bin | patch-cmdline | lzma | uImage lzma | tplink-v1-header
  BOARDNAME := TL-WDR6500-v2
  DEVICE_PROFILE := TLWDR6500V2
  TPLINK_HWID := 0x65000002
  TPLINK_HEADER_VERSION := 2
endef
TARGET_DEVICES += tl-wdr6500-v2

define Device/mw4530r-v1
  $(Device/tl-wdr4300-v1)
  DEVICE_TITLE := Mercury MW4530R v1
  TPLINK_HWID := 0x45300001
endef
TARGET_DEVICES += mw4530r-v1

define Device/tl-wpa8630-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WPA8630 v1
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := TL-WPA8630
  DEVICE_PROFILE := TL-WPA8630
  TPLINK_HWID := 0x86300001
endef
TARGET_DEVICES += tl-wpa8630-v1

define Device/tl-wr1043n-v5
  $(Device/archer-cxx)
  DEVICE_TITLE := TP-LINK TL-WR1043N v5
  BOARDNAME := TL-WR1043N-v5
  SUPPORTED_DEVICES := tl-wr1043n-v5
  DEVICE_PROFILE := TLWR1043
  MTDPARTS := spi0.0:128k(factory-uboot)ro,128k(u-boot)ro,15104k(firmware),128k(product-info)ro,640k(config)ro,64k(partition-table)ro,128k(logs)ro,64k(art)ro
  IMAGE_SIZE := 15104k
  TPLINK_BOARD_ID := TLWR1043NV5
endef
TARGET_DEVICES += tl-wr1043n-v5

define Device/tl-wr1043nd-v1
  $(Device/tplink-8m)
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR1043ND
  DEVICE_PROFILE := TLWR1043
  TPLINK_HWID := 0x10430001
endef
TARGET_DEVICES += tl-wr1043nd-v1

define Device/tl-wr1043nd-v2
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR1043ND-v2
  DEVICE_PROFILE := TLWR1043
  TPLINK_HWID := 0x10430002
endef
TARGET_DEVICES += tl-wr1043nd-v2

define Device/tl-wr1043nd-v3
  $(Device/tl-wr1043nd-v2)
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v3
  TPLINK_HWID := 0x10430003
endef
TARGET_DEVICES += tl-wr1043nd-v3

define Device/tl-wr1043nd-v4
  $(Device/tplink)
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v4
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR1043ND-v4
  DEVICE_PROFILE := TLWR1043
  TPLINK_HWID :=  0x10430004
  MTDPARTS := spi0.0:128k(u-boot)ro,1536k(kernel),14016k(rootfs),128k(product-info)ro,320k(config)ro,64k(partition-table)ro,128k(logs)ro,64k(ART)ro,15552k@0x20000(firmware)
  IMAGE_SIZE := 15552k
  TPLINK_BOARD_ID := TLWR1043NDV4
  KERNEL := kernel-bin | patch-cmdline | lzma | tplink-v1-header
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef
TARGET_DEVICES += tl-wr1043nd-v4

define Device/tl-wr2543-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WR2543N/ND v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR2543N
  DEVICE_PROFILE := TLWR2543
  TPLINK_HWID := 0x25430001
  IMAGE/sysupgrade.bin := append-rootfs | mktplinkfw sysupgrade -v 3.13.99
  IMAGE/factory.bin := append-rootfs | mktplinkfw factory -v 3.13.99
endef
TARGET_DEVICES += tl-wr2543-v1

define Device/tl-wr710n-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WR710N v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := TL-WR710N
  DEVICE_PROFILE := TLWR710
  TPLINK_HWID := 0x07100001
  CONSOLE := ttyATH0,115200
  IMAGE/factory.bin := append-rootfs | mktplinkfw factory -C US
endef
TARGET_DEVICES += tl-wr710n-v1

define Device/tl-wr710n-v2.1
  $(Device/tl-wr710n-v1)
  DEVICE_TITLE := TP-LINK TL-WR710N v2.1
  TPLINK_HWID := 0x07100002
  TPLINK_HWREV := 0x00000002
endef
TARGET_DEVICES += tl-wr710n-v2.1

define Device/tl-wr810n-v1
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WR810N v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := TL-WR810N
  DEVICE_PROFILE := TLWR810
  TPLINK_HWID := 0x08100001
endef
TARGET_DEVICES += tl-wr810n-v1

define Device/tl-wr810n-v2
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WR810N v2
  BOARDNAME := TL-WR810N-v2
  DEVICE_PROFILE := TLWR810
  TPLINK_HWID := 0x08100002
endef
TARGET_DEVICES += tl-wr810n-v2

define Device/tl-wr842n-v1
  $(Device/tplink-8m)
  DEVICE_TITLE := TP-LINK TL-WR842N/ND v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-MR3420
  DEVICE_PROFILE := TLWR842
  TPLINK_HWID := 0x08420001
endef
TARGET_DEVICES += tl-wr842n-v1

define Device/tl-wr842n-v2
  $(Device/tplink-8mlzma)
  DEVICE_TITLE := TP-LINK TL-WR842N/ND v2
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR842N-v2
  DEVICE_PROFILE := TLWR842
  TPLINK_HWID := 0x8420002
endef
TARGET_DEVICES += tl-wr842n-v2

define Device/tl-wr842n-v3
  $(Device/tplink-16mlzma)
  DEVICE_TITLE := TP-LINK TL-WR842N/ND v3
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR842N-v3
  DEVICE_PROFILE := TLWR842
  TPLINK_HWID := 0x08420003
endef
TARGET_DEVICES += tl-wr842n-v3

define Device/tl-wr902ac-v1
  DEVICE_TITLE := TP-LINK TL-WR902AC v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-ath10k ath10k-firmware-qca9887 -swconfig -uboot-envtools
  BOARDNAME := TL-WR902AC-V1
  DEVICE_PROFILE := TLWR902
  TPLINK_BOARD_ID := TL-WR902AC-V1
  TPLINK_HWID := 0x0
  TPLINK_HWREV := 0
  TPLINK_HEADER_VERSION := 1
  SUPPORTED_DEVICES := tl-wr902ac-v1
  IMAGE_SIZE := 7360k
  KERNEL := kernel-bin | patch-cmdline | lzma | tplink-v1-header
  IMAGES += factory.bin
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  MTDPARTS := spi0.0:128k(u-boot)ro,7360k(firmware),640k(tplink)ro,64k(art)ro
endef
TARGET_DEVICES += tl-wr902ac-v1

define Device/tl-wr942n-v1
  DEVICE_TITLE := TP-LINK TL-WR942N v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR942N-V1
  TPLINK_BOARD_ID := TLWR942NV1
  DEVICE_PROFILE := TLWR942
  IMAGE_SIZE := 14464k
  KERNEL := kernel-bin | patch-cmdline | lzma | uImageArcher lzma
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | \
	append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
  MTDPARTS := spi0.0:128k(u-boot)ro,14464k(firmware),64k(product-info)ro,64k(partition-table)ro,256k(oem-config)ro,1344k(oem-vars)ro,64k(ART)ro
  SUPPORTED_DEVICES := tl-wr942n-v1
endef
TARGET_DEVICES += tl-wr942n-v1
