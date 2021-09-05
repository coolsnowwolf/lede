include $(TOPDIR)/rules.mk

PKG_NAME:=libcgroup
PKG_VERSION:=0.41
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/libcg
PKG_HASH:=e4e38bdc7ef70645ce33740ddcca051248d56b53283c0dc6d404e17706f6fb51

PKG_MAINTAINER:=Daniel Danzberger <daniel@dd-wrt.com>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libcgroup
  TITLE:=CGroup config and exec library
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+USE_MUSL:musl-fts +@KERNEL_CGROUPS
endef

define Package/cgroup-tools
  TITLE:=CGroup config and exec tools
  DEPENDS:=+libcgroup
  CATEGORY:=Utilities
endef

define Package/libcgroup/description
  Helpers utils for working with cgroups.
endef

CONFIGURE_ARGS += \
	--enable-tools \
	--enable-shared \
	--disable-daemon \
	--disable-pam

TARGET_LDFLAGS += $(if $(CONFIG_USE_MUSL),-lfts)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libcgroup.h $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libcgroup $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcgroup.so* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libcgroup.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libcgroup/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcgroup.so* $(1)/usr/lib
endef

define Package/cgroup-tools/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/* $(1)/usr/sbin
endef

$(eval $(call BuildPackage,libcgroup))
$(eval $(call BuildPackage,cgroup-tools))
