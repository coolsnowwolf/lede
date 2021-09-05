include $(TOPDIR)/rules.mk

PKG_NAME:=libwslay
PKG_VERSION:=1.1.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=release-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/tatsuhiro-t/wslay/tar.gz/release-$(PKG_VERSION)?
PKG_HASH:=7b9f4b9df09adaa6e07ec309b68ab376c0db2cfd916613023b52a47adfda224a
PKG_BUILD_DIR:=$(BUILD_DIR)/wslay-release-$(PKG_VERSION)

PKG_MAINTAINER:=Peter van Dijk <peter.van.dijk@powerdns.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libwslay
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Wslay is a WebSocket library written in C
  URL:=https://tatsuhiro-t.github.io/wslay/
  BUILDONLY:=1
endef

define Package/libwslay/description
   Wslay is a WebSocket library written in C. It implements the protocol version 13 described
   in RFC 6455. This library offers 2 levels of API: event-based API and frame-based low-level
   API. For event-based API, it is suitable for non-blocking reactor pattern style. You can set
   callbacks in various events. For frame-based API, you can send WebSocket frame directly.
   Wslay only supports data transfer part of WebSocket protocol and does not perform opening
   handshake in HTTP.
endef

CMAKE_OPTIONS += \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON

$(eval $(call BuildPackage,libwslay))
