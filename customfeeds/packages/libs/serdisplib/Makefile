include $(TOPDIR)/rules.mk

PKG_NAME:=serdisplib
PKG_VERSION:=2.02
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/serdisplib
PKG_HASH:=447b74007dc157b0378044245649850b26432b9185b6540ff681fcb0765c4d8b

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=GPL-2.0-or-later
PLG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=libusb-compat

include $(INCLUDE_DIR)/package.mk

define Package/serdisplib
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=serdisplib
  URL:=http://serdisplib.sourceforge.net/
endef

define Package/serdisplib/description
 serdisplib started as a library to drive serial displays with
 built-in controllers. It can optionally dynamically link with
 libusb-compat, libgd and libpthread, some features require having
 those packages installed as well.
endef

define Package/serdisplib-tools
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=serdisplib tools
  URL:=http://serdisplib.sourceforge.net/
  DEPENDS:=+serdisplib +libgd
endef

define Package/serdisplib-tools/description
 serdisplib started as a library to drive serial displays with
 built-in controllers. This package contains tools for serdisplib:
 * l4m132c_tool
 * l4m320t_tool
 * multidisplay
 * sdcmegtron_tool
 * touchscreen_tool
endef

CONFIGURE_ARGS += \
	--enable-dynloading \
	--disable-statictools

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/serdisplib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/serdisplib/*.h $(1)/usr/include/serdisplib
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libserdisp.{a,so*} $(1)/usr/lib/
endef

define Package/serdisplib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libserdisp.so* $(1)/usr/lib/
endef

define Package/serdisplib-tools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/l4m132c_tool $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/l4m320t_tool $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/multidisplay $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sdcmegtron_tool $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/touchscreen_tool $(1)/usr/bin
endef

$(eval $(call BuildPackage,serdisplib))
$(eval $(call BuildPackage,serdisplib-tools))
