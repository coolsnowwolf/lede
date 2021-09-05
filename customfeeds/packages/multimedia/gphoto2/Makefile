#
# Copyright (C) 2006-2012 OpenWrt.org
# Copyright (C) 2017      Leonardo Medici
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gphoto2
PKG_VERSION:=2.5.26
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/gphoto
PKG_HASH:=7653213b05329c1dc2779efea3eff00504e12011436587aedc9aaa1e8665ab2f

PKG_MAINTAINER:=Leonardo Medici <leonardo_medici@me.com>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/gphoto2
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=Gphoto Digital Camera Control
  URL:=http://www.gphoto.org/
  DEPENDS:=+libgphoto2 +libpopt +libpthread +libreadline +libncurses +libexif +libjpeg
endef

define Package/gphoto2/description
 For downloading and controlling digital cameras
endef

CONFIGURE_ARGS += \
	--without-aalib

CONFIGURE_VARS += \
	POPT_CFLAGS="-I$(STAGING_DIR)/usr/include" \
	POPT_LIBS="-L$(STAGING_DIR)/usr/lib -lpopt"

define Package/gphoto2/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/$(PKG_NAME) $(1)/usr/bin/
endef

$(eval $(call BuildPackage,gphoto2))
