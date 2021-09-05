#
# Author: Tibor Dudlák
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tang
PKG_VERSION:=10
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/latchset/$(PKG_NAME)/releases/download/v$(PKG_VERSION)/
PKG_HASH:=082f17e031708ab5e2f88c9b767d228bcf92a0d089fdb164d4ae93845cbd2a48

PKG_MAINTAINER:=Tibor Dudlák <tibor.dudlak@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=meson/host

include $(INCLUDE_DIR)/package.mk
include ../../devel/meson/meson.mk

define Package/tang
  SECTION:=utils
  TITLE:=tang v$(PKG_VERSION) - daemon for binding data to the presence of a third party
  DEPENDS:=+libhttp-parser +xinetd +jose +bash
  URL:=https://github.com/latchset/tang
endef

define Package/tang/description
	Tang is a small daemon for binding data to the presence of a third party.
endef

define Package/tang/conffiles
/etc/xinetd.d/tangdx
/usr/share/tang/db/
endef

define Package/tang/install
	$(INSTALL_DIR)	$(1)/usr/libexec
	$(INSTALL_DIR)	$(1)/etc/xinetd.d/
	$(INSTALL_BIN)	$(PKG_INSTALL_DIR)/usr/libexec/tangd*	$(1)/usr/libexec/
	$(INSTALL_BIN)	./files/tangdw				$(1)/usr/libexec/
	$(CP)		./files/tangdx				$(1)/etc/xinetd.d/
endef

define Package/tang/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	mkdir -p /usr/share/tang/db
	KEYS=$(find /usr/share/tang/db/ -name "*.jw*" -maxdepth 1 | wc -l)
	if [ "${KEYS}" = "0" ]; then # if db is empty generate new key pair
		/usr/libexec/tangd-keygen /usr/share/tang/db/
	(cat /etc/services | grep -E "tangd.*8888\/tcp") > /dev/null \
		|| echo -e "tangd\t\t8888/tcp" >> /etc/services
fi
endef

$(eval $(call BuildPackage,tang))
