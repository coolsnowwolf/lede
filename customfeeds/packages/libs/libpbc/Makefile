include $(TOPDIR)/rules.mk

PKG_NAME:=libpbc
PKG_VERSION:=0.5.14
PKG_RELEASE:=2

PKG_SOURCE:=pbc-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://crypto.stanford.edu/pbc/files/
PKG_HASH:=772527404117587560080241cedaf441e5cac3269009cdde4c588a1dce4c23d2
PKG_BUILD_DIR:=$(BUILD_DIR)/pbc-$(PKG_VERSION)

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=LGPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libpbc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=The Pairing-Based Cryptography Library
  URL:=https://crypto.stanford.edu/pbc/
  DEPENDS:=+libgmp
endef

define Package/libpbc/description
  Pairing-based cryptography is a relatively young area of cryptography
  that revolves around a certain function with special properties.

  The PBC (Pairing-Based Cryptography) library is a free C library
  (released under the GNU Lesser General Public License) built on the GMP
  library that performs the mathematical operations underlying
  pairing-based cryptosystems.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/pbc
	$(CP) $(PKG_INSTALL_DIR)/usr/include/pbc/*.h $(1)/usr/include/pbc
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpbc* $(1)/usr/lib/
endef

define Package/libpbc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpbc.so.*  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libpbc))
