#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=ld.mold

include $(INCLUDE_DIR)/toolchain-build.mk

define Host/Configure
endef

define Host/Compile
endef

define Host/Install
	$(INSTALL_DIR) $(TOOLCHAIN_DIR)/bin
	$(INSTALL_BIN) $(STAGING_DIR_HOST)/bin/mold $(TOOLCHAIN_DIR)/bin/$(REAL_GNU_TARGET_NAME)-ld.mold
endef

define Host/Clean
endef

$(eval $(call HostBuild))
