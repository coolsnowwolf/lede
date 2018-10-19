#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=v2ray
PKG_VERSION:=v3.47
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

ifeq ($(ARCH),x86_64)
	PKG_ARCH_V2RAY:=linux-64
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_V2RAY:=linux-mipsle
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_V2RAY:=linux-mips
endif
ifeq ($(ARCH),i386)
	PKG_ARCH_V2RAY:=linux-32
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_V2RAY:=linux-arm
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_V2RAY:=linux-arm64
endif

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=V2Ray is a cross-platform proxy software
	DEPENDS:=
	URL:=https://github.com/v2ray/v2ray-core
endef

define Package/$(PKG_NAME)/config
menu "V2Ray Configuration"
	depends on PACKAGE_v2ray

config PACKAGE_V2RAY_INCLUDE_V2RAY
	bool "Include v2ray"
	default y

config PACKAGE_V2RAY_SOFTFLOAT
	bool "Use soft-float binaries (mips/mipsle only)"
	depends on PACKAGE_V2RAY_INCLUDE_V2RAY
	default n

config PACKAGE_V2RAY_INCLUDE_V2CTL
	bool "Include v2ctl"
	depends on PACKAGE_V2RAY_INCLUDE_V2RAY
	default y

config PACKAGE_V2RAY_INCLUDE_GEOIP
	bool "Include geoip.dat"
	depends on PACKAGE_V2RAY_INCLUDE_V2CTL
	default n

config PACKAGE_V2RAY_INCLUDE_GEOSITE
	bool "Include geosite.dat"
	depends on PACKAGE_V2RAY_INCLUDE_V2CTL
	default n

endmenu
endef

define Package/$(PKG_NAME)/description
V2Ray is a cross-platform proxy software
endef

define Build/Prepare
	
	wget https://github.com/v2ray/v2ray-core/releases/download/$(PKG_VERSION)/v2ray-$(PKG_ARCH_V2RAY).zip -O $(PKG_BUILD_DIR)/v2ray-$(PKG_VERSION)-$(PKG_ARCH_V2RAY).zip
	unzip -o $(PKG_BUILD_DIR)/v2ray-$(PKG_VERSION)-$(PKG_ARCH_V2RAY).zip -d $(PKG_BUILD_DIR)
ifdef CONFIG_PACKAGE_V2RAY_SOFTFLOAT
	[ -f $(PKG_BUILD_DIR)/v2ray_softfloat ] && mv $(PKG_BUILD_DIR)/v2ray_softfloat $(PKG_BUILD_DIR)/v2ray || echo "Can't find soft-float binary."
	[ -f $(PKG_BUILD_DIR)/v2ctl_softfloat ] && mv $(PKG_BUILD_DIR)/v2ctl_softfloat $(PKG_BUILD_DIR)/v2ctl || echo "Can't find soft-float binary."
endif
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/v2ray
ifdef CONFIG_PACKAGE_V2RAY_INCLUDE_V2RAY
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/v2ray $(1)/usr/bin/v2ray/
endif
ifdef CONFIG_PACKAGE_V2RAY_INCLUDE_V2CTL
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/v2ctl $(1)/usr/bin/v2ray/
endif
ifdef CONFIG_PACKAGE_V2RAY_INCLUDE_GEOIP
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/geoip.dat $(1)/usr/bin/v2ray/
endif
ifdef CONFIG_PACKAGE_V2RAY_INCLUDE_GEOSITE
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/geosite.dat $(1)/usr/bin/v2ray/
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
