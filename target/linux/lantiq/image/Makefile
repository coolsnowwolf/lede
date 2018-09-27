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
		-e 0x00 -n '$(VERSION_DIST) RootFS' \
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
  KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL_INITRAMFS_NAME = $$(KERNEL_NAME)-initramfs
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | uImage lzma
  FILESYSTEMS := squashfs
  IMAGE_SIZE :=
  SUPPORTED_DEVICES := $(subst _,$(comma),$(1))
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
  IMAGES := sysupgrade.bin fullimage.bin
  IMAGE/fullimage.bin := fullimage | check-size $$$$(IMAGE_SIZE)
endef

define Device/AVM
  KERNEL := kernel-bin | append-dtb | lzma | eva-image
  KERNEL_INITRAMFS := $$(KERNEL)
  IMAGE/sysupgrade.bin := append-kernel | pad-to 64k | append-avm-fakeroot | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef

ifeq ($(SUBTARGET),ase)

define Device/allnet_all0333cj
  IMAGE_SIZE := 3700k
  DEVICE_DTS := ALL0333CJ
  DEVICE_TITLE := Allnet ALL0333CJ
  DEVICE_PACKAGES := kmod-ltq-adsl-ase kmod-ltq-adsl-ase-mei \
	kmod-ltq-adsl-ase-fw-b kmod-ltq-atm-ase \
	ltq-adsl-app ppp-mod-pppoe
endef
TARGET_DEVICES += allnet_all0333cj

define Device/netgear_dgn1000b
  IMAGE_SIZE := 6000k
  DEVICE_DTS := DGN1000B
  DEVICE_TITLE := Netgear DGN1000B
  DEVICE_PACKAGES := kmod-ltq-adsl-ase kmod-ltq-adsl-ase-mei \
	kmod-ltq-adsl-ase-fw-b kmod-ltq-atm-ase \
	ltq-adsl-app ppp-mod-pppoe
  SUPPORTED_DEVICES += DGN1000B
endef
TARGET_DEVICES += netgear_dgn1000b

endif

ifeq ($(SUBTARGET),xway_legacy)

define Device/arcadyan_arv4520pw
  IMAGE_SIZE := 3648k
  DEVICE_DTS := ARV4520PW
  DEVICE_TITLE := Easybox 800, WAV-281 - ARV4520PW
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-rt61-pci wpad-mini
  SUPPORTED_DEVICES += ARV4520PW
endef
TARGET_DEVICES += arcadyan_arv4520pw

define Device/arcadyan_arv4525pw
  IMAGE_SIZE := 3776k
  DEVICE_DTS := ARV4525PW
  DEVICE_TITLE := Speedport W502V Typ A - ARV4525PW
  DEVICE_PACKAGES := kmod-ath5k wpad-mini \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa -swconfig
  SUPPORTED_DEVICES += ARV4525PW
endef
TARGET_DEVICES += arcadyan_arv4525pw

define Device/arcadyan_arv452cqw
  IMAGE_SIZE := 3776k
  DEVICE_DTS := ARV452CQW
  DEVICE_TITLE := Easybox 801 - ARV452CQW
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ath5k wpad-mini \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa
  SUPPORTED_DEVICES += ARV452CQW
endef
TARGET_DEVICES += arcadyan_arv452cqw

define Device/arcadyan_arv4518pwr01
  IMAGE_SIZE := 3776k
  DEVICE_DTS := ARV4518PWR01
  DEVICE_TITLE := ARV4518PWR01
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath5k wpad-mini
  SUPPORTED_DEVICES += ARV4518PWR01
endef
TARGET_DEVICES += arcadyan_arv4518pwr01

define Device/arcadyan_arv4518pwr01a
  IMAGE_SIZE := 3776k
  DEVICE_DTS := ARV4518PWR01A
  DEVICE_TITLE := ARV4518PWR01A
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath5k wpad-mini
  SUPPORTED_DEVICES += ARV4518PWR01A
endef
TARGET_DEVICES += arcadyan_arv4518pwr01a

endif

ifeq ($(SUBTARGET),xway)

# Danube

define Device/bt_homehub-v2b
  $(Device/NAND)
  BOARD_NAME := BTHOMEHUBV2B
  DEVICE_DTS := BTHOMEHUBV2B
  DEVICE_TITLE := BT Home Hub 2B
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	kmod-ltq-deu-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini
  SUPPORTED_DEVICES += BTHOMEHUBV2B
endef
TARGET_DEVICES += bt_homehub-v2b

define Device/lantiq_easy50712
  DEVICE_DTS := EASY50712
  IMAGE_SIZE := 3776k
  DEVICE_TITLE := Lantiq Danube - EASY50712
endef
TARGET_DEVICES += lantiq_easy50712

define Device/audiocodes_mp-252
  IMAGE_SIZE := 14848k
  DEVICE_DTS := ACMP252
  DEVICE_TITLE := AudioCodes MediaPack MP-252
  DEVICE_PACKAGES :=  kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-usb-ledtrig-usbport kmod-usb-dwc2 \
	kmod-rt2800-pci \
	ltq-adsl-app ppp-mod-pppoa \
	wpad-mini
  SUPPORTED_DEVICES += ACMP252
endef
TARGET_DEVICES += audiocodes_mp-252

define Device/arcadyan_arv4510pw
  IMAGE_SIZE := 15616k
  DEVICE_DTS := ARV4510PW
  DEVICE_TITLE := Wippies, Elisa - ARV4510PW
  DEVICE_PACKAGES := kmod-usb-ledtrig-usbport kmod-usb2-pci kmod-usb-uhci \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci kmod-ath5k wpad-mini
  SUPPORTED_DEVICES += ARV4510PW
endef
TARGET_DEVICES += arcadyan_arv4510pw

define Device/arcadyan_arv7525pw
  IMAGE_SIZE := 3776k
  DEVICE_DTS := ARV4510PW
  DEVICE_TITLE := Speedport W303V Typ A - ARV7525PW
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa -swconfig
  SUPPORTED_DEVICES += ARV4510PW
endef
TARGET_DEVICES += arcadyan_arv7525pw

define Device/arcadyan_arv4519pw
  IMAGE_SIZE := 3776k
  DEVICE_DTS := ARV4519PW
  DEVICE_TITLE := Vodafone, Pirelli - ARV4519PW
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa
  SUPPORTED_DEVICES += ARV4519PW
endef
TARGET_DEVICES += arcadyan_arv4519pw

define Device/arcadyan_arv7506pw11
  IMAGE_SIZE := 7808k
  DEVICE_DTS := ARV7506PW11
  DEVICE_TITLE := Alice/O2 IAD 4421 - ARV7506PW11
  DEVICE_PACKAGES := kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-rt2800-pci wpad-mini
  SUPPORTED_DEVICES += ARV7506PW11
endef
TARGET_DEVICES += arcadyan_arv7506pw11

define Device/arcadyan_arv7510pw22
  IMAGE_SIZE := 31232k
  DEVICE_DTS := ARV7510PW22
  DEVICE_TITLE := Astoria - ARV7510PW22
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci wpad-mini \
	kmod-usb-uhci kmod-usb2 kmod-usb2-pci
  SUPPORTED_DEVICES += ARV7510PW22
endef
TARGET_DEVICES += arcadyan_arv7510pw22

define Device/arcadyan_arv7518pw
  IMAGE_SIZE := 7872k
  DEVICE_DTS := ARV7518PW
  DEVICE_TITLE := Astoria - ARV7518PW
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini
  SUPPORTED_DEVICES += ARV7518PW
endef
TARGET_DEVICES += arcadyan_arv7518pw

define Device/arcadyan_arv7519pw
  IMAGE_SIZE := 15488k
  DEVICE_DTS := ARV7519PW
  DEVICE_TITLE := Astoria - ARV7519PW
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-a kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-rt2800-pci wpad-mini
  SUPPORTED_DEVICES += ARV7519PW
endef
TARGET_DEVICES += arcadyan_arv7519pw

define Device/arcadyan_arv752dpw
  IMAGE_SIZE := 7872k
  DEVICE_DTS := ARV752DPW
  DEVICE_TITLE := Easybox 802 - ARV752DPW
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci wpad-mini
  SUPPORTED_DEVICES += ARV752DPW
endef
TARGET_DEVICES += arcadyan_arv752dpw

define Device/arcadyan_arv752dpw22
  IMAGE_SIZE := 7616k
  DEVICE_DTS := ARV752DPW22
  DEVICE_TITLE := Easybox 803 - ARV752DPW22
  DEVICE_PACKAGES := kmod-usb2-pci kmod-usb-uhci kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-tapi kmod-ltq-vmmc \
	kmod-rt2800-pci wpad-mini
  SUPPORTED_DEVICES += ARV752DPW22
endef
TARGET_DEVICES += arcadyan_arv752dpw22

define Device/arcadyan_arv8539pw22
  IMAGE_SIZE := 7616k
  DEVICE_DTS := ARV8539PW22
  DEVICE_TITLE := Speedport W504V Typ A - ARV8539PW22
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini
  SUPPORTED_DEVICES += ARV8539PW22
endef
TARGET_DEVICES += arcadyan_arv8539pw22

define Device/siemens_gigaset-sx76x
  IMAGE_SIZE := 7680k
  DEVICE_DTS := GIGASX76X
  DEVICE_TITLE := Gigaset sx76x
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-danube-mei kmod-ltq-adsl-danube \
	kmod-ltq-adsl-danube-fw-b kmod-ltq-atm-danube \
	ltq-adsl-app ppp-mod-pppoe \
	kmod-ath5k wpad-mini
  SUPPORTED_DEVICES += GIGASX76X
endef
TARGET_DEVICES += siemens_gigaset-sx76x


# AR9

define Device/bt_homehub-v3a
  $(Device/NAND)
  BOARD_NAME := BTHOMEHUBV3A
  DEVICE_DTS := BTHOMEHUBV3A
  DEVICE_TITLE := BT Home Hub 3A
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
	kmod-ltq-deu-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini \
	uboot-envtools
  SUPPORTED_DEVICES += BTHOMEHUBV3A
endef
TARGET_DEVICES += bt_homehub-v3a

DGN3500_KERNEL_OFFSET_HEX=0x50000
DGN3500_KERNEL_OFFSET_DEC=327680
define Device/netgear_dgn3500
  DEVICE_DTS := DGN3500
  IMAGE_SIZE := 16000k
  IMAGES := \
	sysupgrade-na.bin sysupgrade.bin \
	factory-na.img factory.img
  IMAGE/sysupgrade-na.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "NA" | \
	pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "WW" | \
	pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory-na.img := \
	pad-extra $(DGN3500_KERNEL_OFFSET_DEC) | append-kernel | append-rootfs | \
	dgn3500-sercom-footer $(DGN3500_KERNEL_OFFSET_HEX) "NA" | pad-rootfs | \
	check-size 16320k | pad-to 16384k
  IMAGE/factory.img := \
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
  SUPPORTED_DEVICES += DGN3500
endef
TARGET_DEVICES += netgear_dgn3500

define Device/netgear_dgn3500b
  DEVICE_DTS := DGN3500B
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
  SUPPORTED_DEVICES += DGN3500B
endef
TARGET_DEVICES += netgear_dgn3500b

define Device/buffalo_wbmr-hp-g300h-a
  IMAGE_SIZE := 31488k
  DEVICE_DTS := WBMR
  DEVICE_TITLE := Buffalo WBMR-HP-G300H (A) - WBMR
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini
  SUPPORTED_DEVICES := WBMR buffalo,wbmr-hp-g300h
endef
TARGET_DEVICES += buffalo_wbmr-hp-g300h-a

define Device/buffalo_wbmr-hp-g300h-b
  IMAGE_SIZE := 31488k
  DEVICE_DTS := WBMR
  DEVICE_TITLE := Buffalo WBMR-HP-G300H (B) - WBMR
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-mini
  SUPPORTED_DEVICES := WBMR buffalo,wbmr-hp-g300h
endef
TARGET_DEVICES += buffalo_wbmr-hp-g300h-b

define Device/avm_fritz7312
  $(Device/AVM)
  DEVICE_DTS := FRITZ7312
  IMAGE_SIZE := 15744k
  DEVICE_TITLE := 1&1 WLAN-MODEM - FRITZ7312
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-mini \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-deu-ar9 -swconfig
endef
TARGET_DEVICES += avm_fritz7312

define Device/avm_fritz7320
  $(Device/AVM)
  DEVICE_DTS := FRITZ7320
  IMAGE_SIZE := 15744k
  DEVICE_TITLE := 1&1 HomeServer - FRITZ7320
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-mini \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-deu-ar9 kmod-usb-dwc2 -swconfig
  SUPPORTED_DEVICES += FRITZ7320
endef
TARGET_DEVICES += avm_fritz7320

define Device/zte_h201l
  IMAGE_SIZE := 7808k
  DEVICE_DTS := H201L
  DEVICE_TITLE := ZTE H201L - H201L
  DEVICE_PACKAGES := kmod-ath9k-htc wpad-mini \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoe \
	kmod-ltq-deu-ar9 kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-tapi kmod-ltq-vmmc
  SUPPORTED_DEVICES += H201L
endef
TARGET_DEVICES += zte_h201l

define Device/zyxel_p-2601hn
  IMAGE_SIZE := 15616k
  DEVICE_DTS := P2601HNFX
  DEVICE_TITLE := ZyXEL P-2601HN-Fx
  DEVICE_PACKAGES := kmod-rt2800-usb wpad-mini \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoe \
	kmod-ltq-deu-ar9 kmod-usb-dwc2
  SUPPORTED_DEVICES += P2601HNFX
endef
TARGET_DEVICES += zyxel_p-2601hn


endif


ifeq ($(SUBTARGET),xrx200)

# VR9

define Device/zyxel_p-2812hnu-f1
  $(Device/NAND)
  BOARD_NAME := P2812HNUF1
  DEVICE_DTS := P2812HNUF1
  DEVICE_TITLE := ZyXEL P-2812HNU-F1
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2 kmod-usb-ledtrig-usbport
  SUPPORTED_DEVICES += P2812HNUF1
endef
TARGET_DEVICES += zyxel_p-2812hnu-f1

define Device/zyxel_p-2812hnu-f3
  $(Device/NAND)
  BOARD_NAME := P2812HNUF3
  DEVICE_DTS := P2812HNUF3
  DEVICE_TITLE := ZyXEL P-2812HNU-F3
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2
  SUPPORTED_DEVICES += P2812HNUF3
endef
TARGET_DEVICES += zyxel_p-2812hnu-f3

define Device/arcadyan_arv7519rw22
  IMAGE_SIZE := 31232k
  DEVICE_DTS := ARV7519RW22
  DEVICE_TITLE := Livebox Astoria ARV7519RW22
  DEVICE_PACKAGES := kmod-usb-dwc2
  SUPPORTED_DEVICES += ARV7519RW22
endef
TARGET_DEVICES += arcadyan_arv7519rw22

define Device/alphanetworks_asl56026
  DEVICE_DTS := ASL56026
  IMAGE_SIZE := 7488k
  DEVICE_TITLE := BT OpenReach - ECI VDSL Modem V-2FUb/I
endef
TARGET_DEVICES += alphanetworks_asl56026

define Device/bt_homehub-v5a
  $(Device/NAND)
  BOARD_NAME := BTHOMEHUBV5A
  DEVICE_DTS := BTHOMEHUBV5A
  DEVICE_TITLE := BT Home Hub 5A
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader \
	kmod-ath10k-ct ath10k-firmware-qca988x-ct wpad-mini kmod-usb-dwc2
  SUPPORTED_DEVICES += BTHOMEHUBV5A
endef
TARGET_DEVICES += bt_homehub-v5a

define Device/netgear_dm200
  DEVICE_DTS := DM200
  IMAGES := sysupgrade.bin factory.img
  IMAGE/sysupgrade.bin := append-kernel | \
	pad-offset 64k 64 | append-uImage-fakehdr filesystem | \
	pad-offset 64k 64 | append-uImage-fakehdr filesystem | \
	append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
  IMAGE/factory.img := $$(IMAGE/sysupgrade.bin) | netgear-dni
  IMAGE_SIZE := 7872k
  DEVICE_TITLE := Netgear DM200
  NETGEAR_BOARD_ID := DM200
  NETGEAR_HW_ID := 29765233+8+0+64+0+0
endef
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID
TARGET_DEVICES += netgear_dm200

define Device/lantiq_easy80920-nand
  $(Device/lantiqFullImage)
  DEVICE_DTS := EASY80920NAND
  IMAGE_SIZE := 64512k
  DEVICE_TITLE := Lantiq VR9 - EASY80920NAND
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-mini kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += lantiq_easy80920-nand

define Device/lantiq_easy80920-nor
  DEVICE_DTS := EASY80920NOR
  IMAGE_SIZE := 7936k
  DEVICE_TITLE := Lantiq VR9 - EASY80920NOR
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-mini kmod-usb-dwc2 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += lantiq_easy80920-nor

define Device/avm_fritz3370
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_DTS := FRITZ3370
  DEVICE_TITLE := AVM Fritz!Box WLan 3370 Rev. 2
  KERNEL_SIZE := 4096k
  UBINIZE_OPTS := -E 5
  IMAGES += eva-kernel.bin eva-filesystem.bin
  IMAGE/eva-kernel.bin := append-kernel
  IMAGE/eva-filesystem.bin := append-ubi
  DEVICE_PACKAGES := kmod-ath9k wpad-mini kmod-usb-dwc2 fritz-tffs
endef

define Device/avm_fritz3370-rev2-hynix
  $(Device/avm_fritz3370)
  DEVICE_DTS := FRITZ3370-REV2-HYNIX
  DEVICE_TITLE := AVM Fritz!Box WLan 3370 Rev. 2 (Hynix NAND)
endef
TARGET_DEVICES += avm_fritz3370-rev2-hynix

define Device/avm_fritz3370-rev2-micron
  $(Device/avm_fritz3370)
  DEVICE_DTS := FRITZ3370-REV2-MICRON
  DEVICE_TITLE := AVM Fritz!Box WLan 3370 Rev. 2 (Micron NAND)
endef
TARGET_DEVICES += avm_fritz3370-rev2-micron

define Device/avm_fritz7360sl
  $(Device/AVM)
  IMAGE_SIZE := 15744k
  DEVICE_DTS := FRITZ7360SL
  DEVICE_TITLE := 1&1 HomeServer - FRITZ7360SL
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-mini kmod-usb-dwc2
  SUPPORTED_DEVICES += FRITZ7360SL
endef
TARGET_DEVICES += avm_fritz7360sl

define Device/arcadyan_vg3503j
  IMAGE_SIZE := 8000k
  DEVICE_DTS := VG3503J
  DEVICE_TITLE := BT OpenReach - ECI VDSL Modem V-2FUb/R
  SUPPORTED_DEVICES += VG3503J
endef
TARGET_DEVICES += arcadyan_vg3503j

include tp-link.mk

define Device/buffalo_wbmr-300hpd
  IMAGE_SIZE := 15616k
  DEVICE_DTS := WBMR300
  DEVICE_TITLE := Buffalo WBMR-300HPD
  DEVICE_PACKAGES := kmod-mt7603 wpad-mini kmod-usb-dwc2
  SUPPORTED_DEVICES += WBMR300
endef
TARGET_DEVICES += buffalo_wbmr-300hpd

define Device/arcadyan_vgv7510kw22-nor
  IMAGE_SIZE := 15232k
  DEVICE_DTS := VGV7510KW22NOR
  DEVICE_TITLE := o2 Box 6431 / Arcadyan VGV7510KW22 (NOR)
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2 kmod-ltq-tapi kmod-ltq-vmmc
  SUPPORTED_DEVICES += VGV7510KW22NOR
endef
TARGET_DEVICES += arcadyan_vgv7510kw22-nor

define Device/arcadyan_vgv7510kw22-brn
  $(Device/lantiqBrnImage)
  IMAGE_SIZE := 7168k
  DEVICE_DTS := VGV7510KW22BRN
  SIGNATURE := BRNDA6431
  MAGIC := 0x12345678
  CRC32_POLY := 0x04c11db7
  DEVICE_TITLE := o2 Box 6431 / Arcadyan VGV7510KW22 (BRN)
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2 kmod-ltq-tapi kmod-ltq-vmmc
  SUPPORTED_DEVICES += VGV7510KW22BRN
endef
TARGET_DEVICES += arcadyan_vgv7510kw22-brn

define Device/arcadyan_vgv7519-nor
  IMAGE_SIZE := 15360k
  DEVICE_DTS := VGV7519NOR
  DEVICE_TITLE := Experiabox 8 VGV7519
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2 kmod-ltq-tapi kmod-ltq-vmmc
  SUPPORTED_DEVICES += VGV7519NOR
endef
TARGET_DEVICES += arcadyan_vgv7519-nor

define Device/arcadyan_vgv7519-brn
  $(Device/lantiqBrnImage)
  IMAGE_SIZE := 7168k
  DEVICE_DTS := VGV7519BRN
  SIGNATURE := 5D00008000
  MAGIC := 0x12345678
  CRC32_POLY := 0x2083b8ed
  DEVICE_TITLE := Experiabox 8 VGV7519 (BRN)
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-mini kmod-usb-dwc2 kmod-ltq-tapi kmod-ltq-vmmc
  SUPPORTED_DEVICES += VGV7519BRN
endef
TARGET_DEVICES += arcadyan_vgv7519-brn

endif


ifeq ($(SUBTARGET),falcon)

define Device/lantiq_easy98000-nor
  IMAGE_SIZE := 3904k
  DEVICE_DTS := EASY98000NOR
  DEVICE_TITLE := EASY98000 - Lantiq Falcon Eval Board NOR
  DEVICE_PACKAGES := kmod-dm9000 kmod-i2c-lantiq kmod-eeprom-at24
endef
TARGET_DEVICES += lantiq_easy98000-nor

define Device/lantiq_easy98000-nand
  IMAGE_SIZE := 3904k
  DEVICE_DTS := EASY98000NAND
  DEVICE_TITLE := EASY98000 - Lantiq Falcon Eval Board NAND
  DEVICE_PACKAGES := kmod-dm9000 kmod-i2c-lantiq kmod-eeprom-at24
endef
TARGET_DEVICES += lantiq_easy98000-nand

define Device/lantiq_easy98000-sflash
  IMAGE_SIZE := 7424k
  DEVICE_DTS := EASY98000SFLASH
  DEVICE_TITLE := EASY98000 - Lantiq Falcon Eval Board SFLASH
  DEVICE_PACKAGES := kmod-dm9000 kmod-i2c-lantiq kmod-eeprom-at24
endef
TARGET_DEVICES += lantiq_easy98000-sflash

define Device/lantiq_falcon-mdu
  IMAGE_SIZE := 7424k
  DEVICE_DTS := FALCON-MDU
  DEVICE_TITLE := MDU - Lantiq Falcon / VINAXdp MDU Board
endef
TARGET_DEVICES += lantiq_falcon-mdu

define Device/lantiq_easy88388
  IMAGE_SIZE := 7424k
  DEVICE_DTS := EASY88388
  DEVICE_TITLE := EASY88388 - Lantiq Falcon FTTDP8 Reference Board
endef
TARGET_DEVICES += lantiq_easy88388

define Device/lantiq_easy88444
  IMAGE_SIZE := 7424k
  DEVICE_DTS := EASY88444
  DEVICE_TITLE := EASY88444 - Lantiq Falcon FTTdp G.FAST Reference Board
endef
TARGET_DEVICES += lantiq_easy88444

define Device/lantiq_falcon-sfp
  IMAGE_SIZE := 7424k
  DEVICE_DTS := FALCON-SFP
  DEVICE_TITLE := SFP - Lantiq Falcon SFP Stick
endef
TARGET_DEVICES += lantiq_falcon-sfp

define Device/lantiq_easy98035synce
  IMAGE_SIZE := 7424k
  DEVICE_DTS := EASY98035SYNCE
  DEVICE_TITLE := EASY98035SYNCE - Lantiq Falcon SFP Stick with Synchronous Ethernet
endef
TARGET_DEVICES += lantiq_easy98035synce

define Device/lantiq_easy98035synce1588
  IMAGE_SIZE := 7424k
  DEVICE_DTS := EASY98035SYNCE1588
  DEVICE_TITLE := EASY98035SYNCE1588 - Lantiq Falcon SFP Stick with SyncE and IEEE1588
endef
TARGET_DEVICES += lantiq_easy98035synce1588

define Device/lantiq_easy98020
  IMAGE_SIZE := 7424k
  DEVICE_DTS := EASY98020
  DEVICE_TITLE := EASY98020 - Lantiq Falcon SFU Reference Board
endef
TARGET_DEVICES += lantiq_easy98020

define Device/lantiq_easy98020-v18
  IMAGE_SIZE := 7424k
  DEVICE_DTS := EASY98020V18
  DEVICE_TITLE := EASY98020V18 - Lantiq Falcon SFU Reference Board V1.8
endef
TARGET_DEVICES += lantiq_easy98020-v18

define Device/lantiq_easy98021
  IMAGE_SIZE := 7424k
  DEVICE_DTS := EASY98021
  DEVICE_TITLE := EASY98021 - Lantiq Falcon HGU Reference Board
endef
TARGET_DEVICES += lantiq_easy98021

endif

$(eval $(call BuildImage))
