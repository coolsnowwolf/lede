include $(TOPDIR)/rules.mk

PKG_NAME:=wg-installer
PKG_RELEASE:=$(AUTORELEASE)

PKG_MAINTAINER:=Nick Hainke <vincent@systemli.org>

include $(INCLUDE_DIR)/package.mk

Build/Compile=

define Package/wg-installer/Default
	SECTION:=net
	CATEGORY:=Network
	TITLE:=WireGuard Installer
	URL:=https://github.com/Freifunk-Spalter/
	PKGARCH:=all
	DEPENDS:=+kmod-wireguard +owipcalc
endef

define Package/wg-installer-server
	$(call Package/wg-installer/Default)
	TITLE+= (server)
	MENU:=1
	DEPENDS:=+rpcd +uhttpd +uhttpd-mod-ubus +owipcalc
endef

define Package/wg-installer-server/install
	$(INSTALL_DIR) $(1)/usr/share/wginstaller/
	$(INSTALL_BIN) ./wg-server/lib/install_wginstaller_user.sh $(1)/usr/share/wginstaller/install_wginstaller_user.sh
	$(INSTALL_BIN) ./wg-server/lib/wg_functions.sh $(1)/usr/share/wginstaller/wg_functions.sh
	$(INSTALL_BIN) ./common/wg.sh $(1)/usr/share/wginstaller/wg.sh

	$(INSTALL_DIR) $(1)/usr/libexec/rpcd/
	$(INSTALL_BIN) ./wg-server/wginstaller.sh $(1)/usr/libexec/rpcd/wginstaller

	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(CP) ./wg-server/config/wginstaller.json $(1)/usr/share/rpcd/acl.d/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./wg-server/config/wgserver.conf $(1)/etc/config/wgserver
endef

define Package/wg-installer-server/postinst
	#!/bin/sh
	if [ -z $${IPKG_INSTROOT} ] ; then
		. /usr/share/wginstaller/install_wginstaller_user.sh
	fi
endef

define Package/wg-installer-server-hotplug-babeld
	$(call Package/wg-installer-server)
	DEPENDS:=wg-installer-server
endef

define Package/wg-installer-server-hotplug-babeld/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/net/
	$(INSTALL_BIN) ./wg-server/hotplug.d/99-mesh-babeld $(1)/etc/hotplug.d/net/99-mesh-babeld
endef

define Package/wg-installer-client
	$(call Package/wg-installer/Default)
	TITLE+= (client)
	DEPENDS:=+curl +wireguard-tools
endef

define Package/wg-installer-client/install
	$(INSTALL_DIR) $(1)/usr/share/wginstaller/
	$(INSTALL_BIN) ./wg-client/lib/rpcd_ubus.sh $(1)/usr/share/wginstaller/rpcd_ubus.sh
	$(INSTALL_BIN) ./common/wg.sh $(1)/usr/share/wginstaller/wg.sh

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./wg-client/wg-client-installer.sh $(1)/usr/bin/wg-client-installer

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./wg-client/config/wgclient.conf $(1)/etc/config/wgclient
endef

$(eval $(call BuildPackage,wg-installer-server))
$(eval $(call BuildPackage,wg-installer-server-hotplug-babeld))
$(eval $(call BuildPackage,wg-installer-client))
