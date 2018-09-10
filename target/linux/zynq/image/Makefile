#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

#################################################
# Default and templates
#################################################

define Device/Default
	PROFILES := Default
	KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
	KERNEL_INITRAMFS_PREFIX := $$(IMG_PREFIX)-$(1)-initramfs
	KERNEL_PREFIX := $$(IMAGE_PREFIX)
	KERNEL_LOADADDR := 0x8000
endef

define Device/FitImageGzip
	KERNEL_SUFFIX := -fit-uImage.itb
	KERNEL = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL_NAME := Image
endef

#################################################
# Devices
#################################################

define Device/digilent_zynq-zybo
	$(call Device/FitImageGzip)
	DEVICE_TITLE := ZYBO Development Board
	DEVICE_DTS := zynq-zybo
endef
TARGET_DEVICES += digilent_zynq-zybo

define Device/digilent_zynq-zybo-z7
	$(call Device/FitImageGzip)
	DEVICE_TITLE := Digilent Zybo Z7 board
	DEVICE_DTS := zynq-zybo-z7
endef
TARGET_DEVICES += digilent_zynq-zybo-z7

define Device/xlnx_zynq-zc702
	$(call Device/FitImageGzip)
	DEVICE_TITLE := ZC702 Development Board
	DEVICE_DTS := zynq-zc702
endef
TARGET_DEVICES += xlnx_zynq-zc702

define Device/xlnx_zynq-zed
	$(call Device/FitImageGzip)
	DEVICE_TITLE := Zed Development Board
	DEVICE_DTS := zynq-zed
endef
TARGET_DEVICES += xlnx_zynq-zed

$(eval $(call BuildImage))
