include $(TOPDIR)/rules.mk

PKG_NAME:=ratched
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Toni Uhlig <matzeton@googlemail.com>
PKG_LICENSE:=GPL-3.0-only

PKG_SOURCE_URL:=https://github.com/johndoe31415/ratched.git
PKG_SOURCE_VERSION:=091d000ba91c5ebc41148cb9b511a4b527c1ea44
PKG_SOURCE_PROTO:=git
PKG_MIRROR_HASH:=67c8ff1a54af34181d8467803e18c709d06fd20929b6fdbba68700af88de71aa

include $(INCLUDE_DIR)/package.mk

MAKE_FLAGS+=CFLAGS='$(TARGET_CFLAGS) -DBUILD_TIMESTAMP_UTC="\"unknown\"" -DBUILD_REVISION="\"$(PKG_VERSION)-$(PKG_RELEASE)\""'
MAKE_FLAGS+=LDFLAGS='$(TARGET_LDFLAGS) -lssl -lcrypto $(if $(CONFIG_USE_GLIBC),-lpthread)'

define Package/ratched
  SECTION:=net
  CATEGORY:=Network
  TITLE:=ratched
  URL:=https://github.com/johndoe31415/ratched
  DEPENDS:=+libopenssl +libpthread
endef

define Package/ratched/description
  Ratched is a transparent Man-in-the-Middle TLS proxy intended for penetration testing.
endef

define Package/ratched/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ratched $(1)/usr/bin/ratched
endef

$(eval $(call BuildPackage,ratched))
