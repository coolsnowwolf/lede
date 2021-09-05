#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sqm-scripts-extra
PKG_VERSION:=2016-06-08
PKG_RELEASE:=1
PKG_LICENSE:=GPLv2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE).tar.xz
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define Package/sqm-scripts-extra
  SECTION:=net
  CATEGORY:=Base system
  DEPENDS:=+sqm-scripts
  TITLE:=SQM Scripts - additional experimental scripts for testing
  PKGARCH:=all
  MAINTAINER:=Kevin Darbyshire-Bryant <kevin@darbyshire-bryant.me.uk>, \
	Sebastian Moeller <moeller0@gmx.de>
endef

define Package/sqm-scripts-extra/description
  A set of experimental scripts for sqm-scripts QoS package. The contents of this package may
  change as new qdiscs like 'cake' are developed and tested.
endef

define Package/sqm-scripts-extra/download
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/sqm-scripts-extra/install
	$(INSTALL_DIR) $(1)/usr/lib/sqm
	$(INSTALL_DATA) ./src/* $(1)/usr/lib/sqm/
endef

$(eval $(call BuildPackage,sqm-scripts-extra))

