#!/bin/sh
#
# Copyright © 2020 David Woodhouse <dwmw2@infradead.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Generate as "scatter file" for use with the MediaTek SP Flash tool for
# writing images to MediaTek boards. This can be used to write images
# even on a bricked board which has no preloader installed, or broken
# U-Boot.
#
# NOTE: At the time of writing (2020-07-20), the Linux tool linked from
# the front page of https://spflashtool.com/ is out of date and does not
# support MT7623. The newer v5.1916 found on the download page at
# https://spflashtool.com/download/ has been tested on UniElec U7623 and
# Banana Pi R2 from Linux, and does work.
#

SOC=$1
IMAGE=${2%.gz}
PROJECT=${3%-scatter.txt}
DEVICENAME="$4"

cat <<EOF
# OpenWrt eMMC scatter file for ${DEVICENAME}
# For use with SP Flash Tool: https://spflashtool.com/download/
#
# Unzip the file system file ${PROJECT}-${IMAGE}.gz before flashing.
# Connect the device's USB port, although it won't appear to the host yet.
# Load this scatter file into the SP Flash Tool, select 'Format All + Download'
# After pressing the 'Download' button, power on the board.
# The /dev/ttyACM0 device should appear on USB and the tool should find it.

- general: MTK_PLATFORM_CFG
  info:
    - config_version: V1.1.2
      platform: ${SOC}
      project: ${PROJECT}
      storage: EMMC
      boot_channel: MSDC_0
      block_size: 0x20000

- partition_index: SYS0
  partition_name: PRELOADER
  file_name: ${PROJECT}-preloader.bin
  is_download: true
  type: SV5_BL_BIN
  linear_start_addr: 0x0
  physical_start_addr: 0x0
  partition_size: 0x40000
  region: EMMC_BOOT_1
  storage: HW_STORAGE_EMMC
  boundary_check: true
  is_reserved: false
  operation_type: BOOTLOADERS
  d_type: FALSE
  reserve: 0x00

- partition_index: SYS1
  partition_name: OPENWRT
  file_name: ${PROJECT}-${IMAGE}
  is_download: true
  type: NORMAL_ROM
  linear_start_addr: 0x00000
  physical_start_addr: 0x00000
  partition_size: 0x4000000
  region: EMMC_USER
  storage: HW_STORAGE_EMMC
  boundary_check: true
  is_reserved: false
  operation_type: UPDATE
  d_type: FALSE
  reserve: 0x00
EOF
