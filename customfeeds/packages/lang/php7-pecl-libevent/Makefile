#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PECL_NAME:=libevent
PECL_LONGNAME:=Libevent - event notification

PKG_NAME:=php7-pecl-$(PECL_NAME)
PKG_VERSION=2019-05-30-$(PKG_SOURCE_VERSION)
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/expressif/pecl-event-libevent.git
PKG_SOURCE_VERSION:=ce255e7e1e293030951fd68d5df06b2a4c077a89
PKG_SOURCE_SUBDIR:=$(PECL_NAME)-$(PKG_SOURCE_VERSION)
PKG_SOURCE:=$(PECL_NAME)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_HASH:=9bb1665e020c2254bb347adc0779bea75876b8c21f2612eca723c99280a780dd

PKG_MAINTAINER:=Michael Heimpold <mhei@heimpold.de>

PKG_LICENSE:=PHP-3.01
PKG_LICENSE_FILES:=

PKG_BUILD_DIR:=$(BUILD_DIR)/pecl-php7/$(PECL_NAME)-$(PKG_SOURCE_VERSION)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../php7/pecl.mk

CONFIGURE_ARGS+= --with-libevent=shared,"$(STAGING_DIR)/usr"

$(eval $(call PHP7PECLPackage,$(PECL_NAME),$(PECL_LONGNAME),+libevent2 +php7-mod-sockets,25))
$(eval $(call BuildPackage,$(PKG_NAME)))
