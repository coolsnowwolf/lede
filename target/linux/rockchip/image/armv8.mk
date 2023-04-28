# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2020 Tobias Maedel

define Device/ariaboard_photonicat
  DEVICE_VENDOR := Ariaboard
  DEVICE_MODEL := Photonicat
  SOC := rk3568
  UBOOT_DEVICE_NAME := photonicat-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r5s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := ath10k-firmware-qca9377-sdio kmod-ath10k kmod-ath10k-sdio pcat-manager
endef
TARGET_DEVICES += ariaboard_photonicat

define Device/dilusense_dlfr100
  DEVICE_VENDOR := Dilusense
  DEVICE_MODEL := DLFR100
  SOC := rk3399
  UBOOT_DEVICE_NAME := dilusense-dlfr100-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += dilusense_dlfr100

define Device/ezpro_mrkaio-m68s
  DEVICE_VENDOR := EZPRO
  DEVICE_MODEL := Mrkaio M68S
  SOC := rk3568
  UBOOT_DEVICE_NAME := mrkaio-m68s-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r5s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-ata-ahci kmod-ata-ahci-platform
endef
TARGET_DEVICES += ezpro_mrkaio-m68s

define Device/ezpro_mrkaio-m68s-plus
  DEVICE_VENDOR := EZPRO
  DEVICE_MODEL := Mrkaio M68S PLUS
  SOC := rk3568
  UBOOT_DEVICE_NAME := mrkaio-m68s-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r5s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8125 kmod-ata-ahci kmod-ata-ahci-platform kmod-nvme kmod-scsi-core
endef
TARGET_DEVICES += ezpro_mrkaio-m68s-plus

define Device/hinlink_common
  DEVICE_VENDOR := HINLINK
  UBOOT_DEVICE_NAME := opc-h68k-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r5s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-ata-ahci-platform kmod-mt7921e kmod-r8125 kmod-usb-serial-cp210x wpad-openssl
endef

define Device/hinlink_opc-h66k
$(call Device/hinlink_common)
  DEVICE_MODEL := OPC-H66K
  SOC := rk3568
endef
TARGET_DEVICES += hinlink_opc-h66k

define Device/hinlink_opc-h68k
$(call Device/hinlink_common)
  DEVICE_MODEL := OPC-H68K
  SOC := rk3568
endef
TARGET_DEVICES += hinlink_opc-h68k

define Device/hinlink_opc-h69k
$(call Device/hinlink_common)
  DEVICE_MODEL := OPC-H69K
  SOC := rk3568
  DEVICE_PACKAGES += kmod-usb-serial-option uqmi
endef
TARGET_DEVICES += hinlink_opc-h69k

define Device/fastrhino_common
  DEVICE_VENDOR := FastRhino
  SOC := rk3568
  UBOOT_DEVICE_NAME := r66s-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r5s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8125
endef

define Device/fastrhino_r66s
$(call Device/fastrhino_common)
  DEVICE_MODEL := R66S
endef
TARGET_DEVICES += fastrhino_r66s

define Device/fastrhino_r68s
$(call Device/fastrhino_common)
  DEVICE_MODEL := R68S
endef
TARGET_DEVICES += fastrhino_r68s

define Device/friendlyarm_nanopi-neo3
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi NEO3
  SOC := rk3328
  UBOOT_DEVICE_NAME := nanopi-r2s-rk3328
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r2s | pine64-bin | gzip | append-metadata
endef
TARGET_DEVICES += friendlyarm_nanopi-neo3

define Device/friendlyarm_nanopi-r2c
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R2C
  SOC := rk3328
  UBOOT_DEVICE_NAME := nanopi-r2c-rk3328
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r2s | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += friendlyarm_nanopi-r2c

define Device/friendlyarm_nanopi-r2s
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R2S
  SOC := rk3328
  UBOOT_DEVICE_NAME := nanopi-r2s-rk3328
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r2s | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += friendlyarm_nanopi-r2s

define Device/friendlyarm_nanopi-r4s
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R4S
  SOC := rk3399
  UBOOT_DEVICE_NAME := nanopi-r4s-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r4s | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += friendlyarm_nanopi-r4s

define Device/friendlyarm_nanopi-r4se
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R4SE
  SOC := rk3399
  UBOOT_DEVICE_NAME := nanopi-r4se-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r4s | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += friendlyarm_nanopi-r4se

define Device/friendlyarm_nanopi-r5c
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R5C
  SOC := rk3568
  UBOOT_DEVICE_NAME := nanopi-r5s-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r5s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8125 kmod-nvme kmod-scsi-core
endef
TARGET_DEVICES += friendlyarm_nanopi-r5c

define Device/friendlyarm_nanopi-r5s
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R5S
  SOC := rk3568
  UBOOT_DEVICE_NAME := nanopi-r5s-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r5s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8125 kmod-nvme kmod-scsi-core
endef
TARGET_DEVICES += friendlyarm_nanopi-r5s

define Device/firefly_station-p2
  DEVICE_VENDOR := Firefly
  DEVICE_MODEL := Station P2
  DEVICE_DTS := rockchip/rk3568-roc-pc
  UBOOT_DEVICE_NAME := station-p2-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r5s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-brcmfmac kmod-ikconfig kmod-ata-ahci-platform station-p2-firmware wpad-openssl
endef
TARGET_DEVICES += firefly_station-p2

define Device/pine64_rockpro64
  DEVICE_VENDOR := Pine64
  DEVICE_MODEL := RockPro64
  SOC := rk3399
  UBOOT_DEVICE_NAME := rockpro64-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := -urngd
endef
TARGET_DEVICES += pine64_rockpro64

define Device/radxa_e25
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := E25
  DEVICE_DTS := rockchip/rk3568-radxa-e25
  UBOOT_DEVICE_NAME := radxa-e25-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r5s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-ata-ahci-platform kmod-r8125
endef
TARGET_DEVICES += radxa_e25

define Device/radxa_rock-3a
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK3 A
  SOC := rk3568
  SUPPORTED_DEVICES := radxa,rock3a
  UBOOT_DEVICE_NAME := rock-3a-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r5s | pine64-img | gzip | append-metadata
endef
TARGET_DEVICES += radxa_rock-3a

define Device/radxa_rock-pi-4
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK Pi 4
  SOC := rk3399
  SUPPORTED_DEVICES := radxa,rockpi4
  UBOOT_DEVICE_NAME := rock-pi-4-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := -urngd
endef
TARGET_DEVICES += radxa_rock-pi-4

define Device/rongpin_king3399
  DEVICE_VENDOR := Rongpin
  DEVICE_MODEL := King3399
  SOC := rk3399
  UBOOT_DEVICE_NAME := rongpin-king3399-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r4s | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd kmod-brcmfmac cypress-firmware-4356-sdio rongpin-king3399-firmware wpad-openssl
endef
TARGET_DEVICES += rongpin_king3399

define Device/rocktech_mpc1903
  DEVICE_VENDOR := Rocktech
  DEVICE_MODEL := MPC1903
  SOC := rk3399
  SUPPORTED_DEVICES := rocktech,mpc1903
  UBOOT_DEVICE_NAME := rocktech-mpc1903-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net-smsc75xx kmod-usb-serial-cp210x -urngd
endef
TARGET_DEVICES += rocktech_mpc1903

define Device/sharevdi_h3399pc
  DEVICE_VENDOR := SHAREVDI
  DEVICE_MODEL := H3399PC
  SOC := rk3399
  UBOOT_DEVICE_NAME := sharevdi-h3399pc-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r4s | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += sharevdi_h3399pc

define Device/sharevdi_guangmiao-g4c
  DEVICE_VENDOR := SHAREVDI
  DEVICE_MODEL := GuangMiao G4C
  SOC := rk3399
  UBOOT_DEVICE_NAME := guangmiao-g4c-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r4s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += sharevdi_guangmiao-g4c

define Device/xunlong_orangepi-r1-plus
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi R1 Plus
  SOC := rk3328
  UBOOT_DEVICE_NAME := orangepi-r1-plus-rk3328
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r2s | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += xunlong_orangepi-r1-plus

define Device/xunlong_orangepi-r1-plus-lts
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi R1 Plus LTS
  SOC := rk3328
  UBOOT_DEVICE_NAME := orangepi-r1-plus-lts-rk3328
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r2s | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += xunlong_orangepi-r1-plus-lts
