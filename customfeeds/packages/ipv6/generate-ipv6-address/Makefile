include $(TOPDIR)/rules.mk

PKG_NAME:=generate-ipv6-address
PKG_VERSION:=0.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=generate-ipv6-address-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.irif.fr/~jch/software/files/
PKG_HASH:=e1356d245d5f891fa39b796a8a2deefdaa89f08130dc97a378738ca5ed9a20fa

PKG_MAINTAINER:=Nick Hainke <vincent@systemli.org>
PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/generate-ipv6-address
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Generate IPv6 Addresses
	URL:=https://www.irif.fr/~jch/
	DEPENDS:=@IPV6
endef

define Package/generate-ipv6-address/description
 Generates IPv6 addresses from a given prefix and either a given MAC-48
 address (an Ethernet hardware address) or a randomly drawn host number.
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) $(PKG_BUILD_DIR)/generate-ipv6-address.c -o $(PKG_BUILD_DIR)/generate-ipv6-address
endef

define Package/generate-ipv6-address/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/generate-ipv6-address $(1)/usr/bin/
endef

$(eval $(call BuildPackage,generate-ipv6-address))
