#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ices
PKG_VERSION:=2.0.2
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://downloads.xiph.org/releases/ices/
PKG_HASH:=96458df835033c39236ad3bb099286c033241f8ea7c138e7ccf714e7ade75900

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Nicolas Thill <nico@openwrt.org>, \
		Ted Hess <thess@kitschensync.net>

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/ices
  SECTION:=sound
  CATEGORY:=Sound
  TITLE:=ices client for Icecast media streaming servers
  URL:=http://www.icecast.org/ices/
  DEPENDS:=+libshout +libxml2 +zlib +libogg +libvorbis +alsa-lib $(ICONV_DEPENDS)
endef

define Package/ices/description
  ices is a command line source client for Icecast media streaming servers.
  It began as the successor of the old "shout" utility, and has since gained a
  lot of useful features.
endef

CONFIGURE_ARGS+= \
	--with-ogg="$(STAGING_DIR)/usr/include" \
	--with-vorbis="$(STAGING_DIR)/usr/include"

TARGET_LDFLAGS+= -logg -lvorbisidec

define Package/ices/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ices $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ices))
