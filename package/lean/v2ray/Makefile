#
# Copyright (C) 2019-2020 Xingwang Liao
# Copyright (C) 2019-2020 Mattraks
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=v2ray
PKG_VERSION:=4.28.1
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/v2ray-core-$(PKG_VERSION)

PKG_SOURCE:=v2ray-core-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/v2fly/v2ray-core/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=9f61500c4a262f8d38833d11218baf73a94bd7592bc89528aa7824ff6192c9e5

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Xingwang Liao <kuoruan@gmail.com>

PKG_CONFIG_DEPENDS := \
	CONFIG_V2RAY_JSON_V2CTL \
	CONFIG_V2RAY_JSON_INTERNAL \
	CONFIG_V2RAY_JSON_NONE \
	CONFIG_V2RAY_EXCLUDE_V2CTL \
	CONFIG_V2RAY_EXCLUDE_ASSETS \
	CONFIG_V2RAY_COMPRESS_GOPROXY \
	CONFIG_V2RAY_COMPRESS_UPX \
	CONFIG_V2RAY_DISABLE_NONE \
	CONFIG_V2RAY_DISABLE_CUSTOM \
	CONFIG_V2RAY_DISABLE_DNS \
	CONFIG_V2RAY_DISABLE_LOG \
	CONFIG_V2RAY_DISABLE_POLICY \
	CONFIG_V2RAY_DISABLE_REVERSE \
	CONFIG_V2RAY_DISABLE_ROUTING \
	CONFIG_V2RAY_DISABLE_STATISTICS \
	CONFIG_V2RAY_DISABLE_BLACKHOLE_PROTO \
	CONFIG_V2RAY_DISABLE_DNS_PROXY \
	CONFIG_V2RAY_DISABLE_DOKODEMO_PROTO \
	CONFIG_V2RAY_DISABLE_FREEDOM_PROTO \
	CONFIG_V2RAY_DISABLE_MTPROTO_PROXY \
	CONFIG_V2RAY_DISABLE_HTTP_PROTO \
	CONFIG_V2RAY_DISABLE_SHADOWSOCKS_PROTO \
	CONFIG_V2RAY_DISABLE_SOCKS_PROTO \
	CONFIG_V2RAY_DISABLE_VMESS_PROTO \
	CONFIG_V2RAY_DISABLE_TCP_TRANS \
	CONFIG_V2RAY_DISABLE_MKCP_TRANS \
	CONFIG_V2RAY_DISABLE_WEBSOCKET_TRANS \
	CONFIG_V2RAY_DISABLE_HTTP2_TRANS \
	CONFIG_V2RAY_DISABLE_DOMAIN_SOCKET_TRANS \
	CONFIG_V2RAY_DISABLE_QUIC_TRANS

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=v2ray.com/core
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:= \
	v2ray.com/core.version=$(PKG_VERSION) \
	v2ray.com/core.build=Lean \
	v2ray.com/core.codename=OpenWrt

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/$(PKG_NAME)
  TITLE:=A platform for building proxies
  URL:=https://www.v2ray.com
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Project V
  DEPENDS:=$(GO_ARCH_DEPENDS) +ca-certificates
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define Package/$(PKG_NAME)/description
Project V is a set of network tools that help you to build your own computer network.
It secures your network connections and thus protects your privacy.

  This package contains v2ray, v2ctl and v2ray-assets.
endef

ifeq ($(CONFIG_V2RAY_COMPRESS_GOPROXY),y)
export GO111MODULE=on
export GOPROXY=https://goproxy.io
#export GOPROXY=https://mirrors.aliyun.com/goproxy/
endif

V2RAY_SED_ARGS:=

ifeq ($(CONFIG_V2RAY_JSON_INTERNAL),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/main\/json"/\/\/ &/; \
	/\/\/ _ "v2ray.com\/core\/main\/jsonem"/s/\/\/ //;
else ifeq ($(CONFIG_V2RAY_JSON_NONE),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/main\/json"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_CUSTOM),y)

ifeq ($(CONFIG_V2RAY_DISABLE_DNS),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/app\/dns"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_LOG),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/app\/log"/\/\/ &/; \
	s/_ "v2ray.com\/core\/app\/log\/command"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_POLICY),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/app\/policy"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_REVERSE),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/app\/reverse"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_ROUTING),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/app\/router"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_STATISTICS),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/app\/stats"/\/\/ &/; \
	s/_ "v2ray.com\/core\/app\/stats\/command"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_BLACKHOLE_PROTO),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/proxy\/blackhole"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_DNS_PROXY),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/proxy\/dns"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_DOKODEMO_PROTO),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/proxy\/dokodemo"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_FREEDOM_PROTO),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/proxy\/freedom"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_MTPROTO_PROXY),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/proxy\/mtproto"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_HTTP_PROTO),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/proxy\/http"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_SHADOWSOCKS_PROTO),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/proxy\/shadowsocks"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_SOCKS_PROTO),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/proxy\/socks"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_VMESS_PROTO),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/proxy\/vmess\/inbound"/\/\/ &/; \
	s/_ "v2ray.com\/core\/proxy\/vmess\/outbound"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_TCP_TRANS),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/transport\/internet\/tcp"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_MKCP_TRANS),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/transport\/internet\/kcp"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_WEBSOCKET_TRANS),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/transport\/internet\/websocket"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_HTTP2_TRANS),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/transport\/internet\/http"/\/\/ &/; \
	s/_ "v2ray.com\/core\/transport\/internet\/headers\/http"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_DOMAIN_SOCKET_TRANS),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/transport\/internet\/domainsocket"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_QUIC_TRANS),y)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/transport\/internet\/quic"/\/\/ &/;
endif

ifeq ($(CONFIG_V2RAY_DISABLE_MKCP_TRANS)$(CONFIG_V2RAY_DISABLE_QUIC_TRANS),yy)
V2RAY_SED_ARGS += \
	s/_ "v2ray.com\/core\/transport\/internet\/headers\/noop"/\/\/ &/; \
	s/_ "v2ray.com\/core\/transport\/internet\/headers\/srtp"/\/\/ &/; \
	s/_ "v2ray.com\/core\/transport\/internet\/headers\/tls"/\/\/ &/; \
	s/_ "v2ray.com\/core\/transport\/internet\/headers\/utp"/\/\/ &/; \
	s/_ "v2ray.com\/core\/transport\/internet\/headers\/wechat"/\/\/ &/; \
	s/_ "v2ray.com\/core\/transport\/internet\/headers\/wireguard"/\/\/ &/;
endif

endif

GEOIP_VER:=latest
GEOIP_FILE:=geoip-$(GEOIP_VER).dat

define Download/geoip.dat
  URL:=https://github.com/v2ray/geoip/releases/$(GEOIP_VER)/download
  URL_FILE:=geoip.dat
  FILE:=$(GEOIP_FILE)
  HASH:=skip
endef

GEOSITE_VER:=latest
GEOSITE_FILE:=geosite-$(GEOSITE_VER).dat

define Download/geosite.dat
  URL:=https://github.com/v2ray/domain-list-community/releases/$(GEOSITE_VER)/download
  URL_FILE:=dlc.dat
  FILE:=$(GEOSITE_FILE)
  HASH:=skip
endef

define Build/Prepare
	$(call Build/Prepare/Default)

ifneq ($(CONFIG_V2RAY_EXCLUDE_ASSETS),y)
	# move file to make sure download new file every build
	mv -f $(DL_DIR)/$(GEOIP_FILE) $(PKG_BUILD_DIR)/release/config/geoip.dat
	mv -f $(DL_DIR)/$(GEOSITE_FILE) $(PKG_BUILD_DIR)/release/config/geosite.dat
endif

ifneq ($(V2RAY_SED_ARGS),)
	( \
		sed -i \
			'$(V2RAY_SED_ARGS)' \
			$(PKG_BUILD_DIR)/main/distro/all/all.go ; \
	)
endif
endef

define Build/Compile
	$(eval GO_PKG_BUILD_PKG:=v2ray.com/core/main)
	$(call GoPackage/Build/Compile)
	mv -f $(GO_PKG_BUILD_BIN_DIR)/main $(GO_PKG_BUILD_BIN_DIR)/v2ray

ifeq ($(CONFIG_V2RAY_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/v2ray
endif

ifneq ($(CONFIG_V2RAY_EXCLUDE_V2CTL),y)
	$(eval GO_PKG_BUILD_PKG:=v2ray.com/core/infra/control/main)
	$(call GoPackage/Build/Compile)
	mv -f $(GO_PKG_BUILD_BIN_DIR)/main $(GO_PKG_BUILD_BIN_DIR)/v2ctl

ifeq ($(CONFIG_V2RAY_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/v2ctl
endif
endif
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/v2ray

	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/v2ray $(1)/usr/bin/v2ray

ifneq ($(CONFIG_V2RAY_EXCLUDE_V2CTL),y)
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/v2ctl $(1)/usr/bin/v2ray
endif

ifneq ($(CONFIG_V2RAY_EXCLUDE_ASSETS),y)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/release/config/{geoip,geosite}.dat $(1)/usr/bin/v2ray
endif
endef

ifneq ($(CONFIG_V2RAY_EXCLUDE_ASSETS),y)
$(eval $(call Download,geoip.dat))
$(eval $(call Download,geosite.dat))
endif

$(eval $(call GoBinPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
