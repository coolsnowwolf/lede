#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gperf
PKG_VERSION:=3.1
PKG_RELEASE:=1
PKG_HASH:=588546b945bba4b70b6a3a616e80b4ab466e3f33024a352fc2198112cdbb3ae2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/gperf
PKG_HOST_ONLY=1

PKG_MAINTAINER:=Espen Jürgensen <espenjurgensen+openwrt@gmail.com>
PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/gperf
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GNU gperf
  BUILDONLY:=1
  URL:=http://www.gnu.org/software/gperf
endef

define Package/gperf/description
 GNU gperf is a perfect hash function generator. For a given list of strings, it
 produces a hash function and hash table, in form of C or C++ code, for looking 
 up a value depending on the input string. The hash function is perfect, which 
 means that the hash table has no collisions, and the hash table lookup needs a
 single string comparison only.
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,gperf))
