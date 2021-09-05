#
# Copyright (C) 2011-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-net-telnet
PKG_VERSION:=3.04
PKG_RELEASE:=4

PKG_SOURCE_URL:=http://www.cpan.org/authors/id/J/JR/JROGERS/
PKG_SOURCE:=Net-Telnet-$(PKG_VERSION).tar.gz
PKG_HASH:=e64d567a4e16295ecba949368e7a6b8b5ae2a16b3ad682121d9b007dc5d2a37a

PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Net-Telnet-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-net-telnet
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Telnet client
  URL:=http://search.cpan.org/dist/Net-Telnet/
  DEPENDS:=perl +perlbase-essential +perlbase-socket +perlbase-symbol
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-net-telnet/install
	$(call perlmod/Install,$(1),Net auto/Net)
endef


$(eval $(call BuildPackage,perl-net-telnet))
