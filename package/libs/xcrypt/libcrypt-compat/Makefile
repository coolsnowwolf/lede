include $(TOPDIR)/rules.mk
include ../libxcrypt-common.mk

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/libcrypt-compat/$(PKG_SOURCE_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/libcrypt-compat
$(Package/libxcrypt/Default)
  TITLE+= - libc compatibility
  DEPENDS:=@USE_GLIBC
endef

Package/libcrypt-compat/description=$(Package/libxcrypt/description)

CONFIGURE_ARGS += \
	--with-pic \
	--enable-year2038 \
	--disable-xcrypt-compat-files \
	--enable-obsolete-api=glibc \
	--enable-hashes=glibc

define Package/libcrypt-compat/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcrypt.so.* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcrypt.{a,la,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	# libcrypt.pc is symlink to libxcrypt.pc
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libxcrypt.pc $(1)/usr/lib/pkgconfig/libcrypt.pc
endef

$(eval $(call BuildPackage,libcrypt-compat))
