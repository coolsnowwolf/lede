#
# hua.shao@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uci2dat
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_CONFIG_DEPENDS:=

PKG_MAINTAINER:=Hua Shao <nossiac@163.com>

include $(INCLUDE_DIR)/package.mk

define Package/uci2dat
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  DEPENDS:=+libuci
  TITLE:=Translate uci config into MTK/Ralink WiFi profile
  SUBMENU:=Applications
endef

define Package/uci2dat/description
  Translate uci config into ralink wifi dat format
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -luci -Wall
TARGET_CFLAGS += -Wno-error=format-security 

define Build/Configure
endef

define Package/uci2dat/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uci2dat $(1)/usr/bin
endef


$(eval $(call BuildPackage,uci2dat))
