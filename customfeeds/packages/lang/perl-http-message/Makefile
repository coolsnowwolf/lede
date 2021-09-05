#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-http-message
PKG_VERSION:=6.22
PKG_RELEASE:=1

PKG_SOURCE:=HTTP-Message-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/O/OA/OALDERS
PKG_HASH:=970efd151b81c95831d2a5f9e117f8032b63a1768cd2cd3f092ad634c85175c3
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/HTTP-Message-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-http-message
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=HTTP style messages
  URL:=https://search.cpan.org/dist/HTTP-Message/
  DEPENDS:=perl +perl-encode-locale +perl-http-date +perl-io-html +perl-lwp-mediatypes +perl-uri +perlbase-base +perlbase-compress +perlbase-encode +perlbase-essential +perlbase-io +perlbase-mime
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-http-message/install
        $(call perlmod/Install,$(1),HTTP auto/HTTP)
endef


$(eval $(call BuildPackage,perl-http-message))
