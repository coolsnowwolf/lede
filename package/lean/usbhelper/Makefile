#
# Copyright (C) 2015 OpenWrt-dist
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=usbhelper
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=R3pl4c3r

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/usbhelper
	SECTION:=usb
	TITLE:=USB Modules Auto Select Helper
	PKGARCH:=all
	DEPENDS:=+blockd +kmod-block2mtd +kmod-ata-ahci +kmod-ata-core +kmod-scsi-generic \
		+kmod-usb-ehci +kmod-usb-ohci +kmod-usb-uhci +kmod-usb2 +kmod-usb3 \
		+kmod-nls-cp936
endef

define Package/usbhelper/description
	USB Modules Auto Select Helper
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/usbhelper/install
endef


$(eval $(call BuildPackage,usbhelper))
