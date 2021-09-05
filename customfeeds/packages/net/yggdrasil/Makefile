include $(TOPDIR)/rules.mk

PKG_NAME:=yggdrasil
PKG_VERSION:=0.3.16
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/yggdrasil-network/yggdrasil-go/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=e03595b78906b171155aaa11c922be3418bd056f8547e4d9f5123b6047316eac
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-go-$(PKG_VERSION)

PKG_MAINTAINER:=William Fleurant <meshnet@protonmail.com>
PKG_LICENSE:=LGPL-3.0-only
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/yggdrasil-network/yggdrasil-go
GO_PKG_BUILD_PKG:=github.com/yggdrasil-network/yggdrasil-go/cmd/...

GO_PKG_LDFLAGS_X:= \
  github.com/yggdrasil-network/yggdrasil-go/src/version.buildName=yggdrasil-openwrt \
  github.com/yggdrasil-network/yggdrasil-go/src/version.buildVersion=$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/yggdrasil
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Routing and Redirection
	TITLE:=Yggdrasil supports end-to-end encrypted IPv6 networks
	URL:=https://yggdrasil-network.github.io/
	DEPENDS:=$(GO_ARCH_DEPENDS) @IPV6 +kmod-tun +dkjson +libuci-lua
endef

define Package/yggdrasil/description
 Yggdrasil builds end-to-end encrypted networks with IPv6.
 Beyond the similarities with cjdns is a different routing
 algorithm. This globally-agreed spanning tree uses greedy
 routing in a metric space. Back-pressure routing techniques
 allow advanced link aggregation bonding on per-stream basis.
 In turn, a single stream will span across multiple network
 interfaces simultaneously with much greater throughput.
endef

define Package/yggdrasil/conffiles
/etc/config/yggdrasil
endef

define Package/yggdrasil/install
	$(INSTALL_DIR) \
		$(1)/etc/init.d \
		$(1)/etc/uci-defaults \
		$(1)/usr/sbin

	$(INSTALL_BIN) \
		$(GO_PKG_BUILD_BIN_DIR)/yggdrasil \
		$(1)/usr/sbin

	$(INSTALL_BIN) \
		$(GO_PKG_BUILD_BIN_DIR)/yggdrasilctl \
		$(1)/usr/sbin

	$(INSTALL_BIN) \
		./files/ygguci \
		$(1)/usr/sbin

	$(INSTALL_BIN) \
		./files/yggdrasil.defaults \
		$(1)/etc/uci-defaults/yggdrasil

	$(INSTALL_BIN) \
		./files/yggdrasil.init \
		$(1)/etc/init.d/yggdrasil
endef

$(eval $(call GoBinPackage,yggdrasil))
$(eval $(call BuildPackage,yggdrasil))
