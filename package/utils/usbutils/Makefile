#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=usbutils
PKG_VERSION:=007
PKG_RELEASE:=7

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/usb/usbutils
PKG_HASH:=7593a01724bbc0fd9fe48e62bc721ceb61c76654f1d7b231b3c65f6dfbbaefa4
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

PKG_MAINTAINER := Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk

define Package/usbutils
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libusb-1.0 +librt +libpthread
  TITLE:=USB devices listing utilities
  URL:=http://www.linux-usb.org/
endef

USB_IDS_REV:=0a6be488b4c1485d30c9efee1cc895af5d4a1383
USB_IDS_FILE:=usb.ids.$(USB_IDS_REV)
define Download/usb_ids
  FILE:=$(USB_IDS_FILE)
  URL_FILE:=usb.ids
  URL:=@GITHUB/gentoo/hwids/$(USB_IDS_REV)
  HASH:=2d0403ffccc0ff7150e32c0bf8e2878816b76ba08d2c3bb1059f33627a3da30f
endef
$(eval $(call Download,usb_ids))

define Build/Prepare
	$(Build/Prepare/Default)
	$(CP) $(DL_DIR)/$(USB_IDS_FILE) $(PKG_BUILD_DIR)/usb.ids
endef

CONFIGURE_ARGS += \
	--disable-zlib

define Package/usbutils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/lsusb $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/usb.ids $(1)/usr/share/
endef

$(eval $(call BuildPackage,usbutils))
