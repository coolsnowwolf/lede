# Copyright (C) 2021 xiaorouji
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hysteria
PKG_VERSION:=0.8.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/HyNetwork/hysteria/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=8f1945eb36dba8ebc47e3f68af4ba421f65ca119cc0c54d552f59228ee824e89

PKG_CONFIG_DEPENDS:= \
	CONFIG_HYSTERIA_COMPRESS_GOPROXY \
	CONFIG_HYSTERIA_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/tobyxdd/hysteria
GO_PKG_BUILD_PKG:=$(GO_PKG)/cmd
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:= \
	main.appVersion=$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Web Servers/Proxies
	DEPENDS:=$$(GO_ARCH_DEPENDS)
	TITLE:=Hysteria is a feature-packed network utility optimized for networks of poor quality (e.g. satellite connections, congested public Wi-Fi, connecting from China to servers abroad)
	URL:=https://github.com/HyNetwork/hysteria
endef

define Package/$(PKG_NAME)/config

menu "Configuration"
	depends on PACKAGE_$(PKG_NAME)

config HYSTERIA_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config HYSTERIA_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y

endmenu

endef

ifeq ($(CONFIG_HYSTERIA_COMPRESS_GOPROXY),y)
export GO111MODULE=on
export GOPROXY=https://goproxy.io
endif

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_HYSTERIA_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/cmd
endif
endef

define Package/$(PKG_NAME)/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cmd $(1)/usr/bin/hysteria
endef

$(eval $(call GoBinPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
