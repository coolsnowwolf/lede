include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

VMLINUX:=$(BIN_DIR)/$(IMG_PREFIX)-vmlinux
UIMAGE:=$(BIN_DIR)/$(IMG_PREFIX)-uImage

ifeq ($(SUBTARGET),ox810se)
include ox810se.mk
endif

ifeq ($(SUBTARGET),ox820)
include ox820.mk
endif

$(eval $(call BuildImage))
