DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID

define Device/dsa-migration
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_COMPAT_MESSAGE := Config cannot be migrated from swconfig to DSA
endef

define Device/arcadyan_arv7519rw22
  $(Device/dsa-migration)
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := ARV7519RW22
  DEVICE_ALT0_VENDOR := Orange
  DEVICE_ALT0_MODEL := Livebox
  DEVICE_ALT0_VARIANT := 2.1
  DEVICE_ALT1_VENDOR := Astoria Networks
  DEVICE_ALT1_MODEL := ARV7519RW22
  KERNEL_SIZE := 2048k
  IMAGE_SIZE := 31232k
  DEVICE_PACKAGES := kmod-usb-dwc2 xrx200-rev1.1-phy22f-firmware xrx200-rev1.2-phy22f-firmware
  SUPPORTED_DEVICES += ARV7519RW22
  DEFAULT := n
endef
TARGET_DEVICES += arcadyan_arv7519rw22

define Device/arcadyan_vgv7510kw22-brn
  $(Device/dsa-migration)
  $(Device/lantiqBrnImage)
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := VGV7510KW22
  DEVICE_VARIANT := BRN
  DEVICE_ALT0_VENDOR := o2
  DEVICE_ALT0_MODEL := Box 6431
  DEVICE_ALT0_VARIANT := BRN
  IMAGE_SIZE := 7168k
  SIGNATURE := BRNDA6431
  MAGIC := 0x12345678
  CRC32_POLY := 0x04c11db7
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-basic-mbedtls kmod-usb-dwc2 kmod-ltq-tapi \
	kmod-ltq-vmmc xrx200-rev1.1-phy22f-firmware xrx200-rev1.2-phy22f-firmware
  SUPPORTED_DEVICES += VGV7510KW22BRN
endef
TARGET_DEVICES += arcadyan_vgv7510kw22-brn

define Device/arcadyan_vgv7510kw22-nor
  $(Device/dsa-migration)
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := VGV7510KW22
  DEVICE_VARIANT := NOR
  DEVICE_ALT0_VENDOR := o2
  DEVICE_ALT0_MODEL := Box 6431
  DEVICE_ALT0_VARIANT := NOR
  IMAGE_SIZE := 15232k
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-basic-mbedtls kmod-usb-dwc2 kmod-ltq-tapi \
	kmod-ltq-vmmc xrx200-rev1.1-phy22f-firmware xrx200-rev1.2-phy22f-firmware
  SUPPORTED_DEVICES += VGV7510KW22NOR
endef
TARGET_DEVICES += arcadyan_vgv7510kw22-nor

define Device/arcadyan_vgv7519-brn
  $(Device/dsa-migration)
  $(Device/lantiqBrnImage)
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := VGV7519
  DEVICE_VARIANT := BRN
  DEVICE_ALT0_VENDOR := KPN
  DEVICE_ALT0_MODEL := Experiabox 8
  DEVICE_ALT0_VARIANT := BRN
  IMAGE_SIZE := 7168k
  SIGNATURE := 5D00008000
  MAGIC := 0x12345678
  CRC32_POLY := 0x2083b8ed
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-basic-mbedtls kmod-usb-dwc2 kmod-ltq-tapi \
	kmod-ltq-vmmc xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware
  SUPPORTED_DEVICES += VGV7519BRN
endef
TARGET_DEVICES += arcadyan_vgv7519-brn

define Device/arcadyan_vgv7519-nor
  $(Device/dsa-migration)
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := VGV7519
  DEVICE_VARIANT := NOR
  DEVICE_ALT0_VENDOR := KPN
  DEVICE_ALT0_MODEL := Experiabox 8
  DEVICE_ALT0_VARIANT := NOR
  IMAGE_SIZE := 15360k
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-basic-mbedtls kmod-usb-dwc2 kmod-ltq-tapi \
	kmod-ltq-vmmc xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware
  SUPPORTED_DEVICES += VGV7519NOR
endef
TARGET_DEVICES += arcadyan_vgv7519-nor

define Device/arcadyan_vrv9510kwac23
  $(Device/dsa-migration)
  $(Device/NAND)
  DEVICE_VENDOR := Arcadyan
  DEVICE_MODEL := VRV9510KWAC23
  DEVICE_ALT0_VENDOR := Livebox
  DEVICE_ALT0_MODEL := Next
  BOARD_NAME := VRV9510KWAC23
  DEVICE_PACKAGES :=  kmod-b43 wpad-basic-mbedtls broadcom-43222-sprom \
	broadcom-4360-sprom kmod-usb-dwc2 kmod-ltq-tapi kmod-ltq-vmmc \
	xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware
  KERNEL_SIZE := 4096k
  SUPPORTED_DEVICES += arcadyan_vrv9510kwac23
endef
TARGET_DEVICES += arcadyan_vrv9510kwac23

define Device/avm_fritz3370
  $(Device/dsa-migration)
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_MODEL := FRITZ!Box 3370
  DEVICE_VARIANT := Rev. 2
  KERNEL_SIZE := 4096k
  UBINIZE_OPTS := -E 5
  IMAGES += eva-kernel.bin eva-filesystem.bin
  IMAGE/eva-kernel.bin := append-kernel
  IMAGE/eva-filesystem.bin := append-ubi
  DEVICE_PACKAGES := kmod-ath9k wpad-basic-mbedtls kmod-usb-dwc2 fritz-tffs \
	xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware
endef

define Device/avm_fritz3370-rev2-hynix
  $(Device/dsa-migration)
  $(Device/avm_fritz3370)
  DEVICE_MODEL := FRITZ!Box 3370
  DEVICE_VARIANT := Rev. 2 (Hynix NAND)
endef
TARGET_DEVICES += avm_fritz3370-rev2-hynix

define Device/avm_fritz3370-rev2-micron
  $(Device/dsa-migration)
  $(Device/avm_fritz3370)
  DEVICE_MODEL := FRITZ!Box 3370
  DEVICE_VARIANT := Rev. 2 (Micron NAND)
endef
TARGET_DEVICES += avm_fritz3370-rev2-micron

define Device/avm_fritz3390
  $(Device/dsa-migration)
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_MODEL := FRITZ!Box 3390
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 49152k
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-usb-dwc2 fritz-tffs xrx200-rev1.1-phy11g-firmware \
	xrx200-rev1.2-phy11g-firmware
endef
TARGET_DEVICES += avm_fritz3390

define Device/avm_fritz3490
  $(Device/dsa-migration)
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_MODEL := FRITZ!Box 3490
  DEVICE_VARIANT := Other NAND
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 49152k
  DEVICE_PACKAGES := kmod-usb-xhci-pci-renesas fritz-tffs \
	xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware -kmod-owl-loader
endef
TARGET_DEVICES += avm_fritz3490

define Device/avm_fritz3490-micron
  $(Device/dsa-migration)
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_MODEL := FRITZ!Box 3490
  DEVICE_VARIANT := Micron NAND
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 49152k
  DEVICE_PACKAGES := kmod-usb-xhci-pci-renesas fritz-tffs \
	xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware -kmod-owl-loader
endef
TARGET_DEVICES += avm_fritz3490-micron

define Device/avm_fritz5490
  $(Device/dsa-migration)
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_MODEL := FRITZ!Box 5490
  DEVICE_ALT0_VENDOR := AVM
  DEVICE_ALT0_MODEL := FRITZ!Box 5491
  DEVICE_VARIANT := Other NAND
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 49152k
  DEVICE_PACKAGES := kmod-dsa-qca8k kmod-phy-qca83xx kmod-usb-xhci-pci-renesas \
	fritz-tffs xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware \
	-ltq-vdsl-vr9-vectoring-fw-installer -kmod-ltq-vdsl-vr9-mei \
	-kmod-ltq-vdsl-vr9 -kmod-ltq-atm-vr9 -kmod-ltq-ptm-vr9 \
	-ltq-vdsl-vr9-app -kmod-owl-loader \
	-dsl-vrx200-firmware-xdsl-a -dsl-vrx200-firmware-xdsl-b-patch
endef
TARGET_DEVICES += avm_fritz5490

define Device/avm_fritz5490-micron
  $(Device/dsa-migration)
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_MODEL := FRITZ!Box 5490
  DEVICE_ALT0_VENDOR := AVM
  DEVICE_ALT0_MODEL := FRITZ!Box 5491
  DEVICE_VARIANT := Micron NAND
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 49152k
  DEVICE_PACKAGES := kmod-dsa-qca8k kmod-phy-qca83xx kmod-usb-xhci-pci-renesas \
	fritz-tffs xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware \
	-ltq-vdsl-vr9-vectoring-fw-installer -kmod-ltq-vdsl-vr9-mei \
	-kmod-ltq-vdsl-vr9 -kmod-ltq-atm-vr9 -kmod-ltq-ptm-vr9 \
	-ltq-vdsl-vr9-app -kmod-owl-loader \
	-dsl-vrx200-firmware-xdsl-a -dsl-vrx200-firmware-xdsl-b-patch
endef
TARGET_DEVICES += avm_fritz5490-micron

define Device/avm_fritz7360sl
  $(Device/dsa-migration)
  $(Device/AVM)
  DEVICE_MODEL := FRITZ!Box 7360 SL
  IMAGE_SIZE := 15744k
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-usb-dwc2 fritz-tffs xrx200-rev1.1-phy11g-firmware \
	xrx200-rev1.2-phy11g-firmware
  SUPPORTED_DEVICES += FRITZ7360SL
endef
TARGET_DEVICES += avm_fritz7360sl

define Device/avm_fritz7360-v2
  $(Device/dsa-migration)
  $(Device/AVM)
  DEVICE_MODEL := FRITZ!Box 7360
  DEVICE_VARIANT := v2
  IMAGE_SIZE := 32128k
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-usb-dwc2 fritz-tffs xrx200-rev1.1-phy11g-firmware \
	xrx200-rev1.2-phy11g-firmware
endef
TARGET_DEVICES += avm_fritz7360-v2

define Device/avm_fritz7362sl
  $(Device/dsa-migration)
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_MODEL := FRITZ!Box 7362 SL
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 49152k
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-usb-dwc2 fritz-tffs xrx200-rev1.1-phy11g-firmware \
	xrx200-rev1.2-phy11g-firmware
endef
TARGET_DEVICES += avm_fritz7362sl

define Device/avm_fritz7412
  $(Device/dsa-migration)
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_MODEL := FRITZ!Box 7412
  BOARD_NAME := FRITZ7412
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 49152k
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	fritz-tffs-nand fritz-caldata xrx200-rev1.1-phy22f-firmware \
	xrx200-rev1.2-phy22f-firmware
endef
TARGET_DEVICES += avm_fritz7412

define Device/avm_fritz7430
  $(Device/dsa-migration)
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_MODEL := FRITZ!Box 7430
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 49152k
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
	kmod-usb-dwc2 fritz-tffs-nand fritz-caldata xrx200-rev1.1-phy22f-firmware \
	xrx200-rev1.2-phy22f-firmware
endef
TARGET_DEVICES += avm_fritz7430

define Device/avm_fritz7490
  $(Device/dsa-migration)
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_MODEL := FRITZ!Box 7490
  DEVICE_VARIANT := Other NAND
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 49152k
  DEVICE_PACKAGES := kmod-usb-xhci-pci-renesas fritz-tffs \
	xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware -kmod-owl-loader
endef
TARGET_DEVICES += avm_fritz7490

define Device/avm_fritz7490-micron
  $(Device/dsa-migration)
  $(Device/AVM)
  $(Device/NAND)
  DEVICE_MODEL := FRITZ!Box 7490
  DEVICE_VARIANT := Micron NAND
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 49152k
  DEVICE_PACKAGES := kmod-usb-xhci-pci-renesas fritz-tffs \
	xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware -kmod-owl-loader
endef
TARGET_DEVICES += avm_fritz7490-micron

define Device/bt_homehub-v5a
  $(Device/dsa-migration)
  $(Device/NAND)
  DEVICE_VENDOR := British Telecom (BT)
  DEVICE_MODEL := Home Hub 5
  DEVICE_VARIANT := Type A
  DEVICE_ALT0_VENDOR := British Telecom (BT)
  DEVICE_ALT0_MODEL := Business Hub 5
  DEVICE_ALT0_VARIANT := Type A
  DEVICE_ALT1_VENDOR := Plusnet
  DEVICE_ALT1_MODEL := Hub One
  BOARD_NAME := BTHOMEHUBV5A
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader  kmod-ath10k-ct \
	ath10k-firmware-qca988x-ct wpad-basic-mbedtls kmod-usb-dwc2 \
	xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware
  SUPPORTED_DEVICES += BTHOMEHUBV5A
endef
TARGET_DEVICES += bt_homehub-v5a

define Device/buffalo_wbmr-300hpd
  $(Device/dsa-migration)
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := WBMR-300HPD
  IMAGE_SIZE := 15616k
  DEVICE_PACKAGES := kmod-mt7603 wpad-basic-mbedtls kmod-usb-dwc2 \
	xrx200-rev1.1-phy22f-firmware xrx200-rev1.2-phy22f-firmware
  SUPPORTED_DEVICES += WBMR300
endef
TARGET_DEVICES += buffalo_wbmr-300hpd

define Device/lantiq_easy80920-nand
  $(Device/dsa-migration)
  $(Device/lantiqFullImage)
  DEVICE_VENDOR := Lantiq
  DEVICE_MODEL := VR9 EASY80920
  DEVICE_VARIANT := NAND
  IMAGE_SIZE := 64512k
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
  	kmod-usb-dwc2 kmod-usb-ledtrig-usbport xrx200-rev1.1-phy11g-firmware \
	xrx200-rev1.2-phy11g-firmware
endef
TARGET_DEVICES += lantiq_easy80920-nand

define Device/lantiq_easy80920-nor
  $(Device/dsa-migration)
  DEVICE_VENDOR := Lantiq
  DEVICE_MODEL := VR9 EASY80920
  DEVICE_VARIANT := NOR
  IMAGE_SIZE := 7936k
  DEVICE_PACKAGES := kmod-ath9k kmod-owl-loader wpad-basic-mbedtls \
  	kmod-usb-dwc2 kmod-usb-ledtrig-usbport xrx200-rev1.1-phy11g-firmware \
	xrx200-rev1.2-phy11g-firmware
endef
TARGET_DEVICES += lantiq_easy80920-nor

define Device/zyxel_p-2812hnu-f1
  $(Device/NAND)
  DEVICE_COMPAT_VERSION := 2.0
  DEVICE_COMPAT_MESSAGE := kernel and ubi partitions had to be resized. \
  Upgrade manually using initramfs, and change u-boot environment to load 5MiB for uImage.
  DEVICE_VENDOR := Zyxel
  DEVICE_MODEL := P-2812HNU
  DEVICE_VARIANT := F1
  BOARD_NAME := P2812HNUF1
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-basic-mbedtls kmod-usb-dwc2 \
	kmod-usb-ledtrig-usbport xrx200-rev1.1-phy11g-firmware \
	xrx200-rev1.2-phy11g-firmware
  KERNEL_SIZE := 5120k
  SUPPORTED_DEVICES += P2812HNUF1
endef
TARGET_DEVICES += zyxel_p-2812hnu-f1

define Device/zyxel_p-2812hnu-f3
  $(Device/dsa-migration)
  $(Device/NAND)
  DEVICE_VENDOR := Zyxel
  DEVICE_MODEL := P-2812HNU
  DEVICE_VARIANT := F3
  BOARD_NAME := P2812HNUF3
  DEVICE_PACKAGES := kmod-rt2800-pci wpad-basic-mbedtls kmod-usb-dwc2 \
	xrx200-rev1.1-phy11g-firmware xrx200-rev1.2-phy11g-firmware
  KERNEL_SIZE := 2048k
  SUPPORTED_DEVICES += P2812HNUF3
  DEFAULT := n
endef
TARGET_DEVICES += zyxel_p-2812hnu-f3
