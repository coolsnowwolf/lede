include $(TOPDIR)/rules.mk

PKG_NAME:=libbsd
PKG_VERSION:=0.3.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=fbf36ed40443e1d0d795adbae8d461952509e610c3ccf0866ae160b723f7fe38
PKG_SOURCE_URL:=http://libbsd.freedesktop.org/releases

PKG_LICENSE:=BSD-4-Clause
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

PKG_INSTALL:=1

define Package/libbsd
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=@USE_GLIBC
  TITLE:=common BSD library
endef

define Package/libbsd/description
 This library provides useful functions commonly found on BSD systems, and lacking on others like GNU systems, thus making it easier to port projects with strong BSD origins, without needing to embed the same code over and over again on each project.
endef

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/lib \
		$(1)/usr/include

	$(CP) \
		$(PKG_INSTALL_DIR)/lib/libbsd.so* \
		$(1)/lib/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/* \
		$(1)/usr/include/

	( cd $(1)/lib ; $(LN) libbsd.so.$(PKG_VERSION) libbsd.so )
endef

define Package/libbsd/install
	$(INSTALL_DIR) \
		$(1)/lib

	$(CP) \
		$(PKG_INSTALL_DIR)/lib/libbsd.so* \
		$(1)/lib/

	( cd $(1)/lib ; $(LN) libbsd.so.$(PKG_VERSION) libbsd.so )
endef

$(eval $(call BuildPackage,libbsd))

