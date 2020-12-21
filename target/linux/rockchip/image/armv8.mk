# 
# Copyright (C) 2020 Tobias Maedel
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Device/friendlyarm_nanopi-r2s
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R2S
  SOC := rk3328
  UBOOT_DEVICE_NAME := nanopi-r2s-rk3328
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r2s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-usb-net kmod-usb-net-rtl8152
endef
TARGET_DEVICES += friendlyarm_nanopi-r2s

define Device/friendlyarm_nanopi-r4s
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R4S
  SOC := rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script nanopi-r4s | pine64-img | gzip | append-metadata
  DEVICE_PACKAGES := kmod-r8169
endef

define Device/friendlyarm_nanopi-r4s-1gb
  $(Device/friendlyarm_nanopi-r4s)
  DEVICE_MODEL += 1GB
  UBOOT_DEVICE_NAME := nanopi-r4s-1gb-rk3399
endef
TARGET_DEVICES += friendlyarm_nanopi-r4s-1gb

define Device/friendlyarm_nanopi-r4s-4gb
  $(Device/friendlyarm_nanopi-r4s)
  DEVICE_MODEL += 4GB
  UBOOT_DEVICE_NAME := nanopi-r4s-4gb-rk3399
endef
TARGET_DEVICES += friendlyarm_nanopi-r4s-4gb

define Device/pine64_rockpro64
  DEVICE_VENDOR := Pine64
  DEVICE_MODEL := RockPro64
  SOC := rk3399
  UBOOT_DEVICE_NAME := rockpro64-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | pine64-img | gzip | append-metadata
endef
TARGET_DEVICES += pine64_rockpro64
