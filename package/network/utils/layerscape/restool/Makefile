#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=restool
PKG_VERSION:=lsdk-1806
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/external/qoriq/qoriq-components/restool
PKG_SOURCE_VERSION:=2815572ab25029a8fb47f0dc6a0b56ae5ff76713
PKG_MIRROR_HASH:=cf76a8fd1239bb9b7696c25d5c26c6e81ea73497ad1ada9488390daf0ef7c291

include $(INCLUDE_DIR)/package.mk

define Package/restool
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Layerscape DPAA2 dynamical management tool
  DEPENDS:=@TARGET_layerscape
endef

MAKE_FLAGS += \
	DESTDIR="$(PKG_BUILD_DIR)"/output/ \
	install

define Package/restool/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/output/usr/local/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,restool))
