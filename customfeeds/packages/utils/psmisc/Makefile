include $(TOPDIR)/rules.mk

PKG_NAME:=psmisc
PKG_VERSION:=23.4
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@SF/psmisc
PKG_HASH:=7f0cceeace2050c525f3ebb35f3ba01d618b8d690620580bdb8cd8269a0c1679

PKG_MAINTAINER:=Josef Schlehofer <pepe.schlehofer@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/psmisc
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=proc utilities
  URL:=https://gitlab.com/psmisc/psmisc/
  DEPENDS:=+libncurses
  ALTERNATIVES:=300:/usr/bin/killall:/usr/libexec/killall
endef

define Package/psmisc/description
  psmisc is a set of additional small useful utilities that use
  the proc filesystem like fuser, killall, prtstat, pstree.
endef

CONFIGURE_ARGS += \
	--disable-harden-flags

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)" \
	CPPFLAGS="$(TARGET_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

define Package/psmisc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{fuser,prtstat,pstree} $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/killall $(1)/usr/libexec
endef

$(eval $(call BuildPackage,psmisc))
