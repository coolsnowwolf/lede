#
# Copyright (C) 2006-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tcl
TCL_MAJOR_VERSION:=8.6
PKG_VERSION:=${TCL_MAJOR_VERSION}.11
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)$(PKG_VERSION)-src.tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_HASH:=8c0486668586672c5693d7d95817cb05a18c5ecca2f40e2836b9578064088258
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)$(PKG_VERSION)

PKG_MAINTAINER:=Joe Mistachkin <joe@mistachkin.com>
PKG_LICENSE:=TCL
PKG_LICENSE_FILES:=license.terms
PKG_CPE_ID:=cpe:/a:tcl_tk:tcl_tk

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/tcl
  SUBMENU:=Tcl
  SECTION:=lang
  CATEGORY:=Languages
  DEPENDS:=+libpthread +zlib
  TITLE:=The Tcl language
  URL:=https://www.tcl.tk/
endef

define Package/tcl/description
  Tcl, or Tool Command Language, is a an elegant, versatile, feature-rich,
  simple-to-learn yet very powerful industrial-strength open-source
  programming language and development platform. It is renowned for its
  stability and utility, and its emphasis on providing a cross-platform
  programming API makes it an ideal choice for an enormous variety of
  programming jobs.
endef

CONFIGURE_PATH := unix

CONFIGURE_VARS += \
	tcl_cv_strtod_unbroken=ok

CONFIGURE_ARGS += \
	--enable-threads

MAKE_PATH := unix

define Build/Prepare
	$(call Build/Prepare/Default)
	rm -rf $(PKG_BUILD_DIR)/pkgs/*
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtcl*.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/tclConfig.sh $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/tclooConfig.sh $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/tcl.pc \
		$(1)/usr/lib/pkgconfig
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/tcl.pc
	$(SED) 's,/usr/lib,$$$${prefix}/lib,g' $(1)/usr/lib/pkgconfig/tcl.pc
endef

define Package/tcl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin

	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/tcl8 $(1)/usr/lib/
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/tcl$(TCL_MAJOR_VERSION) $(1)/usr/lib/
	$(LN) tclsh$(TCL_MAJOR_VERSION) $(1)/usr/bin/tclsh
endef

define Host/Configure
	$(call Host/Configure/Default,$(1),$(2),$(CONFIGURE_PATH)/$(3))
endef

define Host/Compile
	+$(HOST_MAKE_VARS) \
	$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR)/$(MAKE_PATH) \
		$(HOST_MAKE_FLAGS) \
		$(1)
endef

define Host/Install
	$(call Host/Compile,install)
	(cd $(HOST_BUILD_PREFIX)/bin; test -f tclsh || ln -s tclsh$(TCL_MAJOR_VERSION) tclsh)
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,tcl))
