include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=grub-efi

include ../common.mk

TAR_OPTIONS:= --transform 's/grub-${PKG_VERSION}/${PKG_NAME}-${PKG_VERSION}/' $(TAR_OPTIONS)

PKG_BUILD_DEPENDS:=grub2-efi/host

CONFIGURE_ARGS += --with-platform=efi
HOST_CONFIGURE_ARGS += --with-platform=efi --program-suffix=-efi
HOST_BUILD_DIR := $(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)

define Package/grub2-efi
$(call Package/grub2/Default)
HIDDEN:=1
TITLE += (with EFI support)
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,grub2-efi))
