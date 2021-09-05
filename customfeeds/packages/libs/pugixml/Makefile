#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pugixml
PKG_VERSION:=1.11.4
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/zeux/pugixml/releases/download/v$(PKG_VERSION)
PKG_HASH:=8ddf57b65fb860416979a3f0640c2ad45ddddbbafa82508ef0a0af3ce7061716

PKG_MAINTAINER:=
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/pugixml
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=pugixml
  URL:=https://github.com/zeux/pugixml
  DEPENDS:=+libstdcpp
  BUILDONLY:=1
endef

define Package/pugixml/description
  pugixml is a C++ XML processing library, which consists of a DOM-like interface
  with rich traversal/modification capabilities, an extremely fast XML parser which
  constructs the DOM tree from an XML file/buffer, and an XPath 1.0 implementation
  for complex data-driven tree queries. Full Unicode support is also available,
  with Unicode interface variants and conversions between different Unicode encodings
  (which happen automatically during parsing/saving).
endef

TARGET_CXXFLAGS += -flto

define Build/InstallDev
	$(call Build/InstallDev/cmake,$(1))
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/pugixml.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/pugixml.pc
endef

$(eval $(call BuildPackage,pugixml))
