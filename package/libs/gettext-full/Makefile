#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gettext-full
PKG_VERSION:=0.22.5
PKG_RELEASE:=1

PKG_SOURCE:=gettext-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/gettext
PKG_HASH:=fe10c37353213d78a5b83d48af231e005c4da84db5ce88037d88355938259640
PKG_BUILD_DIR:=$(BUILD_DIR)/gettext-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/gettext-$(PKG_VERSION)

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-3.0-or-later
PKG_CPE_ID:=cpe:/a:gnu:gettext

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=gettext-full/host libunistring libxml2
PKG_BUILD_PARALLEL:=0

PKG_FIXUP:=autoreconf

HOST_BUILD_DEPENDS:=gperf/host libiconv-full/host libunistring/host libxml2/host
HOST_BUILD_PARALLEL:=0

PKG_SUBDIRS:= \
	$$$$(foreach dir, \
		intl \
		intl-csharp \
		intl-java \
		its \
		gnulib-lib \
		gnulib-local \
		lib \
		$$$$(SUBDIR_libasprintf) \
		libgettextpo \
		libgrep \
		libtextstyle \
		m4 \
		misc \
		po \
		projects \
		src \
		styles \
		gettext-runtime \
		gettext-tools \
	,$$$$(wildcard $$$$(dir)) )

PKG_MAKEFILES_FAKE:= \
	gnulib-local/m4/Makefile \
	libtextstyle/gnulib-local/Makefile \
	libtextstyle/m4/Makefile \
	m4/Makefile

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/libintl-full
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GNU Internationalization library
  URL:=http://www.gnu.org/software/gettext/
  ABI_VERSION:=8
endef

export GNULIB_SRCDIR:=$(HOST_GNULIB_SRCDIR)

TARGET_CFLAGS += $(FPIC)
ifneq ($(HOST_OS),Linux)
  TARGET_CFLAGS += -I$(STAGING_DIR_HOSTPKG)/include
endif

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-libasprintf \
	--disable-rpath \
	--enable-nls \
	--disable-java \
	--disable-openmp \
	--disable-curses \
	--with-included-gettext \
	--without-libintl-prefix \
	--without-libexpat-prefix \
	--with-libunistring-prefix=$(STAGING_DIR)/usr \
	--with-libxml2-prefix=$(STAGING_DIR) \
	--without-emacs

HOST_CONFIGURE_ARGS += \
	--disable-shared \
	--enable-static \
	--disable-libasprintf \
	--disable-rpath \
	--disable-java \
	--disable-openmp \
	--without-emacs \
	--with-libiconv-prefix=$(STAGING_DIR_HOSTPKG) \
	--with-libunistring-prefix=$(STAGING_DIR_HOSTPKG) \
	--with-libxml2-prefix=$(STAGING_DIR_HOSTPKG)

HOST_CONFIGURE_VARS += \
	EMACS="no" \

HOST_CFLAGS += $(HOST_FPIC)

define Host/Bootstrap
	( \
		cd $(HOST_BUILD_DIR); \
		$(AM_TOOL_PATHS) \
		./autogen.sh \
	)
endef

define Host/Prepare
	$(call Host/Prepare/Default)
	$(if $(QUILT),,$(call Host/Bootstrap))
endef

define Host/Configure
	$(if $(QUILT),$(call Host/Bootstrap))
	$(call Host/Configure/Default)
endef

define Host/Compile
	$(foreach makefile,$(PKG_MAKEFILES_FAKE),printf '%s\n%s\n' 'all:' 'install:' > $(HOST_BUILD_DIR)/$(makefile); )
	$(call Host/Compile/Default,SUBDIRS='$(PKG_SUBDIRS)')
endef

define Build/Bootstrap
	( \
		cd $(PKG_BUILD_DIR); \
		$(AM_TOOL_PATHS) \
		./autogen.sh \
	)
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(if $(QUILT),,$(call Build/Bootstrap))
endef

define Build/Configure
	$(if $(QUILT),$(call Build/Bootstrap))
	$(call Build/Configure/Default)
endef

define Build/Compile
	$(foreach makefile,$(PKG_MAKEFILES_FAKE),printf '%s\n%s\n' 'all:' 'install:' > $(PKG_BUILD_DIR)/$(makefile); )
	$(call Build/Compile/Default,SUBDIRS='$(PKG_SUBDIRS)')
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/libintl-full/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libintl.h $(1)/usr/lib/libintl-full/include/

	$(INSTALL_DIR) $(1)/usr/lib/libintl-full/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libintl.{a,so*} $(1)/usr/lib/libintl-full/lib/

	$(INSTALL_DIR) $(1)/usr/share/aclocal
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/aclocal/* $(1)/usr/share/aclocal/

	$(SED) '/read dummy/d' $(STAGING_DIR_HOSTPKG)/bin/gettextize
endef

define Host/Install
	$(call Host/Compile/Default,install SUBDIRS='$(PKG_SUBDIRS)')
	$(LN) msgfmt $(STAGING_DIR_HOSTPKG)/bin/gmsgfmt
endef

define Build/Install
	$(call Build/Install/Default,install SUBDIRS='$(PKG_SUBDIRS)')
endef

define Package/libintl-full/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libintl.so.* $(1)/usr/lib/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,libintl-full))
