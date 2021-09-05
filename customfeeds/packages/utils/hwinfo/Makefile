include $(TOPDIR)/rules.mk

PKG_NAME:=hwinfo
PKG_VERSION:=21.71
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/openSUSE/hwinfo/tar.gz/$(PKG_VERSION)?
PKG_HASH:=c4c573eb15cbc10103f5044b485d7e4ff941500ed559743a1c98e6a6deb0ebda

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Alberto Bursi <bobafetthotmail@gmail.com>

PKG_BUILD_DEPENDS:= hwinfo/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

# Execute in host build directory
HOST_MAKE_FLAGS += -C $(HOST_BUILD_DIR)

define Host/Configure
	# copy uuid.h to another location in host build dir as that's where this package expects it
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/include/uuid/
	$(CP) $(STAGING_DIR_HOST)/include/e2fsprogs/uuid/uuid.h $(STAGING_DIR_HOST)/include/uuid/uuid.h
endef

define Host/Compile
	# Build using host compiler and let it generate the files we need
	# CFLAGS, CPPFLAGS & LDFLAGS need to be passed with CC because they are being ingored
	CC="$(HOSTCC) $(HOST_CFLAGS) $(HOST_CPPFLAGS) $(HOST_LDFLAGS)" $(MAKE) $(HOST_MAKE_FLAGS)
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/share/hwinfo
	$(CP) $(HOST_BUILD_DIR)/src/isdn/cdb/isdn_cdb $(STAGING_DIR_HOST)/share/hwinfo/
	$(CP) $(HOST_BUILD_DIR)/src/isdn/cdb/mk_isdnhwdb $(STAGING_DIR_HOST)/share/hwinfo/
		
	$(CP) $(HOST_BUILD_DIR)/src/ids/check_hd $(STAGING_DIR_HOST)/share/hwinfo/
endef

define Build/Configure
	# Fetch prebuilt files from host build dir
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/src/isdn/cdb/
	$(CP) $(STAGING_DIR_HOST)/share/hwinfo/isdn_cdb $(PKG_BUILD_DIR)/src/isdn/cdb/
	$(CP) $(STAGING_DIR_HOST)/share/hwinfo/mk_isdnhwdb $(PKG_BUILD_DIR)/src/isdn/cdb/
		
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/src/ids/
	$(CP) $(STAGING_DIR_HOST)/share/hwinfo/check_hd $(PKG_BUILD_DIR)/src/ids/


	# Set copied files modtime to one day in the future
	# to prevent rebuilding them
	perl -e 'utime(time() + 86400, time() + 86400, $$$$ARGV[0])' \
		$(PKG_BUILD_DIR)/src/isdn/cdb/isdn_cdb
	perl -e 'utime(time() + 86400, time() + 86400, $$$$ARGV[0])' \
		$(PKG_BUILD_DIR)/src/isdn/cdb/mk_isdnhwdb
	perl -e 'utime(time() + 86400, time() + 86400, $$$$ARGV[0])' \
		$(PKG_BUILD_DIR)/src/ids/check_hd
endef


define Package/hwinfo
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=probe the hardware present in the system
  URL:=https://github.com/openSUSE/hwinfo
  DEPENDS:= +libuuid
endef

define Package/hwinfo/description
 hwinfo/libhd are used to probe for the hardware present in the system.
 It can be used to generate a system overview log which can be later 
 used for support.
 This project provides a hardware probing library libhd.so and a 
 command line tool hwinfo using it. A major project using this library
 is YaST, the SUSE installation tool.
endef


define Package/hwinfo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/hwinfo $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libhd.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,hwinfo))
$(eval $(call HostBuild))
