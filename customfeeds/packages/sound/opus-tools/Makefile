#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=opus-tools
PKG_VERSION:=0.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.xiph.org/releases/opus/
PKG_HASH:=b4e56cb00d3e509acfba9a9b627ffd8273b876b4e2408642259f6da28fa0ff86

PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Eduardo Abinader <eduardoabinader@gmail.com>

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/opus-tools
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+libogg +libopus +libopusfile +libopusenc
  TITLE:=OPUS Codec tools
  URL:=http://opus-codec.org/
endef

define Package/opus-tools/description
 This package provides command-line utilities to encode, inspect,
 and decode .opus files.
endef

CONFIGURE_ARGS += \
	--without-flac \

define Package/opus-tools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/opus{dec,enc,info} $(1)/usr/bin/
endef

$(eval $(call BuildPackage,opus-tools))
