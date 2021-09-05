#
# Copyright (C) 2014-2015 OpenWrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mac-telnet
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/jow-/MAC-Telnet.git
PKG_SOURCE_DATE:=2015-09-02
PKG_SOURCE_VERSION:=37d83cbc9c07266d6c957d27bf75285b305eceab
PKG_MIRROR_HASH:=ed414f3fb08f1e2bb4313d00fb99327df697de7c60284fe588fc7eb3769adb7a

PKG_LICENSE:=GPL-2.0-or-later
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -ffunction-sections -fdata-sections $(if $(CONFIG_MACTELNET_PLAIN_SUPPORT),-DTELNET_SUPPORT)
TARGET_LDFLAGS += -Wl,--gc-sections

# 1: name
# 2: executable
define BuildPlugin
  define Package/mac-telnet-$(1)
    SECTION:=net
    CATEGORY:=Network
    DEPENDS:=+libubox
    TITLE:=MAC-Telnet / MAC-SSH $(1)
    URL:=https://github.com/jow-/MAC-Telnet
  endef

  define Package/mac-telnet-$(1)/description
    Open source MAC Telnet client and server utilities for connecting to
    Mikrotik RouterOS routers and Linux machines via MAC address.
  endef

  define Package/mac-telnet-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(2) $$(1)/usr/sbin/
	$(call Package/mac-telnet-$(1)/install-extra)
  endef

  $$(eval $$(call BuildPackage,mac-telnet-$(1)))
endef

define Package/mac-telnet-server/config
  config MACTELNET_PLAIN_SUPPORT
	bool "Include support for plain telnet connections"
	depends on PACKAGE_mac-telnet-server
        default y
	help
	  Disable this option to only support SSH logins to the
	  MAC-Telnet server.
endef

define Package/mac-telnet-server/install-extra
	$(INSTALL_DIR) $$(1)/etc/init.d
	$(INSTALL_BIN) ./files/mactelnet.init $$(1)/etc/init.d/mactelnet
	$(INSTALL_DIR) $$(1)/etc/config
	$(INSTALL_CONF) ./files/mactelnet.config $$(1)/etc/config/mactelnet
endef

define Package/mac-telnet-server/conffiles
/etc/config/mactelnet
endef


$(eval $(call BuildPlugin,server,mactelnetd))
$(eval $(call BuildPlugin,client,mactelnet))
$(eval $(call BuildPlugin,ping,macping))
$(eval $(call BuildPlugin,discover,mndp))
