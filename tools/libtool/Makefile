# 
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=libtool
PKG_CPE_ID:=cpe:/a:gnu:libtool
PKG_VERSION:=2.4.7

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=04e96c2404ea70c590c546eba4202a4e12722c640016c12b9b2f1ce3d481e9a8

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

export GNULIB_SRCDIR:=$(HOST_GNULIB_SRCDIR)

HOST_CONFIGURE_VARS += \
	lt_cv_sys_dlsearch_path=""

define Host/Bootstrap
	( \
		cd $(HOST_BUILD_DIR); \
		$(AM_TOOL_PATHS) \
		./bootstrap \
			--copy \
			--force \
			--skip-git \
			--skip-po \
			--gnulib-srcdir=$(GNULIB_SRCDIR) \
	)
endef

define Host/Prepare
	$(call Host/Prepare/Default)
	$(call Host/Uninstall)
	$(if $(QUILT),,$(call Host/Bootstrap))
endef

define Host/Configure
	$(if $(QUILT),$(call Host/Bootstrap))
	$(call Host/Configure/Default)
endef

define Host/Install
	$(call Host/Compile/Default,install)
	$(SED) 's,\(hardcode_into_libs\)=yes,\1=no,g' $(STAGING_DIR_HOST)/bin/libtool
endef

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
	(cd $(STAGING_DIR_HOST)/share/aclocal/ && rm -f libtool.m4 ltdl.m4 lt~obsolete.m4 ltoptions.m4 ltsugar.m4 ltversion.m4)
endef

$(eval $(call HostBuild))
