include $(TOPDIR)/rules.mk

PKG_NAME:=libmd
PKG_VERSION:=1.0.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://archive.hadrons.org/software/libmd/
PKG_HASH:=f51c921042e34beddeded4b75557656559cf5b1f2448033b4c1eec11c07e530f

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-static

define Package/libmd
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Message Digest functions from BSD systems
  ABI_VERSION:=0
endef

define Package/libmd/description
  This library provides message digest functions found on BSD systems either
  on their libc or libmd libraries and lacking on others like GNU systems,
  thus making it easier to port projects with strong BSD origins, without
  needing to embed the same code over and over again on each project.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmd.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libmd*.pc $(1)/usr/lib/pkgconfig/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
endef

$(eval $(call BuildPackage,libmd))
