#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=e2fsprogs
PKG_CPE_ID:=cpe:/a:e2fsprogs_project:e2fsprogs
PKG_VERSION:=1.47.2
PKG_HASH:=08242e64ca0e8194d9c1caad49762b19209a06318199b63ce74ae4ef2d74e63c

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/kernel/people/tytso/e2fsprogs/v$(PKG_VERSION)/

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

ifneq ($(shell $(HOSTCC) --version | grep clang),)
  HOST_CFLAGS += -D__GNUC_PREREQ\(...\)=0 -Dloff_t=off_t
endif
HOST_CFLAGS += $(HOST_FPIC)

HOST_CONFIGURE_ARGS += \
	--disable-elf-shlibs \
	--disable-libuuid \
	--disable-tls \
	--disable-nls \
	--enable-threads=pth \
	--disable-fuse2fs \
	--with-crond-dir=no

# The following uses pkg-config the wrong way around. Just override it.
HOST_CONFIGURE_VARS += \
	ac_cv_lib_uuid_uuid_generate=yes

define Host/Prepare
	$(call Host/Prepare/Default)
	rm -rf $(HOST_BUILD_DIR)/doc
endef

define Host/Install
	$(call Host/Install/Default)
	$(SED) 's|^DIR=.*|DIR=$$$${STAGING_DIR_HOST}/share/et|' $(STAGING_DIR_HOST)/bin/compile_et
	$(SED) 's|^DIR=.*|DIR=$$$${STAGING_DIR_HOST}/share/ss|' $(STAGING_DIR_HOST)/bin/mk_cmds
endef

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
