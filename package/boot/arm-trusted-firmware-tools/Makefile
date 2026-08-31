#
# Copyright 2021 Daniel Golle
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-tools
PKG_VERSION:=2.12
PKG_RELEASE:=1
PKG_HASH:=b4c047493cac1152203e1ba121ae57267e4899b7bf56eb365e22a933342d31c9

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_HOST_ONLY:=1

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/trusted-firmware-a-$(PKG_VERSION)

include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/arm-trusted-firmware-tools
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=ARM Trusted Firmware tools
  URL:=https://www.trustedfirmware.org
  BUILDONLY:=1
endef

ifeq ($(shell uname -s),Linux)
	HOST_MUSL_FLAG:=$(shell [ "$$(2>&1 ldd --version | head -n 1 | cut -d ' ' -f1)" = "musl" ] && echo " -D__MUSL__")
else
	HOST_MUSL_FLAG:=
endif

define Host/Compile
	$(MAKE) -C \
		$(HOST_BUILD_DIR)/tools/fiptool \
		CPPFLAGS="$(HOST_CFLAGS)$(HOST_MUSL_FLAG)" \
		LDFLAGS="$(HOST_LDFLAGS)" \
		OPENSSL_DIR="$(STAGING_DIR_HOST)"
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tools/fiptool/fiptool $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tools/sptool/sptool.py $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/fiptool
	rm -f $(STAGING_DIR_HOST)/bin/sptool.py $(STAGING_DIR_HOST)/bin/sptool
endef

$(eval $(call BuildPackage,arm-trusted-firmware-tools))
$(eval $(call HostBuild))
