include $(TOPDIR)/rules.mk

PKG_NAME:=qca-ssdk-shell
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2022-01-24
PKG_SOURCE_URL:=https://source.codeaurora.org/quic/qsdk/oss/ssdk-shell
PKG_SOURCE_VERSION:=01a60b45ac3498a40bbb011d0c3b7d5d3a1cc9cb
PKG_MIRROR_HASH:=6233c02c296af241eb2e1c25e49dfc8d9c6cf1095cb82f1862b125dd125c758e

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/qca-ssdk-shell
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Shell application for QCA SSDK
  DEPENDS:=@(TARGET_ipq40xx||TARGET_ipq806x||TARGET_ipq807x)
endef

define Package/qca-ssdk-shell/Description
  This package contains a qca-ssdk shell application for QCA chipset
endef

ifndef CONFIG_TOOLCHAIN_BIN_PATH
CONFIG_TOOLCHAIN_BIN_PATH=$(TOOLCHAIN_DIR)/bin
endif

QCASSDK_CONFIG_OPTS+= \
	TOOL_PATH=$(CONFIG_TOOLCHAIN_BIN_PATH) \
	SYS_PATH=$(LINUX_DIR) \
	TOOLPREFIX=$(TARGET_CROSS) \
	KVER=$(LINUX_VERSION) \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	ARCH=$(LINUX_KARCH)

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(strip $(QCASSDK_CONFIG_OPTS))
endef

define Package/qca-ssdk-shell/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/bin/ssdk_sh $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,qca-ssdk-shell))
