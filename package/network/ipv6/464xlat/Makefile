include $(TOPDIR)/rules.mk

PKG_NAME:=464xlat
PKG_RELEASE:=12

PKG_SOURCE_DATE:=2018-01-16
PKG_MAINTAINER:=Hans Dedecker <dedeckeh@gmail.com>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/464xlat
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@IPV6 +kmod-nat46 +ip
  TITLE:=464xlat CLAT support
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/464xlat/description
  464xlat provides support to deploy limited IPv4 access services to mobile
  and wireline IPv6-only edge networks without encapsulation (RFC6877)
endef

define Package/464xlat/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/464xlat.sh $(1)/lib/netifd/proto/464xlat.sh
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/464xlatcfg $(1)/sbin
endef

$(eval $(call BuildPackage,464xlat))
