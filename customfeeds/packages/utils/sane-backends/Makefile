#
# Copyright (C) 2006-2016 OpenWrt.org
# Copyright (C) 2017-2020 Luiz Angelo Daros de Luca <luizluca@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sane-backends
PKG_VERSION:=1.0.32
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://gitlab.com/sane-project/backends/uploads/104f09c07d35519cc8e72e604f11643f
PKG_HASH:=3a28c237c0a72767086202379f6dc92dbb63ec08dfbab22312cba80e238bb114

PKG_MAINTAINER:=Luiz Angelo Daros de Luca <luizluca@gmail.com>
PKG_LICENSE:=GPL-2.0 GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING LICENSE
PKG_CPE_ID:=cpe:/a:sane-backends_project:sane-backends

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

SANE_BACKENDS:=

define Package/sane-backends/Default
  TITLE:=Scanner Access Now Easy
  URL:=http://www.sane-project.org/
endef

define Package/sane-backends/Default/description
 SANE (Scanner Access Now Easy) is a universal scanner interface.
endef

define Package/sane-backends
$(call Package/sane-backends/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+= (drivers)
  DEPENDS:=+ALL:sane-backends-all
endef

define Package/sane-backends/description
$(call Package/sane-backends/Default/description)
.
This
endef

define Package/sane-backends/config
    menu "Select SANE backends"
      depends on PACKAGE_sane-backends


    config PACKAGE_sane-backends-all
      depends on PACKAGE_sane-backends
      tristate "Include all SANE backends (sane-backends-all)"

    comment "Backends"

$(foreach backend,$(SANE_BACKENDS), \
    $(eval \
      $(call Package/Default)
      $(call Package/sane-$(backend))
    )\
    config PACKAGE_sane-$(backend)
      tristate "$(TITLE)"
     $(foreach config_dep,\
       $(filter @%,
         $(foreach v, $(DEPENDS), $(if $(findstring :,$v),,$v))
       ),\
      depends on $(strip $(subst @,,$(config_dep)))
     )
)
  endmenu

endef

# returns conditional dependency on sane backend
# 1: backend name
#
# If there is no config deps on $(1), returns +sane-$(1).
# Otherwise, +(CONFIG1&&CONFIG2&&..):sane-$(1)
define SaneBackendDependency
    $(eval \
        $(call Package/Default)
        $(call Package/sane-$(1))
        FILTER_CONFIG:=$$(strip \
            $$(foreach config_dep, \
                $$(filter @%, \
                    $$(foreach v, \
                        $$(DEPENDS), \
                        $$(if $$(findstring :,$$v),,$$v) \
                    ) \
                ), \
                $$(subst @,,$$(config_dep)) \
            ) \
        )
        ifneq (,$$(FILTER_CONFIG))
           FILTER_CONFIG:=($$(subst $$(space),&&,$$(FILTER_CONFIG))):
        endif
    ) \
    +$(FILTER_CONFIG)sane-$(1)
endef

define Package/sane-backends-all
$(call Package/sane-backends/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+= (all backends)
  DEPENDS:=$(foreach backend,$(SANE_BACKENDS),$(strip $(call SaneBackendDependency,$(backend))))
  HIDDEN:=1
endef

define Package/sane-backends-all/description
$(call Package/sane-backends/Default/description)
.
Metapackage for selecting all SANE Backends
endef

define Package/sane-daemon
$(call Package/sane-backends/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libsane
  TITLE+= (network daemon)
  USERID:=saned:scanner
endef

define Package/sane-daemon/description
$(call Package/sane-backends/Default/description)
This package contains the SANE daemon.
endef

# Run hotplug to grant access to existing scanners
define Package/sane-daemon/postinst
#!/bin/sh
# We only need udevtrigger to have the magic work without
# rebooting on a real system.  In ImageBuilder or
# the like, the boot process will call udevtrigger.
if [ -z "$${IPKG_INSTROOT}" ]; then
	udevtrigger
fi
endef

define Package/libsane
  $(call Package/sane-backends/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS+=+libusb-1.0
  TITLE+= (libraries)
endef

define Package/libsane/description
$(call Package/sane-backends/Default/description)
This package contains the SANE shared libraries.
endef

define Package/sane-frontends
  $(call Package/sane-backends/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libsane +libjpeg +libpng
  TITLE+= (frontends)
endef

define Package/sane-frontends/description
$(call Package/sane-backends/Default/description)
This package contains the SANE frontends.
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-preload \
	--without-snmp \
	--without-avahi \
	--disable-locking \
	--without-usb-record-replay \

# ./configure does not even try to detect mmap if crosscompiling
CONFIGURE_VARS += \
    ac_cv_func_mmap_fixed_mapped="yes"

define Build/Configure
	mkdir -p $(PKG_BUILD_DIR)/backend/.libs
	$(Build/Configure/Default)
endef

define Build/Install
	$(call Build/Install/Default)
	mkdir -p $(PKG_INSTALL_DIR)/usr/share/sane
	$(foreach file, $(filter-out %/unsupported.desc ,$(wildcard $(PKG_BUILD_DIR)/doc/descriptions/*.desc)),
		sed -rn -e '/^:usbid[[:blank:]]+"?0x(....)"?[[:blank:]]+"?0x(....)"?.*/{s//\1 \2/;p}' $(file) | \
			sort -u > $(PKG_INSTALL_DIR)/usr/share/sane/$(basename $(notdir $(file))).usbid; \
		awk '{ print $$$$2 > "$(PKG_INSTALL_DIR)/usr/share/sane/"$$$$1"-$(basename $(notdir $(file))).usbid" }' \
			$(PKG_INSTALL_DIR)/usr/share/sane/$(basename $(notdir $(file))).usbid ;
	)
endef

define Package/libsane/install
	$(INSTALL_DIR) $(1)/usr/lib/sane/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsane.so.* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/sane/libsane-dll.so.* $(1)/usr/lib/sane/
	$(INSTALL_DIR) $(1)/etc/sane.d/dll.d
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/sane.d/dll.conf $(1)/etc/sane.d/
	chmod 0644 $(1)/etc/sane.d/dll.conf
endef

define Package/libsane/conffiles
/etc/sane.d/dll.conf
endef

define Package/sane-daemon/install
	$(INSTALL_DIR) $(1)/etc/sane.d
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/sane.d/saned.conf $(1)/etc/sane.d/
	chmod 0644 $(1)/etc/sane.d/saned.conf
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/saned.sbin $(1)/usr/sbin/saned
	$(INSTALL_DIR) $(1)/usr/lib/sane/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/saned $(1)/usr/lib/sane/saned
	$(INSTALL_DIR) $(1)/etc/xinetd.d
	$(INSTALL_CONF) ./files/saned.xinetd $(1)/etc/xinetd.d/sane-port
	$(INSTALL_DIR) $(1)/etc/hotplug.d/usb/
	$(INSTALL_BIN) ./files/saned.hotplug $(1)/etc/hotplug.d/usb/20-saned
endef

define Package/sane-daemon/conffiles
/etc/sane.d/saned.conf
/etc/xinetd.d/sane-port
endef

define Package/sane-frontends/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

define Package/sane-backends-all/install
	/bin/true
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/sane $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsane.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/tools/sane-backends.pc $(1)/usr/lib/pkgconfig/
endef

# 1: short name
# 2: description
# 3: dependencies on other packages
define SaneBackend
  SANE_BACKENDS+= $(1)

  define Package/sane-$(1)
    $(call Package/sane-backends/Default)
    TITLE:=SANE backend (sane-$(1)) for $(2)
    DEPENDS+= +libsane $(3)
    HIDDEN:=1
  endef

  define Package/sane-$(1)/description
   $(call Package/sane-backends/Default/description)
.
This package contains the SANE backend for $(2).
  endef

  define Package/sane-$(1)/install
	if [ -f "$(PKG_INSTALL_DIR)/etc/sane.d/$(1).conf" ]; then \
		$(INSTALL_DIR) $$(1)/etc/sane.d ; \
		$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/sane.d/$(1).conf $$(1)/etc/sane.d/ ; \
	        chmod 0644 $$(1)/etc/sane.d/$(1).conf ; \
	fi ; \
	$(INSTALL_DIR) $$(1)/usr/lib/sane ; \
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/sane/libsane-$(1).so.* $$(1)/usr/lib/sane/ ; \
	if [ -d "$(PKG_INSTALL_DIR)/usr/share/sane/$(1)" ]; then \
		$(INSTALL_DIR) $$(1)/usr/share/sane/ ; \
		$(CP) -a $(PKG_INSTALL_DIR)/usr/share/sane/$(1) $$(1)/usr/share/sane/ ; \
	fi ; \
	if [ -s "$(PKG_INSTALL_DIR)/usr/share/sane/$(1).usbid" ]; then \
		$(INSTALL_DIR) $$(1)/usr/share/sane/ ; \
		$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/sane/????-$(1).usbid $$(1)/usr/share/sane/ ; \
	fi
  endef

  define Package/sane-$(1)/conffiles
/etc/sane.d/$(1).conf
  endef

  $$(eval $$(call BuildPackage,sane-$(1)))
endef

$(eval $(call BuildPackage,libsane))
$(eval $(call BuildPackage,sane-daemon))
$(eval $(call SaneBackend,abaton,Abaton flatbed scanners))
$(eval $(call SaneBackend,agfafocus,AGFA Focus flatbed scanners))
$(eval $(call SaneBackend,apple,Apple flatbed scanners))
$(eval $(call SaneBackend,artec,Artec flatbed scanners))
$(eval $(call SaneBackend,artec_eplus48u,Artec E+ 48U and re-badged models,+libusb-1.0))
$(eval $(call SaneBackend,as6e,Artec AS6E parallel port interface scanner))
$(eval $(call SaneBackend,avision,Avision and Avision OEM scanners and possibly more flatbed and film scanners,+libusb-1.0))
$(eval $(call SaneBackend,bh,Bell+Howell Copiscan II series document scanners))
$(eval $(call SaneBackend,canon,Canon SCSI scanners,+libusb-1.0))
$(eval $(call SaneBackend,canon630u,the Canon 630u USB flatbed scanner,+libusb-1.0))
$(eval $(call SaneBackend,canon_dr,Canon DR-series scanners,+libusb-1.0))
# Depends on libieee1284
#$(eval $(call SaneBackend,canon_pp,Canon CanoScan Parallel Port flatbed scanners,+libieee1284))
$(eval $(call SaneBackend,canon_lide70,Canon LiDE 70 USB flatbed scanner,))
$(eval $(call SaneBackend,cardscan,Corex CardScan usb scanners,+libusb-1.0))
$(eval $(call SaneBackend,coolscan,Nikon film-scanners,+libusb-1.0))
$(eval $(call SaneBackend,coolscan2,Nikon Coolscan film scanners,+libusb-1.0))
$(eval $(call SaneBackend,coolscan3,Nikon Coolscan film scanners,+libusb-1.0))
$(eval $(call SaneBackend,dc210,Kodak DC210 Digital Camera,+libjpeg))
$(eval $(call SaneBackend,dc240,Kodak DC240 Digital Camera,+libjpeg))
$(eval $(call SaneBackend,dc25,Kodak DC20/DC25 Digital Cameras))
$(eval $(call SaneBackend,dell1600n_net,Dell 1600n,+libtiff +libjpeg))
$(eval $(call SaneBackend,dmc,Polaroid Digital Microscope Camera))
$(eval $(call SaneBackend,epjitsu,Epson-based Fujitsu USB scanners,+libusb-1.0))
$(eval $(call SaneBackend,epson,EPSON scanners,+libusb-1.0))
$(eval $(call SaneBackend,epsonds,EPSON scanners,+libjpeg +libusb-1.0)) ####
$(eval $(call SaneBackend,epson2,EPSON scanners,+libusb-1.0))
#$(eval $(call SaneBackend,escl,AirScan/eSCL devices,+libcurl +libxml2 +libnetsnmp +libavahi))
$(eval $(call SaneBackend,fujitsu,Fujitsu flatbed and ADF scanners,+libusb-1.0))
$(eval $(call SaneBackend,genesys,GL646 GL841 GL843 GL847 and GL124 based USB flatbed scanners,+libstdcpp +libusb-1.0 +libtiff))
$(eval $(call SaneBackend,gphoto2,digital cameras supported by gphoto2,+libjpeg +libgphoto2))
$(eval $(call SaneBackend,gt68xx,GT-68XX based USB flatbed scanners,+libusb-1.0))
$(eval $(call SaneBackend,hp,HP ScanJet scanners,+libusb-1.0))
$(eval $(call SaneBackend,hp3500,Hewlett-Packard ScanJet 3500 series scanners,+libusb-1.0))
$(eval $(call SaneBackend,hp3900,RTS8822 chipset based scanners,+libtiff +libusb-1.0))
$(eval $(call SaneBackend,hp4200,Hewlett-Packard 4200 scanners,+libusb-1.0))
$(eval $(call SaneBackend,hp5400,Hewlett-Packard 54XX scanners,+libusb-1.0))
$(eval $(call SaneBackend,hp5590,Hewlett-Packard 4500C/4570C/5500C/5550C/5590/7650 Workgroup/Document scanners,+libusb-1.0))
$(eval $(call SaneBackend,hpljm1005,Hewlett-Packard LaserJet M1005 MFP Scanner,+libusb-1.0))
# Depends on libieee1284
#$(eval $(call SaneBackend,hpsj5s,HP ScanJet 5S sheet-fed scanner,+libieee1284))
$(eval $(call SaneBackend,hs2p,Ricoh SCSI flatbed/ADF scanners))
$(eval $(call SaneBackend,ibm,IBM and Ricoh SCSI flatbed scanners))
$(eval $(call SaneBackend,kodak,big Kodak flatbed and ADF scanners,+libusb-1.0))
$(eval $(call SaneBackend,kodakaio,Kodak aio printer / scanners,+libusb-1.0))
$(eval $(call SaneBackend,kvs1025,Panasonic KV-S102xC USB ADF scanners,+libusb-1.0))
$(eval $(call SaneBackend,kvs20xx,Panasonic KV-S20xxC USB/SCSI ADF scanners,+libusb-1.0))
$(eval $(call SaneBackend,kvs40xx,Panasonic KV-S40xxC USB/SCSI ADF scanners,+libpthread +libusb-1.0))
$(eval $(call SaneBackend,leo,LEO Technologies scanners))
$(eval $(call SaneBackend,lexmark,Lexmark X1100/X1200 Series scanners,+libusb-1.0))
$(eval $(call SaneBackend,ma1509,Mustek BearPaw 1200F USB scanner,+libusb-1.0))
$(eval $(call SaneBackend,magicolor,KONICA MINOLTA magicolor scanners,+libusb-1.0))
$(eval $(call SaneBackend,matsushita,Panasonic KV-SS high speed scanners))
$(eval $(call SaneBackend,microtek,Microtek scanners))
$(eval $(call SaneBackend,microtek2,Microtek scanners with SCSI-2 command set))
$(eval $(call SaneBackend,mustek,Mustek SCSI flatbed scanners and some other devices))
# Depends on libieee1284
#$(eval $(call SaneBackend,mustek_pp,Mustek parallel port flatbed scanners,+libieee1284))
$(eval $(call SaneBackend,mustek_usb,Mustek USB flatbed scanners,+libusb-1.0))
$(eval $(call SaneBackend,mustek_usb2,SQ113 based USB flatbed scanners,+libpthread +libusb-1.0))
$(eval $(call SaneBackend,nec,NEC scanners))
$(eval $(call SaneBackend,net,network backend))
$(eval $(call SaneBackend,niash,scanners based on the NIASH chipset,+libusb-1.0))
$(eval $(call SaneBackend,p5,the Primax PagePartner))
$(eval $(call SaneBackend,pie,PIE Devcom and AdLib SCSI flatbed scanners))
$(eval $(call SaneBackend,pieusb,PIE Devcom and AdLib SCSI flatbed scanners,+libusb-1.0))
# requires sys/scanio.h
#$(eval $(call SaneBackend,pint,))
$(eval $(call SaneBackend,pixma,Canon Multi-Function Printers and CanoScan Scanners,+libusb-1.0 +libjpeg))
$(eval $(call SaneBackend,plustek,LM983[1/2/3] based USB flatbed scanners,+libusb-1.0))
$(eval $(call SaneBackend,plustek_pp,Plustek parallel port flatbed scanners))
$(eval $(call SaneBackend,qcam,Connectix QuickCam cameras,@(TARGET_x86||TARGET_x86_64)))
$(eval $(call SaneBackend,ricoh,Ricoh flatbed scanners))
$(eval $(call SaneBackend,ricoh2,Ricoh2 flatbed scanners,+libusb-1.0))
$(eval $(call SaneBackend,rts8891,rts8891 based scanners,+libusb-1.0))
$(eval $(call SaneBackend,s9036,Siemens 9036 flatbed scanners))
$(eval $(call SaneBackend,sceptre,SCEPTRE scanners))
$(eval $(call SaneBackend,sharp,SHARP scanners))
$(eval $(call SaneBackend,sm3600,Microtek scanners with M011 USB chip,+libusb-1.0))
$(eval $(call SaneBackend,sm3840,Microtek scanners with SCAN08 USB chip,+libusb-1.0))
$(eval $(call SaneBackend,snapscan,AGFA SnapScan flatbed scanners,+libusb-1.0))
$(eval $(call SaneBackend,sp15c,Fujitsu ScanPartner 15C flatbed scanner))
$(eval $(call SaneBackend,st400,Siemens ST/Highscan flatbed scanners))
$(eval $(call SaneBackend,stv680,STV680 cameras,+libusb-1.0))
$(eval $(call SaneBackend,tamarack,Tamarack flatbed scanners))
$(eval $(call SaneBackend,teco1,TECO / RELISYS scanners))
$(eval $(call SaneBackend,teco2,TECO / RELISYS scanners))
$(eval $(call SaneBackend,teco3,TECO / RELISYS scanners))
$(eval $(call SaneBackend,test,testing frontends))
$(eval $(call SaneBackend,u12,Plustek USB flatbed scanners based on older parport designs,+libusb-1.0))
$(eval $(call SaneBackend,umax,UMAX scanners,+libusb-1.0))
$(eval $(call SaneBackend,umax1220u,the UMAX Astra 1220U and similar scanners,+libusb-1.0))
$(eval $(call SaneBackend,umax_pp,Umax Astra parallel port flatbed scanners))
$(eval $(call SaneBackend,v4l,Video for Linux API,+libv4l +libjpeg +libusb-1.0))
$(eval $(call SaneBackend,xerox_mfp,Xerox Phaser 3200MFP device,+libjpeg +libusb-1.0))
$(eval $(call BuildPackage,sane-backends))
$(eval $(call BuildPackage,sane-backends-all))
$(eval $(call BuildPackage,sane-frontends))
