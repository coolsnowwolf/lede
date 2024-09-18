# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2020 Tobias Maedel

define Device/advantech_rsb4810
  DEVICE_VENDOR := Advantech
  DEVICE_MODEL := RSB4810
  SOC := rk3568
  UBOOT_DEVICE_NAME := rsb4810-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-ata-ahci-dwc -urngd
endef
TARGET_DEVICES += advantech_rsb4810

define Device/ariaboard_photonicat
  DEVICE_VENDOR := Ariaboard
  DEVICE_MODEL := Photonicat
  SOC := rk3568
  UBOOT_DEVICE_NAME := photonicat-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script vop | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := photonicat-firmware kmod-drm-rockchip kmod-ath10k kmod-ath10k-sdio pcat-manager wpad
endef
TARGET_DEVICES += ariaboard_photonicat

define Device/armsom_sige1
  DEVICE_VENDOR := ArmSoM
  DEVICE_MODEL := Sige1
  DEVICE_DTS := rockchip/rk3528-armsom-sige1
  UBOOT_DEVICE_NAME := evb-rk3528
  IMAGE/sysupgrade.img.gz := boot-common | boot-script rk3528 | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := brcmfmac-firmware-43752-sdio kmod-brcmfmac kmod-r8125 wpad -urngd
endef
TARGET_DEVICES += armsom_sige1

define Device/codinge_xiaobao-nas-v1
  DEVICE_VENDOR := Codinge
  DEVICE_MODEL := XiaoBao NAS-I
  SOC := rk3399
  UBOOT_DEVICE_NAME := xiaobao-nas-v1-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-ata-ahci
endef
TARGET_DEVICES += codinge_xiaobao-nas-v1

define Device/dilusense_dlfr100
  DEVICE_VENDOR := Dilusense
  DEVICE_MODEL := DLFR100
  SOC := rk3399
  UBOOT_DEVICE_NAME := dlfr100-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += dilusense_dlfr100

define Device/ezpro_mrkaio-m68s
  DEVICE_VENDOR := EZPRO
  DEVICE_MODEL := Mrkaio M68S
  SOC := rk3568
  UBOOT_DEVICE_NAME := mrkaio-m68s-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script vop | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-ata-ahci-dwc kmod-drm-rockchip
endef
TARGET_DEVICES += ezpro_mrkaio-m68s

define Device/ezpro_mrkaio-m68s-plus
  DEVICE_VENDOR := EZPRO
  DEVICE_MODEL := Mrkaio M68S PLUS
  SOC := rk3568
  UBOOT_DEVICE_NAME := mrkaio-m68s-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script vop | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-ata-ahci-dwc kmod-drm-rockchip kmod-r8125-rss
endef
TARGET_DEVICES += ezpro_mrkaio-m68s-plus

define Device/fastrhino_common
  DEVICE_VENDOR := FastRhino
  SOC := rk3568
  UBOOT_DEVICE_NAME := r66s-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8125-rss
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
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-bin | gzip | append-metadata
endef
TARGET_DEVICES += friendlyarm_nanopi-neo3

define Device/friendlyarm_nanopi-r2c
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R2C
  SOC := rk3328
  UBOOT_DEVICE_NAME := nanopi-r2c-rk3328
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += friendlyarm_nanopi-r2c

define Device/friendlyarm_nanopi-r2s
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R2S
  SOC := rk3328
  UBOOT_DEVICE_NAME := nanopi-r2s-rk3328
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += friendlyarm_nanopi-r2s

define Device/friendlyarm_nanopi-r4s
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R4S
  SOC := rk3399
  UBOOT_DEVICE_NAME := nanopi-r4s-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += friendlyarm_nanopi-r4s

define Device/friendlyarm_nanopi-r4se
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R4SE
  SOC := rk3399
  UBOOT_DEVICE_NAME := nanopi-r4se-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += friendlyarm_nanopi-r4se

define Device/friendlyarm_nanopi-r5c
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R5C
  SOC := rk3568
  UBOOT_DEVICE_NAME := nanopi-r5s-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8125-rss
endef
TARGET_DEVICES += friendlyarm_nanopi-r5c

define Device/friendlyarm_nanopi-r5s
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R5S
  SOC := rk3568
  UBOOT_DEVICE_NAME := nanopi-r5s-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8125-rss
endef
TARGET_DEVICES += friendlyarm_nanopi-r5s

define Device/firefly_station-p2
  DEVICE_VENDOR := Firefly
  DEVICE_MODEL := Station P2
  DEVICE_DTS := rockchip/rk3568-roc-pc
  UBOOT_DEVICE_NAME := station-p2-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-ata-ahci-dwc kmod-brcmfmac kmod-ikconfig wpad-openssl
endef
TARGET_DEVICES += firefly_station-p2

define Device/hinlink_common
  DEVICE_VENDOR := HINLINK
  UBOOT_DEVICE_NAME := opc-h68k-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script vop | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-ata-ahci-dwc kmod-drm-rockchip kmod-hwmon-pwmfan kmod-mt7921e kmod-r8125-rss wpad-openssl
endef

define Device/hinlink_opc-h28k
  DEVICE_VENDOR := HINLINK
  DEVICE_MODEL := OPC-H28K
  SOC := rk3528
  UBOOT_DEVICE_NAME := evb-rk3528
  IMAGE/sysupgrade.img.gz := boot-common | boot-script rk3528 | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += hinlink_opc-h28k

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
  DEVICE_PACKAGES += kmod-mt7916-firmware kmod-usb-serial-option uqmi
endef
TARGET_DEVICES += hinlink_opc-h69k

define Device/hinlink_opc-ht2
  DEVICE_VENDOR := HINLINK
  DEVICE_MODEL := OPC-HT2
  SOC := rk3528
  UBOOT_DEVICE_NAME := evb-rk3528
  IMAGE/sysupgrade.img.gz := boot-common | boot-script rk3528 | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-aic8800 wpad-openssl -urngd
endef
TARGET_DEVICES += hinlink_opc-ht2

define Device/lyt_t68m
  DEVICE_VENDOR := LYT
  DEVICE_MODEL := T68M
  SOC := rk3568
  UBOOT_DEVICE_NAME := lyt-t68m-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script vop | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-drm-rockchip kmod-mt7921e kmod-r8125-rss wpad-openssl uboot-envtools
endef
TARGET_DEVICES += lyt_t68m

define Device/panther_x2
  DEVICE_VENDOR := Panther
  DEVICE_MODEL := X2
  DEVICE_DTS := rockchip/rk3566-panther-x2
  UBOOT_DEVICE_NAME := panther-x2-rk3566
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-brcmfmac kmod-ikconfig brcmfmac-nvram-43430-sdio wpad-openssl
endef
TARGET_DEVICES += panther_x2

define Device/pine64_rockpro64
  DEVICE_VENDOR := Pine64
  DEVICE_MODEL := RockPro64
  SOC := rk3399
  UBOOT_DEVICE_NAME := rockpro64-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := -urngd
endef
TARGET_DEVICES += pine64_rockpro64

define Device/radxa_e20c
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := E20C
  DEVICE_DTS := rockchip/rk3528-radxa-e20c
  UBOOT_DEVICE_NAME := evb-rk3528
  IMAGE/sysupgrade.img.gz := boot-common | boot-script rk3528 | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += radxa_e20c

define Device/radxa_e25
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := E25
  DEVICE_DTS := rockchip/rk3568-radxa-e25
  UBOOT_DEVICE_NAME := radxa-e25-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-ata-ahci-dwc kmod-r8125-rss
endef
TARGET_DEVICES += radxa_e25

define Device/radxa_rock-3a
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK 3A
  SOC := rk3568
  SUPPORTED_DEVICES := radxa,rock3a
  UBOOT_DEVICE_NAME := rock-3a-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
endef
TARGET_DEVICES += radxa_rock-3a

define Device/radxa_rock-3c
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK 3C
  SOC := rk3566
  UBOOT_DEVICE_NAME := rock-3c-rk3566
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-aic8800 wpad-openssl
endef
TARGET_DEVICES += radxa_rock-3c

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
  UBOOT_DEVICE_NAME := king3399-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd kmod-brcmfmac cypress-firmware-4356-sdio wpad-openssl
endef
TARGET_DEVICES += rongpin_king3399

define Device/rocktech_mpc1903
  DEVICE_VENDOR := Rocktech
  DEVICE_MODEL := MPC1903
  SOC := rk3399
  UBOOT_DEVICE_NAME := mpc1903-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net-smsc75xx kmod-usb-serial-cp210x -urngd
endef
TARGET_DEVICES += rocktech_mpc1903

define Device/scensmart_sv901-eaio
  DEVICE_VENDOR := ScenSmart
  DEVICE_MODEL := SV901 EAIO
  SOC := rk3399
  UBOOT_DEVICE_NAME := sv901-eaio-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := brcmfmac-nvram-4356-sdio cypress-firmware-4356-sdio kmod-brcmfmac \
	kmod-switch-rtl8367b swconfig wpad -urngd
endef
TARGET_DEVICES += scensmart_sv901-eaio

define Device/seewo_sv21-rk3568
  DEVICE_VENDOR := Seewo
  DEVICE_MODEL := sv21
  DEVICE_DTS := rockchip/rk3568-seewo-sv21
  UBOOT_DEVICE_NAME := seewo-sv21-rk3568
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net-rtl8152 kmod-ata-ahci-dwc
endef
TARGET_DEVICES += seewo_sv21-rk3568

define Device/sharevdi_h3399pc
  DEVICE_VENDOR := SHAREVDI
  DEVICE_MODEL := H3399PC
  SOC := rk3399
  UBOOT_DEVICE_NAME := h3399pc-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += sharevdi_h3399pc

define Device/sharevdi_guangmiao-g4c
  DEVICE_VENDOR := SHAREVDI
  DEVICE_MODEL := GuangMiao G4C
  SOC := rk3399
  UBOOT_DEVICE_NAME := guangmiao-g4c-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += sharevdi_guangmiao-g4c

define Device/xunlong_orangepi-r1-plus
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi R1 Plus
  SOC := rk3328
  UBOOT_DEVICE_NAME := orangepi-r1-plus-rk3328
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += xunlong_orangepi-r1-plus

define Device/xunlong_orangepi-r1-plus-lts
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi R1 Plus LTS
  SOC := rk3328
  UBOOT_DEVICE_NAME := orangepi-r1-plus-lts-rk3328
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-bin | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += xunlong_orangepi-r1-plus-lts

define Device/widora_mangopi-m28k
  DEVICE_VENDOR := Widora
  DEVICE_MODEL := MangoPi M28K
  SOC := rk3528
  UBOOT_DEVICE_NAME := evb-rk3528
  IMAGE/sysupgrade.img.gz := boot-common | boot-script rk3528 | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-aic8800 kmod-r8168 wpad-openssl -urngd
endef
TARGET_DEVICES += widora_mangopi-m28k

define Device/widora_mangopi-m28k-pro
$(call Device/widora_mangopi-m28k)
  DEVICE_MODEL := MangoPi M28K Pro
  DEVICE_PACKAGES := kmod-aic8800 kmod-i2c-gpio kmod-r8125 wpad-openssl -urngd
endef
TARGET_DEVICES += widora_mangopi-m28k-pro
