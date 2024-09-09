# SPDX-License-Identifier: GPL-2.0-only

include ./common.mk

define Device/allnet_all-sg8208m
  SOC := rtl8382
  IMAGE_SIZE := 7168k
  DEVICE_VENDOR := ALLNET
  DEVICE_MODEL := ALL-SG8208M
  UIMAGE_MAGIC := 0x00000006
  UIMAGE_NAME := 2.2.2.0
endef
TARGET_DEVICES += allnet_all-sg8208m

define Device/apresia_aplgs120gtss
  $(Device/cameo-fw)
  SOC := rtl8382
  IMAGE_SIZE := 14848k
  DEVICE_VENDOR := APRESIA
  DEVICE_MODEL := ApresiaLightGS120GT-SS
  UIMAGE_MAGIC := 0x12345000
  CAMEO_KERNEL_PART_SIZE := 1572864
  CAMEO_KERNEL_PART := 3
  CAMEO_ROOTFS_PART := 4
  CAMEO_CUSTOMER_SIGNATURE := 2
  CAMEO_BOARD_MODEL := APLGS120GTSS
  CAMEO_BOARD_VERSION := 4
endef
TARGET_DEVICES += apresia_aplgs120gtss

define Device/d-link_dgs-1210-10mp-f
  $(Device/d-link_dgs-1210)
  SOC := rtl8380
  DEVICE_MODEL := DGS-1210-10MP
  DEVICE_VARIANT := F
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += d-link_dgs-1210-10mp-f

define Device/d-link_dgs-1210-10p
  $(Device/d-link_dgs-1210)
  SOC := rtl8382
  DEVICE_MODEL := DGS-1210-10P
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += d-link_dgs-1210-10p

define Device/d-link_dgs-1210-16
  $(Device/d-link_dgs-1210)
  SOC := rtl8382
  DEVICE_MODEL := DGS-1210-16
endef
TARGET_DEVICES += d-link_dgs-1210-16

define Device/d-link_dgs-1210-20
  $(Device/d-link_dgs-1210)
  SOC := rtl8382
  DEVICE_MODEL := DGS-1210-20
endef
TARGET_DEVICES += d-link_dgs-1210-20

define Device/d-link_dgs-1210-28
  $(Device/d-link_dgs-1210)
  SOC := rtl8382
  DEVICE_MODEL := DGS-1210-28
endef
TARGET_DEVICES += d-link_dgs-1210-28

define Device/d-link_dgs-1210-28mp-f
  $(Device/d-link_dgs-1210)
  SOC := rtl8382
  DEVICE_MODEL := DGS-1210-28MP
  DEVICE_VARIANT := F
  DEVICE_PACKAGES += realtek-poe kmod-hwmon-lm63
endef
TARGET_DEVICES += d-link_dgs-1210-28mp-f

define Device/d-link_dgs-1210-28p-f
  $(Device/d-link_dgs-1210)
  SOC := rtl8382
  DEVICE_MODEL := DGS-1210-28P
  DEVICE_VARIANT := F
  DEVICE_PACKAGES += realtek-poe kmod-hwmon-lm63
endef
TARGET_DEVICES += d-link_dgs-1210-28p-f

# The "IMG-" uImage name allows flashing the iniramfs from the vendor Web UI.
# Avoided for sysupgrade, as the vendor FW would do an incomplete flash.
define Device/engenius_ews2910p
  SOC := rtl8380
  IMAGE_SIZE := 8192k
  DEVICE_VENDOR := EnGenius
  DEVICE_MODEL := EWS2910P
  DEVICE_PACKAGES += realtek-poe
  UIMAGE_MAGIC := 0x03802910
  KERNEL_INITRAMFS := \
	kernel-bin | \
	append-dtb | \
	libdeflate-gzip | \
	uImage gzip -n 'IMG-0.00.00-c0.0.00'
endef
TARGET_DEVICES += engenius_ews2910p

define Device/hpe_1920-8g
  $(Device/hpe_1920)
  SOC := rtl8380
  DEVICE_MODEL := 1920-8G (JG920A)
  H3C_DEVICE_ID := 0x00010023
endef
TARGET_DEVICES += hpe_1920-8g

define Device/hpe_1920-8g-poe-65w
  $(Device/hpe_1920)
  SOC := rtl8380
  DEVICE_MODEL := 1920-8G-PoE+ 65W (JG921A)
  DEVICE_PACKAGES += realtek-poe
  H3C_DEVICE_ID := 0x00010024
endef
TARGET_DEVICES += hpe_1920-8g-poe-65w

define Device/hpe_1920-8g-poe-180w
  $(Device/hpe_1920)
  $(Device/hwmon-fan-migration)
  SOC := rtl8380
  DEVICE_MODEL := 1920-8G-PoE+ 180W (JG922A)
  DEVICE_PACKAGES += realtek-poe kmod-hwmon-gpiofan
  H3C_DEVICE_ID := 0x00010025
  SUPPORTED_DEVICES += hpe,1920-8g-poe
endef
TARGET_DEVICES += hpe_1920-8g-poe-180w

define Device/hpe_1920-16g
  $(Device/hpe_1920)
  SOC := rtl8382
  DEVICE_MODEL := 1920-16G (JG923A)
  H3C_DEVICE_ID := 0x00010026
endef
TARGET_DEVICES += hpe_1920-16g

define Device/hpe_1920-24g
  $(Device/hpe_1920)
  SOC := rtl8382
  DEVICE_MODEL := 1920-24G (JG924A)
  H3C_DEVICE_ID := 0x00010027
endef
TARGET_DEVICES += hpe_1920-24g

define Device/hpe_1920-24g-poe-180w
  $(Device/hpe_1920)
  $(Device/hwmon-fan-migration)
  SOC := rtl8382
  DEVICE_MODEL := 1920-24G-PoE+ 180W (JG925A)
  DEVICE_PACKAGES += realtek-poe kmod-hwmon-gpiofan
  H3C_DEVICE_ID := 0x00010028
endef
TARGET_DEVICES += hpe_1920-24g-poe-180w

define Device/hpe_1920-24g-poe-370w
  $(Device/hpe_1920)
  $(Device/hwmon-fan-migration)
  SOC := rtl8382
  DEVICE_MODEL := 1920-24G-PoE+ 370W (JG926A)
  DEVICE_PACKAGES += realtek-poe kmod-hwmon-gpiofan
  H3C_DEVICE_ID := 0x00010029
endef
TARGET_DEVICES += hpe_1920-24g-poe-370w

define Device/inaba_aml2-17gp
  SOC := rtl8382
  IMAGE_SIZE := 13504k
  DEVICE_VENDOR := INABA
  DEVICE_MODEL := Abaniact AML2-17GP
  UIMAGE_MAGIC := 0x83800000
endef
TARGET_DEVICES += inaba_aml2-17gp

define Device/iodata_bsh-g24mb
  SOC := rtl8382
  IMAGE_SIZE := 13696k
  DEVICE_VENDOR := I-O DATA
  DEVICE_MODEL := BSH-G24MB
  UIMAGE_MAGIC := 0x83800013
endef
TARGET_DEVICES += iodata_bsh-g24mb

define Device/linksys_lgs310c
  SOC := rtl8380
  IMAGE_SIZE := 13504k
  DEVICE_VENDOR := Linksys
  DEVICE_MODEL := LGS310C
  BELKIN_MODEL := BKS-RTL83xx
  BELKIN_HEADER := 0x07800001
  LINKSYS_MODEL := 60402060
  IMAGES += factory.imag
  IMAGE/factory.imag := \
	append-kernel | \
	pad-to 64k | \
	append-rootfs | \
	pad-rootfs | \
	check-size | \
	append-metadata | \
	linksys-image | \
	belkin-header
endef
TARGET_DEVICES += linksys_lgs310c

# "NGE" refers to the uImage magic
define Device/netgear_nge
  KERNEL := \
	kernel-bin | \
	append-dtb | \
	lzma | \
	uImage lzma
  KERNEL_INITRAMFS := \
	kernel-bin | \
	append-dtb | \
	lzma | \
	uImage lzma
  SOC := rtl8380
  IMAGE_SIZE := 14848k
  UIMAGE_MAGIC := 0x4e474520
  UIMAGE_NAME := 9.9.9.9
  DEVICE_VENDOR := NETGEAR
endef

# "NGG" refers to the uImage magic
define Device/netgear_ngg
  KERNEL := \
	kernel-bin | \
	append-dtb | \
	lzma | \
	uImage lzma
  KERNEL_INITRAMFS := \
	kernel-bin | \
	append-dtb | \
	lzma | \
	uImage lzma
  SOC := rtl8380
  IMAGE_SIZE := 14848k
  UIMAGE_MAGIC := 0x4e474720
  DEVICE_VENDOR := NETGEAR
endef

define Device/netgear_gs108t-v3
  $(Device/netgear_nge)
  DEVICE_MODEL := GS108T
  DEVICE_VARIANT := v3
endef
TARGET_DEVICES += netgear_gs108t-v3

define Device/netgear_gs110tpp-v1
  $(Device/netgear_nge)
  DEVICE_MODEL := GS110TPP
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += netgear_gs110tpp-v1

define Device/netgear_gs110tup-v1
  $(Device/netgear_ngg)
  DEVICE_MODEL := GS110TUP
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += netgear_gs110tup-v1

define Device/netgear_gs308t-v1
  $(Device/netgear_nge)
  DEVICE_MODEL := GS308T
  DEVICE_VARIANT := v1
  UIMAGE_MAGIC := 0x4e474335
endef
TARGET_DEVICES += netgear_gs308t-v1

define Device/netgear_gs310tp-v1
  $(Device/netgear_nge)
  DEVICE_MODEL := GS310TP
  DEVICE_VARIANT := v1
  UIMAGE_MAGIC := 0x4e474335
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += netgear_gs310tp-v1

define Device/panasonic_m16eg-pn28160k
  SOC := rtl8382
  IMAGE_SIZE := 16384k
  DEVICE_VENDOR := Panasonic
  DEVICE_MODEL := Switch-M16eG
  DEVICE_VARIANT := PN28160K
  DEVICE_PACKAGES := kmod-i2c-mux-pca954x
endef
TARGET_DEVICES += panasonic_m16eg-pn28160k

define Device/panasonic_m24eg-pn28240k
  SOC := rtl8382
  IMAGE_SIZE := 16384k
  DEVICE_VENDOR := Panasonic
  DEVICE_MODEL := Switch-M24eG
  DEVICE_VARIANT := PN28240K
  DEVICE_PACKAGES := kmod-i2c-mux-pca954x
endef
TARGET_DEVICES += panasonic_m24eg-pn28240k

define Device/panasonic_m8eg-pn28080k
  SOC := rtl8380
  IMAGE_SIZE := 16384k
  DEVICE_VENDOR := Panasonic
  DEVICE_MODEL := Switch-M8eG
  DEVICE_VARIANT := PN28080K
  DEVICE_PACKAGES := kmod-i2c-mux-pca954x
endef
TARGET_DEVICES += panasonic_m8eg-pn28080k

define Device/tplink_sg2008p-v1
  SOC := rtl8380
  KERNEL_SIZE := 6m
  IMAGE_SIZE := 26m
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := SG2008P
  DEVICE_VARIANT := v1
  DEVICE_PACKAGES := kmod-hwmon-tps23861
endef
TARGET_DEVICES += tplink_sg2008p-v1

define Device/tplink_sg2210p-v3
  SOC := rtl8380
  KERNEL_SIZE := 6m
  IMAGE_SIZE := 26m
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := SG2210P
  DEVICE_VARIANT := v3
  DEVICE_PACKAGES := kmod-hwmon-tps23861
endef
TARGET_DEVICES += tplink_sg2210p-v3

define Device/tplink_t1600g-28ts-v3
  SOC := rtl8382
  KERNEL_SIZE := 6m
  IMAGE_SIZE := 26m
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := T1600G-28TS
  DEVICE_VARIANT := v3
endef
TARGET_DEVICES += tplink_t1600g-28ts-v3

define Device/zyxel_gs1900-10hp
  $(Device/zyxel_gs1900)
  SOC := rtl8380
  DEVICE_MODEL := GS1900-10HP
  ZYXEL_VERS := AAZI
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += zyxel_gs1900-10hp

define Device/zyxel_gs1900-16
  $(Device/zyxel_gs1900)
  SOC := rtl8382
  DEVICE_MODEL := GS1900-16
  ZYXEL_VERS := AAHJ
endef
TARGET_DEVICES += zyxel_gs1900-16

define Device/zyxel_gs1900-8-v1
  $(Device/zyxel_gs1900)
  SOC := rtl8380
  DEVICE_MODEL := GS1900-8
  DEVICE_VARIANT := v1
  ZYXEL_VERS := AAHH
  SUPPORTED_DEVICES += zyxel,gs1900-8
endef
TARGET_DEVICES += zyxel_gs1900-8-v1

define Device/zyxel_gs1900-8-v2
  $(Device/zyxel_gs1900)
  SOC := rtl8380
  DEVICE_MODEL := GS1900-8
  DEVICE_VARIANT := v2
  ZYXEL_VERS := AAHH
  SUPPORTED_DEVICES += zyxel,gs1900-8
endef
TARGET_DEVICES += zyxel_gs1900-8-v2

define Device/zyxel_gs1900-8hp-v1
  $(Device/zyxel_gs1900)
  SOC := rtl8380
  DEVICE_MODEL := GS1900-8HP
  DEVICE_VARIANT := v1
  ZYXEL_VERS := AAHI
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += zyxel_gs1900-8hp-v1

define Device/zyxel_gs1900-8hp-v2
  $(Device/zyxel_gs1900)
  SOC := rtl8380
  DEVICE_MODEL := GS1900-8HP
  DEVICE_VARIANT := v2
  ZYXEL_VERS := AAHI
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += zyxel_gs1900-8hp-v2

define Device/zyxel_gs1900-24-v1
  $(Device/zyxel_gs1900)
  SOC := rtl8382
  DEVICE_MODEL := GS1900-24
  DEVICE_VARIANT := v1
  ZYXEL_VERS := AAHL
endef
TARGET_DEVICES += zyxel_gs1900-24-v1

define Device/zyxel_gs1900-24e
  $(Device/zyxel_gs1900)
  SOC := rtl8382
  DEVICE_MODEL := GS1900-24E
  ZYXEL_VERS := AAHK
endef
TARGET_DEVICES += zyxel_gs1900-24e

define Device/zyxel_gs1900-24ep
  $(Device/zyxel_gs1900)
  SOC := rtl8382
  DEVICE_MODEL := GS1900-24EP
  ZYXEL_VERS := ABTO
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += zyxel_gs1900-24ep

define Device/zyxel_gs1900-24hp-v1
  $(Device/zyxel_gs1900)
  SOC := rtl8382
  DEVICE_MODEL := GS1900-24HP
  DEVICE_VARIANT := v1
  ZYXEL_VERS := AAHM
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += zyxel_gs1900-24hp-v1

define Device/zyxel_gs1900-24hp-v2
  $(Device/zyxel_gs1900)
  SOC := rtl8382
  DEVICE_MODEL := GS1900-24HP
  DEVICE_VARIANT := v2
  ZYXEL_VERS := ABTP
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += zyxel_gs1900-24hp-v2
