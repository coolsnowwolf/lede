#
# Copyright (C) 2010-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

# boards missing since devicetree update
#EASY50712 ARV3527P

KERNEL_LOADADDR = 0x80002000
KERNEL_ENTRY = 0x80002000

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

ifeq ($(SUBTARGET),xway)
  UBIFS_OPTS := -m 512 -e 15872 -c 1959
else
  UBIFS_OPTS := -m 2048 -e 126KiB -c 4096
endif

define Build/append-avm-fakeroot
	cat ./eva.dummy.squashfs >> $@
endef

define Build/dgn3500-sercom-footer
	$(STAGING_DIR_HOST)/bin/dgn3500sum $@ $(1) $(2)
endef

define Build/mkbrncmdline
   mkbrncmdline -i $@ -o $@.new BRN-BOOT
   mv $@.new $@
endef

define Build/mkbrnimg
   mkbrnimg -s $(SIGNATURE) -m $(MAGIC) -p $(CRC32_POLY) -o $@ $(IMAGE_KERNEL) $(IMAGE_ROOTFS)
endef

define Build/fullimage
	mkimage -A mips -O linux -C lzma -T filesystem -a 0x00  \
		-e 0x00 -n 'LEDE RootFS' \
		-d $(IMAGE_ROOTFS) $(IMAGE_ROOTFS).new

	cat $(IMAGE_KERNEL) $(IMAGE_ROOTFS).new > $@.tmp

	mkimage -A mips -O linux -T multi -a 0x00 -C none \
		-e 0x00 -n 'OpenWrt fullimage' \
		-d $@.tmp $@

	rm $(IMAGE_ROOTFS).new
	rm $@.tmp
endef

# Shared device definition: applies to every defined device
define Device/Default
  PROFILES = Default
  KERNEL_DEPENDS = $$(wildcard ../dts/$$(DEVICE_DTS).dts)
  KERNEL_INITRAMFS_NAME = $$(KERNEL_NAME)-initramfs
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | uImage lzma
  FILESYSTEMS := squashfs
  DEVICE_DTS := $(1)
  DEVICE_DTS_DIR := ../dts
  IMAGE_SIZE :=
  SUPPORTED_DEVICES = $$(DEVICE_DTS)
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef
DEVICE_VARS += IMAGE_SIZE

define Device/NAND/xway
  BLOCKSIZE := 16k
  PAGESIZE := 512
  SUBPAGESIZE := 256
  FILESYSTEMS += ubifs
endef

define Device/NAND/xrx200
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  FILESYSTEMS += ubifs
endef

define Device/NAND
  $(Device/NAND/$(SUBTARGET))
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/lantiqBrnImage
  KERNEL := kernel-bin | append-dtb | mkbrncmdline | lzma-no-dict
  IMAGES := factory.bin
  IMAGE/factory.bin := mkbrnimg | check-size $$$$(IMAGE_SIZE)
endef
DEVICE_VARS += SIGNATURE MAGIC CRC32_POLY

define Device/lantiqFullImage
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma | pad-offset 4 0
  IMAGES := sysupgrade.bin fullimage.img
  IMAGE/fullimage.img := fullimage | check-size $$$$(IMAGE_SIZE)
endef

define Device/AVM
  KERNEL := kernel-bin | append-dtb | lzma | eva-image
  IMAGE/sysupgrade.bin := append-kernel | pad-to 64k | append-avm-fakeroot | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef

ifeq ($(SUBTARGET),ase)

define Device/ALL0333CJ
  IMAGE_SIZE := 3700k
  DEVICE_DTS := ALL0333CJ
  DEVICE_TITLE := Allnet ALL0333CJ
  DEVICE_PACKAGES := kmod-ltq-adsl-ase kmod-ltq-adsl-ase-mei \
	kmod-ltq-adsl-ase-fw-b kmod-ltq-atm-ase \
	ltq-adsl-app ppp-mod-pppoe
endef
TARGET_DEVICES += ALL0333CJ

define Device/DGN1000B
  IMAGE_SIZE := 6000k
  DEVICE_DTS := DGN1000B
  DEVICE_TITLE := Netgear DGN1000B
  DEVICE_PACKAGES := kmod-ltq-adsl-ase kmod-ltq-adsl-ase-mei \
	kmod-ltq-adsl-ase-fw-b kmod-ltq-atm-ase \
	ltq-adsl-app ppp-mod-pppoe
endef
TARGET_DEVICES += DGN1000B

endif

ifeq ($(SUBTARGET),xway_legacy)

define Device/ARV4520PW
  IMAGE_SIZE := 3648k
  DEVICE_TITLE := Easybox 800, WAV-281 - ARV4520PW
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-rt61-pci wpad-mini
endef
TARGET_DEVICES += ARV4520PW

define Device/ARV4525PW
  IMAGE_SIZE := 3776k
  DEVICE_TITLE := Speedport W502V Typ A - ARV4525PW
  DEVICE_PACKAGES := kmod-ath5k wpad-mini \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa -swconfig
endef
TARGET_DEVICES += ARV4525PW

define Device/ARV452CQW
  IMAGE_SIZE := 3776k
  DEVICE_TITLE := Easybox 801 - ARV452CQW
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ath5k wpad-mini \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa
endef
TARGET_DEVICES += ARV452CQW

define Device/ARV4518PWR01
  IMAGE_SIZE := 3776k
  DEVICE_TITLE := ARV4518PWR01
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath5k wpad-mini
endef
TARGET_DEVICES += ARV4518PWR01

define Device/ARV4518PWR01A
  IMAGE_SIZE := 3776k
  DEVICE_TITLE := ARV4518PWR01A
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath5k wpad-mini
endef
TARGET_DEVICES += ARV4518PWR01A

endif

ifeq ($(SUBTARGET),xway)

# Danube

define Device/BTHOMEHUBV2B
  $(Device/NAND)
  DEVICE_TITLE := BT Home Hub 2B
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	kmod-ltq-deu-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini
endef
TARGET_DEVICES += BTHOMEHUBV2B

define Device/EASY50712
  IMAGE_SIZE := 3776k
  DEVICE_TITLE := Lantiq Danube - EASY50712
endef
TARGET_DEVICES += EASY50712

define Device/ACMP252
  IMAGE_SIZE := 14848k
  DEVICE_TITLE := AudioCodes MediaPack MP-252
  DEVICE_PACKAGES :=  kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-usb-ledtrig-usbport kmod-usb-dwc2 \
	kmod-rt2800-pci \
	ltq-adsl-app ppp-mod-pppoa \
	wpad-mini
endef
TARGET_DEVICES += ACMP252

define Device/ARV4510PW
  IMAGE_SIZE := 15616k
  DEVICE_TITLE := Wippies, Elisa - ARV4510PW
  DEVICE_PACKAGES := kmod-usb-ledtrig-usbport kmod-usb2-pci kmod-usb-uhci \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci kmod-ath5k wpad-mini
endef
TARGET_DEVICES += ARV4510PW

define Device/ARV7525PW
  IMAGE_SIZE := 3776k
  DEVICE_TITLE := Speedport W303V Typ A - ARV7525PW
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa -swconfig
endef
TARGET_DEVICES += ARV7525PW

define Device/ARV4519PW
  IMAGE_SIZE := 3776k
  DEVICE_TITLE := Vodafone, Pirelli - ARV4519PW
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa
endef
TARGET_DEVICES += ARV4519PW

define Device/ARV7506PW11
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := Alice/O2 IAD 4421 - ARV7506PW11
  DEVICE_PACKAGES := kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-rt2800-pci wpad-mini
endef
TARGET_DEVICES += ARV7506PW11

define Device/ARV7510PW22
  IMAGE_SIZE := 31232k
  DEVICE_TITLE := Astoria - ARV7510PW22
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci wpad-mini \
	kmod-usb-uhci kmod-usb2 kmod-usb2-pci
endef
TARGET_DEVICES += ARV7510PW22

define Device/ARV7518PW
  IMAGE_SIZE := 7872k
  DEVICE_TITLE := Astoria - ARV7518PW
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini
endef
TARGET_DEVICES += ARV7518PW

define Device/ARV7519PW
  IMAGE_SIZE := 15488k
  DEVICE_TITLE := Astoria - ARV7519PW
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-rt2800-pci wpad-mini
endef
TARGET_DEVICES += ARV7519PW

define Device/ARV752DPW
  IMAGE_SIZE := 7872k
  DEVICE_TITLE := Easybox 802 - ARV752DPW
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci wpad-mini
endef
TARGET_DEVICES += ARV752DPW

define Device/ARV752DPW22
  IMAGE_SIZE := 7616k
  DEVICE_TITLE := Easybox 803 - ARV752DPW22
  DEVICE_PACKAGES := kmod-usb2-pci kmod-usb-uhci kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci wpad-mini
endef
TARGET_DEVICES += ARV752DPW22

define Device/ARV8539PW22
  IMAGE_SIZE := 7616k
  DEVICE_TITLE := Speedport W504V Typ A - ARV8539PW22
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini
endef
TARGET_DEVICES += ARV8539PW22

define Device/GIGASX76X
  IMAGE_SIZE := 7680k
  DEVICE_TITLE := Gigaset sx76x
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoe \
	kmod-ath5k wpad-mini
endef
TARGET_DEVICES += GIGASX76X


# AR9

define Device/BTHOMEHUBV3A
  $(Device/NAND)
  DEVICE_TITLE := BT Home Hub 3A
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
	kmod-ltq-deu-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini \
	uboot-envtools
endef
TARGET_DEVICES += BTHOMEHUBV3A

DGN3500_KERNEL_OFFSET_HEX=0x50000
DGN3500_KERNEL_OFFSET_DEC=327680
define Device/DGN3500
  IMAGE_SIZE := 16000k
  IMAGES := \
	sysupgrade-NA.bin sysupgrade-WW.bin \
	factory-NA.img factory-WW.img
  IMAGE/sysupgrade-NA.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "NA" | \
	pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade-WW.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "WW" | \
	pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory-NA.img := \
	pad-extra $(DGN3500_KERNEL_OFFSET_DEC) | append-kernel | append-rootfs | \
	dgn3500-sercom-footer $(DGN3500_KERNEL_OFFSET_HEX) "NA" | pad-rootfs | \
	check-size 16320k | pad-to 16384k
  IMAGE/factory-WW.img := \
	pad-extra $(DGN3500_KERNEL_OFFSET_DEC) | append-kernel | append-rootfs | \
	dgn3500-sercom-footer $(DGN3500_KERNEL_OFFSET_HEX) "WW" | pad-rootfs | \
	check-size 16320k | pad-to 16384k
  DEVICE_TITLE := Netgear DGN3500
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ath9k kmod-owl-loader wpad-mini \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-deu-ar9
endef
TARGET_DEVICES += DGN3500

define Device/DGN3500B
  IMAGE_SIZE := 16000k
  IMAGES += factory.img
  IMAGE/sysupgrade.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "DE" | \
	pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.img := \
	pad-extra $(DGN3500_KERNEL_OFFSET_DEC) | append-kernel | append-rootfs | \
	dgn3500-sercom-footer $(DGN3500_KERNEL_OFFSET_HEX) "DE" | pad-rootfs | \
	check-size 16320k | pad-to 16384k
  DEVICE_TITLE := Netgear DGN3500B
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ath9k kmod-owl-loader wpad-mini \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-deu-ar9
endef
TARGET_DEVICES += DGN3500B

define Device/WBMRA
  IMAGE_SIZE := 31488k
  DEVICE_DTS := WBMR
  DEVICE_TITLE := Buffalo WBMR-HP-G300H (A) - WBMR
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini
endef
TARGET_DEVICES += WBMRA

define Device/WBMRB
  IMAGE_SIZE := 31488k
  DEVICE_DTS := WBMR
  DEVICE_TITLE := Buffalo WBMR-HP-G300H (B) - WBMR
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini
endef
TARGET_DEVICES += WBMRB

define Device/FRITZ7320
  $(Device/AVM)
  IMAGE_SIZE := 15744k
  DEVICE_TITLE := 1&1 HomeServer - FRITZ7320
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-mini \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-deu-ar9 kmod-usb-dwc2 -swconfig
endef
TARGET_DEVICES += FRITZ7320

define Device/H201L
  IMAGE_SIZE := 7808k
  DEVICE_TITLE := ZTE H201L - H201L
  DEVICE_PACKAGES := kmod-ath9k-htc wpad-mini \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoe \
	kmod-ltq-deu-ar9 kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-tapi kmod-ltq-vmmc
endef
TARGET_DEVICES += H201L

define Device/P2601HNFX
  IMAGE_SIZE := 15616k
  DEVICE_TITLE := ZyXEL P-2601HN-Fx
  DEVICE_PACKAGES := kmod-rt2800-usb wpad-mini \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoe \
	kmod-ltq-deu-ar9 kmod-usb-dwc2
endef
TARGET_DEVICES += P2601HNFX


endif


ifeq ($(SUBTARGET),xrx200)

# VR9

define Device/P2812HNUF1
  $(Device/NAND)
  DEVICE_TITLE := ZyXEL P-2812HNU-F1
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += P2812HNUF1

define Device/P2812HNUF3
  $(Device/NAND)
  DEVICE_TITLE := ZyXEL P-2812HNU-F3
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2
endef
TARGET_DEVICES += P2812HNUF3

define Device/ARV7519RW22
  IMAGE_SIZE := 31232k
  DEVICE_TITLE := Livebox Astoria ARV7519RW22
  DEVICE_PACKAGES := kmod-usb-dwc2
endef
TARGET_DEVICES += ARV7519RW22

define Device/ASL56026
  IMAGE_SIZE := 7488k
  DEVICE_TITLE := BT OpenReach - ECI VDSL Modem V-2FUb/I
endef
TARGET_DEVICES += ASL56026

define Device/BTHOMEHUBV5A
  $(Device/NAND)
  DEVICE_TITLE := BT Home Hub 5A
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader \
	kmod-ath10k ath10k-firmware-qca988x wpad-mini kmod-usb-dwc2
endef
TARGET_DEVICES += BTHOMEHUBV5A

define Device/DM200
  IMAGES := sysupgrade.bin factory.img
  IMAGE/factory.img := $$(IMAGE/sysupgrade.bin) | netgear-dni
  IMAGE_SIZE := 7872k
  DEVICE_TITLE := Netgear DM200
  NETGEAR_BOARD_ID := DM200
  NETGEAR_HW_ID := 29765233+8+0+64+0+0
endef
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID
TARGET_DEVICES += DM200

define Device/EASY80920NAND
  $(Device/lantiqFullImage)
  IMAGE_SIZE := 64512k
  DEVICE_TITLE := Lantiq VR9 - EASY80920NAND
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-mini kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += EASY80920NAND

define Device/EASY80920NOR
  IMAGE_SIZE := 7936k
  DEVICE_TITLE := Lantiq VR9 - EASY80920NOR
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-mini kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += EASY80920NOR

define Device/FRITZ3370
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_TITLE := Fritz!Box WLan - FRITZ3370
  DEVICE_PACKAGES := kmod-ath9k wpad-mini kmod-usb-dwc2
endef
TARGET_DEVICES += FRITZ3370

define Device/FRITZ7360SL
  $(Device/AVM)
  IMAGE_SIZE := 15744k
  DEVICE_TITLE := 1&1 HomeServer - FRITZ7360SL
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-mini kmod-usb-dwc2
endef
TARGET_DEVICES += FRITZ7360SL

define Device/VG3503J
  IMAGE_SIZE := 8000k
  DEVICE_TITLE := BT OpenReach - ECI VDSL Modem V-2FUb/R
endef
TARGET_DEVICES += VG3503J

include tp-link.mk

define Device/WBMR300
  IMAGE_SIZE := 15616k
  DEVICE_TITLE := Buffalo WBMR-300HPD
  DEVICE_PACKAGES := kmod-mt7603 wpad-mini kmod-usb-dwc2
endef
TARGET_DEVICES += WBMR300

define Device/VGV7510KW22NOR
  IMAGE_SIZE := 15232k
  DEVICE_TITLE := o2 Box 6431 / Arcadyan VGV7510KW22 (NOR)
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2 kmod-ltq-tapi kmod-ltq-vmmc
endef
TARGET_DEVICES += VGV7510KW22NOR

define Device/VGV7510KW22BRN
  $(Device/lantiqBrnImage)
  IMAGE_SIZE := 7168k
  SIGNATURE := BRNDA6431
  MAGIC := 0x12345678
  CRC32_POLY := 0x04c11db7
  DEVICE_TITLE := o2 Box 6431 / Arcadyan VGV7510KW22 (BRN)
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2 kmod-ltq-tapi kmod-ltq-vmmc
endef
TARGET_DEVICES += VGV7510KW22BRN

define Device/VGV7519NOR
  IMAGE_SIZE := 15360k
  DEVICE_TITLE := Experiabox 8 VGV7519
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2 kmod-ltq-tapi kmod-ltq-vmmc
endef
TARGET_DEVICES += VGV7519NOR

define Device/VGV7519BRN
  $(Device/lantiqBrnImage)
  IMAGE_SIZE := 7168k
  SIGNATURE := 5D00008000
  MAGIC := 0x12345678
  CRC32_POLY := 0x2083b8ed
  DEVICE_TITLE := Experiabox 8 VGV7519 (BRN)
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2 kmod-ltq-tapi kmod-ltq-vmmc
endef
TARGET_DEVICES += VGV7519BRN

endif


ifeq ($(SUBTARGET),falcon)

define Device/EASY98000NOR
  IMAGE_SIZE := 3904k
  DEVICE_TITLE := EASY98000 - Lantiq Falcon Eval Board NOR
  DEVICE_PACKAGES := kmod-dm9000 kmod-i2c-lantiq kmod-eeprom-at24
endef
TARGET_DEVICES += EASY98000NOR

define Device/EASY98000NAND
  IMAGE_SIZE := 3904k
  DEVICE_TITLE := EASY98000 - Lantiq Falcon Eval Board NAND
  DEVICE_PACKAGES := kmod-dm9000 kmod-i2c-lantiq kmod-eeprom-at24
endef
TARGET_DEVICES += EASY98000NAND

define Device/EASY98000SFLASH
  IMAGE_SIZE := 7424k
  DEVICE_TITLE := EASY98000 - Lantiq Falcon Eval Board SFLASH
  DEVICE_PACKAGES := kmod-dm9000 kmod-i2c-lantiq kmod-eeprom-at24
endef
TARGET_DEVICES += EASY98000SFLASH

define Device/FALCON-MDU
  IMAGE_SIZE := 7424k
  DEVICE_TITLE := MDU - Lantiq Falcon / VINAXdp MDU Board
endef
TARGET_DEVICES += FALCON-MDU

define Device/EASY88388
  IMAGE_SIZE := 7424k
  DEVICE_TITLE := EASY88388 - Lantiq Falcon FTTDP8 Reference Board
endef
TARGET_DEVICES += EASY88388

define Device/EASY88444
  IMAGE_SIZE := 7424k
  DEVICE_TITLE := EASY88444 - Lantiq Falcon FTTdp G.FAST Reference Board
endef
TARGET_DEVICES += EASY88444

define Device/FALCON-SFP
  IMAGE_SIZE := 7424k
  DEVICE_TITLE := SFP - Lantiq Falcon SFP Stick
endef
TARGET_DEVICES += FALCON-SFP

define Device/EASY98035SYNCE
  IMAGE_SIZE := 7424k
  DEVICE_TITLE := EASY98035SYNCE - Lantiq Falcon SFP Stick with Synchronous Ethernet
endef
TARGET_DEVICES += EASY98035SYNCE

define Device/EASY98035SYNCE1588
  IMAGE_SIZE := 7424k
  DEVICE_TITLE := EASY98035SYNCE1588 - Lantiq Falcon SFP Stick with SyncE and IEEE1588
endef
TARGET_DEVICES += EASY98035SYNCE1588

define Device/EASY98020
  IMAGE_SIZE := 7424k
  DEVICE_TITLE := EASY98020 - Lantiq Falcon SFU Reference Board
endef
TARGET_DEVICES += EASY98020

define Device/EASY98020V18
  IMAGE_SIZE := 7424k
  DEVICE_TITLE := EASY98020V18 - Lantiq Falcon SFU Reference Board V1.8
endef
TARGET_DEVICES += EASY98020V18

define Device/EASY98021
  IMAGE_SIZE := 7424k
  DEVICE_TITLE := EASY98021 - Lantiq Falcon HGU Reference Board
endef
TARGET_DEVICES += EASY98021

endif

$(eval $(call BuildImage))
