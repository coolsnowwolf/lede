#
# Copyright (C) 2014 Arduino LLC
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NPM_NAME:=cylon
PKG_NAME:=node-$(PKG_NPM_NAME)
PKG_SRC_NAME:=$(PKG_NPM_NAME)-firmata
PKG_VERSION:=0.24.0
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_SRC_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=https://registry.npmjs.org/$(PKG_SRC_NAME)/-/
PKG_HASH:=06ac7a8e2e6012577d2f4b043af766bf28a1d3e2a0d50e46629dab4f0bb65104
PKG_SOURCE_SUBDIR:=$(PKG_SRC_NAME)-$(PKG_VERSION)

PKG_BUILD_DEPENDS:=node/host
PKG_USE_MIPS16:=0

PKG_MAINTAINER:=Hirokazu MORIKAWA <morikw2@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/node-cylon/default
  SUBMENU:=Node.js
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=CylonJS - $(1)
  URL:=https://www.npmjs.org/package/cylon-firmata
  DEPENDS:=+node +node-npm $(2)
endef

define Package/node-cylon
  $(call Package/node-cylon/default,Core)
endef

define Package/node-cylon-i2c
  $(call Package/node-cylon/default,I2C,+node-cylon)
endef

define Package/node-cylon-gpio
  $(call Package/node-cylon/default,GPIO,+node-cylon)
endef

define Package/node-cylon-firmata
  $(call Package/node-cylon/default,Firmata,+node-cylon-gpio +node-cylon-i2c +node-arduino-firmata)
endef

define Package/node-cylon/description
 JavaScript Robotics, By Your Command Next generation robotics framework with support for 36 different platforms Get Started
endef

TAR_OPTIONS+= --strip-components 1
TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)

NODEJS_CPU:=$(subst powerpc,ppc,$(subst aarch64,arm64,$(subst x86_64,x64,$(subst i386,ia32,$(ARCH)))))

define Build/Compile
	$(MAKE_VARS) \
	$(MAKE_FLAGS) \
	npm_config_arch=$(NODEJS_CPU) \
	npm_config_target_arch=$(NODEJS_CPU) \
	npm_config_build_from_source=true \
	npm_config_nodedir=$(STAGING_DIR)/usr/ \
	npm_config_prefix=$(PKG_INSTALL_DIR)/usr/ \
	npm_config_cache=$(TMP_DIR)/npm-cache \
	npm_config_tmp=$(TMP_DIR)/npm-tmp \
	npm install -g $(PKG_BUILD_DIR)
	rm -rf $(TMP_DIR)/npm-tmp
	rm -rf $(TMP_DIR)/npm-cache
endef

define Package/node-cylon/install
	$(INSTALL_DIR) $(1)/usr/lib/node/cylon
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/node_modules/cylon-firmata/node_modules/cylon/* $(1)/usr/lib/node/cylon/
endef

define Package/node-cylon-i2c/install
	$(INSTALL_DIR) $(1)/usr/lib/node/cylon-i2c
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/node_modules/cylon-firmata/node_modules/cylon-i2c/* $(1)/usr/lib/node/cylon-i2c/
endef

define Package/node-cylon-gpio/install
	$(INSTALL_DIR) $(1)/usr/lib/node/cylon-gpio
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/node_modules/cylon-firmata/node_modules/cylon-gpio/* $(1)/usr/lib/node/cylon-gpio/
endef

define Package/node-cylon-firmata/install
	$(INSTALL_DIR) $(1)/usr/lib/node/cylon-firmata
	$(CP) $(PKG_BUILD_DIR)/{package.json,LICENSE,*.md} \
		$(1)/usr/lib/node/cylon-firmata/
	$(CP) $(PKG_BUILD_DIR)/{docs,examples,*.js} \
		$(1)/usr/lib/node/cylon-firmata/
	$(CP) $(PKG_BUILD_DIR)/{lib,spec} \
		$(1)/usr/lib/node/cylon-firmata/
endef

$(eval $(call BuildPackage,node-cylon))
$(eval $(call BuildPackage,node-cylon-i2c))
$(eval $(call BuildPackage,node-cylon-gpio))
$(eval $(call BuildPackage,node-cylon-firmata))
