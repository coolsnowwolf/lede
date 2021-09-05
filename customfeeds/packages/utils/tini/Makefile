include $(TOPDIR)/rules.mk

PKG_NAME:=tini
PKG_VERSION:=0.19.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/krallin/tini/tar.gz/v${PKG_VERSION}?
PKG_HASH:=0fd35a7030052acd9f58948d1d900fe1e432ee37103c5561554408bdac6bbf0d
PKG_SOURCE_VERSION:=de40ad007797e0dcd8b7126f27bb87401d224240
TINI_COMMIT:=de40ad0

PKG_MAINTAINER:=Gerard Ryan <G.M0N3Y.2503@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/tini
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=simplest init utility
  URL:=https://github.com/krallin/tini
  DEPENDS:=
endef

define Package/tini/description
A tiny but valid init process for containers.
endef

CMAKE_OPTIONS += -DTINI_VERSION_GIT='$(TINI_COMMIT)'
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lc -lgcc_eh)

define Package/tini/install
	$(INSTALL_DIR) $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tini-static $(1)/usr/bin/docker-init
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tini $(1)/usr/bin/
endef

$(eval $(call BuildPackage,tini))
