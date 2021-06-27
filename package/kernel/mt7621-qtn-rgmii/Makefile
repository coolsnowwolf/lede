#
# Copyright (C) 2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt7621-qtn-rgmii
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

PKG_MAINTAINER:=Bjørn Mork <bjorn@mork.no>

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt7621-qtn-rgmii
  SECTION:=kernel
  SUBMENU:=Other modules
  TITLE:=Enable RGMII connected Quantenna module on MT7621
  DEPENDS:=@TARGET_ramips_mt7621
  HIDDEN:=1
  FILES:=$(PKG_BUILD_DIR)/mt7621-qtn-rgmii.ko
  AUTOLOAD:=$(call AutoLoad,30,mt7621-qtn-rgmii,1)
endef

define KernelPackage/mt7621-qtn-rgmii/description
  Enable RGMII connected Quantenna module on MT7621.

  The Mitrastar designed ZyXEL WAP6805 has a Quantenna QV840
  module connected to the RGMII pins of the MT7621 SoC. For
  unknown reasons, it is necessary to change the value of
  the register at 0x1e110008 from default (usually 0xc000c)
  to 0x9000c for this connection wo work.

  This driver simply does that without much fuzz.
endef

define Build/Compile
        $(KERNEL_MAKE) M=$(PKG_BUILD_DIR) modules
endef

$(eval $(call KernelPackage,mt7621-qtn-rgmii))
