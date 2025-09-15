# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2016-2019 Yousong Zhou <yszhou4tech@gmail.com>

include $(TOPDIR)/rules.mk

PKG_NAME:=dtc
PKG_VERSION:=1.6.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=65cec529893659a49a89740bb362f507a3b94fc8cd791e76a8d6a2b6f3203473
PKG_SOURCE_URL:=@KERNEL/software/utils/dtc

PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=GPL

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/dtc
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Device Tree Compiler
  URL:=https://git.kernel.org/pub/scm/utils/dtc/dtc.git
endef

define Package/dtc/description
  Device Tree Compiler for Flat Device Trees Device Tree Compiler, dtc, takes
  as input a device-tree in a given format and outputs a device-tree in another
  format for booting kernels on embedded systems.
endef

define Package/dtc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dtc $(1)/usr/bin
endef


# See Documentation/manual.txt for details about each utility
define Package/fdt-utils
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Flat Device Tree Utilities
  URL:=https://git.kernel.org/pub/scm/utils/dtc/dtc.git
  DEPENDS:=+libfdt
endef

define Package/fdt-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/convert-dtsv0 $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fdtdump $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fdtget $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fdtput $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fdtoverlay $(1)/usr/bin
endef


define Package/libfdt
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=a utility library for reading and manipulating dtb files
  URL:=https://git.kernel.org/pub/scm/utils/dtc/dtc.git
endef

define Package/libfdt/description
  This is a library containing functions for manipulating Flat Device Trees.
endef

define Package/libfdt/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfdt*.so* $(1)/usr/lib
endef

MESON_ARGS += \
	-Dtools=true \
	-Dyaml=disabled \
	-Dvalgrind=disabled \
	-Dpython=disabled

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,dtc))
$(eval $(call BuildPackage,fdt-utils))
$(eval $(call BuildPackage,libfdt))
