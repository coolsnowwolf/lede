include $(TOPDIR)/rules.mk

PKG_NAME:=usbgadget
PKG_RELEASE:=1

PKG_LICENSE:=BSD-2-Clause

PKG_MAINTAINER:=Chuanhong Guo <gch981213@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=@USB_GADGET_SUPPORT +kmod-usb-gadget +kmod-usb-lib-composite
  TITLE:=init script to create USB gadgets
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/init.d
	$(INSTALL_CONF) ./files/usbgadget.conf $(1)/etc/config/usbgadget
	$(INSTALL_BIN) ./files/usbgadget.init $(1)/etc/init.d/usbgadget
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

# 1: short name
# 2: description
# 3: dependencies on other packages
define GadgetPreset
  define Package/$(PKG_NAME)-$(1)
    SECTION:=utils
    CATEGORY:=Utilities
    TITLE+= $(2) gadget preset
    DEPENDS+= $(3)
  endef

  define Package/$(PKG_NAME)-$(1)/description
   This package contains the USB gadget preset for $(3).
  endef

  define Package/$(PKG_NAME)-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/share/usbgadget
	$(INSTALL_CONF) ./files/presets/$(1) $$(1)/usr/share/usbgadget
  endef

  $$(eval $$(call BuildPackage,$(PKG_NAME)-$(1)))
endef

$(eval $(call GadgetPreset,ncm,CDC-NCM,+kmod-usb-gadget-ncm))
$(eval $(call GadgetPreset,acm,CDC-ACM,+kmod-usb-gadget-serial))
