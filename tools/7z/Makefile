include $(TOPDIR)/rules.mk

PKG_NAME:=7z
PKG_VERSION:=22.00
PKG_SOURCE_VERSION:=2200

PKG_SOURCE:=$(PKG_NAME)$(PKG_SOURCE_VERSION)-src.tar.xz
PKG_SOURCE_URL:=https://7-zip.org/a/
PKG_HASH:=40969f601e86aff49aaa0ba0df5ce6fd397cf7e2683a84b591b0081e461ef675

# This builds the 7zr variant which supports only 7z, so no non-LGPL code should be included
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=DOC/License.txt DOC/copying.txt

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)

ALONE_DIR=$(HOST_BUILD_DIR)/CPP/7zip/Bundles/Alone7z

define Host/Compile
	$(MAKE) -C $(ALONE_DIR) -f makefile.gcc
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(ALONE_DIR)/_o/7zr $(STAGING_DIR_HOST)/bin/7zr
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/7zr
endef

$(eval $(call HostBuild))
