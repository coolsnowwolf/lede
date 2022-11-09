#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=fakeroot
PKG_VERSION:=1.30.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_SOURCE_URL:=@DEBIAN/pool/main/f/fakeroot
PKG_HASH:=32ebb1f421aca0db7141c32a8c104eb95d2b45c393058b9435fbf903dd2b6a75
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS += \
	ac_cv_header_sys_capability_h=no \
	ac_cv_func_capset=no \
	CPP="$(HOSTCC) -E"

HOST_CONFIGURE_ARGS += \
	--with-ipc=tcp

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
