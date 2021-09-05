include $(TOPDIR)/rules.mk

PKG_NAME:=npth
PKG_VERSION:=1.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://gnupg.org/ftp/gcrypt/$(PKG_NAME)
PKG_HASH:=1393abd9adcf0762d34798dc34fdcf4d0d22a8410721e76f1e3afcd1daa4e2d1

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING.LIB

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libnpth
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=The New GNU Portable Threads Library
  URL:=https://gnupg.org/software/npth/index.html
  DEPENDS:=+libgpg-error
endef

define Package/libnpth/description
nPth is a library to provide the GNU Pth API and thus a non-preemptive threads implementation. 
endef

define Build/InstallDev
	$(INSTALL_DIR) $(2)/bin $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/npth-config \
		$(2)/bin/
	$(SED) \
		's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' \
		$(2)/bin/npth-config
	ln -sf $(STAGING_DIR)/host/bin/npth-config $(1)/usr/bin/npth-config

	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/include/npth.h \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnpth.{la,so*} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/share/aclocal
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/aclocal/npth.m4 \
		$(1)/usr/share/aclocal/
endef

define Package/libnpth/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnpth.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnpth))
