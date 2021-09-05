#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=unrar
PKG_VERSION:=6.0.6
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=unrarsrc-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.rarlab.com/rar
PKG_HASH:=011ef7290d3394a62bb5bfced914cd510a7eea7255cf69417f9c952bb6056588
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/unrar

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>, \
		Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=UnRAR
PKG_LICENSE_FILES:=license.txt
PKG_CPE_ID:=cpe:/a:rarlab:unrar

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

TARGET_CXXFLAGS +=-fno-rtti -flto
TARGET_LDFLAGS +=$(FPIC) -Wl,--gc-sections $(if $(CONFIG_USE_GLIBC),-lpthread)

define Package/unrar/Default
  TITLE:=UnRAR
  SUBMENU:=Compression
  URL:=https://www.rarlab.com/
  DEPENDS:=+libstdcpp +libpthread
endef

define Package/unrar
$(call Package/unrar/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+= (application)
  VARIANT:=bin
endef

define Package/libunrar
$(call Package/unrar/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= (library)
  VARIANT:=lib
endef

define Package/unrar/description
  UnRAR is an application that can decompress files and archives created using
  the RAR compression scheme
endef

define Package/libunrar/description
  UnRAR library is a shared library that provides file extraction from RAR
  archives
endef

ifeq ($(BUILD_VARIANT),lib)
define Build/Compile
	$(call Build/Compile/Default,lib)
endef

define Build/Install
	$(call Build/Install/Default,install-lib)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/unrar
	$(CP) $(PKG_BUILD_DIR)/*.hpp $(1)/usr/include/unrar/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/libunrar.so $(1)/usr/lib/
endef
else
define Build/Compile
	$(call Build/Compile/Default,unrar)
endef

define Build/Install
	$(call Build/Install/Default,install-unrar)
endef
endif

define Package/unrar/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/unrar $(1)/usr/bin/
endef

define Package/libunrar/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/libunrar.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,unrar))
$(eval $(call BuildPackage,libunrar))
