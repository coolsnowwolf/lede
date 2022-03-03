#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=automount
PKG_VERSION:=1
PKG_RELEASE:=36
PKG_ARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/automount
  TITLE:=Mount autoconfig hotplug script.
  MAINTAINER:=Lean
  DEPENDS:=+block-mount +kmod-fs-exfat +kmod-fs-ext4 +kmod-fs-vfat +libblkid \
	+kmod-usb-storage +kmod-usb-storage-extras +!TARGET_ramips:kmod-usb-storage-uas \
	+!LINUX_5_15:antfs-mount +LINUX_5_15:ntfs3-mount
endef

define Package/automount/description
A usb autoconfig hotplug script.
endef

define Build/Compile
endef

define Package/automount/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/15-automount $(1)/etc/15-automount
	$(INSTALL_BIN) ./files/zzz-move-automount $(1)/etc/uci-defaults/zzz-move-automount
endef

$(eval $(call BuildPackage,automount))
