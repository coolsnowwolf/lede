include $(TOPDIR)/rules.mk

PKG_NAME:=overture
PKG_VERSION:=1.7
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/shawn1m/overture/tar.gz/v${PKG_VERSION}?
PKG_HASH:=d3912fe53d2f6a60d20767a8dc5041333f8b5386b7d23d959b4de872d12b5024

PKG_MAINTAINER:=Richard Yu <yurichard3839@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/shawn1m/overture
GO_PKG_BUILD_PKG:=$(GO_PKG)/main
GO_PKG_LDFLAGS_X:=main.version=$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/overture
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  TITLE:=A customized DNS forwarder written in Go
  URL:=https://github.com/shawn1m/overture
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/overture/description
  Overture is a DNS server/forwarder/dispatcher written in Go.
endef

define Package/overture/conffiles
/etc/overture/
endef

define Package/overture/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/main $(1)/usr/bin/overture
	$(INSTALL_DIR) $(1)/etc/overture
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/config.sample.yml $(1)/etc/overture/config.yml
	$(SED) 's@\.\(/[^[:space:]]\{1,\}_sample\)@/etc/overture\1@g;s/^\(bindAddress:[[:space:]]\{0,\}\)[^[:space:]]\{0,\}$$$$/\1127.0.0.3:53/' $(1)/etc/overture/config.yml
	$(INSTALL_DATA) ./files/*_sample $(1)/etc/overture
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/overture.init $(1)/etc/init.d/overture
endef

$(eval $(call GoBinPackage,overture))
$(eval $(call BuildPackage,overture))
