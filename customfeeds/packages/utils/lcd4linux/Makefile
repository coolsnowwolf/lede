#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lcd4linux
PKG_RELEASE:=7

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/feckert/lcd4linux
PKG_SOURCE_DATE:=2019-07-19
PKG_SOURCE_VERSION:=0b83bb3a81e7e7f5c0aa1daa6f5b9abca538fd47
PKG_MIRROR_HASH:=26a3469bce0648b276e2c818add7049685c7f461ea9669d23730dfc3a96d9934

PKG_MAINTAINER:=Florian Eckert <fe@dev.tdt.de>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

PKG_CONFIG_DEPENDS:= \
	$(patsubst %,CONFIG_LCD4LINUX_CUSTOM_DRIVER_%,$(LCD4LINUX_DRIVERS)) \
	$(patsubst %,CONFIG_LCD4LINUX_CUSTOM_PLUGIN_%,$(LCD4LINUX_PLUGINS)) \

LCD4LINUX_DRIVERS:= \
	ASTUSB \
	BeckmannEgle \
	BWCT \
	CrystalFontz \
	Curses \
	Cwlinux \
	D4D \
	DPF \
	EA232graphic \
	EFN \
	FutabaVFD \
	FW8888 \
	G15 \
	GLCD2USB \
	IRLCD \
	$(if $(CONFIG_BROKEN),HD44780) \
	$(if $(CONFIG_BROKEN),HD44780-I2C) \
	LCD2USB \
	$(if $(CONFIG_BROKEN),LCDLinux) \
	LCDTerm \
	LEDMatrix \
	LPH7508 \
	$(if $(CONFIG_BROKEN),LUIse) \
	LW_ABP \
	M50530 \
	MatrixOrbital \
	MatrixOrbitalGX \
	MilfordInstruments \
	Newhaven \
	Noritake \
	NULL \
	Pertelian \
	PHAnderson \
	PICGraphic \
	picoLCD \
	picoLCDGraphic \
	PNG \
	PPM \
	$(if $(CONFIG_BROKEN),SamsungSPF) \
	ShuttleVFD \
	SimpleLCD \
	st2205 \
	T6963 \
	TeakLCM \
	Trefon \
	USBHUB \
	USBLCD \
	VNC \
	WincorNixdorf \
#	ULA200 \
#	X11 \

LCD4LINUX_PLUGINS:= \
	apm \
	asterisk \
	button_exec \
	cpuinfo \
	dbus \
	diskstats \
	dvb \
	event \
	exec \
	fifo \
	file \
	gps \
	hddtemp \
	huawei \
	i2c_sensors \
	iconv \
	imon \
	isdn \
	kvv \
	loadavg \
	netdev \
	netinfo \
	meminfo \
	mpd \
	mpris_dbus \
	mysql \
	netdev \
	pop3 \
	ppp \
	proc_stat \
	qnaplog \
	seti \
	statfs \
	uname \
	uptime \
	w1retap \
	$(if $(CONFIG_BROKEN),wireless) \
	xmms \
#	python

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/lcd4linux/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=LCD display utility
  URL:=http://lcd4linux.bulix.org/
endef

define Package/lcd4linux/Default/description
 LCD4Linux is a small program that grabs information from the kernel and
 some subsystems and displays it on an external liquid crystal display.
endef


define Package/lcd4linux-custom
$(call Package/lcd4linux/Default)
  DEPENDS:= \
	+LCD4LINUX_CUSTOM_NEEDS_libdbus:libdbus \
	+LCD4LINUX_CUSTOM_NEEDS_libgd:libgd \
	$(if $(ICONV_FULL),+LCD4LINUX_CUSTOM_NEEDS_libiconv:libiconv-full) \
	+LCD4LINUX_CUSTOM_NEEDS_libjpeg:libjpeg \
	+LCD4LINUX_CUSTOM_NEEDS_libmpdclient:libmpdclient \
	+LCD4LINUX_CUSTOM_NEEDS_libmysqlclient:libmysqlclient \
	+LCD4LINUX_CUSTOM_NEEDS_libncurses:libncurses \
	+LCD4LINUX_CUSTOM_NEEDS_libsqlite3:libsqlite3 \
	+LCD4LINUX_CUSTOM_NEEDS_libusb:libusb-compat \
#	+LCD4LINUX_CUSTOM_NEEDS_libftdi:libftdi \
#	+LCD4LINUX_CUSTOM_NEEDS_libX11:libX11 \
#	+LCD4LINUX_CUSTOM_NEEDS_python:python
  MENU:=1
  PROVIDES:=lcd4linux
  VARIANT=custom
endef

define Package/lcd4linux-custom/config
	source "$(SOURCE)/Config.in"
endef

define Package/lcd4linux-custom/description
$(call Package/lcd4linux/Default/description)
 .
 This package contains a customized version of LCD4Linux.
endef


define Package/lcd4linux-full
$(call Package/lcd4linux/Default)
  DEPENDS:= @DEVEL \
	+libdbus \
	+libgd \
	$(if $(ICONV_FULL),+libiconv-full) \
	+libmpdclient \
	+libmysqlclient \
	+libncurses \
	+libsqlite3 \
	+libusb-compat \
#	+libftdi \
#	+libX11 \
#	+python
  PROVIDES:=lcd4linux
  VARIANT=full
endef

define Package/lcd4linux-full/description
$(call Package/lcd4linux/Default/description)
 .
 This package contains a version of LCD4Linux built with all supported
 drivers and plugins.
endef


CONFIGURE_ARGS+= \
	--disable-rpath

TARGET_CFLAGS += -std=gnu89

ifeq ($(BUILD_VARIANT),custom)

  LCD4LINUX_CUSTOM_DRIVERS:= $(strip $(foreach c, $(LCD4LINUX_DRIVERS), \
    $(if $(CONFIG_LCD4LINUX_CUSTOM_DRIVER_$(c)),$(c),) \
 ))
  ifeq ($(LCD4LINUX_CUSTOM_DRIVERS),)
    LCD4LINUX_CUSTOM_DRIVERS:=Sample
  endif

  LCD4LINUX_CUSTOM_PLUGINS:= $(strip $(foreach c, $(LCD4LINUX_PLUGINS), \
    $(if $(CONFIG_LCD4LINUX_CUSTOM_PLUGIN_$(c)),$(c)) \
  ))
  ifeq ($(LCD4LINUX_CUSTOM_PLUGINS),)
    LCD4LINUX_CUSTOM_PLUGINS:=sample
  endif

  CONFIGURE_ARGS+= \
	--with-drivers="$(LCD4LINUX_CUSTOM_DRIVERS)" \
	--with-plugins="$(LCD4LINUX_CUSTOM_PLUGINS)" \

  ifneq ($(CONFIG_LCD4LINUX_CUSTOM_NEEDS_libiconv),)
    CONFIGURE_ARGS+= --with-libiconv-prefix="$(ICONV_PREFIX)"
  else
    CONFIGURE_ARGS+= --without-libiconv-prefix
  endif

  ifneq ($(CONFIG_LCD4LINUX_CUSTOM_NEEDS_libmysqlclient),)
    EXTRA_LDFLAGS+= -L$(STAGING_DIR)/usr/lib/mysql
  endif

#  ifneq ($(CONFIG_LCD4LINUX_CUSTOM_NEEDS_python),)
#    CONFIGURE_ARGS+= --with-python
#  else
    CONFIGURE_ARGS+= --without-python
#  endif

#  ifneq ($(CONFIG_LCD4LINUX_CUSTOM_NEEDS_libX11),)
#    CONFIGURE_ARGS+= --with-x
#  else
    CONFIGURE_ARGS+= --without-x
#  endif

endif

ifeq ($(BUILD_VARIANT),full)

  LCD4LINUX_FULL_DRIVERS:= $(strip $(foreach c, $(LCD4LINUX_DRIVERS), \
    $(c) \
  ))

  LCD4LINUX_FULL_PLUGINS:= $(strip $(foreach c, $(LCD4LINUX_PLUGINS), \
    $(c) \
  ))

  CONFIGURE_ARGS+= \
	--with-drivers="$(LCD4LINUX_FULL_DRIVERS)" \
	--with-plugins="$(LCD4LINUX_FULL_PLUGINS)" \
	--with-libiconv-prefix="$(ICONV_PREFIX)" \
	--without-python \
	--without-x

  EXTRA_LDFLAGS+= -L$(STAGING_DIR)/usr/lib/mysql

endif


define Package/lcd4linux/conffiles
/etc/lcd4linux.conf
endef

define Package/lcd4linux/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/lcd4linux $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/lcd4linux.conf.sample $(1)/etc/lcd4linux.conf
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/lcd4linux.init $(1)/etc/init.d/lcd4linux
	$(SED) "s|^\(Display 'GLCD2USB'\)|#\1|g" \
	    -e "s|^\(Layout 'TestLayer'\)|#\1|g" \
	    -e "s|^#\(Display 'Image'\)|\1|g" \
	    -e "s|^#\(Layout 'Default'\)|\1|g" \
	     $(1)/etc/lcd4linux.conf
endef

Package/lcd4linux-custom/conffiles = $(Package/lcd4linux/conffiles)
Package/lcd4linux-custom/install = $(Package/lcd4linux/install)

Package/lcd4linux-full/conffiles = $(Package/lcd4linux/conffiles)
Package/lcd4linux-full/install = $(Package/lcd4linux/install)

$(eval $(call BuildPackage,lcd4linux-custom))
$(eval $(call BuildPackage,lcd4linux-full))
