#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-http-server-simple
PKG_VERSION:=0.52
PKG_RELEASE:=1

PKG_SOURCE:=HTTP-Server-Simple-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.cpan.org/authors/id/B/BP/BPS
PKG_HASH:=d8939fa4f12bd6b8c043537fd0bf96b055ac3686b9cdd9fa773dca6ae679cb4c
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/HTTP-Server-Simple-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-http-server-simple
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lightweight HTTP server
  URL:=https://search.cpan.org/dist/HTTP-Server-Simple/
  DEPENDS:=perl +perl-cgi +perlbase-base +perlbase-essential +perlbase-filehandle +perlbase-socket +perlbase-test
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-http-server-simple/install
        $(call perlmod/Install,$(1),HTTP auto/HTTP)
endef


$(eval $(call BuildPackage,perl-http-server-simple))
