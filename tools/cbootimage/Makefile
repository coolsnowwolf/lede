#
# Copyright (c) 2017-2019 Tomasz Maciej Nowak <tmn505@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME := cbootimage
PKG_VERSION := 1.8

PKG_SOURCE_PROTO := git
PKG_SOURCE_URL := https://github.com/NVIDIA/cbootimage.git
PKG_SOURCE_VERSION := 7c9db585d06cce9efffa2a82245f233233680060
PKG_MIRROR_HASH := 84d9abaaa3eddde05f506dc16effe1c9e18eb94727ed59c5e0a879baeb04e0b2

HOST_BUILD_PARALLEL := 1

include $(INCLUDE_DIR)/host-build.mk

define Host/Configure
	(cd $(HOST_BUILD_DIR); autoreconf --install --symlink)
	$(call Host/Configure/Default)
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/$(PKG_NAME) \
		$(STAGING_DIR_HOST)/share/man/man1/$(PKG_NAME).1
endef

$(eval $(call HostBuild))
