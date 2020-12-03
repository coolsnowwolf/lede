#
# Copyright (C) 2019-2020 Xingwang Liao
# Copyright (C) 2019-2020 Mattraks
# Copyright (C) 2020 xiaorouji
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=xray
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/Xray-core-$(PKG_VERSION)

PKG_SOURCE:=xray-core-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/XTLS/xray-core/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=9f0e8f7a885f082780419ac7211a6e6150529d58fa5317ae4dfb61d3a7ba218c

PKG_LICENSE:=MPL
PKG_LICENSE_FILES:=LICENSE

PKG_CONFIG_DEPENDS:= \
	CONFIG_XRAY_JSON_INTERNAL \
	CONFIG_XRAY_JSON_NONE \
	CONFIG_XRAY_EXCLUDE_ASSETS \
	CONFIG_XRAY_COMPRESS_GOPROXY \
	CONFIG_XRAY_COMPRESS_UPX \
	CONFIG_XRAY_CUSTOM_FEATURES \
	CONFIG_XRAY_WITHOUT_DNS \
	CONFIG_XRAY_WITHOUT_LOG \
	CONFIG_XRAY_WITHOUT_TLS \
	CONFIG_XRAY_WITHOUT_UDP \
	CONFIG_XRAY_WITHOUT_POLICY \
	CONFIG_XRAY_WITHOUT_REVERSE \
	CONFIG_XRAY_WITHOUT_ROUTING \
	CONFIG_XRAY_WITHOUT_STATISTICS \
	CONFIG_XRAY_WITHOUT_BLACKHOLE_PROTO \
	CONFIG_XRAY_WITHOUT_DNS_PROXY \
	CONFIG_XRAY_WITHOUT_DOKODEMO_PROTO \
	CONFIG_XRAY_WITHOUT_FREEDOM_PROTO \
	CONFIG_XRAY_WITHOUT_MTPROTO_PROXY \
	CONFIG_XRAY_WITHOUT_HTTP_PROTO \
	CONFIG_XRAY_WITHOUT_SHADOWSOCKS_PROTO \
	CONFIG_XRAY_WITHOUT_SOCKS_PROTO \
	CONFIG_XRAY_WITHOUT_VMESS_PROTO \
	CONFIG_XRAY_WITHOUT_TCP_TRANS \
	CONFIG_XRAY_WITHOUT_MKCP_TRANS \
	CONFIG_XRAY_WITHOUT_WEBSOCKET_TRANS \
	CONFIG_XRAY_WITHOUT_HTTP2_TRANS \
	CONFIG_XRAY_WITHOUT_DOMAIN_SOCKET_TRANS \
	CONFIG_XRAY_WITHOUT_QUIC_TRANS

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/xtls/xray-core/v1
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:= \
	$(GO_PKG)/core.version=$(PKG_VERSION) \
	$(GO_PKG)/core.build=Lean \
	$(GO_PKG)/core.codename=OpenWrt

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/$(PKG_NAME)
  TITLE:=A platform for building proxies
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Project X
  DEPENDS:=$(GO_ARCH_DEPENDS) +ca-certificates
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

ifeq ($(CONFIG_XRAY_COMPRESS_GOPROXY),y)
export GO111MODULE=on
export GOPROXY=https://goproxy.io
#export GOPROXY=https://mirrors.aliyun.com/goproxy/
endif

XRAY_SED_ARGS:=

ifeq ($(CONFIG_XRAY_JSON_INTERNAL),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/main/json",// &,; \
	s,// \(_ "$(GO_PKG)/core/main/jsonem"\),\1,;
else ifeq ($(CONFIG_XRAY_JSON_NONE),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/main/json",// &,;
endif

ifeq ($(CONFIG_XRAY_CUSTOM_FEATURES),y)

ifeq ($(CONFIG_XRAY_WITHOUT_DNS),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/app/dns",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_LOG),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/app/log",// &,; \
	s,_ "$(GO_PKG)/core/app/log/command",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_TLS),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/transport/internet/tls",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_UDP),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/transport/internet/udp",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_POLICY),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/app/policy",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_REVERSE),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/app/reverse",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_ROUTING),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/app/router",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_STATISTICS),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/app/stats",// &,; \
	s,_ "$(GO_PKG)/core/app/stats/command",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_BLACKHOLE_PROTO),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/proxy/blackhole",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_DNS_PROXY),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/proxy/dns",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_DOKODEMO_PROTO),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/proxy/dokodemo",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_FREEDOM_PROTO),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/proxy/freedom",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_MTPROTO_PROXY),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/proxy/mtproto",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_HTTP_PROTO),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/proxy/http",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_SHADOWSOCKS_PROTO),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/proxy/shadowsocks",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_SOCKS_PROTO),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/proxy/socks",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_VMESS_PROTO),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/proxy/vmess/inbound",// &,; \
	s,_ "$(GO_PKG)/core/proxy/vmess/outbound",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_TCP_TRANS),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/transport/internet/tcp",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_MKCP_TRANS),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/transport/internet/kcp",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_WEBSOCKET_TRANS),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/transport/internet/websocket",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_HTTP2_TRANS),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/transport/internet/http",// &,; \
	s,_ "$(GO_PKG)/core/transport/internet/headers/http",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_DOMAIN_SOCKET_TRANS),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/transport/internet/domainsocket",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_QUIC_TRANS),y)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/transport/internet/quic",// &,;
endif

ifeq ($(CONFIG_XRAY_WITHOUT_MKCP_TRANS)$(CONFIG_XRAY_WITHOUT_QUIC_TRANS),yy)
XRAY_SED_ARGS += \
	s,_ "$(GO_PKG)/core/transport/internet/headers/noop",// &,; \
	s,_ "$(GO_PKG)/core/transport/internet/headers/srtp",// &,; \
	s,_ "$(GO_PKG)/core/transport/internet/headers/tls",// &,; \
	s,_ "$(GO_PKG)/core/transport/internet/headers/utp",// &,; \
	s,_ "$(GO_PKG)/core/transport/internet/headers/wechat",// &,; \
	s,_ "$(GO_PKG)/core/transport/internet/headers/wireguard",// &,;
endif

endif # custom features

GEOIP_VER:=latest
GEOIP_FILE:=geoip-$(GEOIP_VER).dat

define Download/geoip.dat
  URL:=https://github.com/v2fly/geoip/releases/$(GEOIP_VER)/download
  URL_FILE:=geoip.dat
  FILE:=$(GEOIP_FILE)
  HASH:=skip
endef

GEOSITE_VER:=latest
GEOSITE_FILE:=geosite-$(GEOSITE_VER).dat

define Download/geosite.dat
  URL:=https://github.com/v2fly/domain-list-community/releases/$(GEOSITE_VER)/download
  URL_FILE:=dlc.dat
  FILE:=$(GEOSITE_FILE)
  HASH:=skip
endef

define Build/Prepare
	$(call Build/Prepare/Default)

ifneq ($(CONFIG_XRAY_EXCLUDE_ASSETS),y)
	# move file to make sure download new file every build
	mv -f $(DL_DIR)/$(GEOIP_FILE) $(PKG_BUILD_DIR)/release/config/geoip.dat
	mv -f $(DL_DIR)/$(GEOSITE_FILE) $(PKG_BUILD_DIR)/release/config/geosite.dat
endif

ifneq ($(XRAY_SED_ARGS),)
	( \
		$(SED) \
			'$(XRAY_SED_ARGS)' \
			$(PKG_BUILD_DIR)/main/distro/all/all.go ; \
	)
endif
endef

define Build/Compile
	$(eval GO_PKG_BUILD_PKG:=$(GO_PKG)/main)
	$(call GoPackage/Build/Compile)
	mv -f $(GO_PKG_BUILD_BIN_DIR)/main $(GO_PKG_BUILD_BIN_DIR)/xray

ifeq ($(CONFIG_XRAY_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/xray || true
endif
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/xray

	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/xray $(1)/usr/bin/xray

ifneq ($(CONFIG_XRAY_EXCLUDE_ASSETS),y)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/release/config/{geoip,geosite}.dat $(1)/usr/bin/xray
endif
endef

ifneq ($(CONFIG_XRAY_EXCLUDE_ASSETS),y)
$(eval $(call Download,geoip.dat))
$(eval $(call Download,geosite.dat))
endif

$(eval $(call GoBinPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
