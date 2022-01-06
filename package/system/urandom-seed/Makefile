include $(TOPDIR)/rules.mk

PKG_NAME:=urandom-seed
PKG_RELEASE:=3
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define Package/urandom-seed
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+getrandom
  TITLE:=/etc/urandom.seed handling for OpenWrt
  URL:=https://openwrt.org/
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile/Default
endef
Build/Compile = $(Build/Compile/Default)

define Package/urandom-seed/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,urandom-seed))
