#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=automount
PKG_VERSION:=1
PKG_RELEASE:=16
PKG_ARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/automount
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Mount autoconfig hotplug script.
  MAINTAINER:=Lean
  DEPENDS:=block-mount +kmod-usb-storage +kmod-usb-storage-extras +kmod-fs-ext4 +kmod-fs-vfat +kmod-fs-exfat +ntfs-3g
endef

define Package/automount/description
A usb autoconfig hotplug script.
endef

define Build/Compile
endef

define Package/automount/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/block
	$(INSTALL_BIN) ./files/15-automount $(1)/etc/hotplug.d/block/15-automount
endef

$(eval $(call BuildPackage,automount))
