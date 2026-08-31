include $(TOPDIR)/rules.mk
#Based on adb package from AUR https://aur.archlinux.org/packages/adb/ , reused Makefile

PKG_NAME:=adb
PKG_SOURCE_VERSION:=6fe92d1a3fb17545d82d020a3c995f32e6b71f9d
PKG_VERSION:=5.0.2~$(call version_abbrev,$(PKG_SOURCE_VERSION))
PKG_RELEASE:=4

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://android.googlesource.com/platform/system/core
PKG_MIRROR_HASH:=2ff96b4342cd05f475083207a4927635548c6693771c12a24cfa99f175fdb10a

PKG_MAINTAINER:=Henryk Heisig <hyniu@o2.pl>
PKG_CPE_ID:=cpe:/a:google:android_debug_bridge

include $(INCLUDE_DIR)/package.mk

ifeq ($(CONFIG_BIG_ENDIAN),y)
TARGET_CFLAGS+= -DHAVE_BIG_ENDIAN=1
endif
TARGET_CFLAGS+= -D_GNU_SOURCE

define Package/adb
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Android Debug Bridge CLI tool
  URL:=https://developer.android.com/tools/adb
  DEPENDS:=+zlib +libmbedtls +libpthread
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
