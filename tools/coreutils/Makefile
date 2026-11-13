#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=coreutils
PKG_CPE_ID:=cpe:/a:gnu:coreutils
PKG_VERSION:=9.6

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/coreutils
PKG_HASH:=2bec616375002c92c1ed5ead32a092b174fe44c14bc736d32e5961053b821d84

HOST_BUILD_PARALLEL := 1

PKG_PROGRAMS:=chown cp date dd ginstall ln readlink realpath rmdir seq stat touch

include $(INCLUDE_DIR)/host-build.mk

export GNULIB_SRCDIR:=$(HOST_GNULIB_SRCDIR)

HOST_GNULIB_SKIP := \
	lib/iconv_open-aix.gperf \
	lib/iconv_open-hpux.gperf \
	lib/iconv_open-irix.gperf \
	lib/iconv_open-osf.gperf \
	lib/iconv_open-solaris.gperf \
	lib/iconv_open-zos.gperf

HOST_CONFIGURE_ARGS += \
	 --enable-install-program=$(subst $(space),$(comma),$(strip $(PKG_PROGRAMS)))

HOST_MAKE_FLAGS += \
	$(AM_TOOL_PATHS) AUTOPOINT="$(TRUE)" GTKDOCIZE="$(TRUE)" \
	am__CONFIG_DISTCLEAN_FILES= \
	MAINTAINERCLEANFILES='$$$$(filter-out lib/iconv_open% %.texi,$$$$(BUILT_SOURCES))' \
	PROGRAMS="$(patsubst %,src/%,$(PKG_PROGRAMS))" \
	LIBRARIES= MANS= SUBDIRS=.

define Host/Bootstrap
	( \
		cd $(HOST_BUILD_DIR); \
		$(AM_TOOL_PATHS) AUTOPOINT="$(TRUE)" GTKDOCIZE="$(TRUE)" \
		./bootstrap \
			--bootstrap-sync \
			--force \
			--no-git \
			--skip-po \
			--gnulib-srcdir=$(GNULIB_SRCDIR) \
	)
endef

define Host/Prepare
	$(call Host/Prepare/Default)
	$(if $(QUILT),,$(call Host/Bootstrap))
endef

define Host/Configure
	$(if $(QUILT),$(call Host/Bootstrap))
	$(foreach src,$(HOST_GNULIB_SKIP),mv -f $(HOST_BUILD_DIR)/$(src)~ $(HOST_BUILD_DIR)/$(src) || true; )
	$(call Host/Configure/Default)
	$(call Host/Uninstall)
	$(call Host/Compile/Default,lib/config.h)
endef

define Host/Install
	$(INSTALL_DIR) $(1)/bin
	$(foreach prog,$(filter-out g%,$(PKG_PROGRAMS)),$(INSTALL_BIN) $(patsubst %,$(HOST_BUILD_DIR)/src/%,$(prog)) $(1)/bin/g$(prog); )
	$(foreach prog,$(filter g%,$(PKG_PROGRAMS)),$(INSTALL_BIN) $(patsubst %,$(HOST_BUILD_DIR)/src/%,$(prog)) $(1)/bin/$(prog); )
	$(foreach prog,$(filter-out g%,$(PKG_PROGRAMS)),$(LN) g$(prog) $(1)/bin/$(prog); )
	$(foreach prog,$(filter g%,$(PKG_PROGRAMS)),$(LN) $(prog) $(1)/bin/$(patsubst g%,%,$(prog)); )
endef

define Host/Uninstall
	#$(call Host/Compile/Default,uninstall) # Removes necessary symlinks
	-$(call Host/Compile/Default,maintainer-clean) # Clean bootstrap files from the release
endef

$(eval $(call HostBuild))
