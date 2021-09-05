include $(TOPDIR)/rules.mk

PKG_NAME:=perl-netaddr-ip
PKG_VERSION:=4.079
PKG_RELEASE:=1

PKG_SOURCE_NAME:=NetAddr-IP
PKG_SOURCE_URL:=https://www.cpan.org/authors/id/M/MI/MIKER/
PKG_SOURCE:=$(PKG_SOURCE_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=ec5a82dfb7028bcd28bb3d569f95d87dd4166cc19867f2184ed3a59f6d6ca0e7

PKG_LICENSE:=GPL-2.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=Copying
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/perl/NetAddr-IP-$(PKG_VERSION)
HOST_BUILD_DEPENDS:=perl/host
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/NetAddr-IP-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../perl/perlmod.mk

define Package/perl-netaddr-ip
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=NetAddr::IP - Manages IPv4 and IPv6 addresses and subnets
  URL:=http://search.cpan.org/dist/NetAddr::IP/
  DEPENDS:=perl +perlbase-essential +perlbase-test
endef

define Host/Configure
	$(call perlmod/host/Configure,-noxs,,)
	$(call Host/Configure/Default,,,Lite/Util)
endef

define Host/Compile
	$(call Host/Compile/Default,,,Lite/Util)
	$(call perlmod/host/Compile,,)
endef

define Host/Install
	$(call perlmod/host/Install,$(1),)
endef

define Build/Configure
	$(call perlmod/Configure,-noxs,)
	$(call Build/Configure/Default,,,Lite/Util)
endef

define Build/Compile
	$(call Build/Compile/Default,,,Lite/Util)
	$(call perlmod/Compile,,)
endef

define Package/perl-netaddr-ip/install
	$(call perlmod/Install,$(1),NetAddr auto/NetAddr)
endef

$(eval $(call BuildPackage,perl-netaddr-ip))
$(eval $(call HostBuild))
