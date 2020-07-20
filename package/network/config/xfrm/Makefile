
include $(TOPDIR)/rules.mk

PKG_NAME:=xfrm
PKG_VERSION:=1
PKG_RELEASE:=2
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/xfrm/Default
  SECTION:=net
  CATEGORY:=Network
  MAINTAINER:=Andre Valentin <avalentin@marcant.net>
  PKGARCH:=all
endef

define Package/xfrm
$(call Package/xfrm/Default)
  TITLE:=XFRM IPsec Tunnel Interface config support
  DEPENDS:=+kmod-xfrm-interface
endef

define Package/xfrm/description
 XFRM IPsec Tunnel Interface config support (IPv4 and IPv6) in /etc/config/network.
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/xfrm/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/xfrm.sh $(1)/lib/netifd/proto/xfrm.sh
endef

$(eval $(call BuildPackage,xfrm))
