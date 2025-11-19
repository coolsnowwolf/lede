include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=fogvdn
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_HASH:=skip

ifeq ($(ARCH),x86_64)
	PKG_ARCH:=X64
else ifeq ($(ARCH),arm)
	PKG_ARCH:=ARM
else ifeq ($(ARCH),aarch64)
	PKG_ARCH:=ARM64
endif

PKG_SOURCE:=fogvdn_PEAR_$(PKG_ARCH)_LINUX_latest.tar.gz
PKG_SOURCE_URL:=https://download.openfogos.com/release/

include $(INCLUDE_DIR)/package.mk

PKG_UNPACK:=$(HOST_TAR) -C $(PKG_BUILD_DIR)/root --strip-components=1 -xzf $(DL_DIR)/$(PKG_SOURCE)

define Package/fogvdn
  SECTION:=net
  CATEGORY:=Network
  TITLE:=OpenFog Pear P2P CDN
  DEPENDS:=@(aarch64||arm||x86_64) \
	+bash +getopt +jq +ntpdate +whereis \
	+smartmontools +zoneinfo-all +coreutils \
	+coreutils-install +coreutils-df +coreutils-nohup
endef

define Package/fogvdn/description
OpenFog Pear Peer-to-Peer Content Delivery Network
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/root
	$(PKG_UNPACK)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/fogvdn/install
	$(CP) $(PKG_BUILD_DIR)/root/* $(1)/
endef

$(eval $(call BuildPackage,fogvdn))
