include $(TOPDIR)/rules.mk

PKG_NAME:=cni-plugins
PKG_VERSION:=0.9.1
PKG_RELEASE:=$(AUTORELEASE)
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/containernetworking/plugins/archive/v$(PKG_VERSION)
PKG_HASH:=35e96c6c47b9d080d1cbdcfca02808b01a95464607cd2a2c971b3ad596285928

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>, Paul Spooren <mail@aparcar.org>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/containernetworking/plugins/
GO_PKG_BUILD_PKG:=github.com/containernetworking/plugins/plugins/main/... \
		  github.com/containernetworking/plugins/plugins/meta/... \
		  github.com/containernetworking/plugins/plugins/ipam/...

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

PKG_UNPACK:=$(HOST_TAR) -C "$(PKG_BUILD_DIR)" --strip-components=1 -xzf "$(DL_DIR)/$(PKG_SOURCE)"

define Package/cni-plugins
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=cni-plugins
  URL:=https://github.com/containernetworking/cni-plugins
  DEPENDS:=$(GO_ARCH_DEPENDS) +ip-full +kmod-veth
endef

define Package/cni-plugins/description
  Some CNI network plugins, maintained by the containernetworking team. For
  more information, see the individual READMEs.
endef

define Package/cni-plugins/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))
	$(INSTALL_DIR) $(1)/usr/lib/cni
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/lib/cni
endef

$(eval $(call GoBinPackage,cni-plugins))
$(eval $(call BuildPackage,cni-plugins))
