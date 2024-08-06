include $(TOPDIR)/rules.mk

PKG_NAME:=fitblk
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0-only
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/fitblk
  HIDDEN:=1
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=fitblk firmware release tool
  DEPENDS:=@!LINUX_5_15
endef

define Package/fitblk/description
Release uImage.FIT block devices using ioctl.
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall -Werror" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/fitblk/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fitblk $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,fitblk))
