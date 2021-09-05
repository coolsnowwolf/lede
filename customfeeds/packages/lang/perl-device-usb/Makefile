#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-device-usb
PKG_VERSION:=0.38
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://www.cpan.org/authors/id/G/GW/GWADEJ/
PKG_SOURCE:=Device-USB-$(PKG_VERSION).tar.gz
PKG_HASH:=f0c36379db4913406dc9bd546a6cfd4da20d58f7c2cc085893a2fe67a43465dc

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Device-USB-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=perl-inline-c/host

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-device-usb
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=libusb library wrapper
  URL:=https://search.cpan.org/dist/Device-USB/
  DEPENDS:=perl +libusb-compat +perl-inline +perl-inline-c +perlbase-essential +perlbase-test
endef

# Device::USB will generate a second Makefile.PL to build it's native code
# using Inline::C under _Inline/. The goal here is to prevent Inline::C from
# running that one automatically, as we need to supply a bunch of overrides via
# perlmod/Configure.
# _INLINE_C_NO_COMPILE_ is an OpenWRT-specific environment variable to Inline::C
# that does just that.
define Build/Configure
	$(call perlmod/Configure,,)
	
	# Prepare a library directory where we can link againt lusb-0.1(as
	# opposed to -lusb) to avoid confusion at runtime
	mkdir $(PKG_BUILD_DIR)/syslib
	$(LN) $(STAGING_DIR)/usr/lib/libusb.so $(PKG_BUILD_DIR)/syslib/libusb-0.1.so
	
	# Generate _Inline
	$(call perlmod/Compile,config,LIBUSB_LIBDIR=$(PKG_BUILD_DIR)/syslib \
	                              _INLINE_C_NO_COMPILE_=1 \
	                              _INLINE_C_SYSTEM_TYPEMAP_=$(STAGING_DIR)/usr/lib/perl5/$(PERL_VERSION)/ExtUtils/typemap)
	$(call perlmod/Compile,dynamic,_INLINE_C_NO_COMPILE_=1)
	
	# Configure _Inline
	$(call perlmod/Configure,,,$(PKG_BUILD_DIR)/_Inline/build/Device/USB)
	
	# Prevent re-configuring Inline code
	sed 's/^dynamic :: D.*$$$$//' -i $(PKG_BUILD_DIR)/Makefile
endef

define Build/Compile
	$(call perlmod/Compile,,,$(PKG_BUILD_DIR)/_Inline/build/Device/USB)
	$(call perlmod/Compile,,)
endef

define Package/perl-device-usb/install
        $(call perlmod/Install,$(1),Device auto/Device)
endef


$(eval $(call BuildPackage,perl-device-usb))
