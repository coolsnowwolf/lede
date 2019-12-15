include $(TOPDIR)/rules.mk

PKG_NAME:=gptfdisk
PKG_VERSION:=1.0.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.rodsbooks.com/gdisk/
PKG_HASH:=864c8aee2efdda50346804d7e6230407d5f42a8ae754df70404dd8b2fdfaeac7

HOST_BUILD_PARALLEL := 1

include $(INCLUDE_DIR)/host-build.mk

HOST_CPPFLAGS += \
	-I$(STAGING_DIR_HOST)/include/e2fsprogs

define Host/Compile
	$(call Host/Compile/Default,sgdisk)
endef

define Host/Install
        $(INSTALL_BIN) $(HOST_BUILD_DIR)/sgdisk $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
        rm -f $(STAGING_DIR_HOST)/bin/sgdisk
endef

$(eval $(call HostBuild))
