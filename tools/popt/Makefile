include $(TOPDIR)/rules.mk

PKG_NAME:=popt
PKG_VERSION:=1.16

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://rpm5.org/files/popt
PKG_HASH:=e728ed296fe9f069a0e005003c3d6b2dde3d9cad453422a10d6558616d304cc8

HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--disable-nls \
	--disable-rpath \
	--enable-shared=no \
	--without-libiconv-prefix \
	--without-libintl-prefix

$(eval $(call HostBuild))
