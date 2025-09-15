include $(TOPDIR)/rules.mk

PKG_NAME:=ninja
PKG_VERSION:=1.12.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ninja-build/ninja/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=821bdff48a3f683bc4bb3b6f0b5fe7b2d647cf65d52aeb63328c91a6c6df285a

include $(INCLUDE_DIR)/host-build.mk

define Host/Configure
	cd $(HOST_BUILD_DIR) && \
		$(HOST_MAKE_VARS) \
		CXX="$(HOSTCXX_NOCACHE)" \
		$(STAGING_DIR_HOST)/bin/$(PYTHON) configure.py \
			$(if $(shell $(STAGING_DIR_HOST)/bin/ninja --version),,--bootstrap) \
			--no-rebuild \
			--verbose
	-$(Host/Install)
endef

define Host/Compile
	+$(NINJA) -C $(HOST_BUILD_DIR)
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/ninja $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/ninja
endef

$(eval $(call HostBuild))
