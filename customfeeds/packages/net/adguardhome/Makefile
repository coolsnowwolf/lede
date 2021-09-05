#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=adguardhome
PKG_VERSION:=0.106.3
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/AdguardTeam/AdGuardHome
PKG_MIRROR_HASH:=bdf5ad833df90969cd82b9fb6a91846338c738ed01671fc768d867b592102f7c

PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=LICENSE.txt
PKG_MAINTAINER:=Dobroslaw Kijowski <dobo90@gmail.com>

PKG_BUILD_DEPENDS:=golang/host node/host node-yarn/host packr/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

PKG_CONFIG_DEPENDS:= \
	CONFIG_ADGUARDHOME_COMPRESS_GOPROXY \
	CONFIG_ADGUARDHOME_COMPRESS_UPX

GO_PKG:=github.com/AdguardTeam/AdGuardHome
GO_PKG_BUILD_PKG:=github.com/AdguardTeam/AdGuardHome

AGH_BUILD_TIME:=$(shell date -d @$(SOURCE_DATE_EPOCH) +%FT%TZ%z)
AGH_VERSION_PKG:=github.com/AdguardTeam/AdGuardHome/internal/version
GO_PKG_LDFLAGS_X:=$(AGH_VERSION_PKG).channel=release \
	$(AGH_VERSION_PKG).version=$(PKG_SOURCE_VERSION) \
	$(AGH_VERSION_PKG).buildtime=$(AGH_BUILD_TIME) \
	$(AGH_VERSION_PKG).goarm=$(GO_ARM) \
	$(AGH_VERSION_PKG).gomips=$(GO_MIPS)

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/adguardhome
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Network-wide ads and trackers blocking DNS server
	URL:=https://github.com/AdguardTeam/AdGuardHome
	DEPENDS:=$(GO_ARCH_DEPENDS) +ca-bundle
endef

define Package/adguardhome/conffiles
/etc/adguardhome.yaml
endef

define Package/adguardhome/description
Free and open source, powerful network-wide ads and trackers blocking DNS server.
endef

define Package/$(PKG_NAME)/config
config ADGUARDHOME_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config ADGUARDHOME_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default n
endef

ifeq ($(CONFIG_ADGUARDHOME_COMPRESS_GOPROXY),y)
export GO111MODULE=on
export GOPROXY=https://goproxy.baidu.com
endif

define Build/Compile
	( \
		pushd $(PKG_BUILD_DIR) ; \
		make js-deps js-build ; \
		packr -z -v -i internal ; \
		popd ; \
		$(call GoPackage/Build/Compile) ; \
	)
ifeq ($(CONFIG_ADGUARDHOME_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/AdGuardHome
endif
endef

#define Package/adguardhome/install
#	$(call GoPackage/Package/Install/Bin,$(1))
#	$(INSTALL_DIR) $(1)/etc/init.d
#	$(INSTALL_BIN) ./files/adguardhome.init $(1)/etc/init.d/adguardhome
#endef

$(eval $(call GoBinPackage,adguardhome))
$(eval $(call BuildPackage,adguardhome))
