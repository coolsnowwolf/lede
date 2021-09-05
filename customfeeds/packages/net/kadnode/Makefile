include $(TOPDIR)/rules.mk


PKG_NAME:=kadnode
PKG_VERSION:=2.3.0
PKG_RELEASE:=3

PKG_SOURCE_URL:=https://codeload.github.com/mwarning/KadNode/tar.gz/v$(PKG_VERSION)?
PKG_SOURCE:=kadnode-$(PKG_VERSION).tar.gz
PKG_HASH:=abb2ca66fb525fab53157d5486bbb43e3a522a4bdc9280a3dcb8cb403ee08583
PKG_BUILD_DIR:=$(BUILD_DIR)/KadNode-$(PKG_VERSION)

PKG_MAINTAINER:=Moritz Warning <moritzwarning@web.de>
PKG_LICENSE:=MIT

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/kadnode
	SECTION:=net
	CATEGORY:=Network
	TITLE:=P2P DNS Resolver
	SUBMENU:=IP Addresses and Names
	URL:=https://github.com/mwarning/KadNode
	MENU:=1
	DEPENDS:=+KADNODE_ENABLE_BOB:libmbedtls +KADNODE_ENABLE_TLS:libmbedtls +KADNODE_ENABLE_UPNP:libminiupnpc +KADNODE_ENABLE_NATPMP:libnatpmp
endef

define Package/kadnode/description
	A P2P DNS system based on the BitTorrent network. It can be used as a decentralized DynDNS service.
endef

define Package/kadnode/config
	source "$(SOURCE)/Config.in"
endef

FEATURES = 

ifeq ($(CONFIG_KADNODE_ENABLE_CMD),y)
FEATURES += cmd
endif

ifeq ($(CONFIG_KADNODE_ENABLE_LPD),y)
FEATURES += lpd
endif

ifeq ($(CONFIG_KADNODE_ENABLE_TLS),y)
FEATURES += tls
endif

ifeq ($(CONFIG_KADNODE_ENABLE_BOB),y)
FEATURES += bob
endif

ifeq ($(CONFIG_KADNODE_ENABLE_DNS),y)
FEATURES += dns
endif

ifeq ($(CONFIG_KADNODE_ENABLE_UPNP),y)
FEATURES += upnp
endif

ifeq ($(CONFIG_KADNODE_ENABLE_NATPMP),y)
FEATURES += natpmp
endif

ifeq ($(CONFIG_KADNODE_ENABLE_DEBUG),y)
FEATURES += debug
endif

MAKE_FLAGS += FEATURES="$(FEATURES)"

# Make binary smaller
TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

define Package/kadnode/conffiles
/etc/config/kadnode
endef

define Package/kadnode/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/kadnode $(1)/usr/bin/
ifeq ($(CONFIG_KADNODE_ENABLE_CMD),y)
	$(LN) kadnode $(1)/usr/bin/kadnode-ctl
endif
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/kadnode.init $(1)/etc/init.d/kadnode
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/kadnode.config $(1)/etc/config/kadnode
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) files/kadnode.postinst $(1)/etc/uci-defaults/99_kadnode
endef

$(eval $(call BuildPackage,kadnode))
