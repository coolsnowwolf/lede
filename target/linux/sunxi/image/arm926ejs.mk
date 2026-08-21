#
# Copyright (C) 2013-2024 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

KERNEL_LOADADDR=0x81000000

define Device/licheepi_licheepi-nano
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := LicheePi
  DEVICE_MODEL := LicheePi Nano
  DEVICE_PACKAGES := kmod-rtc-sunxi
  SOC := suniv-f1c100s
endef
TARGET_DEVICES += licheepi_licheepi-nano

define Device/sourceparts_popstick-v1.1
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := SourceParts
  DEVICE_MODEL := PopStick
  DEVICE_VARIANT := v1.1
  DEVICE_PACKAGES := kmod-rtc-sunxi
  SOC := suniv-f1c200s
endef
TARGET_DEVICES += sourceparts_popstick-v1.1
