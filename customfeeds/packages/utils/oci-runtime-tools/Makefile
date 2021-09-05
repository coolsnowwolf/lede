include $(TOPDIR)/rules.mk

PKG_NAME:=oci-runtime-tools
PKG_VERSION:=1.0.0-pre20210326
PKG_RELEASE:=$(AUTORELEASE)
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_PROTO:=git
PKG_SOURCE_NAME:=runtime-tools
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/opencontainers/runtime-tools.git
PKG_SOURCE_VERSION:=59cdde06764be8d761db120664020f0415f36045
PKG_MIRROR_HASH:=ef331e3ad5f99220583399fe1c2ded243f4b01f8dabc7acd152d46304da70f6f

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/opencontainers/runtime-tools/
GO_PKG_LDFLAGS_X:=main.gitCommit=$(PKG_SOURCE_VERSION) main.version=$(PKG_SOURCE_VERSION)

BUSYBOX_STATIC_VERSION:=1.31.0

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Download/busybox-static-armv8l
  URL:=https://busybox.net/downloads/binaries/$$(BUSYBOX_STATIC_VERSION)-defconfig-multiarch-musl/
  URL_FILE:=busybox-armv8l
  FILE:=busybox-$(BUSYBOX_STATIC_VERSION)-static-armv8l
  HASH:=141adb1b625a6f44c4b114f76b4387b4ea4f7ab802b88eb40e0d2f6adcccb1c3
endef

define Package/oci-runtime-tool
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=oci-runtime-tool
  URL:=https://github.com/opencontainers/runtime-tools
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/oci-runtime-tool/description
  OCI (Open Container Initiative) runtime tools
  Generate OCI runtime spec configuration files and validate OCI bundles.
endef

define Package/oci-runtime-tests
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=OCI runtimetest tool
  URL:=https://github.com/opencontainers/runtime-tools
  DEPENDS:=@(aarch64||i386||i686||x86_64) oci-runtime-tool +tar
endef

define Package/oci-runtime-tests/description
  OCI (Open Container Initiative) runtime tools
  This package provides test cases as well as artifacts required to
  validate an OCI runtime.
  Best used in combination with a TAP consumer like 'node-tap',
  installable via npm.
endef

GO_PKG_BUILD_VARS += GO111MODULE=auto

define Build/Prepare
	$(call Build/Prepare/Default)
ifdef CONFIG_aarch64
	$(eval $(call Download,busybox-static-armv8l))
	( \
		mkdir -p $(PKG_BUILD_DIR)/rootfs-armv8l ;\
		cd $(PKG_BUILD_DIR)/rootfs-armv8l ;\
		$(TAR) xzf ../rootfs-amd64.tar.gz ;\
		$(CP) $(DL_DIR)/busybox-$(BUSYBOX_STATIC_VERSION)-static-armv8l bin/busybox ;\
		chmod 0755 bin/busybox ;\
		$(TAR) czf ../rootfs-arm64.tar.gz . ;\
	)
endif
endef

define Build/Compile
	$(call GoPackage/Build/Compile)
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))
endef

define Package/oci-runtime-tool/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/oci-runtime-tool $(1)/usr/bin
endef

define Package/oci-runtime-tests/install
	$(INSTALL_DIR) $(1)/usr/libexec/oci-runtime-test
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/runtimetest $(1)/usr/libexec/oci-runtime-test/
	( cd $(PKG_BUILD_DIR) ; $(FIND) ./validation/ -name *.go | grep -v util | \
	xargs -I'%' -n 1 basename % .go | while read testbin; do \
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$$$${testbin} \
			$(1)/usr/libexec/oci-runtime-test/$$$${testbin}.t ; \
	done )
ifdef CONFIG_x86
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/rootfs-386.tar.gz $(1)/usr/libexec/oci-runtime-test
ifdef CONFIG_x86_64
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/rootfs-amd64.tar.gz $(1)/usr/libexec/oci-runtime-test
endif
endif
ifdef CONFIG_aarch64
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/rootfs-arm64.tar.gz $(1)/usr/libexec/oci-runtime-test
endif
endef

$(eval $(call GoBinPackage,oci-runtime-tools))
$(eval $(call BuildPackage,oci-runtime-tool))
$(eval $(call BuildPackage,oci-runtime-tests))
