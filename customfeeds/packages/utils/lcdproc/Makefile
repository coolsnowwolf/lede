#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lcdproc
PKG_VERSION:=0.5.9
PKG_RELEASE:=5

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/lcdproc/lcdproc/releases/download/v$(PKG_VERSION)/
PKG_HASH:=d48a915496c96ff775b377d2222de3150ae5172bfb84a6ec9f9ceab962f97b83

PKG_MAINTAINER:=Harald Geyer <harald@ccbib.org>, \
		Philip Prindeville <philipp@redfish-solutions.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:lcdproc:lcdproc

include $(INCLUDE_DIR)/package.mk

define Package/lcdproc/Default
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=http://lcdproc.org/
endef

define Package/lcdproc/Default-description
LCDProc is a daemon and clients for displaying system information
on various LCD displays
endef


define Package/lcdproc-clients
  $(call Package/lcdproc/Default)
  TITLE:=LCD Display clients
endef

define Package/lcdproc-clients/description
$(call Package/lcdproc/Default-description)

This package contains the clients distributed by the official lcdproc
project:
* lcdproc -- displays system information
* lcdexec -- displays a menu structure to execute commands
* lcdvc -- shows the content of the system console
endef

define Package/lcdproc-clients/conffiles
/etc/lcdproc.conf
/etc/lcdexec.conf
/etc/lcdvc.conf
endef


define Package/lcdproc-server
  $(call Package/lcdproc/Default)
  TITLE:=LCD Display server
  DEPENDS:=+libpthread
endef

define LCDPROC_CORE_DRIVERS_TEXT
bayrad CFontzPacket CFontz CwLnx ea65 EyeboxOne glk icp_a106 imonlcd
imon irtrans joy lb216 lcdm001 lcterm linux_input MD8800 ms6931
mtc_s16209x MtxOrb NoritakeVFD Olimex_MOD_LCD1x9 pyramid rawserial
serialPOS serialVFD sli SureElec text tyan vlsys_m428 yard2LCD
endef

LCDPROC_CORE_DRIVERS:=$(strip $(LCDPROC_CORE_DRIVERS_TEXT))

define Package/lcdproc-server/description
$(call Package/lcdproc/Default-description)

This package contains the server and a core set of display drivers
without external dependencies:
$(LCDPROC_CORE_DRIVERS_TEXT)
endef

define Package/lcdproc-server/conffiles
/etc/LCDd.conf
endef


define Package/lcdproc-drivers
  $(call Package/lcdproc/Default)
  TITLE:=LCD Display extra drivers
  DEPENDS:=+lcdproc-server +libncurses +libusb-1.0 +libusb-compat +libftdi1 \
	+GPIO_SUPPORT:libugpio +serdisplib
endef

define LCDPROC_OTHER_DRIVERS_TEXT
curses futaba glcd hd44780 IOWarrior i2500vfd lis picolcd shuttleVFD ula200
endef

ifeq ($(CONFIG_PACKAGE_kmod-lp),y)
LCDPROC_OTHER_DRIVERS_TEXT+=sdeclcd sed1330 sed1520 stv5730 t6963
endif

LCDPROC_OTHER_DRIVERS:=$(strip $(LCDPROC_OTHER_DRIVERS_TEXT))

define Package/lcdproc-drivers/description
$(call Package/lcdproc/Default-description)

This package contains display drivers with external dependencies:
$(LCDPROC_OTHER_DRIVERS_TEXT)
endef

CONFIGURE_ARGS += \
	--disable-libX11 \
	--disable-libhid \
	--disable-libpng \
	--disable-freetype \
	--enable-drivers='all,!g15,!g15driver,!glcdlib,!irman,!lirc,!mdm166a,!mx5000,!svga,!xosd'

MAKE_FLAGS += \
        CFLAGS="$(TARGET_CFLAGS)" \
        LDFLAGS="$(TARGET_LDFLAGS)"


define Package/lcdproc-clients/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/clients/lcdproc/lcdproc $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/clients/lcdexec/lcdexec $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/clients/lcdvc/lcdvc $(1)/usr/bin/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/clients/lcdproc/lcdproc.conf $(1)/etc/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/clients/lcdexec/lcdexec.conf $(1)/etc/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/clients/lcdvc/lcdvc.conf $(1)/etc/
	$(INSTALL_BIN) ./files/lcd* $(1)/etc/init.d/
endef

define Package/lcdproc-server/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib/lcdproc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/server/LCDd $(1)/usr/sbin/
	$(CP) $(foreach driver,$(LCDPROC_CORE_DRIVERS),$(PKG_BUILD_DIR)/server/drivers/$(driver).so) $(1)/usr/lib/lcdproc/
	$(CP) -p $(PKG_BUILD_DIR)/LCDd.conf $(PKG_BUILD_DIR)/LCDd.conf.orig
	sed -i -r \
		-e 's!^(DriverPath=).*$$$$!\1/usr/lib/lcdproc/!' \
		-e 's!^(Driver=)curses$$$$!\1sdeclcd!' \
		$(PKG_BUILD_DIR)/LCDd.conf
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/LCDd.conf $(1)/etc/
	$(INSTALL_BIN) ./files/LCDd $(1)/etc/init.d/
endef

define Package/lcdproc-drivers/install
	$(INSTALL_DIR) $(1)/usr/lib/lcdproc
	$(CP) $(foreach driver,$(LCDPROC_OTHER_DRIVERS),$(PKG_BUILD_DIR)/server/drivers/$(driver).so) $(1)/usr/lib/lcdproc/
endef


$(eval $(call BuildPackage,lcdproc-clients))
$(eval $(call BuildPackage,lcdproc-server))
$(eval $(call BuildPackage,lcdproc-drivers))

