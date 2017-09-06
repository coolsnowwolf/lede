include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

include $(SUBTARGET).mk

define Image/Build
	$(call Image/Build/$(1),$(1))
endef

$(eval $(call BuildImage))
