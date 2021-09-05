#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libshout
PKG_VERSION:=2.4.5
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://downloads.us.xiph.org/releases/libshout/
PKG_HASH:=d9e568668a673994ebe3f1eb5f2bee06e3236a5db92b8d0c487e1c0f886a6890

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:libshout:libshout

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libshout/default
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Shoutcast client library
  URL:=http://www.icecast.org
  DEPENDS:= +libvorbisidec +libpthread
endef

define Package/libshout
  $(call Package/libshout/default)
  TITLE+= (no theora)
  DEPENDS+= +libopenssl
  VARIANT:=notheora
endef

define Package/libshout-nossl
  $(call Package/libshout/default)
  TITLE+= (no ssl/theora)
  VARIANT:=nossl
endef

define Package/libshout-full
  $(call Package/libshout/default)
  TITLE+=(full)
  DEPENDS+= +libtheora +libopenssl
  VARIANT:=full
endef

define Package/libshout/description/default
 libshout allows applications to easily communicate and broadcast
 to an Icecast streaming media server. It handles the socket connections,
 metadata communication, and data streaming for the calling application,
 and lets developers focus on feature sets instead of implementation
 details.
endef

define Package/libshout/description
  $(call Package/libshout/description/default)
  .
  This package does not have Theora support.
endef

define Package/libshout-nossl/description
  $(call Package/libshout/description/default)
  .
  This package does not have OpenSSL or Theora support.
endef

Package/libshout-full/description=$(Package/libshout/description/default)

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-speex \
	--without-pic

ifeq ($(BUILD_VARIANT),notheora)
  CONFIGURE_ARGS += --disable-theora
endif

ifeq ($(BUILD_VARIANT),nossl)
  CONFIGURE_ARGS += --disable-theora --without-openssl
endif

CONFIGURE_VARS += \
	VORBIS_CFLAGS="-I$(STAGING_DIR)/usr/include/tremor/" \
	VORBIS_LIBS="$(TARGET_LDFLAGS) -lvorbisidec" \

TARGET_CFLAGS += $(FPIC)

PACKAGE_CONFIG_FILE=shout$(if $(findstring $(BUILD_VARIANT),full),-full).pc

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/shout $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libshout.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/shout.pc $(1)/usr/lib/pkgconfig/$(PACKAGE_CONFIG_FILE)
	$(SED) 's| -I/usr/include||' $(1)/usr/lib/pkgconfig/$(PACKAGE_CONFIG_FILE)
endef

define Package/libshout/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libshout.so.* $(1)/usr/lib/
endef

Package/libshout-full/install=$(Package/libshout/install)
Package/libshout-nossl/install=$(Package/libshout/install)

$(eval $(call BuildPackage,libshout))
$(eval $(call BuildPackage,libshout-nossl))
$(eval $(call BuildPackage,libshout-full))
