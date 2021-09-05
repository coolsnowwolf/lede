#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PECL_NAME:=raphf
PECL_LONGNAME:=Resource and persistent handles factory

PKG_VERSION:=2.0.1
PKG_RELEASE:=2
PKG_HASH:=da3566db17422e5ef08b7ff144162952aabc14cb22407cc6b1d2a2d095812bd0

PKG_NAME:=php7-pecl-raphf
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
	--enable-raphf

define Build/InstallDev
	mkdir -p $(STAGING_DIR)/usr/include/php7/ext/$(PECL_NAME)
	cp $(PKG_BUILD_DIR)/php_raphf.h $(STAGING_DIR)/usr/include/php7/ext/$(PECL_NAME)/
	cp $(PKG_BUILD_DIR)/php_raphf_api.h $(STAGING_DIR)/usr/include/php7/ext/$(PECL_NAME)/
endef

$(eval $(call PHP7PECLPackage,$(PECL_NAME),$(PECL_LONGNAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
