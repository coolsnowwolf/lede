#
# <https://github.com/KFERMercer/luci-app-baidupcs-web>
# 
# Copyright (C) 2020 KFERMercer <KFER.Mercer@gmail.com>
# Copyright (C) 2020 [CTCGFW] Project OpenWRT
# 
# THIS IS FREE SOFTWARE, LICENSED UNDER GPLv3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-baidupcs-web
LUCI_TITLE:=LuCI support for BaiduPCS-Go
LUCI_DEPENDS:=+baidupcs-web +wget +openssl-util
LUCI_PKGARCH:=all
PKG_VERSION:=1.0
PKG_RELEASE:=5
PKG_MAINTAINER:=<https://github.com/KFERMercer/luci-app-baidupcs-web>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/baidupcs-web.po $(1)/usr/lib/lua/luci/i18n/baidupcs-web.zh-cn.lmo
endef
