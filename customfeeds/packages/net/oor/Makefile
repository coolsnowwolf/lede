# Copyright (C) 2011, 2015 Cisco Systems, Inc.
# Copyright (C) 2015 CBA research group, Technical University of Catalonia.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include $(TOPDIR)/rules.mk

PKG_NAME:=oor
PKG_VERSION:=1.3.0
PKG_RELEASE:=3

PKG_SOURCE_URL=https://github.com/OpenOverlayRouter/oor/releases/download/$(PKG_VERSION)/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=e2bee73611984f0412798f4b72145f4f00037a9e874d3c2168a49e1bf49e4809

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Albert Lopez <alopez@ac.upc.edu>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

MAKE_FLAGS += platform=openwrt

define Package/oor
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Open Overlay Router
  URL:=https://github.com/OpenOverlayRouter
  DEPENDS:= +librt +confuse +kmod-tun +uci @IPV6
endef

define Package/oor/description
  This packet provides support for the Locator-ID Separation Protocol, VXLAN-GPE.
endef

define Package/oor/conffiles
/etc/config/oor
endef

define Package/oor/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/oor/oor $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/openWRT/oor.uci.example $(1)/etc/config/oor
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openWRT/openwrt.init.script $(1)/etc/init.d/oor
endef

$(eval $(call BuildPackage,oor))
