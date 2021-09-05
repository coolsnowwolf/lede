include $(TOPDIR)/rules.mk

PKG_NAME:=beanstalkd
PKG_VERSION:=1.9
PKG_RELEASE:=1

# for now, build from latest commit since releases are infrequent and
# useful fixes trickle in...
PKG_SOURCE_URL:=https://github.com/kr/beanstalkd.git
PKG_SOURCE_VERSION:=b7b4a6a14b7e8d096dc8cbc255b23be17a228cbb

PKG_SOURCE_PROTO:=git
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-v$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_VERSION).tar.xz
PKG_MIRROR_HASH:=6d5155ab347b731640495b1f0df6a4a849e84194d2ef0c2ec6af3879ee1aca2c
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-v$(PKG_VERSION)
PKG_HASH:=6d5155ab347b731640495b1f0df6a4a849e84194d2ef0c2ec6af3879ee1aca2c

PKG_MAINTAINER:=Philip Prindeville <philipp@redfish-solutions.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:wildbit:beanstalkd

include $(INCLUDE_DIR)/package.mk

define Package/beanstalkd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Beanstalk
endef

define Package/beanstalkd/description
 Beanstalk is a simple, fast work queue.
endef

define Build/Configure
endef

define Build/Compile
	cd $(PKG_BUILD_DIR) && make CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" PREFIX=/usr
endef

define Package/beanstalkd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/beanstalkd $(1)/usr/bin/beanstalkd

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/beanstalkd.init $(1)/etc/init.d/beanstalkd
endef

$(eval $(call BuildPackage,beanstalkd))
