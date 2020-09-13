include $(TOPDIR)/rules.mk

PKG_NAME:=dns2socks
PKG_VERSION:=2.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/kongfl888/dns2socks.git
PKG_SOURCE_VERSION:=HEAD
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_LICENSE:=GPL-3.0
PKG_MAINTAINER:=kongfl888 <kongfl888@outlook.com>
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/dns2socks
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  TITLE:=The utility to resolve DNS requests via a SOCKS5 tunnel.
  URL:=https://sourceforge.net/projects/dns2socks/
  DEPENDS:=+libpthread
endef

define Package/dns2socks/description
  This is a utility to resolve DNS requests via a SOCKS5 tunnel and caches the answers.
endef

define Build/Compile
	$(TARGET_CC) \
	$(TARGET_CFLAGS) \
	$(TARGET_CPPFLAGS) \
	$(FPIC) \
	-o $(PKG_BUILD_DIR)/DNS2SOCKS/$(PKG_NAME) \
	$(PKG_BUILD_DIR)/DNS2SOCKS/DNS2SOCKS.c \
	$(TARGET_LDFLAGS) -pthread
endef

define Package/dns2socks/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/DNS2SOCKS/$(PKG_NAME) $(1)/usr/bin/dns2socks
endef

$(eval $(call BuildPackage,dns2socks))
