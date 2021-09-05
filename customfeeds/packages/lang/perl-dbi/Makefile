#
# Copyright (C) 2014, 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-dbi
PKG_VERSION:=1.643
PKG_RELEASE:=1

PKG_SOURCE:=DBI-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.cpan.org/authors/id/T/TI/TIMB/
PKG_HASH:=8a2b993db560a2c373c174ee976a51027dd780ec766ae17620c20393d2e836fa

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/DBI-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/perl/DBI-$(PKG_VERSION)
HOST_BUILD_DEPENDS:=perl/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../perl/perlmod.mk

define Package/perl-dbi
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Database independent interface for Perl
  URL:=https://search.cpan.org/dist/DBI/
  DEPENDS:=perl +perlbase-base +perlbase-config +perlbase-cwd +perlbase-data +perlbase-dynaloader +perlbase-errno +perlbase-essential +perlbase-fcntl +perlbase-file +perlbase-io +perlbase-ipc +perlbase-scalar +perlbase-storable +perlbase-symbol +perlbase-tie +perlbase-universal +perlbase-utf8
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Host/Configure
	$(call perlmod/host/Configure,,)
endef

define Host/Compile
	$(call perlmod/host/Compile,,)
endef

define Host/Install
	$(call perlmod/host/Install,$(1),)
endef

define Package/perl-dbi/install
	$(call perlmod/Install,$(1),DB* auto/DBI,DBI/W32ODBC.pm auto/DBI/*.h)
endef


$(eval $(call BuildPackage,perl-dbi))
$(eval $(call HostBuild))
