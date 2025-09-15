#
# Copyright (C) 2012-2015 OpenWrt.org
# Copyright (C) 2016 LEDE-Project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=dosfstools
PKG_CPE_ID:=cpe:/a:dosfstools_project:dosfstools
PKG_VERSION:=4.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/dosfstools/dosfstools/releases/download/v$(PKG_VERSION)/ \
		http://fossies.org/linux/misc
PKG_HASH:=64926eebf90092dca21b14259a5301b7b98e7b1943e8a201c7d726084809b527

HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

ifeq ($(HOST_OS),Darwin)
HOST_CFLAGS += -UHAVE_ENDIAN_H
endif

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
