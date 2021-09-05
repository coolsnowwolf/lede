#
# Copyright (C) 2007-2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=glib2
PKG_VERSION:=2.68.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=glib-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNOME/glib/2.68
PKG_HASH:=241654b96bd36b88aaa12814efc4843b578e55d47440103727959ac346944333

PKG_MAINTAINER:=Peter Wagner <tripolar@gmx.at>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gnome:glib

PKG_BUILD_DIR:=$(BUILD_DIR)/glib-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/glib-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=meson/host gettext-full libiconv/host
HOST_BUILD_DEPENDS:=meson/host gettext-full/host libiconv/host libffi/host
PKG_CONFIG_DEPENDS:=CONFIG_BUILD_NLS
PKG_INSTALL:=1

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../devel/meson/meson.mk

define Package/glib2
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=$(ICONV_DEPENDS) $(INTL_DEPENDS) +zlib +libpthread +libffi +libattr
  TITLE:=glib 2.0
  URL:=http://www.gtk.org/
endef

define Package/glib2/description
  The GLib library of C routines
endef

HOST_LDFLAGS += -liconv -Wl,-rpath,$(STAGING_DIR_HOSTPKG)/lib
TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections $(if $(INTL_FULL),-lintl)

COMP_ARGS= \
	-Ddefault_library=both \
	-Diconv=external \
	-Dselinux=disabled \
	-Dlibmount=disabled \
	-Dinternal_pcre=true \
	-Dman=false \
	-Ddtrace=false \
	-Dsystemtap=false \
	-Dsysprof=disabled \
	-Dgtk_doc=false \
	-Dbsymbolic_functions=true \
	-Dforce_posix_threads=true \
	-Dfam=false \
	-Dtests=false \
	-Dinstalled_tests=false \
	-Dnls=$(if $(CONFIG_BUILD_NLS),en,dis)abled \
	-Doss_fuzz=disabled \
	-Dglib_debug=disabled \
	-Dglib_assert=false \
	-Dglib_checks=false \
	-Dlibelf=disabled

MESON_HOST_ARGS += $(COMP_ARGS) -Dxattr=false
MESON_ARGS += $(COMP_ARGS) -Dxattr=true -Db_lto=true

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/* \
		$(1)/usr/bin/

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/glib-2.0 \
		$(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/glib-2.0/include/*.h \
		$(1)/usr/include/glib-2.0/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/gio-unix-2.0 \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/glib-2.0 \
		$(1)/usr/lib/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.{so*,a} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
ifneq ($(INTL_FULL),)
	$(SED) '/^Libs:/s/$$$$/ -lintl/' \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/glib-2.0.pc
endif
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig

	$(INSTALL_DIR) $(2)/share/aclocal/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/aclocal/*.m4  \
		$(2)/share/aclocal/

	$(INSTALL_DIR) $(1)/usr/share/glib-2.0
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/glib-2.0/codegen \
		$(1)/usr/share/glib-2.0/
endef

define Package/glib2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,glib2))
