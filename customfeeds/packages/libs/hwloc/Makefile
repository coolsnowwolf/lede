#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hwloc
PKG_VERSION:=2.3.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://download.open-mpi.org/release/$(PKG_NAME)/v2.3
PKG_HASH:=b607f6097873f69ef6b4b01e66e0dcb45f9939e8979827284664bbf0d4018a64

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/hwloc/Default
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Portable Hardware Locality
  URL:=https://www.open-mpi.org/projects/hwloc/
endef

define Package/hwloc/Default/description
  The Portable Hardware Locality (hwloc) software package provides a
  portable abstraction (across OS, versions, architectures, ...) of the
  hierarchical topology of modern architectures, including NUMA memory
  nodes, sockets, shared caches, cores and simultaneous multithreading. It
  also gathers various system attributes such as cache and memory
  information as well as the locality of I/O devices such as network
  interfaces, InfiniBand HCAs or GPUs.
endef

define Package/hwloc-utils
$(call Package/hwloc/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+= utilities
  DEPENDS+= +libhwloc +libncurses
endef

define Package/hwloc-utils/description
$(call Package/hwloc/Default/description)
  This package contains the hwloc utilities.
endef

define Package/libhwloc
$(call Package/hwloc/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= libraries
  DEPENDS+=+libpciaccess
endef

define Package/libhwloc/description
$(call Package/hwloc/Default/description)
  This package contains the hwloc libraries.
endef

CONFIGURE_ARGS += \
	--disable-libxml2 \
	--disable-libudev

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/hwloc.h $(STAGING_DIR)/usr/include/
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/hwloc
	$(CP) $(PKG_INSTALL_DIR)/usr/include/hwloc/*.h $(STAGING_DIR)/usr/include/hwloc/
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/hwloc/autogen
	$(CP) $(PKG_INSTALL_DIR)/usr/include/hwloc/autogen/*.h $(STAGING_DIR)/usr/include/hwloc/autogen/
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib* $(STAGING_DIR)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig
endef

define Package/hwloc-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

define Package/libhwloc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,hwloc-utils))
$(eval $(call BuildPackage,libhwloc))
