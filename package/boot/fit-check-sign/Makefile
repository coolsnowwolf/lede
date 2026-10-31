include $(TOPDIR)/rules.mk

PKG_NAME:=fit-check-sign
PKG_VERSION:=2026.07
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/fit-check-sign/u-boot-$(PKG_VERSION)

PKG_SOURCE:=u-boot-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://ftp.denx.de/pub/u-boot https://mirror.cyberbits.eu/u-boot
PKG_HASH:=78e8bfc382fe388f9b55aa1daf8c563522a037779b5d4c349d1415e381f1243e
PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/fit-check-sign
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Boot Loaders
  TITLE:=Verify U-Boot FIT images
endef

define Build/Configure
	$(call Build/Compile/Default,tools-only_defconfig)
	$(PKG_BUILD_DIR)/scripts/config --file $(PKG_BUILD_DIR)/.config \
		--disable TOOLS_KWBIMAGE --disable TOOLS_LIBCRYPTO \
		--disable TOOLS_MKEFICAPSULE
endef

MAKE_FLAGS += \
	ARCH="sandbox" \
	TARGET_CFLAGS="$(TARGET_CFLAGS)" \
	TARGET_LDFLAGS="$(TARGET_LDFLAGS)" \
	NO_PYTHON=1

define Build/Compile
	$(call Build/Compile/Default,cross_tools)
endef

define Package/fit-check-sign/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/fit_check_sign $(1)/usr/bin/
endef

$(eval $(call BuildPackage,fit-check-sign))
