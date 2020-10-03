#
# Copyright (c) 2017-2019 Tomasz Maciej Nowak <tomek_n@o2.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME := cbootimage-configs

PKG_SOURCE_DATE := 2017-04-13
PKG_SOURCE_PROTO := git
PKG_SOURCE_URL := https://github.com/NVIDIA/cbootimage-configs.git
PKG_SOURCE_VERSION := 7c3b458b93ed6947cd083623f543e93f9103cc0f
PKG_MIRROR_HASH := 1d24421af8cf74ec2d625e237aa8121b1273774c4380ad333e2954e052a5a4fe

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/share/$(PKG_NAME)
	$(CP) $(HOST_BUILD_DIR)/* \
		$(STAGING_DIR_HOST)/share/$(PKG_NAME)
endef

define Host/Clean
	rm -fR $(STAGING_DIR_HOST)/share/$(PKG_NAME)
endef

$(eval $(call HostBuild))
