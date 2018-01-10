#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uboot-envtools
PKG_DISTNAME:=u-boot
PKG_VERSION:=2015.10
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/u-boot-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_DISTNAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=\
	http://mirror2.openwrt.org/sources \
	ftp://ftp.denx.de/pub/u-boot
PKG_HASH:=bdc68d5f9455ad933b059c735d983f2c8b6b552dafb062e5ff1444f623021955

PKG_BUILD_DEPENDS:=+fstools

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

PKG_FLAGS:=nonshared

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/uboot-envtools
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Boot Loaders
  TITLE:=read/modify U-Boot bootloader environment
  URL:=http://www.denx.de/wiki/U-Boot
endef

define Package/uboot-envtools/description
 This package includes tools to read and modify U-Boot bootloader environment.
endef

define Package/uboot-envtools/config
	source "$(SOURCE)/Config.in"
endef

define Build/Configure
	touch $(PKG_BUILD_DIR)/include/config.mk
	touch $(PKG_BUILD_DIR)/include/config.h
	mkdir -p $(PKG_BUILD_DIR)/include/generated
	touch $(PKG_BUILD_DIR)/include/generated/autoconf.h
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		TARGET_CFLAGS="$(TARGET_CFLAGS)" \
		UBI="$(CONFIG_UBOOT_ENVTOOLS_UBI)" \
		dot-config=0 \
		HOSTLDFLAGS= \
		env
endef

define Package/uboot-envtools/conffiles
/etc/config/ubootenv
/etc/fw_env.config
endef

define Package/uboot-envtools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/env/fw_printenv $(1)/usr/sbin
	$(LN) fw_printenv $(1)/usr/sbin/fw_setenv
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DATA) ./files/uboot-envtools.sh $(1)/lib
ifneq ($(CONFIG_TARGET_ar71xx),)
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/ar71xx $(1)/etc/uci-defaults/30_uboot-envtools
endif
ifneq ($(CONFIG_TARGET_cns3xxx),)
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/cns3xxx $(1)/etc/uci-defaults/30_uboot-envtools
endif
ifneq ($(CONFIG_TARGET_imx6),)
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/imx6 $(1)/etc/uci-defaults/30_uboot-envtools
endif
ifneq ($(CONFIG_TARGET_ipq806x),)
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/ipq $(1)/etc/uci-defaults/30_uboot-envtools
endif
ifneq ($(CONFIG_TARGET_kirkwood),)
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/kirkwood $(1)/etc/uci-defaults/30_uboot-envtools
endif
ifneq ($(CONFIG_TARGET_lantiq),)
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/lantiq $(1)/etc/uci-defaults/30_uboot-envtools
endif
ifneq ($(CONFIG_TARGET_mvebu),)
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/mvebu $(1)/etc/uci-defaults/30_uboot-envtools
endif
ifneq ($(CONFIG_TARGET_mxs),)
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/mxs $(1)/etc/uci-defaults/30_uboot-envtools
endif
ifneq ($(CONFIG_TARGET_oxnas),)
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/oxnas $(1)/etc/uci-defaults/30_uboot-envtools
endif
ifneq ($(CONFIG_TARGET_ramips),)
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/ramips $(1)/etc/uci-defaults/30_uboot-envtools
endif
endef

$(eval $(call BuildPackage,uboot-envtools))
