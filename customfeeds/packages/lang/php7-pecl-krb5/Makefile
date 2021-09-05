#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PECL_NAME:=krb5
PECL_LONGNAME:=Bindings for the Kerberos library

PKG_VERSION:=1.1.4
PKG_RELEASE:=1
PKG_HASH:=8aba35bb9f2892be9092e8717d3e13604a817cdfb478dbc3855a226abc6c5979

PKG_NAME:=php7-pecl-krb5
PKG_SOURCE:=$(PECL_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://pecl.php.net/get/

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=php7
PKG_BUILD_DIR:=$(BUILD_DIR)/pecl-php7/$(PECL_NAME)-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../php7/pecl.mk

CONFIGURE_ARGS+= \
	--with-krb5=shared,"$(STAGING_DIR)/usr" \
	--with-krb5config=$(STAGING_DIR)/usr/bin/krb5-config

$(eval $(call PHP7PECLPackage,krb5,$(PECL_LONGNAME),+krb5-libs,30))
$(eval $(call BuildPackage,$(PKG_NAME)))
