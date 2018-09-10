include $(TOPDIR)/rules.mk
#Based on adb package from AUR https://aur.archlinux.org/packages/adb/ , reused Makefile

PKG_NAME:=adb
PKG_VERSION:=android.5.0.2_r1
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://android.googlesource.com/platform/system/core
PKG_SOURCE_VERSION:=6fe92d1a3fb17545d82d020a3c995f32e6b71f9d
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.xz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)
PKG_MAINTAINER:=Henryk Heisig <hyniu@o2.pl>

include $(INCLUDE_DIR)/package.mk

ifeq ($(CONFIG_BIG_ENDIAN),y)
TARGET_CFLAGS+= -DHAVE_BIG_ENDIAN=1
endif
TARGET_CFLAGS+= -D_GNU_SOURCE

define Package/adb
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Android Debug Bridge CLI tool
  URL:=http://tools.android.com/
  DEPENDS:=+zlib +libopenssl +libpthread
endef

define Package/adb/description
 Android Debug Bridge (adb) is a versatile command line tool that lets you communicate with an emulator instance or connected Android-powered device.
endef

# Nothing just to be sure
#define Build/Configure
#endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/adb/ \
		$(TARGET_CONFIGURE_OPTS) \
		TARGET=Linux \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/adb/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/adb/adb $(1)/usr/bin/
endef

$(eval $(call BuildPackage,adb))
