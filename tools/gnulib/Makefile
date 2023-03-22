include $(TOPDIR)/rules.mk

PKG_NAME:=gnulib
PKG_CPE_ID:=cpe:/a:gnu:$(PKG_NAME)
PKG_VERSION:=f9a4ee73c3e7b544f640d0d04b55983d3a7b894e# # master

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.savannah.gnu.org/cgit/$(PKG_NAME).git/snapshot
PKG_HASH:=514716d58987a9c0de0d69fb22d42bcd19edf80eed099882a004ff162060f1a8

include $(INCLUDE_DIR)/host-build.mk

define Host/Configure
endef

define Host/Install
	$(INSTALL_DIR) $(1)/share/gnulib
	$(CP) $(HOST_BUILD_DIR)/* $(1)/share/gnulib/
endef

define Host/Clean
	rm -rf $(STAGING_DIR_HOST)/share/gnulib
endef

$(eval $(call HostBuild))
