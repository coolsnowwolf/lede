#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PECL_NAME:=propro
PECL_LONGNAME:=Property proxy

PKG_VERSION:=2.1.0
PKG_RELEASE:=1
PKG_HASH:=7bba0653d90cd8f61816e13ac6c0f7102b4a16dc4c4e966095a121eeb4ae8271

PKG_NAME:=php7-pecl-propro
PKG_SOURCE:=$(PECL_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://pecl.php.net/get/

PKG_MAINTAINER:=Michael Heimpold <mhei@heimpold.de>

PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DIR:=$(BUILD_DIR)/pecl-php7/$(PECL_NAME)-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../php7/pecl.mk

CONFIGURE_ARGS+= \
	--enable-propro

define Build/InstallDev
	mkdir -p $(STAGING_DIR)/usr/include/php7/ext/$(PECL_NAME)
	$(CP) $(PKG_BUILD_DIR)/php_propro.h $(STAGING_DIR)/usr/include/php7/ext/$(PECL_NAME)/
	$(CP) $(PKG_BUILD_DIR)/php_propro_api.h $(STAGING_DIR)/usr/include/php7/ext/$(PECL_NAME)/
endef

$(eval $(call PHP7PECLPackage,$(PECL_NAME),$(PECL_LONGNAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
