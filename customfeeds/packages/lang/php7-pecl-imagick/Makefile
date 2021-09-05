#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PECL_NAME:=imagick
PECL_LONGNAME:=Image Processing (ImageMagick binding)

PKG_VERSION:=3.4.4
PKG_RELEASE:=2
PKG_HASH:=8dd5aa16465c218651fc8993e1faecd982e6a597870fd4b937e9ece02d567077

PKG_NAME:=php7-pecl-imagick
PKG_SOURCE:=$(PECL_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://pecl.php.net/get/

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>

PKG_LICENSE:=PHP-3.01
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=php7
PKG_BUILD_DIR:=$(BUILD_DIR)/pecl-php7/$(PECL_NAME)-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../php7/pecl.mk

CONFIGURE_ARGS+= --with-imagick="$(STAGING_DIR)/usr"

$(eval $(call PHP7PECLPackage,imagick,$(PECL_LONGNAME),+imagemagick,30))
$(eval $(call BuildPackage,$(PKG_NAME)))
