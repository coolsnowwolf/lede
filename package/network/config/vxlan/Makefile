include $(TOPDIR)/rules.mk

PKG_NAME:=vxlan
PKG_RELEASE:=7
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/vxlan
  SECTION:=net
  CATEGORY:=Network
  MAINTAINER:=Matthias Schiffer <mschiffer@universe-factory.net>
  TITLE:=Virtual eXtensible LAN config support
  DEPENDS:=+kmod-vxlan
endef

define Package/vxlan/description
 Virtual eXtensible LAN config support in /etc/config/network.
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/vxlan/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/vxlan.sh $(1)/lib/netifd/proto/vxlan.sh
endef

$(eval $(call BuildPackage,vxlan))
