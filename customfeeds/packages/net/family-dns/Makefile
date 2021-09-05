#
# Copyright (c) 2020 Gregory L. Dietsche <Gregory.Dietsche@cuw.edu>
# This is free software, licensed under the MIT License
#

include $(TOPDIR)/rules.mk

PKG_NAME:=family-dns
PKG_VERSION:=1.0.0
PKG_RELEASE:=2
PKG_LICENSE:=MIT
PKG_MAINTAINER:=Gregory L. Dietsche <Gregory.Dietsche@cuw.edu>

include $(INCLUDE_DIR)/package.mk

define Package/family-dns
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Family DNS
	PKGARCH:=all
endef

define Package/family-dns/description
This package configures your router to block adult websites.
The default settings are designed to be appropriate for most businesses,
schools and families.
endef

define Package/family-dns/conffiles
/etc/config/family-dns
endef

define Build/Compile
endef

define Package/family-dns/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/family-dns.uci-defaults $(1)/etc/uci-defaults/family-dns

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/family-dns.conf $(1)/etc/config/family-dns

	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/family-dns-update $(1)/usr/sbin/family-dns-update
	$(INSTALL_BIN) ./files/test-family-dns $(1)/usr/sbin/test-family-dns
endef

define Package/family-dns/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
  /usr/sbin/family-dns-update uninstall
fi
exit 0
endef

$(eval $(call BuildPackage,family-dns))
