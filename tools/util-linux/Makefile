#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=util-linux
PKG_VERSION:=2.40.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/$(PKG_NAME)/v2.40
PKG_HASH:=d78b37a66f5922d70edf3bdfb01a6b33d34ed3c3cafd6628203b2a2b67c8e8b3
PKG_CPE_ID:=cpe:/a:kernel:util-linux

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--disable-all-programs \
	--enable-hexdump \
	--without-util \
	--without-selinux \
	--without-audit \
	--without-udev \
	--without-ncursesw \
	--without-ncurses \
	--without-slang \
	--without-tinfo \
	--without-readline \
	--without-utempter \
	--without-cap-ng \
	--without-libz \
	--without-libmagic \
	--without-user \
	--without-btrfs \
	--without-systemd \
	--without-smack \
	--without-econf \
	--without-python \
	--without-cryptsetup

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/hexdump $(STAGING_DIR_HOST)/bin/
endef

define Host/Uninstall
	rm -f $(STAGING_DIR_HOST)/bin/hexdump
endef

$(eval $(call HostBuild))
