#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-net-http
PKG_VERSION:=6.19
PKG_RELEASE:=1

PKG_SOURCE:=Net-HTTP-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/O/OA/OALDERS
PKG_HASH:=52b76ec13959522cae64d965f15da3d99dcb445eddd85d2ce4e4f4df385b2fc4
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Net-HTTP-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-net-http
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Low-level HTTP connection (client)
  URL:=https://search.cpan.org/dist/Net-HTTP/
  DEPENDS:=perl +perl-uri +perlbase-compress +perlbase-essential +perlbase-io
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-net-http/install
        $(call perlmod/Install,$(1),Net auto/Net)
endef


$(eval $(call BuildPackage,perl-net-http))
