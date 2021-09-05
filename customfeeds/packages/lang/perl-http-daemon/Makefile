#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-http-daemon
PKG_VERSION:=6.06
PKG_RELEASE:=1

PKG_SOURCE:=HTTP-Daemon-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/O/OA/OALDERS
PKG_HASH:=fc03a161b54553f766457a4267e7066767f54ad01cacfe9a91d7caa2a0319bad
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/HTTP-Daemon-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENCE

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-http-daemon
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=A simple http server class
  URL:=http://search.cpan.org/dist/HTTP-Daemon/
  DEPENDS:=perl +perl-http-date +perl-http-message +perl-lwp-mediatypes +perlbase-essential +perlbase-io +perlbase-sys
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-http-daemon/install
        $(call perlmod/Install,$(1),HTTP auto/HTTP)
endef


$(eval $(call BuildPackage,perl-http-daemon))
