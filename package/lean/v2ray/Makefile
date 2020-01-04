include $(TOPDIR)/rules.mk

PKG_NAME:=v2ray
PKG_VERSION:=4.22.1
PKG_MAINTAINER:=kslr

ifeq ($(ARCH),i386)
	PKG_ARCH:=32
endif

ifeq ($(ARCH),x86_64)
	PKG_ARCH:=64
endif

ifeq ($(ARCH),arm)
	PKG_ARCH:=arm
endif

ifeq ($(ARCH),arm)
	PKG_ARCH:=arm
	ifneq ($(BOARD),bcm53xx)
		V2RAY_BIN:=_armv7
  endif
  ifeq ($(BOARD),kirkwood)
		V2RAY_BIN:=
  endif
endif

ifeq ($(ARCH),aarch64)
	PKG_ARCH:=arm64
endif

ifeq ($(ARCH),mips)
	PKG_ARCH:=mips
endif

ifeq ($(ARCH),mipsel)
	PKG_ARCH:=mipsle
endif

PKG_SOURCE:=v2ray-linux-$(PKG_ARCH).zip
PKG_SOURCE_URL:=https://github.com/v2ray/v2ray-core/releases/download/v$(PKG_VERSION)/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
UNZIP_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/$(PKG_NAME)-unzip
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=V2Ray is a cross-platform proxy software
	DEPENDS:=+ca-certificates 
	URL:=https://github.com/v2ray/v2ray-core
endef

define Package/$(PKG_NAME)/description
	V2Ray is a cross-platform proxy software
endef

define Build/Prepare
	mkdir -vp $(UNZIP_DIR)
	unzip -od $(UNZIP_DIR) $(DL_DIR)/$(PKG_SOURCE)
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/v2ray
	$(INSTALL_BIN) $(UNZIP_DIR)/v2ray$(V2RAY_BIN) $(1)/usr/bin/v2ray/v2ray
#	$(INSTALL_BIN) $(UNZIP_DIR)/v2ctl$(V2RAY_BIN) $(1)/usr/bin/v2ray/v2ctl
#	$(INSTALL_BIN) $(UNZIP_DIR)/geosite.dat $(1)/usr/bin/v2ray/geosite.dat
#	$(INSTALL_BIN) $(UNZIP_DIR)/geoip.dat $(1)/usr/bin/v2ray/geoip.dat
	chmod 755 $(1)/usr/bin/v2ray/*
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
