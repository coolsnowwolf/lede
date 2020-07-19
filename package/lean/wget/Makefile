#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wget
PKG_VERSION:=1.20.3
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=31cccfc6630528db1c8e3a06f6decf2a370060b982841cfab2b8677400a5092e
PKG_MAINTAINER:=Peter Wagner <tripolar@gmx.at>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_CPE_ID:=cpe:/a:gnu:wget

include $(INCLUDE_DIR)/package.mk

define Package/wget/Default
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpcre +zlib
  SUBMENU:=File Transfer
  TITLE:=Non-interactive network downloader
  URL:=https://www.gnu.org/software/wget/index.html
  PROVIDES:=gnu-wget
endef

define Package/wget/Default/description
 Wget is a network utility to retrieve files from the Web using http
 and ftp, the two most widely used Internet protocols. It works
 non-interactively, so it will work in the background, after having
 logged off. The program supports recursive retrieval of web-authoring
 pages as well as ftp sites -- you can use wget to make mirrors of
 archives and home pages or to travel the Web like a WWW robot.
endef

define Package/wget
$(call Package/wget/Default)
  DEPENDS+= +libopenssl +librt
  TITLE+= (with SSL support)
  VARIANT:=ssl
  PROVIDES+=wget-ssl
  ALTERNATIVES:=300:/usr/bin/wget:/usr/bin/wget-ssl
endef

define Package/wget/description
$(call Package/wget/Default/description)
 This package is built with SSL support.
endef

define Package/wget-nossl
$(call Package/wget/Default)
  TITLE+= (without SSL support)
  VARIANT:=nossl
  PROVIDES+=wget
  ALTERNATIVES:=300:/usr/bin/wget:/usr/bin/wget-nossl
endef

define Package/wget-nossl/description
$(call Package/wget/Default/description)
 This package is built without SSL support.
endef

CONFIGURE_ARGS+= \
	--disable-rpath \
	--disable-iri \
	--disable-pcre2 \
	--with-included-libunistring \
	--without-libuuid \
	--without-libpsl

CONFIGURE_VARS += \
	ac_cv_header_uuid_uuid_h=no

ifeq ($(BUILD_VARIANT),ssl)
	CONFIGURE_ARGS+= \
		--with-ssl=openssl \
		--with-libssl-prefix="$(STAGING_DIR)/usr"

	CONFIGURE_VARS += \
		ac_cv_libssl=yes
endif

ifeq ($(BUILD_VARIANT),nossl)
	CONFIGURE_ARGS+= \
		--disable-ntlm \
		--without-ssl
endif

define Package/wget/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wget $(1)/usr/bin/wget-ssl
endef

define Package/wget-nossl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/wget $(1)/usr/bin/wget-nossl
endef

$(eval $(call BuildPackage,wget))
$(eval $(call BuildPackage,wget-nossl))
