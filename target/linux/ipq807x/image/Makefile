include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

IPQ807X_KERNEL_LOADADDR = 0x41080000
DEVICE_DTS_CONFIG = "config@hk01"

define Image/BuildKernel/FIT
	gzip -9 -c $(KDIR)/Image > $(KDIR)/Image.gz
	$(call CompressLzma,$(KDIR)/Image,$(KDIR)/Image.gz)
	$(call Image/BuildKernel/MkFIT,$(1), $(KDIR)/Image.gz, $(DTS_DIR)/qcom/$(1).dtb,gzip,$(2),$(2))
	$(CP) $(KDIR)/fit-$(1).itb $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fit-uImage.itb
	mkdir -p $(BIN_DIR)/dtbs/
	$(CP) $(DTS_DIR)/qcom/ipq*.dtb $(BIN_DIR)/dtbs/
	$(CP) $(KDIR)/Image $(BIN_DIR)/dtbs/
endef

define Image/BuildKernel/FITInitramfs
	$(CP) $(KDIR)/Image-initramfs $(BIN_DIR)/dtbs/
	$(CP) $(KDIR)/Image-initramfs $(BIN_DIR)/$(IMG_PREFIX)-vmlinux-initramfs.bin
	$(call Image/BuildKernel/MkFIT,$(1), $(KDIR)/Image-initramfs, $(DTS_DIR)/qcom/$(1).dtb, none,$(2),$(2),-initramfs)
	$(CP) $(KDIR)/fit-$(1)-initramfs.itb $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fit-uImage-initramfs.itb
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
endef

define Image/BuildKernel
	$(call Image/BuildKernel/FIT,ipq8074-hk01,$(IPQ807X_KERNEL_LOADADDR))
endef

define Image/BuildKernel/Initramfs
	$(call Image/BuildKernel/FITInitramfs,ipq8074-hk01,$(IPQ807X_KERNEL_LOADADDR))
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
endef

$(eval $(call BuildImage))
