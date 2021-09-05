# Copyright (C) 2020 Lienol <lawlienol@gmail.com>

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-nginx-pingos
PKG_VERSION:=1.19.6
PKG_RELEASE:=1

PKG_SOURCE:=nginx-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://nginx.org/download/
PKG_HASH:=b11195a02b1d3285ddf2987e02c6b6d28df41bb1b1dd25f33542848ef4fc33b5

PKG_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_DIR:=$(PKG_DIR)/nginx-$(PKG_VERSION)

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  TITLE:=PingOS server
  PKGARCH:=all
  URL:=https://pingos.io/
  DEPENDS:=+libpcre +libopenssl +zlib +libpthread
endef

define Package/$(PKG_NAME)/conffiles
endef

define Build/Prepare
	rm -r $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	tar -zxvf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_DIR)
	$(CP) -pR ./modules $(PKG_BUILD_DIR)/modules
	$(call Build/Prepare/Default,)
endef

ADDITIONAL_MODULES:= --with-http_ssl_module \
	--add-module=./modules/nginx-rtmp-module \
	--add-module=./modules/nginx-client-module \
	--add-module=./modules/nginx-multiport-module \
	--add-module=./modules/nginx-toolkit-module

TARGET_CFLAGS += -fvisibility=hidden -ffunction-sections -fdata-sections -DNGX_LUA_NO_BY_LUA_BLOCK
TARGET_LDFLAGS += -Wl,--gc-sections

CONFIGURE_ARGS += \
			--crossbuild=Linux::$(ARCH) \
			--prefix=/usr \
			--conf-path=/usr/share/pingos/conf/nginx.conf \
			$(ADDITIONAL_MODULES) \
			--error-log-path=/var/etc/pingos/error.log \
			--pid-path=/var/etc/pingos/pingos.pid \
			--lock-path=/var/etc/pingos/pingos.lock \
			--http-log-path=/var/etc/pingos/access.log \
			--http-client-body-temp-path=/var/etc/pingos/lib/body \
			--http-proxy-temp-path=/var/etc/pingos/lib/proxy \
			--with-cc="$(TARGET_CC)" \
			--with-cc-opt="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
			--with-ld-opt="$(TARGET_LDFLAGS)" \
			--without-http_upstream_zone_module

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) ./root/etc/pingos.template $(1)/etc/pingos.template
	
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./root/etc/config/pingos $(1)/etc/config/pingos
	
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/pingos $(1)/etc/init.d/pingos
	
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_CONF) ./root/etc/uci-defaults/* $(1)/etc/uci-defaults
	
	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	cp -pR ./root/usr/share/rpcd/acl.d/* $(1)/usr/share/rpcd/acl.d

	$(INSTALL_DIR) $(1)/usr/share/pingos
	cp -pR $(PKG_INSTALL_DIR)/usr/share/pingos/conf $(1)/usr/share/pingos
	$(INSTALL_DATA) ./root/resource/conf-template/nginx.conf $(1)/usr/share/pingos/conf/nginx.conf
	
	cp -pR $(PKG_INSTALL_DIR)/usr/html $(1)/usr/share/pingos/html
	$(INSTALL_DATA) ./root/resource/crossdomain.xml $(1)/usr/share/pingos/html/crossdomain.xml
	$(INSTALL_DATA) ./root/resource/stat.xsl $(1)/usr/share/pingos/html/stat.xsl
	
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci/
	
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/pingos.po $(1)/usr/lib/lua/luci/i18n/pingos.zh-cn.lmo
	
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/nginx $(1)/usr/sbin/pingos
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
