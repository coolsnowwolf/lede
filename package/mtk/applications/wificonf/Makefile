#
# Hua Shao <nossiac@163.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wificonf
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_CONFIG_DEPENDS:=

PKG_MAINTAINER:=Hua Shao <nossiac@163.com>

include $(INCLUDE_DIR)/package.mk

define Package/wificonf
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=Read/Write MTK WiFi profiles
  DEPENDS:=
  SUBMENU:=Applications
  MENU:=1
endef

define Package/wificonf/description
  Read/Write MTK WiFi profiles
endef

define Package/wificonf/config
	if PACKAGE_wificonf
		config SUPPORT_LSDK_NVRAM_CMD
			bool "Provide compatible usage for LSDK nvram command"
			default n
	endif
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	make -C src clean
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -Wall
TARGET_CFLAGS += -Wno-error=format-security

define Package/wificonf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wificonf $(1)/usr/bin
	if [ "$$(CONFIG_SUPPORT_LSDK_NVRAM_CMD)" != "" ]; then \
		$(INSTALL_BIN) ./files/usr/bin/nvram_get $(1)/usr/bin/nvram_get; \
		$(LN) /usr/bin/nvram_get $(1)/usr/bin/nvram_set; \
	fi
endef

$(eval $(call BuildPackage,wificonf))
