#
# Copyright (C) 2016 Nikil Mehta <nikil.mehta@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wakeonlan
PKG_VERSION:=0.41
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_SOURCE_URL:=http://http.debian.net/debian/pool/main/w/wakeonlan/
PKG_HASH:=581b1b27a7e810ab72668cce4bd9aa9b3e0cea34b2db24dd1a44c09d63ddda98

PKG_MAINTAINER:=Nikil Mehta <nikil.mehta@gmail.com>
PKG_LICENSE:=Artistic-1.0-Perl

include $(INCLUDE_DIR)/package.mk

define Package/wakeonlan
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Sends 'magic packets' to wake-on-LAN enabled ethernet adapters
  URL:=http://gsd.di.uminho.pt/jpo/software/wakeonlan/
  DEPENDS:=+perl +perlbase-getopt +perlbase-net +perlbase-socket
  PKGARCH:=all
endef

define Package/wakeonlan/description
	With this package you can remotely wake up and power on machines which have
	motherboards or network cards that support 'Wake-on-Lan' packets.
	.
	The tool allows you to wake up a single machine, or a group of machines.
	.
	You need the MAC addresses of machines to construct the WOL packets, but,
	in contrast to 'etherwake', you do not need root privileges to use the
	program itself as UDP packets are used.
endef

define Build/Compile
endef

define Package/wakeonlan/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wakeonlan $(1)/usr/bin/
endef

$(eval $(call BuildPackage,wakeonlan))
