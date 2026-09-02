include $(TOPDIR)/rules.mk

PKG_NAME:=gnulib
PKG_CPE_ID:=cpe:/a:gnu:$(PKG_NAME)
PKG_VERSION:=c99c8d491850dc3a6e0b8604a2729d8bc5c0eff1# # stable-202401

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.savannah.gnu.org/cgit/$(PKG_NAME).git/snapshot
PKG_HASH:=8e6f4a907d9677b55fd452e1340a3e030a6f530b138d420c11975da33f086b1e

include $(INCLUDE_DIR)/host-build.mk

define Host/Configure
endef

define Host/Install
	$(INSTALL_DIR) $(1)/share/aclocal
	$(INSTALL_DATA) $(HOST_BUILD_DIR)/m4/*.m4 $(1)/share/aclocal/
	$(INSTALL_DIR) $(1)/share/gnulib
	$(CP) $(HOST_BUILD_DIR)/* $(1)/share/gnulib/
	ln -sf ../share/gnulib/gnulib-tool $(STAGING_DIR_HOST)/bin/gnulib-tool
endef

define Host/Clean
	rm -rf $(STAGING_DIR_HOST)/bin/gnulib-tool $(STAGING_DIR_HOST)/share/gnulib
endef

$(eval $(call HostBuild))
