include $(TOPDIR)/rules.mk

PKG_NAME:=7z
PKG_VERSION:=24.05

PKG_SOURCE:=$(PKG_NAME)$(subst .,,$(PKG_VERSION))-src.tar.xz
PKG_SOURCE_URL:=https://7-zip.org/a/
PKG_HASH:=63f341cf80b8d287c6e945519b3da0fa75553c85572a471b7fa6e68f9a90b790

PKG_CPE_ID:=cpe:/a:7-zip:7-zip

# This builds the 7zr variant which supports only 7z, so no non-LGPL code should be included
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=DOC/License.txt DOC/copying.txt

HOST_MAKE_PATH:=CPP/7zip/Bundles/Alone7z

include $(INCLUDE_DIR)/host-build.mk

TAR_OPTIONS := -C $(HOST_BUILD_DIR) $(TAR_OPTIONS)
HOST_MAKE_FLAGS += -f makefile.gcc

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/$(HOST_MAKE_PATH)/_o/7zr $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/7zr
endef

$(eval $(call HostBuild))
