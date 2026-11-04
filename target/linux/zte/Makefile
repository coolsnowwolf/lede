# SPDX-License-Identifier: GPL-2.0-only
include $(TOPDIR)/rules.mk

BOARD:=zte
BOARDNAME:=ZTE SoCs
SUBTARGETS:=zx279133
FEATURES:=dt gpio ramdisk cpiogz squashfs usb
KERNEL_PATCHVER:=7.3
DEVICE_TYPE:=basic

include $(INCLUDE_DIR)/target.mk
include $(TOPDIR)/target/linux/zte/sr1010.mk
KERNEL_FILE_DEPENDS += $(SR1010_FIRMWARE_DIR)/zx279133-npu.bin

define Kernel/Prepare
	$(call Kernel/Prepare/Default)
	$(INSTALL_DIR) $(LINUX_DIR)/firmware/zte/zx279133
	$(CP) $(SR1010_FIRMWARE_DIR)/zx279133-npu.bin \
		$(LINUX_DIR)/firmware/zte/zx279133/mcode_intel.bin
endef

define Target/Description
	Build images for the ZTE ZX279133 SoC family.
endef

$(eval $(call BuildTarget))
