#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libhttp-parser
PKG_VERSION:=2.9.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/nodejs/http-parser/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=467b9e30fd0979ee301065e70f637d525c28193449e1b13fbcb1b1fab3ad224f
PKG_BUILD_DIR:=$(BUILD_DIR)/http-parser-$(PKG_VERSION)

PKG_MAINTAINER:=Ramanathan Sivagurunathan <ramzthecoder@gmail.com>, Hirokazu MORIKAWA <morikw2@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE-MIT

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libhttp-parser
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A library to parse http request and response
  URL:=https://github.com/nodejs/http-parser
endef

define Package/libhttp-parser/description
  A parser for HTTP messages written in C. It parses both requests and responses.
  The parser is designed to be used in performance HTTP applications.
  It does not make any syscalls nor allocations, it does not buffer data,
  it can be interrupted at anytime. Depending on your architecture,
  it only requires about 40 bytes of data per message stream
  (in a web server that is per connection).
endef

MAKE_FLAGS+=library \
	PREFIX=/usr

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/http_parser.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libhttp_parser.so* $(1)/usr/lib/
endef

define Package/libhttp-parser/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libhttp_parser.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libhttp-parser))
