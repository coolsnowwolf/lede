#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pppossh
PKG_RELEASE:=2
PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>
PKG_LICENSE:=GPLv2

include $(INCLUDE_DIR)/package.mk

define Package/pppossh
  SECTION:=net
  CATEGORY:=Network
  TITLE:=PPPoSSH (Point-to-Point Protocol over SSH)
  DEPENDS:=+ppp +resolveip @(PACKAGE_dropbear||PACKAGE_openssh-client)
  PKGARCH:=all
endef

define Package/pppossh/description
This package adds protocol support for PPP over SSH.  The protocol name is
'pppossh' as in netifd interface config option 'proto'.
endef

define Build/Compile
endef

define Package/pppossh/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/pppossh.sh $(1)/lib/netifd/proto
endef

$(eval $(call BuildPackage,pppossh))
