include $(TOPDIR)/rules.mk

PKG_NAME:=gnulib
PKG_CPE_ID:=cpe:/a:gnu:$(PKG_NAME)

PKG_SOURCE_URL=git://git.git.savannah.gnu.org/$(PKG_NAME).git
PKG_SOURCE_DATE:=2026-07-04
PKG_SOURCE_VERSION:=b22f5a3037712a3c957a03071ce0b219cef4d65b# # stable-202507
PKG_MIRROR_HASH:=d1423a784794e3e08bd03b397544821ece257aa748566386bd97219d1edf98d5

include $(INCLUDE_DIR)/host-build.mk

define Host/Configure
endef

define Host/Install
	$(call Host/Uninstall)
	$(INSTALL_DIR) $(1)/share/aclocal
	$(INSTALL_DATA) $(HOST_BUILD_DIR)/m4/*.m4 $(1)/share/aclocal/
	$(CP) $(HOST_BUILD_DIR)/ $(1)/share/gnulib/
	ln -sf ../share/gnulib/gnulib-tool $(STAGING_DIR_HOST)/bin/gnulib-tool
endef

define Host/Uninstall
	rm -rf $(STAGING_DIR_HOST)/bin/gnulib-tool $(STAGING_DIR_HOST)/share/gnulib
	rm -f $(STAGING_DIR_HOST)/share/aclocal/gl_*.m4
endef

$(eval $(call HostBuild))
