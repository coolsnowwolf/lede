# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NPM_NAME:=javascript-obfuscator
PKG_NAME:=node-$(PKG_NPM_NAME)
PKG_VERSION:=2.9.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NPM_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=https://registry.npmjs.org/$(PKG_NPM_NAME)/-/
PKG_HASH:=db443ed7c07a7a111352dac4aa7c6edd7969f97386f37c1cfe884d5a8c5f5408

PKG_BUILD_DEPENDS:=node/host
HOST_BUILD_PARALLEL:=1

HOST_BUILD_DEPENDS:=node/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

PKG_MAINTAINER:=Zbynek Kocur <zbynek.kocur@fel.cvut.cz>
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE.BSD

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/node-javascript-obfuscator
  SUBMENU:=Node.js
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=JavaScript obfuscator
  URL:=https://www.npmjs.org/package/javascript-obfuscator
  DEPENDS:=+node
endef

define Package/node-javascript-obfuscator/description
 JavaScript obfuscator is a powerful free obfuscator for JavaScript with a wide number of features which provides protection for your source code.
endef

TAR_OPTIONS+= --strip-components 1
TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)

NODEJS_CPU:=$(subst powerpc,ppc,$(subst aarch64,arm64,$(subst x86_64,x64,$(subst i386,ia32,$(ARCH)))))
TMPNPM:=$(shell mktemp -u XXXXXXXXXX)
HOSTTMPNPM:=$(shell mktemp -u XXXXXXXXXX)

TARGET_CFLAGS+=$(FPIC)
TARGET_CPPFLAGS+=$(FPIC)

define Build/Compile
	$(MAKE_VARS) \
	$(MAKE_FLAGS) \
	npm_config_arch=$(NODEJS_CPU) \
	npm_config_target_arch=$(NODEJS_CPU) \
	npm_config_build_from_source=true \
	npm_config_nodedir=$(STAGING_DIR)/usr/ \
	npm_config_prefix=$(PKG_INSTALL_DIR)/usr/ \
	npm_config_cache=$(TMP_DIR)/npm-cache-$(TMPNPM) \
	npm_config_tmp=$(TMP_DIR)/npm-tmp-$(TMPNPM) \
	npm install -g $(PKG_BUILD_DIR)
	rm -rf $(TMP_DIR)/npm-tmp-$(TMPNPM)
	rm -rf $(TMP_DIR)/npm-cache-$(TMPNPM)
endef

define Package/node-javascript-obfuscator/install
	$(INSTALL_DIR) $(1)/usr/lib/node/$(PKG_NPM_NAME)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/node_modules/$(PKG_NPM_NAME)/{package.json,LICENSE.BSD} \
		$(1)/usr/lib/node/$(PKG_NPM_NAME)/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/node_modules/$(PKG_NPM_NAME)/README.md \
		$(1)/usr/lib/node/$(PKG_NPM_NAME)/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/node_modules/$(PKG_NPM_NAME)/{node_modules,bin,dist} \
		$(1)/usr/lib/node/$(PKG_NPM_NAME)/
	$(INSTALL_DIR) $(1)/usr/bin
	$(LN) ../lib/node/javascript-obfuscator/bin/javascript-obfuscator $(1)/usr/bin/javascript-obfuscator
endef

define Host/Compile
	$(HOST_MAKE_VARS) \
	npm_config_nodedir=$(STAGING_DIR)/usr/ \
	npm_config_prefix=$(HOST_INSTALL_DIR)/usr/ \
	npm_config_cache=$(TMP_DIR)/npm-cache-$(HOSTTMPNPM) \
	npm_config_tmp=$(TMP_DIR)/npm-tmp-$(HOSTTMPNPM) \
	npm install -g --no-optional $(HOST_BUILD_DIR)
	rm -rf $(TMP_DIR)/npm-tmp-$(HOSTTMPNPM)
	rm -rf $(TMP_DIR)/npm-cache-$(HOSTTMPNPM)
endef

define Host/Install
	$(INSTALL_DIR) $(1)/lib/node_modules/$(PKG_NPM_NAME)
	$(CP) $(HOST_INSTALL_DIR)/usr/lib/node_modules/$(PKG_NPM_NAME)/{package.json,LICENSE.BSD} \
		$(1)/lib/node_modules/$(PKG_NPM_NAME)/
	$(CP) $(HOST_INSTALL_DIR)/usr/lib/node_modules/$(PKG_NPM_NAME)/README.md \
		$(1)/lib/node_modules/$(PKG_NPM_NAME)/
	$(CP) $(HOST_INSTALL_DIR)/usr/lib/node_modules/$(PKG_NPM_NAME)/{node_modules,bin,dist} \
		$(1)/lib/node_modules/$(PKG_NPM_NAME)/
	$(INSTALL_DIR) $(1)/bin
	$(LN) ../lib/node_modules/javascript-obfuscator/bin/javascript-obfuscator $(1)/bin/javascript-obfuscator
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,node-javascript-obfuscator))
