include $(TOPDIR)/rules.mk

PKG_NAME:=prometheus
PKG_VERSION:=2.25.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/prometheus/prometheus/tar.gz/v${PKG_VERSION}?
PKG_HASH:=85f50c0cfb4db206a59d2c3301e02d685c3fe4b451b41ca943a4eb94935cf4d4

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Paul Spooren <mail@aparcar.org>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/prometheus/prometheus/
GO_PKG_BUILD_PKG:=github.com/prometheus/prometheus/cmd/prometheus/

GO_PKG_LDFLAGS_X:=\
	github.com/prometheus/common/version.Version=v$(PKG_VERSION) \
	github.com/prometheus/common/version.Revision=$(PKG_VERSION)  \
	github.com/prometheus/common/version.Branch="release" \
	github.com/prometheus/common/version.BuildUser=openwrt \
	github.com/prometheus/common/version.BuildDate=$(SOURCE_DATE_EPOCH)

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/prometheus/Default
  TITLE:=Monitoring system & time series database
  USERID:=prometheus=112:prometheus=112
  URL:=http://prometheus.io
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/prometheus
$(call Package/prometheus/Default)
  SECTION:=utils
  CATEGORY:=Utilities
endef

define Package/prometheus/description
Prometheus, a Cloud Native Computing Foundation project, is a systems and
service monitoring system. It collects metrics from configured targets at given
intervals, evaluates rule expressions, displays the results, and can trigger
alerts if some condition is observed to be true.
endef

define Package/prometheus/install
	$(call GoPackage/Package/Install/Bin,$(1))

	$(CP) ./files/* $(1)/
endef

define Package/prometheus/conffiles
/etc/prometheus.yml
endef

$(eval $(call GoBinPackage,prometheus))
$(eval $(call BuildPackage,prometheus))
