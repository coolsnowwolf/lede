include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=mtk-luci-plugin
PKG_RELEASE:=1
PKG_BUILD_DEPENDS:=base-files
PKG_FILE_DEPENDS:=
PKG_LICENSE:=GPL-2.0

PKG_MAINTAINER:=Hua Shao <nossiac@163.com>

include $(INCLUDE_DIR)/package.mk

HTDOCS = /www
LUA_LIBRARYDIR = /usr/lib/lua
LUCI_LIBRARYDIR = $(LUA_LIBRARYDIR)/luci


define Package/mtk-luci-plugin
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Misc
  DEPENDS:=+lua
  TITLE:=MTK's LuCI plugins.
  VERSION:=$(PKG_RELEASE)-$(REVISION)
endef


define Package/mtk-luci-plugin/config
	config LUCI_APP_MTKWIFI
	bool "luci-app-mtkwifi"
	depends on PACKAGE_mtk-luci-plugin
	default y
	help
	  "LuCI plugin to manipulate MTK WiFi drivers"

	config LUCI_APP_WEBCONSOLE
	bool "luci-app-webconsole"
	depends on PACKAGE_mtk-luci-plugin
	default n
	help
	  "LuCI plugin for access shell from web"
endef

define Build/Compile

endef

define Package/common/install
	if [ -d $(2)/luasrc ]; then \
	  $(INSTALL_DIR) $(1)$(LUCI_LIBRARYDIR); \
	  cp -pR ./$(2)/luasrc/* $(1)$(LUCI_LIBRARYDIR)/; \
	else true; fi
	if [ -d $(2)/htdocs ]; then \
	  $(INSTALL_DIR) $(1)$(HTDOCS); \
	  cp -pR ./$(2)/htdocs/* $(1)$(HTDOCS)/; \
	else true; fi
	if [ -d $(2)/root ]; then \
	  $(INSTALL_DIR) $(1)/; \
	  cp -pR ./$(2)/root/* $(1)/; \
	else true; fi
	if [ -d $(2)/src ]; then \
	  $(call Build/Install/Default) \
	  $(CP) ./$(PKG_INSTALL_DIR)/* $(1)/; \
	else true; fi
endef


define Package/mtk-luci-plugin/install
	$(call Package/common/install,$(1),luci-app-mtkwifi)
	$(call Package/common/install,$(1),luci-app-webconsole)
endef


$(eval $(call BuildPackage,mtk-luci-plugin))
