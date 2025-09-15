include $(TOPDIR)/rules.mk

PKG_NAME:=sendat
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ouyangzq/sendat.git
PKG_SOURCE_DATE:=2023-10-26
PKG_SOURCE_VERSION:=c830885db5bb8ae2dcb10b1bfeab481f14935dd1
PKG_MIRROR_HASH:=898a3e98babcafb9f200b80f840dfc655d44df7471180d093fffd00c10194360

MAKE_PATH:=src

include $(INCLUDE_DIR)/package.mk

define Package/sendat
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Send AT Commnad
  DEPENDS:=+libpthread
endef

define Package/sendat/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/sendat $(1)/usr/bin/
endef

$(eval $(call BuildPackage,sendat))
