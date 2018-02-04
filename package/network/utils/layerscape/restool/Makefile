#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=restool
PKG_SOURCE_DATE:=2017-10-23
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/qoriq-open-source/restool.git
PKG_SOURCE_VERSION:=8f08b9d499e84f9057784c2036f0ddf75ae3fc70
PKG_MIRROR_HASH:=29f70ad27c7ab20bc018f0d0b11c680fcf8b829d10e5af5c30af4a3f9228a2fc

include $(INCLUDE_DIR)/package.mk

define Package/restool
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@TARGET_layerscape
  TITLE:=Layerscape DPAA2 dynamical management tool
endef

MAKE_FLAGS += \
	DESTDIR="$(PKG_BUILD_DIR)"/output/ \
	install

define Package/restool/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/output/usr/local/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,restool))
