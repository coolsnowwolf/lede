include $(TOPDIR)/rules.mk

PKG_NAME:=gargoyle-ip-query
PKG_VERSION:=$(GARGOYLE_VERSION)
ifeq ($(GARGOYLE_VERSION),)
     PKG_VERSION:=1.0.0
endif
PKG_RELEASE:=2

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/gargoyle-ip-query
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+libericstools +ewget
	TITLE:=IP Query tool
	MAINTAINER:=Michael Gray <support@lantisproject.com>
endef

define Package/gargoyle-ip-query/description
	IP Query tool to find external IP address
	and GeoLocation, designed for use with
	Gargoyle Web Interface
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		STAGING_DIR="$(STAGING_DIR)" \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include" \
		LDFLAGS="$(TARGET_LDFLAGS) -L$(STAGING_DIR)/usr/lib" \
		USE_OPENSSL="0" \
		STATIC_LIBS="0" \
		all
endef

define Package/gargoyle-ip-query/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gipquery $(1)/usr/bin/gipquery
endef

$(eval $(call BuildPackage,gargoyle-ip-query))
