#
# Copyright (C) 2018 TDT AG <development@tdt.de>
#
# This is free software, licensed under the GNU General Public License v2.
# See https://www.gnu.org/licenses/gpl-2.0.txt for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=proto-bonding
PKG_VERSION:=2021-04-09
PKG_RELEASE:=2

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Helge Mader <ma@dev.tdt.de>

include $(INCLUDE_DIR)/package.mk

define Package/proto-bonding
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Link Aggregation (Channel Bonding) proto handler
  DEPENDS:=+kmod-bonding
  PKGARCH:=all
endef

define Package/proto-bonding/description
  This package contains the channel bonding proto handler for netifd
endef

define Build/Compile
endef

define Package/proto-bonding/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto/
	$(INSTALL_BIN) ./files/lib/netifd/proto/bonding.sh \
		$(1)/lib/netifd/proto/
endef

$(eval $(call BuildPackage,proto-bonding))
