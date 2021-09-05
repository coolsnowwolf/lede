#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vncrepeater
PKG_VERSION:=0.14
PKG_RELEASE:=2
PKG_MAINTAINER:=Jirka Spicak <robutek@gmail.com> 
PKG_LICENSE:=GPL-2.0


PKG_SOURCE:=repeater014.zip
PKG_SOURCE_URL:=http://jtko.mbnet.fi/uvncrepeater
PKG_HASH:=79178e9baa9cac05f26e43f742933958707cb5c0632c51a5706b13922f3bb5b0

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/vncrepeater
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libstdcpp
  TITLE:=UltraVNC repeater for Linux
  URL:=http://www.uvnc.com/addons/repeater.html
endef

define Package/vncrepeater/conffiles
/etc/vncrepeater.conf
endef

define Package/vncrepeater/description
 Viewer can be behind Nat router or directly connected to the internet 
 instead of forwarding serveral ports, you only need to forward 1 port. 
 If the PC that runs the Repeater has access to the local DNS server, 
 you can use your local DNS names instead of 10.10.10.12.
 This could be handy when you have a dynamic DHCP server allocating 
 ip adresses for your PC.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(PKG_UNPACK)
	$(CP) $(PKG_BUILD_DIR)/../Ver014/* $(PKG_BUILD_DIR)/
	rm -rf $(PKG_BUILD_DIR)/../Ver014/
	$(Build/Patch)
endef

define Build/Compile
	$(call Build/Compile/Default, \
		CC="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) -fno-rtti" \
		repeater \
	)
endef

define Package/vncrepeater/install	
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/repeater $(1)/usr/sbin/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc
	$(CP) $(PKG_BUILD_DIR)/uvncrepeater.ini $(1)/etc/vncrepeater.conf
endef

$(eval $(call BuildPackage,vncrepeater))
