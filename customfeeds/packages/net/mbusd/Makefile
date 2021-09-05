include $(TOPDIR)/rules.mk

PKG_NAME:=mbusd
PKG_VERSION:=0.5.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_URL:=https://codeload.github.com/3cky/mbusd/tar.gz/v$(PKG_VERSION)?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=a41fc1254972d41d184d47e82b13c83577f22023f7a540d02062b704bce5c710

PKG_MAINTAINER:=Marcin Jurkowski <marcin1j@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/mbusd
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Modbus TCP gateway
	URL:=https://github.com/3cky/mbusd
endef

define Package/mbusd/description
Mbusd is open-source Modbus TCP to Modbus RTU (RS-232/485) gateway.
It presents a network of RTU slaves as single TCP slave.

That is a TCP-Slave (or server) which acts as a RTU-master to get
data from Modbus RTU-slave devices.
endef

define Package/mbusd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mbusd $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/mbusd.conf $(1)/etc/config/mbusd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/mbusd.init $(1)/etc/init.d/mbusd
endef

$(eval $(call BuildPackage,mbusd))
