include $(TOPDIR)/rules.mk

PKG_NAME:=wipe
PKG_VERSION:=0.24
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/berke/$(PKG_NAME)/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=aefb4403333562f9af5e3e03ecbba4b124e98788c688662240f9b04d80bfdb2f

PKG_MAINTAINER:=Toni Uhlig <matzeton@googlemail.com>
PKG_LICENSE:=GPL-2.0-only

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/wipe
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=securely erase files
  URL:=http://lambda-diode.com/software/wipe/
endef

define Package/wipe/description
  wipe is a little command for securely erasing files from magnetic media.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) linux \
		CC_LINUX="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)"
endef

define Package/wipe/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wipe $(1)/usr/bin/
endef

$(eval $(call BuildPackage,wipe))
