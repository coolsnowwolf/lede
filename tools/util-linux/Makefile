#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=util-linux
PKG_VERSION:=2.41.3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/$(PKG_NAME)/v2.41
PKG_HASH:=3330d873f0fceb5560b89a7dc14e4f3288bbd880e96903ed9b50ec2b5799e58b
PKG_CPE_ID:=cpe:/a:kernel:util-linux

PKG_FIXUP:=autoreconf

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--with-pic \
	--disable-shared \
	--disable-nls \
	--disable-all-programs \
	--enable-fdisks \
	--enable-hexdump \
	--enable-libblkid \
	--enable-libfdisk \
	--enable-libsmartcols \
	--enable-libuuid \
	--without-util \
	--without-selinux \
	--without-audit \
	--without-udev \
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

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
