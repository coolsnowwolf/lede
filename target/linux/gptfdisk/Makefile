include $(TOPDIR)/rules.mk

PKG_NAME:=gptfdisk
PKG_VERSION:=1.0.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.rodsbooks.com/gdisk/
PKG_HASH:=864c8aee2efdda50346804d7e6230407d5f42a8ae754df70404dd8b2fdfaeac7

HOST_BUILD_PARALLEL := 1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS += \
        MAKEFLAGS="$(HOST_JOBS)" \
        CXXFLAGS="$(HOST_CFLAGS)"

HOST_CONFIGURE_ARGS := \
        $(if $(MAKE_JOBSERVER),--parallel="$(MAKE_JOBSERVER)") \
        --prefix=$(STAGING_DIR_HOST)

define Host/Compile
        $(MAKE) LDFLAGS="$(HOST_LDFLAGS)" CXXFLAGS="$(HOST_CFLAGS) -I$(STAGING_DIR_HOST)/include/e2fsprogs" -C $(HOST_BUILD_DIR) sgdisk
endef

define Host/Install
        $(INSTALL_BIN) $(HOST_BUILD_DIR)/sgdisk $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
        rm -f $(STAGING_DIR_HOST)/bin/sgdisk
endef

HOSTCC := $(HOSTCC_NOCACHE)
HOSTCXX := $(HOSTCXX_NOCACHE)

$(eval $(call HostBuild))
