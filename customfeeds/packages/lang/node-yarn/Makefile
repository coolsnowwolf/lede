# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NPM_NAME:=yarn
PKG_NAME:=node-$(PKG_NPM_NAME)
PKG_VERSION:=1.22.10
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NPM_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=https://registry.npmjs.org/$(PKG_NPM_NAME)/-/
PKG_HASH:=05a22fff30d7d8e8005bed277bf20d55111ba2bed65a6b91a0fcd1307b71fd8d

PKG_MAINTAINER:=Hirokazu MORIKAWA <morikw2@gmail.com>, Patrik Laszlo <alabard@gmail.com>, Dobroslaw Kijowski <dobo90@gmail.com>
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_HOST_ONLY:=1
HOST_BUILD_DEPENDS:=node/host
HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/node-yarn
  SUBMENU:=Node.js
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Fast, reliable, and secure dependency management
  URL:=https://yarnpkg.com/
  DEPENDS:=+node
  BUILDONLY:=1
endef

define Package/node-yarn/description
 Fast, reliable, and secure dependency management
endef

TAR_OPTIONS+= --strip-components 1
TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)

HOSTTMPNPM:=$(shell mktemp -u XXXXXXXXXX)

define Host/Compile
	$(HOST_MAKE_VARS) \
	npm_config_nodedir=$(STAGING_DIR)/usr/ \
	npm_config_prefix=$(HOST_INSTALL_DIR)/usr/ \
	npm_config_cache=$(TMP_DIR)/npm-cache-$(HOSTTMPNPM) \
	npm_config_tmp=$(TMP_DIR)/npm-tmp-$(HOSTTMPNPM) \
	npm install -g $(HOST_BUILD_DIR)
	rm -rf $(TMP_DIR)/npm-tmp-$(HOSTTMPNPM)
	rm -rf $(TMP_DIR)/npm-cache-$(HOSTTMPNPM)
endef

define Host/Install
	$(INSTALL_DIR) $(1)/lib/node_modules/$(PKG_NPM_NAME)
	$(CP) $(HOST_INSTALL_DIR)/usr/lib/node_modules/$(PKG_NPM_NAME)/{package.json,LICENSE} \
		$(1)/lib/node_modules/$(PKG_NPM_NAME)/
	$(CP) $(HOST_INSTALL_DIR)/usr/lib/node_modules/$(PKG_NPM_NAME)/README.md \
		$(1)/lib/node_modules/$(PKG_NPM_NAME)/
	$(CP) $(HOST_INSTALL_DIR)/usr/lib/node_modules/$(PKG_NPM_NAME)/{bin,lib} \
		$(1)/lib/node_modules/$(PKG_NPM_NAME)/
	$(INSTALL_DIR) $(1)/bin
	$(LN) ../lib/node_modules/$(PKG_NPM_NAME)/bin/yarn.js $(1)/bin/yarn
	$(LN) ../lib/node_modules/$(PKG_NPM_NAME)/bin/yarn.js $(1)/bin/yarnpkg
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,node-yarn))
