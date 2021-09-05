#
# Copyright (C) 2006-2012 OpenWrt.org
# Copyright (C) 2017-2019 Leonardo Medici
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libgphoto2
PKG_VERSION:=2.5.26
PKG_RELEASE:=3
PORT_VERSION:=0.12.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/gphoto
PKG_HASH:=7740fea3cbb78489071cdfec06c07fab528e69e27ac82734eae6a73eaeeabcd4

PKG_MAINTAINER:=Leonardo Medici <leonardo_medici@me.com>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_LIBTOOL_PATHS:=. libgphoto2_port
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libgphoto2/Default
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=http://www.gphoto.org/
endef

define Package/libgphoto2
  $(call Package/libgphoto2/Default)
  DEPENDS:=+libpthread +libltdl +libusb-compat +libusb-1.0 $(ICONV_DEPENDS) $(INTL_DEPENDS)
  TITLE:=The basic library of the gphoto2 program, version $(PKG_VERSION).
  MENU:=1
endef

define Package/libgphoto2-port
  $(call Package/libgphoto2/Default)
  DEPENDS:=libgphoto2 +libusb-1.0 +libusb-compat
  TITLE:=Gphoto2 drivers for connect cameras
endef

define Package/libgphoto2-drivers-adc65
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for adc65 cameras
endef

define Package/libgphoto2-drivers-agfa_cl20
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for agfa_cl20 cameras
endef

define Package/libgphoto2-drivers-aox
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for aox cameras
endef

define Package/libgphoto2-drivers-ax203
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for ax203 cameras
endef

define Package/libgphoto2-drivers-barbie
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for barbie cameras
endef

define Package/libgphoto2-drivers-canon
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for canon cameras
endef

define Package/libgphoto2-drivers-casio_qv
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for casio_qv cameras
endef

define Package/libgphoto2-drivers-clicksmart310
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for clicksmart310 cameras
endef

define Package/libgphoto2-drivers-digigr8
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for digigr8 cameras
endef

define Package/libgphoto2-drivers-digita
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for digita cameras
endef

define Package/libgphoto2-drivers-dimera3500
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for dimera3500 cameras
endef

define Package/libgphoto2-drivers-directory
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for directory cameras
endef

define Package/libgphoto2-drivers-enigma13
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for enigma13 cameras
endef

define Package/libgphoto2-drivers-fuji
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for fuji cameras
endef

define Package/libgphoto2-drivers-gsmart300
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for gsmart300 cameras
endef

define Package/libgphoto2-drivers-hp215
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for hp215 cameras
endef

define Package/libgphoto2-drivers-iclick
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for iclick cameras
endef

define Package/libgphoto2-drivers-jamcam
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for jamcam cameras
endef

define Package/libgphoto2-drivers-jd11
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for jd11 cameras
endef

define Package/libgphoto2-drivers-jl2005a
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for jl2005a cameras
endef

define Package/libgphoto2-drivers-jl2005c
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for jl2005c cameras
endef

define Package/libgphoto2-drivers-kodak_dc120
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for kodak_dc120 cameras
endef

define Package/libgphoto2-drivers-kodak_dc210
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for kodak_dc210 cameras
endef

define Package/libgphoto2-drivers-kodak_dc240
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for kodak_dc240 cameras
endef

define Package/libgphoto2-drivers-kodak_dc3200
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for kodak_dc3200 cameras
endef

define Package/libgphoto2-drivers-kodak_ez200
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for kodak_ez200 cameras
endef

define Package/libgphoto2-drivers-konica
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for konica cameras
endef

define Package/libgphoto2-drivers-konica_qm150
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for konica_qm150 cameras
endef

define Package/libgphoto2-drivers-largan
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for largan cameras
endef

define Package/libgphoto2-drivers-lg_gsm
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for lg_gsm cameras
endef

define Package/libgphoto2-drivers-mars
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for mars cameras
endef

define Package/libgphoto2-drivers-dimagev
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for dimagev cameras
endef

define Package/libgphoto2-drivers-mustek
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for mustek cameras
endef

define Package/libgphoto2-drivers-panasonic_coolshot
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for panasonic_coolshot cameras
endef

define Package/libgphoto2-drivers-panasonic_l859
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for panasonic_l859 cameras
endef

define Package/libgphoto2-drivers-panasonic_dc1000
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for panasonic_dc1000 cameras
endef

define Package/libgphoto2-drivers-panasonic_dc1580
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for panasonic_dc1580 cameras
endef

define Package/libgphoto2-drivers-pccam300
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for pccam300 cameras
endef

define Package/libgphoto2-drivers-pccam600
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for pccam600 cameras
endef

define Package/libgphoto2-drivers-pentax
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for pentax cameras
endef

define Package/libgphoto2-drivers-polaroid_pdc320
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for polaroid_pdc320 cameras
endef

define Package/libgphoto2-drivers-polaroid_pdc640
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for polaroid_pdc640 cameras
endef

define Package/libgphoto2-drivers-polaroid_pdc700
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for polaroid_pdc700 cameras
endef

define Package/libgphoto2-drivers-ptp2
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for ptp2 cameras
endef

define Package/libgphoto2-drivers-ricoh
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for ricoh cameras
endef

define Package/libgphoto2-drivers-ricoh_g3
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for ricoh_g3 cameras
endef

define Package/libgphoto2-drivers-samsung
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for samsung cameras
endef

define Package/libgphoto2-drivers-sierra
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for sierra cameras
endef

define Package/libgphoto2-drivers-sipix_blink2
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for sipix_blink2 cameras
endef

define Package/libgphoto2-drivers-sipix_web2
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for sipix_web2 cameras
endef

define Package/libgphoto2-drivers-smal
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for smal cameras
endef

define Package/libgphoto2-drivers-sonix
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for sonix cameras
endef

define Package/libgphoto2-drivers-sony_dscf1
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for sony_dscf1 cameras
endef

define Package/libgphoto2-drivers-sony_dscf55
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for sony_dscf55 cameras
endef

define Package/libgphoto2-drivers-soundvision
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for soundvision cameras
endef

define Package/libgphoto2-drivers-spca50x
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for spca50x cameras
endef

define Package/libgphoto2-drivers-sq905
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for sq905 cameras
endef

define Package/libgphoto2-drivers-st2205
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for st2205 cameras
endef

define Package/libgphoto2-drivers-stv0674
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for stv0674 cameras
endef

define Package/libgphoto2-drivers-stv0680
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for stv0680 cameras
endef

define Package/libgphoto2-drivers-sx330z
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for sx330z cameras
endef

define Package/libgphoto2-drivers-topfield
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for topfield cameras
endef

define Package/libgphoto2-drivers-toshiba_pdrm11
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for toshiba_pdrm11 cameras
endef

define Package/libgphoto2-drivers-tp6801
	$(call Package/libgphoto2/Default)
	DEPENDS:=+libgphoto2-port
	TITLE:=Gphoto2 drivers for tp6801 cameras
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-rpath \
	--with-camlibs="all,outdated" \
	--with-gdlib=no \
	--with-libexif=no \
	--with-libusb=no \
	--with-libusb-1.0=auto \
	--with-libxml-2.0=no \
	--without-included-ltdl \
	--without-jpeg \
	--without-libiconv-prefix \
	--without-libintl-prefix \

CONFIGURE_VARS += \
	CPPFLAGS="$$$$CPPFLAGS $(ICONV_CFLAGS)" \
	LDFLAGS="$$$$LDFLAGS $(ICONV_LDFLAGS)" \
	LIBUSB_CFLAGS="$$$$CPPFLAGS" \
	LIBUSB_LIBS="$$$$LDFLAGS -lusb" \
	LIBS="-lltdl" \

#	LIBEXIF_CFLAGS="$$$$CPPFLAGS" \
#	LIBEXIF_LIBS="$$$$LDFLAGS -lexif" \

# If OpenWrt is using the iconv stub, we disable iconv support
# in libgphoto2 entirely since the stub lacks some essential
# conversions like UCS-2 to UTF-8 which will let certain drivers
# fail with "Failed to create iconv converter" .
ifneq ($(ICONV_FULL),1)
  CONFIGURE_VARS += am_cv_func_iconv=no am_cv_lib_iconv=no
endif

MAKE_FLAGS += \
	LIBLTDL="" \

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gphoto2{,-port}-config $(1)/usr/bin/
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' $(1)/usr/bin/gphoto2{,-port}-config
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/gphoto2 $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2{,_port}.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libgphoto2{,_port}.pc $(1)/usr/lib/pkgconfig/
	# remove annoying recursive symlink
	rm -f $(1)/usr/include/gphoto2/gphoto2
	$(INSTALL_DIR) $(2)/bin
	$(LN) ../../usr/bin/gphoto2-config $(2)/bin/gphoto2-config
	$(LN) ../../usr/bin/gphoto2-port-config $(2)/bin/gphoto2-port-config
endef

define Package/libgphoto2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2{,_port}.so.* $(1)/usr/lib/
	$(LN) libgphoto2_port.so.12 $(1)/usr/lib/libgphoto2_port.so.10
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/print-camera-list $(1)/usr/lib/libgphoto2/print-camera-list
endef

define Package/libgphoto2-port/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2_port/$(PORT_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2_port/$(PORT_VERSION)/*.so $(1)/usr/lib/libgphoto2_port/$(PORT_VERSION)
endef

define Package/libgphoto2-drivers-adc65/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/adc65.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-agfa_cl20/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/agfa_cl20.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-aox/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/aox.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-ax203/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/ax203.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-barbie/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/barbie.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-canon/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/canon.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-casio_qv/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/casio_qv.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-clicksmart310/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/clicksmart310.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-digigr8/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/digigr8.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-digita/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/digita.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-dimera3500/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/dimera3500.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-directory/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/directory.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-enigma13/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/enigma13.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-fuji/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/fuji.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-gsmart300/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/gsmart300.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-hp215/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/hp215.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-iclick/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/iclick.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-jamcam/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/jamcam.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-jd11/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/jd11.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-jl2005a/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/jl2005a.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-jl2005c/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/jl2005c.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-kodak_dc120/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/kodak_dc120.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-kodak_dc210/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/kodak_dc210.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-kodak_dc240/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/kodak_dc240.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-kodak_dc3200/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/kodak_dc3200.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-kodak_ez200/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/kodak_ez200.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-konica/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/konica.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-konica_qm150/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/konica_qm150.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-largan/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/largan.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-lg_gsm/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/lg_gsm.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-mars/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/mars.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-dimagev/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/dimagev.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-mustek/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/mustek.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-panasonic_coolshot/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/panasonic_coolshot.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-panasonic_l859/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/panasonic_l859.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-panasonic_dc1000/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/panasonic_dc1000.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-panasonic_dc1580/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/panasonic_dc1580.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-pccam300/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/pccam300.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-pccam600/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/pccam600.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-pentax/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/pentax.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-polaroid_pdc320/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/polaroid_pdc320.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-polaroid_pdc640/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/polaroid_pdc640.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-polaroid_pdc700/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/polaroid_pdc700.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-ptp2/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/ptp2.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-ricoh/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/ricoh.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-ricoh_g3/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/ricoh_g3.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-samsung/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/samsung.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-sierra/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/sierra.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-sipix_blink2/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/sipix_blink2.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-sipix_web2/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/sipix_web2.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-smal/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/smal.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-sonix/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/sonix.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-sony_dscf1/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/sony_dscf1.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-sony_dscf55/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/sony_dscf55.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-soundvision/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/soundvision.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-spca50x/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/spca50x.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-sq905/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/sq905.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-st2205/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/st2205.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-stv0674/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/stv0674.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-stv0680/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/stv0680.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-sx330z/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/sx330z.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-topfield/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/topfield.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-toshiba_pdrm11/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/toshiba_pdrm11.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

define Package/libgphoto2-drivers-tp6801/install
	$(INSTALL_DIR) $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgphoto2/$(PKG_VERSION)/tp6801.so $(1)/usr/lib/libgphoto2/$(PKG_VERSION)
endef

$(eval $(call BuildPackage,libgphoto2))
$(eval $(call BuildPackage,libgphoto2-port))
$(eval $(call BuildPackage,libgphoto2-drivers-adc65))
$(eval $(call BuildPackage,libgphoto2-drivers-agfa_cl20))
$(eval $(call BuildPackage,libgphoto2-drivers-aox))
$(eval $(call BuildPackage,libgphoto2-drivers-ax203))
$(eval $(call BuildPackage,libgphoto2-drivers-barbie))
$(eval $(call BuildPackage,libgphoto2-drivers-canon))
$(eval $(call BuildPackage,libgphoto2-drivers-casio_qv))
$(eval $(call BuildPackage,libgphoto2-drivers-clicksmart310))
$(eval $(call BuildPackage,libgphoto2-drivers-digigr8))
$(eval $(call BuildPackage,libgphoto2-drivers-digita))
$(eval $(call BuildPackage,libgphoto2-drivers-dimera3500))
$(eval $(call BuildPackage,libgphoto2-drivers-directory))
$(eval $(call BuildPackage,libgphoto2-drivers-enigma13))
$(eval $(call BuildPackage,libgphoto2-drivers-fuji))
$(eval $(call BuildPackage,libgphoto2-drivers-gsmart300))
$(eval $(call BuildPackage,libgphoto2-drivers-hp215))
$(eval $(call BuildPackage,libgphoto2-drivers-iclick))
$(eval $(call BuildPackage,libgphoto2-drivers-jamcam))
$(eval $(call BuildPackage,libgphoto2-drivers-jd11))
$(eval $(call BuildPackage,libgphoto2-drivers-jl2005a))
$(eval $(call BuildPackage,libgphoto2-drivers-jl2005c))
$(eval $(call BuildPackage,libgphoto2-drivers-kodak_dc120))
$(eval $(call BuildPackage,libgphoto2-drivers-kodak_dc210))
$(eval $(call BuildPackage,libgphoto2-drivers-kodak_dc240))
$(eval $(call BuildPackage,libgphoto2-drivers-kodak_dc3200))
$(eval $(call BuildPackage,libgphoto2-drivers-kodak_ez200))
$(eval $(call BuildPackage,libgphoto2-drivers-konica))
$(eval $(call BuildPackage,libgphoto2-drivers-konica_qm150))
$(eval $(call BuildPackage,libgphoto2-drivers-largan))
$(eval $(call BuildPackage,libgphoto2-drivers-lg_gsm))
$(eval $(call BuildPackage,libgphoto2-drivers-mars))
$(eval $(call BuildPackage,libgphoto2-drivers-dimagev))
$(eval $(call BuildPackage,libgphoto2-drivers-mustek))
$(eval $(call BuildPackage,libgphoto2-drivers-panasonic_coolshot))
$(eval $(call BuildPackage,libgphoto2-drivers-panasonic_l859))
$(eval $(call BuildPackage,libgphoto2-drivers-panasonic_dc1000))
$(eval $(call BuildPackage,libgphoto2-drivers-panasonic_dc1580))
$(eval $(call BuildPackage,libgphoto2-drivers-pccam300))
$(eval $(call BuildPackage,libgphoto2-drivers-pccam600))
$(eval $(call BuildPackage,libgphoto2-drivers-pentax))
$(eval $(call BuildPackage,libgphoto2-drivers-polaroid_pdc320))
$(eval $(call BuildPackage,libgphoto2-drivers-polaroid_pdc640))
$(eval $(call BuildPackage,libgphoto2-drivers-polaroid_pdc700))
$(eval $(call BuildPackage,libgphoto2-drivers-ptp2))
$(eval $(call BuildPackage,libgphoto2-drivers-ricoh))
$(eval $(call BuildPackage,libgphoto2-drivers-ricoh_g3))
$(eval $(call BuildPackage,libgphoto2-drivers-samsung))
$(eval $(call BuildPackage,libgphoto2-drivers-sierra))
$(eval $(call BuildPackage,libgphoto2-drivers-sipix_blink2))
$(eval $(call BuildPackage,libgphoto2-drivers-sipix_web2))
$(eval $(call BuildPackage,libgphoto2-drivers-smal))
$(eval $(call BuildPackage,libgphoto2-drivers-sonix))
$(eval $(call BuildPackage,libgphoto2-drivers-sony_dscf1))
$(eval $(call BuildPackage,libgphoto2-drivers-sony_dscf55))
$(eval $(call BuildPackage,libgphoto2-drivers-soundvision))
$(eval $(call BuildPackage,libgphoto2-drivers-spca50x))
$(eval $(call BuildPackage,libgphoto2-drivers-sq905))
$(eval $(call BuildPackage,libgphoto2-drivers-st2205))
$(eval $(call BuildPackage,libgphoto2-drivers-stv0674))
$(eval $(call BuildPackage,libgphoto2-drivers-stv0680))
$(eval $(call BuildPackage,libgphoto2-drivers-sx330z))
$(eval $(call BuildPackage,libgphoto2-drivers-topfield))
$(eval $(call BuildPackage,libgphoto2-drivers-toshiba_pdrm11))
$(eval $(call BuildPackage,libgphoto2-drivers-tp6801))
