#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=v2ray
PKG_VERSION:=v4.13.0
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=V2Ray is a cross-platform proxy software
	DEPENDS:=+ca-certificates 
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
	depends on mipsel || mips || mips64el || mips64
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

V2RAY_BIN:=v2ray
V2CTL_BIN:=v2ctl

ifeq ($(ARCH),arm)
	ifneq ($(BOARD),bcm53xx)
		V2RAY_BIN:=v2ray_armv7
		V2CTL_BIN:=v2ctl_armv7
	endif
endif

ifdef CONFIG_PACKAGE_V2RAY_SOFTFLOAT
	V2RAY_BIN:=v2ray_softfloat
	V2CTL_BIN:=v2ctl_softfloat
endif


define Build/Prepare
	[ ! -f $(PKG_BUILD_DIR)/v2ray-$(PKG_VERSION)-$(PKG_ARCH_V2RAY).zip ] && wget https://github.com/v2ray/v2ray-core/releases/download/$(PKG_VERSION)/v2ray-$(PKG_ARCH_V2RAY).zip -O $(PKG_BUILD_DIR)/v2ray-$(PKG_VERSION)-$(PKG_ARCH_V2RAY).zip
	unzip -o $(PKG_BUILD_DIR)/v2ray-$(PKG_VERSION)-$(PKG_ARCH_V2RAY).zip -d $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
	chmod +x $(PKG_BUILD_DIR)/$(V2RAY_BIN) && upx --ultra-brute $(PKG_BUILD_DIR)/$(V2RAY_BIN)
	chmod +x $(PKG_BUILD_DIR)/$(V2CTL_BIN) && upx --ultra-brute $(PKG_BUILD_DIR)/$(V2CTL_BIN)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/v2ray
ifdef CONFIG_PACKAGE_V2RAY_INCLUDE_V2RAY
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(V2RAY_BIN) $(1)/usr/bin/v2ray/v2ray
endif
ifdef CONFIG_PACKAGE_V2RAY_INCLUDE_V2CTL
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(V2CTL_BIN) $(1)/usr/bin/v2ray/v2ctl
endif
ifdef CONFIG_PACKAGE_V2RAY_INCLUDE_GEOIP
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/geoip.dat $(1)/usr/bin/v2ray/
endif
ifdef CONFIG_PACKAGE_V2RAY_INCLUDE_GEOSITE
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/geosite.dat $(1)/usr/bin/v2ray/
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
