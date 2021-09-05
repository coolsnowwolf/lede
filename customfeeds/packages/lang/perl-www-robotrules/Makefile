#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-www-robotrules
PKG_VERSION:=6.02
PKG_RELEASE:=2

PKG_SOURCE_URL:=http://www.cpan.org/authors/id/G/GA/GAAS
PKG_SOURCE:=WWW-RobotRules-$(PKG_VERSION).tar.gz
PKG_HASH:=46b502e7a288d559429891eeb5d979461dd3ecc6a5c491ead85d165b6e03a51e

PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/WWW-RobotRules-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-www-robotrules
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=database of robots.txt-derived permissions
  URL:=http://search.cpan.org/dist/WWW-RobotRules/
  DEPENDS:=perl +perl-uri +perlbase-anydbm-file +perlbase-essential +perlbase-fcntl
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-www-robotrules/install
        $(call perlmod/Install,$(1),WWW auto/WWW)
endef


$(eval $(call BuildPackage,perl-www-robotrules))
