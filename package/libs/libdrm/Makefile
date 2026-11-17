# Copyright (C) 2007-2024 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=libdrm
PKG_VERSION:=2.4.133
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://dri.freedesktop.org/libdrm
PKG_HASH:=fc68f9d0ba2ea63c9432a299e14fea09fad7a8a66e8039fcd7802ca59f77b4f5

PKG_MAINTAINER:=Lucian Cristian <lucian.cristian@gmail.com>
PKG_LICENSE:=BSD-3-Clause

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_libdrm-amdgpu \
	CONFIG_PACKAGE_libdrm-etnaviv \
	CONFIG_PACKAGE_libdrm-intel \
	CONFIG_PACKAGE_libdrm-nouveau \
	CONFIG_PACKAGE_libdrm-omap \
	CONFIG_PACKAGE_libdrm-radeon \
	CONFIG_PACKAGE_libdrm-tegra \
	CONFIG_PACKAGE_libdrm-tests

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/meson.mk

MESON_HOST_ARGS += \
	-Dintel=disabled -Damdgpu=disabled -Dradeon=disabled \
	-Dnouveau=disabled -Dvmwgfx=disabled -Detnaviv=disabled \
	-Dfreedreno=disabled -Dtegra=disabled -Dexynos=disabled \
	-Domap=disabled -Dtests=false -Dman-pages=disabled \
	-Dvalgrind=disabled

define Package/libdrm/Default
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=Video
  TITLE:=libdrm
  URL:=https://dri.freedesktop.org/
endef

define Package/libdrm/description/Default
  LIBDRM is the cross-driver middleware which allows user-space applications
  (such as Mesa and 2D drivers) to communicate with the Kernel by the means
  of the DRI protocol.
endef

define Package/libdrm
$(call Package/libdrm/Default)
  TITLE+= common library
endef

define Package/libdrm/description
$(call Package/libdrm/description/Default)
endef

define Package/libdrm-amdgpu
$(call Package/libdrm/Default)
  TITLE+= for AMD GPUs
  DEPENDS:=+libdrm
endef

define Package/libdrm-amdgpu/description
$(call Package/libdrm/description/Default)

This package provides the library with functions for AMD GPUs.
endef

define Package/libdrm-etnaviv
$(call Package/libdrm/Default)
  TITLE+= for Vivante GPUs
  DEPENDS:=+libdrm @(arm||aarch64||mips||mipsel||mips64||mips64el||riscv64)
endef

define Package/libdrm-etnaviv/description
$(call Package/libdrm/description/Default)

This package provides the library with functions for Vivante GPUs.
endef

define Package/libdrm-intel
$(call Package/libdrm/Default)
  TITLE+= for Intel integrated GPUs
  DEPENDS:=+libdrm +libpciaccess @(i386||i686||x86_64)
endef

define Package/libdrm-intel/description
$(call Package/libdrm/description/Default)

This package provides the library with functions for GPUs integrated into Intel's CPUs.
endef

define Package/libdrm-nouveau
$(call Package/libdrm/Default)
  TITLE+= for nVidia GPUs
  DEPENDS:=+libdrm
endef

define Package/libdrm-nouveau/description
$(call Package/libdrm/description/Default)

This package provides the library with functions for nVidia GPUs.
endef

define Package/libdrm-omap
$(call Package/libdrm/Default)
  TITLE+= for TI OMAP SoCs
  DEPENDS:=+libdrm @(arm||aarch64)
endef

define Package/libdrm-omap/description
$(call Package/libdrm/description/Default)

This package provides the library with functions for TI OMAP SoCs.
endef

define Package/libdrm-radeon
$(call Package/libdrm/Default)
  TITLE+= for AMD/ATI Radeon GPUs
  DEPENDS:=+libdrm
endef

define Package/libdrm-radeon/description
$(call Package/libdrm/description/Default)

This package provides the library with functions for AMD/ATI Radeon GPUs.
endef

define Package/libdrm-tegra
$(call Package/libdrm/Default)
  TITLE+= for nVidia Tegra SoCs
  DEPENDS:=+libdrm @(arm||aarch64)
endef

define Package/libdrm-tegra/description
$(call Package/libdrm/description/Default)

This package provides the library with functions for nVidia Tegra SoCs.
endef

define Package/libdrm-tests
$(call Package/libdrm/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Video
  TITLE+= test/utility programs (modetest, kmstest, ...)
  DEPENDS:=+libdrm
endef

define Package/libdrm-tests/description
$(call Package/libdrm/description/Default)

Test and utility programs shipped with libdrm:
  drmdevice, modetest, modeprint, proptest, vbltest.
endef

MESON_ARGS += \
	-Dintel=$(if $(CONFIG_PACKAGE_libdrm-intel),en,dis)abled \
	-Damdgpu=$(if $(CONFIG_PACKAGE_libdrm-amdgpu),en,dis)abled \
	-Dnouveau=$(if $(CONFIG_PACKAGE_libdrm-nouveau),en,dis)abled \
	-Dvmwgfx=disabled \
	-Domap=$(if $(CONFIG_PACKAGE_libdrm-omap),en,dis)abled  \
	-Dexynos=disabled \
	-Dfreedreno=disabled \
	-Dradeon=$(if $(CONFIG_PACKAGE_libdrm-radeon),en,dis)abled \
	-Dtegra=$(if $(CONFIG_PACKAGE_libdrm-tegra),en,dis)abled \
	-Dvc4=$(if $(CONFIG_arm)$(CONFIG_aarch64),en,dis)abled \
	-Detnaviv=$(if $(CONFIG_PACKAGE_libdrm-etnaviv),en,dis)abled  \
	-Dcairo-tests=disabled \
	-Dman-pages=disabled \
	-Dvalgrind=disabled \
	-Dfreedreno-kgsl=false \
	-Dinstall-test-programs=$(if $(CONFIG_PACKAGE_libdrm-tests),true,false) \
	-Dudev=false

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig
endef

define Package/libdrm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdrm.so.* $(1)/usr/lib/
endef

define Package/libdrm-amdgpu/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdrm_amdgpu.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/share/libdrm
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/libdrm/amdgpu.ids $(1)/usr/share/libdrm
endef

define Package/libdrm-etnaviv/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdrm_etnaviv.so.* $(1)/usr/lib/
endef

define Package/libdrm-intel/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdrm_intel.so.* $(1)/usr/lib/
endef

define Package/libdrm-nouveau/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdrm_nouveau.so.* $(1)/usr/lib/
endef

define Package/libdrm-omap/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdrm_omap.so.* $(1)/usr/lib/
endef

define Package/libdrm-radeon/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdrm_radeon.so.* $(1)/usr/lib/
endef

define Package/libdrm-tegra/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdrm_tegra.so.* $(1)/usr/lib/
endef

define Package/libdrm-tests/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/drmdevice $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/modeprint $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/modetest $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/proptest $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vbltest $(1)/usr/bin/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,libdrm))
$(eval $(call BuildPackage,libdrm-amdgpu))
$(eval $(call BuildPackage,libdrm-etnaviv))
$(eval $(call BuildPackage,libdrm-intel))
$(eval $(call BuildPackage,libdrm-nouveau))
$(eval $(call BuildPackage,libdrm-omap))
$(eval $(call BuildPackage,libdrm-radeon))
$(eval $(call BuildPackage,libdrm-tegra))
$(eval $(call BuildPackage,libdrm-tests))
