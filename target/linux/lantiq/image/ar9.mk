define Device/avm_fritz7312
  $(Device/AVM_preloader)
  DEVICE_MODEL := FRITZ!Box 7312
  SOC := ar9
  IMAGE_SIZE := 15744k
  LOADER_FLASH_OFFS := 0x31000
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-deu-ar9 fritz-tffs -swconfig
endef
TARGET_DEVICES += avm_fritz7312

define Device/avm_fritz7320
  $(Device/AVM_preloader)
  DEVICE_MODEL := FRITZ!Box 7320
  DEVICE_ALT0_VENDOR := 1&1
  DEVICE_ALT0_MODEL := HomeServer
  DEVICE_ALT1_VENDOR := AVM
  DEVICE_ALT1_MODEL := Fritz!Box 7330
  SOC := ar9
  IMAGE_SIZE := 15744k
  LOADER_FLASH_OFFS := 0x31000
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ltq-deu-ar9 kmod-usb-dwc2 fritz-tffs -swconfig
  SUPPORTED_DEVICES += FRITZ7320
endef
TARGET_DEVICES += avm_fritz7320

define Device/bt_homehub-v3a
  $(Device/NAND)
  DEVICE_VENDOR := British Telecom (BT)
  DEVICE_MODEL := Home Hub 3
  DEVICE_VARIANT := Type A
  BOARD_NAME := BTHOMEHUBV3A
  SOC := ar9
  KERNEL_SIZE := 2048k
  DEVICE_PACKAGES := kmod-usb-dwc2 \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
	kmod-ltq-deu-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	uboot-envtools
  SUPPORTED_DEVICES += BTHOMEHUBV3A
  DEFAULT := n
endef
TARGET_DEVICES += bt_homehub-v3a

define Device/buffalo_wbmr-hp-g300h-a
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WBMR-HP-G300H
  DEVICE_VARIANT := A
  IMAGE_SIZE := 31488k
  SOC := ar9
  DEVICE_DTS := ar9_buffalo_wbmr-hp-g300h
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-basic-mbedtls
  SUPPORTED_DEVICES := WBMR buffalo,wbmr-hp-g300h
endef
TARGET_DEVICES += buffalo_wbmr-hp-g300h-a

define Device/buffalo_wbmr-hp-g300h-b
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WBMR-HP-G300H
  DEVICE_VARIANT := B
  IMAGE_SIZE := 31488k
  SOC := ar9
  DEVICE_DTS := ar9_buffalo_wbmr-hp-g300h
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	ltq-adsl-app ppp-mod-pppoa \
	kmod-ath9k kmod-owl-loader wpad-basic-mbedtls
  SUPPORTED_DEVICES := WBMR buffalo,wbmr-hp-g300h
endef
TARGET_DEVICES += buffalo_wbmr-hp-g300h-b

DGN3500_KERNEL_OFFSET_HEX=0x50000
DGN3500_KERNEL_OFFSET_DEC=327680
define Device/netgear_dgn3500
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := DGN3500
  SOC := ar9
  IMAGE_SIZE := 16000k
  KERNEL := kernel-bin | append-dtb | lzma | loader-kernel | uImage none
  KERNEL_INITRAMFS := $$(KERNEL)
  IMAGES := \
	sysupgrade-na.bin sysupgrade.bin \
	factory-na.img factory.img
  IMAGE/sysupgrade-na.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "NA" | \
	pad-rootfs | check-size | append-metadata
  IMAGE/sysupgrade.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "WW" | \
	pad-rootfs | check-size | append-metadata
  IMAGE/factory-na.img := \
	pad-extra $(DGN3500_KERNEL_OFFSET_DEC) | append-kernel | append-rootfs | \
	dgn3500-sercom-footer $(DGN3500_KERNEL_OFFSET_HEX) "NA" | pad-rootfs | \
	check-size 16320k | pad-to 16384k
  IMAGE/factory.img := \
	pad-extra $(DGN3500_KERNEL_OFFSET_DEC) | append-kernel | append-rootfs | \
	dgn3500-sercom-footer $(DGN3500_KERNEL_OFFSET_HEX) "WW" | pad-rootfs | \
	check-size 16320k | pad-to 16384k
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-a kmod-ltq-atm-ar9 \
	kmod-ltq-deu-ar9 ltq-adsl-app ppp-mod-pppoa
  SUPPORTED_DEVICES += DGN3500
endef
TARGET_DEVICES += netgear_dgn3500

define Device/netgear_dgn3500b
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := DGN3500B
  SOC := ar9
  IMAGE_SIZE := 16000k
  KERNEL := kernel-bin | append-dtb | lzma | loader-kernel | uImage none
  KERNEL_INITRAMFS := $$(KERNEL)
  IMAGES += factory.img
  IMAGE/sysupgrade.bin := \
	append-kernel | append-rootfs | dgn3500-sercom-footer 0x0 "DE" | \
	pad-rootfs | check-size | append-metadata
  IMAGE/factory.img := \
	pad-extra $(DGN3500_KERNEL_OFFSET_DEC) | append-kernel | append-rootfs | \
	dgn3500-sercom-footer $(DGN3500_KERNEL_OFFSET_HEX) "DE" | pad-rootfs | \
	check-size 16320k | pad-to 16384k
  DEVICE_PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	kmod-ltq-deu-ar9 ltq-adsl-app ppp-mod-pppoa
  SUPPORTED_DEVICES += DGN3500B
endef
TARGET_DEVICES += netgear_dgn3500b

define Device/zte_h201l
  DEVICE_VENDOR := ZTE
  DEVICE_MODEL := H201L
  IMAGE_SIZE := 7808k
  SOC := ar9
  DEVICE_PACKAGES := kmod-ath9k-htc wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	kmod-ltq-deu-ar9 ltq-adsl-app ppp-mod-pppoe \
	kmod-usb-dwc2 kmod-usb-ledtrig-usbport \
	kmod-ltq-tapi kmod-ltq-vmmc
  SUPPORTED_DEVICES += H201L
  DEFAULT := n
endef
TARGET_DEVICES += zte_h201l

define Device/zyxel_p-2601hn
  DEVICE_VENDOR := Zyxel
  DEVICE_MODEL := P-2601HN
  DEVICE_VARIANT := F1/F3
  IMAGE_SIZE := 15616k
  SOC := ar9
  DEVICE_PACKAGES := kmod-rt2800-usb wpad-basic-mbedtls \
	kmod-ltq-adsl-ar9-mei kmod-ltq-adsl-ar9 \
	kmod-ltq-adsl-ar9-fw-b kmod-ltq-atm-ar9 \
	kmod-ltq-deu-ar9 ltq-adsl-app ppp-mod-pppoe \
	kmod-usb-dwc2
  SUPPORTED_DEVICES += P2601HNFX
endef
TARGET_DEVICES += zyxel_p-2601hn
