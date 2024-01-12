#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pcat-manager
PKG_RELEASE:=1.0.16

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2023-03-17
PKG_SOURCE_URL:=https://github.com/photonicat/rockchip_rk3568_pcat_manager
PKG_SOURCE_VERSION:=5b8d8e098205e92fe9587f03d8bc34f8383987b2
PKG_MIRROR_HASH:=309ff9a0c73701af3428951d0df13e29525bcb5636262486b42f6a17eaebe2c0

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/pcat-manager
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=photonicat watchdog
  URL:=https://photonicat.com
  DEPENDS:=+glib2 +libgpiod +libjson-c +libusb-1.0
endef

define Package/pcat-manager/description
  Photonicat Manager is a system service for
  power management and modem controlling.
endef

define Package/pcat-manager/install
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_CONF) ./files/pcat-manager.conf $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/pcat-manager.init $(1)/etc/init.d/pcat-manager
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/pcat-manager \
		$(1)/usr/bin/pcat-manager
endef

$(eval $(call BuildPackage,pcat-manager))
