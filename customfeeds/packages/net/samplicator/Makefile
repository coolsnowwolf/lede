include $(TOPDIR)/rules.mk

PKG_NAME:=samplicator
PKG_VERSION:=1.3.6
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/sleinen/samplicator/releases/download/v$(PKG_VERSION)
PKG_HASH:=3c4358b4b0992a77251f2b9e2221d4ae945781160732c73504eb126e69d72d40

PKG_MAINTAINER:=Nick Hainke <vincent@systemli.org>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/samplicator
  SECTION:=net
  CATEGORY:=Network
  TITLE:=UDP Samplicator
  URL:=https://github.com/sleinen/samplicator
endef

define Package/samplicator/description
  Send copies of (UDP) datagrams to multiple receivers,
  with optional sampling and spoofing.
endef

define Package/samplicator/conffiles
/etc/samplicator.conf
endef

define Package/samplicator/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/samplicate $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/samplicator.init $(1)/etc/init.d/samplicator
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/samplicator.conf $(1)/etc/
endef

$(eval $(call BuildPackage,samplicator))
