#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pcat-manager
PKG_VERSION:=1.0.16
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2022-11-17
PKG_SOURCE_URL:=https://github.com/photonicat/rockchip_rk3568_pcat_manager
PKG_SOURCE_VERSION:=b35d84e1636bbed8751d15f4b73c911912652435
PKG_MIRROR_HASH:=f06ef0011ed28bb7c42c308f83b89c1f4b586950bdaf5c562a4b5708789e6746

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
