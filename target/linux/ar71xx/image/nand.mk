define Build/MerakiNAND
	-$(STAGING_DIR_HOST)/bin/mkmerakifw \
		-B $(BOARDNAME) -s \
		-i $@ \
		-o $@.new
	@mv $@.new $@
endef

define Build/MerakiNAND-old
	-$(STAGING_DIR_HOST)/bin/mkmerakifw-old \
		-B $(BOARDNAME) -s \
		-i $@ \
		-o $@.new
	@mv $@.new $@
endef


define Device/c-60
  DEVICE_TITLE := AirTight C-60
  DEVICE_PACKAGES := kmod-spi-gpio kmod-usb-core kmod-usb2 kmod-ath9k
  BOARDNAME := C-60
  BLOCKSIZE := 64k
  KERNEL_SIZE := 3648k
  IMAGE_SIZE := 32m
  IMAGES := sysupgrade.tar
  MTDPARTS := spi0.0:256k(u-boot)ro,128k(u-boot-env)ro,3648k(kernel),64k(art)ro;ar934x-nfc:32m(ubi)
  KERNEL := kernel-bin | patch-cmdline | lzma | uImage lzma
  IMAGE/sysupgrade.tar := sysupgrade-tar
endef
TARGET_DEVICES += c-60

define Device/domywifi-dw33d
  DEVICE_TITLE := DomyWifi DW33D
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := DW33D
  IMAGE_SIZE := 16000k
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,14528k(rootfs),1472k(kernel),64k(art)ro,16000k@0x50000(firmware);ar934x-nfc:96m(rootfs_data),32m(backup)ro
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | pad-to 14528k | append-kernel | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += domywifi-dw33d

define Device/sbr-ac1750
  DEVICE_TITLE := Arris sbr-ac1750
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-storage kmod-usb-ledtrig-usbport kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := SBR-AC1750
  IMAGE_SIZE := 95m
  KERNEL_SIZE := 4096k
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  UBINIZE_OPTS := -E 5
  CONSOLE := ttyS0,115200
  MTDPARTS := ar934x-nfc:1m(u-boot)ro,1m(u-boot-env)ro,4m(kernel),95m(ubi),1m(scfgmgr),4m(openwrt),1m(ft),2m(PKI),1m@0x6d00000(art)ro
  IMAGES := sysupgrade.tar kernel1.bin rootfs1.bin
  KERNEL := kernel-bin | patch-cmdline | lzma | uImage lzma
  IMAGE/kernel1.bin := append-kernel | check-size $$$$(KERNEL_SIZE)
  IMAGE/rootfs1.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += sbr-ac1750

define Device/hiveap-121
  DEVICE_TITLE := Aerohive HiveAP-121
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-i2c-gpio-custom kmod-spi-gpio kmod-ath9k kmod-tpm-i2c-atmel
  BOARDNAME := HiveAP-121
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 115m
  KERNEL_SIZE := 5120k
  UBINIZE_OPTS := -E 5
  CONSOLE := ttyS0,9600
  MTDPARTS := spi0.0:512k(u-boot)ro,64k(u-boot-env),64k(hw-info)ro,64k(boot-info)ro,64k(boot-sinfo)ro;ar934x-nfc:4096k(u-boot-1),4096k(u-boot-env-1),5m(kernel),111m(ubi),4096k(wifi-info)ro
  IMAGES := sysupgrade.tar factory.bin
  KERNEL := kernel-bin | patch-cmdline | lzma | uImage lzma
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.tar := sysupgrade-tar
endef
TARGET_DEVICES += hiveap-121

define Device/mr18
  DEVICE_TITLE := Meraki MR18
  DEVICE_PACKAGES := kmod-spi-gpio kmod-ath9k
  BOARDNAME := MR18
  BLOCKSIZE := 64k
  MTDPARTS := ar934x-nfc:512k(nandloader)ro,8M(kernel),8M(recovery),113664k(ubi),128k@130944k(odm-caldata)ro
  IMAGES := sysupgrade.tar
  KERNEL := kernel-bin | patch-cmdline | MerakiNAND
  KERNEL_INITRAMFS := kernel-bin | patch-cmdline | MerakiNAND
  IMAGE/sysupgrade.tar := sysupgrade-tar
endef
TARGET_DEVICES += mr18

define Device/rambutan
  DEVICE_TITLE := 8devices Rambutan
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2
  BOARDNAME := RAMBUTAN
  BLOCKSIZE := 128KiB
  MTDPARTS := ar934x-nfc:3M(u-boot)ro,2M(u-boot-env),1M(art),122M(ubi)
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  IMAGES := factory.ubi sysupgrade.tar
  IMAGE/sysupgrade.tar := sysupgrade-tar
  IMAGE/factory.ubi := append-ubi
endef
TARGET_DEVICES += rambutan

define Device/wi2a-ac200i
  SUPPORTED_DEVICES = $(1)
  DEVICE_TITLE := Nokia WI2A-AC200i
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-ath9k kmod-ath10k ath10k-firmware-qca988x
  BOARDNAME := WI2A-AC200i
  BLOCKSIZE := 64k
  KERNEL_SIZE := 3648k
  IMAGE_SIZE := 32m
  IMAGES := sysupgrade.tar
  MTDPARTS := spi0.0:256k(u-boot)ro,64k(envFacA)ro,64k(envFacB)ro,64k(ART)ro,128k(u-boot-env)
  KERNEL := kernel-bin | patch-cmdline | lzma | uImage lzma
  IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += wi2a-ac200i

define Device/z1
  DEVICE_TITLE := Meraki Z1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport kmod-spi-gpio kmod-ath9k kmod-owl-loader
  BOARDNAME := Z1
  BLOCKSIZE := 64k
  MTDPARTS := ar934x-nfc:128K(loader1)ro,8064K(kernel),128K(loader2)ro,8064K(recovery),114560K(ubi),128K(origcaldata)ro
  IMAGES := sysupgrade.tar
  KERNEL := kernel-bin | patch-cmdline | MerakiNAND-old
  KERNEL_INITRAMFS := kernel-bin | patch-cmdline | MerakiNAND-old
  IMAGE/sysupgrade.tar := sysupgrade-tar
endef
TARGET_DEVICES += z1

define LegacyDevice/R6100
  DEVICE_TITLE := NETGEAR R6100
  DEVICE_PACKAGES := kmod-ath10k ath10k-firmware-qca988x kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += R6100

define LegacyDevice/WNDR3700V4
  DEVICE_TITLE := NETGEAR WNDR3700v4
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += WNDR3700V4

define LegacyDevice/WNDR4300V1
  DEVICE_TITLE := NETGEAR WNDR4300v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
endef
LEGACY_DEVICES += WNDR4300V1

define LegacyDevice/NBG6716
  DEVICE_TITLE := Zyxel NBG 6716
  DEVICE_PACKAGES := kmod-rtc-pcf8563 kmod-ath10k ath10k-firmware-qca988x
endef
LEGACY_DEVICES += NBG6716
