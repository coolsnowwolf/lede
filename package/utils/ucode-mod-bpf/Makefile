include $(TOPDIR)/rules.mk

PKG_NAME:=ucode-mod-bpf
PKG_RELEASE:=1
PKG_LICENSE:=ISC
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/ucode-mod-bpf
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=ucode eBPF module
  DEPENDS:=+libucode +libbpf
endef

define Package/ucode-mod-bpf/description
The bpf plugin provides functionality for loading and interacting with
eBPF modules.

It allows loading full modules and pinned maps/programs and supports
interacting with maps and attaching programs as tc classifiers.
endef

define Package/ucode-mod-bpf/install
	$(INSTALL_DIR) $(1)/usr/lib/ucode
	$(CP) $(PKG_BUILD_DIR)/bpf.so $(1)/usr/lib/ucode/
endef

define Build/Configure
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) $(FPIC) \
		-Wall -ffunction-sections -Wl,--gc-sections -shared -Wl,--no-as-needed -lbpf \
		-o $(PKG_BUILD_DIR)/bpf.so $(PKG_BUILD_DIR)/bpf.c
endef

$(eval $(call BuildPackage,ucode-mod-bpf))
