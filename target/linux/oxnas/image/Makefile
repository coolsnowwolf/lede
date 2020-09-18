include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

VMLINUX:=$(BIN_DIR)/$(IMG_PREFIX)-vmlinux
UIMAGE:=$(BIN_DIR)/$(IMG_PREFIX)-uImage

include $(SUBTARGET).mk

$(eval $(call BuildImage))
