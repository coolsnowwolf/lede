#
# Copyright (C) 2013-2024 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

KERNEL_LOADADDR=0x81000000

define Device/licheepi-nano
  DEVICE_VENDOR := LicheePi
  DEVICE_MODEL := Nano
  DEVICE_PACKAGES := kmod-rtc-sunxi
  SOC := suniv-f1c100s
endef
TARGET_DEVICES += licheepi-nano

define Device/popstick-v1.1
  DEVICE_VENDOR := PopStick
  DEVICE_MODEL := v1.1
  DEVICE_PACKAGES := kmod-rtc-sunxi
  SOC := suniv-f1c200s
endef
TARGET_DEVICES += popstick-v1.1
