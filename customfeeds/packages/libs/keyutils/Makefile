#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=keyutils
PKG_VERSION:=1.6.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/linux/kernel/git/dhowells/keyutils.git/snapshot/
PKG_HASH:=a61d5706136ae4c05bd48f86186bcfdbd88dd8bd5107e3e195c924cfc1b39bb4

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libkeyutils
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Key utilities library
  URL:=https://people.redhat.com/dhowells/keyutils/
  LICENSE:=LGPL-2.1-or-later
  LICENSE_FILES:=LICENSE.LGPL
  ABI_VERSION:=1
endef

define Package/keyctl
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Encryption
  TITLE:=keyctl
  DEPENDS:=+libkeyutils
  LICENSE:=GPL-2.0-or-later
  LICENSE_FILES:=LICENSE.GPL
endef

define Package/keyutils
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Encryption
  TITLE:=keyutils (request-key and key.dns_resolver)
  DEPENDS:=+libkeyutils
  LICENSE:=GPL-2.0-or-later
  LICENSE_FILES:=LICENSE.GPL
endef

define Package/keyutils/description
  Key utilities
endef

MAKE_FLAGS += \
	BINDIR=/usr/bin \
	LIBDIR=/usr/lib \
	SBINDIR=/usr/sbin \
	CFLAGS="$(TARGET_CFLAGS) $(FPIC)"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkeyutils.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkeyutils.so.$(ABI_VERSION)* $(1)/usr/lib/
endef

define Package/libkeyutils/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkeyutils.so.$(ABI_VERSION)* $(1)/usr/lib/
endef

define Package/keyutils/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/keyutils $(1)/etc/request-key.d
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/request-key $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/key.dns_resolver $(1)/usr/sbin/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/request-key.conf $(1)/etc/
endef

define Package/keyctl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/keyctl $(1)/usr/bin
endef

$(eval $(call BuildPackage,libkeyutils))
$(eval $(call BuildPackage,keyutils))
$(eval $(call BuildPackage,keyctl))
