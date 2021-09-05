# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=open-isns
PKG_VERSION:=0.100
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/open-iscsi/open-isns/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=b011edbb0f31690aaca902a8ecf4e1f17b01d6c9e9afc51909d26b0993b4328f

PKG_MAINTAINER:=Lucian CRISTIAN <lucian.cristian@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/open-isns
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=http://www.open-iscsi.com/
  TITLE:=open-isns
  DEPENDS:=+libopenssl
endef

define Package/open-isns/description
This is a partial implementation of iSNS, according to RFC4171.
endef

CONFIGURE_ARGS += \
    --enable-shared

TARGET_CFLAGS += $(FPIC) -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libisns
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/libisns/* $(1)/usr/include/libisns/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libisns.so* $(1)/usr/lib/
endef

define Package/open-isns/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libisns.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,open-isns))
